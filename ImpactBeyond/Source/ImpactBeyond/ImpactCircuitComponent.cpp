#include "ImpactCircuitComponent.h"

#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	const FName CarTag(TEXT("ImpactCircuitCar"));
	const FName PanelTag(TEXT("ImpactCircuitPanel"));
	constexpr float GrabDistance = 750.0f;
	constexpr float CarryDistance = 385.0f;
	constexpr float ThrowSpeed = 6800.0f;
}

UImpactCircuitComponent::UImpactCircuitComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
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
	SetComponentTickEnabled(true);
}

void UImpactCircuitComponent::ToggleGrab()
{
	if (!IsValid(Car) || !Hero->GetController())
	{
		return;
	}
	if (CarState == ECarState::Held)
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
	CarState = ECarState::Held;
}

void UImpactCircuitComponent::ReleaseCar(ECarState NextState)
{
	Car->IgnoreActorWhenMoving(Hero, false);
	Hero->GetCapsuleComponent()->IgnoreActorWhenMoving(Car->GetOwner(), false);
	CarState = NextState;
	Car->SetSimulatePhysics(true);
	Car->WakeAllRigidBodies();
}

void UImpactCircuitComponent::ThrowCar()
{
	if (!IsValid(Car) || CarState != ECarState::Held || !Hero->GetController())
	{
		return;
	}
	FRotator Aim = Hero->GetControlRotation();
	Aim.Pitch = FMath::Clamp(FRotator::NormalizeAxis(Aim.Pitch), -35.0f, 65.0f);
	ReleaseCar(ECarState::Thrown);
	Car->SetPhysicsLinearVelocity(Aim.Vector() * ThrowSpeed + FVector(0.0f, 0.0f, 400.0f));
}

void UImpactCircuitComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!IsValid(Car))
	{
		return;
	}
	if (CarState == ECarState::Held)
	{
		const FRotator ViewYaw(0.0f, Hero->GetControlRotation().Yaw, 0.0f);
		const FVector Current = Car->GetComponentLocation();
		const FVector Target = Hero->GetActorLocation() + ViewYaw.Vector() * CarryDistance + FVector(0.0f, 0.0f, 115.0f);
		const FVector Next = FMath::VInterpConstantTo(Current, Target, DeltaTime, 4200.0f);
		// The car's actual box sweeps against the world independently of the hero's capsule.
		Car->MoveComponent(Next - Current, Car->GetComponentQuat(), true);
		if (FVector::DistSquared(Car->GetComponentLocation(), Hero->GetActorLocation()) > FMath::Square(1050.0f))
		{
			ReleaseCar(ECarState::Free);
		}
	}

#if !UE_BUILD_SHIPPING
	if (GEngine && Hero->IsLocallyControlled())
	{
		const TCHAR* Hint = CarState == ECarState::Held ? TEXT("CARRO SUJETO | G/Circulo: soltar | T/Triangulo: lanzar")
			: TEXT("G/Circulo: agarrar carro | R/Cruceta abajo: reiniciar");
		GEngine->AddOnScreenDebugMessage(1042, 0.1f, FColor::White, Hint);
	}
#endif
}

void UImpactCircuitComponent::OnCarHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if (CarState != ECarState::Thrown || !IsValid(Panel) || OtherComponent != Panel
		|| NormalImpulse.Size() < Car->GetMass() * 420.0f)
	{
		return;
	}

	CarState = ECarState::Free;
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
	if (IsValid(Car))
	{
		UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this)));
	}
}
