#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameModeBase.h"
#include "IBS3Character.generated.h"

class AIBS3PlanetWorld;
class UCameraComponent;
class USpringArmComponent;
class UIBS3MapWidget;

UCLASS()
class IMPACTBEYOND_API UIBS3Movement : public UCharacterMovementComponent
{
    GENERATED_BODY()
public:
    virtual void TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* TickFunction) override;
    virtual void PhysFlying(float DeltaTime,int32 Iterations) override;
};

UCLASS()
class IMPACTBEYOND_API AIBS3Character : public ACharacter
{
    GENERATED_BODY()
public:
    AIBS3Character(const FObjectInitializer& ObjectInitializer);
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<AIBS3PlanetWorld> Planet;
    UPROPERTY(VisibleAnywhere) TObjectPtr<USpringArmComponent> Arm;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UCameraComponent> Camera;
    UPROPERTY(EditAnywhere) TSubclassOf<UIBS3MapWidget> MapWidgetClass;
    UPROPERTY() TObjectPtr<UIBS3MapWidget> Map;
    bool bGlobalMap = false;
    double LastLongitude = 0;
    FVector TangentForward = FVector::UpVector;
    FVector PreviousUp = FVector::ForwardVector;
    bool PrepareMovement();
private:
    friend class FIBS3FlightCommand;
    friend class FIBS3CaptureCommand;
    bool bFrameInitialized = false;
    bool bTraversal = false;
    bool bBrake = false;
    float CameraPitch = -12;
    void Forward(float Value);
    void Right(float Value);
    void Ascend(float Value);
    void Turn(float Value);
    void Look(float Value);
    void TurnStick(float Value);
    void LookStick(float Value);
    void ToggleFlight();
    void ToggleTraversal();
    void ToggleMap();
    void Destination();
    void Interact();
    void Save();
    void BrakeOn();
    void BrakeOff();
};

UCLASS()
class IMPACTBEYOND_API AIBS3GameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    AIBS3GameMode();
    virtual void StartPlay() override;
};
