#if WITH_DEV_AUTOMATION_TESTS

#include "Math/RotationMatrix.h"
#include "Math/Transform.h"
#include "Misc/AutomationTest.h"

namespace
{
	// Centimeters. The small radius makes curvature observable over a short route.
	constexpr double LabRadius = 100000.0;
	constexpr double GravityStrength = 980.0;
	const FVector LabCenter(1.2e9, -2.4e9, 3.6e9);

	FVector RadialUp(const FVector& Position)
	{
		return (Position - LabCenter).GetSafeNormal();
	}

	FVector GravityAt(const FVector& Position)
	{
		return -GravityStrength * RadialUp(Position);
	}

	double SurfaceHeight(const FVector& Position)
	{
		return FVector::Distance(Position, LabCenter) - LabRadius;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FImpactRadialLab, "ImpactBeyond.Planet.RadialLab",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FImpactRadialLab::RunTest(const FString& Parameters)
{
	const FVector North = LabCenter + FVector::UpVector * LabRadius;
	const FVector Equator = LabCenter + FVector::ForwardVector * LabRadius;
	const FVector South = LabCenter - FVector::UpVector * LabRadius;
	TestTrue(TEXT("Gravity at the north pole points down"), GravityAt(North).Equals(FVector(0, 0, -GravityStrength)));
	TestTrue(TEXT("Gravity at the equator points toward the center on X"), GravityAt(Equator).Equals(FVector(-GravityStrength, 0, 0)));
	TestTrue(TEXT("Gravity reverses at the antipode"), GravityAt(South).Equals(FVector(0, 0, GravityStrength)));
	TestTrue(TEXT("The singular center has a finite, zero gravity fallback"), GravityAt(LabCenter).IsZero());

	const FTransform RegionFrame(FRotationMatrix::MakeFromZX(RadialUp(Equator), -FVector::UpVector).ToQuat(), Equator);
	const FVector LocalPoint(817.235, -344.567, 456.789);
	TestTrue(TEXT("Region coordinates round trip at a large origin within 0.01 cm"),
		RegionFrame.InverseTransformPosition(RegionFrame.TransformPosition(LocalPoint)).Equals(LocalPoint, 0.01));
	TestTrue(TEXT("Regional up opposes gravity"),
		RegionFrame.TransformVectorNoScale(FVector::UpVector).Equals(-GravityAt(Equator) / GravityStrength));

	constexpr double ArcAngle = 0.20;
	const FVector AtArc = LabCenter + LabRadius * FVector(FMath::Sin(ArcAngle), 0.0, FMath::Cos(ArcAngle));
	const double TangentPlaneDrop = FVector::DotProduct(North - AtArc, RadialUp(North));
	TestTrue(TEXT("A 200 m arc stays on the curved surface"), FMath::IsNearlyZero(SurfaceHeight(AtArc), 0.01));
	TestTrue(TEXT("The surface drops about 20 m away from the initial tangent plane"),
		TangentPlaneDrop > 1990.0 && TangentPlaneDrop < 2000.0);
	TestTrue(TEXT("Surface normals rotate by the arc angle"),
		FMath::IsNearlyEqual(FVector::DotProduct(RadialUp(North), RadialUp(AtArc)), FMath::Cos(ArcAngle), 1.e-6));

	// Fixed-step free fall from 20 m at the equator. The first crossing must hit the curved floor.
	constexpr double StartHeight = 2000.0;
	constexpr double StepSeconds = 1.0 / 60.0;
	FVector Position = Equator + RadialUp(Equator) * StartHeight;
	FVector Velocity = FVector::ZeroVector;
	double Elapsed = 0.0;
	while (SurfaceHeight(Position) > 0.0 && Elapsed < 3.0)
	{
		Velocity += GravityAt(Position) * StepSeconds;
		Position += Velocity * StepSeconds;
		Elapsed += StepSeconds;
	}
	const double IdealFallSeconds = FMath::Sqrt(2.0 * StartHeight / GravityStrength);
	TestTrue(TEXT("The equatorial fall crosses the surface near the analytic fall time"),
		SurfaceHeight(Position) <= 0.0 && FMath::Abs(Elapsed - IdealFallSeconds) <= StepSeconds);
	TestTrue(TEXT("The fall crosses below the region origin with no lateral drift"),
		RadialUp(Position).Equals(RadialUp(Equator)) && Velocity.X < 0.0 && Velocity.Y == 0.0 && Velocity.Z == 0.0);
	return true;
}

#endif
