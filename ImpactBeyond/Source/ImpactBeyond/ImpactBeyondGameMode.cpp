// Copyright Epic Games, Inc. All Rights Reserved.

#include "ImpactBeyondGameMode.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ImpactBeyondCharacter.h"
#include "ImpactCircuitComponent.h"

AImpactBeyondGameMode::AImpactBeyondGameMode()
{
	HUDClass = AImpactBeyondHUD::StaticClass();
}

void AImpactBeyondHUD::DrawHUD()
{
	Super::DrawHUD();
	const AImpactBeyondCharacter* Hero = PlayerOwner ? Cast<AImpactBeyondCharacter>(PlayerOwner->GetPawn()) : nullptr;
	const UImpactCircuitComponent* Circuit = Hero ? Hero->GetCircuit() : nullptr;
	if (!Canvas || !Circuit || !GEngine)
	{
		return;
	}

	const float UIScale = FMath::Clamp(Canvas->ClipY / 900.0f, 0.7f, 1.6f);
	const UCharacterMovementComponent* Movement = Hero->GetCharacterMovement();
	const float Inset = 26.0f * UIScale;
	const float DisplayWidth = FMath::Min(Canvas->ClipX - 2.0f * Inset, 640.0f * UIScale);
	const FString MovementStatus = Movement->IsFlying()
		? FString::Printf(TEXT("VUELO %d  |  %.0f m/s"), Hero->GetFlightLevel(), Movement->Velocity.Size() / 100.0f)
		: FString::Printf(TEXT("H01  |  %.0f m/s"), Movement->Velocity.Size2D() / 100.0f);
	const FString ActionHint = Circuit->IsCarHeld()
		? TEXT("Mantener T / Triangulo para cargar. Suelta para lanzar.")
		: (Movement->IsFlying() || Movement->IsFalling())
			? TEXT("V / Cruceta arriba: impacto  |  X / Cuadrado: freno")
			: TEXT("Shift / L3 + Espacio / Cruz: supersalto  |  F / R3: vuelo");
	UFont* Font = GEngine->GetSmallFont();
	DrawRect(FLinearColor(0.02f, 0.025f, 0.035f, 0.86f), Inset, Inset, DisplayWidth, 74.0f * UIScale);
	DrawText(MovementStatus, FLinearColor(0.43f, 0.84f, 1.0f), Inset * 1.45f, Inset + 7.0f * UIScale, Font, UIScale * 1.28f);
	DrawText(ActionHint, FLinearColor::White, Inset * 1.45f, Inset + 32.0f * UIScale, Font, UIScale);
	DrawText(TEXT("G / Circulo: agarre  |  R / Cruceta abajo: reinicio  |  F2 / Cruceta izq.: calidad"),
		FLinearColor(0.79f, 0.83f, 0.87f), Inset * 1.45f, Inset + 52.0f * UIScale, Font, UIScale * 0.88f);
	if (!Circuit->IsChargingThrow())
	{
		return;
	}

	const float Charge = Circuit->GetThrowCharge();
	const int32 Level = FMath::Min(FMath::FloorToInt(Charge * 3.0f), 2);
	const TCHAR* Labels[] = {TEXT("BAJA"), TEXT("MEDIA"), TEXT("ALTA")};
	const FLinearColor Colors[] = {
		FLinearColor(0.24f, 0.76f, 0.95f),
		FLinearColor(1.0f, 0.78f, 0.2f),
		FLinearColor(1.0f, 0.38f, 0.17f)
	};
	const float Scale = FMath::Clamp(Canvas->ClipY / 900.0f, 0.65f, 2.0f);
	const float Width = 240.0f * Scale;
	const float X = (Canvas->ClipX - Width) * 0.5f;
	const float Y = Canvas->ClipY * 0.79f;
	DrawRect(FLinearColor(0.02f, 0.025f, 0.035f, 0.92f), X - 12.0f * Scale, Y - 28.0f * Scale,
		Width + 24.0f * Scale, 54.0f * Scale);
	DrawText(FString::Printf(TEXT("POTENCIA  %s"), Labels[Level]), FLinearColor::White, X, Y - 23.0f * Scale,
		GEngine->GetSmallFont(), Scale * 1.2f);

	const float SegmentWidth = Width / 3.0f;
	for (int32 Index = 0; Index < 3; ++Index)
	{
		const float SegmentX = X + Index * SegmentWidth;
		const float Available = SegmentWidth - 3.0f * Scale;
		const float SegmentCharge = FMath::Clamp(Charge * 3.0f - Index, 0.0f, 1.0f);
		DrawRect(FLinearColor(0.21f, 0.24f, 0.28f), SegmentX, Y, Available, 11.0f * Scale);
		DrawRect(Colors[Index], SegmentX, Y, Available * SegmentCharge, 11.0f * Scale);
	}
}
