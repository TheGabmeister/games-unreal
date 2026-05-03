# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**GTASA** is an Unreal Engine 5.7 C++ recreation of GTA: San Andreas gameplay mechanics. The codebase is transitioning from three learning-exercise prototypes (Combat, Platforming, Side-Scrolling) to a single unified GTA:SA gameplay layer. The three `Variant_*` folders under `Source/GTASA/` are being retired.

Engine: UE 5.7 | Module: `GTASA` | Module deps: Engine, AIModule, EnhancedInput, UMG, Slate

## Canonical Documents

- **SPEC.md** — game behavior data (what the original GTA:SA does). Source of truth for mechanics, values, and data file references.
- **Notion: Implementation Plan** — UE5 implementation details per feature (how we build it). 18 feature sections with deliverables, UE5 approach, and animation lists.
- **Notion: Execution Phases & Architecture** — 10 phased milestones with test checklists, dependency graph, and all architectural decisions.

When researching a system, check SPEC.md first for game data, then Notion for implementation approach. Game behavior details belong in SPEC.md; UE5 implementation details belong in Notion.

## Build & Iteration

No build scripts. Compile through the Unreal Editor (Play-in-Editor) or via Unreal Build Tool:

```
# Regenerate Visual Studio solution
UnrealBuildTool.exe -projectfiles -project="C:/dev/games-unreal/GTASanAndreas/GTASA.uproject" -game -rocket -progress

# Build Development Editor (Windows)
UnrealBuildTool.exe GTASAEditor Win64 Development "C:/dev/games-unreal/GTASanAndreas/GTASA.uproject"
```

Hot-reload (Live Coding) is the normal iteration loop inside the editor.

Default map: `/Game/ThirdPerson/Lvl_ThirdPerson`

## Architecture

### Code Philosophy

- **C++ first, thin Blueprint layer.** All gameplay logic in C++. Blueprints only for: asset references (meshes, montages, input actions via BP defaults), `UPROPERTY(EditDefaultsOnly)` parameter tweaking, UMG widget layout, AnimBP graphs, and level design. No gameplay logic in Blueprint event graphs.
- **Component-based player.** `SAPlayerCharacter` composed of: `USAMovementComponent`, `USAWeaponComponent`, `USAStatsComponent`, `USAInteractionComponent`.
- **Data-driven.** Weapon stats, vehicle handling, NPC behavior, zone population all driven by `UDataAsset` subclasses mapping 1:1 to original game data files (weapon.dat, handling.cfg, pedstats.dat, melee.dat).
- **Plain C++ state machines for AI.** Each NPC archetype has its own `AAIController` subclass with a state enum + switch-based `Tick` logic. No StateTree, no Behavior Trees.
- **No GAS.** Damage is `AActor::TakeDamage` with `UDamageType` subclasses. Stats are flat floats on `USAStatsComponent`.
- **Enhanced Input** with dual KB+M and gamepad support via single Input Mapping Context per action set.

### Damage System

All damage flows through `AActor::TakeDamage`. Five damage types: `Bullet`, `Melee`, `Explosion`, `Fire` (DOT), `Fall`. Characters have `Health` + `Armor` as floats on the class (not a component). Vehicles use `USAVehicleDamageComponent` (1000-scale health). Breakable world props use `USABreakableComponent` (binary destroy, no health tracking). Armor absorbs bullet/explosion but NOT fall damage.

### Collision

Two custom trace channels only: `SA_Weapon` (melee + firearm hit detection) and `SA_Interaction` (short-range interaction). No custom object channels — built-in `Pawn`, `Vehicle`, `WorldStatic`, `WorldDynamic` cover all cases.

### Legacy Code (Being Retired)

The `Variant_Combat/`, `Variant_Platforming/`, and `Variant_SideScrolling/` folders contain prototype code from earlier learning exercises. Reusable patterns: AnimNotify-based attack traces, sphere-trace melee hit detection, montage-driven abilities. The variant-specific logic (interfaces like `ICombatAttacker`/`ICombatDamageable`, StateTree AI, EQS contexts) is NOT carried forward.

### Renderer / Platform

Lumen + ray-tracing enabled, Substrate shading model, static lighting disabled. Shader model SM6 (DX12/Vulkan/Metal). Desktop maximum-performance target.
