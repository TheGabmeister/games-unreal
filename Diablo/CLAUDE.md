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

**Runtime module:** [Diablo.Build.cs](Source/Diablo/Diablo.Build.cs). Public dependencies: `EnhancedInput`, `AIModule`, `NavigationSystem`, `StateTreeModule`, `GameplayStateTreeModule`, `UMG`, `Slate`, `SlateCore`. The module log category is `LogDiablo` ([Diablo.h](Source/Diablo/Diablo.h)).

**Editor plugin:** [Plugins/DiabloEditor/](Plugins/DiabloEditor/) — editor-only plugin with `FDiabloAssetGenerator` (static utility struct) and a Slate toolbar panel accessible via **Tools > Diablo > Diablo Tools**. Generates BP subclasses, maps, input assets, imports FBX/audio, and configures Blueprint CDO defaults. Registered in `.uproject` with `"TargetAllowList": ["Editor"]`.

**Plugins enabled** in `.uproject`: `StateTree`, `GameplayStateTree` (reserved for a future AI refactor, not required by current M19), `ModelingToolsEditorMode` (editor only), `DiabloEditor` (editor only).

## Architecture

All C++ classes are `Abstract` with thin BP subclasses (`BP_` prefix) under `Content/Blueprints/`. Widget trees are built in C++ via `RebuildWidget()`, not the UMG designer.

### Class Hierarchy

- `ADiabloGameMode` → `BP_DiabloGameMode` — sets pawn/controller classes
- `ADiabloHero : ACharacter` → `BP_DiabloHero` — orthographic isometric camera, `FDiabloStats`, `UInventoryComponent`, spell system, XP/leveling
- `ADiabloPlayerController` → `BP_DiabloPlayerController` — click-to-move/attack/interact, owns all UI widgets, belt hotkeys 1–8
- `ADiabloEnemy : ACharacter` → `BP_DiabloEnemy` — AI via `ADiabloAIController` (tick-based state machine), archetypes, drop tables, XP rewards
- `ADiabloAIController` — `EAIState` (Idle/Chase/Flee/Attack/Dead), `EDiabloEnemyArchetype` for behavior tuning
- `ASpellProjectile` (Abstract) → `AFirebolt`, `AFireball`, `ALightningBolt`, `AChainLightning` — `UProjectileMovementComponent`-based. `AChainLightning` overrides overlap behavior by rebinding the delegate in `BeginPlay` (unbinds base `OnOverlap`, binds own `OnChainOverlap`) — do not use `virtual` on delegate-bound `UFUNCTION`s, it causes module load failures at runtime
- `APortalActor` — `IInteractable`, bidirectional town↔dungeon teleport. `bReturnsToDungeon` determines direction
- `ADiabloDungeonGenerator` — runtime 40×40 procedural layout via `UTilePalette`, spawns enemies/items/stairs. Single `Lvl_Dungeon` map reused for all 16 floors — floor index on GameInstance drives biome, palette, difficulty, and stair targets

### Key Data Types

- `FDiabloStats` — HP/MaxHP/Mana/MaxMana/Str/Mag/Dex/Vit (shared by hero and enemies)
- `UItemDefinition : UPrimaryDataAsset` — item template (stats, category, equip slot, `EItemUseEffect`, `ScrollSpell`)
- `FItemInstance` — runtime item: `UItemDefinition*` + `TArray<FItemAffix>` + durability + stack count + `bIdentified`
- `USpellDefinition : UPrimaryDataAsset` — spell template (mana cost, cooldown, damage, `ESpellEffect` enum, projectile class, `AoERadius`, `Duration`, `MaxBounces`). `ESpellEffect`: Projectile, Heal, AoE, TownPortal, Teleport (instant reposition to cursor), Debuff (Stone Curse — freezes enemy via `bStoneCursed` + `StoneCurseEndTime` on `ADiabloEnemy`, AI controller skips all ticking while frozen), Buff (Mana Shield — toggle `bManaShieldActive` on hero, `TakeDamage` absorbs from mana 1:1 before HP)
- `FAffixGenerator` — static utility, rolls magic items via `UAffixTable` data assets (`AT_Prefixes`/`AT_Suffixes`)
- `UInventoryComponent` — 10×4 grid + 7 equip slots + 8 belt slots + gold. Broadcasts `FOnInventoryChanged`
- `UDiabloGameInstance` — persists hero/inventory/spell state across `OpenLevel` transitions. Also stores `CurrentFloorIndex` (0=town, 1–16=dungeon) and Town Portal state (`bPortalActive`, `PortalFloorIndex`, `PortalDungeonLocation`, `PortalDungeonSeed`)
- `UDiabloSaveGame` — serializes to disk via `SaveGameToSlot`, single slot `"DiabloSave"`

### UI Widgets (all Abstract, C++ widget trees via `RebuildWidget()`)

`ADiabloPlayerController` creates and owns all widgets in `CreateHUD()`. Toggle keys: **I** inventory, **C** character, **S** spellbook, **ESC** menu.

- `UDiabloHUDWidget` — life/mana globes, XP bar, level text, 8 belt slots. Event-driven via `FOnStatsChanged` + `FOnInventoryChanged`
- `UDiabloInventoryPanel` — equipment row, 10×4 grid, belt row, gold. Drag-drop between grid/equip/belt. Hit-testing uses `GetTickSpaceGeometry()` (not `GetPaintSpaceGeometry`)
- `UDiabloCharacterPanel` — stat display with + buttons for `UnspentStatPoints`
- `UDiabloSpellbookPanel` — known spells list, RMB to bind active spell
- `UDiabloShopPanel` — two-panel buy/sell via `UNPCShopData`. Shows "Repair" button when NPC has `bCanRepair` (Griswold)
- `UDiabloMainMenu` — pause menu (Resume/Save/Load). Save only in town
- `UDiabloDialogWidget` — NPC dialog overlay

### Key Design Decisions (non-obvious, not derivable from code)

- **Targeted hit combat** — click enemy → walk into range → montage → apply damage directly. No physics traces.
- **D1 To-Hit formula:** `50 + Dex/2 + CharLevel - MonsterAC + (CharLevel - MonsterLevel)`, clamped 5–95%.
- **Warrior stat formulae:** `MaxHP = 70 + (Vit-25)*2 + (Level-1)*2`, `MaxMana = 10 + (Mag-10)*1 + (Level-1)*1`. Starting: Str 30, Mag 10, Dex 20, Vit 25. Caps: Str 250, Mag 50, Dex 60, Vit 100.
- **XP table:** quartic curve `Table[L] = Table[L-1] + 2000*L^3/4`, uses `int64`. Level cap 50, +5 stat points per level.
- **No polymorphic dispatch** — item categories, equip effects, affix types, use effects are all enum + switch.
- **Affixes apply when equipped even if unidentified** (D1 behavior).
- **`EItemUseEffect`** drives all consumable use: `RestoreHP`, `RestoreMana`, `RestoreBoth`, `CastSpell`.
- **Belt** — potions/scrolls only (`IsBeltCompatible`). Scroll casting fires in hero's facing direction, no mana cost.
- **`IInteractable`** UInterface — shared click→walk→interact for stairs, NPCs, and portals.
- **Single dungeon map** — `Lvl_Dungeon` is one map reused for all 16 floors. `CurrentFloorIndex` on GameInstance drives everything: `ResolveFloorSettings()` selects biome palette (`TP_Cathedral`/`TP_Catacombs`/`TP_Caves`/`TP_Hell`), scales enemy count/difficulty, sets stair targets, and adjusts lighting. Floors are ephemeral (regenerated each visit, matching D1 behavior). Town stairs set floor index to 1 and open `Lvl_Dungeon`; dungeon stairs increment/decrement floor index and reload the same map.
- **Town Portal seed preservation** — when Town Portal is cast, the current floor's seed is saved to GameInstance. When returning through the portal, `ResolveFloorSettings` pre-populates the GameMode seed map so the layout regenerates identically and the portal location is valid.
- **Durability** — armor degrades on hit taken (`DegradeRandomArmor`), weapons on hit dealt (`DegradeWeapon`). Items at 0 durability provide no stats (`RecomputeDerivedStats` skips them via `IsItemBroken`). Griswold repairs for gold (5g per durability point missing).
- **Widget BPs** must be created via `UWidgetBlueprintFactory` (not `FKismetEditorUtilities::CreateBlueprint`).
- **`RebuildWidget()`** for widget tree setup — `NativeConstruct()` is too late for the Slate layer.
- **`DefaultEngine.ini`** sets `RuntimeGeneration=Dynamic` for runtime navmesh from dungeon tiles.
- **Never make delegate-bound `UFUNCTION`s `virtual`** — UE dynamic delegates (`AddDynamic`) use reflection name lookup, not C++ vtables. Adding `virtual` to a delegate-bound UFUNCTION causes module load failures at runtime. To override overlap/delegate behavior in a subclass, rebind the delegate in `BeginPlay`: `RemoveDynamic` the base function, `AddDynamic` a new UFUNCTION with a different name (see `AChainLightning::OnChainOverlap` pattern).

### Input

**Enhanced Input only.** No legacy `InputComponent` axis bindings.

- IA assets generated by the DiabloEditor plugin under `Content/Input/Actions/` (`IA_Click` mapped to LMB, `IA_Cast` mapped to RMB, `IA_CharPanel` mapped to C, `IA_Inventory` mapped to I, `IA_Spellbook` mapped to S, `IA_Menu` mapped to ESC, `IA_Belt1..8` mapped to keys 1–8, `IA_Move`, `IA_Look`). IMC at `Content/Input/IMC_Diablo`.
- **Don't create IA/IMC at runtime via `NewObject<>`** — assign the editor assets to BP UPROPERTY slots.
- `ADiabloPlayerController` holds `TObjectPtr<UInputAction>` and `TObjectPtr<UInputMappingContext>` slots, binds in `SetupInputComponent`.
- No touch controls — desktop-only (Diablo 1 is mouse+keyboard).
- **Input mode convention:** When closing panels, use `FInputModeGameAndUI` with `SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock)` — never `FInputModeGameOnly()`, which captures the mouse and breaks click-to-move. Modal panels (shop) suppress game input by returning early from `OnClickStarted()` while visible.

### Asset Generator (DiabloEditor Plugin)

`FDiabloAssetGenerator` static methods, callable from the Slate panel (**Tools > Diablo > Diablo Tools**):

**Import** (bring external assets into UE):

| Method | Behavior |
|---|---|
| `ImportWarriorFBX` | Imports mesh + animations from `Tools/blender/out/Warrior.fbx` via `bReplaceExisting` (in-place update). New animations that don't exist yet are imported from separate per-animation FBX files (`Warrior_Death.fbx`, etc.) |
| `ImportAttackSFX` | Imports `Tools/audio/out/SwordSwing.wav` and `SwordHit.wav` under `/Game/Audio/SFX/` |
| `ImportLevelUpSFX` | Imports `Tools/audio/out/LevelUp.wav` under `/Game/Audio/SFX/` |
| `ImportPotionSprite` | Imports `Tools/svg/out/HealingPotion.png` as texture under `/Game/Items/Potions/` |
| `ImportItemIcons` | Imports item icon PNGs from `Tools/svg/out/` under `/Game/Items/Icons/` |

**Setup** (create BP if needed + configure CDO defaults — one button per blueprint):

`SetupHero`, `SetupController`, `SetupGameMode`, `SetupEnemy`, `SetupPotion` (creates both `BP_HealingPotion` and `BP_ManaPotion`), `SetupHUD` (all 7 widget BPs), `SetupInventory` (10 item definitions with `UseEffect`/`ScrollSpell`), `SetupDropMaterial`, `SetupSpells` (11 spell definitions: Firebolt, Fireball, Lightning, Chain Lightning, Nova, Apocalypse, Healing, Town Portal, Teleport, Stone Curse, Mana Shield), `SetupShopData`, `SetupAffixes`, `SetupAllDungeonPalettes` (4 biome palettes + 8 materials), `SetupAllBlueprints` (runs all). See `DiabloAssetGenerator.cpp` for per-method details.

**World:**

| Method | Behavior |
|---|---|
| `GenerateDefaultMap` | Creates `Lvl_Diablo` with PlayerStart, DirectionalLight, floor plane, NavMeshBoundsVolume, test enemy, healing potion, stairs to dungeon floor 1, 6 Tristram NPCs (Griswold with `bCanRepair`, Adria, Pepin, Cain, Wirt, Ogden) — **always recreates** |
| `GenerateDungeonMap` | Creates `Lvl_Dungeon` — single reusable map with dim lighting, NavMeshBoundsVolume, and `ADiabloDungeonGenerator`. The generator reads `CurrentFloorIndex` from GameInstance at runtime to select biome palette, difficulty, and stair targets — **always recreates** |
| `GenerateDebugCombatMap` | Creates `Lvl_DebugCombat` — flat arena with all 6 enemy archetypes, 5 healing potions, 5 mana potions, navmesh. For testing combat and spells — **always recreates** |
| `GenerateInputAssets` | Creates/updates IA_Click/Cast/CharPanel/Inventory/Spellbook/Menu/Belt1..8/Move/Look and IMC_Diablo — **updates in place** |

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
- The main mesh FBX (`warrior_mesh.py`) contains all animations as NLA strips with `bake_anim_use_nla_strips=True`, `bake_anim_use_all_actions=False`. NLA strip names determine UE asset names (`Idle` → `Warrior_Anim_Idle`)
- Each animation also has a **standalone FBX** (e.g., `warrior_anim_idle.py` → `Warrior_Idle.fbx`) for reliable import of new animations on reimport. The armature is defined in `warrior_armature.py` (shared module)
- Character meshes are built at real-world scale in Blender (1.8m tall = 180cm in UE)

Animations are keyframed in Blender Python scripts — the main mesh script and per-animation standalone scripts both share `warrior_armature.py` for bone hierarchy.

**Manually authored editor assets** (not generated by plugin):
- **AnimBlueprints** — `ABP_Warrior` parents to `UDiabloAnimInstance`, has Locomotion state machine (Idle/Walk on `Speed`) + DefaultSlot for montage overlay. Do not attempt programmatic AnimBP generation.
- **AnimMontages** — `AM_Attack` (Melee Attack Trace + PlaySound notifies), `AM_Death` (`bEnableAutoBlendOut = false`). Set on BPs via `SetupHero`/`SetupEnemy`.

### FBX Reimport Workflow

`ImportWarriorFBX` never deletes existing assets — `bReplaceExisting` updates the mesh, skeleton, and animation sequences in place, preserving all references (AnimBP, montages, scene proxies). After the main import, any animation that doesn't exist yet is imported from its standalone FBX (e.g., `Warrior_Death.fbx` → `Warrior_Anim_Death`).

**Adding a new animation:** Create a standalone Blender script using `warrior_armature.py`, export to `Tools/blender/out/`, add an `ImportSingleAnim` call in `ImportWarriorFBX`, and also add the NLA strip to `warrior_mesh.py` for first-time imports.

**Never delete animation sequences or the skeletal mesh during reimport** — this breaks AnimBP references. The UE importer's `bReplaceExisting` handles in-place updates correctly.

**When bone hierarchy changes** (add/remove/rename bones): manually delete `Warrior_Skeleton` in the Content Browser before reimporting. The AnimBP will need to be recreated. This should be rare — the bone hierarchy is defined in `warrior_armature.py` and is stable.

## Coding principles

- **KISS** — simplest thing that works. No clever patterns where a plain `if` does the job.
- **YAGNI** — don't build for hypothetical needs. No interfaces with one implementation, no config knobs with one value.
- **DRY** — remove real duplication, not shape-similar code. Wrong abstraction costs more than repetition.
- **Phased introduction** — each system enters at the phase where gameplay first needs it.
- **Unreal Engine C++ best practices** — use `UPROPERTY`/`UFUNCTION` macros on all exposed members; prefer `TObjectPtr<>` over raw pointers for UObject members; use `CreateDefaultSubobject` for components in constructors; mark base classes `Abstract` when they should never be spawned directly; use `PURE_VIRTUAL` macro (not C++ `= 0`) for abstract virtuals; prefer `TArray`/`TMap`/`TSet` over STL containers; use `FName`/`FString`/`FText` appropriately (FName for identifiers, FString for manipulation, FText for user-facing display); use `UE_LOG(LogDiablo, ...)` for logging; follow UE naming conventions (`A` prefix for Actors, `U` for UObjects, `F` for structs/value types, `E` for enums, `I` for interfaces).

When in doubt, lean KISS over DRY.

## Design Reference

The full Diablo (1997) gameplay reference, milestone roadmap, and verification criteria are in [SPEC.md](SPEC.md). Consult it before making design decisions.

**Combat model:** Diablo 1 uses targeted hit (not physics traces). Click enemy → walk into range → play attack montage → apply damage directly to target. To-Hit% roll + Str-based damage + AC reduction implemented in M6.

## Auto-Memory

The user has a persistent file-based memory system at `C:\Users\Admin\.claude\projects\c--dev-games-unreal\memory\`. Save user/feedback/project memories there per the auto-memory protocol; check existing memories before recommending design choices. Key memories: input system convention (Editor-authored IA/IMC, no runtime `NewObject`), prefer minimal fixes over defensive scaffolding, UE projects are C++ learning vehicles (explain UE/C++ concepts after completing a phase), all animations via Blender Python, true isometric orthographic camera.
