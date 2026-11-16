#include "IBS7TThreatActor.h"
#include "ContactGate.h"
#include "IB_Integration/ImpactContract/ImpactContractUE.h"
#include "IB_S35/IBS35Object.h"
#include "IB_S4/IBS4PreparedTarget.h"
#include "IB_S4/IBS4PresentationComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/LineBatchComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Animation/AnimSequence.h"

namespace
{
const TCHAR* PhaseName(IBS7T::State State)
{
    switch (State)
    {
    case IBS7T::State::Patrol: return TEXT("Patrol");
    case IBS7T::State::Notice: return TEXT("Notice");
    case IBS7T::State::Telegraph: return TEXT("Telegraph");
    case IBS7T::State::Attack: return TEXT("Attack");
    case IBS7T::State::Recover: return TEXT("Recover");
    case IBS7T::State::Stagger: return TEXT("Stagger");
    default: return TEXT("Retreat");
    }
}

bool ValidTransform(const FTransform& Transform)
{
    const FVector Scale = Transform.GetScale3D();
    return !Transform.ContainsNaN() && Transform.GetRotation().IsNormalized()
        && Scale.X > 0 && Scale.Y > 0 && Scale.Z > 0;
}
}

AIBS7TThreatActor::AIBS7TThreatActor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;
    Body = CreateDefaultSubobject<UBoxComponent>(TEXT("PreparedBody"));
    SetRootComponent(Body);
    Body->SetCollisionProfileName(TEXT("BlockAllDynamic"));
    Visual = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ThreatVisual"));
    Visual->SetupAttachment(Body);
    Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Visual->SetGenerateOverlapEvents(false);
    TelegraphLines = CreateDefaultSubobject<ULineBatchComponent>(TEXT("ThreatTelegraph"));
    TelegraphLines->SetupAttachment(Body);
    TelegraphLines->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    TelegraphLines->bCalculateAccurateBounds = true;
    TelegraphLines->PrimaryComponentTick.bCanEverTick = false;
    WarningText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("ThreatWarning"));
    WarningText->SetupAttachment(Body);
    WarningText->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WarningText->SetHorizontalAlignment(EHTA_Center);
    WarningText->SetWorldSize(35);
    WarningText->SetVisibility(false);
    Feedback = CreateDefaultSubobject<UIBS4PresentationComponent>(TEXT("ThreatFeedback"));
    Feedback->Volume = 0.12f;
    for (int32 Index = 0; Index < 3; ++Index)
    {
        UBoxComponent* Box = CreateDefaultSubobject<UBoxComponent>(*FString::Printf(TEXT("PartBox%d"), Index));
        Box->SetupAttachment(Body);
        Box->SetCollisionProfileName(TEXT("BlockAllDynamic"));
        PartBoxes.Add(Box);
        UStaticMeshComponent* Mesh = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("AuthorizedArt%d"), Index));
        Mesh->SetupAttachment(Box);
        Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Modules.Add(Mesh);
    }
    LoosePieces.SetNum(3);
    PartBones.SetNum(3);
}

void AIBS7TThreatActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    const float Scale = bKaiju ? 2.f : 1.f;
    Body->SetBoxExtent(FVector(100, 100, 200) * Scale);
    for (int32 Index = 0; Index < 3; ++Index)
    {
        PartBoxes[Index]->SetBoxExtent(FVector(45, 45, 75) * Scale);
        PartBoxes[Index]->SetRelativeLocation(FVector(Index == 1 ? -130 : 130,
            Index == 2 ? 0 : (Index == 0 ? 140 : -140), Index == 2 ? 130 : 0) * Scale);
        PartBoxes[Index]->SetCollisionEnabled((Index < 2 || bKaiju)
            ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
    }
}

void AIBS7TThreatActor::BeginPlay()
{
    Super::BeginPlay();
    bRuntimeKaiju = bKaiju;
    Model = MakeUnique<IBS7T::Threat>(static_cast<uint64>(GetUniqueID()) + 1, bRuntimeKaiju);
    RenewLife();
    SyncParts();
}

void AIBS7TThreatActor::EndPlay(const EEndPlayReason::Type Reason)
{
    ReleaseCapture();
    // Held parts belong to the grip owner until released; world unload destroys them normally.
    for (AIBS35Object* Piece : LoosePieces)
        if (IsValid(Piece) && !Piece->IsHeld()) Piece->Destroy();
    CanonicalStrike.Unbind();
    Super::EndPlay(Reason);
}

void AIBS7TThreatActor::RenewLife()
{
    TargetLifeId = FGuid::NewGuid();
    // S4 messages carry monotonic serials, not target GUIDs. Retire all packets
    // captured before this life even when a host supplies the current life at dispatch.
    NativeSerialFloor = UIBS4ResponseComponent::AllocateAttackId();
    ImpactRouter.Reset();
    if (SessionId.IsValid())
        ImpactRouter = MakeUnique<IBImpact::TargetRouter<>>(IBImpactUE::Id(SessionId), IBImpactUE::Id(TargetLifeId));
    OutgoingEpoch = 0;
    OutgoingAttack = 0;
}

bool AIBS7TThreatActor::ConfigureIdentity(FName Identity, FGuid WorldSession, FVector Center)
{
    if (Identity.IsNone() || !WorldSession.IsValid() || Center.ContainsNaN()) return false;
    // A live identity cannot be switched after it has accepted damage.
    if (ImpactRouter && ImpactRouter->Size() > 0
        && (StableId != Identity || SessionId != WorldSession)) return false;
    StableId = Identity;
    PlanetCenter = Center;
    if (SessionId != WorldSession || !ImpactRouter)
    {
        SessionId = WorldSession;
        RenewLife();
    }
    return true;
}

void AIBS7TThreatActor::SetRegionReady(bool bReady)
{
    bRegionReady = bReady;
    if (!bReady) ReleaseCapture();
    if (!bReady) { TelegraphLines->Flush(); WarningText->SetVisibility(false); }
}

FVector AIBS7TThreatActor::Up() const
{
    return bRadialGravity ? (GetActorLocation() - PlanetCenter).GetSafeNormal(SMALL_NUMBER, GetActorUpVector()) : FVector::UpVector;
}

IBS7T::PartId AIBS7TThreatActor::PartAt(int32 Index) const
{
    if (bRuntimeKaiju)
        return Index == 0 ? IBS7T::PartId::Horn : Index == 1 ? IBS7T::PartId::Tail : IBS7T::PartId::Shoulder;
    return Index == 0 ? IBS7T::PartId::Hammer : IBS7T::PartId::Claw;
}

void AIBS7TThreatActor::SyncParts()
{
    if (!Model) return;
    for (int32 Index = 0; Index < 3; ++Index)
    {
        const bool Attached = (Index < 2 || bRuntimeKaiju) && Model->attached(PartAt(Index));
        PartBoxes[Index]->SetCollisionEnabled(Attached ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
        const bool HasBone = PartBones.IsValidIndex(Index) && !PartBones[Index].IsNone()
            && Visual->GetBoneIndex(PartBones[Index]) != INDEX_NONE;
        const bool ExternalModule = !PartBones.IsValidIndex(Index) || PartBones[Index].IsNone();
        // Extracted subtrees use the animated rig while attached. An external
        // module has no matching subtree, so its static mesh remains visible.
        Modules[Index]->SetVisibility(Attached && !HasBone && (!bUseSkeletalParts || ExternalModule), true);
        if (PartBones.IsValidIndex(Index) && !PartBones[Index].IsNone() && Visual->GetBoneIndex(PartBones[Index]) != INDEX_NONE)
        {
            if (Attached) Visual->UnHideBoneByName(PartBones[Index]);
            else Visual->HideBoneByName(PartBones[Index], EPhysBodyOp::PBO_None);
        }
    }
    Generation = static_cast<int64>(Model->epoch());
    BodyHealth = static_cast<float>(Model->health());
    if (BodyHealth <= 0) ReleaseCapture();
}

bool AIBS7TThreatActor::ConfigurePartArt(EIBS7TPart Part, UStaticMesh* Mesh, FName BoneName)
{
    if (!Model || !Mesh || (!BoneName.IsNone() && Visual->GetBoneIndex(BoneName) == INDEX_NONE)) return false;
    int32 Index = INDEX_NONE;
    for (int32 Candidate = 0; Candidate < (bRuntimeKaiju ? 3 : 2); ++Candidate)
        if (static_cast<EIBS7TPart>(PartAt(Candidate)) == Part) Index = Candidate;
    if (Index == INDEX_NONE || !Model->attached(PartAt(Index))) return false;
    const FBoxSphereBounds Bounds = Mesh->GetBounds();
    if (Bounds.BoxExtent.ContainsNaN() || Bounds.BoxExtent.GetMax() <= 0) return false;
    if (PartBones.Num() < 3) PartBones.SetNum(3);
    PartBones[Index] = BoneName;
    Modules[Index]->SetStaticMesh(Mesh);
    PartBoxes[Index]->SetBoxExtent(Bounds.BoxExtent.ComponentMax(FVector(2)));
    Modules[Index]->SetRelativeTransform(FTransform(FQuat::Identity, -Bounds.Origin));
    UpdatePartTransforms();
    SyncParts();
    return true;
}

void AIBS7TThreatActor::UpdatePartTransforms()
{
    if (!Model) return;
    for (int32 Index = 0; Index < (bRuntimeKaiju ? 3 : 2); ++Index)
    {
        UStaticMesh* Mesh = Modules[Index]->GetStaticMesh();
        if (!Mesh || !Model->attached(PartAt(Index)) || !PartBones.IsValidIndex(Index) || PartBones[Index].IsNone()
            || Visual->GetBoneIndex(PartBones[Index]) == INDEX_NONE) continue;
        const FTransform Bone = Visual->GetSocketTransform(PartBones[Index], RTS_World);
        // Keep colliders parented to Body: hiding a skeletal bone scales its subtree to zero.
        // A detached collider retains its last full bounds for physical-piece save/restore.
        PartBoxes[Index]->SetWorldTransform(FTransform(FQuat::Identity, Mesh->GetBounds().Origin) * Bone);
    }
}

EIBS7TPart AIBS7TThreatActor::IdentifyPart(UPrimitiveComponent* Component) const
{
    for (int32 Index = 0; Index < (bRuntimeKaiju ? 3 : 2); ++Index)
        if (Component == PartBoxes[Index] || Component == Modules[Index])
            return static_cast<EIBS7TPart>(PartAt(Index));
    return EIBS7TPart::Body;
}

AIBS35Object* AIBS7TThreatActor::GetLoosePiece(EIBS7TPart Part) const
{
    for (int32 Index = 0; Index < (bRuntimeKaiju ? 3 : 2); ++Index)
        if (static_cast<EIBS7TPart>(PartAt(Index)) == Part) return LoosePieces[Index];
    return nullptr;
}

UPrimitiveComponent* AIBS7TThreatActor::GetPartCollider(EIBS7TPart Part) const
{
    for (int32 Index = 0; Index < (bRuntimeKaiju ? 3 : 2); ++Index)
        if (static_cast<EIBS7TPart>(PartAt(Index)) == Part) return PartBoxes[Index];
    return Part == EIBS7TPart::Body ? Body.Get() : nullptr;
}

bool AIBS7TThreatActor::ApplyEnvelope(const IBImpact::Envelope& Envelope, UPrimitiveComponent* Component)
{
    if (!ImpactRouter || !Model || !GetWorld() || !bRegionReady || !IsValid(Component)
        || Component->GetOwner() != this) return false;
    const auto Result = ImpactRouter->Route(Envelope, GetWorld()->GetTimeSeconds(),
        [this](const IBImpact::Envelope& Request)
        {
            FHitResult Trace;
            Trace.ImpactPoint = IBImpactUE::Vec(Request.Contact);
            const auto Packet = UIBS4ResponseComponent::MakeHit(GetWorld(), nullptr, EIBS4Weapon::Punch, 0, Trace);
            return IBImpact::S4Packet{Packet.AttackId, Packet.IssuedAt};
        },
        [this](const IBImpact::Envelope& Request, const IBImpact::S4Packet& Packet)
        {
            IBImpact::Outcome Outcome;
            const auto Part = static_cast<EIBS7TPart>(Request.PartId.D - 1);
            if (Request.PartId.D == 0 || Request.PartId.D > 6 || Request.RequestedHealthDelta >= 0) return Outcome;
            const IBS7T::Part* PriorPart = Part == EIBS7TPart::Body ? nullptr : Model->part(static_cast<IBS7T::PartId>(Part));
            const double Before = PriorPart ? PriorPart->health : Model->health();
            const bool Accepted = Part == EIBS7TPart::Body
                ? Model->damageBody(Model->epoch(), Packet.Serial, -Request.RequestedHealthDelta)
                : Model->damagePart(Model->epoch(), Packet.Serial, static_cast<IBS7T::PartId>(Part), -Request.RequestedHealthDelta);
            if (!Accepted) return Outcome;
            const IBS7T::Part* AfterPart = Part == EIBS7TPart::Body ? nullptr : Model->part(static_cast<IBS7T::PartId>(Part));
            Outcome.Accepted = true;
            Outcome.RequestedHealthDelta = Request.RequestedHealthDelta;
            Outcome.HealthDelta = (AfterPart ? AfterPart->health : Model->health()) - Before;
            Outcome.PhysicalResponse = Request.PhysicsResolved
                ? IBImpact::ImpulseDisposition::AlreadyResolved : IBImpact::ImpulseDisposition::NotSimulating;
            return Outcome;
        });
    SyncParts();
    return Result.State == IBImpact::Status::Accepted;
}

bool AIBS7TThreatActor::AcceptCanonicalImpact(const FIBS35DamageMessage& Message, const FHitResult& Hit)
{
    UPrimitiveComponent* Component = Hit.GetComponent();
    IBImpact::S35Kind Kind;
    if (!GetWorld() || !IsValid(Component) || Hit.GetActor() != this || Component->GetOwner() != this
        || Message.TargetId != TargetLifeId || Message.TargetPart.IsStale()
        || (Message.TargetPart.IsValid() && Message.TargetPart.Get() != Component)
        || !IBImpactUE::Kind(Message.DamageKind, Kind)) return false;
    auto Part = IdentifyPart(Component);
    if (Component == Visual && !Hit.BoneName.IsNone())
        for (int32 Index = 0; Index < (bRuntimeKaiju ? 3 : 2); ++Index)
            if (PartBones.IsValidIndex(Index) && PartBones[Index] == Hit.BoneName)
                Part = static_cast<EIBS7TPart>(PartAt(Index));
    IBImpact::Envelope Envelope;
    if (!IBImpact::SetS35Request(Kind, Message.HealthDelta, Envelope)) return false;
    Envelope.Attack = {IBImpact::Origin::S35, IBImpactUE::Id(SessionId), IBImpactUE::Id(Message.AttackId), 0};
    Envelope.TargetId = IBImpactUE::Id(TargetLifeId);
    Envelope.InstigatorId = IBImpactUE::Id(Message.InstigatorId);
    Envelope.PartId = {0, 0, 1, static_cast<uint32>(Part) + 1};
    Envelope.Contact = IBImpactUE::Vec(Message.Contact);
    Envelope.Impulse = IBImpactUE::Vec(Message.Impulse);
    Envelope.RelativeVelocity = IBImpactUE::Vec(Message.RelativeVelocity);
    Envelope.Direction = IBImpactUE::Vec(Message.RelativeVelocity.GetSafeNormal());
    Envelope.HasRelativeVelocity = true;
    Envelope.PhysicsResolved = Message.bPhysicsResolved;
    Envelope.Airborne = Message.bAirborne;
    Envelope.AttackAgeSeconds = Message.AttackSeconds;
    Envelope.HasAttackAge = true;
    Envelope.ContactWorldSeconds = GetWorld()->GetTimeSeconds();
    Envelope.DamageResponseProfile = "S7PreparedThreat";
    return ApplyEnvelope(Envelope, Component);
}

bool AIBS7TThreatActor::AcceptCanonicalS4Impact(const FIBS4Hit& Hit, FGuid CapturedTargetLife)
{
    IBImpact::S4Weapon Weapon;
    if (!GetWorld() || Hit.AttackId <= NativeSerialFloor || CapturedTargetLife != TargetLifeId || !IsValid(Hit.Part)
        || Hit.Part->GetOwner() != this || !IBImpactUE::Kind(Hit.Weapon, Weapon)) return false;
    IBImpact::Envelope Envelope;
    if (!IBImpact::SetS4Request(Weapon, Hit.Amount, Envelope)) return false;
    Envelope.Attack = {IBImpact::Origin::S4, IBImpactUE::Id(SessionId), {}, Hit.AttackId};
    Envelope.TargetId = IBImpactUE::Id(TargetLifeId);
    Envelope.PartId = {0, 0, 1, static_cast<uint32>(IdentifyPart(Hit.Part)) + 1};
    Envelope.Contact = IBImpactUE::Vec(Hit.Point);
    Envelope.Direction = IBImpactUE::Vec(Hit.Direction);
    Envelope.ContactWorldSeconds = Hit.IssuedAt;
    Envelope.DamageResponseProfile = "S7PreparedThreat";
    return ApplyEnvelope(Envelope, Hit.Part);
}

bool AIBS7TThreatActor::ReceiveImpact(int64 Epoch, int64 AttackSerial, EIBS7TPart Part, float Damage)
{
    // Historical fixture seam; production uses the canonical router exclusively.
    if (ImpactRouter || !Model || Epoch <= 0 || AttackSerial <= 0 || !FMath::IsFinite(Damage) || Damage <= 0
        || static_cast<uint8>(Part) > static_cast<uint8>(EIBS7TPart::Body)) return false;
    const bool Accepted = Part == EIBS7TPart::Body ? Model->damageBody(Epoch, AttackSerial, Damage)
        : Model->damagePart(Epoch, AttackSerial, static_cast<IBS7T::PartId>(Part), Damage);
    SyncParts();
    return Accepted;
}

AIBS35Object* AIBS7TThreatActor::SpawnLoosePiece(int32 Index, const FTransform& Transform)
{
    if (!GetWorld() || !Modules.IsValidIndex(Index) || !Modules[Index]->GetStaticMesh()
        || !ValidTransform(Transform)) return nullptr;
    FIBS35ObjectProfile Profile = FIBS35ObjectProfile::Preset(EIBS35ObjectClass::LoosePart);
    Profile.HalfExtent = PartBoxes[Index]->GetScaledBoxExtent().ComponentMax(FVector(5));
    Profile.MassKg = bRuntimeKaiju ? 250.f : 120.f;
    if (!Profile.IsValid()) return nullptr;
    AIBS35Object* Piece = GetWorld()->SpawnActorDeferred<AIBS35Object>(
        AIBS35Object::StaticClass(), Transform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
    if (!Piece) return nullptr;
    Piece->Profile = Profile;
    Piece->Profile.Variant = FName(*FString::Printf(TEXT("%s_%d"), *StableId.ToString(), Index));
    Piece->bRadialGravity = bRadialGravity;
    Piece->PlanetCenter = PlanetCenter;
    Piece->Appearance->SetStaticMesh(Modules[Index]->GetStaticMesh());
    for (int32 Slot = 0; Slot < Modules[Index]->GetNumMaterials(); ++Slot)
        Piece->Appearance->SetMaterial(Slot, Modules[Index]->GetMaterial(Slot));
    Piece->FinishSpawning(Transform);
    return Piece;
}

bool AIBS7TThreatActor::DetachExposed(EIBS7TPart Part)
{
    if (!Model || !bRegionReady || Part == EIBS7TPart::Body) return false;
    int32 Index = INDEX_NONE;
    for (int32 Candidate = 0; Candidate < (bRuntimeKaiju ? 3 : 2); ++Candidate)
        if (static_cast<EIBS7TPart>(PartAt(Candidate)) == Part) Index = Candidate;
    const auto* Prepared = Model->part(static_cast<IBS7T::PartId>(Part));
    if (Index == INDEX_NONE || !Prepared || Prepared->state != IBS7T::PartState::Exposed) return false;
    const FVector Away = (PartBoxes[Index]->GetComponentLocation() - GetActorLocation()).GetSafeNormal(SMALL_NUMBER, GetActorRightVector());
    const FVector LocalAway = GetActorQuat().UnrotateVector(Away).GetAbs();
    const double Clearance = FVector::DotProduct(Body->GetScaledBoxExtent() + PartBoxes[Index]->GetScaledBoxExtent(), LocalAway) + 5;
    const FTransform Transform(PartBoxes[Index]->GetComponentQuat(), GetActorLocation() + Away * Clearance);
    AIBS35Object* Piece = SpawnLoosePiece(Index, Transform);
    if (!Piece) return false;
    if (!Model->detach(static_cast<IBS7T::PartId>(Part), LogicalDebris))
    {
        Piece->Destroy();
        return false;
    }
    LoosePieces[Index] = Piece;
    // Remove connected collision before making the loose piece collide with its former owner.
    SyncParts();
    ReleaseCapture();
    Piece->Body->AddImpulse(Up() * Piece->Profile.MassKg * 100.f);
    PartRemoved(Part);
    return true;
}

EIBS7TPart AIBS7TThreatActor::FindExposedPart(const FHitResult& Hit, float ToleranceCm) const
{
    if (!Model || !bRegionReady || Hit.GetActor() != this || !IsValid(Hit.GetComponent())
        || Hit.GetComponent()->GetOwner() != this || Hit.ImpactPoint.ContainsNaN()
        || !FMath::IsFinite(ToleranceCm) || ToleranceCm < 0) return EIBS7TPart::Body;
    const EIBS7TPart Exact = IdentifyPart(Hit.GetComponent());
    if (Exact != EIBS7TPart::Body)
    {
        const auto* Part = Model->part(static_cast<IBS7T::PartId>(Exact));
        if (Part && Part->state == IBS7T::PartState::Exposed) return Exact;
    }
    EIBS7TPart Nearest = EIBS7TPart::Body;
    double BestSquared = FMath::Square(FMath::Min(150.f,ToleranceCm));
    for (int32 Index = 0; Index < (bRuntimeKaiju ? 3 : 2); ++Index)
    {
        const auto* Part = Model->part(PartAt(Index));
        if (!Part || Part->state != IBS7T::PartState::Exposed) continue;
        const UBoxComponent* Box = PartBoxes[Index];
        const FVector Local = Box->GetComponentQuat().UnrotateVector(Hit.ImpactPoint-Box->GetComponentLocation());
        const FVector Extent = Box->GetScaledBoxExtent();
        const FVector Closest(FMath::Clamp(Local.X,-Extent.X,Extent.X),FMath::Clamp(Local.Y,-Extent.Y,Extent.Y),FMath::Clamp(Local.Z,-Extent.Z,Extent.Z));
        const double Distance = (Local-Closest).SizeSquared();
        if (Distance <= BestSquared) { BestSquared = Distance; Nearest = static_cast<EIBS7TPart>(PartAt(Index)); }
    }
    return Nearest;
}

AIBS35Object* AIBS7TThreatActor::TryDetachAt(const FHitResult& Hit, float ToleranceCm)
{
    const EIBS7TPart Part = FindExposedPart(Hit,ToleranceCm);
    if (Part == EIBS7TPart::Body || !DetachExposed(Part)) return nullptr;
    return GetLoosePiece(Part);
}

bool AIBS7TThreatActor::AnyPieceHeld() const
{
    for (const AIBS35Object* Piece : LoosePieces)
        if (IsValid(Piece) && Piece->IsHeld()) return true;
    return false;
}

bool AIBS7TThreatActor::ResetThreat()
{
    if (!Model || !GetWorld() || AnyPieceHeld()) return false;
    FCollisionQueryParams Query(SCENE_QUERY_STAT(IBS7TRestore), false, this);
    for (const AIBS35Object* Piece : LoosePieces) if (IsValid(Piece)) Query.AddIgnoredActor(Piece);
    for (const UBoxComponent* Box : PartBoxes)
        if (GetWorld()->OverlapBlockingTestByChannel(Box->GetComponentLocation(), Box->GetComponentQuat(), ECC_Pawn,
            FCollisionShape::MakeBox(Box->GetScaledBoxExtent()), Query)) return false;
    if (!Model->reset(LogicalDebris, true)) return false;
    ReleaseCapture();
    for (AIBS35Object* Piece : LoosePieces) if (IsValid(Piece)) Piece->Destroy();
    for (auto& Piece : LoosePieces) Piece = nullptr;
    bAlternate = false;
    AcceptedContacts = 0;
    RenewLife();
    SyncParts();
    return true;
}

void AIBS7TThreatActor::MoveThreat(float DeltaSeconds)
{
    if (!Model || !bRegionReady || Model->health() <= 0 || !IsValid(Target)) return;
    const auto Phase = Model->phase();
    const bool Charging = Phase == IBS7T::State::Attack && Model->spec().move == IBS7T::Move::Charge;
    const bool Pursuing = Phase == IBS7T::State::Patrol || Phase == IBS7T::State::Notice;
    if (!Charging && !Pursuing) return;
    const FVector UpDirection = Up();
    const FVector Toward = FVector::VectorPlaneProject(Target->GetActorLocation() - GetActorLocation(), UpDirection);
    const FVector Direction = FVector::VectorPlaneProject(Charging ? LockedAim : Toward, UpDirection).GetSafeNormal();
    if (Direction.IsNearlyZero()) return;
    const double StopDistance = Model->spec().rangeCm * 0.75;
    if (!Charging && Toward.Size() <= StopDistance) return;
    const double Speed = Charging ? ChargeSpeed : MoveSpeed;
    if (!FMath::IsFinite(Speed) || Speed <= 0) return;
    FHitResult Hit;
    const FQuat Rotation = FRotationMatrix::MakeFromXZ(Direction, UpDirection).ToQuat();
    Body->MoveComponent(Direction * Speed * FMath::Min(DeltaSeconds, 0.05f), Rotation, true, &Hit);
    if (bRadialGravity)
    {
        // Short ground probe keeps the kinematic body aligned with curved, collidable terrain.
        FHitResult Ground;
        FCollisionQueryParams Query(SCENE_QUERY_STAT(IBS7TGround), false, this);
        const double HalfHeight = Body->GetScaledBoxExtent().Z;
        const FVector Start = GetActorLocation();
        if (GetWorld()->LineTraceSingleByChannel(Ground, Start, Start - Up() * (HalfHeight + 250),
            ECC_WorldStatic, Query) && !Ground.bStartPenetrating)
        {
            const FVector Correction = Up() * (HalfHeight + 2 - Ground.Distance);
            Body->MoveComponent(Correction.GetClampedToMaxSize(40), Body->GetComponentQuat(), true);
        }
    }
}

bool AIBS7TThreatActor::TryCapture(AActor* Actor)
{
    ACharacter* Hero = Cast<ACharacter>(Actor);
    if (!Hero || CapturedHero || !bRegionReady || !Hero->GetCharacterMovement()
        || Hero->GetCharacterMovement()->MovementMode == MOVE_None) return false;
    auto* Movement = Hero->GetCharacterMovement();
    SavedMovementMode = static_cast<uint8>(Movement->MovementMode.GetValue());
    SavedCustomMovementMode = Movement->CustomMovementMode;
    SavedVelocity = Movement->Velocity;
    CapturedHero = Hero;
    GrabRemaining = FMath::Clamp(GrabHoldSeconds, 0.05f, 2.f);
    Movement->DisableMovement();
    return true;
}

void AIBS7TThreatActor::ReleaseCapture()
{
    if (IsValid(CapturedHero))
    {
        auto* Movement = CapturedHero->GetCharacterMovement();
        // Restore only the lock this threat still owns.
        if (Movement && Movement->MovementMode == MOVE_None)
        {
            Movement->SetMovementMode(static_cast<EMovementMode>(SavedMovementMode), SavedCustomMovementMode);
            Movement->Velocity = SavedVelocity.GetClampedToMaxSize(1200);
        }
    }
    CapturedHero = nullptr;
    GrabRemaining = 0;
}

void AIBS7TThreatActor::UpdateCapture(float DeltaSeconds)
{
    if (!IsValid(CapturedHero)) { CapturedHero = nullptr; return; }
    if (!bRegionReady || !Model || Model->health() <= 0 || !Model->attached(IBS7T::PartId::Claw)
        || CapturedHero->GetCharacterMovement()->MovementMode != MOVE_None)
    {
        ReleaseCapture();
        return;
    }
    GrabRemaining -= DeltaSeconds;
    if (GrabRemaining <= 0) { ReleaseCapture(); return; }
    const FVector Desired = GetActorLocation() + GetActorForwardVector() * (Body->GetScaledBoxExtent().X + 120) + Up() * 50;
    FHitResult Hit;
    CapturedHero->SetActorLocation(FMath::VInterpTo(CapturedHero->GetActorLocation(), Desired, DeltaSeconds, 8), true, &Hit);
    if (Hit.bBlockingHit && Hit.GetActor() != this) ReleaseCapture();
}

void AIBS7TThreatActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!Model || !GetWorld() || !FMath::IsFinite(DeltaSeconds) || DeltaSeconds <= 0) return;
    bool Visible = false;
    if (IsValid(Target) && bRegionReady)
    {
        FHitResult Sight;
        FCollisionQueryParams Query(SCENE_QUERY_STAT(IBS7TSight), false, this);
        const bool Blocked = GetWorld()->LineTraceSingleByChannel(Sight, GetActorLocation(), Target->GetActorLocation(), ECC_Visibility, Query);
        Visible = (!Blocked || Sight.GetActor() == Target)
            && FVector::Dist(GetActorLocation(), Target->GetActorLocation()) <= PerceptionRange;
    }
    const auto Before = Model->phase();
    if (Before == IBS7T::State::Patrol)
        Model->choose(bRuntimeKaiju ? (bAlternate ? IBS7T::Move::TailSweep : IBS7T::Move::Charge)
            : (bAlternate ? IBS7T::Move::Grab : IBS7T::Move::Hammer));
    const bool InAttackRange = Visible && IsValid(Target)
        && FVector::Dist(GetActorLocation(), Target->GetActorLocation()) <= Model->spec().rangeCm;
    Model->tick(DeltaSeconds, InAttackRange, bRegionReady);
    if (Model->phase() != Before)
    {
        PhaseLabel = PhaseName(Model->phase());
        if (Model->phase() == IBS7T::State::Telegraph && IsValid(Target))
        {
            LockedAim = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
            TelegraphStarted = GetWorld()->GetTimeSeconds();
            if (bShowTelegraphs && Feedback) Feedback->Impact(GetActorLocation(),
                bRuntimeKaiju ? EIBS4Surface::Concrete : EIBS4Surface::Metal,false);
        }
        if (Model->phase() == IBS7T::State::Recover) bAlternate = !bAlternate;
        PhaseChanged(PhaseLabel, static_cast<int32>(Model->spec().move), static_cast<float>(Model->spec().telegraph));
    }
    const FVector BeforeMove = GetActorLocation();
    MoveThreat(DeltaSeconds);
    UpdateTelegraph();
    UpdatePartTransforms();
    MovementSpeed = static_cast<float>(FVector::Dist(BeforeMove, GetActorLocation()) / DeltaSeconds);
    UAnimSequence* DesiredAnimation = Model->phase() == IBS7T::State::Attack ? AttackAnimation.Get()
        : MovementSpeed > 5 ? WalkAnimation.Get() : IdleAnimation.Get();
    if (DesiredAnimation && (ActiveAnimation != DesiredAnimation
        || (Model->phase() == IBS7T::State::Attack && Before != IBS7T::State::Attack)))
    {
        Visual->PlayAnimation(DesiredAnimation, Model->phase() != IBS7T::State::Attack);
        ActiveAnimation = DesiredAnimation;
    }
    if (Model->phase() == IBS7T::State::Attack) SweepAttack();
    UpdateCapture(DeltaSeconds);
}

void AIBS7TThreatActor::UpdateTelegraph()
{
    TelegraphLines->Flush();
    const bool Visible = Model && bShowTelegraphs && bRegionReady && Model->phase() == IBS7T::State::Telegraph;
    WarningText->SetVisibility(Visible);
    if (!Visible) return;
    const auto Spec = Model->spec();
    const FVector LocalUp = Up();
    const FVector Forward = FVector::VectorPlaneProject(LockedAim,LocalUp).GetSafeNormal(SMALL_NUMBER,GetActorForwardVector());
    const FVector Right = FVector::CrossProduct(LocalUp,Forward).GetSafeNormal();
    const double Progress = FMath::Clamp((GetWorld()->GetTimeSeconds()-TelegraphStarted)/Spec.telegraph,0.0,1.0);
    const FLinearColor Color = FLinearColor::LerpUsingHSV(FLinearColor(1,0.65,0.02),FLinearColor(1,0.05,0.01),static_cast<float>(Progress));
    FVector Ground = GetActorLocation()-LocalUp*Body->GetScaledBoxExtent().Z;
    FHitResult GroundHit;
    FCollisionQueryParams Query(SCENE_QUERY_STAT(IBS7TelegraphGround),false,this);
    if (GetWorld()->LineTraceSingleByChannel(GroundHit,GetActorLocation(),Ground-LocalUp*250,ECC_WorldStatic,Query)) Ground = GroundHit.ImpactPoint;
    Ground += LocalUp*7;
    TArray<FBatchedLine,TInlineAllocator<40>> Lines;
    const auto Line = [&](FVector A,FVector B)
    { Lines.Emplace(A,B,Color,0.f,5.f,static_cast<uint8>(0)); };
    if (Spec.move == IBS7T::Move::Charge)
    {
        const double HalfWidth = 105;
        const FVector End = Ground+Forward*Spec.rangeCm;
        Line(Ground-Right*HalfWidth,End-Right*HalfWidth); Line(Ground+Right*HalfWidth,End+Right*HalfWidth);
        Line(End-Right*HalfWidth,End+Right*HalfWidth);
        Line(End,End-Forward*130+Right*100); Line(End,End-Forward*130-Right*100);
    }
    else
    {
        const FVector Center = Spec.move == IBS7T::Move::TailSweep ? Ground : Ground+Forward*Spec.rangeCm*0.55;
        const double Radius = Spec.move == IBS7T::Move::TailSweep ? Spec.rangeCm : Spec.move == IBS7T::Move::Grab ? 150 : 220;
        constexpr int32 Segments = 28;
        for (int32 Index = 0; Index < Segments; ++Index)
        {
            const double A = 2*PI*Index/Segments, B = 2*PI*(Index+1)/Segments;
            Line(Center+(Forward*FMath::Cos(A)+Right*FMath::Sin(A))*Radius,
                Center+(Forward*FMath::Cos(B)+Right*FMath::Sin(B))*Radius);
        }
    }
    TelegraphLines->DrawLines(MakeArrayView(Lines));
    const TCHAR* Label = Spec.move == IBS7T::Move::Charge ? TEXT("CARGA")
        : Spec.move == IBS7T::Move::TailSweep ? TEXT("BARRIDO") : Spec.move == IBS7T::Move::Grab ? TEXT("AGARRE") : TEXT("GOLPE");
    WarningText->SetText(FText::FromString(Label));
    WarningText->SetTextRenderColor(Color.ToFColor(true));
    WarningText->SetWorldLocation(GetActorLocation()+LocalUp*(Body->GetScaledBoxExtent().Z+65));
    WarningText->SetWorldSize(bRuntimeKaiju ? 55 : 35);
    if (APlayerController* Player = GetWorld()->GetFirstPlayerController())
    {
        FVector CameraLocation; FRotator CameraRotation;
        Player->GetPlayerViewPoint(CameraLocation,CameraRotation);
        const FVector TowardCamera = (CameraLocation-WarningText->GetComponentLocation()).GetSafeNormal();
        WarningText->SetWorldRotation(FRotationMatrix::MakeFromXZ(TowardCamera,LocalUp).Rotator());
    }
}

void AIBS7TThreatActor::SweepAttack()
{
    const auto Spec = Model->spec();
    const FVector Start = GetActorLocation();
    FHitResult Contact;
    FCollisionQueryParams Query(SCENE_QUERY_STAT(IBS7TAttack), false, this);
    if (!GetWorld()->SweepSingleByChannel(Contact, Start, Start + LockedAim * Spec.rangeCm, FQuat::Identity,
        ECC_Visibility, FCollisionShape::MakeSphere(Spec.move == IBS7T::Move::TailSweep ? 100.f : 50.f), Query)
        || Contact.bStartPenetrating) return;
    const double Distance = FVector::Dist(Start, Contact.ImpactPoint);
    if (!IBS7T::validContact(Distance, Spec.rangeCm, Contact.bStartPenetrating, bRegionReady)) return;
    AActor* Victim = Contact.GetActor();
    if (!IsValid(Victim)) return;
    IBS7T::Patch Patch = IBS7T::Patch::Unprepared;
    if (const auto* Prepared = Cast<AIBS4PreparedTarget>(Victim))
        Patch = Prepared->Kind == EIBS4TargetKind::Crater ? IBS7T::Patch::Terrain : IBS7T::Patch::Building;
    else if (Cast<ACharacter>(Victim) || Victim->FindComponentByClass<UIBS4ResponseComponent>())
        Patch = IBS7T::Patch::Character;
    struct FPort final : IBS7T::DamagePort
    {
        AIBS7TThreatActor* Source = nullptr;
        FHitResult Trace;
        bool apply(const IBS7T::HitRequest& Request) override
        {
            return Source->DispatchPreparedContact(Trace, static_cast<int64>(Request.generation),
                static_cast<int64>(Request.attack), static_cast<int32>(Request.move), static_cast<float>(Request.damage));
        }
    } Port;
    Port.Source = this;
    Port.Trace = Contact;
    if (Model->hit(static_cast<uint64>(Victim->GetUniqueID()) + 1, Distance, true, bRegionReady, Patch, Port))
    {
        ++AcceptedContacts;
        if (Spec.move == IBS7T::Move::Grab) TryCapture(Victim);
    }
}

bool AIBS7TThreatActor::DispatchPreparedContact_Implementation(const FHitResult& Contact, int64 Epoch, int64 LocalAttack, int32 Move, float Amount)
{
    if (!CanonicalStrike.IsBound() || !Model || !bRegionReady || Epoch != Generation
        || LocalAttack != static_cast<int64>(Model->attackId()) || Move != static_cast<int32>(Model->spec().move)
        || !IsValid(Contact.GetActor()) || !IsValid(Contact.GetComponent())) return false;
    if (OutgoingEpoch != Epoch || OutgoingAttack != LocalAttack)
    {
        OutgoingPacket = UIBS4ResponseComponent::MakeHit(GetWorld(), this, EIBS4Weapon::Punch, Amount, Contact);
        OutgoingEpoch = Epoch;
        OutgoingAttack = LocalAttack;
    }
    FIBS4Hit Hit = OutgoingPacket;
    Hit.Point = Contact.ImpactPoint;
    Hit.Part = Contact.GetComponent();
    Hit.Direction = LockedAim;
    return CanonicalStrike.Execute(Hit, Contact);
}

bool AIBS7TThreatActor::CaptureSnapshot(FIBS7TThreatSnapshot& Snapshot) const
{
    if (!Model || StableId.IsNone() || AnyPieceHeld() || IsValid(CapturedHero)) return false;
    FIBS7TThreatSnapshot Result;
    Result.StableId = StableId;
    Result.bKaiju = bRuntimeKaiju;
    Result.Transform = GetActorTransform();
    Result.Health = BodyHealth;
    for (int32 Index = 0; Index < (bRuntimeKaiju ? 3 : 2); ++Index)
    {
        const auto* Part = Model->part(PartAt(Index));
        FIBS7TPartSnapshot Saved;
        Saved.Part = static_cast<EIBS7TPart>(PartAt(Index));
        Saved.Health = static_cast<float>(Part->health);
        Saved.State = static_cast<uint8>(Part->state);
        if (IsValid(LoosePieces[Index]))
        {
            Saved.bLoosePiece = true;
            Saved.LooseTransform = LoosePieces[Index]->GetActorTransform();
            Saved.LinearVelocity = LoosePieces[Index]->Body->GetPhysicsLinearVelocity();
            Saved.AngularVelocity = LoosePieces[Index]->Body->GetPhysicsAngularVelocityInRadians();
        }
        Result.Parts.Add(Saved);
    }
    Snapshot = MoveTemp(Result);
    return true;
}

bool AIBS7TThreatActor::CanRestoreSnapshot(const FIBS7TThreatSnapshot& Snapshot) const
{
    if (!Model || !GetWorld() || StableId.IsNone() || Snapshot.Version != 1 || Snapshot.StableId != StableId
        || Snapshot.bKaiju != bRuntimeKaiju || Snapshot.Parts.Num() != static_cast<int32>(Model->partCount())
        || !ValidTransform(Snapshot.Transform) || !Snapshot.Transform.GetScale3D().Equals(GetActorScale3D(),0.001)
        || AnyPieceHeld() || IsValid(CapturedHero)) return false;
    IBS7T::ThreatSnapshot Native = Model->snapshot();
    Native.health = Snapshot.Health;
    for (int32 Index = 0; Index < Snapshot.Parts.Num(); ++Index)
    {
        const auto& Saved = Snapshot.Parts[Index];
        if (Saved.Part != static_cast<EIBS7TPart>(PartAt(Index))
            || Saved.LinearVelocity.ContainsNaN() || Saved.AngularVelocity.ContainsNaN()
            || Saved.LinearVelocity.SizeSquared() > FMath::Square(100000.f)
            || Saved.AngularVelocity.SizeSquared() > FMath::Square(100000.f)
            || (Saved.bLoosePiece && (!ValidTransform(Saved.LooseTransform)
                || !Saved.LooseTransform.GetScale3D().Equals(FVector::OneVector,0.001)
                || !Modules[Index]->GetStaticMesh()
                || Saved.State != static_cast<uint8>(IBS7T::PartState::Detached)))) return false;
        if (Saved.bLoosePiece)
        {
            auto Profile = FIBS35ObjectProfile::Preset(EIBS35ObjectClass::LoosePart);
            Profile.HalfExtent = PartBoxes[Index]->GetScaledBoxExtent().ComponentMax(FVector(5));
            if (!Profile.IsValid()) return false;
        }
        Native.parts[Index] = {PartAt(Index), Saved.Health, static_cast<IBS7T::PartState>(Saved.State)};
    }
    IBS7T::Threat Trial = *Model;
    IBS7T::DebrisPool TrialDebris = LogicalDebris;
    if (!Trial.restore(Native, TrialDebris)) return false;
    FCollisionQueryParams Query(SCENE_QUERY_STAT(IBS7THydrate), false, this);
    for (const AIBS35Object* Piece : LoosePieces) if (IsValid(Piece)) Query.AddIgnoredActor(Piece);
    const FVector Extent = Body->GetUnscaledBoxExtent() * Snapshot.Transform.GetScale3D();
    if (GetWorld()->OverlapBlockingTestByChannel(Snapshot.Transform.GetLocation(), Snapshot.Transform.GetRotation(),
        ECC_Pawn, FCollisionShape::MakeBox(Extent), Query)) return false;
    for (int32 Index = 0; Index < Snapshot.Parts.Num(); ++Index)
    {
        const auto& Saved = Snapshot.Parts[Index];
        if (Saved.State != static_cast<uint8>(IBS7T::PartState::Detached))
        {
            const FTransform PartTransform = PartBoxes[Index]->GetComponentTransform().GetRelativeTransform(GetActorTransform()) * Snapshot.Transform;
            const FVector PartExtent = PartBoxes[Index]->GetUnscaledBoxExtent() * PartTransform.GetScale3D();
            if (GetWorld()->OverlapBlockingTestByChannel(PartTransform.GetLocation(), PartTransform.GetRotation(),
                ECC_Pawn, FCollisionShape::MakeBox(PartExtent), Query)) return false;
        }
        if (Saved.bLoosePiece && GetWorld()->OverlapBlockingTestByChannel(Saved.LooseTransform.GetLocation(),
            Saved.LooseTransform.GetRotation(), ECC_Pawn,
            FCollisionShape::MakeBox(PartBoxes[Index]->GetScaledBoxExtent().ComponentMax(FVector(5))), Query)) return false;
    }
    return true;
}

bool AIBS7TThreatActor::RestoreSnapshot(const FIBS7TThreatSnapshot& Snapshot)
{
    if (!CanRestoreSnapshot(Snapshot)) return false;
    IBS7T::ThreatSnapshot Native = Model->snapshot();
    Native.health = Snapshot.Health;
    for (int32 Index = 0; Index < Snapshot.Parts.Num(); ++Index)
    {
        const auto& Saved = Snapshot.Parts[Index];
        Native.parts[Index] = {PartAt(Index), Saved.Health, static_cast<IBS7T::PartState>(Saved.State)};
    }
    TArray<AIBS35Object*> Created;
    Created.SetNumZeroed(3);
    for (int32 Index = 0; Index < Snapshot.Parts.Num(); ++Index)
    {
        const auto& Saved = Snapshot.Parts[Index];
        if (!Saved.bLoosePiece) continue;
        Created[Index] = SpawnLoosePiece(Index, Saved.LooseTransform);
        if (!Created[Index])
        {
            for (AIBS35Object* Piece : Created) if (Piece) Piece->Destroy();
            return false;
        }
    }
    if (!Model->restore(Native, LogicalDebris))
    {
        for (AIBS35Object* Piece : Created) if (Piece) Piece->Destroy();
        return false;
    }
    for (AIBS35Object* Piece : LoosePieces) if (IsValid(Piece)) Piece->Destroy();
    SetActorTransform(Snapshot.Transform, false, nullptr, ETeleportType::TeleportPhysics);
    for (int32 Index = 0; Index < 3; ++Index)
    {
        LoosePieces[Index] = Created[Index];
        if (Created[Index])
        {
            Created[Index]->Body->SetPhysicsLinearVelocity(Snapshot.Parts[Index].LinearVelocity);
            Created[Index]->Body->SetPhysicsAngularVelocityInRadians(Snapshot.Parts[Index].AngularVelocity);
        }
    }
    RenewLife();
    SyncParts();
    return true;
}
