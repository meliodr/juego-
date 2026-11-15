#include "ImpactFlightFeedback.h"

#include "AudioDeviceHandle.h"
#include "Components/AudioComponent.h"
#include "Components/LineBatchComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Misc/App.h"
#include "SceneManagement.h"
#include "Sound/SoundWaveProcedural.h"

namespace
{
	constexpr int32 SampleRate = 22050;
	constexpr int32 WindSamples = SampleRate;
	constexpr int32 MaxTrailPoints = 12;
	constexpr float TrailInterval = 1.0f / 36.0f;
	constexpr double TrailLifetime = 0.25;
	constexpr float TrailMinimumSpeed = 1100.0f;
	constexpr float WallProbeDistance = 360.0f;

	float SamplePeriodicNoise(const TArray<float>& Samples, float Phase)
	{
		const float Sample = Phase * Samples.Num();
		const int32 Index = FMath::FloorToInt(Sample);
		const float T = Sample - Index;
		return FMath::Lerp(Samples[Index % Samples.Num()], Samples[(Index + 1) % Samples.Num()], T * T * (3.0f - 2.0f * T));
	}
}

void UImpactFlightFeedback::UpdateFlightFeedback(float DeltaSeconds, bool bFlying, bool bTurbo)
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn || !Pawn->IsLocallyControlled())
	{
		if (WindAudio && WindAudio->IsPlaying())
		{
			WindAudio->Stop();
			WindWave->ResetAudio();
		}
		WindVolume = 0.0f;
		TurboBurst = 0.0f;
		bWasTurbo = false;
		ClearTrail();
		return;
	}

	const FVector Velocity = Pawn->GetVelocity();
	const float Speed = static_cast<float>(Velocity.Size());
	FVector Right = FVector::CrossProduct(FVector::UpVector, Velocity).GetSafeNormal();
	if (Right.IsNearlyZero())
	{
		Right = Pawn->GetActorRightVector();
	}
	UpdateWind(DeltaSeconds, Speed, bFlying, bTurbo, Right);
	UpdateTrail(DeltaSeconds, Speed, bFlying, bTurbo, Right);
}

void UImpactFlightFeedback::CreateWind()
{
	if (!GetWorld()->GetAudioDevice())
	{
		return;
	}
	// Use the same native procedural-audio path as the landing impact. A wrapped noise buffer
	// gives a continuous, pitched and filtered wind bed without an external asset or plugin.
	FRandomStream Random(1721);
	const auto MakeNoise = [&Random](int32 NumSamples)
	{
		TArray<float> Samples;
		Samples.SetNumUninitialized(NumSamples);
		for (float& Value : Samples)
		{
			Value = Random.FRandRange(-1.0f, 1.0f);
		}
		return Samples;
	};
	const TArray<float> Rumble = MakeNoise(64);
	const TArray<float> Body = MakeNoise(285);
	const TArray<float> Air = MakeNoise(1920);
	WindLoop.SetNumUninitialized(WindSamples);
	for (int32 Index = 0; Index < WindLoop.Num(); ++Index)
	{
		const float Phase = static_cast<float>(Index) / WindSamples;
		const float Wind = 0.28f * SamplePeriodicNoise(Rumble, Phase) + 0.51f * SamplePeriodicNoise(Body, Phase)
			+ 0.21f * SamplePeriodicNoise(Air, Phase);
		WindLoop[Index] = static_cast<int16>(Wind * 24500.0f);
	}

	WindWave = NewObject<USoundWaveProcedural>(this);
	WindWave->SetSampleRate(SampleRate);
	WindWave->NumChannels = 1;
	WindWave->Duration = INDEFINITELY_LOOPING_DURATION;
	WindWave->bLooping = true;
	WindAudio = NewObject<UAudioComponent>(GetOwner());
	GetOwner()->AddInstanceComponent(WindAudio);
	WindAudio->bAutoActivate = false;
	WindAudio->bAutoDestroy = false;
	WindAudio->bAllowSpatialization = false;
	WindAudio->SetSound(WindWave);
	WindAudio->RegisterComponent();
}

void UImpactFlightFeedback::UpdateWind(float DeltaSeconds, float Speed, bool bFlying, bool bTurbo, const FVector& Right)
{
	if (!WindAudio && bFlying && WindVolumeScale > 0.0f)
	{
		CreateWind();
	}
	if (!WindAudio)
	{
		return;
	}

	WallCheckRemaining -= DeltaSeconds;
	if (bFlying && Speed > 2700.0f && WallCheckRemaining <= 0.0f)
	{
		WallCheckRemaining = 0.14f;
		const FVector Origin = GetOwner()->GetActorLocation();
		const FCollisionQueryParams Query(SCENE_QUERY_STAT(ImpactFlightWhoosh), false, GetOwner());
		NearbyWall = 0.0f;
		for (const float Side : {-1.0f, 1.0f})
		{
			FHitResult Hit;
			if (GetWorld()->LineTraceSingleByChannel(Hit, Origin, Origin + Right * WallProbeDistance * Side, ECC_Visibility, Query))
			{
				NearbyWall = FMath::Max(NearbyWall, 1.0f - Hit.Distance / WallProbeDistance);
			}
		}
	}
	else if (!bFlying || Speed <= 2700.0f)
	{
		NearbyWall = 0.0f;
	}
	WallAccent = FMath::FInterpTo(WallAccent, NearbyWall, DeltaSeconds, NearbyWall > WallAccent ? 15.0f : 3.5f);

	const float SpeedAlpha = FMath::Sqrt(FMath::Clamp(Speed / 32000.0f, 0.0f, 1.0f));
	const bool bTurboActive = bFlying && bTurbo;
	if (bTurboActive && !bWasTurbo)
	{
		TurboBurst = 0.22f;
	}
	bWasTurbo = bTurboActive;
	TurboBurst = bFlying ? FMath::FInterpTo(TurboBurst, 0.0f, DeltaSeconds, 3.8f) : 0.0f;
	const float TurboAccent = (bTurboActive ? 0.17f * SpeedAlpha : 0.0f) + TurboBurst;
	const float TargetVolume = bFlying
		? WindVolumeScale * FMath::Min(0.99f, 0.055f + 0.69f * SpeedAlpha + TurboAccent + WallAccent * 0.17f)
		: 0.0f;
	WindVolume = FMath::FInterpTo(WindVolume, TargetVolume, DeltaSeconds, 5.0f);
	WindPitch = FMath::FInterpTo(WindPitch, 0.80f + SpeedAlpha * 0.61f + TurboAccent + WallAccent * 0.13f,
		DeltaSeconds, 5.0f);
	if (!bFlying && WindVolume <= 0.006f)
	{
		if (WindAudio->IsPlaying())
		{
			WindAudio->Stop();
			WindWave->ResetAudio();
		}
		return;
	}

	const int32 LoopBytes = WindLoop.Num() * sizeof(int16);
	if (WindWave->GetAvailableAudioByteCount() <= LoopBytes)
	{
		WindWave->QueueAudio(reinterpret_cast<const uint8*>(WindLoop.GetData()), LoopBytes);
	}
	WindAudio->SetVolumeMultiplier(WindVolume);
	WindAudio->SetPitchMultiplier(WindPitch);
	if (!WindAudio->IsPlaying())
	{
		WindAudio->Play();
	}
}

void UImpactFlightFeedback::UpdateTrail(float DeltaSeconds, float Speed, bool bFlying, bool bTurbo, const FVector& Right)
{
	if (!bDrawTrail || !bFlying || Speed < TrailMinimumSpeed || !FApp::CanEverRender())
	{
		ClearTrail();
		return;
	}
	TrailAccumulator += DeltaSeconds;
	if (TrailAccumulator < TrailInterval)
	{
		return;
	}
	TrailAccumulator = FMath::Fmod(TrailAccumulator, TrailInterval);
	if (!FlightTrail)
	{
		FlightTrail = NewObject<ULineBatchComponent>(GetOwner());
		GetOwner()->AddInstanceComponent(FlightTrail);
		FlightTrail->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		FlightTrail->bCalculateAccurateBounds = true;
		FlightTrail->RegisterComponent();
	}

	const double Now = GetWorld()->GetTimeSeconds();
	const FVector Origin = GetOwner()->GetActorLocation();
	const float Intensity = FMath::Clamp((Speed - TrailMinimumSpeed) / 14000.0f + (bTurbo ? 0.18f : 0.0f), 0.1f, 1.0f);
	const double Offset = 73.0 + Intensity * 27.0;
	const FVector Left = Origin - Right * Offset;
	const FVector RightPoint = Origin + Right * Offset;
	if (!Trail.IsEmpty() && FVector::DistSquared(Left, Trail.Last().Left) > FMath::Square(5000.0))
	{
		// Do not connect a streak across a teleport or streaming discontinuity.
		Trail.Reset();
	}
	while (!Trail.IsEmpty() && (Now - Trail[0].Time > TrailLifetime || Trail.Num() >= MaxTrailPoints))
	{
		Trail.RemoveAt(0, EAllowShrinking::No);
	}
	Trail.Add({Left, RightPoint, Now});
	TArray<FBatchedLine, TInlineAllocator<2 * MaxTrailPoints>> Lines;
	for (int32 Index = 1; Index < Trail.Num(); ++Index)
	{
		const float Age = static_cast<float>((Now - Trail[Index - 1].Time) / TrailLifetime);
		const float Amount = Intensity * FMath::Square(1.0f - Age);
		const FLinearColor Color(0.24f * Amount, 0.66f * Amount, 0.86f * Amount, 1.0f);
		const float Width = FMath::Lerp(0.65f, 3.2f, Amount);
		Lines.Emplace(Trail[Index - 1].Left, Trail[Index].Left, Color, 0.0f, Width, SDPG_World);
		Lines.Emplace(Trail[Index - 1].Right, Trail[Index].Right, Color, 0.0f, Width, SDPG_World);
	}
	FlightTrail->Flush();
	FlightTrail->DrawLines(Lines);
}

void UImpactFlightFeedback::ClearTrail()
{
	if (!Trail.IsEmpty() && FlightTrail)
	{
		FlightTrail->Flush();
	}
	Trail.Reset();
	TrailAccumulator = 0.0f;
}

void UImpactFlightFeedback::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (WindAudio)
	{
		WindAudio->Stop();
		WindAudio->DestroyComponent();
	}
	if (FlightTrail)
	{
		FlightTrail->DestroyComponent();
	}
	Super::EndPlay(EndPlayReason);
}
