# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**CB** (CrashBandicoot) is a Crash Bandicoot-style 3D platformer built with Unreal Engine 5.7 (C++/Blueprints). The `.uproject` file is `CB.uproject`. The UE5 engine lives at `C:\Program Files\Epic Games\UE_5.7`.

## Related Documents

- **SPEC.md** — complete gameplay spec for the Crash Bandicoot 1996 recreation (mechanics, rules, values, enemy behaviors, boss fights, level list, etc.). Follows the 1996 original, NOT the N. Sane Trilogy.
- **PHASES.md** — ordered implementation phases with deliverables and playtest criteria
- **PLAN_00.md** — Phase 0 rename plan (completed)
- **PLAN_01.md** — Phase 1 core player plan (movement, spin, camera, input, animation)
- **PLAN_02.md** — Phase 2 crates & collectibles plan (crates, pickups, Aku Aku, lives, damage)
- **PLAN_03.md** — Phase 3 enemies plan (10 archetype classes, chain-kill, projectiles, hazards, turtle platform)

## Build Commands

Always use Bash (not PowerShell) for build commands — PowerShell permission patterns have escaping issues.

```bash
# Build editor (Development)
"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" CBEditor Win64 Development "c:\dev\CrashBandicoot\CB.uproject" -WaitMutex 2>&1 | tail -15

# Build game (Development)
"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" CB Win64 Development "c:\dev\CrashBandicoot\CB.uproject" -WaitMutex 2>&1 | tail -15

# Build editor (DebugGame — full variable visibility in debugger)
"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" CBEditor Win64 DebugGame "c:\dev\CrashBandicoot\CB.uproject" -WaitMutex 2>&1 | tail -15

# Run editor
"C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe" "c:\dev\CrashBandicoot\CB.uproject"
```

If the editor is open with Live Coding active, `CBEditor` builds will fail. Either close the editor first, press Ctrl+Alt+F11 in the editor, or build the `CB` (game) target instead. `GenerateProjectFiles.bat` does not exist in UE 5.7 — project files are generated via the VS Code workspace task ("Generate Project Files").

## Module Structure

One C++ module (build target uses `DefaultBuildSettings = BuildSettingsVersion.V6`):

- **CB** (Runtime) — all gameplay code. Dependencies: EnhancedInput, GameplayTags, CommonLoadingScreen, Slate/SlateCore, UMG.

Source follows the standard UE Public/Private split under `Source/CB/`.

## AssetGen Plugin

Editor-only plugin at `Plugins/AssetGen/` for generating Blueprint assets, widget blueprints, and levels via buttons (Window > AssetGen). Dependencies include UMG, UMGEditor (for widget blueprint creation). Current buttons:

**Core Blueprints:**
- **Create BP_GameMode** — `ACBGameMode` child, auto-wires DefaultPawnClass/PlayerControllerClass
- **Create BP_GameInstance** — `UCBGameInstance` child
- **Create BP_PlayerCharacter** — `ACBPlayerCharacter` child, assigns skeletal mesh, input actions, spin montage
- **Create BP_PlayerController** — `ACBPlayerController` child, wires up input actions (IA_MoveAxis, IA_Jump, IA_Spin, IA_PauseMenu) and IMC_Gameplay

**Input:**
- **Create Input Actions** — creates `IA_MoveAxis` (Axis2D), `IA_Jump`, `IA_Spin`, `IA_PauseMenu`
- **Create IMC_Gameplay** — builds mapping context with WASD modifiers (Swizzle YXZ, Negate), gamepad bindings, and deadzone
- **Create BP_Camera** — `ACBCamera` child, configures spring arm (length 600, pitch -15, no inherit rotation, no collision test)

**Animation:**
- **Create Spin Montage** — creates `AM_Manny_Spin` from `A_Manny_Spin` animation sequence

**Phase 2 — Crates & Collectibles:**
- **Create Crate Blueprints** — creates BPs for all 6 crate types (Crate, CrateIron, CrateTNT, CrateBounce, CrateArrow, CrateIronArrow), assigns SM_Crate mesh
- **Create Pickup Blueprints** — creates BP_WumpaFruit (with SM_WumpaFruit mesh), BP_AkuAkuPickup, BP_LifePickup
- **Create BP_AkuAkuMask** — creates mask actor BP with SM_AkuAku mesh
- **Create WBP_GameplayHUD** — creates Widget Blueprint with `UCBGameplayHUD` parent, CanvasPanel root, LivesText (top-left) and WumpaText (top-right)
- **Create WBP_LoadingScreen** — creates Widget Blueprint with centered "Loading Screen" text

**Levels:**
- **Create Debug Level** — compact test level with directional light, all crate types, pickup trails, movement test geometry, and death pit

Each button is idempotent. Extend `AssetGen.cpp` to add new asset generation buttons as needed. Static helpers: `CreateOrLoadBlueprint()`, `SaveBlueprint()`, `SetClassProperty()`, `SetObjectProperty()`, `GetObjectProperty()`, `SpawnBP()`, `SpawnBox()`, `SpawnLabel()`.

### AssetGen Scripts (Procedural Content Pipeline)

Python/Blender scripts at `Plugins/AssetGen/Scripts/` generate placeholder assets:

- **generate_meshes.py** — Blender script, exports static mesh FBX. Run: `"C:\Program Files\Blender Foundation\Blender 5.1\blender.exe" --background --python Scripts/generate_meshes.py -- [output_dir] [mesh_filter]`. Meshes: Character, Monster, Tree, Crate, WumpaFruit, AkuAku.
- **generate_&lt;enemy&gt;.py** — Blender scripts for rigged enemy skeletal meshes + animations. One script per enemy type: `generate_crab.py`, `generate_skunk.py`, `generate_tribesman.py`, `generate_venusflytrap.py`, `generate_turtle.py`, `generate_snake.py`, `generate_rollingmonkey.py`, `generate_bat.py`, `generate_spider.py`, `generate_lizard.py`, `generate_shieldnative.py`, `generate_spearthrower.py`, `generate_labassistant.py`, `generate_electriclab.py`, `generate_greenblob.py`. Run: `"C:\Program Files\Blender Foundation\Blender 5.1\blender.exe" --background --python Scripts/generate_<enemy>.py -- [output_dir]`. Output goes to `Assets/Enemies/<Name>/`.
- **generate_sounds.py** — Python stdlib, exports WAV. Run: `python Scripts/generate_sounds.py [output_dir] [sound_filter]`. SFX: Jump, Coin, Hit, SpinWhoosh, CrateBreak, BounceCrate, WumpaCollect, ExtraLife, AkuAkuPickup, AkuAkuHit, AkuAkuInvincible, TNTTick, TNTExplosion, GameOver, ArrowLaunch.
- **generate_music.py** — Python MIDI + FluidSynth rendering. Run: `python Scripts/generate_music.py [output_dir] [track_filter]`. Tracks: BattleDark, Invincibility.
- **generate_sprites.py** — SVG + Inkscape PNG conversion.
- **blender_anim.py** — Mannequin animation generator using `Rig/SKM_Manny_Simple.FBX`.
- **generate_monster_rig.py** — Full rigged monster with animations. Run: `"C:\Program Files\Blender Foundation\Blender 5.1\blender.exe" --background --python Scripts/generate_monster_rig.py -- [output_dir]`. Reference pattern for rigged skeletal mesh + animation export.

FBX export conventions — **Static meshes**: `apply_unit_scale=True`, `apply_scale_options='FBX_SCALE_ALL'`, `axis_forward='-Y'`, `axis_up='Z'`. **Rigged skeletal meshes**: `apply_unit_scale=False`, `apply_scale_options='FBX_SCALE_NONE'`, `axis_forward='-Y'`, `axis_up='Z'`, `add_leaf_bones=False`, `object_types={'MESH', 'ARMATURE'}`. (`FBX_SCALE_ALL` causes double unit conversion with UE's skeletal mesh importer.)

Blender mesh orientation: model facing **-Y in Blender** (Blender's default forward). Before export, rotate the armature +90° around Z (`rotation_euler[2] = π/2`) so models face **+X forward in UE** (UE's default forward). Do NOT apply the rotation — the FBX exporter bakes it. **Exception**: humanoid models retargeted to the Unreal Mannequin skip this rotation (Mannequin faces +Y).

Blender path: `"C:\Program Files\Blender Foundation\Blender 5.1\blender.exe"`

### McpAutomationBridge REST API (Editor Open — Preferred)

The McpAutomationBridge plugin provides a direct REST endpoint at `POST http://localhost:3000/api/call` for live editor automation. This is the **preferred pipeline** for mesh imports and Blueprint property assignment while the editor is open. Use `--max-time 5` for all calls.

```bash
curl -s --max-time 5 http://localhost:3000/api/call \
  -H "Content-Type: application/json" \
  -d '{"tool":"<tool_name>","arguments":{...}}'
```

**Key operations:**
- **Import mesh/texture/sound**: `{"tool":"manage_asset","arguments":{"action":"import","sourcePath":"c:/path/file.fbx","destinationPath":"/Game/Path"}}` — fire-and-forget (import handler doesn't respond, but operation succeeds). Add `sleep 2` before the next step.
- **Import animations**: **Do NOT use REST** — crashes without skeleton picker. Use `AssetImport` commandlet with `-Type=Animation -Skeleton=/Game/Path/SK_Name_Skeleton`.
- **Set inherited component property**: `{"tool":"manage_blueprint","arguments":{"action":"set_default","requestedPath":"/Game/Path/BP_Name","propertyName":"Mesh.SkeletalMeshAsset","value":"/Game/Path/SK_Name.SK_Name"}}`
- **Set CDO property**: `{"tool":"manage_blueprint","arguments":{"action":"set_default","requestedPath":"/Game/Path/BP_Name","propertyName":"HitPoints","value":"3"}}`
- **Get SCS hierarchy**: `{"tool":"manage_blueprint","arguments":{"action":"get_scs","blueprintPath":"/Game/Path/BP_Name.BP_Name"}}`

Configuration in `Config/DefaultGame.ini` (already set):
```ini
[/Script/McpAutomationBridge.McpAutomationBridgeSettings]
bEnableNativeMCP=True
NativeMCPPort=3000
bLoadAllToolsOnStart=True
```

See `Plugins/McpAutomationBridge/AGENTS.md` for full REST API reference.

### AssetGen Commandlets (CLI — Editor Closed/Headless)

Five commandlets for headless/CI asset management. All require `MSYS_NO_PATHCONV=1` prefix in Bash. **Prefer the REST API above when the editor is open.**

```bash
# Template
MSYS_NO_PATHCONV=1 "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "c:\dev\CrashBandicoot\CB.uproject" -run=<Commandlet> <params> -stdout -unattended -nosplash
```

- **AssetImport** — import FBX/WAV/PNG files. `-SourceFile="path" -DestPath=/Game/Path [-Type=StaticMesh|SkeletalMesh|Animation] [-Skeleton=/Game/Path] [-Replace]`. Sets `bConvertSceneUnit=true` so Blender's meter-scale FBX files convert to UE centimeters. SkeletalMesh imports automatically create and save a `_Skeleton` asset in the same destination folder.
- **BlueprintCreate** — create Blueprint from parent class. `-AssetPath=... -ParentClass=/Script/Module.ClassName`. Idempotent.
- **BlueprintEdit** — modify Blueprint CDO properties. `-AssetPath=... -Set="PropName=Value"`. CDO-level properties only — component sub-properties (dot-path) don't persist through recompile. **Use the REST API `set_default` instead.**
- **BlueprintDump** — read Blueprint contents. `-AssetPath=/Game/Path/BP_Name`. Output: `Saved/BlueprintDump.txt` + stdout.
- **MaterialCreate** — create material instances. `-AssetPath=... -ParentMaterial=/Game/Path [-SetScalar="Name=Value"] [-SetVector="Name=(R=,G=,B=,A=)"] [-SetTexture="Name=/Game/Path"]`. Idempotent.

See `/ue5-assetgen` skill for full usage reference.

## Architecture

### Game Framework
- `UCBGameInstance` — boot splash tracking, loading screen hold, FPS limiting on window focus loss, **lives system** (`Lives`, `AddLife()`, `LoseLife()`, `ResetLives()`, `FOnLivesChanged`), **Wumpa counter** (`WumpaCount`, `AddWumpa()`, auto-awards extra life at 100, `FOnWumpaChanged`), Aku Aku state persistence across levels (`StoredAkuAkuState`).
- `ACBGameMode` — death→restart flow. `PlayerDied()` starts a configurable delay timer (`RestartDelay`, default 3s), then deducts a life and reloads the level (or opens Game Over level if no lives remain). Owns `ResetCurrentLevel()`.
- `ACBWorldSettings` — per-level config: world music, camera class, camera mode (defaults to `Follow`), vertical camera bounds (`CutoffLowerBoundZ`/`CutoffUpperBoundZ`). Set in `DefaultEngine.ini` via `WorldSettingsClassName`.
- `UCBDeveloperSettings` — project-wide settings auto-discovered from INI: audio assets (sound mix, sound classes, music), `bSkipLogoTrain`.
- `UCBGameUserSettings` — player-configurable volumes (`MainVolume`, `MusicVolume`, `SFXVolume`), saved to `GameUserSettings.ini`.
- Default map: `Content/Maps/Debug/Debug` (development). Game mode: `BP_GameMode`, game instance: `BP_GameInstance`.

### Character System
- `ACBPlayerCharacter` — extends `ACharacter` directly (no shared base class with enemies). Uses custom `UCBCharacterMovementComponent`. Features:
  - **No HP** — the `EAkuAkuState` enum IS the health system (`None`, `OneHit`, `TwoHits`, `Invincible`).
  - **Damage**: two entry points — `OnHit(Source)` (checks Aku Aku state, absorbs or dies) and `InstantKill()` (bypasses everything, called by kill volumes). Death plays `DeathSound`, notifies `ACBGameMode::PlayerDied()`, then `Destroy()`s the actor.
  - **Input**: owned by `ACBPlayerController`, not the pawn. Controller persists across death/respawn; pawn can be destroyed and respawned (standard UE5 lifecycle). Input handlers on the character (`Input_Move`, `Input_Jump`, `Input_StopJump`, `Input_Spin`) are called by the controller.
  - **Camera-relative movement**: projects input relative to `PlayerCameraManager->GetCameraRotation()` (not `GetControlRotation()`, which doesn't update without mouse look).
  - **Spin system**: charge-based (3 max, 0.5s regen interval, 0.5s duration). `DoSpin()` plays montage at adjusted play rate, `StopSpin()` cancels. Jump during spin cancels spin. `SpinAttackVolume` (sphere, `OverlapAll` profile) enables on spin, overlaps `ISpinnable` targets. Also checks already-overlapping actors on enable.
  - **Stomp detection**: `Landed()` checks if the landed-on actor implements `IStompable` and Z-velocity exceeds `StompVelocityThreshold`. Target decides the response (bounce, break, launch).
  - **Stomp bounce**: `StompBounceVelocity` (public, default 800) — the default upward velocity when stomping. Crates/enemies read this from the player. Arrow crates override with their own higher `LaunchVelocity`.
  - **Character rotation**: `bUseControllerRotationYaw = false`, `bOrientRotationToMovement = true`, 720°/s rotation rate.
  - **Aku Aku**: state machine (`AddMask()` progresses None→OneHit→TwoHits→Invincible), invincibility timer (20s), `AAkuAkuMaskActor` lifecycle (spawn/reposition/destroy), music hard-cut via `UCBAudioSubsystem`. All on the character, not a component.
  - Invulnerability windows (via `UBlinkComponent`), knockback via `LaunchCharacter`.
- `UCBCharacterMovementComponent` — platformer jump physics: calculates velocity from `ApexJumpHeight` + `JumpMaxHoldTime` (must be > 0, falls back to 0.3s), dynamic gravity easing on fall (`FallBeginGravityScale` -> `FallMaxGravityScale`), coyote time, early-release gravity multiplier, enemy jump support. Exposes `AirborneTime` and `bIsAirborne` as `BlueprintReadOnly` for landing sound gating.
- `UCBAnimInstance` — custom `UAnimInstance` for the AnimBP. Updates `Speed`, `VerticalVelocity`, `bIsFalling`, `bIsSpinning`, `bIsMoving` every frame in `NativeUpdateAnimation`. The AnimBP (`ABP_Manny`) only contains the state machine graph; all variable computation is in C++.

### Interaction Interfaces
Three UE interfaces define how actors respond to player attacks (declared in `Interfaces/InteractionInterfaces.h`):
- `ISpinnable` — `OnSpinHit(Player)`: triggered by the player's `SpinAttackVolume`.
- `IStompable` — `OnJumpHit(Player)`: triggered by the player's `Landed()` when Z-velocity exceeds threshold.
- `IExplodable` — `OnExplosionHit(Origin, Radius)`: triggered by TNT blast radius.

Actors implement only the interfaces they respond to. The player's attack systems query the interface — the target decides the response.

### Crate System
- `ACrateBase` (abstract) — `UStaticMeshComponent` with `BlockAll` collision (player can stand on crates), implements all three interfaces (subclasses override). Handles break sound, `SpawnContents()` hook, `bIsBroken` guard. Default `OnJumpHit` bounces the player (`StompBounceVelocity`) then breaks.
- `ACrate` — standard breakable crate. `ECrateContents` enum (`Wumpa`/`WumpaLarge`/`Life`/`Mask`/`Token`) configurable per-instance. One C++ class, multiple Blueprint presets via material swap.
- `ACrateIron` — unbreakable. All interface methods are no-ops. `bCountsTowardTotal = false`.
- `ACrateTNT` — spin = immediate detonation, stomp = bounce player + start 3-second countdown (`UTextRenderComponent`), explosion = chain detonation with 0.05s delay. Blast radius sphere overlap damages player and triggers `IExplodable` on nearby actors.
- `ACrateBounce` — up to 10 bounces (1 Wumpa each), uses `Player->StompBounceVelocity` for bounce height. Spin breaks instantly (1 Wumpa).
- `ACrateArrow` — launches player upward with own `LaunchVelocity` (3000). Only breaks on spin, not stomp.
- `ACrateIronArrow` — launches but never breaks. `bCountsTowardTotal = false`.

All crate types use the same SM_Crate mesh with different materials assigned per-Blueprint.

### Pickup System
- `APickupBase` (abstract) — `UStaticMeshComponent` (no collision) + `USphereComponent` trigger (Pickup channel, overlaps Player and Pawn channels). Idle rotation via tick. `Collect()` plays sound, calls `OnPickedUp()`, destroys actor.
- `AWumpaFruit` — increments `UCBGameInstance::AddWumpa(1)`.
- `AAkuAkuPickup` — calls `Player->AddMask()`.
- `ALifePickup` — calls `UCBGameInstance::AddLife()`.

### Input System
- `ACBPlayerController` — owns input bindings (Enhanced Input). Adds `IMC_Gameplay` mapping context, binds all input actions, forwards input to possessed pawn. Creates and manages `UCBGameplayHUD`. Binds to `UCBGameInstance` delegates (`FOnLivesChanged`, `FOnWumpaChanged`) for HUD updates.
- `UCBInputModifierDigital` — custom Enhanced Input modifier that normalizes analog stick to full magnitude (1.0) past deadzone threshold, producing digital-feeling movement from analog input.
- Input actions: `IA_MoveAxis` (Axis2D), `IA_Jump` (Boolean), `IA_Spin` (Boolean), `IA_PauseMenu` (Boolean).
- `IMC_Gameplay` maps WASD with Swizzle/Negate modifiers for 2D axis, gamepad stick with deadzone, Space/A for jump, LShift/J/LMB/X/B for spin.

### Enemy AI System
- `ACBEnemyCharacterBase` — extends `ACharacter` directly. Implements `ISpinnable`, `IStompable`, `IExplodable` with virtual defaults. HP system (`HitPoints`/`CurrentHitPoints`), `HandleDefeat()` (sound + disable collision + destroy), `SpawnLaunchedProjectile()` for chain-kill. Capsule overlap bound in C++ BeginPlay — invincible player kills on contact. Patrol spline following, trigger volume system. Initialized via `FEnemyInitializationArgs` struct.
- `ACBEnemyAIControllerBase` — minimal `AAIController` subclass. Enemies self-drive via `Tick()` with C++ enum state machines, not behavior trees.
- `ACBEnemyPatrolRigActor` / `UCBEnemyPatrolRigComponent` — self-contained patrol setup actor. Spawns enemy at first spline point using deferred spawning pattern.
- `UCBPatrolRigDebugVisualizer` — editor-only numbered waypoint visualization via custom show flag "PatrolRigPoints".

### Enemy Archetype Classes (10 archetypes, 18 enemy types via BP config)
- `AEnemyPatrol` — Idle/Patrolling/Dead. Crab, Skunk, Tribesman via BP config (speed, mesh).
- `AEnemyCycling` — Vulnerable/Attacking timer-driven cycle. Venus Fly Trap (green/white), Rolling Monkey. Flags: `bSpikedTop`, `bPatrolsWhileAttacking`, `bInvulnerableWhileAttacking`.
- `AEnemySnake` — Hidden/Emerging/Lunging/Dead. Collision/visibility toggle per state.
- `AEnemyFlying` — Flying (spline patrol) or Perched/Swooping/Returning (swooper variant). GravityScale=0, SetActorLocation.
- `AEnemyCeiling` — Hanging/Dropping/Landed/Climbing. Spider dropper. Line traces for ground.
- `AEnemyJumping` — Grounded/Jumping. Spider jumper, Green/Red Lizard. Red Lizard: `bExtraHighBounce` for 1400 stomp bounce.
- `AEnemyShielded` — Patrol + frontal shield blocking via dot product. Blocks spin from front, stomp/explosion bypass.
- `AEnemyRanged` — Idle/Throwing cycle. Spear Thrower (forward), Beaker Lab (aimed at player). Spawns `AProjectileBase` subclass.
- `AEnemyElectric` — Idle/Patrolling/Pursuing. Electric barrier: spin only works below `ElectricBarrierHeight`, stomp always damages player.
- `AEnemyTurtle` — Idle/Patrolling/Flipped/Dead. Stomp flips (becomes moving platform via `UStaticMeshComponent` with `BlockAll`), spin always kills.

### Chain-Kill & Projectile System
- `ALaunchedEnemyProjectile` — spawned on spin-kill. Velocity + gravity, tumbles, kills enemies via `OnExplosionHit`, triggers `IExplodable` on crates. One chain level (no cascade). Skips player.
- `AProjectileBase` (abstract) — sphere collision, speed/arc/lifetime. Overlaps damage player via `Player->OnHit()`.
- `ASpearProjectile` — straight line, Speed=1200.
- `ABeakerProjectile` — lobbing arc (ArcGravity=980). Red: overlap sphere on impact. Green: spawns `AGreenBlobEnemy`.
- `AGreenBlobEnemy` — 1HP enemy, bounces on timer, 5s lifetime, `bSpinLaunchesAsProjectile=false`.

### Hazard System
- `AHazardBase` (abstract) — `AActor` with mesh + damage sphere. Not defeatable, no HP.
- `AFlyingFishHazard` — timer-driven parabolic arc jump. Hidden during rest.
- `ABouncingBarrelHazard` — sinusoidal vertical oscillation. Always bouncing, always dangerous.

### Camera System
- `ACBCamera` — `Follow` mode smoothly tracks the player on all three axes (X, Y, Z) using `FInterpTo`. `Fixed` mode interpolates to a target point. Uses spring arm. Ticks in `TG_PostPhysics`.
- `UCBCameraSubsystem` (WorldSubsystem) — per-world camera lifecycle. Instantiates camera from world settings class, sets `DefaultCameraMode` from `ACBWorldSettings`, manages view target blending.

### Audio System
- `UCBAudioSubsystem` (GameInstanceSubsystem) — persists across level changes. Manages world music player (single persistent `UAudioComponent`), volume control via sound mix overrides per sound class (Main/Music/SFX). Music switches are hard-cuts (no crossfading), matching the original game. `PlayInvincibilityMusic()` and `PlayWorldMusic()` (no-arg, restores current world music) for Aku Aku invincibility. Individual actors play their own SFX via `PlaySoundAtLocation`.

### UI System
- `UCBGameplayHUD` — `UUserWidget` subclass created by `ACBPlayerController`. Displays lives counter and Wumpa counter via `BindWidget` text blocks (`LivesText`, `WumpaText`). No Aku Aku indicator on HUD (mask state is communicated via the visual on Crash, matching the 1996 original).
- Game Over level does not exist yet. `ACBGameMode::HandleGameOver()` calls `UGameplayStatics::OpenLevel(TEXT("GameOver"))` when lives reach 0.
- CommonUI has been removed — simple `UUserWidget` subclasses with `BindWidget` properties.

### Reusable Components
- `UBlinkComponent` — toggles owner's mesh visibility at a configurable interval. Used for post-hit invulnerability blinking. No game-system knowledge — any actor can use it.
- `AAkuAkuMaskActor` — separate actor spawned and attached to the player for mask visual. `SetNormalAppearance()` / `SetGoldenAppearance()` for material swap.

### Custom Collision Channels
Defined in `DefaultEngine.ini`, with named constants in `CBCollisionChannels.h`:
```cpp
namespace CBCollision
{
    constexpr ECollisionChannel Player = ECC_GameTraceChannel1;
    constexpr ECollisionChannel Pickup = ECC_GameTraceChannel2;
    constexpr ECollisionChannel Enemy  = ECC_GameTraceChannel3;
}
```
Use `CBCollision::Player` etc. instead of raw `ECC_GameTraceChannel` values. Include `"CBCollisionChannels.h"`.

Collision rules:
- Player ↔ Pickup: overlap (player collects pickups)
- Player ↔ Enemy: overlap (damage/stomp detection)
- Pickup ↔ Enemy: ignore (no interaction)

Note: player capsule currently uses default `ECC_Pawn`, not the custom Player channel. Pickup triggers overlap both `CBCollision::Player` and `ECC_Pawn` to handle this.

### Gameplay Tags
- `Input.Layer.Game` — input context

### Key Plugins
EnhancedInput, Water/WaterExtras, CommonLoadingScreen, ChaosVD (visual debugger), Niagara (via CascadeToNiagaraConverter), AssetGen (editor-only asset generation), McpAutomationBridge (REST API for live editor automation — mesh import, Blueprint property assignment).

## Blueprint Asset Layout
```
Content/Blueprints/
├── Crates/       BP_Crate, BP_CrateIron, BP_CrateTNT, BP_CrateBounce, BP_CrateArrow, BP_CrateIronArrow
├── Enemies/      BP_Crab, BP_Skunk, BP_Tribesman, BP_VenusFlyTrapGreen, BP_VenusFlyTrapWhite,
│                 BP_RollingMonkey, BP_Snake, BP_BatFlying, BP_BatSwooping, BP_SpiderCeiling,
│                 BP_SpiderGround, BP_LizardGreen, BP_LizardRed, BP_ShieldNative, BP_SpearThrower,
│                 BP_BeakerLabAssistant, BP_ElectricLabAssistant, BP_Turtle, BP_GreenBlob
├── Game/         BP_Camera, BP_GameInstance, BP_GameMode, BP_PlayerCharacter, BP_PlayerController
├── Hazards/      BP_FlyingFish, BP_BouncingBarrel
├── Pickups/      BP_AkuAkuPickup, BP_LifePickup, BP_WumpaFruit
├── Projectiles/  BP_Spear, BP_BeakerRed, BP_BeakerGreen
├── Props/        BP_AkuAkuMask
└── UI/           WBP_GameplayHUD, WBP_LoadingScreen
```

Enemy asset data (meshes, animations, materials) is organized per-enemy under `Content/Enemies/<Name>/`. Blueprints stay in `Content/Blueprints/Enemies/` for easy access. Projectile/hazard static meshes in `Content/Projectiles/` and `Content/Hazards/<Name>/`. See PLAN_03.md for the full layout.

## Patterns to Follow

- **C++ first**: implement almost everything in C++. Blueprints are a thin layer for exposing parameters and referencing assets only. The exception is UI widgets, which are built in UMG as Widget Blueprints subclassing C++ bases.
- **Deferred spawning** for enemies: `SpawnActorDeferred<T>()` -> init -> `FinishSpawning()`. This allows passing `FEnemyInitializationArgs` before `BeginPlay`.
- **Subsystems over singletons**: Audio uses `UGameInstanceSubsystem` (persists across levels), Camera uses `UWorldSubsystem` (recreated per level).
- **Event communication**:
  - *1 producer → N consumers*: dynamic multicast delegates (loose coupling). Example: `UCBGameInstance::OnLivesChanged` → HUD subscribes.
  - *1 producer → 1 consumer*: direct call. Example: player calls `AudioSubsystem->PlayInvincibilityMusic()` directly.
  - *BlueprintImplementableEvent* for C++-to-Blueprint hooks (one override, presentation logic).
- **Collision setup**: profiles defined in `DefaultEngine.ini`, constants in `CBCollisionChannels.h`, defaults assigned on Blueprint components, runtime changes in C++ (e.g., temporarily ignoring channels during powerups).
- **DeveloperSettings** for project-wide config: assets referenced via `TSoftObjectPtr<>` (loaded on demand, not at startup), auto-registered in Project Settings.
- **Timer handles** (`FTimerHandle`) for all latent gameplay: stun durations, invulnerability windows, powerup timers, boss defeat delays.
- **AnimInstance in C++**: animation variables (`Speed`, `bIsFalling`, etc.) computed in `NativeUpdateAnimation`, not in the AnimBP Event Graph. The AnimBP only contains the state machine graph.
- **PrioritizeCategories**: all gameplay classes use `UCLASS(meta = (PrioritizeCategories = "CB"))` so custom properties appear at the top of the Details panel. All `UPROPERTY`/`UFUNCTION` categories use a flat `"CB"` category — no subcategories.
- **Interaction via interfaces**: attackable actors implement `ISpinnable`/`IStompable`/`IExplodable`. The player's attack systems query the interface — the target decides the response. No switch statements on actor type.

## Naming Convention

The `CB` prefix is reserved for **core game framework** classes:
- **CB prefix**: Game framework (`ACBPlayerCharacter`, `UCBGameInstance`, `ACBCamera`, `ACBPlayerController`), player-related (`UCBCharacterMovementComponent`, `UCBAnimInstance`, `UCBGameplayHUD`), settings (`UCBDeveloperSettings`, `ACBWorldSettings`), input (`UCBInputModifierDigital`).
- **Enemy prefix**: Enemy archetype classes use `AEnemy*` prefix — `AEnemyPatrol`, `AEnemyCycling`, `AEnemySnake`, `AEnemyFlying`, `AEnemyCeiling`, `AEnemyJumping`, `AEnemyShielded`, `AEnemyRanged`, `AEnemyElectric`, `AEnemyTurtle`.
- **No prefix**: Everything else — crates (`ACrateBase`, `ACrate`, `ACrateTNT`), collectibles (`AWumpaFruit`, `AAkuAkuMaskActor`), pickups (`APickupBase`, `ALifePickup`), enemy base (`ACBEnemyCharacterBase`), subsystems (`UCBAudioSubsystem`, `UCBCameraSubsystem`), reusable components (`UBlinkComponent`), interfaces (`ISpinnable`, `IStompable`, `IExplodable`), props, hazards, data assets.

## Conventions

- **Unreal Engine C++ Best Practices and Patterns**
- **KISS** — simplest thing that works
- **YAGNI** — don't build for hypothetical needs
- **DRY** — remove real duplication, not shape-similar code
- **Locality of change** — adding a new entity or feature should touch as few files as possible
- **Bash over PowerShell** — use Bash for all shell commands; PowerShell permission patterns have escaping issues with Claude Code
