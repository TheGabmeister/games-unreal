# PHASES.md — Project Phases

Each phase builds on the previous and ends with a playable milestone. Phases are ordered by dependency — later phases require earlier ones to be complete. No code or implementation details here; those belong in separate documents per phase. For gameplay rules, values, and behaviors, see SPEC.md.

A persistent debug level at `Content/Maps/Debug/` grows with each phase. It is a single open map divided into labeled zones — each phase adds a zone for the mechanics it introduces. It is the primary way to validate mechanics during development and is never shipped.

---

## Key Decisions
- Implement almost everything in C++, using a thin layer of Blueprints only exposing parameters and referencing assets. The exception are UI widgets. 

---

## Phase 0: Rename Project ✓

**Status**: Complete (C++ rename done, Blueprints pending recreation).

All C++ code now uses the `CB` prefix. Project file is `CB.uproject`. Modules are `CB` and `CBEditor`. Blueprint assets were deleted and need to be recreated with the new parent classes (no project prefix in BP names: `BP_GameMode`, `BP_PlayerCharacter`, etc.).

**Remaining**:
- Recreate Blueprint assets (thin wrappers reparenting to CB C++ classes)
- Recreate level maps
- Reconfigure DeveloperSettings in editor
- Verify full build + editor + PIE

See `PLAN_00.md` for full details.

---

## Phase 1: Core Player

**Goal**: Crash can move, jump, spin, take damage, and die in a test level with a working camera.

**Deliver**:
- Player movement and jump mechanics (SPEC §1)
- Spin attack (SPEC §2)
- Damage response: knockback and death (SPEC §3 — Taking Damage, Instant Death). No lives counter yet — infinite respawns for testing. Lives system comes in Phase 2
- Rail camera: behind-the-back perspective only (SPEC §4 — other perspectives in Phase 7)
- Input mapping for gamepad and keyboard (SPEC §16)
- **Audio**: jump sound, landing sound, spin whoosh, damage hit, death jingle
- **Debug level**: build the initial map with a **Movement Playground** zone — flat ground, gaps, platforms at varying heights, pit and water kill volumes, walls

**Playtest**: run around the debug level, jump across gaps, spin, fall into pits, die and respawn. Verify all actions have audio feedback.

---

## Phase 2: Crates & Collectibles

**Goal**: Levels have breakable crates, collectible fruit, mask protection, and a working lives system. Death reloads the level (no checkpoint yet — that's Phase 5.5).

**Deliver**:
- Base crate actor and all crate variants except ! Crate and Checkpoint (SPEC §5)
- Wumpa Fruit pickup and counter (SPEC §6 — Wumpa Fruit)
- Aku Aku mask with 3-tier protection and invincibility (SPEC §6 — Aku Aku Mask)
- Post-hit invulnerability with blinking (SPEC §3 — Taking Damage)
- Lives system with earning, losing, and Game Over (SPEC §3 — Lives, Game Over)
- **Audio**: crate break, Wumpa collect, extra life earned, Aku Aku voice on mask pickup, invincibility music, TNT countdown tick, TNT explosion, Game Over jingle
- **Debug level**: add a **Crate Yard** zone — one of every crate type, Wumpa Fruit, grouped Aku Aku crates for mask-tier testing

**Playtest**: break all crate types, collect fruit to earn a life, pick up masks, take damage and lose a mask, die and restart level, reach 0 lives and see Game Over.

---

## Phase 3: Enemies

**Goal**: Levels populated with enemies that have distinct behaviors. Combat feels right.

**Deliver**:
- Core enemy combat: stomp bounce, spin-launch with chain-kill, defeat effects, respawn rules (SPEC §7 — General Rules)
- All enemy types with their AI behaviors (SPEC §7):
  - Patrol enemies: Crab, Skunk, Basic Tribesman, Turtle
  - Stationary cycling: Venus Fly Trap, Snake, Spear Thrower
  - Movement cycling: Rolling Monkey, Bat, Spider, Green Lizard, Red Lizard
  - Defensive: Shield Native, Electric Lab Assistant
  - Ranged: Beaker Lab Assistant
  - Invulnerable hazards: Flying Fish, Bouncing Barrels
- **Audio**: enemy defeat sound, stomp bounce sound, shield block sound, projectile throw/impact sounds, enemy-specific ambient sounds (e.g., snake hiss, bat screech)
- **Debug level**: add an **Enemy Gauntlet** zone — one of every enemy type in sequence along a corridor, spaced for isolated testing

**Playtest**: encounter each enemy type. Verify spin vs jump interactions, shield blocking, turtle flipping, cycling enemy vulnerability windows, projectile dodging.

---

## Phase 4: Hazards & Obstacles

**Goal**: Levels have environmental dangers and platforming challenges beyond enemies and crates.

**Deliver**:
- All damaging hazards and environmental obstacles (SPEC §9)
- Crush death zones (SPEC §3 — Instant Death)
- **Audio**: hazard-specific sounds (fire whoosh, steam burst, electric buzz, blade swing, spike pop), moving platform rumble, crumbling platform crack
- **Debug level**: add a **Hazard Course** zone — fire jets, hot pipes, steam vents, spikes, pendulum blades, moving platforms, crumbling platforms, conveyor belt, crush zone

**Playtest**: navigate the hazard course. Verify hazards are absorbed by Aku Aku, crush kills through masks, platforms behave correctly.

---

## Phase 5: HUD & Level Flow

**Goal**: Play a complete level from start to finish with full UI feedback.

**Deliver**:
- Gameplay HUD (SPEC §15 — Gameplay HUD)
- Level completion trigger and summary screen (SPEC §11 — Level-End Flow, SPEC §15 — Level-End Summary Screen)
- Pause menu (SPEC §15 — Pause Menu)
- Token collection popup UI widget (SPEC §15 — Contextual Elements). Tokens themselves are not functional until Phase 8 — this is just the display
- Game Over screen integration with level flow
- **Audio**: UI sounds for menu navigation, pause open/close, summary screen gem fanfare, level complete jingle
- **Debug level**: add a **HUD Test** zone — Life Crates, fruit piles, token pickups, and a goal platform

**Playtest**: play start to finish — HUD updates correctly, pause works, summary screen shows correct crate count.

---

## Phase 5.5: Checkpoints & Respawn

**Goal**: Death no longer restarts the entire level. Players respawn at the last checkpoint with world state partially rolled back.

**Deliver**:
- Checkpoint crate (SPEC §5 — Checkpoint) — breaks to set respawn point and snapshot state
- Crate count tracking — total count at level start, broken count during gameplay, rollback on checkpoint
- Stable ID system (`FGuid`) on crates and pickups for cross-reload identification
- `FCheckpointData` on GameInstance — persists respawn position, Wumpa count, broken crate IDs, collected pickup IDs, broken crate count
- Post-load state application — `InitializeAsBroken()` / `InitializeAsCollected()` silent pre-break/pre-collect after level reload
- Death → fade-to-black → level reload → apply checkpoint → spawn at checkpoint position
- Streaming level support — explicitly load sub-level containing checkpoint before spawning player
- Editor validation utility for duplicate StableIDs
- **Audio**: checkpoint activated chime
- **Debug level**: add a **Checkpoint Test** area — checkpoint crate → hazard → more crates → kill volume (validates full rollback loop)

**Playtest**: break crates, hit checkpoint, die, verify level reloads with correct state (crates before checkpoint stay broken, crates after checkpoint are fresh, Wumpa count restored). Test with no checkpoint (full restart). Test Game Over → Continue clears checkpoint.

---

## Phase 6: Overworld & Progression

**Goal**: Navigate between multiple levels on a map with persistent progress, gems, and saving.

**Deliver**:
- Overworld map with 3 islands and level nodes (SPEC §14 — Overworld Map)
- Linear progression with boss gating (SPEC §14)
- Level replay with full reset (SPEC §14)
- Clear gem and colored gem systems (SPEC §11 — Clear Gems, Colored Gems)
- Colored gem path platforms (SPEC §11 — Colored Gem Path Dependencies)
- Save/load system with save slots (SPEC §17)
- **Audio**: overworld map ambient music, level-to-map transition sounds, gem path reveal sound
- **Debug level**: add a **Gem Test** zone — enclosed area with known crate count, a colored gem path platform, and a goal

**Playtest**: complete levels, navigate the map, earn gems, verify save/load persistence, test colored gem path materialization.

---

## Phase 7: Special Level Types

**Goal**: Each specialized level type is playable with its unique mechanics.

**Deliver**:
- Additional camera perspectives: toward-camera, side-scrolling, overhead/isometric (SPEC §4)
- Camera perspective trigger volumes for mid-level switching
- Boulder chase levels (SPEC §12 — Boulder Chase)
- Hog riding levels (SPEC §12 — Hog Riding)
- Dark/firefly levels (SPEC §12 — Dark/Firefly Levels)
- Bridge levels (SPEC §12 — Bridge Levels)
- River/water levels (SPEC §12 — River/Water Levels)
- **Audio**: boulder rumble, hog gallop/grunt, darkness ambience, bridge plank creak/snap, water splash (death), per-level-type background music
- **Debug level**: add a **Gimmick Lanes** zone — side-by-side test lanes for each level type

**Playtest**: play each gimmick lane. Flee a boulder, ride a hog, navigate darkness with mask illumination, cross a bridge with crumbling planks.

---

## Phase 8: Bonus Rounds & Keys

**Goal**: Full bonus round system with keys unlocking secret levels.

**Deliver**:
- Character token collection and tracking (SPEC §6 — Character Tokens)
- Bonus round entry, exit, and rules (SPEC §10 — Bonus Round Rules)
- 3 bonus round types: Tawna, Brio, Cortex (SPEC §10)
- Keys and secret level unlocks on overworld map (SPEC §10 — Keys)
- **Audio**: token collect sound, bonus round portal open, bonus round ambient music, key earned fanfare
- **Debug level**: add a **Bonus Portal** zone — three token sets that each open a minimal side-scrolling bonus round

**Playtest**: collect tokens, enter and complete a bonus round, verify no-life-cost on death, earn a key, see secret level unlocked on map.

---

## Phase 9: Boss Fights

**Goal**: All 6 bosses are playable with unique arenas and mechanics.

**Deliver**:
- Boss arena framework: enclosed arena, HP tracking, escalation
- All 6 bosses: Papu Papu, Ripper Roo, Koala Kong, Pinstripe Potoroo, Dr. Nitrus Brio, Dr. Neo Cortex (SPEC §8)
- Boss nodes on overworld map gate progression
- **Audio**: boss arena music (unique per boss), boss hit/damage sound, boss defeat fanfare, boss-specific attack sounds (club swing, gunfire, beaker shatter, energy blasts, etc.)
- **Debug level**: add a **Boss Arena** zone — enclosed room with a placeholder boss to validate the shared HP/escalation framework

**Playtest**: fight placeholder boss in debug level, then each real boss. Verify defeating a boss unlocks the next level set.

---

## Phase 10: Level Construction

**Goal**: All 25 game levels, 6 boss arenas, and 2 secret levels are built and playable.

This is the content phase — all gameplay systems are complete by Phase 9, so this phase uses them to build the actual game. Work can be parallelized across levels.

**Deliver**:
- All levels organized by island (SPEC §13 — Complete Level List):
  - Island 1: N. Sanity Beach, Jungle Rollers, The Great Gate, Boulders, Upstream + Papu Papu boss
  - Island 2: Rolling Stones, Hog Wild, Native Fortress, Up the Creek + Ripper Roo boss, The Lost City, Temple Ruins, Road to Nowhere, Boulder Dash, Sunset Vista + Koala Kong boss
  - Island 3: Heavy Machinery, Cortex Power, Generator Room, Toxic Waste + Pinstripe boss, The High Road, Slippery Climb, Lights Out, Jaws of Darkness, Castle Machinery + Dr. Nitrus Brio boss, The Lab, The Great Hall + Dr. Neo Cortex boss
  - Secret: Whole Hog, Fumbling in the Dark
- Each level uses the correct level type template (SPEC §12) and camera perspective (SPEC §4)
- Crate placement with correct total counts per level for gem tracking
- Enemy placement matching enemy types to level themes
- Hazard and obstacle placement
- Checkpoint crate placement for difficulty pacing
- Colored gem path areas in levels that require them (SPEC §11 — Colored Gem Path Dependencies)
- Bonus round token placement in designated ? crates
- Bonus round sub-levels for each level that has them
- Per-level music assignment via world settings
- Overworld map wired to all levels with correct progression order and boss gating

**Playtest**: play through every level in sequence. Verify all crate counts are correct, all gem paths work, all bonus rounds are accessible, and boss fights gate progression correctly.

---

## Phase 11: Game Flow & Polish

**Goal**: Complete game experience from title screen to credits.

**Deliver**:
- Full startup sequence: splash → title → main menu → save slot → overworld (SPEC §14 — Startup)
- Context-dependent death animations (SPEC §3 — Death Animations)
- 100% completion and alternate ending (SPEC §11 — 100% Completion)
- Transitions and loading screens
- **Audio**: title screen music, menu music, credits music, full audio mix pass across all levels (volume balancing, music transitions, ambient layering)
- Playtesting pass across all levels and systems
- **Debug level**: add a **Full Flow** zone — simulates the title → map → level → summary → map loop for end-to-end testing

**Playtest**: start a new game from the title screen, play through all levels and bosses start to finish, save and reload, verify the full loop end to end.
