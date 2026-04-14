#include "QuakeGameUserSettings.h"

UQuakeGameUserSettings::UQuakeGameUserSettings()
{
	// Constructor defaults match the UPROPERTY initializers above; SetToDefaults
	// re-applies them on a settings reset.
}

UQuakeGameUserSettings* UQuakeGameUserSettings::Get()
{
	return Cast<UQuakeGameUserSettings>(UGameUserSettings::GetGameUserSettings());
}

void UQuakeGameUserSettings::SetMouseSensitivity(float NewValue)
{
	MouseSensitivity = FMath::Clamp(NewValue, 0.05f, 5.f);
}

void UQuakeGameUserSettings::SetMasterVolume(float NewValue)
{
	MasterVolume = FMath::Clamp(NewValue, 0.f, 1.f);
}

void UQuakeGameUserSettings::SetToDefaults()
{
	Super::SetToDefaults();
	MouseSensitivity = 0.5f;
	MasterVolume = 1.f;
}
