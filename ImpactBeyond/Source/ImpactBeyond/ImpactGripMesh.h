#pragma once

#include "Components/SkeletalMeshComponent.h"
#include "ImpactGripMesh.generated.h"

class UImpactCircuitComponent;
class UIBS35GripComponent;
struct FReferenceSkeleton;

UCLASS()
class UImpactGripMesh : public USkeletalMeshComponent
{
	GENERATED_BODY()

public:
	void SetCircuit(UImpactCircuitComponent* InCircuit);
    void SetSharedGrip(UIBS35GripComponent* InGrip) { SharedGrip = InGrip; }
	void BeginFlightPoseBlend(float Duration = 0.18f);
	virtual void FinalizeBoneTransform() override;
	/** Blend local bone channels, then rebuild component space for the existing grip IK. */
	static bool BlendPoseInLocalSpace(const FReferenceSkeleton& Skeleton,
		const TArray<FTransform>& FromComponentPose, TArray<FTransform>& ToComponentPose, float Alpha);

private:
	void SolveArm(TArray<FTransform>& Pose, const FReferenceSkeleton& Skeleton,
		FName UpperName, FName LowerName, FName HandName, const FVector& Target, float Side, float Weight);

	UPROPERTY(Transient)
	TObjectPtr<UImpactCircuitComponent> Circuit;
    UPROPERTY(Transient) TObjectPtr<UIBS35GripComponent> SharedGrip;
	TArray<FTransform> PreviousPose;
	TWeakObjectPtr<USkeletalMesh> PreviousPoseMesh;
	double BlendStart = 0.0;
	float BlendDuration = 0.18f;
};
