#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IBS4ResponseComponent.h"
#include "IBS4PreparedTarget.generated.h"

class AIBS35Object;

class UStaticMeshComponent;
class UStaticMesh;

UENUM(BlueprintType)
enum class EIBS4TargetKind : uint8 { Wall, Glass, Door, Crater, Robot, MaterialCoupon };

USTRUCT(BlueprintType)
struct FIBS4PreparedSnapshot
{
    GENERATED_BODY()
    UPROPERTY(SaveGame, BlueprintReadWrite) int32 Version = 1;
    UPROPERTY(SaveGame, BlueprintReadWrite) FName ObjectId;
    UPROPERTY(SaveGame, BlueprintReadWrite) EIBS4TargetKind Kind = EIBS4TargetKind::Wall;
    UPROPERTY(SaveGame, BlueprintReadWrite) bool bBroken = false;
    UPROPERTY(SaveGame, BlueprintReadWrite) float Health = 100;
    UPROPERTY(SaveGame, BlueprintReadWrite) float StructuralDamage = 0;
    UPROPERTY(SaveGame, BlueprintReadWrite) FTransform PartTransform;
    UPROPERTY(SaveGame, BlueprintReadWrite) FVector PartVelocity = FVector::ZeroVector;
    UPROPERTY(SaveGame, BlueprintReadWrite) FVector PartAngularVelocity = FVector::ZeroVector;
};

/** Prepared collision states, not arbitrary mesh cutting. Uses native Chaos bodies. */
UCLASS()
class IMPACTBEYOND_API AIBS4PreparedTarget : public AActor
{
    GENERATED_BODY()
public:
    AIBS4PreparedTarget();
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S4") EIBS4TargetKind Kind = EIBS4TargetKind::Wall;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S4") FName ObjectId;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S4") TObjectPtr<UIBS4ResponseComponent> Response;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S4") bool bBroken = false;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S4") bool bCannonEnabled = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S4") float BreakThreshold = 40;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S4") bool bRadialGravity = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S4") FVector PlanetCenter = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S4") bool bHideRobotShapes = false;
    UFUNCTION(BlueprintCallable, Category="S4") bool SetBrokenState(bool Broken);
    UFUNCTION(BlueprintCallable, Category="S4") bool ResetTarget();
    UFUNCTION(BlueprintCallable, Category="S4") bool CaptureSnapshot(FIBS4PreparedSnapshot& Snapshot) const;
    UFUNCTION(BlueprintCallable, Category="S4") bool CanRestoreSnapshot(const FIBS4PreparedSnapshot& Snapshot) const;
    UFUNCTION(BlueprintCallable, Category="S4") bool RestoreSnapshot(const FIBS4PreparedSnapshot& Snapshot);
    UFUNCTION(BlueprintImplementableEvent, Category="S4|Presentation") void PresentBreak(FVector Location, EIBS4Surface Material);
    UPrimitiveComponent* GetPart() const;
    AIBS35Object* GetSharedLoosePart();
private:
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Pieces;
    UPROPERTY() TArray<TObjectPtr<AActor>> Debris;
    UPROPERTY() TObjectPtr<UStaticMesh> Cube;
    UPROPERTY() TObjectPtr<AIBS35Object> SharedLoosePart;
    float AccumulatedDamage = 0;
    void Configure();
    void ClearDebris();
    bool IsRestoreBlocked() const;
    UFUNCTION() void HandleResponse(const FIBS4Hit& Hit, const FIBS4Outcome& Outcome);
};
