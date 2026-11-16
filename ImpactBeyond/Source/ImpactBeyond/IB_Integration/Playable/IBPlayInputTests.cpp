#if WITH_DEV_AUTOMATION_TESTS
#include "IBPlayCharacter.h"
#include "IBPlaySession.h"
#include "IB_S3/IBS3World.h"
#include "IB_S35/IBS35GripComponent.h"
#include "IB_S35/IBS35PunchComponent.h"
#include "IB_S6H/IBS6HBridge.h"
#include "IB_S7/Wave8Threats/IB_S7T/IBS7TWheelComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "HAL/PlatformTime.h"
#include "InputKeyEventArgs.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UnrealClient.h"

namespace
{
struct FIBPlayInputFixture
{
    FAutomationTestBase* Test = nullptr;
    TWeakObjectPtr<AIBPlayCharacter> Hero;
    TWeakObjectPtr<APlayerController> Controller;
    FTransform OriginalTransform;
    FVector OriginalVelocity, OriginalForward, OriginalUp;
    EMovementMode OriginalMode = MOVE_Flying;
    int32 OriginalGear = 1, OriginalHero = 0, OriginalPower = 101;
    bool OriginalMap = false, OriginalHelp = true;
    bool Failed = false, Restored = false;
    int32 ExpectedPower = 0;
    TSet<FKey> Held;
    TSet<FKey> Analog;

    void Expect(bool Value, const TCHAR* Label)
    {
        if (!Test->TestTrue(Label,Value)) Failed = true;
    }
    void Key(FKey Key, bool Down)
    {
        if (!Controller.IsValid()) { Failed = true; return; }
        // UE 5.8 deprecates FInputKeyParams; simulated events retain the actual gamepad FKey.
        Controller->InputKey(FInputKeyEventArgs::CreateSimulated(Key,Down?IE_Pressed:IE_Released,Down?1.f:0.f));
        if (Down) Held.Add(Key); else Held.Remove(Key);
    }
    void Axis(FKey Key, float Value)
    {
        if (Controller.IsValid()) Controller->InputKey(FInputKeyEventArgs::CreateSimulated(Key,IE_Axis,Value,1));
        Analog.Add(Key);
    }
    void ResetCombat()
    {
        Hero->Wheel->Cancel(); Hero->Heroes->CloseSelector(); Hero->bSettings=false;
        Hero->Heroes->ResetAbilities(); Hero->Heroes->SelectHero(0); Hero->Heroes->EquipAbility(101);
        Hero->Punches->CancelAttack(); Hero->StopJumping(); Hero->RefreshWheel();
    }
    void Restore()
    {
        if (Restored) return;
        Restored = true;
        const TArray<FKey> HeldKeys=Held.Array();
        for (FKey KeyValue : HeldKeys) Key(KeyValue,false);
        const TArray<FKey> AnalogKeys=Analog.Array();
        for (FKey KeyValue : AnalogKeys) Axis(KeyValue,0);
        if (Hero.IsValid())
        {
            ResetCombat(); Hero->Heroes->SelectHero(OriginalHero); Hero->Heroes->EquipAbility(OriginalPower);
            Hero->FlightGear=OriginalGear; Hero->bGlobalMap=OriginalMap; Hero->bHelp=OriginalHelp;
            Hero->SetActorTransform(OriginalTransform,false,nullptr,ETeleportType::TeleportPhysics);
            Hero->TangentForward=OriginalForward; Hero->PreviousUp=OriginalUp;
            Hero->GetCharacterMovement()->SetMovementMode(OriginalMode);
            Hero->GetCharacterMovement()->Velocity=OriginalVelocity;
        }
        if (Controller.IsValid()) { Controller->SetInputMode(FInputModeGameOnly()); Controller->bShowMouseCursor=false; }
    }
};

struct FIBPlayInputStep
{
    TFunction<void()> Start;
    TFunction<void()> EachFrame;
    TFunction<void()> Check;
};

class FIBPlayInputSequence final : public IAutomationLatentCommand
{
public:
    virtual ~FIBPlayInputSequence() override { if (Fixture) Fixture->Restore(); }
    TSharedPtr<FIBPlayInputFixture> Fixture;
    TArray<FIBPlayInputStep> Steps;
    int32 Index=0,Frames=0;
    uint64 LastFrame=MAX_uint64;
    double Started=FPlatformTime::Seconds();
    virtual bool Update() override
    {
        if (FPlatformTime::Seconds()-Started>60 || !Fixture->Hero.IsValid() || !Fixture->Controller.IsValid())
        {
            Fixture->Test->AddError(TEXT("Input sequence lost its playable pawn or exceeded 60 seconds."));
            Fixture->Restore(); return true;
        }
        if (GFrameCounter==LastFrame) return false;
        LastFrame=GFrameCounter;
        if (Index>=Steps.Num())
        {
            Fixture->Test->AddInfo(TEXT("Keyboard and gamepad events passed through PlayerController across distinct game ticks. Physical DualSense remains a separate check."));
            Fixture->Restore(); return true;
        }
        auto& Step=Steps[Index];
        if (Frames==0 && Step.Start) Step.Start();
        if (Step.EachFrame) Step.EachFrame();
        // Press/release events need real input processing between injection and observation.
        if (++Frames<3) return false;
        if (Step.Check) Step.Check();
        if (Fixture->Failed) { Fixture->Restore(); return true; }
        Frames=0; ++Index; return false;
    }
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIBPlayInputTest, "ImpactBeyond.Playable.RealInput",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FIBPlayInputTest::RunTest(const FString& Parameters)
{
    if (!FParse::Param(FCommandLine::Get(),TEXT("IBPlayQA")))
    {
        AddError(TEXT("Run the playable map with -IBPlayQA; this test must never change a player's persistent session.")); return false;
    }
    UWorld* World=nullptr;
    for (const FWorldContext& Context : GEngine->GetWorldContexts())
        if (Context.WorldType==EWorldType::Game) { World=Context.World(); break; }
    APlayerController* PC=World?World->GetFirstPlayerController():nullptr;
    AIBPlayCharacter* Hero=PC?Cast<AIBPlayCharacter>(PC->GetPawn()):nullptr;
    if (!TestNotNull(TEXT("Playable pawn in the Play map"),Hero)) return false;
    if (!TestTrue(TEXT("QA session disables persistence"),Hero->Session && !Hero->Session->bPersistence)) return false;
    if (!TestTrue(TEXT("Ready planet and input components"),Hero->Planet && Hero->Planet->IsOperational()
        && Hero->InputComponent && Hero->Heroes && Hero->Wheel && Hero->Punches && Hero->Grip && !Hero->Grip->GetHeld())) return false;
    UGameViewportClient* Viewport=World->GetGameViewport();
    if (!TestTrue(TEXT("Wheel input requires a rendered, focused game viewport; run without -nullrhi"),Viewport && Viewport->Viewport && Viewport->Viewport->HasFocus())) return false;
    if (!TestFalse(TEXT("Start with menus closed"),Hero->IsGameplayLocked())) return false;

    auto S=MakeShared<FIBPlayInputFixture>(); S->Test=this; S->Hero=Hero; S->Controller=PC;
    S->OriginalTransform=Hero->GetActorTransform(); S->OriginalVelocity=Hero->GetVelocity();
    S->OriginalForward=Hero->TangentForward; S->OriginalUp=Hero->PreviousUp;
    S->OriginalMode=Hero->GetCharacterMovement()->MovementMode; S->OriginalGear=Hero->FlightGear;
    S->OriginalHero=Hero->Heroes->ActiveHero(); S->OriginalPower=Hero->Heroes->EquippedAbility();
    S->OriginalMap=Hero->bGlobalMap; S->OriginalHelp=Hero->bHelp;
    auto Sequence=MakeShared<FIBPlayInputSequence>(); Sequence->Fixture=S;
    auto Step=[&](TFunction<void()> Start,TFunction<void()> Check={}) { Sequence->Steps.Add({MoveTemp(Start),{},MoveTemp(Check)}); };
    auto Press=[&](FKey Key,TFunction<void()> Check={}) { Step([S,Key](){S->Key(Key,true);},MoveTemp(Check)); };
    auto Release=[&](FKey Key,TFunction<void()> Check={}) { Step([S,Key](){S->Key(Key,false);},MoveTemp(Check)); };
    auto Tap=[&](FKey Key,TFunction<void()> Check={}) { Press(Key,MoveTemp(Check)); Release(Key); };

    Step([S]()
    {
        S->ResetCombat(); S->Hero->FlightGear=1; S->Hero->bGlobalMap=false; S->Hero->bHelp=false;
        // An empty patch of sky isolates control checks from autonomous encounter contact.
        S->Hero->SetActorLocation(S->OriginalTransform.GetLocation()+S->Hero->GetActorUpVector()*15000,false,nullptr,ETeleportType::TeleportPhysics);
        S->Hero->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
        S->Hero->GetCharacterMovement()->StopMovementImmediately();
    });
    Tap(EKeys::F,[S](){S->Expect(!S->Hero->GetCharacterMovement()->IsFlying(),TEXT("F leaves flight"));});
    Tap(EKeys::F,[S](){S->Expect(S->Hero->GetCharacterMovement()->IsFlying(),TEXT("F enters flight"));});
    Tap(EKeys::Gamepad_RightThumbstick,[S](){S->Expect(!S->Hero->GetCharacterMovement()->IsFlying(),TEXT("R3 leaves flight"));});
    Tap(EKeys::Gamepad_RightThumbstick,[S](){S->Expect(S->Hero->GetCharacterMovement()->IsFlying(),TEXT("R3 enters flight"));});
    Tap(EKeys::E,[S](){S->Expect(S->Hero->FlightGear==2,TEXT("E increases gear"));});
    Tap(EKeys::Q,[S](){S->Expect(S->Hero->FlightGear==1,TEXT("Q decreases gear"));});
    Tap(EKeys::Gamepad_RightShoulder,[S](){S->Expect(S->Hero->FlightGear==2,TEXT("R1 increases gear"));});
    Tap(EKeys::Gamepad_LeftShoulder,[S](){S->Expect(S->Hero->FlightGear==1,TEXT("L1 decreases gear"));});

    Tap(EKeys::H,[S](){S->Expect(S->Hero->Heroes->IsSelectorOpen(),TEXT("H opens hero selector"));});
    Step([S](){S->Key(EKeys::LeftMouseButton,true);S->Key(EKeys::Gamepad_DPad_Down,true);},[S]()
    {
        S->Expect(S->Hero->Punches->GetComboIndex()==0,TEXT("Hero selector suppresses keyboard and pad attacks"));
        S->Key(EKeys::LeftMouseButton,false);S->Key(EKeys::Gamepad_DPad_Down,false);
    });
    Tap(EKeys::Right,[S](){S->Expect(S->Hero->Heroes->SelectorFocus()==1 && S->Hero->Heroes->ActiveHero()==0,TEXT("Right previews hero without switching"));});
    Tap(EKeys::Enter,[S](){S->Expect(!S->Hero->Heroes->IsSelectorOpen() && S->Hero->Heroes->ActiveHero()==1 && !S->Hero->bPressedJump,TEXT("Enter confirms hero without jumping"));});
    Tap(EKeys::H);
    Tap(EKeys::Right,[S](){S->Expect(S->Hero->Heroes->SelectorFocus()==2,TEXT("Keyboard can preview another hero"));});
    Tap(EKeys::Escape,[S](){S->Expect(!S->Hero->Heroes->IsSelectorOpen() && S->Hero->Heroes->ActiveHero()==1 && !S->Hero->bSettings,TEXT("Escape cancels selector without opening settings"));});
    Tap(EKeys::Gamepad_Special_Right,[S](){S->Expect(S->Hero->Heroes->IsSelectorOpen(),TEXT("Options opens selector"));});
    Tap(EKeys::Gamepad_DPad_Right,[S](){S->Expect(S->Hero->Heroes->SelectorFocus()==2,TEXT("D-pad right previews hero"));});
    Tap(EKeys::Gamepad_FaceButton_Bottom,[S](){S->Expect(S->Hero->Heroes->ActiveHero()==2 && !S->Hero->Heroes->IsSelectorOpen() && !S->Hero->bPressedJump,TEXT("Cross confirms selector without jumping"));});
    Tap(EKeys::Gamepad_Special_Right);
    Tap(EKeys::Gamepad_DPad_Left,[S](){S->Expect(S->Hero->Heroes->SelectorFocus()==1,TEXT("D-pad left previews previous hero"));});
    Tap(EKeys::Gamepad_FaceButton_Right,[S](){S->Expect(!S->Hero->Heroes->IsSelectorOpen() && S->Hero->Heroes->ActiveHero()==2,TEXT("Circle cancels selector"));});
    Step([S](){S->ResetCombat();});

    Press(EKeys::Tab,[S](){S->Expect(S->Hero->Wheel->bOpen,TEXT("Held Tab opens power wheel"));});
    Step([S]()
    {
        S->Hero->GetCharacterMovement()->SetMovementMode(MOVE_Falling); S->Hero->StopJumping();
        S->Key(EKeys::SpaceBar,true);S->Key(EKeys::LeftMouseButton,true);S->Key(EKeys::Gamepad_DPad_Down,true);
        S->Key(EKeys::F,true);S->Key(EKeys::E,true);
    },[S]()
    {
        S->Expect(S->Hero->Wheel->bOpen && S->Hero->IsGameplayLocked(),TEXT("Wheel stays modal while held"));
        S->Expect(!S->Hero->bPressedJump && S->Hero->Punches->GetComboIndex()==0,TEXT("Wheel suppresses jump and attack inputs"));
        S->Expect(!S->Hero->GetCharacterMovement()->IsFlying() && S->Hero->FlightGear==1,TEXT("Wheel suppresses flight and gear changes"));
        for(FKey Key:{EKeys::SpaceBar,EKeys::LeftMouseButton,EKeys::Gamepad_DPad_Down,EKeys::F,EKeys::E}) S->Key(Key,false);
    });
    Release(EKeys::Tab,[S](){S->Expect(!S->Hero->Wheel->bOpen && S->Hero->Heroes->EquippedAbility()==101,TEXT("Tab release in dead zone preserves equipped power"));});
    Press(EKeys::Gamepad_DPad_Up,[S](){S->Expect(S->Hero->Wheel->bOpen,TEXT("Held D-pad up opens power wheel"));});
    Press(EKeys::Gamepad_FaceButton_Bottom,[S](){S->Expect(!S->Hero->bPressedJump && S->Hero->Wheel->bOpen,TEXT("Cross does not jump through power wheel"));});
    Release(EKeys::Gamepad_FaceButton_Bottom);
    Sequence->Steps.Add({{},[S](){S->Axis(EKeys::Gamepad_RightY,1);},[S]()
    {
        const int32 Hover=S->Hero->Wheel->HoveredSlot;
        S->Expect(S->Controller->GetInputAnalogKeyState(EKeys::Gamepad_RightY)>.5f,TEXT("Injected stick axis reaches PlayerController"));
        S->Expect(S->Hero->Wheel->Slots.IsValidIndex(Hover),TEXT("Right stick points to a power sector"));
        if(S->Hero->Wheel->Slots.IsValidIndex(Hover)) S->ExpectedPower=S->Hero->Wheel->Slots[Hover].AbilityId;
    }});
    Step([S](){S->Key(EKeys::Gamepad_DPad_Up,false);S->Axis(EKeys::Gamepad_RightY,0);},[S]()
    {
        S->Expect(!S->Hero->Wheel->bOpen && S->Hero->Heroes->EquippedAbility()==S->ExpectedPower,TEXT("Releasing D-pad equips the pointed power"));
    });
    Press(EKeys::Tab);
    Tap(EKeys::Escape,[S](){S->Expect(!S->Hero->Wheel->bOpen && !S->Hero->bSettings,TEXT("Escape cancels wheel without opening settings"));});
    Release(EKeys::Tab);

    Step([S](){S->ResetCombat();S->Hero->GetCharacterMovement()->SetMovementMode(MOVE_Falling);});
    Tap(EKeys::Escape,[S](){S->Expect(S->Hero->bSettings,TEXT("Escape opens world settings"));});
    Step([S]()
    {
        for(FKey Key:{EKeys::SpaceBar,EKeys::Gamepad_FaceButton_Bottom,EKeys::LeftMouseButton,EKeys::Gamepad_DPad_Down}) S->Key(Key,true);
    },[S]()
    {
        S->Expect(!S->Hero->bPressedJump && S->Hero->Punches->GetComboIndex()==0,TEXT("Settings suppress both devices' jump and power events"));
        for(FKey Key:{EKeys::SpaceBar,EKeys::Gamepad_FaceButton_Bottom,EKeys::LeftMouseButton,EKeys::Gamepad_DPad_Down}) S->Key(Key,false);
    });
    Tap(EKeys::Escape,[S](){S->Expect(!S->Hero->IsGameplayLocked(),TEXT("Escape restores gameplay"));});
    Tap(EKeys::LeftMouseButton,[S](){S->Expect(S->Hero->Punches->GetComboIndex()==1,TEXT("Mouse power input works after closing menus"));});
    Step([S](){S->Hero->Punches->CancelAttack();});
    Tap(EKeys::Gamepad_DPad_Down,[S](){S->Expect(S->Hero->Punches->GetComboIndex()==1,TEXT("D-pad power input works after closing menus"));});
    FAutomationTestFramework::Get().EnqueueLatentCommand(Sequence);
    return true;
}
#endif
