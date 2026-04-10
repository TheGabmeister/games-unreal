# AGENTS.md

This file gives repo-specific guidance to coding agents working in this project.

## Project Summary

- Unreal Engine 5.7 single-player FPS inspired by Quake, implemented in C++.
- The full gameplay design lives in `SPEC.md`. Read it before making non-trivial gameplay or architecture changes.
- Module name: `Quake`.
- Current code lives under `Source/Quake/`.

## Core Rules

- Keep gameplay logic in C++.
- Use Blueprints only as thin asset/tuning layers when needed.
- Do not add Blueprint event graph gameplay logic.
- Keep the project single-player only. Do not introduce replication or networking systems unless explicitly requested.
- Preserve the primitive-shape visual direction unless the user asks to expand scope.

## Repo Map

- `Source/Quake/` - gameplay module source.
- `Config/` - version-controlled Unreal project settings.
- `Content/` - editor-authored assets and maps.
- `SPEC.md` - gameplay and systems spec.
- `CLAUDE.md` - additional architecture guidance and implementation notes.

## Existing Gameplay Architecture

- `AQuakeCharacter` owns first-person movement and player-side gameplay state.
- `AQuakePlayerController` owns Enhanced Input setup.
- `AQuakeGameMode` sets core game classes and rules.
- New shared input actions should be added in `AQuakePlayerController` rather than created as editor-only input assets.

## Damage Guidance

- Follow Unreal's built-in damage pipeline: attackers call `UGameplayStatics::ApplyPointDamage`, `ApplyRadialDamage`, or `ApplyDamage`; receivers own the final response in `TakeDamage`.
- Use `UDamageType` subclasses as immutable metadata holders for semantic damage categories such as bullet, explosive, lightning, or world hazards.
- Keep `UDamageType` subclasses lightweight. They should expose defaults and flags, not store runtime state or become a deep per-weapon class tree.
- Do not branch on damage type leaf class identity when a shared base such as `UQuakeDamageType` can expose the needed fields uniformly.
- Avoid duplicating health/armor resolution logic in both `AQuakeCharacter` and enemy classes. Prefer a shared resolver or `UQuakeHealthComponent` that handles armor absorption, self-damage scaling, knockback inputs, death checks, and related bookkeeping.
- Actor-specific `TakeDamage` overrides should stay thin: gather context from `FDamageEvent`, hand the calculation to shared code, then perform actor-specific reactions such as HUD feedback, AI aggro, pain reactions, or death presentation.
- Not every health change belongs in the damage pipeline. Timed effects like megahealth decay should use explicit health-management code rather than being faked as combat damage.

## Build And Tooling

Build the editor target with:

```powershell
& "C:/Program Files/Epic Games/UE_5.7/Engine/Build/BatchFiles/Build.bat" QuakeEditor Win64 Development -Project="c:/dev/games-unreal/Quake/Quake.uproject" -WaitMutex -FromMsBuild
```

If the build fails with `Unable to build while Live Coding is active`, stop and ask the user to close Unreal Editor or press `Ctrl+Alt+F11`. Do not try to work around Live Coding from the CLI.

When adding new `.h` or `.cpp` files, regenerate project files so IntelliSense and compile commands stay current. Use Unreal's bundled .NET SDK, not the system `dotnet`.

```powershell
$env:UE_DOTNET_DIR = "C:/Program Files/Epic Games/UE_5.7/Engine/Binaries/ThirdParty/DotNet/8.0.412/win-x64"
$env:DOTNET_ROOT = $env:UE_DOTNET_DIR
$env:PATH = "$env:UE_DOTNET_DIR;$env:PATH"
$env:DOTNET_MULTILEVEL_LOOKUP = "0"
$env:DOTNET_ROLL_FORWARD = "LatestMajor"
& "$env:UE_DOTNET_DIR/dotnet.exe" "C:/Program Files/Epic Games/UE_5.7/Engine/Binaries/DotNET/UnrealBuildTool/UnrealBuildTool.dll" -projectfiles -project="C:/dev/games-unreal/Quake/Quake.uproject" -game -engine -VSCode
```

## Unreal-Specific Guidance

- Prefer C++ base classes plus editor-assigned defaults over gameplay implemented in Blueprints.
- Treat VS Code IntelliSense errors as secondary to a real Unreal build. Stale compile commands can produce false squiggles.
- Keep project settings changes in `Config/Default*.ini` under version control.
- Avoid editing generated directories such as `Binaries/`, `DerivedDataCache/`, `Intermediate/`, or `Saved/` unless the task explicitly requires it.

## Implementation Preferences

- Favor data-driven tuning for weapons, enemies, pickups, and other balance values even when behavior stays in C++.
- Keep HUD work in C++/Slate unless the user explicitly changes that direction.
- When adding new gameplay systems, match the naming/style of the existing `AQuake*` and `UQuake*` classes.
- For non-trivial gameplay changes, update `SPEC.md` if the implementation changes the design contract.

## Verification

- After meaningful C++ changes, run a real `Build.bat` compile when feasible.
- If you could not build or test, say so clearly in your final handoff.
