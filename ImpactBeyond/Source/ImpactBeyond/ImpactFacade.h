#pragma once

#include "GameFramework/Actor.h"
#include "ImpactFacade.generated.h"

class UHierarchicalInstancedStaticMeshComponent;

UCLASS()
class AImpactFacade : public AActor
{
	GENERATED_BODY()

public:
	AImpactFacade();

private:
	UPROPERTY(VisibleAnywhere, Category="District")
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> Facade;
};
