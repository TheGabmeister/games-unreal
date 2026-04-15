// Copyright Epic Games, Inc. All Rights Reserved.

#include "FF7.h"

void FFF7Module::StartupModule()
{
	// Slate style set registration (SPEC §2.8) lives here once introduced in Phase 6.
}

void FFF7Module::ShutdownModule()
{
	// Unregister style set here in Phase 6.
}

IMPLEMENT_PRIMARY_GAME_MODULE(FFF7Module, FF7, "FF7");
