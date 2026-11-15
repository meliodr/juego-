#include "ImpactCircuitComponent.h"

#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	const FName CarTag(TEXT("ImpactCircuitCar"));
	const FName PanelTag(TEXT("ImpactCircuitPanel"));
	constexpr float GrabDistance = 380.0f;
	constexpr float LiftDuration = 0.55f;
	constexpr float ThrowFollowThrough = 0.18f;
}

UImpactCircuitComponent::UImpactCircuitComponent()
{
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
		if (Actor->ActorHasTag(CarTag))
		{
			Car = Actor->GetStaticMeshComponent();
		}
		else if (Actor->ActorHasTag(PanelTag))
		{
			Panel = Actor->GetStaticMeshComponent();
		}
	}
	if (!Car || !Panel)
	{
		Car = nullptr;
		return;
	}

	Car->SetMobility(EComponentMobility::Movable);
	Car->SetCollisionProfileName(TEXT("PhysicsActor"));
	Car->SetUseCCD(true);
	Car->SetNotifyRigidBodyCollision(true);
	Car->SetMassOverrideInKg(NAME_None, 1200.0f);
	Car->OnComponentHit.AddDynamic(this, &UImpactCircuitComponent::OnCarHit);
	Car->SetSimulatePhysics(true);
	ChassisExtent = Car->GetStaticMesh()->GetBounds().BoxExtent * Car->GetComponentScale().GetAbs();
	AddTickPrerequisiteComponent(Hero->GetCharacterMovement());
	Hero->GetMesh()->AddTickPrerequisiteComponent(this);
	SetComponentTickEnabled(true);
}

void UImpactCircuitComponent::ToggleGrab()
{
	if (!IsValid(Car) || !Hero->GetController())
	{
		return;
	}
	if (CarState == ECarState::Held || CarState == ECarState::Lifting || CarState == ECarState::WindingUp)
	{
		ReleaseCar(ECarState::Free);
		return;
	}
	if (FVector::DistSquared(Hero->GetActorLocation(), Car->GetComponentLocation()) > FMath::Square(GrabDistance))
	{
		return;
	}

	FHitResult Obstruction;
	FCollisionQueryParams Query(SCENE_QUERY_STAT(ImpactCircuitGrab), false, Hero);
	if (GetWorld()->LineTraceSingleByChannel(Obstruction, Hero->GetActorLocation(), Car->GetComponentLocation(), ECC_Visibility, Query)
		&& Obstruction.GetActor() != Car->GetOwner())
	{
		return;
	}

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
		Point.Z = -ChassisExtent.Z - 3.0;
		return Point;
	};
	LeftChassisGrip = MakeGrip(-1.0f);
	RightChassisGrip = MakeGrip(1.0f);
	PhaseTime = 0.0f;
	BlockedTime = 0.0f;
	CarState = ECarState::Lifting;
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
	if (bChargingThrow)
	{
		ChargeTime = FMath::Min(ChargeTime + DeltaTime, FullChargeTime);
	}
	if (CarState == ECarState::Held || CarState == ECarState::Lifting || CarState == ECarState::WindingUp)
	{
		const FVector Current = Car->GetComponentLocation();
		const FVector Body = Hero->GetActorLocation();
		const FVector Forward = Hero->GetActorForwardVector();
		const FVector CarryTarget = Body + Forward * 35.0f + FVector(0, 0, ChassisExtent.Z + 94.0f);
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
			PrePhysicsCarVelocity = Aim.Vector() * LaunchSpeed + FVector(0, 0, 400);
			Car->SetPhysicsLinearVelocity(PrePhysicsCarVelocity);
		}
	}
	else if (CarState == ECarState::Thrown)
	{
		PrePhysicsCarVelocity = Car->GetPhysicsLinearVelocity();
	}

#if !UE_BUILD_SHIPPING
	if (GEngine && Hero->IsLocallyControlled())
	{
		const TCHAR* Hint = CarState == ECarState::Held ? TEXT("CARRO SUJETO | G/Circulo: soltar | Mantener T/Triangulo: cargar")
			: TEXT("G/Circulo: agarrar carro | R/Cruceta abajo: reiniciar");
		GEngine->AddOnScreenDebugMessage(1042, 0.1f, FColor::White, Hint);
	}
#endif
}

void UImpactCircuitComponent::OnCarHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if (CarState != ECarState::Thrown || !IsValid(Panel) || OtherComponent != Panel || !Panel->IsVisible()
		// Multiple contact points can add impulses even when the car approaches slowly.
		|| FMath::Abs(FVector::DotProduct(PrePhysicsCarVelocity, Hit.ImpactNormal)) < PanelImpactThreshold
		|| FMath::Abs(FVector::DotProduct(NormalImpulse, Hit.ImpactNormal)) < Car->GetMass() * PanelImpactThreshold)
	{
		return;
	}

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
	if (IsValid(Car))
	{
		UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this)));
	}
}
