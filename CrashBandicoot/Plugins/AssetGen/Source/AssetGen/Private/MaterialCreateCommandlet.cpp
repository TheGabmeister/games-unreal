#include "MaterialCreateCommandlet.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialInterface.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"
#include "Misc/Paths.h"

static TArray<FString> ParseMultiParam(const FString& Params, const FString& Prefix)
{
    TArray<FString> Results;
    int32 Idx = 0;
    while (true)
    {
        Idx = Params.Find(Prefix, ESearchCase::IgnoreCase, ESearchDir::FromStart, Idx);
        if (Idx == INDEX_NONE) break;

        int32 ValueStart = Idx + Prefix.Len();
        FString Value;

        if (ValueStart < Params.Len() && Params[ValueStart] == TEXT('"'))
        {
            int32 CloseQuote = Params.Find(TEXT("\""), ESearchCase::IgnoreCase, ESearchDir::FromStart, ValueStart + 1);
            if (CloseQuote != INDEX_NONE)
            {
                Value = Params.Mid(ValueStart + 1, CloseQuote - ValueStart - 1);
            }
        }
        else
        {
            int32 SpaceIdx = Params.Find(TEXT(" "), ESearchCase::IgnoreCase, ESearchDir::FromStart, ValueStart);
            if (SpaceIdx != INDEX_NONE)
            {
                Value = Params.Mid(ValueStart, SpaceIdx - ValueStart);
            }
            else
            {
                Value = Params.Mid(ValueStart);
            }
        }

        if (!Value.IsEmpty())
        {
            Results.Add(Value);
        }
        Idx = ValueStart;
    }
    return Results;
}

int32 UMaterialCreateCommandlet::Main(const FString& Params)
{
    TArray<FString> Tokens;
    TArray<FString> Switches;
    TMap<FString, FString> ParamMap;
    ParseCommandLine(*Params, Tokens, Switches, ParamMap);

    const FString* AssetPath = ParamMap.Find(TEXT("AssetPath"));
    const FString* ParentPath = ParamMap.Find(TEXT("ParentMaterial"));

    if (!AssetPath || !ParentPath)
    {
        UE_LOG(LogTemp, Error, TEXT("Usage: -run=MaterialCreate -AssetPath=/Game/Path/MI_Name -ParentMaterial=/Game/Path/Material [-SetScalar=\"Name=Value\"] [-SetVector=\"Name=(R=,G=,B=,A=)\"] [-SetTexture=\"Name=/Game/Path\"]"));
        return 1;
    }

    // Load parent material
    FString ParentFullPath = *ParentPath;
    if (!ParentFullPath.Contains(TEXT(".")))
    {
        FString ParentName = FPaths::GetBaseFilename(ParentFullPath);
        ParentFullPath = ParentFullPath + TEXT(".") + ParentName;
    }

    UMaterialInterface* ParentMaterial = LoadObject<UMaterialInterface>(nullptr, *ParentFullPath);
    if (!ParentMaterial)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load parent material: %s"), **ParentPath);
        return 1;
    }

    // Resolve asset path
    FString PackagePath = *AssetPath;
    FString AssetName = FPaths::GetBaseFilename(PackagePath);
    FString FullPath = PackagePath;
    if (!FullPath.Contains(TEXT(".")))
    {
        FullPath = FullPath + TEXT(".") + AssetName;
    }

    // Load existing or create new
    UMaterialInstanceConstant* MIC = LoadObject<UMaterialInstanceConstant>(nullptr, *FullPath);
    bool bCreated = false;

    if (!MIC)
    {
        UPackage* Package = CreatePackage(*PackagePath);
        if (!Package)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create package: %s"), *PackagePath);
            return 1;
        }

        MIC = NewObject<UMaterialInstanceConstant>(
            Package, UMaterialInstanceConstant::StaticClass(),
            FName(*AssetName), RF_Public | RF_Standalone);

        MIC->SetParentEditorOnly(ParentMaterial);
        FAssetRegistryModule::AssetCreated(MIC);
        bCreated = true;
    }

    // Apply scalar parameters
    TArray<FString> ScalarPairs = ParseMultiParam(Params, TEXT("-SetScalar="));
    for (const FString& Pair : ScalarPairs)
    {
        FString ParamName, ValueStr;
        if (Pair.Split(TEXT("="), &ParamName, &ValueStr))
        {
            FMaterialParameterInfo ParamInfo{FName(*ParamName)};
            MIC->SetScalarParameterValueEditorOnly(ParamInfo, FCString::Atof(*ValueStr));
            UE_LOG(LogTemp, Display, TEXT("SetScalar: %s = %s"), *ParamName, *ValueStr);
        }
    }

    // Apply vector parameters
    TArray<FString> VectorPairs = ParseMultiParam(Params, TEXT("-SetVector="));
    for (const FString& Pair : VectorPairs)
    {
        FString ParamName, ValueStr;
        if (Pair.Split(TEXT("="), &ParamName, &ValueStr))
        {
            FLinearColor Color;
            if (Color.InitFromString(ValueStr))
            {
                FMaterialParameterInfo ParamInfo{FName(*ParamName)};
                MIC->SetVectorParameterValueEditorOnly(ParamInfo, Color);
                UE_LOG(LogTemp, Display, TEXT("SetVector: %s = %s"), *ParamName, *ValueStr);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Failed to parse color for '%s': %s"), *ParamName, *ValueStr);
            }
        }
    }

    // Apply texture parameters
    TArray<FString> TexturePairs = ParseMultiParam(Params, TEXT("-SetTexture="));
    for (const FString& Pair : TexturePairs)
    {
        FString ParamName, TexPath;
        if (Pair.Split(TEXT("="), &ParamName, &TexPath))
        {
            FString TexFullPath = TexPath;
            if (!TexFullPath.Contains(TEXT(".")))
            {
                FString TexName = FPaths::GetBaseFilename(TexFullPath);
                TexFullPath = TexFullPath + TEXT(".") + TexName;
            }

            UTexture* Tex = LoadObject<UTexture>(nullptr, *TexFullPath);
            if (Tex)
            {
                FMaterialParameterInfo ParamInfo{FName(*ParamName)};
                MIC->SetTextureParameterValueEditorOnly(ParamInfo, Tex);
                UE_LOG(LogTemp, Display, TEXT("SetTexture: %s = %s"), *ParamName, *TexPath);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Failed to load texture: %s"), *TexPath);
            }
        }
    }

    MIC->PostEditChange();

    // Save
    UPackage* Package = MIC->GetOutermost();
    Package->MarkPackageDirty();

    FString Filename = FPackageName::LongPackageNameToFilename(
        Package->GetName(), FPackageName::GetAssetPackageExtension());
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    PlatformFile.CreateDirectoryTree(*FPaths::GetPath(Filename));
    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
    UPackage::SavePackage(Package, MIC, *Filename, SaveArgs);

    UE_LOG(LogTemp, Display, TEXT("Material instance %s: %s (Parent: %s)"),
        bCreated ? TEXT("created") : TEXT("updated"), **AssetPath, *ParentMaterial->GetName());
    return 0;
}
