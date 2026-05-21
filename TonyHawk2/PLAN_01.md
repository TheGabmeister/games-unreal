# Phase 1 — Core Skating Movement

Implementation plan for the foundation: a skater capsule that moves, ollies, and lands in a debug skatepark with a chase camera.

**Placeholder approach**: No rigged character or animations. The skater is a capsule + skateboard mesh that changes color based on state (idle = grey, pushing = green, crouching = yellow, airborne = blue, landing = white flash, bailing = red). 3D environment models (ramps, quarterpipes, ground) are generated via Blender Python scripts.

---

## Step 1 — Project Skeleton & Skater Pawn

Set up the core C++ classes that everything else builds on.

**Classes to create:**

- `ATH2SkaterPawn` (APawn subclass) — the player-controlled skater
  - UCapsuleComponent as root (collision)
  - UStaticMeshComponent for the capsule visual (cylinder/capsule shape)
  - UStaticMeshComponent for the skateboard (parented under capsule)
  - USpringArmComponent + UCameraComponent for chase cam
  - USkeletalMeshComponent intentionally skipped — placeholder uses static mesh with dynamic material color
- `UTH2MovementComponent` (UPawnMovementComponent subclass) — custom skating physics
  - Ground movement, gravity, surface detection, velocity management
  - Not UCharacterMovementComponent — skating physics are too different from walk/run/fall
- `ATH2GameMode` (AGameModeBase subclass) — spawns the skater pawn
- `ATH2PlayerController` (APlayerController subclass) — Enhanced Input setup, input routing

**Config:**
- DefaultPawnClass = ATH2SkaterPawn
- PlayerControllerClass = ATH2PlayerController
- Set ATH2GameMode as the default GameMode in DefaultEngine.ini

**Input assets (created in-editor after first build):**
- Input Mapping Context: `IMC_Skating`
- Input Actions: `IA_Move` (Axis2D), `IA_Brake`, `IA_Ollie` (hold/release), `IA_SwitchStance`, `IA_BigDrop`
- These are editor-authored assets assigned via Blueprint — not constructed at runtime

**Deliverable:** Capsule spawns in the level, camera follows it, no movement yet.

---

## Step 2 — Ground Movement

Implement forward momentum, steering, braking, and auto-kick.

**Movement model:**
- Forward velocity vector along the pawn's forward axis
- Steering: left/right input rotates the pawn (yaw). Turn rate is sharper at low speed, wider at high speed
- Braking: backward input applies deceleration force
- Auto-kick (default on): gradual speed decay on flat ground (~0.5% per frame); crouching slows decay rate
- Speed cap: hardcoded default (stat-driven in Phase 7). Use `MaxSpeed = 1200.0f` as starting value
- Surface alignment: pawn rotation pitches to match ground normal via line traces
- Friction model: different deceleration rates for flat vs uphill vs downhill

**Capsule color:**
- Grey when idle/coasting
- Green during active push acceleration

**Deliverable:** Capsule skates around flat ground, steers, brakes, maintains/loses speed naturally.

---

## Step 3 — Ollie System

Hold-to-crouch, release-to-jump with height scaling.

**Mechanics:**
- Hold X: enter crouch state, accumulate charge (0 to 1.0 over ~0.5 seconds)
- Release X: launch upward. Jump impulse = `MinOllieHeight + (MaxOllieHeight - MinOllieHeight) * ChargeRatio`
- Hardcoded defaults: MinOllieHeight = 300, MaxOllieHeight = 600 (units = cm/s upward velocity)
- Transition to airborne state on launch
- Crouch during forward movement: speed is maintained (no push), but decay is slowed

**State transitions:**
- Ground → Crouching (X pressed)
- Crouching → Airborne (X released)
- Airborne → Landing (ground contact detected)

**Capsule color:**
- Yellow while crouching/charging
- Blue while airborne

**Deliverable:** Capsule crouches, launches into the air with variable height, falls under gravity.

---

## Step 4 — Air Physics & Landing

Gravity arc, hangtime, landing detection, and Big Drop recovery.

**Air physics:**
- Gravity: standard UE gravity with a hangtime modifier near the peak of the arc
- Hangtime: when vertical velocity is near zero (within threshold), reduce effective gravity by ~40% to create the floaty THPS feel. Hardcoded default; Hangtime stat drives this in Phase 7
- Air control: minimal lateral adjustment allowed (slight forward/backward influence)
- Maintain horizontal velocity from launch

**Landing detection:**
- Downward line trace from capsule bottom
- Surface normal check: if angle between pawn's forward vector and surface tangent exceeds threshold → bail (overspin check placeholder for Phase 5)
- Clean land: velocity redirected along surface, brief landing recovery state (~0.15s)
- Capsule: white flash on clean landing (lerp back to grey over 0.3s)

**Big Drop recovery:**
- Track fall distance (or vertical velocity at impact)
- If fall exceeds BigDropThreshold and player presses X within a window before landing (~0.2s) → clean landing
- If fall exceeds BigDropThreshold without X press → bail

**Bail state:**
- Capsule turns red
- Velocity zeroed, brief ragdoll-like tumble (capsule tips over and slides, then respawns upright after ~1.5s)
- Respawn at bail location facing original direction

**Deliverable:** Full ollie arc with hangtime feel, clean landings, Big Drop mechanic, bail on excessive drops.

---

## Step 5 — Advanced Launches

Nollie, No Comply, and Boneless/Fastplant — directional ollie variants.

**Input sequences (all require Up + X in various timings):**
- **Nollie** (Up + X simultaneously): slightly different launch arc, ~200 pts base (scoring wired in Phase 5, but track the launch type now)
- **No Comply** (Up then X quickly, within ~8 frames): quick pop, similar height to normal ollie
- **Boneless/Fastplant** (Up, Up, X within ~16 frames / ~0.27s at 60fps): higher launch than normal ollie (~1.2x max height), more airtime

**Implementation:**
- Input buffer system: track recent directional inputs with timestamps
- On X press, check buffer for Up inputs to determine launch variant
- Store launch type enum on the pawn for later scoring use
- Visual: different launch "flash" colors or brief capsule stretch to distinguish variants

**Deliverable:** Three distinct ollie variants with correct input detection and different launch properties.

---

## Step 6 — Switch Stance

Toggle between regular and switch riding.

**Mechanics:**
- R2 toggles switch stance (boolean on the pawn)
- Switch stance is purely a flag in Phase 1 — affects scoring multipliers in later phases
- Visual indicator: capsule gets a subtle stripe or secondary color when in switch (e.g., a band of orange around the middle)
- Persists until toggled again or until bail (bail resets to regular stance)

**Deliverable:** Switch stance toggles and is visually distinguishable. The flag is stored for future scoring use.

---

## Step 7 — Chase Camera

Third-person camera behind and above the skater.

**Setup:**
- USpringArmComponent: arm length ~400cm, offset slightly above pawn center
- Camera lag enabled (slight smoothing on position, ~10.0 lag speed)
- Camera rotation follows pawn yaw with interpolation (not instant snap)
- Pitch: fixed slight downward angle (~-15 degrees)
- No player camera control (no right stick look)

**Vert ramp behavior:**
- When airborne off a vert ramp (detected by launch angle > 60 degrees from horizontal), pull camera back (increase spring arm length by ~1.5x) and tilt down to keep skater in frame
- Lerp back to normal on landing

**Ground behavior:**
- Camera auto-follows pawn rotation
- Collision: spring arm's built-in collision test prevents camera clipping through walls

**Deliverable:** Smooth chase camera that follows the skater, pulls back during vert air.

---

## Step 8 — Debug Skatepark Level

The test environment with enough geometry to validate all Phase 1 mechanics.

**Assets (Blender Python scripts → FBX → import into UE):**

1. **Ground plane**: large flat area (~100m x 100m), concrete material
2. **Quarter pipes** (2): curved ramp surfaces, ~3m tall, positioned facing each other ~30m apart
3. **Vert ramp** (1): full halfpipe, ~4m tall with vert (vertical top section), ~15m wide
4. **Kicker ramps** (2–3): small launch ramps at various angles (30, 45 degrees), ~1.5m tall
5. **Flat banks** (2): gentle slopes connecting different elevation areas
6. **Boundary walls**: invisible or low walls around the perimeter to keep the skater in bounds

**Blender script approach:**
- Python scripts generate meshes procedurally (parametric ramp curves, extruded profiles)
- Export as FBX with collision geometry (simple convex hulls)
- Import into UE, set up materials (solid colors: grey concrete, dark grey asphalt, blue for ramp surfaces)

**Level setup in UE:**
- Place imported meshes in a new level `L_DebugPark`
- Add directional light + skylight for basic lighting
- Set `L_DebugPark` as the editor startup map
- Mark ramp surfaces with a tag or component for surface-type detection (flat, ramp, vert)

**Collision:**
- Wall collision: stop forward velocity, slight bounce-back
- Ramp collision: redirect velocity along ramp surface normal
- Ground: standard floor collision via movement component sweep

**Deliverable:** A playable skatepark with flat ground, ramps, quarterpipes, and a vert halfpipe.

---

## Step 9 — Collision & Surface Response

Proper interaction with the environment geometry.

**Wall collision:**
- On hitting a wall at high speed (above threshold) → bail
- On hitting a wall at low/medium speed → velocity zeroed in wall-normal direction, slide along wall
- Brief "bump" visual (capsule flashes orange)

**Ramp transitions:**
- Detect ramp surface via ground trace normal angle
- Smoothly rotate pawn to align with ramp surface
- Speed adjustment: maintain speed going up (convert horizontal to vertical), gain speed going down
- Ramp-to-air transition: when the ground trace loses contact at the top of a ramp/quarterpipe, launch into air state with velocity direction matching the ramp exit angle

**Vert ramp specifics:**
- Vert sections (near-vertical top of halfpipe) launch the skater straight up
- Air stat would affect height here (hardcoded default for now)
- Landing back on vert: redirect velocity downward along the ramp surface

**Deliverable:** Skater rides up and down ramps naturally, launches off vert, collides with walls appropriately.

---

## Build & Test Order

| Order | Step | Depends On | Key Validation |
|-------|------|-----------|----------------|
| 1 | Step 1 — Skeleton | Nothing | Capsule spawns, camera attached |
| 2 | Step 8 — Debug Park | Nothing (parallel with Step 1) | FBX assets exist, level loads |
| 3 | Step 2 — Ground Movement | Step 1 | Skate around the park |
| 4 | Step 7 — Chase Camera | Step 1 | Camera follows smoothly |
| 5 | Step 3 — Ollie | Step 2 | Variable height jumps |
| 6 | Step 4 — Air & Landing | Step 3, Step 8 | Full ollie arc on flat and off ramps |
| 7 | Step 9 — Collision | Step 4, Step 8 | Ramp transitions, wall stops |
| 8 | Step 5 — Advanced Launches | Step 3 | Nollie/No Comply/Boneless work |
| 9 | Step 6 — Switch Stance | Step 2 | Toggle works, visual indicator |

Steps 1 and 8 can be built in parallel. Steps 7 and 2 can overlap. Steps 5 and 6 are independent of each other.

---

## What Phase 1 Does NOT Include

- No animations or rigged character (capsule placeholder)
- No trick system (Phase 2+)
- No grinding or lip tricks (Phase 3)
- No manual / combo chaining (Phase 4)
- No scoring (Phase 5)
- No Special Meter (Phase 6)
- No UI beyond the capsule color feedback
- No stat system — all values hardcoded (Phase 7)
