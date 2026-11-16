#include "IBS7TWheelHUD.h"
#include "IBS7TWheelComponent.h"
#include "Engine/Canvas.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
void AIBS7TWheelHUD::DrawHUD()
{
    Super::DrawHUD();
    if(!Canvas || !PlayerOwner) return;
    UIBS7TWheelComponent* Wheel=PlayerOwner->FindComponentByClass<UIBS7TWheelComponent>();
    if(!Wheel && PlayerOwner->GetPawn()) Wheel=PlayerOwner->GetPawn()->FindComponentByClass<UIBS7TWheelComponent>();
    if(!Wheel || !Wheel->bOpen || Wheel->Slots.Num()<6) return;
    const float CX=Canvas->ClipX*0.5f,CY=Canvas->ClipY*0.5f;
    const float Radius=FMath::Min(Canvas->ClipX,Canvas->ClipY)*0.25f;
    DrawText(TEXT("S7 LAB - selection only / release to confirm / Esc or B cancel"),FLinearColor::White,CX-250,CY-20);
    for(int32 I=0; I<Wheel->Slots.Num(); ++I) {
        const auto& Slot=Wheel->Slots[I]; const float Angle=2.f*PI*I/Wheel->Slots.Num();
        const float X=CX+FMath::Cos(Angle)*Radius,Y=CY-FMath::Sin(Angle)*Radius;
        const FLinearColor Color=I==Wheel->HoveredSlot?FLinearColor::Yellow:(!Slot.bAvailable || Slot.Cooldown>0)?FLinearColor::Gray:FLinearColor::White;
        DrawRect(FLinearColor(0,0,0,0.85f),X-65,Y-25,130,65);
        // Geometric icon placeholder, not asset artwork. IconId remains available to a real widget.
        DrawRect(Color,X-5,Y-20,10,10);
        DrawText(FString::Printf(TEXT("%d %s"),I+1,*Slot.Label.ToString()),Color,X-60,Y);
        DrawText(FString::Printf(TEXT("CD %.1fs%s"),Slot.Cooldown,Slot.AbilityId==Wheel->SelectedAbility?TEXT(" [selected]"):TEXT("")),Color,X-60,Y+18);
    }
}
