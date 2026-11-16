#include "ImpactBeyondCharacter.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "ImpactCircuitComponent.h"
#include "ImpactLandingComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Misc/AutomationTest.h"

namespace
{
class FFlightStep final : public IAutomationLatentCommand
{
public:
	explicit FFlightStep(TFunction<bool()> InUpdate) : Step(MoveTemp(InUpdate)) {}
	bool Update() override { return Step(); }

private:
	TFunction<bool()> Step;
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FImpactFlightTest, "ImpactBeyond.Flight.Headless",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FImpactFlightTest::RunTest(const FString& Parameters)
{
	UWorld* World = nullptr;
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::Game)
		{
			World = Context.World();
			break;
		}
	}
	APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	AImpactBeyondCharacter* Pawn = PC ? Cast<AImpactBeyondCharacter>(PC->GetPawn()) : nullptr;
	if (!TestNotNull(TEXT("Possessed Third Person pawn"), Pawn))
	{
		return false;
	}

	auto* Movement = Pawn->GetCharacterMovement();
	const FVector Origin(100000, 0, 3000);
	auto MakeBox = [World](FVector Center, FVector Extent)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.ObjectFlags |= RF_Transient;
		AActor* Actor = World->SpawnActor<AActor>(SpawnParameters);
		UBoxComponent* Box = NewObject<UBoxComponent>(Actor);
		Actor->SetRootComponent(Box);
		Box->InitBoxExtent(Extent);
		Box->SetCollisionProfileName(TEXT("BlockAll"));
		Actor->SetActorLocation(Center);
		Box->RegisterComponent();
		return Actor;
	};
	AActor* Floor = MakeBox(Origin - FVector(0, 0, 20), FVector(230000, 120000, 20));
	const auto Wall = MakeShared<AActor*>(nullptr);
	const FRotator OriginalView = PC->GetControlRotation();
	const FVector OriginalLocation = Pawn->GetActorLocation();
	const FRotator OriginalRotation = Pawn->GetActorRotation();
	const auto OriginalMode = Movement->MovementMode.GetValue();
	const float OriginalFlySpeed = Movement->MaxFlySpeed;
	const float OriginalBrake = Movement->BrakingDecelerationFlying;
	const FVector OriginalVelocity = Movement->Velocity;

	auto Key = [PC](FKey Input, bool bHeld)
	{
		PC->InputKey(FInputKeyEventArgs::CreateSimulated(Input, bHeld ? IE_Pressed : IE_Released, bHeld ? 1.f : 0.f));
	};
	// Latent commands can run back-to-back during one automation tick.
	const auto LastFrame = MakeShared<uint64>(MAX_uint64);
	auto ForFrames = [LastFrame](int32 Frames, TFunction<void(int32)> During)
	{
		FAutomationTestFramework::Get().EnqueueLatentCommand(
			MakeShared<FFlightStep>([LastFrame, Frame = 0, Frames, During = MoveTemp(During)]() mutable
			{
				if (*LastFrame == GFrameCounter)
				{
					return false;
				}
				*LastFrame = GFrameCounter;
				During(Frame++);
				return Frame >= Frames;
			}));
	};
	auto Once = [ForFrames](TFunction<void()> Action)
	{
		ForFrames(1, [Action = MoveTemp(Action)](int32) { Action(); });
	};

	Once([=, this]()
	{
		Movement->StopMovementImmediately();
		Pawn->SetActorLocation(Origin + FVector(0, 0, Pawn->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 2));
		PC->SetControlRotation(FRotator::ZeroRotator);
		Movement->SetMovementMode(MOVE_Walking);
	});
	for (int32 Cycle = 0; Cycle < 30; ++Cycle)
	{
		ForFrames(44, [=, this](int32 Frame)
		{
			if (Frame == 0)
			{
				TestTrue(FString::Printf(TEXT("Cycle %d starts on ground"), Cycle + 1), Movement->IsMovingOnGround());
				Key(EKeys::F, true);
			}
			if (Frame == 1)
			{
				Key(EKeys::F, false);
			}
			if (Frame == 43)
			{
				TestTrue(TEXT("Takeoff enters flying"), Movement->IsFlying());
				TestTrue(TEXT("Hover animation plays instead of freezing"), Pawn->GetMesh()->IsPlaying());
				TestTrue(TEXT("Capsule rises above walking height"), Pawn->GetActorLocation().Z > Origin.Z + 115);
				Key(EKeys::C, true);
			}
		});
		ForFrames(35, [=, this](int32 Frame)
		{
			if (Frame == 34)
			{
				TestTrue(TEXT("Descending onto floor returns to walking"), Movement->IsMovingOnGround());
				Key(EKeys::C, false);
			}
		});
	}

	ForFrames(30, [=](int32) { Pawn->DoMove(0, 1); });
	Once([=, this]()
	{
		TestTrue(TEXT("Running on ground before jump"), Movement->IsMovingOnGround() && Movement->Velocity.Size2D() > 400);
		Pawn->DoJumpStart();
	});
	ForFrames(6, [=](int32) { Pawn->DoMove(0, 1); });
	Once([=, this]()
	{
		TestTrue(TEXT("Jump enters falling while rising"), Movement->IsFalling() && Movement->Velocity.Z > 0);
		Pawn->DoJumpEnd();
		Key(EKeys::F, true);
	});
	Once([=, this]() { Key(EKeys::F, false); });
	auto SpeedStage = [=, this](FKey Change, float Target)
	{
		const auto EarlyHeight = MakeShared<double>(0);
		const auto CruiseHeight = MakeShared<double>(0);
		ForFrames(360, [=, this](int32 Frame)
		{
			if (Frame == 0 && Change.IsValid()) Key(Change, true);
			if (Frame == 1 && Change.IsValid()) Key(Change, false);
			Pawn->DoMove(0, 1);
			if (Frame == 84) *EarlyHeight = Pawn->GetActorLocation().Z;
			if (Frame == 144)
			{
				AddInfo(FString::Printf(TEXT("Early cruise %.0f cm/s: delta Z %.2f cm"),
					Target, Pawn->GetActorLocation().Z - *EarlyHeight));
			}
			if (Frame == 299) *CruiseHeight = Pawn->GetActorLocation().Z;
			if (Frame == 359)
			{
				const double Speed = Movement->Velocity.Size();
				const double HeightChange = Pawn->GetActorLocation().Z - *CruiseHeight;
				AddInfo(FString::Printf(TEXT("Steady cruise %.0f cm/s: delta Z %.2f cm"), Target, HeightChange));
				AddInfo(FString::Printf(TEXT("Flight cap %.0f cm/s, measured %.1f cm/s"), Target, Speed));
				TestTrue(TEXT("Horizontal cruise holds altitude"), FMath::Abs(HeightChange) <= 5.0);
				TestTrue(TEXT("Flying at requested cap"), Movement->IsFlying() && FMath::IsNearlyEqual(Speed, Target, 30.0));
				TestTrue(TEXT("Cruise animation plays instead of freezing"), Pawn->GetMesh()->IsPlaying());
			}
		});
	};
	SpeedStage(FKey(), 1200);
	SpeedStage(EKeys::E, 3500);
	SpeedStage(EKeys::Gamepad_RightShoulder, 9000);
	SpeedStage(EKeys::B, 18000);
	SpeedStage(EKeys::Gamepad_RightShoulder, 32000);

	for (const FKey Turbo : {EKeys::LeftShift, EKeys::Gamepad_LeftThumbstick})
	{
		ForFrames(240, [=, this](int32 Frame)
		{
			if (Frame == 0) { Key(Turbo, true); }
			Pawn->DoMove(0, 1);
			if (Frame == 239)
			{
				const double Speed = Movement->Velocity.Size();
				AddInfo(FString::Printf(TEXT("Flight turbo (%s): %.2f m/s"), *Turbo.GetDisplayName().ToString(), Speed / 100.0));
				TestTrue(TEXT("Held turbo reaches 115% of the selected flight cap"), FMath::IsNearlyEqual(Speed, 36800.0, 80.0));
				TestEqual(TEXT("Turbo preserves selected flight level"), Pawn->GetFlightLevel(), 5);
				Key(Turbo, false);
			}
		});
		ForFrames(240, [=, this](int32 Frame)
		{
			Pawn->DoMove(0, 1);
			if (Frame == 239)
			{
				TestTrue(TEXT("Releasing turbo restores the selected flight cap"),
					FMath::IsNearlyEqual(Movement->Velocity.Size(), 32000.0, 80.0));
			}
		});
	}

	const auto CoastSpeed = MakeShared<double>(0);
	ForFrames(21, [=](int32 Frame)
	{
		if (Frame == 20) *CoastSpeed = Movement->Velocity.Size();
	});
	ForFrames(75, [=](int32) { Pawn->DoMove(0, 1); });
	ForFrames(440, [=, this](int32 Frame)
	{
		if (Frame == 0)
		{
			TestTrue(TEXT("Brake comparison starts near top speed"), Movement->Velocity.Size() >= 31000);
			Key(EKeys::X, true);
		}
		Pawn->DoMove(0, 1);
		if (Frame == 20)
		{
			const double BrakeSpeed = Movement->Velocity.Size();
			AddInfo(FString::Printf(TEXT("After 20 frames: coast %.1f, brake %.1f cm/s"), *CoastSpeed, BrakeSpeed));
			TestTrue(TEXT("Brake beats coasting with forward held"), BrakeSpeed + 200 < *CoastSpeed);
		}
		if (Frame == 439)
		{
			TestTrue(TEXT("Brake reaches hover"), Movement->IsFlying() && Movement->Velocity.Size() < 5);
			Key(EKeys::X, false);
		}
	});

	Once([=]() { *Wall = MakeBox(Origin + FVector(150000, 0, 800), FVector(2, 100000, 800)); });

	// Start each sweep with a straight approach long enough to reach the fifth level.
	for (float Lateral : {0.f, 0.12f, -0.12f})
	{
		const bool bTurbo = Lateral < 0;
		Once([=]()
		{
			Movement->StopMovementImmediately();
			Pawn->SetActorLocation(Origin + FVector(3500, Lateral > 0 ? 100 : 0, 500));
			if (bTurbo) { Key(EKeys::LeftShift, true); }
		});
		const auto Peak = MakeShared<double>(0);
		const auto FurthestX = MakeShared<double>(0);
		const double ContactX = 149998 - Pawn->GetCapsuleComponent()->GetScaledCapsuleRadius();
		ForFrames(600, [=, this](int32 Frame)
		{
			Pawn->DoMove(Lateral, 1);
			const double X = Pawn->GetActorLocation().X - Origin.X;
			if (X < ContactX - 10.0)
			{
				*Peak = FMath::Max(*Peak, Movement->Velocity.Size());
			}
			*FurthestX = FMath::Max(*FurthestX, X);
			if (Frame == 599)
			{
				AddInfo(FString::Printf(TEXT("Wall (lateral %.2f): peak %.1f cm/s, max x %.1f cm"),
					Lateral, *Peak, *FurthestX));
				TestTrue(TEXT("Reached maximum flight speed before the thin wall"), *Peak >= (bTurbo ? 36550.0 : 31750.0));
				TestTrue(TEXT("Capsule reached wall without crossing it"), FMath::IsNearlyEqual(*FurthestX, ContactX, 5.0));
				if (bTurbo) { Key(EKeys::LeftShift, false); }
			}
		});
	}

	Once([=, this]()
	{
		(*Wall)->Destroy();
		Movement->StopMovementImmediately();
		Pawn->SetActorLocation(Origin + FVector(0, 0, 500));
	});
	Once([=]() { Key(EKeys::Gamepad_DPad_Right, true); });
	Once([=, this]()
	{
		Key(EKeys::Gamepad_DPad_Right, false);
		TestEqual(TEXT("Gamepad toggle restores the previous control-flight level"), Pawn->GetFlightLevel(), 3);
		Key(EKeys::E, true);
	});
	Once([=, this]()
	{
		Key(EKeys::E, false);
		TestEqual(TEXT("Speed-increase cannot enter Travesia"), Pawn->GetFlightLevel(), 3);
	});
	for (int32 Change = 0; Change < 20; ++Change)
	{
		const bool bDown = Change % 2 == 0;
		const FKey Down = Change % 4 == 0 ? EKeys::Q : EKeys::Gamepad_LeftShoulder;
		const FKey Input = bDown ? Down : EKeys::E;
		Once([=]() { Key(Input, true); });
		Once([=, this]()
		{
			Key(Input, false);
			TestEqual(TEXT("Alternating control-flight level selection"), Pawn->GetFlightLevel(), bDown ? 2 : 3);
		});
	}
	Once([=]() { Key(EKeys::Gamepad_RightThumbstick, true); });
	Once([=, this]()
	{
		Key(EKeys::Gamepad_RightThumbstick, false);
		TestTrue(TEXT("In-air toggle enters falling"), Movement->IsFalling());
	});
	ForFrames(90, [=, this](int32 Frame)
	{
		if (Frame == 89)
		{
			TestTrue(TEXT("Falling lands on fixture floor"), Movement->IsMovingOnGround());
		}
	});
	ForFrames(30, [=](int32) { Pawn->DoMove(0, 1); });
	Once([=, this]()
	{
		TestTrue(TEXT("Running resumes after landing"), Movement->IsMovingOnGround() && Movement->Velocity.Size2D() > 400);
	});

	for (int32 Cycle = 0; Cycle < 20; ++Cycle)
	{
		const FKey SprintKey = Cycle % 2 == 0 ? EKeys::LeftShift : EKeys::Gamepad_LeftThumbstick;
		const auto TurnStart = MakeShared<FVector>();
		const auto bObservedTakeoff = MakeShared<bool>(false);
		ForFrames(90, [=, this](int32 Frame)
		{
			if (Frame == 0)
			{
				Movement->StopMovementImmediately();
				Movement->SetMovementMode(MOVE_Walking);
				Pawn->SetActorLocationAndRotation(Origin + FVector(0, 0, 98), FRotator::ZeroRotator);
				PC->SetControlRotation(FRotator::ZeroRotator);
				Key(SprintKey, true);
			}
			Pawn->DoMove(0, 1);
			if (Frame == 89)
			{
				TestTrue(TEXT("Sprint reaches the requested 18–24 m/s band"),
					Movement->IsMovingOnGround() && Movement->Velocity.X >= 1780.0 && Movement->Velocity.X <= 2420.0);
				*TurnStart = Pawn->GetActorLocation();
				PC->SetControlRotation(FRotator(0, 60, 0));
			}
		});
		ForFrames(35, [=, this](int32 Frame)
		{
			Pawn->DoMove(0, 1);
			if (Frame == 34)
			{
				TestTrue(TEXT("Sprint changes heading without leaving the floor"), Movement->IsMovingOnGround()
					&& Pawn->GetActorLocation().Y > TurnStart->Y + 150.0);
				Key(SprintKey, false);
			}
		});
		ForFrames(50, [=, this](int32 Frame)
		{
			if (Frame == 0)
			{
				TestTrue(TEXT("Sprint release does not instantaneously stop the hero"), Movement->Velocity.Size2D() > 400.0);
			}
			if (Frame == 49)
			{
				TestTrue(TEXT("Ground braking brings sprint to rest"), Movement->Velocity.Size2D() < 100.0);
			}
		});
		ForFrames(32, [=](int32) { Pawn->DoMove(0, 1); });
		ForFrames(44, [=, this](int32 Frame)
		{
			if (Frame == 0)
			{
				TestTrue(TEXT("Running takeoff starts with horizontal momentum"), Movement->Velocity.Size2D() > 350.0);
				Key(EKeys::F, true);
			}
			if (Frame == 1) { Key(EKeys::F, false); }
			Pawn->DoMove(0, 1);
			if (!*bObservedTakeoff && Movement->IsFlying())
			{
				*bObservedTakeoff = true;
				TestTrue(TEXT("Running takeoff carries forward momentum into flight"), Movement->Velocity.Size2D() > 200.0);
			}
			if (Frame == 43)
			{
				TestTrue(TEXT("Running takeoff completes within the animation window"), *bObservedTakeoff);
			}
		});
	}
	const auto GroundSprintPeak = MakeShared<double>(0.0);
	const auto GroundSprintFurthestX = MakeShared<double>(0.0);
	ForFrames(130, [=, this](int32 Frame)
	{
		if (Frame == 0)
		{
			AddInfo(TEXT("Verified 20 sprint, turn, brake and running takeoff cycles"));
			Movement->StopMovementImmediately();
			Movement->SetMovementMode(MOVE_Walking);
			Pawn->SetActorLocationAndRotation(Origin + FVector(0, 0, 98), FRotator::ZeroRotator);
			PC->SetControlRotation(FRotator::ZeroRotator);
			*Wall = MakeBox(Origin + FVector(2300, 0, 200), FVector(2, 1300, 200));
			Key(EKeys::LeftShift, true);
		}
		Pawn->DoMove(0, 1);
		*GroundSprintPeak = FMath::Max(*GroundSprintPeak, Movement->Velocity.Size2D());
		*GroundSprintFurthestX = FMath::Max(*GroundSprintFurthestX, Pawn->GetActorLocation().X - Origin.X);
		if (Frame == 129)
		{
			TestTrue(TEXT("Ground wall test reaches sprint speed"), *GroundSprintPeak >= 1780.0);
			const double ContactX = 2298.0 - Pawn->GetCapsuleComponent()->GetScaledCapsuleRadius();
			TestTrue(TEXT("Sprint capsule reaches a thin building wall without crossing"),
				FMath::IsNearlyEqual(*GroundSprintFurthestX, ContactX, 5.0));
			Key(EKeys::LeftShift, false);
			(*Wall)->Destroy();
		}
	});
	ForFrames(300, [=, this](int32 Frame)
	{
		if (Frame == 0)
		{
			Movement->StopMovementImmediately();
			Pawn->SetActorLocation(Origin + FVector(0, 0, 400));
			PC->SetControlRotation(FRotator(0.0, 0.0, 0.0));
			Key(EKeys::F, true);
		}
		if (Frame == 1) { Key(EKeys::F, false); }
		if (Frame == 4 || Frame == 8 || Frame == 60) { Key(EKeys::E, true); }
		if (Frame == 5 || Frame == 9 || Frame == 61) { Key(EKeys::E, false); }
		if (Frame == 12) { Key(EKeys::Gamepad_DPad_Right, true); }
		if (Frame == 13) { Key(EKeys::Gamepad_DPad_Right, false); }
		if (Frame == 18) { Key(EKeys::Gamepad_LeftShoulder, true); }
		if (Frame == 19) { Key(EKeys::Gamepad_LeftShoulder, false); }
		if (Frame == 59)
		{
			TestEqual(TEXT("Speed-decrease cannot exit Travesia"), Pawn->GetFlightLevel(), 4);
			TestEqual(TEXT("Level four cap"), Movement->MaxFlySpeed, 18000.0f);
		}
		if (Frame == 63)
		{
			TestTrue(TEXT("Level five extends level four without jumping to maximum"),
				Movement->MaxFlySpeed >= 18000.0f && Movement->MaxFlySpeed < 20000.0f);
		}
		if (Frame == 299) { TestTrue(TEXT("Level five approaches its full speed"), Movement->MaxFlySpeed >= 31900.0f); }
	});
	for (int32 Cycle = 0; Cycle < 20; ++Cycle)
	{
		const FKey SprintKey = Cycle % 2 == 0 ? EKeys::LeftShift : EKeys::Gamepad_LeftThumbstick;
		const FKey JumpKey = Cycle % 2 == 0 ? EKeys::SpaceBar : EKeys::Gamepad_FaceButton_Bottom;
		const FKey FlightKey = Cycle % 4 < 2 ? EKeys::F : EKeys::Gamepad_RightThumbstick;
		const auto Start = MakeShared<FVector>();
		const auto Peak = MakeShared<double>(0.0);
		ForFrames(90, [=, this](int32 Frame)
		{
			if (Frame == 0)
			{
				Movement->StopMovementImmediately();
				Pawn->SetActorLocationAndRotation(Origin + FVector(0, 0, 98), FRotator::ZeroRotator);
				Movement->SetMovementMode(MOVE_Walking);
				PC->SetControlRotation(FRotator::ZeroRotator);
				Key(SprintKey, true);
			}
			Pawn->DoMove(0, 1);
			if (Frame == 89)
			{
				TestTrue(TEXT("Supersalto begins above the sprint-speed threshold"), Movement->Velocity.X > 1600.0);
				*Start = Pawn->GetActorLocation();
				Key(JumpKey, true);
			}
		});
		ForFrames(310, [=, this](int32 Frame)
		{
			if (Frame == 1) { Key(JumpKey, false); Key(SprintKey, false); }
			if (Frame == 5)
			{
				TestTrue(TEXT("Supersalto produces an upward impulse without erasing forward speed"),
					Movement->IsFalling() && Movement->Velocity.Z > 1200.0 && Movement->Velocity.X > 1300.0);
			}
			*Peak = FMath::Max(*Peak, Pawn->GetActorLocation().Z - Start->Z);
			if (Cycle % 2 == 0)
			{
				if (Frame == 114 || Frame == 120) { Key(FlightKey, true); }
				if (Frame == 115 || Frame == 121) { Key(FlightKey, false); }
				if (Frame == 119) { TestTrue(TEXT("Supersalto transitions to flight in the air"), Movement->IsFlying()); }
			}
			if (Frame == 309)
			{
				const double Distance = Pawn->GetActorLocation().X - Start->X;
				TestTrue(FString::Printf(TEXT("Supersalto reaches 12–18 m (measured %.2f m)"), *Peak / 100.0),
					*Peak >= 1200.0 && *Peak <= 1800.0);
				TestTrue(TEXT("Supersalto lands on the fixture"), Movement->IsMovingOnGround());
				// Neutral air input lets falling braking gradually spend the launch momentum.
				TestTrue(FString::Printf(TEXT("Supersalto carries forward with neutral input (measured %.2f m)"),
					Distance / 100.0), Distance > 900.0);
				if (Cycle < 2)
				{
					AddInfo(FString::Printf(TEXT("Supersalto cycle %d: apex %.2f m, forward %.2f m, movement mode %d"),
						Cycle + 1, *Peak / 100.0, Distance / 100.0, static_cast<int32>(Movement->MovementMode.GetValue())));
				}
			}
		});
	}
	Once([this]() { AddInfo(TEXT("Completed 20 supersaltos and landings, including 10 transitions into flight")); });
	Once([=]()
	{
		Pawn->SetActorLocationAndRotation(OriginalLocation, OriginalRotation);
		PC->SetControlRotation(OriginalView);
		Movement->SetMovementMode(OriginalMode);
		Movement->MaxFlySpeed = OriginalFlySpeed;
		Movement->BrakingDecelerationFlying = OriginalBrake;
		Movement->Velocity = OriginalVelocity;
		Floor->Destroy();
	});
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FImpactLandingTest, "ImpactBeyond.Landing.Headless",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FImpactLandingTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetCurrentPlayWorld();
	APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	AImpactBeyondCharacter* Pawn = PC ? Cast<AImpactBeyondCharacter>(PC->GetPawn()) : nullptr;
	UImpactLandingComponent* Landing = Pawn ? Pawn->FindComponentByClass<UImpactLandingComponent>() : nullptr;
	if (!TestNotNull(TEXT("Impact landing on the possessed pawn"), Landing))
	{
		return false;
	}

	const FVector Center(140000, 0, 5000);
	const FVector NearStart = Center + FVector(250, 0, 32);
	const FVector FarStart = Center + FVector(1150, 0, 32);
	const FVector UntaggedStart = Center + FVector(0, 250, 32);
	const auto MakeBox = [World](const FVector& Position, const FVector& Extent, bool bPhysical)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.ObjectFlags |= RF_Transient;
		AActor* Actor = World->SpawnActor<AActor>(SpawnParameters);
		UBoxComponent* Box = NewObject<UBoxComponent>(Actor);
		Actor->SetRootComponent(Box);
		Box->InitBoxExtent(Extent);
		Box->SetCollisionProfileName(bPhysical ? TEXT("PhysicsActor") : TEXT("BlockAll"));
		Actor->SetActorLocation(Position);
		Box->RegisterComponent();
		Box->SetSimulatePhysics(bPhysical);
		return Box;
	};
	UBoxComponent* Floor = MakeBox(Center - FVector(0, 0, 20), FVector(4500, 4500, 20), false);
	UBoxComponent* Near = MakeBox(NearStart, FVector(30), true);
	UBoxComponent* Far = MakeBox(FarStart, FVector(30), true);
	UBoxComponent* Untagged = MakeBox(UntaggedStart, FVector(30), true);
	Near->ComponentTags.Add(TEXT("ImpactLandingTarget"));
	Far->ComponentTags.Add(TEXT("ImpactLandingTarget"));
	auto* Movement = Pawn->GetCharacterMovement();
	const FTransform OriginalTransform = Pawn->GetActorTransform();
	const EMovementMode OriginalMode = Movement->MovementMode;
	const FVector OriginalVelocity = Movement->Velocity;
	const double LandingHeight = Center.Z + Pawn->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	const auto Key = [PC](FKey Input, bool bHeld)
	{
		PC->InputKey(FInputKeyEventArgs::CreateSimulated(Input, bHeld ? IE_Pressed : IE_Released, bHeld ? 1.f : 0.f));
	};
	const auto ResetBox = [](UBoxComponent* Box, const FVector& Position)
	{
		Box->SetWorldLocationAndRotation(Position, FRotator::ZeroRotator, false, nullptr, ETeleportType::TeleportPhysics);
		Box->SetPhysicsLinearVelocity(FVector::ZeroVector);
		Box->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	};

	const auto LastFrame = MakeShared<uint64>(MAX_uint64);
	for (int32 Cycle = 0; Cycle < 10; ++Cycle)
	{
		const FKey ImpactKey = Cycle % 2 == 0 ? EKeys::V : EKeys::Gamepad_DPad_Up;
		FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShared<FFlightStep>(
			[=, this, Frame = 0, FastestProp = 0.0]() mutable
			{
				if (*LastFrame == GFrameCounter)
				{
					return false;
				}
				*LastFrame = GFrameCounter;
				switch (Frame)
				{
				case 0:
					Movement->StopMovementImmediately();
					Pawn->SetActorLocation(Center + FVector(0, 0, 550), false, nullptr, ETeleportType::TeleportPhysics);
					Movement->SetMovementMode(Cycle % 2 == 0 ? MOVE_Flying : MOVE_Falling);
					ResetBox(Near, NearStart);
					ResetBox(Far, FarStart);
					ResetBox(Untagged, UntaggedStart);
					TestTrue(TEXT("Impact is eligible from flight or free fall"), Landing->CanStartImpact());
					Key(ImpactKey, true);
					break;
				case 3:
					Key(ImpactKey, false);
					TestTrue(TEXT("Impact enters a fast physical dive in Falling mode"), Landing->IsImpacting()
						&& Movement->IsFalling() && Movement->Velocity.Z <= -1800.0);
					break;
				case 40:
					TestTrue(TEXT("Impact lands on the blocking floor"), Movement->IsMovingOnGround()
						&& FMath::IsNearlyEqual(Pawn->GetActorLocation().Z, LandingHeight, 6.0));
					TestTrue(FString::Printf(TEXT("Impact %d prop launches outward at %.0f cm/s"), Cycle + 1, FastestProp),
						FastestProp > 200.0 && Near->GetComponentLocation().X > NearStart.X + 60.0);
					TestTrue(TEXT("Impact ignores tagged bodies outside the radius"),
						FVector::Dist2D(Far->GetComponentLocation(), FarStart) < 10.0);
					TestTrue(TEXT("Impact ignores nearby bodies without the target tag"),
						FVector::Dist2D(Untagged->GetComponentLocation(), UntaggedStart) < 10.0);
					TestFalse(TEXT("Impact cannot restart on the floor"), Landing->CanStartImpact());
					break;
				}
				FastestProp = FMath::Max(FastestProp, Near->GetPhysicsLinearVelocity().X);
				if (++Frame < 115)
				{
					return false;
				}
				if (Cycle == 9)
				{
					TestFalse(TEXT("Impact has recovered after the last cycle"), Landing->IsImpacting());
					Pawn->SetActorTransform(OriginalTransform, false, nullptr, ETeleportType::TeleportPhysics);
					Movement->SetMovementMode(OriginalMode);
					Movement->Velocity = OriginalVelocity;
					for (UBoxComponent* Box : {Floor, Near, Far, Untagged})
					{
						Box->GetOwner()->Destroy();
					}
					AddInfo(TEXT("Completed 10 impact landings from five flights and five free falls; keyboard and gamepad bindings exercised"));
				}
				return true;
			}));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FImpactCircuitTest, "ImpactBeyond.Circuit.Headless",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FImpactCircuitTest::RunTest(const FString& Parameters)
{
	UWorld* World = nullptr;
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::Game)
		{
			World = Context.World();
			break;
		}
	}
	APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	ACharacter* Pawn = PC ? Cast<ACharacter>(PC->GetPawn()) : nullptr;
	UImpactCircuitComponent* Circuit = Pawn ? Pawn->FindComponentByClass<UImpactCircuitComponent>() : nullptr;
	if (!TestNotNull(TEXT("Circuit on the possessed character"), Circuit))
	{
		return false;
	}
	UStaticMeshComponent* Car = nullptr;
	UStaticMeshComponent* Panel = nullptr;
	for (TActorIterator<AStaticMeshActor> Actor(World); Actor; ++Actor)
	{
		if (Actor->ActorHasTag(TEXT("ImpactCircuitCar"))) { Car = Actor->GetStaticMeshComponent(); }
		if (Actor->ActorHasTag(TEXT("ImpactCircuitPanel"))) { Panel = Actor->GetStaticMeshComponent(); }
	}
	if (!TestNotNull(TEXT("Tagged car"), Car) || !TestNotNull(TEXT("Tagged panel"), Panel))
	{
		return false;
	}
	const FVector InitialCarLocation = Car->GetComponentLocation();
	Pawn->SetActorLocationAndRotation(FVector(InitialCarLocation.X, InitialCarLocation.Y - 320.0, 98.0),
		FRotator(0.0, 90.0, 0.0), false, nullptr, ETeleportType::TeleportPhysics);
	PC->SetControlRotation(FRotator(0.0, 90.0, 0.0));
	Circuit->ToggleGrab();
	TestFalse(TEXT("Grab takes the car out of rigid simulation"), Car->IsSimulatingPhysics());
	Circuit->ToggleGrab();
	TestTrue(TEXT("Drop restores rigid simulation"), Car->IsSimulatingPhysics());
	Circuit->ToggleGrab();
	ADD_LATENT_AUTOMATION_COMMAND(FFlightStep([this, Circuit, Car, Panel, World, Frame = 0]() mutable
	{
		++Frame;
		if (Frame == 50)
		{
			TestTrue(TEXT("Held car lifted above street"), Car->GetComponentLocation().Z > 150.0);
			TestTrue(TEXT("Intact panel blocks collision"), Panel->IsCollisionEnabled());
			TestEqual(TEXT("Canonical target starts at full health"), Circuit->GetCanonicalTargetHealth(), 100.0f);
			TestEqual(TEXT("No canonical hit before physical contact"), Circuit->GetCanonicalAcceptedHits(), 0);
			FVector LeftGrip, RightGrip;
			TestTrue(TEXT("Lifting exposes two chassis contact targets"), Circuit->GetGripTargets(LeftGrip, RightGrip));
			Circuit->ThrowCar();
		}
		if (Frame == 75)
		{
			TestTrue(TEXT("Thrown car simulates physics"), Car->IsSimulatingPhysics());
		}
		if (Frame < 200) { return false; }
		TestFalse(TEXT("Car impact disables panel collision"), Panel->IsCollisionEnabled());
		TestEqual(TEXT("Physical throw reaches the sole S4 receiver once"), Circuit->GetCanonicalAcceptedHits(), 1);
		TestEqual(TEXT("Canonical S4 mutation precedes panel opening"), Circuit->GetCanonicalTargetHealth(), 0.0f);
		const FVector Center = Panel->GetComponentLocation();
		FCollisionQueryParams Query(SCENE_QUERY_STAT(ImpactCircuitOpening), false, Car->GetOwner());
		TestFalse(TEXT("Capsule can cross the opening"), World->SweepTestByChannel(
			FVector(Center.X, Center.Y - 250.0, 110.0), FVector(Center.X, Center.Y + 250.0, 110.0),
			FQuat::Identity, ECC_Pawn, FCollisionShape::MakeCapsule(42.0, 96.0), Query));
		Circuit->ResetCircuit();
		return true;
	}));
	ADD_LATENT_AUTOMATION_COMMAND(FFlightStep([this, PreviousWorld = TWeakObjectPtr<UWorld>(World), InitialCarLocation, Frames = 0]() mutable
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			UWorld* NewWorld = Context.World();
			if (Context.WorldType != EWorldType::Game || !NewWorld || NewWorld == PreviousWorld.Get() || !NewWorld->HasBegunPlay())
			{
				continue;
			}
			int32 CarCount = 0;
			int32 PanelCount = 0;
			for (TActorIterator<AStaticMeshActor> Actor(NewWorld); Actor; ++Actor)
			{
				if (Actor->ActorHasTag(TEXT("ImpactCircuitCar")))
				{
					++CarCount;
					TestTrue(TEXT("Reset puts car back at spawn"), FVector::Dist(Actor->GetActorLocation(), InitialCarLocation) < 15.0);
				}
				if (Actor->ActorHasTag(TEXT("ImpactCircuitPanel")))
				{
					++PanelCount;
					TestTrue(TEXT("Reset restores panel collision and visibility"), Actor->GetStaticMeshComponent()->IsCollisionEnabled()
						&& Actor->GetStaticMeshComponent()->IsVisible());
				}
			}
			TestEqual(TEXT("Reset leaves exactly one car"), CarCount, 1);
			TestEqual(TEXT("Reset leaves exactly one panel"), PanelCount, 1);
			return true;
		}
		if (++Frames < 240) { return false; }
		AddError(TEXT("Circuit did not finish reloading after reset"));
		return true;
	}));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FImpactCityVehiclesTest, "ImpactBeyond.Circuit.CityVehicles",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FImpactCityVehiclesTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetCurrentPlayWorld();
	APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	AImpactBeyondCharacter* Hero = PC ? Cast<AImpactBeyondCharacter>(PC->GetPawn()) : nullptr;
	UImpactCircuitComponent* Circuit = Hero ? Hero->FindComponentByClass<UImpactCircuitComponent>() : nullptr;
	if (!TestNotNull(TEXT("Vehicle interaction component"), Circuit))
	{
		return false;
	}

	TArray<UStaticMeshComponent*> Vehicles;
	for (TActorIterator<AStaticMeshActor> Actor(World); Actor; ++Actor)
	{
		if (Actor->ActorHasTag(TEXT("ImpactCircuitCar")) || Actor->ActorHasTag(TEXT("ImpactSprintVehicle")))
		{
			Vehicles.Add(Actor->GetStaticMeshComponent());
		}
	}
	if (!TestTrue(TEXT("At least six physical city vehicles"), Vehicles.Num() >= 6))
	{
		return false;
	}

	UCharacterMovementComponent* Movement = Hero->GetCharacterMovement();
	const FTransform OriginalHero = Hero->GetActorTransform();
	const FRotator OriginalView = PC->GetControlRotation();
	const EMovementMode OriginalMode = Movement->MovementMode;
	for (int32 Index = 0; Index < Vehicles.Num(); ++Index)
	{
		UStaticMeshComponent* Vehicle = Vehicles[Index];
		const FTransform OriginalVehicle = Vehicle->GetComponentTransform();
		const FKey GrabKey = Index % 2 == 0 ? EKeys::G : EKeys::Gamepad_FaceButton_Right;
		const FKey ThrowKey = Index % 2 == 0 ? EKeys::T : EKeys::Gamepad_FaceButton_Top;
		ADD_LATENT_AUTOMATION_COMMAND(FFlightStep([=, this, Frame = 0]() mutable
		{
			const auto Key = [PC](FKey Input, bool bHeld)
			{
				PC->InputKey(FInputKeyEventArgs::CreateSimulated(Input, bHeld ? IE_Pressed : IE_Released, bHeld ? 1.f : 0.f));
			};
			const auto Approach = [=]()
			{
				const FVector Center = Vehicle->GetComponentLocation();
				const double CenterHeight = Vehicle->Bounds.Origin.Z - Vehicle->Bounds.BoxExtent.Z
					+ Hero->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 2.0;
				Movement->StopMovementImmediately();
				Hero->SetActorLocationAndRotation(FVector(Center.X, Center.Y - 320.0, CenterHeight),
					FRotator(0, 90, 0), false, nullptr, ETeleportType::TeleportPhysics);
				Movement->SetMovementMode(MOVE_Walking);
				PC->SetControlRotation(FRotator(0, 90, 0));
			};
			switch (Frame++)
			{
			case 0:
				TestTrue(FString::Printf(TEXT("Vehicle %d starts with physics enabled and asleep"), Index + 1),
					Vehicle->IsSimulatingPhysics() && !Vehicle->RigidBodyIsAwake());
				Approach();
				Key(GrabKey, true);
				break;
			case 1: Key(GrabKey, false); break;
			case 55:
				TestTrue(FString::Printf(TEXT("Vehicle %d is physically carried above H01"), Index + 1),
					Circuit->IsCarHeld() && !Vehicle->IsSimulatingPhysics()
					&& Vehicle->GetComponentLocation().Z > Hero->GetActorLocation().Z + 100.0);
				Key(GrabKey, true);
				break;
			case 56: Key(GrabKey, false); break;
			case 80:
				TestTrue(FString::Printf(TEXT("Vehicle %d restores free physics on drop"), Index + 1),
					Vehicle->IsSimulatingPhysics() && !Circuit->IsCarHeld());
				break;
			case 110: Approach(); Key(GrabKey, true); break;
			case 111: Key(GrabKey, false); break;
			case 170:
				PC->SetControlRotation(FRotator(55, 90, 0));
				Key(ThrowKey, true);
				break;
			case 220:
				TestTrue(TEXT("Charge keeps the selected vehicle in both hands"),
					Circuit->IsChargingThrow() && !Vehicle->IsSimulatingPhysics());
				break;
			case 230: Key(ThrowKey, false); break;
			case 255:
				TestTrue(FString::Printf(TEXT("Vehicle %d restores simulation and launches along the camera aim"), Index + 1),
					Vehicle->IsSimulatingPhysics() && Vehicle->GetPhysicsLinearVelocity().Z > 3000.0);
				Vehicle->SetWorldTransform(OriginalVehicle, false, nullptr, ETeleportType::TeleportPhysics);
				Vehicle->SetPhysicsLinearVelocity(FVector::ZeroVector);
				Vehicle->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
				Vehicle->PutAllRigidBodiesToSleep();
				if (Index == Vehicles.Num() - 1)
				{
					Hero->SetActorTransform(OriginalHero, false, nullptr, ETeleportType::TeleportPhysics);
					Movement->SetMovementMode(OriginalMode);
					PC->SetControlRotation(OriginalView);
					AddInfo(FString::Printf(TEXT("Verified %d city vehicles start asleep and can be lifted, dropped and launched"), Vehicles.Num()));
				}
				return true;
			}
			return false;
		}));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FImpactTransportCyclesTest, "ImpactBeyond.Circuit.TransportCycles",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FImpactTransportCyclesTest::RunTest(const FString& Parameters)
{
	struct FState
	{
		TWeakObjectPtr<UWorld> LastWorld;
		TWeakObjectPtr<AImpactBeyondCharacter> Hero;
		TWeakObjectPtr<UImpactCircuitComponent> Circuit;
		TWeakObjectPtr<UStaticMeshComponent> Car;
		TWeakObjectPtr<UStaticMeshComponent> Panel;
		FVector InitialCarLocation = FVector::ZeroVector;
		FVector AirStart = FVector::ZeroVector;
		FVector CargoAirStart = FVector::ZeroVector;
		double PeakThrowSpeed = 0.0;
		double FurthestThrowY = 0.0;
		double LeadingWallFace = 0.0;
		double FurthestCargoNose = 0.0;
		FVector PreviousCargoPosition = FVector::ZeroVector;
		double PeakCargoApproachSpeed = 0.0;
		int32 Completed = 0;
		int32 Frame = 0;
		int32 ReloadFrames = 0;
		bool bAwaitingReload = true;
	};
	const auto State = MakeShared<FState>();
	ADD_LATENT_AUTOMATION_COMMAND(FFlightStep([this, State]()
	{
		if (State->bAwaitingReload)
		{
			UWorld* World = nullptr;
			for (const FWorldContext& Context : GEngine->GetWorldContexts())
			{
				UWorld* Candidate = Context.World();
				if (Context.WorldType == EWorldType::Game && Candidate && Candidate->HasBegunPlay()
					&& Candidate != State->LastWorld.Get())
				{
					World = Candidate;
					break;
				}
			}
			APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
			AImpactBeyondCharacter* Hero = PC ? Cast<AImpactBeyondCharacter>(PC->GetPawn()) : nullptr;
			if (!Hero)
			{
				if (++State->ReloadFrames < 300) { return false; }
				AddError(TEXT("No possessed hero appeared after circuit reload"));
				return true;
			}

			UStaticMeshComponent* Car = nullptr;
			UStaticMeshComponent* Panel = nullptr;
			int32 CarCount = 0;
			int32 PanelCount = 0;
			for (TActorIterator<AStaticMeshActor> Actor(World); Actor; ++Actor)
			{
				if (Actor->ActorHasTag(TEXT("ImpactCircuitCar")))
				{
					Car = Actor->GetStaticMeshComponent();
					++CarCount;
				}
				if (Actor->ActorHasTag(TEXT("ImpactCircuitPanel")))
				{
					Panel = Actor->GetStaticMeshComponent();
					++PanelCount;
				}
			}
			UImpactCircuitComponent* Circuit = Hero->FindComponentByClass<UImpactCircuitComponent>();
			if (!TestNotNull(TEXT("Circuit on the freshly possessed hero"), Circuit)
				|| !TestNotNull(TEXT("Tagged car in the reset world"), Car)
				|| !TestNotNull(TEXT("Tagged panel in the reset world"), Panel))
			{
				return true;
			}
			TestEqual(TEXT("Exactly one car after reload"), CarCount, 1);
			TestEqual(TEXT("Exactly one panel after reload"), PanelCount, 1);
			TestTrue(TEXT("Reset restores car physics and panel collision"),
				Car->IsSimulatingPhysics() && Panel->IsCollisionEnabled() && Panel->IsVisible());
			if (State->Frame > 0)
			{
				TestTrue(TEXT("Reset restores the car near its initial location"),
					FVector::Dist(Car->GetComponentLocation(), State->InitialCarLocation) < 20.0);
				++State->Completed;
				if (State->Completed == 20)
				{
					AddInfo(TEXT("Completed 20 pickup, airborne transport, throw, passage and full map reload cycles"));
				}
				if (State->Completed == 50)
				{
					AddInfo(TEXT("Completed 10 low, 10 medium and 10 high charged throws"));
				}
				if (State->Completed == 51)
				{
					TestTrue(TEXT("Reset clears the pending throw charge"),
						!Circuit->IsChargingThrow() && FMath::IsNearlyZero(Circuit->GetThrowCharge()));
					AddInfo(TEXT("Verified that a reset cancels an active charge"));
				}
				if (State->Completed == 56)
				{
					return true;
				}
			}
			else
			{
				State->InitialCarLocation = Car->GetComponentLocation();
			}

			State->Car = Car;
			State->Panel = Panel;
			State->Hero = Hero;
			State->Circuit = Circuit;
			State->LastWorld = World;
			State->Frame = 0;
			State->ReloadFrames = 0;
			State->PeakThrowSpeed = 0.0;
			State->FurthestThrowY = 0.0;
			State->bAwaitingReload = false;
			if (State->Completed >= 20)
			{
				// A short approach makes even a tap-strength throw contact the panel.
				const FVector PanelCenter = Panel->GetComponentLocation();
				Car->SetPhysicsLinearVelocity(FVector::ZeroVector);
				Car->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
				Car->SetWorldLocation(FVector(PanelCenter.X, PanelCenter.Y - 1000.0, State->InitialCarLocation.Z),
					false, nullptr, ETeleportType::TeleportPhysics);
			}
			const FVector CarLocation = Car->GetComponentLocation();
			Hero->SetActorLocationAndRotation(FVector(CarLocation.X, CarLocation.Y - 320.0, 98.0),
				FRotator(0.0, 90.0, 0.0), false, nullptr, ETeleportType::TeleportPhysics);
			PC->SetControlRotation(FRotator(0.0, 90.0, 0.0));
			Circuit->ToggleGrab();
			return false;
		}

		AImpactBeyondCharacter* Hero = State->Hero.Get();
		UImpactCircuitComponent* Circuit = State->Circuit.Get();
		UStaticMeshComponent* Car = State->Car.Get();
		UStaticMeshComponent* Panel = State->Panel.Get();
		APlayerController* PC = Hero ? Cast<APlayerController>(Hero->GetController()) : nullptr;
		if (!PC || !Circuit || !Car || !Panel)
		{
			AddError(TEXT("Lost hero or car before requesting reset"));
			return true;
		}
		const int32 Frame = ++State->Frame;
		const int32 Level = State->Completed % 3;
		auto Key = [PC](FKey Input, bool bHeld)
		{
			PC->InputKey(FInputKeyEventArgs::CreateSimulated(Input, bHeld ? IE_Pressed : IE_Released, bHeld ? 1.f : 0.f));
		};
		if (State->Completed >= 52)
		{
			// One explicit placement and velocity seed per case, then real CMC and cargo ticks.
			// This checks a thin cargo-only obstruction, not a continuous acceleration journey.
			auto* Movement = Hero->GetCharacterMovement();
			const bool bTurbo = State->Completed % 2 != 0;
			const double Speed = bTurbo ? 36800.0 : 32000.0;
			const double Lateral = State->Completed >= 54 ? 0.12 : 0.0;
			const FVector Direction = FVector(-Lateral, 1.0, 0.0).GetSafeNormal();
			if (Frame == 50)
			{
				TestFalse(TEXT("High-speed fixture starts with the car held"), Car->IsSimulatingPhysics());
				const FVector Offset = Car->GetComponentLocation() - Hero->GetActorLocation();
				Hero->SetActorLocation(FVector(1000000, 0, 10000), false, nullptr, ETeleportType::TeleportPhysics);
				Car->SetWorldLocation(Hero->GetActorLocation() + Offset, false, nullptr, ETeleportType::TeleportPhysics);
				Movement->SetMovementMode(MOVE_Flying);
				Movement->StopMovementImmediately();
				Key(EKeys::B, true);
			}
			if (Frame == 51) { Key(EKeys::B, false); Key(EKeys::E, true); }
			if (Frame == 52) { Key(EKeys::E, false); if (bTurbo) { Key(EKeys::LeftShift, true); } }
			if (Frame == 400)
			{
				TestEqual(TEXT("High-speed fixture selects traversal level five"), Hero->GetFlightLevel(), 5);
				TestFalse(TEXT("Car remains held before high-speed approach"), Car->IsSimulatingPhysics());
				AActor* Wall = Hero->GetWorld()->SpawnActor<AActor>();
				if (!TestNotNull(TEXT("High-speed cargo-only wall"), Wall)) { return true; }
				UBoxComponent* Box = NewObject<UBoxComponent>(Wall);
				Wall->SetRootComponent(Box);
				Box->InitBoxExtent(FVector(10000, 4, 80));
				Box->SetCollisionProfileName(TEXT("BlockAll"));
				State->LeadingWallFace = Car->Bounds.Origin.Y + Car->Bounds.BoxExtent.Y + 5000;
				Wall->SetActorLocation(FVector(Hero->GetActorLocation().X, State->LeadingWallFace + 4, 10187));
				Box->RegisterComponent();
				State->AirStart = Hero->GetActorLocation();
				State->PreviousCargoPosition = Car->GetComponentLocation();
				State->PeakCargoApproachSpeed = 0;
				State->FurthestCargoNose = Car->Bounds.Origin.Y + Car->Bounds.BoxExtent.Y;
				Movement->Velocity = Direction * Speed;
			}
			if (Frame >= 400 && Frame < 430)
			{
				Hero->DoMove(Lateral, 1);
				const double Nose = Car->Bounds.Origin.Y + Car->Bounds.BoxExtent.Y;
				if (Frame > 400 && Nose < State->LeadingWallFace - 1000 && !Car->IsSimulatingPhysics())
				{
					State->PeakCargoApproachSpeed = FMath::Max(State->PeakCargoApproachSpeed,
						FVector::Dist(Car->GetComponentLocation(), State->PreviousCargoPosition) / Hero->GetWorld()->GetDeltaSeconds());
				}
				State->PreviousCargoPosition = Car->GetComponentLocation();
				State->FurthestCargoNose = FMath::Max(State->FurthestCargoNose, Nose);
			}
			if (Frame == 430)
			{
				AddInfo(FString::Printf(TEXT("Cargo sweep %.0f cm/s lateral %.2f: measured %.1f cm/s, nose %.1f, wall %.1f"),
					Speed, Lateral, State->PeakCargoApproachSpeed, State->FurthestCargoNose, State->LeadingWallFace));
				TestTrue(TEXT("Car actually approaches at traversal or turbo speed"), State->PeakCargoApproachSpeed >= Speed * 0.98);
				TestTrue(TEXT("High-speed cargo reaches thin wall without tunneling"), FMath::IsNearlyEqual(State->FurthestCargoNose, State->LeadingWallFace, 8.0));
				TestTrue(TEXT("Hero clears the cargo-only wall"), Hero->GetActorLocation().Y > State->LeadingWallFace + 100);
				TestTrue(TEXT("Obstructed cargo is released safely"), Car->IsSimulatingPhysics());
				if (bTurbo) { Key(EKeys::LeftShift, false); }
				State->bAwaitingReload = true;
				Circuit->ResetCircuit();
			}
			return false;
		}
		if (State->Completed == 51)
		{
			if (Frame == 50)
			{
				// An overhang intersects the carried box but clears the hero's capsule.
				AActor* Wall = Hero->GetWorld()->SpawnActor<AActor>();
				if (!TestNotNull(TEXT("Cargo overhang fixture"), Wall)) { return true; }
				UBoxComponent* Box = NewObject<UBoxComponent>(Wall);
				Wall->SetRootComponent(Box);
				Box->InitBoxExtent(FVector(400.0, 4.0, 80.0));
				Box->SetCollisionProfileName(TEXT("BlockAll"));
				State->LeadingWallFace = Car->Bounds.Origin.Y + Car->Bounds.BoxExtent.Y + 80.0;
				Wall->SetActorLocation(FVector(Car->Bounds.Origin.X, State->LeadingWallFace + 4.0, 285.0));
				Box->RegisterComponent();
				State->AirStart = Hero->GetActorLocation();
			}
			if (Frame > 50 && Frame < 100)
			{
				Hero->DoMove(0, 1);
				if (!Car->IsSimulatingPhysics())
				{
					State->FurthestCargoNose = FMath::Max(State->FurthestCargoNose,
						Car->Bounds.Origin.Y + Car->Bounds.BoxExtent.Y);
				}
			}
			if (Frame == 100)
			{
				TestTrue(TEXT("Hero capsule clears the cargo-only obstruction"), Hero->GetActorLocation().Y > State->AirStart.Y + 125.0);
				TestTrue(TEXT("Independent cargo sweep reaches the overhang without crossing"),
					FMath::IsNearlyEqual(State->FurthestCargoNose, State->LeadingWallFace, 8.0));
				State->bAwaitingReload = true;
				Circuit->ResetCircuit();
			}
			return false;
		}
		if (State->Completed >= 20)
		{
			const FKey ThrowKey = State->Completed % 2 == 0 ? EKeys::T : EKeys::Gamepad_FaceButton_Top;
			const int32 HoldFrames[] = {3, 27, 60};
			const int32 ReleaseFrame = 50 + HoldFrames[Level];
			if (Frame == 50)
			{
				TestFalse(TEXT("Charged throw begins with the car held"), Car->IsSimulatingPhysics());
				Key(ThrowKey, true);
			}
			if (State->Completed == 50)
			{
				if (Frame == 55)
				{
					TestTrue(TEXT("An active charge does not release the car"),
						Circuit->IsChargingThrow() && !Car->IsSimulatingPhysics());
					State->bAwaitingReload = true;
					Circuit->ResetCircuit();
				}
				return false;
			}
			if (Frame == ReleaseFrame)
			{
				TestFalse(TEXT("Holding the throw action keeps the car held"), Car->IsSimulatingPhysics());
				Key(ThrowKey, false);
			}
			if (Frame > ReleaseFrame && Car->IsSimulatingPhysics())
			{
				State->PeakThrowSpeed = FMath::Max(State->PeakThrowSpeed, Car->GetPhysicsLinearVelocity().Y);
				State->FurthestThrowY = FMath::Max(State->FurthestThrowY, Car->GetComponentLocation().Y);
			}
			if (Frame == ReleaseFrame + 105)
			{
				const double MinimumSpeeds[] = {1050.0, 3000.0, 5600.0};
				const double MaximumSpeeds[] = {1980.0, 5250.0, 7250.0};
				AddInfo(FString::Printf(TEXT("Charge tier %d measured peak forward speed %.1f cm/s"), Level + 1, State->PeakThrowSpeed));
				TestTrue(TEXT("Measured charge speed falls in the expected band"),
					State->PeakThrowSpeed >= MinimumSpeeds[Level] && State->PeakThrowSpeed <= MaximumSpeeds[Level]);
				if (Level == 0)
				{
					const double PanelGap = Panel->GetComponentLocation().Y - State->FurthestThrowY;
					AddInfo(FString::Printf(TEXT("Low throw reached within %.1f cm of panel center; collision enabled: %s"),
						PanelGap, Panel->IsCollisionEnabled() ? TEXT("yes") : TEXT("no")));
					TestTrue(TEXT("Low throw reaches the panel"), PanelGap <= 300.0);
					TestTrue(TEXT("Low throw leaves the panel intact"), Panel->IsCollisionEnabled());
					TestEqual(TEXT("Low-energy contact does not mutate canonical health"), Circuit->GetCanonicalTargetHealth(), 100.0f);
					TestEqual(TEXT("Low-energy contact dispatches no canonical hit"), Circuit->GetCanonicalAcceptedHits(), 0);
				}
				if (Level == 2)
				{
					TestFalse(TEXT("High charged throw opens the panel"), Panel->IsCollisionEnabled());
					TestEqual(TEXT("Charged throw dispatches one canonical hit"), Circuit->GetCanonicalAcceptedHits(), 1);
					TestEqual(TEXT("Charged throw has one authoritative health state"), Circuit->GetCanonicalTargetHealth(), 0.0f);
				}
				State->bAwaitingReload = true;
				Circuit->ResetCircuit();
			}
			return false;
		}
		if (Frame == 50)
		{
			TestFalse(TEXT("Car remains held after lift"), Car->IsSimulatingPhysics());
			TestTrue(TEXT("Lift raises the car above the hero"), Car->GetComponentLocation().Z > Hero->GetActorLocation().Z + 100.0);
			Key(EKeys::F, true);
		}
		if (Frame == 51) { Key(EKeys::F, false); }
		if (Frame == 107)
		{
			State->AirStart = Hero->GetActorLocation();
			State->CargoAirStart = Car->GetComponentLocation();
		}
		// Use the first flight level for this street-length transport and throw.
		constexpr int32 BrakeFrame = 202;
		if (Frame >= 107 && Frame < BrakeFrame) { Hero->DoMove(0, 1); }
		if (Frame == BrakeFrame) { Key(EKeys::X, true); }
		if (Frame == 202)
		{
			const auto* Movement = Hero->GetCharacterMovement();
			TestTrue(TEXT("Hero flies in the first level with the car"), Movement->IsFlying()
				&& Hero->GetFlightLevel() == 1 && FMath::IsNearlyEqual(Movement->MaxFlySpeed, 1200.0f, 1.0f));
			TestTrue(TEXT("Hero and held car both moved down the street"), Hero->GetActorLocation().Y > State->AirStart.Y + 175.0
				&& Car->GetComponentLocation().Y > State->CargoAirStart.Y + 175.0);
			FVector LeftGrip, RightGrip;
			TestTrue(TEXT("Transport retains two finite, distinct grip targets"), Circuit->GetGripTargets(LeftGrip, RightGrip)
				&& !LeftGrip.ContainsNaN() && !RightGrip.ContainsNaN() && FVector::DistSquared(LeftGrip, RightGrip) > 400.0);
			TestFalse(TEXT("Car remains held during airborne movement"), Car->IsSimulatingPhysics());
		}
		if (Frame == 254)
		{
			Key(EKeys::X, false);
			Circuit->ThrowCar();
		}
		if (Frame == 279)
		{
			TestTrue(TEXT("Throw after braking restores physical simulation"), Car->IsSimulatingPhysics());
		}
		const FVector PanelCenter = Panel->GetComponentLocation();
		if (Frame == 350)
		{
			TestFalse(TEXT("Airborne throw opens the panel"), Panel->IsCollisionEnabled());
			TestEqual(TEXT("Airborne throw dispatches one canonical hit"), Circuit->GetCanonicalAcceptedHits(), 1);
			TestEqual(TEXT("Airborne reaction follows canonical health loss"), Circuit->GetCanonicalTargetHealth(), 0.0f);
			Hero->GetCharacterMovement()->StopMovementImmediately();
			Hero->SetActorLocation(FVector(PanelCenter.X, PanelCenter.Y - 300.0, 115.0), false, nullptr, ETeleportType::TeleportPhysics);
		}
		if (Frame > 350 && Frame < 435) { Hero->DoMove(0, 1); }
		if (Frame == 435)
		{
			TestTrue(TEXT("Hero travelled through the opened panel"), Hero->GetActorLocation().Y > PanelCenter.Y + 125.0);
			State->bAwaitingReload = true;
			Circuit->ResetCircuit();
		}
		return false;
	}));
	return true;
}

#endif
