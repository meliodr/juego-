#include "ImpactGripMesh.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "ReferenceSkeleton.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FImpactGripPoseBlendTest, "ImpactBeyond.Presentation.LocalPoseBlend",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FImpactGripPoseBlendTest::RunTest(const FString&)
{
	// Synthetic skeleton only: no world, licensed mesh, animation or render dependency.
	FReferenceSkeleton Skeleton;
	{
		FReferenceSkeletonModifier Edit(Skeleton, nullptr);
		Edit.Add(FMeshBoneInfo(TEXT("root"), TEXT("root"), INDEX_NONE), FTransform::Identity);
		Edit.Add(FMeshBoneInfo(TEXT("upper"), TEXT("upper"), 0), FTransform(FVector(0, 0, 20)));
		Edit.Add(FMeshBoneInfo(TEXT("lower"), TEXT("lower"), 1), FTransform(FVector(40, 0, 0)));
		Edit.Add(FMeshBoneInfo(TEXT("hand"), TEXT("hand"), 2), FTransform(FVector(30, 0, 0)));
		Edit.Add(FMeshBoneInfo(TEXT("finger"), TEXT("finger"), 3), FTransform(FVector(8, 0, 0)));
	}
	TArray<FTransform> From = Skeleton.GetRefBonePose();
	TArray<FTransform> To = From;
	From[0].SetTranslation(FVector(10, 20, 30));
	To[0].SetTranslation(FVector(20, 40, 60));
	To[0].SetRotation(FQuat(FVector::UpVector, FMath::DegreesToRadians(45.0)));
	To[1].SetRotation(FQuat(FVector::RightVector, FMath::DegreesToRadians(95.0)));
	To[2].SetRotation(FQuat(FVector::UpVector, FMath::DegreesToRadians(90.0)));
	To[3].SetRotation(FQuat(FVector::ForwardVector, FMath::DegreesToRadians(-40.0)));
	const auto ToComponent = [&Skeleton](TArray<FTransform>& Pose)
	{
		for (int32 Bone = 1; Bone < Pose.Num(); ++Bone) Pose[Bone] *= Pose[Skeleton.GetParentIndex(Bone)];
	};
	ToComponent(From);
	ToComponent(To);
	for (int32 Step = 0; Step <= 10; ++Step)
	{
		TArray<FTransform> Actual = To;
		TestTrue(TEXT("Valid hierarchical pose blends"), UImpactGripMesh::BlendPoseInLocalSpace(Skeleton, From, Actual, Step / 10.f));
		for (int32 Bone = 1; Bone < Actual.Num(); ++Bone)
		{
			const int32 Parent = Skeleton.GetParentIndex(Bone);
			const FVector LocalOffset = Skeleton.GetRefBonePose()[Bone].GetTranslation();
			TestTrue(FString::Printf(TEXT("step %d bone %d retains its segment length"), Step, Bone),
				FMath::IsNearlyEqual(FVector::Distance(Actual[Bone].GetTranslation(), Actual[Parent].GetTranslation()), LocalOffset.Size(), 0.001));
			TestTrue(FString::Printf(TEXT("step %d bone %d follows the blended parent frame"), Step, Bone),
				Actual[Bone].GetRelativeTransform(Actual[Parent]).GetTranslation().Equals(LocalOffset, 0.001));
		}
		if (Step == 0 || Step == 10)
		{
			const auto& Expected = Step == 0 ? From : To;
			for (int32 Bone = 0; Bone < Actual.Num(); ++Bone)
				TestTrue(TEXT("Blend endpoints preserve the complete input pose"), Actual[Bone].Equals(Expected[Bone], 0.001));
		}
	}
	TArray<FTransform> ShortPose = From;
	ShortPose.Pop();
	TArray<FTransform> Unchanged = To;
	TestFalse(TEXT("Mismatched skeleton pose is rejected"), UImpactGripMesh::BlendPoseInLocalSpace(Skeleton, ShortPose, Unchanged, 0.5f));
	for (int32 Bone = 0; Bone < To.Num(); ++Bone)
		TestTrue(TEXT("Rejected blend leaves the new animation intact"), Unchanged[Bone].Equals(To[Bone], 0.001));
	return true;
}
#endif
