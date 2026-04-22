# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

FF7 — an Unreal Engine **5.7** C++ project recreating FF7 (1997) gameplay as a vehicle for learning C++ in Unreal. Single runtime module (`FF7`) built from the blank template; `ModelingToolsEditorMode` enabled editor-only.

**Canonical design docs — read these first:**
- [SPEC.md](SPEC.md) — source of truth for game features + implementation (class names, APIs, data layouts, architectural choices, UI layout sketches, save-serialization strategy, enemy AI schema).
- [TASKS.md](TASKS.md) — phase-by-phase roadmap. Each phase references SPEC sections and lists **Scope / Concepts / Tests / Manual**. Phases are ordered for C++/UE learning progression; don't skip.

When an implementation detail changes, update SPEC first — phases inherit automatically. Commits prior to `Add FF7` belong to an unrelated Quake project and should be ignored when reasoning about this codebase.

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

Targets: [Source/FF7.Target.cs](Source/FF7.Target.cs) (game), [Source/FF7Editor.Target.cs](Source/FF7Editor.Target.cs) (editor). Module deps: [Source/FF7/FF7.Build.cs](Source/FF7/FF7.Build.cs) — edit when adding engine modules, then regenerate. The module uses a flat layout (no `Public/Private` split) and `PublicIncludePaths.Add(ModuleDirectory)` so files in subfolders (e.g. `Tests/`, `UI/`) can include siblings without `../` paths.

## Tests

Two flavors per SPEC convention:
- `DEFINE_SPEC` — pure-C++ unit tests under [Source/FF7/Tests/](Source/FF7/Tests/). Fastest feedback, no PIE.
- `AFunctionalTest`-based — actor placed in a test map; runs in PIE; asserts gameplay-level behavior.

**Running headlessly** (no editor window, fastest iteration):
```bash
"/c/Program Files/Epic Games/UE_5.7/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "$PWD/FF7.uproject" -ExecCmds="Automation RunTests FF7+;Quit" -unattended -NullRHI
```
- `FF7+` matches all `FF7.*` tests. Narrow with `FF7.Interact+`, `FF7.Party.SwapActive+`, etc.
- Output goes to [Saved/Logs/FF7.log](Saved/Logs/FF7.log); grep for `Test Completed`, `Result={Fail}`, `Error: Expected`.

**Running in-editor:** Window → Developer Tools → Session Frontend → Automation tab → filter `FF7.`.

Each TASKS phase lists the exact tests that gate its completion.

### Spec-test gotcha — interface / UFUNCTION dispatch needs a primed world

`DEFINE_SPEC` tests that exercise anything going through `ProcessEvent` (`UFUNCTION(Exec)`, `Execute_*` on a UINTERFACE, BP-callable functions, dynamic delegates) **must** call `InitializeActorsForPlay(FURL())` + `BeginPlay()` after `UWorld::CreateWorld`. Without that priming, `FindFunction` still returns a non-null `UFunction`, but invocation silently no-ops.

See [Source/FF7/Tests/FF7InteractSpec.cpp](Source/FF7/Tests/FF7InteractSpec.cpp) `BeforeEach` for the canonical recipe. Pure C++ calls (no reflection) work without this — only `ProcessEvent`-driven paths need it.

## Tools (outside UE build)

- **Mesh pipeline** — [tools/generate_meshes.py](tools/generate_meshes.py) emits OBJ placeholder meshes into `tools/meshes/characters/` from parameterized recipes (`HumanoidRecipe` for party members; dedicated builders for Red XIII quadruped and Cait Sith). Run `python tools/generate_meshes.py` to regenerate all, or pass recipe names to generate specific ones. Import manually into `Content/Placeholder/Characters/` with *Auto Generate Collision*. Details in [SPEC §3.2](SPEC.md#32-placeholder-3d-model-pipeline).
- **Icon pipeline** — [tools/rasterize_icons.sh](tools/rasterize_icons.sh) converts `tools/icons/svg/*.svg` → `tools/icons/png/*.png` via Inkscape CLI (expected at `C:\Program Files\Inkscape\bin\inkscape.exe`; override with `INKSCAPE=/path` env var). Default size 64×64, override with `SIZE=128`. Details in [SPEC §3.1](SPEC.md#31-placeholder-icon-pipeline).

## Architecture (as of Phase 3)

Implemented so far; later phases extend rather than replace:

- **Character rig** — `AFF7CharacterBase` (capsule root → `USceneComponent` visual pivot → `UStaticMeshComponent`; resolves a `TSoftObjectPtr<UStaticMesh>` on `BeginPlay`, falls back to engine cube). `AFF7PlayerPawn` adds `USpringArmComponent` (fixed top-down, no inherit) + `UCameraComponent` + `UFloatingPawnMovement`.
- **Input** — `AFF7PlayerController` holds `TSoftObjectPtr<UInputMappingContext>` + one `TSoftObjectPtr<UInputAction>` per action (`IA_Move`, `IA_Interact`, `IA_MenuToggle`, `IA_Escape`). IMC/IA are **Editor-authored assets** assigned on `BP_FF7PlayerController`; C++ pushes the IMC on `OnPossess` and binds actions in `SetupInputComponent`. `IA_Move` → `ETriggerEvent::Triggered` (per-frame axis). Discrete press-once actions (Interact, MenuToggle, Escape) → `ETriggerEvent::Started`.
- **Interaction / dialogue** — `IFF7Interactable` (UINTERFACE two-class pattern). `AFF7NPCActor` has a `USphereComponent` interact volume that blocks the custom `Interact` trace channel. On `IA_Interact`: controller forward-line-traces for an interactable, falls back to overlap, dispatches via `IFF7Interactable::Execute_Interact(Target, this)`. `AFF7NPCActor::Interact_Implementation` calls `PlayerController->StartDialogue(Table, StartRowId)`. Controller owns dialogue state + popup lifetime.
- **Slate UI** — Phase 2 adds the first widget, `SFF7DialoguePopup` ([Source/FF7/UI/](Source/FF7/UI/)). Attaches via `GEngine->GameViewport->AddViewportWidgetContent`; text binds via `TAttribute<FText>` lambdas reading cached state on the controller. Phase 6 will register a `FSlateStyleSet` in `FFF7Module::StartupModule` (placeholder in [Source/FF7/FF7.cpp](Source/FF7/FF7.cpp)).
- **Persistent state** — `UFF7GameInstance` owns `Roster` (9 `FPartyMember`), `ActivePartyIndices` (3), `Gil`, `WorldFlags`, `Config`, `PlayTimeSeconds`. Populates `Roster` in `Init` from `DefaultRoster` (9 `TSoftObjectPtr<UFF7CharacterDefinition>` slots assigned on `BP_FF7GameInstance`). `OnGilChanged` + `OnConfigChanged` dynamic multicast delegates. `UFF7CharacterDefinition : UPrimaryDataAsset` carries `CharacterId`, `DisplayName`, `PlaceholderMesh`, `DefaultStats`.

### Custom collision channels

Project-defined channels live in **two places that must stay in lockstep**:
1. [Config/DefaultEngine.ini](Config/DefaultEngine.ini) under `[/Script/Engine.CollisionProfile]` — registers the friendly name with the engine/editor.
2. [Source/FF7/FF7CollisionChannels.h](Source/FF7/FF7CollisionChannels.h) — C++ `constexpr` alias in `namespace FF7` (e.g. `FF7::ECC_Interact = ECC_GameTraceChannel1`).

Order of constants in the header MUST match order of `+DefaultChannelResponses` in the ini — never reorder. Always reference the alias from C++, never the raw `ECC_GameTraceChannelN`.

### Exec commands — no dots in names

`UFUNCTION(Exec)` function names must be valid C++ identifiers; they can't contain dots. The SPEC lists commands as `FF7.Party.Dump`; the C++ form is `FF7PartyDump`. Users type the C++ name (case-insensitive) in the console. Dotted names require `IConsoleManager::RegisterConsoleCommand` — deferred unless needed.

## Conventions

- **C++-first.** Blueprints are thin wrappers for asset references and designer tuning only — see SPEC §2.20 for the exact BP inventory.
- **UI is Slate**, not UMG. Rationale: UMG Widget Blueprints are binary `.uasset` files Claude can't edit directly, so the AI-collaborative workflow favors pure-C++ Slate. See SPEC §2.8.
- **Data-driven gameplay.** DataTables and DataAssets hold values; C++ static libraries hold formulas. Separation is what makes golden-value tests possible.
- **Soft object refs over hard refs for swappable art.** `TSoftObjectPtr<UStaticMesh> PlaceholderMesh` lives on character definitions / enemy rows, not hard-referenced on component defaults. Lets one pawn class serve many characters and keeps the skeletal-mesh swap seam trivial (see SPEC §2.2, §3.2).

## Harness constraints

- `.claude/settings.json` denies all tool access to `../**` (paths above the project root). Keep work inside `c:\dev\games-unreal\FF7`.
- `AGENTS.md` is intentionally excluded — do not create or reconcile against one. CLAUDE.md is canonical operational guidance; SPEC.md and TASKS.md are the design/roadmap canon.
