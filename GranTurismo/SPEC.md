# Gran Turismo — Gameplay Systems Spec

Gran Turismo (PlayStation 1) — Japan: December 23, 1997 | PAL: May 8, 1998 | North America: May 12, 1998

---

## 1. Core Gameplay Systems

### Primary Loop

Gran Turismo offers two independent modes: **Arcade Mode** (pick-up-and-play racing) and the career mode, officially called **Simulation Mode** (NTSC) or **Gran Turismo Mode** (PAL). Both share the same physics engine and track set but differ fundamentally in progression logic. This document uses "GT Mode" as a shorthand for the career mode.

The GT Mode loop is:
1. Earn credits by placing in races.
2. Buy and tune cars at the dealership and parts shop.
3. Pass license tests to unlock higher-tier championships.
4. Enter championships with eligible cars; earn prize money and prize cars.
5. Spend winnings on faster cars and deeper tuning to enter harder championships.

### Race Structure

All races are circuit races (fixed laps around a closed course). A typical championship consists of 3–6 individual rounds; final standings are based on cumulative points. There is no damage model — cars do not incur mechanical wear or cosmetic damage from contact.

Races field the player plus **5 AI opponents** (6 cars total on track). The development team originally targeted 12 cars per race but PlayStation hardware constraints imposed the 6-car limit. Overtaking is by position on track; no weapon items or shortcuts exist. The only performance variable under player control is car selection and tuning loadout.

### Arcade Race Rules

- 2 laps per race.
- 5 AI opponents from the same car class.
- Placing 1st advances to the next track; any place completes the race.
- Winning all tracks per difficulty tier unlocks bonus content (see §3).

### GT Mode Race Rules

- Lap count varies by championship (typically 3 laps per round in most cups; 60 laps for endurance races).
- Minimum finishing position to earn championship points: the game awards points for all finishes but requires 3rd place or better to count as a qualifying result for championship unlock progression in some cups.
- Retirement is possible (driving off course repeatedly may trigger a disqualification in some events — unverified; see §15).

### Physics Summary

Gran Turismo 1 uses a simulation-oriented physics model relative to its contemporaries. Key behaviors:

- **Weight transfer**: Braking shifts mass forward, unloading rear tires and making the rear susceptible to lock-up before the front. Acceleration shifts mass rearward.
- **Slip angle**: Each tire has an optimal slip angle range for maximum grip. Exceeding it reduces traction. High rear slip angle relative to front causes yaw (oversteer); high front slip angle causes push (understeer).
- **Drivetrain effect**: FF cars understeer under power; FR/MR cars oversteer under power; 4WD cars distribute torque across all four wheels, reducing individual tire stress.
- **Tuning interaction**: Stiffer front springs reduce oversteer tendency; stiffer rear springs reduce understeer tendency. Softening the opposite end produces the same directional effect.

No publicly available reverse-engineered formula for grip coefficients or tire friction exists for GT1 (see §15).

**Note on Arcade vs. GT Mode physics**: International (NTSC/PAL) versions apply a higher performance multiplier in Arcade Mode compared to the Japanese version, making cars noticeably faster. Additionally, NTSC/PAL Arcade tracks include jump/crest modifications not present in the Japanese release (Grand Valley Speedway, Trial Mountain, Autumn Ring, Grand Valley East, Deep Forest).

---

## 2. Controls & Input

### Supported Controllers

| Controller | Steering Method |
|---|---|
| DualShock Analog Controller | Left analog stick (analog mode) or D-pad (digital mode) |
| Standard Digital Controller | D-pad only |
| Namco NeGcon | Analog twist-axis steering via swivel joint |

### Default Button Mapping (DualShock)

| Input | Action |
|---|---|
| Left Analog Stick (left/right) | Steer |
| X | Accelerate |
| Square | Brake |
| Circle | Handbrake |
| Triangle | Reverse |
| L1 | Rear view (look behind) |
| L2 | Shift down (manual transmission) |
| R1 | Cycle camera view |
| R2 | Shift up (manual transmission) |
| D-pad | Steer (digital mode) |
| Start | Pause |
| Select (in controller config) | Toggle analog/digital steering mode |

### Transmission

Both **automatic** and **manual** transmission are available, selected via the Options menu before a race. In manual mode, L2 downshifts and R2 upshifts. In automatic mode, L2/R2 are unused during gameplay.

### Camera Views

Four in-race camera views, cycled by R1:

| View | Description |
|---|---|
| Bumper | Low hood/bumper perspective |
| Cockpit | Interior driver perspective; rear-view mirror visible only in this view |
| Roof | Overhead perspective from roof level |
| Chase | Third-person trailing camera |

Chase camera offers configurable distance and angle (Options > Configuration 2):
- **Distance**: Normal, Loose, Tight
- **Angle**: Narrow, Normal, Wide

### Replay Playback Controls

| Input | Action |
|---|---|
| Triangle / Circle / X / Square | Switch camera angle |
| D-pad Up / Down | Switch which car the camera tracks |

### Steering Notes

- Analog steering in GT1 is faster/more sensitive than in Gran Turismo 2, which requires adaptation — particularly in high-speed corners where small inputs cause large directional changes.
- No configurable steering sensitivity or deadzone settings exist.
- Analog/digital toggle is accessed via the controller configuration screen.

---

## 3. World Structure

### Track List

Gran Turismo 1 ships with 11 course layouts across 9 base circuits. All courses except Test Course support a reverse direction variant.

| Track | Length | Turns | Notes |
|---|---|---|---|
| High Speed Ring | 3.5 km (2.17 mi) | ~8 | Banked turns; fastest average speeds |
| Trial Mountain Circuit | ~4.0 km (~2.5 mi) | 15 | Sierra Nevada inspiration; chicanes and sweepers |
| Grand Valley Speedway | 4.94 km (3.07 mi) | 17 | Long layout; used for 300km Endurance |
| Grand Valley East | Shorter variant | — | Abbreviated section of Grand Valley Speedway |
| Deep Forest Raceway | N/A | — | Cliff-edge back straight; inspired by Swiss Alps terrain |
| Autumn Ring | 2.9 km (~1.80 mi) | 14 | Rapid technical corners |
| Autumn Ring Mini | 1.3 km (~0.81 mi) | — | Compact variant of Autumn Ring |
| Clubman Stage Route 5 | N/A | — | Short, technical street circuit |
| Special Stage Route 5 | N/A | — | Longer city street layout |
| Special Stage Route 11 | 2.834 km (1.76 mi) | 22 | Hairpins and chicanes; exclusive to GT1 and GT3 |
| Test Course | 10.35 km (6.43 mi) | 2 | Long oval; no reverse variant; used for performance testing (§13) |

All tracks are fictional. Track lengths without citation are approximate.

### Arcade Mode Track Unlocks

Tracks and manufacturers unlock progressively by winning races on Easy difficulty:

| Winning Track | Unlocks Track | Unlocks Manufacturer |
|---|---|---|
| Autumn Ring | Autumn Ring (playable) | Toyota |
| Trial Mountain | Deep Forest Raceway | Subaru |
| Grand Valley East | Special Stage Route 5 | Chevrolet |
| Clubman Stage Route 5 | Grand Valley Speedway | TVR |

Completing all tracks on Normal unlocks the Ending Movie. Completing all tracks on Hard unlocks Hi-Res GT bonus content. Placing 1st on every track with all three car classes (A, B, C) on Normal or higher unlocks the Staff Video.

### No Open World / Overworld

There is no overworld, hub area, or free-roam. All navigation is menu-based. GT Mode is structured as a series of menus: Dealership, Parts Shop, License Center, Race Track (championship selection), and Garage.

---

## 4. Playable Cars

### Roster Size

The exact car count varies by counting methodology:

| Count | What it includes |
|---|---|
| 140 | Official claim printed on the NTSC box art |
| ~178 | Comprehensive count including all color/spec variants (most accepted community figure) |
| 75 | New Car Dealership stock only (total purchase cost: 2,776,990 Cr) |

The discrepancy arises from whether race-modified variants, color variants, and prize-only cars are counted as distinct entries. The Japanese version includes two Honda NSX 1992 models removed from international releases.

### Represented Manufacturers

| Origin | Manufacturers |
|---|---|
| Japan | Honda, Mazda, Mitsubishi, Nissan, Subaru, Toyota |
| Japan (racing sub-brands) | Nismo (listed under Nissan), TRD (listed under Toyota) |
| USA | Chevrolet, Dodge (labeled "Chrysler" in JP/EU versions) |
| USA (NA only) | Acura (Honda luxury brand, NTSC-U only) |
| UK | Aston Martin, TVR |

### Dealerships

GT Mode features two distinct car dealerships:

**New Car Dealership**: Fixed inventory of 75 cars across all manufacturers. Prices are static.

**Used Car Dealership**: Sells Japanese manufacturer cars only (Honda, Mazda, Mitsubishi, Nissan, Subaru, Toyota). Operates on a rotating **600-day cycle**, updating inventory every **10 in-game days** with new cars at lower prices than new. Cars arrive with pre-assigned mileage and color. Four special black race cars appear only during days 694–700 of the cycle.

### Garage

Maximum capacity: **100 cars**. Cars retain all installed upgrades independently.

### Car Stats

Each car is described by four primary attributes:

| Stat | Unit | Notes |
|---|---|---|
| Horsepower | bhp | Stock value; upgrades increase this |
| Torque | lb-ft | Displayed at peak rpm |
| Weight | kg or lbs | Stock value; weight reduction upgrades decrease this |
| Displacement | cc | Fixed; not directly used in eligibility calculations |

There is **no Performance Points (PP) system** in Gran Turismo 1. PP was introduced in Gran Turismo 5 Prologue Spec II. Car eligibility for championships is determined by drivetrain class (FF/FR/MR/4WD) and, for some cups, implicit power/weight thresholds — not a unified PP restriction.

### Drivetrain Classifications

| Class | Description | Handling Tendency |
|---|---|---|
| FF | Front-engine, front-wheel drive | Understeer under power |
| FR | Front-engine, rear-wheel drive | Oversteer under power |
| MR | Mid-engine, rear-wheel drive | Oversteer under power; more responsive than FR |
| 4WD | Four-wheel drive (any engine position) | Neutral to mild understeer; most forgiving |

### Arcade Mode Car Classes

In Arcade Mode, cars are sorted into three tiers — **Class A**, **Class B**, and **Class C** — based on performance. Class A is the fastest, Class C the slowest. AI opponents in a given race are drawn from the same class as the player's selected car.

### Notable Cars

| Car | HP (approx.) | Notes |
|---|---|---|
| Toyota Castrol TOM's Supra | 686 bhp | Top-tier prize car |
| Nissan R33 GT-R LM | — | Prize car; LM racing spec |
| TVR Cerbera LM Edition | — | Prize car; exceptional stock performance |
| Honda del Sol LM Edition | — | Mid-engine; noted for drifting capability |
| NSX Zero | — | Strong all-around car; dominant in its class |
| Aston Martin DB7 Volante | — | Most expensive new car at 164,000 Cr |

### Starting Car Selection

With the initial 10,000 Cr budget, the player must buy from the Used Car Dealership. Typical affordable options on Day 1 include:

| Car | Approx. Price | Notes |
|---|---|---|
| Mazda RX-7 FC Savanna (used) | ~8,000 Cr | Leaves room for one minor upgrade |
| Honda Prelude (used) | ~10,000 Cr | Budget-tight |
| Mitsubishi GTO NA (used) | ~10,000 Cr | 221 hp stock; strong starter |
| Nissan Pulsar (used) | ~10,000–14,000 Cr | May require a Sunday Cup win first |

Exact inventory depends on the Used Car Dealership's 10-day rotation cycle.

---

## 5. GT Mode Progression

### License Requirements

GT Mode gates championship access behind license tiers. All license tests must be **passed** (bronze or better) to unlock the associated championships. Earning **all gold** across an entire license tier awards a bonus prize car:

| License | All-Gold Prize Car |
|---|---|
| B-License | Dodge Viper GTS (or Dodge Concept Car — sources vary) |
| A-License | TRD 3000GT |
| International A-License | Nismo 400R |

### Championship Sequence

Mandatory progression flows as follows. Entry into higher cups requires a minimum finishing position (3rd or better) in prerequisite cups:

| Championship | Entry Requirement | Rounds | Prize Car (1st overall) | Credit Reward |
|---|---|---|---|---|
| Sunday Cup | B-License | 3 | Mazda Demio A-spec | 15,000 Cr (overall) / 24,000 Cr (all rounds) |
| Clubman Cup | B-License | 3 | Chevrolet Camaro Z28 30th Anniversary | N/A |
| Gran Turismo Cup | 3rd+ in Sunday Cup AND Clubman Cup | 4 | N/A | N/A |
| Gran Turismo World Cup | 3rd+ in all three previous cups | 6 | N/A | N/A |

Gran Turismo Cup courses: Grand Valley Speedway, Deep Forest Raceway, Special Stage Route 5, Trial Mountain (3 laps each). The World Cup expands the circuit rotation.

**Pole position bonuses**: Sunday Cup awards a 1,500 Cr pole bonus; Clubman Cup awards 2,500 Cr. Later cups likely scale further but exact amounts are undocumented.

### Special Events (Drive Class Championships)

These championships restrict entry to cars of a specific drivetrain class:

| Championship | Restriction | Rounds |
|---|---|---|
| FF Challenge | FF cars only | ~3 |
| FR Challenge | FR cars only | ~3 |
| 4WD Challenge | 4WD cars only | ~3 |

An MR Challenge is referenced in some GT literature but is **not confirmed** as a standalone event in GT1 sources (see §15).

### Endurance Races

| Race | Track | Laps | Notes |
|---|---|---|---|
| Grand Valley 300km Endurance | Grand Valley Speedway | 60 | Pit stops available (tire change only) |
| Special Stage Route 11 All-Night Endurance Race | Special Stage Route 11 | 30 | Exclusive to GT1 and GT3 |

Endurance races feature **tire wear** and **pit stops** (see §8). There is **no fuel consumption** — pit stops are for tire replacement only. No pit crew animations exist in GT1 (added in GT4).

### Prize Cars

Approximately half of all obtainable cars are prizes rather than purchasable through the dealership. The prize car system is the primary way to acquire racing-spec and LM Edition vehicles. In the Japanese version, prize cars carry a nominal value of 20,000,000 credits; in international versions the assigned resale value is drastically lower (≤35,000 credits).

### Post-Game

After completing the main championship sequence, no formal New Game+ exists. Players can continue entering races, grinding credits, and completing any missed championships or endurance events.

---

## 6. License Tests

### Structure

Three license tiers, each containing **8 individual tests**. Tests are time-attack challenges using a provided car on a specified section of track. Each test awards a bronze, silver, or gold trophy based on finishing time.

**Passing a license tier** (earning at least bronze on all 8 tests) unlocks the corresponding championships. **Earning all gold** on a complete tier awards a bonus prize car for that tier.

### B-License Tests

| Test | Description | Assigned Car |
|---|---|---|
| B-1 | Starting and Stopping 1 | Mazda Demio |
| B-2 | Starting and Stopping 2 | Mitsubishi GTO |
| B-3 | Basics of Cornering 1 | Honda del Sol |
| B-4 | Basics of Cornering 2 | Nissan Silvia |
| B-5 | Basics of Cornering 3 | Mitsubishi GTO |
| B-6 | Basics of Multiple Cornering 1 | Mitsubishi FTO |
| B-7 | Basics of Multiple Cornering 2 | Nissan Silvia |
| B-8 | B-License Final Test | Mazda Eunos Roadster |

**All-gold prize**: Dodge Viper GTS (or Dodge Concept Car — sources vary by region).

**Sample gold/silver/bronze times (B-1):** Gold 0:34.750 / Silver 0:35.200 / Bronze 0:36.000

### A-License Tests

| Test | Description | Assigned Car |
|---|---|---|
| A-1 | Practical Cornering 1 | Toyota Supra RZ |
| A-2 | Practical Cornering 2 | N/A |
| A-3 | Practical Cornering 3 | N/A |
| A-4 | Handling Multiple Corners 1 | N/A |
| A-5 | Handling Multiple Corners 2 | N/A |
| A-6 | Handling Multiple Corners 3 | N/A |
| A-7 | Advanced Techniques | N/A |
| A-8 | A-Class License Final Test | N/A |

**All-gold prize**: TRD 3000GT.

**Sample gold/silver/bronze times (A-License):** Gold 0:32.400 / Silver 0:33.000 / Bronze 0:34.000

### International A-License Tests

| Test | Assigned Car |
|---|---|
| IA-1 through IA-8 | Dodge Viper GTS or TVR Griffith 4.0 (varies by test) |

These tests are all set on High Speed Ring.

**All-gold prize**: Nismo 400R (available in gray, red, silver, or yellow).

**Sample gold/silver/bronze times (IA, High Speed Ring, TVR Griffith):** Gold 1:03.990 / Silver 1:05.200 / Bronze 1:07.000

**Note on lap time parity:** License times are adjusted between NTSC and PAL versions to account for the 60fps vs 50fps frame rate difference. A 60fps run at a given real-world elapsed time will differ from a 50fps run; the target windows are scaled accordingly.

### Demo Replay Availability

| Region | Tests with demo replay |
|---|---|
| Japan | 9 of 24 tests: B-3, B-5, B-7, B-8, A-1, A-4, A-7, A-8, IA-7 |
| North America / PAL | All 24 tests |

---

## 7. Car Tuning & Upgrades

### Upgrade System Overview

The Parts Shop sells upgrade components per car. Upgrades are purchased individually and applied immediately. Each upgrade category offers staged tiers (e.g., Stage 1, Stage 2, Stage 3); purchasing a higher stage replaces the lower one rather than stacking on top of it.

### Upgrade Categories

| Category | Effect |
|---|---|
| Engine internals (Stage upgrades) | Increase peak horsepower and torque |
| Turbocharger / Supercharger | Forced induction; large HP gains |
| Air cleaner | Minor HP gain |
| Exhaust / Muffler | Minor HP gain |
| Spark plugs | Minor HP gain |
| Weight reduction (Stage upgrades) | Reduce car weight; improve power-to-weight ratio and handling |
| Suspension (Stage upgrades) | Unlock adjustable spring rates, dampers, and anti-roll bars |
| Brakes | Improve braking distance and balance |
| Transmission | Unlock adjustable gear ratios and final drive |
| Tires | Change compound for grip improvement (see §8) |
| Wheels | Change wheel size/weight |
| Downforce | Unlock adjustable aerodynamic downforce (front/rear split) |

Specific credit costs for each part and stage are not confirmed in GT1 community sources (see §15).

### Tuning Settings

After purchasing suspension, transmission, and downforce upgrades, a dedicated **Settings** screen unlocks per-car adjustability:

| Setting | Adjustable Parameters |
|---|---|
| Gear ratios | Individual ratio per gear (1st through top gear) + final drive ratio |
| Spring rates | Front and rear independently |
| Dampers | Front and rear independently |
| Anti-roll bars (stabilizers) | Front and rear independently |
| Camber | Negative camber adjustment (no positive camber) |
| Downforce | Front and rear independently |

An **Auto Setup** option exists for gear ratios, which distributes ratios evenly across the range. Manual tuning overrides this.

The transmission tuning screen allows the player to set individual gear ratios and the final drive multiplier, directly controlling the tradeoff between acceleration and top speed.

---

## 8. Tires

### Compound System

Gran Turismo 1 uses a **three-category** tire system, not the granular N1–N4 / S1–S4 / R1–R5 tiers introduced in GT4. The categories are:

| Category | Sub-types | Grip Level |
|---|---|---|
| Normal | (single type) | Lowest; street compound |
| Semi-Racing | Hard, Soft | Moderate |
| Racing Slick | Hard, Soft | Highest |

Within Semi-Racing: Soft > Hard in grip. Within Racing Slick: Soft > Hard in grip.

**Hierarchy**: Normal < Semi-Racing Hard < Semi-Racing Soft < Racing Hard < Racing Soft

Tire compound affects cornering limits, braking distance, and the threshold at which the car begins to slide. Racing Slicks on a high-power car significantly raise outright lap speed but may amplify oversteer on corner entry under braking.

Specific friction coefficient values and price per compound are not confirmed in GT1 sources (see §15).

### Tire Wear

Tire wear is simulated in **endurance races** and can optionally be enabled in **split-screen races**. It is **not active** in standard GT Mode championships or Arcade Mode single races.

Tire condition is displayed via a color-coded indicator:

| Color | Condition |
|---|---|
| Blue | Cold / new — suboptimal grip, tires not up to temperature |
| Green | Warm — optimal grip window |
| Yellow | Moderate wear — grip beginning to degrade |
| Orange | Heavy wear — significant grip loss |
| Red | Critical wear — tires should be replaced |

When tires reach critical wear in endurance races, the player can enter the pit lane for a tire change (see §5). There is **no fuel simulation** — pit stops are for tire replacement only.

---

## 9. Economy

### Currency

The sole currency is **credits (Cr)**. There is no secondary currency or premium resource.

### Starting Capital

Players begin GT Mode with **10,000 credits**.

### Income Sources

| Source | Approximate Amount |
|---|---|
| Sunday Cup (overall win) | 15,000 Cr |
| Sunday Cup (all rounds won) | 24,000 Cr |
| Sunday Cup pole bonus | 1,500 Cr |
| Clubman Cup pole bonus | 2,500 Cr |
| Higher-tier championships | Higher amounts (not documented per race) |
| Selling prize cars (JP version) | Up to 20,000,000 Cr per car |
| Selling prize cars (NA/EU version) | ≤35,000 Cr per car |

The prize car value discrepancy between regional versions significantly affects early-game economy. The Japanese version allows selling prize cars for enormous sums, enabling a fast snowball into competitive cars. NA/EU players must grind race winnings more conventionally.

Approximately **half of all obtainable cars are prize cars** rather than shop purchases, making championship completion directly valuable as a car acquisition strategy, not just a credit source.

### Expenditures

| Category | Price Range |
|---|---|
| Used cars (Used Car Dealership) | ~5,000–50,000 Cr (varies by rotation) |
| New cars (New Car Dealership) | Up to 164,000 Cr (Aston Martin DB7 Volante) |
| All 75 new dealership cars combined | 2,776,990 Cr |
| Upgrade parts | Not fully documented per part |
| Tires | Not fully documented per compound |

The Honda dealership has the highest total catalog cost at 486,490 Cr across all its models.

---

## 10. Arcade Mode

### Structure

Arcade Mode is self-contained and does not share progress, credits, or cars with GT Mode.

**Flow**: Select difficulty → Select manufacturer → Select car class → Race on available tracks.

### Car and Class Selection

- Manufacturers are organized into groups.
- Cars are divided into **Class A** (fastest), **Class B**, and **Class C** (slowest).
- AI opponents are always from the same class as the player.

### Difficulty Settings

| Difficulty | Notes |
|---|---|
| Easy | Default entry point; fewest AI opponents unlocked per track completion |
| Normal | Completing all tracks unlocks Ending Movie bonus |
| Hard (JP) / Difficult (NA) | Completing all tracks unlocks Hi-Res GT bonus |

### Unlock Progression (Easy)

See §3 for the track/manufacturer unlock table tied to Easy difficulty victories.

**Bonus unlocks:**
- 1st place every track, all car classes, Normal or higher: Staff Video
- All tracks + all classes + all three difficulties: all bonus items unlocked

### 2-Player Mode

Gran Turismo 1 supports split-screen 2-player races via Arcade Mode. Specific rules (track selection, car restriction, lap count) are not fully documented.

---

## 11. AI Opponents

### Rubber-Banding

AI rubber-banding is **confirmed** in GT1. The AI can exhibit up to approximately **12 seconds** of variance relative to player position — visually observable by watching AI cars slow when the player falls behind and accelerate when the player pulls ahead. This mechanic is present regardless of the player's absolute pace; a perfectly-driven lap does not guarantee the AI won't compress the gap artificially.

### AI Difficulty

Arcade Mode difficulty (Easy / Normal / Hard) adjusts AI performance. Specific numerical parameters (speed multiplier, mistake frequency) are not documented in GT1 sources.

In GT Mode, AI difficulty scales implicitly with championship tier — later championships field faster opponents — but the underlying adaptation mechanics are not published.

---

## 12. UI & HUD

### Race HUD

The HUD uses an orange color scheme (shared with GT2, GT3, and GT4 Prologue). Layout:

| Element | Position | Notes |
|---|---|---|
| Current position | Top-left | Small italic white font; e.g., "3/6" |
| Current lap / total laps | Top-left | Below position indicator |
| Lap time | Lower area | Current lap timer |
| Speedometer | Bottom area | — |
| Tachometer | Bottom area | Rev counter with redline |
| Gear indicator | Adjacent to tach | Current gear number |
| Tire wear indicator | — | Color-coded; endurance races only (see §8) |

No minimap is displayed during races. No damage indicator exists (no damage model). No split times are shown during normal races (timing data is recorded in Time Trial only).

### Menus (GT Mode)

GT Mode navigation is menu-driven:

| Screen | Contents |
|---|---|
| Main GT Mode menu | New Car Dealership, Used Car Dealership, Parts Shop, License Center, Race Track, Garage, Load/Save |
| New Car Dealership | Cars by manufacturer; fixed inventory; purchase with credits |
| Used Car Dealership | Japanese manufacturers only; rotating inventory (see §4) |
| Parts Shop | Upgrade parts per car; Settings screen for tuning adjustments |
| License Center | Select license tier; run individual tests |
| Race Track | Championship/event selection; entry requirement check |
| Garage | View owned cars; select active car; sell cars |

### Results Screen

Post-race screen shows finishing position, lap times, prize money earned. Championship rounds show cumulative standings after each round.

---

## 13. Engine & Presentation Systems

### Machine Test

The **Test Course** (see §3) doubles as a performance testing facility with three benchmarks:

| Test | Description |
|---|---|
| 0–400 m | Quarter-mile acceleration time |
| 0–1000 m | Kilometer acceleration time |
| Max Speed | Top speed run on the oval's straights |

These tests allow the player to measure the effect of tuning changes and upgrades before entering a race.

### Save System

GT Mode progress saves to a **PlayStation Memory Card**, consuming **5 blocks** (~40 KB). No auto-save; players must manually save from the GT Mode main menu. Arcade Mode progress (unlocked tracks and bonus items) also saves to the memory card.

### Replay System

- Full race replays are available after any completed race.
- The replay system was praised by contemporary reviewers as cinematic in quality.
- Multiple camera angles are available during playback, cycled via face buttons (Triangle / O / X / Square).
- D-pad Up/Down switches the tracked car.
- Specific replay data stored (input record vs. physics state) is not documented.

### Audio/Music System

The soundtrack is licensed music. The game does not dynamically alter music based on race state (no intensity layering, no triggered stingers documented for GT1). Music selection differs by region:

| Region | Notable Tracks |
|---|---|
| Japan | "Second Chance" (credits music) |
| NA/EU | "Skeletal" (credits music); licensed tracks from Garbage, Manic Street Preachers (Chemical Brothers remix), Ash, Cubanate |
| PAL only | Feeder track |
| NTSC only | TMF track |

Engine sounds are car-specific but whether they are unique recordings vs. categorized samples per drivetrain type is not documented.

### Difficulty Settings

Arcade Mode has Easy / Normal / Difficult (Hard in JP). GT Mode has no explicit difficulty slider — challenge scales with championship tier and car/tuning investment.

---

## 14. Regional Version Differences

| Feature | Japan (NTSC-J) | North America (NTSC) | PAL (Europe) |
|---|---|---|---|
| Release date | Dec 23, 1997 | May 12, 1998 | May 8, 1998 |
| Frame rate | 60 fps | 60 fps | 50 fps |
| Career mode name | — | "Simulation Mode" | "Gran Turismo Mode" |
| License demo replays | 9 of 24 tests | All 24 tests | All 24 tests |
| Honda NSX 1992 variants | 2 included | Removed | Removed |
| Arcade Mode: Corvette C1 | Absent | Present | Present |
| Arcade Mode: Mazda Roadster NB | Absent | Present | Present |
| Arcade track modifications (jumps/crests) | Absent | Present | Present |
| Arcade performance multiplier | Lower | Higher | Higher |
| Prize car resale value | Up to 20,000,000 Cr | ≤35,000 Cr | ≤35,000 Cr |
| TVR Griffith name | "Griffith Blackpool B340" | "Griffith 500" | "Griffith 500" |
| TVR livery | Stripes with GT logo | Tuscan Challenge design | Tuscan Challenge design |
| Credits music | "Second Chance" | "Skeletal" | "Skeletal" |
| Unique licensed tracks | — | TMF | Feeder |
| Language options | Japanese | English | English/Spanish/French/Italian/German |
| "Goodies" menu | "Goodies" | "Bonus Items" | "Bonus Items" |
| Difficulty label | "Hard" | "Difficult" | "Difficult" |

License time targets are adjusted for PAL's 50fps to maintain equivalent real-world challenge.

---

## 15. Open Questions / Unverified

The following items are either conflicting across sources or have no confirmed numbers:

1. **MR Challenge**: Referenced in some GT literature as an event in GT1 but not confirmed in GT1-specific sources. Likely GT2-only.

2. **Tire prices and grip coefficients**: No confirmed credit costs or friction values for any tire compound in GT1 exist in public sources.

3. **Upgrade part prices**: No confirmed credit costs for any Stage upgrade in GT1.

4. **Complete dealership price list**: Individual car purchase prices are not comprehensively documented. The aggregate (75 new cars = 2,776,990 Cr) is confirmed but per-car breakdown is incomplete.

5. **Per-race prize money beyond Sunday/Clubman Cup**: Prize amounts for Gran Turismo Cup, World Cup, Special Events, and Endurance Races are not confirmed.

6. **Exact track lengths**: High Speed Ring (3.5 km), Grand Valley Speedway (4.94 km), Special Stage Route 11 (2.834 km), Autumn Ring (2.9 km), and Test Course (10.35 km) have documented lengths. Deep Forest, Clubman Stage Route 5, and Special Stage Route 5 remain unconfirmed.

7. **Championship points formula**: How points are awarded per finishing position (1st = X points, 2nd = Y, etc.) is not documented.

8. **Exact car count**: Box art claims 140; community counts range from 166 to ~178 depending on methodology. No authoritative definitive count exists.

9. **B-License all-gold prize**: Sources vary between Dodge Viper GTS and Dodge Concept Car. May differ by region.

10. **GT1 physics engine internals**: No reverse-engineering or decompilation of GT1's physics code is publicly available. Unlike GT4–GT7 (which use the Adhoc scripting language, reverse-engineered via the OpenAdhoc project on GitHub), GT1's binary has not been cracked.

11. **AI difficulty numerical parameters**: Speed multipliers, mistake rates, and aggression values for Easy/Normal/Hard AI in Arcade Mode are not published.

12. **Race disqualification rules**: Whether the game can retire or disqualify the player for off-track excursions is unconfirmed.

13. **Used Car Dealership cycle details**: The 600-day / 10-day rotation cycle is documented, but exact inventory per day slot is only partially reconstructed by the community.

---

## 16. References

### Wikis & Databases
- [Gran Turismo Wiki (Fandom) — GT1 main article](https://gran-turismo.fandom.com/wiki/Gran_Turismo_(PlayStation))
- [Gran Turismo Wiki — GT Mode (GT1)](https://gran-turismo.fandom.com/wiki/Gran_Turismo_Mode_(GT1))
- [Gran Turismo Wiki — Car List](https://gran-turismo.fandom.com/wiki/Gran_Turismo_(PlayStation)/Car_List)
- [Gran Turismo Wiki — Prize Cars](https://gran-turismo.fandom.com/wiki/Prize_Car)
- [Gran Turismo Wiki — Tires](https://gran-turismo.fandom.com/wiki/Tires)
- [Gran Turismo Wiki — Pit Lane](https://gran-turismo.fandom.com/wiki/Pit_lane)
- [Gran Turismo Wiki — Test Course](https://gran-turismo.fandom.com/wiki/Test_Course)
- [Wikipedia — Gran Turismo (1997 video game)](https://en.wikipedia.org/wiki/Gran_Turismo_(1997_video_game))
- [The Cutting Room Floor — GT1 Regional Differences](https://tcrf.net/Gran_Turismo_(PlayStation)/Regional_Differences)
- [StrategyWiki — Gran Turismo](https://strategywiki.org/wiki/Gran_Turismo)

### Community FAQs & Guides
- [GameFAQs — Gran Turismo (PS) FAQs](https://gamefaqs.gamespot.com/ps/197468-gran-turismo/faqs)
- [GameFAQs — GT1 Tuning FAQ](https://gamefaqs.gamespot.com/ps/197468-gran-turismo/faqs/3915)
- [GTPlanet — GT1 in Numbers](https://www.gtplanet.net/forum/threads/gran-turismo-1-in-numbers.429658/)
- [GTPlanet — GT1 Complete Used Car List (All Regions)](https://www.gtplanet.net/forum/threads/gt1-complete-used-car-list-all-regions.416358/)
- [GTPlanet — GT1 Tuning Settings](https://www.gtplanet.net/forum/threads/gt1-tuning-settings.395848/)

### Additional Sources
- [GT Series Center — GT1 Track Information](http://www.gtseriescenter.com/GT1tracks.htm)
- [John's Race Space — GT1 License Tests](http://johnsgtspace.blogspot.com/2010/11/gran-turismo-1-license-tests.html)
- [John's Race Space — Races of GT1](http://johnsgtspace.blogspot.com/2010/11/races-of-gran-turismo-1.html)

### Reverse Engineering
- [GitHub — OpenAdhoc (covers GT4–GT7 Adhoc scripts; GT1 not included)](https://github.com/Nenkai/OpenAdhoc)
