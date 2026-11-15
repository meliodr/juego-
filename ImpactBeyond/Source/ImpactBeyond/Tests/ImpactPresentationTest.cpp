#include "ImpactBeyondCharacter.h"

#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS

#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "ImpactCircuitComponent.h"
#include "InputKeyEventArgs.h"
#include "Misc/App.h"
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FImpactPresentationTest, "ImpactBeyond.Presentation.Capture",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FImpactCaptureFrames, AImpactBeyondCharacter*, Hero, int32, Frame);

bool FImpactCaptureFrames::Update()
{
	APlayerController* PC = CastChecked<APlayerController>(Hero->GetController());
	UImpactCircuitComponent* Circuit = Hero->FindComponentByClass<UImpactCircuitComponent>();
	const auto Capture = [](const TCHAR* Name)
	{
		const FString Path = FPaths::ConvertRelativePathToFull(
			FPaths::ProjectSavedDir() / TEXT("Screenshots") / TEXT("IB-H01") / Name);
		FScreenshotRequest::RequestScreenshot(Path, true, false);
	};
	const auto Key = [PC](FKey Input, bool bHeld)
	{
		PC->InputKey(FInputKeyEventArgs::CreateSimulated(Input, bHeld ? IE_Pressed : IE_Released, bHeld ? 1.f : 0.f));
	};
	const float Yaw = Hero->GetActorRotation().Yaw;
	switch (Frame++)
	{
	case 0: Key(EKeys::F, true); break;
	case 1: Key(EKeys::F, false); break;
	case 9: Capture(TEXT("Despegue.png")); break;
	case 50: PC->SetControlRotation(FRotator(-5.0, Yaw + 135.0, 0.0)); break;
	case 60: Capture(TEXT("Suspension-tres-cuartos.png")); break;
	case 75: PC->SetControlRotation(FRotator(-5.0, Yaw + 180.0, 0.0)); break;
	case 85: Capture(TEXT("Suspension-frente.png")); break;
	case 100: PC->SetControlRotation(FRotator(-5.0, Yaw + 90.0, 0.0)); break;
	case 110: Capture(TEXT("Suspension-perfil.png")); break;
	case 151: Capture(TEXT("Crucero.png")); break;
	case 170:
	{
		Hero->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		Hero->GetCharacterMovement()->StopMovementImmediately();
		for (TActorIterator<AStaticMeshActor> Actor(Hero->GetWorld()); Actor; ++Actor)
		{
			if (Actor->ActorHasTag(TEXT("ImpactCircuitCar")))
			{
				const FVector Car = Actor->GetActorLocation();
				Hero->SetActorLocationAndRotation(FVector(Car.X, Car.Y - 320.0, 98.0),
					FRotator(0.0, 90.0, 0.0), false, nullptr, ETeleportType::TeleportPhysics);
				break;
			}
		}
		PC->SetControlRotation(FRotator(-8.0, 175.0, 0.0));
		Circuit->ToggleGrab();
		break;
	}
	case 220: Key(EKeys::F, true); break;
	case 221: Key(EKeys::F, false); break;
	case 230: Capture(TEXT("Despegue-con-carga.png")); break;
	case 270: Key(EKeys::T, true); break;
	case 279: Capture(TEXT("Potencia-baja.png")); break;
	case 296: Capture(TEXT("Potencia-media.png")); break;
	case 328: Capture(TEXT("Potencia-alta.png")); break;
	case 338: Key(EKeys::T, false); break;
	case 349: Capture(TEXT("Lanzamiento.png")); break;
	case 365:
		Hero->GetCharacterMovement()->StopMovementImmediately();
		Hero->SetActorLocationAndRotation(FVector(52000.0, 13000.0, 1400.0), FRotator(0, 90, 0),
			false, nullptr, ETeleportType::TeleportPhysics);
		Hero->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
		PC->SetControlRotation(FRotator(-24.0, 90.0, 0.0));
		Key(EKeys::V, true);
		break;
	case 366: Key(EKeys::V, false); break;
	case 382: Capture(TEXT("Descenso-impacto.png")); break;
	case 407: Capture(TEXT("Onda-de-impacto.png")); break;
	case 430: return true;
	default: break;
	}

	if (Frame > 117 && Frame < 166)
	{
		Hero->AddMovementInput(FRotator(0.0, Yaw, 0.0).Vector());
	}
	return false;
}

bool FImpactPresentationTest::RunTest(const FString& Parameters)
{
	if (!FApp::CanEverRender())
	{
		AddError(TEXT("Run the presentation test with a rendering RHI."));
		return false;
	}
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		UWorld* World = Context.World();
		APlayerController* PC = Context.WorldType == EWorldType::Game && World ? World->GetFirstPlayerController() : nullptr;
		AImpactBeyondCharacter* Hero = PC ? Cast<AImpactBeyondCharacter>(PC->GetPawn()) : nullptr;
		if (Hero && Hero->FindComponentByClass<UImpactCircuitComponent>())
		{
			ADD_LATENT_AUTOMATION_COMMAND(FImpactCaptureFrames(Hero, 0));
			return true;
		}
	}
	AddError(TEXT("Load the circuit as a game world before capturing."));
	return false;
}

#endif
