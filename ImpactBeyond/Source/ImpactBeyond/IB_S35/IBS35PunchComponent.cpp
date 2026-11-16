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
    Uppercut.Windup = .14f; Uppercut.Active = .12f; Uppercut.Recovery = .28f;
    Uppercut.Damage = 35; Uppercut.DeltaSpeed = 700; Uppercut.Reach = 155; Uppercut.Radius = 24;
}

void UIBS35PunchComponent::BeginPlay()
{
    Super::BeginPlay(); InstigatorId = FGuid::NewGuid();
    if (auto* Grip = GetOwner()->FindComponentByClass<UIBS35GripComponent>())
    { InstigatorId = Grip->InstigatorId; AddTickPrerequisiteComponent(Grip); }
}

bool UIBS35PunchComponent::BeginPunch(bool bHeavy)
{
    return BeginAttack(bHeavy ? EIBS35PunchStyle::Heavy : EIBS35PunchStyle::Quick);
}

bool UIBS35PunchComponent::BeginUppercut() { return BeginAttack(EIBS35PunchStyle::Uppercut); }
bool UIBS35PunchComponent::BeginChargedPunch(float Charge01)
{
    return FMath::IsFinite(Charge01) && BeginAttack(EIBS35PunchStyle::Heavy, FMath::Clamp(Charge01, 0.f, 1.f));
}

bool UIBS35PunchComponent::BeginAttack(EIBS35PunchStyle InStyle, float Charge01)
{
    if (bAttacking || !GetOwner() || !GetWorld()) return false;
    auto* Grip = GetOwner()->FindComponentByClass<UIBS35GripComponent>();
    AIBS35Object* Tool = Grip ? Grip->GetHeld() : nullptr;
    if (Grip && (Grip->IsCharging() || (Tool && (!Tool->Profile.bMeleeTool || Grip->GetGripWeight() < 1)))) return false;
    // A held prop keeps its existing box sweep; the fist-only rising curve cannot move cargo through a ceiling.
    if (Tool && InStyle == EIBS35PunchStyle::Uppercut) return false;
    Current = InStyle == EIBS35PunchStyle::Uppercut ? Uppercut : InStyle == EIBS35PunchStyle::Heavy ? Heavy : Quick;
    IBS35::PunchCombo NextCombo = Combo;
    if (InStyle == EIBS35PunchStyle::Quick)
    {
        const int Step = NextCombo.Begin();
        if (Step == 2) { Current.Windup *= .85f; Current.Damage *= 1.1f; Current.Reach += 10; }
        if (Step == 3) { Current.Windup *= 1.1f; Current.Recovery *= 1.2f; Current.Damage *= 1.35f; Current.DeltaSpeed *= 1.25f; Current.Reach += 20; }
        Current.Reach = FMath::Min(250.f, Current.Reach);
    }
    else NextCombo.Reset();
    if (Charge01 >= 0)
    {
        Current.Damage *= float(IBS35::ChargedDamageScale(Charge01));
        Current.DeltaSpeed *= float(IBS35::ChargedImpulseScale(Charge01));
        Current.Windup *= FMath::Lerp(1.f, .65f, Charge01);
    }
    Timing = {Current.Windup, Current.Active, Current.Recovery};
    if (!Timing.Valid() || !FMath::IsFinite(Current.Damage) || Current.Damage < 0
        || !FMath::IsFinite(Current.DeltaSpeed) || Current.DeltaSpeed < 0
        || !FMath::IsFinite(Current.Reach) || Current.Reach < 50 || Current.Reach > 250
        || !FMath::IsFinite(Current.Radius) || Current.Radius <= 0 || Current.Radius > 50) return false;
    bTool = Tool != nullptr; AttackTool = Tool; bHeavyAttack = InStyle != EIBS35PunchStyle::Quick;
    Style = InStyle; Combo = NextCombo;
    if (Tool)
    {
        ToolStart = Tool->GetActorLocation(); Grip->SetToolActive(true);
        // Heavier tools telegraph longer; remains the same native box sweep.
        Timing.Windup *= 1 + Grip->GetGestureLoad() * 0.5;
    }
    AttackId = FGuid::NewGuid(); Age = 0; bAttacking = true; bContactCue = false; bPathBlocked = false; bHasContactPosition = false;
    AttackOrigin = GetOwner()->GetActorLocation(); AttackUp = GetOwner()->GetActorUpVector().GetSafeNormal();
    AttackDirection = GetOwner()->GetActorForwardVector();
    if (const auto* Pawn = Cast<APawn>(GetOwner())) AttackDirection = Pawn->GetBaseAimRotation().Vector();
    const auto* Character = Cast<ACharacter>(GetOwner());
    if (InStyle == EIBS35PunchStyle::Uppercut || (Character && Character->GetCharacterMovement()->IsMovingOnGround()))
        AttackDirection = FVector::VectorPlaneProject(AttackDirection, AttackUp);
    AttackDirection = AttackDirection.GetSafeNormal(SMALL_NUMBER, GetOwner()->GetActorForwardVector());
    UE_LOG(LogTemp, Display, TEXT("IBS35 attack=%s heavy=%d tool=%d origin=%s windup=%.4f active=%.4f recovery=%.4f"),
        *AttackId.ToString(), bHeavyAttack, bTool, *AttackOrigin.ToString(), Timing.Windup, Timing.Active, Timing.Recovery);
    OnAttackCue.Broadcast(TEXT("Windup"), 0);
    return true;
}

void UIBS35PunchComponent::CancelAttack()
{
    FinishAttack(true);
}

void UIBS35PunchComponent::FinishAttack(bool bCancelled)
{
    if (auto* Grip = GetOwner()->FindComponentByClass<UIBS35GripComponent>()) Grip->SetToolActive(false);
    const bool bWasActive = bAttacking;
    bAttacking = false; AttackId.Invalidate(); AttackTool.Reset();
    if (bCancelled) Combo.Reset();
    else if (Style == EIBS35PunchStyle::Quick) Combo.Complete();
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
    if (!bAttacking || bPathBlocked) return;
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
        const bool Rising = Style == EIBS35PunchStyle::Uppercut;
        Start = AttackOrigin + AttackUp * (Rising ? FMath::Lerp(5.0,100.0,FromAlpha) : 40.0)
            + AttackDirection * FMath::Lerp(50.0, double(Current.Reach), FromAlpha);
        End = AttackOrigin + AttackUp * (Rising ? FMath::Lerp(5.0,100.0,ToAlpha) : 40.0)
            + AttackDirection * FMath::Lerp(50.0, double(Current.Reach), ToAlpha);
        // Carry the swept fist through ordinary locomotion without skipping the space between frames.
        if (bHasContactPosition) Start = LastContactPosition;
        FCollisionQueryParams Query(SCENE_QUERY_STAT(IBS35Punch), false, GetOwner());
        Query.bReturnPhysicalMaterial = true;
        GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, ECC_Visibility,
            FCollisionShape::MakeSphere(Current.Radius), Query);
    }
    LastContactPosition = End; bHasContactPosition = true;
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
    if (!Hit.bBlockingHit || !Body || (!Target && !CanonicalStrike.IsBound())) return;
    FIBS35DamageMessage Message;
    Message.AttackId = AttackId; Message.InstigatorId = InstigatorId;
    if (Target) Message.TargetId = Target->TargetId; // Capture the contacted life once; never repair a rejected ID.
    Message.InstigatorActor = GetOwner(); Message.TargetPart = Body;
    Message.DamageKind = bTool ? EIBS35DamageKind::HeldTool
        : bHeavyAttack ? EIBS35DamageKind::HeavyPunch : EIBS35DamageKind::QuickPunch;
    Message.HealthDelta = -Current.Damage;
    Message.Contact = Hit.ImpactPoint; Message.AttackSeconds = Age;
    const float TargetMass = Body->IsSimulatingPhysics() ? FMath::Max(1.f, Body->GetMass()) : 80;
    const FVector ImpactDirection = Style == EIBS35PunchStyle::Uppercut ? (AttackDirection * .35 + AttackUp).GetSafeNormal() : AttackDirection;
    Message.Impulse = ImpactDirection * Current.DeltaSpeed * TargetMass;
    Message.RelativeVelocity = (AttackDirection * (Current.Reach - 50)
        + (Style == EIBS35PunchStyle::Uppercut ? AttackUp * 95 : FVector::ZeroVector)) / Timing.Active - Body->GetComponentVelocity();
    Message.SurfaceMaterial = Hit.PhysMaterial.IsValid() ? Hit.PhysMaterial->GetFName() : NAME_None;
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
        Message.bAirborne = !Character->GetCharacterMovement()->IsMovingOnGround();
    const bool Accepted = CanonicalStrike.IsBound() ? CanonicalStrike.Execute(Message, Hit) : Target->Receive(Message, Body);
    if (Accepted) OnHit.Broadcast(Message);
}

void UIBS35PunchComponent::TickComponent(float Dt, ELevelTick TickType, FActorComponentTickFunction* Function)
{
    Super::TickComponent(Dt, TickType, Function);
    if (Dt <= 0 || !FMath::IsFinite(Dt)) return;
    if (!bAttacking) { Combo.Tick(Dt); return; }
    const FVector Motion = GetOwner()->GetActorLocation() - AttackOrigin;
    const double AllowedMotion = FMath::Max(250.0, GetOwner()->GetVelocity().Size() * Dt * 1.5 + 50.0);
    // Flying punches follow ordinary velocity; a teleport still cancels instead of leaving an old-world hitbox.
    if (Motion.ContainsNaN() || Motion.SizeSquared() > FMath::Square(AllowedMotion)) { CancelAttack(); return; }
    const FVector NewUp = GetOwner()->GetActorUpVector().GetSafeNormal();
    AttackDirection = FQuat::FindBetweenNormals(AttackUp, NewUp).RotateVector(AttackDirection);
    AttackUp = NewUp; AttackOrigin += Motion; ToolStart += Motion;
    const double Previous = Age; Age += Dt;
    if (Timing.CrossesContact(Previous, Age))
    {
        if (!bContactCue) { bContactCue = true; OnAttackCue.Broadcast(TEXT("Contact"), 0.5f); }
        if (!bUseNotifyContact) TraceContact(Timing.Alpha(Previous), Timing.Alpha(Age));
    }
    if (bAttacking && Age >= Timing.Duration())
    {
        const double RemainingFrame = Age - Timing.Duration();
        FinishAttack(false);
        Combo.Tick(RemainingFrame);
    }
}

void UIBS35PunchComponent::EndPlay(const EEndPlayReason::Type Reason) { CancelAttack(); CanonicalStrike.Unbind(); Super::EndPlay(Reason); }
