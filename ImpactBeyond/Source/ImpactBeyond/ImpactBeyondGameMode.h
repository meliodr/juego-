// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/HUD.h"
#include "ImpactBeyondGameMode.generated.h"

UCLASS()
class AImpactBeyondHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;
};

/**
 *  Simple GameMode for a third person game
 */
UCLASS(abstract)
class AImpactBeyondGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	
	/** Constructor */
	AImpactBeyondGameMode();
};



