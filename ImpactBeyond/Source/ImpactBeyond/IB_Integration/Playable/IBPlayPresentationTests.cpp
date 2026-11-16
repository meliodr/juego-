#if WITH_DEV_AUTOMATION_TESTS
#include "IBPlayCharacter.h"
#include "IBPlaySession.h"
#include "IB_S35/IBS35GripComponent.h"
#include "IB_S35/IBS35PunchComponent.h"
#include "ImpactLandingComponent.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Animation/BlendSpace.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "HAL/PlatformTime.h"
#include "Math/RotationMatrix.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

/** Runs the production pawn/assets across real game frames. Physics is held still to isolate
 * animation from encounter contacts; RealInput and PlayableSession cover movement and damage.
 * These numerical checks do not judge the artistic quality of a rendered animation. */
struct FIBPlayPresentationFixture final : public IAutomationLatentCommand
{
    struct FStep
    {
        FString Label;
        float Duration;
        TFunction<void()> Start;
        TFunction<void(float)> Frame;
        TFunction<void()> Check;
    };
    FAutomationTestBase& Test;
    TWeakObjectPtr<AIBPlayCharacter> Hero;
    TWeakObjectPtr<AActor> Floor;
    TArray<FStep> Steps;
    FTransform OriginalTransform, OriginalMesh;
    FVector OriginalVelocity, OriginalForward, OriginalUp, OriginalGravity, OriginalSocket;
    FQuat FrameRotation;
    FVector Position;
    EMovementMode OriginalMode=MOVE_None;
    float OriginalFOV=85, OriginalArm=420;
    int32 OriginalGear=1, Index=0;
    bool OriginalActorTick=true, OriginalMovementTick=true, OriginalPunchTick=true;
    bool OriginalBrake=false, OriginalTraversal=false, OriginalTurbo=false;
    bool Initialized=false, Restored=false, Failed=false;
    float Elapsed=0, TurnAngle=0;
    uint32 FirstTransition=0, SavedTransition=0;
    uint64 LastFrame=MAX_uint64;
    double Started=FPlatformTime::Seconds();

    FIBPlayPresentationFixture(FAutomationTestBase& InTest,AIBPlayCharacter* InHero):Test(InTest),Hero(InHero) {}
    virtual ~FIBPlayPresentationFixture() override { Restore(); }
    AIBPlayCharacter* P() const { return Hero.Get(); }
    UAnimSingleNodeInstance* Animation() const { return P()->GetMesh()->GetSingleNodeInstance(); }
    void Expect(bool Value,const TCHAR* Label) { if(!Test.TestTrue(Label,Value)) Failed=true; }
    void Motion(EIBPlayMotion Value,UAnimationAsset* Asset,bool Loop)
    {
        Expect(P()->Presentation.Motion==Value,TEXT("Production motion state matches physical mode"));
        Expect(Animation()&&Animation()->GetCurrentAsset()==Asset,TEXT("Actual single-node instance plays the expected cooked asset"));
        if(Animation()) Expect(Animation()->IsLooping()==Loop,TEXT("Actual animation loop flag matches the action"));
    }
    void Mode(EMovementMode Value,const FVector& LocalVelocity)
    {
        P()->GetCharacterMovement()->SetMovementMode(Value);
        P()->GetCharacterMovement()->Velocity=FrameRotation.RotateVector(LocalVelocity);
    }
    void SetFrame(const FQuat& Rotation)
    {
        const FVector Local=FrameRotation.UnrotateVector(P()->GetVelocity());
        FrameRotation=Rotation;
        P()->SetActorLocationAndRotation(Position,Rotation,false,nullptr,ETeleportType::TeleportPhysics);
        P()->GetCharacterMovement()->SetGravityDirection(-Rotation.GetUpVector());
        P()->GetCharacterMovement()->Velocity=Rotation.RotateVector(Local);
        P()->TangentForward=Rotation.GetForwardVector(); P()->PreviousUp=Rotation.GetUpVector();
        if(Floor.IsValid()) Floor->SetActorLocationAndRotation(Position-Rotation.GetUpVector()*1000,Rotation);
    }
    void Add(const TCHAR* Label,float Duration,TFunction<void()> Start,TFunction<void()> Check,TFunction<void(float)> Frame={})
    { Steps.Add({Label,Duration,MoveTemp(Start),MoveTemp(Frame),MoveTemp(Check)}); }
    bool Initialize()
    {
        bool AssetsReady=P()->GroundLocomotion&&P()->JumpPose&&P()->FallPose&&P()->LandPose
            &&P()->TakeoffPose&&P()->CarryTakeoffPose&&P()->BrakePose&&P()->DivePose&&P()->HeavyPose
            &&P()->HoverPose&&P()->FlightPoses.Num()==5&&P()->AttackPoses.Num()==3;
        for(const auto& Asset:P()->FlightPoses) AssetsReady&=Asset!=nullptr;
        for(const auto& Asset:P()->AttackPoses) AssetsReady&=Asset!=nullptr;
        if(!Test.TestTrue(TEXT("Production locomotion and action assets are loaded"),AssetsReady)) return false;
        auto* Move=P()->GetCharacterMovement();
        OriginalTransform=P()->GetActorTransform(); OriginalMesh=P()->GetMesh()->GetRelativeTransform();
        OriginalVelocity=Move->Velocity; OriginalMode=Move->MovementMode; OriginalGravity=Move->GetGravityDirection();
        OriginalForward=P()->TangentForward; OriginalUp=P()->PreviousUp;
        OriginalGear=P()->FlightGear; OriginalBrake=P()->bBrake; OriginalTraversal=P()->bTraversal; OriginalTurbo=P()->bTurbo;
        OriginalActorTick=P()->IsActorTickEnabled(); OriginalMovementTick=Move->IsComponentTickEnabled();
        OriginalPunchTick=P()->Punches->IsComponentTickEnabled();
        OriginalFOV=P()->Camera->FieldOfView; OriginalArm=P()->Arm->TargetArmLength; OriginalSocket=P()->Arm->SocketOffset;
        Initialized=true;
        P()->SetActorTickEnabled(false); Move->SetComponentTickEnabled(false); P()->Punches->SetComponentTickEnabled(false);
        P()->StopJumping(); P()->Punches->CancelAttack(); P()->Landing->CancelImpactDive();
        P()->bBrake=false; P()->bTraversal=false; P()->bTurbo=false;
        P()->ThrowAnimation=0; P()->TakeoffRemaining=0; P()->LandRemaining=0;
        P()->PreviousPresentationMode=MOVE_None; P()->VisualFacing=FQuat::Identity; P()->FlightBank=0;
        P()->PreviousTravelDirection=FVector::ZeroVector; P()->Playing=nullptr;
        FrameRotation=OriginalTransform.GetRotation(); Position=OriginalTransform.GetLocation()+FrameRotation.GetUpVector()*15000;
        Floor=P()->GetWorld()->SpawnActor<AActor>();
        if(!Floor.IsValid()) { Test.AddError(TEXT("Could not create an isolated dive contact fixture.")); return false; }
        UBoxComponent* Box=NewObject<UBoxComponent>(Floor.Get());
        Floor->SetRootComponent(Box); Floor->AddInstanceComponent(Box);
        Box->SetBoxExtent(FVector(1200,1200,50)); Box->SetCollisionProfileName(TEXT("BlockAll")); Box->RegisterComponent();
        SetFrame(FrameRotation); Mode(MOVE_Walking,FVector::ZeroVector);
        FirstTransition=P()->Presentation.Transition;
        Build(); return true;
    }
    void Build()
    {
        Add(TEXT("Idle"),.15f,{},[this](){ Motion(EIBPlayMotion::Ground,P()->GroundLocomotion,true); });
        Add(TEXT("Walk"),.25f,[this](){Mode(MOVE_Walking,FVector(300,0,0));},[this]()
        {
            Motion(EIBPlayMotion::Ground,P()->GroundLocomotion,true);
            FVector Input,Filtered; Animation()->GetBlendSpaceState(Input,Filtered);
            Expect(FMath::IsNearlyEqual(Input.Y,300.0,1.0)&&FMath::IsNearlyEqual(Animation()->GetPlayRate(),1.f),TEXT("Walk uses the measured 300 cm/s blend-space sample"));
        });
        Add(TEXT("Sprint"),.3f,[this](){Mode(MOVE_Walking,FVector(1800,0,0));},[this]()
        {
            Motion(EIBPlayMotion::Ground,P()->GroundLocomotion,true);
            FVector Input,Filtered; Animation()->GetBlendSpaceState(Input,Filtered);
            Expect(FMath::IsNearlyEqual(Input.Y,600.0,1.0)&&FMath::IsNearlyEqual(Animation()->GetPlayRate(),3.f),TEXT("1800 cm/s sprint retains speed and advances the 600 cm/s jog at rate three"));
        });
        Add(TEXT("Strafe entry"),.025f,[this](){Mode(MOVE_Walking,FVector(0,300,0));},[this]()
        { Expect(P()->Presentation.BlendDirection>10&&P()->Presentation.BlendDirection<90,TEXT("Strafe direction accounts for the still-turning visual heading")); });
        Add(TEXT("Strafe settled"),.55f,{},[this]()
        { Expect(FMath::Abs(P()->Presentation.BlendDirection)<2,TEXT("Visual faces travel and the blend-space converges to forward gait")); });
        Add(TEXT("Jump"),.15f,[this](){Mode(MOVE_Falling,FVector(300,0,650));},[this](){Motion(EIBPlayMotion::Jump,P()->JumpPose,false);});
        Add(TEXT("Fall"),.15f,[this](){Mode(MOVE_Falling,FVector(300,0,-650));},[this](){Motion(EIBPlayMotion::Fall,P()->FallPose,true);});
        Add(TEXT("Land"),.05f,[this](){Mode(MOVE_Walking,FVector(300,0,0));},[this](){Motion(EIBPlayMotion::Land,P()->LandPose,false);});
        Add(TEXT("Resume gait"),.35f,{},[this](){Motion(EIBPlayMotion::Ground,P()->GroundLocomotion,true);});
        Add(TEXT("Takeoff"),.07f,[this](){Mode(MOVE_Flying,FVector(1800,0,0));},[this]()
        {
            Motion(EIBPlayMotion::Takeoff,P()->TakeoffPose,false);
            Expect(!P()->IsGameplayLocked()&&P()->GetCharacterMovement()->IsFlying(),TEXT("Takeoff is presentation only and does not lock controls or movement"));
        });
        Add(TEXT("Attack during takeoff"),.04f,[this](){P()->Quick();},[this]()
        { Expect(P()->Punches->IsAttacking()&&P()->Presentation.Motion==EIBPlayMotion::Attack,TEXT("The production quick-input handler can interrupt takeoff immediately")); });
        Add(TEXT("Resume takeoff"),.04f,[this](){P()->Punches->CancelAttack();},[this](){Motion(EIBPlayMotion::Takeoff,P()->TakeoffPose,false);});
        Add(TEXT("Cruise"),.8f,{},[this]()
        {
            Motion(EIBPlayMotion::Flight,P()->FlightPoses[0],true);
            Expect(P()->Presentation.VisualRotation.Pitch<-55,TEXT("Cruise tilts the body around the capsule instead of keeping it upright"));
            Expect(P()->GetMesh()->GetRelativeLocation().Size()>95&&P()->GetMesh()->GetRelativeLocation().Size()<97,TEXT("Flight rotation preserves the capsule-centered mesh pivot radius"));
        });
        Add(TEXT("Hover"),.55f,[this](){Mode(MOVE_Flying,FVector::ZeroVector);},[this]()
        { Motion(EIBPlayMotion::Hover,P()->HoverPose,true); Expect(FMath::Abs(P()->Presentation.VisualRotation.Pitch)<2,TEXT("Hover returns the body upright")); });
        Add(TEXT("Traversal at gear one"),.55f,[this](){P()->FlightGear=0; P()->bTraversal=true; Mode(MOVE_Flying,FVector(22000,0,0));},[this]()
        {
            Motion(EIBPlayMotion::Flight,P()->FlightPoses[4],true);
            Expect(P()->GetMovementStatusText().Contains(TEXT("Travesia")),TEXT("HUD reports actual traversal state"));
            Expect(P()->Camera->FieldOfView>94&&P()->Camera->FieldOfView<=97.1f,TEXT("Real speed widens FOV smoothly within the production range"));
        });
        Add(TEXT("Brake entry"),.06f,[this](){P()->bBrake=true;},[this]()
        { Motion(EIBPlayMotion::Brake,P()->BrakePose,false); SavedTransition=P()->Presentation.Transition; });
        Add(TEXT("Brake hold"),.65f,{},[this]()
        {
            Motion(EIBPlayMotion::Brake,P()->BrakePose,false);
            Expect(P()->Presentation.Transition==SavedTransition&&Animation()->GetCurrentTime()>=P()->BrakePose->GetPlayLength()-.05f,TEXT("Held brake reaches its final frame without looping or restarting"));
        });
        Add(TEXT("Brake release"),.08f,[this](){P()->bBrake=false;},[this](){Motion(EIBPlayMotion::Flight,P()->FlightPoses[4],true);});
        Add(TEXT("Brake re-entry"),.025f,[this](){P()->bBrake=true;},[this]()
        { Motion(EIBPlayMotion::Brake,P()->BrakePose,false); Expect(Animation()->GetCurrentTime()<.15f,TEXT("A new brake action restarts the same finite clip")); });
        Add(TEXT("Banking turn"),.5f,[this](){P()->bBrake=false; P()->bTraversal=false; TurnAngle=0;},[this]()
        { Expect(FMath::Abs(P()->Presentation.VisualRotation.Roll)>5,TEXT("A genuine tangent-direction turn banks the body")); },[this](float Dt)
        { TurnAngle+=60*Dt; const FVector Travel=FRotator(0,TurnAngle,0).RotateVector(FVector(4000,0,0)); Mode(MOVE_Flying,Travel); });
        Add(TEXT("Steady flight"),.75f,[this](){Mode(MOVE_Flying,FVector(4000,0,0));},[this](){Expect(FMath::Abs(P()->Presentation.VisualRotation.Roll)<2,TEXT("Bank settles after a turn"));});
        Add(TEXT("Sideways local gravity"),.6f,[this]()
        { SetFrame(FQuat(FrameRotation.GetRightVector(),PI*.5)*FrameRotation); },[this]()
        {
            Expect(P()->Presentation.VisualRotation.Pitch<-55&&FMath::Abs(P()->Presentation.VisualRotation.Roll)<2,TEXT("Changing local gravity preserves lean without a false turn bank"));
            Expect(FMath::Abs(P()->Arm->GetRelativeRotation().Roll)<.01,TEXT("Body bank never rolls the camera arm"));
        });
        Add(TEXT("Attack one"),.07f,[this](){Expect(P()->Punches->BeginPunch(true),TEXT("Heavy action starts"));},[this]()
        { Motion(EIBPlayMotion::Attack,P()->HeavyPose,false); SavedTransition=P()->Presentation.Transition; });
        Add(TEXT("Repeated same attack"),.025f,[this](){P()->Punches->CancelAttack(); Expect(P()->Punches->BeginPunch(true),TEXT("Second heavy action starts"));},[this]()
        { Expect(P()->Presentation.Transition>SavedTransition&&Animation()->GetCurrentTime()<P()->HeavyPose->GetPlayLength()*.5f,TEXT("Windup cue restarts a repeated identical attack even without an idle presentation frame")); });
        Add(TEXT("Resume flight"),.2f,[this](){P()->Punches->CancelAttack();},[this](){Motion(EIBPlayMotion::Flight,P()->FlightPoses[1],true);});
        Add(TEXT("Dive"),.06f,[this]()
        {
            Expect(P()->Landing->StartImpactDive(),TEXT("Dive uses a real swept floor contact in local gravity"));
            P()->Landing->SetComponentTickEnabled(false);
        },[this](){Motion(EIBPlayMotion::Dive,P()->DivePose,false); SavedTransition=P()->Presentation.Transition;});
        Add(TEXT("Dive hold"),.65f,{},[this]()
        {
            Expect(P()->Presentation.Transition==SavedTransition&&Animation()->GetCurrentTime()>=P()->DivePose->GetPlayLength()-.05f,TEXT("Dive holds its braced final pose instead of repeating the windup"));
            const float ExpectedFOV=85+12*FMath::Sqrt(FMath::Clamp(float(P()->GetVelocity().Size())/22000.f,0.f,1.f));
            Expect(FMath::Abs(P()->Camera->FieldOfView-ExpectedFOV)<1,TEXT("Dive camera follows achieved airspeed while movement mode is falling"));
        });
        Add(TEXT("Dive released"),.08f,[this](){P()->Landing->CancelImpactDive();},[this](){Motion(EIBPlayMotion::Fall,P()->FallPose,true);});
        Add(TEXT("Re-enter flight"),.06f,[this](){Mode(MOVE_Flying,FVector::ZeroVector);},[this](){Motion(EIBPlayMotion::Takeoff,P()->TakeoffPose,false);});
        Add(TEXT("Second hover"),.55f,{},[this](){Motion(EIBPlayMotion::Hover,P()->HoverPose,true);});
        Add(TEXT("Local-gravity jump"),.08f,[this](){Mode(MOVE_Falling,FVector(0,0,650));},[this](){Motion(EIBPlayMotion::Jump,P()->JumpPose,false);});
        Add(TEXT("Local-gravity fall"),.08f,[this](){Mode(MOVE_Falling,FVector(0,0,-650));},[this](){Motion(EIBPlayMotion::Fall,P()->FallPose,true);});
        Add(TEXT("Second land"),.05f,[this](){Mode(MOVE_Walking,FVector::ZeroVector);},[this](){Motion(EIBPlayMotion::Land,P()->LandPose,false);});
        Add(TEXT("Finish idle"),.45f,{},[this]()
        {
            Motion(EIBPlayMotion::Ground,P()->GroundLocomotion,true);
            Expect(P()->Presentation.Transition-FirstTransition>=20,TEXT("At least twenty actual presentation transitions exercised"));
            Expect(!P()->IsGameplayLocked(),TEXT("Presentation transitions leave the game input gate open"));
        });
    }
    void Restore()
    {
        if(Restored) return;
        Restored=true;
        if(Initialized&&Hero.IsValid())
        {
            auto* Move=P()->GetCharacterMovement();
            P()->Landing->CancelImpactDive(); P()->Punches->CancelAttack();
            P()->SetActorTransform(OriginalTransform,false,nullptr,ETeleportType::TeleportPhysics);
            P()->TangentForward=OriginalForward; P()->PreviousUp=OriginalUp;
            Move->SetGravityDirection(OriginalGravity); Move->SetMovementMode(OriginalMode); Move->Velocity=OriginalVelocity;
            P()->FlightGear=OriginalGear; P()->bBrake=OriginalBrake; P()->bTraversal=OriginalTraversal; P()->bTurbo=OriginalTurbo;
            P()->Camera->FieldOfView=OriginalFOV; P()->Arm->TargetArmLength=OriginalArm; P()->Arm->SocketOffset=OriginalSocket;
            P()->TakeoffRemaining=0; P()->LandRemaining=0; P()->ThrowAnimation=0; P()->Playing=nullptr;
            P()->PreviousPresentationMode=MOVE_None; P()->PreviousTravelDirection=FVector::ZeroVector;
            P()->VisualFacing=FQuat::Identity; P()->FlightBank=0; P()->GetMesh()->SetRelativeTransform(OriginalMesh);
            P()->SetActorTickEnabled(OriginalActorTick); Move->SetComponentTickEnabled(OriginalMovementTick);
            P()->Punches->SetComponentTickEnabled(OriginalPunchTick);
        }
        if(Floor.IsValid()) Floor->Destroy();
    }
    virtual bool Update() override
    {
        if(!Hero.IsValid()||FPlatformTime::Seconds()-Started>45)
        { Test.AddError(TEXT("Presentation fixture lost its pawn or exceeded 45 seconds.")); Restore(); return true; }
        if(GFrameCounter==LastFrame) return false;
        LastFrame=GFrameCounter;
        if(Index>=Steps.Num()||Failed)
        {
            if(!Failed) Test.AddInfo(TEXT("Production animation assets, gait cadence, radial body pose and finite action replay passed across real game frames. Human visual acceptance remains separate."));
            Restore(); return true;
        }
        FStep& Step=Steps[Index];
        const float Dt=FMath::Clamp(P()->GetWorld()->GetDeltaSeconds(),.001f,.1f);
        if(Elapsed==0&&Step.Start) Step.Start();
        if(Step.Frame) Step.Frame(Dt);
        P()->UpdateAnimation(Dt); P()->UpdatePresentationCamera(Dt);
        Elapsed+=Dt;
        if(Elapsed<Step.Duration) return false;
        Test.AddInfo(Step.Label);
        if(Step.Check) Step.Check();
        Expect(P()->GetActorQuat().Equals(FrameRotation,.0001)&&P()->GetActorLocation().Equals(Position,.01),TEXT("Presentation never rotates or moves the collision capsule"));
        Expect(!P()->GetMesh()->GetRelativeTransform().ContainsNaN(),TEXT("Mesh presentation stays finite"));
        Elapsed=0; ++Index; return false;
    }
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIBPlayPresentationTest,"ImpactBeyond.Playable.Presentation",
    EAutomationTestFlags::ClientContext|EAutomationTestFlags::ProductFilter)
bool FIBPlayPresentationTest::RunTest(const FString& Parameters)
{
    if(!FParse::Param(FCommandLine::Get(),TEXT("IBPlayQA")))
    { AddError(TEXT("Run the playable map with -IBPlayQA; never change a persistent player session.")); return false; }
    UWorld* World=nullptr;
    for(const FWorldContext& Context:GEngine->GetWorldContexts())
        if(Context.WorldType==EWorldType::Game) { World=Context.World(); break; }
    APlayerController* PC=World?World->GetFirstPlayerController():nullptr;
    AIBPlayCharacter* Hero=PC?Cast<AIBPlayCharacter>(PC->GetPawn()):nullptr;
    if(!TestNotNull(TEXT("Production pawn exists"),Hero)) return false;
    if(!TestTrue(TEXT("Isolated QA session is ready"),Hero->Session&&!Hero->Session->bPersistence&&!Hero->IsGameplayLocked()
        &&Hero->Grip&&!Hero->Grip->GetHeld()&&!Hero->Punches->IsAttacking()&&!Hero->Landing->IsImpacting())) return false;
    auto Fixture=MakeShared<FIBPlayPresentationFixture>(*this,Hero);
    if(!Fixture->Initialize()) { Fixture->Restore(); return false; }
    FAutomationTestFramework::Get().EnqueueLatentCommand(Fixture);
    return true;
}
#endif
