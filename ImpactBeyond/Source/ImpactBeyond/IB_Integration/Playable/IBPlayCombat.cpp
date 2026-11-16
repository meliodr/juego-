#include "IBPlayCombat.h"
#include "IB_Integration/ImpactContract/ImpactContractUE.h"
#include "IB_S35/IBS35ReceiverComponent.h"
#include "IB_S6H/IBS6HBridge.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "Misc/ScopeExit.h"

struct FIBPlayCombatRuntime
{
    FGuid Session = FGuid::NewGuid();
    FGuid Life;
    FGuid ActiveAttack;
    float InitialHealth = 100;
    int64 SerialFloor = 0;
    bool bDispatching = false;
    bool bPendingReset = false;
    TMap<TWeakObjectPtr<UPrimitiveComponent>, FGuid> Parts;
    TMap<TWeakObjectPtr<AActor>, FGuid> Instigators;
    TUniquePtr<IBImpactUE::S4Bridge> Bridge;

    FGuid Part(UPrimitiveComponent* Body)
    {
        if (!IsValid(Body)) return {};
        const TWeakObjectPtr<UPrimitiveComponent> Key(Body);
        if (const auto* Id = Parts.Find(Key)) return *Id;
        if (Parts.Num() >= 4096) return {};
        return Parts.Add(Key, FGuid::NewGuid());
    }
    FGuid Instigator(AActor* Actor)
    {
        if (!IsValid(Actor)) return {};
        const TWeakObjectPtr<AActor> Key(Actor);
        if (const auto* Id = Instigators.Find(Key)) return *Id;
        if (Instigators.Num() >= 4096) return {};
        return Instigators.Add(Key, FGuid::NewGuid());
    }
};

FGuid UIBPlayCombat::NativeCauseId(int64 AttackSerial)
{
    if (AttackSerial <= 0) return {};
    // S4 allocates serials globally and monotonically. Keep the complete serial,
    // with a process namespace so a saved civic cause cannot alias a later run.
    static const FGuid ProcessNamespace = FGuid::NewGuid();
    const uint64 Serial = static_cast<uint64>(AttackSerial);
    return FGuid(ProcessNamespace.A, ProcessNamespace.B,
        static_cast<uint32>(Serial >> 32), static_cast<uint32>(Serial));
}

UIBPlayCombat::UIBPlayCombat() { PrimaryComponentTick.bCanEverTick = false; }
UIBPlayCombat::UIBPlayCombat(FVTableHelper& Helper) : Super(Helper) {}
UIBPlayCombat::~UIBPlayCombat() = default;
void UIBPlayCombat::BeginPlay() { Super::BeginPlay(); InitializeReceiver(); }

bool UIBPlayCombat::InitializeReceiver()
{
    if (bEndingPlay) return false;
    if (Runtime) return Response && ContactReceiver && Runtime->Bridge.IsValid();
    if (!GetOwner() || !GetWorld()) return false;
    Response = GetOwner()->FindComponentByClass<UIBS4ResponseComponent>();
    ContactReceiver = GetOwner()->FindComponentByClass<UIBS35ReceiverComponent>();
    // Circuit panels and specialized threats retain their existing canonical owner.
    if ((Response && Response->HasConsumerBinding()) || (ContactReceiver && ContactReceiver->HasCanonicalIngress())) return false;
    if (!Response)
    {
        Response = NewObject<UIBS4ResponseComponent>(GetOwner());
        // S4 stores health on a 0..100 scale. Preserve an existing S35 target's
        // health fraction when opting it into that owner, including dead targets.
        if (ContactReceiver && FMath::IsFinite(ContactReceiver->InitialHealth) && ContactReceiver->InitialHealth > 0)
            Response->Health = FMath::Clamp(ContactReceiver->Health / ContactReceiver->InitialHealth * 100.f, 0.f, 100.f);
        GetOwner()->AddInstanceComponent(Response);
        Response->RegisterComponent();
    }
    if (!IBS4::validHealth(Response->Health)) return false;
    if (!ContactReceiver)
    {
        ContactReceiver = NewObject<UIBS35ReceiverComponent>(GetOwner());
        ContactReceiver->InitialHealth = Response->Health;
        GetOwner()->AddInstanceComponent(ContactReceiver);
        ContactReceiver->RegisterComponent();
    }
    if (!ContactReceiver->TargetId.IsValid()) ContactReceiver->ResetReceiver();
    Runtime = MakeUnique<FIBPlayCombatRuntime>();
    Runtime->InitialHealth = Response->Health;
    Runtime->Life = ContactReceiver->TargetId;
    Runtime->Bridge = MakeUnique<IBImpactUE::S4Bridge>(*Response, *this, Runtime->Session,
        Runtime->Life, ContactReceiver->ResponseProfile.IsNone() ? FName(TEXT("Playable")) : ContactReceiver->ResponseProfile);
    FIBS4IngressRouter Ingress = FIBS4IngressRouter::CreateUObject(this, &UIBPlayCombat::RouteNativeS4);
    if (!Response->BindConsumer(this, Ingress)) { Runtime.Reset(); return false; }
    FIBS35CanonicalIngress Contacts = FIBS35CanonicalIngress::CreateUObject(this, &UIBPlayCombat::RouteContact);
    if (!ContactReceiver->BindCanonicalIngress(this, Contacts))
    {
        Response->UnbindConsumer(this);
        Runtime.Reset();
        return false;
    }
    Response->SetBoundDamageFilter(this, FIBS4BoundDamageFilter::CreateUObject(this, &UIBPlayCombat::FilterDamage));
    ResetHandle = ContactReceiver->OnReceiverReset.AddUObject(this, &UIBPlayCombat::ReceiverReset);
    ContactReceiver->Health = Response->Health;
    RefreshHeroState();
    return true;
}

void UIBPlayCombat::EndPlay(const EEndPlayReason::Type Reason)
{
    bEndingPlay = true;
    if (ContactReceiver)
    {
        ContactReceiver->OnReceiverReset.Remove(ResetHandle);
        ContactReceiver->UnbindCanonicalIngress(this);
    }
    if (Response) Response->UnbindConsumer(this);
    if (!Runtime || !Runtime->bDispatching) Runtime.Reset();
    Super::EndPlay(Reason);
}

void UIBPlayCombat::RefreshHeroState()
{
    if (Response) if (const auto* Heroes = GetOwner()->FindComponentByClass<UIBS6HBridge>())
        Response->bH01 = Heroes->ActiveHero() == 0;
}

bool UIBPlayCombat::DispatchS35(const FIBS35DamageMessage& Input, const FHitResult& Hit)
{
    if (!InitializeReceiver() || Runtime->bDispatching) return false;
    if (Runtime->bPendingReset) ReceiverReset();
    UPrimitiveComponent* Body = Hit.GetComponent();
    if (!IsValid(Body) || Body->GetOwner() != GetOwner() || Hit.GetActor() != GetOwner()
        || !Input.AttackId.IsValid() || !Input.InstigatorId.IsValid()
        || (Input.TargetId.IsValid() && Input.TargetId != Runtime->Life)) return false;
    FIBS35DamageMessage Message = Input;
    Message.TargetId = Runtime->Life;
    if (Message.TargetPart.IsStale()) return false;
    if (Message.TargetPart.IsValid() && Message.TargetPart.Get() != Body) return false;
    Message.TargetPart = Body;
    const FGuid Part = Runtime->Part(Body);
    if (!Part.IsValid()) return false;
    RefreshHeroState();
    ON_SCOPE_EXIT {
        if (bEndingPlay) Runtime.Reset();
        else if (Runtime && Runtime->bPendingReset) ReceiverReset();
    };
    TGuardValue<bool> Guard(Runtime->bDispatching, true);
    TGuardValue<FGuid> Attack(Runtime->ActiveAttack, Message.AttackId);
    const auto Result = Runtime->Bridge->ReceiveS35(Message, Body, Part, ContactReceiver->MaxDeltaSpeed);
    if (Result.Result.State != IBImpact::Status::Accepted) return false;
    ContactReceiver->Health = Response->Health;
    ContactReceiver->AcceptedHits = Response->AcceptedHits;
    Message.RequestedHealthDelta = Result.Result.Applied.RequestedHealthDelta;
    Message.HealthDelta = Result.Result.Applied.HealthDelta;
    Message.DamageResponseProfile = ContactReceiver->ResponseProfile;
    ContactReceiver->OnDamage.Broadcast(Message);
    OnCanonicalImpact.Broadcast(Message.AttackId, Message.InstigatorActor.Get(), Message.Contact,
        FMath::Max(0.f, -Message.RequestedHealthDelta));
    return true;
}

bool UIBPlayCombat::RouteContact(FIBS35DamageMessage& Message, UPrimitiveComponent* Body)
{
    // Existing S35 producers capture TargetId at contact. Never repair a stale life here.
    if (!Runtime || Message.TargetId != Runtime->Life || !IsValid(Body) || Body->GetOwner() != GetOwner()) return false;
    FHitResult Hit(GetOwner(), Body, Message.Contact, FVector::UpVector);
    Hit.bBlockingHit = true;
    if (!DispatchS35(Message, Hit)) return false;
    Message.RequestedHealthDelta = Response->LastOutcome.RequestedHealthDelta;
    Message.HealthDelta = Response->LastOutcome.HealthDelta;
    Message.DamageResponseProfile = ContactReceiver->ResponseProfile;
    return true;
}

bool UIBPlayCombat::DispatchS4(const FIBS4Hit& Input, const FHitResult& Trace)
{
    if (!InitializeReceiver() || Runtime->bDispatching) return false;
    if (Runtime->bPendingReset) ReceiverReset();
    if (Input.AttackId <= Runtime->SerialFloor) return false;
    FIBS4Hit Hit = Input;
    if (!Hit.Part) Hit.Part = Trace.GetComponent();
    if (!IsValid(Hit.Part) || Hit.Part->GetOwner() != GetOwner()) return false;
    if (Trace.GetComponent() && Trace.GetComponent() != Hit.Part) return false;
    const FGuid AttackGuid = NativeCauseId(Hit.AttackId);
    const FGuid Part = Runtime->Part(Hit.Part);
    if (!AttackGuid.IsValid() || !Part.IsValid()) return false;
    IBImpactUE::S4Identity Identity;
    Identity.SessionId = Runtime->Session;
    Identity.TargetId = Runtime->Life;
    Identity.InstigatorId = Runtime->Instigator(Hit.Instigator);
    Identity.PartId = Part;
    RefreshHeroState();
    ON_SCOPE_EXIT {
        if (bEndingPlay) Runtime.Reset();
        else if (Runtime && Runtime->bPendingReset) ReceiverReset();
    };
    TGuardValue<bool> Guard(Runtime->bDispatching, true);
    TGuardValue<FGuid> Attack(Runtime->ActiveAttack, AttackGuid);
    const auto Result = Runtime->Bridge->ReceiveS4(Hit, Identity);
    if (Result.Result.State != IBImpact::Status::Accepted) return false;
    ContactReceiver->Health = Response->Health;
    ContactReceiver->AcceptedHits = Response->AcceptedHits;
    OnCanonicalImpact.Broadcast(AttackGuid, Hit.Instigator, Hit.Point,
        FMath::Max(0.f, -float(Result.Result.Applied.RequestedHealthDelta)));
    return true;
}

void UIBPlayCombat::RouteNativeS4(UIBS4ResponseComponent& Receiver, const FIBS4Hit& Hit)
{
    if (&Receiver != Response || !IsValid(Hit.Part)) return;
    FHitResult Trace(GetOwner(), Hit.Part, Hit.Point, FVector::UpVector);
    Trace.bBlockingHit = true;
    DispatchS4(Hit, Trace);
}

float UIBPlayCombat::FilterDamage(const FIBS4Hit& Hit, float Damage)
{
    if (!Runtime || !Runtime->bDispatching || !Runtime->ActiveAttack.IsValid()) return 0;
    auto* Heroes = GetOwner()->FindComponentByClass<UIBS6HBridge>();
    if (!Heroes) return Damage;
    if (IsValid(Hit.Instigator)) Heroes->NotifyIncomingHit(Hit.Instigator->GetActorLocation());
    return FMath::Max(0.f, Damage - Heroes->Absorb(Runtime->ActiveAttack, Damage));
}

FGuid UIBPlayCombat::GetTargetLife() const { return Runtime ? Runtime->Life : FGuid(); }
float UIBPlayCombat::GetHealth() const { return Response ? Response->Health : 0; }

void UIBPlayCombat::ReceiverReset()
{
    if (!Runtime || !Response || !ContactReceiver) return;
    if (Runtime->bDispatching) { Runtime->bPendingReset = true; return; }
    Runtime->bPendingReset = false;
    Runtime->Life = ContactReceiver->TargetId;
    Runtime->Parts.Empty();
    Runtime->SerialFloor = UIBS4ResponseComponent::AllocateAttackId();
    Response->Health = Runtime->InitialHealth;
    ContactReceiver->Health = Response->Health;
    Runtime->Bridge = MakeUnique<IBImpactUE::S4Bridge>(*Response, *this, Runtime->Session,
        Runtime->Life, ContactReceiver->ResponseProfile.IsNone() ? FName(TEXT("Playable")) : ContactReceiver->ResponseProfile);
}

void UIBPlayCombat::ResetCombat()
{
    if (!InitializeReceiver() || Runtime->bDispatching) return;
    ContactReceiver->ResetReceiver();
    if (auto* Heroes = GetOwner()->FindComponentByClass<UIBS6HBridge>()) Heroes->ResetAbilities();
}
