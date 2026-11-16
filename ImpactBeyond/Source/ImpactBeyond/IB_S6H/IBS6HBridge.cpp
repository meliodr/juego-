#include "IBS6HBridge.h"
#include "IBS6HSelectorWidget.h"
#include "IB_S6/Wave8Heroes/IBS6Heroes.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundWaveProcedural.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ImpactCircuitComponent.h"
#include "InputCoreTypes.h"
#if __has_include("IB_S3/IBS3World.h")
#include "IB_S3/IBS3World.h"
#define IBS6H_HAS_S3 1
#else
#define IBS6H_HAS_S3 0
#endif
#if __has_include("IB_S35/IBS35GripComponent.h")
#include "IB_S35/IBS35GripComponent.h"
#include "IB_S35/IBS35Object.h"
#include "IB_S35/IBS35ReceiverComponent.h"
#define IBS6H_HAS_S35 1
#else
#define IBS6H_HAS_S35 0
#endif
#if __has_include("IB_S4/IBS4PowerComponent.h")
#include "IB_S4/IBS4PowerComponent.h"
#define IBS6H_HAS_S4 1
#else
#define IBS6H_HAS_S4 0
#endif

namespace {
FVector ToUnrealVector(IBS6H::Vec V) { return FVector(V.x,V.y,V.z); }
IBS6H::Vec Native(FVector V) { return {V.X,V.Y,V.Z}; }
}

struct FIBS6HRuntime final : IBS6H::WorldPort
{
    UIBS6HBridge& Host;
    TWeakObjectPtr<ACharacter> Character;
    TMap<uint64,TWeakObjectPtr<UBoxComponent>> Effects;
    TMap<uint64,TArray<TWeakObjectPtr<UStaticMeshComponent>>> Visuals;
    struct FTrail { TWeakObjectPtr<UStaticMeshComponent> Mesh; float Remaining = .25f; };
    TArray<FTrail> Trails;
    int32 ConstructStyle = 202;
    TMap<uint64,TWeakObjectPtr<AActor>> Targets;
    TMap<FGuid,uint64> Incoming;
    TWeakObjectPtr<AActor> GripTarget;
    FGuid Session = FGuid::NewGuid();
    uint64 Serial=0, OwnedGrip=0, IncomingShield=0;
    IBS6H::Heroes Heroes;
    IBS6H::Selector Selector;
    explicit FIBS6HRuntime(UIBS6HBridge& H):Host(H),Character(Cast<ACharacter>(H.GetOwner())),Heroes(*this),Selector(Heroes) {}
    ~FIBS6HRuntime() {
        Heroes.reset();
        for (const auto& Trail : Trails) if (Trail.Mesh.IsValid()) Trail.Mesh->DestroyComponent();
    }
    UStaticMeshComponent* Visual(USceneComponent* Parent, FVector Location, FVector Scale, FLinearColor Color) {
        if (!Character.IsValid() || !Host.EffectMesh) return nullptr;
        auto* Mesh = NewObject<UStaticMeshComponent>(Character.Get());
        Mesh->SetStaticMesh(Host.EffectMesh);
        Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Mesh->SetCanEverAffectNavigation(false);
        Mesh->SetCastShadow(false);
        if (Parent) {
            Mesh->SetupAttachment(Parent);
            Mesh->SetRelativeLocation(Location);
        } else {
            Mesh->SetWorldLocationAndRotation(Location, Character->GetActorQuat());
        }
        Mesh->SetRelativeScale3D(Scale);
        Character->AddInstanceComponent(Mesh);
        Mesh->RegisterComponent();
        if (Host.EnergyMaterial) {
            auto* Material = UMaterialInstanceDynamic::Create(Host.EnergyMaterial, Mesh);
            Material->SetVectorParameterValue(TEXT("Color"), Color);
            Material->SetVectorParameterValue(TEXT("BaseColor"), Color);
            Material->SetVectorParameterValue(TEXT("EmissiveColor"), Color * 3);
            Mesh->SetMaterial(0, Material);
        }
        return Mesh;
    }
    void TickPresentation(float Dt) {
        for (int32 Index = Trails.Num() - 1; Index >= 0; --Index) {
            auto& Trail = Trails[Index];
            Trail.Remaining -= Dt;
            if (!Trail.Mesh.IsValid() || Trail.Remaining <= 0) {
                if (Trail.Mesh.IsValid()) {
                    if (Character.IsValid()) Character->RemoveInstanceComponent(Trail.Mesh.Get());
                    Trail.Mesh->DestroyComponent();
                }
                Trails.RemoveAt(Index);
            }
        }
    }
    IBS6H::Id newId() override { return ++Serial; }
    FGuid AttackGuid(uint64 Id) const { return FGuid(Session.A,Session.B,uint32(Id>>32),uint32(Id)); }
    uint64 Target(AActor* Actor) {
        Targets.Empty(); // consumed synchronously; never retain a growing actor registry
        const uint64 Id=newId(); Targets.Add(Id,Actor); return Id;
    }
    IBS6H::Body body() const override {
        IBS6H::Body B;
        const ACharacter* C=Character.Get(); if(!C) { B.radius=0; return B; }
        const UCapsuleComponent* Cap=C->GetCapsuleComponent();
        B.position=Native(Cap->GetComponentLocation()); B.velocity=Native(C->GetVelocity());
        B.up=Native(Cap->GetUpVector()); B.radius=Cap->GetScaledCapsuleRadius();
        B.halfHeight=Cap->GetScaledCapsuleHalfHeight();
        B.airborne=C->GetCharacterMovement()->IsFlying()||C->GetCharacterMovement()->IsFalling();
        if(const UImpactCircuitComponent* Circuit=C->FindComponentByClass<UImpactCircuitComponent>()) B.carrying=Circuit->IsCarHeld();
#if IBS6H_HAS_S35
        if(const UIBS35GripComponent* Grip=C->FindComponentByClass<UIBS35GripComponent>()) B.carrying|=Grip->GetHeld()!=nullptr;
#endif
#if IBS6H_HAS_S4
        if(const UIBS4PowerComponent* Power=C->FindComponentByClass<UIBS4PowerComponent>()) B.carrying|=Power->IsHolding();
#endif
        return B; // Region ownership stays in S3; never fabricate numeric RegionId.
    }
    bool Ready(FVector Start,FVector End,double Extent) const {
#if IBS6H_HAS_S3
        bool Found=false;
        for(TActorIterator<AIBS3PlanetWorld> It(Host.GetWorld());It;++It) {
            Found=true;
            if(!It->IsOperational()||!It->CanTraverse(Start,End,Extent)) return false;
        }
        if(Found) return true;
#endif
        return Host.bAllowNonPlanetLab;
    }
    bool safeBlink(const IBS6H::BlinkRequest& R) const override {
        const ACharacter* C=Character.Get(); if(!C||!Host.GetWorld()||C->GetRootComponent()!=C->GetCapsuleComponent()) return false;
        if(!Ready(ToUnrealVector(R.origin.position),ToUnrealVector(R.destination),R.origin.halfHeight)) return false;
        const auto Now=body();
        if(Now.carrying||!ToUnrealVector(Now.position).Equals(ToUnrealVector(R.origin.position),.01)||
           !ToUnrealVector(Now.velocity).Equals(ToUnrealVector(R.origin.velocity),.01)||!ToUnrealVector(Now.up).Equals(ToUnrealVector(R.origin.up),.00001)||
           Now.radius!=R.origin.radius||Now.halfHeight!=R.origin.halfHeight||Now.airborne!=R.origin.airborne) return false;
        const auto* Cap=C->GetCapsuleComponent();
        const FQuat Rotation=Cap->GetComponentQuat();
        const auto Shape=FCollisionShape::MakeCapsule(float(Now.radius),float(Now.halfHeight));
        FCollisionQueryParams Q(SCENE_QUERY_STAT(IBS6HBlink),false,C);
        const FCollisionResponseParams Response(Cap->GetCollisionResponseToChannels());
        auto* W=Host.GetWorld(); const auto Channel=Cap->GetCollisionObjectType();
        if(W->OverlapBlockingTestByChannel(ToUnrealVector(R.origin.position),Rotation,Channel,Shape,Q,Response)||
           W->OverlapBlockingTestByChannel(ToUnrealVector(R.destination),Rotation,Channel,Shape,Q,Response)) return false;
        FHitResult Hit;
        if(W->SweepSingleByChannel(Hit,ToUnrealVector(R.origin.position),ToUnrealVector(R.destination),Rotation,Channel,Shape,Q,Response)) return false;
        // Grounded blink must land on support within 120 cm, aligned to capsule/radial up.
        if(!Now.airborne) {
            const FVector Foot=ToUnrealVector(R.destination)-ToUnrealVector(Now.up)*(Now.halfHeight-2);
            if(!W->LineTraceSingleByChannel(Hit,Foot,Foot-ToUnrealVector(Now.up)*120,Channel,Q,Response)||
               FVector::DotProduct(Hit.ImpactNormal,ToUnrealVector(Now.up))<.6) return false;
        }
        return true;
    }
    bool commitBlink(const IBS6H::BlinkRequest& R) override {
        // All checks and placement run synchronously on game thread, no latent work/delegates in between.
        if(!IsInGameThread()||!safeBlink(R)) return false;
        ACharacter* C=Character.Get(); const FVector Velocity=C->GetCharacterMovement()->Velocity;
        const bool Moved=C->SetActorLocation(ToUnrealVector(R.destination),false,nullptr,ETeleportType::TeleportPhysics);
        if(!Moved) return false;
        C->GetCharacterMovement()->Velocity=Velocity;
        for (const FVector Position : {ToUnrealVector(R.origin.position), ToUnrealVector(R.destination)}) {
            if (auto* Mesh = Visual(nullptr, Position, FVector(.12f, .6f, 1.7f), FLinearColor(.45f, .18f, 1.f)))
                Trails.Add({Mesh, .25f});
        }
#if IBS6H_HAS_S3
        for(TActorIterator<AIBS3PlanetWorld> It(Host.GetWorld());It;++It)
            It->UpdateResidency(C->GetActorLocation(),Velocity.Size());
#endif
        return true;
    }
    bool createEffect(uint64 Id,IBS6H::Effect Type) override {
        ACharacter* C=Character.Get(); if(!C||Effects.Contains(Id)) return false;
        const bool Shield=Type==IBS6H::Effect::Shield;
        const bool Hammer = !Shield && ConstructStyle == 203;
        const FVector Size=Shield?FVector(12,95,110):(Hammer?FVector(30,65,70):FVector(35,55,55));
        const FVector Offset=Shield?FVector(130,0,0):FVector(140,130,20);
        const FVector Position=C->GetActorTransform().TransformPosition(Offset);
        FCollisionQueryParams Q(SCENE_QUERY_STAT(IBS6HSpawn),false,C);
        // Include our existing effects: they must not overlap each other at spawn.
        for(const auto& Pair:Effects) if(Pair.Value.IsValid()) {
            const UBoxComponent* Box=Pair.Value.Get();
            const FBox SpawnBounds=FBox(-Size,Size).TransformBy(FTransform(C->GetActorQuat(),Position));
            if(Box->Bounds.GetBox().Intersect(SpawnBounds)) return false;
        }
        if(Host.GetWorld()->OverlapBlockingTestByChannel(Position,C->GetActorQuat(),ECC_Pawn,
            FCollisionShape::MakeBox(Size),Q)) return false;
        UBoxComponent* Box=NewObject<UBoxComponent>(C);
        Box->SetBoxExtent(Size); Box->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Box->SetCollisionObjectType(ECC_WorldDynamic); Box->SetCollisionResponseToAllChannels(ECR_Block);
        Box->SetCanEverAffectNavigation(false); Box->SetHiddenInGame(true);
        // World-anchored collider: never attach an unswept volume to a fast moving pawn.
        Box->SetWorldLocationAndRotation(Position,C->GetActorQuat());
        C->AddInstanceComponent(Box); Box->RegisterComponent();
        if(!Box->IsRegistered()) { C->RemoveInstanceComponent(Box); Box->DestroyComponent(); return false; }
        Box->IgnoreActorWhenMoving(C,true); C->GetCapsuleComponent()->IgnoreComponentWhenMoving(Box,true);
        Effects.Add(Id,Box);
        auto& Parts = Visuals.Add(Id);
        const FLinearColor Color(.05f, .9f, .38f);
        auto Add = [&](FVector Location, FVector Scale) {
            if (auto* Mesh = Visual(Box, Location, Scale, Color)) Parts.Add(Mesh);
        };
        if (Shield) {
            Add(FVector::ZeroVector, FVector(.16f, 1.8f, 2.1f));
        } else if (Hammer) {
            Add(FVector(0, 0, -20), FVector(.18f, .18f, 1.f));
            Add(FVector(0, 0, 45), FVector(.6f, 1.3f, .5f));
        } else {
            Add(FVector(-10, 0, -10), FVector(.35f, .8f, .65f));
            for (int32 Finger = 0; Finger < 4; ++Finger)
                Add(FVector(10, -30 + Finger * 20, 25), FVector(.25f, .13f, .55f));
            Add(FVector(10, 48, -15), FVector(.3f, .3f, .18f));
        }
        return Box->IsRegistered();
    }
    void removeEffect(uint64 Id) override {
        if (auto* Parts = Visuals.Find(Id)) for (const auto& Part : *Parts) if (Part.IsValid()) {
            if (Character.IsValid()) Character->RemoveInstanceComponent(Part.Get());
            Part->DestroyComponent();
        }
        Visuals.Remove(Id);
        if(auto* Entry=Effects.Find(Id)) if(UBoxComponent* Box=Entry->Get()) {
            if(ACharacter* C=Character.Get()) { C->GetCapsuleComponent()->IgnoreComponentWhenMoving(Box,false); C->RemoveInstanceComponent(Box); }
            Box->SetCollisionEnabled(ECollisionEnabled::NoCollision); Box->DestroyComponent();
        }
        Effects.Remove(Id);
    }
    bool effectExists(uint64 Id) const override {
        const auto* Entry=Effects.Find(Id); return Entry&&Entry->IsValid()&&Entry->Get()->IsCollisionEnabled();
    }
    bool grabWithConstruct(uint64 Id,uint64 TargetId) override {
#if IBS6H_HAS_S35
        ACharacter* C=Character.Get(); if(!C||!effectExists(Id)||OwnedGrip||body().carrying) return false;
        UIBS35GripComponent* Grip=C->FindComponentByClass<UIBS35GripComponent>();
        const auto* Entry=Targets.Find(TargetId);
        if(Grip&&Entry&&Grip->TryGrab(Cast<AIBS35Object>(Entry->Get()))) { OwnedGrip=Id; GripTarget=Entry->Get(); return true; }
#endif
        return false;
    }
    void dropConstruct(uint64 Id) override {
#if IBS6H_HAS_S35
        if(OwnedGrip==Id) if(ACharacter* C=Character.Get())
            if(UIBS35GripComponent* Grip=C->FindComponentByClass<UIBS35GripComponent>())
                if(GripTarget.IsValid()&&Grip->GetHeld()==GripTarget.Get()) Grip->Drop();
#endif
        if(OwnedGrip==Id) { OwnedGrip=0; GripTarget.Reset(); }
    }
    bool shortAttack(uint64 Attack,IBS6H::Hero Source,uint64 TargetId,double Range) override {
#if IBS6H_HAS_S35
        ACharacter* C=Character.Get(); const auto* Entry=Targets.Find(TargetId);
        AActor* TargetActor=Entry?Entry->Get():nullptr;
        if(!C||!TargetActor||TargetActor==C||Range<=0||Range>250) return false;
        FVector Direction = C->GetBaseAimRotation().Vector();
        if (!body().airborne) Direction = FVector::VectorPlaneProject(Direction, C->GetActorUpVector()).GetSafeNormal();
        if (Direction.IsNearlyZero()) Direction = C->GetActorForwardVector();
        const FVector Start=C->GetActorLocation(), End=Start+Direction*Range;
        FHitResult Hit; FCollisionQueryParams Q(SCENE_QUERY_STAT(IBS6HStrike),false,C);
        if(!Host.GetWorld()->SweepSingleByChannel(Hit,Start,End,FQuat::Identity,ECC_Visibility,
            FCollisionShape::MakeSphere(25),Q)||Hit.GetActor()!=TargetActor) return false;
        if(!Host.CanonicalStrike.IsBound()||!Hit.GetComponent()) return false;
        FIBS35DamageMessage Message;
        Message.AttackId=AttackGuid(Attack);
        Message.InstigatorId=Session; Message.InstigatorActor=C; Message.TargetPart=Hit.GetComponent();
        const bool Heavy = Source != IBS6H::Hero::H03 && (ConstructStyle == 203 || ConstructStyle == 102);
        Message.DamageKind=Heavy?EIBS35DamageKind::HeavyPunch:EIBS35DamageKind::QuickPunch;
        Message.HealthDelta=Heavy?-60:-25;
        if (ConstructStyle == 103) Message.Impulse = C->GetActorUpVector() * 35000;
        Message.Contact=Hit.ImpactPoint; Message.RelativeVelocity=C->GetVelocity();
        Message.AttackSeconds=.2f; Message.bAirborne=body().airborne;
        return Host.CanonicalStrike.Execute(Message,Hit);
#else
        return false;
#endif
    }
};

UIBS6HBridge::UIBS6HBridge() {
    PrimaryComponentTick.bCanEverTick=true;
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    EffectMesh = Cube.Object;
    EnergyMaterial = Material.Object;
}
UIBS6HBridge::UIBS6HBridge(FVTableHelper& Helper) : Super(Helper) {}
UIBS6HBridge::~UIBS6HBridge() = default;
void UIBS6HBridge::BeginPlay() { Super::BeginPlay(); if(Cast<ACharacter>(GetOwner())) Runtime=MakeUnique<FIBS6HRuntime>(*this); }
void UIBS6HBridge::EndPlay(const EEndPlayReason::Type Reason) {
    CloseSelector();
    UnbindLabInput();
    CanonicalStrike.Unbind();
    HostAbility.Unbind();
    HostReleaseAbility.Unbind();
    if (SelectorWidget) SelectorWidget->RemoveFromParent();
    SelectorWidget = nullptr;
    Runtime.Reset();
    Super::EndPlay(Reason);
}
void UIBS6HBridge::TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Fn) {
    Super::TickComponent(Dt,Type,Fn);
    if (!Runtime || !FMath::IsFinite(Dt) || Dt <= 0) return;
    Runtime->Heroes.tick(Dt);
    Runtime->TickPresentation(Dt);
    if (!Runtime->Heroes.paused()) {
        StrikeCooldown = FMath::Max(0.f, StrikeCooldown - Dt);
        ShieldCooldown = FMath::Max(0.f, ShieldCooldown - Dt);
        ConstructCooldown = FMath::Max(0.f, ConstructCooldown - Dt);
    }
}
int32 UIBS6HBridge::ActiveHero() const { return Runtime?int32(Runtime->Heroes.active()):0; }
bool UIBS6HBridge::SelectHero(int32 Index) {
    if(!Runtime||Index<0||Index>2) return false;
    const int32 PreviousHero = ActiveHero();
    const auto Result=Runtime->Heroes.select(static_cast<IBS6H::Hero>(Index));
    LastResult = Result == IBS6H::Result::Ok ? TEXT("") : TEXT("Termina la accion o suelta la carga para cambiar de heroe.");
    if (Result == IBS6H::Result::Ok) PublishHeroChange(PreviousHero);
    return Result==IBS6H::Result::Ok;
}
void UIBS6HBridge::Shield() { if (EquipAbility(201)) ActivateEquipped(); }
void UIBS6HBridge::Construct() { if (EquipAbility(202)) ActivateEquipped(); }
void UIBS6HBridge::Blink() { if (EquipAbility(301)) ActivateEquipped(); }
bool UIBS6HBridge::Grab(AActor* Target) { return Runtime&&IsValid(Target)&&Runtime->Heroes.grab(Runtime->Target(Target))==IBS6H::Result::Ok; }
bool UIBS6HBridge::Strike(AActor* Target) { return Runtime&&IsValid(Target)&&Runtime->Heroes.strike(Runtime->Target(Target))==IBS6H::Result::Ok; }
float UIBS6HBridge::Absorb(const FGuid& Attack,float Damage) {
    if(!Runtime||!Attack.IsValid()||!FMath::IsFinite(Damage)||Damage<=0) return 0;
    if(!Runtime->Heroes.shieldId()) { Runtime->Incoming.Empty(); return 0; }
    if(Runtime->IncomingShield!=Runtime->Heroes.shieldId()) {
        Runtime->Incoming.Empty(); Runtime->IncomingShield=Runtime->Heroes.shieldId();
    }
    // Bounded ledger: fail closed rather than evict IDs and re-absorb old hits.
    uint64* Id=Runtime->Incoming.Find(Attack);
    if(!Id) { if(Runtime->Incoming.Num()>=100) return 0; Id=&Runtime->Incoming.Add(Attack,Runtime->newId()); }
    return float(Runtime->Heroes.absorb(*Id,Damage));
}
void UIBS6HBridge::OpenSelector() {
    if(!Runtime||Runtime->Heroes.shieldId()||Runtime->Heroes.constructId()||Runtime->body().carrying) return;
    Runtime->Selector.open();
    LastResult.Empty();
    RefreshSelector();
}
void UIBS6HBridge::SelectorInput(int32 Control,bool Down) {
    if (!Runtime || Control < 0 || Control > 6) return;
    const int32 PreviousHero = ActiveHero();
    const auto Result = Runtime->Selector.input(static_cast<IBS6H::Control>(Control), Down);
    LastResult = Result == IBS6H::Result::Ok ? TEXT("") : TEXT("No se puede cambiar de heroe durante esta accion.");
    PublishHeroChange(PreviousHero);
    RefreshSelector();
}
void UIBS6HBridge::UnbindLabInput() {
    // Key bindings have no stable public action handle: remove only our bound delegates.
    if(UInputComponent* Input=BoundInput.Get()) for(int32 I=Input->KeyBindings.Num()-1;I>=0;--I)
        if(Input->KeyBindings[I].KeyDelegate.IsBoundToObject(this)) Input->KeyBindings.RemoveAt(I);
    BoundInput.Reset();
}
void UIBS6HBridge::Previous() { SelectorInput(0,true); SelectorInput(0,false); }
void UIBS6HBridge::Next() { SelectorInput(1,true); SelectorInput(1,false); }
void UIBS6HBridge::Confirm() { SelectorInput(2,true); SelectorInput(2,false); }
void UIBS6HBridge::Back() { SelectorInput(3,true); SelectorInput(3,false); }
void UIBS6HBridge::BindLabInput(UInputComponent* Input) {
    if(!Input||BoundInput.Get()==Input) return; UnbindLabInput(); BoundInput=Input;
    Input->BindKey(EKeys::F6,IE_Pressed,this,&UIBS6HBridge::OpenSelector);
    Input->BindKey(EKeys::F7,IE_Pressed,this,&UIBS6HBridge::Shield);
    Input->BindKey(EKeys::F8,IE_Pressed,this,&UIBS6HBridge::Construct);
    Input->BindKey(EKeys::F9,IE_Pressed,this,&UIBS6HBridge::Blink);
    Input->BindKey(EKeys::Left,IE_Pressed,this,&UIBS6HBridge::Previous).bConsumeInput=false;
    Input->BindKey(EKeys::Right,IE_Pressed,this,&UIBS6HBridge::Next).bConsumeInput=false;
    Input->BindKey(EKeys::Enter,IE_Pressed,this,&UIBS6HBridge::Confirm).bConsumeInput=false;
    Input->BindKey(EKeys::Escape,IE_Pressed,this,&UIBS6HBridge::Back).bConsumeInput=false;
}

TArray<FIBS6HHeroDescriptor> UIBS6HBridge::GetHeroDescriptors() const
{
    TArray<FIBS6HHeroDescriptor> Descriptors;
    auto Add = [&](int32 Id, const TCHAR* Name, const TCHAR* Description, FLinearColor Color) {
        FIBS6HHeroDescriptor Item;
        Item.Id = Id;
        Item.Label = FText::FromString(Name);
        Item.Description = FText::FromString(Description);
        Item.Color = Color;
        Descriptors.Add(Item);
    };
    Add(0, TEXT("H01 · Fuerza"), TEXT("Vuelo, fuerza y golpes de gran impacto."), FLinearColor(.8f,.22f,.1f));
    Add(1, TEXT("H02 · Energia"), TEXT("Escudo, mano y martillo de energia."), FLinearColor(.05f,.7f,.3f));
    Add(2, TEXT("H03 · Teletransporte"), TEXT("Blink, contraataque y reposicionamiento."), FLinearColor(.35f,.2f,.85f));
    return Descriptors;
}

float UIBS6HBridge::Energy() const
{
    return Runtime ? float(Runtime->Heroes.power(Runtime->Heroes.active())) : 0;
}

TArray<FIBS6HAbility> UIBS6HBridge::GetAbilities() const
{
    TArray<FIBS6HAbility> Abilities;
    if (!Runtime) return Abilities;
    const bool InputReady = !Runtime->Heroes.paused();
    auto Add = [&](int32 Id, const TCHAR* Label, float Cooldown, float Cost, bool ContextReady) {
        FIBS6HAbility Ability;
        Ability.Id = Id;
        Ability.Label = FText::FromString(Label);
        Ability.Cooldown = Cooldown;
        Ability.EnergyCost = Cost;
        Ability.bAvailable = InputReady && ContextReady && Cooldown <= 0 && Energy() >= Cost;
        Abilities.Add(Ability);
    };
    switch (ActiveHero())
    {
    case 0:
        Add(101, TEXT("Golpe"), StrikeCooldown, 0, !Runtime->body().carrying);
        Add(102, TEXT("Golpe fuerte"), StrikeCooldown, 0, !Runtime->body().carrying);
        Add(103, TEXT("Gancho"), StrikeCooldown, 0, !Runtime->body().carrying);
        Add(104, TEXT("Laser"), StrikeCooldown, 0, HostAbility.IsBound());
        Add(105, TEXT("Telequinesis"), 0, 0, HostAbility.IsBound());
        Add(106, TEXT("Impulso aereo"), StrikeCooldown, 0, HostAbility.IsBound() && Runtime->body().airborne);
        break;
    case 1:
        Add(201, TEXT("Escudo"), ShieldCooldown, 25, !Runtime->Heroes.shieldId());
        Add(202, TEXT("Mano"), Runtime->Heroes.constructId() ? 0 : ConstructCooldown,
            Runtime->Heroes.constructId() ? 0 : 35, !Runtime->Heroes.constructId() || Runtime->ConstructStyle == 202);
        Add(203, TEXT("Martillo"), Runtime->Heroes.constructId() ? StrikeCooldown : ConstructCooldown,
            Runtime->Heroes.constructId() ? 0 : 35, !Runtime->Heroes.constructId() || Runtime->ConstructStyle == 203);
        Add(101, TEXT("Golpe"), StrikeCooldown, 0, !Runtime->body().carrying);
        Add(104, TEXT("Laser"), StrikeCooldown, 0, HostAbility.IsBound());
        Add(105, TEXT("Telequinesis"), 0, 0, HostAbility.IsBound());
        break;
    case 2:
        Add(301, TEXT("Blink"), float(Runtime->Heroes.blinkRemaining()), 20, !Runtime->body().carrying);
        Add(302, TEXT("Contra-blink"), float(Runtime->Heroes.blinkRemaining()), 20, !Runtime->body().carrying);
        Add(303, TEXT("Reposicion"), float(Runtime->Heroes.blinkRemaining()), 20, !Runtime->body().carrying);
        Add(101, TEXT("Golpe"), StrikeCooldown, 0, !Runtime->body().carrying);
        Add(102, TEXT("Golpe fuerte"), StrikeCooldown, 0, !Runtime->body().carrying);
        Add(106, TEXT("Impulso aereo"), StrikeCooldown, 0, HostAbility.IsBound() && Runtime->body().airborne);
        break;
    }
    return Abilities;
}

bool UIBS6HBridge::EquipAbility(int32 AbilityId)
{
    if (!Runtime || IsSelectorOpen()) return false;
    for (const auto& Ability : GetAbilities()) if (Ability.Id == AbilityId)
    {
        // Equipping a cooling power is valid; activation rechecks its current state.
        Equipped = AbilityId;
        return true;
    }
    return false;
}

bool UIBS6HBridge::ActivateEquipped()
{
    if (!Runtime || Runtime->Heroes.paused() || !GetWorld()) return false;
    auto* Character = Runtime->Character.Get();
    if (!Character) return false;
    const auto Abilities = GetAbilities();
    const auto* Descriptor = Abilities.FindByPredicate([this](const auto& Item) { return Item.Id == Equipped; });
    if (!Descriptor || !Descriptor->bAvailable) {
        LastResult = TEXT("Poder en recuperacion, sin energia o incompatible con la accion actual.");
        return false;
    }
    FVector Forward = Character->GetBaseAimRotation().Vector();
    if (!Runtime->body().airborne)
        Forward = FVector::VectorPlaneProject(Forward, Character->GetActorUpVector()).GetSafeNormal();
    if (Forward.IsNearlyZero()) Forward = Character->GetActorForwardVector();
    auto FindTarget = [&]() -> AActor* {
        FHitResult Hit;
        FCollisionQueryParams Query(SCENE_QUERY_STAT(IBS6HAim), false, Character);
        return GetWorld()->SweepSingleByChannel(Hit, Character->GetActorLocation(),
            Character->GetActorLocation() + Forward * 250,
            FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(25), Query) ? Hit.GetActor() : nullptr;
    };
    bool Accepted = false;
    bool IsBlink = false;
    switch (Equipped)
    {
    case 101:
    case 102:
    case 103:
        if (AActor* Target = FindTarget()) {
            const int32 SavedStyle = Runtime->ConstructStyle;
            Runtime->ConstructStyle = Equipped;
            Accepted = Runtime->shortAttack(Runtime->newId(), IBS6H::Hero::H01, Runtime->Target(Target), 250);
            Runtime->ConstructStyle = SavedStyle;
            if (Accepted) StrikeCooldown = Equipped == 101 ? .35f : .75f;
        }
        break;
    case 104:
    case 105:
    case 106:
        Accepted = HostAbility.IsBound() && HostAbility.Execute(Equipped);
        if (Accepted && Equipped != 105) StrikeCooldown = .5f;
        break;
    case 201:
        Accepted = Runtime->Heroes.summon(IBS6H::Effect::Shield) == IBS6H::Result::Ok;
        if (Accepted) ShieldCooldown = 5;
        break;
    case 202:
    case 203:
        if (!Runtime->Heroes.constructId()) {
            Runtime->ConstructStyle = Equipped;
            Accepted = Runtime->Heroes.summon(IBS6H::Effect::Construct) == IBS6H::Result::Ok;
            if (Accepted) ConstructCooldown = 6;
        }
        if (Runtime->Heroes.constructId()) if (AActor* Target = FindTarget()) {
            const bool Action = Equipped == 202 ? Grab(Target) : Strike(Target);
            Accepted |= Action;
            if (Action && Equipped == 203) StrikeCooldown = .5f;
        }
        break;
    case 301:
    case 302:
    case 303:
    {
        FVector Direction = Forward;
        if (Equipped == 302) {
            Direction = GetWorld()->GetTimeSeconds() - LastIncomingAt <= 3
                ? Character->GetActorLocation() - LastAttackSource : -Forward;
        } else if (Equipped == 303) {
            Direction = Character->GetLastMovementInputVector();
            if (Direction.IsNearlyZero()) Direction = Character->GetActorRightVector();
        }
        if (!Runtime->body().airborne)
            Direction = FVector::VectorPlaneProject(Direction, Character->GetActorUpVector());
        Accepted = Runtime->Heroes.blink(Native(Direction), Equipped == 301 ? 30 : 25) == IBS6H::Result::Ok;
        IsBlink = Accepted;
        break;
    }
    default:
        break;
    }
    LastResult = Accepted ? TEXT("") : TEXT("No hay un blanco valido o el destino esta bloqueado.");
    if (Accepted) PresentAbility(IsBlink);
    return Accepted;
}

void UIBS6HBridge::ReleaseEquipped()
{
    if (Runtime && Equipped == 202 && Runtime->OwnedGrip)
        Runtime->dropConstruct(Runtime->OwnedGrip);
    if (HostReleaseAbility.IsBound()) HostReleaseAbility.Execute(Equipped);
}

void UIBS6HBridge::NotifyIncomingHit(FVector Source)
{
    if (!Source.ContainsNaN() && GetWorld()) {
        LastAttackSource = Source;
        LastIncomingAt = GetWorld()->GetTimeSeconds();
    }
}

void UIBS6HBridge::ResetAbilities()
{
    CloseSelector();
    if (Runtime) Runtime->Heroes.reset();
    StrikeCooldown = ShieldCooldown = ConstructCooldown = 0;
    LastIncomingAt = -100;
    LastResult.Empty();
}

bool UIBS6HBridge::IsSelectorOpen() const { return Runtime && Runtime->Selector.isOpen(); }
int32 UIBS6HBridge::SelectorFocus() const { return Runtime ? int32(Runtime->Selector.focus()) : 0; }
void UIBS6HBridge::CloseSelector()
{
    if (Runtime && Runtime->Selector.isOpen()) {
        Runtime->Selector.input(IBS6H::Control::Back, true);
        Runtime->Selector.input(IBS6H::Control::Back, false);
    }
    RefreshSelector();
}

void UIBS6HBridge::RefreshSelector()
{
    if (IsSelectorOpen() && !SelectorWidget) {
        auto* Character = Cast<ACharacter>(GetOwner());
        auto* Controller = Character ? Cast<APlayerController>(Character->GetController()) : nullptr;
        if (Controller && Controller->IsLocalController()) {
            SelectorWidget = CreateWidget<UIBS6HSelectorWidget>(Controller);
            if (SelectorWidget) {
                SelectorWidget->SetBridge(this);
                SelectorWidget->AddToViewport(50);
            }
        }
    }
    if (SelectorWidget) {
        SelectorWidget->SetVisibility(IsSelectorOpen() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
        SelectorWidget->Refresh();
    }
}

void UIBS6HBridge::PublishHeroChange(int32 PreviousHero)
{
    if (ActiveHero() == PreviousHero) return;
    Equipped = (ActiveHero() + 1) * 100 + 1;
    OnHeroChanged.Broadcast(ActiveHero());
    PresentAbility(false);
}

void UIBS6HBridge::PresentAbility(bool bBlink)
{
    USoundBase* Sound = ActivationSound;
    if (!Sound && !GeneratedSound) {
        auto* Wave = NewObject<USoundWaveProcedural>(this);
        constexpr int32 SampleRate = 24000;
        constexpr int32 Samples = 3600;
        Wave->SetSampleRate(SampleRate);
        Wave->NumChannels = 1;
        Wave->Duration = float(Samples) / SampleRate;
        TArray<int16> PCM;
        PCM.SetNumUninitialized(Samples);
        for (int32 Index = 0; Index < Samples; ++Index) {
            const float T = float(Index) / SampleRate;
            const float Envelope = FMath::Sin(PI * float(Index) / Samples);
            PCM[Index] = int16(FMath::Sin(2 * PI * (440 * T + 900 * T * T)) * Envelope * 4000);
        }
        Wave->QueueAudio(reinterpret_cast<const uint8*>(PCM.GetData()), PCM.Num() * sizeof(int16));
        GeneratedSound = Wave;
    }
    Sound = Sound ? Sound : GeneratedSound.Get();
    if (Sound) {
        if (auto* Cue = UGameplayStatics::SpawnSoundAtLocation(this, Sound, GetOwner()->GetActorLocation(),
            FRotator::ZeroRotator, .45f, bBlink ? 1.4f : 1.f)) Cue->FadeOut(.16f, 0);
        // A procedural wave's queued samples are consumed once. Next cue receives fresh PCM.
        if (Sound == GeneratedSound) GeneratedSound = nullptr;
    }
}
