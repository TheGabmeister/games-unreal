#include "AssetImportCommandlet.h"
#include "AssetToolsModule.h"
#include "AssetImportTask.h"
#include "Factories/FbxFactory.h"
#include "Factories/FbxImportUI.h"
#include "Factories/FbxStaticMeshImportData.h"
#include "Factories/FbxSkeletalMeshImportData.h"
#include "Misc/Paths.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/Skeleton.h"
#include "UObject/SavePackage.h"

int32 UAssetImportCommandlet::Main(const FString& Params)
{
    TArray<FString> Tokens;
    TArray<FString> Switches;
    TMap<FString, FString> ParamMap;
    ParseCommandLine(*Params, Tokens, Switches, ParamMap);

    const FString* SourceFile = ParamMap.Find(TEXT("SourceFile"));
    const FString* DestPath = ParamMap.Find(TEXT("DestPath"));

    if (!SourceFile || !DestPath)
    {
        UE_LOG(LogTemp, Error, TEXT("Usage: -run=AssetImport -SourceFile=\"path\" -DestPath=/Game/Path [-DestName=Name] [-Type=StaticMesh|SkeletalMesh|Animation|Auto] [-Skeleton=/Game/Path] [-Replace]"));
        return 1;
    }

    if (!FPaths::FileExists(*SourceFile))
    {
        UE_LOG(LogTemp, Error, TEXT("Source file does not exist: %s"), **SourceFile);
        return 1;
    }

    const FString* DestName = ParamMap.Find(TEXT("DestName"));
    const FString* TypeStr = ParamMap.Find(TEXT("Type"));
    const FString* SkeletonPath = ParamMap.Find(TEXT("Skeleton"));
    bool bReplace = Switches.Contains(TEXT("Replace"));

    FString AssetName = DestName ? *DestName : FPaths::GetBaseFilename(*SourceFile);
    FString Extension = FPaths::GetExtension(*SourceFile).ToLower();
    FString ImportType = TypeStr ? *TypeStr : TEXT("Auto");

    UAssetImportTask* Task = NewObject<UAssetImportTask>();
    Task->Filename = *SourceFile;
    Task->DestinationPath = *DestPath;
    Task->DestinationName = AssetName;
    Task->bReplaceExisting = bReplace;
    Task->bReplaceExistingSettings = bReplace;
    Task->bAutomated = true;
    Task->bSave = true;

    if (Extension == TEXT("fbx"))
    {
        UFbxFactory* FbxFactory = NewObject<UFbxFactory>();
        UFbxImportUI* ImportUI = FbxFactory->ImportUI;

        ImportUI->bAutomatedImportShouldDetectType = false;
        ImportUI->bImportMaterials = false;
        ImportUI->bImportTextures = false;

        if (ImportType.Equals(TEXT("SkeletalMesh"), ESearchCase::IgnoreCase))
        {
            ImportUI->MeshTypeToImport = FBXIT_SkeletalMesh;
            ImportUI->bImportAsSkeletal = true;
            ImportUI->bImportMesh = true;
            ImportUI->bImportAnimations = false;
            ImportUI->SkeletalMeshImportData.Get()->bConvertSceneUnit = true;
        }
        else if (ImportType.Equals(TEXT("Animation"), ESearchCase::IgnoreCase))
        {
            ImportUI->MeshTypeToImport = FBXIT_Animation;
            ImportUI->bImportMesh = false;
            ImportUI->bImportAnimations = true;

            if (!SkeletonPath)
            {
                UE_LOG(LogTemp, Error, TEXT("Animation import requires -Skeleton=/Game/Path/To/Skeleton"));
                return 1;
            }

            FString SkelFullPath = *SkeletonPath;
            if (!SkelFullPath.Contains(TEXT(".")))
            {
                FString SkelName = FPaths::GetBaseFilename(SkelFullPath);
                SkelFullPath = SkelFullPath + TEXT(".") + SkelName;
            }

            USkeleton* Skeleton = LoadObject<USkeleton>(nullptr, *SkelFullPath);
            if (!Skeleton)
            {
                // Try with _Skeleton suffix
                FString AltPath = *SkeletonPath + TEXT("_Skeleton");
                if (!AltPath.Contains(TEXT(".")))
                {
                    FString AltName = FPaths::GetBaseFilename(AltPath);
                    AltPath = AltPath + TEXT(".") + AltName;
                }
                Skeleton = LoadObject<USkeleton>(nullptr, *AltPath);
            }

            if (!Skeleton)
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to load Skeleton: %s"), **SkeletonPath);
                return 1;
            }

            ImportUI->Skeleton = Skeleton;
        }
        else
        {
            ImportUI->MeshTypeToImport = FBXIT_StaticMesh;
            ImportUI->bImportMesh = true;
            ImportUI->bImportAnimations = false;
            ImportUI->StaticMeshImportData.Get()->bConvertSceneUnit = true;
        }

        Task->Factory = FbxFactory;
        Task->Options = ImportUI;
    }

    FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
    TArray<UAssetImportTask*> Tasks = { Task };
    AssetToolsModule.Get().ImportAssetTasks(Tasks);

    TArray<UObject*> ImportedObjects = Task->GetObjects();
    if (ImportedObjects.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Import failed for: %s"), **SourceFile);
        return 1;
    }

    for (UObject* Obj : ImportedObjects)
    {
        UE_LOG(LogTemp, Display, TEXT("Imported: %s (%s)"), *Obj->GetPathName(), *Obj->GetClass()->GetName());

        // For skeletal meshes, find and save the skeleton asset explicitly
        if (USkeletalMesh* SkelMesh = Cast<USkeletalMesh>(Obj))
        {
            USkeleton* Skeleton = SkelMesh->GetSkeleton();
            if (Skeleton)
            {
                UPackage* SkelPackage = Skeleton->GetOutermost();
                if (SkelPackage && SkelPackage != GetTransientPackage())
                {
                    SkelPackage->MarkPackageDirty();
                    FString SkelFilename = FPackageName::LongPackageNameToFilename(
                        SkelPackage->GetName(), FPackageName::GetAssetPackageExtension());
                    FSavePackageArgs SaveArgs;
                    SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
                    UPackage::SavePackage(SkelPackage, Skeleton, *SkelFilename, SaveArgs);
                    UE_LOG(LogTemp, Display, TEXT("Saved skeleton: %s"), *Skeleton->GetPathName());
                }
                else
                {
                    // Skeleton is in transient package — move it to the same folder as the mesh
                    FString SkelAssetName = Obj->GetName() + TEXT("_Skeleton");
                    FString SkelPackagePath = *DestPath / SkelAssetName;
                    UPackage* NewSkelPackage = CreatePackage(*SkelPackagePath);
                    Skeleton->Rename(*SkelAssetName, NewSkelPackage);
                    Skeleton->SetFlags(RF_Public | RF_Standalone);
                    NewSkelPackage->MarkPackageDirty();
                    FString SkelFilename = FPackageName::LongPackageNameToFilename(
                        NewSkelPackage->GetName(), FPackageName::GetAssetPackageExtension());
                    FSavePackageArgs SaveArgs;
                    SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
                    UPackage::SavePackage(NewSkelPackage, Skeleton, *SkelFilename, SaveArgs);
                    UE_LOG(LogTemp, Display, TEXT("Created and saved skeleton: %s"), *Skeleton->GetPathName());
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("SkeletalMesh has no skeleton: %s"), *Obj->GetPathName());
            }
        }
    }

    return 0;
}
