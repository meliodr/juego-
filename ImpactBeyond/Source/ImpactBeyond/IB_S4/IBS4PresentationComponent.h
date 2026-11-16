#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IBS4ResponseComponent.h"
#include "IBS4PresentationComponent.generated.h"

class UStaticMeshComponent;
class UAudioComponent;
class USoundWaveProcedural;

/** Native provisional feedback; Blueprint hooks can drive approved Niagara assets.
 * No Niagara module dependency or Fab bytes are introduced by this lab. */
UCLASS(Blueprintable, ClassGroup=(IBS4), meta=(BlueprintSpawnableComponent))
class IMPACTBEYOND_API UIBS4PresentationComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UIBS4PresentationComponent();
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S4") float Volume = 0.18f;
    UFUNCTION(BlueprintCallable, Category="S4") void Beam(FVector Start, FVector End, bool bVisible);
    UFUNCTION(BlueprintCallable, Category="S4") void Impact(FVector Point, EIBS4Surface Surface, bool bHeavy);
    UFUNCTION(BlueprintImplementableEvent, Category="S4") void NiagaraBeam(FVector Start, FVector End, bool bVisible);
    UFUNCTION(BlueprintImplementableEvent, Category="S4") void NiagaraImpact(FVector Point, EIBS4Surface Surface, bool bHeavy);
private:
    UPROPERTY() TObjectPtr<UStaticMeshComponent> BeamMesh;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> SparkMesh;
    UPROPERTY() TObjectPtr<UAudioComponent> Audio;
    UPROPERTY() TObjectPtr<USoundWaveProcedural> Wave;
    double BeamUntil = 0;
    double SparkUntil = 0;
    double NextAudio = 0;
    double AudioUntil = 0;
};
