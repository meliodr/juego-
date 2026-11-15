#include "ImpactLandingComponent.h"

#include "AudioDeviceHandle.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/LineBatchComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/App.h"
#include "SceneManagement.h"
#include "Sound/SoundWaveProcedural.h"

namespace
{
	const FName TargetTag(TEXT("ImpactLandingTarget"));
	const FName PanelTag(TEXT("ImpactLandingPanel"));
	constexpr float MinimumDescent = 120.0f;
	constexpr float MaximumDescent = 12000.0f;
	constexpr float MaximumDiveTime = 6.0f;
	constexpr float RecoverySeconds = 0.38f;
	constexpr float ShockwaveSeconds = 0.55f;
	constexpr double CooldownSeconds = 1.2;
	constexpr int32 MaxReactions = 8;
}

UImpactLandingComponent::UImpactLandingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UImpactLandingComponent::BeginPlay()
{
	Super::BeginPlay();
	Hero = Cast<ACharacter>(GetOwner());
	if (Hero)
	{
		Hero->LandedDelegate.AddDynamic(this, &UImpactLandingComponent::HandleLanded);
	}
}

bool UImpactLandingComponent::CanStartImpact() const
{
	if (!IsValid(Hero) || !Hero->GetController() || Phase != EPhase::Ready
		|| GetWorld()->GetTimeSeconds() < EarliestStartTime)
	{
		return false;
	}
	const UCharacterMovementComponent* Movement = Hero->GetCharacterMovement();
	return Movement->IsFlying() || Movement->IsFalling();
}

bool UImpactLandingComponent::IsImpacting() const
{
	return Phase == EPhase::Diving || (Phase == EPhase::Recovering && PhaseTime < RecoverySeconds);
}

bool UImpactLandingComponent::IsDiving() const
{
	return Phase == EPhase::Diving;
}

void UImpactLandingComponent::StartImpact()
{
	if (!CanStartImpact())
	{
		return;
	}
	UCharacterMovementComponent* Movement = Hero->GetCharacterMovement();
	const UCapsuleComponent* Capsule = Hero->GetCapsuleComponent();
	const FVector Start = Capsule->GetComponentLocation();
	FHitResult Floor;
	const FCollisionQueryParams Query(SCENE_QUERY_STAT(ImpactLandingFloor), false, Hero);
	if (!GetWorld()->SweepSingleByProfile(Floor, Start, Start - FVector(0, 0, MaximumDescent), FQuat::Identity,
		Capsule->GetCollisionProfileName(), Capsule->GetCollisionShape(), Query)
		|| Floor.Distance < MinimumDescent || !Movement->IsWalkable(Floor))
	{
		return;
	}

	Phase = EPhase::Diving;
	PhaseTime = 0.0f;
	Hero->StopJumping();
	Hero->ConsumeMovementInputVector();
	Movement->SetMovementMode(MOVE_Falling);
	Movement->Velocity = FVector(0.0, 0.0, FMath::Min(Movement->Velocity.Z, -static_cast<double>(DiveSpeed)));
	SetComponentTickEnabled(true);
}

void UImpactLandingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!IsValid(Hero))
	{
		SetComponentTickEnabled(false);
		return;
	}
	PhaseTime += DeltaTime;
	if (Phase == EPhase::Diving)
	{
		Hero->ConsumeMovementInputVector();
		if (PhaseTime >= MaximumDiveTime || !Hero->GetCharacterMovement()->IsFalling())
		{
			Phase = EPhase::Ready;
			SetComponentTickEnabled(false);
		}
	}
	else if (Phase == EPhase::Recovering)
	{
		if (PhaseTime >= ShockwaveSeconds)
		{
			if (ShockwaveLines)
			{
				ShockwaveLines->Flush();
			}
			Phase = EPhase::Ready;
			SetComponentTickEnabled(false);
			return;
		}
		DrawShockwave();
	}
}

void UImpactLandingComponent::HandleLanded(const FHitResult& Hit)
{
	if (!IsDiving() || !Hero->GetCharacterMovement()->IsWalkable(Hit))
	{
		return;
	}
	Phase = EPhase::Recovering;
	PhaseTime = 0.0f;
	EarliestStartTime = GetWorld()->GetTimeSeconds() + CooldownSeconds;
	ImpactNormal = Hit.ImpactNormal.GetSafeNormal();
	ImpactOrigin = Hit.ImpactPoint + ImpactNormal * 8.0f;
	const float ImpactSpeed = static_cast<float>(FMath::Max(0.0, -Hero->GetVelocity().Z));
	ApplyShockwave(ImpactOrigin, ImpactSpeed);
	if (FApp::CanEverRender() && !ShockwaveLines)
	{
		ShockwaveLines = NewObject<ULineBatchComponent>(Hero);
		Hero->AddInstanceComponent(ShockwaveLines);
		ShockwaveLines->bCalculateAccurateBounds = true;
		ShockwaveLines->RegisterComponent();
	}
	PlayImpactSound(ImpactOrigin);
}

void UImpactLandingComponent::ApplyShockwave(const FVector& Origin, float ImpactSpeed)
{
	struct FCandidate
	{
		UPrimitiveComponent* Component;
		FVector Contact;
		float Distance;
		bool bPanel;
	};
	TArray<FOverlapResult> Overlaps;
	const FCollisionQueryParams Query(SCENE_QUERY_STAT(ImpactLandingWave), false, Hero);
	GetWorld()->OverlapMultiByObjectType(Overlaps, Origin, FQuat::Identity,
		FCollisionObjectQueryParams(FCollisionObjectQueryParams::AllObjects), FCollisionShape::MakeSphere(Radius), Query);
	TSet<UPrimitiveComponent*> Seen;
	TArray<FCandidate> Candidates;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		UPrimitiveComponent* Primitive = Overlap.GetComponent();
		AActor* Actor = Overlap.GetActor();
		if (!IsValid(Actor) || !IsValid(Primitive) || Seen.Contains(Primitive))
		{
			continue;
		}
		Seen.Add(Primitive);
		const bool bPanel = Actor->ActorHasTag(PanelTag) || Primitive->ComponentHasTag(PanelTag);
		if (!bPanel && (!(Actor->ActorHasTag(TargetTag) || Primitive->ComponentHasTag(TargetTag))
			|| !Primitive->IsSimulatingPhysics()))
		{
			continue;
		}
		FVector Contact;
		const float Distance = Primitive->GetClosestPointOnCollision(Origin, Contact);
		if (Distance < 0.0f)
		{
			Contact = Primitive->Bounds.Origin;
		}
		const float Range = FVector::Dist2D(Origin, Contact);
		if (Range < Radius)
		{
			Candidates.Add({Primitive, Contact, Range, bPanel});
		}
	}
	Candidates.Sort([](const FCandidate& A, const FCandidate& B) { return A.Distance < B.Distance; });
	int32 Reactions = 0;
	for (const FCandidate& Candidate : Candidates)
	{
		FHitResult Obstruction;
		if (GetWorld()->LineTraceSingleByChannel(Obstruction, Origin + ImpactNormal * 20.0f,
			Candidate.Component->Bounds.Origin, ECC_Visibility, Query) && Obstruction.GetActor() != Candidate.Component->GetOwner())
		{
			continue;
		}
		const float Falloff = 1.0f - Candidate.Distance / Radius;
		const float Strength = FMath::Min(ImpactSpeed, 3600.0f) * Falloff;
		if (Candidate.bPanel)
		{
			// Only dedicated, prepared panels may respond; the circuit's throw panel uses its own hit path.
			if (Strength < 230.0f)
			{
				continue;
			}
			Candidate.Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Candidate.Component->SetVisibility(false, true);
		}
		else
		{
			const FVector Radial = (Candidate.Component->Bounds.Origin - Origin).GetSafeNormal2D();
			const FVector Direction = (Radial + ImpactNormal * 0.48f).GetSafeNormal();
			Candidate.Component->WakeAllRigidBodies();
			Candidate.Component->AddImpulse(Direction * FMath::Min(Strength * 0.42f, 950.0f), NAME_None, true);
		}
		if (++Reactions >= MaxReactions)
		{
			break;
		}
	}
}

void UImpactLandingComponent::DrawShockwave() const
{
	if (!ShockwaveLines)
	{
		return;
	}
	const float Progress = FMath::Clamp(PhaseTime / ShockwaveSeconds, 0.0f, 1.0f);
	const float Opacity = FMath::Square(1.0f - Progress);
	const float Distance = Radius * FMath::InterpEaseOut(0.06f, 1.0f, Progress, 1.65f);
	const FLinearColor Color(0.23f * Opacity, 0.76f * Opacity, Opacity, 1.0f);
	FVector Right;
	FVector Forward;
	ImpactNormal.FindBestAxisVectors(Right, Forward);
	TArray<FBatchedLine, TInlineAllocator<40>> Lines;
	constexpr int32 Sides = 40;
	FVector Last = ImpactOrigin + Right * Distance;
	for (int32 Index = 1; Index <= Sides; ++Index)
	{
		const float Angle = 2.0f * PI * Index / Sides;
		const FVector Point = ImpactOrigin + (Right * FMath::Cos(Angle) + Forward * FMath::Sin(Angle)) * Distance;
		Lines.Emplace(Last, Point, Color, 0.0f, FMath::Lerp(6.0f, 1.5f, Progress), SDPG_World);
		Last = Point;
	}
	ShockwaveLines->Flush();
	ShockwaveLines->DrawLines(Lines);
}

void UImpactLandingComponent::PlayImpactSound(const FVector& Origin)
{
	if (!GetWorld()->GetAudioDevice())
	{
		return;
	}
	// A short synthesized low thump uses the engine audio path and needs no external asset.
	constexpr int32 SampleRate = 22050;
	constexpr float Duration = 0.27f;
	TArray<int16> Samples;
	Samples.SetNumUninitialized(static_cast<int32>(SampleRate * Duration));
	FRandomStream Noise(1749);
	float FilteredNoise = 0.0f;
	for (int32 Index = 0; Index < Samples.Num(); ++Index)
	{
		const float Time = static_cast<float>(Index) / SampleRate;
		FilteredNoise = FMath::Lerp(FilteredNoise, Noise.FRandRange(-1.0f, 1.0f), 0.12f);
		const float Envelope = FMath::Clamp(Time * 450.0f, 0.0f, 1.0f) * FMath::Exp(-Time * 15.0f);
		const float Low = FMath::Sin(2.0f * PI * (72.0f * Time - 52.0f * Time * Time));
		Samples[Index] = static_cast<int16>((Low * 0.70f + FilteredNoise * 0.30f) * Envelope * 24500.0f);
	}
	USoundWaveProcedural* Sound = NewObject<USoundWaveProcedural>(this);
	Sound->SetSampleRate(SampleRate);
	Sound->NumChannels = 1;
	Sound->Duration = Duration;
	Sound->QueueAudio(reinterpret_cast<const uint8*>(Samples.GetData()), Samples.Num() * sizeof(int16));
	if (UAudioComponent* Audio = UGameplayStatics::SpawnSoundAtLocation(this, Sound, Origin, FRotator::ZeroRotator, 0.64f))
	{
		Audio->FadeOut(Duration, 0.0f);
	}
}
