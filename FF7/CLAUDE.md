# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

FF7 — an Unreal Engine **5.7** C++ project. The tree is a fresh scaffold: a single primary game module (`FF7`) built from the blank template plus `EnhancedInput`, with `ModelingToolsEditorMode` enabled editor-only. `DefaultEngine.ini` opens `/Engine/Maps/Templates/OpenWorld` on launch — no project-specific maps, gameplay code, or content exist yet beyond the module stub at [Source/FF7/FF7.cpp](Source/FF7/FF7.cpp).

Earlier commits in `git log` (pre-`Add FF7`) belong to an unrelated project (Quake) and should be ignored when reasoning about this codebase.

## Build & run

Engine lives at `C:\Program Files\Epic Games\UE_5.7`. From the project root, using bash (quote the engine path because of the space):

- **Regenerate VS project files** (after adding/removing `.cs`/`.h`/`.cpp` or plugins):
  ```bash
  "/c/Program Files/Epic Games/UE_5.7/Engine/Build/BatchFiles/Build.bat" -projectfiles -project="$PWD/FF7.uproject" -game -rocket -progress
  ```
- **Build editor target** (what you run day-to-day):
  ```bash
  "/c/Program Files/Epic Games/UE_5.7/Engine/Build/BatchFiles/Build.bat" FF7Editor Win64 Development -Project="$PWD/FF7.uproject" -WaitMutex -FromMsBuild
  ```
- **Build packaged game target**: swap `FF7Editor` → `FF7` above.
- **Launch editor**:
  ```bash
  "/c/Program Files/Epic Games/UE_5.7/Engine/Binaries/Win64/UnrealEditor.exe" "$PWD/FF7.uproject"
  ```

Targets are defined in [Source/FF7.Target.cs](Source/FF7.Target.cs) (game) and [Source/FF7Editor.Target.cs](Source/FF7Editor.Target.cs) (editor). Module deps live in [Source/FF7/FF7.Build.cs](Source/FF7/FF7.Build.cs) — edit this when adding engine modules (e.g. `UMG`, `AIModule`), then regenerate project files.

There is no test framework wired up yet.

## Harness constraints

- `.claude/settings.json` denies all tool access to `../**` (paths above the project root). Keep work inside `c:\dev\games-unreal\FF7`.
- `AGENTS.md` is intentionally excluded from this project — do not create or reconcile against one. CLAUDE.md is canonical.
