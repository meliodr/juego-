#if defined(IBS35_PUNCH_NATIVE_TEST_MAIN)
#include "IBS35PunchComponentPolicy.h"
#include "IBS35Policy.h"
#include <cstdlib>
#include <iostream>
#include <limits>
int main()
{
    int Checks = 0;
    auto Check = [&Checks](bool Value) { ++Checks; if (!Value) std::exit(1); };
    IBS35::PunchCombo Combo;
    for (int Repeat = 0; Repeat < 12; ++Repeat)
    {
        Check(Combo.Begin() == Repeat % 3 + 1);
        Combo.Complete(); Combo.Tick(.2);
    }
    Combo.Tick(.61); Check(Combo.Begin() == 1);
    Combo.Complete(); Combo.Tick(std::numeric_limits<double>::quiet_NaN());
    Check(Combo.Begin() == 2); Combo.Reset(); Check(Combo.Begin() == 1);
    Check(IBS35::ChargedDamageScale(1) > IBS35::ChargedDamageScale(0));
    Check(IBS35::ChargedImpulseScale(1) > IBS35::ChargedImpulseScale(0));
    Check(IBS35::ChargedDamageScale(-1) == IBS35::ChargedDamageScale(0));
    Check(IBS35::ChargedImpulseScale(2) == IBS35::ChargedImpulseScale(1));
    for (double Dt : {.005, .016, .033, 1.0})
    {
        IBS35::AttackTiming Timing{.14,.12,.28};
        double Age = 0, Coverage = 0;
        while (Age < Timing.Duration())
        {
            if (Timing.CrossesContact(Age, Age + Dt)) Coverage += Timing.Alpha(Age + Dt) - Timing.Alpha(Age);
            Age += Dt;
        }
        Check(std::abs(Coverage - 1.0) < 1e-9);
    }
    std::cout << Checks << " punch policy checks PASS\n";
}
#elif WITH_DEV_AUTOMATION_TESTS
#include "IBS35PunchComponent.h"
#include "IBS35LabTarget.h"
#include "IBS35ReceiverComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Math/RotationMatrix.h"
#include "Misc/AutomationTest.h"
#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIBS35PlayablePunchTest, "ImpactBeyond.S35.PlayablePunches",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FIBS35PlayablePunchTest::RunTest(const FString& Parameters)
{
    UWorld* World = nullptr;
    for (const FWorldContext& Context : GEngine->GetWorldContexts())
        if (Context.WorldType == EWorldType::Game) { World = Context.World(); break; }
    if (!TestNotNull(TEXT("Game world"), World)) return false;
    FActorSpawnParameters Spawn;
    Spawn.ObjectFlags |= RF_Transient;
    Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    const FVector Origin(50000000,50000000,50000000);
    ACharacter* Hero = World->SpawnActor<ACharacter>(Origin, FRotator::ZeroRotator, Spawn);
    AIBS35LabTarget* Target = World->SpawnActor<AIBS35LabTarget>(Origin + FVector(125,0,60), FRotator::ZeroRotator, Spawn);
    if (!Hero || !Target) { if (Hero) Hero->Destroy(); if (Target) Target->Destroy(); return false; }
    auto* Punch = NewObject<UIBS35PunchComponent>(Hero);
    Hero->AddInstanceComponent(Punch); Punch->RegisterComponent(); Punch->bDrawContacts = false;
    Hero->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
    Target->Body->SetBoxExtent(FVector(20,60,65));
    int Calls = 0;
    FIBS35DamageMessage Captured;
    Punch->CanonicalStrike.BindLambda([&](const FIBS35DamageMessage& Message, const FHitResult& Hit)
    {
        ++Calls; Captured = Message;
        FIBS35DamageMessage Routed = Message;
        return Target->Receiver->Receive(Routed, Hit.GetComponent());
    });
    for (const FVector Up : {FVector::UpVector, FVector::ForwardVector, -FVector::ForwardVector})
    {
        const FVector Forward = FVector::VectorPlaneProject(FVector(1,1,0), Up).GetSafeNormal();
        const FQuat Rotation = FRotationMatrix::MakeFromXZ(Forward, Up).ToQuat();
        Hero->SetActorLocationAndRotation(Origin, Rotation, false, nullptr, ETeleportType::TeleportPhysics);
        Target->SetActorLocationAndRotation(Origin + Forward * 125 + Up * 60, Rotation, false, nullptr, ETeleportType::TeleportPhysics);
        const int Before = Calls;
        TestTrue(TEXT("Uppercut begins in each gravity frame"), Punch->BeginUppercut());
        Punch->TickComponent(.05f, LEVELTICK_All, nullptr);
        TestEqual(TEXT("No damage before windup"), Calls, Before);
        TestTrue(TEXT("Recovery remains visible"), Punch->GetCooldown() > 0);
        Punch->TickComponent(.6f, LEVELTICK_All, nullptr);
        TestEqual(TEXT("One canonical contact through a hitch"), Calls, Before + 1);
        TestTrue(TEXT("Uppercut impulse follows local up"), FVector::DotProduct(Captured.Impulse.GetSafeNormal(), Up) > .9);
        TestTrue(TEXT("Contact snapshots target life"), Captured.TargetId == Target->Receiver->TargetId);
        TestTrue(TEXT("Airborne contact marked"), Captured.bAirborne);
    }
    Hero->SetActorLocationAndRotation(Origin, FQuat::Identity, false, nullptr, ETeleportType::TeleportPhysics);
    Target->SetActorLocationAndRotation(Origin + FVector(125,0,40), FQuat::Identity, false, nullptr, ETeleportType::TeleportPhysics);
    for (int Index = 1; Index <= 4; ++Index)
    {
        TestTrue(TEXT("Quick chain starts"), Punch->BeginPunch(false));
        TestEqual(TEXT("Three distinct quick stages"), Punch->GetComboIndex(), (Index - 1) % 3 + 1);
        TestFalse(TEXT("Cannot overlap contact windows"), Punch->BeginPunch(false));
        Punch->TickComponent(Punch->GetCooldown() + .01f, LEVELTICK_All, nullptr);
    }
    Punch->TickComponent(1.f, LEVELTICK_All, nullptr);
    TestTrue(TEXT("Combo restarts after timeout"), Punch->BeginPunch(false));
    TestEqual(TEXT("Expired combo resets"), Punch->GetComboIndex(), 1);
    Punch->CancelAttack();
    TestFalse(TEXT("Reject nonfinite charge"), Punch->BeginChargedPunch(std::numeric_limits<float>::quiet_NaN()));
    const int BeforeCancel = Calls;
    TestTrue(TEXT("Charged punch starts"), Punch->BeginChargedPunch(1));
    Punch->CancelAttack(); Punch->TickComponent(1.f, LEVELTICK_All, nullptr);
    TestEqual(TEXT("Cancelled windup has no hit"), Calls, BeforeCancel);
    Hero->GetCharacterMovement()->Velocity = FVector(3000,0,0);
    TestTrue(TEXT("Flying heavy begins"), Punch->BeginChargedPunch(1));
    Hero->SetActorLocation(Origin + FVector(600,0,0), false, nullptr, ETeleportType::TeleportPhysics);
    Target->SetActorLocation(Origin + FVector(725,0,40), false, nullptr, ETeleportType::TeleportPhysics);
    Punch->TickComponent(.3f, LEVELTICK_All, nullptr);
    TestEqual(TEXT("Flight velocity does not cancel heavy windup"), Calls, BeforeCancel + 1);
    Punch->CancelAttack(); Hero->GetCharacterMovement()->Velocity = FVector::ZeroVector;
    const int AcceptedBeforeReject = Target->Receiver->AcceptedHits;
    Punch->CanonicalStrike.BindLambda([](const FIBS35DamageMessage&, const FHitResult&) { return false; });
    TestTrue(TEXT("Rejected routed punch begins"), Punch->BeginPunch(false));
    Punch->TickComponent(1, LEVELTICK_All, nullptr);
    TestEqual(TEXT("Canonical rejection never falls back to legacy receiver"), Target->Receiver->AcceptedHits, AcceptedBeforeReject);
    TestTrue(TEXT("Teleport cancellation starts"), Punch->BeginPunch(false));
    Hero->SetActorLocation(Hero->GetActorLocation() + FVector(2000,0,0), false, nullptr, ETeleportType::TeleportPhysics);
    Punch->TickComponent(.01f, LEVELTICK_All, nullptr);
    TestFalse(TEXT("Teleport cancels old hitbox"), Punch->IsAttacking());
    Hero->Destroy(); Target->Destroy();
    return true;
}
#endif
