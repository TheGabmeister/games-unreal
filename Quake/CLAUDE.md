# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Unreal Engine 5.7 single-player FPS recreating original Quake gameplay with primitive shapes. The full design is in [SPEC.md](SPEC.md) — read it before making non-trivial gameplay changes. Module name: `Quake`. The current scope is the v1 milestone defined in `SPEC.md` section 11.

**SPEC.md is the design source of truth.** When the SPEC and this file disagree about gameplay or data ownership, the SPEC wins — update CLAUDE.md to match. CLAUDE.md is a working-notes file for build/tooling/conventions, not a design doc.

**Implementation phases.** v1 is built in 16 phases (Phase 0–15) defined in [SPEC.md section 11.5](SPEC.md). Each phase has explicit exit criteria — automated tests + manual verification — that must pass before the next phase begins. Before adding gameplay code, find the current phase and stay within its scope. **Phase 1 (the strafe-jumping CMC) is the highest-risk item**; do not build combat code on top of an unverified CMC.

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

## Architecture: State Ownership

Data lives in a specific place depending on its lifecycle. Putting it in the wrong place breaks respawn, level transitions, or the HUD. From SPEC.md's ownership summary:

- **`UQuakeGameInstance`** — inventory (weapons owned, ammo, armor), level-entry snapshot, save-game reference, player profile, difficulty. Survives `OpenLevel` and Character respawn. **Note:** keys are *not* here despite being colloquially "inventory" — see PlayerState below.
- **`AQuakePlayerState`** — current-level stats (kills, secrets, deaths, time), active powerups (`TArray<FQuakeActivePowerup>`), and keys held. **Auto-cleared on `OpenLevel` only** — UE preserves PlayerState across pawn death/respawn, so the death-restart path (SPEC section 6.4) calls `ClearPerLifeState()` explicitly to empty powerups and keys. Cumulative stats (kills, secrets, deaths, time) persist across the level attempt. Don't assume "PlayerState resets on death" without that call. Keys live here because their lifecycle matches powerups, not weapons/ammo/armor.
- **`AQuakeCharacter`** — live health, currently-equipped weapon actor. Tied to the body; destroyed on death and respawn.
- **`AQuakeGameMode`** — level totals (`KillsTotal`, `SecretsTotal`), spawn rules, win conditions. Server-authoritative (prepared for future multiplayer).
- **`AQuakeHUD`** — reads from all of the above; caches weak pointers in the Slate widget and reads them on paint.

Do not put inventory on the Character. Do not put per-level stats on the Character or the GameMode. Do not put settings (sensitivity, volume) in the PlayerState.

## Architecture: Damage Pipeline

All damage flows through UE's built-in pipeline:

1. Attackers call `UGameplayStatics::ApplyPointDamage` / `ApplyRadialDamage` / `ApplyDamage`.
2. Targets override `AActor::TakeDamage` to actually decrement health, apply armor, knockback, etc.
3. **No code outside `TakeDamage` mutates health directly.** If you see `Target->Health -= X` anywhere else, it's a bug.

Damage metadata (self-damage scale, armor bypass, pain suppression, knockback strength) lives on `UDamageType` subclasses defined in SPEC section 1.5 — not on the attacker, not on the target. All Quake damage types inherit from a single abstract base `UQuakeDamageType` that owns every Quake-specific `UPROPERTY`; leaf subclasses add no new properties and only override defaults in their constructor. `TakeDamage` reads these via the CDO of the shared base — never branch on leaf class identity. Adding a new damage source means adding a ~10-line subclass that overrides defaults in its constructor and calling the appropriate `ApplyXDamage` helper with its `StaticClass()`. Splash radius lives on the weapon, not the damage type.

Attribution uses UE's built-in `EventInstigator` (the controller) and `DamageCauser` (the projectile/weapon). Self-damage detection, infighting, and kill credit all derive from `EventInstigator` — no custom event bus needed.

## Architecture: AI Split

AI is split between the body and the brain per standard UE convention:

- **`AQuakeEnemyBase : public ACharacter`** (the pawn / body) — capsule, mesh primitives, movement, health, `TakeDamage` override. Exposes action methods like `MoveToTarget`, `FireAtTarget`, `PlayPainReaction`, `PlayDeathReaction`. **No decision-making code lives on the pawn.**
- **`AQuakeEnemyAIController : public AAIController`** (the brain) — state machine (`Idle → Alert → Chase → Attack → Pain → Dead`), target tracking, owns a `UAIPerceptionComponent` with sight (`UAISenseConfig_Sight`) and hearing (`UAISenseConfig_Hearing`, with `bUseLoSHearing = false` for Quake-style hearing through walls). Calls the pawn's action methods to drive behavior.

Per-enemy behavior variations (Fiend leap, Ogre grenade arc, Zombie revive) live on per-enemy AIController subclasses, not on pawn subclasses. This is what makes the AI debugger (`'` key in PIE) work and keeps "what I am" separate from "what I'm doing."

## Architecture: Activation Chains

Buttons, triggers, doors, spawn points, and the level exit communicate via the `IQuakeActivatable` interface — a **pure C++ virtual** `Activate(AActor* Instigator)`, **not** a `BlueprintNativeEvent`. Do not write `Activate_Implementation` (that suffix is only valid for `BlueprintNativeEvent` methods, and using it with this interface will not compile). Sources hold typed `TArray<TObjectPtr<AActor>>` slots filled per-instance in the editor via the actor picker (eyedropper UX). **No string-name targeting** — Quake's original `targetname`/`target` lookup is intentionally not used because it loses refactor traceability and editor pick-list filtering. See SPEC sections 5.5 and 5.6.

## Risk Note: Strafe-Jumping CMC

The custom `UQuakeCharacterMovementComponent` is **the single biggest risk in the project**. Quake-style strafe jumping requires overriding `PhysFalling` (or `CalcVelocity`) to implement the Quake air-acceleration formula: clamp the **dot product** of current velocity and wishdir, not the velocity magnitude. Stock UE `UCharacterMovementComponent` clamps air velocity to `MaxWalkSpeed`, which breaks strafe jumping fundamentally.

If asked to tune movement:

- Do **not** rewrite `PhysFalling` without first understanding the current formula and testing circle-strafe speed gain on a reference map.
- Feel is binary: either the dot-product clamp is right and the player can gain speed by air-strafing, or the whole game stops feeling like Quake. There is no "70% strafe-jumping."
- Build a movement sandbox (flat plane, ramps, gap) first. Do not integrate CMC work with other systems until the sandbox passes the feel test.

Multiplayer would additionally require `FSavedMove_Character` / `FNetworkPredictionData_Client_Character` overrides for client-side prediction. This is famously the part where "I added Quake movement to UE" projects break, and it's why this project is single-player only (SPEC constraint).

## Architecture: Input Configuration

Enhanced Input uses Editor-authored assets (Input Actions and Input Mapping Context), assigned to `UPROPERTY(EditDefaultsOnly)` slots on `AQuakePlayerController` via a Blueprint subclass (`BP_QuakePlayerController`). The PlayerController adds the mapping context to the Enhanced Input subsystem in `BeginPlay`. Pawns access actions via `GetController<AQuakePlayerController>()->MoveAction` etc. and bind handlers in `SetupPlayerInputComponent`.

When adding a new input action, add a new `UPROPERTY(EditDefaultsOnly)` slot on `AQuakePlayerController`, create the IA asset in the Editor, map it in IMC_Default, then assign it in the BP subclass. Do not create IA/IMC objects at runtime with `NewObject<>()` — that approach was tried and reverted.

## Architecture: Editor-Only Pieces

The following must exist in the Editor; they cannot be created from C++ alone (per `SPEC.md` section 10):

- **Levels** (`.umap`) under `Content/Maps/` — World Partition disabled per level.
- **Enemy placements** — always `AQuakeEnemySpawnPoint` actors with `EnemyClass` set to a `BP_Enemy_*` class. Direct `BP_Enemy_*` placements in a level are decoration and **do not count** toward `KillsTotal` or gate the level-clear scan. This is the canonical authoring path for counted enemies; see SPEC section 5.1.
- **One master material** `M_QuakeBase` with `BaseColor` / `Emissive` / `Metallic` / `Roughness` parameters; everything else is a `MaterialInstance` of it. Runtime tinting (damage flash, powerup overlays) uses `UMaterialInstanceDynamic` from C++.
- **NavMesh** — `NavMeshBoundsVolume` per level, agent radius 35 / height 180 / step height 45 to match the player capsule and movement params (step height MUST equal the player's value or AI cannot path through geometry the player can walk over).
- **Collision channels** — four custom channels (`Pickup`, `Projectile`, `Corpse` object channels + `Weapon` trace channel) defined in `Config/DefaultEngine.ini`. Per-actor responses are set in C++ constructors (`SetCollisionResponseToChannel` / `SetCollisionProfileName`), not via editor profile assets. See SPEC section 1.6 for the full response matrix and per-system rules.
- **Project Settings** in `Config/Default*.ini` — version-controlled.

The per-level checklist in `SPEC.md` section 10.5 is the authoritative reference when creating new maps.

## Auto-Memory

The user has a persistent file-based memory system at `C:\Users\Admin\.claude\projects\c--dev-games-unreal\memory\`. Save user/feedback/project memories there per the auto-memory protocol, and check existing memories before recommending design choices.
