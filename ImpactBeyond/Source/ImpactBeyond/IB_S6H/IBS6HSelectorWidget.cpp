#include "IBS6HSelectorWidget.h"
#include "IBS6HBridge.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

void UIBS6HSelectorWidget::SetBridge(UIBS6HBridge* InBridge)
{
    Bridge = InBridge;
    Refresh();
}

TSharedRef<SWidget> UIBS6HSelectorWidget::RebuildWidget()
{
    if (WidgetTree->RootWidget) return Super::RebuildWidget();
    auto* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>();
    WidgetTree->RootWidget = Canvas;
    auto* Panel = WidgetTree->ConstructWidget<UBorder>();
    Panel->SetBrushColor(FLinearColor(.015f, .025f, .045f, .97f));
    Panel->SetPadding(FMargin(28));
    auto* Position = Canvas->AddChildToCanvas(Panel);
    Position->SetAnchors(FAnchors(.5f, .5f));
    Position->SetAlignment(FVector2D(.5f, .5f));
    Position->SetSize(FVector2D(840, 330));
    auto* Column = WidgetTree->ConstructWidget<UVerticalBox>();
    Panel->SetContent(Column);
    auto* Title = WidgetTree->ConstructWidget<UTextBlock>();
    Title->SetText(FText::FromString(TEXT("Elige tu heroe")));
    Column->AddChildToVerticalBox(Title)->SetPadding(FMargin(0, 0, 0, 20));
    auto* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
    Column->AddChildToVerticalBox(Row);
    const auto Descriptors = Bridge ? Bridge->GetHeroDescriptors() : TArray<FIBS6HHeroDescriptor>();
    for (const auto& Descriptor : Descriptors)
    {
        auto* Button = WidgetTree->ConstructWidget<UButton>();
        auto* Text = WidgetTree->ConstructWidget<UTextBlock>();
        Text->SetText(FText::Format(FText::FromString(TEXT("{0}\n\n{1}")), Descriptor.Label, Descriptor.Description));
        Text->SetAutoWrapText(true);
        Text->SetColorAndOpacity(FSlateColor(FLinearColor::White));
        Button->SetContent(Text);
        auto* CardSlot = Row->AddChildToHorizontalBox(Button);
        CardSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
        CardSlot->SetPadding(FMargin(5));
        Cards.Add(Button);
    }
    if (Cards.Num() == 3)
    {
        Cards[0]->OnClicked.AddDynamic(this, &UIBS6HSelectorWidget::First);
        Cards[1]->OnClicked.AddDynamic(this, &UIBS6HSelectorWidget::Second);
        Cards[2]->OnClicked.AddDynamic(this, &UIBS6HSelectorWidget::Third);
    }
    Status = WidgetTree->ConstructWidget<UTextBlock>();
    Column->AddChildToVerticalBox(Status)->SetPadding(FMargin(0, 20, 0, 12));
    auto* Actions = WidgetTree->ConstructWidget<UHorizontalBox>();
    Column->AddChildToVerticalBox(Actions);
    auto* Accept = WidgetTree->ConstructWidget<UButton>();
    auto* AcceptText = WidgetTree->ConstructWidget<UTextBlock>();
    AcceptText->SetText(FText::FromString(TEXT("Confirmar")));
    Accept->SetContent(AcceptText);
    Accept->OnClicked.AddDynamic(this, &UIBS6HSelectorWidget::Confirm);
    Actions->AddChildToHorizontalBox(Accept)->SetPadding(FMargin(0, 0, 20, 0));
    auto* Cancel = WidgetTree->ConstructWidget<UButton>();
    auto* CancelText = WidgetTree->ConstructWidget<UTextBlock>();
    CancelText->SetText(FText::FromString(TEXT("Volver")));
    Cancel->SetContent(CancelText);
    Cancel->OnClicked.AddDynamic(this, &UIBS6HSelectorWidget::Back);
    Actions->AddChildToHorizontalBox(Cancel);
    Refresh();
    return Super::RebuildWidget();
}

void UIBS6HSelectorWidget::Refresh()
{
    if (!Bridge) return;
    const auto Descriptors = Bridge->GetHeroDescriptors();
    for (int32 Index = 0; Index < Cards.Num(); ++Index)
    {
        const FLinearColor Color = Descriptors.IsValidIndex(Index) ? Descriptors[Index].Color : FLinearColor::White;
        Cards[Index]->SetBackgroundColor(Index == Bridge->SelectorFocus() ? Color : Color * .25f);
    }
    if (Status) Status->SetText(FText::FromString(Bridge->LastResult.IsEmpty()
        ? TEXT("Flechas o cruceta para elegir. Enter / Cruz confirma. Esc / Circulo vuelve.") : Bridge->LastResult));
}

void UIBS6HSelectorWidget::Choose(int32 Index)
{
    if (!Bridge) return;
    Bridge->SelectorInput(Index + 4, true);
    Bridge->SelectorInput(Index + 4, false);
}
void UIBS6HSelectorWidget::First() { Choose(0); }
void UIBS6HSelectorWidget::Second() { Choose(1); }
void UIBS6HSelectorWidget::Third() { Choose(2); }
void UIBS6HSelectorWidget::Confirm()
{
    if (!Bridge) return;
    Bridge->SelectorInput(2, true);
    Bridge->SelectorInput(2, false);
}
void UIBS6HSelectorWidget::Back() { if (Bridge) Bridge->CloseSelector(); }
