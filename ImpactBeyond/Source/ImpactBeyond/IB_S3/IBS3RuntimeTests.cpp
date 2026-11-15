#if WITH_DEV_AUTOMATION_TESTS
#include "IBS3World.h"
#include "IBS3Character.h"
#include "IBS3Geometry.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIBS3RuntimeResidencyTest,"ImpactBeyond.S3.RuntimeResidency",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FIBS3RuntimeResidencyTest::RunTest(const FString& Parameters)
{
    AIBS3PlanetWorld* Planet=nullptr;
    for(const FWorldContext& Context:GEngine->GetWorldContexts())
        if(Context.World() && (Context.WorldType==EWorldType::Game || Context.WorldType==EWorldType::PIE))
            for(TActorIterator<AIBS3PlanetWorld> It(Context.World());It;++It) { Planet=*It; break; }
    if(!TestNotNull(TEXT("Load /Game/IB_S3/Lvl_IB_S3_PlanetLab with -game before running"),Planet)) return false;
    UWorld* World=Planet->GetWorld();
    if(!TestTrue(TEXT("Manifest has four or more cities"),Planet->Manifest && Planet->Manifest->Cities.Num()>=4)) return false;
    auto Find=[World](FName Key,int32& Count)->AIBS3Prop*
    {
        AIBS3Prop* Result=nullptr; Count=0;
        for(TActorIterator<AIBS3Prop> It(World);It;++It)
            if(!It->IsActorBeingDestroyed() && It->State.Key==Key) { Result=*It; ++Count; }
        return Result;
    };
    for(const FIBS3City& C:Planet->Manifest->Cities)
    {
        const FVector P=Planet->CityPosition(C),Up=(P-Planet->GetActorLocation()).GetSafeNormal();
        Planet->UpdateResidency(P+Up*300,32000);
        TestTrue(*FString::Printf(TEXT("%s collider ready before entry"),*C.CityId.ToString()),Planet->IsReady(C.CityId));
        FHitResult Hit;
        TestTrue(TEXT("Actual engine trace hits landing surface"),World->LineTraceSingleByChannel(Hit,P+Up*1000,P-Up*2000,ECC_Visibility));
        TestTrue(TEXT("Landing surface normal is radial"),FVector::DotProduct(Hit.ImpactNormal,Up)>0.99);
    }
    const FIBS3City& Home=Planet->Manifest->Cities[0];
    const FVector HomeP=Planet->CityPosition(Home),Away=Planet->CityPosition(Planet->Manifest->Cities[1]);
    Planet->UpdateResidency(HomeP,0);
    const FString Prefix=Planet->Manifest->BodyId.ToString()+TEXT("/")+Home.RegionId.ToString()+TEXT("/");
    const FName CarKey(*(Prefix+TEXT("car01"))),WallKey(*(Prefix+TEXT("wall01")));
    int32 Count=0; AIBS3Prop* Car=Find(CarKey,Count); AIBS3Prop* Wall=Find(WallKey,Count);
    if(!TestNotNull(TEXT("Car fixture exists"),Car) || !TestNotNull(TEXT("Wall fixture exists"),Wall)) return false;
    // Explicit fixture placement, not a navigation/circumnavigation test.
    const FVector SavedPosition=Car->GetActorLocation()+Car->GetActorRightVector()*300;
    Car->SetActorLocation(SavedPosition,false,nullptr,ETeleportType::TeleportPhysics);
    Wall->Interact(FVector::ForwardVector);
    for(int32 Return=0;Return<10;++Return)
    {
        Planet->UpdateResidency(Away,32000);
        Find(CarKey,Count); TestEqual(TEXT("Car unloaded outside physics bubble"),Count,0);
        Planet->UpdateResidency(HomeP,32000);
        Car=Find(CarKey,Count); TestEqual(TEXT("One car per stable key on reentry"),Count,1);
        if(!Car) return false;
        TestTrue(TEXT("Moved car position survives unload/rehydration"),Car->GetActorLocation().Equals(SavedPosition,0.1));
        Wall=Find(WallKey,Count); TestEqual(TEXT("One wall per stable key"),Count,1);
        TestTrue(TEXT("Broken wall collider stays absent"),Wall && Wall->State.bBroken && Wall->Mesh->GetCollisionEnabled()==ECollisionEnabled::NoCollision);
    }
    const FString OriginalSlot=Planet->SaveSlot;
    const FString FixtureSlot=TEXT("IB_S3_QA_")+FGuid::NewGuid().ToString(EGuidFormats::Digits);
    Planet->SaveSlot=FixtureSlot;
    const bool Saved=Planet->SaveState(); Planet->SaveSlot=OriginalSlot;
    TestTrue(TEXT("USaveGame write succeeded"),Saved);
    auto* Loaded=Cast<UIBS3SaveGame>(UGameplayStatics::LoadGameFromSlot(FixtureSlot,0));
    if(TestNotNull(TEXT("USaveGame file deserialized"),Loaded))
    {
        TestEqual(TEXT("Versioned schema"),Loaded->SchemaVersion,1);
        TestEqual(TEXT("Body ID retained"),Loaded->BodyId,Planet->Manifest->BodyId);
        TSet<FName> Keys;
        for(const FIBS3ObjectDelta& D:Loaded->Deltas)
        {
            TestFalse(TEXT("No duplicate persistent keys"),Keys.Contains(D.Key)); Keys.Add(D.Key);
            if(D.Key==CarKey) TestTrue(TEXT("Saved global car position"),D.GlobalTransform.GetLocation().Equals(SavedPosition+Planet->OriginOffset(),0.1));
            if(D.Key==WallKey) TestTrue(TEXT("Saved broken wall"),D.bBroken);
        }
    }
    TestTrue(TEXT("Only the GUID-named QA save is removed"),UGameplayStatics::DeleteGameInSlot(FixtureSlot,0));
    AddInfo(TEXT("Fixture tests do not prove four character landings, continuous flight, visual maps, restart persistence, DX12, or FPS."));
    return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIBS3ManifestTest,"ImpactBeyond.S3.Manifest",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FIBS3ManifestTest::RunTest(const FString& Parameters)
{
    auto* Data=NewObject<UIBS3WorldManifest>(); FString Error;
    TestTrue(TEXT("Default four-city manifest is valid"),Data->Validate(Error));
    for(int I=4;I<9;++I)
    {
        FIBS3City C; C.CityId=FName(*FString::Printf(TEXT("CITY_QA_%d"),I)); C.RegionId=FName(*FString::Printf(TEXT("earth.qa.%d"),I));
        C.LatitudeDegrees=-50+I; C.LongitudeDegrees=-150+I*15; Data->Cities.Add(C);
    }
    Data->RadiusCm=9000000;
    TestTrue(TEXT("Ninth city and 90 km radius require only data"),Data->Validate(Error));
    Data->Cities.Last().CityId=Data->Cities[0].CityId;
    TestFalse(TEXT("Duplicate CityId rejected"),Data->Validate(Error));
    return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIBS3SaveValidationTest,"ImpactBeyond.S3.SaveValidation",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FIBS3SaveValidationTest::RunTest(const FString& Parameters)
{
    AIBS3PlanetWorld* Planet=nullptr;
    for(const FWorldContext& Context:GEngine->GetWorldContexts())
        if(Context.World() && (Context.WorldType==EWorldType::Game || Context.WorldType==EWorldType::PIE))
            for(TActorIterator<AIBS3PlanetWorld> It(Context.World());It;++It) { Planet=*It; break; }
    if(!TestNotNull(TEXT("S3 lab loaded"),Planet) || !TestTrue(TEXT("Ground physics ready"),Planet->IsOperational())) return false;
    const FString OriginalSlot=Planet->SaveSlot;
    const bool OriginalCompatibility=Planet->bSaveCompatible;
    const FString FixtureSlot=TEXT("IB_S3_QA_Validation_")+FGuid::NewGuid().ToString(EGuidFormats::Digits);
    Planet->SaveSlot=FixtureSlot; Planet->bSaveCompatible=true;
    auto Cleanup=[&]()
    {
        Planet->SaveSlot=OriginalSlot; Planet->bSaveCompatible=OriginalCompatibility;
        UGameplayStatics::DeleteGameInSlot(FixtureSlot,0);
    };
    if(!TestTrue(TEXT("Write isolated save fixture"),Planet->SaveState())) { Cleanup(); return false; }
    auto* Baseline=Cast<UIBS3SaveGame>(UGameplayStatics::LoadGameFromSlot(FixtureSlot,0));
    if(!TestNotNull(TEXT("Read fixture"),Baseline) || !TestTrue(TEXT("Two object records available"),Baseline->Deltas.Num()>=2))
    { Cleanup(); return false; }
    const FName FirstKey=Baseline->Deltas[0].Key;
    const FTransform Before=Planet->Deltas[FirstKey].GlobalTransform;
    auto Reject=[&](const TCHAR* Label,auto Mutate)
    {
        auto* Bad=DuplicateObject<UIBS3SaveGame>(Baseline,GetTransientPackage());
        // A valid earlier record must not commit when a later record fails validation.
        Bad->Deltas[0].GlobalTransform.AddToTranslation(FVector(123,456,789)); Mutate(Bad);
        if(!TestTrue(TEXT("Write intentionally invalid fixture"),UGameplayStatics::SaveGameToSlot(Bad,FixtureSlot,0))) return;
        Planet->bSaveCompatible=true; Planet->LoadState();
        TestFalse(Label,Planet->bSaveCompatible);
        const FIBS3ObjectDelta* Retained=Planet->Deltas.Find(FirstKey);
        TestTrue(TEXT("Rejected file commits no partial deltas"),Retained && Retained->GlobalTransform.Equals(Before,0.001));
        TestFalse(TEXT("Rejected slot cannot be overwritten by SaveState"),Planet->SaveState());
    };
    Reject(TEXT("Finite zero quaternion rejected"),[](UIBS3SaveGame* S){ S->Deltas[1].GlobalTransform.SetRotation(FQuat(0,0,0,0)); });
    Reject(TEXT("Stable origin cannot change"),[](UIBS3SaveGame* S){ S->Deltas[1].OriginRegionId=TEXT("earth.invalid.origin"); });
    Reject(TEXT("Object kind cannot change"),[](UIBS3SaveGame* S){ S->Deltas[1].Kind=TEXT("invalid-kind"); });
    Reject(TEXT("Duplicate stable ID rejected"),[](UIBS3SaveGame* S){ S->Deltas.Add(S->Deltas[0]); });
    Reject(TEXT("Different coordinate revision rejected"),[](UIBS3SaveGame* S){ ++S->ManifestRevision; });
    TestTrue(TEXT("Restore fixture file"),UGameplayStatics::SaveGameToSlot(Baseline,FixtureSlot,0));
    Planet->LoadState(); TestTrue(TEXT("Valid file reload accepted"),Planet->bSaveCompatible);
    Cleanup();
    AddInfo(TEXT("Serialization fixtures exercise USaveGame validation, not full game restart or physical reentry."));
    return !HasAnyErrors();
}
#endif
