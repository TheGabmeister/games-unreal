# SPEC — FF7 (1997) in Unreal Engine 5.7

A gameplay-focused recreation of Final Fantasy VII (1997) built in Unreal Engine 5.7. The project exists to learn C++ in Unreal, so features are scoped to mechanics (not assets or story fidelity). Characters are rendered as placeholder primitive shapes structured so skeletal meshes drop in later. Implementation is C++-first; Slate drives menus; Blueprints are thin wrappers for designer tuning and asset references only.

[TASKS.md](TASKS.md) is the phase roadmap; it references sections of this document rather than restating them. When an implementation choice changes, update it here and the phase inherits it.

---

## 1. Game features

### Party

- Roster of 9 characters; 3 are *active* (battle party) at any time.
- Swapping the active party happens at save points via a **PHS** menu (Party Handler System).
- Characters share inventory, gil, and world progress; stats/equipment/materia are per-character.
- KO'd, Petrified, or otherwise incapacitated members do not act in combat.

### Stats & leveling

- Per-character stats: **HP, MP, Strength, Vitality, Magic, Spirit, Dexterity, Luck**.
- Derived stats: Attack, Defense, Mag.Attack, Mag.Defense, Accuracy, Evade.
- EXP is awarded to the active party after battle; levels drive base-stat curves defined in a DataTable.
- AP is awarded to equipped materia and unlocks new spells/abilities at tier thresholds.

### ATB combat

- Active Time Battle: each combatant has a gauge that fills at a rate driven by Dexterity/Speed + Haste/Slow modifiers.
- When the gauge tops out, the combatant can act; **Wait** mode pauses the gauge while menus are open, **Active** mode keeps time flowing.
- Separate battle level: encounters transition out of the field map into a dedicated battle arena (1997-style). Party/inventory/world flags persist via GameInstance.
- Action set: **Attack, Magic, Summon, Item, Defend, Limit, Escape, <Command-materia actions>**.
- Targeting supports single-enemy, single-ally, all-enemies, all-allies, random, and self.
- Turn outcome: damage formulas produce HP/MP deltas; status effects apply/tick/expire; victory grants EXP/AP/gil/item drops; defeat returns to the last save.
- Battle-start modifiers (pre-emptive / back attack / side attack / pincer) are out of scope; encounters always start neutral.

### Materia

Families:
- **Magic** — grants spells (Fire, Cure, Haste…) at AP thresholds.
- **Command** — grants menu commands (Steal, Throw, Mime…).
- **Summon** — grants a summon ability with limited per-battle uses.
- **Support** — modifies a *linked* adjacent materia (All, Quadra Magic, MP Turbo…).
- **Independent** — passive stat/effect modifiers (HP Plus, Counter, Long Range…).

Rules:
- Equipment has **materia sockets**, some of which are *linked pairs*. Support materia only affects its paired slot.
- Materia grants its effects *while equipped*; unequipping reverts the grant.
- Materia retains AP across equip/unequip and levels independently of the character.

### Equipment

- Three slots per character: **Weapon, Armor, Accessory**.
- Each equipment piece defines: stat modifiers, materia sockets (with link layout), elemental/status affinities, character restriction.
- Accessories grant passive effects (status immunity, auto-regen, haste start-of-battle, etc.).

### Magic, Summons, Limit Breaks

- **Magic** — cast from the Magic command; MP cost + cast animation + effect. Organized by element and tier (Fire/Fira/Firaga).
- **Summons** — invocations with a per-battle use cap (seeded by materia level); stronger than single-tier spells.
- **Limit Breaks** — per-character, unique moves. A **limit gauge** fills when the character takes damage. Each character has multiple **tiers**, each with **two branches**; tiers unlock by kill count / branch usage (classic FF7 rules).

### Status effects

- Buffs: Haste, Barrier, MBarrier, Regen, Shield, Wall, Reflect, Resist, Peerless, Berserk.
- Debuffs/ailments: Poison, Sleep, Confuse, Silence, Slow, Stop, Frog, Small, Paralysis, Darkness, Petrify, Death-sentence, Manipulate, Fury, Sadness.
- Each effect defines duration, stat modifiers, per-tick callbacks, and remove-on-events.

### Inventory, shops, save points

- Flat inventory of consumables, equipment, and key items with stack counts.
- **Gil** (currency) on `UFF7GameInstance`; earned from battle, spent at shops.
- Shops (buy/sell) at field locations; stock defined per shop in a DataTable.
- **Save points** restore HP/MP to full when a **Tent** is used, open the save menu, open the PHS.

### Field exploration & dialogue

- Top-down/isometric camera (fixed spring-arm rig; per-area overrides via camera-volume actors if needed).
- NPCs are interactable actors with dialogue popups and world event flags (TMap of FName → int) gating progression.
- Overlap-volume triggers fire scripted events (start battle, set flag, open cutscene placeholder).

### World map & vehicles

- Separate overworld level; pawns switch via possession to alternate vehicle pawns:
  - **Chocobo** — rental mount; different movement profile per breed.
  - **Buggy**, **Tiny Bronco** (air), **Highwind** (airship), **Submarine**.
- Vehicles restrict/enable traversal (water, mountains, forests).

### Save / Load

- Slot-based (3 slots). Saves party roster, active party, per-character stats/equipment/materia AP, inventory, gil, world flags, current level, player-start tag.
- Versioned header so format changes are detected, not silently corrupted.

### Audio & music (stub today, hot-swap later)

- Track ID (FName) indirection through a DataTable of soft `USoundBase` refs. BGM slot with crossfade; SFX one-shot pool; jingles interrupt+resume BGM.
- All asset references may be null — the subsystem logs missing tracks and remains silent, never crashes. Drop in a WAV later, the row fills, playback works.
- Gameplay fires multicast delegates (`OnBattleStart`, `OnBattleVictory`, `OnLevelUp`, `OnMenuOpen`, `OnFieldEnter`); the audio subsystem binds to them and never references assets directly.

### Out of scope

Explicitly not planned, to keep the learning arc focused:
- Story content, cutscenes, pre-rendered backgrounds, FMV.
- Row (Front/Back) positional damage modifier.
- Battle-start modifiers (pre-emptive, back attack, side attack, pincer).
- Weapon AP-growth multipliers (all weapons pass AP through 1:1 for now).
- Morph / W-Item / W-Magic / W-Summon (double-cast command-materia variants).
- Manipulate enemy control, Sense damage inspection.
- Sadness/Fury as proper long-term stats (Berserk still used as a battle-scope status).
- Network / multiplayer.

### Stretch (late phase, scaffolding only)

- **Mini-games**: Chocobo racing, snowboarding, Fort Condor — track pawn with alternate input mapping + mini-game transition actor.
- **Chocobo breeding**: data model (parent refs, offspring rating, stat inheritance); full content out of scope.

---

## 2. Implementation reference

This section is the source of truth for class names, APIs, data layouts, and architectural choices. [TASKS.md](TASKS.md) phases implement these sections; when a detail changes, change it here.

### 2.1 Build and module layout

- Single `FF7` runtime module. Split into `FF7Core` / `FF7UI` / `FF7Combat` later if compile times hurt.
- `.Build.cs` deps: `Core`, `CoreUObject`, `Engine`, `InputCore`, `EnhancedInput`, `Slate`, `SlateCore`, `UMG`, `GameplayTags`, `DeveloperSettings`. Test-only: `AutomationController`, `FunctionalTesting`.
- Module lifecycle in [Source/FF7/FF7.cpp](Source/FF7/FF7.cpp); the Slate style set (§2.8) registers in `StartupModule` and unregisters in `ShutdownModule`.
- `DefaultEngine.ini` → `[/Script/EngineSettings.GameMapsSettings]`:
  - `GameInstanceClass=/Script/FF7.FF7GameInstance`
  - Default editor/game map points to `L_TestChamber`.
- Build commands: see [CLAUDE.md](CLAUDE.md).

### 2.2 Character representation

- `AFF7CharacterBase : AActor` — capsule collision + `USceneComponent` visual root with placeholder `UStaticMeshComponent` children (cube body, cube head, direction indicator).
- Replacing placeholders with a `USkeletalMeshComponent` later is an isolated edit to the visual root; no gameplay code changes.
- `AFF7PartyMemberActor : AFF7CharacterBase` — party member in battle arenas.
- `AFF7EnemyActor : AFF7CharacterBase` — enemy unit.
- Both implement `IFF7Damageable` and `IFF7StatusTarget` (§2.10) so battle code is side-agnostic.

### 2.3 Player pawn and camera

- `AFF7PlayerPawn : AFF7CharacterBase` — adds `USpringArmComponent` + `UCameraComponent` configured for fixed top-down (~60° pitch, fixed arm length).
- `AFF7PlayerController : APlayerController` — routes input, manages menu open/close, possesses pawns.
- `BP_FF7PlayerController` holds soft refs to `UInputMappingContext` and `UInputAction` assets; C++ applies the IMC on possession and binds actions.
- Input actions:
  - `IA_Move` (Vector2D, WASD + left stick)
  - `IA_Interact` (bool, E / A button)
  - `IA_MenuToggle` (bool, Tab / Start)
  - `IA_Escape` (bool, battle Escape hold)
- `AFF7FieldGameMode` — field-level rules (encounter ticking, interact). Default pawn/controller set via `BP_FF7FieldGameMode`.

### 2.4 NPC and dialogue

- `IFF7Interactable` (UINTERFACE) — `void Interact(AFF7PlayerController* By)` dispatched via `Execute_Interact`.
- `AFF7NPCActor : AActor` — capsule + placeholder mesh + `USphereComponent` interact volume; implements `IFF7Interactable`.
- On `IA_Interact`, the controller line-traces forward against a custom `Interact` trace channel, falling back to closest overlap.
- `FDialogueLineRow : FTableRowBase { FName SpeakerId; FText Line; FName NextId; }` in `DT_Dialogue` drives content from day one.
- `SFF7DialoguePopup` (`SCompoundWidget`, §2.8) — bottom panel, speaker + text, advances on Interact, closes on terminal `NextId`.

### 2.5 Party and game state

- `UFF7GameInstance : UGameInstance` — persistent across level loads. Owns:
  - `TArray<FPartyMember> Roster` (size 9, populated in `Init`)
  - `TArray<int32> ActivePartyIndices` (size 3; default `[0,1,2]`)
  - `int32 Gil` with `AddGil(int32)`, `SpendGil(int32)`, `OnGilChanged` delegate
  - `TMap<FName, int32> WorldFlags`
  - `TOptional<FReturnContext> PendingReturn` where `FReturnContext { FName LevelName; FTransform PlayerTransform; }` — survives `OpenLevel` so the field GM can teleport the pawn back.
  - Owned subobjects: `UFF7Inventory` (§2.7), audio subsystem hooks (§2.15).
- `FPartyMember` (USTRUCT) — `FName CharacterId`, `FCharacterStats Stats`, `UFF7Equipment* Equipment`, `FLimitGauge Limit`.
- Game modes (subclasses of `AGameModeBase`):
  - `AFF7FieldGameMode`
  - `AFF7BattleGameMode`
  - `AFF7WorldMapGameMode`
- Exec console commands on `UFF7GameInstance`:
  - `FF7.Party.Dump`, `FF7.Party.SwapActive <slotIdx> <rosterIdx>`
  - `FF7.Party.GrantEXP <slotIdx> <amount>` (§2.6)
  - `FF7.Inv.Grant <id> <n>`, `FF7.Inv.Use <id> <slotIdx>`, `FF7.Inv.Dump`, `FF7.Inv.GrantMateria <id>` (§2.7, §2.9)

### 2.6 Stats and leveling

- `FCharacterStats` (USTRUCT) — `HP, MaxHP, MP, MaxMP, Str, Vit, Mag, Spr, Dex, Lck, Level, EXP`.
- Derived stats (Attack, Defense, MAttack, MDefense, Accuracy, Evade) are recomputed from base stats + equipment on change, not per tick.
- `FLevelCurveRow : FTableRowBase` — one row per level per character: `Level, EXPToNext, HP, MP, Str, Vit, Mag, Spr, Dex, Lck`.
- CSV-imported `DT_LevelCurve_<Name>` under `Content/Data/`.
- `UFF7LevelCurve` (static utility, not yet a `UBlueprintFunctionLibrary`): `ApplyLevelUp(FCharacterStats&, const UDataTable* Curve)`, `AwardEXP(FCharacterStats&, int32 EXP, const UDataTable* Curve)`. Pure functions — fully testable.

### 2.7 Inventory and Gil

- `UFF7Inventory : UObject` — plain UObject owned by `UFF7GameInstance` (not a `UActorComponent`; GameInstance isn't an `AActor`).
- API: `AddItem(FName RowId, int32 Count)`, `RemoveItem(FName RowId, int32 Count)`, `int32 GetCount(FName RowId) const`; `OnInventoryChanged` multicast delegate.
- Storage: `TMap<FName, int32>`. The same map stores both consumables (resolved via `DT_Items`) and equipment pieces (resolved via `DT_Equipment`, §2.9).
- `FItemRow : FTableRowBase` — `Id, DisplayName, ItemType (Consumable/Key/Misc), GilValue, EffectTag`.
- `UFF7ItemEffects` (static library) — dispatchers keyed by `EffectTag` (`ApplyPotion`, `ApplyPhoenixDown`, …), operating on `FCharacterStats`.
- Gil lives on `UFF7GameInstance` (§2.5), not in inventory — gil is scalar, not a row count.

### 2.8 UI Slate framework

- All menus are Slate (`SCompoundWidget` subclasses). A thin UMG wrapper is used only for HUD overlays that need designer tuning.
- `FFF7MenuStyle : FSlateStyleSet` — central tokens (fonts, colors, padding); registered in `StartupModule`, unregistered in `ShutdownModule`.
- Widgets attach to the viewport via `GEngine->GameViewport->AddViewportWidgetContent`; lifetime held by `TSharedPtr` on the owning subsystem/controller.
- `IA_MenuToggle` on the PlayerController opens/closes `SFF7MainMenu` and sets a pause flag on the field GameMode.
- Widget inventory:
  - `SFF7MainMenu` — tabs: Item, Magic, Materia, Equip, Status, Config, Save, PHS.
  - `SFF7StatusScreen`, `SFF7EquipScreen`, `SFF7MateriaScreen`, `SFF7ShopMenu`, `SFF7PHSMenu`, `SFF7SaveSlotList`.
  - `SFF7DialoguePopup` (§2.4).
  - `SFF7BattleHUD`, `SFF7TargetPicker` (§2.10).
- Live binding via `TAttribute<T>` closures; change notifications via multicast delegates.

### 2.9 Equipment and Materia

- `UFF7Equipment : UObject` — owned per `FPartyMember`; three `FEquippedItem { FName EquipRowId; TArray<UFF7MateriaInstance*> Sockets; }` (Weapon, Armor, Accessory).
- `FEquipmentRow : FTableRowBase` — `Id, Slot enum, StatMods, uint8 SocketCount, uint16 LinkMask` (bit N set = socket N linked to N+1), elemental/status affinities, character restriction.
- `UFF7MateriaDataAsset : UPrimaryDataAsset` — `EMateriaFamily, DisplayName, TArray<FMateriaTier>` (AP threshold → granted ability id).
- `UFF7MateriaInstance : UObject` — hard `UFF7MateriaDataAsset* Def`, `int32 CurrentAP`, computed `int32 Tier`. Hard ref because the instance is useless without its def loaded; SaveGame (§2.14) persists the def by `FSoftObjectPath` and re-resolves on load.
- Equip → stat modifier application: recompute-on-change, not per-tick. Materia grants its effects while equipped; unequipping reverts.
- Support materia effects only fire when paired via `LinkMask` to an adjacent socket.

### 2.10 Combat framework

- `UFF7BattleSubsystem : UTickableWorldSubsystem` — lives only in the battle world; owns combatants, action queue, turn state, delegates. The subsystem itself ticks; no separate ticking actor.
- State machine: `EBattleState { SelectAction, Executing, Resolving, Victory, Defeat }` owned by the subsystem.
- `UFF7BattleAction : UObject` — base class for concrete actions (Attack, Magic, Item, Limit, Summon, Escape). Virtual `Execute(FBattleContext&)`.
- Interfaces:
  - `IFF7Damageable` — `TakePhysicalDamage(...)`, `TakeMagicalDamage(...)`.
  - `IFF7StatusTarget` — `ApplyStatus(...)`, `RemoveStatus(...)`, `HasStatus(...)`.
- Multicast delegates: `OnBattleStart`, `OnBattleVictory`, `OnBattleDefeat`, `OnCombatantActed`, `OnDamageDealt`, `OnGaugeFull`, `OnATBModeChanged`.
- `AFF7BattleGameMode` spawns active party + encounter enemies on BeginPlay. On victory it sets `PendingReturn` and calls `OpenLevel(PendingReturn.LevelName)`; `AFF7FieldGameMode::BeginPlay` checks for a pending return, teleports the possessed pawn to `PlayerTransform`, and clears it. (`OpenLevel` doesn't carry a transform — hence the re-apply.)
- `AFF7EncounterTrigger : AActor` — overlap volume; on player overlap, populates `PendingReturn` with current field + pawn transform, then `OpenLevel(L_BattleArena)`.
- PlayerController (same class as field, §2.3) hosts `SFF7BattleHUD` + `SFF7TargetPicker`. No separate battle controller class.
- Escape rolls against a simple placeholder formula until Phase-15 polish replaces it; always fails against bosses.

### 2.11 ATB

- `FATBGauge` per combatant, range `0..1000`. Fill rate `= f(Dex) * HasteMod` per second.
- `UFF7BattleSubsystem::Tick(float DeltaTime)` advances gauges; a gauge reaching max enqueues its owner into the action queue (ties broken by Luck, then insertion order).
- Wait/Active is a flag on `UFF7GameInstance.Config`; opening a Slate menu sets pause when Wait. `OnATBModeChanged` fires on toggle.
- Battle HUD binds to `OnGaugeFull` / `OnATBModeChanged` — never polls.

### 2.12 Magic, abilities, status effects

- `FSpellRow : FTableRowBase` — `Id, MPCost, Power, Element, TargetType, StatusApplied (optional)`.
- `UFF7DamageCalc : UBlueprintFunctionLibrary` — physical + magical damage formulas; readable FF7-ish, not pixel-accurate. Pure static functions, fully unit-testable.
- `FStatusEffect` + `UFF7StatusLibrary` — apply/tick/remove: Poison DOT, Regen HOT, Haste/Slow gauge modifier, Sleep cured by damage, Berserk forces Attack only.
- Status identity via Gameplay Tags (`Status.Poison`, `Status.Haste`, …) for later resistance/counter logic.
- Magic command in battle enumerates known spells from currently equipped materia (§2.9); MP-gated.
- Visual effects are placeholder primitive flashes on target this phase.

### 2.13 Limit Breaks

- `FLimitGauge` per party member, range `0..255`, fills on damage taken (bound to `OnDamageDealt` in §2.10).
- `UFF7CharacterDefinition : UPrimaryDataAsset` — per-character starting stats, weapon restriction, Limit branches, tier unlock conditions (kill counts, branch usage). Also the home for other per-character constants earlier phases implicitly assumed.
- `UFF7LimitAction : UFF7BattleAction` subclasses — Braver, Cross-Slash (Cloud), Beat Rush (Tifa), etc.
- When a party member's gauge is full, Limit replaces Attack in their command menu. Executing resets the gauge.
- Delegates: `OnLimitGaugeChanged`, `OnLimitReady`.

### 2.14 Save / Load

- `UFF7SaveGame : USaveGame` — fields tagged `UPROPERTY(SaveGame)`. Serializes party, inventory, equipment (by row id), materia defs (as `FSoftObjectPath`) + AP, gil, world flags, current level, player-start tag, save version.
- `UFF7SaveSubsystem : UGameInstanceSubsystem` — `Save(int32 Slot)`, `Load(int32 Slot)`, `ListSlots()` returning `TArray<FSaveSlotSummary>`.
- 3 slots. Header `int32 SaveVersion = 1`; mismatch logs + refuses load (never silently corrupts).
- `SFF7SaveMenu` (§2.8) lists slots with portraits-stub, play time, location name.
- `AFF7SavePoint : AActor` (placed in field levels) — interact opens the save menu; Tent restores HP/MP; opens PHS.

### 2.15 Audio

- `UFF7AudioSubsystem : UGameInstanceSubsystem`.
- API: `PlayBGM(FName)`, `StopBGM(float FadeSeconds)`, `PlaySFX(FName)`, `PlayJingle(FName)` (pauses BGM, plays jingle, resumes).
- `FAudioTrackRow : FTableRowBase { TSoftObjectPtr<USoundBase> Asset; float Volume; bool bLooping; }` in `DT_AudioTracks`.
- Two `UAudioComponent` slots for BGM crossfade; async-load via `FStreamableManager`.
- Null `Asset` → log once, play nothing. Never crash. Drop in a WAV by editing the row.
- Subsystem binds to gameplay delegates from §2.10, §2.6, §2.8 — `OnBattleStart → PlayBGM("BGM_Battle")`, `OnBattleVictory → PlayJingle("Jingle_Victory")`, etc. Gameplay never references audio assets directly.

### 2.16 World map and vehicles

- `L_WorldMap.umap` — placeholder terrain (BSP or a stub landscape). World Partition deferred.
- `AFF7WorldMapPawn : AFF7CharacterBase` — faster profile than field pawn.
- `AFF7ChocoboPawn`, `AFF7BuggyPawn` — alt pawns with different speed + terrain restrictions.
- `AFF7ParkedVehicle : AActor` — interactable; mount/dismount via `AFF7PlayerController::Possess`.
- Encounter triggers reused from §2.10, scaled for overworld density.
- `AFF7WorldMapGameMode` sets default pawn and handles overworld rules.

### 2.17 Shops and save points

- `FShopRow : FTableRowBase` — shop id + stock list (row ids from `DT_Items`/`DT_Equipment`).
- `SFF7ShopMenu` — buy/sell; routes through `UFF7Inventory` (§2.7) and gil (§2.5).
- `AFF7SavePoint` — detailed in §2.14.

### 2.18 Summons, Enemy Skill

- Summons: materia family (§2.9); `UFF7SummonAction : UFF7BattleAction` subclasses (Ifrit, Shiva, Ramuh); per-battle use cap seeded by materia tier.
- Enemy Skill: materia family that learns a registered skill when hit by it. Learned-set tracked on the `UFF7MateriaInstance` directly.

### 2.19 Mini-games and Chocobo breeding (stretch)

- `AFF7MiniGameTrigger` — transitions to a dedicated mini-game level using the same `OpenLevel`/`PendingReturn` pattern as §2.10.
- `AFF7ChocoboRaceTrack` + `AFF7RaceChocoboPawn` — alternate IMC (hold-to-sprint, release-to-regen).
- `UChocoboBreedingSubsystem : UGameInstanceSubsystem` — parent refs, offspring rating, stat inheritance formula. Console-command UI only.

### 2.20 Blueprint layer

Deliberately thin. Only:
- `BP_FF7PlayerController` — holds `UInputMappingContext` + `UInputAction` asset refs.
- `BP_FF7FieldGameMode`, `BP_FF7BattleGameMode`, `BP_FF7WorldMapGameMode` — set default pawn/controller/HUD classes.
- `BP_Item_<Name>`, `BP_Materia_<Name>` — only if per-asset tuning is needed (most live directly in DataTables/DataAssets).
