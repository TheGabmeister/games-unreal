#include "BlueprintEditCommandlet.h"
#include "Engine/Blueprint.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "UObject/UnrealType.h"
#include "UObject/SavePackage.h"
#include "Misc/FileHelper.h"

int32 UBlueprintEditCommandlet::Main(const FString& Params)
{
    TArray<FString> Tokens;
    TArray<FString> Switches;
    TMap<FString, FString> ParamMap;
    ParseCommandLine(*Params, Tokens, Switches, ParamMap);

    const FString* AssetPath = ParamMap.Find(TEXT("AssetPath"));
    if (!AssetPath)
    {
        UE_LOG(LogTemp, Error, TEXT("Usage: -run=BlueprintEdit -AssetPath=/Game/Path/To/BP -Set=\"PropName=Value\""));
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

    if (!BP->GeneratedClass)
    {
        UE_LOG(LogTemp, Error, TEXT("Blueprint has no GeneratedClass: %s"), **AssetPath);
        return 1;
    }

    UObject* CDO = BP->GeneratedClass->GetDefaultObject();
    if (!CDO)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get CDO for: %s"), **AssetPath);
        return 1;
    }

    // Collect all -Set= parameters from the raw command line
    // ParseCommandLine only gives us the last -Set, so we parse manually
    TArray<FString> SetPairs;
    {
        FString RawParams = Params;
        int32 Idx = 0;
        while (true)
        {
            Idx = RawParams.Find(TEXT("-Set="), ESearchCase::IgnoreCase, ESearchDir::FromStart, Idx);
            if (Idx == INDEX_NONE) break;

            int32 ValueStart = Idx + 5; // skip "-Set="
            FString Value;

            if (ValueStart < RawParams.Len() && RawParams[ValueStart] == TEXT('"'))
            {
                int32 CloseQuote = RawParams.Find(TEXT("\""), ESearchCase::IgnoreCase, ESearchDir::FromStart, ValueStart + 1);
                if (CloseQuote != INDEX_NONE)
                {
                    Value = RawParams.Mid(ValueStart + 1, CloseQuote - ValueStart - 1);
                }
            }
            else
            {
                int32 SpaceIdx = RawParams.Find(TEXT(" "), ESearchCase::IgnoreCase, ESearchDir::FromStart, ValueStart);
                if (SpaceIdx != INDEX_NONE)
                {
                    Value = RawParams.Mid(ValueStart, SpaceIdx - ValueStart);
                }
                else
                {
                    Value = RawParams.Mid(ValueStart);
                }
            }

            if (!Value.IsEmpty())
            {
                SetPairs.Add(Value);
            }
            Idx = ValueStart;
        }
    }

    if (SetPairs.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("No -Set= parameters provided."));
        return 1;
    }

    int32 SuccessCount = 0;

    for (const FString& Pair : SetPairs)
    {
        FString PropName, ValueStr;
        if (!Pair.Split(TEXT("="), &PropName, &ValueStr))
        {
            UE_LOG(LogTemp, Warning, TEXT("Invalid Set format (expected PropName=Value): %s"), *Pair);
            continue;
        }

        // Resolve dot-separated property paths (e.g., "Mesh.SkeletalMeshAsset")
        UObject* TargetObj = CDO;
        FString LeafPropName = PropName;

        if (PropName.Contains(TEXT(".")))
        {
            TArray<FString> PathParts;
            PropName.ParseIntoArray(PathParts, TEXT("."));
            LeafPropName = PathParts.Last();

            bool bPathValid = true;
            for (int32 i = 0; i < PathParts.Num() - 1; i++)
            {
                FProperty* NavProp = TargetObj->GetClass()->FindPropertyByName(FName(*PathParts[i]));
                if (!NavProp)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Path segment '%s' not found on %s"), *PathParts[i], *TargetObj->GetClass()->GetName());
                    bPathValid = false;
                    break;
                }

                FObjectProperty* ObjProp = CastField<FObjectProperty>(NavProp);
                if (!ObjProp)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Path segment '%s' is not an object property"), *PathParts[i]);
                    bPathValid = false;
                    break;
                }

                UObject* SubObj = ObjProp->GetObjectPropertyValue(NavProp->ContainerPtrToValuePtr<void>(TargetObj));
                if (!SubObj)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Path segment '%s' resolved to null"), *PathParts[i]);
                    bPathValid = false;
                    break;
                }

                TargetObj = SubObj;
            }

            if (!bPathValid)
            {
                continue;
            }
        }

        FProperty* Prop = TargetObj->GetClass()->FindPropertyByName(FName(*LeafPropName));
        if (!Prop)
        {
            UE_LOG(LogTemp, Warning, TEXT("Property '%s' not found on %s"), *LeafPropName, *TargetObj->GetClass()->GetName());
            continue;
        }

        void* ValuePtr = Prop->ContainerPtrToValuePtr<void>(TargetObj);
        const TCHAR* Result = Prop->ImportText_Direct(*ValueStr, ValuePtr, TargetObj, PPF_None);

        if (Result)
        {
            UE_LOG(LogTemp, Display, TEXT("Set %s = %s"), *PropName, *ValueStr);
            SuccessCount++;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to set '%s' to '%s'"), *PropName, *ValueStr);
        }
    }

    if (SuccessCount == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("No properties were modified."));
        return 1;
    }

    // Compile first, then re-apply component properties on the fresh CDO.
    // Compilation regenerates the CDO and its default subobjects, which wipes
    // in-memory changes to component properties. CDO-level properties survive
    // because they're serialized as Blueprint overrides, but component sub-properties
    // set via dot-path need to be re-applied after compilation.
    FKismetEditorUtilities::CompileBlueprint(BP);

    UObject* FreshCDO = BP->GeneratedClass->GetDefaultObject(true);
    for (const FString& Pair : SetPairs)
    {
        FString PropName, ValueStr;
        if (!Pair.Split(TEXT("="), &PropName, &ValueStr)) continue;
        if (!PropName.Contains(TEXT("."))) continue;

        UObject* TargetObj = FreshCDO;
        TArray<FString> PathParts;
        PropName.ParseIntoArray(PathParts, TEXT("."));

        bool bPathValid = true;
        for (int32 i = 0; i < PathParts.Num() - 1; i++)
        {
            FProperty* NavProp = TargetObj->GetClass()->FindPropertyByName(FName(*PathParts[i]));
            FObjectProperty* ObjProp = NavProp ? CastField<FObjectProperty>(NavProp) : nullptr;
            UObject* SubObj = ObjProp ? ObjProp->GetObjectPropertyValue(NavProp->ContainerPtrToValuePtr<void>(TargetObj)) : nullptr;
            if (!SubObj) { bPathValid = false; break; }
            TargetObj = SubObj;
        }
        if (!bPathValid) continue;

        FProperty* Prop = TargetObj->GetClass()->FindPropertyByName(FName(*PathParts.Last()));
        if (!Prop) continue;

        void* ValuePtr = Prop->ContainerPtrToValuePtr<void>(TargetObj);
        Prop->ImportText_Direct(*ValueStr, ValuePtr, TargetObj, PPF_None);
    }

    UPackage* Package = BP->GetOutermost();
    Package->MarkPackageDirty();

    FString Filename = FPackageName::LongPackageNameToFilename(
        Package->GetName(), FPackageName::GetAssetPackageExtension());
    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
    UPackage::SavePackage(Package, BP, *Filename, SaveArgs);

    UE_LOG(LogTemp, Display, TEXT("Blueprint saved: %s (%d properties modified)"), **AssetPath, SuccessCount);
    return 0;
}
