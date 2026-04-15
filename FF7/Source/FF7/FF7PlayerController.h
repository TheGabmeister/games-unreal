// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FF7PlayerController.generated.h"

class SFF7DialoguePopup;
class UDataTable;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

/**
 * Field / battle player controller (SPEC §2.3, §2.4).
 * EnhancedInput assets (IMC + IAs) are assigned on the BP subclass so
 * designers can swap them without touching C++. On possession, the IMC is
 * pushed to the local player's input subsystem; bindings fire C++ handlers.
 *
 * Also owns the active dialogue state and popup widget lifetime (§2.4).
 */
UCLASS()
class FF7_API AFF7PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FF7|Input")
	TSoftObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FF7|Input")
	TSoftObjectPtr<UInputAction> IA_Move;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FF7|Input")
	TSoftObjectPtr<UInputAction> IA_Interact;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FF7|Input")
	TSoftObjectPtr<UInputAction> IA_MenuToggle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FF7|Input")
	TSoftObjectPtr<UInputAction> IA_Escape;

	/** Distance (cm) ahead of the pawn to line-trace for interactables. */
	UPROPERTY(EditDefaultsOnly, Category = "FF7|Interact")
	float InteractTraceDistance = 200.0f;

	/** Begin dialogue from the given table + starting row. Creates/attaches the popup. */
	void StartDialogue(UDataTable* Table, FName StartRowId);

	/** Advance to NextId; ends dialogue if NextId is None. */
	void AdvanceDialogue();

	/** Tear down the popup and clear dialogue state. */
	void EndDialogue();

	bool IsDialogueActive() const { return bDialogueActive; }
	FName GetCurrentDialogueRowId() const { return CurrentRowId; }

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void SetupInputComponent() override;

private:
	void HandleMove(const FInputActionValue& Value);
	void HandleInteract(const FInputActionValue& Value);

	/** Trace forward, fallback to closest overlapping interactable. */
	AActor* FindInteractable() const;

	/** Look up CurrentRowId in CurrentDialogueTable and refresh cached text. */
	void LoadCurrentRow();

	UPROPERTY(Transient)
	TObjectPtr<UDataTable> CurrentDialogueTable;

	FName CurrentRowId = NAME_None;
	FText CachedSpeakerText;
	FText CachedLineText;
	bool bDialogueActive = false;

	TSharedPtr<SFF7DialoguePopup> DialoguePopup;
};
