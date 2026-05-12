# Dark Souls — Gameplay Systems Spec

Dark Souls, PlayStation 3 / Xbox 360 / PC, 2011. Developed by FromSoftware, published by Bandai Namco. This spec covers the original release (Prepare to Die Edition on PC, which includes the Artorias of the Abyss DLC). Mechanical differences with Dark Souls: Remastered (2018) are noted where relevant.

---

## 1. Core Gameplay Systems

### 1.1 Primary Loop

Explore interconnected areas → defeat enemies for Souls → spend Souls to level up or buy equipment at Bonfires → push deeper into the world → die, lose Souls, respawn at last Bonfire → retrieve dropped Souls or lose them permanently on a second death. Boss kills unlock new areas and advance the story. The loop is punishing by design: death resets all non-boss enemies, and every resource decision is a gamble.

### 1.2 Combat System

Third-person melee-focused action RPG with real-time combat. Menus never pause the game — not even in offline mode.

**Actions and stamina cost:** Every offensive and defensive action (attack, block, roll, sprint, backstep) consumes Stamina from a shared green bar. When Stamina is depleted, the player cannot act until it regenerates. Stamina regenerates continuously when not blocking or sprinting, at a rate modified by equipment weight and shield stability.

**Attack types per weapon:**
- R1 / RB: Light attack (one-handed or two-handed)
- R2 / RT: Heavy attack (one-handed or two-handed)
- Forward + R1: Kick (staggers shields) or weapon-specific thrust
- Forward + R2: Jump attack (forward leap, overhead strike)
- Sprint + R1: Running attack
- Rolling + R1: Rolling attack

**Two-handing:** Pressing Y / Triangle switches the right-hand weapon to two-handed grip, multiplying effective Strength by 1.5× for weapon requirement checks and increasing damage and stagger. A character with 27 STR two-handing meets the requirement for a 40 STR weapon.

**Weapon buffs:** Resins and buff spells (Magic Weapon, Sunlight Blade, etc.) can only be applied to weapons on the Standard, Raw, or Crystal upgrade paths. Weapons with innate elemental damage (Fire, Lightning, Magic, etc.) or unique/boss weapons cannot be buffed.

**Critical attacks:**
- **Backstab:** Approach an enemy from behind and press R1 while adjacent. Deals 2–20× the damage of a light attack depending on weapon critical modifier.
- **Riposte:** After a successful parry (L2 / LT with a small/medium shield), press R1 while adjacent. Deals ~10–30% more damage than a backstab with the same weapon.
- **Plunging attack:** R1 while falling onto an enemy from above.
- Critical damage scales with the weapon's Critical stat (base 100). The Hornet Ring adds +30% to critical damage.

**Blocking:** L1 / LB raises the left-hand shield. Each shield has a Stability stat that determines how much Stamina is consumed per blocked hit, and damage reduction percentages per element. A 100% physical block shield negates all physical damage but still drains Stamina. If Stamina reaches zero while blocking, the player is guard-broken (staggered).

**Parrying:** L2 / LT with a small/medium shield or certain weapons. If the parry window aligns with an incoming attack, the attack is deflected and the enemy enters a stagger state, allowing a riposte.

### 1.3 Poise System

Poise determines resistance to being staggered by incoming attacks. Each attack has a Poise Damage value:

| Weapon Class | Base Poise Damage |
|---|---|
| Daggers, Thrusting Swords | 5 |
| Katanas, Straight Swords | 20 |
| Greatswords, Axes | 35 |
| Ultra Greatswords, Great Hammers | 50 |

Poise damage is multiplied by attack type: 1.0× for one-handed R1, 1.5× for two-handed R1, higher for R2 and running attacks. When accumulated Poise damage exceeds the player's Poise stat, they are staggered. Poise resets after a ~3–5 second timer with no hits taken.

Key breakpoints: 21 (resist dagger R1), 31 (resist straight sword R1), 46 (resist greatsword R1), 61 (resist ultra greatsword R1). Maximum achievable Poise: 161 (Havel's Set 121 + Wolf Ring 40).

### 1.4 Equip Load and Rolling

Equip Load is the total weight of all equipped weapons, shields, catalysts, and armor. Maximum equip load is determined by Endurance, modified by Havel's Ring (+50%), Ring of Favor and Protection (+20%), and Mask of the Father (+5%).

| Equip Load Ratio | Roll Type | Description |
|---|---|---|
| 0–25% | Fast Roll | Longest distance, fastest recovery, full sprint speed |
| 25.1–50% | Mid Roll | Moderate distance and recovery, slightly reduced sprint |
| 50.1–100% | Fat Roll | Short distance, long recovery, greatly reduced sprint |
| >100% | Overloaded | Cannot roll, walk only |

All standard roll types share 11 invincibility frames (i-frames) at 30 fps regardless of weight class. The difference between fast, mid, and fat rolls is total animation length, recovery frames, and distance — not i-frame count. The Dark Wood Grain Ring replaces the roll with a cartwheel/flip that has 13 i-frames with faster recovery. Pre-patch (1.05) the DWGR flip activated below 50% equip load; post-patch (1.06+) the threshold was lowered to below 25%.

### 1.5 Fall Damage

Fall damage scales with height and equip load. Falls below ~5 units of height deal no damage. Falls above ~20 units of height are always lethal regardless of HP, equip load, or spells. Between those thresholds, damage is a percentage of max HP — heavier equip loads increase the percentage. At 0 equip load, the maximum survivable fall costs ~70% HP; at maximum equip load, it costs ~100% HP (leaving 1 HP). The Fall Control sorcery negates all non-lethal fall damage but does not change the lethal threshold.

### 1.6 Damage Calculation

**Attack Rating (AR)** = Weapon Base Damage + Scaling Bonus. Scaling bonus is determined by the weapon's scaling grade (S/A/B/C/D/E) applied to the relevant stat (STR, DEX, INT, FTH). Split-damage weapons calculate each element separately.

**Defense reduction** uses a piecewise function based on the ratio of AR to Defense:

| Condition | Damage Multiplier |
|---|---|
| AR ≤ 0.125 × Def | AR × 0.10 |
| AR ≤ 1.0 × Def | AR × (19.2/49 × (AR/Def − 0.125)² + 0.10) |
| AR ≤ 2.5 × Def | AR × (−0.4/3 × (AR/Def − 2.5)² + 0.70) |
| AR ≤ 8.0 × Def | AR × (−0.8/121 × (AR/Def − 8)² + 0.90) |
| AR > 8.0 × Def | AR × 0.90 |

Each damage type (physical, magic, fire, lightning) is calculated independently against its corresponding defense stat. After flat defense reduction, percentage-based Absorption from armor further reduces the damage.

### 1.7 Status Effects

| Effect | Trigger | Result | Duration | Cure |
|---|---|---|---|---|
| Poison | Build-up bar fills | ~3–4 HP/s drain | 180 s | Purple Moss Clump, Blooming Purple Moss, bonfire |
| Toxic | Build-up bar fills | ~6 HP/s drain, −15 stamina regen/s | 600 s (10 min) | Blooming Purple Moss, Divine Blessing, bonfire |
| Bleed | Build-up bar fills | Instant 30–50% max HP loss (10–15% on bosses) | Instant | 2 s immunity after proc |
| Curse | Build-up bar fills | Instant death; max HP halved until cured | Permanent | Purging Stone, Ingward (NPC) |

Poison and Toxic have separate resistance meters and can be active simultaneously. Curse does not stack in the original version (halves HP once; dying to Curse again does not reduce further). Resistance stat and specific armor/rings increase build-up thresholds.

### 1.8 Death and Soul Recovery

On death:
1. All held Souls and soft Humanity are dropped as a **bloodstain** at the point of death.
2. The player respawns at the last rested Bonfire in Hollowed (undead) form.
3. All non-boss, non-NPC enemies in the area respawn.
4. Returning to the bloodstain and touching it recovers all lost Souls and Humanity.
5. Dying again before reaching the bloodstain permanently destroys it.

---

## 2. Controls & Input

### 2.1 PlayStation 3

| Button | Action |
|---|---|
| Left Stick | Move character |
| Right Stick | Camera control |
| R1 | Light attack (right hand) |
| R2 | Heavy attack (right hand) |
| L1 | Block / left-hand attack |
| L2 | Parry (small shield) / Bash (large shield) |
| Circle (tap) | Backstep |
| Circle (hold) | Sprint / dodge roll (while moving) |
| Triangle | Two-hand right weapon |
| Cross | Interact / confirm |
| Square | Use selected item |
| D-pad Up | Cycle spells |
| D-pad Down | Cycle quick items |
| D-pad Left | Swap left-hand equipment |
| D-pad Right | Swap right-hand equipment |
| R3 | Lock-on / release target |
| Select | Gestures |
| Start | Menu |

### 2.2 Xbox 360

| Button | Action |
|---|---|
| Left Stick | Move character |
| Right Stick | Camera control |
| RB | Light attack (right hand) |
| RT | Heavy attack (right hand) |
| LB | Block / left-hand attack |
| LT | Parry / Bash |
| B (tap) | Backstep |
| B (hold) | Sprint / dodge roll |
| Y | Two-hand right weapon |
| A | Interact / confirm |
| X | Use selected item |
| D-pad Up | Cycle spells |
| D-pad Down | Cycle quick items |
| D-pad Left/Right | Swap weapons by hand |
| RS | Lock-on / release target |
| Back | Gestures |
| Start | Menu |

### 2.3 PC (Keyboard + Mouse)

The original PC port (Prepare to Die Edition) has notoriously poor keyboard/mouse controls. Camera rotation via mouse is not natively supported without DSFix or Remastered. Controller is strongly recommended.

---

## 3. World Structure

### 3.1 World Design

Lordran is a single contiguous 3D world with no loading screens between connected areas (loading only occurs on warps and death). Areas are stacked vertically and folded horizontally, creating a dense interconnected structure where shortcuts loop back to earlier zones. The world has no minimap — navigation is learned through repetition and spatial memory.

### 3.2 Area List and Connections

**Hub:** Firelink Shrine — central hub connecting to most early-game paths.

**Early Game (pre-bells):**
- Northern Undead Asylum (tutorial, revisitable)
- Undead Burg (Upper) → Undead Parish
- Lower Undead Burg → Depths → Blighttown (lower)
- Darkroot Garden / Darkroot Basin
- New Londo Ruins (accessible early, enemies require specific items)

**Mid Game (post-bells, pre-Lordvessel):**
- Sen's Fortress (opens after ringing both Bells of Awakening)
- Anor Londo (accessed via Iron Golem defeat)

**Late Game (post-Lordvessel, non-linear):**
- Duke's Archives → Crystal Cave
- Demon Ruins → Lost Izalith
- The Catacombs → Tomb of the Giants (Catacombs accessible from Firelink from the start; Tomb of the Giants' final boss area requires Lordvessel)
- New Londo Ruins → The Abyss (ruins accessible early; draining the water requires the Lordvessel path via Ingward)
- Kiln of the First Flame (final area, opens after placing all Lord Souls)

**Optional Areas:**
- Painted World of Ariamis (entered via painting in Anor Londo)
- Great Hollow → Ash Lake (entered via illusory walls in Blighttown)
- Return to Northern Undead Asylum (via Firelink Shrine crow)

**DLC Areas (Artorias of the Abyss):**
- Sanctuary Garden → Oolacile Sanctuary → Royal Wood → Oolacile Township → Chasm of the Abyss
- **Access:** Obtain the Broken Pendant from Duke's Archives (dropped by a Crystal Golem near the beginning), then return to Darkroot Basin and interact with the portal behind the Hydra's lake. The DLC is set in the past and is mechanically isolated from the main game's gating.

### 3.3 Illusory Walls

Certain walls throughout Lordran are illusory and disappear when struck with any attack (or rolled into). They conceal hidden bonfires, shortcuts, treasure, and entire optional areas (e.g., the entrance to the Great Hollow in Blighttown). There is no in-game indicator — discovery relies on player messages, audio cues, or experimentation.

### 3.4 Gating Mechanics

Progression is gated by:
- **Keys:** Master Key, Basement Key, Key to Depths, Blighttown Key, Crest of Artorias, etc.
- **Boss defeats:** Many doors and paths open only after specific boss kills.
- **Bells of Awakening:** Two bells (Undead Parish and Blighttown) must be rung to open Sen's Fortress.
- **Lordvessel:** Placing the Lordvessel at Firelink Altar opens golden fog gates to the four Lord Soul areas.
- **Lord Souls:** All four must be collected to open the Kiln of the First Flame.

### 3.5 Bonfires

43 bonfires in the original game (44 in Remastered). Bonfires serve as:
- Respawn checkpoints
- Healing refill points (Estus Flask)
- Leveling stations (spend Souls to increase stats)
- Spell attunement points
- Humanity reversal points (spend 1 Humanity to become Human)
- Kindling points (spend 1 Humanity to increase Estus charges)

**Kindling tiers:**
| Tier | Estus Charges | Requirement |
|---|---|---|
| Unkindled | 5 | Default |
| Kindled (1×) | 10 | 1 Humanity |
| Kindled (2×) | 15 | 1 Humanity + Rite of Kindling |
| Kindled (3×) | 20 | 1 Humanity + Rite of Kindling |

Bonfires tended by Fire Keepers are pre-kindled to 10 charges. The Rite of Kindling is obtained by defeating Pinwheel in the Catacombs. Killing a Fire Keeper extinguishes their bonfire (cannot rest, but can still warp to it).

### 3.6 Fast Travel

No fast travel until the **Lordvessel** is obtained (after defeating Ornstein and Smough in Anor Londo, roughly the midpoint). The Lordvessel enables warping between a subset of ~20 bonfires. Not all bonfires are warp destinations. All DLC bonfires are warpable.

---

## 4. Playable Characters / Classes

### 4.1 Character Creation

Players create a single character, choosing: name, gender (cosmetic only), appearance, starting class, and a burial gift. Classes determine starting stats and equipment but do not restrict future development — any class can eventually reach any stat distribution.

### 4.2 Starting Classes

| Class | SL | VIT | ATT | END | STR | DEX | RES | INT | FTH | Starting Equipment |
|---|---|---|---|---|---|---|---|---|---|---|
| Warrior | 4 | 11 | 8 | 12 | 13 | 13 | 11 | 9 | 9 | Longsword, Heater Shield, chain armor |
| Knight | 5 | 14 | 10 | 10 | 11 | 11 | 10 | 9 | 11 | Broadsword, Tower Kite Shield, heavy armor |
| Wanderer | 3 | 10 | 11 | 10 | 10 | 14 | 12 | 11 | 8 | Scimitar, Leather Shield, wanderer set |
| Thief | 5 | 9 | 11 | 9 | 9 | 15 | 10 | 12 | 11 | Bandit's Knife, Target Shield, Master Key |
| Bandit | 4 | 12 | 8 | 14 | 14 | 9 | 11 | 8 | 10 | Battle Axe, Spider Shield, bandit set |
| Hunter | 4 | 11 | 9 | 11 | 12 | 14 | 11 | 9 | 9 | Short Bow, Shortsword, Large Leather Shield |
| Sorcerer | 3 | 8 | 15 | 8 | 9 | 11 | 8 | 15 | 8 | Mail Breaker, Sorcerer's Catalyst, Soul Arrow |
| Pyromancer | 1 | 10 | 12 | 11 | 12 | 9 | 12 | 10 | 8 | Hand Axe, Pyromancy Flame, Fireball |
| Cleric | 2 | 11 | 11 | 9 | 12 | 8 | 11 | 8 | 14 | Mace, East-West Shield, Talisman, Heal |
| Deprived | 6 | 11 | 11 | 11 | 11 | 11 | 11 | 11 | 11 | Club, Plank Shield |

SL = Soul Level. All classes start with a total of ~80 stat points (71 base + Soul Level). The Pyromancer at SL 1 is the traditional choice for challenge runs.

### 4.3 Burial Gifts

Chosen at character creation. One-time starting bonus:
- Divine Blessing (full heal + status cure)
- Black Firebomb ×10
- Twin Humanities
- Binoculars
- Pendant (no known gameplay effect)
- Master Key (unlocks many shortcut doors)
- Tiny Being's Ring (slight HP boost)
- Old Witch's Ring (enables dialogue with Quelaag's Sister)

### 4.4 Stats

**Primary Attributes** (max 99 each):

| Stat | Effect | Soft Cap | Notes |
|---|---|---|---|
| Vitality | HP | 30, 50 | ~1,100 HP at 50 VIT |
| Attunement | Spell slots | 50 | See §4.5 for slot table |
| Endurance | Stamina + Equip Load | 40 | Stamina caps at 160 at 40 END; equip load continues scaling |
| Strength | Melee scaling, weapon requirements | 40 | 1.5× effective STR when two-handing |
| Dexterity | Melee scaling, casting speed | 40, 45 | Casting speed increases up to 45 DEX |
| Resistance | Poison/status resist, defense | — | Widely considered the weakest stat; not recommended for leveling |
| Intelligence | Sorcery scaling, magic damage | 40, 50 | Required for sorceries and Magic/Enchanted weapons |
| Faith | Miracle scaling, lightning damage | 40, 50 | Required for miracles and Divine/Occult weapons |

Every stat point also raises all four defense types (physical, magic, fire, lightning) by a small amount, regardless of which stat is leveled.

### 4.5 Attunement Slots

| Attunement Level | Slots |
|---|---|
| 10–11 | 1 |
| 12–13 | 2 |
| 14–15 | 3 |
| 16–18 | 4 |
| 19–22 | 5 |
| 23–27 | 6 |
| 28–33 | 7 |
| 34–40 | 8 |
| 41–49 | 9 |
| 50+ | 10 |

White Seance Ring and Darkmoon Seance Ring each add +1 slot, for a maximum of 12.

### 4.6 Leveling

Leveling is done at any Bonfire by spending Souls. Each level increases one stat by 1 point and raises Soul Level by 1.

**Soul cost formula:** `Souls = 0.02x³ + 3.06x² + 105.6x − 895` (where x = target Soul Level).

Maximum Soul Level is 710–713 depending on starting class (Sorcerer reaches 713, the highest). Enemies do not scale with player level.

### 4.7 Humanity

Humanity exists in two forms:
- **Hard Humanity:** Consumable items (Humanity, Twin Humanities) that add to the soft counter.
- **Soft Humanity:** Counter displayed in the top-left HUD corner (number next to the humanity sprite).

**Effects of soft Humanity:**
- Spend 1 at a Bonfire to reverse Hollowing (become Human form, enabling multiplayer).
- Spend 1 at a Bonfire to kindle it (+5 Estus charges).
- Increases Item Discovery up to 10 Humanity (caps at +210 discovery at 10).
- Increases Curse resistance.
- Increases damage of Chaos weapons (scales up to 10 Humanity).
- All soft Humanity is lost on death (recoverable from bloodstain).
- Counter displays up to 99 but gameplay effects cap at 10 (Item Discovery, Chaos scaling, Curse resistance).

---

## 5. Story & Progression

### 5.1 Narrative Structure

Dark Souls uses environmental storytelling and item descriptions rather than cutscenes or dialogue trees. The main narrative is conveyed through:
- Brief NPC dialogue (no dialogue choices)
- Item descriptions on every weapon, armor, ring, and consumable
- Environmental details and enemy placement
- Opening cinematic establishing lore

### 5.2 Main Progression

1. **Northern Undead Asylum** — Tutorial. Defeat Asylum Demon. Rescued by Oscar of Astora.
2. **Ring the two Bells of Awakening:**
   - Bell 1: Atop Undead Parish (defeat Bell Gargoyles)
   - Bell 2: Bottom of Blighttown (defeat Chaos Witch Quelaag)
3. **Sen's Fortress opens** — Navigate the trap-filled fortress, defeat Iron Golem.
4. **Anor Londo** — Defeat Ornstein and Smough to obtain the Lordvessel.
5. **Collect four Lord Souls** (non-linear order):
   - Seath the Scaleless (Duke's Archives / Crystal Cave)
   - Gravelord Nito (Tomb of the Giants)
   - The Bed of Chaos (Lost Izalith)
   - Four Kings (The Abyss / New Londo Ruins)
6. **Kiln of the First Flame** — Defeat Gwyn, Lord of Cinder. Choose ending.

### 5.3 Endings

Two endings, chosen immediately after defeating Gwyn:
- **Link the Fire:** Light the bonfire in Gwyn's arena. The Chosen Undead sacrifices themselves to rekindle the First Flame, prolonging the Age of Fire.
- **The Dark Lord:** Walk away from the bonfire and exit the arena. The Chosen Undead ushers in the Age of Dark, becoming the Dark Lord.

No prerequisites beyond defeating Gwyn. The player need not have spoken to Frampt or Kaathe.

### 5.4 New Game Plus

After completing the game, NG+ begins automatically with all equipment, levels, and most key items retained. Key changes:

| Cycle | Enemy HP Multiplier | Soul Reward Multiplier |
|---|---|---|
| NG+ (2nd) | ~2.0× | ~2–4× |
| NG+2 (3rd) | ~2.14× | ~2.14× over NG+ |
| NG+3 (4th) | ~2.34× | increasing |
| NG+4 (5th) | ~2.54× | increasing |
| NG+5 (6th) | ~2.73× | increasing |
| NG+6 (7th+) | ~2.88× (cap) | cap |

Difficulty plateaus at NG+6 (7th playthrough). Key items, keys, and covenant progress are reset. Bonfires return to unkindled state.

---

## 6. Items & Equipment

### 6.1 Equipment Slots

| Slot | Count | Notes |
|---|---|---|
| Right Hand | 2 | Weapons, catalysts, talismans, flames |
| Left Hand | 2 | Shields, weapons, catalysts |
| Head | 1 | Helms |
| Chest | 1 | Armor |
| Hands | 1 | Gauntlets |
| Legs | 1 | Leggings |
| Rings | 2 | Cannot equip two of the same ring |
| Quick Items | 5 | Consumables, cycled with D-pad |
| Arrows | 2 | Arrow types for equipped bow |
| Bolts | 2 | Bolt types for equipped crossbow |

### 6.2 Weapon Categories

22 weapon categories with distinct movesets:

| Category | Count | Notable Traits |
|---|---|---|
| Daggers | 6 | Fast, high critical modifier, short range |
| Straight Swords | 13 | Balanced speed and range |
| Greatswords | 14 | Slow, high damage, wide sweeps |
| Ultra Greatswords | 5 | Very slow, massive damage and stagger |
| Curved Swords | 7 | Fast slashing, good for Dex builds |
| Katanas | 4 | Fast, bleed buildup, fragile |
| Curved Greatswords | 3 | Sweeping arcs, moderate speed |
| Piercing Swords | 5 | Thrust attacks, high critical, good behind shields |
| Axes | 7 | Moderate speed, high stagger |
| Great Axes | 5 | Slow, devastating damage |
| Hammers | 9 | Strike damage, good vs. armored enemies |
| Great Hammers | 6 | Massive stagger, very slow |
| Fists & Claws | 4 | Very fast, short range |
| Spears | 10 | Thrust while blocking, good range |
| Halberds | 9 | Long range, varied movesets |
| Whips | 3 | Cannot backstab/riposte, ignores shields partially |
| Bows | 5 | Ranged, manual aim or lock-on |
| Greatbows | 2 | Slow, massive ranged damage, staggers |
| Crossbows | 4 | Ranged, no stat scaling, one-hand usable |
| Catalysts | 11 | Cast sorceries, scale with INT |
| Talismans | 7 | Cast miracles, scale with FTH |
| Pyromancy Flames | 2 | Cast pyromancies, upgraded independently |

### 6.3 Weapon Scaling

Weapons have scaling grades per stat:

| Grade | Approximate Bonus |
|---|---|
| S | ~140%+ |
| A | ~100–139% |
| B | ~75–99% |
| C | ~50–74% |
| D | ~25–49% |
| E | ~1–24% |
| — | No scaling |

### 6.4 Weapon Upgrade Paths

| Path | Levels | Materials | Blacksmith | Key Property |
|---|---|---|---|---|
| Standard | +0 → +15 | Titanite Shards → Large → Chunks → Slab | Andre | Increases base damage + stat scaling |
| Raw | +0 → +5 | Large Titanite Shards | Andre | High base, minimal scaling |
| Crystal | +0 → +5 | Titanite Chunks + Slab | Giant | Highest physical damage, cannot be repaired |
| Magic | +0 → +10 | Green Titanite → Blue Chunks → Blue Slab | Rickert | Adds magic damage, INT scaling |
| Enchanted | +0 → +5 | Blue Titanite Chunks + Slab | Rickert | Higher INT scaling than Magic, less base |
| Divine | +0 → +10 | Green Titanite → White Chunks → White Slab | Andre | FTH scaling, prevents skeleton resurrection |
| Occult | +0 → +5 | White Titanite Chunks + Slab | Andre | Higher FTH scaling, bonus vs. divine enemies |
| Fire | +0 → +10 | Green Titanite → Red Chunks → Red Slab | Vamos | Fire damage, no stat scaling |
| Chaos | +0 → +5 | Red Titanite Chunks + Slab | Vamos | Fire damage, scales with soft Humanity (0–10) |
| Lightning | +0 → +5 | Titanite Chunks + Slab | Giant | Lightning damage, no stat scaling |
| Boss (Demon) | +0 → +5 | Demon Titanite | Giant | Unique boss weapons from boss souls |
| Dragon | +0 → +5 | Dragon Scales | Giant | Unique scaling across all stats |

**Ascension branch points:** Weapons don't follow a single linear track. At key reinforcement levels, a blacksmith can ascend the weapon onto a different path (consuming the base weapon):

| Base | Ascends To | Ember Required | Blacksmith |
|---|---|---|---|
| Standard +5 | Standard +6 (continues to +10) | Large Ember | Andre |
| Standard +5 | Raw +0 (continues to +5) | Large Ember | Andre |
| Standard +5 | Divine +0 (continues to +5) | Divine Ember | Andre |
| Standard +5 | Magic +0 (continues to +5) | (none) | Rickert |
| Standard +5 | Fire +0 (continues to +5) | (none) | Vamos |
| Standard +10 | Standard +11 (continues to +15) | Very Large Ember | Andre |
| Standard +10 | Crystal +0 (continues to +5) | Crystal Ember | Giant |
| Standard +10 | Lightning +0 (continues to +5) | (none) | Giant |
| Divine +5 | Divine +6 (continues to +10) | Large Divine Ember | Andre |
| Divine +5 | Occult +0 (continues to +5) | Dark Ember | Andre |
| Magic +5 | Magic +6 (continues to +10) | Large Magic Ember | Rickert |
| Magic +5 | Enchanted +0 (continues to +5) | Enchanted Ember | Rickert |
| Fire +5 | Fire +6 (continues to +10) | Large Flame Ember | Vamos |
| Fire +5 | Chaos +0 (continues to +5) | Chaos Flame Ember | Vamos |

Embers are found as loot throughout the world and must be given to the appropriate blacksmith before ascension is available. Unique weapons (boss weapons, Twinkling Titanite weapons) cannot be ascended — they follow their own upgrade track using special materials.

**Titanite material quantities (Standard path):**
- +0 to +5: 9 Titanite Shards
- +6 to +10: 9 Large Titanite Shards
- +11 to +14: 7 Titanite Chunks
- +15: 1 Titanite Slab

**Special weapon materials:**
- **Twinkling Titanite:** Used to upgrade unique/special weapons (Black Knight weapons, Silver Knight weapons, dragon tail weapons, etc.) up to +5. No ascension.
- **Demon Titanite:** Used to upgrade boss soul weapons up to +5.
- **Dragon Scales:** Used to upgrade Dragon path weapons up to +5.

### 6.5 Armor

Armor is divided by weight class:
- **Light** (0–19.9 total weight): Low poise, low defense, low weight
- **Medium** (20–39.9): Balanced
- **Heavy** (40+): High poise, high defense, high weight

Armor provides:
- **Physical Defense:** Flat damage reduction per piece
- **Elemental Resistance:** Percentage-based reduction (magic, fire, lightning)
- **Poise:** Stagger resistance (see §1.3)
- Armor can be upgraded at blacksmiths using Titanite materials (up to +5 or +10 depending on the set).

### 6.6 Rings

41 rings total. 2 ring slots. Cannot equip duplicates.

Notable rings:
| Ring | Effect |
|---|---|
| Havel's Ring | +50% equip load |
| Ring of Favor and Protection | +20% HP, stamina, and equip load; breaks if removed |
| Dark Wood Grain Ring | Replaces roll with a flip (extended i-frames, faster recovery) |
| Wolf Ring | +40 Poise |
| Hornet Ring | +30% critical damage |
| Bellowing Dragoncrest Ring | +20% sorcery damage |
| Cloranthy Ring | Increases stamina recovery |
| Covetous Gold Serpent Ring | +200 Item Discovery |
| Covetous Silver Serpent Ring | +20% souls gained |
| Red Tearstone Ring | +50% damage when HP < 20% |
| Blue Tearstone Ring | +25% defense when HP < 20% |

### 6.7 Estus Flask

Primary healing item. Cannot be used during rolling or certain animations. Charges refill on resting at a Bonfire. See §3.5 for kindling tiers. Upgraded by giving Fire Keeper Souls to any living Fire Keeper.

| Level | HP Healed |
|---|---|
| +0 | 300 |
| +1 | 400 |
| +2 | 500 |
| +3 | 600 |
| +4 | 650 |
| +5 | 700 |
| +6 | 750 |
| +7 | 800 |

There are 7 Fire Keeper Souls available per playthrough. Three Fire Keepers can perform the upgrade: the Firelink Shrine Fire Keeper (Anastacia), the Daughter of Chaos, and the Darkmoon Knightess in Anor Londo.

### 6.8 Consumables

Key consumable categories:
- **Soul items:** Consumable souls (Soul of a Lost Undead, Large Soul, etc.) granting fixed Soul amounts.
- **Resins/Buffs:** Gold Pine Resin (lightning), Charcoal Pine Resin (fire), Rotten Pine Resin (poison) — temporary weapon buffs.
- **Moss Clumps:** Purple Moss (poison cure), Blooming Purple Moss (toxic cure), Red Moss (bleed cure).
- **Prism Stones:** Dropped to test fall height (scream = lethal).
- **Homeward Bone:** Return to last Bonfire.
- **Humanity / Twin Humanities:** Add to soft Humanity counter.
- **Repair Powder:** Repair equipped weapon.
- **Purging Stone:** Cure Curse.

---

## 7. Magic System

### 7.1 Overview

Three schools of magic, each with its own catalyst, scaling stat, and spell list. Spells have limited uses per attunement, refilled at Bonfires. Some powerful spells require 2 attunement slots.

### 7.2 Sorceries

- **Catalyst:** Staff-type catalysts (11 varieties)
- **Scaling stat:** Intelligence
- **Total spells:** 27 (including DLC)
- **Key spells:**

| Spell | INT Req | Uses | Slots | Effect |
|---|---|---|---|---|
| Soul Arrow | 10 | 30 | 1 | Basic ranged soul projectile |
| Great Soul Arrow | 14 | 20 | 1 | Stronger soul projectile |
| Heavy Soul Arrow | 12 | 12 | 1 | Slow, high-damage projectile |
| Great Heavy Soul Arrow | 16 | 8 | 1 | Powerful slow projectile |
| Homing Soulmass | 18 | 10 | 1 | Orbiting projectiles, auto-fire on lock |
| Crystal Soul Spear | 44 | 4 | 1 | Piercing high-damage projectile |
| Soul Spear | 36 | 4 | 1 | High-damage ranged attack |
| Magic Weapon | 10 | 5 | 1 | Weapon buff (magic damage) |
| Crystal Magic Weapon | 25 | 3 | 1 | Strong weapon buff |
| White Dragon Breath | 50 | 20 | 1 | Line AoE along ground |
| Homing Crystal Soulmass | 36 | 10 | 1 | Stronger homing projectiles |
| Hidden Body | 14 | 3 | 1 | Semi-invisibility |
| Cast Light | 14 | 3 | 1 | Illumination in dark areas |

### 7.3 Miracles

- **Catalyst:** Talismans (7 varieties)
- **Scaling stat:** Faith
- **Total spells:** 23
- **Key spells:**

| Spell | FTH Req | Uses | Slots | Effect |
|---|---|---|---|---|
| Heal | 12 | 5 | 1 | Self heal |
| Great Heal | 24 | 3 | 1 | Large self heal |
| Great Heal Excerpt | 14 | 3 | 1 | Moderate self heal |
| Homeward | 18 | 1 | 1 | Return to bonfire |
| Force | 12 | 21 | 1 | AoE knockback |
| Wrath of the Gods | 28 | 3 | 1 | Powerful AoE damage |
| Lightning Spear | 20 | 10 | 1 | Ranged lightning bolt |
| Great Lightning Spear | 30 | 10 | 1 | Stronger lightning bolt |
| Sunlight Spear | 50 | 5 | 2 | Strongest lightning attack |
| Sunlight Blade | 30 | 1 | 1 | Lightning weapon buff |
| Darkmoon Blade | 30 | 1 | 1 | Magic weapon buff (covenant) |
| Tranquil Walk of Peace | 18 | 3 | 1 | AoE slow |
| Vow of Silence | 30 | 2 | 2 | AoE spell suppression |

### 7.4 Pyromancies

- **Catalyst:** Pyromancy Flame (upgradeable +0 to +15, then Ascended +0 to +5)
- **Scaling stat:** None (damage scales only with Pyromancy Flame upgrade level)
- **Total spells:** 20 (including DLC)
- **Key spells:**

| Spell | Uses | Slots | Effect |
|---|---|---|---|
| Fireball | 8 | 1 | Thrown fireball |
| Great Fireball | 4 | 1 | Larger fireball |
| Great Chaos Fireball | 4 | 2 | Fireball + lingering lava (covenant reward) |
| Combustion | 16 | 1 | Close-range fire burst |
| Great Combustion | 8 | 1 | Stronger close-range burst |
| Fire Surge | 80 | 1 | Continuous fire stream |
| Power Within | 1 | 1 | Self-buff: +40% damage, drains HP |
| Iron Flesh | 3 | 1 | Massive defense boost, slows movement |
| Flash Sweat | 3 | 1 | Fire resistance buff |
| Black Flame | 8 | 1 | Close-range dark fire (DLC) |

Pyromancies are uniquely stat-agnostic — any build can use them effectively as long as they have attunement slots.

---

## 8. Enemies & Bosses

### 8.1 Enemy Design Philosophy

Enemies do not scale with player level. Each area has fixed enemy types with set stats. Enemy aggression patterns include:
- **Patrol:** Walk a set route, attack on sight
- **Ambush:** Hidden enemies triggered by proximity
- **Group:** Multiple enemies in close quarters
- **Ranged:** Archers and casters positioned at elevation

All non-boss enemies respawn when the player rests at a Bonfire or dies.

**Notable enemy types:**
- **Mimics:** Disguised as treasure chests. Opening one triggers a grab attack that deals massive damage. Identified by a straightened chain (real chests have a coiled chain) or by watching for subtle breathing. Hitting a Mimic with a Lloyd's Talisman forces it to open without combat. Each Mimic has a fixed loot drop plus a random chance to drop the Symbol of Avarice (head piece that boosts Item Discovery and Souls gained but drains HP).
- **Black Knights:** Powerful armored enemies found throughout the world. Do not respawn until NG+. Drop rare Black Knight weapons with high base damage.
- **Titanite Demons:** Mini-boss enemies found in several locations. High HP and damage. Drop Demon Titanite for boss weapon upgrades.
- **Crystal Lizards:** Flee on sight and vanish if not killed quickly. Drop titanite upgrade materials. Limited spawn count per area per playthrough.
- **Necromancers:** Catacombs enemies that reanimate nearby skeletons. Killing the Necromancer prevents skeleton respawns in their zone (Divine weapons also prevent skeleton resurrection).
- **Skeletons (Catacombs):** Reassemble after death unless killed with a Divine weapon or after their linked Necromancer is dead.

### 8.2 Boss List

26 bosses total (22 base game + 4 DLC). Mandatory bosses marked with ★. Bosses without ★ can be skipped entirely — several early bosses (Taurus Demon, Capra Demon, Gaping Dragon) become optional with the Master Key gift, which opens alternate routes bypassing their areas.

| Boss | Area | HP (NG) | Souls (NG) | Key Drop |
|---|---|---|---|---|
| Asylum Demon ★ | N. Undead Asylum | 813 | 2,000 | — |
| Taurus Demon | Undead Burg | ~1,215 | 3,000 | Demon's Greataxe (rare), 1 Humanity, Homeward Bone |
| Bell Gargoyles ★ | Undead Parish | 1,000 / 480 | 10,000 | Twin Humanities |
| Capra Demon | Lower Undead Burg | 1,176 | 6,000 | Key to Depths, 1 Humanity |
| Gaping Dragon | Depths | ~4,401 | 25,000 | Blighttown Key, Twin Humanities |
| Moonlight Butterfly | Darkroot Garden | ~1,506 | 10,000 | Soul of the Moonlight Butterfly |
| Chaos Witch Quelaag ★ | Blighttown | 3,139 | 20,000 | Soul of Quelaag, Twin Humanities |
| Great Grey Wolf Sif ★ | Darkroot Garden | ~3,432 | 40,000 | Covenant of Artorias, Soul of Sif |
| Iron Golem ★ | Sen's Fortress | 2,880 | 40,000 | Core of an Iron Golem, 1 Humanity |
| Ornstein & Smough ★ | Anor Londo | O: 1,642 / S: 2,645 | 50,000 | Soul of Ornstein or Smough, Leo Ring (if Super Ornstein) |
| Crossbreed Priscilla | Painted World | ~2,000 | 25,000 | Soul of Priscilla |
| Dark Sun Gwyndolin | Anor Londo | ~2,529 | 40,000 | Soul of Gwyndolin |
| Stray Demon | N. Undead Asylum (revisit) | ~5,250 | 20,000 | Titanite Slab, 2 Humanity |
| Pinwheel ★ | Catacombs | ~1,326 | 15,000 | Rite of Kindling, 1 Humanity |
| Gravelord Nito ★ | Tomb of the Giants | 4,317 | 60,000 | Lord Soul |
| Seath the Scaleless ★ | Crystal Cave | 5,525 | 60,000 | Bequeathed Lord Soul Shard |
| Ceaseless Discharge ★ | Demon Ruins | ~4,200 | 20,000 | 1 Humanity |
| Demon Firesage | Demon Ruins | ~5,950 | 20,000 | Demon's Catalyst |
| Centipede Demon | Demon Ruins | ~3,432 | 40,000 | Orange Charred Ring |
| Bed of Chaos ★ | Lost Izalith | 1 (puzzle) | 60,000 | Lord Soul |
| Four Kings ★ | The Abyss | 9,504 (shared) | 60,000 | Bequeathed Lord Soul Shard |
| Gwyn, Lord of Cinder ★ | Kiln of the First Flame | 4,185 | 70,000 | Soul of Gwyn |
| **DLC Bosses** | | | | |
| Sanctuary Guardian | Sanctuary Garden | ~2,560 | 30,000 | Guardian Soul |
| Knight Artorias | Oolacile Colosseum | ~3,750 | 50,000 | Soul of Artorias |
| Manus, Father of the Abyss | Chasm of the Abyss | ~6,665 | 60,000 | Soul of Manus |
| Black Dragon Kalameet | Royal Wood | ~5,400 | 60,000 | Calamity Ring |

HP values marked with ~ are community-verified estimates where exact datamined values vary by source. Super Ornstein has 2,981 HP; Super Smough has 4,094 HP.

### 8.3 Boss Design Patterns

- **Gank bosses:** Multiple enemies (Bell Gargoyles — second gargoyle at 50% HP; Ornstein & Smough — two simultaneous)
- **Puzzle bosses:** Bed of Chaos (destroy two roots + jump to bug core; progress persists through deaths)
- **DPS race:** Four Kings (new king spawns every ~45 seconds; must kill fast to avoid being overwhelmed)
- **Phase transitions:** Ornstein & Smough (killing one empowers the other into "Super" form with new moves and increased stats)
- **Gimmick bosses:** Ceaseless Discharge (can be killed instantly by luring to fog gate), Iron Golem (can be staggered off the bridge)
- **Self-buff:** Knight Artorias powers up if given time, increasing damage until death

---

## 9. Economy

### 9.1 Currency

**Souls** are the single universal currency used for:
- Leveling up at Bonfires
- Purchasing items from merchants
- Weapon/armor reinforcement at blacksmiths
- Feeding to Frampt for item exchange

Souls are gained by killing enemies, consuming Soul items, and multiplayer activity. All held Souls are lost on death (recoverable once).

### 9.2 Merchants

| Merchant | Location | Specialty |
|---|---|---|
| Undead Male Merchant | Undead Burg | Basic consumables, arrows, Residence Key |
| Undead Female Merchant | Lower Undead Burg | Moss, Transient Curse, arrows |
| Andre of Astora | Undead Parish | Standard/Raw/Divine/Occult upgrades, smithbox |
| Rickert of Vinheim | New Londo Ruins | Magic/Enchanted upgrades, sorceries |
| Vamos | Catacombs | Fire/Chaos upgrades |
| Giant Blacksmith | Anor Londo | Crystal/Lightning/Boss/Dragon upgrades, arrows |
| Domhnall of Zena | Depths → Firelink | Unique armor sets, boss equipment |
| Crestfallen Merchant | Sen's Fortress | Arrows, consumables |
| Patches | Tomb of the Giants → Firelink | Cleric equipment, consumables |
| Eingyi | Quelaag's Domain | Pyromancies (if infected with Egg-head) |

### 9.3 Spell Trainers

| Trainer | Location | School |
|---|---|---|
| Griggs of Vinheim | Lower Undead Burg → Firelink | Sorcery |
| Big Hat Logan | Sen's Fortress → Duke's Archives | Advanced sorcery |
| Dusk of Oolacile | Darkroot Basin | Special sorceries |
| Petrus of Thorolund | Firelink Shrine | Miracles |
| Rhea of Thorolund | Tomb of the Giants → Undead Parish | Advanced miracles |
| Laurentius of the Great Swamp | Depths → Firelink | Pyromancy |
| Quelana of Izalith | Blighttown | Advanced pyromancy |
| Eingyi | Quelaag's Domain | Toxic/special pyromancy |

### 9.4 Kingseeker Frampt

The Primordial Serpent Frampt, found at Firelink Shrine after ringing both bells, can consume items in exchange for Souls. Useful for converting unneeded equipment and titanite into Souls.

---

## 10. Multiplayer

### 10.1 Online Structure

Asynchronous and synchronous multiplayer layered onto the single-player world. No dedicated lobbies or matchmaking screens.

**Asynchronous features:**
- **Messages:** Players leave short preset messages on the ground for others to read (warnings, hints, jokes). Messages can be rated, healing the author.
- **Bloodstains:** Touch another player's bloodstain to see a ghost replay of their final seconds before death.
- **Ghosts:** Faint phantoms of other players in the same area appear briefly.

### 10.2 Summoning (Co-op)

- Requires the host to be in **Human form**.
- Summoned players place a **White Sign Soapstone** sign on the ground. The host touches it to summon.
- Phantoms remain until the area boss is defeated, the host dies, or the phantom dies.
- Successful boss kill rewards the phantom with Souls and restores their Humanity.
- **Warriors of Sunlight** covenant members appear as gold phantoms and have unique interactions.

**Co-op level range:** Host Level ± (10 + 10% of Host Level).

**NPC summons:** Several NPCs place gold or white summon signs before specific boss fog gates. The player must be in Human form to see them. NPC phantoms fight alongside the player but increase the boss's HP. Notable NPC phantoms:

| NPC | Available For |
|---|---|
| Solaire of Astora | Bell Gargoyles, Gaping Dragon, Ornstein & Smough, Centipede Demon, Gwyn |
| Knight Lautrec | Bell Gargoyles, Gaping Dragon |
| Witch Beatrice | Moonlight Butterfly, Four Kings |
| Iron Tarkus | Iron Golem |
| Paladin Leeroy | Pinwheel, Gravelord Nito |
| Maneater Mildred | Chaos Witch Quelaag |

NPC availability depends on quest progression — killing or failing to rescue the NPC removes their summon sign.

### 10.3 Invasions (PvP)

- **Red Eye Orb / Cracked Red Eye Orb:** Invade another player's world as a hostile dark spirit. Goal: kill the host.
- **Blue Eye Orb:** Invade sinners (players who have committed sins like killing NPCs or betraying covenants).
- Host must be in Human form to be invaded.

**Invasion level range:** Invader Level − 10% (low end) to Invader Level + (20 + 10%) (high end). Invasions skew upward — invaders can invade players much higher level than themselves.

### 10.4 Remastered Differences

Dark Souls Remastered adds:
- Weapon Level matchmaking (in addition to Soul Level)
- 6-player multiplayer (up from 4)
- Dedicated servers (replacing peer-to-peer)
- Password matchmaking
- Covenant switching at Bonfires without sin
- Estus mechanics for phantoms (halved Estus charges, dried finger mechanics)
- Extra Bonfire near Vamos in the Catacombs

---

## 11. Covenants

9 covenants total. Players may belong to only one at a time. Leaving a covenant halves accumulated offerings and incurs sin (except in Remastered).

### 11.1 Covenant Details

| Covenant | Leader | Location | Join Requirement | Offerings | Rank Rewards |
|---|---|---|---|---|---|
| Way of White | Petrus | Firelink Shrine | Talk to Petrus | None | Reduced invasion frequency |
| Princess's Guard | Gwynevere | Anor Londo | Talk to Gwynevere | None | AoE healing miracles |
| Warriors of Sunlight | Altar of Sunlight | Undead Parish | 25 FTH (reduced by −5 per co-op boss kill) | Sunlight Medals | +0: Lightning Spear; +1: Great Lightning Spear; +3: Sunlight Spear (from Gwyn's soul) |
| Blade of the Darkmoon | Gwyndolin | Anor Londo | Darkmoon Séance Ring | Souvenirs of Reprisal | +0: Blue Eye Orb; +1: Darkmoon Blade miracle; +3: upgraded Darkmoon Blade |
| Forest Hunter | Alvina | Darkroot Garden | Talk to Alvina | None | Auto-summoned to defend forest against trespassers |
| Darkwraith | Kaathe | The Abyss | Defeat Four Kings before placing Lordvessel | Humanity | +0: Dark Hand; +1: Red Eye Orb; +2: Dark Armor set |
| Gravelord Servant | Nito's Coffin | Catacombs | Use Eye of Death in coffin | Eyes of Death | +0: Gravelord Sword + Gravelord Sword Dance; +1: Gravelord Greatsword Dance |
| Path of the Dragon | Everlasting Dragon | Ash Lake | Kneel before dragon | Dragon Scales | +0: Dragon Eye; +1: Dragon Head Stone; +2: Dragon Torso Stone |
| Chaos Servant | Quelaag's Sister | Quelaag's Domain | Talk to Fair Lady | Humanity | +0: Great Chaos Fireball; +2: Chaos Storm + shortcut to Lost Izalith |

**Rank thresholds:** +1 = 10 offerings, +2 = 30 offerings, +3 = 80 offerings.

**Sin and absolution:** Breaking a covenant or killing NPCs accrues sin. Oswald of Carim in the Gargoyle bell tower absolves sin for 500 Souls × Soul Level.

---

## 12. UI & HUD

### 12.1 HUD Layout

**Top-left corner:**
- **HP bar** (red): Horizontal bar showing current / max health
- **Stamina bar** (green): Below HP bar, shows current / max stamina
- **Humanity counter:** Number to the left of HP bar with a humanity sprite. Glowing = Human form; dim = Hollow form.
- **Status effect icons:** Displayed below stamina bar when active (poison, toxic, bleed buildup, etc.)

**Bottom-left corner:**
- **Equipped weapon/shield icons:** Shows current right-hand and left-hand equipment
- **Equipped spell icon:** Shows currently attuned spell with remaining uses
- **Quick item icon:** Shows currently selected consumable

**Bottom-right corner:**
- **Soul counter:** Current held Souls displayed numerically

### 12.2 HUD States

- **Normal exploration:** Full HUD visible
- **Boss encounters:** Boss HP bar appears at the bottom-center of the screen with boss name
- **Status buildup:** A meter bar appears on-screen when poison/toxic/bleed/curse is building up
- **Interaction prompts:** Context-sensitive button prompts appear when near interactable objects (bonfires, ladders, doors, items, NPCs)
- **Invasion/summoning notifications:** Text banners appear ("Dark spirit [name] has invaded!" / "[Name] has been summoned")

### 12.3 Menu Screens

- **Inventory:** Equipment, consumables, key items — browsed with tabs
- **Equipment screen:** Paper doll showing all equipped items with stat comparisons
- **Stats screen:** All character attributes, defenses, resistances displayed numerically
- **Attunement screen:** Slot spell loadout at Bonfires
- **System menu:** Save & Quit, options, network settings (no manual save — game auto-saves continuously)

---

## 13. Engine & Presentation Systems

### 13.1 Save System

Continuous autosave. The game saves after every significant action (killing enemies, picking up items, entering areas, opening doors). "Save & Quit" from the menu saves exact position and state. Up to 10 character slots, but only one save file per character — no manual save points or branching saves. Death is saved immediately — no save-scumming without external tools.

### 13.2 Camera

Third-person camera with lock-on targeting system:
- **Free camera:** Right stick controls camera rotation
- **Lock-on:** R3 / RS snaps camera to nearest target. Right stick switches between targets. Lock-on enables strafing movement.
- Camera can clip through walls in tight spaces (a known issue, especially in the original release)

### 13.3 Difficulty

No selectable difficulty levels. Difficulty is static and universal. The game's reputation for difficulty comes from:
- High enemy damage relative to player HP
- Limited healing resources
- Permanent Soul loss on repeated death
- Environmental hazards and ambushes
- No pause during gameplay
- Minimal hand-holding or guidance

### 13.4 Audio System

- **Dynamic music:** Most areas have no background music — only ambient sound. Music triggers during boss fights and a few key areas (Firelink Shrine, Ash Lake, Anor Londo). Boss music intensifies during phase changes.
- **Audio cues:** Enemy footsteps, weapon sounds, and environmental audio are critical gameplay information. Mimics have a distinct breathing sound. Hidden enemies can be detected by audio before visual contact.
- **Sound design in multiplayer:** Invasion audio cue plays when a dark spirit enters the world.

### 13.5 Framerate

Original console versions and Prepare to Die Edition run at 30 fps. Blighttown is notorious for dropping to 10–15 fps on original hardware. Dark Souls Remastered runs at 60 fps on all platforms, with 30 fps docked on Nintendo Switch.

---

## 14. Open Questions / Unverified

- **Exact i-frame counts per roll type:** Current wiki consensus is 11 frames at 30 fps for all standard rolls and 13 for the DWGR flip, but older forum posts report varying numbers (9 for fat roll). Frame counting methodology and game version may explain discrepancies.
- **Precise weapon scaling formulas:** The exact polynomial mapping scaling grade + stat level → bonus AR is known to the community through datamining, but coefficients differ between sources (SoulsPlanners vs. wikis).
- **Poise recovery timer:** Commonly cited as 3–5 seconds, but exact value (likely frame-based) has conflicting reports.
- **Boss HP discrepancies:** Some boss HP values on wikis differ by small amounts, likely due to rounding in datamine tools or version differences between original and Remastered.
- **Defense formula coefficients:** The piecewise defense formula in §1.6 is the most widely cited version, but minor coefficient variations exist across community resources.
- **Dark Wood Grain Ring equip load threshold:** Pre-patch allowed the flip at <50% equip load; post-patch (1.06) changed to <25%. Some sources disagree on whether the threshold is 25% or 50% in Remastered.

---

## 15. References

### Wikis
- [Dark Souls Wiki — Fextralife](https://darksouls.wiki.fextralife.com/)
- [Dark Souls Wiki — Wikidot](http://darksouls.wikidot.com/)
- [Dark Souls Wiki — Fandom](https://darksouls.fandom.com/)

### Community Guides
- [Dark Souls Cheat Sheet (smcnabb)](https://smcnabb.github.io/dark-souls-cheat-sheet/)
- [SoulsPlanner Dark Souls Calculator](https://soulsplanner.com/darksouls/)
- [Dark Souls Multi-Player Soul Level Range Calculator](https://mpql.net/tools/dark-souls/)
- [DS1 Soul Level Calculator](https://trifectaiii.github.io/DS1-Soul-Level-Calculator/)

### Analysis
- [Game UI Database — Dark Souls](https://www.gameuidatabase.com/gameData.php?id=25)

### Version Comparison
- Steam community discussions on Prepare to Die Edition vs. Remastered differences
