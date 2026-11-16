#if defined(IBS7_WHEEL_REFRESH_NATIVE_TEST_MAIN)
#include "PowerWheel.h"
#include <cstdlib>
#include <iostream>
#include <limits>
int main()
{
    int Checks=0;
    auto Check=[&Checks](bool Value){++Checks;if(!Value){std::cerr<<"Failure "<<Checks<<'\n';std::exit(1);}};
    struct FProvider final : IBS7T::PowerWheelPort
    {
        bool Accept=true; int Calls=0;
        bool select(uint32_t) override {++Calls;return Accept;}
    } Provider;
    IBS7T::PowerWheel Wheel;
    std::array<IBS7T::Slot,8> Slots{};
    for(unsigned I=0;I<6;++I) Slots[I]={101+I,"Power","Icon",0,true};
    Check(Wheel.configure(Slots,6));
    Check(Wheel.synchronizeSelection(101)); Check(Wheel.selected()==101);
    Check(!Wheel.selectPrevious(Provider)); Check(Provider.Calls==0);
    Check(Wheel.open(true)); Wheel.point(.5,.8660254037844386,true);
    Check(Wheel.confirm(Provider)); Check(Wheel.selected()==102);
    for(int Refresh=0;Refresh<200;++Refresh)
    {
        Check(Wheel.updateCooldown(105,(Refresh%10)*.15));
        Check(Wheel.updateAvailable(105,Refresh%2==0));
        Check(Wheel.synchronizeSelection(102));
        Check(Wheel.selected()==102);
    }
    Check(Wheel.updateAvailable(101,false)); Check(!Wheel.selectPrevious(Provider));
    Check(Wheel.updateAvailable(101,true)); Check(Wheel.updateCooldown(101,1)); Check(!Wheel.selectPrevious(Provider));
    Check(Wheel.updateCooldown(101,0)); Check(Wheel.selectPrevious(Provider)); Check(Wheel.selected()==101);
    Check(Wheel.synchronizeSelection(101)); Check(Wheel.selectPrevious(Provider)); Check(Wheel.selected()==102);
    Check(Wheel.synchronizeSelection(103)); Check(Wheel.selectPrevious(Provider)); Check(Wheel.selected()==102);
    Check(Wheel.selectPrevious(Provider)); Check(Wheel.selected()==103);
    // Confirming the same power is a no-op for history, including when refresh runs while open.
    Check(Wheel.open(true,.5)); Wheel.point(-.5,.8660254037844386,true);
    const int Hover=Wheel.hovered();
    Check(Wheel.updateAvailable(104,false)); Check(Wheel.synchronizeSelection(103));
    Check(Wheel.hovered()==Hover && Wheel.requestedTimeScale()==.5);
    Check(Wheel.confirm(Provider)); Check(Wheel.selectPrevious(Provider)); Check(Wheel.selected()==102);
    Provider.Accept=false; Check(!Wheel.selectPrevious(Provider)); Check(Wheel.selected()==102); Provider.Accept=true;
    Check(!Wheel.synchronizeSelection(999)); Check(!Wheel.updateAvailable(999,true));
    Check(!Wheel.updateCooldown(101,std::numeric_limits<double>::quiet_NaN())); Check(Wheel.selected()==102);
    Check(Wheel.selectPrevious(Provider)); Check(Wheel.selected()==103);
    for(unsigned I=0;I<6;++I) Slots[I].ability=201+I;
    Check(Wheel.configure(Slots,6)); Check(Wheel.synchronizeSelection(201));
    Check(!Wheel.selectPrevious(Provider)); Check(Wheel.selected()==201);
    std::cout<<Checks<<" wheel refresh checks PASS\n";
}
#elif WITH_DEV_AUTOMATION_TESTS
#include "../IB_S7T/IBS7TWheelComponent.h"
#include "Misc/AutomationTest.h"
#include <limits>
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIBS7WheelRefreshTest,"ImpactBeyond.S7.WheelRefresh",
    EAutomationTestFlags::ClientContext|EAutomationTestFlags::ProductFilter)
bool FIBS7WheelRefreshTest::RunTest(const FString& Parameters)
{
    UIBS7TWheelComponent* Component=NewObject<UIBS7TWheelComponent>();
    TArray<FIBS7TSlot> Slots;
    for(int32 I=0;I<6;++I)
    {
        FIBS7TSlot Slot; Slot.AbilityId=101+I; Slot.Label=FText::FromString(FString::Printf(TEXT("Poder %d"),I)); Slots.Add(Slot);
    }
    TestTrue(TEXT("Initial update seeds current selection"),Component->UpdateSlots(Slots,101));
    TestEqual(TEXT("Public selection reflects model"),Component->SelectedAbility,101);
    TestTrue(TEXT("External equip synchronizes"),Component->UpdateSlots(Slots,102));
    Slots[0].Label=FText::FromString(TEXT("Golpe actualizado")); Slots[0].IconId=TEXT("UpdatedIcon");
    Slots[0].Cooldown=1.25f; Slots[0].bAvailable=false;
    for(int32 Repeat=0;Repeat<20;++Repeat) TestTrue(TEXT("Periodic descriptor refresh"),Component->UpdateSlots(Slots,102));
    TestEqual(TEXT("Periodic refresh retains equipped model"),Component->SelectedAbility,102);
    TestTrue(TEXT("Display label updates"),Component->Slots[0].Label.EqualTo(Slots[0].Label));
    TestEqual(TEXT("Display icon updates"),Component->Slots[0].IconId,Slots[0].IconId);
    TestEqual(TEXT("Cooldown updates"),Component->Slots[0].Cooldown,1.25f);
    TestFalse(TEXT("Availability updates"),Component->Slots[0].bAvailable);
    auto Invalid=Slots; Invalid[0].Cooldown=9; Invalid[5].Cooldown=std::numeric_limits<float>::quiet_NaN();
    TestFalse(TEXT("Invalid late descriptor rejects whole update"),Component->UpdateSlots(Invalid,102));
    TestEqual(TEXT("Rejected update leaves earlier descriptors untouched"),Component->Slots[0].Cooldown,1.25f);
    TestFalse(TEXT("Unknown equipped ability rejected"),Component->UpdateSlots(Slots,999));
    Invalid=Slots; Invalid[5].AbilityId=Invalid[0].AbilityId;
    TestFalse(TEXT("Duplicate identities rejected"),Component->UpdateSlots(Invalid,102));
    TestEqual(TEXT("Rejected updates preserve selected power"),Component->SelectedAbility,102);
    for(int32 I=0;I<6;++I) Slots[I].AbilityId=201+I;
    TestTrue(TEXT("New hero set configures then seeds"),Component->UpdateSlots(Slots,201));
    TestEqual(TEXT("New hero has its equipped power"),Component->SelectedAbility,201);
    TestEqual(TEXT("No old hero slot survives"),Component->Slots[0].AbilityId,201);
    return true;
}
#endif
