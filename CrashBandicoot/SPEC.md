# SPEC.md — Crash Bandicoot (1996) Gameplay Recreation

This spec documents every gameplay system from the original 1996 Crash Bandicoot that we need to recreate in the CB project (UE 5.7). It is a gameplay-only recreation — we are not replicating the original art, music, or story. We focus on mechanics, rules, and feel.

---

## 1. Player Movement

Crash has a simple moveset: move, jump, and spin. No sprint, no crouch, no slide, no body slam (those were added in sequels).

### Ground Movement
- Single movement speed — no walk/run distinction. Pressing a direction reaches full speed quickly (short acceleration ramp, feels near-instant). Movement is digital in feel even with an analog stick — any stick deflection past the deadzone results in full-speed movement, matching the original D-pad-only controls.
- No momentum-based sliding. Releasing the stick stops Crash quickly.
- Movement direction is **camera-relative**: "forward" always means away from the camera, regardless of camera perspective.
- Walking into walls causes Crash to slide along the surface (standard wall sliding).

### Jump
- **Variable-height jump**: holding the jump button longer produces a higher jump. Releasing early produces a short hop. The critical input window is roughly the first 30–60ms of the jump. Jump height is controlled by hold duration, not stick tilt.
- **No double jump** (that's Crash 2).
- **Air control**: full left/right steering while airborne. Essential for platforming precision.
- **Coyote time**: Crash can jump for a brief window (~2 frames at 30fps, ~66ms) after walking off a ledge.
- **Landing**: no landing lag or recovery frames. Crash can act immediately on landing.
- **Falling**: gravity ramps up during a fall, giving Crash a "heavy" feel. This is not a floaty character — the descent is noticeably faster than the ascent.

### Collision
- The original game used rectangular/box hitboxes. The N. Sane Trilogy switched to pill/capsule collision, which introduced the infamous "sliding off platform edges" problem. We should use UE5's default capsule (since `UCharacterMovementComponent` is built around it) but compensate with generous platform collision and slight edge-correction to avoid the sliding-off feel.

---

## 2. Spin Attack

The spin is Crash's only attack (besides jumping on enemies).

- **Activation**: press the spin button at any time — on ground, in air, while moving or standing still.
- **Range**: approximately one character-width horizontally in all directions (360-degree sweep).
- **Duration**: brief animation, roughly 0.5s.
- **Effect on enemies**: defeats most enemies, launching them off-screen. Launched enemies can collide with and defeat other enemies.
- **Effect on crates**: breaks wooden crates. Spinning a TNT crate causes **immediate detonation** (unlike jumping on it, which starts a timer).
- **Spin cooldown**: the spin uses a hidden charge system. Crash has a small number of charges (~3–4) that deplete with each spin and passively recharge over time. When all charges are depleted, Crash simply cannot spin until at least one recharges. There is no visible "dizzy" animation or movement penalty — the player just presses spin and nothing happens until charges recover. Recovery is brief (~1–2 seconds for a full recharge). This prevents spin spam but is almost never noticed in normal play.
- **Movement during spin**: Crash maintains his current velocity during the spin. He can spin while moving and while airborne.
- **No horizontal boost**: spinning does not change Crash's speed or trajectory.
- **Jump cancel**: Crash can jump during a spin, cancelling the spin animation.
- **No effect on fall**: spinning in the air does not slow or change Crash's fall speed.

---

## 3. Damage, Death & Lives

### Taking Damage
- Without a mask, any hit kills Crash (costs a life).
- With 1–2 Aku Aku masks, a hit consumes one mask instead of killing Crash.
- **Knockback**: when Crash takes damage (whether a mask absorbs it or he dies), he is knocked backward away from the damage source. The knockback is a short, fixed-distance push — enough to separate him from the enemy, but not dramatic. Knockback **can** push Crash off a ledge or into a pit. During knockback, Crash briefly loses player control (~0.3–0.5s).
- **Post-hit invulnerability**: after a mask absorbs a hit, Crash gets ~1–2 seconds of invulnerability to prevent instant follow-up damage. During this window, Crash blinks/flashes. The invulnerability starts after knockback ends.

### Instant Death (ignores Aku Aku masks)
- Bottomless pits / falling off the level.
- Water (Crash cannot swim — touching water is instant death in most contexts).
- Being crushed between moving surfaces.
- Boulder contact (chase levels).

### Death Animations
Cosmetic only — mechanically they all cost 1 life and respawn at checkpoint.

| Cause | Animation |
|-------|-----------|
| Enemy hit (no mask) | Crash spins and falls backward |
| Bottomless pit | Falls with descending whistle |
| Water/drowning | Splash into water |
| Fire / hot pipe / steam | Burns to ashes |
| TNT / explosion | Blown apart (shoes and eyes float) |
| Electrocution | Skeleton visible through body |
| Crushing / boulder | Squashed flat |

### Lives
- **Starting lives**: 4.
- **Maximum lives**: 99.
- **Earning extra lives**: collecting 100 Wumpa Fruit, breaking Life Crates, or finding lives in bonus rounds.

### Respawn
- Crash respawns at the last activated checkpoint (or level start if none activated).
- All state since the last checkpoint is rolled back: Wumpa count, crate count, character token progress, and enemies/crates are restored.
- Aku Aku masks are lost — respawn with 0 masks.
- Crash reappears with a brief invulnerability window. Control is returned immediately (no respawn animation delay).

### Game Over
- When lives reach 0, a Game Over screen appears.
- Options: Continue (restart current level with starting lives) or Quit to title.
- **No dynamic difficulty adjustment**: the original game did not add extra checkpoints or masks after repeated deaths (that was added in the N. Sane Trilogy).

---

## 4. Camera System

The game uses a **rail camera** — the camera follows a predetermined track and the player cannot rotate it. There are four perspectives:

### Behind-the-Back (most common)
- Camera positioned behind and above Crash, following him along a corridor.
- Crash moves into the screen. Left/right on the stick moves laterally.
- Used in jungle, temple, factory, castle, and most other level types.

### Toward-the-Camera (chase levels)
- Camera faces Crash, positioned ahead of him. Crash runs toward the player.
- Used in boulder chase levels. Obstacles appear from behind Crash, giving minimal reaction time.
- Also used in segments of hog-riding levels.

### Side-Scrolling (2D)
- Traditional left-to-right profile view.
- Movement is primarily horizontal with vertical jumping.
- Used in some level segments and all bonus rounds.

### Overhead/Isometric
- Camera above at an angle, showing a top-down view.
- Used in Generator Room-style factory levels.

All perspectives are fixed per level segment. The camera switches automatically at predetermined trigger points. The player never controls the camera.

---

## 5. Crates

Crates are core to the game. Every level has a specific crate count; breaking all of them earns a gem (see Gems & Completion).

| Crate | Appearance | Break Method | Contents / Behavior |
|-------|-----------|--------------|---------------------|
| **Basic** | Plain wood | Spin, jump, or enemy impact | 1–2 Wumpa Fruit |
| **? Crate** | Wood with "?" | Spin, jump, or enemy impact | Variable: Wumpa Fruit (up to 10), character tokens, or extra life |
| **Aku Aku Crate** | Aku Aku face | Spin, jump, or enemy impact | 1 Aku Aku mask |
| **Life Crate** | Crash's face | Spin, jump, or enemy impact | 1 extra life |
| **Checkpoint** | "C" marking | Spin, jump, or enemy impact | Sets respawn point. Snapshots Wumpa count, crate count, and character token progress |
| **Bounce Crate** | Vertical stripes | Jump on it (up to 10 bounces) or spin (instant break) | 1 Wumpa per bounce (up to 10). Spin gives fewer fruit |
| **Arrow Crate** | Upward arrow | Jump on it or spin (wooden variant) | Strong upward launch. Wooden variant breaks after one bounce; iron variant is unbreakable |
| **TNT** | Red, "TNT" text | Jump on it: 3-second countdown then explodes. Spin: immediate detonation | Countdown displays 3, 2, 1 above crate (world-space). Explosion damages Crash within blast radius. Explosion can chain-detonate nearby TNT crates and break nearby wooden crates |
| **! Crate** | Metal, "!" | Any attack | Triggers environmental change: spawns platforms, opens paths, detonates remote crates. Some are consumed on hit, some can be re-activated |
| **Iron/Steel** | Solid metal | **Unbreakable** | Used as platforms and barriers. Does NOT count toward crate total |
| **Iron Arrow** | Metal + arrow | **Unbreakable** | Provides upward bounce but cannot be broken. Does NOT count |

**Important**: Nitro crates do NOT exist in Crash 1 (introduced in Crash 2).

---

## 6. Collectibles

### Wumpa Fruit
- Primary collectible. Small mango/apple-like fruit.
- **100 Wumpa = 1 extra life**. Counter resets to 0 after reaching 100.
- Found loose in levels and inside crates.
- Fruit placed in the world have no pickup magnetism — Crash must physically collide with them. Fruit that pop out of broken crates scatter briefly and can be walked through to collect.
- **On death**: reverts to checkpoint value (see Damage, Death & Lives).
- **Between levels**: counter resets to 0. Fruit do not carry over.

### Aku Aku Mask
Protective power-up with 3 tiers:
- **1 mask**: absorbs 1 hit, then disappears. Mask floats beside Crash.
- **2 masks**: absorbs 2 hits total. Mask appears golden.
- **3 masks**: temporary invincibility for ~20 seconds. Aku Aku attaches to Crash's face. Fast-paced music plays. Crash defeats enemies on contact. Timer expires → back to 0 masks. Invincibility does NOT protect against instant-death hazards (pits, water, crushing).
- **Between levels**: masks carry over (1 or 2 masks persist). Invincibility does not carry over.
- **On death**: all masks lost. Respawn with 0 masks.
- For damage response details (knockback, invulnerability window, blinking), see Damage, Death & Lives.

### Character Tokens (Tawna, Brio, Cortex)
- Found inside certain ? crates.
- Collecting 3 of the same type in a single level opens that character's bonus round.
- Lost on death before collecting all 3.
- See Bonus Rounds & Keys for details.

---

## 7. Enemies

### General Rules
- Most enemies can be defeated by **either** spin or jump.
- Enemies with **spikes or defenses on top** cannot be jumped on — must be spun.
- Enemies with **frontal shields** cannot be spun from the front — must be jumped on or attacked from behind.
- **Invulnerable hazards** (fire jets, rolling stones, retracting walls) cannot be defeated and must be avoided.
- Enemies defeated by spin are **launched off-screen** and can hit/defeat other enemies in their path.
- Jumping on an enemy gives Crash a small upward bounce (like a stomp). The bounce height is roughly half a normal jump — enough to chain into another jump or reach a slightly higher platform, but not a full jump.
- Enemies respawn when Crash dies and returns to a checkpoint. Enemies always respawn when re-entering a level.
- Defeated enemies play a brief defeat animation/effect (spin away, poof, etc.) before disappearing.

### Wildlife/Jungle Enemies (Islands 1–2)

**Crab**
- **Defeat**: spin or jump.
- **Patrol**: walks sideways back and forth along a short fixed path on flat ground (beaches). Reverses direction at each end of its patrol range, never walks off ledges.
- **Awareness**: none. Does not react to Crash. Pure timing obstacle.
- **Speed**: slow, steady pace. Easily predictable.

**Skunk**
- **Defeat**: spin or jump.
- **Patrol**: walks back and forth on a fixed path. Reverses at patrol endpoints. Stays on its platform — stops at ledge edges and turns around.
- **Awareness**: none. Does not react to Crash in any way.
- **Speed**: moderate walking pace, slightly faster than the crab.

**Venus Fly Trap**
- **Defeat**: spin (always), jump (only on variants without teeth/spikes on top).
- **Stationary**: does not move from its rooted position.
- **Cycle**: alternates between two states on a fixed timer:
  - **Open/resting** (~2–3s): mouth open and idle, vulnerable to attack.
  - **Snapping** (~1s): mouth snaps shut rapidly. Contact during this phase damages Crash.
- **Awareness**: none. Snaps on a fixed timer regardless of Crash's position.
- **Variants**: green (jumpable, no top spikes) and white/spiky (spin only, spikes on top hurt Crash if he lands on it).

**Turtle**
- **Defeat**: spin (launches it away like other enemies) or jump (flips it upside-down instead of killing it).
- **Patrol**: walks back and forth on a fixed path at slow speed. Reverses at patrol endpoints. Stays on its platform.
- **Flip behavior**: when jumped on, flips upside-down and becomes a stationary platform Crash can stand on. While flipped, the turtle slides slightly in the direction it was hit. After ~5 seconds, it self-rights (flips back over) and resumes patrolling. Crash is knocked off if he's standing on it when it self-rights.
- **Awareness**: none. Pure patrol enemy.
- **Bridge usage**: on bridge levels, turtles patrol across planks and serve as critical mobile platforms for crossing wide gaps. Jump on one to flip it, ride it briefly, then jump to the next surface.

**Rolling Monkey**
- **Defeat**: spin or jump, but only during the brief pause between rolls.
- **Cycle**: alternates between two states:
  - **Rolling** (~3–4s): curls into a ball and rolls forward at high speed along its path. Completely invulnerable during this phase — spin and jump both fail, and contact damages Crash.
  - **Paused** (~1–2s): uncurls and stands still, catching its breath. Vulnerable to attack during this window.
- **Patrol**: rolls in one direction, pauses, then rolls back. Fixed path, reverses at endpoints.
- **Awareness**: none. Does not react to Crash.

**Snake**
- **Defeat**: jump (timing required). Spinning is risky — the snake's low profile and lunge can hit Crash before the spin connects.
- **Stationary**: lives in a hole in the ground. Never leaves the hole.
- **Cycle**: repeats on a fixed timer:
  - **Hidden** (~2–3s): fully underground, invisible, harmless.
  - **Emerge + bob** (~2s): rises out of the hole and sways side to side. Vulnerable to a well-timed jump during this phase, but contact with its body damages Crash.
  - **Lunge** (~0.5s): strikes forward rapidly in one direction. Long reach, fast. Damages Crash on contact.
  - Returns to hidden state.
- **Awareness**: none. Cycle is purely timer-based.

**Bat**
- **Defeat**: spin or jump.
- **Movement**: flies along a preset path (spline) in a loop. Some bats hang from ceilings and swoop down in an arc when Crash approaches a trigger zone, then return to their perch.
- **Swooping variant awareness**: triggered by proximity — when Crash enters a zone below the bat, it swoops. The swoop follows a fixed arc (not homing). If Crash passes through without being hit, the bat returns to its perch and can swoop again.
- **Flying variant awareness**: none. Follows its loop regardless of Crash.

**Spider**
- **Defeat**: spin or jump.
- **Variants**:
  - **Ceiling dropper**: hangs from a web on the ceiling. When Crash walks beneath it (proximity trigger), drops straight down. After landing, sits briefly (~1s), then climbs back up on its web. Drops again if Crash is still below.
  - **Ground jumper**: sits on the ground, periodically jumps upward in place on a fixed timer. Vulnerable when grounded, dangerous when airborne (landing on a jumping spider damages Crash).
- **Speed**: dropping is fast (near free-fall). Climbing back up is slow.

**Flying Fish**
- **Not defeatable**. Environmental hazard only.
- **Behavior**: leaps from water in a fixed arc pattern on a repeating timer. Always follows the same arc — same height, same distance, same timing. Multiple fish are often staggered to create a rhythmic obstacle pattern.
- **Damage**: contact with a fish damages Crash. The arcs cross over platforms that Crash must traverse, so the challenge is timing movement between fish jumps.

**Lizard (Green)**
- **Defeat**: spin or jump.
- **Behavior**: sits in place, then jumps in a low arc on a fixed timer. The jump covers a short horizontal distance. Lands and sits again. Repeats.
- **Awareness**: none. Fixed timer, fixed arc.
- **Danger zone**: Crash takes damage from contact during the lizard's jump arc, but can safely jump on it from above.

**Lizard (Red)**
- **Defeat**: spin or jump.
- **Behavior**: sits in place, then jumps in a high arc on a fixed timer. Higher and longer arc than the green lizard.
- **Bounce platform**: when Crash jumps on a red lizard mid-air (during its high arc), Crash gets an extra-high bounce — higher than a normal enemy stomp. This is used as a level design tool to reach otherwise inaccessible platforms.
- **Awareness**: none. Fixed timer, fixed arc.

### Tribal/Native Enemies (Islands 1–2)

**Basic Tribesman**
- **Defeat**: spin or jump.
- **Patrol**: walks back and forth on a fixed path carrying a club. Reverses at patrol endpoints. Stays on its platform — stops at ledges.
- **Awareness**: none. Does not react to Crash, does not chase, does not speed up.
- **Attack**: no active attack animation — simply damages Crash on contact. The club is part of his collision, giving him a slightly wider hitbox than his body.
- **Speed**: moderate walking pace.

**Shield Native**
- **Defeat**: jump on top (always works), or spin from behind (shield only blocks frontal spin attacks).
- **Patrol**: walks back and forth on a fixed path, always facing the direction of movement. Reverses at patrol endpoints.
- **Shield mechanic**: holds a large wooden shield in front of his body. If Crash spins into the shield (from the front or sides), the spin is blocked — Crash bounces back and takes no damage, but the enemy is not defeated. The shield does not break.
- **Awareness**: none. Does not turn to face Crash, does not track the player. This means if Crash gets behind the shield native (by jumping over it), the shield is facing away and a spin from behind connects normally.
- **Speed**: slow patrol.

**Spear Thrower**
- **Defeat**: spin or jump (must close the distance first).
- **Stationary**: stands in a fixed position, does not patrol.
- **Attack**: throws spears at regular intervals (~2–3s between throws). Spears travel in a straight line at moderate speed in the direction the thrower is facing. Spears are a fixed trajectory — not homing, not aimed at Crash dynamically.
- **Spear properties**: each spear is a projectile that damages Crash on contact. Spears cannot be spun away or deflected. They disappear after traveling a fixed distance or hitting geometry.
- **Facing**: always faces one fixed direction (set by level placement). Does not turn to track Crash.
- **Vulnerability**: can be defeated when Crash reaches melee range by spin or jump. Between throws, the thrower is idle and vulnerable.

### Industrial/Lab Enemies (Island 3)

**Beaker-throwing Lab Assistant**
- **Defeat**: spin or jump (must close the distance first).
- **Stationary**: stands in a fixed position behind a table or on a platform.
- **Attack**: throws chemical beakers at regular intervals (~2–3s). Two beaker types:
  - **Red beaker**: explodes on contact with the ground, creating a small damaging area-of-effect. The explosion lingers briefly (~0.5s). Avoid the landing zone.
  - **Green beaker**: shatters on contact with the ground and spawns a small green blob creature that bounces around the area. The blob damages Crash on contact and bounces in a fixed pattern for a few seconds before disappearing.
- **Beaker trajectory**: thrown in a lobbing arc toward Crash's general direction (aimed at where Crash is at the moment of the throw, not predictive). Fixed arc speed.
- **Vulnerability**: between throws, the lab assistant is idle. Close in and spin or jump on him.

**Electric Lab Assistant**
- **Defeat**: spin only (electricity on top prevents jumping on him).
- **Patrol**: walks back and forth on a platform, similar to basic patrol enemies.
- **Electrical barrier**: projects a horizontal electrical field from his body. The barrier extends outward from him, creating a damaging zone around his upper body and head. Contact with the electricity damages Crash.
- **Awareness**: some variants become aware of Crash when he's nearby. When aware, the lab assistant walks toward Crash at a slow, deliberate pace (not a chase — a slow advance). If Crash moves away, the assistant stops advancing and returns to patrol.
- **Spin strategy**: Crash must spin into the assistant's lower body (below the electrical field) to knock him off the platform. Timing is important — the spin must connect before the assistant closes distance.

**Bouncing Barrels**
- **Not defeatable**. Invulnerable environmental hazard.
- **Behavior**: bounce up and down in a fixed pattern at set positions. Each barrel has a fixed landing spot (sometimes indicated by floor markings). The bounce follows a fixed rhythm — same height, same timing, every cycle.
- **Damage**: contact with a barrel (while it's airborne or landing) damages Crash.
- **Avoidance**: run or walk between bounces. Multiple barrels are often staggered to create lanes of safe passage that require timing.

---

## 8. Boss Fights

Six bosses, one after each set of levels. All boss arenas are enclosed — no pits or environmental death.

### Boss 1: Papu Papu
- **HP**: 5
- **Arena**: His hut, Crash can stand on the throne.
- **Pattern**: swings his club in a wide horizontal arc. After completing the swing, raises it to slam down — that's the opening.
- **Damage**: jump on his head during the opening.
- **Escalation**: swings get faster after each hit.

### Boss 2: Ripper Roo
- **HP**: 3
- **Arena**: 3×3 grid of stone platforms over water. TNT crates float down a waterfall.
- **Pattern**: bounces platform to platform in a set pattern. Pattern changes after each hit.
- **Damage**: jump on a TNT crate to start its timer, position so the explosion hits Ripper Roo when he lands on an adjacent platform. Cannot attack him directly.

### Boss 3: Koala Kong
- **HP**: 4
- **Arena**: cavern with lava river between Crash and Koala Kong. Mine cart track runs between them.
- **Pattern**: throws rocks at Crash (small = dodge, large = reflect). After throwing, he flexes/poses (opening).
- **Damage**: spin a large boulder back at him while he's posing. Mine carts passing on the track can block the returned boulder.
- **Escalation**: TNT crates fall from ceiling in later phases.

### Boss 4: Pinstripe Potoroo
- **HP**: 6
- **Arena**: office boardroom with desk and furniture.
- **Pattern**: fires Tommy gun in sweeping arcs from behind desk or corners. Periodically repositions. Gun occasionally jams.
- **Damage**: take cover behind furniture (indestructible), rush in and spin when gun jams or during repositioning.
- **Escalation**: final phase — jumps onto desk, gun jams one last time.

### Boss 5: Dr. Nitrus Brio
- **HP**: 9 total (two phases)
- **Phase 1**: throws beakers. Red = explosive (dodge), green = spawns bouncing green blobs. Jump on blobs to damage Brio (blob juice hits him). Multiple blob hits required.
- **Phase 2**: drinks potion, transforms into large monster. Smashes floor, causing rubble to fall from ceiling. Rubble becomes temporary stepping-stone platforms. Jump on rubble, then jump on Brio's head. 3 head-jumps to finish.

### Boss 6: Dr. Neo Cortex (Final Boss)
- **HP**: 5
- **Arena**: top of Cortex's airship, open sky.
- **Pattern**: fires colored energy blasts from ray gun:
  - **Purple blasts**: homing, must dodge (cannot reflect).
  - **Blue blasts**: straight/wavy lines, must dodge (cannot reflect).
  - **Green blasts**: can be **reflected back** by spinning into them. Each reflected green blast = 1 hit.
- **Escalation**: patterns get faster, more purple/blue mixed in. Final phase: only green blasts.

---

## 9. Hazards & Obstacles

### Damaging (absorbed by Aku Aku)
- Enemy contact damage
- TNT crate explosions
- Fire / flame jets (stationary or timed)
- Hot pipes (red = lethal, blue = safe in factory levels)
- Steam vents (periodic eruption)
- Electrical barriers / electric fences
- Rolling stone cylinders (timing obstacles)
- Pop-up spikes (on timers)
- Retracting walls / spiked pillars
- Chemical beakers (from enemies)
- Swinging pendulum blades (castle/dark levels)

### Environmental Obstacles (non-damaging)
- Moving platforms (timed riding)
- Falling/crumbling platforms (collapse shortly after being stepped on)
- Conveyor belts (push Crash in a direction)
- Bouncy pads (launch upward)
- Bridge ropes (walkable but very narrow and slippery)
- Rotating platforms

Note: instant-death hazards (pits, water, crushing, boulders) are listed in Damage, Death & Lives.

---

## 10. Bonus Rounds & Keys

Accessed by collecting 3 matching character tokens from ? crates within a single level.

### Tawna Bonus Rounds
- **Difficulty**: easy.
- **Content**: treetop platforming, crates with Wumpa Fruit and extra lives.
- **Completion reward**: save opportunity (original game only — see Save System).
- **One-time**: once completed in a level, tokens no longer appear.

### N. Brio Bonus Rounds
- **Difficulty**: medium–hard. Precise jumps, TNT crates.
- **Content**: cave/underground setting. Contains 6 life crates each.
- **Completion reward**: items collected during the round.

### Cortex Bonus Rounds
- **Difficulty**: hardest. Only exist in 2 levels (Sunset Vista, Jaws of Darkness).
- **Completion reward**: a **Key** + save opportunity (original game only).

### Keys
Two keys in the game, each earned from a Cortex bonus round:

| Key | Source | Unlocks |
|-----|--------|---------|
| Key 1 | Cortex Bonus Round in Sunset Vista | Whole Hog (secret hog-riding level) |
| Key 2 | Cortex Bonus Round in Jaws of Darkness | Fumbling in the Dark (secret dark level) |

### Bonus Round Rules
- **Camera**: all bonus rounds use a side-scrolling (2D) perspective.
- **Entry**: when 3 matching tokens are collected, a warp portal appears, teleporting Crash into the bonus round. On completion (reaching the end) or death (falling into a pit), Crash is returned to the exact point in the main level where the portal appeared.
- **Death in bonus rounds does NOT cost a life.** Crash is sent back to the main level and must re-collect tokens to retry.
- **Bonus round deaths do NOT count** toward the "no death" requirement for colored gems.
- **Bonus round crates do NOT count** toward the level's crate total for gem purposes.

---

## 11. Gems & Completion

### Clear Gems (20 total)
- Earned by breaking **all** crates in a level (excluding bonus round crates and iron/steel crates).
- No death requirement — you can die and still earn the clear gem if all crates are broken.
- Some levels have crates behind colored gem paths, making their clear gem impossible until the required colored gem is found.

### Colored Gems (6 total)
All require: **break all crates AND complete the level without dying**.

| Color | Level | Notes |
|-------|-------|-------|
| Green | The Lost City | Easiest colored gem |
| Orange | Generator Room | |
| Blue | Toxic Waste | Very difficult |
| Red | Slippery Climb | Very difficult |
| Yellow | The Lab | Very difficult |
| Purple | Lights Out | Requires Yellow gem first for full crate access |

### Colored Gem Path Dependencies
Certain levels have platforms that only materialize when the corresponding colored gem is owned:

| Gem Needed | Levels It Unlocks Paths In |
|------------|---------------------------|
| Green | Jungle Rollers, Castle Machinery |
| Blue | Rolling Stones, Cortex Power, Jaws of Darkness |
| Red | Native Fortress, Road to Nowhere |
| Orange | Upstream |
| Yellow | The Great Gate, Lights Out |
| Purple | Boulder Dash |

### Level-End Flow
- Each level ends with a **goal** — typically a stone platform or portal that Crash steps onto.
- A **summary screen** shows crates broken vs. total. If all crates are broken (and no-death requirement met for colored gem levels), a gem is awarded.
- After the summary, Crash returns to the overworld map and the next level node is unlocked.
- **Replaying a completed level**: the player can revisit any level. Gems already earned remain earned. If the player didn't get the gem previously, they can earn it on replay.

### 100% Completion
Collecting all 26 gems unlocks an alternate ending accessible through The Great Hall.

---

## 12. Level Types

### Jungle Corridor (behind-the-back)
Standard 3D corridor through jungle. Linear path with enemies, crates, and gaps.
- Levels: N. Sanity Beach, Jungle Rollers, Rolling Stones

### Temple/Ruins Vertical Climb
Significant vertical platforming — climbing upward through temple structures with retracting walls, timed obstacles.
- Levels: The Great Gate, Native Fortress, The Lost City, Temple Ruins, Sunset Vista, Slippery Climb

### Boulder Chase (toward-camera)
Crash runs toward the camera fleeing a giant boulder. Obstacles appear from behind with minimal reaction time. Requires memorization. The boulder is indestructible — contact is instant death.
- Levels: Boulders, Boulder Dash

### Bridge Levels
Long wooden bridges over chasms with multiple plank types:
- **Light brown planks**: safe, solid.
- **Dark planks**: collapse after a moment of standing on them.
- **Missing planks**: gaps to jump over.
- Bridge ropes on sides can be walked on (very narrow, slippery) as expert shortcuts.
- Turtles serve as mobile platforms — jump to flip, then use as stepping stone (self-rights after a few seconds).
- Levels: Road to Nowhere, The High Road

### Hog Riding
Crash rides a wild boar that accelerates automatically. Cannot stop or slow down.
- Controls: left/right to steer, jump button to jump. Cannot spin while riding.
- Toward-camera perspective in some segments.
- Levels: Hog Wild, Whole Hog (secret)

### Dark/Firefly Levels
Levels are nearly pitch-black. Aku Aku masks provide temporary illumination for a few seconds when collected. Taking a hit removes light AND protection.
- Must navigate in darkness between mask pickups.
- Levels: Lights Out, Fumbling in the Dark (secret), Jaws of Darkness

### River/Water Levels
Feature water with leaf/lily pad platforms that slowly sink when stood on.
- Fish enemies jump from water in timed arcs.
- Blue plant platforms bounce Crash to different heights.
- Water = instant death.
- Levels: Upstream, Up the Creek

### Factory/Industrial
Mechanical hazards: hot pipes (red = lethal, blue = safe), steam vents, conveyor belts, electric fences, bouncy pads.
- Some segments use overhead/isometric camera (Generator Room).
- Lab assistant enemies.
- Levels: Heavy Machinery, Cortex Power, Generator Room, Toxic Waste

### Castle/Laboratory
Mechanical castle environments with gears, trap doors, electrical hazards.
- ! crates are critical — activating them opens doors, closes trap doors, changes the environment.
- Levels: Castle Machinery, The Lab, The Great Hall

---

## 13. Complete Level List

### Island 1 — N. Sanity Island
1. N. Sanity Beach (Jungle Corridor) — tutorial level
2. Jungle Rollers (Jungle Corridor) — Green gem path
3. The Great Gate (Temple Climb) — Yellow gem path
4. Boulders (Boulder Chase)
5. Upstream (River) — Orange gem path
- **Boss: Papu Papu**

### Island 2
6. Rolling Stones (Jungle Corridor) — Blue gem path
7. Hog Wild (Hog Riding)
8. Native Fortress (Temple Climb) — Red gem path
9. Up the Creek (River)
- **Boss: Ripper Roo**
10. The Lost City (Temple Climb) — **Green gem**
11. Temple Ruins (Temple/Ruin)
12. Road to Nowhere (Bridge) — Red gem path
13. Boulder Dash (Boulder Chase) — Purple gem path
14. Sunset Vista (Temple Climb) — Cortex bonus → Key 1
- **Boss: Koala Kong**

### Island 3
15. Heavy Machinery (Factory)
16. Cortex Power (Factory) — Blue gem path
17. Generator Room (Factory/Isometric) — **Orange gem**
18. Toxic Waste (Factory) — **Blue gem**
- **Boss: Pinstripe Potoroo**
19. The High Road (Bridge)
20. Slippery Climb (Castle Climb) — **Red gem**
21. Lights Out (Dark) — **Purple gem**, Yellow gem path
22. Jaws of Darkness (Dark) — Blue gem path, Cortex bonus → Key 2
23. Castle Machinery (Castle/Factory) — Green gem path
- **Boss: Dr. Nitrus Brio**
24. The Lab (Castle/Lab) — **Yellow gem**
25. The Great Hall (Castle Corridor) — alternate ending with 20+ gems
- **Boss: Dr. Neo Cortex** (Final)

### Secret Levels
- Whole Hog (Hog Riding) — unlocked by Key 1
- Fumbling in the Dark (Dark) — unlocked by Key 2

---

## 14. Game Flow & Overworld

### Startup
1. **Studio logo / splash screen** (skippable).
2. **Title screen**: game logo with "Press Start" prompt.
3. **Main menu**: New Game, Continue (load save), Options.
4. **Save slot select**: multiple save slots (3 is standard). Each slot shows completion progress (gems collected) and island reached.

### Overworld Map
- **3 islands** connected linearly. Each island contains a set of levels.
- The map is a 3D scene — Crash walks along a path between level nodes. Pressing the action button on a node enters that level.
- Levels are nodes on the path. Progression is **strictly linear** — must complete in order.
- Defeating a boss unlocks the next set of levels. Island 2 is split by two bosses (Ripper Roo gates levels 10–14, Koala Kong gates Island 3).
- Completed levels show a visual indicator. Levels with earned gems show the gem on the node.
- Player can move backward to replay any completed level (for missed gems, farming lives). All crates, enemies, and tokens reset fresh for each attempt.
- Secret levels appear as branching paths off the main path once unlocked with keys.
- Boss nodes appear between level groups. They do not have crate counts or gems.

### Core Loop
Overworld map → select level → loading transition → gameplay → reach goal → summary screen → return to map → next node unlocked.

### Bonus Round Flow
Collect 3 matching tokens → portal appears → teleport into bonus round → play through → on completion or death → return to main level at the portal point.

### Death & Game Over
Death in level → death animation → brief pause → respawn at checkpoint. Lives counter decrements. At 0 lives → Game Over screen → Continue (restart level with starting lives) or Quit to title.

---

## 15. HUD & UI

### Gameplay HUD
The HUD is minimal — clean screen with only essential information:
- **Lives counter** (bottom-left): Crash's face icon + life count number (e.g., "x 4").
- **Wumpa Fruit counter** (bottom-right): Wumpa Fruit icon + current count (0–99).
- **No Aku Aku indicator on HUD** — mask state is communicated entirely through the visual on Crash (floating mask beside player, golden mask for 2 hits, mask attached to face during invincibility).
- **No health bar, no minimap, no timer.**

### Contextual Elements
- **TNT countdown**: 3, 2, 1 displayed above the crate in world-space.
- **Token display**: brief popup when a character token is collected (e.g., 1/3, 2/3).
- **Boss HP**: no explicit health bar in the original. Player infers progress from behavior changes. For our recreation, we may add a subtle indicator — design decision for implementation.

### Level-End Summary Screen
- Full-screen overlay after reaching the level goal.
- Displays: crate icon with "X / Y" (broken / total).
- If gem earned: gem awarded with fanfare animation.
- "Continue" prompt returns to overworld map.

### Pause Menu
- Triggered by Start/Escape. Game world freezes — no enemies move, no timers tick.
- Options: Resume, Restart Level (costs a life), Quit to Map.

---

## 16. Controls

### Gamepad (Xbox layout)
| Action | Button |
|--------|--------|
| Move | Left Stick |
| Jump | A (hold for higher jump) |
| Spin Attack | X or B |
| Pause | Start / Menu |
| Navigate menus | D-pad or Left Stick |
| Confirm (menus) | A |
| Back (menus) | B |

### Keyboard
| Action | Key |
|--------|-----|
| Move | WASD or Arrow Keys |
| Jump | Space (hold for higher jump) |
| Spin Attack | Left Click, J, or Left Shift |
| Pause | Escape |
| Navigate menus | WASD or Arrow Keys |
| Confirm (menus) | Space or Enter |
| Back (menus) | Escape |

### Notes
- The game should support both input methods simultaneously and switch HUD button prompts based on the last input device used.

---

## 17. Save System

For our recreation, we'll modernize saving. The original required completing bonus rounds or earning gems to save; we'll use save-at-will instead.

**Saved data**: level completion, gems collected, keys obtained, lives count.

---

## 18. What We Are NOT Building

Features from sequels or remakes that are **excluded**:

- Double jump (Crash 2)
- Body slam / belly flop (Crash 2)
- Slide move (Crash 2)
- Crouch (Crash 2)
- Nitro crates (Crash 2)
- Nitro detonator crates (Crash 2)
- Time trial / relics (Crash 3, retroactively added in N. Sane Trilogy)
- Dynamic difficulty adjustment / extra checkpoints after deaths (N. Sane Trilogy addition)
- Slot crates (Crash 3)
- Warp rooms (Crash 2+)
- Percentage completion counter (N. Sane Trilogy)

---