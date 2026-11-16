#include "ImpactGripMesh.h"

#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "ImpactCircuitComponent.h"
#include "IB_S35/IBS35GripComponent.h"
#include "TwoBoneIK.h"

void UImpactGripMesh::SetCircuit(UImpactCircuitComponent* InCircuit)
{
	Circuit = InCircuit;
}

void UImpactGripMesh::BeginFlightPoseBlend(float Duration)
{
	if (!GetWorld() || !GetSkeletalMeshAsset() || !FMath::IsFinite(Duration) || Duration <= 0)
	{
		PreviousPose.Reset();
		PreviousPoseMesh.Reset();
		return;
	}
	PreviousPose = GetComponentSpaceTransforms();
	PreviousPoseMesh = GetSkeletalMeshAsset();
	BlendStart = GetWorld()->GetTimeSeconds();
	BlendDuration = Duration;
}

bool UImpactGripMesh::BlendPoseInLocalSpace(const FReferenceSkeleton& Skeleton,
	const TArray<FTransform>& FromComponentPose, TArray<FTransform>& ToComponentPose, float Alpha)
{
	const int32 Count = ToComponentPose.Num();
	if (!FMath::IsFinite(Alpha) || Count == 0 || FromComponentPose.Num() != Count || Skeleton.GetNum() != Count)
	{
		return false;
	}
	for (int32 Bone = 0; Bone < Count; ++Bone)
	{
		const int32 Parent = Skeleton.GetParentIndex(Bone);
		if ((Parent != INDEX_NONE && (Parent < 0 || Parent >= Bone))
			|| FromComponentPose[Bone].ContainsNaN() || ToComponentPose[Bone].ContainsNaN())
		{
			return false;
		}
	}
	if (Alpha <= 0) { ToComponentPose = FromComponentPose; return true; }
	if (Alpha >= 1) return true;

	// Work from leaves to root so each incoming parent is still in component
	// space when its child is converted. Interpolating component-space positions
	// cuts across a rotating joint's arc and visibly shortens the bone chain.
	for (int32 Bone = Count - 1; Bone >= 0; --Bone)
	{
		const int32 Parent = Skeleton.GetParentIndex(Bone);
		const FTransform FromLocal = Parent == INDEX_NONE ? FromComponentPose[Bone]
			: FromComponentPose[Bone].GetRelativeTransform(FromComponentPose[Parent]);
		const FTransform ToLocal = Parent == INDEX_NONE ? ToComponentPose[Bone]
			: ToComponentPose[Bone].GetRelativeTransform(ToComponentPose[Parent]);
		ToComponentPose[Bone].Blend(FromLocal, ToLocal, Alpha);
	}
	// Restore component space in parent-first order. Preserve authored local
	// translation/scale changes; do not force every animation into the bind pose.
	for (int32 Bone = 0; Bone < Count; ++Bone)
	{
		const int32 Parent = Skeleton.GetParentIndex(Bone);
		if (Parent != INDEX_NONE) ToComponentPose[Bone] *= ToComponentPose[Parent];
	}
	return true;
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
			if (Alpha >= 1.0f || PreviousPoseMesh.Get() != GetSkeletalMeshAsset()
				|| !BlendPoseInLocalSpace(GetSkeletalMeshAsset()->GetRefSkeleton(), PreviousPose, Pose, Alpha))
			{
				PreviousPose.Reset();
				PreviousPoseMesh.Reset();
			}
		}

		FVector Left, Right;
		if ((SharedGrip && SharedGrip->GetGripTargets(Left, Right)) || (Circuit && Circuit->GetGripTargets(Left, Right)))
		{
			const FReferenceSkeleton& Skeleton = GetSkeletalMeshAsset()->GetRefSkeleton();
			const float Weight = SharedGrip ? SharedGrip->GetGripWeight() : Circuit->GetGripWeight();
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
	const FVector ElbowOffset = Owner->GetActorRightVector() * Side * 48.0f - Owner->GetActorUpVector() * 32.0f
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
