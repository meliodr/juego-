#include "ImpactFacade.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"

AImpactFacade::AImpactFacade()
{
	Facade = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("Facade"));
	SetRootComponent(Facade);
	Facade->SetMobility(EComponentMobility::Static);
	Facade->SetCollisionProfileName(TEXT("NoCollision"));
	Facade->SetCanEverAffectNavigation(false);
}
