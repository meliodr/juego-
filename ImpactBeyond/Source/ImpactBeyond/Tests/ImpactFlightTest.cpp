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
		ForFrames(10, [=, this](int32 Frame)
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
			if (Frame == 9)
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
	SpeedStage(EKeys::Gamepad_RightShoulder, 3200);

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
			TestEqual(TEXT("Alternating speed selection"), Movement->MaxFlySpeed, bDown ? 1800.f : 3200.f);
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
	Pawn->SetActorLocation(FVector(InitialCarLocation.X, InitialCarLocation.Y - 500.0, 98.0), false, nullptr, ETeleportType::TeleportPhysics);
	PC->SetControlRotation(FRotator(0.0, 90.0, 0.0));
	Circuit->ToggleGrab();
	TestFalse(TEXT("Grab takes the car out of rigid simulation"), Car->IsSimulatingPhysics());
	Circuit->ToggleGrab();
	TestTrue(TEXT("Drop restores rigid simulation"), Car->IsSimulatingPhysics());
	Circuit->ToggleGrab();
	ADD_LATENT_AUTOMATION_COMMAND(FFlightStep([this, Circuit, Car, Panel, World, Frame = 0]() mutable
	{
		++Frame;
		if (Frame == 12)
		{
			TestTrue(TEXT("Held car lifted above street"), Car->GetComponentLocation().Z > 150.0);
			TestTrue(TEXT("Intact panel blocks collision"), Panel->IsCollisionEnabled());
			Circuit->ThrowCar();
			TestTrue(TEXT("Thrown car simulates physics"), Car->IsSimulatingPhysics());
		}
		if (Frame < 160) { return false; }
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

#endif
