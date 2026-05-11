


#include "Settings/CBDeveloperSettings.h"

const UCBDeveloperSettings* UCBDeveloperSettings::GetCBDeveloperSettings()
{
	return GetDefault<UCBDeveloperSettings>();
}

FName UCBDeveloperSettings::GetContainerName() const
{
	static const FName ContainerName("Project");
	return ContainerName;
}

FName UCBDeveloperSettings::GetCategoryName() const
{
	static const FName EditorCategoryName("Project");
	return EditorCategoryName;
}

FName UCBDeveloperSettings::GetSectionName() const
{
	static const FName TargetSectionName("CB Developer Settings");
	return TargetSectionName;
}

#if WITH_EDITOR
FText UCBDeveloperSettings::GetSectionText() const
{
	static const FText TargetSectionText = FText::FromString("CB Settings"); 
	return TargetSectionText;
}

FText UCBDeveloperSettings::GetSectionDescription() const
{
	static const FText TargetSectionDescription = FText::FromString("Project settings specific to CB");
	return TargetSectionDescription;
}

#endif

bool UCBDeveloperSettings::ShouldSkipLogoTrain()
{
	bool result = false; 

#if WITH_EDITOR
	result = bSkipLogoTrain;
#endif

	return result;
}