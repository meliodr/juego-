#include "ImpactGripMesh.h"

#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "ImpactCircuitComponent.h"
#include "TwoBoneIK.h"

void UImpactGripMesh::SetCircuit(UImpactCircuitComponent* InCircuit)
{
	Circuit = InCircuit;
}

void UImpactGripMesh::BeginFlightPoseBlend(float Duration)
{
	PreviousPose = GetComponentSpaceTransforms();
	BlendStart = GetWorld()->GetTimeSeconds();
	BlendDuration = Duration;
}

void UImpactGripMesh::FinalizeBoneTransform()
{
	if (bNeedToFlipSpaceBaseBuffers && GetSkeletalMeshAsset())
	{
		TArray<FTransform>& Pose = GetEditableComponentSpaceTransforms();
		if (!PreviousPose.IsEmpty())
		{
			const float Elapsed = static_cast<float>(GetWorld()->GetTimeSeconds() - BlendStart);
			const float Alpha = FMath::SmoothStep(0.0f, BlendDuration, Elapsed);
			if (Alpha >= 1.0f || Pose.Num() != PreviousPose.Num())
			{
				PreviousPose.Reset();
			}
			else
			{
				for (int32 Bone = 0; Bone < Pose.Num(); ++Bone)
				{
					Pose[Bone].Blend(PreviousPose[Bone], FTransform(Pose[Bone]), Alpha);
				}
			}
		}

		FVector Left, Right;
		if (Circuit && Circuit->GetGripTargets(Left, Right))
		{
			const FReferenceSkeleton& Skeleton = GetSkeletalMeshAsset()->GetRefSkeleton();
			const float Weight = Circuit->GetGripWeight();
			SolveArm(Pose, Skeleton, TEXT("upperarm_l"), TEXT("lowerarm_l"), TEXT("hand_l"), Left, -1.0f, Weight);
			SolveArm(Pose, Skeleton, TEXT("upperarm_r"), TEXT("lowerarm_r"), TEXT("hand_r"), Right, 1.0f, Weight);
		}
	}
	Super::FinalizeBoneTransform();
}

void UImpactGripMesh::SolveArm(TArray<FTransform>& Pose, const FReferenceSkeleton& Skeleton,
	FName UpperName, FName LowerName, FName HandName, const FVector& Target, float Side, float Weight)
{
	const int32 Upper = Skeleton.FindBoneIndex(UpperName);
	const int32 Lower = Skeleton.FindBoneIndex(LowerName);
	const int32 Hand = Skeleton.FindBoneIndex(HandName);
	if (!Pose.IsValidIndex(Upper) || !Pose.IsValidIndex(Lower) || !Pose.IsValidIndex(Hand))
	{
		return;
	}

	const FTransform MeshToWorld = GetComponentTransform();
	const FVector Effector = FMath::Lerp(Pose[Hand].GetLocation(), MeshToWorld.InverseTransformPosition(Target), Weight);
	const AActor* Owner = GetOwner();
	const FVector ElbowOffset = Owner->GetActorRightVector() * Side * 48.0f - FVector::UpVector * 32.0f
		+ Owner->GetActorForwardVector() * 8.0f;
	const FVector Pole = Pose[Upper].GetLocation() + MeshToWorld.InverseTransformVectorNoScale(ElbowOffset);
	const TArray<FTransform> Original = Pose;
	AnimationCore::SolveTwoBoneIK(Pose[Upper], Pose[Lower], Pose[Hand], Pole, Effector, false, 1.0f, 1.0f);

	TBitArray<> Changed(false, Pose.Num());
	Changed[Upper] = true;
	Changed[Lower] = true;
	Changed[Hand] = true;
	for (int32 Bone = Upper + 1; Bone < FMath::Min(Pose.Num(), Skeleton.GetNum()); ++Bone)
	{
		const int32 Parent = Skeleton.GetParentIndex(Bone);
		if (Parent >= 0 && Changed[Parent] && !Changed[Bone])
		{
			Pose[Bone] = Original[Bone].GetRelativeTransform(Original[Parent]) * Pose[Parent];
			Changed[Bone] = true;
		}
	}
}
