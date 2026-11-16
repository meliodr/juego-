#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "IBS4Policy.h"
#include "IBS4ResponseComponent.h"
#include "IBS4Threat.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UStaticMeshComponent;
class AIBS4PreparedTarget;
class UAnimSequence;
class USkeletalMesh;
class USkeletalMeshComponent;

UENUM(BlueprintType)
enum class EIBS4ThreatRole : uint8 { Officer, Tank, Robot, Drone, AerialRival };

UCLASS()
class IMPACTBEYOND_API AIBS4Missile : public AActor
{
    GENERATED_BODY()
public:
    AIBS4Missile();
    UPROPERTY(VisibleAnywhere) TObjectPtr<USphereComponent> Collision;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UProjectileMovementComponent> Movement;
    UFUNCTION(BlueprintImplementableEvent, Category="S4|Presentation") void PresentExplosion(FVector Point);
private:
    bool bExploded = false;
    UFUNCTION() void Impact(UPrimitiveComponent* Component, AActor* Other, UPrimitiveComponent* OtherComponent,
        FVector Impulse, const FHitResult& Hit);
};

/** Existing bounded alert/fire policy, with ground and aerial encounter roles. */
UCLASS()
class IMPACTBEYOND_API AIBS4Threat : public ACharacter
{
    GENERATED_BODY()
public:
    AIBS4Threat();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S4") bool bTank = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S4") EIBS4ThreatRole ThreatRole = EIBS4ThreatRole::Officer;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S4") bool bRadialGravity = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S4") FVector PlanetCenter = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S4") bool bRegionReady = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S4") bool bHostileOnSight = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S4", meta=(ClampMin="0")) float HostileGraceSeconds = 15;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S4") TObjectPtr<AIBS4PreparedTarget> PreparedWeapon;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S4|Animation") TObjectPtr<UAnimSequence> IdleAnimation;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S4|Animation") TObjectPtr<UAnimSequence> WalkAnimation;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S4|Animation") TObjectPtr<UAnimSequence> AttackAnimation;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S4") TObjectPtr<AActor> Target;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S4") FVector PatrolOffset = FVector(0, 250, 0);
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S4") TObjectPtr<UStaticMeshComponent> TankVisual;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S4|Presentation") TObjectPtr<USkeletalMeshComponent> Outfit;
    /** Animate Manny on GetMesh; the prevalidated outfit follows shared bone names. */
    UFUNCTION(BlueprintCallable, Category="S4|Presentation") bool SetSharedOutfit(USkeletalMesh* Skin);
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S4") TObjectPtr<UIBS4ResponseComponent> Response;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S4") int32 ShotsFired = 0;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S4") FString StateLabel = TEXT("Patrol");
    UFUNCTION(BlueprintCallable, Category="S4") bool ReportIncident(int64 IncidentId, AActor* Responsible, bool bVerified);
    UFUNCTION(BlueprintCallable, Category="S4") void ClearAlert();
    UFUNCTION(BlueprintImplementableEvent, Category="S4|Presentation") void PresentState(const FString& State);
    UFUNCTION(BlueprintImplementableEvent, Category="S4|Presentation") void PresentShot(FVector Start, FVector End, bool Missile);
private:
    IBS4::Alert Alert;
    FVector Home;
    FQuat HomeRotation = FQuat::Identity;
    bool bPatrolReturn = false;
    double NextShot = 0;
    int32 Burst = 0;
    bool HasSight(FHitResult& Trace) const;
    FVector LocalUp() const;
    bool IsAerial() const;
    double NextHostileNotice = 0;
    UPROPERTY() TObjectPtr<UAnimSequence> ActiveAnimation;
    double AttackAnimationUntil = 0;
};
