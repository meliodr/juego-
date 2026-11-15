#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Character.h"
#include "IBS4ResponseComponent.h"
#include "IBS4LabPawn.generated.h"

class UInputAction;
class UInputMappingContext;
class UCameraComponent;
class USpringArmComponent;
class UIBS4PowerComponent;
struct FInputActionValue;

/** Lab-only contract: proximity never constitutes landing intent. */
UCLASS(ClassGroup=(IBS4), meta=(BlueprintSpawnableComponent))
class IMPACTBEYOND_API UIBS4FlightIntentComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UIBS4FlightIntentComponent();
    virtual void TickComponent(float Dt, ELevelTick Type, FActorComponentTickFunction* Fn) override;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S4") bool bFlightIntent = true;
    UFUNCTION(BlueprintCallable, Category="S4") void SetFlightIntent(bool Flying);
};

/** Independent CMC test pawn. Does not call shared circuit/landing components. */
UCLASS()
class IMPACTBEYOND_API AIBS4LabPawn : public ACharacter
{
    GENERATED_BODY()
public:
    AIBS4LabPawn();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
    virtual void Tick(float Dt) override;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S4") TObjectPtr<UIBS4ResponseComponent> Response;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S4") TObjectPtr<UIBS4PowerComponent> Powers;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S4") TObjectPtr<UIBS4FlightIntentComponent> Flight;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="S4", meta=(ClampMin="0", ClampMax="0.4")) float MicroShake = 0.12f;
private:
    UPROPERTY() TObjectPtr<USpringArmComponent> Boom;
    UPROPERTY() TObjectPtr<UCameraComponent> Camera;
    UPROPERTY() TObjectPtr<UInputMappingContext> Mapping;
    UPROPERTY() TArray<TObjectPtr<UInputAction>> Actions;
    float ShakeTime = 0;
    float CameraShakeOffset = 0;
    int32 LastShakePushCount = 0;
    void Forward(const FInputActionValue& Value);
    void Right(const FInputActionValue& Value);
    void Vertical(const FInputActionValue& Value);
    void Yaw(const FInputActionValue& Value);
    void Pitch(const FInputActionValue& Value);
    void YawRate(const FInputActionValue& Value);
    void PitchRate(const FInputActionValue& Value);
    void Trigger();
    void StopTrigger();
    void Grab();
    void Punch();
    void ToggleFlight();
    void ResetLab();
    void TestAlert();
    UFUNCTION() void React(const FIBS4Hit& Hit, float Delta);
};
