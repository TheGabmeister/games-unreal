#include "BlueprintDumpCommandlet.h"
#include "Engine/Blueprint.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "Components/ActorComponent.h"
#include "UObject/UnrealType.h"
#include "Misc/FileHelper.h"

int32 UBlueprintDumpCommandlet::Main(const FString& Params)
{
    TArray<FString> Tokens;
    TArray<FString> Switches;
    TMap<FString, FString> ParamMap;
    ParseCommandLine(*Params, Tokens, Switches, ParamMap);

    const FString* AssetPath = ParamMap.Find(TEXT("AssetPath"));
    if (!AssetPath)
    {
        UE_LOG(LogTemp, Error, TEXT("Usage: -run=BlueprintDump -AssetPath=/Game/Path/To/Blueprint"));
        return 1;
    }

    FString FullPath = *AssetPath;
    if (!FullPath.Contains(TEXT(".")))
    {
        FString AssetName = FPaths::GetBaseFilename(FullPath);
        FullPath = FullPath + TEXT(".") + AssetName;
    }

    UBlueprint* BP = LoadObject<UBlueprint>(nullptr, *FullPath);
    if (!BP)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load Blueprint: %s"), **AssetPath);
        return 1;
    }

    FString Output;
    Output += FString::Printf(TEXT("Blueprint: %s\n"), *BP->GetPathName());
    Output += FString::Printf(TEXT("Parent Class: %s\n"), *BP->ParentClass->GetPathName());
    Output += FString::Printf(TEXT("Generated Class: %s\n"), BP->GeneratedClass ? *BP->GeneratedClass->GetPathName() : TEXT("None"));
    Output += TEXT("\n");

    // CDO properties
    if (BP->GeneratedClass)
    {
        UObject* CDO = BP->GeneratedClass->GetDefaultObject();
        if (CDO)
        {
            Output += TEXT("=== Default Properties ===\n");
            for (TFieldIterator<FProperty> It(BP->GeneratedClass); It; ++It)
            {
                FProperty* Prop = *It;
                if (Prop->GetOwnerClass() == UObject::StaticClass() ||
                    Prop->GetOwnerClass() == AActor::StaticClass())
                {
                    continue;
                }

                FString ValueStr;
                const void* Value = Prop->ContainerPtrToValuePtr<void>(CDO);
                Prop->ExportTextItem_Direct(ValueStr, Value, nullptr, CDO, PPF_None);

                Output += FString::Printf(TEXT("  [%s] %s = %s\n"),
                    *Prop->GetOwnerClass()->GetName(),
                    *Prop->GetName(),
                    *ValueStr);
            }
            Output += TEXT("\n");
        }
    }

    // Components from SCS
    if (BP->SimpleConstructionScript)
    {
        const TArray<USCS_Node*>& Nodes = BP->SimpleConstructionScript->GetAllNodes();
        if (Nodes.Num() > 0)
        {
            Output += TEXT("=== Components (SCS) ===\n");
            for (const USCS_Node* Node : Nodes)
            {
                if (!Node || !Node->ComponentTemplate) continue;

                UActorComponent* Template = Node->ComponentTemplate;
                Output += FString::Printf(TEXT("  %s (%s)\n"),
                    *Node->GetVariableName().ToString(),
                    *Template->GetClass()->GetName());

                for (TFieldIterator<FProperty> It(Template->GetClass()); It; ++It)
                {
                    FProperty* Prop = *It;
                    if (!Prop->HasAnyPropertyFlags(CPF_Edit | CPF_BlueprintVisible))
                        continue;
                    if (Prop->GetOwnerClass() == UObject::StaticClass() ||
                        Prop->GetOwnerClass() == UActorComponent::StaticClass())
                        continue;

                    FString ValueStr;
                    const void* Value = Prop->ContainerPtrToValuePtr<void>(Template);
                    Prop->ExportTextItem_Direct(ValueStr, Value, nullptr, Template, PPF_None);

                    if (!ValueStr.IsEmpty())
                    {
                        Output += FString::Printf(TEXT("    %s = %s\n"),
                            *Prop->GetName(), *ValueStr);
                    }
                }
            }
            Output += TEXT("\n");
        }
    }

    // Write to file
    FString OutputPath = FPaths::ProjectSavedDir() / TEXT("BlueprintDump.txt");
    FFileHelper::SaveStringToFile(Output, *OutputPath);
    UE_LOG(LogTemp, Display, TEXT("Blueprint dump written to: %s"), *OutputPath);
    UE_LOG(LogTemp, Display, TEXT("\n%s"), *Output);

    return 0;
}
