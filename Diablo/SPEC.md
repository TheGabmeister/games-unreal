# Diablo Recreation — Specification

Recreation of the original Diablo (1997, Blizzard North) in Unreal Engine 5.7. **Gameplay over graphics** — we do not attempt pixel-by-pixel fidelity. All assets are procedurally generated (Blender Python, Inkscape SVG→PNG, Python audio synthesis). C++ first; Blueprints are thin asset-reference holders only.

This document is both the design spec and the learning-ordered roadmap. Each milestone introduces one or two Unreal Engine subsystems. Abstractions (interfaces, components, data-assets, StateTree) are only introduced when a milestone's complexity justifies them.

---

## Design Reference — Diablo (1997)

Condensed from [Jarulf's Guide](https://wheybags.gitlab.io/jarulfs-guide/), [Diablo Wiki](https://diablo.fandom.com/wiki/Diablo_I), [Boris the Brave's dungeon analysis](https://www.boristhebrave.com/2019/07/14/dungeon-generation-in-diablo-1/), and the original manual.

### Core Loop
Town (Tristram) ↔ dungeon pendulum across 16 levels in 4 biomes:
- Cathedral (L1–L4) — stone halls, recursive room-and-corridor "budding"
- Catacombs (L5–L8) — crypts, 3×3 pattern-match corridors
- Caves (L9–L12) — caverns + lava, marching-squares
- Hell (L13–L16) — bounded 20×20 bud, dense stonework, Diablo on L16

Town Portal scroll is the primary transit.

### Classes (3)
| Class | Start Str/Mag/Dex/Vit | Start HP/Mana | Str/Mag/Dex/Vit caps | Starts with | Style |
|---|---|---|---|---|---|
| **Warrior** | 30/10/20/25 | 70/10 | **250**/50/60/100 | Short Sword + Buckler, Repair Item | Melee tank |
| **Rogue** | 20/15/30/20 | 45/22 | 55/70/**250**/80 | Short Bow, Disarm Trap | Ranged, best block |
| **Sorcerer** | 15/35/15/20 | 30/70 | 45/**250**/85/80 | Short Staff (Charged Bolt), Recharge Staff | Caster |

### Stats
- **Strength** — weapon damage scaling, equip gate for heavy gear
- **Magic** — mana pool, gates reading spellbooks and equipping staves
- **Dexterity** — to-hit, AC/5, block chance, Rogue damage, bow gate
- **Vitality** — HP pool

**To-Hit** (clamped 5–95%):
```
ToHit% = 50 + (Dex / 2) + CharLevel + ItemBonus
         - MonsterAC + (CharLevel - MonsterLevel)
```

**Block** (requires shield): `Dex + 2×CharLevel − 2×AttackerLevel`

**Damage mitigation:** flat damage-reduction from armor affixes for physical; % resistance (cap 75%) for elemental (fire/lightning/magic).

### Progression
- Level cap **50**
- **+5 stat points** per level-up, spent freely subject to class caps
- Steep quartic-ish XP curve (L1→2 ≈ 2,000 XP; L49→50 ≈ 1.3 billion)
- Monster XP zeroes at `CharLevel − MonsterLevel ≥ 10`

### Items
- **7 equipment slots:** Helm, Amulet, Chest, 2× Ring, MainHand, OffHand (shield / 2H spans both)
- **Tiers:** Normal (white) / Magic (blue, prefix + suffix) / Unique (gold, fixed rolls)
- **Inventory:** 10×4 grid. Footprints: 1×1, 1×2, 1×3, 2×2, 2×3
- **Belt:** 8 slots, keys **1–8**, potions and scrolls only
- **Durability** on all equipment; Griswold repairs (max durability degrades)
- **Identify:** Cain, 100 gold flat
- **Gold:** stacks to 5000/cell
- **No stash** — inventory is the only storage

### Spells (~25)

**Learning modes:**
- **Book** — permanent, up to spell level 15, costs escalating Magic stat, grants small mana bonus
- **Scroll** — one-shot cast, ignores mana, caster's current learned level
- **Staff** — finite charges

**Damage:** Firebolt, Fireball, Lightning, Chain Lightning, Charged Bolt, Nova, Apocalypse, Flame Wave, Fire Wall, Holy Bolt, Inferno, Blood Star, Bone Spirit, Elemental, Guardian

**Defense/control:** Stone Curse, Flash, Mana Shield, Golem

**Support:** Healing, Heal Other, Resurrect

**Utility:** Town Portal, Teleport, Phasing, Telekinesis, Identify, Infravision

**Class-learned:** Repair Item (Warrior), Disarm Trap (Rogue), Recharge Staff (Sorcerer)

### Enemies
Archetypes: melee grunt / fast melee / ranged / spellcaster / summoner.

**Unique monsters:** Named champions with 2–3 affixes (tinted sprite).

**Quest bosses:** Butcher (L2, "Ah, fresh meat!" room reveal), Skeleton King (L3 summoner), Zhar the Mad (L8 sorcerer), Lazarus (L15 via red portal), **Diablo** (L16).

**AI:** tile-based aggro + A* pathing. Most monsters chase-and-attack. Fallen flee at low HP + cheer when ally kills player. Casters kite. **Monsters freeze off-screen** (no simulation outside ~screen radius).

### Town (Tristram) NPCs
- **Griswold** — smith: buys/sells weapons/armor/shields/helms/jewelry, repair
- **Adria** — witch: scrolls, staves, spellbooks, mana potions, elixirs
- **Pepin** — healer: instant free heal, potions, cures poison/disease
- **Deckard Cain** — sage: identify (100g flat)
- **Wirt** — peg-leg: one random item per town visit at 5–10× markup (50g peek fee)
- **Ogden** — innkeeper: quest-giver
- **Farnham**, **Gillian** — dialogue only

### Controls
- **LMB** terrain → walk; **LMB** enemy → attack (melee auto-ranges; ranged fires in place)
- **RMB** → cast bound spell at cursor
- **Shift+LMB** → attack in place (no movement)
- **Shift+RMB** → cast in place
- **1–8** → quaff belt slot
- **F1–F8** → spell hotbar (swap bound spell)
- **S/I/C/Q/Tab** → spellbook / inventory / character / quest / minimap

### Camera
True isometric: orthographic projection, fixed 45° pitch, no yaw, no zoom. Screen-centered on character.

### Audio Identity (Matt Uelmen)
- Tristram acoustic theme = sonic "safe zone"
- Dungeons = near-silent ambient dread broken by monster stingers
- Monster voice lines (Butcher's "Ah, fresh meat!", Diablo's rumble)
- Weapon-class-specific combat SFX (swords ring, maces thud)

### Quirks Worth Preserving
- Stairs are level-load triggers (not seamless)
- No auto-save (vanilla)
- Shrines give random permanent buff/curse
- Mana Shield drains mana per point of damage taken
- Unidentified magic items hide affixes until Cain reveals them
- Fallen "cheer" mechanic: killing player near them buffs the pack

---

## Asset Pipeline

### 3D Models — Blender (`C:\Program Files\Blender Foundation\Blender 5.1\blender.exe`)
- Headless: `blender.exe --background --python <script>.py`
- Scripts: [Tools/blender/](Tools/blender/)
- Output: FBX to `Tools/blender/out/`, then imported into UE
- Convention: Z-up, centimeter scale, single armature per character
- Style: low-poly primitive composition (boxes/cylinders)

### Icons/Sprites — Inkscape (`C:\Program Files\Inkscape\bin\inkscape.exe`)
- `inkscape.exe --export-type=png --export-dpi=<N> --export-filename=<out.png> <in.svg>`
- Scripts: [Tools/svg/](Tools/svg/)
- Uses: spell icons, potion icons, equipment slot frames, UMG frames

### Audio — Python + numpy + scipy
- Pure-code procedural synthesis (oscillators, envelopes, filters, convolution reverb)
- Output: 16-bit PCM WAV, imports directly to UE `USoundWave`
- Scripts: [Tools/audio/](Tools/audio/)
- One-time setup: `python -m pip install numpy scipy`
- Deferred: FluidSynth + soundfont for MIDI-based music (not needed until mid-milestones)

---

## Milestones

Each milestone ends with a manual play-test. No unit tests until complexity demands them.

### M0 — Clean Slate
Delete template (all Variant_* folders, ThirdPerson content, template root C++ classes). Strip `Diablo.Build.cs` include paths. Update `Config/DefaultEngine.ini`. Minimal `ADiabloGameMode` in C++. Bootstrap asset tooling (Blender/Inkscape/Python smoke tests). Build an **Editor Utility Widget** (`UDiabloEditorToolWidget`) that generates editor-authored assets (BP subclasses, maps, input assets) via button press — eliminating manual editor work for reproducible project setup.

**Teaches:** UBT build loop, `.uproject`/`.Build.cs` basics, Editor Utility Widgets, programmatic asset creation (`UBlueprintFactory`, `FKismetEditorUtilities`, `UWorld::SaveMap`).

### M1 — Isometric Camera + Click-to-Move Hero (Warrior)
`ADiabloHero : ACharacter` with `USpringArmComponent` (fixed pitch −45°, yaw 45°, length ~1200, no rotation inheritance) + `UCameraComponent` (`ProjectionMode = Orthographic`, `OrthoWidth` tuned for isometric framing). `ADiabloPlayerController`: Enhanced Input `IA_Click` → trace from cursor → `UAIBlueprintHelperLibrary::SimpleMoveToLocation`. Navmesh covers the test level. Placeholder low-poly hero mesh via Blender script.

**Teaches:** `ACharacter`, `USpringArmComponent`, orthographic `UCameraComponent`, navmesh, Enhanced Input basics.

### M2 — Basic Melee Attack
LMB on `ADiabloEnemy` → hero walks into range, plays `UAnimMontage`, `UAnimNotify_Attack` triggers sphere trace from forward bone, `AActor::TakeDamage` overrides deal hardcoded damage. Enemy has HP `UPROPERTY`, no AI yet (stationary target).

**Teaches:** `UAnimMontage`, `UAnimNotify`, traces, damage events.

### M3 — Enemy AI (C++ State Machine)
`ADiabloAIController : AAIController`. Logic is a plain `enum class EAIState` + switch in `Tick` (Idle / Chase / Attack / Dead), with `AIMoveTo` for pathing and a 0.5s distance-check for aggro. **Deliberately no StateTree** — abstraction not justified for ≤4 states.

**Teaches:** `AAIController`, `AIMoveTo`, tick-based state machines. Sets up the "state machine pain" that justifies StateTree in M19.

### M4 — HP + Death + Respawn
Shared `FCombatStats` struct (HP/MaxHP) used by hero and enemy — just a struct, not a component yet. Hero death → 2s timer → `AGameModeBase::RestartPlayer`. Enemy death → destroy after death animation.

**Teaches:** `USTRUCT`, GameMode respawn flow, `FTimerManager`.

### M5 — HUD v1 (Life Globe + XP Bar)
`UDiabloHUDWidget : UUserWidget` with red life globe (left), blue mana globe (right, placeholder 0/0), XP bar between. `NativeTick` binds to hero. Added to viewport in `ADiabloPlayerController::BeginPlay`.

**Teaches:** UMG, `UUserWidget`, `NativeTick` binding, viewport HUD.

### M6 — XP + Leveling + Stat Points
Enemies award XP on death. XP table is a `TArray<int32>` constant in `ADiabloHero` (hardcoded quartic curve). On level-up: play SFX, grant +5 unspent stat points, recompute derived stats (`MaxHP`, `MaxMana`, damage range). Character panel UMG with +/− buttons for Str/Mag/Dex/Vit (respecting class caps — Warrior Str 250, etc.).

**Teaches:** Derived-stat recomputation (first justified helper function), UMG button bindings, class caps.

### M7 — Inventory (Grid + Equipment)
First real abstractions — complexity demands them:
- `UItemDefinition : UPrimaryDataAsset` — icon, name, grid size (w×h), stackable, equip slot, base stats
- `FItemInstance` struct — `UItemDefinition*` + affix list (empty for now) + current durability
- `UInventoryComponent : UActorComponent` on hero — 10×4 grid, 7 equipment slots, gold. API: `TryAddItem`, `MoveItem`, `Equip`, `Unequip`
- UMG inventory panel with drag-drop (`UDragDropOperation`)

**Teaches:** `UDataAsset`, `UActorComponent`, complex UMG (drag-drop), first real abstraction layer.

### M8 — Loot Drops
`ADroppedItem` actor holding an `FItemInstance`. Enemies roll a simple weighted drop table on death (`TArray<FDropTableEntry>` on `ADiabloEnemy`) and spawn a dropped item with a small impulse. Click-to-pick-up routes to `UInventoryComponent::TryAddItem`.

**Teaches:** Actor spawning, overlap events, world ↔ inventory handoff, weighted random selection.

### M9 — Equipment Stats + `IEquippable` Interface
Equipping modifies derived stats via `ApplyEquipBonuses` / `RemoveEquipBonuses` called by `UInventoryComponent`. Introduce `IEquippable` as a proper UE UINTERFACE — complexity now justifies it because item categories (weapon / armor / ring) need polymorphic equip behavior:
- Weapon → updates attack damage range
- Armor → updates AC
- Ring/Amulet → stacks flat bonuses

**Teaches:** `UINTERFACE`/`IInterface` pattern, polymorphic dispatch. First use of a real UE interface.

### M10 — First Spell (Firebolt) + Mana
`ASpellProjectile` base actor with `UProjectileMovementComponent`. `AFirebolt` subclass with VFX placeholder. RMB cast: spawn projectile from hero forward, cost mana, cooldown. Hero gains a `Mana` stat (Sorcerer's domain — but Warrior can cast from scrolls/staves too in D1). Add Healing Potion as a consumable item that restores HP.

**Teaches:** Projectile actors, `UProjectileMovementComponent`, overlap damage, cooldowns, consumables.

### M11 — Spellbook + Hotbar
`USpellDefinition : UPrimaryDataAsset` — mana cost, cast animation, spell class (`TSubclassOf<ASpellProjectile>` or instant-effect function). Hero keeps `TArray<USpellDefinition*>` known spells. Spellbook UMG (`S`) lists all known spells; right-click binds to LMB or RMB slot. Add Fireball, Lightning, Nova, Healing.

**Teaches:** Data-driven spell system, generalization from hardcoded Firebolt to table-driven dispatch.

### M12 — Hand-Placed Dungeon L1 (Cathedral)
`Lvl_Cathedral_L1.umap` — hand-authored stone corridors and rooms. `ADungeonStairs` actor calls `UGameplayStatics::OpenLevel`. Pre-placed enemy spawn markers + loot.

**Teaches:** `OpenLevel`, level transitions, the "state loss on map change" problem that motivates M13.

### M13 — `UGameInstance` for Persistent State
`UDiabloGameInstance` owns hero's inventory, stats, spell knowledge across level transitions. Hero `BeginPlay` reads from GameInstance; pre-transition hook writes back.

**Teaches:** `UGameInstance` lifetime, state-ownership boundaries.

### M14 — Save / Load (Town-Only)
`UDiabloSaveGame : USaveGame` serializes GameInstance's hero state. Save via main menu (town only, matching D1 vanilla). Load recreates hero in Tristram.

**Teaches:** `USaveGame`, `SaveGameToSlot`, serialization gotchas with soft pointers and `UDataAsset` refs.

### M15 — Tristram + NPCs + `IInteractable`
`Lvl_Tristram.umap`. Static NPC actors (Griswold, Adria, Pepin, Cain, Wirt, Ogden) as `AActor` subclasses implementing `IInteractable`. LMB interacts, shows dialog UMG with name + generic text.

**Teaches:** Second UE interface, modal UMG, multi-level management.

### M16 — Shop UI (Griswold, Adria, Pepin)
Two-panel UMG trade window (NPC inventory left, player inventory right). NPC stock from `UNPCInventoryData : UDataAsset`. Buy/sell at list price (simple flat markup). Pepin free-heal on click. Cain identify for 100g flat.

**Teaches:** Two-panel UMG, data-driven NPC stock, modal economy flows.

### M17 — Item Affixes (Magic Items, Blue Tier)
Prefix/suffix tables as `UDataAsset` entries: name + stat modifier + qlvl gate. On drop: roll magic chance; if magic, roll 0–1 prefix + 0–1 suffix from qlvl-appropriate pool. Affixes stored in `FItemInstance::Affixes`. Derived stats recompute on equip (leverages M9 dispatch). Unidentified items hide affix names and bonuses until Cain runs identify.

**Teaches:** Randomized data composition, qlvl gating, identify-reveal flow.

### M18 — Procedural Dungeon Generation (Cathedral-Style)
C++ tile generator following Boris the Brave's D1 algorithm:
- 40×40 tile grid
- Recursive room-and-corridor "budding"
- Output: `TArray<ETileType>`
- Instantiate at level start via `SpawnActor` using `UTilePalette : UDataAsset` (`ETileType → TSubclassOf<AActor>`)
- Seed stored on GameMode, frozen per dungeon floor

**Teaches:** Runtime level construction, grid→world transforms, `SpawnActor` at scale.

### M19 — Migrate AI to StateTree
Behaviors have multiplied (grunt, fast-melee, ranged-archer, spellcaster, summoner, boss) — the M3 C++ state machine is painful. Migrate to `UStateTreeComponent` with custom tasks as `USTRUCT`s in a `DiabloStateTreeUtility.h`:
- `FStateTreeChaseTask`, `FStateTreeMeleeAttackTask`, `FStateTreeRangedAttackTask`, `FStateTreeCastSpellTask`, `FStateTreeFleeTask`, `FStateTreeSummonTask`

Enable Fallen cheer/flee patterns.

**Teaches:** StateTree tasks/conditions, `STATETREE_POD_INSTANCEDATA`, `EnterState`/`ExitState`/`Tick` virtuals. Justified by 6+ distinct AI behaviors.

### M20 — Belt + Potions + Hotkeys (1–8)
Belt = dedicated 8-slot sub-inventory on `UInventoryComponent`, potions/scrolls only. Keys 1–8 bound to `IA_Belt1..8`. Healing / Mana / Rejuvenation potions and scrolls of various spells. Scroll use dispatches through an `IConsumable` interface (third interface — justified by diverging scroll effects).

**Teaches:** Hotkey input, per-slot action dispatch, third interface.

### M21+ — Content, Polish, Boss
- Full 16-level dungeon progression (Catacombs/Caves/Hell biome palettes via M18's palette system)
- Unique monsters (base monster + 2–3 affix buffs)
- Bosses: Butcher (L2 scripted room-reveal with voice stinger), Skeleton King (L3 summoner), Diablo (L16)
- Shrines — interactable prop with random buff/curse table
- Town Portal spell (bidirectional portal actor, persistent across levels)
- Durability + repair (Griswold)
- Remaining spells: Chain Lightning, Apocalypse, Teleport, Town Portal, Stone Curse, Mana Shield
- Audio integration: Uelmen-inspired ambient pass, monster stingers, per-weapon-class SFX
- Balance pass + XP curve tuning
- Classes 2 and 3: Rogue (bows) and Sorcerer (higher mana cap)

---

## Verification

Every milestone ends with a manual play-test against specific criteria. Build via:

```bash
"/c/Program Files/Epic Games/UE_5.7/Engine/Build/BatchFiles/Build.bat" DiabloEditor Win64 Development -Project="c:/dev/games-unreal/Diablo/Diablo.uproject" -WaitMutex -FromMsBuild
```

**Milestone smoke tests:**
- **M0:** Editor opens into `Lvl_Diablo.umap` with no errors; `LogDiablo` visible in Output Log; asset tool smoke tests pass.
- **M1:** LMB on terrain → hero walks there. Camera stays fixed orthographic.
- **M2:** LMB on enemy → hero attacks → enemy HP drops → dies at 0.
- **M3:** Stationary enemy transitions Idle→Chase when hero approaches; attacks in range; returns to Idle if hero flees far.
- **M4:** Hero death → respawn at `PlayerStart` after 2s.
- **M5+:** HUD globes reflect HP/mana in real time.
- *(and so on per milestone)*

**Asset pipeline smoke tests** (one-time before M0 relies on them):
- `blender.exe --background --python Tools/blender/smoke_cube.py` → outputs `out/cube.fbx`
- `inkscape.exe ... Tools/svg/smoke_icon.svg` → outputs `out/icon.png`
- `python Tools/audio/smoke_sine.py` → outputs `out/sine.wav` (440 Hz, 1 s)
