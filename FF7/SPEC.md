# SPEC — FF7 (1997) in Unreal Engine 5.7

A gameplay-focused recreation of Final Fantasy VII (1997) built in Unreal Engine 5.7. The project exists to learn C++ in Unreal, so features are scoped to mechanics (not assets or story fidelity). Characters are rendered as placeholder primitive shapes structured so skeletal meshes drop in later. Implementation is C++-first; Slate drives menus; Blueprints are thin wrappers for designer tuning and asset references only.

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
- Action set: **Attack, Magic, Summon, Item, Defend, Limit, Escape, <Command-materia actions>** (Escape: hold both shoulder buttons / configured key to flee; fails against bosses).
- Targeting supports single-enemy, single-ally, all-enemies, all-allies, random, and self.
- Turn outcome: damage formulas produce HP/MP deltas; status effects apply/tick/expire; victory grants EXP/AP/gil/item drops; defeat returns to the last save.
- Battle-start modifiers (pre-emptive / back attack / side attack / pincer) are **out of scope** for the initial build; encounter triggers always start neutral.

### Materia

Families:
- **Magic** — grants spells (Fire, Cure, Haste…) at AP thresholds.
- **Command** — grants menu commands (Steal, Throw, Mime…).
- **Summon** — grants a summon ability with limited per-battle uses.
- **Support** — modifies a *linked* adjacent materia (All, Quadra Magic, MP Turbo…).
- **Independent** — passive stat/effect modifiers (HP Plus, Counter, Long Range…).

Rules:
- Equipment has **materia sockets**, some of which are *linked pairs* (drawn as connected in UI). Support materia only affects its paired slot.
- Materia grants its effects *while equipped*; unequipping reverts the grant.
- Materia retains AP across equip/unequip and levels independently of the character.

### Equipment

- Three slots per character: **Weapon, Armor, Accessory**.
- Each equipment piece defines: stat modifiers, materia sockets (with link layout), elemental/status affinities, restrictions (character-specific weapons).
- Accessories grant passive effects (status immunity, auto-regen, haste start-of-battle, etc.).

### Magic, Summons, Limit Breaks

- **Magic** — cast from the Magic command; MP cost + cast animation + effect. Organized by element and tier (Fire/Fira/Firaga).
- **Summons** — invocations with a per-battle use cap (seeded by materia level); stronger than single-tier spells.
- **Limit Breaks** — per-character, unique moves. A **limit gauge** fills when the character takes damage; threshold fires the Limit command. Each character has multiple **tiers**, each with **two branches**; tiers unlock by kill count / branch usage (classic FF7 rules).

### Status effects

- Buffs (Haste, Barrier, MBarrier, Regen, Shield, Wall, Reflect, Resist, Peerless, Berserk).
- Debuffs / ailments (Poison, Sleep, Confuse, Silence, Slow, Stop, Frog, Small, Paralysis, Darkness, Petrify, Death-sentence, Manipulate, Fury, Sadness).
- Each effect defines: duration (ticks or permanent), stat modifiers, per-tick callbacks (poison DOT, regen HOT), remove-on-events (Sleep → remove on damage).

### Inventory, shops, save points

- Flat inventory of consumables, equipment (weapons/armor/accessories), and key items with stack counts. Equipment pieces resolve their definition from `DT_Equipment`; consumables from `DT_Items`.
- **Gil** (currency) is a single integer on `UFF7GameInstance`; earned from battle, spent at shops.
- Shops (buy/sell) at field locations; stock defined per shop in a DataTable.
- **Save points** restore HP/MP to full when a **Tent** is used, open the save menu, open the PHS.

### Field exploration & dialogue

- Top-down/isometric camera (fixed spring-arm rig; per-area overrides via camera-volume actors if needed).
- NPCs are `AFF7NPCActor` instances with interact volumes implementing `IFF7Interactable`.
- Dialogue runs through a Slate popup; world **event flags** (TMap of FName → int) gate progression and NPC state.
- Triggers (overlap volumes) fire scripted events (start battle, open cutscene placeholder, set flag).

### World map & vehicles

- Separate overworld level; pawns switch via `AFF7PlayerController::Possess` to alternate vehicle pawns:
  - **Chocobo** — rental mount; different movement profile per breed.
  - **Buggy**, **Tiny Bronco** (air), **Highwind** (airship), **Submarine**.
- Vehicles restrict/enable traversal (water, mountains, forests).

### Save / Load

- Slot-based (3 slots). `UFF7SaveGame` serializes: party roster, active party, per-character stats/equipment/materia AP, inventory, gil, world flags, current level, player-start tag.
- Versioned header so format changes are detected, not silently corrupted.

### Audio & music (stub today, hot-swap later)

- `UFF7AudioSubsystem` (GameInstanceSubsystem) indexed by track ID (FName) via a `UDataTable` of `TSoftObjectPtr<USoundBase>`.
- BGM slot with crossfade; SFX one-shot pool; jingle (victory/level-up) layer that interrupts+resumes BGM.
- All asset references may be null — the subsystem logs missing tracks and remains silent, never crashes. Drop in a WAV later, the row fills, playback works.
- Gameplay fires multicast delegates (`OnBattleStart`, `OnBattleVictory`, `OnLevelUp`, `OnMenuOpen`, `OnFieldEnter`); the audio subsystem binds to these and calls its own `PlayBGM` / `PlaySFX` / `PlayJingle` — gameplay never references an asset directly.

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

## 2. High-level implementation

### Game framework

- `UFF7GameInstance` — persistent across level loads. Owns party roster, inventory, gil, world flags, current save slot.
- `AFF7GameMode` — base class; subclasses per level type:
  - `AFF7FieldGameMode` — exploration rules, encounter ticking, interact.
  - `AFF7BattleGameMode` — combat rules, ATB subsystem, battle-end return.
  - `AFF7WorldMapGameMode` — overworld rules, vehicle possession.
- `AFF7PlayerController` — input routing, menu open/close, possesses pawns. `BP_FF7PlayerController` holds IMC/IA asset references (established pattern from prior project).
- `AFF7PlayerPawn` — field pawn; `AFF7VehiclePawn` subclasses for overworld.

### Character representation

- `AFF7CharacterBase` (AActor): capsule collision + **`USceneComponent` visual root**. Placeholder visuals (cube/cylinder/sphere `UStaticMeshComponent`s) attach to the visual root. Replacing with a `USkeletalMeshComponent` later is an isolated edit to the visual root — no gameplay code changes.
- `AFF7PartyMemberActor : AFF7CharacterBase` — represents a party member in battle arenas.
- `AFF7EnemyActor : AFF7CharacterBase` — enemy units.

### Data-driven design

- **DataTables** (`FTableRowBase` subclasses): items, equipment, spells, summons, enemies, encounter pools, shop stock, level-stat curves, limit tiers, audio tracks, dialogue lines.
- **DataAssets** (`UPrimaryDataAsset`): character definitions (starting stats, weapon restriction, limit branches), materia definitions (family, AP thresholds, granted abilities), status-effect definitions.
- Formulas live in C++ static utility classes (`UFF7DamageCalc`, `UFF7LevelCurve`) — **data is values, code is behavior**, kept separate for testability.

### Combat

- `UFF7BattleSubsystem : UTickableWorldSubsystem` (battle-world only) — owns combatants, ATB gauges, action queue, turn order; the subsystem itself ticks so no dedicated tick actor is needed.
- `UFF7BattleAction` (UObject) — base for Attack / Magic / Item / Limit / Summon / Escape; virtual `Execute(FBattleContext&)`.
- `IFF7Damageable` / `IFF7StatusTarget` interfaces — implemented by both party and enemy actors so action code is side-agnostic.
- Wait / Active mode is a flag on the subsystem; opening a Slate menu sets it when the config says Wait.
- `AFF7PlayerController` (same class used on the field) owns the battle HUD widget (`SFF7BattleHUD`, target picker); no separate battle controller class.

### Inventory / equipment / materia

- `UFF7Inventory` (`UObject` owned by `UFF7GameInstance`): `TMap<FName, int32>` of row-ID → count, plus broadcast delegate `OnInventoryChanged`. Chosen as a plain UObject — not a `UActorComponent` — because `UGameInstance` isn't an `AActor`.
- `UFF7Equipment` (`UObject` owned per `FPartyMember`): three slots holding `FEquippedItem { FName EquipRowId; TArray<UFF7MateriaInstance*> Sockets; }`.
- `UFF7MateriaInstance` (UObject) — per-socket instance data (AP, master flag) with a hard `UFF7MateriaDataAsset* Def` reference (materia is useless without its definition loaded, so soft refs add no value at runtime; save-game layer stores the def by path).
- Linked slots expressed as a bitfield on the equipment row (bit N set = socket N is linked to socket N+1).

### UI

- **Slate** for all menus (`SCompoundWidget` subclasses: `SFF7MainMenu`, `SFF7BattleHUD`, `SFF7DialoguePopup`, `SFF7ShopMenu`, `SFF7PHSMenu`, `SFF7SaveSlotList`, `SFF7StatusScreen`, `SFF7EquipScreen`, `SFF7MateriaScreen`).
- A thin UMG wrapper is used only where designer tuning of HUD overlays is required.
- Widget lifetime managed via `TSharedPtr` in the owning subsystem/controller; widgets are added to the viewport via `GEngine->GameViewport->AddViewportWidgetContent`.

### Save system

- `UFF7SaveGame` (USaveGame) — `UPROPERTY`-tagged fields serialize automatically. Versioned via an `int32 SaveVersion` header.
- `UFF7SaveSubsystem` (GameInstanceSubsystem) — wraps `UGameplayStatics::SaveGameToSlot` / `LoadGameFromSlot`, exposes `Save(Slot)` / `Load(Slot)` / `ListSlots()`.

### Audio

- `UFF7AudioSubsystem` (GameInstanceSubsystem) — `PlayBGM(FName)`, `PlaySFX(FName)`, `PlayJingle(FName)`. Crossfade via two `UAudioComponent` slots.
- `FAudioTrackRow : FTableRowBase { TSoftObjectPtr<USoundBase> Asset; float Volume; bool bLooping; }`.
- Null `Asset` → log warning, skip. Never crash. Drops in real WAVs by editing the row.

### Blueprint layer

Kept deliberately thin. Only:
- `BP_FF7PlayerController` — holds IMC (`UInputMappingContext`) and IA (`UInputAction`) asset references so designers can reassign bindings without recompiling.
- `BP_FF7FieldGameMode`, `BP_FF7BattleGameMode` — set default pawn/controller/HUD classes.
- `BP_Item_<Name>`, `BP_Materia_<Name>` — only if per-asset tuning is needed (most live in DataTables/DataAssets directly).

### Module structure

Single `FF7` runtime module for now. Split into `FF7Core` / `FF7UI` / `FF7Combat` later if compile times become painful.

---

## 3. Cross-references

| System | TASKS phase |
|---|---|
| Build pipeline | Phase 0 |
| Player pawn & movement | Phase 1 |
| NPC & dialogue | Phase 2 |
| Party roster / GameInstance | Phase 3 |
| Stats & leveling | Phase 4 |
| Inventory & items | Phase 5 |
| Slate UI foundation | Phase 6 |
| Equipment & materia | Phase 7 |
| Combat scaffold (turn-based) | Phase 8 |
| ATB system | Phase 9 |
| Magic / status effects | Phase 10 |
| Limit Breaks | Phase 11 |
| Save / Load | Phase 12 |
| Audio / music stub | Phase 13 |
| World map & vehicles | Phase 14 |
| Shops, summons, polish | Phase 15 |
| Mini-games / Chocobo breeding (stretch) | Phase 16 |
