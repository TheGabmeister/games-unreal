# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

Tony Hawk's Pro Skater 2 recreation in Unreal Engine 5.7. Single C++ module (`TonyHawk2`), no plugins beyond ModelingToolsEditorMode (editor-only). Enhanced Input System is the input framework (configured in DefaultInput.ini). The project is in early development — SPEC.md has the full gameplay systems spec, PHASES.md has the 17-phase implementation roadmap.

## Build

Engine is installed at `C:\Program Files\Epic Games\UE_5.7`. All build commands run from that directory.

```
# Build (Development Editor — the day-to-day target)
"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" TonyHawk2Editor Win64 Development "C:\dev\games-unreal\TonyHawk2\TonyHawk2.uproject" -waitmutex

# Build (Game, various configs: Debug | DebugGame | Development | Test | Shipping)
"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" TonyHawk2 Win64 Development "C:\dev\games-unreal\TonyHawk2\TonyHawk2.uproject" -waitmutex

# Clean
"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Clean.bat" TonyHawk2Editor Win64 Development "C:\dev\games-unreal\TonyHawk2\TonyHawk2.uproject" -waitmutex

# Regenerate VS Code project files
"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\RunUBT.bat" -projectfiles -vscode -project="C:\dev\games-unreal\TonyHawk2\TonyHawk2.uproject" -game -engine -dotnet
```

The .code-workspace file has VS Code tasks for all platform/config combinations (Win64, Android × Debug through Shipping).

## Module Dependencies

Declared in `Source/TonyHawk2/TonyHawk2.Build.cs`:
- **Public**: Core, CoreUObject, Engine, InputCore, EnhancedInput
- Add Slate/SlateCore when UI work begins; add OnlineSubsystem if multiplayer needs networking

## Source Layout

All C++ lives under `Source/TonyHawk2/`. Currently just the module bootstrap (`TonyHawk2.h`, `TonyHawk2.cpp`). As gameplay classes are added, organize by system:
- New `.h`/`.cpp` files go directly in `Source/TonyHawk2/` (flat structure until complexity warrants subdirectories)
- Target configs: `Source/TonyHawk2.Target.cs` (Game) and `Source/TonyHawk2Editor.Target.cs` (Editor)

## Design Documents

- **SPEC.md** — full gameplay systems spec (trick system, scoring formulas, stat tables, level goals, economy, multiplayer modes). This is the source of truth for game mechanics.
- **PHASES.md** — 17-phase implementation plan. Phases 1–6 = vertical slice (movement through special tricks). Phases 7–17 = remaining systems and content expansion.

When implementing a phase, cross-reference both documents: PHASES.md for what to build, SPEC.md for exact values (damage formulas, stat distributions, thresholds).

## Engine Config Notes

- Enhanced Input is the input system — do not use legacy input. Input Mapping Contexts and Input Actions are editor-authored assets assigned in Blueprint, not constructed at runtime via NewObject.
- Rendering: DX12/SM6, ray tracing enabled, Substrate materials, Virtual Shadow Maps. Target hardware is desktop maximum.
- AnimBPs are manually authored in the editor — do not attempt to construct Animation Blueprint graphs programmatically.
