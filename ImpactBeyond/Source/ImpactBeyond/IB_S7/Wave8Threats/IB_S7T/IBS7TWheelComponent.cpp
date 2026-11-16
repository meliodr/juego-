#include "IBS7TWheelComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "UnrealClient.h"

UIBS7TWheelComponent::UIBS7TWheelComponent() { PrimaryComponentTick.bCanEverTick=true; }
bool UIBS7TWheelComponent::ConfigureSlots(const TArray<FIBS7TSlot>& Descriptors)
{
    Cancel(); Slots.Reset(); SelectedAbility=0;
    std::array<IBS7T::Slot,8> Values{};
    if(Descriptors.Num()<6 || Descriptors.Num()>8) { Wheel.configure(Values,0); return false; }
    for(int32 I=0; I<Descriptors.Num(); ++I) {
        const auto& D=Descriptors[I];
        if(D.AbilityId<=0 || D.Label.IsEmpty()) { Wheel.configure(Values,0); return false; }
        Values[I]={static_cast<uint32>(D.AbilityId),"Provider label","Provider icon",D.Cooldown,D.bAvailable};
    }
    if(!Wheel.configure(Values,Descriptors.Num())) return false;
    Slots=Descriptors; return true;
}
bool UIBS7TWheelComponent::UpdateSlots(const TArray<FIBS7TSlot>& Descriptors,int32 EquippedAbility)
{
    // Validate the whole update before touching an open wheel or a usable previous selection.
    if(Descriptors.Num()<6 || Descriptors.Num()>8 || EquippedAbility<=0) return false;
    bool SameLayout=Descriptors.Num()==Slots.Num(), HasEquipped=false;
    TSet<int32> Ids;
    for(int32 I=0;I<Descriptors.Num();++I)
    {
        const auto& D=Descriptors[I];
        if(D.AbilityId<=0 || D.Label.IsEmpty() || !FMath::IsFinite(D.Cooldown) || D.Cooldown<0 || Ids.Contains(D.AbilityId)) return false;
        Ids.Add(D.AbilityId); HasEquipped|=D.AbilityId==EquippedAbility;
        SameLayout&=Slots.IsValidIndex(I) && Slots[I].AbilityId==D.AbilityId;
    }
    if(!HasEquipped) return false;
    if(!SameLayout)
    {
        // A hero change or sector reorder invalidates the hover, then seeds the real equipped power.
        if(!ConfigureSlots(Descriptors)) return false;
        const bool Seeded=Wheel.synchronizeSelection(static_cast<uint32>(EquippedAbility));
        Refresh(); return Seeded;
    }
    IBS7T::PowerWheel Updated=Wheel;
    for(const auto& D:Descriptors)
        if(!Updated.updateCooldown(static_cast<uint32>(D.AbilityId),D.Cooldown)
            || !Updated.updateAvailable(static_cast<uint32>(D.AbilityId),D.bAvailable)) return false;
    if(!Updated.synchronizeSelection(static_cast<uint32>(EquippedAbility))) return false;
    Wheel=Updated; Slots=Descriptors; Refresh();
    return true;
}
bool UIBS7TWheelComponent::BindController(APlayerController* Controller, bool bInstallLabBindings)
{
    UnbindController();
    if(!IsValid(Controller) || !Controller->IsLocalController() || !MouseOpenKey.IsValid() || !PadOpenKey.IsValid() || MouseOpenKey==PadOpenKey) return false;
    BoundController=Controller;
    if (!bInstallLabBindings) return true;
    LocalInput=NewObject<UInputComponent>(Controller);
    LocalInput->RegisterComponent(); LocalInput->Priority=5; LocalInput->bBlockInput=false;
    LocalInput->BindKey(MouseOpenKey,IE_Pressed,this,&UIBS7TWheelComponent::OpenMouse);
    LocalInput->BindKey(MouseOpenKey,IE_Released,this,&UIBS7TWheelComponent::ConfirmMouse);
    LocalInput->BindKey(PadOpenKey,IE_Pressed,this,&UIBS7TWheelComponent::OpenPad);
    LocalInput->BindKey(PadOpenKey,IE_Released,this,&UIBS7TWheelComponent::ConfirmPad);
    // Cancel consumes input only while open; closed wheel does not steal Escape/B.
    LocalInput->BindKey(EKeys::Escape,IE_Pressed,this,&UIBS7TWheelComponent::Cancel).bConsumeInput=false;
    LocalInput->BindKey(EKeys::Gamepad_FaceButton_Right,IE_Pressed,this,&UIBS7TWheelComponent::Cancel).bConsumeInput=false;
    Controller->PushInputComponent(LocalInput); return true;
}
void UIBS7TWheelComponent::UnbindController()
{
    Cancel();
    if(IsValid(BoundController) && LocalInput) BoundController->PopInputComponent(LocalInput);
    if(LocalInput) LocalInput->DestroyComponent();
    LocalInput=nullptr; BoundController=nullptr;
}
void UIBS7TWheelComponent::EndPlay(const EEndPlayReason::Type Reason) { UnbindController(); Super::EndPlay(Reason); }
void UIBS7TWheelComponent::OpenMouse() { Open(false); }
void UIBS7TWheelComponent::OpenPad() { Open(true); }
void UIBS7TWheelComponent::Open(bool Gamepad)
{
    if(Wheel.open(bInputAllowed && IsValid(BoundController),1)) {
        bPad=Gamepad; bOpen=true; MouseOffset=FVector2D::ZeroVector;
        BoundController->SetIgnoreLookInput(true); bOwnLookLock=true; Refresh();
    }
    // No global time dilation mutation: partial pause requires a shared lease owner.
}
void UIBS7TWheelComponent::Refresh()
{
    HoveredSlot=Wheel.hovered(); SelectedAbility=static_cast<int32>(Wheel.selected());
    if(LocalInput) for(int32 I=4; I<LocalInput->KeyBindings.Num(); ++I) LocalInput->KeyBindings[I].bConsumeInput=bOpen;
}
void UIBS7TWheelComponent::ReleaseLookLock()
{
    if(bOwnLookLock && IsValid(BoundController)) BoundController->SetIgnoreLookInput(false);
    bOwnLookLock=false;
}
void UIBS7TWheelComponent::Cancel() { Wheel.cancel(); bOpen=false; ReleaseLookLock(); Refresh(); }
bool UIBS7TWheelComponent::CommitSelection_Implementation(int32 AbilityId)
{
    return SelectionProvider.IsBound() && SelectionProvider.Execute(AbilityId);
}
void UIBS7TWheelComponent::OpenWheel(bool bGamepad) { Open(bGamepad); }
void UIBS7TWheelComponent::ConfirmWheel(bool bGamepad) { if (bPad == bGamepad) Confirm(); }
bool UIBS7TWheelComponent::SelectPrevious()
{
    if (!bInputAllowed || bOpen) return false;
    struct FPreviousPort final : IBS7T::PowerWheelPort
    {
        UIBS7TWheelComponent* Owner = nullptr;
        bool select(uint32_t Ability) override { return Owner->CommitSelection(static_cast<int32>(Ability)); }
    } Port;
    Port.Owner = this;
    const bool Selected = Wheel.selectPrevious(Port);
    Refresh();
    if (Selected) OnSelectionChanged.Broadcast(SelectedAbility);
    return Selected;
}
void UIBS7TWheelComponent::ConfirmMouse() { if(!bPad) Confirm(); }
void UIBS7TWheelComponent::ConfirmPad() { if(bPad) Confirm(); }
void UIBS7TWheelComponent::Confirm()
{
    if(!bInputAllowed) { Cancel(); return; }
    struct SelectionPort final:IBS7T::PowerWheelPort {
        UIBS7TWheelComponent* Owner=nullptr;
        bool select(uint32_t Ability) override { return Owner->CommitSelection(static_cast<int32>(Ability)); }
    } Port;
    Port.Owner=this;
    const bool Selected=Wheel.confirm(Port); bOpen=false; ReleaseLookLock(); Refresh();
    if(Selected) OnSelectionChanged.Broadcast(SelectedAbility);
}
bool UIBS7TWheelComponent::SetCooldown(int32 AbilityId,float Seconds)
{
    if(AbilityId<=0 || !Wheel.updateCooldown(AbilityId,Seconds)) return false;
    for(auto& Slot:Slots) if(Slot.AbilityId==AbilityId) Slot.Cooldown=Seconds;
    return true;
}
void UIBS7TWheelComponent::TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime,TickType,ThisTickFunction);
    if(!bOpen) return;
    const UGameViewportClient* Viewport=GetWorld()?GetWorld()->GetGameViewport():nullptr;
    if(!bInputAllowed || !IsValid(BoundController) || !Viewport || !Viewport->Viewport || !Viewport->Viewport->HasFocus()) { Cancel(); return; }
    if(bPad) {
        float X=0,Y=0; BoundController->GetInputAnalogStickState(EControllerAnalogStick::CAS_RightStick,X,Y);
        Wheel.point(X,Y,true);
    } else {
        float X=0,Y=0; int32 W=0,H=0; BoundController->GetViewportSize(W,H);
        if(W<=0 || H<=0) { Cancel(); return; }
        BoundController->GetInputMouseDelta(X,Y);
        const double Radius=FMath::Min(W,H)*0.25;
        MouseOffset+=FVector2D(X,-Y)/Radius;
        MouseOffset=MouseOffset.GetClampedToMaxSize(1.0);
        Wheel.point(MouseOffset.X,MouseOffset.Y,false);
    }
    Refresh();
}
