# The Elder Scrolls III: Morrowind — Gameplay Systems Spec

PC / Xbox, 2002. Expansions: Tribunal (Nov 2002), Bloodmoon (Jun 2003). This spec covers the Game of the Year Edition (all expansions).

---

## 1. Core Gameplay Systems

### 1.1 Primary Loop

Open-world first-person RPG. The player explores the island of Vvardenfell freely, accepting quests from NPCs, clearing dungeons, advancing through faction ranks, and pursuing the main quest at their own pace. No level-gating; the entire world is accessible from the start, though enemy difficulty varies by region.

### 1.2 Attributes

Eight primary attributes, each ranging 0–100 (base). Starting values determined by race + gender + class favored attributes (+10 each) + birthsign.

| Attribute | Governed Skills | Derived Effects |
|-----------|----------------|-----------------|
| **Strength** | Acrobatics, Armorer, Axe, Blunt Weapon, Long Blade | Melee damage modifier (+1% per point above 50, −1% below), carry weight (STR × 5), starting Health (STR + END) / 2, max Fatigue |
| **Intelligence** | Alchemy, Conjuration, Enchant, Security | Max Magicka (INT × Magicka Multiplier) |
| **Willpower** | Alteration, Destruction, Mysticism, Restoration | Spell success chance (WIL / 5), max Fatigue |
| **Agility** | Block, Light Armor, Marksman, Sneak | Hit chance (AGI / 5), evasion (AGI / 5), max Fatigue |
| **Speed** | Athletics, Hand-to-Hand, Short Blade, Unarmored | Movement speed |
| **Endurance** | Heavy Armor, Medium Armor, Spear | Health gain per level (END / 10), starting Health, max Fatigue |
| **Personality** | Illusion, Mercantile, Speechcraft | NPC disposition modifier ((PER − 50) / 2) |
| **Luck** | None | Minor bonus to all checks (LCK / 10) |

**Derived Stats:**
- **Health** = (Strength + Endurance) / 2 + (Level − 1) × (Endurance / 10). Health gain is NOT retroactive — only current base Endurance at the moment of level-up counts.
- **Magicka** = Intelligence × Magicka Multiplier (base 1.0×; racial and birthsign bonuses are additive).
- **Fatigue** = Strength + Willpower + Agility + Endurance.
- **Encumbrance** = Strength × 5.

### 1.3 Skills (27 Total)

Each skill belongs to one governing attribute and one of three specializations (Combat, Magic, Stealth). Nine skills per specialization.

**Combat Specialization:**

| Skill | Attribute | Function |
|-------|-----------|----------|
| Armorer | Strength | Repair weapons and armor with repair hammers |
| Athletics | Speed | Running and swimming speed |
| Axe | Strength | War axes (1H) and battle axes (2H) |
| Block | Agility | Automatic shield blocking chance |
| Blunt Weapon | Strength | Clubs, maces, morningstars (1H); warhammers, staves (2H) |
| Heavy Armor | Endurance | Daedric, Ebony, Dwemer, Imperial Templar, Steel, Iron armor |
| Long Blade | Strength | Longswords, broadswords, katanas, sabers (1H); claymores, dai-katanas (2H) |
| Medium Armor | Endurance | Orcish, Bonemold, Imperial Chain, Indoril armor |
| Spear | Endurance | Spears and halberds (2H) |

**Magic Specialization:**

| Skill | Attribute | Function |
|-------|-----------|----------|
| Alchemy | Intelligence | Brew potions from ingredients using apparatus |
| Alteration | Willpower | Water Walking, Levitate, Shield, Lock, Open, Jump, Slowfall, Feather, Burden |
| Conjuration | Intelligence | Summon creatures, Bound weapons/armor, Turn Undead, Command |
| Destruction | Willpower | Fire/Frost/Shock/Poison damage, Drain, Damage, Disintegrate, Weakness |
| Enchant | Intelligence | Create/use/recharge enchanted items |
| Illusion | Personality | Invisibility, Chameleon, Light, Charm, Calm, Frenzy, Demoralize, Paralyze, Blind, Sound, Silence |
| Mysticism | Willpower | Soul Trap, Telekinesis, Absorb, Dispel, Mark, Recall, Intervention, Detect, Reflect, Spell Absorption |
| Restoration | Willpower | Restore/Fortify attributes and stats, Cure effects, Resist effects, Turn Undead |
| Unarmored | Speed | Defense rating for unarmored body slots |

**Stealth Specialization:**

| Skill | Attribute | Function |
|-------|-----------|----------|
| Acrobatics | Strength | Jump height and fall damage reduction |
| Hand-to-Hand | Speed | Unarmed combat (damages Fatigue; damages Health on knocked-down targets) |
| Light Armor | Agility | Netch Leather, Chitin, Glass armor |
| Marksman | Agility | Bows, crossbows, throwing stars, throwing knives |
| Mercantile | Personality | Buy/sell price improvement |
| Security | Intelligence | Lockpicking and trap disarming |
| Short Blade | Speed | Daggers, shortswords, tantos, wakizashis (1H) |
| Sneak | Agility | Stealth movement, pickpocketing, sneak attacks |
| Speechcraft | Personality | Persuasion (Admire, Intimidate, Taunt, Bribe) |

**Starting Skill Values:**
- Major skills: 30
- Minor skills: 15
- Miscellaneous skills: 5
- Specialization bonus: +5 to all 9 skills in the matching specialization
- Racial bonuses: +5, +10, or +15 (varies by race; 45 total bonus points per race)
- All stack additively.

**Skill Advancement Rates:**
- Major skills: 0.75× normal XP required
- Minor skills: 1.0× (normal)
- Miscellaneous skills: 1.25× normal XP required
- Matching specialization: 0.8× (multiplicative with the above)

### 1.4 Leveling

A level-up triggers after 10 increases across any combination of Major and Minor skills. Miscellaneous skill increases do NOT count. The player must then rest (§11.5) to receive the level-up.

**Attribute Multiplier Table:**

At level-up, the player chooses 3 attributes to increase. The multiplier depends on how many skills governed by that attribute increased since the last level-up (ALL skill types count — Major, Minor, and Miscellaneous).

| Skill Increases | Multiplier |
|-----------------|-----------|
| 0 | ×1 (+1) |
| 1–4 | ×2 (+2) |
| 5–7 | ×3 (+3) |
| 8–9 | ×4 (+4) |
| 10+ | ×5 (+5) |

Luck always receives +1 (no skills govern it). Maximum theoretical attribute gain per level: +15 to attributes (+5 × 3) plus +1 to Luck. Attributes cap at 100 (base); Fortify effects can exceed this.

**Health Gain:** Endurance / 10 per level (using Endurance at the moment of level-up, including any points just added). Not retroactive.

**Level Cap:** No hard cap. Soft cap when all Major/Minor skills reach 100 — approximately level 65–78 depending on build. Skill trainers and items can push higher.

### 1.5 Fatigue — The Universal Modifier

The Fatigue Term `0.75 + 0.5 × (Current Fatigue / Max Fatigue)` affects nearly every action:

- Weapon hit chance
- Spell success chance
- Block chance
- Evasion / dodge
- Sneak detection
- Lockpicking / trap disarming
- Speechcraft persuasion
- Alchemy potion strength
- Mercantile barter prices
- Jump height

At full Fatigue: 1.25× multiplier. At zero Fatigue: 0.75× multiplier.

### 1.6 Combat — Melee

**Hit Chance:**
```
Hit Rate = (Weapon Skill + Agility/5 + Luck/10) × Fatigue Term
           + Fortify Attack − Blind on Attacker
Evasion  = (Defender Agility/5 + Defender Luck/10) × Defender Fatigue Term
           + Sanctuary on Defender
Chance   = Hit Rate − Evasion (clamped 0–100, rolled vs d100)
```

**Weapon Damage:**
```
Base Damage = Weapon Min + (Weapon Max − Weapon Min) × Charge Percent
Final       = Base Damage × Strength Modifier × Critical Multiplier
```

- **Charge:** Player holds the attack button; tap = minimum damage, full hold (~0.5s) = maximum. Scales linearly.
- **Attack Direction:** Standing = Chop. Strafing left/right = Slash. Moving forward/back = Thrust. Each weapon has separate min–max damage for each type. "Always Use Best Attack" option in Prefs overrides direction to use the weapon's highest-damage type.
- **Strength Modifier:** STR 100 = 1.5×, STR 50 = 1.0×, STR 0 = 0.5×.

**Sneak Attacks:**
- Melee critical: 4× damage.
- Ranged critical: 1.5× damage.
- Requires: target unaware, player sneaking and undetected.
- Enchantment on-strike damage is NOT multiplied.

**Blocking:**
```
Block Chance = (Block Skill + Agility/5 + Luck/10) × Fatigue Term
```
- Requires a shield equipped. Two-handed weapons prevent blocking.
- Automatic — no player input beyond equipping a shield.
- Hard cap: 50% block chance. Minimum: 10% when shield equipped.

### 1.7 Combat — Armor

**Armor Rating per piece:**
```
Piece AR = Base AR × (Armor Skill / 30)
```

**Slot Weight for Total AR:**

| Slot | Weight |
|------|--------|
| Cuirass | 30% |
| Shield, Helm, Greaves, Boots, R. Pauldron, L. Pauldron | 10% each |
| R. Gauntlet, L. Gauntlet | 5% each |

**Unarmored (empty slot or clothing):**
```
Slot AR = Unarmored Skill² × 0.0065
```

**Damage Reduction:**
```
Actual Damage = Attack Damage / min(1 + AR / Attack Damage, 4)
```
Damage can never be reduced below 25% of the original hit.

**Armor Material Tiers (ascending):**
- Light: Netch Leather → Chitin → Glass
- Medium: Imperial Chain → Bonemold → Orcish → Indoril
- Heavy: Iron → Steel → Dwemer → Imperial Templar → Ebony → Daedric

Full Daedric set: 266 base AR, 354 lbs weight, 300 enchantment capacity (450 with shield).

### 1.8 Combat — Ranged

Bows, crossbows, throwing stars, and throwing knives governed by Marksman skill. Same hit-chance formula as melee but using Marksman skill. Arrows and bolts are consumed on use. Sneak attack multiplier: 1.5× (vs 4× for melee).

### 1.9 Magic

**Six Schools:** Alteration, Conjuration, Destruction, Illusion, Mysticism, Restoration (see §1.3 for effect lists).

**Spell Cost:**
```
Effect Cost = ((Min Magnitude + Max Magnitude) × (Duration + 1) + Area) × Base Cost / 40
```
On Target range: multiply by 1.5×. Multiple On Target effects compound the 1.5× multiplier. Duration 0 (instant) counts as 1. Final cost rounded down.

**Spell Success Chance:**
```
Chance = (Skill × 2 + Willpower/5 + Luck/10 − Spell Cost − Sound Magnitude) × Fatigue Term
```
Clamped to [0, 100]. Racial powers and birthsign powers always succeed.

**Magicka Regeneration:** Only via rest (~15% of Intelligence per hour of rest). No passive regeneration while awake. Atronach birthsign prevents rest-based regeneration entirely (Spell Absorption is the primary recovery method).

### 1.10 Enchanting

**Four Types:**
1. **Cast Once** — scrolls; single use, destroyed after.
2. **Cast When Used** — any equippable item; costs charges; activated from magic menu.
3. **Cast When Strikes** — weapons only; triggers on hit; costs charges.
4. **Constant Effect** — permanent while equipped; requires soul value ≥ 400.

**Self-Enchant Success:**
```
Success% = (Enchant Skill + Intelligence/5 + Luck/10 − 3 × Enchantment Points) × Fatigue Term
```
Constant Effect: additional 0.5× multiplier. Higher Enchant skill reduces charge consumption per use.

**Soul Gems:**

| Soul Gem | Capacity |
|----------|----------|
| Petty | 30 |
| Lesser | 60 |
| Common | 120 |
| Greater | 180 |
| Grand | 600 |
| Azura's Star | ~15,000 (reusable) |

Key soul values: Golden Saint = 400, Ascended Sleeper = 400, Vivec = 500. Soul value ≥ 400 required for Constant Effect enchantments.

### 1.11 Alchemy

Each ingredient has 4 effects. Effects are hidden until discovered by eating the ingredient (reveals the 1st effect; higher Alchemy reveals more) or successfully combining two ingredients that share an effect.

**Apparatus (4 types, 4 quality tiers each):**

| Apparatus | Role | Quality Tiers |
|-----------|------|---------------|
| Mortar & Pestle | Required; base potion strength | Apprentice (0.5), Journeyman (1.0), Master (1.2), Grandmaster (1.5) |
| Retort | Increases positive effect strength | Same tiers |
| Alembic | Reduces negative effect strength | Same tiers |
| Calcinator | Increases ALL effect strength | Same tiers |

**Potion Strength:**
```
Strength = (Alchemy + Intelligence/10 + Luck/10) × Mortar Quality / (3 × Effect Base Cost)
```

### 1.12 Stealth

**Sneak Detection:**
```
Elusiveness = (Sneak + Agility/5 + Luck/10 − Shoe Weight)
              × (0.5 + Distance/500) × Fatigue Term + Chameleon Magnitude
Detection   = (NPC Sneak + NPC Agility/5 + NPC Luck/10 − Blind on NPC)
              × NPC Fatigue Term × Direction Multiplier
```
Player is hidden when Elusiveness > Detection. Checked continuously.

**Pickpocket:**
```
x = (0.2 × PC Agility + 0.1 × PC Luck + PC Sneak) × PC Fatigue Term
y = (Value Term + NPC Sneak + 0.2 × NPC Agility + 0.1 × NPC Luck) × NPC Fatigue Term
Chance = 2x − y
```
Known bug: effective cap ~56% because the check is rolled twice (once when taking, once when closing inventory).

**Security (Lockpicking / Trap Disarming):**
```
Success% = (Security + Agility/5 + Luck/10) × Equipment Modifier × Fatigue Term − Lock Level
```
Lock levels: 1–100. Probe (trap disarm) uses the same formula with Trap Level.

**Speechcraft Persuasion:**
Four options during dialogue:
- **Admire** — based on Speechcraft and Personality. Success raises disposition; failure lowers it.
- **Intimidate** — based on player level vs NPC level and Strength. Success raises temporary disposition but lowers permanent disposition.
- **Taunt** — lowers disposition and may provoke the NPC to attack (not a crime for the player to then fight back).
- **Bribe** — three tiers: 10 / 100 / 1,000 gold. Based on Mercantile skill. Gold deducted regardless of success.

All persuasion checks apply the Fatigue Term and factor in both player and NPC stats.

---

## 2. Controls & Input

### 2.1 PC Default (Keyboard + Mouse)

**Movement:**

| Key | Action |
|-----|--------|
| W / S | Forward / Backward |
| A / D | Strafe Left / Right |
| Left Shift | Run (hold) |
| Caps Lock | Toggle Always Run |
| Left Ctrl | Sneak (hold) |
| E | Jump |

**Actions:**

| Key | Action |
|-----|--------|
| Spacebar | Activate (use, open, talk, pick up) |
| F | Ready / sheathe weapon |
| R | Ready / sheathe magic |
| LMB | Attack (hold to charge) / Cast readied spell |
| Tab | Toggle first-person / third-person (hold + mouse = vanity camera) |

**UI & Menus:**

| Key | Action |
|-----|--------|
| RMB | Toggle Menu Mode (opens all four windows) |
| J | Journal |
| T | Rest / Wait |
| F1 | Quick Key assignment (assign spells/items to 1–9) |
| 1–9 | Use assigned Quick Key |
| 0 | Switch to Hand-to-Hand |
| F5 | Quicksave |
| F9 | Quickload |
| Escape | Options / Pause |
| ~ (tilde) | Console (PC only) |
| Mouse Wheel | Zoom in/out (third-person vanity view) |

### 2.2 Xbox Controller

| Input | Action |
|-------|--------|
| Left Stick | Move |
| Right Stick | Look / Aim |
| L-Stick Click | Sneak |
| R-Stick Click | Toggle POV |
| A | Activate |
| B | Menu Mode |
| X | Ready Weapon |
| Y | Ready Magic |
| RT | Attack / Cast |
| LT | Jump |
| RB | Rest |
| LB | Journal |
| Start | Pause |
| Back | Quick Key Menu |
| D-Pad | Quick Key selection (1–4) |

### 2.3 Context-Sensitive Controls

- **Combat:** Hold LMB to charge; movement direction at attack start determines Chop / Slash / Thrust.
- **Magic Ready:** When magic is readied (R), LMB casts instead of swinging a weapon.
- **Weapon Drawn:** Having a weapon drawn reduces NPC disposition by a fixed amount.
- **"Always Use Best Attack":** Option in Prefs overrides directional attacks, always using the weapon's highest-damage type.

---

## 3. World Structure

### 3.1 Vvardenfell Regions (9)

| Region | Character | Key Locations |
|--------|-----------|---------------|
| **Ascadian Isles** | Fertile agricultural south | Vivec, Pelagiad, Suran, Ebonheart |
| **Ashlands** | Ash desert surrounding Red Mountain | Ashlander camps, Ghostfence border |
| **Azura's Coast** | Rocky eastern shore | Tel Branora, Tel Mora, Tel Aruhn, Sadrith Mora |
| **Bitter Coast** | Western swamps | Seyda Neen (starting town), Hla Oad, Gnaar Mok |
| **Grazelands** | Northeastern pastures | Vos, Tel Vos, Ashlander camps |
| **Molag Amur** | Volcanic southeast interior | Molag Mar |
| **Red Mountain** | Central volcano, Ghostfence-enclosed | Dagoth Ur's citadel |
| **Sheogorad** | Northern islands | Dagon Fel |
| **West Gash** | Rocky western highlands | Balmora, Ald'ruhn, Caldera, Gnisis, Maar Gan |

Major cities: **Vivec** (largest; 9 cantons), **Balmora** (House Hlaalu seat), **Ald'ruhn** (House Redoran seat), **Sadrith Mora** (House Telvanni seat), **Ebonheart** (Imperial seat). Approximately 25+ named settlements total.

### 3.2 Travel Systems

**Silt Strider Network** (land-based, inter-city, costs gold):
Routes: Seyda Neen ↔ Balmora ↔ Ald'ruhn ↔ Gnisis ↔ Khuul; Balmora ↔ Vivec ↔ Suran ↔ Seyda Neen; Vivec ↔ Molag Mar; Ald'ruhn ↔ Maar Gan. ~9 stops total. Passes in-game time and counts as rest.

**Boat Network** (coastal, costs gold):
~12 stops counter-clockwise: Dagon Fel → Khuul → Gnaar Mok → Hla Oad → Ebonheart → Vivec → Molag Mar → Tel Branora → Sadrith Mora → Tel Aruhn → Tel Mora → Vos. Passes in-game time and counts as rest.

**Mages Guild Teleportation** (Guild Guide; instant, costs gold):
5 locations: Ald'ruhn, Balmora, Caldera, Vivec, Wolverine Hall (Sadrith Mora). Instant — does NOT count as rest.

**Propylon Chamber Network** (10 Dunmer strongholds; instant, free):
Circular chain of 10 strongholds (Valenvaryon → Indoranyon → Berandas → Andasreth → Rotheran → Falensarano → Telasero → Marandus → Falasmaryon → Hlormaren). Each connects to two adjacent chambers. Requires the correct Propylon Index item per destination. Master Propylon Index (official plugin) links all to Caldera.

**Spell-Based Travel:**
- **Mark / Recall** — Mark sets a single return point; Recall teleports to it.
- **Almsivi Intervention** — Teleports to nearest Tribunal Temple.
- **Divine Intervention** — Teleports to nearest Imperial Cult shrine.

All spell-based travel is instant and does NOT restore Health/Magicka.

### 3.3 Day/Night Cycle and Weather

- **Time scale:** 30:1. Two real minutes = one game hour. 48 real minutes = one full game day.
- **Weather types (base game):** Clear, Cloudy, Foggy, Overcast, Rain, Thunder, Ash Storm, Blight Storm.
- **Bloodmoon additions:** Snow, Blizzard (Solstheim-specific).
- Blight Storms originate from Red Mountain and carry blight diseases; they cease after completing the main quest.
- Each region has its own weather probability table.

---

## 4. Playable Characters / Classes

### 4.1 Races (10)

All races have a total of 310 attribute points. Gender redistributes ~10 points (typically between STR/END/SPD/PER/WIL). Each race receives 45 skill bonus points.

#### Altmer (High Elf)

| | STR | INT | WIL | AGI | SPD | END | PER | LCK |
|-|-----|-----|-----|-----|-----|-----|-----|-----|
| Male | 30 | 50 | 40 | 40 | 30 | 40 | 40 | 40 |
| Female | 30 | 50 | 40 | 40 | 40 | 30 | 40 | 40 |

Skills: Alchemy +10, Destruction +10, Enchant +10, Alteration +5, Conjuration +5, Illusion +5.
Abilities: Fortify Max Magicka 1.5×, Weakness to Magicka 50%, Weakness to Fire 50%, Weakness to Frost 25%, Weakness to Shock 25%, Resist Disease 75%.

#### Argonian

| | STR | INT | WIL | AGI | SPD | END | PER | LCK |
|-|-----|-----|-----|-----|-----|-----|-----|-----|
| Male | 40 | 40 | 30 | 50 | 50 | 30 | 30 | 40 |
| Female | 40 | 50 | 40 | 40 | 40 | 30 | 30 | 40 |

Skills: Athletics +15, Alchemy +5, Illusion +5, Medium Armor +5, Mysticism +5, Spear +5, Unarmored +5.
Abilities: Resist Disease 75%, Immune to Poison, Water Breathing.

#### Bosmer (Wood Elf)

| | STR | INT | WIL | AGI | SPD | END | PER | LCK |
|-|-----|-----|-----|-----|-----|-----|-----|-----|
| M/F | 30 | 40 | 30 | 50 | 50 | 30 | 40 | 40 |

Only race with identical male/female attributes.
Skills: Marksman +15, Light Armor +10, Sneak +10, Acrobatics +5, Alchemy +5.
Abilities: Resist Disease 75%. Power: Beast Tongue (Command Creature 5 pts for 600s on Target).

#### Breton

| | STR | INT | WIL | AGI | SPD | END | PER | LCK |
|-|-----|-----|-----|-----|-----|-----|-----|-----|
| Male | 40 | 50 | 50 | 30 | 30 | 30 | 40 | 40 |
| Female | 30 | 50 | 50 | 30 | 40 | 30 | 40 | 40 |

Skills: Conjuration +10, Mysticism +10, Restoration +10, Alchemy +5, Alteration +5, Illusion +5.
Abilities: Fortify Max Magicka 0.5×, Resist Magicka 50%. Power: Dragon Skin (Shield 50 pts for 60s on Self).

#### Dunmer (Dark Elf)

| | STR | INT | WIL | AGI | SPD | END | PER | LCK |
|-|-----|-----|-----|-----|-----|-----|-----|-----|
| Male | 40 | 40 | 30 | 40 | 50 | 40 | 30 | 40 |
| Female | 40 | 40 | 30 | 40 | 50 | 30 | 40 | 40 |

Skills: Destruction +10, Short Blade +10, Athletics +5, Light Armor +5, Long Blade +5, Marksman +5, Mysticism +5.
Abilities: Resist Fire 75%. Power: Ancestor Guardian (Sanctuary 50 pts for 60s on Self).

#### Imperial

| | STR | INT | WIL | AGI | SPD | END | PER | LCK |
|-|-----|-----|-----|-----|-----|-----|-----|-----|
| Male | 40 | 40 | 30 | 30 | 40 | 40 | 50 | 40 |
| Female | 40 | 40 | 40 | 30 | 30 | 40 | 50 | 40 |

Skills: Long Blade +10, Mercantile +10, Speechcraft +10, Blunt Weapon +5, Hand-to-Hand +5, Light Armor +5.
Powers: Star of the West (Absorb Fatigue 200 pts for 1s on Touch), Voice of the Emperor (Charm 25 pts for 15s on Target).

#### Khajiit

| | STR | INT | WIL | AGI | SPD | END | PER | LCK |
|-|-----|-----|-----|-----|-----|-----|-----|-----|
| Male | 40 | 40 | 30 | 50 | 40 | 30 | 40 | 40 |
| Female | 30 | 40 | 30 | 50 | 40 | 40 | 40 | 40 |

Skills: Acrobatics +15, Athletics +5, Hand-to-Hand +5, Light Armor +5, Security +5, Short Blade +5, Sneak +5.
Powers: Eye of Fear (Demoralize Humanoid 100 pts for 30s on Target), Eye of Night (Night Eye 50 pts for 30s on Self).

#### Nord

| | STR | INT | WIL | AGI | SPD | END | PER | LCK |
|-|-----|-----|-----|-----|-----|-----|-----|-----|
| Male | 50 | 30 | 40 | 30 | 40 | 50 | 30 | 40 |
| Female | 50 | 30 | 50 | 30 | 40 | 40 | 30 | 40 |

Skills: Axe +10, Blunt Weapon +10, Long Blade +10, Medium Armor +10, Heavy Armor +5, Spear +5.
Abilities: Resist Frost 100%, Immune to Frost. Powers: Woad (Shield 30 pts for 60s on Self), Thunder Fist (Frost Damage 25 pts on Touch).

#### Orc (Orsimer)

| | STR | INT | WIL | AGI | SPD | END | PER | LCK |
|-|-----|-----|-----|-----|-----|-----|-----|-----|
| Male | 45 | 30 | 50 | 35 | 30 | 50 | 30 | 40 |
| Female | 45 | 40 | 45 | 35 | 30 | 50 | 25 | 40 |

Skills: Armorer +10, Block +10, Heavy Armor +10, Medium Armor +10, Axe +5.
Abilities: Resist Magicka 25%. Power: Berserk (Fortify Health 20 + Fortify Attack 100 + Fortify Fatigue 200 + Drain Agility 100 for 60s on Self).

#### Redguard

| | STR | INT | WIL | AGI | SPD | END | PER | LCK |
|-|-----|-----|-----|-----|-----|-----|-----|-----|
| Male | 50 | 30 | 30 | 40 | 40 | 50 | 30 | 40 |
| Female | 40 | 30 | 30 | 40 | 40 | 50 | 40 | 40 |

Skills: Long Blade +15, Athletics +5, Axe +5, Blunt Weapon +5, Heavy Armor +5, Medium Armor +5, Short Blade +5.
Abilities: Resist Disease 75%, Resist Poison 75%. Power: Adrenaline Rush (Fortify AGI/END/SPD/STR 50 pts + Fortify Health 25 pts for 60s on Self).

### 4.2 Birthsigns (13)

Powers are once per 24 in-game hours, cost no Magicka, and always succeed. Abilities are permanent passives. Spells cost Magicka and can fail.

| Birthsign | Type | Effects |
|-----------|------|---------|
| **The Warrior** | Ability | Fortify Attack 10 pts |
| **The Mage** | Ability | Fortify Max Magicka 0.5× |
| **The Thief** | Ability | Sanctuary 10 pts |
| **The Lady** | Ability | Fortify Endurance 25 pts, Fortify Personality 25 pts |
| **The Lord** | Power + Ability | Blood of the North (power): Restore Health 2 pts for 30s. Trollkin (ability): Weakness to Fire 100% |
| **The Lover** | Power + Ability | Mooncalf (ability): Fortify Agility 25 pts. Lover's Kiss (power): Paralyze 60s on Target + Damage Fatigue 200 pts on Self |
| **The Apprentice** | Ability | Fortify Max Magicka 1.5×, Weakness to Magicka 50% |
| **The Atronach** | Ability | Fortify Max Magicka 2.0×, Spell Absorption 50 pts, Stunted Magicka (no rest regeneration) |
| **The Ritual** | Power + Spells | Mara's Gift (power): Restore Health 100 pts. Blessed Word/Touch (spells): Turn Undead 100 pts for 30s |
| **The Serpent** | Spell | Star-Curse: Poison 3 pts for 30s on Touch + Damage Health 1 pt for 30s on Self |
| **The Shadow** | Power | Moonshadow: Invisibility 60s on Self |
| **The Steed** | Ability | Fortify Speed 25 pts |
| **The Tower** | Power + Spell | Tower Key (power): Open 50 pts on Touch. Beggar's Nose (spell): Detect Animal/Enchantment/Key 200 ft for 60s |

**Magicka Multiplier Stacking:** Base 1.0× + Race bonus + Birthsign bonus. Example: Altmer (1.5×) + Atronach (2.0×) + base (1.0×) = 4.5× Intelligence as Magicka.

### 4.3 Predefined Classes (21)

Seven per specialization. Each defines 1 specialization, 2 favored attributes (+10 each), 5 Major skills, and 5 Minor skills. Full class table in [docs/morrowind/classes.md](docs/morrowind/classes.md).

**Custom Class Rules:**
- Choose 1 specialization (Combat / Magic / Stealth)
- Choose 2 favored attributes (each +10)
- Choose 5 Major skills (start at 30) and 5 Minor skills (start at 15)
- Remaining 17 are Miscellaneous (start at 5)

---

## 5. Story & Progression

### 5.1 Main Quest

**Phase 1 — Intelligence Gathering (Blades Recruit → Operative):**
1. Deliver coded package to Caius Cosades in Balmora.
2. Investigate through Fighters Guild, Mages Guild, and Vivec City informants.
3. Contact the Urshilaku Ashlanders; learn about the Nerevarine prophecy.

**Phase 2 — Sixth House Investigation:**
4. Assault the Sixth House base at Ilunibi (near Gnaar Mok).
5. Player contracts Corprus disease.

**Phase 3 — Corprus Cure:**
6. Travel to Tel Fyr. Divayth Fyr administers a cure that removes Corprus's negative effects while retaining permanent immunity to all diseases and Corprus stat bonuses.
7. Caius Cosades departs Vvardenfell; player promoted to Blades Operative.

**Phase 4 — The Seven Trials of the Nerevarine (completable in any order):**
1. Born on a certain day to uncertain parents (character creation).
2. Neither blight nor age can harm him (Corprus cure).
3. Azura's vision at the Cave of the Incarnate.
4. Become **Hortator** of all three Great Houses (Hlaalu, Redoran, Telvanni).
5. Become **Nerevarine** of all four Ashlander tribes (Urshilaku, Ahemmusa, Zainab, Erabenimsun).
6. Honor the blood of the tribe unmourned.
7. Free the cursed false gods.

**Phase 5 — Tools of Kagrenac:**
- **Wraithguard** — enchanted gauntlet, given by Vivec after becoming Hortator and Nerevarine.
- **Sunder** — enchanted warhammer, in Citadel Vemynal (Dagoth Vemyn).
- **Keening** — enchanted short blade, in Citadel Odrosal (Dagoth Odros).

**Phase 6 — Final Assault:**
- Enter Dagoth Ur's facility through the Ghostfence on Red Mountain.
- Confront Dagoth Ur (dialogue before combat).
- Enter the Akulakhan Chamber. Equip Wraithguard, strike the Heart of Lorkhan with Sunder, then strike it 5 times with Keening.
- Heart is destroyed. Dagoth Ur dies permanently. Blight ceases.

### 5.2 The Back Path (Alternate Route)

If the player kills Vivec:
- Retrieve the Unique Dwemer Artifact from Vivec's corpse.
- Find Kagrenac's Journal (Endusal) and Kagrenac's Planbook (Tureynulal).
- Bring all three to Yagrum Bagarn (last living Dwemer, in the Corprusarium).
- Wait 24 hours. He creates a broken Wraithguard that causes **200–225 permanent Health loss** when equipped.
- Proceed to acquire Sunder and Keening and assault Red Mountain normally.

### 5.3 Expansion: Tribunal

Set in **Mournhold**, capital of mainland Morrowind.

- Investigate tensions between King Hlaalu Helseth and the Living God Almalexia.
- Explore ruins beneath Mournhold; defeat a Goblin Army.
- Reconstruct **Trueflame** (Nerevar's lost sword — 1H Long Blade, fire damage, functions as a torch).
- Enter the **Clockwork City** (Sotha Sil's mechanical realm).
- Final boss: **Almalexia** (goes mad, attacks the player). Defeating her yields **Hopesfire** (twin blade, shock damage).

### 5.4 Expansion: Bloodmoon

Set on **Solstheim**, frozen island northwest of Vvardenfell.

**Bloodmoon Prophecy:** Werewolf attacks and Hircine's Hunt. Player can become a werewolf or fight against them. Final confrontation with the Daedric Prince **Hircine** and his aspects.

**East Empire Company Colony (Raven Rock):** Build an ebony mining colony from bare snow to a functioning settlement through progressive construction quests. Player becomes Factor of the colony and receives a personal estate upon completion.

---

## 6. Items & Equipment

### 6.1 Equipment Slots (18)

| Slot | Category | Notes |
|------|----------|-------|
| Helmet | Armor | |
| Cuirass | Armor | |
| Left Pauldron | Armor | |
| Right Pauldron | Armor | |
| Greaves | Armor | |
| Boots / Shoes | Armor / Clothing | Mutually exclusive |
| Left Gauntlet / Bracer / Glove | Armor / Clothing | Mutually exclusive |
| Right Gauntlet / Bracer / Glove | Armor / Clothing | Mutually exclusive |
| Shield | Armor | Left hand |
| Weapon | — | Right hand |
| Shirt | Clothing | |
| Pants | Clothing | |
| Skirt | Clothing | |
| Robe | Clothing | Covers all clothing visually |
| Belt | Clothing | |
| Amulet / Necklace | Jewelry | |
| Left Ring | Jewelry | |
| Right Ring | Jewelry | |

### 6.2 Weapon Types

| Type | Skill | Hands | Attack Types |
|------|-------|-------|-------------|
| Daggers, Shortswords, Tantos, Wakizashis | Short Blade | 1H | Chop / Slash / Thrust |
| Longswords, Broadswords, Katanas, Sabers | Long Blade | 1H | Chop / Slash / Thrust |
| Claymores, Dai-katanas | Long Blade | 2H | Chop / Slash / Thrust |
| Clubs, Maces, Morningstars | Blunt Weapon | 1H | Chop / Slash / Thrust |
| Warhammers, Staves | Blunt Weapon | 2H | Chop / Slash / Thrust |
| War Axes | Axe | 1H | Chop / Slash / Thrust |
| Battle Axes | Axe | 2H | Chop / Slash / Thrust |
| Spears, Halberds | Spear | 2H | Chop / Slash / Thrust |
| Bows, Crossbows | Marksman | 2H (ranged) | Single type |
| Throwing Stars, Throwing Knives | Marksman | Ranged | Single type |

**Material Tiers (ascending damage):** Chitin → Iron → Steel → Silver → Dwemer → Elven → Orcish → Ebony → Glass → Daedric.

Silver and above damage creatures immune to normal weapons (Daedra, ghosts, some undead).

### 6.3 Daedric Artifacts

Seven Daedric Prince quests, each yielding a unique artifact:

| Prince | Artifact | Type | Key Effect |
|--------|----------|------|-----------|
| Azura | Azura's Star | Reusable soul gem | Capacity ~15,000; not consumed on use |
| Boethiah | Goldbrand | Long Blade | Fire damage |
| Malacath | Helm of Oreyn Bearclaw | Helmet | Fortify Agility + Endurance |
| Mehrunes Dagon | Mehrunes' Razor | Short Blade | Disintegrate Armor |
| Mephala | Ring of Khajiit | Ring | Chameleon + Speed |
| Molag Bal | Mace of Molag Bal | Blunt Weapon | Absorb Magicka |
| Sheogorath | Spear of Bitter Mercy | Spear | Summon Storm Atronach |

---

## 7. Enemies & Opponents

### 7.1 Creatures by Category

Full bestiary with stats in [docs/morrowind/bestiary.md](docs/morrowind/bestiary.md).

**Beasts (Wildlife):**
Rats, Nix-Hounds, Guars, Alits, Cliff Racers, Shalks, Kagoutis, Bull Netch, Betty Netch. Cliff Racers are the most common overworld nuisance — aggressive, flying, and found in nearly every region.

**Kwama (Egg Mines):** Foragers (scouts, aggressive), Workers (docile), Warriors (colony defenders), Queens (immobile, non-combatant).

**Undead:** Skeletons, Bonewalkers (Drain Attribute curses), Greater Bonewalkers, Bonelords, Ancestor Ghosts (immune to normal weapons), Dwarven Spectres (immune to normal weapons).

**Dwemer Constructs:** Centurion Spiders, Centurion Spheres, Steam Centurions. Machines — soul value 0 (cannot be soul trapped).

**Daedra:** Scamps, Clannfear, Hunger (destroys equipment), Daedroth, Ogrim, Dremora (immune to normal weapons), Dremora Lords, Winged Twilight, Golden Saints (soul value 400 — key for Constant Effect enchanting), Flame/Frost/Storm Atronachs.

**Ash Creatures (Sixth House):** Ash Slaves, Ash Zombies, Ash Ghouls (spellcasters), Ascended Sleepers (soul value 400, extremely dangerous), Ash Vampires (unique named Dagoth bosses).

**Corpus Creatures:** Lame Corprus (slow, weak), Corprus Stalkers (fast, aggressive).

**Bloodmoon additions:** Wolves, Bears, Spriggans, Berserkers, Draugr, Rieklings, Grahl, Werewolves.

**Tribunal additions:** Fabricants (mechanical), Goblins, Liches.

### 7.2 Leveled Lists vs Fixed Encounters

Morrowind uses leveled creature lists but does NOT scale individual creatures. Each list specifies different creatures at level thresholds. A level-1 player may face rats in a dungeon while a level-15 player faces Daedra in the same dungeon — entirely different creatures, not scaled versions.

Most dungeon bosses and named NPCs are fixed (not leveled). Overworld encounters use leveled lists. Sixth House bases escalate from Corprus Stalkers (low level) to Ash Ghouls and Ascended Sleepers (high level).

---

## 8. Economy

### 8.1 Merchant Gold

Merchants carry limited gold, restocking every 24 in-game hours.

| Merchant Type | Gold |
|---------------|------|
| Typical shopkeeper | 200–800 |
| Wealthy merchants (armorsmiths, enchanters) | 1,000–3,000 |
| **Creeper** (Scamp in Ghorak Manor, Caldera) | **5,000** |
| **Mudcrab Merchant** (island east of Mzahnch, Azura's Coast) | **10,000** |

Creeper and Mudcrab Merchant are creatures with 0 Mercantile/Personality — they trade at base item value with no markup/markdown.

### 8.2 Barter Formula

Factors: player and NPC Mercantile skill, Personality (0.2× weight), Luck (0.1× weight), NPC disposition, and the Fatigue Term.

Known bug: at Mercantile >70, high merchant disposition can paradoxically reduce sell prices (fixed by Morrowind Code Patch).

### 8.3 Training

Cost scales with current skill level and is affected by the barter formula (Mercantile, disposition, Personality, Fatigue). Trainers can only train up to their own skill level.

### 8.4 Spellmaking Service Costs

Scribing fee = 7× the spell's Magicka cost (minimum cost applies). Available at Spellmaker NPCs.

### 8.5 Stolen Goods

Morrowind has NO stolen item flag system. Any merchant buys any item regardless of how it was obtained. There are no dedicated fences.

---

## 9. Factions

### 9.1 Joinable Factions

All factions use 10 ranks (0–9). Requirements escalate per rank:

| Rank | Primary Skill | Other Skills (×2) | Favored Attributes |
|------|--------------|-------------------|--------------------|
| 0 (join) | — | — | 30 / 30 |
| 1 | 10 | — | 30 / 30 |
| 2 | 20 | — | 30 / 30 |
| 3 | 30 | 5 | 30 / 30 |
| 4 | 40 | 10 | 30 / 30 |
| 5 | 50 | 15 | 31 / 31 |
| 6 | 60 | 20 | 32 / 32 |
| 7 | 70 | 25 | 33 / 33 |
| 8 | 80 | 30 | 34 / 34 |
| 9 | 90 | 35 | 35 / 35 |

**Guilds:**

| Faction | Favored Attrs | Favored Skills | Rank Names |
|---------|--------------|----------------|-----------|
| **Fighters Guild** | STR, END | Axe, Long Blade, Blunt Weapon, Heavy Armor, Armorer, Block | Associate → Champion → Master |
| **Mages Guild** | INT, WIL | Alchemy, Mysticism, Illusion, Alteration, Destruction, Enchant | Associate → Master-Wizard → Arch-Mage |
| **Thieves Guild** | SPD, AGI | Acrobatics, Light Armor, Marksman, Security, Short Blade, Sneak | Toad → Mastermind → Master Thief |
| **Imperial Legion** | END, PER | Athletics, Block, Blunt Weapon, Heavy Armor, Long Blade, Spear | Recruit → Knight of the Garland → Knight of the Imperial Dragon |
| **Imperial Cult** | PER, WIL | Blunt Weapon, Conjuration, Mysticism, Restoration, Enchant, Speechcraft, Unarmored | Layman → Invoker → Primate |
| **Tribunal Temple** | PER, WIL | Alchemy, Blunt Weapon, Conjuration, Mysticism, Restoration, Unarmored | Layman → Diviner → Patriarch |
| **Morag Tong** | SPD, INT | Acrobatics, Illusion, Light Armor, Marksman, Short Blade, Sneak | Associate → Exalted Master → Grandmaster |

**Great Houses (mutually exclusive — may only join one):**

| House | Favored Attrs | Favored Skills |
|-------|--------------|----------------|
| **Hlaalu** | SPD, AGI | Light Armor, Marksman, Mercantile, Security, Short Blade, Speechcraft |
| **Redoran** | STR, END | Athletics, Heavy Armor, Long Blade, Medium Armor, Spear, Armorer |
| **Telvanni** | INT, WIL | Alteration, Conjuration, Destruction, Enchant, Illusion, Mysticism |

All three use the same rank names: Hireling → Retainer → Oathman → Lawman → Kinsman → House Cousin → House Brother → House Father → Councilman → Grandmaster.

**Blades:** Non-standard; ranks granted through main quest progression by Caius Cosades. Player reaches Operative (~rank 5) before Caius departs.

**East Empire Company (Bloodmoon):** Joined through the Raven Rock colony questline.

### 9.2 Disposition Formula

```
Disposition = NPC Base Disposition
            + Same Race Bonus (+5)
            + Personality Modifier ((PER − 50) / 2)
            + Faction Modifier (Reaction × 5 × (0.5 + Faction Rank × 0.5))
            − Bounty Penalty
            − Disease Penalty (−5 if diseased)
            − Weapon Drawn Penalty (−5 if weapon out)
```

Faction Reaction ranges from −3 to +3 (inter-faction relationships).

---

## 10. UI & HUD

### 10.1 Gameplay HUD

**Bottom-left corner:**
- Three horizontal bars (top to bottom): Health (red), Magicka (blue), Fatigue (green).
- Two small square icons beneath: equipped weapon (left), readied spell (right).
- Weapon condition bar beneath the weapon icon (durability).
- Enchantment charge bar beneath enchanted item icons.
- Active magic effect icons appear above/near the stat bars.
- Sneak indicator: appears near stat bars when successfully sneaking undetected.

**Bottom-right corner:**
- Minimap: small square automap with fog of war, revealing as the player explores.

**Center:** Crosshair dot.

**Contextual elements:**
- Breath meter: Health bar visually drains when underwater (~20s of air, varies with Endurance; 3 damage/sec after depletion).

**HUD Pinning:** Any of the four menu windows (Stats, Inventory, Magic, Map) can be pinned to remain visible during gameplay via a small icon in each window's top-right corner. Resizable and repositionable.

### 10.2 Menu System

Right Mouse Button toggles Menu Mode, displaying four simultaneous resizable/repositionable windows:

**Stats Window:** Character name/race/class/level, all 8 attributes (current/base), all 27 skills with progress bars, faction memberships and ranks, reputation, bounty, birthsign.

**Inventory Window:** Left side: 3D paper doll (drag items to equip). Right side: scrollable item list with five filter tabs (Weapons, Apparel, Magic Items, Alchemy Ingredients, Miscellaneous). Encumbrance bar at bottom.

**Magic Window:** Organized sections — Spells (cost Magicka), Powers (once/day, free), Abilities (passive), Magic Items (Cast When Used enchantments). Clicking selects the active spell.

**Map Window:** Two modes — Local Map (overhead automap with fog of war, supports custom text annotations) and World Map (full Vvardenfell, cities marked with icons, scrollable).

### 10.3 Journal

- Base game: purely chronological entries.
- With Tribunal/Bloodmoon (GOTY): quest-based sorting, active/completed quest filtering, alphabetical topic search.
- No quest markers or compass waypoints — all navigation relies on reading journal directions and NPC dialogue.

### 10.4 Dialogue Window

- NPC portrait (rotating 3D head model), NPC name, disposition value (0–100).
- Left panel: clickable topic list.
- Right panel: NPC response text.
- Bottom: Persuasion buttons (Admire, Intimidate, Taunt, Bribe) and service buttons (Barter, Spells, Training, Travel, Spellmaking, Enchanting, Repair).

**Keyword/Hyperlink System:** Blue-highlighted words in NPC response text are clickable, navigating to that topic. New topics unlock when an NPC mentions them. Topics only appear if the NPC has a valid response (filtered by race, class, faction, cell, gender, disposition threshold, quest state).

**Generic vs Unique Dialogue:** Most dialogue is generic — shared across NPCs, filtered by conditions. Unique dialogue is limited to quest-specific NPCs and key characters. All NPCs draw from a single dialogue database.

---

## 11. Engine & Presentation Systems

### 11.1 Dialogue System

See §10.4. Keyword-based hyperlink system. No voice acting for dialogue (only greetings, combat barks, and some ambient lines are voiced).

### 11.2 Save System

- **Manual Save:** Unlimited slots from the pause menu. No restrictions during combat.
- **Quicksave:** F5. Single quicksave slot (F9 to load).
- **Autosave:** Toggleable; triggers on rest only. Single autosave slot.
- No autosave on travel. Saving available at any time outside dialogue/menus.

### 11.3 Camera

- **First-person:** Default. Player sees hands/weapon.
- **Third-person:** Tab toggle. Behind-and-above camera.
- **Vanity camera:** Hold Tab in third-person + move mouse to orbit. Mouse wheel zooms.
- No camera change during dialogue (text window overlay, not cinematic).

### 11.4 Difficulty

Continuous slider from −100 (easiest) to +100 (hardest), default 0.

| Setting | Player Damage Dealt | Player Damage Received |
|---------|--------------------|-----------------------|
| −100 | ~6× normal | ~0.17× normal |
| 0 | 1× | 1× |
| +100 | ~0.17× normal | ~6× normal |

Affects physical weapon damage only. Does NOT affect spell damage, AI, enemy stats, or loot. Can be changed at any time.

### 11.5 Rest / Wait

**Rest (Sleep):**
- Key: T. Select hours (1–24) or "Rest Until Healed."
- Restores Health, Magicka, and Fatigue to full. Recharges enchanted items.
- Triggers level-up if 10 Major/Minor skill increases accumulated.
- Atronach birthsign: does NOT regenerate Magicka from rest.
- Wilderness/dungeons: allowed if no enemies nearby; risk of creature ambush.
- Towns: only in a bed the player owns, rents (inn), or has permission to use (guild halls for members).
- "You cannot rest here, enemies are nearby" if hostiles detected.

**Wait:**
- Available only in towns without using a bed.
- Passes time, recovers Fatigue, recharges enchanted items.
- Does NOT restore Health or Magicka. Does NOT trigger level-up.

### 11.6 Crime System

**Bounty Amounts:**

| Crime | Bounty |
|-------|--------|
| Theft | Item's gold value |
| Failed Pickpocket | Reported by victim only |
| Lockpicking (attempt) | 5 |
| Trespassing (civilian) | 50 |
| Trespassing (authority) | 100 |
| Assault (peaceful NPC) | 40 |
| Assault (guard) | 60 |
| Murder | 1,000 |

Crimes must be witnessed to generate bounty. Self-defense is not a crime.

**Guard Response (3 options):**
1. **Pay Fine** — full bounty in gold; stolen items confiscated.
2. **Go to Jail** — 1 day per 100 gold (minimum 1 day). Each day served causes 1 random skill to lose a point. Bounty cleared.
3. **Resist Arrest** — all guards in area turn hostile.

**Death Warrant:** At 5,000+ bounty, guards attack on sight with no dialogue. Only clearable through the Thieves Guild.

**Thieves Guild Service:** Guild savants remove bounty at half price (50% of bounty). Available in Balmora, Ald'ruhn, Sadrith Mora, Vivec. Only way to clear a Death Warrant without fighting.

**Morag Tong Writs:** Members who kill an authorized target can present an Honorable Writ of Execution to any guard to clear the bounty.

### 11.7 Werewolf System (Bloodmoon)

Contract Sanies Lupinus from a werewolf attack; transforms after 3 days.

**Transformation:** Automatic at nightfall (9 hours), or via Hircine's Ring (6 hours).

**Werewolf Stats (fixed, override normal):** STR 100, INT 100, WIL 100, AGI 100, SPD 100, END 100, Health 500, Magicka 200, Fatigue 400.

**Abilities:** Night Eye 25 pts, Detect Animal 4000 ft, 100% Disease Resistance, Restore Health 1 pt, enhanced Sneak (95), enhanced Acrobatics (80).

**Restrictions:** Cannot equip items/armor, use inventory, pick locks, or loot corpses while transformed. Feeding on corpses restores health.

**Social Penalty:** If witnessed transforming, ALL NPCs in the game permanently attack on sight (even in human form).

Reverts at dawn; all equipment unequipped.

---

## 12. Open Questions / Unverified

- **Exact pickpocket formula:** Sources differ on coefficient values and the double-roll bug specifics. OpenMW source code is the authoritative reference.
- **Self-enchant success formula:** Two slightly different versions in community sources (INT/5 vs INT/4, 3× vs 2.5× enchantment point penalty). OpenMW-verified version uses /5 and /10 with 3× multiplier.
- **Block chance modifiers:** Some sources cite a movement-direction multiplier (1.25× if not moving forward) and a charge-based modifier; others omit these. Needs OpenMW source verification.
- **Barter formula precise coefficients:** The Mercantile >70 inversion bug is documented but the exact formula coefficients vary between sources.
- **Difficulty slider damage formula:** The exact exponential function mapping the slider range to damage multipliers is not universally agreed upon. The values at extremes (~6× and ~0.17×) are approximate.
- **Breath meter duration:** Reported as ~20 seconds but the exact relationship to Endurance (if any) needs verification.
- **Some race attribute values** were cross-referenced from multiple secondary sources rather than directly from UESP (which was unavailable for direct fetch). Minor discrepancies possible for Imperial, Khajiit, Nord, Orc, and Redguard gender splits.

---

## 13. References

### Wikis
- UESP Wiki — en.uesp.net/wiki/Morrowind (primary authoritative source for all mechanics)
- Elder Scrolls Fandom Wiki — elderscrolls.fandom.com/wiki/The_Elder_Scrolls_III:_Morrowind
- OpenMW Wiki — wiki.openmw.org (reverse-engineered formulas, Research namespace)

### Guides
- Steam Community: "The Arch-Mage's Handbook" (id:2834812759)
- Steam Community: "Easy Guide to Combat" (id:472931211)
- GameFAQs Morrowind guides and forum mechanics threads
- "Playing a Pure Mage in Morrowind" (GitHub Gist by herohiralal)

### Technical
- OpenMW source code and documentation (openmw.readthedocs.io) — verified formulas
- TES3 Construction Set documentation (tes3cs.pages.dev)
- Tamriel Rebuilt dialogue/quest documentation

### Companion Docs
- [docs/morrowind/classes.md](docs/morrowind/classes.md) — Full predefined class table (21 classes)
- [docs/morrowind/bestiary.md](docs/morrowind/bestiary.md) — Complete creature stat tables
