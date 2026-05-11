# Mass Effect 2 — Gameplay Systems Spec

Mass Effect 2, Xbox 360 / PS3 / PC, 2010. BioWare. Original release (Legendary Edition differences noted where relevant).

---

## 1. Core Gameplay Systems

### Primary Gameplay Loop

Recruit squadmates, earn their loyalty, upgrade the Normandy, and survive the Suicide Mission. Between story missions: scan planets for mineral resources, research upgrades at the tech lab, and build relationships with crew. The game is structured into three acts gated by recruitment count and mission triggers.

### Combat System

Cover-based third-person shooter with squad-based tactics and pausable power usage.

- **Cover**: Snap to walls/objects, vault over low cover, blind-fire, and storm (sprint) between cover positions. Cover provides roughly 50–75% damage reduction (exact value not publicly datamined).
- **Thermal Clips**: ME2 replaced ME1's weapon-overheat system with disposable thermal clips (functionally ammunition). Each weapon type draws from a shared ammo pool per category. Clips are picked up from fallen enemies and ammo crates scattered through levels. Running dry forces a weapon switch — there is no overheat fallback.
- **Power Wheel**: Holding the radial menu button pauses combat. Select any power for Shepard or either squadmate. Powers can also be hotkeyed (PC) or mapped to d-pad (console).
- **Global Cooldown**: Using any power triggers a shared cooldown on ALL powers for that character. Base cooldowns vary by power (3–6s for Shepard). Reduction comes from class passives and research upgrades.
- **Squadmate Cooldowns**: Generally 2× Shepard's (e.g., 6s becomes 12s).
- **Deferred-cooldown powers**: Tactical Cloak, Barrier, Fortification, Geth Shield Boost — cooldown starts after the power expires, not on activation.

### Medi-Gel & Death

- **Medi-Gel**: Consumable healing resource. Base capacity: 3 uses. Increased by Microscanner research (+1 per tier).
- **Unity**: Shepard's innate squad power. Consumes 1 medi-gel to revive all downed squadmates and heal the squad. Medical VI research upgrade makes Unity heal squad to full; Shield Harmonics upgrade also restores squad shields.
- **Squadmate down**: When a squadmate's HP reaches zero, they collapse and are out of combat until revived by Unity or until combat ends (auto-revive at partial health).
- **Shepard down**: When Shepard's HP reaches zero, the mission fails and reloads from the last checkpoint. There is no bleedout or self-revive.

### Protection System

Four protection types displayed as colored health bars above enemies:

| Type | Color | Regenerates? | Weak To | Blocks CC? |
|------|-------|-------------|---------|-----------|
| Health | Red | No (except vorcha regen) | Shredder Ammo, general weapons | No |
| Shields | Blue | Yes (~3–5s delay) | Overload, Disruptor Ammo, Energy Drain, SMGs | Yes |
| Barriers | Purple | Yes (~3–5s delay) | Warp, Reave, Warp Ammo, Concussive Shot (3.5×) | Yes |
| Armor | Yellow | No | Incinerate, Incendiary Ammo, Warp, snipers/pistols | Yes |

Enemies with any active protection layer are **immune to crowd-control** (Pull, Throw, Singularity lift, Cryo freeze). Protection must be fully stripped before CC applies. On Hardcore/Insanity, every enemy has at least one protection layer — this is the single most important combat mechanic at high difficulty.

### Base Character Stats

| Character | HP | Protection |
|-----------|----|-----------|
| Default Shepard | 200 | 250 Shields |
| Soldier Shepard | 250 | 250 Shields |
| Default squadmates | 200 | 250 Shields |
| Grunt | 300 | 300 Armor (no shields) |
| Legion | 200 | 350 Shields |

### Damage Formula

Core weapon damage calculation (from peddroelm, community-verified):

```
Total Damage = AmmoPowerDamage × ScalingFactor + WeaponDamage × ScalingFactor

AmmoPowerDamage = BaseWeaponDmg × Ammo% × (1 + ΣPowerDmgBonuses) × VsDefenseMod
WeaponDamage    = BaseWeaponDmg × (1 + ΣWeaponDmgBonuses) × VsDefenseMod
```

Key damage bonuses (additive within each category):

| Source | Bonus |
|--------|-------|
| Class passive (max) | +15% weapon or power damage |
| Adrenaline Rush | +100% (Heightened: +140%) |
| Tactical Cloak | +20% / +40% / +75% (Assassination) |
| Headshot (humanoid) | +75% |
| Headshot (YMIR) | +50% |
| Sniper headshot research | +50% (multiplicative with above) |
| Vulnerable target (frozen, lifted, neural-shocked) | +100% |
| Squad attack order | +50% |

**Difficulty scaling**: Enemy HP does not change across difficulties. Instead, a ScalingFactor reduces player/squad damage output based on difficulty + Shepard's level, maintaining expected TTK as the player upgrades. Exact ScalingFactor values are not publicly datamined (see §12).

### Melee Damage

- Base: 125 damage
- Heavy Muscle Weave research: +25%
- Melee deals equal damage to all protection types (no defense multipliers)
- Formula: BaseMelee × WeaponMeleeBonuses × (1 + ΣPowerDmgBonuses)

### Range Mechanics

Each weapon has short/medium/long range thresholds:
- ARs, Heavy Pistols, SMGs: 8m / 15m / 20m
- Shotguns: 4m / 8m / 12m
- Closer than optimal: additive bonus to weapon damage (not ammo damage)
- Beyond optimal: additive penalty to both weapon and ammo damage

---

## 2. Controls & Input

### Default Control Scheme (Xbox 360 / PC)

PS3 controls map directly to Xbox equivalents: × = A, ○ = B, L1 = LB, R1 = RB, L2 = LT, R2 = RT.

| Action | Xbox 360 | PC |
|--------|----------|-----|
| Move | Left Stick | WASD |
| Camera | Right Stick | Mouse |
| Take Cover / Storm | A | Space |
| Shoot | RT | Left Click |
| Aim / Zoom | LT | Right Click |
| Melee | B | F |
| Power Wheel (pause) | LB | Shift (hold) |
| Squad Power 1 | LB + A/X | Mapped hotkeys |
| Squad Power 2 | LB + B/Y | Mapped hotkeys |
| Move Squad | D-pad | — |
| Rally Squad | D-pad Up | — |
| Weapon Select | D-pad Left/Right | Mouse wheel / 1-4 |
| Interact | A | E |
| Paragon Interrupt | LT | Q / Left Click |
| Renegade Interrupt | RT | E / Right Click |
| Quicksave | — | F5 |

### Context-Sensitive Inputs

- **In cover**: A/Space vaults or detaches; aim peeks from cover edge
- **Near climbable**: A/Space mantles
- **Storm**: Sprint between cover points while A/Space held toward destination cover
- **Power Wheel open**: Click/select to target enemy or terrain for squad commands
- **Conversation**: Dialogue wheel uses left stick / mouse to select; interrupts use trigger/button prompts
- **Planet scanning**: Left stick / mouse moves scanner; RT / left click fires probe
- **Galaxy map**: Click to select clusters/systems/planets; navigate hierarchically

### Squad AI Configuration

Squadmate power usage can be set to:
- **Automatic**: AI decides when to use powers
- **Manual only**: Powers fire only on player command (recommended for Hardcore/Insanity)

---

## 3. World Structure

### Galaxy Map Navigation

Hierarchical layers: **Galaxy → Cluster → System → Planet**

- Travel between **systems within a cluster** costs fuel (1 credit/unit at Fuel Depots)
- Travel between **clusters** uses Mass Relays (free, instant)
- Base fuel capacity: 1,000 units. Extended Fuel Cells upgrade: 1,500
- If fuel runs out: resources consumed to return to gateway system; if no resources, EDI uses emergency FTL (no penalty)

### Hub Worlds

**Normandy SR-2** (persistent hub between missions):
- Deck 1: Captain's Cabin (fish tank, model ships, personal terminal)
- Deck 2 (CIC): Galaxy Map, Yeoman Kelly Chambers, Armory, Tech Lab (research), Briefing Room
- Deck 3 (Crew): Med Bay, Main Battery (Garrus), Observation Decks (Samara/Kasumi, Thane), Life Support (Thane), AI Core (Legion)
- Deck 4 (Engineering): Drive Core, Engineers Gabby & Ken, Jack's hideout (sub-deck), Zaeed's area
- Deck 5: Shuttle Bay

**Citadel** (Zakera Ward): C-Sec, shops (Rodam Expeditions, Sirta Foundation, Saronis Applications, Citadel Souvenirs), Dark Star Lounge

**Omega**: Afterlife Club (Aria T'Loak), Kenn's Salvage, Omega Market, Harrot's Emporium

**Illium** (Nos Astra): Trading Floor (Serrice Technologies, Baria Frontiers, Gateway Personal Defense), Eternity Bar, Liara's Office

**Tuchanka**: Urdnot Camp, Ratch's Wares, Chief Scout, Shaman

### Planet Scanning / Probing

Replaced ME1's Mako exploration. Orbiting a non-mission planet shows a scanner overlay:
- Move scanner reticle across the surface; graph column spikes when minerals detected; controller vibrates proportionally
- Fire probes at spike locations to extract resources; yield scales logarithmically with graph height
- Four resources: **Element Zero** (rarest), **Iridium**, **Palladium**, **Platinum**
- Planet depletion states: Rich → Good → Moderate → Poor → Depleted
- Base probe capacity: 30; Modular Probe Bay upgrade doubles to 60
- Scanning can also discover **anomalies** that unlock N7 side missions

---

## 4. Playable Characters / Classes

### Shepard's 6 Classes

| Class | Unique Power | Other Powers | Weapon Access |
|-------|-------------|-------------|---------------|
| Soldier | Adrenaline Rush | Concussive Shot, Disruptor Ammo, Incendiary Ammo, Cryo Ammo | AR, Sniper, Shotgun, Pistol, Heavy |
| Adept | Singularity | Warp, Throw, Pull, Shockwave | Pistol, SMG, Heavy |
| Engineer | Combat Drone | Overload, Incinerate, Cryo Blast, AI Hacking | Pistol, SMG, Heavy |
| Infiltrator | Tactical Cloak | Disruptor Ammo, Cryo Ammo, Incinerate, AI Hacking | Sniper, Pistol, SMG, Heavy |
| Sentinel | Tech Armor | Throw, Warp, Overload, Cryo Blast | Pistol, SMG, Heavy |
| Vanguard | Biotic Charge | Pull, Shockwave, Incendiary Ammo, Cryo Ammo | Shotgun, Pistol, SMG, Heavy |

Each class also has a passive power (see Class Passive Specializations below). All classes can equip one **Bonus Power** via Advanced Training at the Normandy tech lab (costs 5,000 Element Zero to acquire or change). The bonus power pool consists of loyalty powers unlocked from completed squadmate loyalty missions.

**Advanced Weapon Training**: During the Collector Ship mission (~mid-game), any class gains ONE additional weapon type (Assault Rifles, Shotguns, or Sniper Rifles).

### Class Passive Specializations (Rank 4 Branch)

Each class passive offers a damage-focused or survivability-focused evolution at rank 4:

| Class | Passive | Branch A (Damage) | Branch B (Survivability) |
|-------|---------|-------------------|--------------------------|
| Soldier | Combat Mastery | Commando: +15% wpn/pwr dmg, +50% storm | Shock Trooper: +40% HP, +100% Paragon/Renegade |
| Adept | Biotic Mastery | Nemesis: +15% pwr dmg, CDR | Bastion: +15% pwr duration, CDR |
| Engineer | Tech Mastery | Demolisher: +15% pwr dmg, CDR | Mechanic: +15% pwr duration, CDR |
| Infiltrator | Operative | Assassin: +15% wpn/pwr dmg | Agent: +15% pwr duration, CDR |
| Sentinel | Defender | Raider: +15% pwr dmg, CDR | Guardian: +15% pwr duration, CDR |
| Vanguard | Assault Mastery | Destroyer: +15% pwr/wpn dmg | Champion: +15% pwr duration, CDR |

### Powers Overview

Powers are divided into **Biotic**, **Tech**, **Combat**, and **Ammo** categories. Each has 4 ranks costing 1/2/3/4 squad points. Rank 4 offers a permanent choice between two evolutions.

See [docs/powers.md](docs/powers.md) for the complete power list with all evolution branches, cooldowns, and damage values.

**Key power interactions**:
- **Biotic Combos**: Warp or Throw detonates targets affected by Pull or Singularity, dealing bonus AoE damage
- **Ammo vs Protection**: Each ammo power is effective against specific protection types and useless against others (see §1 Protection System)
- **CC Immunity**: All crowd-control blocked by active protection layers

### Squadmates

12 recruitable squadmates (10 base + 2 DLC):

| # | Squadmate | Weapons | Base Powers | Loyalty Power | Recruitment |
|---|-----------|---------|------------|---------------|-------------|
| 1 | Jacob Taylor | Pistol, Shotgun | Pull, Incendiary Ammo | Barrier | Prologue |
| 2 | Miranda Lawson | Pistol, SMG | Overload, Warp | Slam | Prologue |
| 3 | Mordin Solus | Pistol, SMG | Incinerate, Cryo Blast | Neural Shock | Omega |
| 4 | Garrus Vakarian | AR, Sniper | Overload, Concussive Shot | Armor-Piercing Ammo | Omega |
| 5 | Jack | Pistol, Shotgun | Shockwave, Pull | Warp Ammo | Purgatory |
| 6 | Grunt | AR, Shotgun | Concussive Shot, Incendiary Ammo | Fortification | Korlus |
| 7 | Tali'Zorah | Pistol, Shotgun | Combat Drone, AI Hacking | Energy Drain | Haestrom |
| 8 | Thane Krios | Pistol, SMG, Sniper | Throw, Warp | Shredder Ammo | Illium |
| 9 | Samara | AR, SMG | Throw, Pull | Reave | Illium |
| 10 | Legion | AR, Sniper | Combat Drone, AI Hacking | Geth Shield Boost | Derelict Reaper |
| 11 | Zaeed Massani (DLC) | AR, Sniper | Concussive Shot, Disruptor Ammo | Inferno Grenade | Omega |
| 12 | Kasumi Goto (DLC) | Pistol, SMG | Shadow Strike, Overload | Flashbang Grenade | Citadel |

Each squadmate also has a class passive power. Completing a loyalty mission unlocks the 4th power, an alternate outfit, and improved Suicide Mission survival odds.

**Special case**: Samara's loyalty mission allows recruiting **Morinth** instead, who replaces Samara and grants **Dominate** instead of Reave as a bonus power.

### Squad Mechanics

- Squadmate weapon damage: 40–65% of Shepard's base (varies by weapon type)
- Squadmates have near-perfect accuracy but cannot headshot
- **Move/Cover order**: Send a squadmate to a position or cover point
- **Attack order**: Focus fire on a target (+50% squad damage bonus)
- **Rally**: Call squadmates back to Shepard
- Without orders, squadmates behave aggressively and advance toward enemies — dangerous on higher difficulties

---

## 5. Story & Progression

### Mission Structure

**Prologue**: Save Joker → Awakening (Lazarus Station) → Freedom's Progress

**Act 1** — Recruitment (before Horizon):

| Mission | Squadmate | Location |
|---------|-----------|----------|
| Dossier: The Professor | Mordin Solus | Omega |
| Dossier: Archangel | Garrus Vakarian | Omega |
| Dossier: The Convict | Jack | Purgatory |
| Dossier: The Warlord | Grunt | Korlus |
| Dossier: The Veteran (DLC) | Zaeed Massani | Omega |
| Dossier: The Master Thief (DLC) | Kasumi Goto | Citadel |

**Horizon** triggers automatically after recruiting any 4 of the above.

**Act 2** — Remaining recruitment + loyalty missions:

| Mission | Squadmate | Location |
|---------|-----------|----------|
| Dossier: Tali | Tali'Zorah | Haestrom |
| Dossier: The Assassin | Thane Krios | Illium |
| Dossier: The Justicar | Samara | Illium |

Story-critical missions in Act 2:
- **Collector Ship** — triggers automatically after ~5 missions post-Horizon
- **Acquire Reaper IFF** — Derelict Reaper (recruits Legion)

### Loyalty Missions

| Mission | Squadmate |
|---------|-----------|
| The Gift of Greatness | Jacob |
| The Prodigal | Miranda |
| Subject Zero | Jack |
| Old Blood | Mordin |
| Rite of Passage | Grunt |
| Eye for an Eye | Garrus |
| The Ardat-Yakshi | Samara |
| Treason | Tali |
| Sins of the Father | Thane |
| A House Divided | Legion |
| The Price of Revenge | Zaeed (DLC) |
| Stealing Memory | Kasumi (DLC) |

Loyalty missions unlock after Horizon. Each grants: 4th power, alternate outfit, and survival advantage in the Suicide Mission.

### Loyalty Conflicts

Two pairs of squadmates have mutually exclusive loyalty conflicts triggered by story events. After both loyalty missions in a pair are complete, a confrontation occurs on the Normandy. Shepard must pass a Paragon or Renegade check to side-step the conflict; otherwise, one squadmate loses loyalty.

| Conflict | Trigger | Resolution |
|----------|---------|------------|
| Jack vs Miranda | After both loyalty missions complete | Paragon/Renegade check; failing forces Shepard to side with one, losing the other's loyalty |
| Tali vs Legion | After both loyalty missions complete | Same as above |

Lost loyalty can be regained later with a sufficiently high Paragon/Renegade score by speaking to the disloyal squadmate. But if morality is too low, the loyalty loss is permanent for that playthrough — directly affecting Suicide Mission survival.

### Suicide Mission (Act 3)

Triggered by entering the Omega 4 Relay. A complex branching system determines who lives and dies.

**Ship Upgrades** — three critical Normandy upgrades. For each one missing, a specific squadmate dies during the approach:

| Missing Upgrade | Cost | Suggested By | Who Dies |
|-----------------|------|-------------|----------|
| Silaris Heavy Ship Armor | 15,000 Palladium | Jacob | Jack |
| Cyclonic Barrier Technology | 15,000 Palladium | Tali | Kasumi > Legion > Tali > Thane > Garrus > Zaeed > Grunt |
| Thanix Cannon | 15,000 Platinum | Garrus | Thane > Garrus > Zaeed > Grunt > Jack > Samara/Morinth |

**Crew Survival Timing** — after the crew is abducted (triggered by galaxy map access post-IFF):
- 0 extra missions: entire crew survives
- 1–3 extra missions: half the crew dies (Kelly Chambers, etc.)
- 4+ extra missions: all crew die except Dr. Chakwas

**Role Assignments**:

| Role | Ideal Choices (must be loyal) | Wrong Choice Consequence |
|------|------------------------------|--------------------------|
| Vent Specialist | Tali, Legion (Kasumi can survive but is less commonly verified) | Specialist dies |
| 1st Fire Team Leader | Garrus, Miranda, Jacob | Gets vent specialist killed |
| Biotic Specialist | Jack, Samara/Morinth | One squadmate carried off by swarms |
| 2nd Fire Team Leader | Garrus, Miranda, Jacob | Squadmate deaths |
| Crew Escort | Any loyal member (Mordin ideal) | Crew dies en route if no escort |

**Hold the Line** — Defense Score system for remaining squadmates:

| Tier | Characters | Loyal / Disloyal Score |
|------|-----------|----------------------|
| Tanks | Garrus, Grunt, Zaeed | 4 / 3 |
| Soldiers | Thane, Legion, Samara/Morinth, Jacob, Miranda | 2 / 1 |
| Weak | Mordin, Tali, Kasumi, Jack | 1 / 0 |

Average score ≥ 2.0: everyone survives. 1.0–1.99: 1 dies. < 1.0: 2 die.
Death priority (weakest first): Mordin → Tali → Kasumi → Jack → Miranda → Jacob → Garrus → Samara/Morinth → Legion → Thane → Zaeed → Grunt.

**Shepard Dies** if fewer than 2 squadmates survive the entire mission.

**Collector Base Decision**: Destroy (Paragon) or Preserve for Cerberus (Renegade). Affects ME3 war assets and Illusive Man dialogue.

### ME1 Import System

| ME1 Level | ME2 Starting XP | Credits | Resources (each) |
|-----------|----------------|---------|-------------------|
| 1–49 | 1,000 (level 2) | 20,000 | 2,500 |
| 50–59 | 2,000 (level 3) | 30,000 | 5,000 |
| 60 | 4,000 (level 5) | 50,000 | 10,000 |

Rich Achievement (1M credits in ME1): +100,000 additional credits.
Morality: up to 190 Paragon + 190 Renegade points carried over (requires ME1 bars ≥ 50% full).

**Major decisions that carry over**:

| ME1 Decision | ME2 Consequence |
|-------------|----------------|
| Virmire Survivor (Ashley/Kaidan) | Survivor appears on Horizon |
| Wrex alive/dead | Leads Clan Urdnot on Tuchanka vs. brother Wreav leads |
| Council saved/destroyed | Original Council reinstated vs. replacement Council |
| Human Councilor (Udina/Anderson) | Appears in Citadel interactions |
| Rachni Queen freed/killed | Freed: asari messenger on Illium. Killed: news reports only |
| ME1 Romance | Referenced in ME2; can stay loyal or pursue new romance |
| Feros colonists saved | Representative (Shiala if alive) appears on Illium with Medical Scans assignment |
| Conrad Verner (charmed/intimidated) | Appears on Illium; bug causes him to always act intimidated (fixed in LE) |
| Gianna Parasini (helped/ignored) | Helped: appears on Illium with new assignment. Ignored: dies offscreen |
| Captain Kirrahe (survived/died) | Referenced in Mordin's dialogue |
| Rana Thanoptis (spared/killed) | Spared: appears during Grunt's recruitment on Korlus |
| Balak — Bring Down the Sky (spared/killed) | Spared: referenced in news reports |
| Garrus: Dr. Saleon (stand down/kill) | Affects Garrus's morality outlook and dialogue |

**Default choices** (no import): Wrex dead, Ashley survived, Council killed, no romance.

### Morality System

Paragon and Renegade are **dual independent scales** — earning one does not reduce the other.

- Standard dialogue: +2 points. Significant choices: +5. Major decisions: +15
- Approximately 970 points to fill each bar (community datamining; not official)
- Cannot max both in a single playthrough

**Charm/Intimidate checks** use a percentage-based system: success depends on the proportion of available Paragon/Renegade points earned up to that point, not an absolute threshold. Players must commit predominantly to one alignment to pass high-tier checks.

**Interrupts**: 24 Paragon (blue, left trigger) + 24 Renegade (red, right trigger) = 48 total. Timed button prompts during cutscenes; optional with no penalty for ignoring.

**Appearance effect**: Paragon heals Shepard's facial scars. Renegade deepens them (glowing red eyes, visible scarring). Med-Bay upgrade (50,000 Platinum) resets scarring.

### Romance System

ME2 has 9 romance options across both genders. Romances develop through conversation on the Normandy between missions and culminate in a cabin scene before the Suicide Mission.

| Romance Option | Available To | Condition |
|---------------|-------------|-----------|
| Miranda Lawson | Male Shepard | Complete loyalty mission |
| Jack | Male Shepard | Complete loyalty mission; specific dialogue path |
| Tali'Zorah | Male Shepard | Complete loyalty mission |
| Thane Krios | Female Shepard | Complete loyalty mission |
| Garrus Vakarian | Female Shepard | Complete loyalty mission |
| Jacob Taylor | Female Shepard | Dialogue progression |
| Kelly Chambers | Male or Female | Dinner invitation; does not conflict with other romances |
| Samara | Male or Female | Partial — she declines at the last moment |
| Morinth | Male or Female | Only if recruited instead of Samara; results in Shepard's death (game over) |

Pursuing a new romance while having an ME1 romance import counts as "cheating" and affects ME3 dialogue. The Lair of the Shadow Broker DLC allows rekindling the Liara romance from ME1.

### New Game+

Carries over: level, all weapons and armor, +200,000 credits, +50,000 each resource, +500 fuel, +30 probes, +25% bonus XP, unlocked bonus powers.
Does NOT carry over: Paragon/Renegade scores, squad loyalty, ship upgrades, research, assignments.
Class and weapon specialization are locked.

### Leveling

- Level cap: 30
- XP per level: flat 1,000
- Squad points: 51 total for Shepard; 30 for most squadmates (31 for Miranda/Jacob)
- Each power costs 10 points to max (1+2+3+4)

---

## 6. Items & Equipment

### Weapon Categories

| Category | Class Access | Protection Bonus |
|----------|-------------|-----------------|
| Heavy Pistols | All classes | 1.5× vs Armor |
| Submachine Guns | All except Soldier | 1.5× vs Shields and Barriers |
| Assault Rifles | Soldier (others via training) | 1.25× vs all protections |
| Shotguns | Soldier, Vanguard (others via training) | 1.5× vs Shields and Barriers |
| Sniper Rifles | Soldier, Infiltrator (others via training) | 1.5× vs Armor |
| Heavy Weapons | All classes | Varies by weapon |

See [docs/weapons.md](docs/weapons.md) for the complete weapon list with per-weapon stats, damage values, and special properties.

**Known bug**: Shotgun penetration research upgrade does NOT apply against Barriers.

### Armor System

Shepard's armor uses a mix-and-match system with 5 slots: **Helmet, Chest, Shoulders, Arms, Legs**. Each piece provides specific stat bonuses.

Alternatively, **full armor sets** replace all pieces with a unified look and combined bonuses (cannot mix pieces, except the Kestrel set).

See [docs/armor.md](docs/armor.md) for all armor pieces, full sets, stat bonuses, and acquisition sources.

### Research & Upgrades

Upgrades are researched at the Tech Lab (available after recruiting Mordin) using mineral resources.

**Weapon damage upgrades**: +10% per tier, 5–7 tiers per weapon type. Special upgrades unlock after 2–3 base tiers.
**Armor/protection upgrades**: Health, shields, medi-gel capacity, melee damage.
**Biotic/Tech upgrades**: +10% damage per tier, +20% duration, +20% cooldown reduction.

See [docs/research.md](docs/research.md) for the complete research tree with costs and effects.

### Resources

| Resource | Primary Use | Approximate Total Needed |
|----------|-----------|-------------------------|
| Palladium | Pistols, shields, armor, ship armor | ~237,500 |
| Platinum | Shotguns, snipers, medical, ship weapons | ~305,000 |
| Iridium | ARs, SMGs, heavy weapons, scanner | ~242,500 |
| Element Zero | Biotic/tech upgrades, respec, bonus powers | ~35,000 |

Additional Element Zero costs: Advanced Training (bonus power) = 5,000 per use. Respec = 2,500 per use.

### Normandy Ship Upgrades

| Upgrade | Cost | Effect | Suicide Mission Critical? |
|---------|------|--------|--------------------------|
| Silaris Heavy Ship Armor | 15,000 Palladium | Hull protection | **YES** |
| Cyclonic Barrier Technology | 15,000 Palladium | Kinetic barriers | **YES** |
| Thanix Cannon | 15,000 Platinum | Main weapon | **YES** |
| Advanced Mineral Scanner | 15,000 Iridium | Faster scanning | No |
| Modular Probe Bay | 15,000 Iridium | 60 probe capacity | No |
| Extended Fuel Cells | 3,000 Element Zero | +50% fuel | No |
| Med-Bay Upgrade | 50,000 Platinum | Heals facial scarring | No |

---

## 7. Enemies & Opponents

### Factions

Six major enemy factions plus wildlife, each with distinct unit compositions and protection profiles:

- **Collectors**: Drones, Guardians (hex shields), Assassins, Captains, Oculus (drone sphere); possessed by Harbinger
- **Reaper Forces**: Husks (melee swarm), Abominations (explode), Scions (shockwave cannon), Praetorians (twin beams + barrier regen)
- **Geth**: Troopers, Rocket Troopers, Shock Troopers, Destroyers, Hunters (cloaked), Primes (elite), Colossus
- **Blue Suns**: Troopers, Legionnaires (Tech Armor), Centurions, Commanders, Heavies (rockets)
- **Blood Pack**: Vorcha Troopers (regen), Pyros (flamethrower, fuel tank weak point), Boom-Squad (grenades), Krogan Warriors (charge + regen)
- **Eclipse**: Troopers, Engineers (drones), Vanguards (barriers), Operatives, Commandos (3 protection layers)
- **Mechs**: LOKI (basic), FENRIS (fast melee, explode on death), YMIR (mini-boss; 2,150 Shields / 2,150 Armor / 500 Health)

See [docs/enemies.md](docs/enemies.md) for the complete enemy roster with per-unit protection values, weapons, and behaviors.

### Difficulty Protection Scaling

| Difficulty | Enemy Protection |
|-----------|-----------------|
| Casual/Normal | Many enemies have health only |
| Hardcore | Basic enemies gain +75 protection (shields/barriers/armor added) |
| Insanity | Basic enemies gain +150 protection; virtually all enemies have at least one protection layer |

### Boss Encounters

**Harbinger** (Possessed Collector): Uses only biotic powers (CollectorWarp, Singularity variant, close-range CollectorPulse). Immune to freeze/Stasis. When killed, immediately possesses another Collector Drone. Drops ammo on death. Barriers + Armor, no health bar.

**Praetorian** (Horizon, Collector Ship): Twin particle beams (3s burst, 2.5s cooldown). Death Choir AoE regenerates its barrier every time ¼ of its armor is depleted. Weak point: cluster of human heads in its maw. High Barriers + Very High Armor.

**Human-Reaper Larva** (Final Boss): Glowing weak points (eyes, chest, mouth) take headshot-equivalent bonus damage. Charge-up beam prevents shield regen. Collector reinforcements arrive periodically on platforms. Shields + Armor + Health.

**Thresher Maw** (Tuchanka): Survive 5 minutes to pass. Killing it earns unique prestige dialogue (last krogan to do so was Urdnot Wrex). Area-effect acid spit tracks even cloaked targets.

**Shadow Broker** (DLC): Incapacitates one squadmate before fight (2v1). Revenant LMG + devastating charge that destroys cover. Vulnerable to Stasis even through protections. Shields + Armor + Health.

---

## 8. Economy

### Credits

- **Total available** (all missions + assignments + DLC): ~1,280,000 credits
- **Cerberus Funding**: Catch-up mechanic. Each mission has a total credit value; Cerberus Funding fills the gap between collected and total (e.g., Freedom's Progress = 30,000 total)
- **Other sources**: Datapads, wall safes, salvage during missions; Renegade dialogue sometimes yields extra credits
- **Total needed** to buy everything useful: ~1,253,000 credits (with discounts)

### Credit Sinks

| Category | Example Costs |
|----------|--------------|
| Armor pieces | 2,000–8,000 per piece |
| Weapon upgrades (shop) | Up to 60,000 per tier |
| Star charts (Baria Frontiers) | ~2,000 total for all maps |
| Fuel | 1 credit/unit (max 1,000–1,500 capacity) |
| Probes | 100 credits per 5 probes |
| Cabin decorations | 500–8,000 (fish), ~500 (model ships) |

### Shops

| Location | Shop | Specialty |
|----------|------|-----------|
| Citadel | Rodam Expeditions | Weapons |
| Citadel | Sirta Foundation | Health/defense |
| Citadel | Saronis Applications | Tech upgrades |
| Citadel | Citadel Souvenirs | Model ships, fish |
| Omega | Harrot's Emporium | Weapon mods |
| Omega | Kenn's Salvage | Salvaged tech (discount via dialogue) |
| Omega | Omega Market | Miscellaneous |
| Illium | Gateway Personal Defense | Weapons, armor |
| Illium | Serrice Technology | Biotic/tech |
| Illium | Baria Frontiers | Star charts |
| Illium | Memories of Illium | Collectibles |
| Tuchanka | Ratch's Wares | Heavy weapon ammo, misc |
| Tuchanka | Fortack's Database | Weapon upgrades |

Most shops offer 10–20% discounts via Charm/Intimidate dialogue or completing related assignments.

---

## 9. Minigames & Side Systems

### Hacking (Code Matching)

Three columns of code blocks scroll upward. Match Target Code Segments (shown top-left) by selecting the correct block from the scrolling columns. Red "X" blocks cancel a previous match. Three wrong selections = failure. Timer varies by object difficulty. Hack Module (purchased on Omega) extends the timer.

### Bypass (Circuit Board Matching)

Circuit board with 3–5 node pairs. Reveal hidden icons by selecting nodes; find matching pairs from memory or by tracing circuit lines. Real-time countdown; taking combat damage = instant failure. Bypass Module (purchased on Illium) extends the timer.

### Firewalker / Hammerhead Vehicle (DLC)

The Firewalker Pack adds the **M-44 Hammerhead**, a hover-tank used in 5 dedicated assignments and portions of the Overlord DLC. The Hammerhead has:
- Hover propulsion with boost/dodge capability
- Main cannon (rapid-fire energy weapon)
- Health bar that regenerates when not taking damage
- No squad powers or weapon switching — vehicle combat only

Project Firewalker assignments involve navigating lava/hazard terrain, recovering Prothean artifacts, and vehicle combat against geth and mercenaries. Completing all 5 yields a Prothean Relic cabin item and XP.

### N7 Assignments

19 side missions discovered primarily through planet scanning anomalies. Organized into Act 1 (6 missions, pre-Horizon) and Act 2 (13 missions, post-Horizon). Several form chains triggered by email after completing the prior mission (e.g., Archeological Dig Site → MSV Strontium Mule → Blue Suns Base). Star charts from Baria Frontiers on Illium unlock clusters containing later N7 missions.

### Hub World Assignments

Optional quests on hub worlds, discovered through overheard conversations, NPC dialogue, or email:
- **Citadel**: Crime in Progress, Krogan Sushi, Found Forged ID, False Positives, Special Ingredients
- **Omega**: The Patriarch, Packages for Ish, Batarian Bartender
- **Illium**: Liara: The Observer, Liara: Systems Hacking, Conrad Verner, Gianna Parasini, Medical Scans, Blue Rose of Illium, Indentured Service
- **Tuchanka**: Combustion Manifold, Pyjak shooting

### Collection Systems

- **Model Ships**: Collectible ship models displayed in the Captain's Cabin (~500 credits each)
- **Fish**: Aquarium fish purchased from various shops (500–8,000 credits); must be fed regularly or they die
- **Codex Entries**: Extensive encyclopedia with primary (narrated by Neil Ross) and secondary (text-only) entries

---

## 10. UI & HUD

### Combat HUD

| Position | Element |
|----------|---------|
| Bottom-left | Current weapon icon/name, thermal clip counter (shots/reserve) |
| Bottom-center | Squad bar with two portraits (health, shield status, cooldown indicators) |
| Center | Targeting reticle (shape varies by weapon), power cooldown semicircles (sweep inward; meet at center = ready) |
| Upper-left | Quick-slot power icons (PC hotkeys) |
| Lower-right | Notification feed (pickups, morality points, acquisitions) |

### HUD State Changes

| State | HUD Behavior |
|-------|-------------|
| Exploration | Minimal; no weapon/ammo display; squad bar visible |
| Combat | Full HUD with weapon info, ammo, squad status |
| Aim mode | Camera zooms to tight over-shoulder; reticle tightens |
| Conversation | HUD hidden; dialogue wheel appears bottom-center |
| Taking damage | Screen edges flash red (health damage); distinct shield-break visual/audio cue |

### Damage Feedback

- Shield bar (light blue semicircle) only appears when shields are depleting
- Health bar only visible when shields are fully broken
- Enemy health/protection bars appear above targeted enemies
- Interaction prompts near cover points, doors, loot

### Power Wheel (Radial Menu)

Pauses combat. Squad portraits left and right; Shepard's powers center/top. Power icons colored: **orange** = effective against target, **red** = ineffective but usable, **grey** = on cooldown. Can also direct squad movement or target enemies.

### Menu Structure

- **Squad**: Level-up, spend squad points, view details
- **Map**: 2D overhead of current area
- **Journal**: Active missions and assignments, ordered by acquisition
- **Codex**: Two-tier encyclopedia (Primary: narrated; Secondary: text-only). Categories: Aliens (Council/Non-Council), Humanity, Organizations, Planets, Science, Ships, Weapons/Armor, Tutorials
- **Options**: Gameplay, Graphics, Sound
- **Save / Load / Main Menu**

---

## 11. Engine & Presentation Systems

### Conversation System

**Dialogue Wheel** layout (6 positions):
- Right side (progresses conversation): Top-right = Paragon, Middle-right = Neutral, Bottom-right = Renegade
- Left side (deepens conversation): Top-left = Charm (blue, requires Paragon threshold), Middle-left = Investigate, Bottom-left = Intimidate (red, requires Renegade threshold)

**Interrupts**: Timed prompts during cutscenes. Paragon (blue icon, bottom-left) accompanied by an uplifting hum. Renegade (red icon, bottom-right) accompanied by a metal gong. Appear for 2–3 seconds; entirely optional.

Camera uses BioWare's custom cinematic staging on UE3 Matinee — hand-placed shots per conversation node (establishing, medium, close-up, over-the-shoulder, dramatic low angle). Not procedural.

### Save System

- **Autosave**: Checkpoints at area transitions, before/after major encounters, dialogue starts
- **Quicksave**: F5 on PC; single rotating slot
- **Manual save**: From pause menu; unavailable during active combat or the Suicide Mission sequence
- Post-ending autosave stores complete game state for ME3 import
- Save location: `Documents\BioWare\Mass Effect 2\Save\`

### Difficulty Settings

| Difficulty | Description | Unlock |
|-----------|-------------|--------|
| Casual | Enemies have mostly health only; high player damage | Available from start |
| Normal | Bosses scale; some enemies gain protections | Available from start |
| Veteran | Most enemies gain protection layers | Available from start |
| Hardcore | All common enemies gain armor; player damage ~50% of Normal | Complete any playthrough |
| Insanity | All enemies gain shields/barriers/armor; player damage ~25–33% of Casual; aggressive AI | Complete Hardcore |

### Camera Behavior

- **Exploration**: Over-the-shoulder, offset right; Shepard occupies ~¼ of screen width. Default FOV ~70°. Cannot switch shoulder in vanilla.
- **Aim mode**: Tighter over-shoulder zoom; precision reticle
- **Cover**: Snaps to cover edge; peeking shifts camera to show firing angle
- **Conversation**: Hand-authored cinematic angles per dialogue node

### Audio System

- **Engine**: Audiokinetic Wwise (rebuilt from ME1's ISACT)
- **Dynamic music**: Four discrete activity levels with hand-scripted transitions (30–60 per level). Bass and harmony remain constant; texture and instrumentation change. Three-layered combat system avoids abrupt on/off transitions.
- **Ambient design**: Environments have distinct sonic identities (Omega: distant gunshots and arguments; Collector environments: deep bass with reverbed synths)
- **Codex narrator**: Primary entries voiced by Neil Ross; secondary entries text-only

---

## 12. Open Questions / Unverified

| Topic | Issue |
|-------|-------|
| Difficulty ScalingFactor | Exact formula relating player level + difficulty to the damage scalar is not publicly datamined. Multiple sources confirm it exists but acknowledge the gap. |
| Cover damage reduction | No authoritative source provides exact cover DR percentages. Community estimates 50–75% based on gameplay testing. |
| Incendiary Ammo rank 1 | Gamestegy shows 20% (consistent with original ME2 files); some Fandom/CBR sources show 30% (may reflect Legendary Edition). The 20/30/40/60 progression appears more consistent with other ammo power scaling. |
| Weapon base damage values | Community numbers (Predator ~22.5, Mantis ~263.1, Widow ~368.3) come from coalesced.ini extraction and may differ between original ME2 and Legendary Edition. |
| Morality bar maximum | ~970 points from player testing, not official BioWare data. The percentage-based check system makes absolute totals less important than ratio. |
| Shotgun barrier penetration bug | Confirmed by community testing that the shotgun penetration research upgrade does not apply to Barriers; unclear if this was fixed in Legendary Edition. |
| Squadmate weapon multipliers | 40–65% range is confirmed across multiple sources but exact per-weapon-type values are not consistently documented. |

---

## 13. References

### Wikis
- [Mass Effect Wiki (Fandom)](https://masseffect.fandom.com/) — primary source for mission structure, powers, weapons, lore
- [Fextralife ME2 Wiki](https://masseffect2.wiki.fextralife.com/) — research costs, armor pieces, DLC content

### Community Datamining
- [peddroelm Weapon Damage Formula (BSN Archive)](https://bsn.boards.net/thread/1672/me2-weapon-damage-formula-peddroelm) — definitive damage calculations
- [BioWare Developer Gameplay Data (BSN Archive)](https://bsn.boards.net/thread/1472/mass-effect-gameplay-bioware-developers) — official dev-posted multipliers
- [BSN Experience Level Chart](https://bsn.boards.net/thread/14763/me2-experience-needed-level-chart) — XP progression
- [GameBanshee Advancement Table](https://www.gamebanshee.com/masseffect2/advancementtable.php)

### Strategy Guides
- [GameFAQs Weapon Damage Formulas](https://gamefaqs.gamespot.com/boards/944907-mass-effect-2/55117441)
- [GameFAQs Upgrades & Research Cost Breakdown](https://gamefaqs.gamespot.com/boards/944907-mass-effect-2/53382677)
- [RPG Site — Ship Upgrades Guide](https://www.rpgsite.net/feature/11150-mass-effect-2-ship-upgrades-normandy-research-guide)
- [RPG Site — Suicide Mission Guide](https://www.rpgsite.net/feature/11118-mass-effect-2-suicide-mission-choices)
- [RPG Site — Mission Order Guide](https://www.rpgsite.net/feature/11148-mass-effect-2-mission-order-optimal-dlc-dossier-loyalty-quest-sequence)
- [RPG Site — Assignments Guide](https://www.rpgsite.net/feature/11245-mass-effect-2-assignments-every-side-quest-how-to-trigger-them)
- [Steam Community — Comprehensive Weapon Guide](https://steamcommunity.com/sharedfiles/filedetails/?id=871343467)

### Audio / Presentation
- [Designing Sound — ME2 Sound Team Interview](https://designingsound.org/2011/01/24/exclusive-interview-with-the-sound-design-team-of-mass-effect-2/)
- [VGSoundTest — ME2 Dynamic Music Study](https://vgsoundtest.blogspot.com/2012/07/mass-effect-2-study-in-dynamic-music.html)

### Companion Documents
- [docs/powers.md](docs/powers.md) — Complete power list with evolution branches, cooldowns, and damage values
- [docs/weapons.md](docs/weapons.md) — All weapons by category with stats and special properties
- [docs/armor.md](docs/armor.md) — Armor pieces, full sets, stat bonuses, and acquisition
- [docs/enemies.md](docs/enemies.md) — Enemy factions, unit types, protections, and boss mechanics
- [docs/research.md](docs/research.md) — Complete research upgrade tree with costs

### DLC Content

| DLC Pack | Content |
|----------|---------|
| Normandy Crash Site | 1 non-combat memorial assignment |
| Zaeed: The Price of Revenge | Squadmate + loyalty mission + M-451 Firestorm |
| Kasumi: Stolen Memory | Squadmate + loyalty mission + M-12 Locust |
| Firewalker Pack | M-44 Hammerhead vehicle, 5 assignments |
| Overlord | 4 assignments on Aite (rogue VI), Hammerhead sequences |
| Lair of the Shadow Broker | Full mission arc with Liara, Stasis bonus power, Shadow Broker terminal (respec, intel, investments) |
| Arrival | Solo bridge mission to ME3 |
| Cerberus Weapon and Armor | Cerberus Assault Armor, M-22 Eviscerator |
| Firepower Pack | M-5 Phalanx, M-96 Mattock, Geth Plasma Shotgun |
| Aegis Pack | M-29 Incisor, Kestrel Armor set |
| Equalizer Pack | Capacitor Helmet, Archon Visor, Inferno Armor |
| Arc Projector | Arc Projector heavy weapon |
| Blood Dragon Armor | Dragon Age crossover set |
| Collectors' Edition | Collector Assault Rifle, Collector Armor |
| Terminus Pack | Terminus Assault Armor, M-490 Blackstorm |

All DLC included free in the Legendary Edition (2021).
