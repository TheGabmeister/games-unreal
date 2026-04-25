#include "DiabloMainMenu.h"
#include "DiabloPlayerController.h"
#include "Diablo.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Blueprint/WidgetTree.h"

void UDiabloMainMenu::Init(ADiabloPlayerController* InController)
{
	OwnerController = InController;
}

void UDiabloMainMenu::UpdateButtonStates(bool bCanSave, bool bCanLoad)
{
	if (SaveButton)
	{
		SaveButton->SetIsEnabled(bCanSave);
	}
	if (LoadButton)
	{
		LoadButton->SetIsEnabled(bCanLoad);
	}
	if (SaveButtonText)
	{
		SaveButtonText->SetText(FText::FromString(TEXT("Save Game")));
	}
}

static UTextBlock* MakeMenuLabel(UWidgetTree* Tree, const FName& Name, const FString& Initial, int32 FontSize = 14)
{
	UTextBlock* Text = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
	Text->SetText(FText::FromString(Initial));
	Text->SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.85f, 0.6f, 1.f)));

	FSlateFontInfo Font = Text->GetFont();
	Font.Size = FontSize;
	Text->SetFont(Font);

	return Text;
}

static UButton* MakeMenuButton(UWidgetTree* Tree, const FName& Name, const FString& Label,
	TObjectPtr<UTextBlock>& OutText, int32 FontSize = 14)
{
	UButton* Btn = Tree->ConstructWidget<UButton>(UButton::StaticClass(), Name);

	FButtonStyle Style = Btn->GetStyle();
	Style.Normal.TintColor = FSlateColor(FLinearColor(0.15f, 0.12f, 0.08f, 0.9f));
	Style.Hovered.TintColor = FSlateColor(FLinearColor(0.3f, 0.25f, 0.15f, 0.9f));
	Style.Pressed.TintColor = FSlateColor(FLinearColor(0.1f, 0.08f, 0.05f, 0.9f));
	Style.Disabled.TintColor = FSlateColor(FLinearColor(0.1f, 0.1f, 0.1f, 0.5f));
	Btn->SetStyle(Style);

	OutText = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), FName(*(Name.ToString() + TEXT("Text"))));
	OutText->SetText(FText::FromString(Label));
	OutText->SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.85f, 0.6f, 1.f)));
	OutText->SetJustification(ETextJustify::Center);

	FSlateFontInfo Font = OutText->GetFont();
	Font.Size = FontSize;
	OutText->SetFont(Font);

	Btn->AddChild(OutText);
	return Btn;
}

TSharedRef<SWidget> UDiabloMainMenu::RebuildWidget()
{
	if (!WidgetTree->RootWidget)
	{
		UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("MenuRoot"));
		WidgetTree->RootWidget = Root;

		UBorder* BG = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("MenuBG"));
		BG->SetBrushColor(FLinearColor(0.03f, 0.03f, 0.05f, 0.88f));
		BG->SetPadding(FMargin(20.f));

		UCanvasPanelSlot* BGSlot = Root->AddChildToCanvas(BG);
		BGSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
		BGSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		BGSlot->SetPosition(FVector2D(0.f, 0.f));
		BGSlot->SetSize(FVector2D(250.f, 220.f));

		UVerticalBox* VBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MenuVBox"));
		BG->AddChild(VBox);

		UTextBlock* Title = MakeMenuLabel(WidgetTree, TEXT("MenuTitle"), TEXT("MENU"), 20);
		Title->SetJustification(ETextJustify::Center);
		UVerticalBoxSlot* TitleSlot = VBox->AddChildToVerticalBox(Title);
		TitleSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 16.f));
		TitleSlot->SetHorizontalAlignment(HAlign_Center);

		TObjectPtr<UTextBlock> ResumeText;
		ResumeButton = MakeMenuButton(WidgetTree, TEXT("ResumeBtn"), TEXT("Resume"), ResumeText);
		UVerticalBoxSlot* ResumeSlot = VBox->AddChildToVerticalBox(ResumeButton);
		ResumeSlot->SetPadding(FMargin(0.f, 4.f));
		ResumeSlot->SetHorizontalAlignment(HAlign_Fill);

		SaveButton = MakeMenuButton(WidgetTree, TEXT("SaveBtn"), TEXT("Save Game"), SaveButtonText);
		UVerticalBoxSlot* SaveSlot = VBox->AddChildToVerticalBox(SaveButton);
		SaveSlot->SetPadding(FMargin(0.f, 4.f));
		SaveSlot->SetHorizontalAlignment(HAlign_Fill);

		TObjectPtr<UTextBlock> LoadText;
		LoadButton = MakeMenuButton(WidgetTree, TEXT("LoadBtn"), TEXT("Load Game"), LoadText);
		UVerticalBoxSlot* LoadSlot = VBox->AddChildToVerticalBox(LoadButton);
		LoadSlot->SetPadding(FMargin(0.f, 4.f));
		LoadSlot->SetHorizontalAlignment(HAlign_Fill);

		ResumeButton->OnClicked.AddDynamic(this, &UDiabloMainMenu::OnResumeClicked);
		SaveButton->OnClicked.AddDynamic(this, &UDiabloMainMenu::OnSaveClicked);
		LoadButton->OnClicked.AddDynamic(this, &UDiabloMainMenu::OnLoadClicked);
	}

	return Super::RebuildWidget();
}

void UDiabloMainMenu::OnResumeClicked()
{
	if (OwnerController)
	{
		OwnerController->CloseMainMenu();
	}
}

void UDiabloMainMenu::OnSaveClicked()
{
	if (OwnerController)
	{
		OwnerController->SaveGame();
	}
}

void UDiabloMainMenu::OnLoadClicked()
{
	if (OwnerController)
	{
		OwnerController->LoadGame();
	}
}
