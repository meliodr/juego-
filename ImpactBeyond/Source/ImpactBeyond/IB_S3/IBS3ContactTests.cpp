#if WITH_DEV_AUTOMATION_TESTS
#include "IBS3Character.h"
#include "IBS3World.h"
#include "IBS3Geometry.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Math/RotationMatrix.h"
#include "Misc/AutomationTest.h"
#include "Misc/App.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"

namespace
{
AIBS3Character* FindPilot()
{
    for(const FWorldContext& C:GEngine->GetWorldContexts())
        if(C.World() && (C.WorldType==EWorldType::Game || C.WorldType==EWorldType::PIE))
            if(auto* P=Cast<AIBS3Character>(UGameplayStatics::GetPlayerPawn(C.World(),0))) return P;
    return nullptr;
}
FName ObjectKey(AIBS3PlanetWorld* W,const TCHAR* Kind)
{
    return FName(*(W->Manifest->BodyId.ToString()+TEXT("/")+W->Manifest->Cities[0].RegionId.ToString()+TEXT("/")+Kind+TEXT("01")));
}
AIBS3Prop* FindProp(UWorld* W,FName Key,int32* Count=nullptr)
{
    AIBS3Prop* Found=nullptr; int32 N=0;
    for(TActorIterator<AIBS3Prop> It(W);It;++It)
        if(!It->IsActorBeingDestroyed() && It->State.Key==Key) { Found=*It; ++N; }
    if(Count) *Count=N;
    return Found;
}
void PlaceFixture(AIBS3Character* P,FVector Position,FVector Forward)
{
    P->GetCharacterMovement()->StopMovementImmediately();
    const FVector Up=(Position-P->Planet->GetActorLocation()).GetSafeNormal();
    P->SetActorLocationAndRotation(Position,FRotationMatrix::MakeFromXZ(Forward,Up).ToQuat(),false,nullptr,ETeleportType::TeleportPhysics);
    P->TangentForward=Forward; P->PreviousUp=Up;
    P->Planet->UpdateResidency(Position,0); P->PrepareMovement();
}
}

// Each site has one EXPLICIT fixture placement, then real world ticks, falling, contact and walking.
// No position writes during the measured fall/walk. This does not claim travel between the sites.
class FIBS3ContactCommand : public IAutomationLatentCommand
{
    FAutomationTestBase* Test;
    int32 Site=0; bool Placed=false,SawFalling=false; double Started=0,LandedAt=0;
    FVector WalkStart;
public:
    explicit FIBS3ContactCommand(FAutomationTestBase* In):Test(In) {}
    bool Update() override
    {
        AIBS3Character* P=FindPilot();
        if(!IsValid(P) || !IsValid(P->Planet)) { Test->AddError(TEXT("S3 player/planet unavailable")); return true; }
        auto* W=P->Planet.Get(); auto* M=P->GetCharacterMovement();
        if(!W->IsOperational()) { Test->AddError(TEXT("Planet physics not operational")); return true; }
        if(!Placed)
        {
            IBS3::Geo G;
            if(Site<4) { const auto& C=W->Manifest->Cities[Site]; G={FMath::DegreesToRadians(C.LatitudeDegrees),FMath::DegreesToRadians(C.LongitudeDegrees),C.SurfaceOffsetCm+2000}; }
            else G={Site==4?IBS3::Pi/2:-IBS3::Pi/2,0,2000};
            const auto V=IBS3::position(G,W->Manifest->RadiusCm); const auto F=IBS3::frame(G);
            PlaceFixture(P,W->GetActorLocation()+FVector(V.x,V.y,V.z),FVector(F.north.x,F.north.y,F.north.z));
            M->SetMovementMode(MOVE_Falling); Started=P->GetWorld()->GetTimeSeconds(); LandedAt=0; SawFalling=false; Placed=true;
            Test->AddInfo(FString::Printf(TEXT("Contact fixture %d placed 20m above surface; subsequent motion uses CMC"),Site));
        }
        const double Now=P->GetWorld()->GetTimeSeconds();
        const FVector Up=(P->GetActorLocation()-W->GetActorLocation()).GetSafeNormal();
        SawFalling |= M->IsFalling() && FVector::DotProduct(P->GetVelocity(),Up)<-50;
        if(M->IsMovingOnGround() && LandedAt==0)
        {
            LandedAt=Now; WalkStart=P->GetActorLocation();
            Test->TestTrue(TEXT("Observed actual inward falling velocity"),SawFalling);
            Test->TestTrue(TEXT("CMC reports walkable floor contact"),M->CurrentFloor.IsWalkableFloor());
            Test->TestTrue(TEXT("Contact floor normal radial"),FVector::DotProduct(M->CurrentFloor.HitResult.ImpactNormal,Up)>0.99);
            Test->TestTrue(TEXT("Capsule up aligned with radial normal"),FVector::DotProduct(P->GetActorUpVector(),Up)>0.999);
            // Turn right on the actual floor, then move through the normal movement input path.
            P->TangentForward=FVector::CrossProduct(Up,P->TangentForward).GetSafeNormal();
            if(auto* PC=Cast<APlayerController>(P->GetController())) PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::W,IE_Pressed,1.f));
        }
        if(LandedAt>0)
        {
            if(Now-LandedAt>1.5)
            {
                Test->TestTrue(TEXT("Walked at least 4m after landing and turning"),FVector::Distance(WalkStart,P->GetActorLocation())>400);
                Test->TestTrue(TEXT("Maintained floor after walking"),M->IsMovingOnGround());
                Test->AddInfo(FString::Printf(TEXT("Contact site %d: fall/walk %.3fs, altitude %.3fcm, floor=%s"),Site,Now-Started,
                    FVector::Distance(P->GetActorLocation(),W->GetActorLocation())-W->Manifest->RadiusCm,*GetNameSafe(M->CurrentFloor.HitResult.GetActor())));
                if(auto* PC=Cast<APlayerController>(P->GetController())) PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::W,IE_Released,0.f));
                P->ConsumeMovementInputVector(); M->StopMovementImmediately(); Placed=false;
                if(++Site==6) return true;
            }
        }
        if(Now-Started>15) { Test->AddError(FString::Printf(TEXT("Landing timed out at site %d; mode=%d, location=%s"),Site,int32(M->MovementMode),*P->GetActorLocation().ToString())); return true; }
        return false;
    }
};
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIBS3ContactsTest,"ImpactBeyond.S3.Contacts",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FIBS3ContactsTest::RunTest(const FString& Parameters)
{
    ADD_LATENT_AUTOMATION_COMMAND(FIBS3ContactCommand(this)); return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIBS3RestartWriteTest,"ImpactBeyond.S3.RestartWrite",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FIBS3RestartWriteTest::RunTest(const FString& Parameters)
{
    auto* P=FindPilot();
    if(!TestNotNull(TEXT("S3 player"),P) || !TestNotNull(TEXT("S3 planet"),P->Planet.Get())) return false;
    auto* W=P->Planet.Get();
    if(!TestTrue(TEXT("Explicit unique QA slot required"),W->SaveSlot.StartsWith(TEXT("IB_S3_QA_Restart_")))) return false;
    if(!TestFalse(TEXT("Never overwrite an existing restart fixture"),UGameplayStatics::DoesSaveGameExist(W->SaveSlot,0))) return false;
    for(const TCHAR* Kind:{TEXT("wall"),TEXT("car"),TEXT("object")})
    {
        auto* Prop=FindProp(P->GetWorld(),ObjectKey(W,Kind));
        if(!TestNotNull(TEXT("Physical representative exists"),Prop)) return false;
        if(Prop->State.Kind==TEXT("wall")) Prop->Interact(FVector::ForwardVector);
        else
        {
            // Deliberate QA displacement, persisted before this process exits.
            Prop->SetActorLocation(Prop->GetActorLocation()+Prop->GetActorRightVector()*700,false,nullptr,ETeleportType::TeleportPhysics);
            Prop->Mesh->SetPhysicsLinearVelocity(FVector::ZeroVector); Prop->Mesh->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);
        }
    }
    W->DestinationCity=W->Manifest->Cities[2].CityId;
    W->Discovered.Add(W->Manifest->Cities[3].CityId);
    TestTrue(TEXT("Write USaveGame for next process"),W->SaveState());
    AddInfo(TEXT("Restart write complete; only another process can prove RestartVerify."));
    return !HasAnyErrors();
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIBS3RestartVerifyTest,"ImpactBeyond.S3.RestartVerify",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FIBS3RestartVerifyTest::RunTest(const FString& Parameters)
{
    auto* P=FindPilot();
    if(!TestNotNull(TEXT("S3 player"),P) || !TestNotNull(TEXT("S3 planet"),P->Planet.Get())) return false;
    auto* W=P->Planet.Get();
    if(!TestTrue(TEXT("Explicit QA restart slot required"),W->SaveSlot.StartsWith(TEXT("IB_S3_QA_Restart_")))) return false;
    auto* Save=Cast<UIBS3SaveGame>(UGameplayStatics::LoadGameFromSlot(W->SaveSlot,0));
    if(!TestNotNull(TEXT("Previous-process save deserialized"),Save)) return false;
    TestEqual(TEXT("Destination restored by BeginPlay"),W->DestinationCity,W->Manifest->Cities[2].CityId);
    TestTrue(TEXT("Discovered city restored by BeginPlay"),W->Discovered.Contains(W->Manifest->Cities[3].CityId));
    for(const TCHAR* Kind:{TEXT("wall"),TEXT("car"),TEXT("object")})
    {
        const FName Key=ObjectKey(W,Kind); int32 Count=0;
        auto* Prop=FindProp(P->GetWorld(),Key,&Count);
        TestEqual(TEXT("One actor per saved key after process restart"),Count,1);
        const auto* Delta=Save->Deltas.FindByPredicate([Key](const FIBS3ObjectDelta& D){return D.Key==Key;});
        if(!TestNotNull(TEXT("Saved row"),Delta) || !TestNotNull(TEXT("Hydrated actor"),Prop)) return false;
        // Chaos may settle a movable body between BeginPlay and automation startup. Bound that motion.
        const double Error=FVector::Distance(Prop->GetActorLocation()+W->OriginOffset(),Delta->GlobalTransform.GetLocation());
        TestTrue(TEXT("Restored global transform within 150cm settling tolerance"),Error<150);
        if(Prop->State.Kind==TEXT("wall")) TestTrue(TEXT("Broken wall and absent collision survived restart"),Prop->State.bBroken && Prop->Mesh->GetCollisionEnabled()==ECollisionEnabled::NoCollision);
        AddInfo(FString::Printf(TEXT("Restart %s global transform delta %.3fcm"),Kind,Error));
    }
    return !HasAnyErrors();
}

// Uses the renderer's screenshot path, never desktop pixels or synthetic imagery.
class FIBS3CaptureCommand : public IAutomationLatentCommand
{
    FAutomationTestBase* Test; int32 Stage=0; double Started=0; bool Requested=false;
    FString File;
    const FString RunId=FGuid::NewGuid().ToString(EGuidFormats::Digits);
public:
    explicit FIBS3CaptureCommand(FAutomationTestBase* In):Test(In) {}
    bool Update() override
    {
        auto* P=FindPilot(); if(!IsValid(P) || !IsValid(P->Planet)) { Test->AddError(TEXT("S3 player missing")); return true; }
        if(!FApp::CanEverRender()) { Test->AddError(TEXT("VisualEvidence requires a rendered process, not nullrhi")); return true; }
        const double Now=P->GetWorld()->GetTimeSeconds();
        if(Started==0)
        {
            auto* W=P->Planet.Get(); const auto& City=W->Manifest->Cities[Stage<4?Stage:0];
            const auto Frame=IBS3::frame({FMath::DegreesToRadians(City.LatitudeDegrees),FMath::DegreesToRadians(City.LongitudeDegrees)});
            const FVector Up(Frame.up.x,Frame.up.y,Frame.up.z),North(Frame.north.x,Frame.north.y,Frame.north.z);
            const double Height=Stage==6?W->Manifest->RadiusCm*2:Stage==5?80000:Stage==4?10000:350;
            PlaceFixture(P,W->CityPosition(City)+Up*Height-North*6000,North);
            P->GetCharacterMovement()->SetMovementMode(Stage<4?MOVE_Falling:MOVE_Flying); P->bGlobalMap=Stage==4;
            P->CameraPitch=Stage==6?-85:Stage==5?-45:-12;
            File=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("IB_S3_QA/Captures")/RunId/FString::Printf(TEXT("S3-%d.png"),Stage));
            IFileManager::Get().MakeDirectory(*FPaths::GetPath(File),true); Started=Now;
            Test->AddInfo(FString::Printf(TEXT("Visual QA fixture %d; file %s"),Stage,*File));
        }
        if(Now-Started>4 && !Requested) { FScreenshotRequest::RequestScreenshot(File,true,false,false,FIntRect(),true); Requested=true; }
        if(Requested && !FScreenshotRequest::IsScreenshotRequested() && IFileManager::Get().FileSize(*File)>0)
        {
            Test->AddInfo(TEXT("Rendered capture written: ")+File);
            if(++Stage==7) return true;
            Started=0; Requested=false;
            return false;
        }
        if(Now-Started>30) { Test->AddError(TEXT("Renderer screenshot timed out: ")+File); return true; }
        return false;
    }
};
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIBS3VisualTest,"ImpactBeyond.S3.VisualEvidence",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FIBS3VisualTest::RunTest(const FString& Parameters)
{
    ADD_LATENT_AUTOMATION_COMMAND(FIBS3CaptureCommand(this)); return true;
}
#endif
