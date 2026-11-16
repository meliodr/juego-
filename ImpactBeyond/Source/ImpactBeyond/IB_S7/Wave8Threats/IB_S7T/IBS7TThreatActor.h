#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IB_S35/IBS35Types.h"
#include "IB_S4/IBS4ResponseComponent.h"
#include "IB_Integration/ImpactContract/ImpactContract.h"
#include "../ThreatModel.h"
#include "IBS7TThreatActor.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class USkeletalMeshComponent;
class UPrimitiveComponent;
class AIBS35Object;
class ACharacter;
class UAnimSequence;
class UStaticMesh;
class ULineBatchComponent;
class UTextRenderComponent;
class UIBS4PresentationComponent;

UENUM(BlueprintType)
enum class EIBS7TPart : uint8 { Hammer, Claw, Horn, Tail, Shoulder, Body };

USTRUCT(BlueprintType)
struct FIBS7TPartSnapshot
{
    GENERATED_BODY()
    UPROPERTY(SaveGame, BlueprintReadWrite) EIBS7TPart Part = EIBS7TPart::Body;
    UPROPERTY(SaveGame, BlueprintReadWrite) float Health = 100;
    UPROPERTY(SaveGame, BlueprintReadWrite) uint8 State = 0;
    UPROPERTY(SaveGame, BlueprintReadWrite) bool bLoosePiece = false;
    UPROPERTY(SaveGame, BlueprintReadWrite) FTransform LooseTransform;
    UPROPERTY(SaveGame, BlueprintReadWrite) FVector LinearVelocity = FVector::ZeroVector;
    UPROPERTY(SaveGame, BlueprintReadWrite) FVector AngularVelocity = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FIBS7TThreatSnapshot
{
    GENERATED_BODY()
    UPROPERTY(SaveGame, BlueprintReadWrite) int32 Version = 1;
    UPROPERTY(SaveGame, BlueprintReadWrite) FName StableId;
    UPROPERTY(SaveGame, BlueprintReadWrite) bool bKaiju = false;
    UPROPERTY(SaveGame, BlueprintReadWrite) FTransform Transform;
    UPROPERTY(SaveGame, BlueprintReadWrite) float Health = 500;
    UPROPERTY(SaveGame, BlueprintReadWrite) TArray<FIBS7TPartSnapshot> Parts;
};

DECLARE_DELEGATE_RetVal_TwoParams(bool, FIBS7TCanonicalStrike, const FIBS4Hit&, const FHitResult&);

/** Prepared threat with one native health owner and canonical contact routing. */
UCLASS()
class IMPACTBEYOND_API AIBS7TThreatActor : public AActor
{
    GENERATED_BODY()
public:
    AIBS7TThreatActor();
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    virtual void Tick(float DeltaSeconds) override;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S7") TObjectPtr<UBoxComponent> Body;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S7") TObjectPtr<USkeletalMeshComponent> Visual;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S7") TArray<TObjectPtr<UStaticMeshComponent>> Modules;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S7") TArray<FName> PartBones;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S7") bool bUseSkeletalParts = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S7|Presentation") bool bShowTelegraphs = true;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S7|Presentation") TObjectPtr<ULineBatchComponent> TelegraphLines;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S7|Presentation") TObjectPtr<UTextRenderComponent> WarningText;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S7|Presentation") TObjectPtr<UIBS4PresentationComponent> Feedback;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S7|Animation") TObjectPtr<UAnimSequence> IdleAnimation;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S7|Animation") TObjectPtr<UAnimSequence> WalkAnimation;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S7|Animation") TObjectPtr<UAnimSequence> AttackAnimation;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S7|Animation") float MovementSpeed = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="S7") bool bKaiju = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S7") bool bRegionReady = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S7") TObjectPtr<AActor> Target;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S7") FName StableId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S7") float MoveSpeed = 280;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S7") float ChargeSpeed = 1800;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S7") float PerceptionRange = 3000;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S7") float GrabHoldSeconds = 0.6f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S7") bool bRadialGravity = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S7") FVector PlanetCenter = FVector::ZeroVector;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S7") FString PhaseLabel;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S7") int64 Generation = 1;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S7") float BodyHealth = 500;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S7") int32 AcceptedContacts = 0;
    FIBS7TCanonicalStrike CanonicalStrike;
    bool ConfigureIdentity(FName Identity, FGuid WorldSession, FVector Center);
    FGuid GetTargetLifeId() const { return TargetLifeId; }
    bool AcceptCanonicalImpact(const FIBS35DamageMessage& Message, const FHitResult& Hit);
    bool AcceptCanonicalS4Impact(const FIBS4Hit& Hit, FGuid CapturedTargetLife);
    UFUNCTION(BlueprintCallable, Category="S7") void SetRegionReady(bool bReady);
    UFUNCTION(BlueprintCallable, Category="S7") bool ReceiveImpact(int64 Epoch, int64 AttackSerial, EIBS7TPart Part, float Damage);
    UFUNCTION(BlueprintCallable, Category="S7") bool DetachExposed(EIBS7TPart Part);
    UFUNCTION(BlueprintPure, Category="S7") EIBS7TPart FindExposedPart(const FHitResult& Hit, float ToleranceCm = 80) const;
    UFUNCTION(BlueprintCallable, Category="S7") AIBS35Object* TryDetachAt(const FHitResult& Hit, float ToleranceCm = 80);
    // Extracted mesh uses bone-local space. NAME_None denotes a visible external
    // module centered on its prepared collider. Call after mesh assignment/FinishSpawning.
    UFUNCTION(BlueprintCallable, Category="S7") bool ConfigurePartArt(EIBS7TPart Part, UStaticMesh* Mesh, FName BoneName);
    UFUNCTION(BlueprintCallable, Category="S7") bool ResetThreat();
    UFUNCTION(BlueprintCallable, Category="S7") EIBS7TPart IdentifyPart(UPrimitiveComponent* Component) const;
    UFUNCTION(BlueprintPure, Category="S7") AIBS35Object* GetLoosePiece(EIBS7TPart Part) const;
    UFUNCTION(BlueprintPure, Category="S7") UPrimitiveComponent* GetPartCollider(EIBS7TPart Part) const;
    UFUNCTION(BlueprintCallable, Category="S7") bool CaptureSnapshot(FIBS7TThreatSnapshot& Snapshot) const;
    UFUNCTION(BlueprintCallable, Category="S7") bool CanRestoreSnapshot(const FIBS7TThreatSnapshot& Snapshot) const;
    UFUNCTION(BlueprintCallable, Category="S7") bool RestoreSnapshot(const FIBS7TThreatSnapshot& Snapshot);
    UFUNCTION(BlueprintNativeEvent, Category="S7|Integration") bool DispatchPreparedContact(const FHitResult& Contact, int64 Epoch, int64 LocalAttack, int32 Move, float Amount);
    UFUNCTION(BlueprintImplementableEvent, Category="S7|Presentation") void PhaseChanged(const FString& Phase, int32 Move, float TelegraphSeconds);
    UFUNCTION(BlueprintImplementableEvent, Category="S7|Presentation") void PartRemoved(EIBS7TPart Part);
private:
    TUniquePtr<IBS7T::Threat> Model;
    TUniquePtr<IBImpact::TargetRouter<>> ImpactRouter;
    IBS7T::DebrisPool LogicalDebris;
    bool bRuntimeKaiju = false;
    bool bAlternate = false;
    FVector LockedAim = FVector::ForwardVector;
    FGuid SessionId;
    FGuid TargetLifeId;
    FIBS4Hit OutgoingPacket;
    int64 OutgoingEpoch = 0;
    int64 OutgoingAttack = 0;
    int64 NativeSerialFloor = 0;
    float GrabRemaining = 0;
    double TelegraphStarted = 0;
    uint8 SavedMovementMode = 0;
    uint8 SavedCustomMovementMode = 0;
    FVector SavedVelocity = FVector::ZeroVector;
    UPROPERTY() TObjectPtr<ACharacter> CapturedHero;
    UPROPERTY() TObjectPtr<UAnimSequence> ActiveAnimation;
    UPROPERTY() TArray<TObjectPtr<UBoxComponent>> PartBoxes;
    UPROPERTY() TArray<TObjectPtr<AIBS35Object>> LoosePieces;
    IBS7T::PartId PartAt(int32 Index) const;
    void SyncParts();
    void UpdatePartTransforms();
    void UpdateTelegraph();
    void RenewLife();
    void SweepAttack();
    void MoveThreat(float DeltaSeconds);
    void UpdateCapture(float DeltaSeconds);
    void ReleaseCapture();
    bool TryCapture(AActor* Actor);
    bool AnyPieceHeld() const;
    AIBS35Object* SpawnLoosePiece(int32 Index, const FTransform& Transform);
    bool ApplyEnvelope(const IBImpact::Envelope& Envelope, UPrimitiveComponent* Component);
    FVector Up() const;
};
