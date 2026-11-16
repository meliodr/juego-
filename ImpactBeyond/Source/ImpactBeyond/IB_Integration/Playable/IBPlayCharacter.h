#pragma once
#include "CoreMinimal.h"
#include "IB_S3/IBS3Character.h"
#include "GameFramework/HUD.h"
#include "IB_S35/IBS35Types.h"
#include "IB_S4/IBS4ResponseComponent.h"
#include "IBPlayCharacter.generated.h"

class UIBS35GripComponent;
class UIBS35PunchComponent;
class UIBS6HBridge;
class UIBS7TWheelComponent;
class UIBS4PowerComponent;
class UIBS4PresentationComponent;
class UImpactFlightFeedback;
class UImpactLandingComponent;
class UIBPlayCombat;
class UIBPlayArt;
class AIBPlaySession;
class UAnimSequence;
class UAnimationAsset;
class UBlendSpace;

enum class EIBPlayMotion : uint8 { Ground, Jump, Fall, Land, Takeoff, Hover, Flight, Brake, Carry, Throw, Attack, Dive };

/** Observed presentation only: movement, collision and damage remain owned by their components. */
struct FIBPlayPresentation
{
    EIBPlayMotion Motion = EIBPlayMotion::Ground;
    float Speed = 0;
    float GroundSpeed = 0;
    float BlendDirection = 0;
    float PlaybackRate = 1;
    int32 FlightPose = 0;
    FRotator VisualRotation = FRotator::ZeroRotator;
    uint32 Transition = 0;
};

/** Production pawn reuses S3 radial CMC and S2 presentation with one common action set. */
UCLASS()
class IMPACTBEYOND_API AIBPlayCharacter : public AIBS3Character
{
    GENERATED_BODY()
public:
    AIBPlayCharacter(const FObjectInitializer& Init);
    virtual void BeginPlay() override;
    virtual void Tick(float Dt) override;
    virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
    virtual bool PrepareMovement() override;
    virtual FRotator GetBaseAimRotation() const override;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UIBS35GripComponent> Grip;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UIBS35PunchComponent> Punches;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UIBS6HBridge> Heroes;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UIBS7TWheelComponent> Wheel;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UIBS4ResponseComponent> Response;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UIBPlayCombat> Combat;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UIBS4PowerComponent> Powers;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UIBS4PresentationComponent> Effects;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UImpactFlightFeedback> FlightFeedback;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UImpactLandingComponent> Landing;
    UPROPERTY(VisibleAnywhere) TObjectPtr<USkeletalMeshComponent> HeroVisual;
    UPROPERTY() TObjectPtr<UIBPlayArt> Art;
    UPROPERTY() TObjectPtr<AIBPlaySession> Session;
    int32 FlightGear = 1;
    bool bHelp = false;
    bool bSettings = false;
    FString Notice;
    float NoticeSeconds = 0;
    bool IsGameplayLocked() const;
    FString GetMovementStatusText() const;
    const FIBPlayPresentation& GetPresentation() const { return Presentation; }
    void ShowNotice(const FString& Text);
    bool DispatchS35(const FIBS35DamageMessage& Message,const FHitResult& Hit);
    bool DispatchS4(const FIBS4Hit& Message,const FHitResult& Hit);
    void RefreshWheel();
    UFUNCTION() void HeroChanged(int32 Id);
protected:
    virtual double RequestedFlightSpeed() const override;
    virtual void ReadDigitalMovement(APlayerController* Player) override;
private:
    friend struct FIBPlayPresentationFixture;
    UPROPERTY() TArray<TObjectPtr<UAnimSequence>> FlightPoses;
    UPROPERTY() TObjectPtr<UBlendSpace> GroundLocomotion;
    UPROPERTY() TObjectPtr<UAnimSequence> IdlePose;
    UPROPERTY() TObjectPtr<UAnimSequence> WalkPose;
    UPROPERTY() TObjectPtr<UAnimSequence> RunPose;
    UPROPERTY() TObjectPtr<UAnimSequence> CarryPose;
    UPROPERTY() TObjectPtr<UAnimSequence> BrakePose;
    UPROPERTY() TObjectPtr<UAnimSequence> ThrowPose;
    UPROPERTY() TObjectPtr<UAnimationAsset> Playing;
    UPROPERTY() TArray<TObjectPtr<UAnimSequence>> AttackPoses;
    UPROPERTY() TObjectPtr<UAnimSequence> HeavyPose;
    UPROPERTY() TObjectPtr<UAnimSequence> HoverPose;
    UPROPERTY() TObjectPtr<UAnimSequence> DivePose;
    UPROPERTY() TObjectPtr<UAnimSequence> JumpPose;
    UPROPERTY() TObjectPtr<UAnimSequence> FallPose;
    UPROPERTY() TObjectPtr<UAnimSequence> LandPose;
    UPROPERTY() TObjectPtr<UAnimSequence> TakeoffPose;
    UPROPERTY() TObjectPtr<UAnimSequence> CarryTakeoffPose;
    FIBPlayPresentation Presentation;
    FQuat VisualFacing = FQuat::Identity;
    FQuat PreviousPresentationFrame = FQuat::Identity;
    FVector PreviousPresentationUp = FVector::UpVector;
    FVector PreviousTravelDirection = FVector::ZeroVector;
    EMovementMode PreviousPresentationMode = MOVE_None;
    float FlightBank = 0;
    float TakeoffRemaining = 0;
    float LandRemaining = 0;
    uint32 AttackSerial = 0;
    uint32 PresentedAttackSerial = 0;
    uint32 ThrowSerial = 0;
    uint32 PresentedThrowSerial = 0;
    bool bUsingTravelPose = false;
    float WheelRefresh = 0;
    float ThrowAnimation = 0;
    bool bTurbo = false;
    bool bLaserActive = false;
    bool bSelectorWasOpen = false;
    bool bChargingPunch = false;
    float PunchCharge = 0;
    bool bDivePadLatch = false;
    bool HostAbility(int32 Id);
    void ReleaseHostAbility(int32 Id);
    void UpdateAnimation(float Dt);
    void UpdatePresentationCamera(float Dt);
    UFUNCTION() void OnPunchCue(FName Phase, float NormalizedTime);
    void Flight();
    void GearUp();
    void GearDown();
    void Travel();
    void Grab();
    bool GrabAimedPart(bool bTelekinesis);
    void ThrowStart();
    void ThrowRelease();
    void Activate();
    void Release();
    void Quick();
    void Heavy();
    void OpenWheelMouse();
    void CloseWheelMouse();
    void OpenWheelPad();
    void CloseWheelPad();
    void Selector();
    void Back();
    void SaveGame();
    void InteractWorld();
    void ToggleHelp();
    void NextDestination();
    void GlobalMap();
    void ClockSpeed();
    void ClockHour();
    void Consequences();
    void JumpPressed();
    void JumpReleased();
    void SelectorPrevious();
    void SelectorNext();
    void PreviousPower();
    void ImpactDive();
    void LookX(float V);
    void LookY(float V);
    void StickX(float V);
    void StickY(float V);
};

UCLASS()
class IMPACTBEYOND_API AIBPlayHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
};

UCLASS()
class IMPACTBEYOND_API AIBPlayGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    AIBPlayGameMode();
    virtual void StartPlay() override;
};
