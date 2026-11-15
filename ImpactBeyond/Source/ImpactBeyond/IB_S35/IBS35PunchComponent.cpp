#include "IB_S35/IBS35PunchComponent.h"
#include "IB_S35/IBS35GripComponent.h"
#include "IB_S35/IBS35Object.h"
#include "IB_S35/IBS35ReceiverComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

UIBS35PunchComponent::UIBS35PunchComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    Heavy.Windup = 0.28f; Heavy.Active = 0.14f; Heavy.Recovery = 0.35f;
    Heavy.Damage = 60; Heavy.DeltaSpeed = 950; Heavy.Reach = 180; Heavy.Radius = 24;
}

void UIBS35PunchComponent::BeginPlay()
{
    Super::BeginPlay(); InstigatorId = FGuid::NewGuid();
    if (auto* Grip = GetOwner()->FindComponentByClass<UIBS35GripComponent>())
    { InstigatorId = Grip->InstigatorId; AddTickPrerequisiteComponent(Grip); }
}

bool UIBS35PunchComponent::BeginPunch(bool bHeavy)
{
    if (bAttacking) return false;
    auto* Grip = GetOwner()->FindComponentByClass<UIBS35GripComponent>();
    AIBS35Object* Tool = Grip ? Grip->GetHeld() : nullptr;
    if (Grip && (Grip->IsCharging() || (Tool && (!Tool->Profile.bMeleeTool || Grip->GetGripWeight() < 1)))) return false;
    Current = bHeavy ? Heavy : Quick;
    Timing = {Current.Windup, Current.Active, Current.Recovery};
    if (!Timing.Valid() || !FMath::IsFinite(Current.Damage) || Current.Damage < 0
        || !FMath::IsFinite(Current.DeltaSpeed) || Current.DeltaSpeed < 0
        || !FMath::IsFinite(Current.Reach) || Current.Reach < 50 || Current.Reach > 250
        || !FMath::IsFinite(Current.Radius) || Current.Radius <= 0 || Current.Radius > 50) return false;
    bTool = Tool != nullptr; AttackTool = Tool; bHeavyAttack = bHeavy;
    if (Tool)
    {
        ToolStart = Tool->GetActorLocation(); Grip->SetToolActive(true);
        // Heavier tools telegraph longer; remains the same native box sweep.
        Timing.Windup *= 1 + Grip->GetGestureLoad() * 0.5;
    }
    AttackId = FGuid::NewGuid(); Age = 0; bAttacking = true; bContactCue = false; bPathBlocked = false;
    AttackOrigin = GetOwner()->GetActorLocation(); AttackDirection = GetOwner()->GetActorForwardVector();
    UE_LOG(LogTemp, Display, TEXT("IBS35 attack=%s heavy=%d tool=%d origin=%s windup=%.4f active=%.4f recovery=%.4f"),
        *AttackId.ToString(), bHeavyAttack, bTool, *AttackOrigin.ToString(), Timing.Windup, Timing.Active, Timing.Recovery);
    OnAttackCue.Broadcast(TEXT("Windup"), 0);
    return true;
}

void UIBS35PunchComponent::CancelAttack()
{
    if (auto* Grip = GetOwner()->FindComponentByClass<UIBS35GripComponent>()) Grip->SetToolActive(false);
    const bool bWasActive = bAttacking;
    bAttacking = false; AttackId.Invalidate(); AttackTool.Reset();
    if (bWasActive) OnAttackCue.Broadcast(TEXT("End"), 1);
}

void UIBS35PunchComponent::NotifyContact()
{
    if (bAttacking && bUseNotifyContact && Age >= Timing.Windup && Age <= Timing.Windup + Timing.Active)
        TraceContact(0, 1);
}

void UIBS35PunchComponent::TraceContact(double FromAlpha, double ToAlpha)
{
    // Never resume on the far side of a blocker on a later frame of this attack.
    if (bPathBlocked) return;
    FHitResult Hit;
    FVector Start, End;
    if (bTool)
    {
        auto* Grip = GetOwner()->FindComponentByClass<UIBS35GripComponent>();
        AIBS35Object* Tool = AttackTool.Get();
        if (!Grip || !Tool || Grip->GetHeld() != Tool) { CancelAttack(); return; }
        Start = Tool->GetActorLocation();
        End = ToolStart + AttackDirection * (ToAlpha * Current.Reach);
        Grip->SweepTool(End - Start, Hit);
    }
    else
    {
        // Deterministic contact curve until an inspected montage is bound. Never traces behind the first wall.
        Start = AttackOrigin + FVector(0,0,40) + AttackDirection * FMath::Lerp(50.0, double(Current.Reach), FromAlpha);
        End = AttackOrigin + FVector(0,0,40) + AttackDirection * FMath::Lerp(50.0, double(Current.Reach), ToAlpha);
        FCollisionQueryParams Query(SCENE_QUERY_STAT(IBS35Punch), false, GetOwner());
        Query.bReturnPhysicalMaterial = true;
        GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, ECC_Visibility,
            FCollisionShape::MakeSphere(Current.Radius), Query);
    }
    if (bDrawContacts)
    {
        DrawDebugLine(GetWorld(), Start, End, Hit.bBlockingHit ? FColor::Red : FColor::Cyan, false, 0.3f, 0, 2);
        DrawDebugSphere(GetWorld(), Hit.bBlockingHit ? Hit.ImpactPoint : End, Current.Radius, 12, FColor::Yellow, false, 0.3f);
    }
    UE_LOG(LogTemp, Display, TEXT("IBS35 contact attack=%s t=%.4f from=%s to=%s blocking=%d actor=%s"),
        *AttackId.ToString(), Age, *Start.ToString(), *End.ToString(), Hit.bBlockingHit, *GetNameSafe(Hit.GetActor()));
    bPathBlocked = Hit.bBlockingHit;
    AActor* Actor = Hit.GetActor();
    UPrimitiveComponent* Body = Hit.GetComponent();
    auto* Target = Actor ? Actor->FindComponentByClass<UIBS35ReceiverComponent>() : nullptr;
    if (!Hit.bBlockingHit || !Target || !Body) return;
    FIBS35DamageMessage Message;
    Message.AttackId = AttackId; Message.InstigatorId = InstigatorId; Message.TargetId = Target->TargetId;
    Message.DamageKind = bTool ? EIBS35DamageKind::HeldTool
        : bHeavyAttack ? EIBS35DamageKind::HeavyPunch : EIBS35DamageKind::QuickPunch;
    Message.HealthDelta = -Current.Damage;
    Message.Contact = Hit.ImpactPoint; Message.AttackSeconds = Age;
    const float TargetMass = Body->IsSimulatingPhysics() ? FMath::Max(1.f, Body->GetMass()) : 80;
    Message.Impulse = AttackDirection * Current.DeltaSpeed * TargetMass;
    Message.RelativeVelocity = AttackDirection * ((Current.Reach - 50) / Timing.Active) - Body->GetComponentVelocity();
    Message.SurfaceMaterial = Hit.PhysMaterial.IsValid() ? Hit.PhysMaterial->GetFName() : NAME_None;
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
        Message.bAirborne = !Character->GetCharacterMovement()->IsMovingOnGround();
    if (Target->Receive(Message, Body)) OnHit.Broadcast(Message);
}

void UIBS35PunchComponent::TickComponent(float Dt, ELevelTick TickType, FActorComponentTickFunction* Function)
{
    Super::TickComponent(Dt, TickType, Function);
    if (!bAttacking || Dt <= 0 || !FMath::IsFinite(Dt)) return;
    // A teleport or extreme traversal invalidates the short punch instead of striking at an old location.
    if (FVector::DistSquared(AttackOrigin, GetOwner()->GetActorLocation()) > FMath::Square(250.f)) { CancelAttack(); return; }
    const double Previous = Age; Age += Dt;
    if (Timing.CrossesContact(Previous, Age))
    {
        if (!bContactCue) { bContactCue = true; OnAttackCue.Broadcast(TEXT("Contact"), 0.5f); }
        if (!bUseNotifyContact) TraceContact(Timing.Alpha(Previous), Timing.Alpha(Age));
    }
    if (Age >= Timing.Duration()) CancelAttack();
}

void UIBS35PunchComponent::EndPlay(const EEndPlayReason::Type Reason) { CancelAttack(); Super::EndPlay(Reason); }
