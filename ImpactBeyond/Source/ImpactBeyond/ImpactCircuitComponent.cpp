#include "ImpactCircuitComponent.h"

#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "IB_Integration/ImpactContract/ImpactContractUE.h"

// One route for the circuit panel's lifetime. This is a producer hookup to the
// canonical adapter, not another damage policy or a second S35 health receiver.
struct FImpactCircuitCanonical
{
	TWeakObjectPtr<UIBS4ResponseComponent> Response;
	FGuid Session = FGuid::NewGuid();
	FGuid TargetLife = FGuid::NewGuid();
	FGuid Part = FGuid::NewGuid();
	FGuid Instigator = FGuid::NewGuid();
	TUniquePtr<IBImpactUE::S4Bridge> Bridge;
	TWeakObjectPtr<UObject> Owner;
	explicit FImpactCircuitCanonical(UIBS4ResponseComponent& InResponse, UObject& InOwner) : Response(&InResponse), Owner(&InOwner)
	{
		Bridge = MakeUnique<IBImpactUE::S4Bridge>(InResponse, InOwner, Session, TargetLife, FName(TEXT("Circuit.Panel.Concrete")));
		// This panel currently accepts only captured S35 physical contacts below.
		// Native S4 ingress has no captured identity here and must fail closed.
		FIBS4IngressRouter Ingress;
		Ingress.BindLambda([](UIBS4ResponseComponent&, const FIBS4Hit&) {});
		if (!InResponse.BindConsumer(&InOwner, Ingress)) { Bridge.Reset(); }
	}
	~FImpactCircuitCanonical()
	{
		if (Response.IsValid() && Owner.IsValid()) { Response->UnbindConsumer(Owner.Get()); }
	}
};

void FImpactCircuitCanonicalDeleter::operator()(FImpactCircuitCanonical* Ptr) const { delete Ptr; }
UImpactCircuitComponent::~UImpactCircuitComponent() = default;

void UImpactCircuitComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Canonical.Reset();
	Super::EndPlay(Reason);
}

int32 UImpactCircuitComponent::GetCanonicalAcceptedHits() const
{
	return Canonical && Canonical->Response.IsValid() ? Canonical->Response->AcceptedHits : 0;
}

float UImpactCircuitComponent::GetCanonicalTargetHealth() const
{
	return Canonical && Canonical->Response.IsValid() ? Canonical->Response->Health : -1.0f;
}

namespace
{
	const FName CarTag(TEXT("ImpactCircuitCar"));
	const FName VehicleTag(TEXT("ImpactSprintVehicle"));
	const FName PanelTag(TEXT("ImpactCircuitPanel"));
	const FName GrabbablePropTag(TEXT("ImpactLandingTarget"));
	constexpr float GrabDistance = 380.0f;
	constexpr float LiftDuration = 0.55f;
	constexpr float ThrowFollowThrough = 0.18f;
}

UImpactCircuitComponent::UImpactCircuitComponent()
{
	static ConstructorHelpers::FObjectFinderOptional<UStaticMesh> ConceptCar(
		TEXT("/Game/Car/SM_AutomotiveTP_Car.SM_AutomotiveTP_Car"));
	static ConstructorHelpers::FObjectFinderOptional<UStaticMesh> BackupCar(
		TEXT("/Game/IB_Donors/Carro/carro/StaticMeshes/carro.carro"));
	CarAppearance = ConceptCar.Get() ? ConceptCar.Get() : BackupCar.Get();
	static ConstructorHelpers::FObjectFinderOptional<UStaticMesh> Taxi(TEXT("/Game/IB_Donors/SprintVehicles/taxi/StaticMeshes/taxi.taxi"));
	static ConstructorHelpers::FObjectFinderOptional<UStaticMesh> Electric(TEXT("/Game/IB_Donors/SprintVehicles/carro_eletrico/StaticMeshes/carro_eletrico.carro_eletrico"));
	TaxiAppearance = Taxi.Get();
	ElectricAppearance = Electric.Get();
	// The template car's tyres extend below the handhold under its chassis.
	VehicleGripInset = ConceptCar.Get() ? 24.0f : 0.0f;
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UImpactCircuitComponent::BeginPlay()
{
	Super::BeginPlay();
	Hero = Cast<ACharacter>(GetOwner());
	if (!Hero)
	{
		return;
	}
	for (TActorIterator<AStaticMeshActor> Actor(GetWorld()); Actor; ++Actor)
	{
		if (Actor->ActorHasTag(CarTag) || Actor->ActorHasTag(VehicleTag))
		{
			UStaticMeshComponent* Chassis = Actor->GetStaticMeshComponent();
			const bool bPrimary = Actor->ActorHasTag(CarTag);
			if (bPrimary) { CircuitCar = Chassis; }
			Grabbables.Add(Chassis);
			Vehicles.Add(Chassis);
			Chassis->SetMobility(EComponentMobility::Movable);
			Chassis->SetCollisionProfileName(TEXT("PhysicsActor"));
			Chassis->SetUseCCD(true);
			Chassis->SetNotifyRigidBodyCollision(true);
			const bool bTaxi = Actor->ActorHasTag(TEXT("ImpactVehicleTaxi"));
			const bool bElectric = Actor->ActorHasTag(TEXT("ImpactVehicleElectric"));
			Chassis->SetMassOverrideInKg(NAME_None, bTaxi ? 1450.0f : (bElectric ? 1650.0f : 1200.0f));
			Chassis->OnComponentHit.AddDynamic(this, &UImpactCircuitComponent::OnCarHit);
			Chassis->SetSimulatePhysics(true);
			Chassis->PutAllRigidBodiesToSleep();
			FitCarAppearance(Chassis, bTaxi && TaxiAppearance ? TaxiAppearance.Get()
				: (bElectric && ElectricAppearance ? ElectricAppearance.Get() : CarAppearance.Get()));
		}
		else if (Actor->ActorHasTag(PanelTag))
		{
			Panel = Actor->GetStaticMeshComponent();
		}
		else if (Actor->ActorHasTag(GrabbablePropTag))
		{
			Grabbables.Add(Actor->GetStaticMeshComponent());
		}
	}
	Car = CircuitCar;
	if (!Car || !Panel)
	{
		Car = nullptr;
		return;
	}

	ChassisExtent = Car->GetStaticMesh()->GetBounds().BoxExtent * Car->GetComponentScale().GetAbs();
	// The circuit owns this tagged panel. Never stack another health receiver on it.
	if (!Panel->GetOwner()->FindComponentByClass<UIBS4ResponseComponent>())
	{
		UIBS4ResponseComponent* Response = NewObject<UIBS4ResponseComponent>(Panel->GetOwner());
		Response->Surface = EIBS4Surface::Concrete;
		Response->bH01 = false;
		Panel->GetOwner()->AddInstanceComponent(Response);
		Response->RegisterComponent();
		Canonical.Reset(new FImpactCircuitCanonical(*Response, *this));
	}
	AddTickPrerequisiteComponent(Hero->GetCharacterMovement());
	Hero->GetMesh()->AddTickPrerequisiteComponent(this);
	SetComponentTickEnabled(true);
}

bool UImpactCircuitComponent::IsCarHeld() const
{
	return CarState == ECarState::Held || CarState == ECarState::Lifting || CarState == ECarState::WindingUp;
}

void UImpactCircuitComponent::FitCarAppearance(UStaticMeshComponent* Chassis, UStaticMesh* Appearance)
{
	if (!Appearance)
	{
		return;
	}
	const FBoxSphereBounds Source = Appearance->GetBounds();
	const FVector Extent = Chassis->GetStaticMesh()->GetBounds().BoxExtent * Chassis->GetComponentScale().GetAbs();
	const bool bTurnVehicle = (Source.BoxExtent.X > Source.BoxExtent.Y) != (Extent.X > Extent.Y);
	const FQuat Rotation = bTurnVehicle ? FQuat(FVector::UpVector, UE_HALF_PI) : FQuat::Identity;
	const FVector TargetExtent = bTurnVehicle
		? FVector(Extent.Y, Extent.X, Extent.Z) : Extent;
	const FVector Scale = TargetExtent / Source.BoxExtent;

	UStaticMeshComponent* Visual = NewObject<UStaticMeshComponent>(Chassis->GetOwner(), TEXT("CarBodyAppearance"));
	Visual->SetMobility(EComponentMobility::Movable);
	Visual->SetStaticMesh(Appearance);
	Visual->SetCollisionProfileName(TEXT("NoCollision"));
	Visual->SetCanEverAffectNavigation(false);
	Visual->SetupAttachment(Chassis);
	Visual->SetAbsolute(false, false, true);
	Visual->RegisterComponent();
	const FQuat WorldRotation = Chassis->GetComponentQuat() * Rotation;
	Visual->SetWorldRotation(WorldRotation);
	Visual->SetWorldScale3D(Scale);
	Visual->SetWorldLocation(Chassis->GetComponentLocation() - WorldRotation.RotateVector(Source.Origin * Scale));
	Chassis->SetVisibility(false);
}

void UImpactCircuitComponent::ToggleGrab()
{
	if (!IsValid(Car) || !Hero->GetController())
	{
		return;
	}
	if (IsCarHeld())
	{
		ReleaseCar(ECarState::Free);
		return;
	}
	UStaticMeshComponent* Grabbable = FindGrabbable();
	if (!Grabbable)
	{
		return;
	}
	Car = Grabbable;
	if (Car == LaunchedVehicle)
	{
		LaunchedVehicle = nullptr;
	}
	ChassisExtent = Car->GetStaticMesh()->GetBounds().BoxExtent * Car->GetComponentScale().GetAbs();

	Car->SetPhysicsLinearVelocity(FVector::ZeroVector);
	Car->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	Car->SetSimulatePhysics(false);
	Car->IgnoreActorWhenMoving(Hero, true);
	Hero->GetCapsuleComponent()->IgnoreActorWhenMoving(Car->GetOwner(), true);
	const FVector TowardCar = Car->GetComponentLocation() - Hero->GetActorLocation();
	if (!TowardCar.IsNearlyZero())
	{
		Hero->SetActorRotation(FRotator(0, TowardCar.Rotation().Yaw, 0));
	}
	LiftOffset = Car->GetComponentLocation() - Hero->GetActorLocation();
	CarCarryRotationOffset = Hero->GetActorQuat().Inverse() * Car->GetComponentQuat();
	const auto MakeGrip = [this](float Side)
	{
		FVector Point = CarCarryRotationOffset.UnrotateVector(FVector(-10.0f, Side * 30.0f, 0.0f));
		Point.X = FMath::Clamp(Point.X, -ChassisExtent.X + 5.0, ChassisExtent.X - 5.0);
		Point.Y = FMath::Clamp(Point.Y, -ChassisExtent.Y + 5.0, ChassisExtent.Y - 5.0);
		Point.Z = -ChassisExtent.Z - 3.0 + (Car == CircuitCar ? VehicleGripInset : 0.0f);
		return Point;
	};
	LeftChassisGrip = MakeGrip(-1.0f);
	RightChassisGrip = MakeGrip(1.0f);
	PhaseTime = 0.0f;
	BlockedTime = 0.0f;
	CarState = ECarState::Lifting;
}

UStaticMeshComponent* UImpactCircuitComponent::FindGrabbable() const
{
	UStaticMeshComponent* Closest = nullptr;
	double ClosestDistanceSquared = FMath::Square(GrabDistance);
	const FVector Origin = Hero->GetActorLocation();
	const FCollisionQueryParams Query(SCENE_QUERY_STAT(ImpactCircuitGrab), false, Hero);
	for (const TWeakObjectPtr<UStaticMeshComponent>& WeakCandidate : Grabbables)
	{
		UStaticMeshComponent* Candidate = WeakCandidate.Get();
		if (!Candidate || !Candidate->GetStaticMesh())
		{
			continue;
		}
		const double DistanceSquared = FVector::DistSquared(Origin, Candidate->GetComponentLocation());
		FHitResult Obstruction;
		if (DistanceSquared <= ClosestDistanceSquared
			&& (!GetWorld()->LineTraceSingleByChannel(Obstruction, Origin, Candidate->GetComponentLocation(), ECC_Visibility, Query)
				|| Obstruction.GetActor() == Candidate->GetOwner()))
		{
			Closest = Candidate;
			ClosestDistanceSquared = DistanceSquared;
		}
	}
	return Closest;
}

void UImpactCircuitComponent::ReleaseCar(ECarState NextState)
{
	Car->IgnoreActorWhenMoving(Hero, false);
	Hero->GetCapsuleComponent()->IgnoreActorWhenMoving(Car->GetOwner(), false);
	CarState = NextState;
	PhaseTime = 0.0f;
	bChargingThrow = false;
	Car->SetSimulatePhysics(true);
	Car->WakeAllRigidBodies();
}

void UImpactCircuitComponent::ThrowCar()
{
	ChargeTime = FullChargeTime;
	StartThrow(1.0f);
}

void UImpactCircuitComponent::BeginThrowCharge()
{
	if (CarState == ECarState::Held && !bChargingThrow)
	{
		ChargeTime = 0.0f;
		bChargingThrow = true;
	}
}

void UImpactCircuitComponent::ReleaseThrowCharge()
{
	if (bChargingThrow)
	{
		bChargingThrow = false;
		StartThrow(GetThrowCharge());
	}
}

float UImpactCircuitComponent::GetThrowCharge() const
{
	return FMath::Clamp(ChargeTime / FullChargeTime, 0.0f, 1.0f);
}

bool UImpactCircuitComponent::IsPlayingThrow() const
{
	return CarState == ECarState::WindingUp
		|| (CarState == ECarState::Thrown && PhaseTime < ThrowFollowThrough);
}

void UImpactCircuitComponent::StartThrow(float Charge)
{
	if (!IsValid(Car) || CarState != ECarState::Held || !Hero->GetController())
	{
		return;
	}
	bChargingThrow = false;
	LaunchSpeed = FMath::Lerp(MinimumThrowSpeed, FMath::Max(MinimumThrowSpeed, MaximumThrowSpeed), Charge);
	CarState = ECarState::WindingUp;
	PhaseTime = 0.0f;
}

float UImpactCircuitComponent::GetGripWeight() const
{
	switch (CarState)
	{
	case ECarState::Lifting:
		return FMath::SmoothStep(0.0f, 0.3f, PhaseTime);
	case ECarState::Held:
	case ECarState::WindingUp:
		return 1.0f;
	case ECarState::Thrown:
		return 1.0f - FMath::SmoothStep(0.0f, ThrowFollowThrough, PhaseTime);
	default:
		return 0.0f;
	}
}

bool UImpactCircuitComponent::GetGripTargets(FVector& LeftWorld, FVector& RightWorld) const
{
	if (!IsValid(Car) || !Hero || GetGripWeight() <= 0.0f)
	{
		return false;
	}
	if (CarState == ECarState::Thrown)
	{
		const FVector FollowThrough = Hero->GetActorLocation() + Hero->GetActorForwardVector() * 64.0f + FVector(0, 0, 71);
		LeftWorld = FollowThrough - Hero->GetActorRightVector() * 28.0f;
		RightWorld = FollowThrough + Hero->GetActorRightVector() * 28.0f;
		return true;
	}
	const FTransform CarTransform(Car->GetComponentQuat(), Car->GetComponentLocation());
	LeftWorld = CarTransform.TransformPosition(LeftChassisGrip);
	RightWorld = CarTransform.TransformPosition(RightChassisGrip);
	return true;
}

void UImpactCircuitComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!IsValid(Car))
	{
		return;
	}
	PhaseTime += DeltaTime;
	if (IsValid(LaunchedVehicle))
	{
		PrePhysicsCarVelocity = LaunchedVehicle->GetPhysicsLinearVelocity();
	}
	if (bChargingThrow)
	{
		ChargeTime = FMath::Min(ChargeTime + DeltaTime, FullChargeTime);
	}
	if (CarState == ECarState::Held || CarState == ECarState::Lifting || CarState == ECarState::WindingUp)
	{
		const FVector Current = Car->GetComponentLocation();
		const FVector Body = Hero->GetActorLocation();
		const FVector Forward = Hero->GetActorForwardVector();
		const float GripInset = Car == CircuitCar ? VehicleGripInset : 0.0f;
		const FVector CarryTarget = Body + Forward * 35.0f + FVector(0, 0, ChassisExtent.Z + 94.0f - GripInset);
		FVector Target = CarryTarget;
		if (CarState == ECarState::Lifting)
		{
			const float Alpha = FMath::SmoothStep(0.0f, LiftDuration, PhaseTime);
			const FVector Start = Body + LiftOffset;
			Target = FMath::Lerp(Start, CarryTarget, Alpha);
			Target.Z += FMath::Sin(Alpha * PI) * 20.0f;
		}
		else if (CarState == ECarState::WindingUp)
		{
			const float Load = FMath::SmoothStep(0.0f, 0.085f, PhaseTime);
			const float Push = FMath::SmoothStep(0.085f, ThrowReleaseTime, PhaseTime);
			Target += Forward * (-25.0f * Load + 58.0f * Push) + FVector(0, 0, -18.0f * Load + 28.0f * Push);
		}
		else if (bChargingThrow)
		{
			Target -= Forward * (14.0f * GetThrowCharge());
		}
		// The car's actual box sweeps against the world independently of the hero's capsule.
		FHitResult Obstruction;
		Car->MoveComponent(Target - Current, Car->GetComponentQuat(), true, &Obstruction);
		const FQuat CarryRotation = Hero->GetActorQuat() * CarCarryRotationOffset;
		if (CarState != ECarState::Lifting && !Car->GetComponentQuat().Equals(CarryRotation, 0.001f))
		{
			// MoveComponent sweeps translation only. Check the rotated chassis before accepting a turn.
			FCollisionQueryParams Query(SCENE_QUERY_STAT(ImpactCircuitCarryTurn), false, Hero);
			Query.AddIgnoredActor(Car->GetOwner());
			const FQuat NextRotation = FMath::QInterpTo(Car->GetComponentQuat(), CarryRotation, DeltaTime, 22.0f);
			if (!GetWorld()->OverlapBlockingTestByChannel(Car->GetComponentLocation(), NextRotation,
				Car->GetCollisionObjectType(), FCollisionShape::MakeBox(ChassisExtent), Query))
			{
				Car->MoveComponent(FVector::ZeroVector, NextRotation, false);
			}
		}
		const bool bOutOfReach = FVector::DistSquared(Car->GetComponentLocation(), CarryTarget) > FMath::Square(115.0f);
		BlockedTime = Obstruction.bBlockingHit && bOutOfReach ? BlockedTime + DeltaTime : 0.0f;
		if (BlockedTime > 0.22f || (CarState != ECarState::Lifting && bOutOfReach))
		{
			ReleaseCar(ECarState::Free);
		}
		else if (CarState == ECarState::Lifting && PhaseTime >= LiftDuration)
		{
			CarState = ECarState::Held;
		}
		else if (CarState == ECarState::WindingUp && PhaseTime >= ThrowReleaseTime)
		{
			FRotator Aim = Hero->GetControlRotation();
			Aim.Pitch = FMath::Clamp(FRotator::NormalizeAxis(Aim.Pitch), -35.0f, 65.0f);
			ReleaseCar(ECarState::Thrown);
			const FVector LaunchVelocity = Aim.Vector() * LaunchSpeed + FVector(0, 0, 400);
			if (Vehicles.Contains(Car.Get()))
			{
				LaunchedVehicle = Car;
				ThrowAttackId = FGuid::NewGuid(); // one ID per physical release, not per contact
				PrePhysicsCarVelocity = LaunchVelocity;
			}
			// REPIRK PhysicsGrabber: release first, then apply a mass-independent impulse and haptic pulse.
			Car->AddImpulse(LaunchVelocity - Car->GetPhysicsLinearVelocity(), NAME_None, true);
			if (APlayerController* PC = Cast<APlayerController>(Hero->GetController()))
			{
				const float Strength = FMath::Lerp(0.15f, 0.65f, GetThrowCharge());
				PC->PlayDynamicForceFeedback(Strength, 0.16f, true, true, true, true);
			}
		}
	}
}

void UImpactCircuitComponent::OnCarHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!IsValid(LaunchedVehicle) || HitComponent != LaunchedVehicle
		|| !IsValid(Panel) || OtherComponent != Panel || !Panel->IsVisible()
		// Multiple contact points can add impulses even when the car approaches slowly.
		|| FMath::Abs(FVector::DotProduct(PrePhysicsCarVelocity, Hit.ImpactNormal)) < PanelImpactThreshold
		|| FMath::Abs(FVector::DotProduct(NormalImpulse, Hit.ImpactNormal)) < LaunchedVehicle->GetMass() * PanelImpactThreshold)
	{
		return;
	}

	if (!Canonical || !Canonical->Bridge || !ThrowAttackId.IsValid()) { return; }
	FIBS35DamageMessage Contact;
	Contact.AttackId = ThrowAttackId;
	Contact.TargetId = Canonical->TargetLife;
	Contact.InstigatorId = Canonical->Instigator;
	Contact.InstigatorActor = Hero;
	Contact.TargetPart = OtherComponent;
	Contact.DamageKind = EIBS35DamageKind::ThrownObject;
	// Preserve the old circuit's thresholded, single-impact break; S4 alone mutates HP.
	Contact.HealthDelta = -100.0f;
	Contact.Contact = Hit.ImpactPoint;
	Contact.RelativeVelocity = PrePhysicsCarVelocity - OtherComponent->GetPhysicsLinearVelocity();
	Contact.Impulse = NormalImpulse;
	Contact.bPhysicsResolved = true; // Chaos has resolved this contact; no second impulse
	Contact.bAirborne = Hero->GetCharacterMovement()->IsFlying();
	Contact.AttackSeconds = PhaseTime;
	Contact.SurfaceMaterial = TEXT("Concrete");
	const auto Record = Canonical->Bridge->ReceiveS35(Contact, OtherComponent, Canonical->Part);
	if (Record.Result.State != IBImpact::Status::Accepted) { return; }

	Panel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Panel->SetVisibility(false);

	// Generate four small rigid bodies from the panel's existing mesh and material.
	for (int32 Index = 0; Index < 4; ++Index)
	{
		const FVector Offset((Index - 1.5f) * 105.0f, 70.0f, (Index % 2) * 110.0f - 85.0f);
		AStaticMeshActor* Fragment = GetWorld()->SpawnActor<AStaticMeshActor>(Panel->GetComponentLocation() + Offset,
			FRotator(0.0f, Index * 12.0f, Index * 7.0f));
		if (Fragment)
		{
			UStaticMeshComponent* Mesh = Fragment->GetStaticMeshComponent();
			Mesh->SetMobility(EComponentMobility::Movable);
			Mesh->SetStaticMesh(Panel->GetStaticMesh());
			Mesh->SetMaterial(0, Panel->GetMaterial(0));
			Mesh->SetWorldScale3D(Panel->GetComponentScale() * FVector(0.13f, 0.58f, 0.21f));
			Mesh->SetCollisionProfileName(TEXT("PhysicsActor"));
			Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
			Mesh->SetSimulatePhysics(true);
			Mesh->SetPhysicsLinearVelocity(FVector((Index - 1.5f) * 140.0f, 480.0f, 130.0f));
			Fragment->SetLifeSpan(8.0f);
		}
	}
}

void UImpactCircuitComponent::ResetCircuit()
{
	bChargingThrow = false;
	if (IsValid(CircuitCar))
	{
		UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this)));
	}
}
