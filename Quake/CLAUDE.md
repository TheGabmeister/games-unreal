# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Unreal Engine 5.7 single-player FPS recreating original Quake gameplay with primitive shapes. The full design is in [SPEC.md](SPEC.md) — read it before making non-trivial gameplay changes. Module name: `Quake`. The current scope is the v1 milestone defined in `SPEC.md` section 11.

## Build Commands

Build the editor target (the most common command during development):

```bash
"/c/Program Files/Epic Games/UE_5.7/Engine/Build/BatchFiles/Build.bat" QuakeEditor Win64 Development -Project="c:/dev/games-unreal/Quake/Quake.uproject" -WaitMutex -FromMsBuild
```

If a build fails with **"Unable to build while Live Coding is active"**, the user must close the Unreal Editor or press Ctrl+Alt+F11 to apply Live Coding. Do not work around this — ask the user to handle it.

Regenerate VS Code project files (after adding new `.h` / `.cpp` files, so IntelliSense and `compileCommands_Quake.json` pick them up):

```bash
export UE_DOTNET_DIR='/c/Program Files/Epic Games/UE_5.7/Engine/Binaries/ThirdParty/DotNet/8.0.412/win-x64'
export DOTNET_ROOT="$UE_DOTNET_DIR" PATH="$UE_DOTNET_DIR:$PATH" DOTNET_MULTILEVEL_LOOKUP=0 DOTNET_ROLL_FORWARD=LatestMajor
dotnet '/c/Program Files/Epic Games/UE_5.7/Engine/Binaries/DotNET/UnrealBuildTool/UnrealBuildTool.dll' -projectfiles -project='C:/dev/games-unreal/Quake/Quake.uproject' -game -engine -VSCode
```

A `PostToolUse` hook in `.claude/settings.local.json` runs this automatically after `Write` of any `Source/**.{h,cpp}` file. Manually generated files (created in the IDE) will not trigger it.

The system installs only .NET 10; UnrealBuildTool requires .NET 8. The bundled SDK at the path above is the working version — never invoke a system `dotnet` for UBT.

## IDE Integration Notes

IntelliSense errors like `cannot open source file "InputModifiers.h"` are usually false positives from VS Code's C++ extension when `compileCommands_Quake.json` is stale. The compile-test of truth is a real `Build.bat` run, not the IDE squiggles. Regenerate project files (above) before assuming an include is wrong.

## Architecture: C++ First With Thin Blueprint Layer

Per [SPEC.md](SPEC.md) section "Constraints", **all gameplay logic lives in C++**. Blueprints are used **only** as thin subclasses of C++ base classes for two purposes:

1. **Asset references** — assigning meshes, materials, and (future) sound assets to `UPROPERTY` slots.
2. **Per-instance parameter tuning** — tweaking values like enemy health or weapon damage without recompiling.

Blueprints contain **no nodes in event graphs** — only property defaults. Do not propose adding gameplay logic to Blueprints. If a feature requires runtime behavior, add it to the C++ base class.

## Architecture: Input Configuration

Enhanced Input uses Editor-authored assets (Input Actions and Input Mapping Context), assigned to `UPROPERTY(EditDefaultsOnly)` slots on `AQuakePlayerController` via a Blueprint subclass (`BP_QuakePlayerController`). The PlayerController adds the mapping context to the Enhanced Input subsystem in `BeginPlay`. Pawns access actions via `GetController<AQuakePlayerController>()->MoveAction` etc. and bind handlers in `SetupPlayerInputComponent`.

When adding a new input action, add a new `UPROPERTY(EditDefaultsOnly)` slot on `AQuakePlayerController`, create the IA asset in the Editor, map it in IMC_Default, then assign it in the BP subclass. Do not create IA/IMC objects at runtime with `NewObject<>()` — that approach was tried and reverted.

## Architecture: Editor-Only Pieces

The following must exist in the Editor; they cannot be created from C++ alone (per `SPEC.md` section 10):

- **Levels** (`.umap`) under `Content/Maps/` — World Partition disabled per level.
- **One master material** `M_QuakeBase` with `BaseColor` / `Emissive` / `Metallic` / `Roughness` parameters; everything else is a `MaterialInstance` of it. Runtime tinting (damage flash, powerup overlays) uses `UMaterialInstanceDynamic` from C++.
- **NavMesh** — `NavMeshBoundsVolume` per level, agent radius 35 / height 180 to match the player capsule.
- **Project Settings** in `Config/Default*.ini` — version-controlled.

The per-level checklist in `SPEC.md` section 10.5 is the authoritative reference when creating new maps.

## Auto-Memory

The user has a persistent file-based memory system at `C:\Users\Admin\.claude\projects\c--dev-games-unreal\memory\`. Save user/feedback/project memories there per the auto-memory protocol, and check existing memories before recommending design choices.
