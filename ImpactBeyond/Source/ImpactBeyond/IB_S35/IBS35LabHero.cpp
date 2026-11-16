#include "IB_S35/IBS35LabHero.h"
#include "IB_S35/IBS35GripComponent.h"
#include "IB_S35/IBS35PunchComponent.h"
#include "IB_S35/IBS35Object.h"
#include "IB_S35/IBS35LabTarget.h"
#include "IB_S35/IBS35ReceiverComponent.h"
#include "Components/InputComponent.h"
#include "InputCoreTypes.h"
#include "DrawDebugHelpers.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"

AIBS35LabHero::AIBS35LabHero(const FObjectInitializer& Initializer) : Super(Initializer)
{
    LabGrip = CreateDefaultSubobject<UIBS35GripComponent>(TEXT("S35Grip"));
    LabPunch = CreateDefaultSubobject<UIBS35PunchComponent>(TEXT("S35Punch"));
}

void AIBS35LabHero::SetupPlayerInputComponent(UInputComponent* Input)
{
    Super::SetupPlayerInputComponent(Input);
    // Remove only these inherited legacy bindings on this LAB instance. The production class is unchanged.
    for (int32 Index = Input->KeyBindings.Num() - 1; Index >= 0; --Index)
    {
        const FKey Key = Input->KeyBindings[Index].Chord.Key;
        if (Key == EKeys::G || Key == EKeys::T || Key == EKeys::R || Key == EKeys::Gamepad_FaceButton_Right
            || Key == EKeys::Gamepad_FaceButton_Top || Key == EKeys::Gamepad_DPad_Down
            || Key == EKeys::Gamepad_DPad_Up || Key == EKeys::Gamepad_DPad_Right)
            Input->KeyBindings.RemoveAt(Index);
    }
    Input->BindKey(EKeys::G, IE_Pressed, this, &AIBS35LabHero::ToggleGrip);
    Input->BindKey(EKeys::Gamepad_FaceButton_Right, IE_Pressed, this, &AIBS35LabHero::ToggleGrip);
    Input->BindKey(EKeys::T, IE_Pressed, this, &AIBS35LabHero::Charge);
    Input->BindKey(EKeys::T, IE_Released, this, &AIBS35LabHero::Throw);
    Input->BindKey(EKeys::Gamepad_FaceButton_Top, IE_Pressed, this, &AIBS35LabHero::Charge);
    Input->BindKey(EKeys::Gamepad_FaceButton_Top, IE_Released, this, &AIBS35LabHero::Throw);
    Input->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AIBS35LabHero::QuickHit);
    Input->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &AIBS35LabHero::HeavyHit);
    Input->BindKey(EKeys::Gamepad_DPad_Up, IE_Pressed, this, &AIBS35LabHero::QuickHit);
    Input->BindKey(EKeys::Gamepad_DPad_Right, IE_Pressed, this, &AIBS35LabHero::HeavyHit);
    Input->BindKey(EKeys::BackSpace, IE_Pressed, this, &AIBS35LabHero::Cancel);
    Input->BindKey(EKeys::Gamepad_Special_Left, IE_Pressed, this, &AIBS35LabHero::Cancel);
    Input->BindKey(EKeys::R, IE_Pressed, this, &AIBS35LabHero::ResetLab);
    Input->BindKey(EKeys::Gamepad_DPad_Down, IE_Pressed, this, &AIBS35LabHero::ResetLab);
}

void AIBS35LabHero::ToggleGrip()
{
    LabPunch->CancelAttack();
    if (LabGrip->GetHeld()) LabGrip->Drop(); else LabGrip->GrabNearest();
}
void AIBS35LabHero::Charge() { if (!LabPunch->IsAttacking()) LabGrip->BeginCharge(); }
void AIBS35LabHero::Throw() { if (!LabPunch->IsAttacking()) LabGrip->CommitThrow(GetControlRotation().Vector()); }
void AIBS35LabHero::QuickHit() { LabPunch->BeginPunch(false); }
void AIBS35LabHero::HeavyHit() { LabPunch->BeginPunch(true); }
void AIBS35LabHero::Cancel() { LabPunch->CancelAttack(); LabGrip->CancelCharge(); }
void AIBS35LabHero::ResetLab()
{
    Cancel(); LabGrip->Drop();
    for (TActorIterator<AIBS35Object> It(GetWorld()); It; ++It)
        if (!It->ResetObject()) UE_LOG(LogTemp, Warning, TEXT("IBS35 reset blocked at occupied spawn: %s"), *It->GetName());
    for (TActorIterator<AIBS35LabTarget> It(GetWorld()); It; ++It) It->Receiver->ResetReceiver();
}
void AIBS35LabHero::UnPossessed() { Cancel(); LabGrip->Drop(); Super::UnPossessed(); }

void AIBS35LabHero::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    FVector Left, Right;
    if (LabPunch->bDrawContacts && LabGrip->GetGripTargets(Left, Right))
    {
        DrawDebugSphere(GetWorld(), Left, 5, 8, FColor::Green, false, 0, 0, 1);
        DrawDebugSphere(GetWorld(), Right, 5, 8, FColor::Green, false, 0, 0, 1);
    }
    if (APlayerController* PC = Cast<APlayerController>(Controller))
        if (PC->IsPaused()) Cancel();
}
