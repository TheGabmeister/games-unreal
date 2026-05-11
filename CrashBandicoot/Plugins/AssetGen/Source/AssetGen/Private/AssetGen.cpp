#include "AssetGen.h"

#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Framework/Docking/TabManager.h"

#include "Kismet2/KismetEditorUtilities.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Blueprint.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Game/CBGameMode.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/WorldSettings.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "UObject/SavePackage.h"
#include "UObject/Package.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "EnhancedActionKeyMapping.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TextRenderActor.h"
#include "Components/TextRenderComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"

#include "FileHelpers.h"
#include "Subsystems/EditorActorSubsystem.h"
#include "Editor.h"
#include "Misc/MessageDialog.h"
#include "WidgetBlueprint.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"

#define LOCTEXT_NAMESPACE "FAssetGenModule"

static const FName AssetGenTabName("AssetGen");

void FAssetGenModule::StartupModule()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(AssetGenTabName,
		FOnSpawnTab::CreateRaw(this, &FAssetGenModule::OnSpawnTab))
		.SetDisplayName(LOCTEXT("TabTitle", "AssetGen"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FAssetGenModule::RegisterMenus));
}

void FAssetGenModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(AssetGenTabName);
}

void FAssetGenModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
	FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
	Section.AddEntry(FToolMenuEntry::InitMenuEntry(
		"AssetGen",
		LOCTEXT("MenuEntry", "AssetGen"),
		LOCTEXT("MenuEntryTooltip", "Open the AssetGen tool window"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([](){ FGlobalTabmanager::Get()->TryInvokeTab(AssetGenTabName); }))
	));
}

TSharedRef<SDockTab> FAssetGenModule::OnSpawnTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot().AutoHeight().Padding(10, 10, 10, 2)
			[ SNew(STextBlock).Text(LOCTEXT("Header", "Core Blueprints")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 11)) ]

			+ SVerticalBox::Slot().AutoHeight().Padding(10, 2)
			[ SNew(SButton).Text(LOCTEXT("GenGM", "Create BP_GameMode")).OnClicked_Static(&FAssetGenModule::OnCreateGameModeClicked) ]

			+ SVerticalBox::Slot().AutoHeight().Padding(10, 2)
			[ SNew(SButton).Text(LOCTEXT("GenGI", "Create BP_GameInstance")).OnClicked_Static(&FAssetGenModule::OnCreateGameInstanceClicked) ]

			+ SVerticalBox::Slot().AutoHeight().Padding(10, 2)
			[ SNew(SButton).Text(LOCTEXT("GenPC", "Create BP_PlayerCharacter")).OnClicked_Static(&FAssetGenModule::OnCreatePlayerCharacterClicked) ]

			+ SVerticalBox::Slot().AutoHeight().Padding(10, 2)
			[ SNew(SButton).Text(LOCTEXT("GenCtrl", "Create BP_PlayerController")).OnClicked_Static(&FAssetGenModule::OnCreatePlayerControllerClicked) ]

			+ SVerticalBox::Slot().AutoHeight().Padding(10, 14, 10, 2)
			[ SNew(STextBlock).Text(LOCTEXT("InputHeader", "Input")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 11)) ]

			+ SVerticalBox::Slot().AutoHeight().Padding(10, 2)
			[ SNew(SButton).Text(LOCTEXT("GenInput", "Create Input Actions")).OnClicked_Static(&FAssetGenModule::OnCreateInputActionsClicked) ]

			+ SVerticalBox::Slot().AutoHeight().Padding(10, 2)
			[ SNew(SButton).Text(LOCTEXT("GenIMC", "Create IMC_Gameplay")).OnClicked_Static(&FAssetGenModule::OnCreateIMCClicked) ]

			+ SVerticalBox::Slot().AutoHeight().Padding(10, 2)
			[ SNew(SButton).Text(LOCTEXT("GenCam", "Create BP_Camera")).OnClicked_Static(&FAssetGenModule::OnCreateCameraClicked) ]

			+ SVerticalBox::Slot().AutoHeight().Padding(10, 14, 10, 2)
			[ SNew(STextBlock).Text(LOCTEXT("AnimHeader", "Animation")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 11)) ]

			+ SVerticalBox::Slot().AutoHeight().Padding(10, 2)
			[ SNew(SButton).Text(LOCTEXT("GenSpinMontage", "Create Spin Montage")).OnClicked_Static(&FAssetGenModule::OnCreateSpinMontageClicked) ]

			+ SVerticalBox::Slot().AutoHeight().Padding(10, 14, 10, 2)
			[ SNew(STextBlock).Text(LOCTEXT("Phase2Header", "Phase 2 — Crates & Collectibles")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 11)) ]

			+ SVerticalBox::Slot().AutoHeight().Padding(10, 2)
			[ SNew(SButton).Text(LOCTEXT("GenCrates", "Create Crate Blueprints")).OnClicked_Static(&FAssetGenModule::OnCreateCratesClicked) ]

			+ SVerticalBox::Slot().AutoHeight().Padding(10, 2)
			[ SNew(SButton).Text(LOCTEXT("GenPickups", "Create Pickup Blueprints")).OnClicked_Static(&FAssetGenModule::OnCreatePickupsClicked) ]

			+ SVerticalBox::Slot().AutoHeight().Padding(10, 2)
			[ SNew(SButton).Text(LOCTEXT("GenMask", "Create BP_AkuAkuMask")).OnClicked_Static(&FAssetGenModule::OnCreateAkuAkuMaskClicked) ]

			+ SVerticalBox::Slot().AutoHeight().Padding(10, 2)
			[ SNew(SButton).Text(LOCTEXT("GenHUD", "Create WBP_GameplayHUD")).OnClicked_Static(&FAssetGenModule::OnCreateGameplayHUDClicked) ]

			+ SVerticalBox::Slot().AutoHeight().Padding(10, 2)
			[ SNew(SButton).Text(LOCTEXT("GenLoading", "Create WBP_LoadingScreen")).OnClicked_Static(&FAssetGenModule::OnCreateLoadingScreenClicked) ]

			+ SVerticalBox::Slot().AutoHeight().Padding(10, 14, 10, 2)
			[ SNew(STextBlock).Text(LOCTEXT("LevelHeader", "Levels")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 11)) ]

			+ SVerticalBox::Slot().AutoHeight().Padding(10, 2)
			[ SNew(SButton).Text(LOCTEXT("GenDebug", "Create Debug Level")).OnClicked_Static(&FAssetGenModule::OnCreateDebugLevelClicked) ]
		];
}

// --- Helpers ---

static UBlueprint* CreateOrLoadBlueprint(const FString& PackagePath, const FString& AssetName, UClass* ParentClass, bool& bOutCreated)
{
	bOutCreated = false;
	FString FullPath = PackagePath + TEXT(".") + AssetName;

	UBlueprint* BP = LoadObject<UBlueprint>(nullptr, *FullPath);
	if (BP) return BP;

	if (!ParentClass) return nullptr;

	UPackage* Package = CreatePackage(*PackagePath);
	if (!Package) return nullptr;

	BP = FKismetEditorUtilities::CreateBlueprint(
		ParentClass, Package, FName(*AssetName), BPTYPE_Normal,
		UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());

	if (BP)
	{
		FAssetRegistryModule::AssetCreated(BP);
		bOutCreated = true;
	}
	return BP;
}

static void SaveBlueprint(UBlueprint* BP)
{
	FKismetEditorUtilities::CompileBlueprint(BP);
	UPackage* Package = BP->GetOutermost();
	Package->MarkPackageDirty();
	FString Filename = FPackageName::LongPackageNameToFilename(
		Package->GetName(), FPackageName::GetAssetPackageExtension());
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	PlatformFile.CreateDirectoryTree(*FPaths::GetPath(Filename));
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	UPackage::SavePackage(Package, BP, *Filename, SaveArgs);
}

static void SetClassProperty(UObject* CDO, const TCHAR* PropName, UClass* Value)
{
	FClassProperty* Prop = CastField<FClassProperty>(CDO->GetClass()->FindPropertyByName(PropName));
	if (Prop && Value)
	{
		Prop->SetObjectPropertyValue(Prop->ContainerPtrToValuePtr<void>(CDO), Value);
	}
}

static bool SetObjectProperty(UObject* CDO, const TCHAR* PropName, UObject* Value)
{
	FObjectPropertyBase* Prop = CastField<FObjectPropertyBase>(CDO->GetClass()->FindPropertyByName(PropName));
	if (Prop && Value)
	{
		Prop->SetObjectPropertyValue(Prop->ContainerPtrToValuePtr<void>(CDO), Value);
		return true;
	}
	return false;
}

static UObject* GetObjectProperty(UObject* CDO, const TCHAR* PropName)
{
	FObjectPropertyBase* Prop = CastField<FObjectPropertyBase>(CDO->GetClass()->FindPropertyByName(PropName));
	if (Prop)
	{
		return Prop->GetObjectPropertyValue(Prop->ContainerPtrToValuePtr<void>(CDO));
	}
	return nullptr;
}

// --- Button Handlers ---

FReply FAssetGenModule::OnCreateGameModeClicked()
{
	bool bCreated = false;
	UBlueprint* BP = CreateOrLoadBlueprint(
		TEXT("/Game/Blueprints/Game/BP_GameMode"), TEXT("BP_GameMode"),
		ACBGameMode::StaticClass(), bCreated);

	if (!BP)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("GMFail", "Failed to create BP_GameMode."));
		return FReply::Handled();
	}

	FString Warnings;
	UObject* CDO = BP->GeneratedClass->GetDefaultObject();

	UBlueprint* BPChar = LoadObject<UBlueprint>(nullptr,
		TEXT("/Game/Blueprints/Player/BP_PlayerCharacter.BP_PlayerCharacter"));
	if (BPChar)
	{
		SetClassProperty(CDO, TEXT("DefaultPawnClass"), BPChar->GeneratedClass);
	}
	else
	{
		Warnings += TEXT("\n  - BP_PlayerCharacter not found (DefaultPawnClass not set)");
	}

	UBlueprint* BPPC = LoadObject<UBlueprint>(nullptr,
		TEXT("/Game/Blueprints/Player/BP_PlayerController.BP_PlayerController"));
	if (BPPC)
	{
		SetClassProperty(CDO, TEXT("PlayerControllerClass"), BPPC->GeneratedClass);
	}
	else
	{
		Warnings += TEXT("\n  - BP_PlayerController not found (PlayerControllerClass not set)");
	}

	SaveBlueprint(BP);

	FString Msg = bCreated ? TEXT("BP_GameMode created.") : TEXT("BP_GameMode updated.");
	if (!Warnings.IsEmpty()) { Msg += TEXT("\n\nWarnings:") + Warnings; }
	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Msg));
	return FReply::Handled();
}

FReply FAssetGenModule::OnCreateGameInstanceClicked()
{
	UClass* ParentClass = StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("/Script/CB.CBGameInstance"));
	if (!ParentClass)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("GINoParent", "Parent class /Script/CB.CBGameInstance not found."));
		return FReply::Handled();
	}

	bool bCreated = false;
	UBlueprint* BP = CreateOrLoadBlueprint(
		TEXT("/Game/Blueprints/Game/BP_GameInstance"), TEXT("BP_GameInstance"),
		ParentClass, bCreated);

	if (!BP)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("GIFail", "Failed to create BP_GameInstance."));
		return FReply::Handled();
	}

	SaveBlueprint(BP);

	FString Msg = bCreated ? TEXT("BP_GameInstance created.") : TEXT("BP_GameInstance already exists.");
	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Msg));
	return FReply::Handled();
}

FReply FAssetGenModule::OnCreatePlayerCharacterClicked()
{
	UClass* ParentClass = StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("/Script/CB.CBPlayerCharacter"));
	if (!ParentClass)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("CharNoParent", "Parent class /Script/CB.CBPlayerCharacter not found."));
		return FReply::Handled();
	}

	bool bCreated = false;
	UBlueprint* BP = CreateOrLoadBlueprint(
		TEXT("/Game/Blueprints/Player/BP_PlayerCharacter"), TEXT("BP_PlayerCharacter"),
		ParentClass, bCreated);

	if (!BP)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("CharFail", "Failed to create BP_PlayerCharacter."));
		return FReply::Handled();
	}

	FString Warnings;
	UObject* CDO = BP->GeneratedClass->GetDefaultObject();
	ACharacter* CharCDO = Cast<ACharacter>(CDO);

	// --- Skeletal Mesh ---
	if (CharCDO && CharCDO->GetMesh())
	{
		USkeletalMesh* SkelMesh = LoadObject<USkeletalMesh>(nullptr,
			TEXT("/Game/Characters/Manny/SKM_Manny_Simple.SKM_Manny_Simple"));
		if (SkelMesh)
		{
			CharCDO->GetMesh()->SetSkeletalMeshAsset(SkelMesh);
		}
		else
		{
			Warnings += TEXT("\n  - SKM_Manny_Simple not found (Mesh not set)");
		}
	}

	// --- Input Actions ---
	UInputAction* IA_MoveAxis = LoadObject<UInputAction>(nullptr,
		TEXT("/Game/Input/Gameplay/Actions/IA_MoveAxis.IA_MoveAxis"));
	UInputAction* IA_Jump = LoadObject<UInputAction>(nullptr,
		TEXT("/Game/Input/Gameplay/Actions/IA_Jump.IA_Jump"));
	UInputAction* IA_Spin = LoadObject<UInputAction>(nullptr,
		TEXT("/Game/Input/Gameplay/Actions/IA_Spin.IA_Spin"));
	UInputMappingContext* IMC = LoadObject<UInputMappingContext>(nullptr,
		TEXT("/Game/Input/Gameplay/IMC_Gameplay.IMC_Gameplay"));

	if (IA_MoveAxis) { SetObjectProperty(CDO, TEXT("IA_MoveAxis"), IA_MoveAxis); }
	else { Warnings += TEXT("\n  - IA_MoveAxis not found (run Create Input Actions first)"); }

	if (IA_Jump) { SetObjectProperty(CDO, TEXT("IA_Jump"), IA_Jump); }
	else { Warnings += TEXT("\n  - IA_Jump not found"); }

	if (IA_Spin) { SetObjectProperty(CDO, TEXT("IA_Spin"), IA_Spin); }
	else { Warnings += TEXT("\n  - IA_Spin not found (run Create Input Actions first)"); }

	if (IMC) { SetObjectProperty(CDO, TEXT("IMC_Gameplay"), IMC); }
	else { Warnings += TEXT("\n  - IMC_Gameplay not found"); }

	// --- Spin Montage ---
	UAnimMontage* SpinMontage = LoadObject<UAnimMontage>(nullptr,
		TEXT("/Game/Characters/Manny/Animations/AM_Manny_Spin.AM_Manny_Spin"));
	if (SpinMontage) { SetObjectProperty(CDO, TEXT("SpinMontage"), SpinMontage); }
	else { Warnings += TEXT("\n  - AM_Manny_Spin not found (run Create Spin Montage first)"); }

	SaveBlueprint(BP);

	FString Msg = bCreated ? TEXT("BP_PlayerCharacter created and configured.") : TEXT("BP_PlayerCharacter updated.");
	if (!Warnings.IsEmpty()) { Msg += TEXT("\n\nWarnings:") + Warnings; }
	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Msg));
	return FReply::Handled();
}

FReply FAssetGenModule::OnCreatePlayerControllerClicked()
{
	UClass* ParentClass = StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("/Script/CB.CBPlayerController"));
	if (!ParentClass)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("PCNoParent", "Parent class /Script/CB.CBPlayerController not found."));
		return FReply::Handled();
	}

	bool bCreated = false;
	UBlueprint* BP = CreateOrLoadBlueprint(
		TEXT("/Game/Blueprints/Player/BP_PlayerController"), TEXT("BP_PlayerController"),
		ParentClass, bCreated);

	if (!BP)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("PCFail", "Failed to create BP_PlayerController."));
		return FReply::Handled();
	}

	FString Warnings;
	UObject* CDO = BP->GeneratedClass->GetDefaultObject();

	// Wire up input actions and IMC
	const FString ActionsPath = TEXT("/Game/Input/Gameplay/Actions/");
	UInputAction* IA_MoveAxis = LoadObject<UInputAction>(nullptr, *(ActionsPath + TEXT("IA_MoveAxis.IA_MoveAxis")));
	UInputAction* IA_Jump = LoadObject<UInputAction>(nullptr, *(ActionsPath + TEXT("IA_Jump.IA_Jump")));
	UInputAction* IA_Spin = LoadObject<UInputAction>(nullptr, *(ActionsPath + TEXT("IA_Spin.IA_Spin")));
	UInputAction* IA_PauseMenu = LoadObject<UInputAction>(nullptr, *(ActionsPath + TEXT("IA_PauseMenu.IA_PauseMenu")));
	UInputMappingContext* IMC = LoadObject<UInputMappingContext>(nullptr,
		TEXT("/Game/Input/Gameplay/IMC_Gameplay.IMC_Gameplay"));

	if (IA_MoveAxis) { SetObjectProperty(CDO, TEXT("IA_MoveAxis"), IA_MoveAxis); }
	else { Warnings += TEXT("\n  - IA_MoveAxis not found"); }
	if (IA_Jump) { SetObjectProperty(CDO, TEXT("IA_Jump"), IA_Jump); }
	else { Warnings += TEXT("\n  - IA_Jump not found"); }
	if (IA_Spin) { SetObjectProperty(CDO, TEXT("IA_Spin"), IA_Spin); }
	else { Warnings += TEXT("\n  - IA_Spin not found"); }
	if (IA_PauseMenu) { SetObjectProperty(CDO, TEXT("IA_PauseMenu"), IA_PauseMenu); }
	else { Warnings += TEXT("\n  - IA_PauseMenu not found"); }
	if (IMC) { SetObjectProperty(CDO, TEXT("IMC_Gameplay"), IMC); }
	else { Warnings += TEXT("\n  - IMC_Gameplay not found"); }

	SaveBlueprint(BP);

	FString Msg = bCreated ? TEXT("BP_PlayerController created and configured.") : TEXT("BP_PlayerController updated.");
	if (!Warnings.IsEmpty()) { Msg += TEXT("\n\nWarnings:") + Warnings; }
	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Msg));
	return FReply::Handled();
}

// --- Movement Playground Helpers ---

static AStaticMeshActor* SpawnBox(UWorld* World, UStaticMesh* CubeMesh, UMaterial* Mat,
	const FVector& Location, const FVector& Scale, const FRotator& Rotation = FRotator::ZeroRotator)
{
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AStaticMeshActor* Actor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(),
		Location, Rotation, Params);
	if (Actor)
	{
		Actor->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
		Actor->SetActorScale3D(Scale);
		if (Mat)
		{
			Actor->GetStaticMeshComponent()->SetMaterial(0, Mat);
		}
	}
	return Actor;
}

static void SpawnLabel(UWorld* World, const FString& Text, const FVector& Location)
{
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ATextRenderActor* Label = World->SpawnActor<ATextRenderActor>(ATextRenderActor::StaticClass(),
		Location, FRotator(0.0f, 180.0f, 0.0f), Params);
	if (Label)
	{
		UTextRenderComponent* TextComp = Label->GetTextRender();
		TextComp->SetText(FText::FromString(Text));
		TextComp->SetWorldSize(50.0f);
		TextComp->SetHorizontalAlignment(EHTA_Center);
	}
}

static AActor* SpawnBP(UWorld* World, const TCHAR* BPPath, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator)
{
	UBlueprint* BP = LoadObject<UBlueprint>(nullptr, BPPath);
	if (!BP || !BP->GeneratedClass) return nullptr;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	return World->SpawnActor<AActor>(BP->GeneratedClass, Location, Rotation, Params);
}

static void BuildDebugLevel(UWorld* World)
{
	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	UMaterial* DefaultMat = LoadObject<UMaterial>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (!Cube) return;

	// One large floor platform — everything is on this
	SpawnBox(World, Cube, DefaultMat,
		FVector(0.0f, 0.0f, -50.0f), FVector(50.0f, 50.0f, 1.0f));

	// Player Start at origin
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		World->SpawnActor<APlayerStart>(APlayerStart::StaticClass(), FVector(0.0f, 0.0f, 50.0f),
			FRotator::ZeroRotator, Params);
	}

	// ===== Front: Basic crates (5 in a row) =====
	SpawnLabel(World, TEXT("BASIC"), FVector(400.0f, -400.0f, 150.0f));
	for (int32 i = 0; i < 5; ++i)
	{
		SpawnBP(World, TEXT("/Game/Blueprints/Crates/BP_Crate.BP_Crate"),
			FVector(400.0f + i * 120.0f, -400.0f, 35.0f));
	}

	// ===== TNT cluster =====
	SpawnLabel(World, TEXT("TNT"), FVector(400.0f, -100.0f, 150.0f));
	SpawnBP(World, TEXT("/Game/Blueprints/Crates/BP_CrateTNT.BP_CrateTNT"),
		FVector(400.0f, -100.0f, 35.0f));
	SpawnBP(World, TEXT("/Game/Blueprints/Crates/BP_CrateTNT.BP_CrateTNT"),
		FVector(520.0f, -50.0f, 35.0f));
	SpawnBP(World, TEXT("/Game/Blueprints/Crates/BP_CrateTNT.BP_CrateTNT"),
		FVector(520.0f, -150.0f, 35.0f));

	// ===== Bounce crates =====
	SpawnLabel(World, TEXT("BOUNCE"), FVector(400.0f, 200.0f, 150.0f));
	SpawnBP(World, TEXT("/Game/Blueprints/Crates/BP_CrateBounce.BP_CrateBounce"),
		FVector(400.0f, 200.0f, 35.0f));
	SpawnBP(World, TEXT("/Game/Blueprints/Crates/BP_CrateBounce.BP_CrateBounce"),
		FVector(550.0f, 200.0f, 35.0f));

	// ===== Arrow crates (wooden + iron) =====
	SpawnLabel(World, TEXT("ARROW"), FVector(400.0f, 500.0f, 150.0f));
	SpawnBP(World, TEXT("/Game/Blueprints/Crates/BP_CrateArrow.BP_CrateArrow"),
		FVector(400.0f, 500.0f, 35.0f));
	SpawnBP(World, TEXT("/Game/Blueprints/Crates/BP_CrateIronArrow.BP_CrateIronArrow"),
		FVector(550.0f, 500.0f, 35.0f));

	// ===== Iron crate stack =====
	SpawnLabel(World, TEXT("IRON"), FVector(400.0f, 800.0f, 150.0f));
	SpawnBP(World, TEXT("/Game/Blueprints/Crates/BP_CrateIron.BP_CrateIron"),
		FVector(400.0f, 800.0f, 35.0f));
	SpawnBP(World, TEXT("/Game/Blueprints/Crates/BP_CrateIron.BP_CrateIron"),
		FVector(400.0f, 800.0f, 105.0f));
	SpawnBP(World, TEXT("/Game/Blueprints/Crates/BP_CrateIron.BP_CrateIron"),
		FVector(400.0f, 800.0f, 175.0f));

	// ===== Left side: Pickups =====

	// Wumpa trail (20 fruit)
	SpawnLabel(World, TEXT("WUMPA"), FVector(-400.0f, -600.0f, 150.0f));
	for (int32 i = 0; i < 20; ++i)
	{
		SpawnBP(World, TEXT("/Game/Blueprints/Pickups/BP_WumpaFruit.BP_WumpaFruit"),
			FVector(-400.0f - i * 60.0f, -600.0f, 30.0f));
	}

	// Aku Aku pickups (3 for tier progression)
	SpawnLabel(World, TEXT("AKU AKU"), FVector(-400.0f, -200.0f, 150.0f));
	SpawnBP(World, TEXT("/Game/Blueprints/Pickups/BP_AkuAkuPickup.BP_AkuAkuPickup"),
		FVector(-400.0f, -200.0f, 30.0f));
	SpawnBP(World, TEXT("/Game/Blueprints/Pickups/BP_AkuAkuPickup.BP_AkuAkuPickup"),
		FVector(-550.0f, -200.0f, 30.0f));
	SpawnBP(World, TEXT("/Game/Blueprints/Pickups/BP_AkuAkuPickup.BP_AkuAkuPickup"),
		FVector(-700.0f, -200.0f, 30.0f));

	// Life pickup
	SpawnLabel(World, TEXT("LIFE"), FVector(-400.0f, 100.0f, 150.0f));
	SpawnBP(World, TEXT("/Game/Blueprints/Pickups/BP_LifePickup.BP_LifePickup"),
		FVector(-400.0f, 100.0f, 30.0f));

	// Aku Aku crates (Mask contents — 3 for tier testing)
	SpawnLabel(World, TEXT("MASK CRATES"), FVector(-400.0f, 400.0f, 150.0f));
	SpawnBP(World, TEXT("/Game/Blueprints/Crates/BP_Crate.BP_Crate"),
		FVector(-400.0f, 400.0f, 35.0f));
	SpawnBP(World, TEXT("/Game/Blueprints/Crates/BP_Crate.BP_Crate"),
		FVector(-550.0f, 400.0f, 35.0f));
	SpawnBP(World, TEXT("/Game/Blueprints/Crates/BP_Crate.BP_Crate"),
		FVector(-700.0f, 400.0f, 35.0f));

	// Life crate
	SpawnLabel(World, TEXT("LIFE CRATE"), FVector(-400.0f, 700.0f, 150.0f));
	SpawnBP(World, TEXT("/Game/Blueprints/Crates/BP_Crate.BP_Crate"),
		FVector(-400.0f, 700.0f, 35.0f));

	// ===== Behind: Movement tests =====

	// Ramp
	SpawnLabel(World, TEXT("RAMP"), FVector(0.0f, -900.0f, 200.0f));
	SpawnBox(World, Cube, DefaultMat,
		FVector(0.0f, -1100.0f, 50.0f), FVector(4.0f, 4.0f, 0.3f),
		FRotator(15.0f, 0.0f, 0.0f));

	// Platforms (ascending steps)
	SpawnLabel(World, TEXT("PLATFORMS"), FVector(0.0f, 900.0f, 150.0f));
	SpawnBox(World, Cube, DefaultMat,
		FVector(0.0f, 1000.0f, 50.0f), FVector(2.0f, 2.0f, 1.0f));
	SpawnBox(World, Cube, DefaultMat,
		FVector(0.0f, 1300.0f, 150.0f), FVector(2.0f, 2.0f, 1.0f));
	SpawnBox(World, Cube, DefaultMat,
		FVector(0.0f, 1600.0f, 300.0f), FVector(2.0f, 2.0f, 1.0f));

	// ===== Death pit (gap in floor at edge) =====
	SpawnLabel(World, TEXT("DEATH PIT"), FVector(1800.0f, 0.0f, 150.0f));
	SpawnBox(World, Cube, DefaultMat,
		FVector(2000.0f, 0.0f, -50.0f), FVector(2.0f, 3.0f, 1.0f));
	// Gap, then landing
	SpawnBox(World, Cube, DefaultMat,
		FVector(2500.0f, 0.0f, -50.0f), FVector(2.0f, 3.0f, 1.0f));
}

FReply FAssetGenModule::OnCreateDebugLevelClicked()
{
	UBlueprint* BPGameMode = LoadObject<UBlueprint>(nullptr,
		TEXT("/Game/Blueprints/Game/BP_GameMode.BP_GameMode"));
	if (!BPGameMode)
	{
		FMessageDialog::Open(EAppMsgType::Ok,
			LOCTEXT("NoGM", "BP_GameMode not found. Create it first."));
		return FReply::Handled();
	}

	UWorld* NewWorld = UEditorLoadingAndSavingUtils::NewBlankMap(true);
	if (!NewWorld)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("MapFail", "Failed to create new map."));
		return FReply::Handled();
	}

	AWorldSettings* WorldSettings = NewWorld->GetWorldSettings();
	if (WorldSettings)
	{
		WorldSettings->DefaultGameMode = BPGameMode->GeneratedClass;

		UBlueprint* BPCamera = LoadObject<UBlueprint>(nullptr,
			TEXT("/Game/Blueprints/Game/BP_Camera.BP_Camera"));
		if (BPCamera)
		{
			SetClassProperty(WorldSettings, TEXT("CameraActorClass"), BPCamera->GeneratedClass);
		}
	}

	// Directional light (sun)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ADirectionalLight* Sun = NewWorld->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(),
			FVector(0.0f, 0.0f, 1000.0f), FRotator(-50.0f, -30.0f, 0.0f), Params);
		if (Sun)
		{
			Sun->GetLightComponent()->SetIntensity(3.0f);
			Sun->SetActorLabel(TEXT("Sun"));
		}
	}

	// Build level geometry
	BuildDebugLevel(NewWorld);

	FString Warnings;
	// Check for missing blueprint assets
	const TCHAR* RequiredBPs[] = {
		TEXT("/Game/Blueprints/Crates/BP_Crate.BP_Crate"),
		TEXT("/Game/Blueprints/Crates/BP_CrateTNT.BP_CrateTNT"),
		TEXT("/Game/Blueprints/Crates/BP_CrateBounce.BP_CrateBounce"),
		TEXT("/Game/Blueprints/Crates/BP_CrateArrow.BP_CrateArrow"),
		TEXT("/Game/Blueprints/Crates/BP_CrateIronArrow.BP_CrateIronArrow"),
		TEXT("/Game/Blueprints/Crates/BP_CrateIron.BP_CrateIron"),
		TEXT("/Game/Blueprints/Pickups/BP_WumpaFruit.BP_WumpaFruit"),
		TEXT("/Game/Blueprints/Pickups/BP_AkuAkuPickup.BP_AkuAkuPickup"),
		TEXT("/Game/Blueprints/Pickups/BP_LifePickup.BP_LifePickup"),
	};
	for (const TCHAR* Path : RequiredBPs)
	{
		if (!LoadObject<UBlueprint>(nullptr, Path))
		{
			Warnings += FString::Printf(TEXT("\n  - %s not found"), Path);
		}
	}

	const FString LevelPath = TEXT("/Game/Maps/Debug/Debug");
	FString LevelFilename = FPackageName::LongPackageNameToFilename(
		LevelPath, FPackageName::GetMapPackageExtension());

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	PlatformFile.CreateDirectoryTree(*FPaths::GetPath(LevelFilename));

	UEditorLoadingAndSavingUtils::SaveMap(NewWorld, LevelPath);

	FString Msg = TEXT("Debug level created (compact layout around spawn):\n  Crates: Basic, TNT, Bounce, Arrow, Iron\n  Pickups: Wumpa trail, Aku Aku, Life\n  Movement: Ramp, Platforms, Death Pit");
	if (!Warnings.IsEmpty()) { Msg += TEXT("\n\nMissing blueprints (spawn skipped):") + Warnings; }
	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Msg));

	return FReply::Handled();
}

static UInputAction* CreateOrLoadInputAction(const FString& PackagePath, const FString& AssetName, EInputActionValueType ValueType, bool& bOutCreated)
{
	bOutCreated = false;
	FString FullPath = PackagePath + TEXT(".") + AssetName;

	UInputAction* IA = LoadObject<UInputAction>(nullptr, *FullPath);
	if (IA) return IA;

	UPackage* Package = CreatePackage(*PackagePath);
	if (!Package) return nullptr;

	IA = NewObject<UInputAction>(Package, UInputAction::StaticClass(), FName(*AssetName), RF_Public | RF_Standalone);
	if (IA)
	{
		IA->ValueType = ValueType;
		FAssetRegistryModule::AssetCreated(IA);
		Package->MarkPackageDirty();

		FString Filename = FPackageName::LongPackageNameToFilename(
			Package->GetName(), FPackageName::GetAssetPackageExtension());
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		PlatformFile.CreateDirectoryTree(*FPaths::GetPath(Filename));
		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		UPackage::SavePackage(Package, IA, *Filename, SaveArgs);

		bOutCreated = true;
	}
	return IA;
}

static UInputMappingContext* CreateOrLoadIMC(const FString& PackagePath, const FString& AssetName, bool& bOutCreated)
{
	bOutCreated = false;
	FString FullPath = PackagePath + TEXT(".") + AssetName;

	UInputMappingContext* IMC = LoadObject<UInputMappingContext>(nullptr, *FullPath);
	if (IMC) return IMC;

	UPackage* Package = CreatePackage(*PackagePath);
	if (!Package) return nullptr;

	IMC = NewObject<UInputMappingContext>(Package, UInputMappingContext::StaticClass(),
		FName(*AssetName), RF_Public | RF_Standalone);
	if (IMC)
	{
		FAssetRegistryModule::AssetCreated(IMC);
		Package->MarkPackageDirty();

		FString Filename = FPackageName::LongPackageNameToFilename(
			Package->GetName(), FPackageName::GetAssetPackageExtension());
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		PlatformFile.CreateDirectoryTree(*FPaths::GetPath(Filename));
		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		UPackage::SavePackage(Package, IMC, *Filename, SaveArgs);

		bOutCreated = true;
	}
	return IMC;
}

FReply FAssetGenModule::OnCreateInputActionsClicked()
{
	FString Results;
	const FString ActionsPath = TEXT("/Game/Input/Gameplay/Actions/");
	bool bCreated = false;

	UInputAction* IA_MoveAxis = CreateOrLoadInputAction(
		ActionsPath + TEXT("IA_MoveAxis"), TEXT("IA_MoveAxis"),
		EInputActionValueType::Axis2D, bCreated);
	Results += IA_MoveAxis ? (bCreated ? TEXT("IA_MoveAxis created (Axis2D).\n") : TEXT("IA_MoveAxis exists.\n"))
		: TEXT("Failed: IA_MoveAxis.\n");

	UInputAction* IA_Jump = CreateOrLoadInputAction(
		ActionsPath + TEXT("IA_Jump"), TEXT("IA_Jump"),
		EInputActionValueType::Boolean, bCreated);
	Results += IA_Jump ? (bCreated ? TEXT("IA_Jump created.\n") : TEXT("IA_Jump exists.\n"))
		: TEXT("Failed: IA_Jump.\n");

	UInputAction* IA_Spin = CreateOrLoadInputAction(
		ActionsPath + TEXT("IA_Spin"), TEXT("IA_Spin"),
		EInputActionValueType::Boolean, bCreated);
	Results += IA_Spin ? (bCreated ? TEXT("IA_Spin created.\n") : TEXT("IA_Spin exists.\n"))
		: TEXT("Failed: IA_Spin.\n");

	UInputAction* IA_PauseMenu = CreateOrLoadInputAction(
		ActionsPath + TEXT("IA_PauseMenu"), TEXT("IA_PauseMenu"),
		EInputActionValueType::Boolean, bCreated);
	Results += IA_PauseMenu ? (bCreated ? TEXT("IA_PauseMenu created.\n") : TEXT("IA_PauseMenu exists.\n"))
		: TEXT("Failed: IA_PauseMenu.\n");

	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Results));
	return FReply::Handled();
}

FReply FAssetGenModule::OnCreateIMCClicked()
{
	FString Results;
	const FString ActionsPath = TEXT("/Game/Input/Gameplay/Actions/");

	// Load input actions
	UInputAction* IA_MoveAxis = LoadObject<UInputAction>(nullptr, *(ActionsPath + TEXT("IA_MoveAxis.IA_MoveAxis")));
	UInputAction* IA_Jump = LoadObject<UInputAction>(nullptr, *(ActionsPath + TEXT("IA_Jump.IA_Jump")));
	UInputAction* IA_Spin = LoadObject<UInputAction>(nullptr, *(ActionsPath + TEXT("IA_Spin.IA_Spin")));
	UInputAction* IA_PauseMenu = LoadObject<UInputAction>(nullptr, *(ActionsPath + TEXT("IA_PauseMenu.IA_PauseMenu")));

	if (!IA_MoveAxis || !IA_Jump || !IA_Spin)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("IMCNoActions", "Input actions not found. Run Create Input Actions first."));
		return FReply::Handled();
	}

	// Create or load IMC
	bool bIMCCreated = false;
	UInputMappingContext* IMC = CreateOrLoadIMC(
		TEXT("/Game/Input/Gameplay/IMC_Gameplay"), TEXT("IMC_Gameplay"), bIMCCreated);

	if (!IMC)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("IMCFail", "Failed to create IMC_Gameplay."));
		return FReply::Handled();
	}

	IMC->UnmapAll();

	// --- IA_MoveAxis: WASD + Left Stick ---
	{
		// W — forward (+Y axis)
		FEnhancedActionKeyMapping& W = IMC->MapKey(IA_MoveAxis, EKeys::W);
		UInputModifierSwizzleAxis* SwizzleW = NewObject<UInputModifierSwizzleAxis>(IMC);
		SwizzleW->Order = EInputAxisSwizzle::YXZ;
		W.Modifiers.Add(SwizzleW);
	}
	{
		// S — backward (-Y axis)
		FEnhancedActionKeyMapping& S = IMC->MapKey(IA_MoveAxis, EKeys::S);
		UInputModifierSwizzleAxis* SwizzleS = NewObject<UInputModifierSwizzleAxis>(IMC);
		SwizzleS->Order = EInputAxisSwizzle::YXZ;
		S.Modifiers.Add(SwizzleS);
		S.Modifiers.Add(NewObject<UInputModifierNegate>(IMC));
	}
	IMC->MapKey(IA_MoveAxis, EKeys::D);
	{
		// A — left (-X axis)
		FEnhancedActionKeyMapping& A = IMC->MapKey(IA_MoveAxis, EKeys::A);
		A.Modifiers.Add(NewObject<UInputModifierNegate>(IMC));
	}
	{
		// Gamepad left stick
		FEnhancedActionKeyMapping& Stick = IMC->MapKey(IA_MoveAxis, EKeys::Gamepad_Left2D);
		UInputModifierDeadZone* DeadZone = NewObject<UInputModifierDeadZone>(IMC);
		DeadZone->LowerThreshold = 0.2f;
		Stick.Modifiers.Add(DeadZone);
	}
	Results += TEXT("IA_MoveAxis: WASD + Left Stick\n");

	// --- IA_Jump ---
	IMC->MapKey(IA_Jump, EKeys::SpaceBar);
	IMC->MapKey(IA_Jump, EKeys::Gamepad_FaceButton_Bottom);
	Results += TEXT("IA_Jump: Space, Gamepad A\n");

	// --- IA_Spin ---
	IMC->MapKey(IA_Spin, EKeys::LeftShift);
	IMC->MapKey(IA_Spin, EKeys::J);
	IMC->MapKey(IA_Spin, EKeys::LeftMouseButton);
	IMC->MapKey(IA_Spin, EKeys::Gamepad_FaceButton_Left);
	IMC->MapKey(IA_Spin, EKeys::Gamepad_FaceButton_Right);
	Results += TEXT("IA_Spin: LShift, J, LMB, Gamepad X/B\n");

	// --- IA_PauseMenu ---
	if (IA_PauseMenu)
	{
		IMC->MapKey(IA_PauseMenu, EKeys::Escape);
		IMC->MapKey(IA_PauseMenu, EKeys::Gamepad_Special_Right);
		Results += TEXT("IA_PauseMenu: Escape, Start\n");
	}

	// Save
	UPackage* IMCPackage = IMC->GetOutermost();
	IMCPackage->MarkPackageDirty();
	FString IMCFilename = FPackageName::LongPackageNameToFilename(
		IMCPackage->GetName(), FPackageName::GetAssetPackageExtension());
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	UPackage::SavePackage(IMCPackage, IMC, *IMCFilename, SaveArgs);

	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Results));
	return FReply::Handled();
}

FReply FAssetGenModule::OnCreateCameraClicked()
{
	UClass* ParentClass = StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("/Script/CB.CBCamera"));
	if (!ParentClass)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("CamNoParent", "Parent class /Script/CB.CBCamera not found."));
		return FReply::Handled();
	}

	bool bCreated = false;
	UBlueprint* BP = CreateOrLoadBlueprint(
		TEXT("/Game/Blueprints/Camera/BP_Camera"), TEXT("BP_Camera"),
		ParentClass, bCreated);

	if (!BP)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("CamFail", "Failed to create BP_Camera."));
		return FReply::Handled();
	}

	UObject* CDO = BP->GeneratedClass->GetDefaultObject();

	// Configure the spring arm for behind-the-back view
	USpringArmComponent* SpringArm = Cast<USpringArmComponent>(
		GetObjectProperty(CDO, TEXT("SpringArmComponent")));

	if (SpringArm)
	{
		SpringArm->TargetArmLength = 600.0f;
		SpringArm->SetRelativeRotation(FRotator(-15.0f, 0.0f, 0.0f));
		SpringArm->bInheritPitch = false;
		SpringArm->bInheritYaw = false;
		SpringArm->bInheritRoll = false;
		SpringArm->bDoCollisionTest = false;
	}

	SaveBlueprint(BP);

	FString Msg = bCreated
		? TEXT("BP_Camera created and configured (arm length 600, pitch -15).")
		: TEXT("BP_Camera updated (arm length 600, pitch -15).");
	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Msg));
	return FReply::Handled();
}

FReply FAssetGenModule::OnCreateSpinMontageClicked()
{
	const FString MontagePath = TEXT("/Game/Characters/Manny/Animations/AM_Manny_Spin");
	const FString MontageAssetName = TEXT("AM_Manny_Spin");

	// Check if montage already exists
	UAnimMontage* ExistingMontage = LoadObject<UAnimMontage>(nullptr, *(MontagePath + TEXT(".") + MontageAssetName));
	if (ExistingMontage)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("MontageExists", "AM_Manny_Spin already exists."));
		return FReply::Handled();
	}

	// Load the spin animation sequence
	UAnimSequence* SpinAnim = LoadObject<UAnimSequence>(nullptr,
		TEXT("/Game/Characters/Manny/Animations/A_Manny_Spin.A_Manny_Spin"));
	if (!SpinAnim)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoSpinAnim", "A_Manny_Spin not found at /Game/Characters/Manny/Animations/."));
		return FReply::Handled();
	}

	// Create the montage package
	UPackage* Package = CreatePackage(*MontagePath);
	if (!Package)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("MontagePkgFail", "Failed to create montage package."));
		return FReply::Handled();
	}

	UAnimMontage* Montage = NewObject<UAnimMontage>(Package, UAnimMontage::StaticClass(),
		FName(*MontageAssetName), RF_Public | RF_Standalone);

	if (!Montage)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("MontageFail", "Failed to create AnimMontage."));
		return FReply::Handled();
	}

	// Set the skeleton from the animation
	Montage->SetSkeleton(SpinAnim->GetSkeleton());

	// Create a composite section with the spin animation
	FSlotAnimationTrack& SlotTrack = Montage->SlotAnimTracks[0];
	SlotTrack.SlotName = FName("DefaultSlot");

	FAnimSegment Segment;
	Segment.SetAnimReference(SpinAnim);
	Segment.StartPos = 0.0f;
	Segment.AnimEndTime = SpinAnim->GetPlayLength();

	FCompositeSection& DefaultSection = Montage->CompositeSections[0];

	SlotTrack.AnimTrack.AnimSegments.Add(Segment);

	// Disable root motion
	Montage->bEnableRootMotionTranslation = false;
	Montage->bEnableRootMotionRotation = false;

	// Update cached data
	Montage->UpdateLinkableElements();
	Montage->MarkPackageDirty();

	FAssetRegistryModule::AssetCreated(Montage);

	// Save
	FString Filename = FPackageName::LongPackageNameToFilename(
		Package->GetName(), FPackageName::GetAssetPackageExtension());
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	PlatformFile.CreateDirectoryTree(*FPaths::GetPath(Filename));
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	UPackage::SavePackage(Package, Montage, *Filename, SaveArgs);

	FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("MontageCreated", "AM_Manny_Spin montage created at /Game/Characters/Manny/Animations/."));
	return FReply::Handled();
}

// --- Phase 2: Crates & Collectibles ---

FReply FAssetGenModule::OnCreateCratesClicked()
{
	struct FCrateInfo { const TCHAR* Name; const TCHAR* Class; };
	FCrateInfo Crates[] = {
		{ TEXT("BP_Crate"),          TEXT("/Script/CB.Crate") },
		{ TEXT("BP_CrateIron"),      TEXT("/Script/CB.CrateIron") },
		{ TEXT("BP_CrateTNT"),       TEXT("/Script/CB.CrateTNT") },
		{ TEXT("BP_CrateBounce"),    TEXT("/Script/CB.CrateBounce") },
		{ TEXT("BP_CrateArrow"),     TEXT("/Script/CB.CrateArrow") },
		{ TEXT("BP_CrateIronArrow"), TEXT("/Script/CB.CrateIronArrow") },
	};

	FString Results;
	UStaticMesh* CrateMesh = LoadObject<UStaticMesh>(nullptr,
		TEXT("/Game/Meshes/Crates/SM_Crate.SM_Crate"));

	for (const FCrateInfo& Info : Crates)
	{
		UClass* ParentClass = StaticLoadClass(UObject::StaticClass(), nullptr, Info.Class);
		if (!ParentClass)
		{
			Results += FString::Printf(TEXT("FAILED: %s (class %s not found)\n"), Info.Name, Info.Class);
			continue;
		}

		FString Path = FString::Printf(TEXT("/Game/Blueprints/Crates/%s"), Info.Name);
		bool bCreated = false;
		UBlueprint* BP = CreateOrLoadBlueprint(Path, Info.Name, ParentClass, bCreated);
		if (!BP)
		{
			Results += FString::Printf(TEXT("FAILED: %s\n"), Info.Name);
			continue;
		}

		// Set the crate mesh on MeshComponent
		if (CrateMesh)
		{
			UObject* CDO = BP->GeneratedClass->GetDefaultObject();
			UObject* MeshComp = GetObjectProperty(CDO, TEXT("MeshComponent"));
			if (UStaticMeshComponent* SMC = Cast<UStaticMeshComponent>(MeshComp))
			{
				SMC->SetStaticMesh(CrateMesh);
			}
		}

		SaveBlueprint(BP);
		Results += FString::Printf(TEXT("%s: %s\n"), Info.Name, bCreated ? TEXT("created") : TEXT("exists"));
	}

	if (!CrateMesh)
	{
		Results += TEXT("\nWarning: SM_Crate not found at /Game/Meshes/Crates/. Import it first.");
	}

	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Results));
	return FReply::Handled();
}

FReply FAssetGenModule::OnCreatePickupsClicked()
{
	struct FPickupInfo { const TCHAR* Name; const TCHAR* Class; const TCHAR* MeshPath; };
	FPickupInfo Pickups[] = {
		{ TEXT("BP_WumpaFruit"),  TEXT("/Script/CB.WumpaFruit"),  TEXT("/Game/Meshes/Pickups/SM_WumpaFruit.SM_WumpaFruit") },
		{ TEXT("BP_AkuAkuPickup"), TEXT("/Script/CB.AkuAkuPickup"), nullptr },
		{ TEXT("BP_LifePickup"),  TEXT("/Script/CB.LifePickup"),  nullptr },
	};

	FString Results;

	for (const FPickupInfo& Info : Pickups)
	{
		UClass* ParentClass = StaticLoadClass(UObject::StaticClass(), nullptr, Info.Class);
		if (!ParentClass)
		{
			Results += FString::Printf(TEXT("FAILED: %s (class %s not found)\n"), Info.Name, Info.Class);
			continue;
		}

		FString Path = FString::Printf(TEXT("/Game/Blueprints/Pickups/%s"), Info.Name);
		bool bCreated = false;
		UBlueprint* BP = CreateOrLoadBlueprint(Path, Info.Name, ParentClass, bCreated);
		if (!BP)
		{
			Results += FString::Printf(TEXT("FAILED: %s\n"), Info.Name);
			continue;
		}

		if (Info.MeshPath)
		{
			UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, Info.MeshPath);
			if (Mesh)
			{
				UObject* CDO = BP->GeneratedClass->GetDefaultObject();
				UObject* MeshComp = GetObjectProperty(CDO, TEXT("MeshComponent"));
				if (UStaticMeshComponent* SMC = Cast<UStaticMeshComponent>(MeshComp))
				{
					SMC->SetStaticMesh(Mesh);
				}
			}
		}

		SaveBlueprint(BP);
		Results += FString::Printf(TEXT("%s: %s\n"), Info.Name, bCreated ? TEXT("created") : TEXT("exists"));
	}

	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Results));
	return FReply::Handled();
}

FReply FAssetGenModule::OnCreateAkuAkuMaskClicked()
{
	UClass* ParentClass = StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("/Script/CB.AkuAkuMaskActor"));
	if (!ParentClass)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("MaskNoParent", "Parent class /Script/CB.AkuAkuMaskActor not found."));
		return FReply::Handled();
	}

	bool bCreated = false;
	UBlueprint* BP = CreateOrLoadBlueprint(
		TEXT("/Game/Blueprints/AkuAku/BP_AkuAkuMask"), TEXT("BP_AkuAkuMask"),
		ParentClass, bCreated);

	if (!BP)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("MaskFail", "Failed to create BP_AkuAkuMask."));
		return FReply::Handled();
	}

	FString Warnings;
	UObject* CDO = BP->GeneratedClass->GetDefaultObject();

	// Set the mask mesh
	UStaticMesh* MaskMesh = LoadObject<UStaticMesh>(nullptr,
		TEXT("/Game/Meshes/AkuAku/SM_AkuAku.SM_AkuAku"));
	if (MaskMesh)
	{
		UObject* MeshComp = GetObjectProperty(CDO, TEXT("MeshComponent"));
		if (UStaticMeshComponent* SMC = Cast<UStaticMeshComponent>(MeshComp))
		{
			SMC->SetStaticMesh(MaskMesh);
		}
	}
	else
	{
		Warnings += TEXT("\n  - SM_AkuAku not found. Import it first.");
	}

	SaveBlueprint(BP);

	FString Msg = bCreated ? TEXT("BP_AkuAkuMask created.") : TEXT("BP_AkuAkuMask updated.");
	if (!Warnings.IsEmpty()) { Msg += TEXT("\n\nWarnings:") + Warnings; }
	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Msg));
	return FReply::Handled();
}

FReply FAssetGenModule::OnCreateGameplayHUDClicked()
{
	const FString PackagePath = TEXT("/Game/Blueprints/UI/WBP_GameplayHUD");
	const FString AssetName = TEXT("WBP_GameplayHUD");
	const FString FullPath = PackagePath + TEXT(".") + AssetName;

	// Check if already exists
	UWidgetBlueprint* ExistingWBP = LoadObject<UWidgetBlueprint>(nullptr, *FullPath);
	if (ExistingWBP)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("HUDExists", "WBP_GameplayHUD already exists."));
		return FReply::Handled();
	}

	// Load parent class
	UClass* ParentClass = StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("/Script/CB.CBGameplayHUD"));
	if (!ParentClass)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("HUDNoParent", "Parent class CBGameplayHUD not found."));
		return FReply::Handled();
	}

	// Create package and widget blueprint
	UPackage* Package = CreatePackage(*PackagePath);
	if (!Package)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("HUDPkgFail", "Failed to create package."));
		return FReply::Handled();
	}

	UWidgetBlueprint* WBP = CastChecked<UWidgetBlueprint>(
		FKismetEditorUtilities::CreateBlueprint(
			ParentClass, Package, FName(*AssetName), BPTYPE_Normal,
			UWidgetBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass()));

	if (!WBP)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("HUDFail", "Failed to create WBP_GameplayHUD."));
		return FReply::Handled();
	}

	// Build widget tree: CanvasPanel root with two TextBlocks
	UCanvasPanel* Canvas = NewObject<UCanvasPanel>(WBP, UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	WBP->WidgetTree->RootWidget = Canvas;

	// Lives text — top left
	UTextBlock* LivesText = WBP->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LivesText"));
	UCanvasPanelSlot* LivesSlot = Canvas->AddChildToCanvas(LivesText);
	LivesSlot->SetAnchors(FAnchors(0.0f, 0.0f));
	LivesSlot->SetPosition(FVector2D(40.0f, 20.0f));
	LivesSlot->SetAutoSize(true);
	LivesText->SetText(FText::FromString(TEXT("4")));
	FSlateFontInfo LivesFont = LivesText->GetFont();
	LivesFont.Size = 32;
	LivesText->SetFont(LivesFont);

	// Wumpa text — top right
	UTextBlock* WumpaText = WBP->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("WumpaText"));
	UCanvasPanelSlot* WumpaSlot = Canvas->AddChildToCanvas(WumpaText);
	WumpaSlot->SetAnchors(FAnchors(1.0f, 0.0f));
	WumpaSlot->SetPosition(FVector2D(-120.0f, 20.0f));
	WumpaSlot->SetAutoSize(true);
	WumpaText->SetText(FText::FromString(TEXT("0")));
	FSlateFontInfo WumpaFont = WumpaText->GetFont();
	WumpaFont.Size = 32;
	WumpaText->SetFont(WumpaFont);

	// Compile and save
	FKismetEditorUtilities::CompileBlueprint(WBP);
	FAssetRegistryModule::AssetCreated(WBP);
	Package->MarkPackageDirty();

	FString Filename = FPackageName::LongPackageNameToFilename(
		Package->GetName(), FPackageName::GetAssetPackageExtension());
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	PlatformFile.CreateDirectoryTree(*FPaths::GetPath(Filename));
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	UPackage::SavePackage(Package, WBP, *Filename, SaveArgs);

	FMessageDialog::Open(EAppMsgType::Ok,
		LOCTEXT("HUDCreated", "WBP_GameplayHUD created with LivesText (top-left) and WumpaText (top-right).\n\nAssign it to BP_PlayerController > Gameplay HUD Class."));
	return FReply::Handled();
}

FReply FAssetGenModule::OnCreateLoadingScreenClicked()
{
	const FString PackagePath = TEXT("/Game/Blueprints/UI/WBP_LoadingScreen");
	const FString AssetName = TEXT("WBP_LoadingScreen");
	const FString FullPath = PackagePath + TEXT(".") + AssetName;

	UWidgetBlueprint* ExistingWBP = LoadObject<UWidgetBlueprint>(nullptr, *FullPath);
	if (ExistingWBP)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("LoadExists", "WBP_LoadingScreen already exists."));
		return FReply::Handled();
	}

	UPackage* Package = CreatePackage(*PackagePath);
	if (!Package)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("LoadPkgFail", "Failed to create package."));
		return FReply::Handled();
	}

	UWidgetBlueprint* WBP = CastChecked<UWidgetBlueprint>(
		FKismetEditorUtilities::CreateBlueprint(
			UUserWidget::StaticClass(), Package, FName(*AssetName), BPTYPE_Normal,
			UWidgetBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass()));

	if (!WBP)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("LoadFail", "Failed to create WBP_LoadingScreen."));
		return FReply::Handled();
	}

	UCanvasPanel* Canvas = NewObject<UCanvasPanel>(WBP, UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	WBP->WidgetTree->RootWidget = Canvas;

	UTextBlock* LoadingText = WBP->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LoadingText"));
	UCanvasPanelSlot* Slot = Canvas->AddChildToCanvas(LoadingText);
	Slot->SetAnchors(FAnchors(0.5f, 0.5f));
	Slot->SetAlignment(FVector2D(0.5f, 0.5f));
	Slot->SetPosition(FVector2D(0.0f, 0.0f));
	Slot->SetAutoSize(true);
	LoadingText->SetText(FText::FromString(TEXT("Loading Screen")));
	FSlateFontInfo Font = LoadingText->GetFont();
	Font.Size = 48;
	LoadingText->SetFont(Font);
	LoadingText->SetColorAndOpacity(FSlateColor(FLinearColor::White));

	FKismetEditorUtilities::CompileBlueprint(WBP);
	FAssetRegistryModule::AssetCreated(WBP);
	Package->MarkPackageDirty();

	FString Filename = FPackageName::LongPackageNameToFilename(
		Package->GetName(), FPackageName::GetAssetPackageExtension());
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	PlatformFile.CreateDirectoryTree(*FPaths::GetPath(Filename));
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	UPackage::SavePackage(Package, WBP, *Filename, SaveArgs);

	FMessageDialog::Open(EAppMsgType::Ok,
		LOCTEXT("LoadCreated", "WBP_LoadingScreen created with centered \"Loading Screen\" text."));
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAssetGenModule, AssetGen)
