#include "BlueprintCreateCommandlet.h"
#include "Engine/Blueprint.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"
#include "Misc/Paths.h"

int32 UBlueprintCreateCommandlet::Main(const FString& Params)
{
    TArray<FString> Tokens;
    TArray<FString> Switches;
    TMap<FString, FString> ParamMap;
    ParseCommandLine(*Params, Tokens, Switches, ParamMap);

    const FString* AssetPath = ParamMap.Find(TEXT("AssetPath"));
    const FString* ParentClassStr = ParamMap.Find(TEXT("ParentClass"));

    if (!AssetPath || !ParentClassStr)
    {
        UE_LOG(LogTemp, Error, TEXT("Usage: -run=BlueprintCreate -AssetPath=/Game/Path/BP_Name -ParentClass=/Script/Module.ClassName"));
        return 1;
    }

    // Check if already exists
    FString FullPath = *AssetPath;
    FString AssetName = FPaths::GetBaseFilename(FullPath);
    if (!FullPath.Contains(TEXT(".")))
    {
        FullPath = FullPath + TEXT(".") + AssetName;
    }

    UBlueprint* ExistingBP = LoadObject<UBlueprint>(nullptr, *FullPath);
    if (ExistingBP)
    {
        UE_LOG(LogTemp, Display, TEXT("Blueprint already exists: %s"), **AssetPath);
        return 0;
    }

    // Resolve parent class
    UClass* ParentClass = StaticLoadClass(UObject::StaticClass(), nullptr, **ParentClassStr);
    if (!ParentClass)
    {
        // Try loading as a Blueprint path
        FString BPFullPath = *ParentClassStr;
        if (!BPFullPath.Contains(TEXT(".")))
        {
            FString BPName = FPaths::GetBaseFilename(BPFullPath);
            BPFullPath = BPFullPath + TEXT(".") + BPName;
        }
        UBlueprint* ParentBP = LoadObject<UBlueprint>(nullptr, *BPFullPath);
        if (ParentBP && ParentBP->GeneratedClass)
        {
            ParentClass = ParentBP->GeneratedClass;
        }
    }

    if (!ParentClass)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to resolve parent class: %s"), **ParentClassStr);
        return 1;
    }

    // Create package
    FString PackagePath = *AssetPath;
    UPackage* Package = CreatePackage(*PackagePath);
    if (!Package)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create package: %s"), *PackagePath);
        return 1;
    }

    // Create Blueprint
    UBlueprint* BP = FKismetEditorUtilities::CreateBlueprint(
        ParentClass, Package, FName(*AssetName), BPTYPE_Normal,
        UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());

    if (!BP)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create Blueprint: %s"), **AssetPath);
        return 1;
    }

    // Compile and save
    FKismetEditorUtilities::CompileBlueprint(BP);
    FAssetRegistryModule::AssetCreated(BP);
    Package->MarkPackageDirty();

    FString Filename = FPackageName::LongPackageNameToFilename(
        Package->GetName(), FPackageName::GetAssetPackageExtension());
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    PlatformFile.CreateDirectoryTree(*FPaths::GetPath(Filename));
    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
    UPackage::SavePackage(Package, BP, *Filename, SaveArgs);

    UE_LOG(LogTemp, Display, TEXT("Blueprint created: %s (Parent: %s)"), **AssetPath, *ParentClass->GetName());
    return 0;
}
