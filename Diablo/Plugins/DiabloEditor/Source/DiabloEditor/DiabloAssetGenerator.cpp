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
#include "Sound/SoundWave.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "WidgetBlueprint.h"
#include "WidgetBlueprintFactory.h"
#include "DiabloHUDWidget.h"
#include "DiabloCharacterPanel.h"
#include "DiabloInventoryPanel.h"
#include "ItemDefinition.h"

void FDiabloAssetGenerator::GenerateAllAssets()
{
	SetupAllBlueprints();
	GenerateDefaultMap();
	GenerateInputAssets();
	ImportWarriorFBX();
	ImportLevelUpSFX();
	ImportPotionSprite();

	UE_LOG(LogTemp, Display, TEXT("[DiabloTools] All assets generated."));
}

void FDiabloAssetGenerator::SetupAllBlueprints()
{
	SetupGameMode();
	SetupHero();
	SetupController();
	SetupEnemy();
	SetupPotion();
	SetupHUD();
	SetupInventory();
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
		{ TEXT("IA_Click"),     EInputActionValueType::Boolean, EKeys::LeftMouseButton },
		{ TEXT("IA_Move"),      EInputActionValueType::Axis2D,  EKeys::Invalid },
		{ TEXT("IA_Look"),      EInputActionValueType::Axis2D,  EKeys::Invalid },
		{ TEXT("IA_CharPanel"), EInputActionValueType::Boolean, EKeys::C },
		{ TEXT("IA_Inventory"), EInputActionValueType::Boolean, EKeys::I },
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

	// Never delete existing assets — bReplaceExisting updates the mesh,
	// skeleton, and animation sequences in place, preserving all references
	// (AnimBP, montages, scene proxies). New animations that don't exist yet
	// are imported from separate per-animation FBX files after the main import.

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

	// Import additional animations from separate per-animation FBX files.
	// The main FBX contains all animations via NLA strips, which works on
	// first import. On reimport the UE importer skips new NLA strips, so
	// each animation also has a standalone FBX as a reliable fallback.
	if (!ExistingSkeleton)
	{
		ExistingSkeleton = LoadObject<USkeleton>(nullptr, TEXT("/Game/Characters/Warrior/Warrior_Skeleton.Warrior_Skeleton"));
	}

	auto ImportSingleAnim = [&](const FString& AnimFBXPath, const FString& AnimName)
	{
		if (!FPaths::FileExists(AnimFBXPath))
		{
			UE_LOG(LogTemp, Warning, TEXT("[DiabloTools] %s not found, skipping"), *AnimFBXPath);
			return;
		}

		// Skip if the main import already created this animation
		const FString AnimObjPath = DestPath / AnimName + TEXT(".") + AnimName;
		if (LoadObject<UAnimSequence>(nullptr, *AnimObjPath))
		{
			UE_LOG(LogTemp, Display, TEXT("[DiabloTools] %s already exists from main import, skipping"), *AnimName);
			return;
		}

		UFbxFactory* AnimFactory = NewObject<UFbxFactory>();
		AnimFactory->ImportUI->bImportMesh = false;
		AnimFactory->ImportUI->bImportAnimations = true;
		AnimFactory->ImportUI->bImportMaterials = false;
		AnimFactory->ImportUI->bImportTextures = false;
		AnimFactory->ImportUI->bIsObjImport = false;
		AnimFactory->ImportUI->bAutomatedImportShouldDetectType = false;
		AnimFactory->ImportUI->MeshTypeToImport = FBXIT_Animation;
		AnimFactory->ImportUI->Skeleton = ExistingSkeleton;

		UAssetImportTask* AnimTask = NewObject<UAssetImportTask>();
		AnimTask->Filename = AnimFBXPath;
		AnimTask->DestinationPath = DestPath;
		AnimTask->DestinationName = AnimName;
		AnimTask->bReplaceExisting = true;
		AnimTask->bAutomated = true;
		AnimTask->bSave = true;
		AnimTask->Factory = AnimFactory;

		AssetTools.ImportAssetTasks({ AnimTask });
		UE_LOG(LogTemp, Display, TEXT("[DiabloTools] Imported %s (%d objects)"),
			*AnimName, AnimTask->GetObjects().Num());
	};

	const FString AnimDir = FPaths::ProjectDir() / TEXT("Tools/blender/out");
	ImportSingleAnim(AnimDir / TEXT("Warrior_Idle.fbx"), TEXT("Warrior_Anim_Idle"));
	ImportSingleAnim(AnimDir / TEXT("Warrior_Walk.fbx"), TEXT("Warrior_Anim_Walk"));
	ImportSingleAnim(AnimDir / TEXT("Warrior_Attack.fbx"), TEXT("Warrior_Anim_Attack"));
	ImportSingleAnim(AnimDir / TEXT("Warrior_Death.fbx"), TEXT("Warrior_Anim_Death"));

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
// Import level-up sound effect
// ---------------------------------------------------------------------------

void FDiabloAssetGenerator::ImportLevelUpSFX()
{
	const FString DestPath = TEXT("/Game/Audio/SFX");
	const FString WavPath = FPaths::ProjectDir() / TEXT("Tools/audio/out/LevelUp.wav");

	if (!FPaths::FileExists(WavPath))
	{
		UE_LOG(LogTemp, Error, TEXT("[DiabloTools] LevelUp.wav not found — run levelup_sfx.py first"));
		return;
	}

	UAssetImportTask* Task = NewObject<UAssetImportTask>();
	Task->Filename = WavPath;
	Task->DestinationPath = DestPath;
	Task->DestinationName = TEXT("LevelUp");
	Task->bReplaceExisting = true;
	Task->bAutomated = true;
	Task->bSave = true;

	IAssetTools& AssetTools = FAssetToolsModule::GetModule().Get();
	AssetTools.ImportAssetTasks({ Task });

	UE_LOG(LogTemp, Display, TEXT("[DiabloTools] Imported LevelUp SFX"));
}

// ---------------------------------------------------------------------------
// Import healing potion sprite texture
// ---------------------------------------------------------------------------

void FDiabloAssetGenerator::ImportPotionSprite()
{
	const FString DestPath = TEXT("/Game/Items/Potions");

	const FString PNGPath = FPaths::ProjectDir() / TEXT("Tools/svg/out/HealingPotion.png");
	if (!FPaths::FileExists(PNGPath))
	{
		UE_LOG(LogTemp, Error, TEXT("[DiabloTools] HealingPotion.png not found at %s — run convert.sh first"), *PNGPath);
		return;
	}

	UAssetImportTask* Task = NewObject<UAssetImportTask>();
	Task->Filename = PNGPath;
	Task->DestinationPath = DestPath;
	Task->DestinationName = TEXT("T_HealingPotion");
	Task->bReplaceExisting = true;
	Task->bAutomated = true;
	Task->bSave = true;

	IAssetTools& AssetTools = FAssetToolsModule::GetModule().Get();
	AssetTools.ImportAssetTasks({ Task });

	UE_LOG(LogTemp, Display, TEXT("[DiabloTools] Imported HealingPotion sprite texture"));
}

// ---------------------------------------------------------------------------
// Import item icon textures
// ---------------------------------------------------------------------------

void FDiabloAssetGenerator::ImportItemIcons()
{
	const FString DestPath = TEXT("/Game/Items/Icons");
	const FString SrcDir = FPaths::ProjectDir() / TEXT("Tools/svg/out");

	struct FIconEntry { FString PngName; FString AssetName; };
	TArray<FIconEntry> Icons = {
		{ TEXT("T_ShortSword.png"), TEXT("T_ShortSword") },
		{ TEXT("T_Buckler.png"), TEXT("T_Buckler") },
		{ TEXT("T_SkullCap.png"), TEXT("T_SkullCap") },
		{ TEXT("T_Rags.png"), TEXT("T_Rags") },
		{ TEXT("T_Ring.png"), TEXT("T_Ring") },
	};

	IAssetTools& AssetTools = FAssetToolsModule::GetModule().Get();

	for (const FIconEntry& Entry : Icons)
	{
		const FString PngPath = SrcDir / Entry.PngName;
		if (!FPaths::FileExists(PngPath))
		{
			UE_LOG(LogTemp, Warning, TEXT("[DiabloTools] %s not found — run convert_items.sh first"), *Entry.PngName);
			continue;
		}

		UAssetImportTask* Task = NewObject<UAssetImportTask>();
		Task->Filename = PngPath;
		Task->DestinationPath = DestPath;
		Task->DestinationName = Entry.AssetName;
		Task->bReplaceExisting = true;
		Task->bAutomated = true;
		Task->bSave = true;

		AssetTools.ImportAssetTasks({ Task });
		UE_LOG(LogTemp, Display, TEXT("[DiabloTools] Imported item icon: %s"), *Entry.AssetName);
	}
}

// ---------------------------------------------------------------------------
// Configure dropped item (healing potion) defaults
// ---------------------------------------------------------------------------

void FDiabloAssetGenerator::SetupPotion()
{
	CreateBlueprintFromClass(ADroppedItem::StaticClass(), TEXT("/Game/Blueprints"), TEXT("BP_HealingPotion"));

	UBlueprint* PotionBP = LoadObject<UBlueprint>(nullptr, TEXT("/Game/Blueprints/BP_HealingPotion.BP_HealingPotion"));
	UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
	UTexture2D* PotionTex = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Items/Potions/T_HealingPotion.T_HealingPotion"));

	if (!PotionBP || !PotionBP->GeneratedClass)
	{
		return;
	}

	// Create an unlit translucent material from the sprite texture
	UMaterial* PotionMat = nullptr;
	const FString MatPath = TEXT("/Game/Items/Potions/M_HealingPotion");
	const FString MatObjPath = MatPath + TEXT(".M_HealingPotion");
	PotionMat = LoadObject<UMaterial>(nullptr, *MatObjPath);

	if (!PotionMat && PotionTex)
	{
		UPackage* MatPkg = CreatePackage(*MatPath);
		MatPkg->FullyLoad();
		PotionMat = NewObject<UMaterial>(MatPkg, TEXT("M_HealingPotion"), RF_Public | RF_Standalone);
		PotionMat->MaterialDomain = EMaterialDomain::MD_Surface;
		PotionMat->BlendMode = EBlendMode::BLEND_Translucent;
		PotionMat->SetShadingModel(EMaterialShadingModel::MSM_Unlit);
		PotionMat->TwoSided = true;

		// Create texture sample node
		UMaterialExpressionTextureSample* TexSample = NewObject<UMaterialExpressionTextureSample>(PotionMat);
		TexSample->Texture = PotionTex;
		PotionMat->GetEditorOnlyData()->ExpressionCollection.Expressions.Add(TexSample);

		// Wire RGB to emissive, alpha to opacity
		PotionMat->GetEditorOnlyData()->EmissiveColor.Connect(0, TexSample);
		PotionMat->GetEditorOnlyData()->Opacity.Connect(4, TexSample);

		PotionMat->PreEditChange(nullptr);
		PotionMat->PostEditChange();

		SaveAsset(PotionMat, MatPkg, MatPath);
		NotifyAssetCreated(PotionMat);
		UE_LOG(LogTemp, Display, TEXT("[DiabloTools] Created M_HealingPotion material"));
	}

	AActor* CDO = Cast<AActor>(PotionBP->GeneratedClass->GetDefaultObject());
	if (CDO && PlaneMesh)
	{
		if (UStaticMeshComponent* MeshComp = CDO->FindComponentByClass<UStaticMeshComponent>())
		{
			MeshComp->SetStaticMesh(PlaneMesh);
			MeshComp->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.3f));
			// Stand upright facing the isometric camera (pitch -45, yaw 225)
			MeshComp->SetRelativeRotation(FRotator(90.f, 45.f, 0.f));
			if (PotionMat)
			{
				MeshComp->SetMaterial(0, PotionMat);
			}
			UE_LOG(LogTemp, Display, TEXT("[DiabloTools] BP_HealingPotion: set plane mesh + sprite material"));
		}
	}

	FKismetEditorUtilities::CompileBlueprint(PotionBP);
	SaveAsset(PotionBP, PotionBP->GetOutermost(), TEXT("/Game/Blueprints/BP_HealingPotion"));
}

// ---------------------------------------------------------------------------
// Configure Blueprint CDO defaults (individual + combined)
// ---------------------------------------------------------------------------

void FDiabloAssetGenerator::SetupHero()
{
	CreateBlueprintFromClass(ADiabloHero::StaticClass(), TEXT("/Game/Blueprints"), TEXT("BP_DiabloHero"));

	UBlueprint* HeroBP = LoadObject<UBlueprint>(nullptr, TEXT("/Game/Blueprints/BP_DiabloHero.BP_DiabloHero"));
	USkeletalMesh* SkMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Characters/Warrior/Warrior.Warrior"));
	UAnimBlueprint* AnimBP = LoadObject<UAnimBlueprint>(nullptr, TEXT("/Game/Characters/Warrior/ABP_Warrior.ABP_Warrior"));
	UAnimMontage* AttackMontage = LoadObject<UAnimMontage>(nullptr, TEXT("/Game/Characters/Warrior/AM_Attack.AM_Attack"));
	UAnimMontage* DeathMontage = LoadObject<UAnimMontage>(nullptr, TEXT("/Game/Characters/Warrior/AM_Death.AM_Death"));
	USoundWave* LevelUpSound = LoadObject<USoundWave>(nullptr, TEXT("/Game/Audio/SFX/LevelUp.LevelUp"));

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

	if (LevelUpSound)
	{
		if (FProperty* Prop = HeroBP->GeneratedClass->FindPropertyByName(TEXT("LevelUpSound")))
		{
			if (FObjectProperty* ObjProp = CastField<FObjectProperty>(Prop))
			{
				ObjProp->SetObjectPropertyValue(ObjProp->ContainerPtrToValuePtr<void>(HeroCDO), LevelUpSound);
				UE_LOG(LogTemp, Display, TEXT("[DiabloTools] BP_DiabloHero: set LevelUpSound"));
			}
		}
	}

	FKismetEditorUtilities::CompileBlueprint(HeroBP);
	SaveAsset(HeroBP, HeroBP->GetOutermost(), TEXT("/Game/Blueprints/BP_DiabloHero"));
}

void FDiabloAssetGenerator::SetupController()
{
	CreateBlueprintFromClass(ADiabloPlayerController::StaticClass(), TEXT("/Game/Blueprints"), TEXT("BP_DiabloPlayerController"));

	UBlueprint* ControllerBP = LoadObject<UBlueprint>(nullptr, TEXT("/Game/Blueprints/BP_DiabloPlayerController.BP_DiabloPlayerController"));
	UInputAction* ClickAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/Actions/IA_Click.IA_Click"));
	UInputAction* CharPanelAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/Actions/IA_CharPanel.IA_CharPanel"));
	UInputAction* InventoryAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/Actions/IA_Inventory.IA_Inventory"));
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
		if (FProperty* CPProp = ControllerBP->GeneratedClass->FindPropertyByName(TEXT("CharPanelAction")))
		{
			if (FObjectProperty* ObjProp = CastField<FObjectProperty>(CPProp))
			{
				if (CharPanelAction)
				{
					ObjProp->SetObjectPropertyValue(ObjProp->ContainerPtrToValuePtr<void>(CDO), CharPanelAction);
					UE_LOG(LogTemp, Display, TEXT("[DiabloTools] BP_DiabloPlayerController: set CharPanelAction"));
				}
			}
		}
		if (FProperty* InvProp = ControllerBP->GeneratedClass->FindPropertyByName(TEXT("InventoryAction")))
		{
			if (FObjectProperty* ObjProp = CastField<FObjectProperty>(InvProp))
			{
				if (InventoryAction)
				{
					ObjProp->SetObjectPropertyValue(ObjProp->ContainerPtrToValuePtr<void>(CDO), InventoryAction);
					UE_LOG(LogTemp, Display, TEXT("[DiabloTools] BP_DiabloPlayerController: set InventoryAction"));
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

void FDiabloAssetGenerator::SetupGameMode()
{
	CreateBlueprintFromClass(ADiabloGameMode::StaticClass(), TEXT("/Game/Blueprints"), TEXT("BP_DiabloGameMode"));

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

void FDiabloAssetGenerator::SetupEnemy()
{
	CreateBlueprintFromClass(ADiabloEnemy::StaticClass(), TEXT("/Game/Blueprints"), TEXT("BP_DiabloEnemy"));

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
// HUD Widget Blueprint
// ---------------------------------------------------------------------------

void FDiabloAssetGenerator::SetupHUD()
{
	const FString BPPath = TEXT("/Game/Blueprints/BP_DiabloHUD");
	const FString BPObjPath = BPPath + TEXT(".BP_DiabloHUD");

	UWidgetBlueprint* HUDBP = LoadObject<UWidgetBlueprint>(nullptr, *BPObjPath);
	if (!HUDBP)
	{
		UPackage* Package = CreatePackage(*BPPath);
		Package->FullyLoad();

		UWidgetBlueprintFactory* Factory = NewObject<UWidgetBlueprintFactory>();
		Factory->ParentClass = UDiabloHUDWidget::StaticClass();

		HUDBP = Cast<UWidgetBlueprint>(Factory->FactoryCreateNew(
			UWidgetBlueprint::StaticClass(),
			Package,
			TEXT("BP_DiabloHUD"),
			RF_Public | RF_Standalone,
			nullptr,
			GWarn
		));

		if (!HUDBP)
		{
			UE_LOG(LogTemp, Error, TEXT("[DiabloTools] Failed to create BP_DiabloHUD"));
			return;
		}

		FKismetEditorUtilities::CompileBlueprint(HUDBP);
		SaveAsset(HUDBP, Package, BPPath);
		NotifyAssetCreated(HUDBP);
		UE_LOG(LogTemp, Display, TEXT("[DiabloTools] Created BP_DiabloHUD"));
	}

	// --- Create BP_DiabloCharPanel ---
	const FString CPPath = TEXT("/Game/Blueprints/BP_DiabloCharPanel");
	const FString CPObjPath = CPPath + TEXT(".BP_DiabloCharPanel");

	UWidgetBlueprint* CharPanelBP = LoadObject<UWidgetBlueprint>(nullptr, *CPObjPath);
	if (!CharPanelBP)
	{
		UPackage* CPPackage = CreatePackage(*CPPath);
		CPPackage->FullyLoad();

		UWidgetBlueprintFactory* CPFactory = NewObject<UWidgetBlueprintFactory>();
		CPFactory->ParentClass = UDiabloCharacterPanel::StaticClass();

		CharPanelBP = Cast<UWidgetBlueprint>(CPFactory->FactoryCreateNew(
			UWidgetBlueprint::StaticClass(),
			CPPackage,
			TEXT("BP_DiabloCharPanel"),
			RF_Public | RF_Standalone,
			nullptr,
			GWarn
		));

		if (CharPanelBP)
		{
			FKismetEditorUtilities::CompileBlueprint(CharPanelBP);
			SaveAsset(CharPanelBP, CPPackage, CPPath);
			NotifyAssetCreated(CharPanelBP);
			UE_LOG(LogTemp, Display, TEXT("[DiabloTools] Created BP_DiabloCharPanel"));
		}
	}

	// --- Create BP_DiabloInventoryPanel ---
	const FString InvPath = TEXT("/Game/Blueprints/BP_DiabloInventoryPanel");
	const FString InvObjPath = InvPath + TEXT(".BP_DiabloInventoryPanel");

	UWidgetBlueprint* InvPanelBP = LoadObject<UWidgetBlueprint>(nullptr, *InvObjPath);
	if (!InvPanelBP)
	{
		UPackage* InvPackage = CreatePackage(*InvPath);
		InvPackage->FullyLoad();

		UWidgetBlueprintFactory* InvFactory = NewObject<UWidgetBlueprintFactory>();
		InvFactory->ParentClass = UDiabloInventoryPanel::StaticClass();

		InvPanelBP = Cast<UWidgetBlueprint>(InvFactory->FactoryCreateNew(
			UWidgetBlueprint::StaticClass(),
			InvPackage,
			TEXT("BP_DiabloInventoryPanel"),
			RF_Public | RF_Standalone,
			nullptr,
			GWarn
		));

		if (InvPanelBP)
		{
			FKismetEditorUtilities::CompileBlueprint(InvPanelBP);
			SaveAsset(InvPanelBP, InvPackage, InvPath);
			NotifyAssetCreated(InvPanelBP);
			UE_LOG(LogTemp, Display, TEXT("[DiabloTools] Created BP_DiabloInventoryPanel"));
		}
	}

	// --- Set HUDWidgetClass, CharPanelClass, InventoryPanelClass on BP_DiabloPlayerController ---
	UBlueprint* ControllerBP = LoadObject<UBlueprint>(nullptr,
		TEXT("/Game/Blueprints/BP_DiabloPlayerController.BP_DiabloPlayerController"));

	if (ControllerBP && ControllerBP->GeneratedClass)
	{
		FKismetEditorUtilities::CompileBlueprint(ControllerBP);

		UObject* CDO = ControllerBP->GeneratedClass->GetDefaultObject();

		if (HUDBP && HUDBP->GeneratedClass)
		{
			if (FProperty* Prop = ControllerBP->GeneratedClass->FindPropertyByName(TEXT("HUDWidgetClass")))
			{
				if (FObjectProperty* ObjProp = CastField<FObjectProperty>(Prop))
				{
					ObjProp->SetObjectPropertyValue(ObjProp->ContainerPtrToValuePtr<void>(CDO), HUDBP->GeneratedClass);
					UE_LOG(LogTemp, Display, TEXT("[DiabloTools] BP_DiabloPlayerController: set HUDWidgetClass -> BP_DiabloHUD"));
				}
			}
		}

		if (CharPanelBP && CharPanelBP->GeneratedClass)
		{
			if (FProperty* Prop = ControllerBP->GeneratedClass->FindPropertyByName(TEXT("CharPanelClass")))
			{
				if (FObjectProperty* ObjProp = CastField<FObjectProperty>(Prop))
				{
					ObjProp->SetObjectPropertyValue(ObjProp->ContainerPtrToValuePtr<void>(CDO), CharPanelBP->GeneratedClass);
					UE_LOG(LogTemp, Display, TEXT("[DiabloTools] BP_DiabloPlayerController: set CharPanelClass -> BP_DiabloCharPanel"));
				}
			}
		}

		if (InvPanelBP && InvPanelBP->GeneratedClass)
		{
			if (FProperty* Prop = ControllerBP->GeneratedClass->FindPropertyByName(TEXT("InventoryPanelClass")))
			{
				if (FObjectProperty* ObjProp = CastField<FObjectProperty>(Prop))
				{
					ObjProp->SetObjectPropertyValue(ObjProp->ContainerPtrToValuePtr<void>(CDO), InvPanelBP->GeneratedClass);
					UE_LOG(LogTemp, Display, TEXT("[DiabloTools] BP_DiabloPlayerController: set InventoryPanelClass -> BP_DiabloInventoryPanel"));
				}
			}
		}

		SaveAsset(ControllerBP, ControllerBP->GetOutermost(),
			TEXT("/Game/Blueprints/BP_DiabloPlayerController"));
	}
}

// ---------------------------------------------------------------------------
// Item Definitions
// ---------------------------------------------------------------------------

void FDiabloAssetGenerator::SetupInventory()
{
	struct FItemDef
	{
		FString Name;
		EItemCategory Category;
		EEquipSlot Slot;
		int32 W, H;
		float MinDmg, MaxDmg, AC;
		float Str, Mag, Dex, Vit;
		int32 Durability, GoldValue;
		float HealAmount;
		bool bStackable;
		int32 MaxStack;
		FString IconAsset;
	};

	TArray<FItemDef> Items = {
		{ TEXT("Short Sword"), EItemCategory::Weapon, EEquipSlot::RightHand, 1, 3,
		  2.f, 6.f, 0.f, 0.f, 0.f, 0.f, 0.f, 24, 50, 0.f, false, 1,
		  TEXT("/Game/Items/Icons/T_ShortSword.T_ShortSword") },
		{ TEXT("Buckler"), EItemCategory::Shield, EEquipSlot::LeftHand, 1, 2,
		  0.f, 0.f, 5.f, 0.f, 0.f, 0.f, 0.f, 16, 30, 0.f, false, 1,
		  TEXT("/Game/Items/Icons/T_Buckler.T_Buckler") },
		{ TEXT("Skull Cap"), EItemCategory::Helm, EEquipSlot::Head, 2, 2,
		  0.f, 0.f, 3.f, 0.f, 0.f, 0.f, 0.f, 15, 25, 0.f, false, 1,
		  TEXT("/Game/Items/Icons/T_SkullCap.T_SkullCap") },
		{ TEXT("Rags"), EItemCategory::Armor, EEquipSlot::Chest, 2, 3,
		  0.f, 0.f, 2.f, 0.f, 0.f, 0.f, 0.f, 12, 15, 0.f, false, 1,
		  TEXT("/Game/Items/Icons/T_Rags.T_Rags") },
		{ TEXT("Ring of Strength"), EItemCategory::Ring, EEquipSlot::LeftRing, 1, 1,
		  0.f, 0.f, 0.f, 5.f, 0.f, 0.f, 0.f, 0, 100, 0.f, false, 1,
		  TEXT("/Game/Items/Icons/T_Ring.T_Ring") },
		{ TEXT("Healing Potion"), EItemCategory::Potion, EEquipSlot::None, 1, 1,
		  0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0, 50, 50.f, true, 20,
		  TEXT("/Game/Items/Potions/T_HealingPotion.T_HealingPotion") },
	};

	const FString BasePath = TEXT("/Game/Items/Definitions");

	for (const FItemDef& Def : Items)
	{
		FString SafeName = Def.Name;
		SafeName.ReplaceInline(TEXT(" "), TEXT("_"));
		const FString FullPath = BasePath / (TEXT("ID_") + SafeName);
		const FString ObjPath = FullPath + TEXT(".ID_") + SafeName;

		UItemDefinition* Existing = LoadObject<UItemDefinition>(nullptr, *ObjPath);
		if (Existing)
		{
			UE_LOG(LogTemp, Display, TEXT("[DiabloTools] Item definition already exists: %s"), *Def.Name);
			continue;
		}

		UPackage* Package = CreatePackage(*FullPath);
		Package->FullyLoad();

		UItemDefinition* ItemDef = NewObject<UItemDefinition>(
			Package, *(TEXT("ID_") + SafeName), RF_Public | RF_Standalone);

		ItemDef->DisplayName = FText::FromString(Def.Name);
		ItemDef->Category = Def.Category;
		ItemDef->EquipSlot = Def.Slot;
		ItemDef->GridWidth = Def.W;
		ItemDef->GridHeight = Def.H;
		ItemDef->MinDamage = Def.MinDmg;
		ItemDef->MaxDamage = Def.MaxDmg;
		ItemDef->ArmorClass = Def.AC;
		ItemDef->BonusStr = Def.Str;
		ItemDef->BonusMag = Def.Mag;
		ItemDef->BonusDex = Def.Dex;
		ItemDef->BonusVit = Def.Vit;
		ItemDef->MaxDurability = Def.Durability;
		ItemDef->GoldValue = Def.GoldValue;
		ItemDef->HealAmount = Def.HealAmount;
		ItemDef->bStackable = Def.bStackable;
		ItemDef->MaxStack = Def.MaxStack;

		if (!Def.IconAsset.IsEmpty())
		{
			UTexture2D* IconTex = LoadObject<UTexture2D>(nullptr, *Def.IconAsset);
			if (IconTex)
			{
				ItemDef->Icon = IconTex;
			}
		}

		if (SaveAsset(ItemDef, Package, FullPath))
		{
			NotifyAssetCreated(ItemDef);
			UE_LOG(LogTemp, Display, TEXT("[DiabloTools] Created item definition: %s"), *Def.Name);
		}
	}
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
