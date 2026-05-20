# Ace Combat 7: Skies Unknown — Gameplay Systems Spec

PlayStation 4 / Xbox One / PC, January 18, 2019 (PS4), February 1, 2019 (PC/Xbox). Unreal Engine 4. Developed by Project Aces / Bandai Namco Studios.

---

## 1. Core Gameplay Systems

### 1.1 Primary Loop

Arcade flight combat. The player selects a mission from a linear 20-mission campaign, chooses an aircraft and loadout, completes objectives (destroy targets, escort allies, survive), earns MRP (Military Result Points) and a Clear Rank, then spends MRP on the Aircraft Tree to unlock new planes, weapons, and parts. Repeat at higher difficulties or in multiplayer.

### 1.2 Flight Model

Semi-realistic arcade model. Aircraft have six degrees of freedom controlled via throttle, pitch, roll, and yaw. Two control schemes — Standard (assisted banking) and Expert (pure roll + pitch) — detailed in §2.

**Speed and throttle.** Analog throttle: RT accelerates, LT decelerates. Speed displayed in knots. Each aircraft has a unique top speed (2,200–5,750 internal units). Faster aircraft turn wider; slower aircraft turn tighter but risk stalling.

**Altitude.** Ceiling ~39,000–40,000 ft in most missions; Mission 19 raises it to ~42,000–43,000 ft. No altitude floor — terrain collision is instant death.

**Stall.** Occurs at approximately 120 km/h. The Stability stat determines how resistant an aircraft is to stalling. Stalling causes loss of control: nose drops, player must pitch up and apply throttle to recover. Icing from extended cloud exposure also forces a stall (§1.3).

**High-G turns.** Holding both accelerate and decelerate simultaneously while turning executes a High-G turn — a much tighter turn radius at the cost of rapid speed bleed. Sustained High-G turning leads to stall. The ECU Software Update part extends the safe window.

**Post-stall maneuvers (PSM).** Available on select aircraft (see §4). Requires slowing to ~430–500 km/h, releasing stick input, holding accel+decel, then pulling a direction. Three types:

- **Pugachev's Cobra** — nose pitches to vertical and drops back, causing pursuers to overshoot
- **Kulbit** — continues the Cobra into a full tight loop
- **Knife-Edge Slide** — tight yaw pivot while maintaining velocity

PSMs bleed significant energy and leave the pilot vulnerable if used carelessly. 19 aircraft support full PSM (Cobra + Kulbit + Knife-Edge); 11 support partial PSM (Cobra only).

### 1.3 Cloud and Weather System

Clouds are a core gameplay mechanic unique to AC7:

**Stealth.** Aircraft inside clouds are visually obscured. Still appear on radar/HUD indicators, but degree of concealment depends on cloud density.

**Missile disruption.** Lock-on times increase when targeting through clouds. Lock-on range decreases when the target is in a cloud. Missiles entering clouds lose homing capability. Clouds are the most effective defense against energy weapons (TLS, PLSL).

**Icing.** Extended cloud exposure causes ice buildup; too much ice forces a stall. The Anti-Icing/De-Icing System part (7 Misc slots, 10,000 MRP) prevents icing.

**Lightning.** Flying in thunderstorms risks strikes that distort the HUD: radar and gauges become unreliable, unit containers disappear. Player must rely on visual guidance until effects fade.

**Turbulence.** High winds push the aircraft off course and affect missile flight paths.

### 1.4 Combat System

Three weapon classes are always available:

**Machine Gun (GUN).** Three gun types exist:

| Gun Type | RoF | Damage | Aircraft | Hard/Ace Ammo |
|----------|-----|--------|----------|---------------|
| 20mm Vulcan | High | Lower | Most American planes, MiG-21bis, MiG-31B | 2,400 |
| 30mm Autocannon | Lower (burst) | Higher | Most Russian/European planes | 800 |
| 30mm Gatling (GAU-8/A) | High | High | A-10C, X-02S Strike Wyvern | 4,800 / 3,600 |

GUN ammo is infinite on Easy and Normal. Finite on Hard and Ace.

**Standard Missiles (MSL).** Lock-on homing missiles effective against air and ground. Counts vary by aircraft tier (62–250). Damage: 40 per hit (datamined). When all standard missiles are expended, 2 emergency missiles regenerate after several seconds. On Casual/Easy, standard missiles are infinite.

**Special Weapons (SP.W).** Each aircraft carries 3 special weapon options; the player picks one per sortie. Categories: air-to-air missiles, air-to-ground ordnance, and energy/exotic weapons. Full weapon catalog in [docs/weapons.md](docs/weapons.md).

### 1.5 Weapon Switching & Lock-On

**Weapon cycling.** Square/X cycles through MSL → SP.W → MSL. Only one weapon type is active at a time; the active type determines lock-on behavior.

**Standard lock-on.** When an enemy enters lock range, a circle appears around the target reticle. Holding the target in the reticle fills the circle; once solid, the weapon is locked and can be fired. Lock-on speed and range depend on the weapon, equipped parts, stealth rating of the target, and cloud interference.

**Multi-lock weapons** (4AAM, 6AAM, 8AAM, 4AGM, 8AGM, XSDB). Holding the fire button enters multi-lock mode: sweeping the reticle over targets sequentially locks each one (up to the weapon's target count). Releasing the button fires all locked missiles simultaneously. The same target can be locked multiple times to stack missiles on it.

**Semi-active weapons** (SAAM). After firing, the player must keep the target within the crosshair for continued guidance. Breaking line of sight or losing the reticle on target causes the missile to go ballistic.

### 1.6 Target Types

**TGT (mission-critical targets).** Marked with an orange TGT label. Must be destroyed to advance the mission or complete the current phase. Always highlighted on radar.

**Non-TGT enemies.** Optional targets that award score points when destroyed. Destroying them improves Clear Rank but is not required for mission completion.

**Score-threshold missions.** Missions 06 and 11 require accumulating a minimum score from any targets (TGT or otherwise) within a time limit rather than destroying specific TGTs.

### 1.7 Missile Evasion

**Flares.** Deployed with LS+RS. Limited quantity per aircraft (2–6 depending on tier). Must be deployed at the last moment before impact — early deployment fails. Low-tier planes carry more flares (5–6); stealth/high-tier carry fewer (2–3).

**Maneuvering.** Turn perpendicular to incoming missile trajectory. High-G turns are highly effective.

**Clouds.** Flying into clouds degrades missile tracking and can break locks.

**Speed.** Afterburner at extreme range can outrun missiles; less effective at close range.

### 1.8 Damage Model

Each aircraft has a base HP value:

| Tier | HP Range | Examples |
|------|----------|---------|
| Low | 80–100 | MiG-21bis (80), F-16C (90), F-14D (100) |
| Mid | 100–120 | Most mid-tree fighters (105–120) |
| High | 125–143 | F-22A (130), Su-57 (130), ADF-11F/CFA-44/XFA-27 (143) |

Datamined weapon damage values:

| Weapon | Damage |
|--------|--------|
| Standard MSL | 40 |
| HCAA | 63 |
| 4AAM / 6AAM / 8AAM | 80 |
| EML | 120 |
| HPAA | 160 |

Difficulty affects damage taken. The Bulletproof Fuel Tank part increases durability; the Automated Fire Extinguisher (campaign only, 10 slots) restores HP after a damage threshold.

### 1.9 Scoring System

Total Score = Destruction Score + Time Bonus.

- **Destruction Score** — sum of all enemies the player personally destroys
- **Time Bonus** — starts at a mission-specific maximum, held until a time threshold, then decays at 30–50 pts/sec (mission-dependent) until zero

Exceptions: Missions 06 and 11 are score-only (no time bonus; see §1.6). Checkpoint restarts do NOT reset the elapsed timer, penalizing Time Bonus (§11.2).

Clear Ranks: D, C, B, A, S. S-rank thresholds are identical across all difficulties. Full S-rank requirements in §5.3.

### 1.10 Progression: Aircraft Tree and MRP

**MRP (Military Result Points).** Single currency earned from campaign missions and multiplayer matches. Difficulty multipliers affect earnings (§5.4). Spent on aircraft, weapons, and parts.

**Aircraft Tree.** A branching left-to-right technology tree. Leftmost nodes are cheapest/weakest, rightmost are strongest/most expensive. Nodes include aircraft, special weapons, and tuning parts. Multiple branches diverge from early aircraft; the player chooses which paths to advance. Color coding: blue (fighters/A2A), red (attackers/A2G), purple (multirole), yellow/green (parts).

**Starting aircraft.** F-16C is free from the start. F-104C and MiG-21bis unlock free after completing Mission 04.

---

## 2. Controls & Input

### 2.1 Gamepad Layout (PS4 / Xbox)

| Input | PS4 | Xbox | Function |
|-------|-----|------|----------|
| Flight control | Left Stick | Left Stick | Pitch + Turn (Standard) or Pitch + Roll (Expert) |
| Camera | Right Stick | Right Stick | Free look |
| Yaw left | L1 | LB | Rudder left |
| Yaw right | R1 | RB | Rudder right |
| Decelerate | L2 | LT | Throttle down / brake |
| Accelerate | R2 | RT | Throttle up / afterburner |
| Machine gun | X | A | Fire GUN |
| Fire missile/weapon | O | B | Fire locked MSL or SP.W |
| Change target | Triangle | Y | Cycle target lock |
| Change weapon | Square | X | Cycle MSL / SP.W |
| Camera view | R3 | RS | Toggle cockpit / chase / HUD-off |
| Deploy flares | L3 + R3 | LS + RS | Countermeasures |
| Radar map | Touchpad | View | Toggle radar zoom |
| Pause | Options | Menu | Pause menu |

### 2.2 Control Variants

**Standard vs Expert.** Standard maps horizontal stick to assisted banking — stick left/right turns the plane directly. Expert maps it to pure roll, requiring the player to combine roll + pitch to turn, with yaw on L1/R1 (LB/RB). Expert is required for advanced maneuvers and competitive play.

**Type A vs Type B.** Swaps throttle/yaw mapping. Type B puts accelerate/decelerate on bumpers and yaw on triggers.

High-G turn and PSM inputs are described in §1.2.

---

## 3. World Structure

### 3.1 Mission-Based Structure

No open world. Each of the 20 campaign missions is a self-contained sortie over a unique map region. The player cannot freely navigate between missions or revisit areas outside of mission replay / Free Flight mode.

### 3.2 Mission Environments

Missions span diverse terrain: ocean (carrier operations), desert, mountains, urban cities, canyons, archipelagos, and the space elevator (Lighthouse). Weather conditions include clear skies, thunderstorms (Mission 07), sandstorms (Mission 08), nighttime (Mission 14), and mixed cloud layers.

### 3.3 Return Line

A boundary line behind the player's starting area. Crossing it mid-mission resupplies ammunition and repairs damage. On Ace difficulty, repair is disabled (§5.4).

### 3.4 Carrier Operations & Mid-Air Refueling

**Carrier takeoff.** Some missions begin on an aircraft carrier deck. The player throttles up and launches from the catapult. Carrier landing occurs at the end of select missions — the player must line up with the deck and touch down within the arresting wire zone.

**Mid-air refueling.** Present in select missions (notably Mission 16 and the DLC missions). The player flies into formation behind a tanker aircraft and holds position within a target zone while fuel transfers. Drifting out of the zone aborts the refueling.

Both mechanics contribute to the "There And Back Again" trophy (complete all takeoffs, refuelings, and landings).

### 3.5 Mission Briefing & Debriefing

**Briefing.** Before each sortie, a tactical briefing presents the mission situation, objectives, and threat layout on a map screen. The player then selects an aircraft, special weapon, and parts loadout.

**Debriefing.** After mission completion, a results screen shows the score breakdown (destruction score + time bonus), Clear Rank, MRP earned, and any medals or aces encountered.

### 3.6 Free Flight

Unlocked after completing the campaign. 10-minute free roam over any campaign mission map with no enemies or objectives. Used for practice, exploration, and testing aircraft.

---

## 4. Playable Aircraft

28 base-game aircraft + 19 DLC aircraft = 47 total playable planes.

Aircraft are categorized as Fighter (air-to-air focused), Attacker (air-to-ground focused), or Multirole.

### 4.1 PSM Capability

**Full PSM** (Cobra + Kulbit + Knife-Edge) — 19 aircraft: F-22A, Su-35S, Su-37, Su-47, Su-57, X-02S, Su-30SM, Su-30M2, ADF-01, ADF-11F, ADFX-01, ASF-X, CFA-44, XFA-27, F-15 S/MTD, MiG-35D, F-14A (TGM), F/A-18E (TGM), 5th Gen Fighter (TGM).

**Partial PSM** (Cobra only) — 11 aircraft: F/A-18F, F-35C, MiG-29A, Su-33, Su-34, Gripen E, Rafale M, Typhoon, F/A-18F Block III, F-2A Super Kai, F/A-18E.

### 4.2 Aircraft Roster

Full aircraft tables with datamined stats, costs, weapon loadouts, and unlock conditions are in [docs/aircraft.md](docs/aircraft.md).

### 4.3 Stealth Rating

Internal 0–6 scale. Higher stealth means enemies take longer to lock on, missiles lose tracking more easily, and detection range is reduced. Key stealth values:

| Rating | Aircraft |
|--------|----------|
| 6 | F-35C, YF-23, ADF-01, ADF-11F |
| 5 | F-22A, X-02S, 5th Gen Fighter (TGM), FB-22 |
| 4 | Su-57 |
| 3 | ASF-X, XFA-27, CFA-44, DarkStar, MiG-35D, F/A-18F Block III |
| 2 | F-2A Super Kai |
| 1 | F-2A, F/A-18F, Rafale M, Typhoon, Su-35S, Su-47, ADFX-01, F/A-18E |
| 0 | All others |

---

## 5. Story & Progression

### 5.1 Campaign Structure

20 missions, strictly linear (no branching). The story follows "Trigger" (the player) through three acts defined by squadron assignment:

**Act 1 — Mage Squadron (Missions 1–4).** The Lighthouse War between Osea and Erusea begins. The Arsenal Bird Liberty superweapon is introduced. Trigger is falsely accused of murdering former President Harling during Mission 04 and court-martialed.

**Act 2 — Spare Squadron (Missions 5–10).** Trigger is sent to the 444th Air Base penal unit. Dangerous, expendable missions. First encounter with rival ace Mihaly A. Shilage. Trigger encounters the ADFX-10 prototype drone and is eventually transferred to the LRSSG.

**Act 3 — LRSSG / Strider Squadron (Missions 11–20).** Trigger leads Strider Squadron. Osean counteroffensive. Stonehenge railgun destroys Arsenal Bird Liberty (Mission 12). Capital Farbanti falls but satellite destruction causes a communications blackout via Kessler syndrome (Mission 15). Erusean civil war erupts. Final battles against AI drones Hugin and Munin at the space elevator, culminating in a tunnel chase sequence (Mission 20).

Story is told through gameplay, briefings, and cutscenes following multiple POV characters: Avril Mead (mechanic), Princess Rosa Cossette D'Elise, and Mihaly A. Shilage.

### 5.2 Mission List

| # | Name | Type | Key Mechanic |
|---|------|------|-------------|
| 01 | Charge Assault | Air superiority | Bomber interception |
| 02 | Charge the Enemy | Mixed air+ground | Base attack + UAV introduction |
| 03 | Two-pronged Strategy | Mixed | Arsenal Bird encounter |
| 04 | Rescue | Escort/stealth | Radar infiltration; VIP extraction |
| 05 | 444 | Air superiority | No weapons for first 2 minutes; evasion only |
| 06 | Long Day | Ground attack (score) | 15-min timer; score-only ranking |
| 07 | First Contact | Mixed | Thunderstorm; first Mihaly encounter (invincible) |
| 08 | Pipeline Destruction | Ground attack | Sandstorm time limit |
| 09 | Faceless Soldier | Mixed | Drones using false IFF (friendly-fire deception) |
| 10 | Transfer Orders | Escort | ADFX-10 prototype encounter |
| 11 | Fleet Destruction | Naval (score) | 15-min timer; score-only ranking |
| 12 | Stonehenge Defensive | Mixed / boss | Arsenal Bird Liberty destroyed by railgun |
| 13 | Bunker Buster | Ground attack | IRBM interception chase |
| 14 | Cape Rainy Assault | Stealth/ground | Night canyon with spotlight avoidance |
| 15 | Battle for Farbanti | Mixed / boss | Urban warfare; comms blackout (Kessler syndrome) |
| 16 | Last Hope | Escort | Drone betrayal; ground-launched UAVs |
| 17 | Homeward | Mixed | Retreat cover; refugee escort |
| 18 | Lost Kingdom | Air superiority / boss | Mihaly dogfight at Shilage Castle |
| 19 | Lighthouse | Mixed / boss | Arsenal Bird Justice; APS shield mechanic |
| 20 | Dark Blue | Air superiority / boss | ADF-11F dual boss; undersea tunnel sequence |

### 5.3 S-Rank Requirements

| # | Mission | S-Rank Score | Time Limit |
|---|---------|-------------|------------|
| 01 | Charge Assault | 20,500 | 4:50 |
| 02 | Charge the Enemy | 33,000 | 6:00 |
| 03 | Two-pronged Strategy | 44,000 | 9:00 |
| 04 | Rescue | 50,000 | 10:30 |
| 05 | 444 | 35,000 | 8:00 |
| 06 | Long Day | 27,000 | N/A (score only) |
| 07 | First Contact | 46,350 | 12:30 |
| 08 | Pipeline Destruction | 40,550 | 10:00 |
| 09 | Faceless Soldier | 43,000 | 11:00 |
| 10 | Transfer Orders | 27,000 | 12:00 |
| 11 | Fleet Destruction | 40,000 | N/A (score only) |
| 12 | Stonehenge Defensive | 50,000 | 16:30 |
| 13 | Bunker Buster | 38,000 | 7:00 |
| 14 | Cape Rainy Assault | 32,000 | 9:00 |
| 15 | Battle for Farbanti | 57,000 | 21:30 |
| 16 | Last Hope | 39,040 | 18:00 |
| 17 | Homeward | 44,000 | 15:30 |
| 18 | Lost Kingdom | 37,000 | 12:30 |
| 19 | Lighthouse | 67,500 | 20:30 |
| 20 | Dark Blue | 36,000 | 10:00 |

### 5.4 Difficulty Settings

| Aspect | Easy | Normal | Hard | Ace |
|--------|------|--------|------|-----|
| MRP multiplier | 0.8x | 1.0x | 1.2x | 1.5x |
| GUN ammo | Infinite | Infinite | Finite | Finite |
| MSL count | Generous | Standard | Reduced | Reduced |
| Damage taken | Reduced | Standard | Increased | Greatly increased |
| Enemy AI | Passive | Standard | Aggressive | Relentless |
| Return Line repair | Yes | Yes | Yes | **No** |
| Named aces spawn | No | Yes | Yes | Yes |
| Unlock condition | — | — | — | Complete all missions on Hard |

### 5.5 DLC Missions

Three DLC missions (Season Pass) set between Missions 13 and 14, following the Alicorn submarine storyline:

| # | Name | Objective |
|---|------|-----------|
| SP01 | Unexpected Visitor | Intercept the Alicorn at Artiglio Port; Mimic Squadron introduced |
| SP02 | Anchorhead Raid | Destroy Erusean Ran Fleet to isolate the Alicorn |
| SP03 | Ten Million Relief Plan | Sink the Alicorn before it fires a nuclear railgun shell at Oured |

### 5.6 VR Missions (Originally PSVR Exclusive; later PC via SteamVR)

Three standalone missions set in 2014 starring Mobius 1 (protagonist of Ace Combat 04):

| # | Map | Objective |
|---|-----|-----------|
| VR01 | Fort Grays Island | Three-wave bomber interception |
| VR02 | Scofields Plateau | Dogfight + base defense |
| VR03 | Waiapolo Mountains | Dogfight; Ghost Squadron boss encounter |

Additional VR features: Free Flight (10-min roam), Airshow Mode (watch aerobatics from a carrier deck), Hangar (inspect aircraft in VR).

---

## 6. Items & Equipment

### 6.1 Parts System

Three categories: **Body**, **Arms**, **Misc**. Each aircraft has a per-category slot budget (low-tier ~42 total slots; high-tier ~28). Maximum 8 parts equipped simultaneously regardless of remaining slots. Parts are purchased from the Aircraft Tree with MRP.

- **Lv.1 parts** — available in Campaign and Multiplayer
- **Lv.2/3 parts** — Multiplayer only (higher cost, more slots, stronger effects)

### 6.2 Body Parts

16 campaign parts covering acceleration, deceleration, top speed, maneuverability (pitch/roll/yaw), stability, durability, and stealth. Slot costs range 2–10; MRP costs range free–70,000. Notable parts:

- **Variable Cycle Engine** (6 slots, 70,000 MRP) — acceleration + top speed; most efficient all-around engine boost
- **ECU Software Update** (5 slots, 50,000 MRP) — extends High-G turn window before stall
- **Bulletproof Fuel Tank** (4 slots, 35,000 MRP) — increases HP / durability
- **Automated Fire Extinguisher** (10 slots, 50,000 MRP) — restores HP after damage threshold; campaign only
- **Queen's Custom** (5 slots, free) — all-around boost; unlocked by completing Mission 07

### 6.3 Arms Parts

Arms parts cover six weapon categories: Machine Gun, Standard Missiles (MSL), Special Missiles (Sp.W), Special Bombs, Energy Weapons (TLS/PLSL/EML), and Rockets (RKT/GRKT). Each category has power, speed/range, ammo capacity, reload, homing, lock-on range, and/or lock-on speed upgrades. Slot costs range 3–9; MRP costs range 10,000–70,000.

### 6.4 Misc Parts

7 campaign parts covering stealth detection, collision resistance, cloud lock-on penalties, icing prevention, and GUN targeting automation. The **Onboard Self-Defense Jammer** (9 slots, 35,000 MRP) reduces enemy missile homing and is campaign only.

Full parts tables for all categories (Body, Arms, Misc) including multiplayer-only Lv.2/3 variants are in [docs/parts.md](docs/parts.md).

### 6.5 Skins

Each aircraft has a default skin plus additional unlockable skins. 24 "Special" skins are earned by shooting down the corresponding Named Ace (§9.2). Additional skins from DLC packs. S-ranking all missions on a given difficulty unlocks a skin set.

---

## 7. Enemies & Opponents

### 7.1 Standard Enemies

**Fighters.** Various real-world and fictional aircraft flown by Erusean forces and Sol Squadron. Standard fighters take 1–2 standard missile hits. Named aces are tougher and more evasive.

**Bombers.** Large, slow, high-HP targets. Take multiple hits. Often escorted.

**UAVs/Drones.** MQ-99 (small, fast, expendable), MQ-101 (Arsenal Bird escorts), ADFX-10 (prototype, Mission 10), ADF-11F (final boss drones). Drones are highly agile and can use false IFF signals (Mission 09).

**Ground targets.** Tanks, SAM sites, radar vehicles, AA guns, oil facilities, missile silos, bunkers. Some are marked as TGT (required); others are optional for score.

**Naval targets.** Destroyers, cruisers, aircraft carriers, submarines. The Alicorn (DLC) is a submersible aviation cruiser with a nuclear railgun.

### 7.2 Boss Encounters

| Mission | Boss | Aircraft | Mechanics |
|---------|------|----------|-----------|
| 07 | Mihaly (first) | Su-30SM | Invincible — cannot be shot down; story event |
| 12 | Arsenal Bird Liberty | Superweapon | Defend Stonehenge; APS shield must be disabled by allies; railgun destroys it |
| 15 | Sol Squadron | Various | Multi-ace dogfight during urban chaos |
| 18 | Mihaly A. Shilage | X-02S Strike Wyvern | Castle environment; aggressive PSM usage; must damage him in phases |
| 19 | Arsenal Bird Justice | Superweapon | APS shield mechanic — destroy propeller drones to lower shield, then attack |
| 20 | Hugin & Munin | ADF-11F Raven ×2 | Phase 1: aerial dogfight. Phase 2: chase one drone through undersea tunnels |

### 7.3 Sol Squadron

Erusean elite ace squadron led by Mihaly A. Shilage (Sol 1). Recurring antagonists throughout Act 3. Members fly high-performance aircraft (Su-30SM, Su-35S, Su-37). Mihaly's flight data is used to develop the AI combat drones that serve as the final bosses.

---

## 8. Economy

### 8.1 Currency

Single currency: **MRP (Military Result Points)**. Earned from campaign missions and multiplayer matches. Difficulty multipliers affect campaign earnings (§5.4).

### 8.2 Income & Spending

| Source / Sink | MRP Range |
|---------------|-----------|
| Campaign missions (Normal) | ~5,000–80,000+ depending on score and mission |
| Multiplayer matches | Variable; performance evaluation bonus 500–5,000 |
| Aircraft (base game) | 65,000–2,000,000 |
| Special weapons | 10,000–100,000 (purchased as tree nodes) |
| Parts (Lv.1 campaign) | 10,000–70,000 |
| Parts (Lv.2/3 multiplayer) | 100,000–500,000 |

Per-aircraft costs and stats are in [docs/aircraft.md](docs/aircraft.md).

---

## 9. Side Systems & Collectibles

### 9.1 Medal System

25 campaign medals + 10 multiplayer medals.

**Cumulative campaign medals:**

| Medal | Requirement |
|-------|-------------|
| Bronze / Silver / Gold Ace | 1,000 / 2,000 / 3,000 enemies destroyed |
| Bronze / Silver / Gold Marksman | 50 / 200 / 300 GUN kills |
| Raze The Roof | 1,500 surface targets destroyed |
| Ship Of Liberty | 100 ships sunk |
| Ghostbuster | 200 drones shot down |

**Mission-specific medals (selected):**

| Medal | Mission | Requirement |
|-------|---------|-------------|
| Mass Destruction | 03 | Destroy 30+ UAVs |
| Silence Is Golden | 04 | Penetrate radar within 3 minutes |
| First Try | 06 | 50+ ground targets without Return Line |
| Clairvoyant | 13 | Destroy all silos within 5 minutes |
| Getting The Job Done | 18 | Beat Mihaly in 5 min, no special weapons |

**Challenge medals:**

| Medal | Requirement |
|-------|-------------|
| Not A Scratch | Complete campaign without taking damage |
| Machine Gun Maniac | Complete campaign using only GUN |
| Photon Blitz | Complete campaign in cumulative 4 hours or less |

### 9.2 Named Aces

24 hidden named aces across the campaign. Spawn conditions:
- Only on Normal difficulty or higher
- Only when replaying missions (after first campaign completion)
- Each has a unique spawn trigger (specific feat within the mission)

Shooting down an ace unlocks the "Special" skin for that ace's aircraft type. All 24 unlock the "Bird of Prey" achievement and the X-02S Strike Wyvern Special skin.

Full named ace table with spawn conditions in [docs/aces.md](docs/aces.md).

### 9.3 Achievements / Trophies

50 total (1 Platinum, 2 Gold, 11 Silver, 36 Bronze). Key milestones:

| Trophy | Tier | Requirement |
|--------|------|-------------|
| SKIES UNKNOWN | Platinum | All trophies |
| ACE OF ACES | Gold | S-rank all missions on Ace |
| The Highest Achiever | Gold | All campaign medals |
| What A Wonderful World | Silver | Complete campaign |
| Bird Of Prey | Silver | Destroy all 24 named aces |
| Patron Of The Parts | Silver | All aircraft, weapons, and parts purchased |
| MRP Aplenty | Silver | Earn 40,000,000 MRP total |

---

## 10. UI & HUD

### 10.1 HUD Layout

The combat HUD displays:

- **Center** — pitch ladder (horizon lines with degree markings), aircraft nose reticle, and gun/missile targeting reticle
- **Top-left** — speed indicator (knots), altitude indicator (feet)
- **Top-right** — current weapon and ammo count, weapon switching indicator
- **Bottom-left** — radar display (circular, shows friendlies as green, hostiles as red, mission targets as orange TGT markers)
- **Bottom-right** — throttle gauge
- **Bottom-center** — objective text / radio subtitles
- **Target boxes** — green squares for allies, red squares for enemies, orange for mission-critical targets. Distance displayed next to each. Damage bar appears below locked target.
- **Missile alert** — "MISSILE" warning text + directional indicator when incoming. Flashes red when impact is imminent.
- **Lock-on circle** — appears around target when within lock range; fills as lock progresses; solid = locked

### 10.2 HUD States

- **Normal flight** — full HUD with all indicators
- **Cloud entry** — HUD elements flicker and degrade with icing buildup
- **Lightning strike** — HUD distorts; radar scrambles; target boxes disappear temporarily
- **Stall warning** — "STALL" text flashes; speed indicator turns red
- **Damage critical** — screen edges tint red; smoke trails from aircraft
- **Cutscene/briefing** — HUD hidden entirely
- **Mission 05 special** — weapon indicators hidden for first 2 minutes (no weapons available)

### 10.3 Camera Views

Three views cycled with R3/RS:
1. **Third-person chase** — default; camera behind and above aircraft
2. **Cockpit** — first-person interior with functional instruments
3. **HUD-only** — third-person with minimal HUD overlay

### 10.4 Radar

Circular minimap in bottom-left. Toggle zoom with Touchpad/View button. Shows:
- Green dots — allied aircraft
- Red dots — enemy aircraft
- Orange markers — mission targets (TGT)
- Terrain silhouette at edges

### 10.5 Target Display

When locked onto an enemy:
- Target name and aircraft type displayed
- Distance in meters
- HP bar below target box (for bosses and durable targets)
- Special weapon lock indicators (multi-lock weapons show numbered diamonds)

---

## 11. Engine & Presentation Systems

### 11.1 Save System

**Autosave** at mission checkpoints and mission completion. Single save slot per difficulty. Campaign progress, MRP balance, and Aircraft Tree purchases are persistent across all difficulties (shared progression).

### 11.2 Checkpoint System

Missions have mid-mission checkpoints. Restarting from checkpoint restores HP and ammo but does NOT reset the elapsed timer (penalizing Time Bonus for S-rank). Full mission restart resets everything.

### 11.3 Camera Behavior

Third-person chase camera follows behind the aircraft with smooth lag. Camera automatically adjusts during High-G turns (pulls wider). During PSMs the camera dramatically swings to show the maneuver. Cockpit view includes head tracking in VR mode.

### 11.4 Audio System

- Dynamic music system: intensity scales with combat engagement. Calm exploration music transitions to aggressive combat tracks when enemies engage.
- Radio chatter is continuous — allied and enemy pilots communicate mission status, callouts, and story dialogue during gameplay.
- Missile alert audio: escalating beep pattern as missile approaches; distinct lock-on warning tone.
- Engine sound varies with throttle position and aircraft type (jet whine, afterburner roar).
- Doppler effect on flyby sounds.
- DLC includes a Music Player feature (Season Pass) for listening to the soundtrack.

### 11.5 Replay System

Post-mission replay allows reviewing the sortie from multiple camera angles. Player can scrub through the timeline, switch between aircraft perspectives, and save replay data.

---

## 12. Multiplayer

### 12.1 Modes

**Battle Royal (Free-For-All).** 2–8 players. Score-based — NOT elimination. Players respawn after a short wait when shot down. Points awarded based on the target's aircraft cost, equipped parts, and special weapons (higher-tier targets = more points). Partial damage awards proportional points. Win by reaching the score limit first or having the highest score when time expires.

**Team Deathmatch.** 2–8 players split into two teams. Same scoring mechanics. Team with highest combined score wins.

### 12.2 Maps

| Map | Setting |
|-----|---------|
| Fort Grays Island | Ocean with concealing clouds |
| Anchorhead Bay | Nighttime urban, bridges, tall buildings |
| Waiapolo Mountains | Mountain peaks with cloud cover |
| Yinshi Valley (Morning) | Rocky ravines |
| Yinshi Valley (Night) | Dark, stormy terrain |
| Roca Roja | Arid desert, evening sunset |

### 12.3 Room Settings

Host configures: game mode, map, player count (2–8), Total Cost Limit (restricts loadout by point value), special weapons on/off, team change on/off, hot join on/off, private room on/off.

### 12.4 Total Cost Limit

Each aircraft, special weapon, and part has a point cost. The Total Cost Limit caps the combined value of a player's loadout. Presets include 2000 and Unlimited among others. Prevents fully-unlocked players from overwhelming newcomers.

### 12.5 Performance Evaluations

Post-match ranks award MRP bonuses: S (5,000), A (3,000), B (2,000), C (1,000), D (500).

### 12.6 Multiplayer-Only Parts

Lv.2 and Lv.3 versions of Body and Arms parts with stronger effects at higher slot/MRP costs. Also MRP-earning bonus parts (Rookie Pilot Bonus, Fighter/Multirole/Attacker Pilot Bonus, weapon-type Fanatic bonuses).

---

## 13. Open Questions / Unverified

- **Exact damage formulas for GUN types.** Community consensus is that 20mm Vulcan does less per-round than 30mm Autocannon, but precise per-round damage values are not publicly datamined.
- **Missile homing degradation curves.** The exact formula for how clouds reduce missile homing (linear? percentage?) is not documented.
- **Stealth stat mechanics.** The 0–6 stealth scale's precise effects on lock-on time, detection range, and missile tracking are not fully mapped. Community testing suggests each point adds ~0.5s to enemy lock time, but this is unverified.
- **Per-aircraft slot budgets.** Exact per-category slot limits (Body/Arms/Misc) for each aircraft are not consistently documented across sources. Total slots range ~28–42 with low-tier having more to compensate for weaker stats.
- **Su-37 third special weapon.** Sources conflict on whether the Su-37 Terminator's third weapon is TLS (Tactical Laser System) or UAV (Weapon UAV). The same ambiguity applies to the ADF-01 FALKEN's third slot (FAEB vs UAV). Current spec lists TLS/FAEB respectively based on per-aircraft wiki data, but datamine cross-references suggest UAV may be correct for one or both.
- **Time Bonus decay rates.** Decay rates of 30–50 pts/sec per mission are sourced from community S-rank guides but not directly datamined.
- **Su-30SM stats.** Datamined speed/acceleration values for the Su-30SM are missing from the primary community stats repository.

---

## 14. References

### Wikis
- Ace Combat Wiki (acecombat.wiki.gg) — aircraft, weapons, missions, parts, aces
- Acepedia (acecombat.fandom.com) — medals, achievements, DLC, difficulty
- StrategyWiki — controls, general mechanics

### Community Resources
- GitHub: rolex20/ace-combat-7-stats-viewer — datamined aircraft stats
- Steam Community Guides — S-rank requirements, speed data, named aces, medals
- GameFAQs — mission walkthroughs, scoring strategies

### Companion Documents
- [docs/aircraft.md](docs/aircraft.md) — full aircraft roster with stats, costs, weapons, unlock conditions
- [docs/weapons.md](docs/weapons.md) — complete special weapon catalog with descriptions and mechanics
- [docs/aces.md](docs/aces.md) — all 24 named aces with spawn conditions and rewards
- [docs/parts.md](docs/parts.md) — complete parts list (Body, Arms, Misc) for campaign and multiplayer
