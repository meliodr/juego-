#if WITH_DEV_AUTOMATION_TESTS
#include "IBS3Character.h"
#include "IBS3World.h"
#include "IBS3Geometry.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Math/RotationMatrix.h"
#include "Misc/AutomationTest.h"

class FIBS3FlightCommand : public IAutomationLatentCommand
{
    FAutomationTestBase* Test; bool Loop;
    TWeakObjectPtr<AIBS3Character> Pilot;
    int32 Phase=0,Returns=0,Crossings=0,BrakeFrames=0;
    double Started=0,PhaseAt=0,LastAt=0,Angle=0,MaxAltitudeError=0,PeakSpeed=0,OldSpeed=0,LastProgress=0;
    FVector Initial,Previous,BrakeStart,CarMoved;
    double BrakeInitialSpeed=0;
    FName LastRegion,CarKey,WallKey;
    AIBS3Prop* Prop(FName Key,int32& Count)
    {
        AIBS3Prop* Found=nullptr; Count=0;
        for(TActorIterator<AIBS3Prop> It(Pilot->GetWorld());It;++It)
            if(!It->IsActorBeingDestroyed() && It->State.Key==Key) { Found=*It; ++Count; }
        return Found;
    }
    bool Finish()
    {
        if(Pilot.IsValid())
        {
            Pilot->BrakeOff(); Pilot->bTraversal=false; Pilot->ConsumeMovementInputVector();
            Pilot->GetCharacterMovement()->StopMovementImmediately();
            if(IsValid(Pilot->Planet)) Pilot->Planet->Manifest->TraversalSpeedCmS=OldSpeed;
        }
        return true;
    }
    void Brake(int32 NextPhase,double Now)
    {
        BrakeStart=Pilot->GetActorLocation(); BrakeInitialSpeed=Pilot->GetVelocity().Size();
        BrakeFrames=0;
        Pilot->BrakeOn(); Phase=NextPhase; PhaseAt=Now;
    }
    void CheckBrake()
    {
        const double Distance=FVector::Distance(BrakeStart,Pilot->GetActorLocation());
        const double Bound=BrakeInitialSpeed*BrakeInitialSpeed/(2*Pilot->Planet->Manifest->BrakeCmS2)+2000;
        Test->TestTrue(TEXT("Brake acts over multiple physical frames, not an instantaneous velocity reset"),BrakeInitialSpeed<10000 || BrakeFrames>2);
        Test->TestTrue(TEXT("Actual stop remains within v^2/(2a) plus 20m tick allowance"),Distance<=Bound);
        Test->AddInfo(FString::Printf(TEXT("Brake from %.3fm/s: actual displacement %.3fm; bound %.3fm"),BrakeInitialSpeed/100,Distance/100,Bound/100));
    }
public:
    FIBS3FlightCommand(FAutomationTestBase* In,bool InLoop):Test(In),Loop(InLoop) {}
    bool Update() override
    {
        if(!Pilot.IsValid() && Started==0)
        {
            for(const FWorldContext& C:GEngine->GetWorldContexts())
                if(C.World() && C.WorldType==EWorldType::Game) Pilot=Cast<AIBS3Character>(UGameplayStatics::GetPlayerPawn(C.World(),0));
            if(!Pilot.IsValid() || !IsValid(Pilot->Planet)) { Test->AddError(TEXT("S3 game player not found")); return true; }
            auto* W=Pilot->Planet.Get(); OldSpeed=W->Manifest->TraversalSpeedCmS;
            W->Manifest->TraversalSpeedCmS=Loop?27000:32000;
            const FVector Up=FVector::ForwardVector,Forward=-FVector::RightVector;
            Initial=W->GetActorLocation()+Up*(W->Manifest->RadiusCm+50000);
            Pilot->SetActorLocationAndRotation(Initial,FRotationMatrix::MakeFromXZ(Forward,Up).ToQuat(),false,nullptr,ETeleportType::TeleportPhysics);
            Pilot->PreviousUp=Up; Pilot->TangentForward=Forward; Pilot->bTraversal=true; Pilot->BrakeOff();
            Pilot->GetCharacterMovement()->StopMovementImmediately(); Pilot->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
            W->UpdateResidency(Initial,0); Pilot->PrepareMovement(); Previous=Initial;
            Started=PhaseAt=LastAt=Pilot->GetWorld()->GetTimeSeconds(); LastRegion=W->RegionAt(Initial);
            const FString Prefix=W->Manifest->BodyId.ToString()+TEXT("/")+W->Manifest->Cities[0].RegionId.ToString()+TEXT("/");
            CarKey=FName(*(Prefix+TEXT("car01"))); WallKey=FName(*(Prefix+TEXT("wall01")));
            if(!Loop)
            {
                int32 Count=0; auto* Car=Prop(CarKey,Count); auto* Wall=Prop(WallKey,Count);
                if(!Car || !Wall) { Test->AddError(TEXT("Missing physical reentry fixtures")); return Finish(); }
                CarMoved=Car->GetActorLocation()+Car->GetActorRightVector()*700;
                Car->SetActorLocation(CarMoved,false,nullptr,ETeleportType::TeleportPhysics);
                Car->Mesh->SetPhysicsLinearVelocity(FVector::ZeroVector); Wall->Interact(Forward);
            }
            Test->AddInfo(TEXT("One initial QA fixture placement at altitude 500m. All measured travel thereafter uses flight inputs and swept CMC; no pose writes."));
        }
        if(!Pilot.IsValid() || !IsValid(Pilot->Planet)) { Test->AddError(TEXT("Player destroyed during measured flight")); return true; }
        auto* W=Pilot->Planet.Get(); const FVector P=Pilot->GetActorLocation(),Center=W->GetActorLocation();
        const double Now=Pilot->GetWorld()->GetTimeSeconds(),Dt=Now-LastAt,Speed=Pilot->GetVelocity().Size();
        const FVector A=(Previous-Center).GetSafeNormal(),B=(P-Center).GetSafeNormal();
        const double StepAngle=FMath::Atan2(FVector::CrossProduct(A,B).Size(),FVector::DotProduct(A,B));
        Angle+=StepAngle; MaxAltitudeError=FMath::Max(MaxAltitudeError,FMath::Abs(FVector::Distance(P,Center)-W->Manifest->RadiusCm-50000));
        PeakSpeed=FMath::Max(PeakSpeed,Speed);
        if((Phase==1 || Phase==3) && Speed>1) ++BrakeFrames;
        if(FVector::Distance(Previous,P)>W->Manifest->TraversalSpeedCmS*FMath::Max(Dt,0.0)+1000)
        { Test->AddError(TEXT("Discontinuous displacement exceeded physical speed/time bound")); return Finish(); }
        if(MaxAltitudeError>20 || FVector::DotProduct(Pilot->GetActorUpVector(),B)<0.999)
        { Test->AddError(FString::Printf(TEXT("Radial flight coherence failed: altitude error %.3fcm"),MaxAltitudeError)); return Finish(); }
        const FName Region=W->RegionAt(P);
        if(Region!=LastRegion) { ++Crossings; LastRegion=Region; Test->AddInfo(TEXT("Actual region crossing: ")+Region.ToString()); }
        Previous=P; LastAt=Now;
        if(Now-LastProgress>30)
        {
            LastProgress=Now;
            UE_LOG(LogTemp,Display,TEXT("IB_S3 flight QA: elapsed=%.1fs angle=%.3fdeg returns=%d speed=%.3fm/s altitude_error=%.4fcm"),Now-Started,FMath::RadiansToDegrees(Angle),Returns,Speed/100,MaxAltitudeError);
        }
        if(Loop)
        {
            if(Phase==0)
            {
                Pilot->Forward(1);
                if(Angle>=IBS3::Tau)
                {
                    const double ReturnError=FVector::Distance(P,Initial),Seconds=Now-Started;
                    Test->TestTrue(TEXT("One full actual circumnavigation completed in 8-12 game minutes"),Seconds>=480 && Seconds<=720);
                    Test->TestTrue(TEXT("Return within one conservative 10m sample segment"),ReturnError<1000);
                    Test->TestTrue(TEXT("At least five actual region crossings"),Crossings>=5);
                    Test->AddInfo(FString::Printf(TEXT("Actual CMC circumnavigation: %.6fs, return %.6fcm, altitude error %.6fcm, crossings=%d, peak=%.3fm/s"),Seconds,ReturnError,MaxAltitudeError,Crossings,PeakSpeed/100));
                    Brake(1,Now);
                }
            }
            else if(Speed<1) { CheckBrake(); return Finish(); }
        }
        else
        {
            const double Distance=W->Manifest->RadiusCm*FMath::Acos(FMath::Clamp(FVector::DotProduct((Initial-Center).GetSafeNormal(),B),-1.0,1.0));
            if(Phase==0) { Pilot->Forward(1); if(Distance>=220000) Brake(1,Now); }
            else if(Phase==1 && Speed<1)
            {
                CheckBrake(); int32 Count=0; Prop(CarKey,Count);
                Test->TestEqual(TEXT("Car actually unloaded after outward flight"),Count,0);
                Pilot->TangentForward=-Pilot->TangentForward; Pilot->BrakeOff(); Phase=2; PhaseAt=Now;
            }
            else if(Phase==2) { Pilot->Forward(1); if(Distance<65000) Brake(3,Now); }
            else if(Phase==3 && Speed<1)
            {
                CheckBrake(); int32 Count=0; auto* Car=Prop(CarKey,Count);
                Test->TestEqual(TEXT("Single car on physical flight reentry"),Count,1);
                Test->TestTrue(TEXT("Car displacement retained after flight unload/reentry"),Car && FVector::Distance(Car->GetActorLocation(),CarMoved)<150);
                auto* Wall=Prop(WallKey,Count); Test->TestEqual(TEXT("Single wall on physical reentry"),Count,1);
                Test->TestTrue(TEXT("Broken wall collision remains absent"),Wall && Wall->State.bBroken && Wall->Mesh->GetCollisionEnabled()==ECollisionEnabled::NoCollision);
                Test->AddInfo(FString::Printf(TEXT("Physical reentry %d: distance %.3fm altitude drift %.6fcm, elapsed %.3fs"),++Returns,Distance/100,MaxAltitudeError,Now-Started));
                if(Returns==10)
                {
                    Test->TestTrue(TEXT("320m/s achieved during measured flights"),PeakSpeed>=31900);
                    Test->TestTrue(TEXT("At least five measured region crossings"),Crossings>=5);
                    return Finish();
                }
                Pilot->TangentForward=-Pilot->TangentForward; Pilot->BrakeOff(); Phase=0; PhaseAt=Now;
            }
            if(Now-PhaseAt>40) { Test->AddError(FString::Printf(TEXT("Flight phase %d timed out at %.3fm, speed %.3fm/s; %s"),Phase,Distance/100,Speed/100,*W->Status)); return Finish(); }
        }
        if(Now-Started>750) { Test->AddError(TEXT("Flight test exceeded 750 game seconds")); return Finish(); }
        return false;
    }
};
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIBS3FlightReentryTest,"ImpactBeyond.S3.FlightReentry",EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FIBS3FlightReentryTest::RunTest(const FString& Parameters) { ADD_LATENT_AUTOMATION_COMMAND(FIBS3FlightCommand(this,false)); return true; }
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIBS3CircumnavigationTest,"ImpactBeyond.S3.Circumnavigation",EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FIBS3CircumnavigationTest::RunTest(const FString& Parameters) { ADD_LATENT_AUTOMATION_COMMAND(FIBS3FlightCommand(this,true)); return true; }
#endif
