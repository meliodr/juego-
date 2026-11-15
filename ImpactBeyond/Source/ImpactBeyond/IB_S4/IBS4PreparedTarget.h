#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IBS4ResponseComponent.h"
#include "IBS4PreparedTarget.generated.h"

class UStaticMeshComponent;
class UStaticMesh;

UENUM(BlueprintType)
enum class EIBS4TargetKind : uint8 { Wall, Glass, Door, Crater, Robot, MaterialCoupon };

/** Prepared collision states, not arbitrary mesh cutting. Uses native Chaos bodies. */
UCLASS()
class IMPACTBEYOND_API AIBS4PreparedTarget : public AActor
{
    GENERATED_BODY()
public:
    AIBS4PreparedTarget();
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S4") EIBS4TargetKind Kind = EIBS4TargetKind::Wall;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S4") FName ObjectId;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S4") TObjectPtr<UIBS4ResponseComponent> Response;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S4") bool bBroken = false;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S4") bool bCannonEnabled = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S4") float BreakThreshold = 40;
    UFUNCTION(BlueprintCallable, Category="S4") bool SetBrokenState(bool Broken);
    UFUNCTION(BlueprintCallable, Category="S4") bool ResetTarget();
    UFUNCTION(BlueprintImplementableEvent, Category="S4|Presentation") void PresentBreak(FVector Location, EIBS4Surface Material);
    UPrimitiveComponent* GetPart() const;
private:
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Pieces;
    UPROPERTY() TArray<TObjectPtr<AActor>> Debris;
    UPROPERTY() TObjectPtr<UStaticMesh> Cube;
    float AccumulatedDamage = 0;
    void Configure();
    void ClearDebris();
    bool IsRestoreBlocked() const;
    UFUNCTION() void HandleResponse(const FIBS4Hit& Hit, const FIBS4Outcome& Outcome);
};
