# Tony Hawk's Pro Skater 2 — Phased Implementation Plan

17 phases. Phases 1–6 deliver a vertical slice: one skater, one debug park, all five trick categories, full scoring, and special tricks. Phases 7–15 build the remaining systems (stats, career, competition, shop, save, create-a-skater, multiplayer, create-a-park). Phases 16–17 expand content and wire up secrets/cheats.

All stat-dependent behaviors (jump height, speed, rotation speed, balance stability) use hardcoded default values in Phases 1–6. Phase 7 introduces the data-driven stat system that makes these values per-skater.

---

## Phase 1 — Core Skating Movement

The foundation: a skater that moves, ollies, and lands in a 3D skatepark with a responsive chase camera.

- Ground movement: D-pad steering, forward momentum, braking (D-pad Down)
- Auto-kick system (default: on, toggle in options): On = auto-push with gradual speed decay on flat ground, crouching slows decay; Off = no auto-push but speed does not decay on flat ground
- Ollie: hold X to crouch (max ~0.5 seconds), release to jump; height scales with hold duration
- Air physics: gravity arc, hangtime behavior
- Landing detection: surface alignment check
- Big Drop recovery: press X just before landing from excessive height to prevent bail
- Advanced launches: Nollie (Up + X, ~200 pts), No Comply (Up → X quick, ~200 pts), Boneless/Fastplant (Up, Up, X within ~16 frames, ~250 pts)
- Switch stance toggle (R2): riding switch tracked for trick scoring in later phases
- Basic collision with walls, ramps, and ground geometry
- Chase camera: fixed third-person, behind and slightly above skater, auto-follows rotation, pulls back when airborne off vert ramps
- 2-player camera support deferred to Phase 14
- Debug level: flat skatepark area with ramps, two quarterpipes, and a vert ramp

### Assets

**3D Models**
- Placeholder skater model (rigged, basic skating animations)
- Skateboard model
- Debug skatepark environment (flat ground, ramps, quarterpipes, vert ramp)

**Animations**
- Push/ride cycle
- Crouch (ollie windup)
- Ollie launch
- Air idle (neutral airborne pose)
- Land (clean)
- Bail (ragdoll/tumble)
- Nollie, No Comply, Boneless launch variants
- Switch stance ride cycle

**Audio**
- Wheel rolling SFX (flat ground)
- Ollie pop SFX
- Landing thud SFX
- Bail impact SFX
- Wind/air SFX (airborne)

**UI**
- Auto-kick toggle in options placeholder

---

## Phase 2 — Air Tricks & Spin

The two airborne trick categories and the spin system that modifies their value — the first layer of the trick system.

- Flip tricks: direction + Square while airborne; 12 assignable slots per skater; instant score, no hold; base values ~100 (Pop Shove-It, Kickflip) to ~500 (Kickflip to Indy, Heelflip Varial Lien)
- Grab tricks: direction + Circle while airborne; 12 assignable slots per skater; hold accumulates ~50 pts per half-second; base values ~150 (Nosegrab, Tailgrab) to ~250 (Airwalk, Judo)
- Spin: L1 = rotate left, R1 = rotate right (Spin stat affects speed — hardcoded default until Phase 7)
- Rotation multiplier applied per trick: 180 = 1.5×, 360 = 2.0×, 540 = 3.0×, 720 = 4.0×, 900 = 5.0×, 1080 = 6.0×
- Trick name display at bottom-center on execution
- Base point value displayed on landing (single trick, no combo chaining yet — chaining in Phase 4)
- Skater starts with a representative loadout of 4 flip tricks and 4 grab tricks for testing; full 12-slot customization deferred to Skate Shop (Phase 11)
- Debug level: Phase 1 vert ramp provides sufficient air for all trick types; no changes needed

### Assets

**Animations**
- 4 flip trick animations (Kickflip, Heelflip, Pop Shove-It, 360 Flip)
- 4 grab trick animations (Nosegrab, Tailgrab, Melon, Indy)
- Spin rotation blend (layered on trick animations)

**Audio**
- Flip trick whoosh SFX
- Grab trick catch SFX

**VFX**
- Spin trail (subtle arc indicator during rotation)

**UI**
- Trick name text (bottom-center, appears on execution)
- Point value text (bottom-center, appears on landing)

---

## Phase 3 — Grind & Lip Tricks

Rail/ledge grinding and vert lip tricks — the two Triangle-button trick categories, both with balance mechanics.

- Grind system: press Triangle near any grindable edge; approach angle + directional input determines grind type
- Grind point accumulation: ~137–180 pts/sec depending on type; Kiss the Rail (very short grind) = ~25 pts
- Grind balance: invisible meter (PS1 style); skater's visual tilt is the only indicator; Rail Balance stat affects stability (hardcoded default until Phase 7)
- Grind tricks are NOT customizable — fixed by approach angle and directional input
- Lip tricks: Triangle + direction at the top of a halfpipe/quarterpipe; 4 assignable slots per skater; base values ~250 (Axle Stall) to ~400+ (Disaster, One Foot Invert); points accumulate while held
- Lip balance: invisible meter; Lip Balance stat affects stability (hardcoded default until Phase 7)
- Wallride: Triangle while airborne near a wall; can ollie off wall for continued combo (in Phase 4+)
- Skater starts with 2 lip tricks (Axle Stall, Invert) for testing; full 4-slot customization deferred to Phase 11
- Debug level: add 3 rails (straight, curved, elevated), 2 ledges/benches, 1 wall section for wallrides; ensure halfpipe from Phase 1 has proper lip-trick-ready edges

### Assets

**3D Models**
- Debug rails (3 variants: straight, curved, elevated)
- Debug ledges/benches (2 variants)
- Debug wall section (wallrides)

**Animations**
- 4 grind animations (50-50, Boardslide, Nosegrind, Smith Grind)
- 2 lip trick animations (Axle Stall, Invert)
- Wallride animation
- Grind mount/dismount transitions
- Lip mount/dismount transitions

**Audio**
- Grind scraping SFX (metal rail)
- Grind scraping SFX (concrete ledge)
- Lip trick stall SFX
- Wallride scrape SFX

**VFX**
- Grind sparks (metal surface)
- Grind dust (concrete surface)

---

## Phase 4 — Manual & Combo Chaining

The defining THPS2 innovation: the Manual connects air tricks and grinds into extended combos, turning isolated trick-score-land cycles into a continuous chaining system.

- Manual: Up → Down on ground; ~50 base pts
- Nose Manual: Down → Up on ground; ~50 base pts; counts as a separate trick for multiplier
- Manual balance meter: visible vertical bar near skater; player counterbalances with left/right D-pad; meter becomes more erratic the longer the manual held; Manual Balance stat affects starting stability (hardcoded default until Phase 7)
- Combo chaining: Air Trick → Manual → Grind → Manual → Air Trick → ... ; combo stays alive as long as the chain is unbroken
- Combo multiplier: count of distinct tricks in the chain; full formula in Phase 5
- Trick string: stacked list of trick names in current combo (bottom-center); grows as combo extends
- Base score × multiplier display (bottom-center, below trick string): e.g., "4,500 × 5"
- Switching between Manual and Nose Manual mid-balance counts as a separate trick for multiplier
- Switch stance: tricks performed in switch tracked separately from regular-stance; +20% base value bonus for switch tricks
- Speed decreases during manuals
- Balance difficulty increases when the same balance type (manual, grind, lip) is used multiple times consecutively in a combo
- Debug level: add open flat runs connecting ramps, rails, and quarterpipes to enable long manual-linked combo routes

### Assets

**Animations**
- Manual pose (tail-down balance)
- Nose Manual pose (nose-down balance)
- Manual balance wobble (left/right lean blend)
- Switch stance riding variant

**Audio**
- Manual wheel scrub SFX (tail dragging)
- Nose Manual wheel scrub SFX (nose dragging)
- Combo land SFX (satisfying thud when long combo lands cleanly)

**UI**
- Manual balance meter (vertical bar, near skater)
- Combo trick string (bottom-center, stacked trick names)
- Base score × multiplier display (bottom-center, below trick string)

---

## Phase 5 — Scoring Engine

The full scoring formula with landing quality, trick degradation, and bail conditions — turning raw trick execution into a tuned risk/reward system.

- Full combo formula: `Combo Score = (Sum of spin-modified trick values) × Combo Multiplier`
- Individual trick value: `base_value × rotation_multiplier × degradation × landing_modifier`
- Combo multiplier: count of tricks + count of gaps in combo (gap contribution deferred to Phase 8; multiplier counts tricks only for now); caps at ×30
- Landing quality: Perfect (green) = +50% bonus on trick value; Sloppy (red) = −30% penalty on trick value
- Trick degradation per session: 1st = 100%, 2nd = 75%, 3rd = 50%, 4th = 25%, 5th+ = 10%; switch-stance tricks tracked separately from regular-stance
- Bail conditions: overspin on landing (not aligned with travel direction), balance meter reaches extreme during manual or grind, landing from excessive height without Big Drop recovery, colliding with walls/objects at high speed
- Bail penalty: forfeits ALL accumulated combo points; empties Special Meter (Special Meter built in Phase 6 — bail-empties-meter wired then)
- Score counter: running session total, top-left
- Debug level: no changes needed — existing environment supports all scoring scenarios

### Assets

**Audio**
- Perfect landing SFX (distinct from normal land)
- Sloppy landing SFX
- Bail crash SFX (heavier variant)
- Score tick-up SFX (points adding to session total)

**VFX**
- Perfect landing flash (green)
- Sloppy landing flash (red)
- Combo land point burst (particle pop on large combo score)

**UI**
- Score counter (top-left, running session total)
- Landing quality indicator (green "Perfect" / red "Sloppy" flash)
- Trick degradation indicator (dimmed trick name on repeated tricks)

---

## Phase 6 — Special Meter & Special Tricks

The reward layer for consistently landing combos: a Special Meter that gates high-value signature tricks and a speed boost.

- Special Meter: horizontal bar displayed below score counter (top-left)
- Fills by landing tricks; higher-scoring combos fill faster; ~3,000 points in a single landed combo fills the meter completely
- Visual: glows gold when full
- Enables: Special Trick inputs become available; maximum speed increases
- Drain: constantly decreases when not performing tricks
- Bail: completely empties the meter
- No "on fire" state — binary full/not-full only (THPS2 has no unlimited-special mechanic)
- Special trick inputs: two directional taps + a trick button (e.g., Right, Down + Circle for The 900)
- Each skater has 3 default specials; use Tony Hawk's specials for testing: The 900 (R,D+Circle, 15,000), Sacktap (U,D+Circle, 10,000), BS Overturn (D,L+Tri, 8,500)
- Special trick values: 7,500–20,000 pts
- Per-skater specials for remaining roster deferred to Phase 7
- Debug level: no changes needed — existing environment provides enough scoring opportunity to fill the meter

### Assets

**Animations**
- The 900 (Tony Hawk air grab special)
- Sacktap (Tony Hawk air grab special)
- BS Overturn (Tony Hawk lip special)

**Audio**
- Special meter full SFX (activation chime)
- Special trick execution SFX (dramatic whoosh/impact, distinct from normal tricks)
- Special meter drain ambient (subtle hum while full)

**VFX**
- Special Meter gold glow effect
- Special trick execution flash (screen flash or particle burst)

**UI**
- Special Meter bar (top-left, below score counter)
- Special Meter gold glow state
- "SPECIAL" text indicator when meter is full

---

**Vertical slice checkpoint — A single skater (Tony Hawk) can skate a debug park, perform all five trick categories (flip, grab, grind, lip, manual), chain combos via manuals, earn points through the full scoring formula with rotation multipliers, degradation, and landing quality, fill the Special Meter and execute The 900, Sacktap, and BS Overturn. No session timer, no career goals, no real levels — the skeleton plays like THPS2 in a free-play sandbox. Stats are hardcoded (Phase 7), gap bonuses are not wired (Phase 8), career progression does not exist (Phase 9), and progress does not persist (Phase 12).**

---

## Phase 7 — Stat System & Skater Roster

Data-driven stats that differentiate how each skater plays, and the full 13-skater pro roster with unique distributions, specials, and trick loadouts.

- 10 stats on a 1–10 scale, replacing hardcoded defaults from Phases 1–6:
  - Air (height off vert ramps/quarterpipes)
  - Hangtime (air duration)
  - Ollie (jump height from flat ground)
  - Speed (maximum ground speed)
  - Spin (rotation speed with L1/R1)
  - Landing (forgiveness on perfect/sloppy threshold)
  - Switch (stability and performance in switch stance)
  - Rail Balance (grind balance stability)
  - Lip Balance (lip trick balance stability)
  - Manual Balance (manual balance stability)
- All 13 default pro skaters: 50 total stat points each, starting distributions per SPEC §4.1
- Character archetypes from stat distributions:
  - Vert specialists (high Air/Hangtime/Lip): Tony Hawk, Rune Glifberg, Bucky Lasek
  - Street specialists (high Ollie/Rail/Manual): Rodney Mullen, Eric Koston, Chad Muska, Andrew Reynolds
  - All-around: Bob Burnquist, Steve Caballero, Elissa Steamer
- Per-skater signature specials: all 13 skaters get their 3 specials from SPEC §4.2
- Per-skater default trick loadouts: 12 flip slots, 12 grab slots, 4 lip slots filled with character-appropriate tricks
- Stance per skater: Regular or Goofy (per SPEC §4.1)
- Character select screen: choose from 13 skaters before a session
- Stat upgrades (purchase) deferred to Skate Shop (Phase 11); stats are read-only for now
- Debug level: no changes — test different skaters in existing environment to verify stat differentiation

### Assets

**3D Models**
- 13 pro skater models (Tony Hawk, Bob Burnquist, Steve Caballero, Kareem Campbell, Rune Glifberg, Eric Koston, Bucky Lasek, Rodney Mullen, Chad Muska, Andrew Reynolds, Geoff Rowley, Elissa Steamer, Jamie Thomas) — all share the base animation rig and movement animation set from Phase 1; unique geometry and textures per skater
- Per-skater default skateboard deck (1 each)

**Animations**
- Remaining special trick animations for all 13 skaters (~28 unique new specials — some are shared between skaters, e.g. Mute Backflip used by Muska and Rowley; 3 Tony Hawk specials already built in Phase 6)
- Additional flip trick animations (~8 more unique flips to fill 12-slot loadouts; Phase 2 introduced 4)
- Additional grab trick animations (~8 more unique grabs to fill loadouts)
- Additional lip trick animations (Disaster, One Foot Invert — 2 more; Phase 3 introduced 2)
- Additional grind animations (Feeble, Crooked, Bluntslide, Darkslide — 4 more; Phase 3 introduced 4)

**Audio**
- Character select cursor SFX

**UI**
- Character select screen (13 skaters with portrait, name, stance, stat bar/radar preview)
- Stat display (bar chart or radar for 10 stats)

---

## Phase 8 — Gap System

Named location-based trick challenges that reward exploration and add a discovery layer on top of every level's geometry.

- Gap triggers: predefined named challenges tied to specific locations; correct trick type at correct location triggers the gap
- Gap types: Air, Grind, Manual, Lip, Wallride, Ground — each requires the matching trick category
- Gap bonus values: 50 pts (small transfers) to 5,000+ pts (complex multi-part gaps)
- Gap bonuses do NOT degrade with repetition
- Gap bonuses add to combo score before the multiplier AND add +1 to the combo multiplier — extends Phase 5 combo formula (combo multiplier now = count of tricks + count of gaps)
- Gap discovery notification: blue text flash at center screen with gap name
- Gap checklist: view discovered gaps per level, accessible from Options menu; tracks all ~367 gaps across 10 levels
- Completing all gaps in the game unlocks Private Carrera — unlock wired in Phase 17
- Debug level: place 8–10 representative gaps across existing geometry:
  - 2 Air gaps (transfers between ramps)
  - 2 Grind gaps (rail-to-rail or full-length grinds)
  - 2 Manual gaps (flat stretches between features)
  - 1 Lip gap (on halfpipe)
  - 1 Wallride gap
  - 1 multi-part gap (chained trick types)

### Assets

**Audio**
- Gap discovery shutter/camera SFX
- Gap name appearance SFX

**VFX**
- Gap name text flash (blue, center screen, fades after ~2 seconds)

**UI**
- Gap notification display (center screen, blue text)
- Gap checklist screen (per-level list, discovered vs. undiscovered, total count)

---

## Phase 9 — Career Mode & Goals

The progression backbone: level-by-level career with 10 goals per level, cash rewards, and level unlocking — plus Single Session and Free Skate as standalone session modes.

- Career Mode: progress through levels in fixed order; complete goals to earn cash; cumulative cash unlocks subsequent levels
- Session timer: 2:00 countdown for regular career levels and Single Session mode
- 10 goals per regular level, consistent structure:
  1. High Score threshold
  2. Pro Score threshold
  3. SICK Score threshold
  4. Collect S-K-A-T-E letters
  5. Themed collectible set (5 items per level)
  6. Trick challenge (specific trick at specific location)
  7. Level-specific objective
  8. Gap challenge
  9. Find the Secret Tape
  10. 100% Goals and Cash (meta-goal)
- The Hangar as the first real level: High 10,000 / Pro 25,000 / SICK 75,000; 21 gaps; ~$2,350 total cash; goals: Collect 5 Pilot Wings, Barrel Hunt, Nosegrind over the Pipe, Hit 3 Hangtime Gaps (per SPEC §3.2)
- Cash icon pickups scattered through levels
- Goal completion notification with cash earned
- Single Session mode: 2-minute high-score run on any unlocked level using current skater loadout
- Free Skate mode: no timer, no objectives — practice and explore
- Level select screen (unlocked levels only)
- Pause menu: Resume, Restart Run, End Run, Volume, Change Soundtrack (skip to next song)
- Options menu: Controller Configuration (remap P1/P2), Auto-kick toggle (from Phase 1), Vibration toggle, Sound/Music volume sliders, Gap Checklist (extends Phase 8)
- Soundtrack: 15 licensed tracks play sequentially, cycling through the playlist during gameplay; pause menu skip advances to the next track; volume sliders for music and SFX
- Main menu: Career Mode, Single Session, Free Skate, 2 Player (placeholder — Phase 14), Create Skater (placeholder — Phase 13), Park Editor (placeholder — Phase 15), Options, Movies (placeholder — Phase 17), Cheats (placeholder — Phase 17)
- Competition levels (Marseille, Skatestreet, Bullring) appear as placeholders in the career level sequence — judge scoring deferred to Phase 10
- Secret levels (Chopper Drop, Skate Heaven) are hidden entries — unlock conditions in Phase 17
- Debug level remains available as a Free Skate environment

### Assets

**3D Models**
- The Hangar level (full geometry: hangar interior, halfpipe, rails, exterior area, hidden tape location, barrel objects)
- S-K-A-T-E letter collectible model (5 floating letters)
- Pilot Wings collectible model (5 Hangar-themed items)
- Secret Tape model
- Cash icon model ($)

**Audio**
- Goal completion SFX (success chime)
- S-K-A-T-E letter pickup SFX
- Cash icon pickup SFX
- Collectible item pickup SFX
- Timer warning SFX (last 10 seconds)
- Session end buzzer SFX
- Gameplay music track 1 (placeholder or original)

**VFX**
- Collectible pickup sparkle
- Goal completion banner flash

**UI**
- Session timer (top-right, countdown from 2:00)
- Goal list screen (per-level, 10 goals with completion status and cash reward)
- Goal completion notification (center, goal name + cash earned)
- Level select screen
- Main menu (Career Mode, Single Session, Free Skate, Options + placeholders)
- Pause menu (Resume, Restart Run, End Run, Volume, Change Soundtrack)
- Options screen (controller config, auto-kick, vibration, volume sliders, gap checklist link)
- Session results screen (score summary, goals completed, cash earned)
- Career wallet display (total cash)

---

## Phase 10 — Competition Format

Judge-based scoring for competition levels — a distinct mode from regular career goals, with AI opponents and medal placement.

- 3 rounds, 1 minute each
- 5 AI judges score each run on a 0–99.9 scale
- Highest and lowest judge scores discarded; remaining 3 averaged for the run score
- Best 2 of 3 run scores summed for final score (weakest run dropped; max theoretical ~199.8)
- Player must place 3rd or better to earn a medal and advance
- Judge evaluation criteria: trick variety, trick difficulty, execution quality; bails heavily penalize scores
- Scoring calibration: ~60,000 trick points with no bails → scores in the 90s; ~200,000+ → max 99.9
- AI opponent scores (simulated; AI skaters do not physically skate — results are generated)
- Gold Medal counts as a career goal
- Competition cash rewards: Marseille $7,000, Skatestreet $20,000, Bullring $65,000 (Skatestreet and Bullring levels built in Phase 16)
- No fixed score threshold for medals — player must outscore the AI opponents' generated scores
- Marseille as the first playable competition level; 33 gaps placed in this phase; extends Phase 9 career progression (Marseille is career level 3)

### Assets

**3D Models**
- Marseille competition level (bowl, rails, quarterpipes, spectator area)

**Audio**
- Competition round start SFX
- Competition round end SFX
- Medal award fanfare (Gold / Silver / Bronze variants)
- Judge score reveal SFX
- Crowd ambient SFX (competition atmosphere)
- Gameplay music track 2

**UI**
- Competition HUD (round indicator replaces standard timer)
- Judge score display (5 scores with highest/lowest crossed out)
- Run score summary (per-round breakdown)
- Final standings (player rank vs. AI opponents)
- Medal award screen (Gold / Silver / Bronze)

---

## Phase 11 — Skate Shop & Economy

The purchase system for stat upgrades, tricks, specials, and cosmetic decks — the primary cash sink driving career replayability.

- Skate Shop: accessible from career menu and main menu
- Stat point upgrades: ~$500 per point; extends Phase 7 stat system (stats now writable via purchase)
- Standard trick purchases: ~$500 each; purchased tricks expand the pool available for slot assignment
- Special trick purchases: $3,750–$15,000; ~40+ unique specials in the pool (signatures from all characters plus generic specials); any skater can equip any 3 purchased specials; The 900 is the most expensive at $15,000
- Deck purchases: cosmetic only, no stat effect; 148 total decks across all skaters
- Trick customization screen: assign purchased tricks to slots — extends Phase 7 loadouts:
  - Flip tricks: 12 slots (any direction + Square)
  - Grab tricks: 12 slots (any direction + Circle)
  - Lip tricks: 4 slots (any direction + Triangle at lip)
  - Special tricks: 3 slots (any purchased special assignable to any skater)
  - Grinds: NOT customizable (reminder — approach-angle-based, per Phase 3)
- Career wallet: total cash earned, cash spent, cash available
- Maximum career cash: ~$150,000 (full career clear across all levels)

### Assets

**Audio**
- Shop purchase SFX (ka-ching)
- Shop browse SFX (cursor movement)
- Insufficient funds SFX (denied buzz)

**UI**
- Skate Shop main screen (category tabs: Stats, Tricks, Specials, Decks)
- Stat upgrade screen (10 stats with current value, +/− buttons, cost per point)
- Trick purchase screen (categorized list with price, owned status)
- Special trick purchase screen (full pool with skater origin, input command, point value, price)
- Deck gallery (thumbnail grid with price, owned indicator)
- Trick assignment screen (slot grid with select-assign for each trick category)

---

## Phase 12 — Save System

Persistence: save and load career progress, purchases, unlocks, and custom content across sessions.

- Save data model: career progress per skater (goals completed, cash earned, levels unlocked), stat upgrades purchased, tricks purchased, decks owned, unlocked characters, unlocked cheats, gap checklist progress; data slots for created skaters and created parks are defined here but wired when Phases 13 and 15 ship
- Manual save: triggered from pause menu
- Autosave at career milestones (level unlock, goal completion)
- Save slot management: single save file supporting career data for all skaters + 4 custom skater slots
- Title screen load flow: extends Phase 9 main menu with Continue option (loads existing save) alongside New Game
- Save confirmation and error feedback

### Assets

**Audio**
- Save confirmation SFX
- Load confirmation SFX

**UI**
- Save prompt (pause menu integration)
- Save/Load confirmation dialog
- Title screen with Continue option (extends Phase 9 main menu)
- Save file info display (completion percentage, cash, last played)

---

## Phase 13 — Create-A-Skater

Custom character creation: biography, style-based trick loadout, stat allocation, and cosmetic appearance.

- 4 custom skater slots
- Biography: Name, hometown, age, stance (Regular/Goofy), weight (88–378 lbs)
- Style selection: All-Around, Vert, or Street — determines starting trick loadout
- Starting stats: 5 free stat points to allocate across 10 stats; additional points purchasable via Skate Shop (extends Phase 11)
- Appearance: complexion/skin tone, hairstyle, torso, pants, shoes, socks
- Decks: start with 10 (vs. 1 for pro skaters); 10 more purchasable from Skate Shop
- Male-only (PS1 limitation)
- Custom skaters play full Career Mode and count toward cheat progression (Phase 17)
- Created skaters persisted via save system (extends Phase 12)

### Assets

**3D Models**
- Modular custom skater body (base mesh with swap points for clothing/hair)
- Hairstyle variants (4–6)
- Torso variants (4–6 shirts/jackets)
- Pants variants (4–6)
- Shoe variants (3–4)
- Custom skater deck set (10 default + 10 purchasable)

**Audio**
- Customization cursor SFX
- Style selection confirmation SFX

**UI**
- Create-A-Skater slot selection screen (4 slots)
- Biography input screen (name, hometown, age, stance, weight)
- Style selection screen (All-Around / Vert / Street with stat/trick preview)
- Stat allocation screen (5 points across 10 stats, with +/− controls)
- Appearance customization screen (category tabs with real-time 3D preview)
- Deck selection screen

---

## Phase 14 — Multiplayer Modes

Local 2-player split-screen with five competitive and cooperative modes.

- Horizontal split-screen (top/bottom); each half renders its own complete HUD
- Per-player chase camera (extends Phase 1 camera for split-screen)
- Player collision: slower player bails and loses their combo on collision
- **Trick Attack**: highest score wins within a set time (1, 2, 5, or 10 minutes)
- **Graffiti**: tag skateable objects by tricking on them — objects change to the player's color on combo land; opponent steals with a higher combo on the same object; most tagged objects at time expiry wins; bailing colors nothing
- **HORSE**: custom word (3–10 letters, default "HORSE"); alternating 10-second turns; lower scorer or bailer earns a letter; both players move to a new location after each exchange; first to spell the full word loses
- **Tag**: 10-second initial scoring; lower scorer becomes "it"; tagged player must touch opponent before timer expires; tricks while "it" increase speed and tag range
- **Free Skate (2-Player)**: no timer, no objectives
- 2-Player mode: extends Phase 9 main menu (replaces placeholder)
- Level selection: any unlocked level plus debug level and created parks (Phase 15)
- All trick and scoring systems from Phases 1–8 fully functional per player

### Assets

**3D Models**
- Tagged-object color overlay material (Graffiti mode — player 1 / player 2 colors)

**Audio**
- Graffiti tag SFX (object claimed)
- Graffiti steal SFX (object reclaimed by opponent)
- HORSE letter earned SFX
- Tag contact SFX
- Per-player positional audio mix (split-screen)

**VFX**
- Graffiti object glow (player 1 blue / player 2 red)
- Tag "it" indicator (glow on tagged player)
- HORSE letter display flash

**UI**
- Split-screen HUD (two complete HUD sets, top/bottom)
- 2-Player mode select screen (Trick Attack, Graffiti, HORSE, Tag, Free Skate)
- Trick Attack time select (1 / 2 / 5 / 10 minutes)
- HORSE custom word input
- Graffiti tag counter (per player)
- HORSE letter progress display
- Tag timer display
- Multiplayer results screen (winner/loser, scores/counts)

---

## Phase 15 — Create-A-Park

The level editor: build custom skatepark layouts on a grid with themed pieces, test mid-construction, and save parks.

- Grid sizes: 16×16, 24×24, 30×30, 30×18, 60×6
- Themes: Power Plant, Industrial, Outdoor, School (cosmetic — affect ground/wall textures and skybox)
- 100+ pieces across 18 categories: Starts, Gaps, Risers, Quarter Pipes (metal/wood), Rails, Offset Rails, Low Walls, Slopes, Stairs, Pools, Funboxes, Kickers, Benches, Signs, Floor textures, Foliage, Misc (dividers/pillars)
- Custom gaps: set two markers, name the gap, assign 50–5,000 point value, specify gap type (air/rail/manual/wallride) — extends Phase 8 gap system
- Piece rotation and resize
- Test Play mode: skate the park mid-construction with full trick system active
- Save/load: up to 50 saved parks — extends Phase 12 save data
- Created parks playable in Single Session, Free Skate, and Multiplayer modes

### Assets

**3D Models**
- 100+ park editor pieces across 18 categories (each with grindable/skateable surface markup):
  - Starts (2 spawn point variants)
  - Risers (4 height variants)
  - Quarter Pipes (4 variants: metal small/large, wood small/large)
  - Rails (6 variants: straight, curved, kinked, low, high, handrail)
  - Offset Rails (3 variants)
  - Low Walls (4 variants)
  - Slopes (4 variants: gentle, steep, short, long)
  - Stairs (3 sets: small, medium, large)
  - Pools (3 variants: round, kidney, square)
  - Funboxes (4 variants)
  - Kickers (3 variants)
  - Benches (2 variants)
  - Signs (3 variants)
  - Foliage (4 variants: tree, bush, planter, hedge)
  - Misc (4 variants: divider, pillar, barrier, cone)
- Theme texture sets (4 themes × ground/wall/skybox)

**Audio**
- Piece placement SFX
- Piece rotation SFX
- Piece deletion SFX
- Editor cursor movement SFX

**UI**
- Park editor main screen (top-down grid view, piece palette, category tabs)
- Piece category browser (18 categories with thumbnails)
- Grid size selection
- Theme selection
- Custom gap setup dialog (marker placement, name input, point value, gap type)
- Park save/load screen (50 slots with preview thumbnails)
- Test Play toggle

---

## Phase 16 — Content Expansion

All remaining career levels, competition levels, and secret levels — populating the existing career, competition, and gap systems with the full game's content.

**Remaining Career Levels** (extends Phase 9 — 10 goals each, gaps per SPEC §9.1):
- School II: High 15,000 / Pro 40,000 / SICK 100,000; 43 gaps; ~$4,350 total cash; goals include Wallride 5 Bells, Collect 5 Hall Passes, Kickflip TC's Roof Gap, Grind 3 Roll Call Rails
- NY City: High 20,000 / Pro 50,000 / SICK 150,000; 41 gaps; ~$9,950 total cash; goals include Ollie the Hydrants, Collect 5 Subway Tokens, 50-50 Joey's Sculpture, Grind the Subway Rails
- Venice Beach: High 40,000 / Pro 100,000 / SICK 200,000; 41 gaps; ~$12,750 total cash; goals include Ollie the Magic Bum 5 times, Collect 5 Spray Cans, Tailslide Venice Ledge, Hit 4 VB Transfers
- Philadelphia: High 50,000 / Pro 125,000 / SICK 250,000; 41 gaps; ~$22,000 total cash; goals include Drain the Fountain, Collect 5 Bells, Bluntside the Awning, Liptrick 4 Skatepark Lips

**Remaining Competition Levels** (extends Phase 10):
- Skatestreet: 36 gaps; $20,000 competition cash
- Bullring: 31 gaps; $65,000 competition cash

**Secret Levels** (no career goals; untimed free-skate format; gaps to discover and cash to collect):
- Chopper Drop: lone halfpipe shaped like a ship hull in the Pacific Ocean; helicopter skids for grinding/lip tricks above the halfpipe; distance-based gaps (70 ft, 80 ft, 90 ft launches); 9 gaps; unlock conditions wired in Phase 17
- Skate Heaven: outer-space theme with loop-de-loops, translucent halfpipes, volcano activated by grinding a specific rail, surreal skate structures; 71 gaps; unlock conditions wired in Phase 17

**Full Content Completion**:
- All ~367 gaps placed across all levels (minus those already placed in The Hangar/Marseille from Phases 9–10 and debug level from Phase 8)
- Full trick pool available in Skate Shop: all ~40+ specials, all flip/grab/lip trick variants (extends Phase 11)
- 148 skateboard decks across all skaters (extends Phase 11)
- Cash icon placements in all levels
- Per-level goal-specific objects (bells, hydrants, tokens, spray cans, Magic Bum NPC, fountain, etc. — per SPEC §3.2)
- 13 remaining gameplay music tracks (15 total; tracks 1–2 introduced in Phases 9–10)

### Assets

**3D Models**
- School II level
- NY City level
- Venice Beach level
- Philadelphia level
- Skatestreet level
- Bullring level
- Chopper Drop level (halfpipe hull, helicopter, ocean environment)
- Skate Heaven level (space environment, loop-de-loops, translucent halfpipes, volcano)
- Per-level goal collectibles: School II bells (5) + hall passes (5), NY City hydrants + subway tokens (5), Venice Beach spray cans (5) + Magic Bum NPC, Philadelphia bells (5) + drainable fountain
- Remaining skateboard deck models (148 total minus those from Phases 7/13)

**Audio**
- 13 gameplay music tracks (total 15 across all phases)
- Per-level ambient sounds: School II school bells, NY City traffic/subway, Venice Beach ocean/boardwalk crowd, Philadelphia urban, Skatestreet indoor echoes, Bullring crowd/bull, Chopper Drop helicopter rotor/ocean waves, Skate Heaven space ambience
- Level-specific SFX (fountain drain, bull charge, helicopter pass, volcano eruption)

**Animations**
- Remaining purchasable trick animations beyond Phase 7 starting loadouts (additional flips, grabs, lips for Skate Shop pool)

---

## Phase 17 — Secret Characters & Cheats

The endgame reward layer: secret characters with unique specials, the progressive cheat system, cheat codes, end-game videos, and completion tracking.

**Secret Characters** (extends Phase 7 roster):
- Officer Dick: unlocked by 1st career 100% completion; specials: Assume the Position (grab), Lazy Azz Grind (grind), Salute (grab)
- McSqueeb (80's Tony): unlocked by 4th career 100% completion, or completing 100% with Tony Hawk specifically
- Spider-Man: unlocked by 12th career 100% completion, or completing 100% with a created skater; stats: Air 7, Hang 7, Ollie 5, Speed 5, Spin 5, Land 4, Switch 4, Rail 6, Lip 3, Manual 4 (total 50); specials: Spidey Flip (U,D+Circle), Spidey Grind (L,R+Tri), Spidey Varial (L,R+Sq)
- Private Carrera: unlocked by completing every gap in the gap checklist (excluding secret levels; Free Skate completions count); specials: Ho-Ho Handplant (lip), Double Splits (grab), Ho Slide (grind)

**Progressive Cheat System** (extends Phase 9 career tracking):
- Each career 100% completion with a different eligible skater unlocks the next cheat in sequence
- Tony Hawk, Officer Dick, and Spider-Man completions do NOT advance the sequence — only the other 12 default pro skaters and created skaters count
- 15 cheats in sequence:
  1. Officer Dick (secret character)
  2. Skip to Restart
  3. Kid Mode (child-sized skaters, enhanced stats)
  4. McSqueeb (secret character)
  5. Perfect Balance (infinite manual/grind/lip balance)
  6. Always Special (meter stays full permanently)
  7. Stud Mode (all stats maxed to 13, beyond the normal 10 cap)
  8. Weight Mode (altered body proportions, stackable)
  9. Wireframe Mode (wireframe rendering)
  10. Slow-NIC (slow-motion tricks during jumps and grinds)
  11. Big Head Mode (enlarged heads)
  12. Spider-Man (secret character)
  13. Moon Physics (reduced gravity, higher/longer jumps)
  14. Sim Mode (realistic physics — less air, faster ground speed)
  15. Smooth Mode (no textures, flat-shaded surfaces)
- Cheats submenu: toggle unlocked cheats from main menu or pause menu
- Pause menu cheat codes (hold L1, enter button sequence): Level Flip (mirror mode), Blood Mode (nose squirt on bail, cosmetic), Flight Mode (free fly), All Levels (unlock all), Extra Money (add cash)

**End-Game Videos**:
- Bail montage video: triggers on first career completion (beating Bullring, level 8)
- Per-skater ending video: triggers on earning all Gold Medals in all 3 competitions
- Movies menu: unlocked FMVs viewable from main menu (extends Phase 9 menu)

**Completion Tracking**:
- Per-skater 100% tracking (all goals + all cash + gold medals)
- Overall completion tracking (across all skaters — drives Skate Heaven unlock)
- Secret level unlock conditions:
  - Chopper Drop: Gold Medals in all 3 competitions with every pro skater
  - Skate Heaven: 100% completion with every character (all pros, Officer Dick, Spider-Man, and a custom skater)

### Assets

**3D Models**
- Officer Dick skater model
- McSqueeb (80's Tony) skater model
- Spider-Man skater model
- Private Carrera skater model

**Animations**
- Officer Dick specials (3): Assume the Position, Lazy Azz Grind, Salute
- McSqueeb (reuses Tony Hawk base animations with visual differences)
- Spider-Man specials (3): Spidey Flip, Spidey Grind, Spidey Varial — plus unique movement flair
- Private Carrera specials (3): Ho-Ho Handplant, Double Splits, Ho Slide

**Audio**
- Cheat activation SFX
- Cheat code input SFX
- Secret character unlock fanfare
- Level unlock fanfare
- Bail montage audio
- Per-skater ending video audio

**VFX**
- Moon Physics visual (extended air trail)
- Wireframe Mode rendering pass
- Smooth Mode rendering pass (flat-shaded, no textures)
- Big Head Mode (head scale override)
- Kid Mode (body scale override)
- Weight Mode (body proportion override)
- Blood Mode (nose squirt particle on bail)
- Flight Mode (flight trail)

**UI**
- Cheats submenu (toggle list with unlock status, on/off per cheat)
- Cheat code input overlay (shows button sequence feedback)
- Secret character unlock notification
- Level unlock notification (Chopper Drop, Skate Heaven)
- Movies menu (video gallery, extends Phase 9 main menu)
- Video player screen
- Per-skater completion percentage display
- Overall completion tracker
