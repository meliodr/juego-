#if WITH_DEV_AUTOMATION_TESTS
#include "IBPlaySession.h"
#include "IBPlayCharacter.h"
#include "IBPlayArt.h"
#include "IBPlayCombat.h"
#include "IB_S3/IBS3World.h"
#include "IB_S35/IBS35GripComponent.h"
#include "IB_S35/IBS35Object.h"
#include "IB_S35/IBS35ReceiverComponent.h"
#include "IB_S5/IBS5WorldLife.h"
#include "IB_S4/IBS4PreparedTarget.h"
#include "IB_S6H/IBS6HBridge.h"
#include "IB_S7/Wave8Threats/IB_S7T/IBS7TThreatActor.h"
#include "IB_S7/Wave8Threats/IB_S7T/IBS7TWheelComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "HAL/PlatformTime.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Math/RotationMatrix.h"
#include "UObject/StrongObjectPtr.h"

namespace
{
bool IsProductionSkin(const USkeletalMesh* Mesh)
{
    if (!Mesh) return false;
    const FString Path = Mesh->GetPathName();
    return Path.StartsWith(TEXT("/Game/")) && !Path.Contains(TEXT("Mannequin"))
        && !Path.Contains(TEXT("SKM_Manny")) && !Path.Contains(TEXT("SKM_Quinn"));
}

class FIBPlaySessionCommand final : public IAutomationLatentCommand
{
    FAutomationTestBase* Test;
    const double WallStarted = FPlatformTime::Seconds();
    TWeakObjectPtr<AIBPlayCharacter> Hero;
    TWeakObjectPtr<AIBPlaySession> Session;
    TWeakObjectPtr<AIBS35Object> Cargo;
    TWeakObjectPtr<AIBS7TThreatActor> Heavy;
    TStrongObjectPtr<UIBPlaySave> Initial;
    TStrongObjectPtr<UIBPlaySave> BeforeCarry;
    TStrongObjectPtr<UIBPlaySave> Damaged;
    TMap<TWeakObjectPtr<AIBS5Citizen>, FVector> CitizenStarts;
    FVector CargoBeforeLift = FVector::ZeroVector;
    int32 Phase = 0;
    int32 SelectedHero = 0;
    int32 ActorCountBeforeRestore = 0;
    double PhaseStarted = 0;
    bool bFinished = false;

    int32 ObjectCount() const
    {
        int32 Count = 0;
        if (Session.IsValid()) for (TActorIterator<AIBS35Object> It(Session->GetWorld()); It; ++It)
            if (!It->IsActorBeingDestroyed()) ++Count;
        return Count;
    }

    bool Capture(TStrongObjectPtr<UIBPlaySave>& Destination)
    {
        UIBPlaySave* Saved = nullptr;
        if (!Session->CaptureSnapshot(Saved) || !Saved) return false;
        Destination.Reset(Saved);
        return true;
    }

    bool Finish()
    {
        if (bFinished) return true;
        bFinished = true;
        if (Hero.IsValid())
        {
            Hero->Wheel->Cancel(); Hero->Heroes->CloseSelector(); Hero->Grip->Drop();
            Hero->GetCharacterMovement()->StopMovementImmediately();
            Hero->Combat->ResetCombat();
        }
        if (Session.IsValid() && Initial.IsValid())
            Test->TestTrue(TEXT("QA cleanup restores initial in-memory session"), Session->RestoreSnapshot(Initial.Get()));
        Initial.Reset(); BeforeCarry.Reset(); Damaged.Reset();
        return true;
    }

    bool CheckReady()
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (!Context.World() || Context.WorldType != EWorldType::Game) continue;
            Hero = Cast<AIBPlayCharacter>(UGameplayStatics::GetPlayerPawn(Context.World(),0));
            for (TActorIterator<AIBPlaySession> It(Context.World()); It; ++It) { Session = *It; break; }
            break;
        }
        if (!Hero.IsValid() || !Session.IsValid() || !Session->Planet || !Session->Life
            || !Session->Planet->IsOperational() || !Session->Life->bRegionReady
            || Session->Life->GetCitizenCount() < 3) return false;
        for (TActorIterator<AIBS35Object> It(Session->GetWorld()); It; ++It)
            if (!It->IsActorBeingDestroyed() && It->Profile.ObjectClass == EIBS35ObjectClass::Rock
                && It->FindComponentByClass<UIBPlayCombat>()) { Cargo = *It; break; }
        for (TActorIterator<AIBS7TThreatActor> It(Session->GetWorld()); It; ++It)
            if (!It->IsActorBeingDestroyed() && !It->bKaiju) { Heavy = *It; break; }
        return Cargo.IsValid() && Heavy.IsValid() && Session->Hero == Hero.Get();
    }

    void CheckComposition()
    {
        auto* World = Session->GetWorld();
        Test->TestTrue(TEXT("Production map is loaded"), World->GetMapName().Contains(TEXT("Lvl_IB_Playable")));
        Test->TestTrue(TEXT("Production game mode owns the pawn"), World->GetAuthGameMode() && World->GetAuthGameMode()->IsA<AIBPlayGameMode>());
        Test->TestFalse(TEXT("QA session does not write player saves"), Session->bPersistence);
        Test->TestFalse(TEXT("QA citizens do not write player saves"), Session->Life->bEnablePersistence);
        Test->TestTrue(TEXT("Hero uses the operational planet"), Hero->Planet == Session->Planet);
        Test->TestTrue(TEXT("Selected city collision is ready"), Session->Planet->IsReady(Session->Planet->ActiveCity));
        Test->TestTrue(TEXT("At least four city destinations exist"), Session->Planet->Manifest->Cities.Num() >= 4);
        Test->TestEqual(TEXT("Three selectable heroes"), Hero->Heroes->GetHeroDescriptors().Num(),3);
        Test->TestTrue(TEXT("Loaded production art asset"), IsValid(Hero->Art));
        if (Hero->Art)
        {
            Test->TestEqual(TEXT("Three production hero skins"), Hero->Art->Heroes.Num(),3);
            for (USkeletalMesh* Skin : Hero->Art->Heroes)
                Test->TestTrue(TEXT("Hero skin is real imported art"), IsProductionSkin(Skin));
        }
        Test->TestTrue(TEXT("Robot renders its assigned production skeleton"), IsProductionSkin(Heavy->Visual->GetSkeletalMeshAsset()));
        Test->TestTrue(TEXT("Robot body has real blocking collision"), Heavy->Body->GetCollisionEnabled() != ECollisionEnabled::NoCollision);
        Test->TestTrue(TEXT("Robot uses native health"), Heavy->BodyHealth > 0);
        Test->TestTrue(TEXT("Robot has a canonical target life"), Heavy->GetTargetLifeId().IsValid());
        Test->TestTrue(TEXT("S35 cargo has actual imported geometry"), Cargo->Appearance->GetStaticMesh() != nullptr);
        Test->TestTrue(TEXT("S35 cargo has physical simulation"), Cargo->Body->IsSimulatingPhysics());
        int32 VisibleCitizens = 0;
        for (TActorIterator<AIBS5Citizen> It(World); It; ++It) if (!It->IsHidden())
        {
            ++VisibleCitizens;
            CitizenStarts.Add(*It,It->GetActorLocation());
            Test->TestTrue(TEXT("Citizen has active collision"), It->Capsule->GetCollisionEnabled() != ECollisionEnabled::NoCollision);
            Test->TestTrue(TEXT("Citizen has assigned production skin"), IsProductionSkin(It->Outfit->GetSkeletalMeshAsset()));
        }
        Test->TestTrue(TEXT("At least three live citizens are visible"), VisibleCitizens >= 3);
    }

    bool CheckHeroSelector()
    {
        Hero->Heroes->OpenSelector();
        if (!Test->TestTrue(TEXT("Runtime selector opens"), Hero->Heroes->IsSelectorOpen())) return false;
        Hero->Heroes->SelectorInput(4 + SelectedHero,true);
        Hero->Heroes->SelectorInput(4 + SelectedHero,false);
        Test->TestEqual(TEXT("Selector focuses requested card"), Hero->Heroes->SelectorFocus(),SelectedHero);
        Test->TestFalse(TEXT("Open selector prevents power activation"), Hero->Heroes->ActivateEquipped());
        Hero->Heroes->SelectorInput(2,true); Hero->Heroes->SelectorInput(2,false);
        Test->TestEqual(TEXT("Selector commits requested hero"), Hero->Heroes->ActiveHero(),SelectedHero);
        Test->TestFalse(TEXT("Confirmation closes selector"), Hero->Heroes->IsSelectorOpen());
        if (Hero->Art && Hero->Art->Heroes.IsValidIndex(SelectedHero))
            Test->TestTrue(TEXT("Selection updates visible imported character"), Hero->HeroVisual->GetSkeletalMeshAsset() == Hero->Art->Heroes[SelectedHero]);
        Hero->RefreshWheel();
        Test->TestEqual(TEXT("Every hero exposes six real wheel slots"),Hero->Wheel->Slots.Num(),6);
        TSet<int32> Ids;
        for (const auto& Slot : Hero->Wheel->Slots)
        {
            Test->TestTrue(TEXT("Wheel labels and IDs are populated"), Slot.AbilityId > 0 && !Slot.Label.IsEmpty());
            Test->TestFalse(TEXT("Wheel has no duplicated ability IDs"),Ids.Contains(Slot.AbilityId));
            Ids.Add(Slot.AbilityId);
        }
        APlayerController* Controller = Cast<APlayerController>(Hero->GetController());
        const int32 Equipped = Hero->Heroes->EquippedAbility();
        const float Energy = Hero->Heroes->Energy();
        for (bool Gamepad : {false,true})
        {
            const bool IgnoredBefore = Controller && Controller->IsLookInputIgnored();
            Hero->Wheel->OpenWheel(Gamepad);
            Test->TestTrue(TEXT("Mouse/gamepad wheel path opens"), Hero->Wheel->bOpen);
            Test->TestTrue(TEXT("Wheel owns look lock while open"), Controller && Controller->IsLookInputIgnored());
            Hero->Wheel->Cancel();
            Test->TestTrue(TEXT("Wheel releases only its look lock"), Controller && Controller->IsLookInputIgnored() == IgnoredBefore);
            Test->TestEqual(TEXT("Opening/cancelling does not equip another power"),Hero->Heroes->EquippedAbility(),Equipped);
            Test->TestEqual(TEXT("Opening/cancelling never spends energy"),Hero->Heroes->Energy(),Energy);
        }
        return true;
    }

    bool StartCarry()
    {
        Hero->Heroes->SelectHero(0);
        const FVector Up = (Hero->GetActorLocation()-Session->Planet->GetActorLocation()).GetSafeNormal();
        const FVector Position = Hero->GetActorLocation()+Up*3000;
        const FVector Forward = FVector::VectorPlaneProject(Hero->GetActorForwardVector(),Up).GetSafeNormal();
        const FQuat Rotation = FRotationMatrix::MakeFromXZ(Forward,Up).ToQuat();
        Hero->SetActorLocationAndRotation(Position,Rotation,false,nullptr,ETeleportType::TeleportPhysics);
        Hero->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
        Hero->GetCharacterMovement()->StopMovementImmediately();
        const float Separation = Cargo->Profile.HalfExtent.X + Hero->GetCapsuleComponent()->GetScaledCapsuleRadius()+30;
        Cargo->SetActorLocationAndRotation(Position+Forward*Separation,Rotation,false,nullptr,ETeleportType::TeleportPhysics);
        Cargo->Body->SetPhysicsLinearVelocity(FVector::ZeroVector);
        Cargo->Body->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);
        if (!Test->TestTrue(TEXT("Unheld session can be captured with persistence disabled"),Capture(BeforeCarry))) return false;
        CargoBeforeLift = Cargo->GetActorLocation();
        if (!Test->TestTrue(TEXT("Shared grip acquires an existing session object"),Hero->Grip->TryGrab(Cargo.Get()))) return false;
        Test->TestTrue(TEXT("Grip and physical object agree on ownership"),Cargo->IsHeld() && Hero->Grip->GetHeld() == Cargo.Get());
        UIBPlaySave* Rejected = nullptr;
        Test->TestFalse(TEXT("Session capture refuses held cargo"),Session->CaptureSnapshot(Rejected));
        Test->TestFalse(TEXT("Restore refuses a held cargo transaction"),Session->RestoreSnapshot(BeforeCarry.Get()));
        Test->TestTrue(TEXT("Rejected persistence keeps grip ownership"),Hero->Grip->GetHeld() == Cargo.Get() && Cargo->IsHeld());
        Test->TestFalse(TEXT("Hero change refuses active shared grip"),Hero->Heroes->SelectHero(1));
        Test->AddInfo(TEXT("One controlled fixture relocation puts an existing session rock and the production pawn in clear air; measured lifting thereafter uses real grip ticks and root sweeps."));
        return true;
    }

    void CheckDamage()
    {
        auto* Combat = Cargo->FindComponentByClass<UIBPlayCombat>();
        const float Health = Combat->GetHealth();
        const int32 Accepted = Combat->Response->AcceptedHits;
        FHitResult Hit(Cargo.Get(),Cargo->Body,Cargo->GetActorLocation(),Hero->GetActorUpVector());
        FIBS35DamageMessage Message;
        Message.AttackId = FGuid::NewGuid(); Message.InstigatorId = Hero->Grip->InstigatorId;
        Message.InstigatorActor = Hero.Get(); Message.TargetId = Combat->GetTargetLife(); Message.TargetPart = Cargo->Body;
        Message.DamageKind = EIBS35DamageKind::HeavyPunch; Message.HealthDelta = -17; Message.Contact = Hit.ImpactPoint;
        Test->TestTrue(TEXT("Production session routes S35 damage"),Session->DispatchS35(Message,Hit));
        Test->TestFalse(TEXT("Redelivered S35 contact is rejected"),Session->DispatchS35(Message,Hit));
        Test->TestEqual(TEXT("Cargo loses health exactly once"),Combat->GetHealth(),FMath::Max(0.f,Health-17));
        Test->TestEqual(TEXT("S4 health owner records one accepted contact"),Combat->Response->AcceptedHits,Accepted+1);
        Test->TestEqual(TEXT("S35 observer mirrors the same health owner"),Combat->ContactReceiver->Health,Combat->GetHealth());

        FHitResult HeavyHit(Heavy.Get(),Heavy->Body,Heavy->GetActorLocation(),Heavy->GetActorUpVector());
        const float HeavyHealth = Heavy->BodyHealth;
        Message.AttackId = FGuid::NewGuid(); Message.TargetId = Heavy->GetTargetLifeId();
        Message.TargetPart = Heavy->Body; Message.Contact = HeavyHit.ImpactPoint;
        Test->TestTrue(TEXT("Session routes to S7 native health"),Session->DispatchS35(Message,HeavyHit));
        Test->TestFalse(TEXT("S7 body deduplicates canonical contact"),Session->DispatchS35(Message,HeavyHit));
        Test->TestEqual(TEXT("S7 body takes one damage delta"),Heavy->BodyHealth,HeavyHealth-17);

        FHitResult HeroHit(Hero.Get(),Hero->GetCapsuleComponent(),Hero->GetActorLocation(),Hero->GetActorUpVector());
        FIBS4Hit Bullet = UIBS4ResponseComponent::MakeHit(Session->GetWorld(),Heavy.Get(),EIBS4Weapon::Bullet,10,HeroHit);
        const float HeroHealth = Hero->Combat->GetHealth();
        Test->TestTrue(TEXT("Hostile S4 hit enters shared hero receiver"),Session->DispatchS4(Bullet,HeroHit));
        Test->TestFalse(TEXT("Hostile S4 contact cannot repeat"),Session->DispatchS4(Bullet,HeroHit));
        Test->TestEqual(TEXT("H01 bullet resistance survives integration"),Hero->Combat->GetHealth(),HeroHealth);

        AIBS4PreparedTarget* Wall = nullptr;
        for (TActorIterator<AIBS4PreparedTarget> It(Session->GetWorld()); It; ++It)
            if (It->Kind == EIBS4TargetKind::Wall && It->ObjectId.ToString().EndsWith(TEXT(".Wall"))) { Wall = *It; break; }
        if (Test->TestNotNull(TEXT("Playable region includes a prepared wall"),Wall))
        {
            FHitResult WallHit;
            const FTransform Transform = Wall->GetActorTransform();
            Session->GetWorld()->LineTraceSingleByChannel(WallHit,Transform.TransformPosition(FVector(-100,0,120)),
                Transform.TransformPosition(FVector(100,0,120)),ECC_Visibility);
            if (Test->TestTrue(TEXT("Real wall panel blocks the attack path"),WallHit.GetActor() == Wall))
            {
                Message.AttackId = FGuid::NewGuid(); Message.TargetId.Invalidate();
                Message.TargetPart = WallHit.GetComponent(); Message.Contact = WallHit.ImpactPoint; Message.HealthDelta = -60;
                Test->TestTrue(TEXT("Shared routing breaks a prepared wall"),Session->DispatchS35(Message,WallHit));
                Test->TestTrue(TEXT("Permanent wall state changes"),Wall->bBroken);
                FHitResult Passage;
                Session->GetWorld()->LineTraceSingleByChannel(Passage,Transform.TransformPosition(FVector(-100,0,120)),
                    Transform.TransformPosition(FVector(100,0,120)),ECC_Visibility);
                Test->TestTrue(TEXT("Broken wall has a physical passage"),Passage.GetActor() != Wall);
            }
        }
    }

public:
    explicit FIBPlaySessionCommand(FAutomationTestBase* InTest) : Test(InTest) {}
    bool Update() override
    {
        if (FPlatformTime::Seconds()-WallStarted > 120)
        {
            Test->AddError(FString::Printf(TEXT("Playable runtime timed out in phase %d (waiting for world/citizens/cargo/hero state)."),Phase));
            return Finish();
        }
        if (Phase == 0)
        {
            if (!CheckReady()) return false;
            if (Session->bPersistence || Session->Life->bEnablePersistence)
            { Test->AddError(TEXT("-IBPlayQA did not isolate persistent saves; refusing session mutation.")); return Finish(); }
            CheckComposition();
            if (!Test->TestTrue(TEXT("Initial in-memory snapshot captures"),Capture(Initial))) return Finish();
            Phase = 1; PhaseStarted = Session->GetWorld()->GetTimeSeconds();
        }
        if (!Hero.IsValid() || !Session.IsValid() || !Cargo.IsValid() || !Heavy.IsValid())
        { Test->AddError(TEXT("Production actor disappeared during session test.")); return Finish(); }
        const double Now = Session->GetWorld()->GetTimeSeconds();
        if (Phase == 1 && Now-PhaseStarted >= 0.2)
        {
            if (!CheckHeroSelector()) return Finish();
            PhaseStarted = Now;
            if (++SelectedHero == 3) Phase = 2;
        }
        else if (Phase == 2 && Now-PhaseStarted >= 0.5)
        {
            bool MovedCitizen = false;
            for (const auto& Pair : CitizenStarts)
                if (Pair.Key.IsValid() && FVector::DistSquared(Pair.Key->GetActorLocation(),Pair.Value)>1) MovedCitizen = true;
            Test->TestTrue(TEXT("Citizen routine actually moves in the running map"),MovedCitizen);
            if (!StartCarry()) return Finish();
            Phase = 3; PhaseStarted = Now;
        }
        else if (Phase == 3 && Now-PhaseStarted >= Cargo->Profile.LiftSeconds+0.2)
        {
            Test->TestTrue(TEXT("Shared grip remains stable over real world ticks"),Hero->Grip->GetHeld() == Cargo.Get());
            Test->TestTrue(TEXT("Cargo really moves during swept lifting"),FVector::DistSquared(CargoBeforeLift,Cargo->GetActorLocation())>25);
            Test->TestTrue(TEXT("Carry animation receives grip weight"),Hero->Grip->GetGripWeight()>0.95f);
            Test->TestTrue(TEXT("Charge begins only after lift completed"),Hero->Grip->BeginCharge());
            Hero->Grip->CancelCharge(); Hero->Grip->Drop();
            if (!Test->TestTrue(TEXT("Drop restores physical object ownership"),!Cargo->IsHeld() && !Hero->Grip->GetHeld())) return Finish();
            Test->TestTrue(TEXT("Drop restores physics"),Cargo->Body->IsSimulatingPhysics());
            CheckDamage();
            if (!Test->TestTrue(TEXT("Damaged unheld session captures"),Capture(Damaged))) return Finish();
            TSet<FName> Ids;
            for (const auto& Object : Damaged->Objects)
            {
                Test->TestFalse(TEXT("Snapshot never duplicates stable object IDs"),Ids.Contains(Object.Id));
                Ids.Add(Object.Id);
            }
            ActorCountBeforeRestore = ObjectCount();
            const FVector SavedPosition = Cargo->GetActorLocation();
            const FTransform SavedHero = Hero->GetActorTransform();
            const float SavedHealth = Cargo->FindComponentByClass<UIBPlayCombat>()->GetHealth();
            Cargo->SetActorLocation(SavedPosition+Hero->GetActorUpVector()*150,false,nullptr,ETeleportType::TeleportPhysics);
            Hero->SetActorLocation(SavedHero.GetLocation()+Hero->GetActorUpVector()*150,false,nullptr,ETeleportType::TeleportPhysics);
            Test->TestTrue(TEXT("Session snapshot restores existing objects"),Session->RestoreSnapshot(Damaged.Get()));
            Test->TestTrue(TEXT("Restored cargo returns to saved position"),Cargo->GetActorLocation().Equals(SavedPosition,0.2));
            Test->TestEqual(TEXT("Restored object retains canonical health"),Cargo->FindComponentByClass<UIBPlayCombat>()->GetHealth(),SavedHealth);
            Test->TestEqual(TEXT("Restored S35 mirror agrees with canonical health"),Cargo->FindComponentByClass<UIBPlayCombat>()->ContactReceiver->Health,SavedHealth);
            Test->TestTrue(TEXT("Restore returns hero to the saved pose"),Hero->GetActorTransform().Equals(SavedHero,0.2));
            Test->TestTrue(TEXT("Applying a snapshot twice remains valid"),Session->RestoreSnapshot(Damaged.Get()));
            Test->TestEqual(TEXT("Repeated restore does not spawn duplicate actors"),ObjectCount(),ActorCountBeforeRestore);
            UIBPlaySave* Invalid = DuplicateObject<UIBPlaySave>(Damaged.Get(),GetTransientPackage());
            Invalid->Version = -1;
            Test->TestFalse(TEXT("Invalid snapshot fails before mutation"),Session->RestoreSnapshot(Invalid));
            Test->TestEqual(TEXT("Rejected snapshot preserves actor count"),ObjectCount(),ActorCountBeforeRestore);
            if (Damaged->Threats.Num() > 0 && Damaged->Objects.Num() > 0)
            {
                UIBPlaySave* InvalidNested = DuplicateObject<UIBPlaySave>(Damaged.Get(),GetTransientPackage());
                InvalidNested->Threats[0].Health = -1;
                InvalidNested->Objects[0].Health = 1;
                Test->TestFalse(TEXT("Corrupt nested threat rejects whole snapshot"),Session->RestoreSnapshot(InvalidNested));
                TStrongObjectPtr<UIBPlaySave> AfterRejected;
                if (Test->TestTrue(TEXT("Session remains capturable after rejected nested state"),Capture(AfterRejected)))
                {
                    const auto* Unchanged = AfterRejected->Objects.FindByPredicate([&](const FIBPlayObjectState& State)
                        { return State.Id == Damaged->Objects[0].Id; });
                    Test->TestTrue(TEXT("Nested preflight failure cannot partially change another object"),Unchanged && Unchanged->Health == Damaged->Objects[0].Health);
                }
            }
            Phase = 4; PhaseStarted = Now;
        }
        else if (Phase == 4 && Now-PhaseStarted >= 0.6)
        {
            Test->TestEqual(TEXT("Residency ticks do not duplicate restored cargo"),ObjectCount(),ActorCountBeforeRestore);
            TStrongObjectPtr<UIBPlaySave> RoundTrip;
            Test->TestTrue(TEXT("Restored session can be captured again"),Capture(RoundTrip));
            if (RoundTrip.IsValid())
            {
                Test->TestEqual(TEXT("Object snapshot count survives restore and residency"),RoundTrip->Objects.Num(),Damaged->Objects.Num());
                Test->TestEqual(TEXT("Threat snapshot count survives restore and residency"),RoundTrip->Threats.Num(),Damaged->Threats.Num());
                Test->TestEqual(TEXT("Prepared target count survives restore"),RoundTrip->Prepared.Num(),Damaged->Prepared.Num());
                for (const auto& Prepared : Damaged->Prepared)
                {
                    const auto* Restored = RoundTrip->Prepared.FindByPredicate([&](const FIBS4PreparedSnapshot& Candidate)
                        { return Candidate.ObjectId == Prepared.ObjectId; });
                    Test->TestTrue(TEXT("Prepared hole state survives session hydration"),Restored && Restored->bBroken == Prepared.bBroken);
                }
            }
            Test->AddInfo(TEXT("Validated production map composition, live citizen motion, three selector cards, six slots per hero, mouse/gamepad wheel paths, real grip ticks, single damage ownership and in-memory persistence. Physical controller, rendered visual quality and DX12 performance remain separate checks."));
            return Finish();
        }
        return false;
    }
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIBPlaySessionTest,"ImpactBeyond.Integration.PlayableSession",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FIBPlaySessionTest::RunTest(const FString& Parameters)
{
    if (!FParse::Param(FCommandLine::Get(),TEXT("IBPlayQA")))
    {
        AddError(TEXT("Run on /Game/IB_Playable/Lvl_IB_Playable with -IBPlayQA to isolate player persistence."));
        return false;
    }
    ADD_LATENT_AUTOMATION_COMMAND(FIBPlaySessionCommand(this));
    return true;
}
#endif
