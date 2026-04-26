#include "DiabloDialogWidget.h"
#include "DiabloPlayerController.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Blueprint/WidgetTree.h"

void UDiabloDialogWidget::Init(ADiabloPlayerController* InController)
{
	OwnerController = InController;
}

void UDiabloDialogWidget::SetDialog(const FText& Name, const FText& Text)
{
	if (NameText)
	{
		NameText->SetText(Name);
	}
	if (DialogTextBlock)
	{
		DialogTextBlock->SetText(Text);
	}
}

TSharedRef<SWidget> UDiabloDialogWidget::RebuildWidget()
{
	if (!WidgetTree->RootWidget)
	{
		UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("DialogRoot"));
		WidgetTree->RootWidget = Root;

		UBorder* BG = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("DialogBG"));
		BG->SetBrushColor(FLinearColor(0.03f, 0.03f, 0.05f, 0.9f));
		BG->SetPadding(FMargin(16.f));

		UCanvasPanelSlot* BGSlot = Root->AddChildToCanvas(BG);
		BGSlot->SetAnchors(FAnchors(0.5f, 1.f, 0.5f, 1.f));
		BGSlot->SetAlignment(FVector2D(0.5f, 1.f));
		BGSlot->SetPosition(FVector2D(0.f, -80.f));
		BGSlot->SetSize(FVector2D(420.f, 140.f));

		UVerticalBox* VBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("DialogVBox"));
		BG->AddChild(VBox);

		NameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("NameText"));
		NameText->SetText(FText::FromString(TEXT("NPC")));
		NameText->SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.85f, 0.6f, 1.f)));
		FSlateFontInfo NameFont = NameText->GetFont();
		NameFont.Size = 16;
		NameText->SetFont(NameFont);
		UVerticalBoxSlot* NameSlot = VBox->AddChildToVerticalBox(NameText);
		NameSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 6.f));

		DialogTextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DialogText"));
		DialogTextBlock->SetText(FText::FromString(TEXT("...")));
		DialogTextBlock->SetColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.85f, 0.85f, 1.f)));
		DialogTextBlock->SetAutoWrapText(true);
		FSlateFontInfo DialogFont = DialogTextBlock->GetFont();
		DialogFont.Size = 12;
		DialogTextBlock->SetFont(DialogFont);
		UVerticalBoxSlot* DialogSlot = VBox->AddChildToVerticalBox(DialogTextBlock);
		DialogSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));
		DialogSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

		CloseButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("CloseBtn"));

		FButtonStyle Style = CloseButton->GetStyle();
		Style.Normal.TintColor = FSlateColor(FLinearColor(0.15f, 0.12f, 0.08f, 0.9f));
		Style.Hovered.TintColor = FSlateColor(FLinearColor(0.3f, 0.25f, 0.15f, 0.9f));
		Style.Pressed.TintColor = FSlateColor(FLinearColor(0.1f, 0.08f, 0.05f, 0.9f));
		CloseButton->SetStyle(Style);

		UTextBlock* CloseLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CloseLabel"));
		CloseLabel->SetText(FText::FromString(TEXT("Close")));
		CloseLabel->SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.85f, 0.6f, 1.f)));
		CloseLabel->SetJustification(ETextJustify::Center);
		FSlateFontInfo CloseFont = CloseLabel->GetFont();
		CloseFont.Size = 12;
		CloseLabel->SetFont(CloseFont);
		CloseButton->AddChild(CloseLabel);

		UVerticalBoxSlot* CloseSlot = VBox->AddChildToVerticalBox(CloseButton);
		CloseSlot->SetHorizontalAlignment(HAlign_Right);

		CloseButton->OnClicked.AddDynamic(this, &UDiabloDialogWidget::OnCloseClicked);
	}

	return Super::RebuildWidget();
}

void UDiabloDialogWidget::OnCloseClicked()
{
	if (OwnerController)
	{
		OwnerController->CloseDialog();
	}
}
