# GTA:SA — Unreal Engine Implementation Plan

This document details every feature system for the GTA: San Andreas Unreal Engine recreation. For execution phases, test checklists, and architectural decisions, see the companion document: **GTA:SA — Execution Phases & Architecture**.

The existing codebase has three prototype variants (Combat, Platforming, Side-Scrolling) that served as C++/UE learning exercises. This plan retires those and establishes a single unified GTA:SA gameplay layer.

---

## Current State

| System | Status | Notes |
| --- | --- | --- |
| Player Movement | Prototype only | 3 variant characters; none match GTA:SA movement model |
| Melee Combat | Partial | Combat variant has combo/charged attacks, interfaces (ICombatAttacker, ICombatDamageable) — reusable pattern |
| AI | Partial | StateTree + EQS in Combat variant — pattern reusable, logic must be rewritten |
| Vehicles | None | — |
| RPG / Stats | None | Only basic HP tracking exists |
| World Systems | None | No day/night, weather, zones, or traffic |
| UI | Minimal | Health bar widget only |

---

# Feature Systems

---

## Player Character & Combat

# 1. Player Movement

<aside>
🏃

**SPEC §1** — Full on-foot movement state machine, stamina, lung capacity, fall damage.

</aside>

### Key Deliverables

- **SAPlayerCharacter** — single `ACharacter` subclass replacing all three variant characters
- **Custom CharacterMovementComponent** — extends `UCharacterMovementComponent` with GTA:SA-specific states
- **Movement State Machine** — Walk, Run, Sprint, Crouch, Jump, Swim (surface + dive), Climb, Combat Roll
- **Third-person camera** — GTA-style orbit cam with smooth interpolation and future lock-on support
- **Enhanced Input** — unified Input Mapping Context for all on-foot actions
- **Stamina system** — gates sprint/fast-swim duration
- **Lung capacity** — gates underwater time
- **Fall damage** — non-linear, height-based

### UE5 Approach

- Custom movement modes on `UCharacterMovementComponent` for swim-surface, swim-dive, climb, and combat-roll
- Movement state tracked via enum mirroring the original `eMoveState`
- Stamina/lung as float properties on the character (not GAS — too lightweight to warrant it)
- Camera on `USpringArmComponent` + `UCameraComponent`; separate camera mode object for future vehicle cameras

### Animations Needed

*To be filled in — expected categories: locomotion blend space (walk/run/sprint), crouch walk, jump/land, swim surface (breaststroke + front crawl), swim dive, climb/mantle, combat roll, fall/freefall*

---

# 2. Hand-to-Hand Combat

<aside>
🥊

**SPEC §2** — Four fighting styles with combos, blocking, and muscle-scaled damage.

</aside>

### Key Deliverables

**Four Fighting Styles:**

| Style | ID | Gym | Combo Chain | Running Attack | Ground Attack | Kill Power |
| --- | --- | --- | --- | --- | --- | --- |
| Default | 4 | — | 2-hit: punch → kick (kick knocks down) | Same as standing punch | Same as standing punch | Low |
| Boxing | 5 | Ganton, LS | Left uppercut → right hook | Rush punch | Downward punch to prone enemy | Medium |
| Kung Fu | 6 | Cobra MA, SF | Rapid punches/kicks → roundhouse/spinning kick | Flying kick (most visually impressive) | Stomp | Medium |
| Muay Thai | 7 | Below the Belt, LV | 4-hit: knees + punches → elbow strike (1-hit kill on normal NPCs) | Short jab rush | Hardcore stomp | **High** |

**Combo System:**

- Combo timing is **time-based, not frame-based** — driven by a `Chain` float (seconds) per attack move defined in `melee.dat`
- Timed presses advance through the combo chain; **button mashing resets to the first move**
- Each move in a chain has its own damage value (byte) — different hits deal different amounts
- No visual combo counter — the system is entirely internal
- The Muay Thai final elbow deals extremely high damage that exceeds normal NPC health pools (1-hit kill on regular peds) but does NOT kill armored/high-health NPCs (story targets, FBI, military, gym trainers — they survive with very low health)

**Blocking:**

- Hold lock-on (R1/RMB) without pressing attack
- **No stamina cost** — blocking is free to maintain
- Reduces incoming melee damage (exact multiplier not publicly documented)
- NPCs drop their guard after sustained pressure — this is an AI behavior transition, not a discrete hit-count threshold

**Running Attacks:**

- Triggered by: lock on to target + moving toward them + attack. **Sprint is NOT required** — jogging while locked on is sufficient
- Each style has a unique running attack animation (the `_M` / moving attack animation)

**Ground Attacks:**

- Target must be in a knockdown/prone animation state
- Same input as normal attack while locked on to a downed enemy — game detects target is down and plays ground-specific animation
- Works on any knocked-down NPC

**Style Acquisition:**

- Requires 35% Muscle minimum
- **Real combat encounter** against a trainer NPC with boosted health (not scripted)
- On-screen prompts teach new moves during the fight
- If CJ loses: hospital respawn (normal death penalty)
- If CJ **forfeits** mid-fight (leaves gym): **loses all carried weapons** (significant penalty)
- LV kickboxing trainer is hardest — max health and muscle recommended
- On completion: unarmed moveset permanently replaced; cannot recover previous style
- All 3 trainer defeats required for 100% completion

**Melee Damage Scaling:**

- Muscle stat (0–1000) acts as damage multiplier on all melee attacks
- Max muscle + kickboxing can destroy a car in ~25 punches

### UE5 Approach

- **Style data:** each style is a `USAFightingStyleAsset` (`UDataAsset`) containing: style ID, combo chain definition (array of `FSAComboMove` structs with damage, chain window, hit level, animation), running attack montage, ground attack montage, block animation, idle animation
- **Combo state machine:** `USAWeaponComponent` tracks current combo index + chain timer. On attack input: if within chain window → advance to next move; if outside window or button mashed too fast → reset to move 0. Timer is delta-time driven (not frame-dependent)
- **Hit detection:** sphere traces fired during AnimNotify windows (existing pattern). Each combo move’s `FSAComboMove` specifies trace radius and hit level
- **Ground attack detection:** before playing attack montage, check if locked-on target’s movement mode is ragdoll/prone. If yes → play ground attack montage instead of combo
- **Running attack detection:** if locked on + moving speed > walk threshold + attack input → play running attack montage instead of combo
- **Muay Thai elbow:** final combo move has damage value high enough to exceed normal NPC health (e.g., 500 damage). Special NPCs survive due to higher max health, not a special flag
- **Style swap:** `LinkAnimClassLayers()` on Anim Layer Interface to swap montage sets. Called once when style is learned
- **Trainer fights:** gym interior sub-level with ring boundaries. Trainer is an NPC with boosted health using the gym’s fighting style. On CJ death → normal death flow. On forfeit (leaving ring volume) → weapon inventory cleared. On trainer defeat → set new style on player

### Animations Needed

**Per fighting style (4 sets — maps to original FIGHT_B/C/D + base):**

- Idle stance, combo chain (2–4 attacks), block pose + block hit-react, running/moving attack, ground attack (stomp or punch variant)

**Shared (on defender, not style-specific):**

- 3 hit reaction animations (low/mid/high per `HitLevel`), knockdown/fall, get-up from prone

---

# 3. Weapon Combat

<aside>
🔫

**SPEC §3** — 13 weapon slots, all firearms and melee weapons, lock-on targeting, weapon skill progression, dual-wielding, headshots, drive-by.

</aside>

### Key Deliverables

- **Melee weapons** — 10 types (Slot 0–1) with distinct damage/range from weapon.dat
- **Firearms** — Slots 2–9: handguns, shotguns, SMGs, assault rifles, rifles, heavy weapons, thrown, handheld
- **Weapon data asset** — `UDataAsset` driven by weapon.dat values (damage, clip, fire rate, range, accuracy per tier)
- **Lock-on targeting** — 90° frontal cone, closest hostile, reticle color states, target cycling
- **Weapon skill progression** — Poor → Gangster → Hitman; snap tier transitions; per-weapon point tracking
- **Dual-wielding** — Sawn-Off, Micro SMG, Tec-9, Pistol at Hitman tier (disables lock-on)
- **Headshot system** — binary instant-kill flag (`SUFFERS_CRITICAL_HITS`) on NPCs, not a damage multiplier

### UE5 Approach

- Weapon inventory as component (`USAWeaponComponent`) managing 13 slots
- Hit detection: sphere traces for melee (existing pattern), line traces for firearms
- Lock-on via overlap sphere + dot-product cone filter

**Damage System:**

- **Characters (player + NPCs):** `Health` and `Armor` as float properties on the character base class (not a component). All damage flows through `AActor::TakeDamage` override. Armor absorbs bullet/explosion damage but NOT fall damage — checked via `TSubclassOf<UDamageType>`. Player: max health 100→176, max armor 100→150. NPCs: `bSuffersCriticalHits` bool — headshot = instant kill if true
- **Vehicles:** `USAVehicleDamageComponent` manages health (1000 scale), visual stages, fire timer, panel detachment, and `fCollisionDamageMultiplier`. Vehicle’s `TakeDamage` forwards to the component
- **Breakable world props:** `USABreakableComponent` — attach to any actor in the editor. `UPROPERTY` fields for: broken mesh, bExplodes, explosion radius, particle/sound effects. Listens for `OnComponentHit`, swaps mesh on contact. No health tracking — binary intact/destroyed
- **Damage types:** `USADamageType_Bullet`, `USADamageType_Melee`, `USADamageType_Explosion`, `USADamageType_Fire` (DOT), `USADamageType_Fall`
- **Damage flow:** weapon component traces → `UGameplayStatics::ApplyDamage` with damage type → target’s `TakeDamage` handles armor, headshots, health reduction, death

**Damage Sources (how damage enters the system):**

- **One-shot:** melee trace, firearm trace, fall landing → single `ApplyDamage` call
- **Radial:** explosions → `UGameplayStatics::ApplyRadialDamage` (built-in), hits all actors in radius. Chain explosions: one vehicle exploding damages nearby vehicles via radial damage
- **Zone / DOT:** Molotov fire area, flamethrower, tear gas → persistent actor with overlap volume + timer calling `ApplyDamage(FireType)` or `ApplyDamage(MeleeType)` each tick on overlapping actors. The zone actor handles duration and spread, the receiver’s `TakeDamage` handles health reduction normally
- **Continuous:** chainsaw → weapon component calls `ApplyDamage` every tick while trace overlaps the target
- **Internal drains:** drowning (breath depleted), starvation (hunger timer) → directly modify `Health` on the character from stat timer callbacks. These bypass `TakeDamage` entirely since they’re not external damage
- **Bypass:** stealth kill → triggers death directly via animation callback, no damage calculation
- **Special:** spike strip → not damage. Contact calls `PopAllTires()` on the vehicle, no health reduction

**Vehicle Occupant Protection:**

- When a vehicle receives `TakeDamage`, damage applies to the vehicle only — never forwarded to the player/passengers inside. Occupants are protected until they exit or the vehicle explodes. The vehicle’s explosion kills/ejects all occupants directly

### Animations Needed

*To be filled in — expected categories: melee weapon swings (per weapon type), firearm aim/fire/reload (per weapon class), dual-wield variants, thrown weapon arc, drive-by shooting (driver, passenger, motorcycle)*

---

# 4. Stealth

<aside>
🕵️

**SPEC §4** — Crouch-based stealth, knife stealth kills, detection via LOS + noise + facing.

</aside>

### Key Deliverables

- **Stealth mode** — toggle crouch; minimap blip turns blue; silent movement
- **Knife stealth kill** — from behind, locked on, crouch; throat-slit animation; instant kill; silent
- **Detection system** — ~120° frontal cone, ~15–20 unit range, LOS + noise + approach direction
- **Silenced weapons** — quieter but not perfectly silent; ~5 unit detection radius
- **Gunfire alert** — alerts all NPCs in radius regardless of facing

### UE5 Approach

- Stealth detection as a polling check on AI controllers (not a separate stealth component)
- Detection cone via dot-product check against NPC forward vector
- Noise as a radius-based event broadcast; silenced weapons use smaller radius

### Animations Needed

*To be filled in — expected categories: stealth knife raise, throat-slit kill, crouch-walk with weapon*

---

# AnimBP Architecture

All AnimBP graphs are authored in Blueprint, driven by C++ variables exposed from `USAAnimInstance`. The C++ side updates every tick in

## C++ Layer: USAAnimInstance

Base `UAnimInstance` subclass at `Source/GTASA/Animation/SAAnimInstance.h`. Exposes all variables as `UPROPERTY(BlueprintReadOnly)` organized by system:

| Category | Variables | Source |
| --- | --- | --- |
| **Locomotion** | `Speed`, `Direction`, `MovementState` (enum), `bIsSprinting` | CharacterMovementComponent |
| **Ground State** | `bIsOnGround`, `bIsInAir`, `bIsFalling`, `bIsCrouching` | CharacterMovementComponent |
| **Swimming** | `bIsSwimmingSurface`, `bIsSwimmingDive`, `SwimSpeed` | Custom movement mode |
| **Climbing** | `bIsClimbing` | Custom movement mode |
| **Combat** | `bIsAiming`, `bIsBlocking`, `AimPitch`, `AimYaw` | WeaponComponent / Controller |
| **Weapon** | `ActiveWeaponType` (enum), `bIsDualWielding`, `bIsReloading` | WeaponComponent |
| **Fighting Style** | `FightingStyle` (enum) | Character / StatsComponent |
| **Body Shape** | `FatLevel` (0–1), `MuscleLevel` (0–1), `bUseFatLocomotion` | StatsComponent |
| **Vehicle** | `bIsInVehicle`, `VehicleSeatType` (enum) | Character state |

`NativeUpdateAnimation` pulls these from the owning pawn every tick. No casting chains — the anim instance caches a typed pointer to `ASAPlayerCharacter` on init.

## Blueprint Layer: AnimBP Setup Instructions

These are the AnimBPs you need to create in the editor. Each one uses `USAAnimInstance` (or a subclass) as its parent class.

### ABP_Player (Player On-Foot)

**Parent class:** `USAAnimInstance`

**State Machine: Locomotion**

1. **Idle** → transition to Walk/Run when `Speed > 0`
2. **Walk/Run/Sprint** → 1D Blend Space driven by `Speed`; output selects walk→run→sprint
3. **Crouch** → separate 1D Blend Space (crouch idle → crouch walk) driven by `Speed`; enter when `bIsCrouching`
4. **Jump** → 3 states: JumpStart → InAir (loop) → JumpLand; transitions on `bIsInAir` and `bIsOnGround`
5. **Fall** → enter from InAir when fall time > threshold; plays freefall loop
6. **SwimSurface** → Blend Space: idle treading → breaststroke → front crawl; driven by `SwimSpeed` and `bIsSprinting`
7. **SwimDive** → dive forward loop; driven by `SwimSpeed`
8. **Climb** → mantle/pull-up animation; enter on `bIsClimbing`
9. **CombatRoll** → montage-driven (not a state machine state)

**Layer: Upper Body (Aiming)**

- Blended on top of locomotion (spine and above)
- Active when `bIsAiming == true`
- Uses `AimPitch` / `AimYaw` for aim offset (2D Blend Space)
- Weapon type (`ActiveWeaponType`) selects the correct aim pose set via Anim Layer Interface

**Layer: Montage Slots**

- `DefaultSlot` — full-body montages (melee combos, stealth kills, interactions)
- `UpperBodySlot` — upper-body montages (fire, reload) that blend with locomotion

**Fat Override Logic:**

- In the Locomotion state machine, the Walk/Run/Sprint Blend Space node checks `bUseFatLocomotion`
- If true: swap to FatWalk/FatSprint Blend Space (use a "Blend Poses by Bool" node)
- This is a node-level switch, not a separate state

### ABP_Player_FightingStyle (Anim Layer Interface)

**Purpose:** Swap combat montage sets per fighting style without rebuilding the AnimBP.

1. Create an **Anim Layer Interface** (`ALI_FightingStyle`) with these layer functions:
    - `GetComboChain` — returns the combo montage sequence
    - `GetRunningAttack`
    - `GetGroundAttack`
2. Create **4 Linked Anim Layer implementations** (one per style):
    - `ABP_Style_Default`, `ABP_Style_Boxing`, `ABP_Style_KungFu`, `ABP_Style_MuayThai`
3. At runtime, C++ calls `LinkAnimClassLayers()` to swap the active style

### ABP_NPC (Pedestrian / Gang Member / Cop)

**Parent class:** `USANPCAnimInstance` (subclass of `USAAnimInstance` with NPC-specific vars)

Simplified version of ABP_Player:

- **Locomotion:** Idle → Walk → Run (no sprint blend, no swimming/climbing)
- **Combat:** Aim/fire montages per weapon type
- **Reactions:** Flinch, flee run, cower, death/ragdoll
- **No fighting style layers** — NPCs use a fixed melee set

### ABP_Vehicle_Car / ABP_Vehicle_Motorcycle / ABP_Vehicle_Bicycle

**Parent class:** `USAVehicleAnimInstance` (separate from `USAAnimInstance`)

These are simpler pose-based AnimBPs:

- **Car:** static driving pose; head turns with look input; drive-by upper body layer
- **Motorcycle:** lean blend space driven by `LeanAngle`; wheelie/stoppie pose blend
- **Bicycle:** pedaling loop driven by `PedalSpeed`; bunny hop montage; lean

### Wiring Checklist

When setting up each AnimBP in the editor:

- [ ]  Set parent class to the correct C++ AnimInstance subclass
- [ ]  Create the state machine(s) listed above
- [ ]  Wire transition rules to read the `BlueprintReadOnly` variables (no casting needed — they’re on the anim instance itself)
- [ ]  Create Blend Spaces as separate assets; reference them in the state machine nodes
- [ ]  Add Montage Slot nodes (`DefaultSlot`, `UpperBodySlot`) so C++ can play montages
- [ ]  For fighting styles: create the Anim Layer Interface and 4 implementations
- [ ]  Assign the AnimBP to the skeletal mesh component in the character’s Blueprint defaults

---

## Vehicles

# 5. Vehicles: Ground

<aside>
🚗

**SPEC §5 (partial)** — Cars, motorcycles, bicycles with handling.cfg-driven physics, damage model, enter/exit, tires, nitrous, vehicle skills.

</aside>

### Key Deliverables

- **Base vehicle class** — `ASAVehicle` using Chaos Vehicle system (`UChaosWheeledVehicleMovementComponent`)
- **Handling data asset** — mirrors handling.cfg: 34 parameters per vehicle (mass, traction, gears, top speed, etc.)
- **Cars** — suspension, steering, braking, drivetrain (FWD/RWD/AWD)
- **Motorcycles** — lean angle, wheelie/stoppie physics, fall-off conditions, bike skill scaling
- **Bicycles** — pedaling (tap sprint), bunny hop (charge + release), stamina drain, wheelie speed boost
- **Vehicle damage model** — 5 health stages (1000→0), smoke/fire/explosion sequence, body panel detachment
- **Collision damage** — `impactForce × fCollisionDamageMultiplier` per vehicle
- **Tire puncture** — individual tire blowout, spike strip = all tires, handling degradation
- **Enter/exit** — player ↔ vehicle transition (animation + possession swap)
- **Vehicle skills** — Driving, Cycling, Bike Skill (0–1000 each)
- **Nitrous oxide** — 2x/5x/10x tanks, acceleration boost, auto-refill, UI indicator
- **Drive-by shooting** — driver (SMGs, left/right window), passenger (wider arc), motorcycle (one-handed)

### UE5 Approach

- Chaos Vehicle plugin for wheeled vehicles — feed handling.cfg values into `UChaosWheeledVehicleMovementComponent`
- Motorcycle as subclass with custom lean/wheelie/stoppie logic layered on Chaos Vehicle
- Bicycle as a separate `ACharacter`-based pawn (not Chaos Vehicle — pedaling is animation-driven, not engine-driven)
- Damage model as a component (`USAVehicleDamageComponent`) tracking HP and triggering VFX/SFX per stage
- Enter/exit via `APawn::PossessedBy` swap — player controller possesses vehicle pawn, character hidden/attached
- Drive-by as an aiming mode on the weapon component that constrains firing arc based on seat

### Animations Needed

*To be filled in — expected categories: vehicle enter/exit (car, motorcycle, bicycle), driving pose (car, motorcycle, bicycle), pedaling, bunny hop, wheelie/stoppie rider lean, motorcycle fall-off/ragdoll, car-jack*

---

# 6. Vehicles: Air, Water & Special

<aside>
✈️

**SPEC §5 (remaining)** — Aircraft, boats, trains, parachute, Combine Harvester, RC vehicles.

</aside>

### Key Deliverables

- **Fixed-wing aircraft** — throttle, pitch, roll, rudder, retractable landing gear; performance degrades with damage
- **Helicopters** — collective (altitude), cyclic (pitch/roll), tail rotor (yaw)
- **Boats** — water vehicle physics; 11 types
- **Trains** — rail-locked; throttle + brake only; derail above ~190 km/h on curves; cinematic camera
- **Parachute** — auto-added when exiting aircraft at altitude; deploy, steer, flare/faceplant landing
- **Combine Harvester** — pedestrian interaction (rear chute ejects parts); locked doors
- **RC Vehicles** — 6 toy vehicles for Zero's missions
- **Flying Skill** — reduces turbulence, improves responsiveness; Pilot School boost

### UE5 Approach

- Aircraft as custom `APawn` with physics-driven flight model (not Chaos Vehicle — too car-oriented)
- Helicopter as a separate pawn with rotor thrust + collective/cyclic input mapping
- Boats using buoyancy system (Chaos or custom float-point model)
- Trains on spline tracks with constrained movement component
- Parachute as a temporary actor attached to player character with drag physics

### Animations Needed

*To be filled in — expected categories: aircraft pilot sitting pose, helicopter pilot pose, boat steering pose, train throttle pose, parachute deploy/steer/land/faceplant, freefall, aircraft enter/exit, boat enter/exit*

---

## Player Progression

# 7. RPG Stats & Progression

<aside>
📊

**SPEC §6** — All trackable stats, health/armor, hunger, stat gain/loss rates, daily caps.

</aside>

### Key Deliverables

- **Stat component** (`USAStatsComponent`) — centralized tracking for all stats (0–1000 scales):
    - Stamina, Muscle, Fat, Lung Capacity, Weapon Skills (per-weapon), Driving, Flying, Cycling, Bike, Gambling
- **Stat gain/loss rules** — activity-based rates from SPEC §6 (daily caps, starvation decay, etc.)
- **Health/Armor** — max health 176 (Paramedic L12), max armor 150 (Vigilante L12)
- **Hunger timer** — must eat every 72 game-hours; starvation drains fat → muscle → health
- **Eating system** — food items with fat gain values; overeating = vomit
- **Respect** — composite stat (40% actions + 36% missions + 6% territory + 6% money + 4% fitness + 4% girlfriends + 4% appearance)
- **Sex appeal** — 50% clothing + 50% vehicle; muscle modifier

### UE5 Approach

- `USAStatsComponent` on player character — flat struct of floats, no GAS overhead
- Respect/sex appeal as derived values recalculated when inputs change (not polled)
- Food/store interactions via interaction interface on world actors

### Animations Needed

*To be filled in — expected categories: eating/vomiting, gym workouts (treadmill, bench press, dumbbells, exercise bike)*

---

# 8. Body & Appearance

<aside>
👔

**SPEC §1 (Fat Effects), §6 (Body Morphing), §7 (Clothing & Appearance)** — Body morphing, clothing, tattoos, haircuts.

</aside>

### Key Deliverables

- **Body morphing** — fat/muscle drive morph targets on skeletal mesh (4 visual states)
- **Fat effects on movement** — swap to FatSprint/FatWalk animation groups above 25% fat
- **Clothing system** — 7 equipment slots, 6 stores with price/stat tiers, special outfits
- **Tattoos** — 7 body areas, respect or sex appeal contribution, permanent/replaceable
- **Haircuts** — barber shops, respect + sex appeal effects

### UE5 Approach

- **Body morphing:** two morph targets on the base skeletal mesh — `MT_Fat` and `MT_Muscle` — each driven by `FatLevel` and `MuscleLevel` (0–1) from `USAStatsComponent`. Both can be active simultaneously for the bulky build. Updated on stat-change delegate, not every tick
- **Clothing:** modular skeletal mesh system using `USkeletalMeshComponent` per slot, attached to the character’s main mesh via **Leader Pose Component** (clothing follows the same skeleton). Each clothing item is a `UDataAsset` with mesh + material + stat modifiers. Equipping swaps the mesh on the slot component
- **Morph propagation to clothing:** clothing meshes include the same morph targets (`MT_Fat`, `MT_Muscle`) as the base body. When body morph changes, clothing components receive the same morph values so they deform together and avoid clipping
- **Special outfits:** override torso + legs + shoes slots simultaneously; stored as a single `UDataAsset` with 3 mesh references. Hats/watches/chains/glasses remain independently equipped on their own slot components
- **Fat locomotion swap:** `bUseFatLocomotion` bool on the anim instance; AnimBP uses Blend Poses by Bool node to swap between normal and fat locomotion blend spaces

### Animations Needed

*To be filled in — expected categories: FatWalk/FatSprint locomotion set, buff walk/run override, barber/tattoo chair sit, clothing change*

---

## UI & Core Systems

# 18. UI, HUD & Save System

<aside>
📺

**SPEC cross-cutting** — HUD, minimap, weapon wheel, pause menu, save/load, 100% completion. Needed from Phase 1 onward.

</aside>

### Key Deliverables

**HUD Elements (always visible):**

- Health bar + armor bar (bottom-left)
- Money counter (top-right)
- Wanted stars (top-right, below money)
- Minimap / radar (bottom-left corner)
- Weapon icon + ammo count (top-right)
- Game clock

**HUD Elements (contextual — appear/fade as needed):**

- Stamina bar — appears during sprint, fades when full
- Breath bar — appears while diving, fades when surfaced
- Vehicle damage indicator — while driving
- Nitrous gauge — while in nitrous-equipped vehicle

**Minimap / Radar:**

- Real-time top-down view; player arrow at center, rotates with camera
- Territory colors, mission markers, collectible blips, NPC dots, police radius
- Zone name display on entry

**Weapon Wheel:**

- 13-slot radial selector matching SPEC §3 weapon slots
- Hold to open, right stick/mouse selects sector, release to confirm
- Shows weapon icon, name, and ammo per slot

**Pause Menu:**

- Full-screen map with zoom/pan
- Stats page (all RPG stats, weapon skills, vehicle skills)
- Brief (current mission objectives)
- Game settings (audio, video, controls)

**Save/Load:**

- `USaveGame` subclass persisting all game state: player stats (all 15+), weapon inventory + ammo + skill levels, money, health/armor, game time + day, weather state, zone unlock progress, gang territory ownership (53 entries), safehouse ownership, vehicle garage contents (class + mods per slot), asset business ownership + accumulated income, mission completion flags, collectible bitfields (tags/photos/horseshoes/oysters/jumps), girlfriend relationship percentages, clothing equipped per slot, tattoos, haircut, 100% completion checklist
- Save triggered at safehouses (bed interaction)

**100% Completion:**

- Checklist tracking all requirements
- Reward: $1M + infinite ammo + doubled vehicle durability + Hydra + Rhino spawns

### UE5 Approach

- **HUD architecture:** `USAHUDWidget` (C++ base) with child widgets per element. Each child binds to delegates on gameplay components (`OnHealthChanged`, `OnWantedLevelChanged`, `OnAmmoChanged`, etc.) — no tick polling
- **Minimap:** `USceneCaptureComponent2D` aimed downward above player; renders to `UTextureRenderTarget2D`; dynamic material instance overlays zone colors, blips, and player arrow
- **Weapon wheel:** radial menu widget; hold trigger opens, right stick/mouse selects sector, release confirms; each sector mapped to weapon slot index
- **Save system:** single `USaveGame` subclass; serialized via `UGameplayStatics::SaveGameToSlot` / `LoadGameFromSlot`; save trigger flushes all component state into the save object before writing
- **Pause menu map:** same scene capture as minimap but full-resolution with zoom/pan via stick or mouse drag

### Animations Needed

*None — UI system, no character animations.*

---

## World, AI & Law Enforcement

# 9. Open World: Time, Weather & Zones

<aside>
🌍

**SPEC §10** — Day/night cycle, weather system, zone-based region locking.

</aside>

### Key Deliverables

- **Time system** — 1 real second = 1 game minute; day-of-week tracking; game clock on HUD
- **Weather system** — 5 types (sunny, cloudy, rain, fog, sandstorm); region-restricted rules; progressive transitions
- **Zone system** — 8 regions with named sub-zones; story-based locks (Act 1/2/3); 4-star penalty for early entry

### UE5 Approach

- **Time system:** `USAWorldSubsystem` ticks a `GameMinute` counter at 1 per real second. Drives `ASkyAtmosphere` sun angle and `ADirectionalLight` rotation for sunrise/sunset. Broadcasts `OnHourChanged` and `OnDayChanged` delegates for dependent systems (hunger timer, asset income, NPC spawn schedules)
- **Weather system:** weather stored as enum state on the subsystem. Transitions lerp between `FSAWeatherPreset` structs (fog density, rain particle rate, cloud coverage, wind intensity, sky color tint, post-process exposure). Lerp duration ~30 game-minutes. Region rules enforced when selecting next state (skip rain if player zone is desert-flagged; only allow sandstorm in Bone County)
- **Zone detection:** `ASAZoneVolume` actors placed in the level, each with metadata: region enum, zone name string, gang affiliation, population density tier, `LockedUntilAct` flag. Player overlap tracked via `OnComponentBeginOverlap` / `EndOverlap`. Current zone cached on player controller; zone name displayed on HUD on entry. If `LockedUntilAct > CurrentAct`, immediately sets 4 stars on the wanted level component
- **NPC population:** zone density tier + time-of-day slot (12 two-hour intervals) determines target ped/vehicle count. Spawner runs on timer, not every frame

### Animations Needed

*None — this is a systems/rendering feature with no character animations.*

---

# 10. NPC AI

<aside>
🧍

**SPEC §10 (NPC AI section)** — Pedestrian behavior, traffic AI, gang member AI, decision makers.

</aside>

### Key Deliverables

- **Pedestrian AI** — pedstats.dat-driven behavior (fear, temper, lawfulness); decision maker event-response tables; flee/scream/attack/call-police reactions
- **Traffic AI** — density by time-of-day and zone (popcycle.dat); neighborhood-appropriate vehicle spawning; path-node following; panic mode
- **Gang member AI** — color-coded territory presence; verbal warnings → weapon draw → fire; high temper + low lawfulness
- **NPC spawning** — streaming-based; spawn outside camera frustum; despawn at distance; population caps by zone type and time
- **NPC archetypes** — 7 types, each with distinct StateTree and pedstats: Civilian, Gang Member (per gang), Police Officer, SWAT, FBI Agent, Military Soldier, Mission NPC
- **Decision maker system** — event-response probability tables per archetype. Each event (gunshot heard, explosion, player attack, vehicle theft witnessed) has weighted outcomes: flee, cower, scream, fight back, call police, ignore

### UE5 Approach

- **C++ state machine per archetype:** each AI controller subclass owns a state enum and runs transitions in `Tick`. High-level states:
    - **Civilian:** Idle → Wander → React (flee/cower/fight/call police) → Return to normal
    - **Gang Member:** Idle in group → Patrol territory → Warn intruder → Draw weapon → Engage combat → Flee if outgunned
    - **Police (1–2 star):** Patrol → Detect crime → Pursue → Attempt arrest → Escalate to lethal if player is armed
    - **Police (3+ star):** Aggressive pursuit → Shoot on sight → Coordinate with helicopter
    - **SWAT/FBI/Military:** Assault mode only — advance on player, use cover, flanking
- **Pedstats data asset:** `USAPedStatsAsset` maps 1:1 to pedstats.dat — fear (0–100), temper (0–100), lawfulness (0–100), flee distance, heading change rate. Referenced by each AI controller for decision thresholds
- **Decision makers:** `USADecisionMakerAsset` per archetype — each event type has a `TArray<FSADecisionEntry>` with task enum + weight. At runtime, weighted random selects the response. Checked in AI controller state transitions
- **Spawn budget:** `USANPCSpawnerSubsystem` manages global NPC count. Budget = zone density tier × time-of-day multiplier. Spawns outside camera frustum at 50–80 m. Despawns beyond 120 m. Soft cap ~30 peds + ~15 vehicles (tunable via `UPROPERTY`). Spawn points along sidewalk splines (peds) and road splines (vehicles)
- **Traffic:** vehicles follow road spline networks via spline-following movement component. Each spline has speed limit and lane data. NPC drivers check for obstacles via forward traces. Panic mode: max speed, ignore lanes, erratic steering

### Animations Needed

*To be filled in — expected categories: NPC civilian idle/walk/run variations, NPC flee/cower/scream, gang member idle/taunt/draw weapon, NPC driving poses, NPC reactions (call police, fight back), crowd ambient behaviors*

---

# 11. Wanted Level

<aside>
🚨

**SPEC §9** — Chaos points, 6-star police escalation, evasion mechanics, death/arrest penalties.

</aside>

### Key Deliverables

- **Chaos points** — internal counter; crime values from SPEC §9 table; proximity modifier (x2 within 14 units of cop)
- **6-star escalation** — thresholds (50/180/550/1200/2400/4600); max cops/cars/helicopters per level
- **Police behavior tiers** — patrol → armed → helicopter → SWAT (rappel) → FBI (SUVs) → Military (Rhino tanks)
- **Helicopter AI** — spotlight tracking, autocannon, SWAT rappelling at 4+ stars
- **Evasion** — chaos decay (urban vs. rural rates); Pay 'n' Spray; police bribes (-1 star); clothing change; safehouse save
- **Star availability** — 1–4 from start; 5 after Act 2; 6 after Act 3
- **Death/arrest penalties** — lose weapons + money; respawn at hospital/police station; girlfriend overrides

### UE5 Approach

- Wanted level as a `UActorComponent` on player controller — tracks chaos, manages police spawning/despawning
- Police units as AI-controlled pawns with StateTree; behavior tree switches per star level
- Helicopter as a separate AI pawn class with spotlight (spot light component) + line trace targeting

### Animations Needed

*To be filled in — expected categories: police arrest sequence, handcuff/bust, SWAT rappel from helicopter, cop weapon draw/fire, wasted/death camera animation*

---

# 12. Gang Territory

<aside>
🟢

**SPEC §8** — 53 territories, offensive/defensive wars, recruitment, rewards.

</aside>

### Key Deliverables

- **53 territories** — map overlay with gang colors (green/purple/yellow)
- **Offensive war trigger** — kill 3 rival gang members on foot in their territory
- **3 attack waves** — escalating enemy count and weapon quality; health/armor pickups between waves
- **Defensive wars** — random attacks on held territory; 1 wave; territory flashes red
- **Difficulty scaling** — territory darkness = difficulty; darker = more enemies + armed gang cars
- **Rewards** — +30% running respect per win; money pickup at Johnson House proportional to held territories
- **Gang recruitment** — target GSF member + recruit; max based on respect; follow/hold-position commands

### UE5 Approach

- Territory data as a `UDataAsset` array — each entry: zone ref, owning gang, darkness level, war state
- Gang war as a state machine on a `USAGangWarComponent` — idle → triggered → wave 1/2/3 → victory/defeat
- Map overlay via dynamic material on minimap widget

### Animations Needed

*To be filled in — expected categories: gang member combat anims (if distinct from NPC set), gang recruit follow/hold-position idles*

---

## Content & Economy

# 13. Economy & Properties

<aside>
💰

**SPEC §12, §18, §5 (Vehicle Customization)** — Money, shops, safehouses, businesses, mod garages.

</aside>

### Key Deliverables

- **Money** — player wallet; earn/spend tracking
- **Ammu-Nation** — weapon purchasing; inventory varies by location and story progress; prices from SPEC §18
- **Clothing stores** — 6 stores (Binco → Didier Sachs); buy/equip flow; respect/sex appeal contribution
- **Mod garages** — TransFender (65 car models), Loco Low Co. (8 lowriders), Wheel Arch Angels (6 tuners); paint, wheels, exhaust, nitrous, hydraulics
- **Safehouses** — 37 total (8 free + 29 purchasable); save point, wardrobe, garage
- **Asset businesses** — 10 assets; purchase cost + mission unlock; daily income with per-asset caps; collection pickups
- **Vehicle garage** — persistent vehicle storage at safehouses
- **Pay 'n' Spray** — clear wanted level; vehicle respray

### UE5 Approach

- **Shop interaction flow:** player enters trigger volume → sub-level streams in (interior) → shop UI widget activates → player browses categorized item list → selecting an item previews it on the character/vehicle in real-time → confirm purchase deducts money → exit trigger unloads sub-level
- **Shop data:** each shop is a `UDataAsset` containing: shop type enum, item catalog (array of item data assets), location-specific pricing modifiers (e.g., LV TransFender 20% markup), story-progress unlock flags
- **Vehicle customization:** each mod is a `USAVehicleModAsset` with: mesh component to swap (exhaust, spoiler, wheels), material override (paint), and price. Applied by swapping static/skeletal mesh components on the vehicle. Mod state persisted as array of mod asset references per vehicle in save data
- **Safehouse system:** `ASASafehouse` actor with child components — save trigger (bed interaction → write `USaveGame`), wardrobe trigger (opens clothing UI), garage volume (stores vehicle references). Purchasable safehouses have a buy-trigger checking player money
- **Vehicle garage persistence:** when a vehicle enters the garage volume, its class + applied mods + paint color are serialized into the save game. On load, the vehicle is respawned from this data at the garage spawn point
- **Asset income:** `USAWorldSubsystem` tracks each owned business. On `OnDayChanged` delegate, each business accumulates income up to its per-asset cap. Pickup actor at business entrance shows current uncollected amount; collecting resets to zero
- **Pay 'n' Spray:** drive-in trigger → garage door closes (Sequencer) → brief timer → random paint material applied → wanted level cleared → door opens. Crime during post-spray flash period reinstates full wanted level

### Animations Needed

*To be filled in — expected categories: shop browse/purchase interaction, Pay 'n' Spray vehicle enter/exit, safehouse save (bed), wardrobe interaction, money pickup, garage door open/close*

---

# 14. Side Activities & Minigames

<aside>
🎰

**SPEC §11, §16** — Gambling, vehicle missions, sports, burglar, races, gym, schools.

</aside>

### Key Deliverables

- **Vehicle missions** — Vigilante, Paramedic, Firefighter, Taxi (level progression + rewards)
- **Gambling** — Blackjack, Poker, Roulette, Slots, Horse Racing (LV only; skill-gated bet limits)
- **Schools** — Driving (12 lessons), Pilot (10), Boat (5), Bike (6); medal scoring (70/85/100%); vehicle rewards
- **Sports** — Pool (8-ball physics), Basketball (2v2), Dancing (rhythm), Lowrider Challenge (rhythm)
- **Burglar missions** — 20:00–06:00; noise meter; $20 x n squared reward; $10K milestone = infinite stamina
- **Races** — ~30 street/air/mountain bike races; stadium events (Bloodring, 8-Track, Dirt Track)
- **Gym workouts** — treadmill, weights, exercise bike (stat gain with daily caps)
- **Ammu-Nation shooting range** — timed target challenges

### UE5 Approach

- Minigames as self-contained game mode overrides or actor-driven sequences
- Rhythm games (dancing, lowrider) via input-timing evaluation against beat maps
- Schools as sequential challenge levels with scoring against time/accuracy thresholds

### Animations Needed

*To be filled in — expected categories: pool cue aim/shoot, basketball dribble/shoot/pass, dance moves (rhythm game), lowrider bounce, burglar sneak/carry furniture, spray paint tag, camera photo snap, gym workout sets*

---

# 15. Mission Framework

<aside>
📌

**SPEC §14** — Mission triggering, state machine, phone calls, failure/retry, multi-stage missions.

</aside>

### Key Deliverables

- **Mission trigger system** — colored markers at giver locations; letter icons on map
- **Mission state machine** — intro cutscene → objectives → success/failure → reward
- **Phone call system** — 7 threads, priority-based, cancel conditions
- **Failure/retry** — return to open world; drive back to retry
- **Multi-stage missions** — seamless transitions between combat/driving/flying segments
- **Story arc** — 4 acts, zone unlocks per act, ~100 missions (86 mandatory + 14 optional)

### UE5 Approach

- Mission framework as a `USAMissionSubsystem` — loads mission `UDataAsset` definitions, manages state, tracks completion
- Cutscenes via Sequencer
- Multi-stage as sequential objectives within a single mission data asset

### Animations Needed

*To be filled in — expected categories: cutscene-specific character animations (mission-dependent), phone pickup/talk/hangup*

---

# 16. Girlfriend System

<aside>
❤️

**SPEC §15** — 6 girlfriends, dating mechanics, relationship rewards.

</aside>

### Key Deliverables

- **6 girlfriends** with body/stat requirements
- **Dating flow** — restaurant, dancing, driving; +5%/+10% per date
- **Relationship rewards** at 35% (car access) and 100% (outfit + gameplay perk)
- **Jealousy system**
- **Millie shortcut** — can be killed on first date for casino keycard

### UE5 Approach

- Girlfriend as `UDataAsset` per character — requirements, date types, reward thresholds
- Date activities reuse existing minigame systems (restaurant, dancing)

### Animations Needed

*To be filled in — expected categories: girlfriend date interactions (kiss, gift), date-specific activities*

---

# 17. Collectibles & Import/Export

<aside>
🌟

**SPEC §13, §17** — All collectible types and Easter Basin Docks vehicle trading.

</aside>

### Key Deliverables

**Collectibles:**

- 100 Spray Tags (LS), 50 Photo Ops (SF), 50 Horseshoes (LV), 50 Oysters (underwater, entire map), 70 Unique Stunt Jumps
- Per-item and completion rewards

**Import/Export:**

- Easter Basin Docks; 3 lists of 10 vehicles; condition-based payout; magnetic crane
- List completion bonuses: $50K / $100K / $200K
- Exported vehicles become purchasable on day-of-week rotation at 80% export value

### UE5 Approach

- Collectibles as placed actors with a global tracking `UDataAsset` (bitfield per type)
- Import/Export as a UI-driven checklist system with vehicle condition evaluation on delivery

### Animations Needed

*To be filled in — expected categories: spray paint tag, camera photo snap*

---

*For execution phases, test checklists, architectural decisions, and the dependency graph, see GTA:SA — Execution Phases & Architecture* **The C++ side updates every tick in `NativeUpdateAnimation`; the Blueprint side reads those variables in state machine transitions and blend space inputs.** No gameplay logic in the AnimBP — it only reads state and selects animations.

## C++ Layer: USAAnimInstance

Base `UAnimInstance` subclass at `Source/GTASA/Animation/SAAnimInstance.h`. Exposes all variables as `UPROPERTY(BlueprintReadOnly)` organized by system:

| Category | Variables | Source |
| --- | --- | --- |
| **Locomotion** | `Speed`, `Direction`, `MovementState` (enum), `bIsSprinting` | CharacterMovementComponent |
| **Ground State** | `bIsOnGround`, `bIsInAir`, `bIsFalling`, `bIsCrouching` | CharacterMovementComponent |
| **Swimming** | `bIsSwimmingSurface`, `bIsSwimmingDive`, `SwimSpeed` | Custom movement mode |
| **Climbing** | `bIsClimbing` | Custom movement mode |
| **Combat** | `bIsAiming`, `bIsBlocking`, `AimPitch`, `AimYaw` | WeaponComponent / Controller |
| **Weapon** | `ActiveWeaponType` (enum), `bIsDualWielding`, `bIsReloading` | WeaponComponent |
| **Fighting Style** | `FightingStyle` (enum) | Character / StatsComponent |
| **Body Shape** | `FatLevel` (0–1), `MuscleLevel` (0–1), `bUseFatLocomotion` | StatsComponent |
| **Vehicle** | `bIsInVehicle`, `VehicleSeatType` (enum) | Character state |

`NativeUpdateAnimation` pulls these from the owning pawn every tick. No casting chains — the anim instance caches a typed pointer to `ASAPlayerCharacter` on init.

## Blueprint Layer: AnimBP Setup Instructions

These are the AnimBPs you need to create in the editor. Each one uses `USAAnimInstance` (or a subclass) as its parent class.

### ABP_Player (Player On-Foot)

**Parent class:** `USAAnimInstance`

**State Machine: Locomotion**

1. **Idle** → transition to Walk/Run when `Speed > 0`
2. **Walk/Run/Sprint** → 1D Blend Space driven by `Speed`; output selects walk→run→sprint
3. **Crouch** → separate 1D Blend Space (crouch idle → crouch walk) driven by `Speed`; enter when `bIsCrouching`
4. **Jump** → 3 states: JumpStart → InAir (loop) → JumpLand; transitions on `bIsInAir` and `bIsOnGround`
5. **Fall** → enter from InAir when fall time > threshold; plays freefall loop
6. **SwimSurface** → Blend Space: idle treading → breaststroke → front crawl; driven by `SwimSpeed` and `bIsSprinting`
7. **SwimDive** → dive forward loop; driven by `SwimSpeed`
8. **Climb** → mantle/pull-up animation; enter on `bIsClimbing`
9. **CombatRoll** → montage-driven (not a state machine state)

**Layer: Upper Body (Aiming)**

- Blended on top of locomotion (spine and above)
- Active when `bIsAiming == true`
- Uses `AimPitch` / `AimYaw` for aim offset (2D Blend Space)
- Weapon type (`ActiveWeaponType`) selects the correct aim pose set via Anim Layer Interface

**Layer: Montage Slots**

- `DefaultSlot` — full-body montages (melee combos, stealth kills, interactions)
- `UpperBodySlot` — upper-body montages (fire, reload) that blend with locomotion

**Fat Override Logic:**

- In the Locomotion state machine, the Walk/Run/Sprint Blend Space node checks `bUseFatLocomotion`
- If true: swap to FatWalk/FatSprint Blend Space (use a "Blend Poses by Bool" node)
- This is a node-level switch, not a separate state

### ABP_Player_FightingStyle (Anim Layer Interface)

**Purpose:** Swap combat montage sets per fighting style without rebuilding the AnimBP.

1. Create an **Anim Layer Interface** (`ALI_FightingStyle`) with these layer functions:
    - `GetComboChain` — returns the combo montage sequence
    - `GetRunningAttack`
    - `GetGroundAttack`
2. Create **4 Linked Anim Layer implementations** (one per style):
    - `ABP_Style_Default`, `ABP_Style_Boxing`, `ABP_Style_KungFu`, `ABP_Style_MuayThai`
3. At runtime, C++ calls `LinkAnimClassLayers()` to swap the active style

### ABP_NPC (Pedestrian / Gang Member / Cop)

**Parent class:** `USANPCAnimInstance` (subclass of `USAAnimInstance` with NPC-specific vars)

Simplified version of ABP_Player:

- **Locomotion:** Idle → Walk → Run (no sprint blend, no swimming/climbing)
- **Combat:** Aim/fire montages per weapon type
- **Reactions:** Flinch, flee run, cower, death/ragdoll
- **No fighting style layers** — NPCs use a fixed melee set

### ABP_Vehicle_Car / ABP_Vehicle_Motorcycle / ABP_Vehicle_Bicycle

**Parent class:** `USAVehicleAnimInstance` (separate from `USAAnimInstance`)

These are simpler pose-based AnimBPs:

- **Car:** static driving pose; head turns with look input; drive-by upper body layer
- **Motorcycle:** lean blend space driven by `LeanAngle`; wheelie/stoppie pose blend
- **Bicycle:** pedaling loop driven by `PedalSpeed`; bunny hop montage; lean

### Wiring Checklist

When setting up each AnimBP in the editor:

- [ ]  Set parent class to the correct C++ AnimInstance subclass
- [ ]  Create the state machine(s) listed above
- [ ]  Wire transition rules to read the `BlueprintReadOnly` variables (no casting needed — they're on the anim instance itself)
- [ ]  Create Blend Spaces as separate assets; reference them in the state machine nodes
- [ ]  Add Montage Slot nodes (`DefaultSlot`, `UpperBodySlot`) so C++ can play montages
- [ ]  For fighting styles: create the Anim Layer Interface and 4 implementations
- [ ]  Assign the AnimBP to the skeletal mesh component in the character’s Blueprint defaults

---

## C++ First, Thin Blueprint Layer

All gameplay systems, logic, and architecture are implemented in C++. Blueprints serve only as a thin configuration layer:

- **Asset references** — material instances, meshes, montages, sound cues, input actions/mapping contexts assigned via Blueprint defaults (not hardcoded paths in C++)
- **Parameter tweaking** — exposed `UPROPERTY(EditDefaultsOnly)` values tuned in BP subclasses (damage numbers, timers, thresholds, curves)
- **Widget layout** — UMG widget trees designed in the Widget Blueprint editor; logic in C++ base classes
- **AnimBPs** — animation graphs authored in Blueprint (state machines, blend spaces); driven by C++ variables via the anim instance interface
- **Level design** — actors placed and configured in the level editor

No gameplay logic lives in Blueprint event graphs. If a system needs iteration speed, expose the knobs as `UPROPERTY` — don't move the logic.

## No GAS (Gameplay Ability System)

GTA:SA's combat is animation-montage-driven with simple stat lookups, not ability-centric. GAS adds unnecessary complexity. Stats are flat floats on components; damage is direct function calls through interfaces.

## Dual Input: Keyboard+Mouse & Gamepad

All gameplay must work with both keyboard+mouse and gamepad. Enhanced Input handles this natively:

- **Single Input Mapping Context per action set** — each action binds both KB+M and gamepad keys in the same IMC (no separate "gamepad mode")
- **Analog vs digital** — gamepad sticks provide analog magnitude (walk/run/sprint blend); WASD uses digital-to-axis conversion with a dead zone ramp so movement still feels smooth
- **Aim input** — mouse provides raw delta for camera look; right stick uses a scaled turn rate with acceleration curve
- **Lock-on targeting** — L2/R2 (gamepad) and right-click (mouse) share the same IA; target cycling via shoulder buttons or scroll wheel
- **Drive-by aiming** — constrained arc works identically on both; stick direction or mouse position selects left/right window
- **UI navigation** — weapon wheel, pause menu, shop UIs support both d-pad/stick and mouse cursor; no mode toggle needed (Enhanced Input handles priority)
- **Context-sensitive prompts** — HUD button prompts swap icons based on last input device used (detected via `GetLastInputDevice()` on the player controller)

## Collision Channels

No custom object channels — built-in `Pawn`, `Vehicle`, `WorldStatic`, `WorldDynamic` cover all cases. Overlap-only volumes (zones, triggers, garages) are just collision profile settings, not new channels.

**2 custom trace channels:**

- **`SA_Weapon`** — melee sphere traces + firearm line traces. Hits: Pawn, Vehicle, WorldStatic, WorldDynamic (breakable props). Ignores: trigger volumes, water volumes. Used by weapon component for all hit detection including bone-specific checks (headshots, tire pops)
- **`SA_Interaction`** — short-range interaction trace. Hits: actors with interaction component (shops, pickups, NPCs, doors). Ignores: breakable props, vehicles, world geometry. Used by player interaction component for "press to interact" prompts

All other queries use built-in channels: `Camera` for spring arm, `Visibility` for AI line-of-sight and explosion radius checks.

## Chaos Vehicle for Wheeled Vehicles Only

Cars and motorcycles use Chaos Vehicle. Bicycles, aircraft, boats, and trains use custom pawn classes — their physics models are too different from wheeled vehicle simulation.