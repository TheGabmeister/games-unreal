# Tomb Raider (2013) — Gameplay Systems Spec

Crystal Dynamics / Square Enix. Xbox 360, PS3, PC (2013); Definitive Edition on PS4, Xbox One (2014). This spec covers the base single-player game; multiplayer is documented in §9. Definitive Edition differences noted in §11.

---

## 1. Core Gameplay Systems

### 1.1 Primary Loop

Third-person action-adventure. The loop is: **explore hub area → collect resources/XP → upgrade at base camp → advance story → unlock new gear → backtrack to reach gated areas**. Combat encounters punctuate exploration, ranging from stealth-optional infiltrations to arena-style firefights and scripted set pieces.

### 1.2 Combat

Cover-based third-person shooting. Lara auto-crouches behind waist-high objects when enemies are nearby — there is no manual cover button. Combat is built around four ranged weapons (§6.1), melee finishers, and a dodge-counter system.

**Aiming**: Hold aim to enter over-the-shoulder view. Shoulder swap is available while aiming. Further zoom available for precision shots.

**Dodge**: Directional dodge/roll while aiming or in melee range. Dodge is the foundation of the Brawler skill tree — successful dodges open enemies to counters and instant kills (§4.3).

**Melee**: The climbing axe serves as the melee weapon. Melee is primarily used through Brawler skills (Axe Strikes, Dodge Counter, Dodge Kill) and QTE finishers, not as a primary combat approach.

**Stealth**: Lara auto-crouches when enemies are nearby. The bow is inherently silent. Stealth kills are available from behind (bow strangle → combat knife → climbing axe as gear evolves). Survival Instinct highlights enemies white (safe to kill silently) or red (will alert others). Clearing an area undetected awards bonus XP. Silencer upgrades are available for the pistol (350 salvage) and rifle (500 salvage). Many encounters are scripted to begin with Lara already detected.

**Environmental Combat**: Explosive red fuel barrels detonate when shot, dealing area damage. Lanterns can be picked up and thrown (hold aim for trajectory arc) to ignite enemies, gas clouds, and flammable objects. Gas valves release flammable gas into rooms — ignitable via fire arrows or thrown lanterns for large explosions. Toxic green gas kills Lara over time and must be shut off via pry axe valves. Destructible cover and ledges can be shot out to expose or topple enemies.

### 1.3 Health

No visible health bar. Damage is communicated through screen desaturation toward greyscale, blood splatter at screen edges, and labored animations. Health regenerates automatically when Lara stops taking damage. On Hard difficulty, health does not regenerate during combat.

### 1.4 Progression

Two parallel upgrade currencies:

- **XP → Skill Points**: XP fills a shield-shaped meter. Each fill awards 1 skill point, spent at base camps across three skill trees (§4). Exact XP-per-level thresholds are not publicly documented and appear to scale upward.
- **Salvage → Weapon Upgrades**: Salvage is spent at base camps to purchase weapon modifications (§6.2).

**XP sources**:

| Source | XP |
|---|---|
| Standard kill | ~10 |
| Headshot | ~25 |
| Stealth kill | ~25 |
| Finishing move | ~25 |
| Hunting animal | ~20 |
| Arrow retrieval (with skill) | 20 per arrow |
| Challenge item found | 10 |
| Challenge completed | 150 |
| Challenge Tomb completed | 1,250 |
| Story milestones | 100–300 |

**Salvage sources**: Salvage crates (~10 per small crate, ~20 from hidden bundles), salvage nets (burn to release), enemy corpses, animal corpses, challenge tombs (250 per tomb), and finding all 75 GPS caches (1,000 bonus). Survivor skills (Advanced Salvaging, Bone Collector) increase salvage yields.

---

## 2. Controls & Input

### 2.1 PC (Keyboard + Mouse)

| Action | Key |
|---|---|
| Move | W/A/S/D |
| Walk | Ctrl + direction |
| Jump / Wall Scramble | Space (tap twice for scramble) |
| Dodge / Roll | Shift |
| Interact / Climb Axe | E |
| Survival Instinct | Q |
| Aim | Right Mouse |
| Fire | Left Mouse |
| Alternate Fire | Middle Mouse |
| Melee / Finisher | F |
| Reload | R |
| Shoulder Swap (aiming) | C |
| Zoom (aiming) | Z |
| Map / Objectives | Tab |
| Select Bow / Pistol / Shotgun / Rifle | 1 / 2 / 3 / 4 |
| Cycle Weapons | Mouse Wheel |

### 2.2 Xbox 360 Controller

| Action | Button |
|---|---|
| Move | Left Stick |
| Jump / Wall Scramble | A (tap twice) |
| Dodge / Roll | B |
| Interact / Climb Axe | X |
| Survival Instinct | LB |
| Aim | LT |
| Fire | RT |
| Alternate Fire | RB |
| Melee / Finisher | Y |
| Shoulder Swap | L3 |
| Zoom | R3 |
| Map | Back |
| Bow / Pistol / Shotgun / Rifle | D-Pad Up / Down / Left / Right |

### 2.3 PS3 Controller

| Action | Button |
|---|---|
| Move | Left Stick |
| Jump / Wall Scramble | X (tap twice) |
| Dodge / Roll | Circle |
| Interact / Climb Axe | Square |
| Survival Instinct | L2 |
| Aim | L1 |
| Fire | R1 |
| Alternate Fire | R2 |
| Melee / Finisher | Triangle |
| Shoulder Swap | L3 |
| Zoom | R3 |
| Map | Select |
| Bow / Pistol / Shotgun / Rifle | D-Pad Up / Down / Left / Right |

Definitive Edition (PS4) swaps to L2/R2 for aim/fire, R1 for alternate fire, L1 for Survival Instinct.

### 2.4 Accessibility & Remapping

PC supports full keyboard rebinding via Options → Button Mapping. Gamepad controls are fixed on all platforms. Subtitles are available with multiple language pairings. No colorblind mode, FOV slider, QTE assist, or toggle-aim option. Accessibility options are limited compared to later entries in the series.

### 2.5 Context-Sensitive Actions

- **Rope Arrow**: Aim bow at rope-wrapped target to fire. Near a notched post, creates a traversable zipline.
- **Pry Axe**: Interact near barricaded doors to pry open.
- **Climbing Axe**: Interact near craggy rock surfaces to latch on.
- **Torch**: Interact near fire sources to light; dodge button to extinguish.

---

## 3. World Structure

### 3.1 Area Layout

The game takes place on the island of Yamatai across 21 unique locations, some visited multiple times during the story (Mountain Village ×3, Shantytown ×2, Shipwreck Beach ×2, Mountain Pass ×2). Walkthrough guides typically list ~29 entries counting each visit separately. Hub areas are revisitable via fast travel; linear areas are one-time visits with no collectibles.

| # | Area | Type | Notes |
|---|---|---|---|
| 1 | Scavenger's Den | Linear | Tutorial, no return |
| 2 | Coastal Bluffs | Linear | No return |
| 3 | Coastal Forest | Hub | First open area |
| 4 | Mountain Temple | Hub | |
| 5 | Mountain Village | Hub | Largest area, 3 story visits |
| 6 | Base Approach | Hub | |
| 7 | Mountain Base | Hub | |
| 8 | Base Exterior | Hub | |
| 9 | Cliffside Village | Linear | No return |
| 10 | Mountain Pass | Linear | No return |
| 11 | Chasm Monastery | Linear | First Oni encounter |
| 12 | Mountain Descent | Linear | Parachute sequence |
| 13 | Shantytown | Hub | Major combat area |
| 14 | Cavern Entrance | Linear | No return |
| 15 | Geothermal Caverns | Hub | |
| 16 | Solarii Fortress | Linear | Nikolai fight |
| 17 | Fortress Tower | Linear | No return |
| 18 | Summit Forest | Hub | |
| 19 | Gondola Transport | Linear | Transition |
| 20 | Shipwreck Beach | Hub | Major hub, 2 story visits |
| 21 | Cliffside Bunker | Hub | Boris fight |
| 22 | Research Base | Hub | |
| 23 | Chasm Stronghold | Linear | No return |
| 24 | Chasm Shrine | Hub | Point of no return after leaving |
| 25 | Chasm Ziggurat | Linear | Final level |

After completing the game, Lara respawns at Forest Ruins base camp (Coastal Forest) and can fast travel to any hub area to collect remaining items.

### 3.2 Base Camps

Two types:

- **Fast Travel Camps** (tent icon): Skill spending, weapon upgrades, and fast travel to any previously discovered fast travel camp. Fast travel unlocks upon reaching Mountain Village.
- **Day Camps** (campfire icon): Skill spending and weapon upgrades only. No fast travel. Found inside optional tombs and linear areas.

Both types autosave on use.

### 3.3 Traversal

| Mechanic | Requirement | Description |
|---|---|---|
| Ledge shimmy | None | Traverse narrow ledges, jump gaps between ledges |
| Wall scramble | None | Run at marked walls (white paint at top), double-jump to reach higher ledges |
| Ladders | None | Standard climbable surfaces |
| Craggy wall climbing | Climbing Axe | Embed axe into rough rock; climb, shimmy, jump between surfaces |
| Rope slides / Ziplines | Climbing Axe | Slide downhill on ropes using the axe |
| Rope bridges | Rope Arrows | Create traversable ropes between anchor points |
| Rope ascent | Rope Ascender | Travel uphill on ziplines; rapidly pull heavy objects |

No swimming or diving. Lara wades through shallow water; river/current sequences are scripted.

### 3.4 Gear-Gated Progression

Equipment acquired at story milestones gates access to previously blocked paths:

| Gear | Acquired | Gates |
|---|---|---|
| Pry Axe | Coastal Forest | Salvage crates, certain doors, wooden barriers |
| Climbing Axe | Mountain Village | Craggy rock walls, rope slides |
| Fire Striker | Base Exterior | Light torch independently; burn cloth barriers |
| Rope Arrows | Cliffside Village | Rope bridges, pull rope-wrapped objects/doors |
| Shotgun | Chasm Monastery | Wooden barb-wire barricades |
| Fire Arrows | Shantytown | Ignite flammable objects/barriers at range |
| Grenade Launcher | Solarii Fortress | Metal barricades |
| Rope Ascender | Cliffside Bunker | Ascend ropes; pull cement barricades |

### 3.5 Collectibles

| Collectible | Total | Notes |
|---|---|---|
| Documents | 54 | 8 sets, voice-acted, award XP |
| Relics | 42 | 15 sets, 3D-inspectable |
| GPS Caches | 75 | Finding all awards 1,000 salvage + 2 hidden documents |
| Treasure Maps | 7+ | From challenge tombs; reveal all collectibles in area |
| Challenge Tombs | 7 | +1 DLC (Tomb of the Lost Adventurer) |
| Challenges | 13 | 84 individual items across all areas |

**Document Sets (54 total):**

| Set | Count | Subject |
|---|---|---|
| Ancient Scrolls | 10 | Ambassador, Hoshi, and General perspectives on Yamatai's history |
| Lara's Journals | 10 | Lara's personal entries as the crisis unfolds |
| Endurance Officers | 7 | Whitman, Reyes, and Roth's private writings |
| Endurance Crew | 7 | Sam, Grim, Alex, and Jonah's perspectives |
| Diaries of a Madman | 7 | Mathias's descent from castaway to cult leader |
| Wartime Intelligence | 6 | WWII-era scientist and soldier reports on the island |
| Confessions of a Solarii | 5 | Cult members' accounts of life under Mathias |
| GPS Secrets | 2 | Unlocked by finding all 75 GPS caches |

**Relic Sets (42 total across 15 sets):**

| Set | Count | Set | Count |
|---|---|---|---|
| Kanpo Herbs | 3 | Yuan Dynasty Jade | 3 |
| Noh Masks | 3 | Ceremonial Fans | 3 |
| Edo Period Fans | 2 | Vases | 3 |
| Semper Fi | 3 | Daggers | 3 |
| Senshi Elite | 2 | Animal Statuary | 3 |
| Ancient Coins | 3 | Inro | 4 |
| Bronze Coins | 2 | Helmets | 2 |
| A Family Outing | 3 | | |

**Per-area collectible counts**:

| Area | Documents | Relics | GPS Caches | Tombs | Challenges |
|---|---|---|---|---|---|
| Coastal Forest | 6 | 3 | 5 | 1 (DLC) | Ghost Hunter (10) |
| Mountain Temple | 3 | 2 | 2 | — | Pyromaniac (5) |
| Mountain Village | 8 | 5 | 15 | 2 | Illumination (10), Egg Poacher (5) |
| Base Approach | 0 | 2 | 2 | — | — |
| Mountain Base | 3 | 0 | 2 | — | Non-Believer (5) |
| Base Exterior | 2 | 2 | 1 | — | — |
| Shantytown | 5 | 7 | 15 | 2 | Laid to Rest (5), Silencer (4) |
| Geothermal Caverns | 3 | 3 | 5 | — | Firestarter (6) |
| Summit Forest | 3 | 3 | 5 | 1 | Red Cap Roundup (10) |
| Shipwreck Beach | 5 | 5 | 15 | 2 | Cairn Raider (5), Mine Sweeper (10) |
| Cliffside Bunker | 4 | 3 | 5 | — | Previous Inhabitants (4) |
| Research Base | 3 | 2 | 3 | — | Sun Killer (5) |
| Chasm Shrine | 3 | 3 | 0 | — | — |

### 3.6 Challenge Tombs

Seven optional tombs (base game), each a self-contained puzzle with no combat. Every tomb awards **1,250 XP + 250 salvage + 1 treasure map**.

| Tomb | Area | Puzzle Theme |
|---|---|---|
| Tomb of the Unworthy | Mountain Village | Burning sacks to raise lifts; weighted cages |
| Hall of Ascension | Mountain Village | Wind mechanics; timed cranks |
| Well of Tears | Shantytown | Weighted lift platform with containers |
| Chamber of Judgment | Shantytown | Weighted ramp with rope-activated mechanics |
| Stormguard Sanctum | Summit Forest | Fire arrows igniting barricades; flammable gas |
| Temple of the Handmaidens | Shipwreck Beach | Buoy contraptions with rope arrows and timing |
| The Flooded Vault | Shipwreck Beach | Electrical hazards; raft mechanics with rope arrows |
| Tomb of the Lost Adventurer (DLC) | Coastal Forest | Multi-stage fire mechanics |

### 3.7 Area Challenges

Each area challenge requires finding and interacting with specific objects. Each object found awards 10 XP; completing a full challenge awards 150 XP.

| Area | Challenge | Object | Action | Count |
|---|---|---|---|---|
| Coastal Forest | Ghost Hunter | Hanging skull totems | Shoot | 10 |
| Mountain Temple | Pyromaniac | Glass lanterns | Shoot | 5 of 7 |
| Mountain Village | Illumination | Statues with braziers | Light with torch | 10 |
| Mountain Village | Egg Poacher | Bird nests | Collect | 5 |
| Mountain Base | Non-Believer | Sun Queen banners | Burn | 5 |
| Shantytown | Laid to Rest | Cloth effigies | Burn | 5 of 10 |
| Shantytown | Silencer | Crank-powered alarms | Destroy | 4 of 5 |
| Geothermal Caverns | Firestarter | Ceiling sacks | Burn with fire arrows | 6 |
| Summit Forest | Red Cap Roundup | Red-capped mushrooms | Collect | 10 |
| Shipwreck Beach | Cairn Raider | Stone cairns | Dismantle | 5 |
| Shipwreck Beach | Mine Sweeper | WWII naval mines | Shoot until explode | 10 |
| Cliffside Bunker | Previous Inhabitants | Sun-symbol flags | Burn | 4 |
| Research Base | Sun Killer | Hanging totems | Shoot | 5 |

---

## 4. Skills

Three skill trees with three tiers each: **Rookie** (available from start), **Hardened** (unlocks after purchasing any 7 skills total), **Specialist** (unlocks after 14 skills total). Each skill costs 1 skill point. 24 total skills. The Brawler tree is locked until Base Approach.

### 4.1 Survivor (9 skills)

Bonus XP, salvage, and ammo from scavenging.

**Rookie:**

| Skill | Effect |
|---|---|
| Animal Instincts | Spot hard-to-find animals and food sources via Survival Instinct |
| Survivalist | Bonus XP from looting animal corpses and food caches |
| Advanced Salvaging | Extra salvage from crates and caches |
| Bone Collector | Extra salvage from animal bodies |
| Arrow Retrieval | Recover arrows from enemy corpses (+20 XP per retrieval) |
| Scavenging | Discover hidden ammo on enemy corpses (requires Arrow Retrieval) |

**Hardened:**

| Skill | Effect |
|---|---|
| Climber's Agility | Faster climbing, reduced fall damage |
| Orienteering | Nearby collectibles visible through walls via Survival Instinct |

**Specialist:**

| Skill | Effect |
|---|---|
| Cartography | All tomb entrances and map locations revealed on map (requires Orienteering) |

### 4.2 Hunter (8 skills)

Combat effectiveness and weapon proficiency.

**Rookie:**

| Skill | Effect |
|---|---|
| Steady Shot | Extended bow aiming duration for precision shots |
| Ammo Capacity | Increased max ammo for all weapons |
| Accomplished Killer | Greater XP from stylish kills |

**Hardened:**

| Skill | Effect |
|---|---|
| Heavy Lifter | Maximizes ammo capacity (requires Ammo Capacity) |

**Specialist:**

| Skill | Effect |
|---|---|
| Bow Expert | Close-range arrow finishes with bonus XP; headshot reticle for bow |
| Pistol Expert | Point-blank execution; headshot reticle for pistol |
| Rifle Expert | Close-range finishing moves with bonus rewards |
| Shotgun Expert | Close-range blast stops enemies, earns extra rewards |

### 4.3 Brawler (7 skills)

Melee combat, survivability, and dodge mechanics. Locked until Base Approach.

**Rookie:**

| Skill | Effect |
|---|---|
| Pain Tolerance | Increased health / damage resistance |
| Dirty Tricks | Throw dirt/rocks to blind enemies, creating vulnerability window |

**Hardened:**

| Skill | Effect |
|---|---|
| Axe Strikes | Two swift axe strikes to stun enemies |
| Axe Expert | Deadly axe finishing move (requires Axe Strikes) |
| Dodge Counter | After dodging, stab enemy in the knee with an arrow to cripple |
| Dodge Kill | Instant melee execution after successful dodge (requires Dodge Counter) |

**Specialist:**

| Skill | Effect |
|---|---|
| Dodge Kill Mastery | Dodge Kill works on nearly all enemy types (requires Dodge Kill) |

---

## 5. Story & Progression

### 5.1 Structure

Three-act structure across Yamatai island.

**Act One — Survival and Discovery (Scavenger's Den → Cliffside Village)**: The crew of the research vessel *Endurance* shipwrecks on Yamatai. Sam is kidnapped by cult leader Mathias. Lara survives increasingly desperate situations, kills her first human (Vladimir), and transitions from survivor to fighter. Key gear acquired: bow, pistol, climbing axe, rifle, fire striker, rope arrows.

**Act Two — Escalation (Mountain Village 2nd visit → Shipwreck Beach)**: Storms destroy a rescue plane and helicopter. The Oni/Stormguard — undead samurai guardians of Sun Queen Himiko — are revealed as a supernatural threat. Lara witnesses Sam's attempted sacrifice and rescues her. Mathias kills Roth (Lara's mentor) with an axe throw. Key gear acquired: shotgun, fire arrows, grenade launcher, compound bow.

**Act Three — Climax (Cliffside Bunker → Chasm Ziggurat)**: Whitman betrays the group; Sam is re-captured. Lara discovers that Himiko's "Ascension" ritual transfers her soul into a new host body. Lara fights through the Stormguard, defeats the Oni Stalker boss, confronts Mathias in a QTE duel, and destroys Himiko's decaying body with a torch to save Sam. The storms cease; survivors sail away. Key gear acquired: rope ascender.

### 5.2 Key Character Deaths

| Character | Cause | Narrative Weight |
|---|---|---|
| Steph | Killed by Solarii | Early, minimal buildup |
| Grim | Dies aboard Solarii ship | Moderate |
| Alex | Self-sacrifice | Emotional, dies alone |
| Roth | Killed by Mathias (axe throw) | Most impactful death |
| Whitman | Killed by Oni after betrayal | Predictable |

### 5.3 Post-Game

After the finale and credits, the game autosaves. Loading this save spawns Lara at the Forest Ruins camp in Coastal Forest with access to all gear. All hub areas with fast travel camps are available for 100% completion. Linear areas without fast travel camps (Scavenger's Den, Coastal Bluffs, Mountain Pass, etc.) cannot be revisited, but contain no collectibles. Cliffside Bunker is reachable on foot from Shipwreck Beach. No New Game+.

The **Sacred Passage** camp in Chasm Shrine is the point of no return — proceeding past it locks out fast travel until the post-game save.

---

## 6. Items & Equipment

### 6.1 Weapons

Four weapon families, each upgradeable through tiers via weapon parts found in the world. Parts auto-upgrade the weapon at the next camp visit once enough are collected.

**Bow** — Primary stealth/precision weapon. Silent. Chargeable for more damage.

| Tier | Name | How Obtained |
|---|---|---|
| 1 | Makeshift Longbow | Coastal Forest (story) |
| 2 | Recurve Bow | Collect bow parts (sources conflict: 2 or 3 parts; majority say 2) |
| 3 | Compound Bow | Given by Jonah at Shipwreck Beach (story) |
| 4 | Competition Bow | 3 bow parts after Compound Bow |

Ammo: Normal arrows, fire arrows (shared pool), rope arrows (unlimited). Explosive arrows share ammo with grenades.

**Pistol** — Reliable sidearm with good fire rate.

| Tier | Name | How Obtained |
|---|---|---|
| 1 | Semi-Auto Pistol | Mountain Temple (story, first human kill) |
| 2 | Tactical Pistol | 3 handgun parts |
| 3 | Magnum Pistol | 3 more handgun parts |

**Rifle** — Automatic fire for sustained combat.

| Tier | Name | How Obtained |
|---|---|---|
| 1 | WWII Submachine Gun | Mountain Base (story) |
| 2 | Assault Rifle | 3 rifle parts |
| 3 | Commando Rifle | 3 more rifle parts |

**Shotgun** — Devastating at close range.

| Tier | Name | How Obtained |
|---|---|---|
| 1 | Trench Shotgun | Chasm Monastery (story) |
| 2 | Pump-Action Shotgun | 3 shotgun parts |
| 3 | Combat Shotgun | 3 more shotgun parts |

### 6.2 Weapon Upgrades

Purchased at base camps with salvage. Each weapon has a branching upgrade tree; some upgrades require specific weapon tiers or prerequisite upgrades.

**Bow Upgrades** (total ~2,900 salvage):

| Upgrade | Tier | Cost | Prereq | Effect |
|---|---|---|---|---|
| Reinforced Limbs | Makeshift | 250 | — | Increased damage |
| Wrapped String | Makeshift | 200 | — | Faster fire rate |
| Key Ring Trigger | Recurve | 250 | Wrapped String | Further fire rate increase |
| Napalm Arrows | Compound | 350 | — | Fire arrows spread fire pool on impact |
| Plaited String | Compound | 250 | Reinforced Limbs | Increased arrow damage |
| Explosive Arrows | Competition | 650 | — | Arrows with grenade tips |
| Penetrating Arrows | Competition | 650 | — | Charged shots penetrate armor / soft targets |
| Stabilizing Weight | Competition | 300 | Plaited String | Improved accuracy, increased damage |

**Pistol Upgrades** (total ~3,025 salvage):

| Upgrade | Tier | Cost | Effect |
|---|---|---|---|
| Burst Fire Mod | Semi-Auto | 450 | Three-round burst alternate fire |
| Extended Mag | Semi-Auto | 200 | Magazine holds 10 rounds |
| Port Vented Slide | Semi-Auto | 200 | Reduced recoil, increased damage |
| Muzzle Brake | Semi-Auto | 250 | Increased damage |
| Rapid Fire Mod | Semi-Auto | 300 | Hair trigger, faster fire rate |
| Magazine Well | Tactical | 325 | Faster reload |
| Ergonomic Grip | Tactical | 300 | Reduced recoil |
| Silencer | Tactical | 350 | Silent shots, slightly reduced damage |
| High Capacity Mag | Magnum | 250 | Magazine holds 12 rounds |
| Polished Barrel | Magnum | 400 | Increased damage |

**Rifle Upgrades** (total ~3,425 salvage):

| Upgrade | Tier | Cost | Effect |
|---|---|---|---|
| High Capacity Mag | WWII SMG | 225 | +15 rounds |
| Padded Stock | WWII SMG | 300 | Steadier aim while firing |
| Muzzle Brake | WWII SMG | 350 | Reduced recoil |
| Taped Double Mag | WWII SMG | 300 | Faster reload |
| Polished Ejector | WWII SMG | 250 | Faster fire rate |
| Barrel Shroud | Assault | 350 | Increased damage |
| Grenade Launcher | Commando | Free | Story acquisition (Solarii Fortress) |
| Frag Grenades | Commando | 500 | Wider blast radius with shrapnel |
| Match Grade Barrel | Commando | 400 | Increased bullet speed and damage |
| Scope | Commando | 250 | Increased zoom for long range |
| Silencer | Commando | 500 | Silent shots, slightly reduced damage |

**Shotgun Upgrades** (total ~3,100 salvage):

| Upgrade | Tier | Cost | Effect |
|---|---|---|---|
| Full Choke | Trench | 400 | Concentrated pellet spread for range (alt fire) |
| Barrel Shroud | Trench | 325 | Increased damage |
| Polished Bolt | Trench | 300 | Faster cycle between shots |
| Padded Grip | Trench | 300 | Steadier aim, reduced kick |
| Incendiary Shells | Pump-Action | 400 | Pellets ignite enemies |
| Wrapped Stock | Pump-Action | 300 | Steadier aim while firing |
| Shell Mag | Pump-Action | 350 | No individual loading, faster reload |
| Modified Receiver | Combat | 325 | Increased damage |
| Drum Mag | Combat | 400 | Doubles magazine to 12, faster reload |

**Axe Upgrade**: Strengthen (50 salvage) — reinforces the pry axe to operate cranks.

**Grand total all weapon upgrades: ~12,500 salvage.**

### 6.3 Story Gear (Chronological Acquisition)

| # | Gear | Location | Function |
|---|---|---|---|
| 1 | Torch | Scavenger's Den | Lights fires, illuminates dark areas. Initially requires existing fire source. |
| 2 | Makeshift Longbow | Coastal Forest | First weapon. Taken from corpse hanging in a tree. |
| 3 | Pry Axe | Coastal Forest | Opens crates, pries doors, smashes barriers. Found in underground bunker. |
| 4 | Semi-Auto Pistol | Mountain Temple | Second weapon. Taken during Lara's first human kill. |
| 5 | Climbing Axe | Mountain Village | Climb craggy walls, zipline, melee weapon. Given by Roth. |
| 6 | WWII Submachine Gun | Mountain Base | Third weapon. Taken from enemy during "Cry for Help" mission. |
| 7 | Fire Striker | Base Exterior | Light torch anywhere without existing fire source. |
| 8 | Rope Arrows | Cliffside Village | Create ziplines, pull rope-wrapped objects/doors. Unlimited ammo. |
| 9 | Trench Shotgun | Chasm Monastery | Fourth weapon. Found on desiccated corpse. |
| 10 | Lighter | Shantytown | Enables fire arrows permanently. Taken from dead helicopter pilot. |
| 11 | Grenade Launcher | Solarii Fortress | Rifle alternate fire. Ripped from enemy turret. |
| 12 | Compound Bow | Shipwreck Beach | Major bow upgrade. Given by Jonah. |
| 13 | Rope Ascender | Cliffside Bunker | Ascend ropes, pull cement barricades. Taken from Boris (mini-boss). |

---

## 7. Enemies & Opponents

### 7.1 Solarii Brotherhood (Human)

The primary antagonist faction for roughly two-thirds of the game.

| Enemy | Behavior | Counter |
|---|---|---|
| Light Soldier | Fast melee charges with machetes/sickles | Dodge attacks, ranged fire before they close |
| Rifleman | Mid-range SMG fire from cover | Headshots |
| Archer | Ranged bow fire (including fire arrows), maintains distance | Prioritize in group fights; suppresses from safety |
| Shotgunner | Aggressive close-range, rarely uses cover | Headshots, knee shots |
| Sapper | Throws dynamite and Molotovs from behind cover | Shoot explosives mid-air; weak in direct combat |
| Heavy Soldier | Heavily armored, polearm melee | Must shoot off armor pieces first |
| Enforcer | Assault rifle, heavy armor, uses cover well | Shoot off armor; grenade launcher effective |
| Guardsman | Riot shield + lance, slow-moving | Dodge Kill/Counter to bypass shield; flank |
| Elite | Katana + compound bow, minimal armor | Headshots, attrition |

### 7.2 Oni / Stormguard (Undead Samurai)

Late-game enemies guarding Sun Queen Himiko. Significantly tougher than Solarii.

| Enemy | Behavior | Counter |
|---|---|---|
| Oni Warrior | Katana melee, heavily armored with helmets | Shoot off helmet, then headshot; dodge counters |
| Oni Archer | Bow + fire arrows, lighter armor but helmeted | Close distance for melee; headshots after helmet removal |
| Oni Pikeman | Shield + naginata, most defensive enemy type | Dodge skills to bypass shield; cannot be shot frontally |
| Oni Stalker | Nearly twice normal height, massive club (kanabo) | Dodge charges, shoot exposed back; shotgun effective |

### 7.3 Wildlife

- **Wolves**: Hostile. Aggressive pack hunters that do not disengage. Early encounters involve QTE grapple sequences; later appear as standard combat enemies.
- **Deer**: Found in Coastal Forest, Mountain Village. Large animals; first story deer kill awards ~150 XP; subsequent kills award less (~50 XP).
- **Boar**: Found in forested areas. Large animal, comparable XP to deer.
- **Rabbits**: Small, low XP. Found throughout wilderness areas.
- **Chickens**: Small, low XP. Plentiful in Mountain Village.
- **Crows / Seagulls**: Small birds, low XP (~20 XP for gathering bird meat).
- **Rats**: Small, low XP. Found in Mountain Base and interior areas.

Hunting has no survival mechanics — animals are killed purely for XP and salvage. Large animals (deer, boar) yield significantly more than small animals. When an area is "hunted out" (depleted), kills drop to 1 XP. Survivor skills (Animal Instincts, Survivalist, Bone Collector) enhance hunting rewards.

### 7.4 Boss / Mini-Boss Encounters

**Vladimir (Mountain Temple)** — Entirely QTE. Lara's first human kill. Kick → bite → grab dropped gun → struggle → shoot.

**Nikolai (Solarii Fortress)** — Multi-phase set piece. Advance through courtyard under mounted gun fire → climb wall → zipline onto Nikolai → acquire grenade launcher → fire grenade at Nikolai.

**Boris (Cliffside Bunker)** — Armored brute mini-boss. Only the head is vulnerable. Dodge his lunges → melee counter (Lara stabs his leg with an arrow) → shoot exposed head. Repeat cycle until kill QTE.

**Oni Stalker (Chasm Ziggurat)** — Multi-phase boss. Phase 1: Dodge attacks, shoot exposed back. Phase 2: Oni archers spawn as adds; continue targeting Stalker's back. Phase 3: Mask/helmet knocked off, head becomes target; increased aggression. Final QTE melee finisher sequence.

**Mathias (Chasm Ziggurat, Final Boss)** — Entirely QTE immediately following the Oni Stalker. Aim and fire → timed melee axe swing → steady-pace interact to push away (too fast fails on PC) → alternating fire/alternate fire to drive him off the edge.

### 7.5 QTE System

Four input types appear throughout the game:

| Type | Input | Usage |
|---|---|---|
| Circle Event | Single timed press when outer circle reaches inner circle (turns red) | Melee finishers, precision moments |
| Struggle/Wiggle | Alternate Left/Right rapidly | Breaking grapples, escaping holds |
| Interact Prompt | Single press or rhythmic mashing (hand icon) | Grabbing, pushing, maintaining grip |
| Melee/Combat Prompt | Timed press (triangle/exclamation icon) | Kicks, axe strikes, combat finishers |

QTE failure triggers context-specific death animations and respawns at last checkpoint.

---

## 8. Economy

### 8.1 Currencies

| Currency | Earned From | Spent On |
|---|---|---|
| XP | Kills, collectibles, challenges, tombs, story milestones | Skill points (automatic conversion) |
| Salvage | Crates, enemy/animal loot, tombs, GPS cache bonus | Weapon upgrades at base camps |

### 8.2 Salvage Budget

Total weapon upgrade cost is ~12,500 salvage. Total salvage available in the game exceeds this if explored thoroughly. Challenge tombs alone provide 1,750 salvage (7 × 250). The GPS cache completion bonus adds 1,000. Survivor skills further increase yield.

There are no shops, merchants, or trading systems.

---

## 9. Multiplayer

Developed by Eidos Montreal. Supports up to 8 players (4v4 in team modes). Two factions: **Survivors** (Lara's crew) and **Solarii** (island cult).

### 9.1 Game Modes

| Mode | Structure | Win Condition |
|---|---|---|
| Team Deathmatch | 4v4, best of 3 rounds, 10-min timer | First to 25 kills or most kills at time |
| Rescue | 4v4 asymmetric, best of 3 | Survivors: deliver 5 medical supply kits. Solarii: 20 kills. Solarii must finish downed Survivors with melee. |
| Cry for Help | 4v4 asymmetric, best of 3 | Survivors: activate 3 of 4 radio transmitters (zone capture, faster with more players). Solarii: collect 20 batteries from killed Survivors. |
| Free for All | 8 players, no teams | Score-based. "Executioner" mechanic: kill streak grants map vision of all players; others stop respawning; Executioner wins by eliminating all. |

### 9.2 Maps

5 base maps (Beach, Chasm, Monastery, Shrine, Underground) + 8 DLC maps (Caves, Cliffs, Village, Shanty Town, Lost Fleet, Forest, Himiko's Cradle, Dogfight).

### 9.3 Loadout System

Each loadout: primary weapon + secondary weapon + explosive/gadget + two skills (one offensive, one survival).

**Survivor primaries**: Combat Shotgun, Modified SMG, Survivor Recurve Bow, Commando Rifle, Full-Auto Shotgun, LMG.
**Solarii primaries**: Solarii Recurve Bow, Trench Shotgun, Crossbow, Marksman Rifle, Double-Barrel Shotgun, Compound Bow.

**Offensive skills**: Increased Ammo Capacity, Harder Hits, Spite, Rancor, Second Wind, Speedloader, Steady Shot, Eagle Eye.
**Survival skills**: Thick Skin, Pincushion, Bulletproof, Fireproof, Grenadier, Tracker, Lightfoot, Rejuvenation.

### 9.4 Environmental Traps (Multiplayer)

Multiplayer maps include interactive hazards: bear traps, spike traps, rope traps, and lightning rods. Players can set traps intentionally. Destructible environmental elements (explosive barrels) and traversal mechanics (climbing, ziplines, rope swinging) carry over from single-player.

### 9.5 Progression

Max level 60. Kills, objectives, and trap kills award XP. Salvage currency purchases unlocked items. **Prestige system**: Reset to level 1 after 60, keeping characters and salvage but re-locking weapons/skills. Multiple prestige tiers (The General character requires Prestige 3).

### 9.6 Characters

**Survivors** (12+): Alex, Jonah, Liam, Roth, Samantha (default); Grim (Lv.8, 250 salvage), Reyes (Lv.16, 500), Steph (Lv.24, 1000), Victor (Lv.32, 2000), Whitman (Lv.40, 3000), Pilot (Lv.48, 5000), Lara (Lv.60, 10000).

**Solarii** (15+): Creeper, Father Mathias, Smoker, Bruiser, Saboteur, Soldier, Archer, and others at various unlock levels. The General at Prestige 3.

---

## 10. UI & HUD

### 10.1 HUD Layout

Minimalist, contextual design. Most elements appear only when relevant and fade during exploration.

- **Health**: No health bar. Screen desaturates toward greyscale on damage; blood splatters at edges; labored animations when near death.
- **XP Meter**: Shield-shaped meter on the left side of screen. Appears briefly when XP is earned, then fades. Fills to award 1 skill point.
- **Ammo Counter**: Top-right corner when weapon is drawn or aiming. Shows magazine / reserve. Last few rounds highlighted in red.
- **Weapon Indicator**: Near ammo counter when drawn.
- **Reticle**: Standard targeting reticle while aiming. Changes to red dot with radiating lines for rope arrow targets. Hunter Expert skills add headshot reticle.
- **Objective Beacon**: Column of light marking next story destination, visible through Survival Instinct. Custom waypoints appear as blue columns.
- **Interaction Prompts**: Contextual button icons near interactable objects.
- **Autosave Indicator**: Small "TR" logo at bottom-right corner during saves.
- **Pickup Notifications**: Brief pop-ups for salvage, weapon parts, documents, relics, GPS caches.

### 10.2 HUD States

- **Exploration**: Nearly clean screen. XP meter appears briefly on earn, then fades. Interaction prompts near objects.
- **Combat**: Ammo counter and weapon indicator persist while aiming. Reticle active. Damage effects provide health feedback.
- **Cutscenes**: All HUD removed. Seamless transitions between gameplay and cutscenes.

### 10.3 Survival Instinct

Hold-to-activate vision mode (Q / LB / L2). No cooldown, no duration limit. World desaturates; key objects glow:

- **Interactables, ammo, collectibles, loot**: Yellow/gold glow
- **Enemies (safe to stealth kill)**: White glow
- **Enemies (kill will alert others)**: Red glow
- **Objective Beacon**: Tall column of light toward next story goal
- **Custom waypoint**: Blue column of light

Enhanced by skills: Animal Instincts (highlights prey), Orienteering (collectibles visible through walls), Cartography (tomb entrances on map).

On Hard difficulty, enemy highlighting is disabled.

### 10.4 Menu Screens

**Base Camp Menu**: Skills screen (three-column layout: Survivor / Hunter / Brawler, each with Rookie / Hardened / Specialist tiers) | Gear screen (visual weapon diagram with branching upgrade nodes, salvage costs shown) | Fast Travel (map with discovered camps, fast travel camps only).

**Map Screen** (Tab / Back / Select): Top-down area map. Diamond icon for next objective. Campfire/tent icons for camps. Treasure maps reveal all collectible locations. Collectible completion counters per area. Custom waypoint placement.

**Relic Viewer**: 3D-inspectable artifacts with rotation. Documents are voice-acted and award XP.

---

## 11. Engine & Presentation Systems

### 11.1 Difficulty Settings

Three levels. Difficulty only affects combat — traversal, puzzles, and exploration are identical. Can be changed at any time.

| Setting | Easy | Normal | Hard |
|---|---|---|---|
| Aim Assist | On | Off | Off |
| Ammo Frequency | Plentiful | Rare | Rare |
| Enemy Health | Reduced | Standard | Increased |
| Enemy Damage | Reduced | Standard | Increased |
| Enemy Detection Speed | Standard | Standard | Faster |
| Survival Instinct Enemies | Highlighted | Highlighted | Not shown |
| Health Regen in Combat | Yes | Yes | No |
| Hit Marker on Reticle | Yes | Yes | No |

### 11.2 Save System

Autosave only — no manual save button. Saves trigger at: area transitions, combat encounter completion, story triggers, base camp use, key item pickup. "TR" logo appears at bottom-right during saves. Save slots: 3 on console, up to 99 on PC (post-patch 1.0.730.0), switchable from pause menu via "Change Save Slot." Death reloads last checkpoint with unlimited retries.

### 11.3 Camera

Third-person, slightly behind and above Lara's right shoulder. Aiming shifts to tighter over-the-shoulder view with shoulder swap and zoom. Lara auto-adjusts posture (crouches in tight spaces, leans against cover). Set-piece sequences use cinematic camera angles with simplified player input. Seamless transitions between gameplay and cutscenes create a "continuous shot" feel.

### 11.4 Audio

Composed by Jason Graves. A custom-built physical instrument (wood, glass, metal — played percussively or with a bow modeled after Lara's longbow) is the sole music source for the first 15 minutes, establishing the island's sonic identity before orchestral elements enter.

**Dynamic music system**: Exploration uses sparse piano/strings/ambient tones. Combat layers in percussion, brass, and stingers scaled to encounter severity. Stealth maintains tense but restrained layering. Scripted sequences use full orchestral scoring. Transitions use micro-scoring — short phrases bridging states instantly.

Leitmotif approach: Lara's main melody was composed first; all other character/faction/location themes derive their notes from hers.

Each area has distinct ambient soundscapes (mountain wind, dripping caves, jungle wildlife, ocean, industrial Solarii camp sounds).

### 11.5 Definitive Edition (PS4/Xbox One, January 2014)

Developed by United Front Games and Nixxes Software. **No gameplay changes** — purely visual and input enhancements.

- 1080p native resolution on both consoles
- TressFX hair rendering (AMD realistic hair physics)
- Remodeled Lara face with wider facial animation range
- Sub-surface scattering on skin and translucent materials
- Higher-res textures, improved particles and lighting
- Physics applied to Lara's gear (bow, axe sway with movement)
- PS4: unlocked framerate (~53fps average); Xbox One: locked 30fps
- PS4: DualShock 4 light bar changes color (red/orange for torch, flashes when shooting), touchpad for map, Remote Play
- Xbox One: Kinect voice commands for weapon switching and menu navigation, hand gestures for relic inspection
- All DLC bundled: Tomb of the Lost Adventurer, six outfits, eight MP maps, six MP weapons, four MP characters, digital comic and art book
- Backward compatible with PS5 and Xbox Series X|S

---

## 12. Open Questions / Unverified

- **XP-per-level thresholds**: Exact XP required for each successive skill point is not publicly documented. Thresholds appear to scale upward but no source provides the formula or table.
- **Weapon damage values**: No source provides numeric damage stats per weapon or per upgrade. Damage is described qualitatively ("increased damage") without values.
- **Health values**: Exact HP for Lara and all enemy types are undocumented.
- **Ammo capacities**: Magazine sizes are documented through upgrades (pistol Extended Mag: 10, Magnum High Capacity: 12, rifle High Capacity: +15, shotgun Drum Mag: 12). Base reserve capacities and post-skill totals (Ammo Capacity, Heavy Lifter) are not publicly documented.
- **Recurve Bow part count**: Sources conflict — majority say 2 bow parts for Makeshift → Recurve upgrade, but some guides list 3. Competition Bow consistently requires 3 parts.
- **Total salvage available**: Confirmed sufficient for all upgrades with thorough exploration, but the exact total is not documented.
- **Weapon part locations**: Handgun parts are confirmed to come primarily from Challenge Tombs; rifle and shotgun part locations are less precisely documented across sources.
- **Stealth detection model**: Exact sight cone angles, sound propagation distances, and awareness thresholds are not documented.
- **Hunting XP scaling**: First deer kill awards ~150 XP, subsequent kills less (~50 XP). Exact per-species XP tables are not documented. "Hunted out" areas drop to 1 XP per kill.

---

## 13. References

### Wikis
- Tomb Raider Wiki (Fandom) — weapon upgrades, skill trees, enemy types, area lists, collectible counts, gear acquisition order
- WikiRaider (wikiraider.com) — weapon part details, base camp lists, animal/hunting data, XP mechanics
- StrategyWiki — Tomb Raider (2013) walkthrough and area data

### Guides
- Stella's Tomb Raider Site (tombraiders.net) — definitive walkthrough; document/relic set inventories, upgrade costs, gear chronology, 100% completion tracking
- GameFAQs — user-written FAQs covering collectibles, challenge locations, weapon part locations
- IGN Tomb Raider (2013) Wiki Guide — area walkthroughs and challenge tomb solutions
- GamePressure — equipment guide and upgrade documentation

### Technical / Presentation
- GDC 2014 — Jason Graves and Alex Wilmer presentation on Tomb Raider dynamic audio and "The Instrument"
- Digital Foundry — Definitive Edition performance analysis (PS4 vs Xbox One)
- PCGamingWiki — PC-specific settings, key remapping, accessibility, save slot details

### Community
- Steam community guides — save system behavior, difficulty breakdowns, multiplayer mechanics
- Speedrun community resources — QTE timing and checkpoint behavior documentation
