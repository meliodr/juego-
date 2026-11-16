#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IBS35LabTarget.generated.h"

UCLASS()
class IMPACTBEYOND_API AIBS35LabTarget : public AActor
{
    GENERATED_BODY()
public:
    AIBS35LabTarget();
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<class UBoxComponent> Body;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<class UStaticMeshComponent> Visual;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<class UIBS35ReceiverComponent> Receiver;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector HalfExtent = FVector(35,80,100);
    UFUNCTION(BlueprintCallable, CallInEditor) void RefreshGeometry();
    virtual void OnConstruction(const FTransform& Transform) override;
};
