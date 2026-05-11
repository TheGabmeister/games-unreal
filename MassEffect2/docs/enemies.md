# Mass Effect 2 — Enemies Reference

All enemy factions, unit types, protection values, and boss mechanics. Referenced from [SPEC.md §7](../SPEC.md).

---

## Protection Layer System

Enemies have up to three protection layers displayed as colored bars:
- **Health** (red) — base layer, all enemies have this
- **Shields** (blue) — kinetic barriers; weak to Overload, Disruptor Ammo, SMGs
- **Barriers** (purple) — biotic barriers; weak to Warp, Reave, Concussive Shot (3.5×)
- **Armor** (yellow) — weak to Incinerate, Incendiary Ammo, Warp, snipers/pistols

While any protection layer is active, enemies are **immune to crowd-control** (Throw, Pull, Cryo freeze, Singularity lift).

### Difficulty Scaling

| Difficulty | Protection Changes |
|-----------|-------------------|
| Casual/Normal | Many basic enemies have health only |
| Hardcore | Basic enemies gain +75 protection bonus (shields/barriers/armor added) |
| Insanity | Basic enemies gain +150 protection bonus; virtually every enemy has at least one layer |

---

## Collectors

| Unit | Weapon | Protections (Normal) | Protections (Insanity) | Behavior |
|------|--------|---------------------|----------------------|----------|
| Collector Drone | Collector AR | Health | Health + Barriers | Basic infantry; can be possessed by Harbinger |
| Collector Guardian | Collector AR, Warp Ammo | Health + Barriers | Health + Barriers (stronger) | Deploys hex shield that blocks projectiles |
| Collector Assassin | Collector Particle Beam | Health + light Barriers | Health + Barriers | Stronger barriers on higher difficulties |
| Collector Captain | Collector AR | Health + Barriers (2×) | Health + Barriers (2×) | Summons Seeker Plagues; 2× health; vulnerable while summoning |

### Oculus (Collector Drone)

- **Encounters**: Suicide Mission approach (space), Suicide Mission interior (ground)
- **Protections**: Armor only (very high)
- **Attacks**: Sustained energy beam (high damage, sweeps across area)
- **Behavior**: First encountered as a flying sphere during the approach to the Collector Base (controlled by Joker/EDI aboard the Normandy). Later encountered on foot inside the base. Weak to heavy weapons and anti-armor powers.

### Harbinger (Possessed Collector)

- Protections: Barriers + Armor (no health bar)
- Attacks: CollectorWarp, Singularity variant, close-range CollectorPulse (nova)
- Never fires weapons — only biotic powers
- Immune to freeze/Stasis
- Always advances directly on Shepard
- When killed, immediately possesses another Collector Drone
- Drops ammo pickups on death

---

## Reaper Forces / Husks

| Unit | Protections (Normal) | Protections (Insanity) | Behavior |
|------|---------------------|----------------------|----------|
| Husk | Health | Armor | Melee rushers; swarm in waves |
| Abomination | Health | Armor | Explosive husk; detonates on proximity or death; splash damage |
| Scion | Armor only | Armor only | Shockwave cannon penetrates cover; weak points: shoulder sacs, head; immune to most CC (Stasis works); slow |
| Praetorian | Barriers + Armor | Barriers + Armor (very high) | See Boss Encounters below |

---

## Geth

| Unit | Weapon | Protections (Normal) | Protections (Insanity) | Behavior |
|------|--------|---------------------|----------------------|----------|
| Geth Trooper | Geth Pulse Rifle | Shields + Health | Shields + Health (stronger) | Standard infantry; can use Geth Shield Boost |
| Geth Rocket Trooper | Rocket Launcher | Shields + Health | Shields + Health | Anti-armor/anti-air; rockets stagger |
| Geth Shock Trooper | Geth Pulse Rifle | Shields + Health | Shields + Health | Heavy infantry; Geth Barriers; Carnage |
| Geth Destroyer | Geth Pulse Shotgun | Strong Shields + Health | Strong Shields + Health | Close-quarters charge/melee; Carnage |
| Geth Hunter | Geth Plasma Shotgun | Shields + Health | Shields + Health | Tactical Cloak; untargetable by squadmates while cloaked; fast |
| Geth Prime | Pulse Rifle + Drone | High Shields + Heavy Armor + Health | Very High Shields + Armor + Health | Deploys combat/repair drones; no cover-seeking; headshot zone above eye |
| Geth Colossus | Siege Pulse + MG | Very Heavy Shields + Armor | Very Heavy Shields + Armor | Heavy walker; devastating siege pulse |
| Geth Bomber | Grenades + electrical | Light Shields + Armor | Shields + Armor | Close air support drone |

---

## Blue Suns

| Unit | Protections (Normal) | Protections (Insanity) | Behavior |
|------|---------------------|----------------------|----------|
| Blue Suns Trooper | Health | Shields + Health | Basic infantry |
| Blue Suns Legionnaire | Shields + Health | Shields + Health (stronger) | Tech Armor; shield regeneration |
| Blue Suns Centurion | Shields + Health | Shields + Health (stronger) | Second-highest rank; Tech Armor |
| Blue Suns Commander | Shields + High Armor + Health | Shields + High Armor + Health | Buffs nearby troops; Tech Armor; very durable |
| Blue Suns Heavy | Health | Shields + Health + Tech Armor | ML-77 rockets; always human; rockets stagger |

---

## Blood Pack

| Unit | Species | Protections (Normal) | Protections (Insanity) | Behavior |
|------|---------|---------------------|----------------------|----------|
| Blood Pack Trooper | Vorcha | Health | Armor + Health | Cannon fodder; health regeneration |
| Blood Pack Pyro | Vorcha | Health | Armor + Health | Flamethrower; health regen; fuel tank weak point (back) |
| Blood Pack Boom-Squad | Vorcha | Health | Armor + Health | Grenade Launcher; area denial |
| Blood Pack Warrior | Krogan | Health + Armor | Health + Armor (stronger) | Krogan officer; health regen; Carnage blasts; devastating melee charge |

---

## Eclipse

| Unit | Protections (Normal) | Protections (Insanity) | Behavior |
|------|---------------------|----------------------|----------|
| Eclipse Trooper | Health | Shields + Health | Human/salarian basic infantry |
| Eclipse Engineer | Shields + Health | Shields + Health (stronger) | Incinerate, Combat Drone |
| Eclipse Vanguard | Barriers + Health | Barriers + Health (stronger) | Biotic protection; can use Tech Armor to replenish barriers |
| Eclipse Operative | Shields + Health | Shields + Health | M-9 Tempest SMG; durable engineer variant |
| Eclipse Heavy | Shields + Health | Shields + Health | Rocket launcher |
| Eclipse Commando | Barriers + Armor + Health | Barriers + Armor + Health | Three strong protection layers; asari biotics; can recharge barriers; extremely durable |

---

## Mechs

| Unit | Protections | Behavior |
|------|-----------|----------|
| LOKI Mech | Health (Normal) / Armor + Health (Insanity) | M-4 Shuriken SMG; no recoil; electrical melee at close range; frequently explodes via headshot |
| FENRIS Mech | Health (Normal) / Armor + Health (Insanity) | Quadrupedal; short-range taser; among fastest enemies; swarm in packs; explode on death (splash + stagger) |
| YMIR Mech | 2,150 Shields + 2,150 Armor + 500 Health | Twin autocannons (right arm) + rocket launcher (left arm); immune to CC/stagger while any protection intact; vulnerable to freeze/stun when unprotected; violent self-destruct on death |

### YMIR Mech Details

The YMIR is a recurring mini-boss with consistent stats across encounters (some variants: 2,000/2,000/500):
- Focuses almost exclusively on Shepard
- Missiles can be dodge-rolled with precise timing
- Self-destruct explosion has large radius — clear the area when it drops to health-only
- Immune to ALL force effects, stagger, and crowd-control while shields or armor remain
- Once unprotected (health only): vulnerable to freeze, stun, and CC

---

## Wildlife

| Unit | Protections | Behavior |
|------|-----------|----------|
| Varren | Health (Normal) / Armor + Health (Insanity) | Melee chargers; used by Blood Pack as attack animals |
| Klixen | Armor + Health | Insectoid; fire-breathing; explode violently on death; found on Tuchanka |

---

## Boss Encounters

### Praetorian

- **Encounters**: Horizon, Collector Ship
- **Protections**: High Barriers + Very High Armor
- **Attacks**: Twin Particle Beams (3s burst, 2.5s cooldown — lethal in seconds); Death Choir (ground slam AoE)
- **Phase mechanic**: Barrier regenerates via Death Choir every time ¼ of armor bar is depleted. Praetorian lands, slams, then refreshes barrier before resuming flight.
- **Weak point**: Cluster of human heads visible in its maw (headshot zone)
- **Strategy**: Maintain distance; exploit 2.5s cooldown window; avoid triggering Death Choir from squad proximity

### Human-Reaper Larva (Final Boss)

- **Location**: Collector Base final battle
- **Weak points**: Two glowing eyes (left socket), one eye (right socket), large glowing chest, glowing mouth — all take headshot-equivalent bonus damage
- **Attacks**: Charge-up beam (prevents shield regen on hit, indicated by red lightning); attack frequency increases below 25% health
- **Reinforcements**: Collector soldiers (including Harbinger) arrive on fly-in platforms periodically; killing them can drop heavy weapon ammo
- **Effective weapons**: Collector Particle Beam (most effective overall; no headshot bonus), sniper rifles, heavy pistols
- **Effective powers**: Incinerate, Warp, Reave (fully evolved)

### Thresher Maw (Tuchanka — Grunt: Rite of Passage)

- **Objective**: Survive 5 minutes. Killing it earns unique prestige (last krogan to do so was Urdnot Wrex).
- **Attacks**: Area-effect acid spit that tracks even cloaked targets
- **Reward for kill**: Breeding requests for Grunt; respect dialogue from krogan NPCs

### Shadow Broker (Lair of the Shadow Broker DLC)

- **Setup**: Incapacitates one squadmate before the fight — effectively 2v1 (Shepard + Liara)
- **Protections**: Shields + Armor + Health
- **Attacks**: Revenant LMG fire; devastating charge that destroys cover pylons and deals damage through cover
- **Vulnerabilities**: Stasis works even through intact protections; Flashbang Grenades stagger
- **Strategy**: Kite using twin staircases; conventional weapons more effective than heavy weapons (swap time penalty)

### A-61 Mantis Gunship (Multiple Encounters)

- **Locations**: Dossier: Archangel, Kasumi: Stealing Memory, others
- **Protections**: Armor only (Hock's variant adds military-grade kinetic barriers)
- **Attacks**: Inferno PKR rockets (twin bays), M350 autocannons
- **Note**: During Archangel, initially drops troops and fires rockets through windows before becoming targetable

### Harbinger (Recurring)

- **Protections**: Barriers + Armor (no health bar)
- **Behavior**: Possesses Collector Drones continuously; when one possessed form is destroyed, immediately takes over another
- **Attacks**: CollectorWarp, Singularity variant, close-range CollectorPulse (nova)
- **Immune to**: Freeze, Stasis
- **Drops**: Ammo pickups on death of each possessed form
