#pragma once

#include "Components/SkeletalMeshComponent.h"
#include "ImpactGripMesh.generated.h"

class UImpactCircuitComponent;

UCLASS()
class UImpactGripMesh : public USkeletalMeshComponent
{
	GENERATED_BODY()

public:
	void SetCircuit(UImpactCircuitComponent* InCircuit);
	void BeginFlightPoseBlend(float Duration = 0.18f);
	virtual void FinalizeBoneTransform() override;

private:
	void SolveArm(TArray<FTransform>& Pose, const FReferenceSkeleton& Skeleton,
		FName UpperName, FName LowerName, FName HandName, const FVector& Target, float Side, float Weight);

	UPROPERTY(Transient)
	TObjectPtr<UImpactCircuitComponent> Circuit;
	TArray<FTransform> PreviousPose;
	double BlendStart = 0.0;
	float BlendDuration = 0.18f;
};
