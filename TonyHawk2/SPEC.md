# Tony Hawk's Pro Skater 2 — Gameplay Systems Spec

PlayStation 1, September 20, 2000. Developed by Neversoft, published by Activision.

---

## 1. Core Gameplay Systems

### 1.1 Primary Loop

The player controls a skateboarder through timed 2-minute runs in skate park environments. The core loop is: build speed, ollie into the air or onto rails, perform trick combos, land cleanly, and repeat. Points are earned only when a combo lands successfully — bailing forfeits all accumulated combo points.

THPS2's defining innovation over its predecessor is the **Manual** — a flatground balance trick that links air tricks and grinds into extended combos. This turns the game from isolated trick-score-land cycles into a continuous combo-chaining system.

### 1.2 Trick System

Five trick categories, each mapped to a button:

| Category | Button | Context | Hold Mechanic |
|----------|--------|---------|---------------|
| Flip Tricks | Square | Airborne | Instant — no hold |
| Grab Tricks | Circle | Airborne | Hold for more points |
| Grind Tricks | Triangle | Near rail/ledge | Accumulates over time |
| Lip Tricks | Triangle | At vert ramp lip | Accumulates over time |
| Manual | D-pad (Up→Down or Down→Up) | On ground | Accumulates over time |

**Flip Tricks**: Performed with direction + Square while airborne. 12 assignable slots per skater. Base values range from ~100 (Pop Shove-It, Kickflip) to ~500 (Kickflip to Indy, Heelflip Varial Lien). No hold mechanic — score is fixed per execution.

**Grab Tricks**: Performed with direction + Circle while airborne. 12 assignable slots per skater. Base values range from ~150 (Nosegrab, Tailgrab) to ~250 (Airwalk, Judo). Holding the button accumulates additional points (~50 pts per half-second).

**Grind Tricks**: Performed by pressing Triangle near any grindable edge. The specific grind depends on approach angle and directional input. Grinds are **not customizable** — they cannot be swapped in trick slots. Points accumulate per tick while grinding (~137–180 pts/sec depending on grind type). A "Kiss the Rail" (very short grind) awards ~25 points.

**Lip Tricks**: Performed by pressing Triangle + direction at the top of a halfpipe/quarterpipe. 4 assignable slots per skater. Base values range from ~250 (Axle Stall) to ~400+ (Disaster, One Foot Invert). Points accumulate while held.

**Manual / Nose Manual**: The combo connector. Up→Down for Manual, Down→Up for Nose Manual. Awards ~50 base points. Speed decreases during manuals.

**Special Tricks**: High-value signature moves requiring a full Special Meter (§1.4). Input: two directional taps + a trick button (e.g., Right, Down + Circle). Each skater has 3 default specials. Point values range from 7,500 to 20,000. See §4 for per-skater specials.

### 1.3 Combo & Scoring System

A combo is an unbroken chain of tricks. The display shows base score and multiplier separately. Final combo score is awarded only on a clean landing.

**Formula**:

```
Combo Score = (Sum of spin-modified trick values) × (Combo Multiplier)
```

Where:
- **Individual trick value** = `base_value × rotation_multiplier × degradation × landing_modifier`
- **Combo Multiplier** = count of tricks + count of gaps in the combo. Caps at **×30**.

Rotations modify the individual trick's base value **before** it enters the combo sum. They do **not** add to the combo multiplier.

**Rotation multiplier** (applied per trick):

| Rotation | Multiplier |
|----------|-----------|
| 180 | 1.5× |
| 360 | 2.0× |
| 540 | 3.0× |
| 720 | 4.0× |
| 900 | 5.0× |
| 1080 | 6.0× |

**Example**: A 360 Kickflip (100 × 2.0 = 200) followed by a Manual (50) = 250 base × 2 (two tricks) = **500 points**.

**Combo chaining** (the THPS2 innovation):
```
Air Trick → Manual → Grind → Manual → Air Trick → Manual → Grind → ...
```
Without the Manual, combos end on every landing. With it, skilled players can chain tricks across an entire level in a single combo. Switching between Manual and Nose Manual mid-balance counts as a separate trick for multiplier purposes.

**Landing quality**:
- **Perfect** (green): +50% bonus on trick value
- **Sloppy** (red): −30% penalty on trick value

**Trick degradation** (per session):

| Repetition | Score Retained |
|------------|---------------|
| 1st | 100% |
| 2nd | 75% |
| 3rd | 50% |
| 4th | 25% |
| 5th+ | 10% |

Switch-stance tricks are tracked separately from regular-stance versions. Performing a trick in switch also grants a **+20% base value bonus**.

**Bail conditions**:
- Overspin on landing (not aligned with direction of travel)
- Balance meter reaches either extreme during manual or grind
- Landing from excessive height without pressing X (Big Drop recovery)
- Colliding with walls/objects at high speed
- Bailing empties the Special Meter completely and forfeits all combo points

### 1.4 Special Meter

A horizontal bar displayed below the score counter.

- **Fills by**: Landing tricks. Higher-scoring combos fill it faster. Approximately 3,000 points in a single landed combo fills the meter completely.
- **Visual**: Glows **gold** when full.
- **Enables**: Special Trick inputs become available. Maximum speed increases.
- **Drain**: Constantly decreases when not performing tricks.
- **Bail**: Completely empties the meter.
- **No "on fire" state**: THPS2 has only binary full/not-full. The unlimited-special "on fire" mechanic was introduced in later entries.

### 1.5 Balance Meter

**Manual Balance**: A visible vertical bar appears during manuals. The player must counterbalance with left/right D-pad inputs. The longer the manual, the more erratic the meter becomes. The Manual Balance stat affects starting stability.

**Grind Balance**: In the PS1 original, grind balance is **invisible** — the skater's visual tilt is the only indicator. A visible grind balance meter was added in THPS2x (Xbox). The Rail Balance stat affects stability.

**Lip Balance**: Similar to grind balance for held lip tricks. The Lip Balance stat affects stability.

Balance difficulty increases when the same balance type is used multiple times consecutively in a combo.

### 1.6 Stat System

Each skater has 10 attributes on a **1–10 scale**:

| Stat | Effect |
|------|--------|
| Air | Height off vert ramps / quarterpipes |
| Hangtime | Duration spent in the air |
| Ollie | Jump height from flat ground |
| Speed | Maximum ground speed |
| Spin | Rotation speed in air (L1/R1) |
| Landing | Forgiveness on landing angle (perfect/sloppy threshold) |
| Switch | Performance and stability when riding switch |
| Rail Balance | Grind balance stability |
| Lip Balance | Lip trick balance stability |
| Manual Balance | Manual balance stability |

All 13 default pro skaters start with exactly **50 total stat points**. Additional stat points are purchased in the Skate Shop using career cash. See §4 for per-skater starting distributions.

---

## 2. Controls & Input

### 2.1 PS1 Control Scheme

| Button | Ground | Air | On Rail / At Lip |
|--------|--------|-----|------------------|
| **D-Pad** | Steer / Brake (Down) | Trick direction modifier | Balance correction |
| **X** | Ollie (hold to crouch, release to jump) | — | — |
| **Square** | — | Flip trick (+ direction) | — |
| **Circle** | — | Grab trick (+ direction, hold) | — |
| **Triangle** | — | Wallride (near wall) | Grind (+ direction) / Lip trick (+ direction) |
| **L1** | — | Spin left | — |
| **R1** | — | Spin right | — |
| **R2** | Toggle switch stance | Toggle switch stance | — |
| **Start** | Pause menu | — | — |

### 2.2 Advanced Inputs

| Move | Input | Base Points | Notes |
|------|-------|-------------|-------|
| Manual | Up, Down (on ground) | ~50 | Combo connector; balance meter appears |
| Nose Manual | Down, Up (on ground) | ~50 | Alternate manual; separate trick for multiplier |
| Nollie | Up + X (before ollie) | ~200 | Nollie bonus added to next trick |
| No Comply | Up, X (quick) | ~200 | Quick launch variant |
| Boneless / Fastplant | Up, Up, X (within ~16 frames) | ~250 | Higher launch, more air time |
| Big Drop Recovery | X just before landing | 0 | Prevents bail from excessive height |
| Wallride | Triangle while airborne near wall | Varies | Can ollie off wall for continued combo |
| Special Trick | Dir, Dir + Button | 7,500–20,000 | Requires full Special Meter |

### 2.3 Ollie Mechanics

Hold X to crouch — longer hold means higher ollie. Maximum hold is approximately **0.5 seconds** for full height. The Ollie stat affects maximum height from flat ground; the Air stat affects height off vert ramps.

### 2.4 Auto-Kick

A toggle in the Options menu (default: **on**).

- **On**: The skater automatically pushes to maintain forward momentum, but speed gradually decays on flat ground. Crouching (holding X) slows the rate of speed decay.
- **Off**: No automatic pushing. However, speed does **not decay** on flat ground — if you ollie off a grind onto flat, you maintain that exact speed indefinitely until you hit something or go uphill. Preferred setting for speedrunners and advanced players.

---

## 3. World Structure

### 3.1 Level Progression

Career Mode has **10 levels** played in fixed order. Levels unlock based on cumulative cash earned from completing goals.

| # | Level | Type | Goals | Time Limit |
|---|-------|------|-------|------------|
| 1 | The Hangar | Regular | 10 | 2:00 |
| 2 | School II | Regular | 10 | 2:00 |
| 3 | Marseille | Competition | Medal | 3 × 1:00 |
| 4 | NY City | Regular | 10 | 2:00 |
| 5 | Venice Beach | Regular | 10 | 2:00 |
| 6 | Skatestreet | Competition | Medal | 3 × 1:00 |
| 7 | Philadelphia | Regular | 10 | 2:00 |
| 8 | Bullring | Competition | Medal | 3 × 1:00 |
| 9 | Chopper Drop | Secret / Free Skate | None | Untimed |
| 10 | Skate Heaven | Secret / Free Skate | None | Untimed |

Secret levels are bonus playground arenas with no career goals, score thresholds, S-K-A-T-E letters, or secret tapes. They contain gaps to discover and cash to collect but no formal objectives.

- **Chopper Drop**: A lone halfpipe shaped like a ship hull floating in the Pacific Ocean. Features helicopter skids for grinding/lip tricks above the halfpipe and distance-based gaps (70 ft, 80 ft, 90 ft launches).
- **Skate Heaven**: An outer-space themed level with loop-de-loops, translucent halfpipes, a volcano activated by grinding a specific rail, and surreal skate structures.

**Secret level unlock conditions**:
- **Chopper Drop**: Earn Gold Medals in all 3 competitions with every pro skater
- **Skate Heaven**: Achieve 100% completion with every character (all pro skaters, Officer Dick, Spider-Man, and a custom skater)

### 3.2 Goal Types (Regular Levels)

Each regular level has 10 goals following a consistent structure: 3 score thresholds, S-K-A-T-E letters, a themed collectible set, a trick challenge, a level-specific objective, a gap challenge, the secret tape, and 100% completion.

**The Hangar** (High: 10,000 / Pro: 25,000 / SICK: 75,000):

| # | Goal | Cash |
|---|------|------|
| 1–3 | Score thresholds (above) | $100 / $200 / $500 |
| 4 | Collect S-K-A-T-E | $150 |
| 5 | Collect 5 Pilot Wings | $250 |
| 6 | Barrel Hunt | $150 |
| 7 | Nosegrind over the Pipe | $150 |
| 8 | Hit 3 Hangtime Gaps | $150 |
| 9 | Find the Secret Tape | $150 |
| 10 | 100% Goals and Cash | $200 + $500 |

**School II** (High: 15,000 / Pro: 40,000 / SICK: 100,000):

| # | Goal | Cash |
|---|------|------|
| 1–3 | Score thresholds (above) | $200 / $350 / $500 |
| 4 | Collect S-K-A-T-E | $400 |
| 5 | Wallride 5 Bells | $500 |
| 6 | Collect 5 Hall Passes | $400 |
| 7 | Kickflip TC's Roof Gap | $400 |
| 8 | Grind 3 Roll Call Rails | $500 |
| 9 | Find the Secret Tape | $500 |
| 10 | 100% Goals and Cash | $500 + $750 |

**NY City** (High: 20,000 / Pro: 50,000 / SICK: 150,000):

| # | Goal | Cash |
|---|------|------|
| 1–3 | Score thresholds (above) | $750 / $1,000 / $1,250 |
| 4 | Collect S-K-A-T-E | $800 |
| 5 | Ollie the Hydrants | $700 |
| 6 | Collect 5 Subway Tokens | $800 |
| 7 | 50-50 Joey's Sculpture | $900 |
| 8 | Grind the Subway Rails | $1,100 |
| 9 | Find the Secret Tape | $1,200 |
| 10 | 100% Goals and Cash | $500 + $1,000 |

**Venice Beach** (High: 40,000 / Pro: 100,000 / SICK: 200,000):

| # | Goal | Cash |
|---|------|------|
| 1–3 | Score thresholds (above) | $1,500 / $1,750 / $2,000 |
| 4 | Collect S-K-A-T-E | $1,250 |
| 5 | Ollie the Magic Bum 5 times | $1,500 |
| 6 | Collect 5 Spray Cans | $1,250 |
| 7 | Tailslide Venice Ledge | $1,000 |
| 8 | Hit 4 VB Transfers | $1,500 |
| 9 | Find the Secret Tape | $1,250 |
| 10 | 100% Goals and Cash | $500 + $1,500 |

**Philadelphia** (High: 50,000 / Pro: 125,000 / SICK: 250,000):

| # | Goal | Cash |
|---|------|------|
| 1–3 | Score thresholds (above) | $2,500 / $3,000 / $4,000 |
| 4 | Collect S-K-A-T-E | $2,000 |
| 5 | Drain the Fountain | $2,500 |
| 6 | Collect 5 Bells | $2,000 |
| 7 | Bluntside the Awning | $2,000 |
| 8 | Liptrick 4 Skatepark Lips | $2,500 |
| 9 | Find the Secret Tape | $2,000 |
| 10 | 100% Goals and Cash | $500 + $2,000 |

Competition levels use judge-based scoring instead of thresholds (§3.3).

### 3.3 Competition Format

Three career levels are competitions: **Marseille**, **Skatestreet**, **Bullring**.

- **3 rounds**, 1 minute each
- **5 judges** score each run on a **0–99.9 scale**
- Highest and lowest judge scores are **discarded**; remaining 3 are averaged for the run score
- Only the **2 best run scores** are summed for the final score (weakest run dropped; max theoretical: ~199.8)
- Player must place **3rd or better** to earn a medal and advance
- Judges evaluate: trick variety, trick difficulty, execution quality (bails heavily penalize scores)
- ~60,000 trick points with no bails typically yields scores in the 90s; ~200,000+ maxes out all judges at 99.9
- No fixed score threshold for medals — player must outscore the AI opponents
- Gold Medal counts as a level goal

---

## 4. Playable Characters

### 4.1 Default Roster (13 Pro Skaters)

All stats on a 1–10 scale. Total: 50 for all default skaters.

| Skater | Stance | Air | Hang | Ollie | Speed | Spin | Land | Switch | Rail | Lip | Manual |
|--------|--------|-----|------|-------|-------|------|------|--------|------|-----|--------|
| Tony Hawk | Goofy | 7 | 5 | 2 | 6 | 8 | 5 | 3 | 3 | 6 | 5 |
| Bob Burnquist | Regular | 6 | 5 | 5 | 5 | 5 | 3 | 8 | 2 | 7 | 4 |
| Steve Caballero | Goofy | 6 | 5 | 5 | 6 | 3 | 5 | 5 | 6 | 5 | 4 |
| Kareem Campbell | Regular | 7 | 2 | 6 | 5 | 8 | 4 | 5 | 6 | 2 | 5 |
| Rune Glifberg | Regular | 7 | 7 | 5 | 6 | 5 | 3 | 4 | 4 | 6 | 3 |
| Eric Koston | Goofy | 4 | 3 | 7 | 5 | 4 | 4 | 7 | 7 | 3 | 6 |
| Bucky Lasek | Regular | 7 | 7 | 3 | 6 | 7 | 3 | 5 | 3 | 7 | 2 |
| Rodney Mullen | Goofy | 2 | 2 | 6 | 5 | 8 | 2 | 8 | 7 | 2 | 8 |
| Chad Muska | Regular | 4 | 3 | 8 | 6 | 4 | 7 | 5 | 8 | 3 | 2 |
| Andrew Reynolds | Regular | 4 | 2 | 8 | 4 | 5 | 7 | 5 | 8 | 4 | 3 |
| Geoff Rowley | Regular | 6 | 2 | 6 | 4 | 5 | 5 | 4 | 8 | 7 | 3 |
| Elissa Steamer | Regular | 6 | 4 | 5 | 5 | 5 | 4 | 5 | 6 | 5 | 5 |
| Jamie Thomas | Regular | 4 | 3 | 7 | 6 | 4 | 8 | 4 | 7 | 4 | 3 |

**Character archetypes**:
- **Vert specialists** (high Air/Hangtime/Lip): Tony Hawk, Rune Glifberg, Bucky Lasek
- **Street specialists** (high Ollie/Rail/Manual): Rodney Mullen, Eric Koston, Chad Muska, Andrew Reynolds
- **All-around**: Bob Burnquist, Steve Caballero, Elissa Steamer

### 4.2 Special Tricks Per Skater

Each skater has 3 signature specials. Input format: Direction, Direction + Button.

| Skater | Special 1 | Special 2 | Special 3 |
|--------|-----------|-----------|-----------|
| **Tony Hawk** | The 900 (R,D+Circle) 15,000 | Sacktap (U,D+Circle) 10,000 | BS Overturn (D,L+Tri) 8,500 |
| **Bob Burnquist** | Racket Air (L,D+Circle) 10,000 | BS Rocket Tailslide (U,D+Tri) 9,000 | One Foot Smith (R,D+Tri) 7,500 |
| **Steve Caballero** | FS 540 (R,L+Circle) 9,500 | Triple Kickflip (U,L+Sq) | Hang Ten Nosegrind (R,U+Tri) 10,000 |
| **Kareem Campbell** | Ghetto Bird (D,U+Sq) 11,000 | Nosegrind to Pivot (D,U+Tri) | Casper (L,D+Circle) |
| **Rune Glifberg** | Christ Air (L,R+Circle) 8,500 | Kickflip 1-Foot Tail (D,U+Tri) | One Foot Bluntslide (L,U+Tri) |
| **Eric Koston** | Pizza Guy (D,L+Circle) 11,500 | Indy Frontflip (D,U+Circle) 11,000 | The Fandangle (R,D+Tri) 9,500 |
| **Bucky Lasek** | Fingerflip Airwalk (L,R+Circle) 8,000 | One Foot Japan (U,R+Circle) | The Big Hitter (L,D+Tri) |
| **Rodney Mullen** | Triple Heelflip (U,R+Sq) | Hardflip Late Flip (U,D+Sq) | Nosegrab Tailslide (U,D+Tri) |
| **Chad Muska** | Mute Backflip (U,D+Circle) 11,500 | Half Flip Casper (R,L+Sq) | BS Hurricane (D,R+Tri) |
| **Andrew Reynolds** | Nollie Flip Underflip (D,L+Sq) | Heelflip Darkslide (R,L+Tri) 11,500 | BS Hurricane (D,R+Tri) |
| **Geoff Rowley** | Mute Backflip (U,D+Circle) | Half Flip Casper (R,L+Sq) | Rowley Darkslide (L,R+Tri) 9,500 |
| **Elissa Steamer** | Indy Backflip (U,D+Circle) 11,500 | Hospital Flip (L,R+Sq) | Madonna Tailslide (U,L+Tri) |
| **Jamie Thomas** | Laser Flip (D,R+Sq) 9,000 | Kickflip 1-Foot Tail (L,D+Sq) | Beni F-Flip Crooks (D,U+Tri) |

*Key: U=Up, D=Down, L=Left, R=Right, Sq=Square, Tri=Triangle. Point values listed where confirmed; unlisted values are in the 7,500–11,500 range.*

### 4.3 Secret Characters

Characters unlock via the progressive cheat system (§5.3) or through specific alternate paths:

| Character | Progressive Unlock | Alternate Path |
|-----------|-------------------|----------------|
| **Officer Dick** | 1st career 100% completion | — |
| **McSqueeb** (80's Tony) | 4th career 100% completion | Complete 100% with Tony Hawk specifically |
| **Spider-Man** | 12th career 100% completion | Complete 100% with a created skater |
| **Private Carrera** | — | Complete every gap in the gap checklist (excluding secret levels; Free Skate completions count) |

**Spider-Man stats**: Air 7, Hangtime 7, Ollie 5, Speed 5, Spin 5, Landing 4, Switch 4, Rail 6, Lip 3, Manual 4 (total: 50).

Spider-Man specials: Spidey Flip (U,D+Circle), Spidey Grind (L,R+Tri), Spidey Varial (L,R+Sq).

Officer Dick specials: Assume the Position (grab), Lazy Azz Grind (grind), Salute (grab). Private Carrera specials: Ho-Ho Handplant (lip), Double Splits (grab), Ho Slide (grind).

### 4.4 Create-A-Skater

- **4 custom skater slots**
- **Biography**: Name, hometown, age, stance (Regular/Goofy), weight (88–378 lbs)
- **Style**: All-Around, Vert, or Street (determines starting trick loadout)
- **Starting stats**: 5 free stat points to allocate; additional points purchased via career cash
- **Appearance**: Complexion/skin tone, hairstyle, torso, pants, shoes, socks
- **Trick customization**: Starting tricks depend on chosen style; new tricks and specials purchased from Skate Shop
- **Decks**: Start with 10 decks (vs. pro skaters who start with 1); 10 more purchasable
- **PS1 limitation**: Male-only custom skaters; THPS2x (Xbox) added female option

---

## 5. Story & Progression

### 5.1 Game Modes

| Mode | Players | Description |
|------|---------|-------------|
| Career | 1 | Progress through levels, complete objectives, earn cash, upgrade skater (§5.2–5.3) |
| Single Session | 1 | 2-minute high-score run on any unlocked level using current skater loadout |
| Free Skate | 1 | No timer, no objectives — practice and explore |
| 2 Player | 2 | Access to multiplayer modes (§8) |

### 5.2 Career Mode Structure

Career Mode is a level-by-level progression through increasingly difficult skate environments. There is no narrative story — progression is purely goal-based.

1. Complete goals in each level to earn cash
2. Cumulative cash unlocks subsequent levels
3. Place 3rd or better in competitions to advance past them
4. Completing Career Mode (beating Bullring) triggers a bail montage video
5. All Gold Medals across competitions unlocks a skater-specific ending video
6. 100% completion with all characters unlocks Skate Heaven

### 5.3 Unlockable Cheats

Each time a player completes Career Mode 100% (all goals + all cash + gold medals) with a different eligible skater, the next cheat in the sequence unlocks. Tony Hawk, Officer Dick, and Spider-Man completions give their own specific rewards (§4.3) and do not advance the cheat sequence — only the other 12 default pro skaters and created skaters count.

Cheats are toggled in the Cheats submenu from the main menu or pause menu. The unlock sequence (sources conflict on exact ordering after #4):

| # | Cheat | Effect |
|---|-------|--------|
| 1 | Officer Dick | Unlocks secret character |
| 2 | Skip to Restart | Allows restarting from any checkpoint |
| 3 | Kid Mode | Skaters become child-sized; enhanced stats |
| 4 | 80's Tony (McSqueeb) | Unlocks retro Tony Hawk character |
| 5 | Perfect Balance | Infinite balance on manuals, grinds, and lip tricks |
| 6 | Always Special | Special Meter stays full permanently |
| 7 | Stud Mode | Maxes all stats (reportedly to 13, beyond the normal 10 cap) |
| 8 | Weight Mode | Alters skater body proportions (fat/thin); stackable |
| 9 | Wireframe Mode | Renders all geometry as wireframe |
| 10 | Slow-NIC | Tricks play in slow motion during jumps and grinds |
| 11 | Big Head Mode | Enlarged skater heads |
| 12 | Spider-Man | Unlocks secret character |
| 13 | Moon Physics | Reduced gravity; higher/longer jumps |
| 14 | Sim Mode | Realistic physics — less air, faster ground speed |
| 15 | Smooth Mode | Removes all textures; flat-shaded surfaces |

Additional cheats available via **pause menu cheat codes** (hold L1, then enter button sequence):
- **Level Flip**: Mirror mode (all levels flipped horizontally)
- **Blood Mode**: Blood squirts from skater's nose on bails (cosmetic only; removed from N64 version)
- **Flight Mode**: Skater can fly freely through levels
- **All Levels**: Unlocks all levels immediately
- **Extra Money**: Adds cash to career wallet

---

## 6. Trick Customization & Skate Shop

### 6.1 Customizable Trick Slots

| Category | Slots | Customizable? |
|----------|-------|--------------|
| Flip Tricks | 12 | Yes — assign any purchased flip to any directional input |
| Grab Tricks | 12 | Yes — assign any purchased grab to any directional input |
| Lip Tricks | 4 | Yes — assign any purchased lip trick to any directional input |
| Special Tricks | 3 | Yes — any purchased special can be assigned to any skater |
| Grind Tricks | All | **No** — grinds are fixed, determined by approach angle |
| Manual | 2 | **No** — Manual and Nose Manual are fixed |

### 6.2 Special Trick Pool

~40+ unique special tricks exist in the game's pool (signature moves from all characters plus generic specials). **Any skater can equip any 3 specials** from this pool once purchased. This means Tony Hawk can use Rodney Mullen's specials, and vice versa.

### 6.3 Skate Shop Pricing

| Item | Price Range |
|------|------------|
| Stat point upgrade | ~$500 per point |
| Standard trick | ~$500 per trick |
| Special trick | $3,750–$15,000 (The 900 is the most expensive at $15,000) |
| Skateboard decks | Varies (cosmetic only — no stat effect) |

148 total skateboard decks across all skaters.

---

## 7. Economy

### 7.1 Currency

**Cash ($)** — the single currency. Earned by completing goals and collecting cash icons in levels.

### 7.2 Income Sources

| Source | Amount |
|--------|--------|
| Goal completion | Scales with level difficulty ($100–$4,000 per goal) |
| Cash icon pickups | Scattered through each level |
| Competition medals | Marseille: $7,000, Skatestreet: $20,000, Bullring: $65,000 |

**Maximum career cash**: $150,000 (represents full career clear).

### 7.3 Cash Per Level (Approximate Totals)

| Level | Total Cash Available |
|-------|---------------------|
| The Hangar | ~$2,350 |
| School II | ~$4,350 |
| Marseille | ~$7,500 |
| NY City | ~$9,950 |
| Venice Beach | ~$12,750 |
| Skatestreet | ~$21,000 |
| Philadelphia | ~$22,000 |
| Bullring | ~$67,500 |

### 7.4 Sinks

- **Stat point upgrades** (~$500 each)
- **New tricks** (~$500 each for standard; $3,750–$15,000 for specials)
- **Skateboard decks** (cosmetic)

---

## 8. Multiplayer

All multiplayer modes are **local 2-player split-screen** (horizontal split, top/bottom). No online play on PS1.

### 8.1 Modes

**Trick Attack**
- Both players compete for highest score within a set time limit (1, 2, 5, or 10 minutes)
- If players collide, the slower player bails and loses their combo

**Graffiti**
- Players "tag" skateable objects by tricking on them; objects change to the player's color when the combo lands
- An opponent can steal a tagged object by scoring a higher combo on it
- Most tagged objects at time expiry wins
- Bailing colors nothing

**HORSE**
- Custom word (3–10 letters; default "HORSE")
- Alternating 10-second turns; each player performs one combo
- Lower scorer (or bailer) earns a letter
- After each exchange, both players move to a new location
- First to spell the full word loses

**Tag**
- Both players get 10 seconds to score; lower scorer becomes "it"
- The tagged player must touch the opponent before their timer runs out
- When "it" tags the opponent, the opponent's timer begins and the tagger's stops
- Performing tricks while "it" increases speed and tag range

**Free Skate (2-Player)**
- No timer, no objectives — practice together

---

## 9. Minigames & Side Systems

### 9.1 Gap System

Gaps are pre-defined named challenges tied to specific locations. When the player performs the correct trick type at the correct location, the gap name appears on screen with a bonus point value added to the current combo.

**Gap count by level**:

| Level | Gaps |
|-------|------|
| The Hangar | 21 |
| School II | 43 |
| Marseille | 33 |
| NY City | 41 |
| Venice Beach | 41 |
| Skatestreet | 36 |
| Philadelphia | 41 |
| Bullring | 31 |
| Chopper Drop | 9 |
| Skate Heaven | 71 |
| **Total** | **~367** |

**Gap types**: Air, Grind, Manual, Lip, Wallride, Ground. Each requires the matching trick category to trigger.

**Gap bonus values**: Range from 50 pts (small transfers) to 5,000+ pts (complex multi-part gaps). Gap bonuses **do not degrade** with repetition. Gap bonuses add to combo score before the multiplier and also add +1 to the multiplier. Completing all gaps in the game unlocks Private Carrera.

### 9.2 Create-A-Park

| Property | Value |
|----------|-------|
| Grid sizes | 16×16, 24×24, 30×30, 30×18, 60×6 |
| Themes | Power Plant, Industrial, Outdoor, School (cosmetic only) |
| Total pieces | 100+ across 18 categories |
| Max saved parks | 50 |

**Piece categories** (18 sets): Starts (spawn points), Gaps (custom gap markers with naming and point values), Risers (elevation), Quarter Pipes (metal and wood variants), Rails, Offset Rails, Low Walls, Slopes, Stairs, Pools, Funboxes, Kickers, Benches, Signs, Floor textures, Foliage, Misc (dividers/pillars).

**Custom gaps**: Players can set two markers, name the gap, assign a 50–5,000 point value, and specify the gap type (air/rail/manual/wallride).

Pieces are rotatable and resizable. Includes a Test Play mode to skate mid-construction. PS1 version: local save only (Memory Card), no sharing capability.

---

## 10. UI & HUD

### 10.1 HUD Layout

| Element | Position | Details |
|---------|----------|---------|
| Score counter | Top-left | Running session total |
| Special Meter | Top-left (below score) | Horizontal bar; glows gold when full |
| Timer | Top-right | Countdown from 2:00 (career/single session) |
| Trick string | Bottom-center | Stacked list of trick names in current combo |
| Base score + Multiplier | Bottom-center (below trick string) | e.g., "4,500 × 5" — shows running combo value |
| Balance meter | Near skater (center) | Vertical bar during manuals; arrow indicator during grinds (Xbox only) |
| Gap notification | Center | Blue text flash when a new gap is landed |

### 10.2 HUD States

- **Normal gameplay**: Full HUD visible
- **Combo active**: Trick string and multiplier appear at bottom-center; grows as combo extends
- **Special active**: Special Meter glows gold; special trick inputs become available
- **Competition**: Judge scores replace the standard timer; round indicator shown
- **Pause**: Full-screen pause menu overlay
- **2-Player**: Horizontal split (top/bottom), each half with its own complete HUD

### 10.3 Menu Structure

**Main Menu**: Career Mode, Single Session, Free Skate, 2 Player, Create Skater, Park Editor, Options, Movies (unlocked FMVs), Cheats

**Pause Menu**: Resume, Restart Run, End Run, Volume, Change Soundtrack (skip to next song)

**Options**: Controller Configuration (remap for P1/P2), Auto-Kick toggle (§2.4), Vibration toggle, Sound/Music volume sliders, Gaps Checklist (view discovered gaps per level), Cheats submenu (toggle unlocked cheats)

---

## 11. Engine & Presentation Systems

### 11.1 Save System

- **PS1**: Memory Card save. Each save file supports career slots for all skaters + 4 custom skaters.
- **Data persisted**: Career progress per skater, stat upgrades purchased, tricks purchased, decks owned, unlocked levels/characters/cheats, gap checklist progress, created skaters, created parks.
- **Save is manual** — triggered from pause menu or at certain progression points.

### 11.2 Camera

Fixed third-person chase camera positioned behind and slightly above the skater. No user-adjustable camera controls. The camera automatically follows the skater's rotation and dynamically adjusts during vert ramp transitions (pulls back when airborne). In 2-player, the screen splits horizontally.

### 11.3 Audio System

**Soundtrack**: 15 licensed tracks that play sequentially, cycling through the playlist during gameplay.

| # | Artist | Song |
|---|--------|------|
| 1 | Rage Against the Machine | Guerrilla Radio |
| 2 | Papa Roach | Blood Brothers |
| 3 | Anthrax & Public Enemy | Bring the Noise |
| 4 | Bad Religion | You |
| 5 | Powerman 5000 | When Worlds Collide |
| 6 | Millencolin | No Cigar |
| 7 | Naughty by Nature | Pin the Tail on the Donkey |
| 8 | Lagwagon | May 16 |
| 9 | Fu Manchu | Evil Eye |
| 10 | Dub Pistols | Cyclone |
| 11 | Consumed | Heavy Metal Winner |
| 12 | Swingin' Utters | Five Lessons Learned |
| 13 | Styles of Beyond | Subculture (Upbeats Remix) |
| 14 | A Tribe Called Quest / Hi-Tek ft. Mos Def | B-Boy Document '99 |
| 15 | BlackPlanetMusic ft. Alleylife | Out with the Old |

The pause menu allows skipping to the next track. Volume sliders for music and SFX. No per-song enable/disable in the PS1 original.

**Sound effects**: Ollie pop, grind scraping (varies with surface), wheel rolling (varies with terrain), bail impacts, landing thuds, gap discovery shutter sound, ambient level sounds.

### 11.4 Frame Rate

PS1 runs at **30 FPS**. N64 version has improved frame rate. Dreamcast and Xbox versions target 60 FPS.

---

## 12. Open Questions / Unverified

1. **180 rotation multiplier**: Slateman's FAQ (one of the oldest sources) lists 180 = 1× (no bonus), while the GBA guide and other sources list 180 = 1.5×. The 1.5× value is more widely cited and is used in §1.3, but the 1× value may reflect THPS1 behavior or a documentation error.
2. **L2 button function**: One source says "no function," multiple others assign it to Nollie/Fakie. Weight of evidence favors L2 = Nollie.
3. **Grab trick base values**: Sources show variance (150–500 range). Likely reflects different grabs having different values plus the hold-duration accrual making "base" ambiguous.
4. **Grind directional inputs**: The exact grind depends on approach angle as well as directional input, making some inputs context-dependent rather than absolute. The grind system may have additional internal logic not fully documented.
5. **Officer Dick / Private Carrera / McSqueeb stats**: No numeric stats found for these PS1-exclusive secret characters. Officer Dick may start with "all 5s" but this is unconfirmed.
6. **Exact cash thresholds per level unlock**: The progression system is definitively cash-based, but the specific dollar amount required to unlock each level is not documented in available online guides.
7. **Special trick point values**: Not all specials have confirmed point values. The range is 7,500–20,000, with the most expensive confirmed as Pogo Air and Layback Grind at 20,000 each.
8. **Cheat unlock sequence**: Sources disagree on the exact ordering of cheats after position #4. The list in §5.3 uses the most commonly cited ordering but some sources swap positions or include/exclude character unlocks. The total count varies between 15–16 across sources.
9. **Skate Heaven gap count**: One source lists 71 gaps, another lists 56. The gap checklist from StrategyWiki was used for §9.1 (71), but this may include sub-gaps or variants.

---

## 13. Platform Differences

| Platform | Developer | Release | Key Differences |
|----------|-----------|---------|----------------|
| **PS1** | Neversoft | Sep 2000 | Canonical version. 98/100 Metacritic. |
| **Dreamcast** | Treyarch | Nov 2000 | Improved textures/resolution. Mechanically identical. |
| **N64** | Edge of Reality | Aug 2001 | Soundtrack cut to 6 instrumental loops (no lyrics), no FMV, reduced draw distance, but faster load times. |
| **PC** | LTI Gray Matter | Oct 2000 | Higher resolution. Korean version included Fin.K.L. characters and 7 bonus songs. |
| **Xbox (THPS2x)** | Treyarch | Nov 2001 | Visible grind/manual balance meter, 5 new levels, 4-player and LAN multiplayer, female custom skaters, THPS1 levels included. |
| **GBA** | Vicarious Visions | Nov 2001 | Isometric perspective, full visible balance meters, Kid Mode, synthesized soundtrack. |
| **GBC** | Natsume | Nov 2000 | Side-scrolling/isometric overhead, 13 skaters, 7 stages, password saves. |
| **iOS** | — | Apr 2010 | Virtual controls. Career/Single Session/Free Skate only. Removed from App Store 2014. |

**Mechanics NOT in THPS2** (added in later entries):
- **Revert** — THPS3
- **Spine Transfer** — THPS4
- **Acid Drop / Caveman** — THUG
- **Sticker Slap** — THUG

---

## 14. References

### Wikis
- [Tony Hawk's Pro Skater 2 — Wikipedia](https://en.wikipedia.org/wiki/Tony_Hawk%27s_Pro_Skater_2)
- [Tony Hawk Games Wiki — THPS2](https://tonyhawkgames.fandom.com/wiki/Tony_Hawk%27s_Pro_Skater_2)
- [Tony Hawk Games Wiki — Special Bar](https://tonyhawkgames.fandom.com/wiki/Special_Bar)
- [Tony Hawk Games Wiki — Balance Meter](https://tonyhawkgames.fandom.com/wiki/Balance_Meter)
- [Tony Hawk Games Wiki — Combo](https://tonyhawkgames.fandom.com/wiki/Combo)
- [Tony Hawk Games Wiki — Gap](https://tonyhawkgames.fandom.com/wiki/Gap)

### Strategy Guides
- [StrategyWiki — THPS2 Gameplay](https://strategywiki.org/wiki/Tony_Hawk%27s_Pro_Skater_2/Gameplay)
- [StrategyWiki — THPS2 Controls](https://strategywiki.org/wiki/Tony_Hawk%27s_Pro_Skater_2/Controls)
- [StrategyWiki — THPS2 Moves](https://strategywiki.org/wiki/Tony_Hawk%27s_Pro_Skater_2/Moves)
- [StrategyWiki — THPS2 Characters](https://strategywiki.org/wiki/Tony_Hawk%27s_Pro_Skater_2/Characters)
- [StrategyWiki — THPS2 Gaps](https://strategywiki.org/wiki/Tony_Hawk%27s_Pro_Skater_2/Gaps)

### GameFAQs
- [Move List by Bullet_Reload](https://gamefaqs.gamespot.com/ps/199061-tony-hawks-pro-skater-2/faqs/8994)
- [Move List and Guide by slateman](https://gamefaqs.gamespot.com/ps/199061-tony-hawks-pro-skater-2/faqs/7688)
- [High-Score FAQ by mike_tru](https://gamefaqs.gamespot.com/ps/199061-tony-hawks-pro-skater-2/faqs/11601)
- [Gap List by Shotgunnova](https://gamefaqs.gamespot.com/ps/199061-tony-hawks-pro-skater-2/faqs/41674)
- [Character FAQ by MattRyanPerez](https://gamefaqs.gamespot.com/ps/199061-tony-hawks-pro-skater-2/faqs/25330)
- [GBA Trick FAQ by Flit](https://gamefaqs.gamespot.com/gba/471231-tony-hawks-pro-skater-2/faqs/13125)

### Community Resources
- [Neoseeker — Character Specials](https://www.neoseeker.com/pro-skater-2/faqs/36959-tony-hawks-char-specials.html)
- [Activision Support — Scoring and Combos](https://support.activision.com/tony-hawks-pro-skater-1-2/articles/tony-hawks-pro-skater-1-2-scoring-and-combos)
- [Speedrun.com — THPS Mechanics Guide](https://www.speedrun.com/thps1/guides/1wg8v)
- [Slateman's THPS DC FAQ](https://slateman.net/faqs/pro_skater_dc.txt)
- [CheatCodes.com — GBA Strategy Guide](http://www.cheatcodes.com/guide/strategy-guide-tony-hawks-pro-skater-2-gba-18403/)
- [RPGClassics — THPS2 Shrine](https://shrines.rpgclassics.com/psx/thps2/)
