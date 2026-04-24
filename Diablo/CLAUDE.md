# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Unreal Engine 5.7 project intended to recreate Diablo gameplay. **Currently the unmodified UE 5.7 Third Person template** (commit `eea2da6` "Add Diablo base project") — no Diablo-specific code, design docs, or game systems exist yet. Treat existing `Diablo*` / `Combat*` / `Platforming*` / `SideScrolling*` classes as **template scaffolding to learn from and largely replace**, not as load-bearing infrastructure.

Module name: `Diablo`. Engine version pinned to **5.7** (`Diablo.uproject`, `IncludeOrderVersion = Unreal5_7`).

**C++ first, thin BP layer.** All gameplay logic in C++; Blueprints are thin subclasses with **zero event-graph nodes** — asset-reference slots and per-instance property tuning only. The template ships some `BlueprintImplementableEvent` hooks (`BP_ToggleCamera`, `SetJumpTrailState`, `DealtDamage`, `ReceivedDamage`) for cosmetic FX — those are the legitimate exception.

## Build Commands

Build the editor target:

```bash
"/c/Program Files/Epic Games/UE_5.7/Engine/Build/BatchFiles/Build.bat" DiabloEditor Win64 Development -Project="c:/dev/games-unreal/Diablo/Diablo.uproject" -WaitMutex -FromMsBuild
```

If the build fails with **"Unable to build while Live Coding is active"**, ask the user to close the Editor or press Ctrl+Alt+F11. Do not work around it.

Regenerate VS Code project files manually after adding new `.h`/`.cpp` files:

```bash
export UE_DOTNET_DIR='/c/Program Files/Epic Games/UE_5.7/Engine/Binaries/ThirdParty/DotNet/8.0.412/win-x64'
export DOTNET_ROOT="$UE_DOTNET_DIR" PATH="$UE_DOTNET_DIR:$PATH" DOTNET_MULTILEVEL_LOOKUP=0 DOTNET_ROLL_FORWARD=LatestMajor
dotnet '/c/Program Files/Epic Games/UE_5.7/Engine/Binaries/DotNET/UnrealBuildTool/UnrealBuildTool.dll' -projectfiles -project='C:/dev/games-unreal/Diablo/Diablo.uproject' -game -engine -VSCode
```

The system has only .NET 10; UnrealBuildTool needs .NET 8. The bundled SDK at the path above is the working version — never invoke a system `dotnet` for UBT.

IntelliSense errors like `cannot open source file "X.h"` are usually false positives from a stale `.vscode/compileCommands_*.json`. The truth is a real `Build.bat` run, not IDE squiggles — regenerate project files before assuming an include is wrong.

## Module Layout

[Diablo.Build.cs](Source/Diablo/Diablo.Build.cs) declares `PublicIncludePaths` for **every variant subfolder** (`Variant_Combat/AI`, `Variant_SideScrolling/UI`, etc.), so headers across variants can be included by short name without relative paths. Plugins enabled in the `.uproject`: `StateTree`, `GameplayStateTree`, `ModelingToolsEditorMode` (editor only). Public dependencies include `EnhancedInput`, `AIModule`, `StateTreeModule`, `GameplayStateTreeModule`, `UMG`, `Slate`.

## Architecture: Three Template Variants

The template ships three parallel, **independent** sets of GameMode/Character/PlayerController under [Source/Diablo/](Source/Diablo/):

- **Root `ADiabloCharacter` / `ADiabloPlayerController` / `ADiabloGameMode`** — minimal third-person base (move/look/jump only). The umbrella module log macro is `LogDiablo` ([Diablo.h](Source/Diablo/Diablo.h)).
- **`Variant_Combat/`** — third-person melee with combo strings, charged attacks, damage/death/respawn, AI enemies driven by **StateTree** ([CombatStateTreeUtility.h](Source/Diablo/Variant_Combat/AI/CombatStateTreeUtility.h)) plus **EQS** contexts (`EnvQueryContext_Player`, `EnvQueryContext_Danger`), enemy spawners, hazard volumes (lava floor), checkpoints, and a UMG life bar. Closest variant to a Diablo-style action game.
- **`Variant_Platforming/`** — third-person platformer with double-jump / wall-jump / dash / coyote-time.
- **`Variant_SideScrolling/`** — 2D-axis camera with custom `SideScrollingCameraManager`, soft platforms, jump pads, moving platforms, NPC interaction, and its own StateTree utility.

**Variants don't share base classes** beyond `ACharacter` / `APlayerController` / `AGameModeBase`. Each variant has its own `*Character` and reimplements input/movement; the umbrella `ADiabloCharacter` is just a template entry point. Picking up a Diablo build will most likely mean **adopting one variant (probably Combat) and deleting the others**, including their `PublicIncludePaths` entries in [Diablo.Build.cs](Source/Diablo/Diablo.Build.cs).

## Architecture: Combat Variant Patterns

Worth keeping when building the Diablo game on top of this template:

- **Interface-driven combat.** `ICombatAttacker` ([CombatAttacker.h](Source/Diablo/Variant_Combat/Interfaces/CombatAttacker.h)) and `ICombatDamageable` ([CombatDamageable.h](Source/Diablo/Variant_Combat/Interfaces/CombatDamageable.h)) are pure C++ virtual interfaces (`UINTERFACE(MinimalAPI, NotBlueprintable)`) — players and enemies both implement both. `ICombatActivatable` separates "trigger source → spawner/checkpoint" wiring. **Note:** `b9e98e2 "Fix interface bug"` is recent — interface signatures are likely the source-of-truth shape; preserve it.
- **AnimNotify-driven attack timing.** Attack montages emit `AnimNotify_DoAttackTrace`, `AnimNotify_CheckCombo`, `AnimNotify_CheckChargedAttack` — these call back into the implementing `ICombatAttacker`. Damage windows live in **animation data**, not C++ timers.
- **Player vs. AI symmetry.** `ACombatCharacter` (player) and `ACombatEnemy` (AI) duplicate the attack/damage UPROPERTY block (`MeleeTraceDistance`, `MeleeDamage`, `ComboAttackMontage`, etc.). If you need to share behavior, factor a base class — but the template's deliberate split keeps player input handlers separate from AI-driven `DoAIComboAttack` / `DoAIChargedAttack`. Review before refactoring.
- **AI = AIController + StateTree (no Behavior Trees).** `ACombatAIController` runs `UStateTreeComponent`. Custom tasks/conditions live as `USTRUCT`s in `CombatStateTreeUtility.h` (`FStateTreeComboAttackTask`, `FStateTreeChargedAttackTask`, `FStateTreeWaitForLandingTask`, `FStateTreeFaceActorTask`, `FStateTreeIsInDangerCondition`, etc.). This is the StateTree pattern — `EnterState` / `ExitState` / `Tick` virtuals, `FInstanceDataType` typedef, `STATETREE_POD_INSTANCEDATA` for POD data, `#if WITH_EDITOR GetDescription` for editor display. Match this shape when adding tasks.
- **Enemy lifecycle.** `ACombatEnemy` exposes `FOnEnemyDied` (BlueprintAssignable) plus internal `FOnEnemyAttackCompleted` / `FOnEnemyLanded` delegates that StateTree tasks bind lambdas to. `ACombatEnemySpawner` subscribes to `OnEnemyDied` to respawn or fire downstream `ICombatActivatable`s.
- **Damage flow.** Override `AActor::TakeDamage`, route to `ICombatDamageable::ApplyDamage` (which handles HP + knockback impulse + ragdoll + `HandleDeath`). `NotifyDanger` is the pre-impact warning that lets enemies dodge — issued by the attacker via a sphere trace before swinging.

## Architecture: Input

**Enhanced Input only.** No legacy `InputComponent` axis bindings.

- IA assets live under [Content/Input/Actions/](Content/Input/Actions/), IMC assets at [Content/Input/IMC_Default.uasset](Content/Input/IMC_Default.uasset) and `IMC_MouseLook.uasset`.
- `ADiabloPlayerController` exposes `DefaultMappingContexts` and `MobileExcludedMappingContexts` as `TArray<UInputMappingContext*>` UPROPERTYs — assigned in `BP_DiabloPlayerController` (and equivalently in `BP_CombatPlayerController` etc.). PC adds them to `UEnhancedInputLocalPlayerSubsystem` in `SetupInputComponent`.
- Each Character class holds its own `UInputAction*` UPROPERTY slots (Jump/Move/Look/MouseLook/ComboAttack/...) and binds in `SetupPlayerInputComponent`. **Don't create IA/IMC at runtime via `NewObject<>`** — assign the editor assets to the BP slots. (This matches the user's standing feedback memory on input — see [memory/feedback_input_system.md](../../../Users/Admin/.claude/projects/c--dev-games-unreal/memory/feedback_input_system.md).)
- **Touch controls** are first-class: `bForceTouchControls` config bool + `MobileControlsWidgetClass` UMG widget + `SVirtualJoystick::ShouldDisplayTouchInterface()` auto-detection. Each Character also exposes `DoMove` / `DoLook` / `DoJumpStart` etc. as `BlueprintCallable` so touch widgets can drive the same code paths as keyboard input. Preserve this dual-entry pattern when rewriting input.

## Architecture: Editor-Authored Pieces

These can't be created from C++ alone:

- **Levels** under `Content/<Variant>/Lvl_*.umap` (e.g., `Lvl_Combat.umap`, `Lvl_ThirdPerson.umap`). Set the active map in **Project Settings → Maps & Modes**.
- **BP framework subclasses** under `Content/<Variant>/Blueprints/` — every `UCLASS(abstract)` C++ class needs a BP child to be placed/used. The C++ classes are marked `abstract` deliberately; never spawn them directly.
- **Input assets** (IA/IMC) — see Input section above.
- **AnimMontages + AnimNotify placements** — combo timing, charge loop section names, attack trace bones all reference montage section names by `FName`. Names must match the C++-side `ComboSectionNames` / `ChargeLoopSection` / `ChargeAttackSection` UPROPERTYs.
- **StateTree assets** under `Content/Variant_Combat/AI/` — reference the C++ task/condition structs declared in `CombatStateTreeUtility.h`.
- **DataTables / Project Settings** — none yet; the template doesn't centralize stats. 

## Coding principles

- **KISS** — simplest thing that works. No clever patterns where a plain `if` does the job.
- **YAGNI** — don't build for hypothetical needs. No interfaces with one implementation, no config knobs with one value.
- **DRY** — remove real duplication, not shape-similar code. Wrong abstraction costs more than repetition.
- **Phased introduction** — each system enters at the phase where gameplay first needs it.

When in doubt, lean KISS over DRY.

## Auto-Memory

The user has a persistent file-based memory system at `C:\Users\Admin\.claude\projects\c--dev-games-unreal\memory\`. Save user/feedback/project memories there per the auto-memory protocol; check existing memories before recommending design choices. Existing relevant memories: input system convention (Editor-authored IA/IMC, no runtime `NewObject`), prefer minimal fixes over defensive scaffolding, UE projects are C++ learning vehicles (explain UE/C++ concepts after completing a phase).
