#include "ImpactFlightFeedback.h"

#include "AudioDeviceHandle.h"
#include "Components/AudioComponent.h"
#include "Components/LineBatchComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Misc/App.h"
#include "SceneManagement.h"
#include "Sound/SoundWaveProcedural.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#endif

namespace
{
	constexpr int32 SampleRate = 22050;
	constexpr int32 WindSamples = SampleRate;
	constexpr int32 MaxAirStrokes = 8;
	constexpr int32 SegmentsPerStroke = 5;
	constexpr float WallProbeDistance = 360.0f;
	using FAirLines = TArray<FBatchedLine, TInlineAllocator<MaxAirStrokes * SegmentsPerStroke>>;

	float Follow(float Current, float Target, float DeltaSeconds, float Rise, float Fall)
	{
		return FMath::Lerp(Current, Target, 1.0f - FMath::Exp(-DeltaSeconds / (Target > Current ? Rise : Fall)));
	}

	bool IsDiscontinuity(const FVector& Previous, const FVector& Current, float Speed, float DeltaSeconds)
	{
		// Permit real high-speed travel; suppress spatial jumps and stale/paused samples.
		return DeltaSeconds > 0.25f || FVector::DistSquared(Previous, Current)
			> FMath::Square(FMath::Max(1500.0f, Speed * DeltaSeconds * 2.0f + 200.0f));
	}

	float BuildAirStrokes(FAirLines& Lines, const FVector& LeftHand, const FVector& RightHand,
		const FVector& Direction, const FVector& Right, float Strength, float Boost, float Phase,
		float RequestedLength, float OpacityScale)
	{
		const float MaxLength = FMath::Clamp(RequestedLength, 50.0f, 400.0f);
		const float Length = MaxLength * FMath::Clamp(0.20f + Strength * 0.60f + Boost * 0.15f, 0.0f, 0.95f);
		const float Span = Length * (0.28f + 0.10f * Boost);
		const FVector Normal = FVector::CrossProduct(Direction, Right).GetSafeNormal();
		float Extent = 0.0f;
		for (int32 Stroke = 0; Stroke < MaxAirStrokes; ++Stroke)
		{
			const float Weight = FMath::Clamp(3.0f + Strength * 3.0f + Boost * 2.0f - Stroke, 0.0f, 1.0f);
			const float Age = FMath::Frac(Phase + Stroke * 0.618034f);
			const float Envelope = FMath::Square(FMath::Sin(PI * Age));
			const float Opacity = FMath::Clamp(OpacityScale, 0.0f, 1.0f) * Strength * Weight * Envelope;
			if (Opacity < 0.003f) continue;
			const float Side = Stroke % 2 == 0 ? -1.0f : 1.0f;
			const FVector Anchor = Side < 0 ? LeftHand : RightHand;
			const float Head = Age * (Length - Span);
			const auto Point = [&](float T)
			{
				const float Distance = Head + Span * T;
				const float Curve = FMath::Sin(PI * T) * MaxLength * (0.018f + 0.012f * Strength);
				const FVector Offset = -Direction * Distance + Right * Side * (Stroke / 2 * 3.0f + Curve)
					+ Normal * (FMath::Sin(Stroke * 2.1f + T * 1.6f) * MaxLength * 0.025f);
				return Anchor + Offset.GetClampedToMaxSize(MaxLength);
			};
			FVector Start = Point(0.0f);
			for (int32 Segment = 0; Segment < SegmentsPerStroke; ++Segment)
			{
				const FVector End = Point(static_cast<float>(Segment + 1) / SegmentsPerStroke);
				const float Taper = FMath::Sin(PI * (Segment + 0.5f) / SegmentsPerStroke);
				const float Alpha = Opacity * Taper * 0.60f;
				// UE 5.8's line-batch proxy uses DrawTranslucentLine: fade alpha, not white towards black.
				const FLinearColor Color(0.88f, 0.94f, 1.0f, Alpha);
				Lines.Emplace(Start, End, Color, 0.0f, 0.65f + 1.10f * Opacity * Taper, SDPG_World);
				Extent = FMath::Max(Extent, static_cast<float>(FMath::Max((Start - Anchor).Size(), (End - Anchor).Size())));
				Start = End;
			}
		}
		return Extent;
	}

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
		SpeedStrength = BoostStrength = 0.0f;
		ObservedSpeedCmS = ObservedAccelerationCmS2 = 0.0f;
		bHasMotionSample = false;
		ClearTrail();
		return;
	}

	const FVector Velocity = Pawn->GetVelocity();
	const float Speed = static_cast<float>(Velocity.Size());
	const FVector Location = Pawn->GetActorLocation();
	if (!FMath::IsFinite(DeltaSeconds) || DeltaSeconds <= 0.0f || !FMath::IsFinite(Speed) || Location.ContainsNaN())
	{
		ClearTrail();
		return;
	}
	const bool bFirstSample = !bHasMotionSample;
	const bool bDiscontinuity = bHasMotionSample && IsDiscontinuity(PreviousLocation, Location,
		FMath::Max(Speed, PreviousSpeed), DeltaSeconds);
	AdvancePresentation(DeltaSeconds, Speed, bFlying, bTurbo, bDiscontinuity);
	PreviousLocation = Location;
	const FVector Direction = ResolveFlowDirection(Velocity, Pawn->GetActorForwardVector(), bFirstSample || bDiscontinuity);
	FVector Right = FVector::CrossProduct(GetOwner()->GetActorUpVector(), Direction).GetSafeNormal();
	if (Right.IsNearlyZero())
	{
		Right = FVector::VectorPlaneProject(Pawn->GetActorRightVector(), Direction).GetSafeNormal();
	}
	UpdateWind(DeltaSeconds, bFlying, Right);
	UpdateTrail(Direction, Right);
}

FVector UImpactFlightFeedback::ResolveFlowDirection(const FVector& Velocity, const FVector& Fallback, bool bReset)
{
	if (bReset) LastFlowDirection = Fallback.GetSafeNormal();
	// Retain the last airflow as it fades after braking, including backward/lateral flight.
	// Tiny stopping corrections must not spin the remaining strokes around the hands.
	if (Velocity.SizeSquared() >= 100.0 * 100.0) LastFlowDirection = Velocity.GetSafeNormal();
	return LastFlowDirection;
}

void UImpactFlightFeedback::AdvancePresentation(float DeltaSeconds, float Speed, bool bFlying, bool bTurbo, bool bDiscontinuity)
{
	ObservedSpeedCmS = Speed;
	ObservedAccelerationCmS2 = bHasMotionSample && !bDiscontinuity ? (Speed - PreviousSpeed) / DeltaSeconds : 0.0f;
	PreviousSpeed = Speed;
	bHasMotionSample = true;
	if (bDiscontinuity)
	{
		++DiscontinuityResets;
		SpeedStrength = BoostStrength = FlowPhase = 0.0f;
		NearbyWall = WallAccent = 0.0f;
		ClearTrail();
		return;
	}
	const float Onset = FMath::SmoothStep(600.0f, 1400.0f, Speed);
	const float TargetStrength = bFlying ? Onset * FMath::Sqrt(FMath::Clamp((Speed - 600.0f) / 21400.0f, 0.0f, 1.0f)) : 0.0f;
	SpeedStrength = Follow(SpeedStrength, TargetStrength, DeltaSeconds, 0.10f, 0.18f);
	const float TargetBoost = bFlying && bTurbo && Speed > 1200.0f
		? Onset * FMath::Clamp((ObservedAccelerationCmS2 - 1500.0f) / 7500.0f, 0.0f, 1.0f) : 0.0f;
	BoostStrength = Follow(BoostStrength, TargetBoost, DeltaSeconds, 0.045f, 0.16f);
	FlowPhase = FMath::Frac(FlowPhase + DeltaSeconds * (0.80f + SpeedStrength * 2.50f + BoostStrength * 1.50f));
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
		const float Wind = 0.28f * SamplePeriodicNoise(Rumble, Phase) + 0.60f * SamplePeriodicNoise(Body, Phase)
			+ 0.12f * SamplePeriodicNoise(Air, Phase);
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

void UImpactFlightFeedback::UpdateWind(float DeltaSeconds, bool bFlying, const FVector& Right)
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
	if (bFlying && ObservedSpeedCmS > 2700.0f && WallCheckRemaining <= 0.0f)
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
	else if (!bFlying || ObservedSpeedCmS <= 2700.0f)
	{
		NearbyWall = 0.0f;
	}
	WallAccent = Follow(WallAccent, NearbyWall, DeltaSeconds, 0.07f, 0.28f);
	const float TargetVolume = bFlying
		? FMath::Clamp(WindVolumeScale, 0.0f, 1.0f) * (0.012f + 0.60f * SpeedStrength + 0.12f * BoostStrength + WallAccent * 0.08f)
		: 0.0f;
	WindVolume = Follow(WindVolume, TargetVolume, DeltaSeconds, 0.10f, 0.25f);
	WindPitch = Follow(WindPitch, 0.80f + SpeedStrength * 0.42f + BoostStrength * 0.09f + WallAccent * 0.07f,
		DeltaSeconds, 0.10f, 0.20f);
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

void UImpactFlightFeedback::UpdateTrail(const FVector& Direction, const FVector& Right)
{
	if (!bDrawTrail || SpeedStrength < 0.005f || TrailOpacityScale <= 0.0f || !FApp::CanEverRender())
	{
		ClearTrail();
		return;
	}
	if (!FlightTrail)
	{
		FlightTrail = NewObject<ULineBatchComponent>(GetOwner());
		GetOwner()->AddInstanceComponent(FlightTrail);
		FlightTrail->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		FlightTrail->bCalculateAccurateBounds = true;
		FlightTrail->RegisterComponent();
	}

	const FVector Origin = GetOwner()->GetActorLocation();
	FVector LeftHand = Origin - Right * 55.0f;
	FVector RightHand = Origin + Right * 55.0f;
	bUsingHandAnchors = false;
	if (const ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		const USkeletalMeshComponent* Mesh = Character->GetMesh();
		if (Mesh && Mesh->GetBoneIndex(TEXT("hand_l")) != INDEX_NONE && Mesh->GetBoneIndex(TEXT("hand_r")) != INDEX_NONE)
		{
			const FVector L = Mesh->GetSocketLocation(TEXT("hand_l"));
			const FVector R = Mesh->GetSocketLocation(TEXT("hand_r"));
			if (!L.ContainsNaN() && !R.ContainsNaN() && FVector::DistSquared(Origin, L) < 62500.0
				&& FVector::DistSquared(Origin, R) < 62500.0)
			{
				LeftHand = L;
				RightHand = R;
				bUsingHandAnchors = true;
			}
		}
	}
	// Rebuild only around current anchors. No stored world positions can bridge a teleport.
	FAirLines Lines;
	LastTrailExtentCm = BuildAirStrokes(Lines, LeftHand, RightHand, Direction, Right, SpeedStrength,
		BoostStrength, FlowPhase, MaxTrailLengthCm, TrailOpacityScale);
	LastDrawnSegments = Lines.Num();
	FlightTrail->Flush();
	FlightTrail->DrawLines(Lines);
}

void UImpactFlightFeedback::ClearTrail()
{
	if (FlightTrail)
	{
		FlightTrail->Flush();
	}
	LastDrawnSegments = 0;
	LastTrailExtentCm = 0.0f;
	bUsingHandAnchors = false;
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

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FImpactFlightFeedbackPresentationTest, "ImpactBeyond.Presentation.FlightFeedback",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FImpactFlightFeedbackPresentationTest::RunTest(const FString& Parameters)
{
	UImpactFlightFeedback* Feedback = NewObject<UImpactFlightFeedback>();
	for (const FVector Travel : {-FVector::ForwardVector, FVector::RightVector, FVector::UpVector})
	{
		Feedback->ResolveFlowDirection(Travel * 4000.0, FVector::ForwardVector, true);
		TestTrue(TEXT("Braking preserves the residual airflow direction"),
			Feedback->ResolveFlowDirection(FVector::ZeroVector, FVector::ForwardVector, false).Equals(Travel));
		TestTrue(TEXT("Tiny opposite stopping corrections cannot flip the residual trail"),
			Feedback->ResolveFlowDirection(-Travel * 20.0, FVector::ForwardVector, false).Equals(Travel));
	}
	TestTrue(TEXT("A stationary teleport discards the previous airflow direction"),
		Feedback->ResolveFlowDirection(FVector::ZeroVector, -FVector::RightVector, true).Equals(-FVector::RightVector));
	for (int32 Frame = 0; Frame < 120; ++Frame)
		Feedback->AdvancePresentation(1.0f / 60.0f, 0.0f, true, true, false);
	TestEqual(TEXT("Hover has no air strokes or turbo gust"), Feedback->SpeedStrength + Feedback->BoostStrength, 0.0f);
	Feedback->AdvancePresentation(0.016f, 4000.0f, true, false, true);
	for (int32 Frame = 0; Frame < 60; ++Frame)
		Feedback->AdvancePresentation(1.0f / 60.0f, 4000.0f, true, true, false);
	TestEqual(TEXT("Turbo at constant speed does not invent acceleration"), Feedback->BoostStrength, 0.0f);
	TestTrue(TEXT("Ordinary cruise remains visible without turbo"), Feedback->SpeedStrength > 0.25f && Feedback->SpeedStrength < 0.60f);
	for (int32 Frame = 1; Frame <= 12; ++Frame)
		Feedback->AdvancePresentation(1.0f / 60.0f, 4000.0f + Frame * 150.0f, true, true, false);
	TestTrue(TEXT("Measured acceleration produces a gust"), Feedback->BoostStrength > 0.8f);
	Feedback->AdvancePresentation(0.016f, 28600.0f, true, true, true);
	TestEqual(TEXT("Teleport discards old acceleration and gust"), Feedback->ObservedAccelerationCmS2 + Feedback->BoostStrength, 0.0f);
	Feedback->AdvancePresentation(0.016f, 28600.0f, true, true, false);
	TestEqual(TEXT("The first post-teleport sample does not invent a gust"), Feedback->BoostStrength, 0.0f);
	for (int32 Frame = 0; Frame < 120; ++Frame)
		Feedback->AdvancePresentation(1.0f / 60.0f, 28600.0f, false, true, false);
	TestTrue(TEXT("Grounded motion fades flight cues even at high speed"), Feedback->SpeedStrength < 0.001f && Feedback->BoostStrength < 0.001f);

	UImpactFlightFeedback* SlowFrames = NewObject<UImpactFlightFeedback>();
	UImpactFlightFeedback* FastFrames = NewObject<UImpactFlightFeedback>();
	for (int32 Frame = 0; Frame < 30; ++Frame)
		SlowFrames->AdvancePresentation(1.0f / 30.0f, 22000.0f, true, false, false);
	for (int32 Frame = 0; Frame < 120; ++Frame)
		FastFrames->AdvancePresentation(1.0f / 120.0f, 22000.0f, true, false, false);
	TestTrue(TEXT("Strength transition follows elapsed time at 30/120 Hz"),
		FMath::IsNearlyEqual(SlowFrames->SpeedStrength, FastFrames->SpeedStrength, 0.0001f));
	TestFalse(TEXT("Real travel at 368 m/s is not a teleport"), IsDiscontinuity(FVector::ZeroVector, FVector(3680,0,0), 36800.0f, 0.10f));
	TestTrue(TEXT("A 25 m blink at cruise resets feedback"), IsDiscontinuity(FVector::ZeroVector, FVector(2500,0,0), 4000.0f, 0.016f));

	for (const FVector Direction : {FVector::ForwardVector, FVector::UpVector, -FVector::RightVector})
	{
		const FVector Right = FVector::CrossProduct(Direction, FVector(1,1,1)).GetSafeNormal();
		const FVector Offset(9000000, -7000000, 5000000);
		const FVector LeftHand = Offset - Right * 55.0f;
		const FVector RightHand = Offset + Right * 55.0f;
		for (const float Phase : {0.0f, 0.17f, 0.54f, 0.99f})
		{
			FAirLines Lines;
			const float Extent = BuildAirStrokes(Lines, LeftHand, RightHand, Direction, Right, 1.0f, 1.0f, Phase, 50000.0f, 1.0f);
			TestTrue(TEXT("Air strokes have a fixed rendering budget"), Lines.Num() > 0 && Lines.Num() <= 40);
			TestTrue(TEXT("Even an excessive configured length is capped at four metres"), Extent <= 400.01f);
			for (const FBatchedLine& Line : Lines)
			for (const FVector Point : {Line.Start, Line.End})
			{
				TestFalse(TEXT("Radial air stroke remains finite"), Point.ContainsNaN());
				TestTrue(TEXT("All vertices belong to current anchors, never the pre-teleport origin"),
					FMath::Min(FVector::DistSquared(Point, LeftHand), FVector::DistSquared(Point, RightHand)) <= FMath::Square(400.01));
			}
		}
	}
	return true;
}
#endif
