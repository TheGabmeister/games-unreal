// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"

/**
 * First Slate widget (SPEC §2.4, §2.8). A dialogue popup anchored at
 * bottom-center that shows the current speaker + line text. Advancing
 * and closing are driven by the controller — this widget is view-only.
 *
 * Values are bound via TAttribute closures so the same widget instance
 * updates as the controller mutates its CachedSpeakerText/CachedLineText.
 */
class FF7_API SFF7DialoguePopup : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFF7DialoguePopup) {}
		SLATE_ATTRIBUTE(FText, SpeakerText)
		SLATE_ATTRIBUTE(FText, LineText)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TAttribute<FText> SpeakerText;
	TAttribute<FText> LineText;
};
