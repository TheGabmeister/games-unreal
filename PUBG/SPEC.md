# PUBG: Battlegrounds — Gameplay Systems Spec

PC (primary), Xbox, PlayStation. Originally released March 2017 (Early Access), full release December 2017. Developed by KRAFTON (formerly PUBG Corporation / Bluehole). Transitioned to free-to-play in January 2022; the paid Battlegrounds Plus upgrade ($12.99) unlocks Ranked mode, custom matches, career stats, and a cosmetic starter pack. F2P players have full access to Normal and Arcade modes.

---

## 1. Core Gameplay Systems

### 1.1 Battle Royale Loop

Up to 100 players parachute onto an island, scavenge weapons and equipment from the environment, and fight to be the last player (or team) standing. The playable area shrinks over time via a lethal "Blue Zone" barrier, forcing survivors into increasingly tight encounters.

**Match phases:**

1. **Lobby/Spawn Island** — players wait in a staging area (~60s).
2. **Cargo plane** — flies a randomized linear path across the map. Players choose when to jump.
3. **Freefall/Parachute** — max freefall speed ~234 km/h (dive straight down + W). Parachute auto-deploys at ~300m AGL; manual deploy with F. Max lateral glide distance ~3 km using the wave technique (alternate W/release to sustain 20–25 km/h vertical).
4. **Loot phase** — scavenge buildings and ground spawns for weapons, armor, healing, attachments.
5. **Combat/survival** — engage other players, manage health/boost, rotate with the shrinking zone.
6. **Endgame** — final circles force direct confrontation. Last surviving player/team wins ("Winner Winner Chicken Dinner").

Normal matches backfill lobbies with **AI bots** to ensure fast queue times. Bot ratio decreases as player account level increases. Ranked mode has zero bots.

### 1.2 Blue Zone (Playzone)

A circular safe zone that shrinks in phases. Players outside it take escalating damage per second. Each phase has a wait period (zone visible on map), then a shrink period (wall moves inward). Late-game phases deal lethal damage within seconds.

**Erangel (8×8 km) — 9 phases:**

| Phase | Wait | Shrink | Diameter | Dmg/sec |
|-------|------|--------|----------|---------|
| 1 | 270s | ~150s | 3,994m | 0.4% |
| 2 | 180s | ~300s | 2,397m | 0.6% |
| 3 | 130s | ~220s | 1,318m | 0.8% |
| 4 | 120s | ~180s | 725m | 1.0% |
| 5 | 100s | ~160s | 363m | 3.0% |
| 6 | 90s | ~120s | 181m | 5.0% |
| 7 | 70s | ~100s | 91m | 7.0% |
| 8 | 60s | ~90s | 45m | 9.0% |
| 9 | 30s | ~90s | 0m (closes) | 11.0% |

Total match duration ~32–33 minutes. Phase 1 begins after a ~120s flight delay.

Blue zone damage scales with distance from the safe zone edge — players farther outside take more damage per tick.

**Sanhok (4×4 km)** uses a dynamic circle that adjusts speed based on surviving player count. Faster pace (~25 min total). Final phase damage: 16%/sec.

**Miramar (8×8 km)** runs slightly longer (~36 min) with wider early circles.

### 1.3 Red Zone

Random artillery bombardment that spawns periodically within the current safe zone. Duration ~20–30 seconds. Direct bomb hit is instant kill. Players are safe inside solid buildings. Designed to force movement and mask gunfire audio. Removed from Ranked mode.

### 1.4 Care Packages (Air Drops)

Dropped from a C-130 at semi-random intervals. Visible descent with white flashing light; red smoke on landing.

**Crate contents:**
- Crate-exclusive weapons: AWM, Lynx AMR, Groza, AUG A3, Mk14 EBR, DBS, MG3, P90
- Equipment: Level 3 Helmet (guaranteed), Level 3 Vest, Level 3 Backpack, Ghillie Suit
- Consumables: Adrenaline Syringe, Med Kit
- Attachments: 8× CQBSS, 15× PM II, suppressors

**Flare Gun** — world-spawn item. Firing inside the safe zone summons a super-airdrop (equivalent to 2 normal crates). Firing outside the safe zone summons a BRDM-2 armored vehicle.

### 1.5 Health System

**Base HP:** 100

**Healing items:**

| Item | Heal | Cast Time | Max Heal To | Weight |
|------|------|-----------|-------------|--------|
| Bandage | +10 HP over 3s | 4.0s | 75 HP | 2 |
| First Aid Kit | Sets HP to 75 | 5.9s | 75 HP | 10 |
| Med Kit | Sets HP to 100 | 7.9s | 100 HP | 20 |

First Aid Kit and Med Kit cannot be used while moving. Multiple bandages can stack but only heal to 75 HP.

**Boost items** fill a boost bar (0–100, 4 segments) that provides passive healing over time:

| Item | Boost Added | Cast Time | Weight |
|------|------------|-----------|--------|
| Energy Drink | +40 | 4.0s | 4 |
| Painkiller | +60 | 5.9s | 10 |
| Adrenaline Syringe | +100 (full bar) | 5.9s | 20 |

Adrenaline Syringe is primarily crate loot.

**Boost bar tiers:**

| Tier | Range | Heal Rate | Speed Bonus |
|------|-------|-----------|-------------|
| 1 | 0–25 | 1% per 8s | None |
| 2 | 26–50 | 2% per 8s | None |
| 3 | 51–75 | 3% per 8s | +2.5% |
| 4 | 76–100 | 4% per 8s | +6.2% |

The bar depletes over time. Energy Drink provides ~23% total healing over 2 min; Painkiller ~40% over 3 min; Adrenaline Syringe ~68% over 5 min.

### 1.6 Damage Rules

- **Fall damage:** scales with fall height. Falls above ~4m begin dealing damage; lethal above ~8–10m. Parachuting onto steep slopes can trigger fall damage.
- **Friendly fire:** enabled in all squad/duo modes. Teammate damage is full (no reduction). Repeated teamkilling triggers automated penalties (§14.4).
- **Drowning:** players die after ~20 seconds underwater without surfacing.

### 1.7 DBNO (Down But Not Out)

In Duo and Squad modes, lethal damage puts a player into a knocked state instead of killing them (requires at least one living, non-DBNO teammate).

- DBNO players can crawl at reduced speed and swim at half speed.
- Teammate revive: hold F nearby, ~3 seconds. Revived player has 10% HP.
- Progressive bleed-out timers per knock in a match: 1st = 90s, 2nd = 30s, 3rd = 30s, 4th = 10s, 5th+ = 7s down to 2s.
- Timer pauses during active revive.
- Teammates can pick up and carry DBNO players (reduces bleed-out timer while carried).
- If the last standing teammate goes down, all DBNO players die instantly.
- **Self AED** (Taego exclusive): world-loot item allowing self-revive without a teammate.

---

## 2. Controls & Input

### 2.1 PC Keyboard + Mouse (Defaults)

**Movement:**

| Action | Key |
|--------|-----|
| Move | W / A / S / D |
| Sprint | Left Shift (hold) |
| Walk (quiet) | Left Ctrl (hold) |
| Jump / Vault / Climb | Space |
| Crouch (toggle) | C |
| Prone | Z |
| Auto-Run | = |
| Free Look | Left Alt (hold) |
| Toggle FPP/TPP | V |

**Combat:**

| Action | Key |
|--------|-----|
| Fire | LMB |
| ADS / Aim | RMB (toggle or hold, configurable) |
| Reload | R |
| Cycle Firing Mode | B |
| Lean Left / Right | Q / E |
| Hold Breath (scoped) | Left Shift |
| Cook Grenade | R (while holding throwable) |
| Zeroing Up / Down | Page Up / Page Down |

**Weapons:**

| Action | Key |
|--------|-----|
| Primary 1 / Primary 2 / Sidearm | 1 / 2 / 3 |
| Melee / Throwable | 4 / 5 |
| Holster (unarmed) | X |
| Healing quickslots | 7 (Med Kit), 8 (First Aid), 9 (Bandage), 0 (Boost) |

**Vehicles:**

| Action | Key |
|--------|-----|
| Accelerate / Brake | W / S |
| Steer | A / D |
| Handbrake | Space |
| Boost | Left Shift |
| Horn | LMB |
| Toggle Engine | Z |
| Switch Seats | Ctrl+1 through Ctrl+4 |

**Communication:**

| Action | Key |
|--------|-----|
| Push to Talk | T |
| Radio Message Wheel | Mouse Wheel Click / F3 |
| Emote Wheel | F4 |
| Switch Voice Channel | Ctrl+Y |

### 2.2 Context-Sensitive Inputs

- **ADS vs Hip-Fire:** RMB is configurable as toggle or hold. A separate "Aim" mode provides over-the-shoulder third-person targeting without full scope.
- **Hold Breath:** Left Shift while scoped steadies the reticle. Drains a breath meter; when empty, sway increases sharply.
- **Lean/Peek:** Q/E only active when aiming or in FPP mode.
- **Vehicle vs on-foot:** controls remap automatically upon entering/exiting vehicles. Motor Glider uses W/S for pitch, A/D for roll.

---

## 3. World Structure

### 3.1 Maps

| Map | Size | Players | Setting | Released | Unique Features |
|-----|------|---------|---------|----------|-----------------|
| Erangel | 8×8 km | 100 | Eastern European military island | Mar 2017 | The original; Military Base, Sosnovka Island, ferries |
| Miramar | 8×8 km | 100 | Central American desert | Dec 2017 | Long sightlines, Hacienda del Patron |
| Sanhok | 4×4 km | 100 | Southeast Asian jungle | Jun 2018 | Dynamic circle, Bootcamp, loot trucks |
| Vikendi | 6×6 km | 100 | Adriatic snow island | Dec 2018 | Train system (11 stations), snow footprint tracking |
| Karakin | 2×2 km | 64 | North African desert | Jan 2020 | Black Zone (destroys buildings), tunnels, Sticky Bombs |
| Paramo | 3×3 km | 64 | South American volcano | Oct 2020 | Dynamic terrain (landmarks relocate), lava (5 dmg/0.5s), helicopter drops |
| Haven | 1×1 km | 32 | Industrial urban | Dec 2020 | PvPvE — AI "Pillar" faction (guards, commander, scout heli, tactical truck) |
| Taego | 8×8 km | 100 | South Korean island | Jul 2021 | Comeback BR (eliminated players fight in arena, winners redeploy Phase 3), Self AED |
| Deston | 8×8 km | 100 | North American urban/swamp | Jul 2022 | Ascender/descender cables, jump towers, security key rooms, airboat |
| Rondo | 8×8 km | 100 | Chinese bamboo/arid terrain | 2024 | Fully accessible terrain, abundant vehicles, biome contrast |

Karakin, Paramo, and Haven have been rotated out of the standard map pool at various times and may only be available in custom matches or limited-time events depending on the current season.

**Map-specific spawns:** some vehicles and weapons are exclusive to certain maps. Examples: Win94 (Miramar only), G36C (Vikendi only), QBU (Sanhok only), MP5K (Vikendi only), Snowmobile/Zima (Vikendi only), Tukshai (Sanhok only), Loot Trucks (Sanhok).

### 3.2 Weather

Several maps support weather variants (fog, rain, overcast, clear) that affect visibility and audio propagation.

### 3.3 Map Rotation

Maps rotate by region on a schedule set by KRAFTON. A map preference feature allows players to select up to 5 preferred maps.

---

## 4. Movement & Traversal

### 4.1 Movement Speeds

| Stance | Speed (m/s) | % of Sprint |
|--------|-------------|-------------|
| Standing Sprint | 6.3 | 100% |
| Crouch Sprint | 4.8 | 76% |
| Standing Run | 4.7 | 75% |
| Crouch Run | 3.4 | 54% |
| Standing Walk | 1.7 | 27% |
| Crouch Walk | 1.3 | 21% |
| Prone Crawl | 1.2 | 19% |
| Swimming (surface) | 2.9 | 46% |

### 4.2 Weapon Weight on Sprint Speed

| Weapon Type | Sprint Speed | Penalty |
|-------------|-------------|---------|
| Unarmed | 6.30 m/s | Baseline |
| Pistol | 6.32 m/s | +0.3% (fastest) |
| SMG | 6.15 m/s | −2.3% |
| Assault Rifle | 6.01 m/s | −4.6% |
| Sniper/DMR | 5.99 m/s | −4.8% |
| Shotgun | 5.91 m/s | −6.1% |

### 4.3 Special Movement

- **Vaulting/Climbing:** context-sensitive on approach; non-cancellable animation. Low vault, high vault, and climb variants based on obstacle height.
- **Leaning/Peeking:** tilts the character left or right while aiming, exposing less body around cover. Can be combined with strafing ("jiggle peeking").
- **Free Look:** 360-degree camera rotation independent of movement direction. Does not affect heading.
- **ADS Movement:** significantly reduced speed; penalty scales with scope magnification.
- **Swimming:** surface speed 2.9 m/s. Players can dive for up to ~20 seconds before drowning; full breath recovery takes ~9 seconds after surfacing.

---

## 5. Loot System

### 5.1 World Spawns

Fixed spawn-point locations exist across all maps (buildings, rooms, rooftops, ground). Items are randomized each match from weighted loot tables. Not every spawn point activates every match.

**Loot tier zones:** certain named locations (Military Base on Erangel, Hacienda on Miramar) have elevated probabilities for better weapons and equipment.

### 5.2 Spawn Categories

Weapons, Attachments (scopes, grips, muzzles, magazines, stocks), Armor (Level 1–3 helmets and vests), Backpacks (Level 1–3), Ammo, Consumables, Throwables.

### 5.3 Special Loot Mechanics

- **Loot Trucks:** mobile AI-driven trucks; 4 per match at designated spawn points. Destroy them for high-tier loot.
- **Flare Gun:** fires into sky to summon special drop (§1.4).
- **Secret Rooms:** map-specific locked rooms with guaranteed high-tier gear; require world-spawn keys.

---

## 6. Weapons & Equipment

Full weapon stats, damage model, attachment details, armor values, ballistics, and throwable data are in the companion document: [docs/weapons.md](docs/weapons.md).

### 6.1 Weapon Categories

- **Assault Rifles (13):** AKM, Beryl M762, ACE32, Groza*, Mk47 Mutant, M16A4, M416, SCAR-L, QBZ95, AUG A3*, G36C, K2, FAMAS
- **DMRs (8):** Mini 14, QBU, Mk12, SKS, SLR, Dragunov, Mk14 EBR*, VSS Vintorez
- **Sniper Rifles (6):** Win94, Kar98k, Mosin-Nagant, M24, AWM*, Lynx AMR*
- **SMGs (9):** Micro UZI, Vector, MP9, JS9, MP5K, P90*, PP-19 Bizon, UMP45, Tommy Gun
- **Shotguns (6):** S686, S1897, S12K, DBS*, Sawed-Off, O12
- **LMGs (3):** DP-28, M249, MG3*
- **Pistols (7):** P92, P18C, Skorpion, P1911, R1895, R45, Deagle
- **Crossbow (1):** Crossbow
- **Melee (4):** Pan, Machete, Crowbar, Sickle

\* = Crate-exclusive

### 6.2 Tactical Gear Slot

A dedicated equipment slot (replaces the sidearm) for utility items:

- **EMT Gear:** reduces healing item cast times by ~50%.
- **Drone Tablet:** deploys a remote-controlled drone for scouting; can pick up and deliver loot.
- **Spotter Scope:** 4× binoculars that mark spotted enemies for teammates without scope glint.
- **Blue Zone Grenade Launcher:** fires a grenade that creates a small temporary blue zone field on impact.
- **Tactical Pack:** extra backpack capacity; allows carrying a third primary weapon.

Only one Tactical Gear item can be equipped at a time.

---

## 7. Vehicles

| Vehicle | Type | Seats | Top Speed | HP |
|---------|------|-------|-----------|----|
| Motorcycle | Two-wheel | 2 | 152 km/h | ~1,025 |
| Motorcycle w/ Sidecar | Three-wheel | 3 | 130 km/h | ~1,025 |
| Dirt Bike | Two-wheel | 1 | 140 km/h | — |
| Buggy | Off-road | 2 | 100 km/h | ~1,540 |
| UAZ (Open/Closed Top) | Off-road | 5 | 105–115 km/h | ~1,820 |
| Dacia 1300 | Sedan | 4 | 139 km/h | ~1,820 |
| Mirado | Coupe | 4 | 152 km/h | ~2,000 |
| Coupe RB | Coupe | 2 | 150 km/h | ~1,800 |
| Pickup / Rony | Truck | 4 | 110–115 km/h | — |
| Van | Bus | 6 | 110 km/h | ~1,680 |
| Tukshai | Three-wheel | 3 | — | — |
| Zima | Snow off-road | 4 | — | — |
| Snowmobile | Snow | 2 | — | — |
| Mountain Bike | Bicycle | 1 | — | — |
| BRDM-2 | Armored | 4 | — | Very High |
| PG-117 | Boat | 5 | 90 km/h | ~1,520 |
| Aquarail | Jet ski | 2 | 90 km/h | — |
| Airboat | Amphibious | — | — | — |
| Motor Glider | Aircraft | 2 | — | — |

### 7.1 Vehicle Mechanics

- All fuel-based vehicles consume fuel. Refuel with gas cans found in the world.
- Vehicle HP depletes from gunfire, collisions, and falls. At 0 HP: engine disables, catches fire, explodes after ~5 seconds.
- Exploding vehicles kill all passengers and nearby players.
- **Roadkill:** running over players at sufficient speed is lethal.
- **Tire destruction:** individual tires can be shot out, degrading handling.
- **BRDM-2:** heavily armored, amphibious, bulletproof windows. Summoned via Flare Gun outside the safe zone.
- **Motor Glider:** requires ~65–70 km/h ground speed for takeoff. Two seats (pilot + passenger who can shoot).

---

## 8. Inventory & Economy

### 8.1 Inventory (Weight-Based)

| Equipment | Capacity |
|-----------|----------|
| Base (no equipment) | 20 |
| Utility Belt | +50 |
| Vest (any level) | +70 |
| Backpack Level 1 | +170 |
| Backpack Level 2 | +220 |
| Backpack Level 3 | +270 |
| **Maximum** | **410** |

**Inventory mechanics:**
- Drag-and-drop interface (Tab/I to open).
- Proximity loot: nearby ground items listed on the left side of inventory screen.
- Auto-equip: weapons and equipment auto-equip to empty slots on pickup; attachments auto-attach.

### 8.2 In-Match Economy

No currency system within matches. All items are found as world loot or in care packages. No trading between players.

### 8.3 Meta Economy

- **G-COIN:** premium currency for cosmetics (purchased with real money).
- **BP (Battle Points):** earned from match performance; spent on cosmetic crates.
- **Survivor Pass:** seasonal battle pass with free and premium tracks (§9.3).

---

## 9. Game Modes & Progression

### 9.1 Game Modes

**Core Battle Royale:**
- **Solo:** 1 player, no DBNO — death is instant. Up to 100 players.
- **Duo:** 2-player teams. DBNO enabled. Up to 100 players.
- **Squad:** 4-player teams (queue with 1–4; optional auto-fill). DBNO enabled. Up to 100 players.

**Ranked Mode:**
- Squad-based, 64 players (no bots).
- Curated ruleset: increased loot, no Red Zone, faster early circles, seasonal map pool.
- Requires Survival Mastery Level 80 and Battlegrounds Plus for F2P accounts.

**Arcade Modes:**
- **Team Deathmatch (TDM):** 8v8, first to target kills wins; instant respawn; small arena maps.
- **War Mode:** 30-minute deathmatch with parachute respawns.
- Rotating arcade modes (BlueBomb Rush, etc.).

**Training Mode (Camp Jackal):**
- 2×2 km island, 5–20 players, 30-minute sessions.
- Players cannot die (HP floors at 1).
- 400m, 800m, and 1000m shooting ranges. CQC course. Vehicle tracks. Parachute practice.
- All weapons, attachments, and vehicles available.

**Custom Matches:**
- Full configuration: map, weather, blue zone per-phase timing/damage, red zone toggle, loot density, starting equipment, spectator slots.

### 9.2 Ranked System

**8 tiers:** Bronze → Silver → Gold → Platinum → Crystal → Diamond → Master → Survivor

Each tier has 4 divisions (IV–I) except Survivor (top players only).

**RP (Rank Points):**
- Range: −44 to +44 per match.
- Factors: survival time, kills (weighted most), assists, damage, final placement.
- 5 placement matches determine initial rank.
- Unified across Duo/Squad and FPP/TPP — single shared rank.

**Rank Decay (after 7 days inactive):**

| Tier | Daily RP Loss |
|------|--------------|
| Bronze / Silver | 2 |
| Gold | 3 |
| Platinum | 4 |
| Crystal | 5 |
| Diamond | 6 |
| Master / Survivor | 7 |

Cannot decay below a tier's minimum floor.

### 9.3 Survivor Pass

- 3 tracks × 30 levels + 1 bonus track × 10 levels (max level 70).
- Duration: 4 weeks per pass.
- XP from daily missions, weekly missions, and playtime.
- Rewards: Survivor Points, G-COIN, schematics, cosmetics, Survivor's Chests (premium only).

### 9.4 Other Progression

- **Survival Mastery:** overall account level from cumulative match performance.
- **Weapon Mastery:** per-weapon progression tracking kills, headshots, longest kill distance.

---

## 10. UI & HUD

### 10.1 HUD Layout

- **Compass (top center):** horizontal strip, bearings 0–360. Cardinal/intercardinal labels. Orange ticks mark nearby gunshot directions. Teammate pings visible from all directions.
- **Minimap (bottom right):** circular; shows player position, facing direction, teammate positions (color-coded), safe zone / blue zone boundaries. Full map via M.
- **Health bar (bottom center/left):** 0–100 HP horizontal bar.
- **Boost bar:** directly above health bar, 4 segments.
- **Armor icons:** helmet and vest with level indicator (Lv1/2/3) and remaining durability.
- **Weapon slots (bottom right, above minimap):** two primary + one sidearm displayed vertically. Active weapon highlighted with firing mode icon, ammo count (magazine/reserve).
- **Kill feed (top right):** scrolling elimination log. Killer name, weapon icon, victim name. Own kills highlighted.
- **Alive counter (top right):** surviving player count.
- **Zone timer (top center / near minimap):** countdown to next zone phase.
- **Teammate status (left side):** squad names with individual health bars, knock status, connection indicators. Numbered and color-coded (yellow, blue, green, orange).

### 10.2 HUD States

- **Normal gameplay:** full HUD visible.
- **ADS/Scoped:** scope overlay replaces center screen; peripheral HUD remains.
- **DBNO:** bleed-out timer visible; crawl-only movement indicated.
- **Spectating:** observer camera controls replace gameplay HUD; kill feed and player list shown.
- **Vehicle:** speedometer replaces weapon info; fuel gauge shown.
- **Inventory screen:** full-screen overlay with equipped gear, ground loot, and drag-and-drop management. Replaces gameplay view.

### 10.3 Map Screen

Full-screen map (M key). Shows: current blue zone, next white zone circle, zone phase timer, teammate markers (color-coded), custom markers (Insert key), care package locations (after landing), vehicle locations (some maps).

---

## 11. Communication

### 11.1 Voice Chat

- **Team voice:** always available to squad/duo mates regardless of distance.
- **Proximity voice (PC only):** audible to all nearby players (enemies included) within ~100m. Toggle channel with Ctrl+Y.
- Push-to-talk (T) or open mic (configurable).

### 11.2 Ping System

- **Map marker:** Insert key places a colored marker visible to all teammates.
- **Screen ping:** Mouse Wheel Click / F3 places a 3D world ping visible through terrain/walls on teammates' screens and compass.
- **Enemy spotted:** double-click ping or radio wheel option places a red marker with audio callout.

### 11.3 Radio Message Wheel

Hold Mouse Wheel Click / F3. 8 preset messages per page: "Enemy Spotted," "Need Ammo," "Need Healing," "Let's Go Here," "Help," "Wait," "Affirmative," "Negative." Can ping care packages and ground loot.

### 11.4 Emotes

F4 wheel with two pages of 8 slots (16 total equipped). 21+ default emotes; additional unlockable/purchasable.

---

## 12. Audio Systems

### 12.1 Sound Model

Stereo system with HRTF (Head-Related Transfer Function) processing for directional cues. Sound propagation speed: 340 m/s (realistic). Surface-material-aware footstep sounds (grass, wood, concrete, metal, water). Volume scales with movement speed.

### 12.2 Gunshot Audible Distances

**Unsuppressed:**

| Weapon Type | Range |
|-------------|-------|
| Pistols / SMGs | 400m |
| Assault Rifles | 700m |
| Sniper Rifles | 1,000m |

**Suppressed:**

| Weapon Type | Range | Reduction |
|-------------|-------|-----------|
| Pistols / SMGs | 100m | 75% |
| Assault Rifles | 350m | 50% |
| Sniper Rifles | 700m | 30% |
| VSS (built-in) | 125m | — |

### 12.3 Sound Layers for Gunfire

Three potential sounds when shot at:
1. **Muzzle report:** the gun's firing sound, travels at 340 m/s (delayed by distance).
2. **Bullet crack / sonic boom:** supersonic shockwave as the bullet passes nearby; heard before the muzzle report at long range.
3. **Whizz:** flyby sound when a round passes close but misses.

### 12.4 Footstep & Vehicle Audio

| Movement | Audible Range |
|----------|--------------|
| Jump / Land | ~50m |
| Walk / Crouch | ~40m |
| Prone crawl | ~10m |

Vehicle engines are audible at several hundred meters. Engine type and RPM are identifiable by sound (useful for identifying vehicle type at distance). Engine can be toggled off (Z key) for silent coasting.

---

## 13. Replay & Spectator Systems

### 13.1 Death Cam

Plays automatically upon elimination. Shows the last ~20 seconds from the killer's perspective (third-person behind the killer). Can be skipped.

### 13.2 3D Replay

Records all player activity within a 1 km radius of the player. Up to 20 replays stored locally.

- Timeline with kill markers from all recorded players.
- Playback speed: 0.25×–2×.
- Camera modes: Player View (exact POV), Following (rotatable third-person on any player), Free Camera (fly-cam, any position/angle).

### 13.3 Chain Spectator

After elimination, option to spectate the player who killed you. If that player dies, view transfers to their killer. Chain continues until match end or spectator leaves.

---

## 14. Engine & Presentation Systems

### 14.1 Perspective

Two camera modes:
- **TPP (Third-Person Perspective):** camera behind and above the character. Allows peeking around corners without exposure.
- **FPP (First-Person Perspective):** locked to character's eye level.

FPP and TPP have separate matchmaking queues. Ranked supports both perspectives.

### 14.2 Save System

No mid-match saves. All progression (rank, mastery, pass) is server-side and persistent. Match replays stored locally (up to 20).

### 14.3 Anti-Cheat

- **Zakynthos:** KRAFTON's proprietary anti-cheat. Hardware ban system (prevents ban evasion via new accounts).
- AI/ML behavioral analysis across matches.
- Detects: memory editing, injection, DMA hacks, recoil manipulation, speed hacks, aimbots, wallhacks.
- Scale: ~1.5–1.7 million accounts permanently banned per half-year.

### 14.4 Reporting

Categories: cheating/hacking, teamkilling, toxic behavior, bug exploitation, teaming (unauthorized cooperation). Penalties range from temporary bans (3–7 days for teamkilling) to permanent + hardware bans (confirmed cheating).

---

## 15. Open Questions / Unverified

- Exact blue zone damage-per-distance scaling formula (community knows it scales, exact curve undocumented).
- Precise RP gain/loss formula for Ranked — KRAFTON has not published the weights for kills vs placement vs damage.
- Exact fuel consumption rates for many newer vehicles (Dirt Bike, Rondo vehicles, etc.).
- MG3 exact damage difference between 660 RPM and 990 RPM fire rate modes (believed identical base damage, different DPS).
- Lynx AMR vehicle damage multipliers and cover penetration specifics.
- Several vehicle HP values for newer additions (Mountain Bike, Airboat, Food Truck, etc.).
- Dragunov RPM discrepancy between sources (182 vs higher values) — may reflect a recent rebalance.
- Tactical Gear slot exact stats (EMT Gear cast-time reduction, Tactical Pack capacity bonus) — values vary across patch notes.
- Exact fall damage curve (threshold height, damage scaling formula).

---

## 16. References

### Wikis
- [PUBG Wiki (wiki.gg)](https://pubg.wiki.gg/) — primary community wiki; Playzone, DBNO, Weapons, Vehicles, Maps
- [PUBG Interactive Map (pubgmap.io)](https://pubgmap.io/) — weapons, consumables, attachments data

### Official Sources
- [PUBG Official Site](https://pubg.com/) — patch notes, dev letters, game info
- [PUBG Support](https://support.pubg.com/) — Ranked FAQ, reporting, emotes, parachute system

### Community Data
- [Battlegrounds.party](https://battlegrounds.party/) — datamined weapon stats
- [PUBG Statistics](https://pubgstatistics.com/weapons) — weapon stat aggregation
- [Steam Community Guides](https://steamcommunity.com/app/578080/guides/) — parachuting, weapon stats, controls

### Companion Documents
- [docs/weapons.md](docs/weapons.md) — full weapon stat tables, damage model, armor values, attachments, ballistics, throwables
