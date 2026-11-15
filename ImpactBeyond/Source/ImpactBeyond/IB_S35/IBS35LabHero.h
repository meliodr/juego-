#pragma once
#include "CoreMinimal.h"
#include "ImpactBeyondCharacter.h"
#include "IBS35LabHero.generated.h"

UCLASS()
class IMPACTBEYOND_API AIBS35LabHero : public AImpactBeyondCharacter
{
    GENERATED_BODY()
public:
    AIBS35LabHero(const FObjectInitializer& Initializer);
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<class UIBS35GripComponent> LabGrip;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<class UIBS35PunchComponent> LabPunch;
protected:
    virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void UnPossessed() override;
private:
    void ToggleGrip();
    void Charge();
    void Throw();
    void QuickHit();
    void HeavyHit();
    void Cancel();
    void ResetLab();
};
