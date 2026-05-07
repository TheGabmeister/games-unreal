# Tony Hawk's Pro Skater -- Gameplay Systems Spec

PlayStation, 1999. Developed by Neversoft, published by Activision. Also released on N64 (2000, Edge of Reality), Dreamcast (2000, Treyarch), and Game Boy Color (2000, Natsume). This spec covers the PS1 NTSC-U version unless noted otherwise; see **S12** for version differences.

---

## 1. Core Gameplay Systems

### 1.1 Primary Loop

The player controls a professional skateboarder through 2-minute timed sessions across 9 levels. During each session the player performs skateboarding tricks to score points, collect items, and complete level objectives (called "tapes"). Tapes accumulate across sessions and unlock later levels. The career ends when the player earns a medal at the final competition level, Roswell.

### 1.2 Trick System

Four trick categories, each mapped to a face button combined with directional input. Every skater has **8 grab tricks**, **8 flip tricks**, and **8 grind tricks**. Skaters are classified as **Vert** or **Street**, which determines which diagonal-input tricks they have access to (see S4).

#### Flip Tricks (Square)

Performed in the air. Quick execution, lower base values.

| Trick | Input | Base Pts | Notes |
|---|---|---|---|
| Kickflip | Left + Square | 100 | |
| Heelflip | Right + Square | 100 | |
| Pop Shove-It | Up + Square | 250 | |
| Impossible | Down + Square | 250 | |
| Fingerflip | UR + Square | 600 | Vert only |
| Front-Foot Impossible | DR + Square | 600 | Vert only |
| Varial | DL + Square | 800 | Vert only |
| Hardflip | UR + Square | 300 | Street only |
| 360 Flip | DR + Square | 300 | Street only |
| Sex Change | DL + Square | 500 | Street only |
| Kickflip to Indy | UL + Square | 625 | Holdable |

#### Grab Tricks (Circle)

Performed in the air. All grabs are **holdable** -- holding Circle longer accumulates additional points beyond the base value.

| Trick | Input | Base Pts |
|---|---|---|
| Method | Right + Circle | 315 |
| Indy Nosebone | Left + Circle | 315 |
| Tail Grab | Down + Circle | 315 |
| Stalefish | DL + Circle | 315 |
| Japan Air | Up + Circle | 367 |
| Rocket Air | UR + Circle | 367 |
| Benihana | DR + Circle | 420 |
| Madonna | UL + Circle | 525 |

#### Grind Tricks (Triangle)

Performed by pressing Triangle near a rail, ledge, or lip. Grinds accumulate points per tick while held. Balance is maintained via D-pad left/right, but in THPS1 there is **no visible balance meter** -- the skater visually wobbles and bails if tilted too far. The visible balance meter was added in THPS2. Grinding on vert lip edges has a built-in safety net: wobbling off the vert side ends the combo without a full bail (no crash animation, no score penalty beyond losing the combo).

| Trick | Input | Base Rate |
|---|---|---|
| 50-50 | Triangle (neutral) | 100+ |
| FS Boardslide | Left + Triangle | 100+ |
| BS Boardslide | Right + Triangle | 100+ |
| 5-0 Grind | Down + Triangle | 125+ |
| Nosegrind | Up + Triangle | 125+ |
| Crooked Grind | UL or UR + Triangle | 150+ |
| Smith Grind | DL or DR + Triangle | 150+ |

#### Lip Tricks

Performed by pressing Triangle when approaching the lip/coping of a halfpipe from below. THPS1 treats lip tricks as a subset of the grind system. The primary named lip trick is the **Handplant** (Up + Triangle at lip). Granular lip trick variety (Axle Stall, Rock to Fakie, Invert as distinct moves) was expanded in later entries.

#### Ollie Variants

| Move | Input | Base Pts |
|---|---|---|
| Ollie | Hold X, release to jump | 0 |
| 180 Ollie | L1 or R1 + X | 50 |
| Nollie | Up + X (tap Up within 10 frames before release) | 200 |
| Fastplant | Up, Up + X (each within 8 frames, 16-frame window) | 250 |

### 1.3 Special Tricks

Each skater has **3 unique special tricks** that become available only when the Special Meter is full (S1.5). Tony Hawk is the exception with **4 specials** -- The 900 was added late in development after Hawk landed the real trick at the 1999 X Games. Specials require two directional taps followed by a face button. Special trick values range from **1,500 to 8,000 points**. Per-character specials are listed in S4.

### 1.4 Combo System

A combo is a continuous sequence of tricks without cleanly touching the ground. Combos form by:

- Chaining air tricks (flip + grab in one jump)
- Adding spin rotation to tricks
- Landing from air directly onto a rail (air-to-grind)
- Ollieing off a grind into more air tricks
- Wallrides (Triangle near a wall while airborne)

**What breaks a combo:**
- Landing on flat ground (no manual exists in THPS1 -- see note below)
- Bailing (crashing, losing grind balance)

**Manuals are NOT in THPS1.** They were added in THPS2. This is the defining mechanical constraint of the original: once the skater lands on flat ground, the combo ends. The only way to extend combos on the ground is via grinds and wallrides.

**Bail penalty:** Bailing mid-combo forfeits the **entire combo score** -- no partial banking. The Special Meter is also **completely emptied** on bail. This makes high-risk combos a genuine gamble: a long combo can multiply into massive points, but a bail at any point in the chain loses everything.

**Combo score formula:**

```
Combo Score = (Sum of all modified trick values) x Combo Multiplier
```

**Combo Multiplier** = number of tricks in the chain. The first trick starts at 1x; each subsequent trick adds +1. Gaps also add +1 each.

| Tricks in Combo | Multiplier |
|---|---|
| 1 | 1x |
| 2 | 2x |
| 3 | 3x |
| 4 | 4x |
| ... | ... |

**Spin rotation** modifies individual trick values, not the combo multiplier. Each 180 degrees of spin roughly doubles the trick's base value:

| Rotation | Trick Value Modifier |
|---|---|
| 0 (no spin) | 1x |
| 180 | ~2x |
| 360 | ~3x |
| 540 | ~4x |
| 720 | ~5x |
| 900 | ~6x |

So a 360 Kickflip (100 base x ~3 for 360 spin = ~300) inside a 4-trick combo (4x multiplier) contributes its modified value to the sum, and the entire sum is multiplied by 4.

Grinding resets accumulated spin. A spin built in the air is applied to that air trick's value when the skater lands on a grind; rotation starts fresh for the next air segment.

### 1.5 Special Meter

A gauge that fills as the player performs tricks.

- **Filling**: Any successful trick contributes. Higher combos fill it faster.
- **When full**: The meter flashes yellow. The skater gains access to special trick inputs (S1.3) and a **max speed increase**.
- **Draining**: Bailing empties the meter **instantly to zero**. The meter also gradually decays if the player stops performing tricks. Maintaining trick activity keeps it topped off.

### 1.6 Trick Repetition Decay

Repeating the same trick within a 2-minute session (not just within a combo -- across the entire run) degrades its value:

| Use # | Value Retained |
|---|---|
| 1st | 100% |
| 2nd | 75% |
| 3rd | 50% |
| 4th | 25% |
| 5th+ | 10% |

This forces trick variety. See S13 for conflicting data on exact percentages.

### 1.7 Gap Bonuses

Specific named transfers between level geometry award fixed bonus points (50 to 5,000+). Each level has a set of discoverable gaps. Examples:
- Garbage Ollie: 50 pts
- Channel Gap: 250 pts
- Taxi Gap: 600 pts
- Over A Huge 32 Stair Gap: 2,000 pts
- Lombard Gap: 5,000+ pts

### 1.8 Speed Mechanics

- **Autokick ON** (default): The skater pushes automatically. Natural speed decay on flat ground.
- **Autokick OFF**: Player holds Square on the ground to push. Higher top speed, better acceleration, no natural decay on flat ground.
- Downhill slopes increase speed; uphill slopes reduce speed.
- Full Special Meter raises max speed.
- Higher combo score increases movement speed (increments per 100 points of combo value).

---

## 2. Controls & Input

### 2.1 PS1 Control Scheme

#### Ground
| Button | Action |
|---|---|
| X (hold + release) | Ollie (charge + jump) |
| D-pad | Steer |
| Up, Up + X | Fastplant |
| Up + X | Nollie |
| L1/R1 + X | 180 Ollie |

#### Air
| Button | Action |
|---|---|
| Square + direction | Flip trick |
| Circle + direction | Grab trick (hold for bonus points) |
| Triangle | Grind (near rail/lip) or Wallride (near wall) |
| L1 | Spin left (gradual) |
| R1 | Spin right (gradual) |
| L2 | Instant 180 turn left |
| R2 | Instant 180 turn right |
| D-pad | Air steering |

#### Grind
| Button | Action |
|---|---|
| Triangle + direction | Initiate specific grind type |
| D-pad left/right | Balance |
| X | Ollie off grind |

#### Special Tricks
Two directional taps + face button while Special Meter is full. Example: Right, Down + Circle = The 900 (Tony Hawk).

### 2.2 N64 Mapping

| PS1 Button | N64 Equivalent |
|---|---|
| X (Ollie) | A |
| Square (Flip) | B |
| Circle (Grab) | C-buttons (C-Down default) |
| Triangle (Grind) | C-Up |
| L1/R1 (Spin) | L/R |
| D-pad | Analog stick or D-pad |

The N64 controller's lack of a second set of shoulder buttons means L2/R2 (instant 180 turn) must be remapped to C-button combinations.

### 2.3 Dreamcast Mapping

The Dreamcast controller maps face buttons similarly (A=Ollie, B=Flip, X=Grab, Y=Grind) but has only one pair of analog triggers (L/R) instead of the DualShock's four shoulder buttons. The D-pad feel differs significantly from the PS1 DualShock.

---

## 3. World Structure

### 3.1 Level Progression

9 levels divided into three arcs. Levels unlock via cumulative tape count or earning a medal in the preceding competition.

**Arc 1:**
1. Warehouse (Woodland Hills, CA) -- Normal
2. School (Miami, FL) -- Normal
3. Mall (New York City, NY) -- Normal
4. Skatepark Chicago (Chicago, IL) -- **Competition**

**Arc 2:**
5. Downtown (Minneapolis, MN) -- Normal
6. Downhill Jam (Phoenix, AZ) -- Normal
7. Burnside (Portland, OR) -- **Competition**

**Arc 3:**
8. Streets (San Francisco, CA) -- Normal
9. Roswell (Roswell, NM) -- **Competition** (Final)

### 3.2 Unlock Requirements

| Level | Requirement |
|---|---|
| Warehouse | Available from start |
| School | 2 tapes |
| Mall | ~5 tapes |
| Skatepark Chicago | ~8 tapes |
| Downtown | Medal at Skatepark Chicago |
| Downhill Jam | ~13 tapes |
| Burnside | ~15 tapes + Skatepark medal |
| Streets | Medal at Burnside |
| Roswell | 26 goals total |

Exact tape-gate numbers for Mall, Skatepark Chicago, Downhill Jam, and Burnside are poorly documented. School = 2 and Roswell = 26 are confirmed. Others are estimates from speedrun data.

### 3.3 Session Structure

- **Normal levels**: 2-minute timer per run
- **Competition levels**: 3 heats of 1 minute each

**Persistence rules:** Completed objectives (tapes) persist across sessions -- the player can earn different tapes on different runs. However, each objective is **atomic**: all 5 S-K-A-T-E letters must be collected in a single 2-minute run, all 5 level objects must be destroyed in one run, and score thresholds must be reached within one run. Partial progress on an objective resets when the session ends.

### 3.4 Level Details

#### Warehouse (Woodland Hills, CA)
| Goal | Detail |
|---|---|
| High Score | 5,000 |
| Pro Score | 15,000 |
| S-K-A-T-E | 5 floating letters |
| Destroy 5 Boxes | Wooden crates scattered around |
| Secret Tape | Above the half-pipe on a hidden platform |

14 gaps. Secret area above the half-pipe accessed via big air.

#### School (Miami, FL)
| Goal | Detail |
|---|---|
| High Score | 7,500 |
| Pro Score | 25,000 |
| S-K-A-T-E | 5 floating letters |
| Grind 5 Tables | Lunch tables around the school |
| Secret Tape | Grind the 2x4 across the awning from the A/C unit to reach it |

19 gaps. Rooftop above the gym accessible via air conditioner grind.

#### Mall (New York City, NY)
| Goal | Detail |
|---|---|
| High Score | 10,000 |
| Pro Score | 30,000 |
| S-K-A-T-E | 5 floating letters |
| Destroy 5 Directories | Mall directory signs |
| Secret Tape | Grind the suspended wire above the small pool |

12 gaps. Upper level areas accessible via escalators and rail grinds.

#### Skatepark Chicago (Chicago, IL) -- Competition
Medal placement only. 15 gaps. See S7 for competition scoring.

#### Downtown (Minneapolis, MN)
| Goal | Detail |
|---|---|
| High Score | 15,000 |
| Pro Score | 40,000 |
| S-K-A-T-E | 5 floating letters |
| Break 5 No Skating Signs | Scattered around Downtown |
| Secret Tape | Jump across rooftops; turn left on second rooftop |

25 gaps. Rooftop area accessed by grinding between buildings; smashable glass to interior room.

#### Downhill Jam (Phoenix, AZ)
| Goal | Detail |
|---|---|
| High Score | 20,000 |
| Pro Score | 40,000 |
| S-K-A-T-E | 5 floating letters |
| Open 5 Valves | Hydrant/pipe valves along the downhill course |
| Secret Tape | Grind across valves over gaps to a dam platform |

3 gaps (fewest in the game -- downhill format limits lateral exploration). Dam area platform is the main secret.

#### Burnside (Portland, OR) -- Competition
Medal placement only. Significantly harder than Skatepark Chicago.

#### Streets (San Francisco, CA)
| Goal | Detail |
|---|---|
| High Score | 25,000 |
| Pro Score | 50,000 |
| S-K-A-T-E | 5 floating letters |
| Wreck 5 Cop Cars | Police cars around the level |
| Secret Tape | Jump into the plaza fountain, follow ramps to rooftops |

22 gaps. Rooftop network accessible from the plaza fountain.

#### Roswell (Roswell, NM) -- Competition (Final)
Medal placement only. 10 gaps. Hardest competition; requires strong trick knowledge and consistency.

### 3.5 Modes

| Mode | Description |
|---|---|
| Career | Progress through levels collecting tapes and medals |
| Single Session | 2-minute timed high score on any unlocked level |
| Free Skate | No timer, no objectives -- practice mode |

---

## 4. Playable Characters

### 4.1 Stat System

Four stats. Base values range from **3 to 7** across the roster, upgradeable toward **10** as career progresses. Stats increase automatically with tape collection; the player does not choose which stats to raise. (Collectible stat point pickups with player-chosen allocation were added in THPS2.)

| Stat | Effect |
|---|---|
| Ollie | Jump height |
| Speed | Movement speed and acceleration |
| Air | Hang time and height off ramps |
| Balance | Stability on grinds and lip tricks |

### 4.2 Roster

#### Vert Skaters

Higher Speed/Air, lower Ollie/Balance. Access to Fingerflip, Front-Foot Impossible, and Varial as diagonal flip tricks.

**Tony Hawk** -- Goofy, 6'2", Age 31

| Ollie | Speed | Air | Balance |
|---|---|---|---|
| 3 | 7 | 7 | 4 |

| Special Trick | Input | Pts |
|---|---|---|
| The 900 | Right, Down + Circle | 8,000 |
| Kickflip McTwist | Right, Right + Circle | 4,000 |
| 540 Board Varial | Left, Left + Square | 2,000 |
| 360 Flip to Mute | Down, Right + Square | 1,500 |

**Bob Burnquist** -- Regular, 5'11", Age 22

| Ollie | Speed | Air | Balance |
|---|---|---|---|
| 4 | 6 | 6 | 4 |

| Special Trick | Input | Pts |
|---|---|---|
| Backflip | Up, Down + Circle | 4,000 |
| Burntwist | Left, Up + Triangle | 5,000 |
| One-Footed Smith | Right, Right + Triangle | varies |

**Rune Glifberg** -- Regular, 5'11", Age 25

| Ollie | Speed | Air | Balance |
|---|---|---|---|
| 4 | 7 | 7 | 3 |

| Special Trick | Input | Pts |
|---|---|---|
| Kickflip McTwist | Right, Right + Circle | 4,000 |
| Christ Air | Left, Right + Circle | 2,100 |
| Front/Back Kickflip | Up, Down + Square | varies |

**Bucky Lasek** -- Regular, 5'11", Age 26

| Ollie | Speed | Air | Balance |
|---|---|---|---|
| 5 | 7 | 6 | 3 |

| Special Trick | Input | Pts |
|---|---|---|
| Varial McTwist | Right, Right + Circle | varies |
| Fingerflip Airwalk | Right, Left + Circle | 2,000 |
| Varial Heelflip Judo | Down, Up + Square | 2,500 |

#### Street Skaters

Higher Ollie/Balance, lower Speed/Air. Access to Hardflip, 360 Flip, and Sex Change as diagonal flip tricks.

**Kareem Campbell** -- Regular, 6'0"

| Ollie | Speed | Air | Balance |
|---|---|---|---|
| 7 | 4 | 4 | 6 |

| Special Trick | Input | Pts |
|---|---|---|
| Frontflip | Down, Up + Circle | 4,000 |
| Casper Slide | Up, Down + Circle | varies |
| Kickflip Underflip | Left, Right + Square | 1,500 |

**Chad Muska** -- Regular, 5'10", Age 22

| Ollie | Speed | Air | Balance |
|---|---|---|---|
| 6 | 4 | 4 | 7 |

| Special Trick | Input | Pts |
|---|---|---|
| Frontflip | Down, Up + Circle | 4,000 |
| One-Foot 5-0 Thumpin' | Right, Down + Triangle | varies |
| 360 Shove-It Rewind | Right, Right + Square | 1,500 |

**Andrew Reynolds** -- Regular, 6'2", Age 20

| Ollie | Speed | Air | Balance |
|---|---|---|---|
| 6 | 5 | 3 | 7 |

| Special Trick | Input | Pts |
|---|---|---|
| Backflip | Up, Down + Circle | 4,000 |
| Heelflip to Bluntside | Down, Down + Triangle | varies |
| Triple Kickflip | Right, Right + Square | 1,500 |

**Geoff Rowley** -- Regular, 5'8", Age 23

| Ollie | Speed | Air | Balance |
|---|---|---|---|
| 7 | 5 | 3 | 6 |

| Special Trick | Input | Pts |
|---|---|---|
| Backflip | Up, Down + Circle | 4,000 |
| Double Hardflip | Right, Down + Triangle | 1,500 |
| Dark Side | Right, Left + Triangle | varies |

**Jamie Thomas** -- Regular, 5'10", Age 24

| Ollie | Speed | Air | Balance |
|---|---|---|---|
| 5 | 5 | 4 | 7 |

| Special Trick | Input | Pts |
|---|---|---|
| Frontflip | Down, Up + Circle | 4,000 |
| One-Foot Nosegrind | Up, Up + Triangle | varies |
| 540 Flip | Left, Down + Square | 1,500 |

**Elissa Steamer** -- Regular, 5'4"

| Ollie | Speed | Air | Balance |
|---|---|---|---|
| 6 | 4 | 5 | 6 |

| Special Trick | Input | Pts |
|---|---|---|
| Backflip | Up, Down + Circle | 4,000 |
| Primo Grind | Left, Left + Triangle | varies |
| Judo Madonna | Left, Down + Circle | 1,500 |

#### Secret Characters

**Officer Dick** -- Regular, 5'11", Age 43
- **Unlock**: Complete all tapes/medals with any one character
- Stats: 5 / 5 / 5 / 5

| Special Trick | Input | Pts |
|---|---|---|
| Yeehaw Frontflip | Down, Up + Circle | 4,000 |
| Neckbreak Grind | Left, Right + Triangle | varies |
| Assume The Position | Left, Left + Circle | varies |

**Private Carrera** -- Cheat code only
- **Unlock**: Pause game, hold L1, enter: Triangle, Up, Triangle, Up, Circle, Up, Left, Triangle
- Stats: 10 / 10 / 10 / 10 (all maxed)

| Special Trick | Input | Pts |
|---|---|---|
| The Well Hardflip | Left, Down + Circle | 5,500 |
| Somi Spin | Left, Right + Square | 5,500 |
| Ho Ho Ho | Left, Up + Triangle | 2,000 |

Private Carrera can cause game freezes if the player restarts career as her.

---

## 5. Story & Progression

### 5.1 Career Structure

Career mode has no narrative story. The player selects a skater and progresses through 9 levels by collecting tapes (completing objectives) and earning competition medals. Each skater has an independent career save.

### 5.2 Completion Rewards

| Achievement | Reward |
|---|---|
| Medal at Roswell | Career complete |
| All Gold medals with a character | Real-life skateboarding video of that skater |
| All 30 tapes with a character | Unlocks Officer Dick |
| All tapes + all golds with all characters | Full completion |

### 5.3 Post-Game

No New Game+ system. Players can replay any unlocked level in Single Session or Free Skate. Completing career with additional characters unlocks their reward videos.

### 5.4 Cheat Codes

All cheats are entered from the **Pause menu** while holding **L1**.

| Cheat | Input (holding L1) |
|---|---|
| Unlock all levels, tapes, stats, Officer Dick | Circle, Right, Up, Down, Circle, Right, Up, Square, Triangle |
| Max all stats to 10 | Square, Triangle, Up, Down |
| Special meter always full | X, Triangle, Circle, Down, Up, Right |
| Slow motion | Square, Left, Up, Square, Left |
| Unlock Private Carrera | Triangle, Up, Triangle, Up, Circle, Up, Left, Triangle |

---

## 6. Items & Equipment

### 6.1 Collectibles

| Item | Location | Effect |
|---|---|---|
| S-K-A-T-E Letters | 5 per normal level, floating in the air | Collecting all 5 in one run earns a tape |
| Secret Tape | 1 per normal level, hidden in hard-to-reach spots | VHS cassette; earns a tape |
| Skateboards | Unlocked via career goals | Cosmetic deck swaps |

Objectives are tracked as "tapes" -- a period-appropriate reference to VHS videotapes. The secret tape collectible is a literal floating VHS cassette in the level geometry.

### 6.2 Level Interactables

Each normal level has 5 destructible or interactable objects as a goal:

| Level | Objects |
|---|---|
| Warehouse | 5 wooden boxes |
| School | 5 lunch tables (grind to destroy) |
| Mall | 5 directory signs |
| Downtown | 5 No Skating signs |
| Downhill Jam | 5 pipe valves (open) |
| Streets | 5 cop cars (wreck) |

---

## 7. Competition System

### 7.1 Format

- **3 heats** of **1 minute** each
- **5 judges** score each heat on a 0.0--99.9 scale
- Highest and lowest judge scores are **dropped**
- Remaining 3 scores are averaged for the heat score
- After all 3 heats, the **worst heat is dropped**
- Remaining 2 heat scores are averaged for the **final score**
- Player competes against **9 NPC skaters** (other roster members)
- **Top 3 earn medals**: 1st = Gold, 2nd = Silver, 3rd = Bronze

### 7.2 Judge Scoring Factors

| Factor | Impact |
|---|---|
| Total points scored | Primary driver |
| Trick variety | Heavy weight -- mixing different tricks |
| Special tricks used | Heavily weighted |
| Bails | Significant penalty |
| Combo quality | Rewards sustained, unbroken combos |

### 7.3 Target Scores

- **Skatepark Chicago**: Judge scores >91.5 needed for gold. ~14,500+ points with zero bails yields ~91. NPC scores cluster around 90--92.
- **Burnside**: Significantly harder than Chicago.
- **Roswell**: Hardest competition. Requires strong trick knowledge and consistency.

Medal thresholds are relative placement (top 3 out of 10), not fixed score values.

---

## 8. Economy

No currency or economy system. There are no shops, no purchasable items, and no trading. All progression is objective-based.

---

## 9. Multiplayer

All multiplayer is **local split-screen, 2 players**.

### 9.1 Trick Attack

Both players skate simultaneously on the same level. 2-minute timer. Highest cumulative score wins.

### 9.2 Graffiti

Level surfaces and objects can be "tagged" with a player's color by performing tricks on/over them. Performing a higher-scoring trick on an opponent's tagged surface steals it. Player with the most tagged surfaces when time expires wins.

### 9.3 HORSE

Turn-based trick duel. Each round: ~8 seconds or until a trick lands. The player with the lower score that round receives a letter. First to spell "HORSE" (or a custom word chosen pre-match) loses.

---

## 10. UI & HUD

### 10.1 Gameplay HUD

| Element | Position | Description |
|---|---|---|
| Score | Top-left | Running session score |
| Combo display | Center | Active combo trick list + running combo value; appears during combos, disappears on landing |
| Special Meter | Bottom-center | Gauge that fills with trick activity; flashes yellow when full |
| Timer | Top-right | Countdown from 2:00 (normal) or 1:00 (competition) |
| Trick name | Center | Name of the most recent trick performed |

### 10.2 Balance Feedback

THPS1 has **no visible balance meter**. Grind and lip trick balance is communicated solely through the skater's body animation -- the character wobbles visibly when balance is off. Players must read the skater's posture to maintain balance. The on-screen balance meter was added in THPS2.

### 10.3 Competition HUD

During competition heats, the standard HUD applies. Between heats, judge scores are displayed individually with the high and low scores crossed out. A scoreboard shows the player's ranking against NPC competitors.

### 10.4 Collectible Indicators

S-K-A-T-E letters, secret tapes, and destructible objects are visible in the 3D world as floating items or highlighted geometry. No minimap or radar. No waypoints.

### 10.5 Menus

- **Main Menu**: Career, Single Session, Free Skate, Multiplayer, Options
- **Pause Menu**: Resume, Restart Run, Options, Quit
- **Character Select**: Skater roster with stat bars displayed
- **Options**: Audio, controller config, autokick toggle

---

## 11. Engine & Presentation Systems

### 11.1 Camera

Third-person follow camera behind and slightly above the skater. The camera tracks the skater's movement direction and adjusts dynamically during air tricks and grinds. No manual camera control.

### 11.2 Save System

Career progress (tapes collected, medals earned, unlocked levels) saves per character. Each character's career is independent.

### 11.3 Audio System

Licensed soundtrack plays continuously during gameplay, cycling through tracks. Punk, ska, and hip-hop. 10 tracks on the NTSC-U PS1 version:

| # | Artist | Track |
|---|---|---|
| 1 | Dead Kennedys | "Police Truck" |
| 2 | Goldfinger | "Superman" |
| 3 | Primus | "Jerry Was a Race Car Driver" |
| 4 | The Suicide Machines | "New Girl" |
| 5 | The Vandals | "Euro-Barge" |
| 6 | Suicidal Tendencies | "Cyco Vision" |
| 7 | The Ernies | "Here & Now" |
| 8 | Unsane | "Committed" |
| 9 | Evenrude | "Vilified" |
| 10 | Speedealer | "Screamer" / "Nothing to Me" |

**PAL version** swapped 3 tracks: Speedealer replaced by Grand Unified "Le Hot '99", The Vandals replaced by Aim feat. YZ "Ain't Got Time to Waste", Evenrude replaced by Aquasky "Blue Thunder".

**N64 version** removed 3 tracks entirely (Euro-Barge, Vilified, Nothing to Me) and stripped vocals from 3 others (Jerry Was a Race Car Driver, Police Truck, Screamer). All tracks at lower bitrate. Character voice lines also removed.

Trick sound effects (board impacts, grinds, grabs) play contextually. Each skater has unique voice lines on bails and big tricks (PS1/Dreamcast only).

### 11.4 Difficulty

No selectable difficulty setting. Difficulty is implicit in the escalating score thresholds and competition AI across the 9 levels. Competition NPC scores increase across the three competition levels.

### 11.5 Physics

Frame-data details from the speedrun community:
- **Ollie**: Hold X to charge; release triggers jump. Acceleration occurs on frames 22--26 of the ollie animation.
- **Extended charge**: Holding X during frames 6--10 adds extra acceleration. Holding during frames 22--26 doubles the rate (caps after 3 frames).
- **Fastplant**: Up, Up input before ollie grants significantly more height and distance than a standard ollie.
- **Wallride window**: ~20 frames to input Triangle before touching a wall.
- **Wallride factors**: Speed (more = more upward momentum), distance from wall (closer = higher), angle of approach (perpendicular = higher/shorter; parallel = lower/farther).
- **Grind approach**: More parallel approach to a rail = faster grind speed. Approach angle determines which grind type initiates.

---

## 12. Version Differences

### 12.1 PlayStation (NTSC-U, September 29, 1999)

The definitive original version and the basis for this spec. Developed by Neversoft. Rated T for Teen. Full soundtrack, FMV cutscenes, per-skater ending videos, blood effects. 30 FPS target with PS1-era texture warping artifacts.

### 12.2 Nintendo 64 (March 15, 2000)

Ported by Edge of Reality. Rated E for Everyone.

- **Core gameplay identical** to PS1 -- same levels, characters, trick system, and modes
- **Blood removed** for E rating
- **Soundtrack severely cut**: 3 songs removed, 3 stripped to instrumentals, all at lower bitrate (see S11.3)
- **No FMV**: no ending videos, no in-level video screens
- **No voice lines**: character-specific sounds removed
- **Better visuals**: sharper than PS1, more stable framerate, faster load times (cartridge)

### 12.3 Dreamcast (May 24, 2000)

Ported by Treyarch (their first Tony Hawk project). Published by Crave Entertainment.

- **Core gameplay identical** to PS1
- **Visual upgrades**: higher-resolution textures, ~3x polygon count on character models, interpolated trick animations, dynamic shadows (PS1 used simple black ellipses), dynamic lighting
- **Full soundtrack and FMV** intact
- **Higher quality audio** than PS1
- 30 FPS including split-screen
- Planned online multiplayer was cut (Sega's network infrastructure wasn't ready)
- Dreamcast controller less ideal than DualShock for THPS inputs

### 12.4 Game Boy Color (March 30, 2000)

Developed by Natsume. A 2D isometric adaptation, not a port. Completely different gameplay from the 3D console versions.

### 12.5 Notable Omissions

- **No PC version** of THPS1 exists. The PC series started with THPS2.
- Tony Hawk's Pro Skater 2x (Xbox, 2001) included THPS1 career content as a bonus, but with THPS2 mechanics (manuals, stat points) and inflated starting stats, trivializing the original challenges.

---

## 13. Open Questions / Unverified

### Combo Score Formula
The interaction between spin rotation and the combo multiplier has conflicting descriptions across sources. Some treat spin as a modifier on individual trick values (used in this spec, S1.4); others describe spin as directly adding to the combo multiplier. The Fandom wiki clearly states the combo multiplier is incremented by trick count, and the speedrun community's spin table (180=~2x, 360=~3x) most likely applies to individual trick values. The exact multipliers per 180 degrees of spin may not be clean integers.

### Trick Repetition Decay
Sources conflict on exact percentages:
- **Slateman FAQ (Dreamcast)**: 100% / 75% / 50% / 25% / 10%
- **Speedrun.com Mechanics Guide**: 100% / 90% / 75% / 50% / 25%
- **Wikipedia**: "down to one-tenth of their original score"

The speedrun data is likely more precise (frame-level analysis). The Dreamcast FAQ may reflect version-specific values or approximations. This spec uses the Slateman figures (S1.6) as a conservative model.

### Tape Unlock Thresholds
Exact tape counts to unlock Mall, Skatepark Chicago, Downhill Jam, and Burnside are inconsistently documented. School = 2 and Roswell = 26 are confirmed. Others are best estimates from speedrun route data.

### Special Trick Point Values
Confirmed values: The 900 = 8,000; Kickflip McTwist = 4,000; Backflip/Frontflip = 4,000; Burntwist = 5,000; 540 Board Varial = 2,000; 360 Flip to Mute = 1,500; Triple Kickflip/Double Hardflip/etc. = 1,500. Grind specials have variable values (tick-based). Some character-specific special values remain individually unconfirmed.

### Rune Glifberg Stats
One source lists Rune's stats as Ollie 4, Speed 7, Air 7, Balance 3 (matching other vert skaters). Another source swaps Ollie/Balance with Speed/Air (Ollie 7, Speed 4, Air 4, Balance 6 -- a street-skater profile). The vert-skater profile is more likely given his classification.

### Stat Progression Formula
Stats increase automatically with career progression, but the exact formula (how many tapes = how many stat points, which stats increase first) is undocumented.

### Competition NPC Score Ranges
No source provides exact NPC score distributions per competition level. Gold placement requires outscoring 9 NPC competitors, whose scores increase across the three competitions.

---

## 14. References

### Wikis
- [Tony Hawk's Games Wiki (Fandom)](https://tonyhawkgames.fandom.com/) -- Competition, Balance Meter, Combo, Career Mode, Special Tricks articles
- [StrategyWiki -- Tony Hawk's Pro Skater](https://strategywiki.org/wiki/Tony_Hawk%27s_Pro_Skater) -- Controls, level walkthroughs

### FAQs & Guides
- [Slateman's Move List FAQ (GameFAQs)](https://gamefaqs.gamespot.com/ps/199060-tony-hawks-pro-skater/faqs/5074) -- Complete move list with per-character tricks
- [CheatCodes.com FAQ/Walkthrough (Guide 15891)](http://www.cheatcodes.com/guide/faq-walkthrough-tony-hawks-pro-skater-playstation-15891/) -- Full walkthrough with score thresholds
- [CheatCodes.com Strategy Guide (Guide 12571)](http://www.cheatcodes.com/guide/strategy-guide-tony-hawks-pro-skater-playstation-12571/) -- Character stats and special trick values

### Speedrun Community
- [Speedrun.com THPS1 Mechanics Guide](https://www.speedrun.com/thps1/guides/1wg8v) -- Frame data, ollie mechanics, speed mechanics, trick decay

### Technical
- [Tony Hawk's Underground Source (GitHub)](https://github.com/RetailGameSourceCode/TonyHawksUnderground) -- THUG (2003) source code; evolved from the same Neversoft engine as THPS1. Physics constants may be similar/descended.
- [32bits Substack -- Under the Microscope: THPS2](https://32bits.substack.com/p/under-the-microscope-tony-hawks-pro) -- Engine analysis of the THPS2 port (shares engine lineage)

### General
- [Wikipedia -- Tony Hawk's Pro Skater](https://en.wikipedia.org/wiki/Tony_Hawk%27s_Pro_Skater_(video_game)) -- Release dates, version differences, general overview
