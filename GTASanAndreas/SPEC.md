# SPEC.md — GTA: San Andreas Gameplay Mechanics

This document catalogs every major gameplay system in GTA: San Andreas (2004) for recreation in Unreal Engine 5. No implementation details — only what the game does, not how we'll build it.

---

## 1. Player Movement

### On-Foot Locomotion

| Mode | Behavior |
|------|----------|
| Walk | Slow speed; analog-stick modulated |
| Run | Default speed; uses `run_civi` animation |
| Sprint | ~13% faster than running (measured by speedrunners); tapping sprint is slightly faster than holding; drains stamina |
| Crouch | Silent movement; minimap blip turns blue; enables combat roll; slower than walking |
| Jump | Forward velocity preserved; auto-climbs reachable ledges on contact |
| Swim (surface) | Breaststroke (default, slow) or front crawl (hold sprint, fast, drains stamina) |
| Swim (dive) | Enter by pressing fire while on surface; sprint button for forward thrust; drains breath |
| Climb | Auto-latches to ledges/fences/walls when jumping toward them; pull-up animation plays |
| Combat roll | While aiming + crouching, press jump + direction; brief invulnerability frames; requires firearm equipped |

### Physics Constants

| Parameter | Value | Notes |
|-----------|-------|-------|
| Gravity | 0.008 per frame at 25 fps | Address `0x863984`; equivalent to ~0.2 m/s² per tick |
| Terminal fall velocity | 55 m/s | Uniform; no realistic acceleration curve |
| Game units | 1 unit ≈ 1 meter | Community consensus from MTA/SA-MP testing |
| Frame-rate dependency | Physics tied to frame rate | 25 fps canonical; swimming is measurably faster at 25 fps vs 30 fps |

### Fall Damage

- No damage below a threshold height (small drops)
- Appears non-linear (quadratic) — moderate heights deal disproportionately high damage
- NOT capped: extreme heights are lethal
- With max health (176 from Paramedic), survivable from any height without parachute
- Armor provides NO fall damage protection

### Stamina

| Property | Value |
|----------|-------|
| Internal scale | 0–1000 (displayed as 0–100%) |
| Starting value (new game) | 100 (10%) |
| Gain: running/cycling | +50 points per 300 s (5 min) |
| Gain: swimming | +50 points per 150 s (2.5 min) |
| Gain: gym treadmill/bike | +40 points per 14 s |
| Daily cap | +200 points (20%) per game-day |
| Infinite stamina unlock | Earn $10,000 cumulative from Burglar missions (sets bool at `0xB7CEE4`) |
| Effect | Determines sprint/fast-swim/fast-cycle duration; at max stat (~1000) sprint is effectively infinite |

### Lung Capacity (Breath)

| Property | Value |
|----------|-------|
| Internal scale | 0–1000 (address `0xB791A4`) |
| Gain rate | +50 points per 60 s spent underwater |
| Min capacity duration | ~15–20 s underwater |
| Max capacity duration | ~2–3 min underwater |
| When depleted | Health drains rapidly until surfacing or death |
| Oyster bonus | All 50 Oysters → infinite lung capacity |
| Mission gate | "Amphibious Assault" requires minimum 20% |

### Movement State Machine

The game tracks `eMoveState` at CPed offset `+0x534`:
- 1 = STILL, 4 = STARTING, 5 = JOG, 6 = RUN, 7 = SPRINT

Jump state at CPed offset `+0x46D`: 32 = landed, 34 = airborne, 36 = landing

**No coyote time exists.** Leaving a ledge without jumping immediately enters airborne state.

### Fat Effects on Movement

- At >20–25% fat: game switches to `FatSprint` and `FatWalk` animation groups (inherently slower playback)
- Reduced jump height
- No exact percentage reduction documented; implemented as animation-group swap rather than speed multiplier

---

## 2. Hand-to-Hand Combat

### Fighting Styles

| Style | ID | Gym Location | Unlock Requirement | Anim Group | Kill Power |
|-------|----|--------------|--------------------|------------|------------|
| Default | 4 | — | Starting style | Base fight anims | Low |
| Boxing | 5 | Ganton Gym, LS | 35% Muscle + defeat trainer | FIGHT_B | Medium |
| Kung Fu | 6 | Cobra Martial Arts, SF | 35% Muscle + defeat trainer | FIGHT_C | Medium |
| Muay Thai | 7 | Below the Belt, LV | 35% Muscle + defeat trainer | FIGHT_D | **High** |

- Only one style active at a time; learning a new one replaces the previous permanently
- Cannot recover a previous style once replaced
- All 3 trainer defeats required for 100% completion

### Per-Style Combo Chains

| Style | Combo | Running Attack | Ground Attack |
|-------|-------|----------------|---------------|
| Default | 2-hit: punch → kick (kick knocks down) | Same as standing punch | Same as standing punch |
| Boxing | Left uppercut → right hook | Rush punch | Downward punch to prone enemy |
| Kung Fu | Rapid punches/kicks → roundhouse/spinning kick | Flying kick (most visually impressive) | Stomp |
| Muay Thai | 4-hit: knees + punches → elbow strike (1-hit kill on normal NPCs) | Short jab rush | Hardcore stomp |

**Muay Thai elbow:** deals extremely high damage (~500) that exceeds normal NPC health pools. Armored/high-health NPCs (story targets, FBI, military, gym trainers) survive with very low health remaining. Not literally infinite damage.

### Combo Mechanics

- Timing is **time-based, not frame-based** — driven by a `Chain` float (seconds) per attack move in `melee.dat`
- **Timed presses advance** through the combo chain; **button mashing resets to the first move**
- Each move in a chain has its own damage value (different hits deal different amounts)
- No visual combo counter — the system is entirely internal
- Data source: `data/melee.dat` defines per-combo entries with fields: Hit, Chain (follow-on time), Radius, HitLevel, Damage

### Running Attacks

- Triggered by: **lock on to target + moving toward them + attack**
- Sprint is NOT required — jogging while locked on is sufficient
- Each style has a unique running attack animation (the `_M` / moving attack animation)

### Ground Attacks

- Target must be in a knockdown/prone animation state
- Same input as normal attack while locked on to downed enemy — game detects target is down and plays ground-specific animation
- Works on any knocked-down NPC

### Blocking

- Hold lock-on button without pressing attack to block
- **No stamina cost** — blocking is free to maintain
- Reduces incoming melee damage (exact multiplier not publicly documented)
- Guard breaking under sustained pressure is an **NPC AI behavior transition**, not a discrete hit-count threshold — NPCs drop guard after sustained hits
- NPCs block more when their health is low
- Available with all styles including default

### Gym Trainer Fights

- Requires 35% Muscle minimum
- **Real combat encounter** against a trainer NPC with boosted health (not scripted)
- On-screen prompts teach new moves during the fight
- If CJ loses: hospital respawn (normal death penalty)
- If CJ **forfeits mid-fight** (leaves gym): **loses all carried weapons** (significant penalty)
- LV kickboxing trainer is hardest — max health and muscle recommended
- On completion: unarmed moveset permanently replaced
- CPed fields: `m_nFightingStyle` (eFightingStyle enum), `m_nAllowedAttackMoves`, `m_nAttackTimer`

### Hit Reactions

- GTA:SA does NOT have ragdoll physics (introduced in GTA IV) — hit reactions use canned animations
- 3 hit reaction animations per set (low/mid/high), determined by `HitLevel` field in `melee.dat`
- Hit reactions are tied to the **defender's** animation set, not the attacker's style
- All enemies use the same stagger/stumble/fall animations regardless of what style hit them
- The kickboxing elbow causes a unique knockdown/death animation due to extreme damage

### Melee Damage Scaling

- Muscle stat (0–1000) acts as damage multiplier on all melee attacks
- Higher muscle = more damage per hit; applies to all styles and melee weapons
- Max muscle + kickboxing can destroy a car in ~25 punches
- Visual model changes at 50% Muscle (walk animation shifts)
- 100% Muscle: slight sex appeal penalty

---

## 3. Weapon Combat

### Weapon Slots (13)

| Slot | Category | Weapons |
|------|----------|---------|
| 0 | Hand | Fist, Brass Knuckles |
| 1 | Melee | Knife, Baseball Bat, Katana, Chainsaw, Golf Club, Shovel, Pool Cue, Nightstick |
| 2 | Handguns | 9mm Pistol, Silenced 9mm, Desert Eagle |
| 3 | Shotguns | Pump-Action, Sawn-Off, Combat Shotgun (SPAS-12) |
| 4 | SMGs | Tec-9, Micro SMG (Micro Uzi), SMG (MP5) |
| 5 | Assault Rifles | AK-47, M4 |
| 6 | Rifles | Country Rifle, Sniper Rifle |
| 7 | Heavy | Flamethrower, Rocket Launcher, Heat-Seeking RPG, Minigun |
| 8 | Thrown | Tear Gas, Molotov, Grenade, Satchel Charges |
| 9 | Handheld | Fire Extinguisher, Spray Can, Camera |
| 10 | Gifts | Flowers, Cane, Dildo, Vibrator |
| 11 | Special | NVG, Thermal Goggles, Parachute |
| 12 | Detonator | Satchel Charge Detonator |

### Firearm Data (from weapon.dat)

#### Handguns

| Weapon | Damage | Clip | Fire Rate (ms) | Skill Tiers |
|--------|--------|------|----------------|-------------|
| 9mm Pistol | 25 | 17 (34 dual) | ~300 | Poor/Gangster@40/Hitman@999 |
| Silenced 9mm | 40 | 17 | ~240 | Poor/Gangster@500/Hitman@999 |
| Desert Eagle | 70→**140** | 7 | ~800 | Poor/Gangster@200/Hitman@999 |

Desert Eagle damage DOUBLES at Gangster level (70→140). This is unique among all weapons.

#### Shotguns

| Weapon | Damage/Pellet | Pellets | Clip | Fire Rate (ms) | Effective Dmg/Shot |
|--------|---------------|---------|------|----------------|--------------------|
| Pump-Action | 10 | 15 | 1 | ~1060 | 150 max |
| Sawn-Off | 10 | 15 | 2 (4 dual) | ~300 | 150 max/barrel |
| Combat Shotgun (SPAS-12) | 15 | 8 | 7 | ~300 | 120 max |

All shotguns: Gangster@200, Hitman@999.

#### Sub-Machine Guns

| Weapon | Damage | Clip | Fire Rate (ms) | Effective RPM |
|--------|--------|------|----------------|---------------|
| Micro SMG | 20 | 50 (100 dual) | ~50 | ~1200 |
| Tec-9 | 20 | 50 (100 dual) | ~50 | ~1200 |
| SMG (MP5) | 25 | 30 | ~90 | ~667 |

Micro/Tec-9: Gangster@50, Hitman@999. MP5: Gangster@250, Hitman@999.

#### Assault Rifles

| Weapon | Damage | Clip | Range | Fire Rate (ms) | Accuracy (Poor→Hit) |
|--------|--------|------|-------|----------------|---------------------|
| AK-47 | 30 | 30 | 70.0 | ~120 | 0.40→0.60 |
| M4 | 30 | 50 | 90.0 | ~120 | 0.45→0.80 |

Both: Gangster@200, Hitman@999.

#### Rifles

| Weapon | Damage | Clip | Range | Notes |
|--------|--------|------|-------|-------|
| Country Rifle | 75 | 1 | 100.0 | Free-aim only; no lock-on |
| Sniper Rifle | 125 | 1 | 100.0 | Scoped; can decapitate |

Rifles have only one active skill tier (Gangster@300). No dual-wield, no Hitman.

#### Heavy Weapons

| Weapon | Damage | Ammo | Range | RPM | Notes |
|--------|--------|------|-------|-----|-------|
| Minigun | 140 | 500 | 75.0 | ~3000 | Requires spin-up |
| Rocket Launcher | 75 + explosion | 1 | 55.0 | — | Impact-triggered |
| Heat-Seeking RPG | 75 + explosion | 1 | 55.0 | — | Tracks heat sources |
| Flamethrower | 25 + burn DOT | 500 | 5.1 | — | Area effect |

#### Thrown Weapons

| Weapon | Damage | Fuse/Lifespan | Spread (blast multiplier) |
|--------|--------|---------------|---------------------------|
| Grenade | 75 + explosion | 800 (game units) | 1.0 |
| Molotov | 75 + fire DOT | 2000 | 5.0 (larger fire area) |
| Tear Gas | Stun/DOT | 800 | 1.0 |
| Satchel Charges | 75 + explosion | Until detonated | 1.0 |

Explosion radius: ~5–10 game units (hardcoded in explosion handler, not in weapon.dat).

#### Melee Weapons

| Weapon | Damage | Range | Combos | Notes |
|--------|--------|-------|--------|-------|
| Fist | 1–5 | 1.76 | 4 | Scales with Muscle |
| Brass Knuckles | 15 | 1.76 | 4 | |
| Knife | 15 | 1.76 | 1 | Stealth kill = instant |
| Baseball Bat | 20 | 1.76 | 1 | |
| Katana | 20 | 1.76 | 1 | Can decapitate |
| Golf Club | 10 | 1.76 | 1 | |
| Nightstick | 10 | 1.76 | 1 | |
| Pool Cue | 10 | 1.76 | 1 | |
| Shovel | 15 | 1.60 | 1 | |
| Chainsaw | 100 (continuous) | 1.76 | 1 | Sustained damage while held |

### Headshot / Body Damage

- **Headshots are instant kills** on peds with `SUFFERS_CRITICAL_HITS` = TRUE (default for most NPCs)
- This is a binary flag, NOT a damage multiplier
- Mission-critical NPCs often have this flag set to FALSE
- CJ (player) does NOT receive instant-kill headshots
- **No body-part damage differentiation** — all non-head hits deal flat weapon damage (no torso vs. limb multiplier)

### Weapon Skill Progression (Per-Tier Absolute Stats)

The game stores **separate complete stat profiles** per tier (not multipliers). Key changes by tier:

| Property | Poor → Gangster → Hitman |
|----------|--------------------------|
| Lock-on range | +5 units per tier (e.g., 25→30→35) |
| Accuracy | +25–50% per tier (e.g., 0.75→1.0→1.25) |
| Move speed while aiming | +0.0 to +0.5 total (e.g., 1.0→1.2→1.5) |
| Damage | Constant (exception: Deagle doubles at Gangster) |
| Clip size | Doubles for dual-wield weapons at Hitman |
| Animation group | Upgrades (e.g., `pythonbad`→`python`) |

Tier transition is a snap (not interpolation) — player immediately gets all stats of the new tier.

**Skill gain rate (points per hit on damageable target):**

| Weapon | Points/Hit | Hits to Gangster | Hits to Hitman (999) |
|--------|-----------|-----------------|---------------------|
| Pistol | 1 | 40 | 999 |
| Silenced Pistol | 5 | 100 | ~200 |
| Desert Eagle | 3 | ~67 | 333 |
| Shotguns | 0.4–0.5 | 400–500 | ~2000 |
| Micro SMG / Tec-9 | 0.4 | 125 | ~2500 |
| MP5 | 1.5 | ~167 | 666 |
| AK-47 | 3 | ~67 | 333 |
| M4 | 2 | 100 | ~500 |

### Lock-On Range by Weapon and Tier (game units ≈ meters)

| Weapon | Poor | Gangster | Hitman |
|--------|------|----------|--------|
| Pistols (all) | 25 | 30 | 35 |
| Shotgun / Sawnoff | 25–30 | 30–35 | 35–40 |
| Combat Shotgun | 35 | 40 | 40 |
| Micro SMG / Tec-9 | 25 | 30 | 35 |
| MP5 | 35 | 40 | 45 |
| AK-47 / M4 | 40 | 45 | 50 |
| Minigun | 65 | — | — |

### Lock-On / Targeting System

- Auto-aim cone: ~90° frontal arc
- Locks closest hostile within cone
- Reticle color: Green (healthy) → Orange (damaged) → Red (critical) → Black (dead)
- Flick right stick or press L2/R2 to cycle targets
- Rifles, snipers, heavy weapons, and thrown weapons CANNOT lock on
- Free-aim fallback if no target in range
- **Dual-wielding disables lock-on** — must free-aim

### Drive-By Mechanics

- **Driver:** SMGs only (Micro SMG, Tec-9, MP5); fire out left or right window; cannot aim forward/behind
- **Passenger:** Wider aiming arc; can use pistols, SMGs, shotguns, assault rifles
- **Motorcycle:** One-handed SMG; more flexible aiming than car drive-by
- **AI gang members:** Perform drive-bys automatically during gang warfare

---

## 4. Stealth

### Entering Stealth

- Toggle crouch to enter stealth mode
- Visual indicator: minimap blip turns blue (hidden)
- Movement is silent while crouched
- Running/sprinting generates noise and breaks stealth

### Knife Stealth Kill

1. Equip Knife (only weapon that enables stealth kills)
2. Enter crouch
3. Approach target from **directly behind**
4. Lock on to target
5. Visual cue: CJ raises knife arm above head (indicates kill range)
6. Attack button → throat-slit animation → instant kill, completely silent

Does NOT alert nearby enemies unless they have direct line of sight to the kill.

### Detection System

- **Basis:** Line-of-sight + noise + approach direction (front vs. back)
- **Detection cone:** ~120° frontal arc (derived from pedstats.dat Heading Change Rate)
- **Detection range:** ~15–20 game units for alert enemies
- **Lighting:** Simple zone check ("in shadow" vs. "not in shadow"); not dynamic light calculation
- **Time to detect:** Near-instantaneous if in frontal cone and within range (no gradual awareness meter)
- **Once detected:** Enemy enters combat state; cannot be stealth-killed
- **Gunfire:** Alerts all NPCs in radius regardless of facing direction
- **Silenced 9mm:** Quieter but not perfectly silent; very close NPCs (~5 units) may still hear

### Noise Meter (Burglar Missions Only)

- Visible bar on HUD during house burglary
- Fills based on movement speed and bumping furniture
- If full: resident wakes → 10 s to flee before 3-star wanted level
- NOT present in general gameplay stealth

---

## 5. Vehicles

### Types (212 total)

| Category | Count | Subtypes |
|----------|-------|----------|
| Cars | ~100+ | Sports (15), 2-door (18), 4-door (21), Lowriders (8), Tuners (6), 4WD (11), Vans (8), Service (14), Industrial (16), Emergency (18), Fun (14) |
| Motorcycles | 10 | Sport bikes, choppers, dirt bikes |
| Bicycles | 3 | BMX, Mountain Bike, Bike |
| Boats | 11 | Speedboats, sailboats, dinghies |
| Planes | 11 | Fixed-wing aircraft |
| Helicopters | 9 | Civilian + military |
| RC Vehicles | 6 | Toy cars/planes |
| Trains | 3 | Freight, Brown Streak, Tram |
| Trailers | 6 | Towable |

### Vehicle Health & Damage Model

| Health Range | Engine State | Visual |
|-------------|-------------|--------|
| 1000–651 | Normal | No visible damage |
| 650–550 | Overheating | White smoke from engine |
| 549–390 | Severe overheating | Grey smoke |
| 389–250 | Critical | Black smoke + audible clunking |
| 249–1 | **On fire** | Engine fire → explosion in ~5 seconds |
| 0 | Destroyed | Instant explosion |

**Key damage model rules:**
- Body panels (doors, hoods, trunks) dangle before detaching
- Bumpers separate from body
- Headlights break from frontal damage
- Chassis mesh does NOT soft-deform (no crumpling)
- Front quarter panels are NOT removable
- Doors can be "closed shut" by swaying vehicle side-to-side (makes them harder to detach)
- Cars do NOT lose performance from damage (speed/handling unchanged until destroyed)
- **Exception:** Fixed-wing aircraft lose performance with damage; flaps dangle

**Collision damage formula:**
```
healthLoss = impactForce × fCollisionDamageMultiplier
```
Where `fCollisionDamageMultiplier` ranges 0.0–10.0 per vehicle (defined in handling.cfg):
- Infernus (sports car): 0.72
- Tampa (sedan): 0.52
- NRG-500 (motorcycle): 0.15
- Rhino (tank): 0.09 (nearly indestructible)

### Vehicle Handling System (handling.cfg)

Each vehicle is defined by 34 parameters in handling.cfg. Key parameters:

| Parameter | Type | Example (Infernus) | Description |
|-----------|------|-------------------|-------------|
| fMass | kg | 1400.0 | Vehicle mass |
| fTurnMass | float | 2725.3 | Rotational inertia (higher = resists spinning) |
| fDragMult | float | 1.1 | Air resistance |
| CentreOfMass x/y/z | m | 0.0, 0.0, -0.25 | CoM offset from geometric center |
| nPercentSubmerged | % | 70 | How deep before floating |
| fTractionMultiplier | float | 0.89 | Tire grip multiplier |
| fTractionLoss | float | 0.9 | Minimum grip when skidding |
| fTractionBias | 0–1 | 0.50 | Front-to-rear grip ratio |
| nNumberOfGears | int | 5 | Gear count |
| fMaxVelocity | km/h | 240 | Top speed cap |
| fEngineAcceleration | m/s² | 70.0 | Engine power |
| fEngineInertia | 0–50 | 2.0 | Throttle response (higher = slower) |
| nDriveType | char | 4 | F=front, R=rear, 4=AWD |
| nEngineType | char | P | P=petrol, D=diesel, E=electric |
| fBrakeDeceleration | m/s² | 11.0 | Braking force |
| fBrakeBias | 0–1 | 0.51 | Front-to-rear brake ratio |
| fSteeringLock | deg | 30.0 | Max steering angle |
| fSuspensionForceLevel | float | 1.2 | Spring stiffness |
| fSuspensionDampingLevel | float | 0.19 | Damping |
| fCollisionDamageMultiplier | 0–10 | 0.72 | Damage received from collisions |

**Representative vehicles:**

| Vehicle | Mass (kg) | Top Speed | Accel | Drive | Brake | Steering | DmgMult |
|---------|-----------|-----------|-------|-------|-------|----------|---------|
| Infernus | 1400 | 240 km/h | 70.0 | AWD | 11.0 | 30° | 0.72 |
| Tampa | 1700 | 160 km/h | 24.0 | RWD | 8.17 | 35° | 0.52 |
| Rhino | 25000 | 60 km/h | 15.0 | AWD | 5.0 | 35° | 0.09 |
| NRG-500 | 400* | 191 km/h | 75.0 | RWD | 35.0 | 35° | 0.15 |

*NRG-500 mass in handling.cfg is 40000 (game scaling; effective mass is different for bikes)

### Motorcycle-Specific Physics (! section in handling.cfg)

| Parameter | NRG-500 | Description |
|-----------|---------|-------------|
| MaxLean | 55.0° | Maximum lean angle from vertical |
| FullAnimLean | 38.0° | Max visual lean for rider model |
| WheelieAng | -35.0° | Angle threshold for wheelie physics |
| StoppieAng | 30.0° | Angle threshold for stoppie physics |
| WheelieStabMult | 0.7 | Stabilizing counter-force during wheelie |
| StoppieStabMult | 0.6 | Stabilizing counter-force during stoppie |
| DesLean | 0.95 | Lean speed (lower = faster response) |

**Fall-off conditions:**
- Low Bike Skill: minor collisions eject rider
- High-speed head-on collisions regardless of skill
- Exceeding WheelieAng balance point
- At MAX skill: only extreme impacts dismount
- Bike Skill scales the collision-force threshold for ejection

**Wheelie rewards:** trigger after 5+ seconds on rear wheel (distance measured in feet × 0.06)
**Stoppie rewards:** trigger after 2+ seconds on front wheel

### Bicycle Physics

| Property | Value |
|----------|-------|
| Types | BMX (tricks), Mountain Bike (fastest, hill climbing), Bike (middle) |
| Pedaling | Tap sprint to pedal faster; stamina drains during sprint-pedaling |
| Bunny hop | Hold secondary fire → release to jump; height scales with Cycling Skill (0–100%) |
| Super bunny hop (glitch) | Release hop button → immediately press fire → launches ~12.5 m high |
| Wheelie speed boost | Performing a wheelie increases top speed beyond normal pedaling |
| Damage immunity | Bicycles cannot be destroyed (except upside-down fire glitch) |
| Stamina drain | Sprint-pedaling uses same stamina bar as on-foot sprinting |

### Nitrous Oxide

| Property | Details |
|----------|---------|
| Tank sizes | 2x ($200), 5x ($500), 10x ($1,000) — number of boosts before depletion |
| Effect | Boosts acceleration/torque; does NOT increase top speed |
| Indicator | Tank-shaped bar near radar |
| Refill | Auto-refills over time; blue exhaust fumes = refilling; normal smoke = ready |
| Ready threshold | Usable once bar exceeds ~45% |
| Police trigger | Using nitrous within sight of police → 1-star wanted |

### Tire Puncture

- Tires can be shot out by gunfire
- If ANY tire hits a spike strip: ALL tires pop simultaneously
- Flat tires: severe handling degradation and loss of control
- Severely damaged vehicle hitting strips may instantly combust
- Bicycles are immune to tire puncture

### Vehicle Fire Behavior

- Engine fire starts below 250 HP
- Fire-to-explosion timer: ~5 seconds (frame-rate dependent)
- External fire (Molotov, Flamethrower) can ignite without reaching 250 HP threshold
- Some vehicles explode twice (residual fire triggers second smaller explosion)
- Shooting fuel cap can trigger fire

### Vehicle Customization (Mod Garages)

| Garage | Locations | Specialization | Vehicle Count |
|--------|-----------|----------------|---------------|
| TransFender | 3 (LS, SF, LV) | General cars | ~65 models |
| Loco Low Co. | 1 (Willowfield, LS) | Lowriders | 8 models |
| Wheel Arch Angels | 1 (Ocean Flats, SF) | Tuners | 6 models |

**TransFender modifications & prices:**
- Paint: 63 colors, $150/color (primary + secondary)
- Wheels: 10 designs, $620–$1,560
- Exhausts: $150–$350 (Small/Medium/Large/Twin/Upswept)
- Hoods: $100–$250 (Fury/Champ/Worx/Race Scoop)
- Hood Vents: $100–$150 (Oval/Square)
- Spoilers: 8 styles, $200–$550
- Side Skirts: $500
- Fog Lamps (off-road only): $50–$100
- Nitrous: $200/$500/$1,000 (2x/5x/10x)
- Hydraulics: $1,500
- Bass Boost: $100
- LV location charges 20% premium (except paint)

**Unlock conditions:**
- TransFender + Loco Low Co.: after "Cesar Vialpando" mission
- Wheel Arch Angels: after "Zeroing In" mission

### Special Vehicles

**Trains:**
- Controls: throttle (accelerate) and brake only; no steering
- Locked to rails; cannot derail at normal speeds
- Derails above ~190 km/h (speed value ~50) on curves
- Safe operating speed: 45–50 on in-game speedometer
- Only cinematic camera available while driving
- Freight Train Challenge: stop at stations within time limits (2 levels, $50,000 reward)

**Combine Harvester:**
- Running over pedestrians: rear chute ejects body parts
- Door locks when player driving (police cannot bust)
- Found at Blueberry Acres after "Body Harvest" mission

**Parachute:**
- Auto-added to inventory when exiting aircraft at sufficient altitude
- Manual deploy required (press fire button); brief freefall period before deploy possible
- Controls: forward = faster descent, back = slow/flare, left/right = steer
- Landing: pull back = running landing on feet; push forward = faceplant with potential injury/death
- Failure to deploy = lethal fall damage
- Single-use; removed from inventory after landing

**Aircraft controls:**
- Planes: throttle, pitch, roll, rudder, retractable landing gear
- Helicopters: collective (altitude), cyclic (pitch/roll), tail rotor (yaw)
- Fixed-wing performance degrades with damage

### Driving Skill

| Property | Details |
|----------|---------|
| Gain rate | ~1% per 5 min driving any 4-wheeled vehicle; Driving School medals boost |
| NOT gained from | Motorcycles or bicycles (separate skills) |
| Effects | Better airborne control, reduced skidding, easier wheelies |
| Unlock | At 20%: stadium events (8-Track, Blood Ring) |

### Flying Skill

| Property | Details |
|----------|---------|
| Gain rate | Increases by flying any aircraft; Pilot School provides large boosts |
| Effects | Reduces turbulence/shaking, improves control responsiveness |
| Pilot School rewards | Bronze: Rustler + Pilot's License; Silver: Stunt Plane; Gold: Hunter |
| Pilot's License | Grants legal airport entry |

### Cycling Skill

| Property | Details |
|----------|---------|
| Gain rate | Fastest of all vehicle skills; gained by riding bicycles or gym exercise bike |
| Effects | Higher bunny-hop height, faster top speed, less chance of falling off, more responsive steering |

---

## 6. RPG Stats & Progression

### Health & Armor

| Property | Default | Maximum | How to Increase |
|----------|---------|---------|-----------------|
| Max Health | 100 | 176 | Complete Paramedic Level 12 |
| Max Armor | 100 | 150 | Complete Vigilante Level 12 |

- Current health: CPed offset +0x540 (float)
- Max health: CPed offset +0x544 (float)
- Current armor: CPed offset +0x548 (float)
- Armor absorbs gunfire and explosive damage but NOT fall damage
- **No passive health regeneration** — must eat, save, use health pickups, or visit prostitutes

### All Stats — Detailed Gain/Loss Rates

#### Stamina (0–1000)

| Activity | Gain Rate |
|----------|-----------|
| Running or cycling | +50 pts / 300 s |
| Swimming | +50 pts / 150 s |
| Gym treadmill or stationary bike | +40 pts / 14 s |
| **Daily cap** | **+200 pts (20%)** |

Never decreases. Infinite stamina after $10,000 Burglar earnings.

#### Muscle (0–1000)

| Activity | Gain Rate |
|----------|-----------|
| Gym: weight lifting (bench press, dumbbells) | +10 pts per repetition (~1%) |
| Gym: treadmill/stationary bike | +10 pts / 14 s |
| Running | +10 pts / 150 s |
| Swimming or cycling | +10 pts / 100 s |
| **Daily cap** | **+200 pts (20%) at gym** |

Lost by: starvation when fat = 0 (muscle consumed instead of fat).

Effects: melee damage multiplier, +respect, model change at 500 pts (50%), walk/run animation change.

#### Fat (0–1000, starts at 200)

**Gain sources:**

| Source | Fat Gain |
|--------|----------|
| Large meal (Burger Shot/Cluckin' Bell/Pizza) | +30–35 pts (3–3.5%) |
| Medium meal | +20 pts (2%) |
| Small meal | +10 pts (1%) |
| Salad/healthy option | 0 |
| Sprunk vending machine | +2.5 pts (0.25%) |
| Hotdog/noodle vendor | +5 pts (0.5%) |

**Loss sources:**

| Activity | Fat Loss Rate |
|----------|--------------|
| Running | -15 pts / 150 s |
| Cycling or swimming | -15 pts / 100 s |
| Gym treadmill/bike | -25 pts / 14 s |
| Gym weights | -25 pts per repetition |
| Starvation | -25 pts per in-game hour |
| **Daily cap for gym loss** | **-400 pts (40%)** |

Effects: `FatSprint`/`FatWalk` animation groups (slower), reduced jump height, NPC comments.

**Overeating:** 12+ meals within 6 game-hours → CJ vomits → all fat from those meals removed.

#### Hunger Timer

- Must eat at least once every **72 in-game hours** (~48 real minutes)
- Starvation progression: fat consumed at -25 pts/game-hour → when fat = 0, muscle consumed → when muscle = 0, health drains → death

#### Lung Capacity (0–1000)

- Gain: +50 pts / 60 s underwater
- Never decreases
- All 50 Oysters: maximizes permanently

#### Weapon Skills (0–999 per weapon)

- Gain: see Section 3 table (points per hit)
- Never decreases
- Thresholds: 0 = Poor, 40–500 = Gangster (varies), 999 = Hitman

#### Driving / Flying / Cycling / Bike Skill (0–1000 each)

- Driving: ~10 pts / 300 s of driving 4-wheeled vehicles
- Flying: similar rate; Pilot School = large boost
- Cycling: fastest gain of all vehicle skills
- Bike (motorcycle): gained by riding motorcycles
- None decrease

#### Gambling Skill (0–1000)

- Gain: 1 pt per $100 spent at casinos
- Tiers: Gambler (default, $1K max bet) → Professional → Hi-Roller ($100K) → Whale ($1M)

### Respect (0–100%)

**Composition (maximum contributions):**

| Component | Max % |
|-----------|-------|
| Running Respect (actions) | 40% |
| Mission advancement | 36% |
| Territories owned | 6% |
| Money accumulated | 6% |
| Physical fitness (muscle) | 4% |
| Girlfriend progress | 4% |
| Appearance (clothes/tattoos) | 4% |

**Action values:**

| Action | Respect Change |
|--------|---------------|
| Territory won (gang war) | +30% running respect |
| Territory lost (failed defense) | -3% |
| Kill rival gang member (outside war) | +0.5% |
| Kill drug dealer | +0.005% |
| Kill own gang member | -0.005% |
| Own gang member dies | -2.0% |

**Recruitment thresholds:**

| Respect | Max Recruits |
|---------|-------------|
| >1% | 2 |
| >10% | 3 |
| >20% | 4 |
| >40% | 5 |
| >60% | 6 |
| >80% | 7 |

### Sex Appeal (Composite)

- 50% from clothing/appearance (expensive = higher)
- 50% from last vehicle CJ used (sports cars and lowriders = highest; must be undamaged; effect persists within 120 ft after exiting)
- Moderate muscle helps; 100% muscle penalizes
- Collecting all 50 Oysters bypasses sex appeal requirements for all girlfriends

### Body/Physique Model Morphing

| State | Visual Change | Gameplay |
|-------|--------------|----------|
| Fat > 500 | Obese model, round face, thick limbs | `FatSprint`/`FatWalk` anims (slower), lower jump |
| Muscle > 500 | Buff model, defined arms, wider shoulders | Walk/run animation change, higher melee damage |
| Both > 500 | Large/bulky build | Mixed effects |
| Both < 200 | Skinny/default | Normal speed, base damage |

---

## 7. Clothing & Appearance

### Equipment Slots (7)

1. Torso (shirts, jackets, tank tops, hoodies)
2. Legs (pants, shorts, jeans)
3. Shoes (sneakers, boots, dress shoes)
4. Watch (wristwatches)
5. Chain/Necklace (jewelry)
6. Glasses/Mask (glasses, bandanas, masks)
7. Hat (caps, beanies, cowboy hats)

### Clothing Stores (6)

| Store | Price Range | Stat Contribution | Notes |
|-------|-------------|-------------------|-------|
| Binco | Cheap | Lowest respect/sex appeal | Budget/gang-style |
| Sub Urban | Moderate | Moderate | Casual/streetwear |
| ZIP | Moderate | Moderate | Athletic/casual |
| ProLaps | Expensive | High | Designer sportswear |
| Victim | Expensive | High | Fashion/trendy |
| Didier Sachs | Most expensive | Highest | Luxury suits |

Clothing accounts for ~4% of total respect and ~50% of sex appeal.

### Special Outfits

- Awarded through missions and 100% girlfriend relationships
- Replace torso + legs + shoes simultaneously
- Hats, watches, chains, glasses remain visible on top

### Tattoos

- 4 tattoo parlors across the map
- 7 body areas: upper-left arm, upper-right arm, lower-left arm, lower-right arm, back, chest, stomach
- Each provides up to +3% respect OR +3% sex appeal (not both)
- Permanent; replaceable with different design but not removable
- Prices: $50–$500 per tattoo

### Haircuts

- 7 barber shops
- Different styles affect both respect and sex appeal
- Prices: $10–$500
- Facial hair options available at certain barbers
- First barber (Reece's, Idlewood) unlocked after "Ryder" mission

---

## 8. Gang Territory System

### Overview

- 53 contestable territories in Los Santos
- Three gangs: Grove Street Families (green), Ballas (purple), Los Santos Vagos (yellow)
- Unlocked after mission "Doberman"; disabled between "The Green Sabre" and "Home Coming"
- Map shading: darker shade = more heavily defended territory

### Taking Territory (Offensive War)

**Trigger:** Kill 3 enemy gang members on foot in their territory within a short time window.

**3 waves with ~10 s between waves:**

| Wave | Enemies | Weapons | Notes |
|------|---------|---------|-------|
| 1 | 6–8 | Baseball Bats, Pistols, Micro SMGs | Health pickups spawn immediately |
| 2 | 8–10 | SMGs, AK-47s | Armor pickups spawn after Wave 1 cleared |
| 3 | 10–12 | Primarily AK-47s | Darker territories: gang car with 2 armed enemies may appear |

- Pickups despawn 60 s after war ends
- Enemies spawn outside player's field of vision at fixed distance
- Camera direction manipulation controls spawn location

**Victory:** Territory turns green; +30% running respect.

### Defending Territory

- Random attacks on GSF turf; territory flashes red on map; flag icon on radar
- Only **1 wave** of 8–12 enemies (shotguns and mixed weapons)
- Can use vehicles (Rhino effective)
- Failure: territory reverts to enemy; -3% running respect

### Difficulty Scaling

- Territory darkness on map directly correlates with difficulty
- Darker = more gang members present = harder waves
- Darker territories may spawn armed gang cars during waves
- Lightly-shaded territories have fewer members (harder to trigger war but easier to win)

### Rewards

- +30% running respect per territory won
- Money pickup spawns at Johnson House (proportional to total territories held)
- 6% of total respect stat comes from territory ownership (proportional)
- **19+ territories required** to unlock final mission "End of the Line"
- **All 53 territories:** permanently removes Ballas and Vagos from city

### Gang Recruitment

- Target a GSF member + press recruit button
- Max recruits based on respect level (see Section 6)
- Commands: follow CJ (default) or hold position
- Recruited members: attack enemies, engage police, auto drive-by from vehicles

---

## 9. Wanted Level System

### Chaos Points System

Wanted level is driven by an internal "chaos points" counter (from `CWanted.cpp`):

| Stars | Chaos Threshold | Max Cops | Max Cop Cars | Roadblock Chance | Helicopters |
|-------|----------------|----------|--------------|-----------------|-------------|
| 0 | <50 | 0 | 0 | 0% | 0 |
| 1 | 50 | 1 | 1 | 0% | 0 |
| 2 | 180 | 3 | 2 | 0% | 0 |
| 3 | 550 | 4 | 2 | 12% | 1 |
| 4 | 1200 | 6 | 2 | 18% | 2 |
| 5 | 2400 | 8 | 3 | 24% | 2 |
| 6 | 4600 | 10 | 3 | 30% | 2 |

Maximum simultaneous cops capped at 10 (`m_pCopsInPursuit[10]`).

### Crime Point Values

| Crime | Points | Notes |
|-------|--------|-------|
| Damage ped / speeding | 5 | |
| Run red light | 10 | |
| Car steal | 15 | |
| Kill ped with car | 18 | |
| Set ped/car on fire | 20 | |
| Explosion | 25 | |
| Damage car (property) | 30 | |
| Stab ped | 35 | |
| Damage cop (melee) | 45 | |
| Destroy vehicle | 70 | |
| Damage cop (firearms) / Kill cop with car | 80 | |
| Stab cop | 100 | |
| Destroy helicopter/plane | 400 | |
| Aim gun at cop | 2 | |

**Proximity modifier:** Committing a crime within **14 units** of a cop **doubles** the chaos points.

### Police Behavior by Star Level

| Stars | Units | Behavior |
|-------|-------|----------|
| 1 | Patrol cops | Attempt arrest; only shoot if CJ holds a gun; 1 car |
| 2 | Armed police | Shoot to kill; aggressive PIT maneuvers; 2 cars |
| 3 | + Helicopter | Roadblocks placed; helicopter tracks from above; 2 cars + heli |
| 4 | SWAT | Armored Enforcers; SWAT rappel from helicopter (up to 4); body armor |
| 5 | FBI | 4-agent SUVs replace all local police; streets clear of civilians |
| 6 | Military | Barracks trucks + Rhino tanks; tanks = instant kill on collision; M4 rifles |

### Helicopter Behavior

- Spawn at 3+ stars
- Weapon: invisible autocannon (AK/M4 sound)
- Must maintain spotlight lock on player for several seconds before firing
- Constantly changing direction prevents targeting
- At 4+ stars: drops up to 4 rappelling SWAT officers
- Top speed: ~200 km/h
- Count: 1 at 3 stars, 2 at 4–6 stars

### Evasion Mechanics

| Parameter | Value |
|-----------|-------|
| Police detection radius | 18.0 game units |
| Chaos decay rate (0–1 star, urban) | -1 point/second when no police within 18 units |
| Chaos decay rate (rural) | -2 points/second |
| Crime queue timeout | 10,000 ms |
| Crime report delay | 500 ms between reports |

At 2+ stars: natural chaos decay alone cannot reduce stars — must use Pay 'n' Spray, bribes, etc.

### Star Availability by Story Progress

- 1–4 stars: from game start
- 5 stars: unlocked after "The Green Sabre" (San Fierro access)
- 6 stars: unlocked after "Yay Ka-Boom-Boom" (Las Venturas access)
- Entering locked regions: automatic 4-star wanted level

### Methods to Lose Wanted Level

| Method | Effect | Caveat |
|--------|--------|--------|
| Pay 'n' Spray | Clears all stars | Stars flash after use; crime during flash = full reinstatement |
| Police Bribes (pickup) | -1 star (sets chaos to 20 above next lower threshold) | Scattered around map |
| Safehouse save | Clears all | Must enter and save |
| Clothing change | Clears all | At any wardrobe/clothing shop; same flash caveat |
| Mod garage | Clears all | Any vehicle modification shop |
| Evasion | Stars flash then clear | Chaos must decay below threshold (only works at 0–1 star naturally) |
| Wasted | Clears all | Respawn at hospital; lose ALL weapons + $100 |
| Busted | Clears all | Respawn at police station; lose ALL weapons + $100–$1,500 |

### Death/Arrest Penalties

| Event | Money Lost | Weapons Lost | Respawn | Exception |
|-------|-----------|--------------|---------|-----------|
| Wasted (death) | $100 | All | Nearest hospital | Katie Zhan girlfriend: keep weapons, no fee |
| Busted (arrest) | $100–$1,500 | All | Nearest police station | Barbara girlfriend: keep weapons, no fee |

---

## 10. World Structure

### Geography

| Region | Based On | Key Features |
|--------|---------|--------------|
| Los Santos | Los Angeles | Gang neighborhoods, affluent hills, industrial zones |
| San Fierro | San Francisco | Hilly terrain, Chinatown, waterfront, Garver Bridge |
| Las Venturas | Las Vegas | Casinos, neon strip, desert outskirts |
| Red County | Rural California | Farms, small towns (Palomino Creek, Blueberry) |
| Flint County | Northern California | Forests, rural highways |
| Whetstone | Sierra Nevada | Mount Chiliad, mountainous |
| Tierra Robada | Central California | Semi-arid, between SF and LV |
| Bone County | Nevada desert | Area 69 military base, ghost towns |

Total map area: ~36 km² (4× GTA Vice City, 4.4× GTA III)

### Zone Locks (Story Progression)

| Phase | Accessible Regions | Trigger Mission |
|-------|-------------------|-----------------|
| Act 1 | Los Santos only | Game start |
| Act 2 | + Red County, Flint County, Whetstone, San Fierro | "The Green Sabre" |
| Act 3 | + Tierra Robada, Bone County, Las Venturas (full map) | "Yay Ka-Boom-Boom" |

Penalty for early entry: automatic 4-star wanted level (story justification: earthquake damaged bridges).

### Day/Night Cycle

| Property | Value |
|----------|-------|
| Time scale | 1 real second = 1 game minute |
| Full day cycle | 24 real minutes |
| Days of the week | Tracked (affects some events) |
| Time slots for NPC spawning | 12 two-hour intervals per day (from popcycle.dat) |

### Weather System

| Type | Regions | Gameplay Effects |
|------|---------|-----------------|
| Sunny/Clear | All | Default; normal visibility and traffic |
| Cloudy | All | Most frequent overall |
| Rain/Thunderstorm | LS, SF, countryside | Reduced NPC/vehicle density; peds seek shelter |
| Fog | Countryside, SF | Ranges from light to impenetrable; dramatic visibility reduction |
| Sandstorm | Bone County / LV desert ONLY | Near-zero visibility; blows aircraft off course; police helicopters do NOT spawn |

**Weather rules:**
- Rain NEVER occurs in Las Venturas or desert regions
- Sandstorms ONLY in desert/Bone County
- Weather transitions progressively (cloudy → drizzle → rain → thunderstorm)
- Regional weather defined in popcycle.dat

### NPC AI

**Pedestrian behavior (governed by pedstats.dat):**

| Ped Stat | Cops | Gang Members | Civilians |
|----------|------|-------------|-----------|
| Fear (0–100) | 10 | 20 | 50–80 |
| Temper (0–100) | 30 | 65 | 20–40 |
| Lawfulness (0–100) | 100 | 10 | 50–80 |
| Flee Distance | 20.0 | 17.0 | varies |
| Shooting Rate | high | medium | low |

**Decision Makers:** Event-response probability tables (weighted random) determine NPC reactions to stimuli. Each event has up to 6 possible tasks (flee, scream, attack, call police, etc.).

**Traffic AI:**
- Density varies by time of day and zone type (from popcycle.dat)
- Different vehicle types spawn in appropriate neighborhoods
- NPC drivers follow path nodes; no explicit speed limits enforced
- Panic mode after being attacked (erratic high-speed driving)
- Known "Deadly Drivers" behavior (NPCs swerve into player, more prominent on PC)

**Gang member AI:**
- Appear in groups in color-coded territories
- Give verbal warnings before becoming hostile
- Draw weapons and hold at side before firing
- Attack on sight if player enters rival territory
- High Temper (65) + low Lawfulness (10) = low provocation threshold

---

## 11. Side Activities & Minigames

### Gambling (Las Venturas Only)

| Game | Mechanics | Bet Range (at max skill) |
|------|-----------|--------------------------|
| Blackjack | Standard 21; hit/stand/double/split | Up to $1,000,000 |
| Video Poker | 5-card draw; hold/exchange | Machine-dependent |
| Roulette | Numbers, colors, odd/even, high/low | Table-dependent |
| Slots | Pure luck; pull handle | Coin-based |
| Horse Racing | Pick horse + bet; odds determine payout | Variable |

Gambling Skill: 1 pt per $100 spent; max 1000; tiers: Gambler ($1K) → Professional ($10K) → Hi-Roller ($100K) → Whale ($1M max bet)

### Vehicle Missions

| Mission | Vehicle | Completion | Cash Reward | Gameplay Reward |
|---------|---------|-----------|-------------|-----------------|
| Vigilante | Police vehicles, Rhino, Hunter | Level 12 (78 criminals) | $32,500 | +50% max armor (150 total) |
| Paramedic | Ambulance | Level 12 (78 saves) | $37,500 | +50% max health (176 total) |
| Firefighter | Firetruck | Level 12 (78 fires) | $45,900 | Fireproof on foot |
| Taxi | Taxi/Cabbie | 50 fares | Up to $27,500 | Nitrous + jump for all taxis |
| Pimping | Broadway | 10 levels | Variable | Prostitutes pay CJ |
| Freight Train | Brown Streak/Freight | 2 levels (10 deliveries) | $50,000 | — |

### Sports & Rhythm Games

- **Pool:** 8-ball rules; physics-based cue mechanics; bet money against opponents in bars
- **Basketball:** 2-on-2 at court outside Sweet's house
- **Dancing:** Arrow-based rhythm game at dance clubs; match directional inputs to music timing
- **Lowrider Challenge:** Hydraulic bounce-to-beat rhythm game; time hydraulic jumps to music; at El Corona station

### Burglar Missions

| Property | Value |
|----------|-------|
| Hours | 20:00–06:00 only |
| Vehicle | Black Boxville van (special) |
| HUD | Noise meter (fills with movement/furniture bumps) |
| Detection | Noise bar full → resident wakes → 10 s to flee → 3-star wanted |
| Reward formula | $20 × n² (n = items stolen per session) |
| Milestone | $10,000 cumulative → infinite sprint/swim/cycle stamina + $3,000 bonus |

### Races

| Category | Location | Count | Notes |
|----------|----------|-------|-------|
| LS Street Races | Little Mexico | ~10 | |
| SF Street Races | Behind Wang Cars | ~7 | |
| LV Street Races | LVA Freight Depot | ~4 | |
| Air Races | LV Airport | 6 | Checkpoints only, no opponents or timer |
| Mount Chiliad | Whetstone | 3 | Bicycle downhill |
| **Total** | | ~25–30 | |

- Stadium events: Bloodring (demolition derby), 8-Track (stock cars), Dirt Track (ATVs) — unlock at 20% Driving Skill
- Triathlons ("Beat the Cock!"): multi-stage run/swim/cycle races

### Other Activities

- **Ammu-Nation Shooting Range:** Timed target challenges; awards weapon skill points
- **BMX Challenge:** Glen Park skate park; collect 19 coronas before timer expires
- **NRG-500 Challenge:** Motorcycle checkpoint course
- **Gym workouts:** Treadmill (stamina), bench press/weights (muscle), exercise bike (stamina + cycling skill)
- **Schools:** See Section 16

---

## 12. Property System

### Safehouses (37 total)

| Type | Count | Price Range | Total Cost |
|------|-------|-------------|------------|
| Free (story-unlocked) | 8 | — | — |
| Purchasable | 29 | $6,000–$120,000 | $879,000 |

All provide: save point, wardrobe (clothing change), garage (vehicle storage).

### Asset Businesses (10)

| Asset | Cost | Daily Income | Unlock Condition |
|-------|------|--------------|------------------|
| Verdant Meadows Airfield | $80,000 | $10,000 | Complete "Green Goo" |
| Wang Cars | $50,000 | $8,000 | Complete "Puncture Wounds" |
| Zero RC Shop | $30,000 | $5,000 | Complete "New Model Army" |
| Johnson House | Free | $10,000 | Complete "Doberman"; scales with territories |
| RS Haul | Free | $2,000 | Complete Trucking missions |
| Vank Hoff Hotel | Free | $2,000 | Complete Valet Parking ("555 We Tip") |
| Hunter Quarry | Free | $2,000 | Complete Quarry missions ("Explosive Situation") |
| Roboi's Food Mart | Free | $2,000 | Complete LS Courier |
| Hippy Shopper | Free | $2,000 | Complete SF Courier |
| Burger Shot | Free | $2,000 | Complete LV Courier |

**Income mechanics:**
- Accumulates per in-game day
- Each asset has a daily cap (value above); income stops at cap until collected
- Collection: walk into rotating dollar sign outside business
- Does NOT accumulate beyond cap across multiple days
- Total max daily passive income: ~$45,000/day (all assets)
- All assets required for 100% completion

---

## 13. Collectibles

| Type | Count | Region | Per-Item | Completion Reward |
|------|-------|--------|----------|-------------------|
| Spray Tags | 100 | Los Santos | — | AK-47, Sawn-Off, TEC-9, Molotov spawn at Johnson House; GSF gang upgrades (Desert Eagles, SMGs, Knives) |
| Photo Ops | 50 | San Fierro | — | $100,000 + Micro Uzi, Grenades, Pump Shotgun, Sniper Rifle at Doherty Garage |
| Horseshoes | 50 | Las Venturas | $100 each | $100,000 + SMG, Satchel Charges, Combat Shotgun, M4 at Four Dragons Casino; +luck (gambling) |
| Oysters | 50 | Entire map (underwater) | — | $100,000 + max lung capacity + bypass all sex appeal requirements |
| Unique Stunt Jumps | 70 | Entire map | — | — |

---

## 14. Mission System

### Scale

- ~100 story missions (86 mandatory + 14 optional)
- Longest GTA campaign at time of release

### Mission Givers & Chains

| Giver | Location | ~Missions | Focus |
|-------|----------|-----------|-------|
| Sweet Johnson | Grove Street, LS | 8 | Gang territory, family loyalty |
| Big Smoke | Idlewood, LS | 5 | Drug trade (secretly) |
| Ryder | Grove Street, LS | 4 | Criminal errands |
| OG Loc | Burger Shot, LS | 4 | Rap career/comedy |
| C.R.A.S.H. (Tenpenny) | Various | 6 | Corrupt police errands |
| Cesar Vialpando | Various | 5 | Lowriders, vehicle theft |
| Catalina | Countryside | 4 | Robbery spree |
| The Truth | Countryside | 2 | Counterculture |
| Wu Zi Mu (Woozie) | SF / LV | 10+ | Triads, casino heist |
| Jizzy/T-Bone/Toreno | SF / Desert | 8+ | Syndicate, then CIA ops |
| Zero | San Fierro | 3 (optional) | RC vehicle challenges |
| Wang Cars | San Fierro | 5 (optional) | Car theft for dealership |

### Structure

- Triggered by walking into colored map markers at mission-giver locations
- Each giver has a unique letter icon (S = Sweet, BS = Big Smoke, C = Cesar, etc.)
- Mission failure returns to open world; must drive back to marker to retry
- **Original (2004):** No mid-mission checkpoints; unskippable cutscenes
- **Definitive Edition (2021):** Some checkpoints added + "Trip Skip" for long drives (inconsistent)

### Mission Types

- On-foot combat (gang fights, building assaults)
- Driving/chasing (pursuits, escapes, deliveries)
- Stealth infiltration (Madd Dogg's Rhymes, Black Project)
- Flying (crop dusting, bombing runs, jet theft)
- RC vehicles (Zero's missions)
- Escort/protect (guarding NPCs/vehicles)
- Timed objectives (deliveries, target kills before escape)
- Scripted setpieces (casino heist, final riot chase)
- Multi-stage (shift between combat and driving mid-mission)

### Mission Rewards (Representative Cash Values)

| Phase | Typical Range | Notable Maximums |
|-------|--------------|-----------------|
| Los Santos | $0–$500 (most = Respect only) | "High Stakes, Low Rider": $1,000 |
| Countryside | $1,000–$10,000 | "Small Town Bank": $10,000 |
| San Fierro | $3,000–$25,000 | "Yay Ka-Boom-Boom": $25,000 |
| Desert | $1,000–$20,000 | "Stowaway"/"Green Goo": $20,000 |
| Las Venturas | $5,000–$100,000 | "Breaking The Bank At Caligula's": **$100,000** |
| Return to LS | $0–$250,000 | "End Of The Line": **$250,000** |

### Story Arc (4 Acts)

1. **Los Santos (1992):** CJ returns for mother Beverly's funeral. Grove Street Families in decline. Corrupt CRASH officers (Tenpenny, Pulaski) frame CJ. CJ rebuilds gang with brother Sweet. "The Green Sabre" — Big Smoke and Ryder revealed as traitors (orchestrated Beverly's murder). Sweet shot/arrested; CJ exiled to countryside.

2. **Countryside & San Fierro:** Robbery sprees with Catalina. Meets Wu Zi Mu through street races. Tracks Loco Syndicate drug operation to SF. Systematically dismantles it (kills Jizzy, T-Bone, Ryder). Builds legitimate businesses. Agent Toreno blackmails CJ into CIA jobs for Sweet's prison release.

3. **Desert & Las Venturas:** Toreno's missions (flying, espionage, arms). CJ partners with Woozie's Triads. Elaborate Caligula's Casino heist. Dates Millie for security keycard. Establishes power.

4. **Return to Los Santos:** Sweet released. Reclaim Grove Street from Ballas/Vagos. Tenpenny acquitted → citywide riots (paralleling 1992 LA riots). CJ storms Big Smoke's crack palace, kills him ("I got caught up in the money, the power..."). Tenpenny steals money, flees in fire truck. CJ/Sweet pursue. Tenpenny's truck crashes off bridge into Grove Street; he dies. GSF victorious.

### Phone Call System

| Property | Value |
|----------|-------|
| Call threads | 7 independent threads running simultaneously |
| Check interval | 90–1000 ms per thread |
| High-priority recall delay | 20 seconds |
| Low-priority recall delay | 60 seconds |
| Conditions | Only when not on mission AND not in interior |
| Cancellation | Shooting, entering vehicles, entering interiors |

---

## 15. Girlfriend System

### Six Girlfriends

| Name | Location | Body Requirement | Car (35–50%) | 100% Reward |
|------|----------|-----------------|--------------|-------------|
| Denise Robinson | Ganton, LS | None (story mission) | Dark green Hustler | Pimp Suit |
| Millie Perkins | Prickle Pine, LV | None (story mission) | Pink Club | Casino keycard (at 35%) |
| Helena Wankstein | Red County | Low fat, low muscle | Bandito (+Sadler at 100%) | Rural Clothes |
| Katie Zhan | SF (near golf course) | High muscle | White Romero (hearse) | Medic Outfit + keep weapons on death |
| Michelle Cannes | SF Driving School | Average build | Monster Truck | Racing Suit |
| Barbara Schternvart | El Quebrados | Overweight | Ranger (police SUV) | Cop Outfit + keep weapons on arrest |

### Dating Mechanics

| Property | Value |
|----------|-------|
| Date types | Restaurant, dancing at club, driving around |
| Successful date | +5% relationship |
| Special interaction | +10% relationship |
| Gifts (flowers) | +1% per gift |
| Kiss attempt | +/- 1% depending on progress |
| Car access | At 35–50% relationship |
| Outfit unlock | At 100% relationship |
| Body bypass | Max sex appeal stat OR all 50 Oysters |
| Jealousy | Girlfriends become jealous if CJ dates others; can stalk during dates |
| Story requirement | Only Millie needed (keycard for casino heist at 35%) |
| Alternative | Millie can be killed on first date for keycard (skips dating entirely) |

---

## 16. Schools

| School | Location | Lessons | Medal Thresholds |
|--------|----------|---------|-----------------|
| Driving School | Doherty, SF | 12 | Bronze 70–84%, Silver 85–99%, Gold 100% |
| Pilot School | Verdant Meadows Airstrip | 10 | Same |
| Boat School | Bayside Marina, SF | 5 | Same |
| Bike School | Blackfield, LV | 6 | Same |

### Rewards by Medal

| School | Bronze | Silver | Gold |
|--------|--------|--------|------|
| Driving | Super GT | Bullet | Hotknife |
| Pilot | Rustler + Pilot's License | Stunt Plane | Hunter (attack heli) |
| Boat | Marquis | Squalo | Jetmax |
| Bike | Freeway | FCR-900 | NRG-500 |

**Pilot School lessons:** Takeoff, Land Plane, Circle Airstrip, Circle + Land, Helicopter Takeoff, Land Helicopter, Destroy Targets, Loop-the-Loop, Barrel Roll, Parachute onto Target.

All bronze medals in each school required for 100% completion. Silver/Gold only unlock bonus vehicles.

---

## 17. Import/Export

| Property | Details |
|----------|---------|
| Location | Easter Basin Docks, San Fierro |
| Unlock | After "Customs Fast Track" mission |
| Structure | 3 sequential lists of 10 vehicles each (30 total) |
| Payout | Scales with vehicle condition (damage reduces value) |
| Delivery method | Magnetic crane on cargo ship or drive up ramp |
| List 1 bonus | $50,000 |
| List 2 bonus | $100,000 |
| List 3 bonus | $200,000 |
| Total potential | ~$1.44M (all vehicles in perfect condition + bonuses) |
| Import system | Exported vehicles become purchasable on day-of-week rotation |
| Import price | 80% of maximum export value |

### Vehicle Lists

**List 1:** Admiral, Buffalo, Camper, Feltzer, Infernus, Patriot, Remington, Sanchez, Sentinel, Stretch

**List 2:** Blista Compact, Cheetah, Comet, FCR-900, Rancher, Sabre, Slamvan, Stafford, Stallion, Tanker

**List 3:** Banshee, BF Injection, Blade, Euros, Freeway, Huntley, Journey, Mesa, Super GT, ZR-350

---

## 18. Economy

### Ammu-Nation Prices

| Weapon | Price | Ammo Price |
|--------|-------|-----------|
| 9mm Pistol | $200 | — |
| Silenced 9mm | $600 | — |
| Desert Eagle | $1,200 | — |
| Tec-9 | $300 | — |
| Micro SMG | $500 | — |
| SMG (MP5) | $2,000 | — |
| Shotgun | $600 | — |
| Sawn-Off | $800 | — |
| Combat Shotgun | $1,000 | — |
| AK-47 | $3,500 | — |
| M4 | $4,500 | — |
| Country Rifle | $1,000 | — |
| Sniper Rifle | $5,000 | — |
| Grenades | $300 | — |
| Satchel Charges | $2,000 | — |
| Body Armor | $200 | — |

Inventory varies by location and story progress.

### 100% Completion Reward

$1,000,000 + infinite ammo + doubled vehicle durability + Hydra jet on Sweet's roof + Rhino tank under Ganton Bridge.

---

## References

### Data Files (Implementation Authority)
- [GTAMods — weapon.dat format](https://gtamods.com/wiki/Weapon.dat)
- [GTAMods — handling.cfg format](https://gtamods.com/wiki/Handling.cfg)
- [GTAMods — pedstats.dat](https://gtamods.com/wiki/Pedstats.dat)
- [GTAMods — popcycle.dat](https://gtamods.com/wiki/Popcycle.dat)
- [GTAMods — roadblox.dat](https://gtamods.com/wiki/Roadblox.dat)
- [GTAMods — Memory Addresses (SA)](https://gtamods.com/wiki/Memory_Addresses_(SA))
- [open.mp — Vehicle Health](https://open.mp/docs/scripting/resources/vehiclehealth)
- [Raw weapon.dat (iOS port)](https://github.com/Google61/gta3sa.app/blob/2.01/weapon.dat)
- [SA-MP weapon-config (pellet counts, fire rates)](https://github.com/oscar-broman/samp-weapon-config)

### Reverse Engineering
- [gta-reversed-modern (CWanted.cpp)](https://github.com/gta-reversed/gta-reversed-modern)
- [DK22Pac/plugin-sdk (CPlayerData.h, CPed.h)](https://github.com/DK22Pac/plugin-sdk)
- [MTA Wiki — Weapon Skill Levels](https://wiki.multitheftauto.com/wiki/Weapon_skill_levels)
- [MTA Wiki — GetPedMoveState](https://wiki.multitheftauto.com/wiki/GetPedMoveState)

### Wikis
- [GTA Wiki (Fandom)](https://gta.fandom.com/wiki/Grand_Theft_Auto:_San_Andreas)
- [Grand Theft Wiki](https://www.grandtheftwiki.com/Grand_Theft_Auto:_San_Andreas)
- [GTAMods Wiki](https://gtamods.com/wiki/Main_Page)

### Guides & Databases
- [GTABase — San Andreas](https://www.gtabase.com/gta-san-andreas/)
- [GTA-SanAndreas.com](https://www.gta-sanandreas.com/)
- [StrategyWiki — GTA:SA](https://strategywiki.org/wiki/Grand_Theft_Auto:_San_Andreas)
- [GTA.cz English — San Andreas](https://www.gta.cz/eng/san-andreas/)
- [Project Cerbera — SA Handling Tutorial](https://projectcerbera.com/gta/sa/tutorials/handling)

### Speedrunning / Technical
- [SDA Knowledge Base — Game Mechanics & Glitches](https://kb.speeddemosarchive.com/Grand_Theft_Auto:_San_Andreas/Game_Mechanics_and_Glitches)
- [Speedrun.com — GTA:SA Frame Limiter Guide](https://www.speedrun.com/gtasa/guides/ux8e4)

### Community
- [GTAForums](https://gtaforums.com/)
- [Steam Community Guides](https://steamcommunity.com/app/12120/guides/)
- [GameFAQs — Stats FAQ](https://gamefaqs.gamespot.com/pc/924362-grand-theft-auto-san-andreas/faqs/47686)
- [Wikibooks — GTA:SA Stats](https://en.wikibooks.org/wiki/Grand_Theft_Auto:_San_Andreas/Basics/Stats)

### General
- [Wikipedia — Grand Theft Auto: San Andreas](https://en.wikipedia.org/wiki/Grand_Theft_Auto:_San_Andreas)
