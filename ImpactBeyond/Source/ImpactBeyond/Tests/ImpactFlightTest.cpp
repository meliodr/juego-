#include "ImpactBeyondCharacter.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "ImpactCircuitComponent.h"
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
	AActor* Floor = MakeBox(Origin - FVector(0, 0, 20), FVector(24000, 8000, 20));
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
		ForFrames(240, [=, this](int32 Frame)
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
			if (Frame == 179) *CruiseHeight = Pawn->GetActorLocation().Z;
			if (Frame == 239)
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
	SpeedStage(FKey(), 900);
	SpeedStage(EKeys::E, 1800);
	SpeedStage(EKeys::Gamepad_RightShoulder, 4400);

	const auto CoastSpeed = MakeShared<double>(0);
	ForFrames(21, [=](int32 Frame)
	{
		if (Frame == 20) *CoastSpeed = Movement->Velocity.Size();
	});
	ForFrames(75, [=](int32) { Pawn->DoMove(0, 1); });
	ForFrames(60, [=, this](int32 Frame)
	{
		if (Frame == 0)
		{
			TestTrue(TEXT("Brake comparison starts near top speed"), Movement->Velocity.Size() >= 3170);
			Key(EKeys::X, true);
		}
		Pawn->DoMove(0, 1);
		if (Frame == 20)
		{
			const double BrakeSpeed = Movement->Velocity.Size();
			AddInfo(FString::Printf(TEXT("After 20 frames: coast %.1f, brake %.1f cm/s"), *CoastSpeed, BrakeSpeed));
			TestTrue(TEXT("Brake beats coasting with forward held"), BrakeSpeed + 200 < *CoastSpeed);
		}
		if (Frame == 59)
		{
			TestTrue(TEXT("Brake reaches hover"), Movement->IsFlying() && Movement->Velocity.Size() < 5);
			Key(EKeys::X, false);
		}
	});

	Once([=]() { *Wall = MakeBox(Origin + FVector(12000, 0, 800), FVector(2, 2400, 800)); });

	// Start each sweep with a straight approach long enough to reach the third level.
	for (float Lateral : {0.f, 0.12f})
	{
		Once([=]()
		{
			Movement->StopMovementImmediately();
			Pawn->SetActorLocation(Origin + FVector(6500, Lateral > 0 ? 100 : 0, 500));
		});
		const auto Peak = MakeShared<double>(0);
		const auto FurthestX = MakeShared<double>(0);
		ForFrames(200, [=, this](int32 Frame)
		{
			Pawn->DoMove(Lateral, 1);
			*Peak = FMath::Max(*Peak, Movement->Velocity.Size());
			*FurthestX = FMath::Max(*FurthestX, Pawn->GetActorLocation().X - Origin.X);
			if (Frame == 199)
			{
				const double ContactX = 11998 - Pawn->GetCapsuleComponent()->GetScaledCapsuleRadius();
				AddInfo(FString::Printf(TEXT("Wall (lateral %.2f): peak %.1f cm/s, max x %.1f cm"),
					Lateral, *Peak, *FurthestX));
				TestTrue(TEXT("Reached top speed before wall"), *Peak >= 3170);
				TestTrue(TEXT("Capsule reached wall without crossing it"), FMath::IsNearlyEqual(*FurthestX, ContactX, 5.0));
			}
		});
	}

	Once([=, this]()
	{
		(*Wall)->Destroy();
		Movement->StopMovementImmediately();
		Pawn->SetActorLocation(Origin + FVector(0, 0, 500));
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
			TestEqual(TEXT("Alternating speed selection"), Movement->MaxFlySpeed, bDown ? 1800.f : 4400.f);
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
				if (State->Completed == 52)
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
				}
				if (Level == 2)
				{
					TestFalse(TEXT("High charged throw opens the panel"), Panel->IsCollisionEnabled());
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
		if (Level >= 1 && Frame == 100) { Key(EKeys::E, true); }
		if (Level >= 1 && Frame == 101) { Key(EKeys::E, false); }
		if (Level == 2 && Frame == 103) { Key(EKeys::Gamepad_RightShoulder, true); }
		if (Level == 2 && Frame == 104) { Key(EKeys::Gamepad_RightShoulder, false); }
		if (Frame == 107)
		{
			State->AirStart = Hero->GetActorLocation();
			State->CargoAirStart = Car->GetComponentLocation();
		}
		// Start braking earlier at level three so the faster run still throws from in front of the panel.
		const int32 BrakeFrame = Level == 2 ? 171 : 202;
		if (Frame >= 107 && Frame < BrakeFrame) { Hero->DoMove(0, 1); }
		if (Frame == BrakeFrame) { Key(EKeys::X, true); }
		if (Frame == 202)
		{
			const auto* Movement = Hero->GetCharacterMovement();
			const float FlightCaps[] = {900.0f, 1800.0f, 4400.0f};
			TestTrue(TEXT("Hero flies in the selected level with the car"), Movement->IsFlying()
				&& FMath::IsNearlyEqual(Movement->MaxFlySpeed, FlightCaps[Level], 1.0f));
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
