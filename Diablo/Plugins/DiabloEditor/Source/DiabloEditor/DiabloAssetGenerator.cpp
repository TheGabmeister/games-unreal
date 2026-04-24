#include "DiabloAssetGenerator.h"
#include "DiabloGameMode.h"
#include "DiabloHero.h"
#include "DiabloPlayerController.h"
#include "DiabloEnemy.h"
#include "DroppedItem.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "FileHelpers.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/PackageName.h"
#include "UObject/SavePackage.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/PlayerStart.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "Builders/CubeBuilder.h"
#include "BSPOps.h"
#include "Engine/Polys.h"
#include "Model.h"
#include "Components/BrushComponent.h"
#include "Animation/AnimBlueprint.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "AssetImportTask.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "Factories/FbxFactory.h"
#include "Factories/FbxImportUI.h"
#include "RenderingThread.h"

void FDiabloAssetGenerator::GenerateAllAssets()
{
	GenerateBlueprintSubclasses();
	GenerateDefaultMap();
	GenerateInputAssets();
	ImportWarriorFBX();
	ImportPotionFBX();
	ConfigureBlueprintDefaults();

	UE_LOG(LogTemp, Display, TEXT("[DiabloTools] All assets generated."));
}

void FDiabloAssetGenerator::GenerateBlueprintSubclasses()
{
	CreateBlueprintFromClass(
		ADiabloGameMode::StaticClass(),
		TEXT("/Game/Blueprints"),
		TEXT("BP_DiabloGameMode")
	);

	CreateBlueprintFromClass(
		ADiabloHero::StaticClass(),
		TEXT("/Game/Blueprints"),
		TEXT("BP_DiabloHero")
	);

	CreateBlueprintFromClass(
		ADiabloPlayerController::StaticClass(),
		TEXT("/Game/Blueprints"),
		TEXT("BP_DiabloPlayerController")
	);

	CreateBlueprintFromClass(
		ADiabloEnemy::StaticClass(),
		TEXT("/Game/Blueprints"),
		TEXT("BP_DiabloEnemy")
	);

	CreateBlueprintFromClass(
		ADroppedItem::StaticClass(),
		TEXT("/Game/Blueprints"),
		TEXT("BP_HealingPotion")
	);
}

void FDiabloAssetGenerator::GenerateDefaultMap()
{
	const FString MapPackagePath = TEXT("/Game/Maps/Lvl_Diablo");

	UWorld* NewWorld = UEditorLoadingAndSavingUtils::NewBlankMap(false);
	if (!NewWorld)
	{
		UE_LOG(LogTemp, Error, TEXT("[DiabloTools] Failed to create blank map"));
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	NewWorld->SpawnActor<APlayerStart>(APlayerStart::StaticClass(), FTransform(FVector(0.f, 0.f, 100.f)), SpawnParams);

	ADirectionalLight* Sun = NewWorld->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(), FTransform::Identity, SpawnParams);
	if (Sun)
	{
		Sun->SetActorRotation(FRotator(-45.f, -45.f, 0.f));
	}

	SpawnFloorPlane(NewWorld);
	SpawnNavMeshVolume(NewWorld);

	// Spawn a test enemy from the BP class
	UBlueprint* EnemyBP = LoadObject<UBlueprint>(nullptr, TEXT("/Game/Blueprints/BP_DiabloEnemy.BP_DiabloEnemy"));
	if (EnemyBP && EnemyBP->GeneratedClass)
	{
		NewWorld->SpawnActor<AActor>(EnemyBP->GeneratedClass, FTransform(FVector(500.f, 0.f, 100.f)), SpawnParams);
		UE_LOG(LogTemp, Display, TEXT("[DiabloTools] Spawned BP_DiabloEnemy in map"));
	}

	// Spawn a healing potion
	UBlueprint* PotionBP = LoadObject<UBlueprint>(nullptr, TEXT("/Game/Blueprints/BP_HealingPotion.BP_HealingPotion"));
	if (PotionBP && PotionBP->GeneratedClass)
	{
		NewWorld->SpawnActor<AActor>(PotionBP->GeneratedClass, FTransform(FVector(300.f, 200.f, 100.f)), SpawnParams);
		UE_LOG(LogTemp, Display, TEXT("[DiabloTools] Spawned BP_HealingPotion in map"));
	}

	const FString FilePath = FPackageName::LongPackageNameToFilename(MapPackagePath, FPackageName::GetMapPackageExtension());
	FEditorFileUtils::SaveMap(NewWorld, FilePath);

	UE_LOG(LogTemp, Display, TEXT("[DiabloTools] Created map: %s"), *MapPackagePath);
}

void FDiabloAssetGenerator::SpawnFloorPlane(UWorld* World)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AStaticMeshActor* Floor = World->SpawnActor<AStaticMeshActor>(
		AStaticMeshActor::StaticClass(), FTransform::Identity, SpawnParams);

	if (!Floor)
	{
		UE_LOG(LogTemp, Error, TEXT("[DiabloTools] Failed to spawn floor actor"));
		return;
	}

	UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(
		nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));

	if (PlaneMesh)
	{
		Floor->GetStaticMeshComponent()->SetStaticMesh(PlaneMesh);
		Floor->SetActorScale3D(FVector(100.f, 100.f, 1.f));
		Floor->SetActorLabel(TEXT("Floor"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[DiabloTools] Failed to load /Engine/BasicShapes/Plane"));
	}
}

void FDiabloAssetGenerator::SpawnNavMeshVolume(UWorld* World)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ANavMeshBoundsVolume* NavVol = World->SpawnActor<ANavMeshBoundsVolume>(
		ANavMeshBoundsVolume::StaticClass(), FTransform::Identity, SpawnParams);

	if (!NavVol)
	{
		UE_LOG(LogTemp, Error, TEXT("[DiabloTools] Failed to spawn NavMeshBoundsVolume"));
		return;
	}

	NavVol->PreEditChange(nullptr);

	NavVol->Brush = NewObject<UModel>(NavVol, NAME_None, RF_Transactional);
	NavVol->Brush->Initialize(nullptr, true);
	NavVol->Brush->Polys = NewObject<UPolys>(NavVol->Brush, NAME_None, RF_Transactional);
	NavVol->GetBrushComponent()->Brush = NavVol->Brush;

	UCubeBuilder* Builder = NewObject<UCubeBuilder>();
	Builder->X = 10000.f;
	Builder->Y = 10000.f;
	Builder->Z = 2000.f;
	NavVol->BrushBuilder = DuplicateObject<UBrushBuilder>(Builder, NavVol);
	Builder->Build(World, NavVol);
	FBSPOps::csgPrepMovingBrush(NavVol);

	for (int32 i = 0; i < NavVol->Brush->Polys->Element.Num(); ++i)
	{
		NavVol->Brush->Polys->Element[i].Material = nullptr;
	}

	NavVol->PostEditChange();
	NavVol->SetActorLabel(TEXT("NavMeshBounds"));

	UE_LOG(LogTemp, Display, TEXT("[DiabloTools] Created NavMeshBoundsVolume"));
}

void FDiabloAssetGenerator::GenerateInputAssets()
{
	struct FInputActionDef
	{
		FString Name;
		EInputActionValueType ValueType;
		FKey DefaultKey;
	};

	TArray<FInputActionDef> Actions = {
		{ TEXT("IA_Click"), EInputActionValueType::Boolean, EKeys::LeftMouseButton },
		{ TEXT("IA_Move"),  EInputActionValueType::Axis2D,  EKeys::Invalid },
		{ TEXT("IA_Look"),  EInputActionValueType::Axis2D,  EKeys::Invalid },
	};

	const FString InputBasePath = TEXT("/Game/Input/Actions");
	TArray<UInputAction*> CreatedActions;

	for (const FInputActionDef& Def : Actions)
	{
		const FString FullPath = InputBasePath / Def.Name;
		const FString ObjPath = FullPath + TEXT(".") + Def.Name;

		UInputAction* Existing = LoadObject<UInputAction>(nullptr, *ObjPath);
		if (Existing)
		{
			Existing->ValueType = Def.ValueType;
			SaveAsset(Existing, Existing->GetOutermost(), FullPath);
			CreatedActions.Add(Existing);
			UE_LOG(LogTemp, Display, TEXT("[DiabloTools] Updated input action: %s"), *Def.Name);
			continue;
		}

		UPackage* Package = CreatePackage(*FullPath);
		Package->FullyLoad();

		UInputAction* Action = NewObject<UInputAction>(
			Package,
			*Def.Name,
			RF_Public | RF_Standalone
		);
		Action->ValueType = Def.ValueType;

		if (SaveAsset(Action, Package, FullPath))
		{
			NotifyAssetCreated(Action);
			CreatedActions.Add(Action);
			UE_LOG(LogTemp, Display, TEXT("[DiabloTools] Created input action: %s"), *Def.Name);
		}
	}

	const FString IMCPath = TEXT("/Game/Input/IMC_Diablo");
	const FString IMCObjPath = IMCPath + TEXT(".IMC_Diablo");
	{
		UInputMappingContext* IMC = LoadObject<UInputMappingContext>(nullptr, *IMCObjPath);
		UPackage* IMCPackage = nullptr;

		if (IMC)
		{
			IMC->UnmapAll();
			IMCPackage = IMC->GetOutermost();
		}
		else
		{
			IMCPackage = CreatePackage(*IMCPath);
			IMCPackage->FullyLoad();
			IMC = NewObject<UInputMappingContext>(
				IMCPackage,
				TEXT("IMC_Diablo"),
				RF_Public | RF_Standalone
			);
			NotifyAssetCreated(IMC);
		}

		for (int32 i = 0; i < CreatedActions.Num(); ++i)
		{
			IMC->MapKey(CreatedActions[i], Actions[i].DefaultKey);
		}

		SaveAsset(IMC, IMCPackage, IMCPath);
		UE_LOG(LogTemp, Display, TEXT("[DiabloTools] Updated input mapping context: IMC_Diablo"));
	}
}

// ---------------------------------------------------------------------------
// FBX Import
// ---------------------------------------------------------------------------

void FDiabloAssetGenerator::ImportWarriorFBX()
{
	const FString DestPath = TEXT("/Game/Characters/Warrior");

	const FString FBXPath = FPaths::ProjectDir() / TEXT("Tools/blender/out/Warrior.fbx");
	if (!FPaths::FileExists(FBXPath))
	{
		UE_LOG(LogTemp, Error, TEXT("[DiabloTools] Warrior.fbx not found at %s — run warrior_mesh.py first"), *FBXPath);
		return;
	}

	// Delete existing assets except manually-authored ones (ABP_Warrior,
	// AM_Attack) and the skeleton (preserving it keeps the AnimBP wired).
	IFileManager& FM = IFileManager::Get();
	const FString ContentDir = FPackageName::LongPackageNameToFilename(DestPath, TEXT(""));
	TArray<FString> AssetFiles;
	FM.FindFiles(AssetFiles, *(ContentDir / TEXT("*.uasset")), true, false);

	for (const FString& FileName : AssetFiles)
	{
		if (FileName.Contains(TEXT("ABP_Warrior"))
			|| FileName.Contains(TEXT("AM_Attack"))
			|| FileName.Contains(TEXT("AM_Death"))
			|| FileName.Contains(TEXT("Warrior_Skeleton")))
		{
			continue;
		}

		FString AssetName = FPaths::GetBaseFilename(FileName);
		FString PackagePath = DestPath / AssetName;
		UPackage* Pkg = FindPackage(nullptr, *PackagePath);
		if (Pkg)
		{
			TArray<UObject*> ObjectsInPackage;
			GetObjectsWithPackage(Pkg, ObjectsInPackage);
			for (UObject* Obj : ObjectsInPackage)
			{
				if (!Obj->IsRooted())
				{
					Obj->ClearFlags(RF_Standalone | RF_Public);
					Obj->MarkAsGarbage();
				}
			}
		}

		FM.Delete(*(ContentDir / FileName));
		UE_LOG(LogTemp, Display, TEXT("[DiabloTools] Deleted %s"), *FileName);
	}

	CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
	FlushRenderingCommands();
	FAssetRegistryModule::GetRegistry().ScanPathsSynchronous({DestPath}, true);

	// If the skeleton already exists, tell the importer to reuse it so
	// ABP_Warrior and AM_Attack references stay valid.
	USkeleton* ExistingSkeleton = LoadObject<USkeleton>(nullptr, TEXT("/Game/Characters/Warrior/Warrior_Skeleton.Warrior_Skeleton"));

	UFbxFactory* FbxFactory = NewObject<UFbxFactory>();
	FbxFactory->ImportUI->bImportMesh = true;
	FbxFactory->ImportUI->bImportAnimations = true;
	FbxFactory->ImportUI->bImportMaterials = false;
	FbxFactory->ImportUI->bImportTextures = false;
	FbxFactory->ImportUI->bIsObjImport = false;
	FbxFactory->ImportUI->bAutomatedImportShouldDetectType = false;
	FbxFactory->ImportUI->MeshTypeToImport = FBXIT_SkeletalMesh;
	FbxFactory->ImportUI->bImportAsSkeletal = true;
	if (ExistingSkeleton)
	{
		FbxFactory->ImportUI->Skeleton = ExistingSkeleton;
		UE_LOG(LogTemp, Display, TEXT("[DiabloTools] Reusing existing Warrior_Skeleton"));
	}

	UAssetImportTask* Task = NewObject<UAssetImportTask>();
	Task->Filename = FBXPath;
	Task->DestinationPath = DestPath;
	Task->DestinationName = TEXT("Warrior");
	Task->bReplaceExisting = true;
	Task->bAutomated = true;
	Task->bSave = true;
	Task->Factory = FbxFactory;

	IAssetTools& AssetTools = FAssetToolsModule::GetModule().Get();
	AssetTools.ImportAssetTasks({ Task });

	TArray<UObject*> ImportedObjects = Task->GetObjects();
	UE_LOG(LogTemp, Display, TEXT("[DiabloTools] Imported %d objects from Warrior.fbx"), ImportedObjects.Num());
	for (UObject* Obj : ImportedObjects)
	{
		UE_LOG(LogTemp, Display, TEXT("[DiabloTools]   -> %s (%s)"), *Obj->GetName(), *Obj->GetClass()->GetName());
	}

	// The importer creates animation sequences in memory but may not save them
	// when the skeleton was freshly recreated. Scan for unsaved AnimSequences
	// in the destination package path and save them explicitly.
	FAssetRegistryModule::GetRegistry().ScanPathsSynchronous({DestPath}, true);
	FARFilter Filter;
	Filter.PackagePaths.Add(FName(*DestPath));
	Filter.ClassPaths.Add(UAnimSequence::StaticClass()->GetClassPathName());
	TArray<FAssetData> AnimAssets;
	FAssetRegistryModule::GetRegistry().GetAssets(Filter, AnimAssets);
	for (const FAssetData& Asset : AnimAssets)
	{
		if (UObject* AnimObj = Asset.GetAsset())
		{
			UPackage* Pkg = AnimObj->GetOutermost();
			if (Pkg && Pkg->IsDirty())
			{
				FString PackageFilename = FPackageName::LongPackageNameToFilename(Pkg->GetName(), FPackageName::GetAssetPackageExtension());
				FSavePackageArgs SaveArgs;
				SaveArgs.TopLevelFlags = RF_Standalone;
				UPackage::SavePackage(Pkg, AnimObj, *PackageFilename, SaveArgs);
				UE_LOG(LogTemp, Display, TEXT("[DiabloTools] Saved animation: %s"), *AnimObj->GetName());
			}
		}
	}
}

// ---------------------------------------------------------------------------
// Import attack sound effects
// ---------------------------------------------------------------------------

void FDiabloAssetGenerator::ImportAttackSFX()
{
	const FString DestPath = TEXT("/Game/Audio/SFX");
	const TArray<FString> WavNames = { TEXT("SwordSwing"), TEXT("SwordHit") };

	IAssetTools& AssetTools = FAssetToolsModule::GetModule().Get();

	for (const FString& Name : WavNames)
	{
		const FString WavPath = FPaths::ProjectDir() / TEXT("Tools/audio/out") / (Name + TEXT(".wav"));
		if (!FPaths::FileExists(WavPath))
		{
			UE_LOG(LogTemp, Error, TEXT("[DiabloTools] %s.wav not found at %s — run attack_sfx.py first"), *Name, *WavPath);
			continue;
		}

		UAssetImportTask* Task = NewObject<UAssetImportTask>();
		Task->Filename = WavPath;
		Task->DestinationPath = DestPath;
		Task->DestinationName = Name;
		Task->bReplaceExisting = true;
		Task->bAutomated = true;
		Task->bSave = true;

		AssetTools.ImportAssetTasks({ Task });
		UE_LOG(LogTemp, Display, TEXT("[DiabloTools] Imported %s"), *Name);
	}
}

// ---------------------------------------------------------------------------
// Import healing potion static mesh
// ---------------------------------------------------------------------------

void FDiabloAssetGenerator::ImportPotionFBX()
{
	const FString DestPath = TEXT("/Game/Items/Potions");

	const FString FBXPath = FPaths::ProjectDir() / TEXT("Tools/blender/out/HealingPotion.fbx");
	if (!FPaths::FileExists(FBXPath))
	{
		UE_LOG(LogTemp, Error, TEXT("[DiabloTools] HealingPotion.fbx not found at %s — run potion_mesh.py first"), *FBXPath);
		return;
	}

	UFbxFactory* FbxFactory = NewObject<UFbxFactory>();
	FbxFactory->ImportUI->bImportMesh = true;
	FbxFactory->ImportUI->bImportAnimations = false;
	FbxFactory->ImportUI->bImportMaterials = true;
	FbxFactory->ImportUI->bImportTextures = false;
	FbxFactory->ImportUI->bIsObjImport = false;
	FbxFactory->ImportUI->bAutomatedImportShouldDetectType = false;
	FbxFactory->ImportUI->MeshTypeToImport = FBXIT_StaticMesh;
	FbxFactory->ImportUI->bImportAsSkeletal = false;

	UAssetImportTask* Task = NewObject<UAssetImportTask>();
	Task->Filename = FBXPath;
	Task->DestinationPath = DestPath;
	Task->DestinationName = TEXT("HealingPotion");
	Task->bReplaceExisting = true;
	Task->bAutomated = true;
	Task->bSave = true;
	Task->Factory = FbxFactory;

	IAssetTools& AssetTools = FAssetToolsModule::GetModule().Get();
	AssetTools.ImportAssetTasks({ Task });

	TArray<UObject*> ImportedObjects = Task->GetObjects();
	UE_LOG(LogTemp, Display, TEXT("[DiabloTools] Imported %d objects from HealingPotion.fbx"), ImportedObjects.Num());
}

// ---------------------------------------------------------------------------
// Configure dropped item (healing potion) defaults
// ---------------------------------------------------------------------------

void FDiabloAssetGenerator::ConfigureDroppedItemDefaults()
{
	UBlueprint* PotionBP = LoadObject<UBlueprint>(nullptr, TEXT("/Game/Blueprints/BP_HealingPotion.BP_HealingPotion"));
	UStaticMesh* PotionMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Items/Potions/HealingPotion.HealingPotion"));

	if (!PotionBP || !PotionBP->GeneratedClass)
	{
		return;
	}

	AActor* CDO = Cast<AActor>(PotionBP->GeneratedClass->GetDefaultObject());
	if (CDO && PotionMesh)
	{
		if (UStaticMeshComponent* MeshComp = CDO->FindComponentByClass<UStaticMeshComponent>())
		{
			MeshComp->SetStaticMesh(PotionMesh);
			UE_LOG(LogTemp, Display, TEXT("[DiabloTools] BP_HealingPotion: set static mesh"));
		}
	}

	FKismetEditorUtilities::CompileBlueprint(PotionBP);
	SaveAsset(PotionBP, PotionBP->GetOutermost(), TEXT("/Game/Blueprints/BP_HealingPotion"));
}

// ---------------------------------------------------------------------------
// Configure Blueprint CDO defaults (individual + combined)
// ---------------------------------------------------------------------------

void FDiabloAssetGenerator::ConfigureBlueprintDefaults()
{
	ConfigureHeroDefaults();
	ConfigureControllerDefaults();
	ConfigureGameModeDefaults();
	ConfigureEnemyDefaults();
	ConfigureDroppedItemDefaults();
}

void FDiabloAssetGenerator::ConfigureHeroDefaults()
{
	UBlueprint* HeroBP = LoadObject<UBlueprint>(nullptr, TEXT("/Game/Blueprints/BP_DiabloHero.BP_DiabloHero"));
	USkeletalMesh* SkMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Characters/Warrior/Warrior.Warrior"));
	UAnimBlueprint* AnimBP = LoadObject<UAnimBlueprint>(nullptr, TEXT("/Game/Characters/Warrior/ABP_Warrior.ABP_Warrior"));
	UAnimMontage* AttackMontage = LoadObject<UAnimMontage>(nullptr, TEXT("/Game/Characters/Warrior/AM_Attack.AM_Attack"));
	UAnimMontage* DeathMontage = LoadObject<UAnimMontage>(nullptr, TEXT("/Game/Characters/Warrior/AM_Death.AM_Death"));

	if (!HeroBP || !HeroBP->GeneratedClass)
	{
		return;
	}

	AActor* CDO = Cast<AActor>(HeroBP->GeneratedClass->GetDefaultObject());
	if (CDO)
	{
		if (USkeletalMeshComponent* MeshComp = CDO->FindComponentByClass<USkeletalMeshComponent>())
		{
			if (SkMesh)
			{
				MeshComp->SetSkeletalMesh(SkMesh);
				UE_LOG(LogTemp, Display, TEXT("[DiabloTools] BP_DiabloHero: set skeletal mesh"));
			}
			if (AnimBP && AnimBP->GeneratedClass)
			{
				MeshComp->SetAnimInstanceClass(AnimBP->GeneratedClass);
				UE_LOG(LogTemp, Display, TEXT("[DiabloTools] BP_DiabloHero: set anim class"));
			}
		}
	}

	UObject* HeroCDO = HeroBP->GeneratedClass->GetDefaultObject();

	if (AttackMontage)
	{
		if (FProperty* Prop = HeroBP->GeneratedClass->FindPropertyByName(TEXT("AttackMontage")))
		{
			if (FObjectProperty* ObjProp = CastField<FObjectProperty>(Prop))
			{
				ObjProp->SetObjectPropertyValue(ObjProp->ContainerPtrToValuePtr<void>(HeroCDO), AttackMontage);
				UE_LOG(LogTemp, Display, TEXT("[DiabloTools] BP_DiabloHero: set AttackMontage"));
			}
		}
	}

	if (DeathMontage)
	{
		if (FProperty* Prop = HeroBP->GeneratedClass->FindPropertyByName(TEXT("DeathMontage")))
		{
			if (FObjectProperty* ObjProp = CastField<FObjectProperty>(Prop))
			{
				ObjProp->SetObjectPropertyValue(ObjProp->ContainerPtrToValuePtr<void>(HeroCDO), DeathMontage);
				UE_LOG(LogTemp, Display, TEXT("[DiabloTools] BP_DiabloHero: set DeathMontage"));
			}
		}
	}

	FKismetEditorUtilities::CompileBlueprint(HeroBP);
	SaveAsset(HeroBP, HeroBP->GetOutermost(), TEXT("/Game/Blueprints/BP_DiabloHero"));
}

void FDiabloAssetGenerator::ConfigureControllerDefaults()
{
	UBlueprint* ControllerBP = LoadObject<UBlueprint>(nullptr, TEXT("/Game/Blueprints/BP_DiabloPlayerController.BP_DiabloPlayerController"));
	UInputAction* ClickAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/Actions/IA_Click.IA_Click"));
	UInputMappingContext* IMC = LoadObject<UInputMappingContext>(nullptr, TEXT("/Game/Input/IMC_Diablo.IMC_Diablo"));

	if (!ControllerBP || !ControllerBP->GeneratedClass)
	{
		return;
	}

	UObject* CDO = ControllerBP->GeneratedClass->GetDefaultObject();
	if (CDO)
	{
		if (FProperty* ClickProp = ControllerBP->GeneratedClass->FindPropertyByName(TEXT("ClickAction")))
		{
			if (FObjectProperty* ObjProp = CastField<FObjectProperty>(ClickProp))
			{
				if (ClickAction)
				{
					ObjProp->SetObjectPropertyValue(ObjProp->ContainerPtrToValuePtr<void>(CDO), ClickAction);
					UE_LOG(LogTemp, Display, TEXT("[DiabloTools] BP_DiabloPlayerController: set ClickAction"));
				}
			}
		}
		if (FProperty* IMCProp = ControllerBP->GeneratedClass->FindPropertyByName(TEXT("DefaultMappingContext")))
		{
			if (FObjectProperty* ObjProp = CastField<FObjectProperty>(IMCProp))
			{
				if (IMC)
				{
					ObjProp->SetObjectPropertyValue(ObjProp->ContainerPtrToValuePtr<void>(CDO), IMC);
					UE_LOG(LogTemp, Display, TEXT("[DiabloTools] BP_DiabloPlayerController: set DefaultMappingContext"));
				}
			}
		}
	}

	FKismetEditorUtilities::CompileBlueprint(ControllerBP);
	SaveAsset(ControllerBP, ControllerBP->GetOutermost(), TEXT("/Game/Blueprints/BP_DiabloPlayerController"));
}

void FDiabloAssetGenerator::ConfigureGameModeDefaults()
{
	UBlueprint* GameModeBP = LoadObject<UBlueprint>(nullptr, TEXT("/Game/Blueprints/BP_DiabloGameMode.BP_DiabloGameMode"));
	UBlueprint* HeroBP = LoadObject<UBlueprint>(nullptr, TEXT("/Game/Blueprints/BP_DiabloHero.BP_DiabloHero"));
	UBlueprint* ControllerBP = LoadObject<UBlueprint>(nullptr, TEXT("/Game/Blueprints/BP_DiabloPlayerController.BP_DiabloPlayerController"));

	if (!GameModeBP || !GameModeBP->GeneratedClass || !HeroBP || !ControllerBP)
	{
		return;
	}

	AGameModeBase* CDO = Cast<AGameModeBase>(GameModeBP->GeneratedClass->GetDefaultObject());
	if (CDO)
	{
		if (HeroBP->GeneratedClass)
		{
			CDO->DefaultPawnClass = HeroBP->GeneratedClass;
			UE_LOG(LogTemp, Display, TEXT("[DiabloTools] BP_DiabloGameMode: set DefaultPawnClass -> BP_DiabloHero"));
		}
		if (ControllerBP->GeneratedClass)
		{
			CDO->PlayerControllerClass = ControllerBP->GeneratedClass;
			UE_LOG(LogTemp, Display, TEXT("[DiabloTools] BP_DiabloGameMode: set PlayerControllerClass -> BP_DiabloPlayerController"));
		}
	}

	FKismetEditorUtilities::CompileBlueprint(GameModeBP);
	SaveAsset(GameModeBP, GameModeBP->GetOutermost(), TEXT("/Game/Blueprints/BP_DiabloGameMode"));
}

void FDiabloAssetGenerator::ConfigureEnemyDefaults()
{
	UBlueprint* EnemyBP = LoadObject<UBlueprint>(nullptr, TEXT("/Game/Blueprints/BP_DiabloEnemy.BP_DiabloEnemy"));
	USkeletalMesh* SkMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Characters/Warrior/Warrior.Warrior"));
	UAnimBlueprint* AnimBP = LoadObject<UAnimBlueprint>(nullptr, TEXT("/Game/Characters/Warrior/ABP_Warrior.ABP_Warrior"));
	UAnimMontage* AttackMontage = LoadObject<UAnimMontage>(nullptr, TEXT("/Game/Characters/Warrior/AM_Attack.AM_Attack"));
	UAnimMontage* DeathMontage = LoadObject<UAnimMontage>(nullptr, TEXT("/Game/Characters/Warrior/AM_Death.AM_Death"));

	if (!EnemyBP || !EnemyBP->GeneratedClass)
	{
		return;
	}

	AActor* CDO = Cast<AActor>(EnemyBP->GeneratedClass->GetDefaultObject());
	if (CDO)
	{
		if (USkeletalMeshComponent* MeshComp = CDO->FindComponentByClass<USkeletalMeshComponent>())
		{
			if (SkMesh)
			{
				MeshComp->SetSkeletalMesh(SkMesh);
				UE_LOG(LogTemp, Display, TEXT("[DiabloTools] BP_DiabloEnemy: set skeletal mesh"));
			}
			if (AnimBP && AnimBP->GeneratedClass)
			{
				MeshComp->SetAnimInstanceClass(AnimBP->GeneratedClass);
				UE_LOG(LogTemp, Display, TEXT("[DiabloTools] BP_DiabloEnemy: set anim class"));
			}
		}
	}

	UObject* EnemyCDO = EnemyBP->GeneratedClass->GetDefaultObject();

	if (AttackMontage)
	{
		if (FProperty* Prop = EnemyBP->GeneratedClass->FindPropertyByName(TEXT("AttackMontage")))
		{
			if (FObjectProperty* ObjProp = CastField<FObjectProperty>(Prop))
			{
				ObjProp->SetObjectPropertyValue(ObjProp->ContainerPtrToValuePtr<void>(EnemyCDO), AttackMontage);
				UE_LOG(LogTemp, Display, TEXT("[DiabloTools] BP_DiabloEnemy: set AttackMontage"));
			}
		}
	}

	if (DeathMontage)
	{
		if (FProperty* Prop = EnemyBP->GeneratedClass->FindPropertyByName(TEXT("DeathMontage")))
		{
			if (FObjectProperty* ObjProp = CastField<FObjectProperty>(Prop))
			{
				ObjProp->SetObjectPropertyValue(ObjProp->ContainerPtrToValuePtr<void>(EnemyCDO), DeathMontage);
				UE_LOG(LogTemp, Display, TEXT("[DiabloTools] BP_DiabloEnemy: set DeathMontage"));
			}
		}
	}

	FKismetEditorUtilities::CompileBlueprint(EnemyBP);
	SaveAsset(EnemyBP, EnemyBP->GetOutermost(), TEXT("/Game/Blueprints/BP_DiabloEnemy"));
}

// ---------------------------------------------------------------------------
// Utility functions
// ---------------------------------------------------------------------------

bool FDiabloAssetGenerator::CreateBlueprintFromClass(UClass* ParentClass, const FString& AssetPath, const FString& AssetName)
{
	const FString FullPath = AssetPath / AssetName;

	if (FPackageName::DoesPackageExist(FullPath))
	{
		UE_LOG(LogTemp, Display, TEXT("[DiabloTools] Blueprint already exists, skipping: %s"), *FullPath);
		return true;
	}

	UPackage* Package = CreatePackage(*FullPath);
	Package->FullyLoad();

	UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(
		ParentClass,
		Package,
		*AssetName,
		BPTYPE_Normal,
		UBlueprint::StaticClass(),
		UBlueprintGeneratedClass::StaticClass()
	);

	if (!Blueprint)
	{
		UE_LOG(LogTemp, Error, TEXT("[DiabloTools] Failed to create blueprint: %s"), *AssetName);
		return false;
	}

	if (SaveAsset(Blueprint, Package, FullPath))
	{
		NotifyAssetCreated(Blueprint);
		UE_LOG(LogTemp, Display, TEXT("[DiabloTools] Created blueprint: %s"), *FullPath);
		return true;
	}

	return false;
}

bool FDiabloAssetGenerator::SaveAsset(UObject* Asset, UPackage* Package, const FString& PackagePath)
{
	Package->MarkPackageDirty();

	FString FilePath = FPackageName::LongPackageNameToFilename(PackagePath, FPackageName::GetAssetPackageExtension());

	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;

	FSavePackageResultStruct Result = UPackage::Save(Package, Asset, *FilePath, SaveArgs);
	if (!Result.IsSuccessful())
	{
		UE_LOG(LogTemp, Error, TEXT("[DiabloTools] Failed to save: %s"), *PackagePath);
		return false;
	}
	return true;
}

void FDiabloAssetGenerator::NotifyAssetCreated(UObject* Asset)
{
	FAssetRegistryModule::AssetCreated(Asset);
}
