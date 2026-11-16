#include "IB_S35/IBS35LabTarget.h"
#include "IB_S35/IBS35ReceiverComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AIBS35LabTarget::AIBS35LabTarget()
{
    Body = CreateDefaultSubobject<UBoxComponent>(TEXT("Body")); SetRootComponent(Body);
    Body->SetCollisionProfileName(TEXT("BlockAll"));
    Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual")); Visual->SetupAttachment(Body);
    Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    Visual->SetStaticMesh(Cube.Object);
    Receiver = CreateDefaultSubobject<UIBS35ReceiverComponent>(TEXT("Receiver"));
    Receiver->InitialHealth = 100000;
}
void AIBS35LabTarget::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    Body->SetBoxExtent(HalfExtent.ComponentMax(FVector(2)));
    Visual->SetRelativeScale3D(Body->GetUnscaledBoxExtent() / 50);
}
void AIBS35LabTarget::RefreshGeometry()
{
    if (!HasActorBegunPlay()) OnConstruction(GetActorTransform());
}
