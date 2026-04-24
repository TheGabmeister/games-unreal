#include "DiabloAssetGenerator.h"
#include "DiabloGameMode.h"
#include "DiabloHero.h"
#include "DiabloPlayerController.h"
#include "DiabloAnimInstance.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "FileHelpers.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
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
#include "Animation/AnimBlueprintGeneratedClass.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "AnimGraphNode_StateMachine.h"
#include "AnimGraphNode_SequencePlayer.h"
#include "AnimGraphNode_StateResult.h"
#include "AnimGraphNode_TransitionResult.h"
#include "AnimStateNode.h"
#include "AnimStateEntryNode.h"
#include "AnimStateTransitionNode.h"
#include "AnimationGraph.h"
#include "AnimationGraphSchema.h"
#include "AnimationStateMachineGraph.h"
#include "AnimationStateMachineSchema.h"
#include "AnimationStateGraph.h"
#include "AnimationTransitionGraph.h"
#include "AssetImportTask.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "Factories/FbxFactory.h"
#include "Factories/FbxImportUI.h"
#include "K2Node_CallFunction.h"
#include "K2Node_VariableGet.h"
#include "Kismet/KismetMathLibrary.h"
#include "EdGraphSchema_K2.h"
#include "ObjectTools.h"

void FDiabloAssetGenerator::GenerateAllAssets()
{
	GenerateBlueprintSubclasses();
	GenerateDefaultMap();
	GenerateInputAssets();
	ImportWarriorFBX();
	GenerateAnimBlueprint();
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

	UFbxFactory* FbxFactory = NewObject<UFbxFactory>();
	FbxFactory->ImportUI->bImportMesh = true;
	FbxFactory->ImportUI->bImportAnimations = true;
	FbxFactory->ImportUI->bImportMaterials = false;
	FbxFactory->ImportUI->bImportTextures = false;
	FbxFactory->ImportUI->bIsObjImport = false;
	FbxFactory->ImportUI->bAutomatedImportShouldDetectType = false;
	FbxFactory->ImportUI->MeshTypeToImport = FBXIT_SkeletalMesh;
	FbxFactory->ImportUI->bImportAsSkeletal = true;

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
}

// ---------------------------------------------------------------------------
// AnimBlueprint generation with full state machine
// ---------------------------------------------------------------------------

UAnimStateNode* FDiabloAssetGenerator::SpawnStateNode(
	UAnimationStateMachineGraph* Graph, const FString& Name,
	UAnimSequence* Anim, const FVector2f& Pos)
{
	FEdGraphSchemaAction_NewStateNode Action;
	Action.NodeTemplate = NewObject<UAnimStateNode>(Graph);
	UAnimStateNode* State = Cast<UAnimStateNode>(
		Action.PerformAction(Graph, nullptr, Pos, false));

	if (!State) return nullptr;

	State->Rename(*Name);

	if (Anim && State->BoundGraph)
	{
		FGraphNodeCreator<UAnimGraphNode_SequencePlayer> SeqCreator(*State->BoundGraph);
		UAnimGraphNode_SequencePlayer* SeqNode = SeqCreator.CreateNode();
		SeqNode->SetAnimationAsset(Anim);
		SeqNode->NodePosX = -300;
		SeqNode->NodePosY = 0;
		SeqCreator.Finalize();

		UEdGraphPin* PosePin = State->GetPoseSinkPinInsideState();
		if (PosePin)
		{
			for (UEdGraphPin* Pin : SeqNode->Pins)
			{
				if (Pin->Direction == EGPD_Output && Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Struct)
				{
					Pin->MakeLinkTo(PosePin);
					break;
				}
			}
		}
	}

	return State;
}

void FDiabloAssetGenerator::CreateTransitionRule(
	UAnimationStateMachineGraph* Graph,
	UAnimStateNode* From, UAnimStateNode* To,
	bool bGreaterThan, float Threshold, const FVector2f& Pos)
{
	FEdGraphSchemaAction_NewStateNode Action;
	Action.NodeTemplate = NewObject<UAnimStateTransitionNode>(Graph);
	UAnimStateTransitionNode* Trans = Cast<UAnimStateTransitionNode>(
		Action.PerformAction(Graph, nullptr, Pos, false));

	if (!Trans) return;

	Trans->CreateConnections(From, To);

	UAnimationTransitionGraph* TransGraph = Cast<UAnimationTransitionGraph>(Trans->BoundGraph);
	if (!TransGraph) return;

	UAnimGraphNode_TransitionResult* ResultNode = TransGraph->MyResultNode;
	if (!ResultNode) return;

	UEdGraphPin* CanEnterPin = ResultNode->FindPin(TEXT("bCanEnterTransition"));
	if (!CanEnterPin) return;

	// Create Speed variable getter
	FGraphNodeCreator<UK2Node_VariableGet> VarCreator(*TransGraph);
	UK2Node_VariableGet* SpeedGet = VarCreator.CreateNode();
	SpeedGet->VariableReference.SetSelfMember(FName(TEXT("Speed")));
	SpeedGet->NodePosX = -400;
	SpeedGet->NodePosY = 0;
	VarCreator.Finalize();
	SpeedGet->AllocateDefaultPins();

	// Create Greater_DoubleDouble (or Less) comparison
	FGraphNodeCreator<UK2Node_CallFunction> CompCreator(*TransGraph);
	UK2Node_CallFunction* Compare = CompCreator.CreateNode();
	const FName FuncName = bGreaterThan
		? GET_FUNCTION_NAME_CHECKED(UKismetMathLibrary, Greater_DoubleDouble)
		: GET_FUNCTION_NAME_CHECKED(UKismetMathLibrary, LessEqual_DoubleDouble);
	Compare->FunctionReference.SetExternalMember(FuncName, UKismetMathLibrary::StaticClass());
	Compare->NodePosX = -200;
	Compare->NodePosY = 0;
	CompCreator.Finalize();
	Compare->AllocateDefaultPins();

	// Wire Speed output -> comparison A pin
	UEdGraphPin* SpeedOutPin = SpeedGet->FindPin(TEXT("Speed"), EGPD_Output);
	UEdGraphPin* APin = Compare->FindPin(TEXT("A"));
	if (SpeedOutPin && APin)
	{
		SpeedOutPin->MakeLinkTo(APin);
	}

	// Set threshold on B pin
	UEdGraphPin* BPin = Compare->FindPin(TEXT("B"));
	if (BPin)
	{
		BPin->DefaultValue = FString::SanitizeFloat(Threshold);
	}

	// Wire comparison result -> bCanEnterTransition
	UEdGraphPin* ReturnPin = Compare->FindPin(UEdGraphSchema_K2::PN_ReturnValue);
	if (ReturnPin)
	{
		ReturnPin->MakeLinkTo(CanEnterPin);
	}
}

void FDiabloAssetGenerator::GenerateAnimBlueprint()
{
	const FString ABPPath = TEXT("/Game/Characters/Warrior/ABP_Warrior");

	DeleteExistingAsset(ABPPath);

	// Find the imported skeleton
	USkeleton* Skeleton = LoadObject<USkeleton>(nullptr, TEXT("/Game/Characters/Warrior/Warrior_Skeleton.Warrior_Skeleton"));
	if (!Skeleton)
	{
		Skeleton = LoadObject<USkeleton>(nullptr, TEXT("/Game/Characters/Warrior/Warrior_Armature_Skeleton.Warrior_Armature_Skeleton"));
	}
	if (!Skeleton)
	{
		UE_LOG(LogTemp, Error, TEXT("[DiabloTools] Cannot find Warrior skeleton — import FBX first"));
		return;
	}

	USkeletalMesh* SkMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Characters/Warrior/Warrior.Warrior"));
	if (!SkMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("[DiabloTools] Cannot find Warrior skeletal mesh"));
		return;
	}

	// Find animation sequences — FBX import names vary
	UAnimSequence* IdleAnim = LoadObject<UAnimSequence>(nullptr, TEXT("/Game/Characters/Warrior/Idle.Idle"));
	if (!IdleAnim)
		IdleAnim = LoadObject<UAnimSequence>(nullptr, TEXT("/Game/Characters/Warrior/Warrior_Anim_Idle.Warrior_Anim_Idle"));
	if (!IdleAnim)
		IdleAnim = LoadObject<UAnimSequence>(nullptr, TEXT("/Game/Characters/Warrior/Warrior_Idle.Warrior_Idle"));

	UAnimSequence* WalkAnim = LoadObject<UAnimSequence>(nullptr, TEXT("/Game/Characters/Warrior/Walk.Walk"));
	if (!WalkAnim)
		WalkAnim = LoadObject<UAnimSequence>(nullptr, TEXT("/Game/Characters/Warrior/Warrior_Anim_Walk.Warrior_Anim_Walk"));
	if (!WalkAnim)
		WalkAnim = LoadObject<UAnimSequence>(nullptr, TEXT("/Game/Characters/Warrior/Warrior_Walk.Warrior_Walk"));

	if (!IdleAnim || !WalkAnim)
	{
		UE_LOG(LogTemp, Error, TEXT("[DiabloTools] Cannot find Idle/Walk animation sequences (Idle=%s Walk=%s)"),
			IdleAnim ? TEXT("found") : TEXT("MISSING"),
			WalkAnim ? TEXT("found") : TEXT("MISSING"));
		return;
	}

	// Create the AnimBlueprint
	UPackage* Package = CreatePackage(*ABPPath);
	Package->FullyLoad();

	UAnimBlueprint* AnimBP = CastChecked<UAnimBlueprint>(
		FKismetEditorUtilities::CreateBlueprint(
			UDiabloAnimInstance::StaticClass(),
			Package,
			FName(TEXT("ABP_Warrior")),
			BPTYPE_Normal,
			UAnimBlueprint::StaticClass(),
			UBlueprintGeneratedClass::StaticClass()
		));

	AnimBP->TargetSkeleton = Skeleton;
	if (UAnimBlueprintGeneratedClass* GenClass = Cast<UAnimBlueprintGeneratedClass>(AnimBP->GeneratedClass))
	{
		GenClass->TargetSkeleton = Skeleton;
	}
	AnimBP->SetPreviewMesh(SkMesh);

	// Find the main AnimGraph
	UAnimationGraph* AnimGraph = nullptr;
	for (UEdGraph* Graph : AnimBP->FunctionGraphs)
	{
		if (UAnimationGraph* AG = Cast<UAnimationGraph>(Graph))
		{
			AnimGraph = AG;
			break;
		}
	}

	if (!AnimGraph)
	{
		UE_LOG(LogTemp, Error, TEXT("[DiabloTools] No AnimGraph found in new AnimBlueprint"));
		return;
	}

	// Create state machine node in the AnimGraph
	FEdGraphSchemaAction_NewStateNode SMAction;
	SMAction.NodeTemplate = NewObject<UAnimGraphNode_StateMachine>(AnimGraph);
	UAnimGraphNode_StateMachine* SMNode = Cast<UAnimGraphNode_StateMachine>(
		SMAction.PerformAction(AnimGraph, nullptr, FVector2f(200.f, 0.f), false));

	if (!SMNode)
	{
		UE_LOG(LogTemp, Error, TEXT("[DiabloTools] Failed to create state machine node"));
		return;
	}

	UAnimationStateMachineGraph* SMGraph = SMNode->EditorStateMachineGraph;
	if (!SMGraph)
	{
		UE_LOG(LogTemp, Error, TEXT("[DiabloTools] State machine has no graph"));
		return;
	}

	// Connect state machine output to AnimGraph result
	UEdGraphPin* SMOutputPin = SMNode->FindPin(TEXT("Pose"));
	if (!SMOutputPin)
	{
		for (UEdGraphPin* Pin : SMNode->Pins)
		{
			if (Pin->Direction == EGPD_Output)
			{
				SMOutputPin = Pin;
				break;
			}
		}
	}

	for (UEdGraphNode* Node : AnimGraph->Nodes)
	{
		if (Node->IsA<UAnimGraphNode_StateResult>())
		{
			for (UEdGraphPin* Pin : Node->Pins)
			{
				if (Pin->Direction == EGPD_Input && SMOutputPin)
				{
					SMOutputPin->MakeLinkTo(Pin);
					break;
				}
			}
			break;
		}
	}

	// Create states
	UAnimStateNode* IdleState = SpawnStateNode(SMGraph, TEXT("Idle"), IdleAnim, FVector2f(300.f, -50.f));
	UAnimStateNode* WalkState = SpawnStateNode(SMGraph, TEXT("Walk"), WalkAnim, FVector2f(600.f, -50.f));

	if (!IdleState || !WalkState)
	{
		UE_LOG(LogTemp, Error, TEXT("[DiabloTools] Failed to create state nodes"));
		return;
	}

	// Connect entry to Idle
	if (SMGraph->EntryNode)
	{
		UEdGraphPin* EntryOut = SMGraph->EntryNode->GetOutputPin();
		if (EntryOut && IdleState->Pins.Num() > 0)
		{
			for (UEdGraphPin* Pin : IdleState->Pins)
			{
				if (Pin->Direction == EGPD_Input)
				{
					EntryOut->MakeLinkTo(Pin);
					break;
				}
			}
		}
	}

	// Create transitions: Idle -> Walk (Speed > 5) and Walk -> Idle (Speed <= 5)
	CreateTransitionRule(SMGraph, IdleState, WalkState, true, 5.0f, FVector2f(450.f, -100.f));
	CreateTransitionRule(SMGraph, WalkState, IdleState, false, 5.0f, FVector2f(450.f, 0.f));

	// Compile and save
	FKismetEditorUtilities::CompileBlueprint(AnimBP);

	if (SaveAsset(AnimBP, Package, ABPPath))
	{
		NotifyAssetCreated(AnimBP);
		UE_LOG(LogTemp, Display, TEXT("[DiabloTools] Created ABP_Warrior with Idle<->Walk state machine"));
	}
}

// ---------------------------------------------------------------------------
// Configure Blueprint CDO defaults
// ---------------------------------------------------------------------------

void FDiabloAssetGenerator::ConfigureBlueprintDefaults()
{
	// --- BP_DiabloHero: set skeletal mesh + anim blueprint ---
	UBlueprint* HeroBP = LoadObject<UBlueprint>(nullptr, TEXT("/Game/Blueprints/BP_DiabloHero.BP_DiabloHero"));
	USkeletalMesh* SkMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Characters/Warrior/Warrior.Warrior"));
	UAnimBlueprint* AnimBP = LoadObject<UAnimBlueprint>(nullptr, TEXT("/Game/Characters/Warrior/ABP_Warrior.ABP_Warrior"));

	if (HeroBP && HeroBP->GeneratedClass)
	{
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

		FKismetEditorUtilities::CompileBlueprint(HeroBP);
		SaveAsset(HeroBP, HeroBP->GetOutermost(), TEXT("/Game/Blueprints/BP_DiabloHero"));
	}

	// --- BP_DiabloPlayerController: set input action + mapping context ---
	UBlueprint* ControllerBP = LoadObject<UBlueprint>(nullptr, TEXT("/Game/Blueprints/BP_DiabloPlayerController.BP_DiabloPlayerController"));
	UInputAction* ClickAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/Actions/IA_Click.IA_Click"));
	UInputMappingContext* IMC = LoadObject<UInputMappingContext>(nullptr, TEXT("/Game/Input/IMC_Diablo.IMC_Diablo"));

	if (ControllerBP && ControllerBP->GeneratedClass)
	{
		UObject* CDO = ControllerBP->GeneratedClass->GetDefaultObject();
		if (CDO)
		{
			if (FProperty* ClickProp = ControllerBP->GeneratedClass->FindPropertyByName(TEXT("ClickAction")))
			{
				FObjectProperty* ObjProp = CastField<FObjectProperty>(ClickProp);
				if (ObjProp && ClickAction)
				{
					ObjProp->SetObjectPropertyValue(ObjProp->ContainerPtrToValuePtr<void>(CDO), ClickAction);
					UE_LOG(LogTemp, Display, TEXT("[DiabloTools] BP_DiabloPlayerController: set ClickAction"));
				}
			}
			if (FProperty* IMCProp = ControllerBP->GeneratedClass->FindPropertyByName(TEXT("DefaultMappingContext")))
			{
				FObjectProperty* ObjProp = CastField<FObjectProperty>(IMCProp);
				if (ObjProp && IMC)
				{
					ObjProp->SetObjectPropertyValue(ObjProp->ContainerPtrToValuePtr<void>(CDO), IMC);
					UE_LOG(LogTemp, Display, TEXT("[DiabloTools] BP_DiabloPlayerController: set DefaultMappingContext"));
				}
			}
		}

		FKismetEditorUtilities::CompileBlueprint(ControllerBP);
		SaveAsset(ControllerBP, ControllerBP->GetOutermost(), TEXT("/Game/Blueprints/BP_DiabloPlayerController"));
	}

	// --- BP_DiabloGameMode: set default pawn + controller classes ---
	UBlueprint* GameModeBP = LoadObject<UBlueprint>(nullptr, TEXT("/Game/Blueprints/BP_DiabloGameMode.BP_DiabloGameMode"));

	if (GameModeBP && GameModeBP->GeneratedClass && HeroBP && ControllerBP)
	{
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
}

// ---------------------------------------------------------------------------
// Utility functions
// ---------------------------------------------------------------------------

void FDiabloAssetGenerator::DeleteExistingAsset(const FString& PackagePath)
{
	if (!FPackageName::DoesPackageExist(PackagePath)) return;

	const FString FilePath = FPackageName::LongPackageNameToFilename(
		PackagePath, FPackageName::GetAssetPackageExtension());

	// Unload the package from memory if loaded
	if (UPackage* Loaded = FindPackage(nullptr, *PackagePath))
	{
		TArray<UObject*> ObjectsInPackage;
		ForEachObjectWithPackage(Loaded, [&](UObject* Obj)
		{
			ObjectsInPackage.Add(Obj);
			return true;
		});

		for (UObject* Obj : ObjectsInPackage)
		{
			if (!Obj->IsRooted())
			{
				Obj->ClearFlags(RF_Public | RF_Standalone);
				Obj->MarkAsGarbage();
			}
		}

		Loaded->ClearFlags(RF_Public | RF_Standalone);
		Loaded->MarkAsGarbage();
	}

	// Delete the file on disk
	if (IFileManager::Get().FileExists(*FilePath))
	{
		IFileManager::Get().Delete(*FilePath, false, true, true);
		UE_LOG(LogTemp, Display, TEXT("[DiabloTools] Deleted existing asset: %s"), *PackagePath);
	}
}

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
