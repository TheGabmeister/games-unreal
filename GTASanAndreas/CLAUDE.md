# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**GTASA** is an Unreal Engine 5.7 C++ project structured as three independent gameplay variant prototypes — Combat, Platforming, and Side-Scrolling — all living in a single module (`GTASA`) under `Source/GTASA/`. Each variant has its own Character, GameMode, PlayerController, and gameplay objects. Shared base classes (`GTASACharacter`, `GTASAGameMode`, `GTASAPlayerController`) live at the module root.

Engine: UE 5.7 | Module deps: Engine, AIModule, UMG | Plugins: StateTree, GameplayStateTree, ModelingToolsEditorMode

## Build & Iteration

There are no build scripts. Compile through the Unreal Editor (Play-in-Editor) or via Unreal Build Tool from the command line:

```
# Regenerate Visual Studio solution
UnrealBuildTool.exe -projectfiles -project="C:/dev/games-unreal/GTASanAndreas/GTASA.uproject" -game -rocket -progress

# Build Development Editor (Windows)
UnrealBuildTool.exe GTASAEditor Win64 Development "C:/dev/games-unreal/GTASanAndreas/GTASA.uproject"
```

Hot-reload (Live Coding) is the normal iteration loop inside the editor.

Default map: `/Game/ThirdPerson/Lvl_ThirdPerson`

## Architecture

### Variant Structure

Each variant follows the same layout:

```
Variant_<Name>/
  <Name>Character.h/.cpp          — ACharacter subclass with variant-specific movement
  <Name>GameMode.h/.cpp
  <Name>PlayerController.h/.cpp
  AI/                             — AIController, NPC/Enemy, StateTree utilities, EQS contexts
  Animation/                      — AnimNotify subclasses (montage callbacks)
  Gameplay/                       — AActor gameplay objects (volumes, pickups, platforms)
  Interfaces/                     — UInterface definitions
  UI/                             — UUserWidget subclasses
```

Content mirrors this under `Content/Variant_<Name>/`.

### Input

Uses **Enhanced Input** exclusively. Input Actions and Mapping Contexts live in `Content/Input/` (base) and `Content/Variant_*/Input/`. Input contexts are assigned and activated in each variant's PlayerController. Mobile touch controls are handled in `GTASAPlayerController` via UMG.

### Combat Variant

- Interface-based damage model: `ICombatAttacker` (DoAttackTrace / CheckCombo / CheckChargedAttack) and `ICombatDamageable` (ApplyDamage / HandleDeath / ApplyHealing / NotifyDanger).
- Attack timing driven by AnimNotifies on montages: `AnimNotify_DoAttackTrace`, `AnimNotify_CheckCombo`, `AnimNotify_CheckChargedAttack`.
- Melee hit detection via sphere traces in `CombatCharacter`.
- AI uses `StateTreeAIComponent` on `CombatAIController`; EQS contexts for player and danger locations.
- Respawn: `CombatCheckpointVolume` calls `SetRespawnTransform()` on the PlayerController; `CombatPlayerController` respawns on death.

### Platforming Variant

- Coyote time: 0.16 s grace period after leaving ground.
- Wall jump: forward sphere trace detects walls; applies 800 cm/s horizontal + 900 cm/s vertical impulse. 0.1 s input lockout after wall jump.
- Dash: montage-based; `AnimNotify_EndDash` clears dash state.
- State queries: `HasDoubleJumped()`, `HasWallJumped()` exposed for Blueprint/AnimBP.

### Side-Scrolling Variant

- Movement constrained to 2D plane; custom `SideScrollingCameraManager`.
- Soft platforms (`SideScrollingSoftPlatform`): one-way collision — drop through by holding down.
- NPC interaction via `ISideScrollingInteractable`; 3 s cooldown after interaction.
- `SideScrollingGameMode` spawns the HUD widget and tracks pickup count.

### Renderer / Platform

Lumen + ray-tracing enabled, Substrate shading model, static lighting disabled. Shader model SM6 (DX12/Vulkan/Metal). Desktop maximum-performance target. No mobile rendering path despite touch input support.
