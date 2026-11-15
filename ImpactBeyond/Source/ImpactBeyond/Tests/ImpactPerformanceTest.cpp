#include "ImpactBeyondCharacter.h"
#include "ProfilingDebugging/CsvProfiler.h"

#if WITH_DEV_AUTOMATION_TESTS && CSV_PROFILER

#include "Async/Future.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformTime.h"
#include "ImpactCircuitComponent.h"
#include "ImpactLandingComponent.h"
#include "InputKeyEventArgs.h"
#include "Misc/App.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UnrealClient.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FImpactPerformanceTest, "ImpactBeyond.Performance.City",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FImpactPerformanceTest::RunTest(const FString& Parameters)
{
	if (!FApp::CanEverRender())
	{
		AddError(TEXT("Run the city performance test with a rendering RHI."));
		return false;
	}
	UWorld* World = GEngine->GetCurrentPlayWorld();
	if (!World || !World->GetMapName().EndsWith(TEXT("Lvl_IB_C00")))
	{
		AddError(TEXT("Run the city performance test in /Game/IB_C00/Lvl_IB_C00."));
		return false;
	}

	class FRoute final : public IAutomationLatentCommand
	{
	public:
		FRoute(FAutomationTestBase& InTest, float InSeconds) : Test(InTest), Duration(InSeconds) {}

		bool Update() override
		{
			if (Csv.IsValid())
			{
				if (!Csv.IsReady()) { return false; }
				Test.AddInfo(FString::Printf(TEXT("City performance profile: %s; completed laps: %d"), *Csv.Get(), Laps));
				return true;
			}
			const double Now = FPlatformTime::Seconds();
			if (Now < ReadyAt) { return false; }
			if (Start == 0.0)
			{
				Start = LapStart = Now;
				FCsvProfiler::SetMetadata(TEXT("IB_RHI"), *FApp::GetGraphicsRHI());
				FCsvProfiler::SetMetadata(TEXT("IB_CPU"), *FPlatformMisc::GetCPUBrand());
				FCsvProfiler::SetMetadata(TEXT("IB_GPU"), *FPlatformMisc::GetPrimaryGPUBrand());
				if (GEngine->GameViewport && GEngine->GameViewport->Viewport)
				{
					const FIntPoint Resolution = GEngine->GameViewport->Viewport->GetSizeXY();
					FCsvProfiler::SetMetadata(TEXT("IB_RenderExtent"),
						*FString::Printf(TEXT("%dx%d"), Resolution.X, Resolution.Y));
				}
				FCsvProfiler::Get()->BeginCapture();
			}
			if (Now - Start >= Duration && Stage == 0)
			{
				Csv = FCsvProfiler::Get()->EndCapture();
				return false;
			}

			UWorld* World = GEngine->GetCurrentPlayWorld();
			APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
			AImpactBeyondCharacter* Hero = PC ? Cast<AImpactBeyondCharacter>(PC->GetPawn()) : nullptr;
			if (!Hero)
			{
				if (Now - LapStart > 65.0)
				{
					Test.AddError(TEXT("City route lost its playable pawn."));
					Csv = FCsvProfiler::Get()->EndCapture();
				}
				return false;
			}
			const auto Key = [PC](FKey Input, bool bHeld)
			{
				PC->InputKey(FInputKeyEventArgs::CreateSimulated(Input, bHeld ? IE_Pressed : IE_Released, bHeld ? 1.f : 0.f));
			};
			UCharacterMovementComponent* Movement = Hero->GetCharacterMovement();
			UImpactCircuitComponent* Circuit = Hero->FindComponentByClass<UImpactCircuitComponent>();
			UImpactLandingComponent* Landing = Hero->FindComponentByClass<UImpactLandingComponent>();
			if (!Circuit || !Landing)
			{
				Test.AddError(TEXT("City route requires both interaction components."));
				Csv = FCsvProfiler::Get()->EndCapture();
				return false;
			}
			const double LapTime = Now - LapStart;
			constexpr double Times[] = {0, 4, 9, 11, 12, 16, 16.2, 16.4, 16.6, 16.8, 17, 17.2, 17.4,
				23, 25, 26, 28.5, 29.8, 34, 34.3, 36, 45, 51};
			// Leave a stopping margin before the eastern highway ends at x=1780 m.
			const double EastSpeed = FMath::Max(0.0, Movement->Velocity.X);
			const double StoppingDistance = FMath::Square(EastSpeed) / (2.0 * 18000.0);
			const bool bAtAvenueBrakeMark = Stage == 13
				&& Hero->GetActorLocation().X + StoppingDistance >= 173000.0;
			if (LapTime >= Times[Stage] || bAtAvenueBrakeMark)
			{
				switch (Stage++)
				{
				case 0: CSV_EVENT_GLOBAL(TEXT("IB:Plaza")); PC->SetControlRotation(FRotator(-7, 0, 0)); break;
				case 1: CSV_EVENT_GLOBAL(TEXT("IB:Street")); break;
				case 2: CSV_EVENT_GLOBAL(TEXT("IB:Sprint")); Key(EKeys::LeftShift, true); break;
				case 3: CSV_EVENT_GLOBAL(TEXT("IB:Superjump")); Key(EKeys::SpaceBar, true); break;
				case 4:
					Test.TestTrue(TEXT("City route reached supersalto height before switching to flight"),
						Movement->IsFalling() && Hero->GetActorLocation().Z > 900.0);
					Key(EKeys::SpaceBar, false); Key(EKeys::LeftShift, false);
					CSV_EVENT_GLOBAL(TEXT("IB:Flight")); Key(EKeys::F, true); break;
				case 5: Key(EKeys::F, false); CSV_EVENT_GLOBAL(TEXT("IB:Accelerate")); Key(EKeys::E, true); break;
				case 6: case 8: Key(EKeys::E, false); break;
				case 7: case 11: Key(EKeys::E, true); break;
				case 9: Key(EKeys::B, true); break;
				case 10: Key(EKeys::B, false); break;
				case 12: Key(EKeys::E, false); Key(EKeys::LeftShift, true); break;
				case 13:
					CSV_EVENT_GLOBAL(TEXT("IB:Brake"));
					Test.AddInfo(FString::Printf(TEXT("City flight speed before braking: %.2f m/s at x=%.1f m"),
						Movement->Velocity.Size() / 100.0, Hero->GetActorLocation().X / 100.0));
					Key(EKeys::LeftShift, false);
					Key(EKeys::X, true);
					break;
				case 14: Key(EKeys::X, false); PC->SetControlRotation(FRotator(-13, 85, 0)); break;
				case 15:
					CSV_EVENT_GLOBAL(TEXT("IB:Carry"));
					for (TActorIterator<AStaticMeshActor> Actor(World); Actor; ++Actor)
					{
						if (Actor->ActorHasTag(TEXT("ImpactCircuitCar")))
						{
							const FVector Car = Actor->GetActorLocation();
							Hero->SetActorLocationAndRotation(FVector(Car.X, Car.Y - 320.0, 98.0),
								FRotator(0, 90, 0), false, nullptr, ETeleportType::TeleportPhysics);
							Movement->SetMovementMode(MOVE_Walking);
							Movement->StopMovementImmediately();
							PC->SetControlRotation(FRotator(0, 90, 0));
							Circuit->ToggleGrab();
							break;
						}
					}
					break;
				case 16:
					Test.TestTrue(TEXT("City route grabbed the car"), Circuit->IsCarHeld());
					Circuit->BeginThrowCharge();
					break;
				case 17: CSV_EVENT_GLOBAL(TEXT("IB:Throw")); Circuit->ReleaseThrowCharge(); break;
				case 18:
					for (TActorIterator<AStaticMeshActor> Actor(World); Actor; ++Actor)
					{
						if (Actor->ActorHasTag(TEXT("ImpactCircuitPanel")))
						{
							Test.TestEqual(TEXT("City route opened the panel"),
								Actor->GetStaticMeshComponent()->GetCollisionEnabled(), ECollisionEnabled::NoCollision);
						}
					}
					Hero->SetActorLocation(FVector(52000, 13000, 975), false, nullptr, ETeleportType::TeleportPhysics);
					Movement->StopMovementImmediately();
					Movement->SetMovementMode(MOVE_Falling);
					PC->SetControlRotation(FRotator(-19, 90, 0));
					CSV_EVENT_GLOBAL(TEXT("IB:Impact")); Key(EKeys::V, true);
					break;
				case 19: Key(EKeys::V, false); Test.TestTrue(TEXT("City route started impact"), Landing->IsImpacting()); break;
				case 20: Test.TestTrue(TEXT("City route landed after impact"), Movement->IsMovingOnGround()); break;
				case 21: CSV_EVENT_GLOBAL(TEXT("IB:Reset")); Circuit->ResetCircuit(); break;
				case 22: ++Laps; Stage = 0; LapStart = Now; break;
				}
			}
			if ((Stage >= 2 && Stage <= 4) || (Stage >= 5 && Stage <= 13))
			{
				Hero->AddMovementInput(FVector::ForwardVector);
			}
			return false;
		}

	private:
		FAutomationTestBase& Test;
		const double Duration;
		const double ReadyAt = FPlatformTime::Seconds() + 10.0;
		double Start = 0.0;
		double LapStart = 0.0;
		int32 Stage = 0;
		int32 Laps = 0;
		TSharedFuture<FString> Csv;
	};

	float Seconds = 45.0f;
	FParse::Value(FCommandLine::Get(), TEXT("IBSoakSeconds="), Seconds);
	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShared<FRoute>(*this, FMath::Clamp(Seconds, 45.0f, 900.0f)));
	return true;
}

#endif
