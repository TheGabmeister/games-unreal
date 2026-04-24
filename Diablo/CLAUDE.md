# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Unreal Engine 5.7 project recreating **Diablo (1997)** gameplay. All gameplay systems are built from scratch per the milestone roadmap in [SPEC.md](SPEC.md).

Module name: `Diablo`. Engine version pinned to **5.7** (`Diablo.uproject`, `IncludeOrderVersion = Unreal5_7`).

**C++ first, thin BP layer.** All gameplay logic in C++; Blueprints are thin subclasses with **zero event-graph nodes** — asset-reference slots and per-instance property tuning only. The one exception is **AnimBlueprints**, which are authored manually in the editor (state machines + transitions are too fragile to generate programmatically).

## Build Commands

Build the editor target:

```bash
"/c/Program Files/Epic Games/UE_5.7/Engine/Build/BatchFiles/Build.bat" DiabloEditor Win64 Development -Project="c:/dev/games-unreal/Diablo/Diablo.uproject" -WaitMutex -FromMsBuild
```

If the build fails with **"Unable to build while Live Coding is active"**, ask the user to close the Editor or press Ctrl+Alt+F11. Do not work around it.

**C++ changes require editor restart.** Constructor changes (component setup, default values) are not picked up by Live Coding — the user must close and reopen the editor. Runtime logic changes (Tick, BeginPlay) can use Live Coding but it's unreliable.

Regenerate VS Code project files manually after adding new `.h`/`.cpp` files:

```bash
export UE_DOTNET_DIR='/c/Program Files/Epic Games/UE_5.7/Engine/Binaries/ThirdParty/DotNet/8.0.412/win-x64'
export DOTNET_ROOT="$UE_DOTNET_DIR" PATH="$UE_DOTNET_DIR:$PATH" DOTNET_MULTILEVEL_LOOKUP=0 DOTNET_ROLL_FORWARD=LatestMajor
dotnet '/c/Program Files/Epic Games/UE_5.7/Engine/Binaries/DotNET/UnrealBuildTool/UnrealBuildTool.dll' -projectfiles -project='C:/dev/games-unreal/Diablo/Diablo.uproject' -game -engine -VSCode
```

The system has only .NET 10; UnrealBuildTool needs .NET 8. The bundled SDK at the path above is the working version — never invoke a system `dotnet` for UBT.

IntelliSense errors like `cannot open source file "X.h"` are usually false positives from a stale `.vscode/compileCommands_*.json`. The truth is a real `Build.bat` run, not IDE squiggles — regenerate project files before assuming an include is wrong.

## Module Layout

**Runtime module:** [Diablo.Build.cs](Source/Diablo/Diablo.Build.cs). Public dependencies: `EnhancedInput`, `AIModule`, `NavigationSystem`, `StateTreeModule`, `GameplayStateTreeModule`, `UMG`, `Slate`. The module log category is `LogDiablo` ([Diablo.h](Source/Diablo/Diablo.h)).

**Editor plugin:** [Plugins/DiabloEditor/](Plugins/DiabloEditor/) — editor-only plugin with `FDiabloAssetGenerator` (static utility struct) and a Slate toolbar panel accessible via **Tools > Diablo > Diablo Tools**. Generates BP subclasses, maps, input assets, imports FBX/audio, and configures Blueprint CDO defaults. Registered in `.uproject` with `"TargetAllowList": ["Editor"]`.

**Plugins enabled** in `.uproject`: `StateTree`, `GameplayStateTree` (needed for AI in M19), `ModelingToolsEditorMode` (editor only), `DiabloEditor` (editor only).

## Architecture

### Core Classes (M1)

- `ADiabloGameMode` (Abstract) — sets `DefaultPawnClass` and `PlayerControllerClass` to C++ bases; BP subclass `BP_DiabloGameMode` overrides to BP versions
- `ADiabloHero : ACharacter` (Abstract) — isometric camera via `USpringArmComponent` (pitch -45°, yaw 225°, arm 1800, no collision test) + `UCameraComponent` (orthographic, OrthoWidth 2048). Capsule sized to 90 half-height, mesh offset -90 Z to align feet with capsule bottom. Uses `FDiabloStats` (HP 70/70 Warrior defaults). `TakeDamage` override → `Die()` on HP <= 0. `Heal(Amount)` for pickups. `IsDead()` check.
- `ADiabloPlayerController` (Abstract) — Enhanced Input click-to-move, click-to-attack, click-to-pickup. LMB on ground → `SimpleMoveToLocation`; LMB on enemy → stores `TargetEnemy`, walks into `AttackRange`, then calls `Hero->StartAttack()`; LMB on `ADroppedItem` → walks into `PickupRange`, calls `OnPickedUp`. UPROPERTY slots for `ClickAction` and `DefaultMappingContext` assigned via BP. `OnHeroDeath()` → disable input, camera fade to black, 2s timer → `RestartPlayer` at `PlayerStart` → fade in.
- `UDiabloAnimInstance` (Abstract) — exposes `float Speed` from pawn velocity in `NativeUpdateAnimation`. AnimBP `ABP_Warrior` parents to this class (manual editor creation)

### Combat (M2)

- `ADiabloHero::StartAttack()` — plays `AttackMontage` via the DefaultSlot in the AnimBP. Sets `AttackTarget` so the notify knows who to damage. `bIsAttacking` flag prevents re-triggering mid-swing.
- `UAnimNotify_Attack` (DisplayName "Melee Attack Trace") — fires on a montage keyframe. Reads `AttackTarget` from either `ADiabloHero` or `ADiabloEnemy` and applies `TakeDamage` directly (no sphere trace — Diablo 1 uses targeted hit, not physics). `Damage` is configurable on the notify. Guards against dead targets.
- `ADiabloEnemy : ACharacter` (Abstract) — uses `FDiabloStats` for HP. `TakeDamage` override; on death plays `DeathMontage`, disables collision, delays `Destroy()` by montage duration + 2s. Has `StartAttack(Target)`, `AttackMontage`, `AttackTarget` for AI-driven attacks. Capsule explicitly blocks `ECC_Visibility` for cursor click detection.
- `AM_Attack` (AnimMontage) — manually authored in editor from `Warrior_Anim_Attack`. Contains Melee Attack Trace notify at the hit frame plus optional PlaySound notifies for SwordSwing/SwordHit.
- `AM_Death` (AnimMontage) — manually authored in editor from `Warrior_Anim_Death`. `bEnableAutoBlendOut = false` so the death pose holds. Used by both hero and enemy.

### Enemy AI (M3)

- `ADiabloAIController : AAIController` — tick-based state machine with `EAIState` enum (Idle / Chase / Attack / Dead). `FindTarget()` returns the player pawn (returns `nullptr` if hero is dead). Aggro at `AggroRange` (800), attacks at `AttackRange` (200), leashes at `LeashRange` (1500). Uses `MoveToActor` for pathfinding (only issued when `GetMoveStatus() != Moving` to prevent stutter).
- `ADiabloEnemy` sets `AIControllerClass = ADiabloAIController` and `AutoPossessAI = PlacedInWorldOrSpawned` in constructor.

### HP + Death + Respawn (M4)

- `FDiabloStats` ([DiabloStats.h](Source/Diablo/DiabloStats.h)) — USTRUCT with HP/MaxHP/Mana/MaxMana/Str/Mag/Dex/Vit. Used by both `ADiabloHero` and `ADiabloEnemy`. Only HP/MaxHP active in M4; other fields populated in M6.
- **Hero death:** `TakeDamage` → `Die()` → disable movement, play `DeathMontage` → controller `OnHeroDeath()` → `DisableInput`, camera fade to black (0.5s), 2s `FTimerHandle` → `UnPossess` + `Destroy` dead pawn → `AGameModeBase::RestartPlayer` spawns fresh hero at `PlayerStart` with full HP → `EnableInput`, fade in (0.5s).
- **Enemy death:** `TakeDamage` → HP <= 0 → disable collision + movement, play `DeathMontage` → delayed `Destroy()` after montage + 2s corpse linger.
- `ADroppedItem : AActor` ([DroppedItem.h](Source/Diablo/DroppedItem.h)) (Abstract) — `UStaticMeshComponent` root that blocks `ECC_Visibility` for cursor detection. `HealAmount = 50`. `OnPickedUp(Hero)` heals and destroys. BP subclass `BP_HealingPotion`.

### Input

**Enhanced Input only.** No legacy `InputComponent` axis bindings.

- IA assets generated by the DiabloEditor plugin under `Content/Input/Actions/` (`IA_Click` mapped to LMB, `IA_Move`, `IA_Look`). IMC at `Content/Input/IMC_Diablo`.
- **Don't create IA/IMC at runtime via `NewObject<>`** — assign the editor assets to BP UPROPERTY slots.
- `ADiabloPlayerController` holds `TObjectPtr<UInputAction>` and `TObjectPtr<UInputMappingContext>` slots, binds in `SetupInputComponent`.
- No touch controls — desktop-only (Diablo 1 is mouse+keyboard).

### Asset Generator (DiabloEditor Plugin)

`FDiabloAssetGenerator` static methods, callable from the Slate panel:

| Method | Behavior on re-run |
|---|---|
| `GenerateBlueprintSubclasses` | Creates `BP_DiabloGameMode`, `BP_DiabloHero`, `BP_DiabloPlayerController`, `BP_DiabloEnemy`, `BP_HealingPotion` — **skips if exists** |
| `GenerateDefaultMap` | Creates `Lvl_Diablo` with PlayerStart, DirectionalLight, floor plane, NavMeshBoundsVolume, test enemy, healing potion — **always recreates** |
| `GenerateInputAssets` | Creates/updates IA_Click/Move/Look and IMC_Diablo — **updates in place** |
| `ImportWarriorFBX` | Deletes Warrior assets except `ABP_Warrior`, `AM_Attack`, `AM_Death`, and `Warrior_Skeleton`, then reimports `Tools/blender/out/Warrior.fbx`. Reuses existing skeleton so AnimBP/montage references survive — **recreates mesh + animations only** |
| `ImportAttackSFX` | Imports `Tools/audio/out/SwordSwing.wav` and `SwordHit.wav` under `/Game/Audio/SFX/` — **replaces existing** |
| `ImportPotionFBX` | Imports `Tools/blender/out/HealingPotion.fbx` as static mesh under `/Game/Items/Potions/` — **replaces existing** |
| `ConfigureBlueprintDefaults` | Calls all Configure methods below |
| `ConfigureHeroDefaults` | Sets skeletal mesh, anim class, attack montage, death montage on `BP_DiabloHero` |
| `ConfigureControllerDefaults` | Sets ClickAction, DefaultMappingContext on `BP_DiabloPlayerController` |
| `ConfigureGameModeDefaults` | Sets DefaultPawnClass, PlayerControllerClass on `BP_DiabloGameMode` |
| `ConfigureEnemyDefaults` | Sets skeletal mesh, anim class, attack montage, death montage on `BP_DiabloEnemy` |
| `ConfigureDroppedItemDefaults` | Sets static mesh on `BP_HealingPotion` |

**Do not attempt programmatic AnimBP generation.** State machine graph construction via K2 nodes is too fragile (wrong pin names, function reference ordering, MinimalAPI exports). AnimBPs are the one asset type authored manually in the editor.

**Do not use `ObjectTools::ForceDeleteObjects`** to delete existing assets — it crashes the editor by triggering content browser notifications during the operation. Use `IFileManager::Delete` on the `.uasset` files after unloading packages (see `ImportWarriorFBX` for the pattern).

## Asset Pipeline

All assets are generated programmatically — no manual art creation. Scripts live under [Tools/](Tools/), build artifacts go to `Tools/*/out/` (gitignored).

| Asset type | Tool | Invocation |
|---|---|---|
| 3D models + animations | Blender 5.1 Python | `"C:\Program Files\Blender Foundation\Blender 5.1\blender.exe" --background --python <script>.py` |
| Icons / sprites | Inkscape SVG→PNG | `"C:\Program Files\Inkscape\bin\inkscape.exe" --export-type=png --export-dpi=<N> --export-filename=<out> <in>` |
| Audio SFX | Python + numpy + scipy | `python Tools/audio/<script>.py` (16-bit PCM WAV, UE imports directly) |
| BP subclasses, maps, input assets, FBX import, CDO config | DiabloEditor plugin | **Tools > Diablo > Diablo Tools** panel in UE Editor |

### Blender FBX Export Convention

- Build meshes facing +Y in Blender (natural convention), then rotate mesh + armature -90° around Z and apply transform **before** parenting with auto-weights — this makes the character face +X (UE forward) with correct bone weights
- Animation bone rotations use local Y axis `(0, angle, 0)` for forward/back swing (not X, because the skeleton was rotated before parenting)
- `global_scale=1.0` (not 100 — UE's importer handles meter→cm conversion via `apply_unit_scale=True`)
- `axis_forward="-Z"`, `axis_up="Y"`
- Animations: use NLA strips (unmuted) with `bake_anim_use_nla_strips=True`, `bake_anim_use_all_actions=False` — each NLA strip exports as a separate FBX animation stack. NLA strip names determine UE asset names (`Idle` → `Warrior_Anim_Idle`)
- Character meshes are built at real-world scale in Blender (1.8m tall = 180cm in UE)

Animations are keyframed in the same Blender scripts that generate meshes — armature + bone keyframes baked into the FBX.

**Editor-only assets** that require the DiabloEditor plugin or editor interaction:
- **BP subclasses** under `Content/Blueprints/` — every `UCLASS(Abstract)` C++ class needs a BP child to be placed
- **Levels** under `Content/Maps/` — generated by editor tool or hand-authored
- **Input assets** under `Content/Input/` — generated by editor tool
- **AnimBlueprints** — manually authored in editor; parent to C++ `UDiabloAnimInstance` subclass. `ABP_Warrior` has a Locomotion state machine (Idle/Walk based on `Speed`) with a DefaultSlot node for montage overlay.
- **AnimMontages** — `AM_Attack` created manually from `Warrior_Anim_Attack` with Melee Attack Trace and PlaySound notifies. `AM_Death` created manually from `Warrior_Anim_Death` with `bEnableAutoBlendOut = false`. Both set on hero and enemy BPs via `ConfigureHeroDefaults`/`ConfigureEnemyDefaults`.
- **StateTree assets** — needed starting M19; reference C++ task/condition structs

### FBX Reimport Workflow

`ImportWarriorFBX` opens a blank map first (to unload actors referencing the mesh and avoid scene proxy crashes), then deletes all Warrior assets except `Warrior_Skeleton`, `ABP_Warrior`, `AM_Attack`, and `AM_Death`. The FBX importer reuses the existing skeleton (`FbxFactory->ImportUI->Skeleton`), so AnimBP and montage references stay valid. The skeletal mesh, physics asset, and animation sequences are deleted and freshly recreated — this ensures new animations (like Death) are imported alongside existing ones.

**When bone hierarchy changes** (add/remove/rename bones): manually delete `Warrior_Skeleton` in the Content Browser before reimporting. The AnimBP will need to be recreated. This should be rare — the bone hierarchy is defined by `add_bone` calls in `warrior_mesh.py` and is stable.

## Coding principles

- **KISS** — simplest thing that works. No clever patterns where a plain `if` does the job.
- **YAGNI** — don't build for hypothetical needs. No interfaces with one implementation, no config knobs with one value.
- **DRY** — remove real duplication, not shape-similar code. Wrong abstraction costs more than repetition.
- **Phased introduction** — each system enters at the phase where gameplay first needs it.
- **Unreal Engine C++ best practices** — use `UPROPERTY`/`UFUNCTION` macros on all exposed members; prefer `TObjectPtr<>` over raw pointers for UObject members; use `CreateDefaultSubobject` for components in constructors; mark base classes `Abstract` when they should never be spawned directly; use `PURE_VIRTUAL` macro (not C++ `= 0`) for abstract virtuals; prefer `TArray`/`TMap`/`TSet` over STL containers; use `FName`/`FString`/`FText` appropriately (FName for identifiers, FString for manipulation, FText for user-facing display); use `UE_LOG(LogDiablo, ...)` for logging; follow UE naming conventions (`A` prefix for Actors, `U` for UObjects, `F` for structs/value types, `E` for enums, `I` for interfaces).

When in doubt, lean KISS over DRY.

## Design Reference

The full Diablo (1997) gameplay reference, milestone roadmap, and verification criteria are in [SPEC.md](SPEC.md). Consult it before making design decisions.

**Combat model:** Diablo 1 uses targeted hit (not physics traces). Click enemy → walk into range → play attack montage → apply damage directly to target. To-hit% roll comes in M6 with stats.

## Auto-Memory

The user has a persistent file-based memory system at `C:\Users\Admin\.claude\projects\c--dev-games-unreal\memory\`. Save user/feedback/project memories there per the auto-memory protocol; check existing memories before recommending design choices. Key memories: input system convention (Editor-authored IA/IMC, no runtime `NewObject`), prefer minimal fixes over defensive scaffolding, UE projects are C++ learning vehicles (explain UE/C++ concepts after completing a phase), all animations via Blender Python, true isometric orthographic camera.
