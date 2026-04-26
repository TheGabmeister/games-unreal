#include "DiabloPlayerController.h"
#include "DiabloHero.h"
#include "DiabloHUDWidget.h"
#include "DiabloCharacterPanel.h"
#include "DiabloInventoryPanel.h"
#include "DiabloSpellbookPanel.h"
#include "DiabloMainMenu.h"
#include "DiabloDialogWidget.h"
#include "DiabloShopPanel.h"
#include "DiabloNPC.h"
#include "DiabloSaveGame.h"
#include "DiabloGameInstance.h"
#include "Interactable.h"
#include "InventoryComponent.h"
#include "DiabloEnemy.h"
#include "DroppedItem.h"
#include "Diablo.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameModeBase.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"

ADiabloPlayerController::ADiabloPlayerController()
{
	bShowMouseCursor = true;
}

void ADiabloPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}

	CreateHUD();
}

void ADiabloPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (ADiabloHero* Hero = Cast<ADiabloHero>(InPawn))
	{
		if (HUDWidget)
		{
			HUDWidget->InitForHero(Hero);
			HUDWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		if (CharPanel)
		{
			CharPanel->InitForHero(Hero);
		}
		if (InventoryPanel)
		{
			InventoryPanel->InitForInventory(Hero->Inventory);
		}
		if (SpellbookPanel)
		{
			SpellbookPanel->InitForHero(Hero);
		}
	}
}

void ADiabloPlayerController::CreateHUD()
{
	if (!HUDWidgetClass)
	{
		UE_LOG(LogDiablo, Warning, TEXT("HUDWidgetClass is null — run Setup HUD in DiabloTools"));
		return;
	}

	HUDWidget = CreateWidget<UDiabloHUDWidget>(this, HUDWidgetClass);
	if (HUDWidget)
	{
		HUDWidget->AddToViewport();

		if (ADiabloHero* Hero = Cast<ADiabloHero>(GetPawn()))
		{
			HUDWidget->InitForHero(Hero);
		}
	}

	if (CharPanelClass)
	{
		CharPanel = CreateWidget<UDiabloCharacterPanel>(this, CharPanelClass);
		if (CharPanel)
		{
			CharPanel->AddToViewport();
			CharPanel->SetVisibility(ESlateVisibility::Collapsed);

			if (ADiabloHero* Hero = Cast<ADiabloHero>(GetPawn()))
			{
				CharPanel->InitForHero(Hero);
			}
		}
	}

	if (InventoryPanelClass)
	{
		InventoryPanel = CreateWidget<UDiabloInventoryPanel>(this, InventoryPanelClass);
		if (InventoryPanel)
		{
			InventoryPanel->AddToViewport();
			InventoryPanel->SetVisibility(ESlateVisibility::Collapsed);

			if (ADiabloHero* Hero = Cast<ADiabloHero>(GetPawn()))
			{
				InventoryPanel->InitForInventory(Hero->Inventory);
			}
		}
	}

	if (SpellbookPanelClass)
	{
		SpellbookPanel = CreateWidget<UDiabloSpellbookPanel>(this, SpellbookPanelClass);
		if (SpellbookPanel)
		{
			SpellbookPanel->AddToViewport();
			SpellbookPanel->SetVisibility(ESlateVisibility::Collapsed);

			if (ADiabloHero* Hero = Cast<ADiabloHero>(GetPawn()))
			{
				SpellbookPanel->InitForHero(Hero);
			}
		}
	}

	if (MainMenuClass)
	{
		MainMenu = CreateWidget<UDiabloMainMenu>(this, MainMenuClass);
		if (MainMenu)
		{
			MainMenu->AddToViewport(100);
			MainMenu->SetVisibility(ESlateVisibility::Collapsed);
			MainMenu->Init(this);
		}
	}

	if (DialogWidgetClass)
	{
		DialogWidget = CreateWidget<UDiabloDialogWidget>(this, DialogWidgetClass);
		if (DialogWidget)
		{
			DialogWidget->AddToViewport(50);
			DialogWidget->SetVisibility(ESlateVisibility::Collapsed);
			DialogWidget->Init(this);
		}
	}

	if (ShopPanelClass)
	{
		ShopPanel = CreateWidget<UDiabloShopPanel>(this, ShopPanelClass);
		if (ShopPanel)
		{
			ShopPanel->AddToViewport(60);
			ShopPanel->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void ADiabloPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EIC->BindAction(ClickAction, ETriggerEvent::Started, this, &ADiabloPlayerController::OnClickStarted);
		if (CharPanelAction)
		{
			EIC->BindAction(CharPanelAction, ETriggerEvent::Started, this, &ADiabloPlayerController::OnToggleCharPanel);
		}
		if (InventoryAction)
		{
			EIC->BindAction(InventoryAction, ETriggerEvent::Started, this, &ADiabloPlayerController::OnToggleInventory);
		}
		if (CastAction)
		{
			EIC->BindAction(CastAction, ETriggerEvent::Started, this, &ADiabloPlayerController::OnCastStarted);
		}
		if (SpellbookAction)
		{
			EIC->BindAction(SpellbookAction, ETriggerEvent::Started, this, &ADiabloPlayerController::OnToggleSpellbook);
		}
		if (MenuAction)
		{
			EIC->BindAction(MenuAction, ETriggerEvent::Started, this, &ADiabloPlayerController::OnToggleMainMenu);
		}
	}
}

void ADiabloPlayerController::OnToggleCharPanel()
{
	if (!CharPanel)
	{
		return;
	}

	if (CharPanel->GetVisibility() == ESlateVisibility::Collapsed)
	{
		CharPanel->SetVisibility(ESlateVisibility::Visible);
		bShowMouseCursor = true;
		FInputModeGameAndUI Mode;
		Mode.SetWidgetToFocus(CharPanel->TakeWidget());
		SetInputMode(Mode);
	}
	else
	{
		CharPanel->SetVisibility(ESlateVisibility::Collapsed);
		FInputModeGameAndUI Mode;
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(Mode);
	}
}

void ADiabloPlayerController::OnToggleInventory()
{
	if (!InventoryPanel)
	{
		return;
	}

	if (InventoryPanel->GetVisibility() == ESlateVisibility::Collapsed)
	{
		InventoryPanel->SetVisibility(ESlateVisibility::Visible);
		bShowMouseCursor = true;
		FInputModeGameAndUI Mode;
		Mode.SetWidgetToFocus(InventoryPanel->TakeWidget());
		SetInputMode(Mode);
	}
	else
	{
		InventoryPanel->SetVisibility(ESlateVisibility::Collapsed);
		FInputModeGameAndUI Mode;
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(Mode);
	}
}

void ADiabloPlayerController::OnToggleSpellbook()
{
	if (!SpellbookPanel)
	{
		return;
	}

	if (SpellbookPanel->GetVisibility() == ESlateVisibility::Collapsed)
	{
		SpellbookPanel->SetVisibility(ESlateVisibility::Visible);
		bShowMouseCursor = true;
		FInputModeGameAndUI Mode;
		Mode.SetWidgetToFocus(SpellbookPanel->TakeWidget());
		SetInputMode(Mode);
	}
	else
	{
		SpellbookPanel->SetVisibility(ESlateVisibility::Collapsed);
		FInputModeGameAndUI Mode;
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(Mode);
	}
}

void ADiabloPlayerController::OnClickStarted()
{
	if (ShopPanel && ShopPanel->GetVisibility() == ESlateVisibility::Visible)
	{
		return;
	}
	if (DialogWidget && DialogWidget->GetVisibility() == ESlateVisibility::Visible)
	{
		CloseDialog();
	}

	ADiabloHero* Hero = Cast<ADiabloHero>(GetPawn());
	if (!Hero || Hero->IsDead())
	{
		return;
	}

	FHitResult HitResult;
	if (!GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility), false, HitResult))
	{
		return;
	}

	AActor* HitActor = HitResult.GetActor();
	UE_LOG(LogDiablo, Display, TEXT("Click hit: %s (Component: %s)"),
		HitActor ? *HitActor->GetName() : TEXT("null"),
		HitResult.GetComponent() ? *HitResult.GetComponent()->GetName() : TEXT("null"));

	if (ADiabloEnemy* Enemy = Cast<ADiabloEnemy>(HitActor))
	{
		if (!Enemy->IsDead())
		{
			TargetEnemy = Enemy;
			TargetItem = nullptr;
			TargetInteractable = nullptr;
			UE_LOG(LogDiablo, Display, TEXT("Targeting enemy: %s"), *Enemy->GetName());
			UAIBlueprintHelperLibrary::SimpleMoveToActor(this, TargetEnemy);
			return;
		}
	}

	if (ADroppedItem* Item = Cast<ADroppedItem>(HitActor))
	{
		TargetItem = Item;
		TargetEnemy = nullptr;
		TargetInteractable = nullptr;
		UE_LOG(LogDiablo, Display, TEXT("Targeting item: %s"), *Item->GetName());
		UAIBlueprintHelperLibrary::SimpleMoveToActor(this, Item);
		return;
	}

	if (HitActor && HitActor->Implements<UInteractable>())
	{
		TargetInteractable = HitActor;
		TargetEnemy = nullptr;
		TargetItem = nullptr;
		UE_LOG(LogDiablo, Display, TEXT("Targeting interactable: %s"), *HitActor->GetName());
		UAIBlueprintHelperLibrary::SimpleMoveToActor(this, HitActor);
		return;
	}

	TargetEnemy = nullptr;
	TargetItem = nullptr;
	TargetInteractable = nullptr;
	UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, HitResult.ImpactPoint);
}

void ADiabloPlayerController::OnCastStarted()
{
	ADiabloHero* Hero = Cast<ADiabloHero>(GetPawn());
	if (!Hero || Hero->IsDead())
	{
		return;
	}

	FHitResult HitResult;
	if (!GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility), false, HitResult))
	{
		return;
	}

	TargetEnemy = nullptr;
	TargetItem = nullptr;
	TargetInteractable = nullptr;

	Hero->CastSpell(HitResult.ImpactPoint);
}

void ADiabloPlayerController::OnHeroDeath()
{
	TargetEnemy = nullptr;
	TargetItem = nullptr;
	TargetInteractable = nullptr;

	if (DialogWidget && DialogWidget->GetVisibility() == ESlateVisibility::Visible)
	{
		CloseDialog();
	}

	if (HUDWidget)
	{
		HUDWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	DisableInput(this);

	GetWorldTimerManager().SetTimer(RespawnTimerHandle, this,
		&ADiabloPlayerController::OnRespawnTimerExpired, 2.f, false);
}

void ADiabloPlayerController::OnRespawnTimerExpired()
{
	if (APawn* DeadPawn = GetPawn())
	{
		UnPossess();
		DeadPawn->Destroy();
	}

	if (AGameModeBase* GM = GetWorld()->GetAuthGameMode())
	{
		GM->RestartPlayer(this);
	}

	EnableInput(this);
}

void ADiabloPlayerController::OnToggleMainMenu()
{
	if (!MainMenu)
	{
		return;
	}

	if (MainMenu->GetVisibility() == ESlateVisibility::Visible)
	{
		CloseMainMenu();
	}
	else
	{
		if (CharPanel && CharPanel->GetVisibility() != ESlateVisibility::Collapsed)
		{
			CharPanel->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (InventoryPanel && InventoryPanel->GetVisibility() != ESlateVisibility::Collapsed)
		{
			InventoryPanel->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (SpellbookPanel && SpellbookPanel->GetVisibility() != ESlateVisibility::Collapsed)
		{
			SpellbookPanel->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (DialogWidget && DialogWidget->GetVisibility() != ESlateVisibility::Collapsed)
		{
			DialogWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (ShopPanel && ShopPanel->GetVisibility() != ESlateVisibility::Collapsed)
		{
			ShopPanel->SetVisibility(ESlateVisibility::Collapsed);
		}

		const FString LevelName = UGameplayStatics::GetCurrentLevelName(this, true);
		const bool bInTown = LevelName == TEXT("Lvl_Diablo");
		const bool bSaveExists = UGameplayStatics::DoesSaveGameExist(UDiabloSaveGame::SaveSlotName, UDiabloSaveGame::UserIndex);

		MainMenu->UpdateButtonStates(bInTown, bSaveExists);
		MainMenu->SetVisibility(ESlateVisibility::Visible);

		UGameplayStatics::SetGamePaused(this, true);
		bShowMouseCursor = true;
		FInputModeGameAndUI Mode;
		Mode.SetWidgetToFocus(MainMenu->TakeWidget());
		SetInputMode(Mode);
	}
}

void ADiabloPlayerController::CloseMainMenu()
{
	if (MainMenu)
	{
		MainMenu->SetVisibility(ESlateVisibility::Collapsed);
	}
	UGameplayStatics::SetGamePaused(this, false);
	FInputModeGameAndUI Mode;
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(Mode);
}

void ADiabloPlayerController::SaveGame()
{
	ADiabloHero* Hero = Cast<ADiabloHero>(GetPawn());
	if (!Hero)
	{
		return;
	}

	Hero->SaveToGameInstance();

	UDiabloGameInstance* GI = Cast<UDiabloGameInstance>(GetGameInstance());
	if (!GI)
	{
		return;
	}

	UDiabloSaveGame* Save = Cast<UDiabloSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UDiabloSaveGame::StaticClass()));
	Save->PopulateFromGameInstance(GI);

	UGameplayStatics::SaveGameToSlot(Save, UDiabloSaveGame::SaveSlotName, UDiabloSaveGame::UserIndex);
	UE_LOG(LogDiablo, Display, TEXT("Game saved to slot: %s"), *UDiabloSaveGame::SaveSlotName);

	CloseMainMenu();
}

void ADiabloPlayerController::LoadGame()
{
	UDiabloSaveGame* Save = Cast<UDiabloSaveGame>(
		UGameplayStatics::LoadGameFromSlot(UDiabloSaveGame::SaveSlotName, UDiabloSaveGame::UserIndex));
	if (!Save)
	{
		UE_LOG(LogDiablo, Warning, TEXT("Failed to load save from slot: %s"), *UDiabloSaveGame::SaveSlotName);
		return;
	}

	UDiabloGameInstance* GI = Cast<UDiabloGameInstance>(GetGameInstance());
	if (!GI)
	{
		return;
	}

	Save->ApplyToGameInstance(GI);
	UE_LOG(LogDiablo, Display, TEXT("Game loaded from slot: %s"), *UDiabloSaveGame::SaveSlotName);

	CloseMainMenu();
	UGameplayStatics::OpenLevel(this, FName(TEXT("Lvl_Diablo")));
}

void ADiabloPlayerController::ShowDialog(const FText& Name, const FText& Text)
{
	if (!DialogWidget)
	{
		return;
	}

	DialogWidget->SetDialog(Name, Text);
	DialogWidget->SetVisibility(ESlateVisibility::Visible);
	bShowMouseCursor = true;
	FInputModeGameAndUI Mode;
	Mode.SetWidgetToFocus(DialogWidget->TakeWidget());
	SetInputMode(Mode);
}

void ADiabloPlayerController::CloseDialog()
{
	if (DialogWidget)
	{
		DialogWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	FInputModeGameAndUI Mode;
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(Mode);
}

void ADiabloPlayerController::OpenShop(ADiabloNPC* NPC)
{
	if (!ShopPanel || !NPC)
	{
		return;
	}

	ShopPanel->Init(this, NPC);
	ShopPanel->SetVisibility(ESlateVisibility::Visible);
	bShowMouseCursor = true;
	FInputModeGameAndUI Mode;
	Mode.SetWidgetToFocus(ShopPanel->TakeWidget());
	SetInputMode(Mode);
}

void ADiabloPlayerController::CloseShop()
{
	if (ShopPanel)
	{
		ShopPanel->SetVisibility(ESlateVisibility::Collapsed);
	}
	FInputModeGameAndUI Mode;
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(Mode);
}

void ADiabloPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ADiabloHero* Hero = Cast<ADiabloHero>(GetPawn());
	if (!Hero || Hero->IsDead())
	{
		TargetEnemy = nullptr;
		TargetItem = nullptr;
		TargetInteractable = nullptr;
		return;
	}

	if (TargetInteractable && IsValid(TargetInteractable))
	{
		const float Dist = FVector::Dist(Hero->GetActorLocation(), TargetInteractable->GetActorLocation());
		if (Dist <= InteractRange)
		{
			StopMovement();
			Hero->GetCharacterMovement()->StopActiveMovement();
			if (IInteractable* Interactable = Cast<IInteractable>(TargetInteractable))
			{
				Interactable->Interact(Hero);
			}
			TargetInteractable = nullptr;
		}
		return;
	}
	else
	{
		TargetInteractable = nullptr;
	}

	if (TargetItem && !IsValid(TargetItem))
	{
		TargetItem = nullptr;
	}

	if (TargetItem)
	{
		const float ItemDist = FVector::Dist(Hero->GetActorLocation(), TargetItem->GetActorLocation());
		if (ItemDist <= PickupRange)
		{
			StopMovement();
			Hero->GetCharacterMovement()->StopActiveMovement();
			TargetItem->OnPickedUp(Hero);
			TargetItem = nullptr;
		}
		return;
	}

	if (!TargetEnemy || TargetEnemy->IsDead())
	{
		TargetEnemy = nullptr;
		return;
	}

	const float Distance = FVector::Dist(Hero->GetActorLocation(), TargetEnemy->GetActorLocation());
	if (Distance <= AttackRange)
	{
		StopMovement();
		Hero->GetCharacterMovement()->StopActiveMovement();

		const FVector Direction = (TargetEnemy->GetActorLocation() - Hero->GetActorLocation()).GetSafeNormal2D();
		if (!Direction.IsNearlyZero())
		{
			Hero->SetActorRotation(Direction.Rotation());
		}

		if (!Hero->IsAttacking())
		{
			UE_LOG(LogDiablo, Display, TEXT("In range (%.0f <= %.0f), attacking"), Distance, AttackRange);
			Hero->AttackTarget = TargetEnemy;
			Hero->StartAttack();
		}
	}
}
