# AGENTS.md

Guidance for AI coding agents working in this Unreal Engine project.

## Project Shape

This repo is an Unreal Engine 5.7 recreation of Diablo (1997), focused on gameplay fidelity over graphics. Treat [SPEC.md](SPEC.md) as the design contract and milestone roadmap. [CLAUDE.md](CLAUDE.md) contains deeper implementation notes and should be checked before changing existing systems.

Module name: `Diablo`.

Engine target: Unreal Engine 5.7.

Project file: [Diablo.uproject](Diablo.uproject).

Runtime source lives in [Source/Diablo](Source/Diablo). The editor-only tooling plugin lives in [Plugins/DiabloEditor](Plugins/DiabloEditor).

## Development Principles

- Keep gameplay logic in C++.
- Keep Blueprints thin: asset references, default property tuning, and manually authored AnimBlueprint state machines only.
- Prefer the existing milestone architecture in `SPEC.md` over introducing new abstractions early.
- Do not replace existing simple enum/data-driven flows with polymorphic UObject systems unless the current milestone genuinely needs it.
- Preserve the Diablo 1 control feel: click-to-move, click-to-attack, RMB spell casting, fixed orthographic isometric camera.
- Generated/procedural assets are preferred over imported third-party art. Use the existing `Tools/` pipelines for Blender, SVG, and audio assets.

## Build And Verification

Build the editor target with:

```bash
"/c/Program Files/Epic Games/UE_5.7/Engine/Build/BatchFiles/Build.bat" DiabloEditor Win64 Development -Project="c:/dev/games-unreal/Diablo/Diablo.uproject" -WaitMutex -FromMsBuild
```

If Unreal reports that Live Coding is active, ask the user to close the editor or press Ctrl+Alt+F11. Do not work around that state.

After adding new `.h` or `.cpp` files, regenerate project files with UnrealBuildTool using the bundled .NET SDK, not the system `dotnet`. See `CLAUDE.md` for the exact command.

Each milestone should end with a manual play-test against the criteria in `SPEC.md`. For code-only changes, at minimum run the editor target build when practical.

## Source Conventions

- Follow Unreal naming conventions: `A` actors, `U` objects/components/widgets, `F` structs, `E` enums.
- Use `UPROPERTY` and `UFUNCTION` where Unreal reflection, serialization, asset assignment, or editor exposure is required.
- Prefer `TObjectPtr<>` for UObject references in reflected fields.
- Use `LogDiablo` from [Source/Diablo/Diablo.h](Source/Diablo/Diablo.h) for project logging.
- Keep widget layout logic in C++ for this project. Widget Blueprints are thin subclasses.
- For UI hit testing, existing panels use tick-space geometry; match nearby code before changing coordinate-space assumptions.

## Existing Systems To Respect

- `ADiabloHero` owns core hero stats, camera, combat, spells, inventory, XP, and persistence hooks.
- `ADiabloPlayerController` owns input routing and UI panel lifetime.
- `UInventoryComponent` owns the 10x4 inventory grid, equipment, gold, item use, buy/sell flows, and inventory change delegate.
- `FItemInstance` is a struct backed by `UItemDefinition` data assets. Do not force items into actor/UObject polymorphism just to add stat behavior.
- `USpellDefinition` data assets drive spell casting. Projectile spells use `ASpellProjectile` subclasses; instant spells are dispatched through definition fields.
- `UDiabloGameInstance` persists hero state across `OpenLevel`; `UDiabloSaveGame` persists it to disk.
- `IInteractable` is the shared interaction path for stairs and NPCs.
- The editor plugin generates maps, BP subclasses, input assets, item/spell/shop data, and imports generated assets.

## Asset Pipeline

Use existing pipeline folders:

- Blender scripts: [Tools/blender](Tools/blender)
- SVG/icon scripts: [Tools/svg](Tools/svg)
- Audio scripts: [Tools/audio](Tools/audio)

Procedural outputs should flow into Unreal assets through the editor tooling where possible. Avoid adding binary assets by hand unless the user explicitly asks.

## Git And Generated Files

- Do not edit or commit `Binaries/`, `DerivedDataCache/`, `Intermediate/`, or `Saved/` unless explicitly requested.
- Avoid unrelated formatting churn.
- The worktree may contain user changes. Do not revert files you did not intentionally change.
- Do not remove generated assets or maps unless the task specifically calls for regeneration or cleanup.

## When In Doubt

Read `SPEC.md` first for intended gameplay and milestone scope. Read `CLAUDE.md` next for current implementation details. Then inspect the relevant C++ files before editing.
