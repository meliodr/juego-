// Copyright Epic Games, Inc. All Rights Reserved.

#include "ImpactBeyondGameMode.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
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
	if (!Canvas || !Circuit || !Circuit->IsChargingThrow())
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
