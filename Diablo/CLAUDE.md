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

**Plugins enabled** in `.uproject`: `StateTree`, `GameplayStateTree` (needed for AI in M19), `ModelingToolsEditorMode` (editor only), `DiabloEditor` (editor only).

## Architecture

### Core Classes (M1)

- `ADiabloGameMode` (Abstract) — sets `DefaultPawnClass` and `PlayerControllerClass` to C++ bases; BP subclass `BP_DiabloGameMode` overrides to BP versions
- `ADiabloHero : ACharacter` (Abstract) — isometric camera via `USpringArmComponent` (pitch -45°, yaw 225°, arm 1800, no collision test) + `UCameraComponent` (orthographic, OrthoWidth 2048). Capsule sized to 90 half-height, mesh offset -90 Z to align feet with capsule bottom. Uses `FDiabloStats` (HP 70/70 Warrior defaults). `TakeDamage` override → `Die()` on HP <= 0. `Heal(Amount)` for pickups. `IsDead()` check.
- `ADiabloPlayerController` (Abstract) — Enhanced Input click-to-move, click-to-attack, click-to-pickup. LMB on ground → `SimpleMoveToLocation`; LMB on enemy → stores `TargetEnemy`, walks into `AttackRange`, then calls `Hero->StartAttack()`; LMB on `ADroppedItem` → walks into `PickupRange`, calls `OnPickedUp`. UPROPERTY slots for `ClickAction` and `DefaultMappingContext` assigned via BP. `OnHeroDeath()` → disable input, 2s timer → `RestartPlayer` at `PlayerStart`.
- `UDiabloAnimInstance` (Abstract) — exposes `float Speed` from pawn velocity in `NativeUpdateAnimation`. AnimBP `ABP_Warrior` parents to this class (manual editor creation)

### Combat (M2)

- `ADiabloHero::StartAttack()` — plays `AttackMontage` via the DefaultSlot in the AnimBP. Sets `AttackTarget` so the notify knows who to damage. `bIsAttacking` flag prevents re-triggering mid-swing.
- `UAnimNotify_Attack` (DisplayName "Melee Attack Trace") — fires on a montage keyframe. Reads `AttackTarget` from either `ADiabloHero` or `ADiabloEnemy`. Hero→enemy: computes D1 To-Hit% (`50 + Dex/2 + CharLevel - MonsterAC + (CharLevel - MonsterLevel)`, clamped 5–95%), rolls, then deals `BaseDamage + Str/5`. Enemy→hero: flat `Damage` reduced by hero AC (`Dex/5`, min 1). Guards against dead targets.
- `ADiabloEnemy : ACharacter` (Abstract) — uses `FDiabloStats` for HP. `TakeDamage` override; on death plays `DeathMontage`, disables collision, delays `Destroy()` by montage duration + 2s. Has `StartAttack(Target)`, `AttackMontage`, `AttackTarget` for AI-driven attacks. Capsule explicitly blocks `ECC_Visibility` for cursor click detection.
- `AM_Attack` (AnimMontage) — manually authored in editor from `Warrior_Anim_Attack`. Contains Melee Attack Trace notify at the hit frame plus optional PlaySound notifies for SwordSwing/SwordHit.
- `AM_Death` (AnimMontage) — manually authored in editor from `Warrior_Anim_Death`. `bEnableAutoBlendOut = false` so the death pose holds. Used by both hero and enemy.

### Enemy AI (M3)

- `ADiabloAIController : AAIController` — tick-based state machine with `EAIState` enum (Idle / Chase / Attack / Dead). `FindTarget()` returns the player pawn (returns `nullptr` if hero is dead). Aggro at `AggroRange` (800), attacks at `AttackRange` (200), leashes at `LeashRange` (1500). Uses `MoveToActor` for pathfinding (only issued when `GetMoveStatus() != Moving` to prevent stutter).
- `ADiabloEnemy` sets `AIControllerClass = ADiabloAIController` and `AutoPossessAI = PlacedInWorldOrSpawned` in constructor.

### HP + Death + Respawn (M4)

- `FDiabloStats` ([DiabloStats.h](Source/Diablo/DiabloStats.h)) — USTRUCT with HP/MaxHP/Mana/MaxMana/Str/Mag/Dex/Vit. Used by both `ADiabloHero` and `ADiabloEnemy`. All fields active as of M6.
- **Hero death:** `TakeDamage` → `Die()` → disable movement, play `DeathMontage` → controller `OnHeroDeath()` → `DisableInput`, 2s `FTimerHandle` → `UnPossess` + `Destroy` dead pawn → `AGameModeBase::RestartPlayer` spawns fresh hero at `PlayerStart` with full HP → `EnableInput`.
- **Enemy death:** `TakeDamage` → HP <= 0 → disable collision + movement, play `DeathMontage` → delayed `Destroy()` after montage + 2s corpse linger.
- `ADroppedItem : AActor` ([DroppedItem.h](Source/Diablo/DroppedItem.h)) — `UStaticMeshComponent` root (plane mesh with sprite material, rotated to face isometric camera) that blocks `ECC_Visibility` for cursor detection. `HealAmount = 50`. `OnPickedUp(Hero)` adds `ItemData` to inventory if valid, else falls back to heal-and-destroy. `InitFromItem(FItemInstance)` configures mesh + dynamic material for runtime-spawned drops. BP subclass `BP_HealingPotion` uses `T_HealingPotion` sprite from SVG pipeline.

### HUD (M5)

- `UDiabloHUDWidget : UUserWidget` ([DiabloHUDWidget.h](Source/Diablo/DiabloHUDWidget.h)) (Abstract) — builds widget tree in `RebuildWidget()` override (not `NativeConstruct` — that's too late for the Slate layer). Red life globe (bottom-left, `UProgressBar` BottomToTop fill), blue mana globe (bottom-right), gold XP bar (bottom-center, stretches between globes), level text (centered above XP bar). **Event-driven** via `FOnStatsChanged` delegate — no `NativeTick` polling.
- `ADiabloPlayerController` owns the HUD: `HUDWidgetClass` UPROPERTY (set via `SetupHUD` editor tool → `BP_DiabloHUD`). `CreateHUD()` in `BeginPlay` → `CreateWidget` + `AddToViewport`. `OnHeroDeath()` collapses widget. `OnPossess()` re-binds to new hero and shows widget after respawn.
- `BP_DiabloHUD` — Widget Blueprint created via `UWidgetBlueprintFactory` (not `FKismetEditorUtilities::CreateBlueprint` — widget BPs need the proper factory). No event-graph nodes — exists purely as an asset reference for `HUDWidgetClass`.

**Do not generate Widget Blueprints programmatically via the UMG designer graph** — same fragility as AnimBPs. Build widget trees in C++ (UCanvasPanel + UProgressBar + USizeBox), use the BP only as a thin subclass. Use `RebuildWidget()` to populate the widget tree before Slate construction, not `NativeConstruct()`.

### XP + Leveling (M6)

- `FOnStatsChanged` multicast delegate on `ADiabloHero` — broadcast from `TakeDamage`, `Heal`, `AwardXP`. HUD subscribes via `AddUObject` in `InitForHero`, unsubscribes in `NativeDestruct`. No polling.
- **XP table:** `GetXPTable()` returns a static `TArray<int64>` with 51 entries (indices 0–50). Quartic curve approximation: `Table[L] = Table[L-1] + 2000 * L^3 / 4`. Uses `int64` because L49→50 exceeds `int32` max.
- **Level-up:** `AwardXP` loops `while (CurrentXP >= GetXPForNextLevel())` → `LevelUp()` grants +5 `UnspentStatPoints`, calls `RecomputeDerivedStats()`, restores HP/Mana to full, plays `LevelUpSound`. Level cap 50.
- **Derived stats:** `RecomputeDerivedStats()` computes `MaxHP = 70 + (Vit-25)*2 + (Level-1)*2`, `MaxMana = 10 + (Mag-10)*1 + (Level-1)*1` (Warrior formulae). Called on level-up; will also be called from equip/unequip in M9.
- **Warrior starting stats:** Str 30, Mag 10, Dex 20, Vit 25, HP 70/70, Mana 10/10 (D1 reference). Class caps: Str 250, Mag 50, Dex 60, Vit 100.
- **Enemy XP:** `ADiabloEnemy` has `XPReward` (default 100) and `MonsterLevel` (default 1). On death, awards XP to player hero if `CharLevel - MonsterLevel < 10` (D1 zeroing rule).
- `LevelUpSound` — `USoundWave` UPROPERTY on hero, set via `SetupHero`. Generated by `Tools/audio/levelup_sfx.py`, imported via `ImportLevelUpSFX`.
- **Stat allocation:** `SpendStatPoint(FName)` increments one stat by 1, decrements `UnspentStatPoints`, respects Warrior class caps, calls `RecomputeDerivedStats` + broadcasts `OnStatsChanged`.
- `UDiabloCharacterPanel : UUserWidget` ([DiabloCharacterPanel.h](Source/Diablo/DiabloCharacterPanel.h)) (Abstract) — C++ widget tree (VerticalBox with stat rows, + buttons, HP/Mana display). Toggle via **C key** (`IA_CharPanel`). + buttons hidden when no unspent points. Event-driven via `FOnStatsChanged`.
- `ADiabloPlayerController` owns the character panel: `CharPanelClass` UPROPERTY (set via `SetupHUD` → `BP_DiabloCharPanel`). Toggle in `OnToggleCharPanel()` switches between `Collapsed` / `Visible` + `GameAndUI` input mode.

### Inventory (M7)

- `UItemDefinition : UPrimaryDataAsset` ([ItemDefinition.h](Source/Diablo/ItemDefinition.h)) — icon, display name, grid size (w×h), stackable, equip slot, category (`EItemCategory`), base stats (BonusStr/Mag/Dex/Vit, MinDamage/MaxDamage, ArmorClass), durability, gold value, heal amount. Data assets live under `Content/Items/Definitions/` (prefix `ID_`).
- `FItemInstance` ([ItemInstance.h](Source/Diablo/ItemInstance.h)) — `UItemDefinition*` + `TArray<FItemAffix>` (empty for now, M17) + current durability + stack count.
- `EEquipSlot` — Head, Chest, LeftHand, RightHand, LeftRing, RightRing, Amulet (7 slots).
- `EItemCategory` — Misc, Weapon, Armor, Shield, Helm, Ring, Amulet, Potion, Scroll, Gold.
- `UInventoryComponent : UActorComponent` ([InventoryComponent.h](Source/Diablo/InventoryComponent.h)) — 10×4 grid, 7 equipment slots (`TMap<EEquipSlot, FItemInstance>`), gold. Occupancy grid tracks multi-cell items. API: `TryAddItem`, `TryAddItemAt`, `MoveItem`, `RemoveItemAt`, `Equip`, `Unequip`, `UseItem`, `AddGold`, `SpendGold`. `UseItem` consumes potions (heal + decrement stack) or falls back to `Equip` for equippable items. Broadcasts `FOnInventoryChanged` delegate.
- `ADiabloHero` has `UInventoryComponent* Inventory` (created in constructor via `CreateDefaultSubobject`).
- `ADroppedItem` has `FItemInstance ItemData` — if valid, `OnPickedUp` adds to inventory via `TryAddItem`; if invalid, falls back to legacy `HealAmount` heal-and-destroy.
- `UDiabloInventoryPanel : UUserWidget` ([DiabloInventoryPanel.h](Source/Diablo/DiabloInventoryPanel.h)) (Abstract) — C++ widget tree (hover item name, equipment slots row, 10×4 grid, gold display). Toggle via **I key** (`IA_Inventory`). Drag-drop via panel-level `NativeOnMouseButtonDown`/`NativeOnDragDetected`/`NativeOnDrop` with `UInventoryDragDrop` operation. Right-click uses items (potions consume, equippables equip) or unequips (equipment slot). `NativeOnMouseMove` updates gold hover text with item name. Hit-testing uses `GetTickSpaceGeometry().AbsoluteToLocal()` (not `GetPaintSpaceGeometry` — coordinate space mismatch). Event-driven via `FOnInventoryChanged`.
- `ADiabloPlayerController` owns the inventory panel: `InventoryPanelClass` UPROPERTY (set via `SetupHUD` → `BP_DiabloInventoryPanel`), `InventoryAction` (set via `SetupController` → `IA_Inventory`). Toggle in `OnToggleInventory()`.
- Icon sprites generated via SVG pipeline (`Tools/svg/`) → Inkscape CLI conversion → imported via `ImportItemIcons`.
- `SetupInventory` creates starter `UItemDefinition` data assets (Short Sword, Buckler, Skull Cap, Rags, Ring of Strength, Healing Potion) with icon texture references.

### Loot Drops (M8)

- `FDropTableEntry` struct ([DiabloEnemy.h](Source/Diablo/DiabloEnemy.h)) — `UItemDefinition*` + `DropChance` (0–1 roll) + `Weight` (for future weighted selection).
- `ADiabloEnemy` has `TArray<FDropTableEntry> DropTable`. On death, `SpawnDrops()` iterates entries, rolls `DropChance`, and spawns `ADroppedItem` at the enemy's location with a random XY offset.
- `ADroppedItem::InitFromItem(FItemInstance)` — sets `ItemData` and configures the mesh component (plane mesh with `M_ItemDrop` dynamic material instance using the item's icon texture).
- `M_ItemDrop` — unlit translucent material with a `TextureSampleParameter2D` named "Texture", created by `SetupDropMaterial`. Used at runtime via `UMaterialInstanceDynamic::SetTextureParameterValue`.
- `ADroppedItem` is no longer `Abstract` — can be spawned directly at runtime for loot drops. `BP_HealingPotion` still exists as a hand-placed subclass.
- `SetupEnemy` configures `BP_DiabloEnemy` drop table: Healing Potion (50%), Short Sword (25%), Ring of Strength (15%).

### Equipment Stats (M9)

- `Equip` / `Unequip` in `UInventoryComponent` call `ADiabloHero::RecomputeDerivedStats()` + broadcast `OnStatsChanged` so the HUD updates immediately.
- `RecomputeDerivedStats()` iterates all equipped `FItemInstance`s, sums `BonusStr/Mag/Dex/Vit` (applied to MaxHP/MaxMana formulae), and populates `EquipMinDamage`/`EquipMaxDamage`/`ArmorFromEquipment` via `switch` on `EItemCategory` (Weapon → damage range, Armor/Shield/Helm → AC, Ring/Amulet → flat bonuses only).
- **Hero attack damage:** `UAnimNotify_Attack` checks `EquipMaxDamage > 0` — if a weapon is equipped, rolls `FRandRange(EquipMinDamage, EquipMaxDamage) + Str/5`; unarmed falls back to the notify's flat `Damage + Str/5`.
- **Hero AC:** `TakeDamage` uses `Dex/5 + ArmorFromEquipment` to reduce incoming damage (min 1).
- No polymorphic dispatch — item categories are an enum, stat contributions are fields on `UItemDefinition`.

### Spells (M10–M11)

- `USpellDefinition : UPrimaryDataAsset` ([SpellDefinition.h](Source/Diablo/SpellDefinition.h)) — data-driven spell: `DisplayName`, `ManaCost`, `Cooldown`, `Damage`, `ProjectileClass` (`TSubclassOf<ASpellProjectile>`), `bIsProjectile`, `HealAmount`. Data assets live under `Content/Spells/Definitions/` (prefix `SD_`).
- `ASpellProjectile : AActor` ([SpellProjectile.h](Source/Diablo/SpellProjectile.h)) (Abstract) — base projectile with `USphereComponent` (overlap-all-dynamic), `UProjectileMovementComponent` (zero gravity, rotation follows velocity). `OnOverlap` damages enemies and destroys self. `InitialLifeSpan = 5s`.
- **Projectile subclasses:** `AFirebolt` (small sphere, fast), `AFireball` (large sphere, slow, high damage), `ALightningBolt` (narrow cube, very fast). Each sets its own mesh placeholder in constructor.
- **Instant spells:** Nova (AoE `OverlapMultiByChannel` within 500 units, damages all enemies), Healing (calls `Heal()` on self). Dispatched via `bIsProjectile` / `HealAmount` in `CastSpell`.
- `ADiabloHero::KnownSpells` — `TArray<USpellDefinition*>`, set via `SetupHero` from spell definitions. `ActiveSpell` is the currently bound RMB spell. `SetActiveSpell()` changes it.
- `ADiabloHero::CastSpell(TargetLocation)` — reads `ManaCost`/`Cooldown` from `ActiveSpell`. Checks mana, checks cooldown, deducts mana. For projectiles: spawns from `ProjectileClass`, overrides `Damage` from the definition. For instant: heals or does AoE. `SpellCooldownRemaining` decremented in `Tick`.
- **RMB cast:** `IA_Cast` mapped to RMB. `ADiabloPlayerController::OnCastStarted()` traces cursor, calls `CastSpell(ImpactPoint)`.
- `UDiabloSpellbookPanel : UUserWidget` ([DiabloSpellbookPanel.h](Source/Diablo/DiabloSpellbookPanel.h)) (Abstract) — C++ widget tree listing known spells. Toggle via **S key** (`IA_Spellbook`). RMB-click a spell to bind it to the active (RMB) slot. Active spell highlighted in gold. Anchored top-right.
- `SetupSpells` creates 5 `USpellDefinition` data assets: Firebolt, Fireball, Lightning, Nova, Healing.

### Dungeon + Level Transitions (M12)

- `ADungeonStairs : AActor` ([DungeonStairs.h](Source/Diablo/DungeonStairs.h)) — cube mesh that blocks `ECC_Visibility` for cursor click detection. `TargetLevelName` (FName) set per-instance. `OnInteract()` calls `UGameplayStatics::OpenLevel`. Click-to-interact uses the same walk-into-range pattern as enemies/items; `InteractRange = 200`.
- `ADiabloPlayerController` has `TargetStairs` — click stairs → walk into range → `OnInteract()` triggers level transition. Checked in `Tick` before items/enemies.
- `Lvl_Diablo` (town) has `Stairs_Down` pointing to `Lvl_Cathedral_L1`. Cathedral has `Stairs_Up` pointing back to `Lvl_Diablo`.
- `GenerateCathedralMap` creates `Lvl_Cathedral_L1` with dim lighting, cube-wall corridors, 3 enemies, a healing potion, NavMesh, and return stairs.
### Persistent State (M13)

- `UDiabloGameInstance : UGameInstance` ([DiabloGameInstance.h](Source/Diablo/DiabloGameInstance.h)) — persists across `OpenLevel` transitions. Holds `bHasSavedState` flag, `SavedStats`, `SavedCharLevel`, `SavedCurrentXP`, `SavedUnspentStatPoints`, full inventory state (grid items, occupancy, equipped items, gold), `SavedKnownSpells`, `SavedActiveSpell`.
- `ADiabloHero::SaveToGameInstance()` — copies all hero + inventory state into the `UDiabloGameInstance`. Called by `ADungeonStairs::OnInteract()` just before `OpenLevel`.
- `ADiabloHero::BeginPlay()` → `LoadFromGameInstance()` — if `bHasSavedState` is true, restores stats, inventory, and spells from the GameInstance. Called automatically when the hero spawns in a new level.
- `UInventoryComponent::RestoreState()` — bulk-replaces grid items, occupancy, equipped items, and gold.
- `GameInstanceClass` set in `DefaultEngine.ini` → `/Script/Diablo.DiabloGameInstance`.

### Input

**Enhanced Input only.** No legacy `InputComponent` axis bindings.

- IA assets generated by the DiabloEditor plugin under `Content/Input/Actions/` (`IA_Click` mapped to LMB, `IA_Cast` mapped to RMB, `IA_CharPanel` mapped to C, `IA_Inventory` mapped to I, `IA_Spellbook` mapped to S, `IA_Move`, `IA_Look`). IMC at `Content/Input/IMC_Diablo`.
- **Don't create IA/IMC at runtime via `NewObject<>`** — assign the editor assets to BP UPROPERTY slots.
- `ADiabloPlayerController` holds `TObjectPtr<UInputAction>` and `TObjectPtr<UInputMappingContext>` slots, binds in `SetupInputComponent`.
- No touch controls — desktop-only (Diablo 1 is mouse+keyboard).

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

| Method | Creates | Configures |
|---|---|---|
| `SetupHero` | `BP_DiabloHero` | Skeletal mesh, anim class, attack montage, death montage, LevelUpSound, KnownSpells, ActiveSpell |
| `SetupController` | `BP_DiabloPlayerController` | ClickAction, CastAction, CharPanelAction, InventoryAction, SpellbookAction, DefaultMappingContext |
| `SetupGameMode` | `BP_DiabloGameMode` | DefaultPawnClass, PlayerControllerClass |
| `SetupEnemy` | `BP_DiabloEnemy` | Skeletal mesh, anim class, attack montage, death montage, drop table |
| `SetupPotion` | `BP_HealingPotion` | Plane mesh with sprite material (upright, facing isometric camera) |
| `SetupHUD` | `BP_DiabloHUD` + `BP_DiabloCharPanel` + `BP_DiabloInventoryPanel` + `BP_DiabloSpellbookPanel` (WidgetBlueprints) | Creates WBPs parented to C++ widget classes, sets `HUDWidgetClass`, `CharPanelClass`, `InventoryPanelClass`, `SpellbookPanelClass` on `BP_DiabloPlayerController` |
| `SetupInventory` | Starter `UItemDefinition` data assets | Creates ID_Short_Sword, ID_Buckler, ID_Skull_Cap, ID_Rags, ID_Ring_of_Strength, ID_Healing_Potion under `/Game/Items/Definitions/` |
| `SetupDropMaterial` | `M_ItemDrop` material | Unlit translucent material with `TextureSampleParameter2D` for runtime dropped item sprites |
| `SetupSpells` | `USpellDefinition` data assets | Creates SD_Firebolt, SD_Fireball, SD_Lightning, SD_Nova, SD_Healing under `/Game/Spells/Definitions/` |
| `SetupAllBlueprints` | All of the above | All of the above |

**World:**

| Method | Behavior |
|---|---|
| `GenerateDefaultMap` | Creates `Lvl_Diablo` with PlayerStart, DirectionalLight, floor plane, NavMeshBoundsVolume, test enemy, healing potion, stairs to cathedral — **always recreates** |
| `GenerateCathedralMap` | Creates `Lvl_Cathedral_L1` with dim lighting, cube-wall corridors, 3 enemies, healing potion, NavMesh, return stairs — **always recreates** |
| `GenerateInputAssets` | Creates/updates IA_Click/Move/Look and IMC_Diablo — **updates in place** |

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

**Editor-only assets** that require the DiabloEditor plugin or editor interaction:
- **BP subclasses** under `Content/Blueprints/` — every `UCLASS(Abstract)` C++ class needs a BP child to be placed
- **Levels** under `Content/Maps/` — generated by editor tool or hand-authored
- **Input assets** under `Content/Input/` — generated by editor tool
- **AnimBlueprints** — manually authored in editor; parent to C++ `UDiabloAnimInstance` subclass. `ABP_Warrior` has a Locomotion state machine (Idle/Walk based on `Speed`) with a DefaultSlot node for montage overlay.
- **AnimMontages** — `AM_Attack` created manually from `Warrior_Anim_Attack` with Melee Attack Trace and PlaySound notifies. `AM_Death` created manually from `Warrior_Anim_Death` with `bEnableAutoBlendOut = false`. Both set on hero and enemy BPs via `SetupHero`/`SetupEnemy`.
- **Widget Blueprints** — `BP_DiabloHUD` under `Content/Blueprints/`; parents to C++ `UDiabloHUDWidget`. Widget tree built entirely in C++ — BP is a thin asset-reference shell.
- **StateTree assets** — needed starting M19; reference C++ task/condition structs

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
