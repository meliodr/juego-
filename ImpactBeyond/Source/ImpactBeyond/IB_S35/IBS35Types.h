#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "IBS35Types.generated.h"

UENUM(BlueprintType)
enum class EIBS35ObjectClass : uint8 { Car, Container, Pole, Rock, LoosePart };

UENUM(BlueprintType)
enum class EIBS35DamageKind : uint8 { QuickPunch, HeavyPunch, HeldTool, ThrownObject };

USTRUCT(BlueprintType)
struct FIBS35ObjectProfile
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EIBS35ObjectClass ObjectClass = EIBS35ObjectClass::Car;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName Variant = TEXT("Sport");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="1", Units="kg")) float MassKg = 1200;
    // Half extents in cm. Root UBoxComponent is the sole blocking physical collider.
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector HalfExtent = FVector(210, 90, 65);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector LeftContact = FVector(-10, -30, -65);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector RightContact = FVector(-10, 30, -65);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName LeftSocket;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName RightSocket;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float GrabRange = 400;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxCarryError = 140;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float CarrySpeed = 1800;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float LiftSeconds = 0.55f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MinThrowSpeed = 1300;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxThrowSpeed = 6800;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bMeleeTool = false;
    bool IsValid() const;
    static FIBS35ObjectProfile Preset(EIBS35ObjectClass Kind, int32 VariantIndex = 0, float Scale = 1);
};

UCLASS(BlueprintType)
class IMPACTBEYOND_API UIBS35ObjectProfileAsset : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FIBS35ObjectProfile Profile;
};

USTRUCT(BlueprintType)
struct FIBS35DamageMessage
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly) FGuid AttackId;
    UPROPERTY(BlueprintReadOnly) EIBS35DamageKind DamageKind = EIBS35DamageKind::QuickPunch;
    UPROPERTY(BlueprintReadOnly) float HealthDelta = 0;
    // kg * cm / s. Physical contacts already resolved by Chaos must not apply it twice.
    UPROPERTY(BlueprintReadOnly) FVector Impulse = FVector::ZeroVector;
    UPROPERTY(BlueprintReadOnly) FGuid InstigatorId;
    UPROPERTY(BlueprintReadOnly) FGuid TargetId;
    UPROPERTY(BlueprintReadOnly) FName DamageResponseProfile;
    UPROPERTY(BlueprintReadOnly) FVector Contact = FVector::ZeroVector;
    UPROPERTY(BlueprintReadOnly) FVector RelativeVelocity = FVector::ZeroVector;
    UPROPERTY(BlueprintReadOnly) FName SurfaceMaterial;
    UPROPERTY(BlueprintReadOnly) float AttackSeconds = 0;
    UPROPERTY(BlueprintReadOnly) bool bPhysicsResolved = false;
    UPROPERTY(BlueprintReadOnly) bool bAirborne = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FIBS35DamageEvent, const FIBS35DamageMessage&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FIBS35AttackCue, FName, Phase, float, NormalizedTime);
