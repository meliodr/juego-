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
    bool bHelp = true;
    bool bSettings = false;
    FString Notice;
    float NoticeSeconds = 0;
    bool IsGameplayLocked() const;
    void ShowNotice(const FString& Text);
    bool DispatchS35(const FIBS35DamageMessage& Message,const FHitResult& Hit);
    bool DispatchS4(const FIBS4Hit& Message,const FHitResult& Hit);
    void RefreshWheel();
    UFUNCTION() void HeroChanged(int32 Id);
protected:
    virtual double RequestedFlightSpeed() const override;
    virtual void ReadDigitalMovement(APlayerController* Player) override;
private:
    UPROPERTY() TArray<TObjectPtr<UAnimSequence>> FlightPoses;
    UPROPERTY() TObjectPtr<UAnimSequence> IdlePose;
    UPROPERTY() TObjectPtr<UAnimSequence> WalkPose;
    UPROPERTY() TObjectPtr<UAnimSequence> RunPose;
    UPROPERTY() TObjectPtr<UAnimSequence> CarryPose;
    UPROPERTY() TObjectPtr<UAnimSequence> BrakePose;
    UPROPERTY() TObjectPtr<UAnimSequence> ThrowPose;
    UPROPERTY() TObjectPtr<UAnimSequence> Playing;
    UPROPERTY() TArray<TObjectPtr<UAnimSequence>> AttackPoses;
    UPROPERTY() TObjectPtr<UAnimSequence> HeavyPose;
    UPROPERTY() TObjectPtr<UAnimSequence> HoverPose;
    UPROPERTY() TObjectPtr<UAnimSequence> DivePose;
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
