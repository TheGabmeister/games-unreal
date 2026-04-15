# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

FF7 — an Unreal Engine **5.7** C++ project recreating FF7 (1997) gameplay as a vehicle for learning C++ in Unreal. Single primary runtime module (`FF7`) built from the blank template, with `EnhancedInput` as a `.Build.cs` dep and `ModelingToolsEditorMode` enabled editor-only.

**Canonical design docs — read these first:**
- [SPEC.md](SPEC.md) — source of truth for game features + implementation (class names, APIs, data layouts, architectural choices, UI layout sketches, save-serialization strategy, enemy AI schema).
- [TASKS.md](TASKS.md) — phase-by-phase roadmap. Each phase references SPEC sections and lists **Scope / Concepts / Tests / Manual**. Phases are ordered for C++/UE learning progression; don't skip.

When an implementation detail changes, update SPEC first — phases inherit automatically. The code today is still just the module stub at [Source/FF7/FF7.cpp](Source/FF7/FF7.cpp); Phase 0 onward builds everything else. Commits prior to `Add FF7` belong to an unrelated Quake project and should be ignored when reasoning about this codebase.

## Build & run

Engine lives at `C:\Program Files\Epic Games\UE_5.7`. From the project root, using bash (quote the engine path because of the space):

- **Regenerate VS project files** (after adding/removing `.cs`/`.h`/`.cpp` or plugins):
  ```bash
  "/c/Program Files/Epic Games/UE_5.7/Engine/Build/BatchFiles/Build.bat" -projectfiles -project="$PWD/FF7.uproject" -game -rocket -progress
  ```
- **Build editor target** (day-to-day):
  ```bash
  "/c/Program Files/Epic Games/UE_5.7/Engine/Build/BatchFiles/Build.bat" FF7Editor Win64 Development -Project="$PWD/FF7.uproject" -WaitMutex -FromMsBuild
  ```
- **Build packaged game target**: swap `FF7Editor` → `FF7` above.
- **Launch editor**:
  ```bash
  "/c/Program Files/Epic Games/UE_5.7/Engine/Binaries/Win64/UnrealEditor.exe" "$PWD/FF7.uproject"
  ```

Targets: [Source/FF7.Target.cs](Source/FF7.Target.cs) (game), [Source/FF7Editor.Target.cs](Source/FF7Editor.Target.cs) (editor). Module deps: [Source/FF7/FF7.Build.cs](Source/FF7/FF7.Build.cs) — edit when adding engine modules, then regenerate project files.

## Tests

Phase 0 wires the Unreal Automation Framework. Tests run via **Session Frontend** (Window → Developer Tools → Session Frontend → Automation tab), filtered by `FF7.`. Two flavors per SPEC convention:
- `DEFINE_SPEC` — pure-C++ unit tests under `Source/FF7/Tests/`. Fastest feedback, no PIE.
- `AFunctionalTest`-based — actor placed in a test map; runs in PIE; asserts gameplay-level behavior.

Each TASKS phase lists the exact tests that gate its completion.

## Tools

- **Icon pipeline** — [tools/rasterize_icons.sh](tools/rasterize_icons.sh) converts `tools/icons/svg/*.svg` → `tools/icons/png/*.png` via Inkscape CLI (expected at `C:\Program Files\Inkscape\bin\inkscape.exe`; override with `INKSCAPE=/path` env var). Default size 64×64, override with `SIZE=128`. Run manually when icons change; not part of the UE build. Details in [SPEC §3.1](SPEC.md#31-placeholder-icon-pipeline).

## Conventions

- **C++-first.** Blueprints are thin wrappers for asset references and designer tuning only — see SPEC §2.20 for the exact BP inventory.
- **UI is Slate**, not UMG. Rationale: UMG Widget Blueprints are binary `.uasset` files Claude can't edit directly, so the AI-collaborative workflow favors pure-C++ Slate. See SPEC §2.8.
- **Data-driven gameplay.** DataTables and DataAssets hold values; C++ static libraries hold formulas. Separation is what makes golden-value tests possible.

## Harness constraints

- `.claude/settings.json` denies all tool access to `../**` (paths above the project root). Keep work inside `c:\dev\games-unreal\FF7`.
- `AGENTS.md` is intentionally excluded — do not create or reconcile against one. CLAUDE.md is canonical operational guidance; SPEC.md and TASKS.md are the design/roadmap canon.
