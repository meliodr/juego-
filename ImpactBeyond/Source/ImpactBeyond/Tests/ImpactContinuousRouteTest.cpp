#include "ImpactBeyondCharacter.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformTime.h"
#include "InputKeyEventArgs.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FImpactContinuousRouteTest, "ImpactBeyond.Integration.ContinuousRoute",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FImpactContinuousRouteTest::RunTest(const FString& Parameters)
{
    class FRoute final : public IAutomationLatentCommand
    {
    public:
        explicit FRoute(FAutomationTestBase& InTest) : Test(InTest)
        {
            FParse::Value(FCommandLine::Get(), TEXT("IBEvidenceDir="), Directory);
            if (Directory.IsEmpty()) { Directory = FPaths::ProjectSavedDir() / TEXT("ContinuousRoute"); }
            IFileManager::Get().MakeDirectory(*Directory, true);
        }
        bool Update() override
        {
            UWorld* World = GEngine->GetCurrentPlayWorld();
            APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
            AImpactBeyondCharacter* Hero = PC ? Cast<AImpactBeyondCharacter>(PC->GetPawn()) : nullptr;
            if (!Hero || !World->GetMapName().EndsWith(TEXT("Lvl_IB_C00")))
            {
                Test.AddError(TEXT("Continuous route requires the possessed IB_C00 hero"));
                return true;
            }
            auto* Movement = Hero->GetCharacterMovement();
            auto Key = [PC](FKey K, bool Held) { PC->InputKey(FInputKeyEventArgs::CreateSimulated(K, Held ? IE_Pressed : IE_Released, Held ? 1.f : 0.f)); };
            auto Tap = [&Key](FKey K) { Key(K, true); Key(K, false); };
            const double Now = FPlatformTime::Seconds();
            const FVector Position = Hero->GetActorLocation();
            if (Start == 0)
            {
                Start = StageStart = Now;
                Initial = Previous = Position;
                PC->SetControlRotation(FRotator(-8, 0, 0));
                Test.AddInfo(FString::Printf(TEXT("CONTINUOUS start %s; no test position or velocity writes"), *Position.ToString()));
            }
            if (Now - LastSample >= 0.2)
            {
                Trace += FString::Printf(TEXT("%.3f,%d,%.3f,%.3f,%.3f,%.3f,%d\n"), Now-Start, Stage, Position.X, Position.Y, Position.Z, Movement->Velocity.Size(), int32(Movement->MovementMode));
                LastSample = Now;
            }
            if (FVector::Dist(Position, Previous) > 5000)
            {
                Test.AddError(TEXT("Unexpected position discontinuity over 50 m in one automation update"));
                return Finish();
            }
            Previous = Position;
            if (Now-Start > 180 || Now-StageStart > 45)
            {
                Test.AddError(FString::Printf(TEXT("Continuous route timed out at stage %d position %s"), Stage, *Position.ToString()));
                Key(EKeys::C,false); Key(EKeys::X,false);
                return Finish();
            }
            auto Advance = [&](const TCHAR* Label)
            {
                Test.AddInfo(FString::Printf(TEXT("CONTINUOUS %s at %.3fs position %s speed %.2f"), Label, Now-Start, *Position.ToString(), Movement->Velocity.Size()));
                FScreenshotRequest::RequestScreenshot(Directory / FString::Printf(TEXT("%02d-%s.png"), Stage, Label), true, false);
                ++Stage; StageStart=Now;
            };
            auto FlyToward = [&](const FVector& Target, double StopDistance)
            {
                const FVector Delta=Target-Position;
                PC->SetControlRotation(FRotator(-8, Delta.Rotation().Yaw, 0));
                if (Delta.Size() <= StopDistance) { Key(EKeys::X,true); return true; }
                Hero->AddMovementInput(Delta.GetSafeNormal());
                return false;
            };
            switch (Stage)
            {
            case 0:
                if (Now-Start < 3) { break; }
                Hero->AddMovementInput(FVector::ForwardVector);
                if (Position.X-Initial.X > 2000)
                {
                    Test.TestTrue(TEXT("Walk segment stayed grounded"),Movement->IsMovingOnGround());
                    Tap(EKeys::F); Advance(TEXT("walk-takeoff"));
                }
                break;
            case 1:
                if (Movement->IsFlying()) { Hero->AddMovementInput(FVector::UpVector); }
                if (Position.Z > 1400) { Key(EKeys::X,true); Advance(TEXT("airborne")); }
                break;
            case 2:
                if (Movement->Velocity.Size()<30 && Now-StageStart>2)
                {
                    Test.TestTrue(TEXT("Hover is airborne above street"), Movement->IsFlying() && Position.Z>1200);
                    Key(EKeys::X,false); Tap(EKeys::E); Advance(TEXT("hover"));
                }
                break;
            case 3:
                if (FlyToward(FVector(24000,0,1600),1000)) { Advance(TEXT("avenue-brake")); }
                break;
            case 4:
                if (Movement->Velocity.Size()<30) { Key(EKeys::X,false); Advance(TEXT("avenue-stopped")); }
                break;
            case 5:
                if (FlyToward(FVector(24800,6000,3600),500)) { Advance(TEXT("roof-approach")); }
                break;
            case 6:
                if (Movement->Velocity.Size()<30) { Key(EKeys::X,false); Key(EKeys::C,true); Advance(TEXT("roof-descent")); }
                break;
            case 7:
                if (Movement->IsMovingOnGround())
                {
                    Key(EKeys::C,false);
                    Test.TestTrue(TEXT("Landed on elevated roof"), Position.Z>2000 && FMath::Abs(Position.X-24800)<2000 && FMath::Abs(Position.Y-6000)<2000);
                    Advance(TEXT("roof-contact"));
                }
                break;
            case 8:
                if (Now-StageStart>2) { Tap(EKeys::F); Advance(TEXT("roof-takeoff")); }
                break;
            case 9:
                if (Movement->IsFlying()) { Hero->AddMovementInput(FVector::UpVector); }
                if (Position.Z>10000) { Key(EKeys::X,true); Advance(TEXT("city-clearance")); }
                break;
            case 10:
                if (Movement->Velocity.Size()<30)
                {
                    Key(EKeys::X,false); Tap(EKeys::E); Tap(EKeys::E); Advance(TEXT("coast-flight"));
                }
                break;
            case 11:
                if (FlyToward(FVector(165000,0,10000),5200)) { Advance(TEXT("coast-brake")); }
                break;
            case 12:
                if (Movement->Velocity.Size()<30)
                {
                    Test.TestTrue(TEXT("Reached coast continuously"),Position.X>150000 && Movement->IsFlying());
                    Advance(TEXT("coast-hover"));
                }
                break;
            case 13:
                if (Now-StageStart>2) { Key(EKeys::X,false); return Finish(); }
                break;
            }
            return false;
        }
    private:
        bool Finish() { FFileHelper::SaveStringToFile(*Trace, *(Directory/TEXT("continuous-trace.csv"))); return true; }
        FAutomationTestBase& Test;
        FString Directory;
        FString Trace=TEXT("seconds,stage,x_cm,y_cm,z_cm,speed_cm_s,movement_mode\n");
        FVector Initial=FVector::ZeroVector, Previous=FVector::ZeroVector;
        double Start=0, StageStart=0, LastSample=0;
        int32 Stage=0;
    };
    FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShared<FRoute>(*this));
    return true;
}
#endif
