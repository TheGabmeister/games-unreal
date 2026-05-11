# PLAN_03.md — Phase 3: Enemies

## Context

Phase 2 shipped crates, collectibles, Aku Aku protection, and spin/stomp hit detection via `ISpinnable`/`IStompable`/`IExplodable` interfaces. The player can attack the world, but there's nothing that fights back. Phase 3 populates levels with all 18 standard enemy types across 10 C++ archetype classes, a chain-kill system for spin-launched enemies, a projectile framework for ranged enemies, a hazard base for non-defeatable obstacles, and the turtle's flip-to-platform mechanic. Bosses are deferred to a later phase.

## Objective

Levels populated with enemies exhibiting distinct behaviors. The player can defeat enemies via spin, stomp, and Aku Aku invincibility contact-kill. Spin-launched enemies become projectiles that can chain-kill other enemies and trigger TNT crates (one level of chain, no cascading). The turtle flips upside-down and becomes a moving platform. All 18 standard enemy types are implemented across 10 archetype classes, plus 2 invulnerable hazard types. No bosses.

---

## Assets Needed

### 3D Models / Meshes (Placeholder Shapes via generate_meshes.py)

Enemies with animations are rigged skeletal meshes (SK\_). Projectiles, hazards, and props remain static meshes (SM\_).

#### Skeletal Meshes (rigged, with armature)

| Mesh Name | Shape Description |
|-----------|-------------------|
| SK_Crab | Flat wide ellipsoid (body), two front cones (claws) with claw joints |
| SK_Skunk | Elongated cylinder with raised tail cone, 4 leg stubs |
| SK_Tribesman | Capsule torso, arm bones, wider horizontal bar (club) on right hand |
| SK_VenusFlyTrap | Cylinder stalk with two half-sphere jaws on top, jaw hinge joint |
| SK_Turtle | Dome (shell) on flat disc, 4 leg stubs, head bone |
| SK_RollingMonkey | Sphere with appendage cones, spine bone for curl/uncurl |
| SK_Snake | Cylinder tube with tapered head, 3–4 spine segments for sway/lunge |
| SK_Bat | Small sphere body, two flat triangular wings with wing root joints |
| SK_Spider | Sphere body, 8 thin cylinder legs with knee joints |
| SK_LizardGreen | Low wide capsule, four leg stubs, tail cylinder, hip joint for jump crouch |
| SK_LizardRed | Same as green, slightly larger, red tint |
| SK_ShieldNative | Capsule torso, arm bones, flat box (shield) on left hand |
| SK_SpearThrower | Capsule torso, arm bones, thin cylinder (spear) on right hand |
| SK_LabAssistant | Capsule torso, arm bones, box (beaker) on right hand |
| SK_ElectricLabAssistant | Capsule torso, arm bones, wire cones on top (electrodes) |
| SK_GreenBlob | Small sphere, single root bone for squash/stretch |

#### Static Meshes (no rig)

| Mesh Name | Shape Description |
|-----------|-------------------|
| SM_FlyingFish | Elongated ellipsoid with fin triangles (movement is purely positional) |
| SM_Barrel | Standard cylinder (movement is purely positional) |
| SM_Spear | Thin long cylinder with cone tip (projectile) |
| SM_Beaker | Small cylinder with sphere top (projectile) |
| SM_InvisiblePlatform | Flat disc (turtle platform — invisible in-game, visible in editor) |
| SM_LaunchedEnemy | Generic tumbling capsule used for spin-launched enemy projectiles (all enemies share this) |
| SM_SnakeHole | Dark circle/disc with slight rim — ground marker for snake spawn point |
| SM_SpiderThread | Thin cylinder (thread/silk connecting ceiling spider to ceiling anchor point) |
| SM_ExplosionDisc | Flat translucent circle for red beaker AoE ground indicator |

#### Materials (via MaterialCreate commandlet or REST API)

All material instances parent to the project-wide master material at `/Game/Shared/Materials/M_MASTER`.

| Material Name | Parent | Description |
|---------------|--------|-------------|
| MI_Crab | M_MASTER | Orange tint |
| MI_Skunk | M_MASTER | Black body, white tail stripe |
| MI_Tribesman | M_MASTER | Brown/tan skin tone |
| MI_VenusFlyTrap_Green | M_MASTER | Green body, red jaw interior |
| MI_VenusFlyTrap_White | M_MASTER | White body, spike-textured top (spin-only variant) |
| MI_Turtle | M_MASTER | Brown shell, green skin |
| MI_RollingMonkey | M_MASTER | Brown fur tone |
| MI_Snake | M_MASTER | Green with darker pattern bands |
| MI_Bat | M_MASTER | Dark grey/purple |
| MI_Spider | M_MASTER | Black body, red eyes (emissive dot) |
| MI_LizardGreen | M_MASTER | Green body, lighter belly |
| MI_LizardRed | M_MASTER | Red body, lighter belly |
| MI_ShieldNative | M_MASTER | Tan skin, wooden shield has brown tint |
| MI_SpearThrower | M_MASTER | Tan skin, spear has grey metallic tip |
| MI_LabAssistant | M_MASTER | White lab coat, skin tone hands |
| MI_ElectricLab | M_MASTER | White lab coat, blue emissive on electrodes |
| MI_GreenBlob | M_MASTER | Translucent green, slight emissive |
| MI_FlyingFish | M_MASTER | Silver/blue iridescent |
| MI_Barrel | M_MASTER | Brown wood slats |
| MI_Spear | M_MASTER | Brown shaft, grey tip |
| MI_Beaker_Red | M_MASTER | Red translucent glass |
| MI_Beaker_Green | M_MASTER | Green translucent glass |
| MI_LaunchedEnemy | M_MASTER | Bright red (danger indicator, highly visible) |
| MI_SnakeHole | M_MASTER | Dark brown/black, unlit |
| MI_SpiderThread | M_MASTER | White, thin opacity |
| MI_ExplosionDisc | M_MASTER | Red, translucent, unlit, fades over 0.5s |

### Animations (via blender_anim.py, exported as FBX per skeleton)

Each skeletal mesh needs the following animation sequences. All are placeholder-quality (simple bone transforms, no polish).

#### SK_Crab
| Animation | Description |
|-----------|-------------|
| A_Crab_Idle | Subtle claw pinch cycle |
| A_Crab_Walk | Side-to-side shuffle, legs alternate |
| A_Crab_Death | Flips onto back, legs curl |

#### SK_Skunk
| Animation | Description |
|-----------|-------------|
| A_Skunk_Idle | Tail sway |
| A_Skunk_Walk | Forward walk cycle, legs alternate |
| A_Skunk_Death | Falls sideways |

#### SK_Tribesman
| Animation | Description |
|-----------|-------------|
| A_Tribesman_Idle | Club resting on shoulder, slight sway |
| A_Tribesman_Walk | Forward march, club bobs |
| A_Tribesman_Death | Drops club, falls backward |

#### SK_VenusFlyTrap
| Animation | Description |
|-----------|-------------|
| A_VenusFlyTrap_Open | Jaws spread wide (vulnerable state) |
| A_VenusFlyTrap_Snap | Jaws slam shut rapidly (attacking state) |
| A_VenusFlyTrap_Death | Stalk wilts, jaws droop |

#### SK_Turtle
| Animation | Description |
|-----------|-------------|
| A_Turtle_Idle | Head bobs, legs shift weight |
| A_Turtle_Walk | Slow plod cycle, legs alternate |
| A_Turtle_Flip | Legs tuck, shell rocks, flips upside-down |
| A_Turtle_Flipped | Legs wiggle helplessly (looping) |
| A_Turtle_Unflip | Rocks back to upright |
| A_Turtle_Death | Shell spins, falls flat |

#### SK_RollingMonkey
| Animation | Description |
|-----------|-------------|
| A_RollingMonkey_Idle | Standing pose, scratches head (vulnerable pause) |
| A_RollingMonkey_Roll | Curled into ball, tumble spin (attacking state, looping) |
| A_RollingMonkey_Death | Uncurls, falls flat |

#### SK_Snake
| Animation | Description |
|-----------|-------------|
| A_Snake_Emerge | Rises from ground, spine segments extend upward |
| A_Snake_Bob | Sway side-to-side (emerging idle, looping) |
| A_Snake_Lunge | Head snaps forward rapidly, body follows |
| A_Snake_Hide | Retracts downward into ground |
| A_Snake_Death | Coils and falls limp |

#### SK_Bat
| Animation | Description |
|-----------|-------------|
| A_Bat_Fly | Wing flap cycle (looping) |
| A_Bat_Swoop | Wings tuck, dive pose |
| A_Bat_Perch | Wings folded, hanging (swooping variant idle) |
| A_Bat_Death | Wings crumple, tumbles |

#### SK_Spider
| Animation | Description |
|-----------|-------------|
| A_Spider_Hang | Legs slightly curled, idle dangle (ceiling) |
| A_Spider_Drop | Legs splay outward on descent |
| A_Spider_Land | Legs compress on impact |
| A_Spider_Climb | Legs alternate upward pull (ceiling return) |
| A_Spider_Jump | Legs compress then extend upward (ground jumper) |
| A_Spider_Death | Legs curl inward |

#### SK_LizardGreen / SK_LizardRed
| Animation | Description |
|-----------|-------------|
| A_Lizard_Idle | Low crouch, slight tail sway |
| A_Lizard_Jump | Legs push off, body arcs |
| A_Lizard_Land | Legs absorb impact |
| A_Lizard_Death | Rolls onto side |

#### SK_ShieldNative
| Animation | Description |
|-----------|-------------|
| A_ShieldNative_Idle | Shield held forward, shifts weight |
| A_ShieldNative_Walk | March with shield up |
| A_ShieldNative_Block | Shield jolts from impact, recoil |
| A_ShieldNative_Death | Drops shield, falls |

#### SK_SpearThrower
| Animation | Description |
|-----------|-------------|
| A_SpearThrower_Idle | Holds spear at ready |
| A_SpearThrower_Windup | Arm draws back |
| A_SpearThrower_Throw | Arm snaps forward, releases spear |
| A_SpearThrower_Death | Drops spear, falls |

#### SK_LabAssistant
| Animation | Description |
|-----------|-------------|
| A_LabAssistant_Idle | Holds beaker, fidgets |
| A_LabAssistant_Windup | Arm draws back with beaker |
| A_LabAssistant_Throw | Lobs beaker in arc |
| A_LabAssistant_Death | Stumbles backward, falls |

#### SK_ElectricLabAssistant
| Animation | Description |
|-----------|-------------|
| A_ElectricLab_Idle | Electrodes spark (subtle arm twitch) |
| A_ElectricLab_Walk | Forward march, arms slightly raised |
| A_ElectricLab_Pursue | Faster walk, leaning forward |
| A_ElectricLab_Death | Electrodes short out, collapse |

#### SK_GreenBlob
| Animation | Description |
|-----------|-------------|
| A_GreenBlob_Idle | Squash/stretch pulse (looping) |
| A_GreenBlob_Bounce | Compress then spring upward |
| A_GreenBlob_Death | Splats flat |

### Sound Effects (via generate_sounds.py — shared generic sounds)

| Sound Name | Description |
|-----------|-------------|
| SFX_EnemyDefeat | Generic poof/pop when enemy dies |
| SFX_EnemyHit | Hit sound when enemy takes damage but survives |
| SFX_ShieldBlock | Metallic clank when spin bounces off shield |

Remaining sounds (SpearThrow, BeakerThrow, ElectricBuzz, SnakeHiss, BatScreech, BarrelBounce, FishSplash, TurtleFlip) use shared generic sounds or existing SFX. Distinct per-enemy audio is deferred to Polish.

### Asset Import Plan

Generated assets land in a staging directory on disk, then get imported into UE content via the AssetImport commandlet. The on-disk staging structure and UE content paths are:

#### On-Disk Staging (generated output, not checked in)

Each enemy is self-contained in its own folder — mesh, animations, and sounds together.

```
Assets/
├── Enemies/
│   ├── Crab/
│   │   ├── SK_Crab.fbx
│   │   ├── A_Crab_Idle.fbx
│   │   ├── A_Crab_Walk.fbx
│   │   └── A_Crab_Death.fbx
│   ├── Skunk/
│   │   ├── SK_Skunk.fbx
│   │   ├── A_Skunk_Idle.fbx
│   │   ├── A_Skunk_Walk.fbx
│   │   └── A_Skunk_Death.fbx
│   ├── Tribesman/
│   │   ├── SK_Tribesman.fbx
│   │   ├── A_Tribesman_Idle.fbx
│   │   ├── A_Tribesman_Walk.fbx
│   │   └── A_Tribesman_Death.fbx
│   ├── VenusFlyTrap/
│   │   ├── SK_VenusFlyTrap.fbx
│   │   ├── A_VenusFlyTrap_Open.fbx
│   │   ├── A_VenusFlyTrap_Snap.fbx
│   │   └── A_VenusFlyTrap_Death.fbx
│   ├── Turtle/
│   │   ├── SK_Turtle.fbx
│   │   ├── A_Turtle_Idle.fbx
│   │   ├── A_Turtle_Walk.fbx
│   │   ├── A_Turtle_Flip.fbx
│   │   ├── A_Turtle_Flipped.fbx
│   │   ├── A_Turtle_Unflip.fbx
│   │   └── A_Turtle_Death.fbx
│   ├── RollingMonkey/
│   │   ├── SK_RollingMonkey.fbx
│   │   ├── A_RollingMonkey_Idle.fbx
│   │   ├── A_RollingMonkey_Roll.fbx
│   │   └── A_RollingMonkey_Death.fbx
│   ├── Snake/
│   │   ├── SK_Snake.fbx
│   │   ├── SM_SnakeHole.fbx
│   │   ├── A_Snake_Emerge.fbx
│   │   ├── A_Snake_Bob.fbx
│   │   ├── A_Snake_Lunge.fbx
│   │   ├── A_Snake_Hide.fbx
│   │   └── A_Snake_Death.fbx
│   ├── Bat/
│   │   ├── SK_Bat.fbx
│   │   ├── A_Bat_Fly.fbx
│   │   ├── A_Bat_Swoop.fbx
│   │   ├── A_Bat_Perch.fbx
│   │   └── A_Bat_Death.fbx
│   ├── Spider/
│   │   ├── SK_Spider.fbx
│   │   ├── SM_SpiderThread.fbx
│   │   ├── A_Spider_Hang.fbx
│   │   ├── A_Spider_Drop.fbx
│   │   ├── A_Spider_Land.fbx
│   │   ├── A_Spider_Climb.fbx
│   │   ├── A_Spider_Jump.fbx
│   │   └── A_Spider_Death.fbx
│   ├── Lizard/
│   │   ├── SK_LizardGreen.fbx
│   │   ├── SK_LizardRed.fbx
│   │   ├── A_Lizard_Idle.fbx
│   │   ├── A_Lizard_Jump.fbx
│   │   ├── A_Lizard_Land.fbx
│   │   └── A_Lizard_Death.fbx
│   ├── ShieldNative/
│   │   ├── SK_ShieldNative.fbx
│   │   ├── A_ShieldNative_Idle.fbx
│   │   ├── A_ShieldNative_Walk.fbx
│   │   ├── A_ShieldNative_Block.fbx
│   │   └── A_ShieldNative_Death.fbx
│   ├── SpearThrower/
│   │   ├── SK_SpearThrower.fbx
│   │   ├── A_SpearThrower_Idle.fbx
│   │   ├── A_SpearThrower_Windup.fbx
│   │   ├── A_SpearThrower_Throw.fbx
│   │   └── A_SpearThrower_Death.fbx
│   ├── LabAssistant/
│   │   ├── SK_LabAssistant.fbx
│   │   ├── A_LabAssistant_Idle.fbx
│   │   ├── A_LabAssistant_Windup.fbx
│   │   ├── A_LabAssistant_Throw.fbx
│   │   └── A_LabAssistant_Death.fbx
│   ├── ElectricLab/
│   │   ├── SK_ElectricLabAssistant.fbx
│   │   ├── A_ElectricLab_Idle.fbx
│   │   ├── A_ElectricLab_Walk.fbx
│   │   ├── A_ElectricLab_Pursue.fbx
│   │   └── A_ElectricLab_Death.fbx
│   └── GreenBlob/
│       ├── SK_GreenBlob.fbx
│       ├── A_GreenBlob_Idle.fbx
│       ├── A_GreenBlob_Bounce.fbx
│       └── A_GreenBlob_Death.fbx
├── Hazards/
│   ├── FlyingFish/
│   │   └── SM_FlyingFish.fbx
│   └── BouncingBarrel/
│       └── SM_Barrel.fbx
├── Projectiles/
│   ├── SM_Spear.fbx
│   ├── SM_Beaker.fbx
│   └── SM_LaunchedEnemy.fbx
├── Props/
│   ├── SM_InvisiblePlatform.fbx
│   └── SM_ExplosionDisc.fbx
└── Sounds/
    ├── SFX_EnemyDefeat.wav
    ├── SFX_EnemyHit.wav
    └── SFX_ShieldBlock.wav
```

#### UE Content Paths (after import)

Each enemy gets its own folder containing all related assets — mesh, skeleton, animations, AnimBP, material, and Blueprint. Everything you need for one enemy is in one place.

```
Content/
├── Enemies/
│   ├── Crab/
│   │   ├── SK_Crab                    (SkeletalMesh + Skeleton)
│   │   ├── A_Crab_Idle, A_Crab_Walk, A_Crab_Death
│   │   ├── ABP_Crab                   (AnimBlueprint)
│   │   └── MI_Crab                    (MaterialInstance)
│   ├── Skunk/
│   │   ├── SK_Skunk
│   │   ├── A_Skunk_Idle, A_Skunk_Walk, A_Skunk_Death
│   │   ├── ABP_Skunk
│   │   └── MI_Skunk
│   ├── Tribesman/
│   │   ├── SK_Tribesman
│   │   ├── A_Tribesman_Idle, A_Tribesman_Walk, A_Tribesman_Death
│   │   ├── ABP_Tribesman
│   │   └── MI_Tribesman
│   ├── VenusFlyTrap/
│   │   ├── SK_VenusFlyTrap            (shared by both variants)
│   │   ├── A_VenusFlyTrap_Open, A_VenusFlyTrap_Snap, A_VenusFlyTrap_Death
│   │   ├── ABP_VenusFlyTrap
│   │   ├── MI_VenusFlyTrap_Green
│   │   └── MI_VenusFlyTrap_White
│   ├── Turtle/
│   │   ├── SK_Turtle
│   │   ├── A_Turtle_Idle, A_Turtle_Walk, A_Turtle_Flip, A_Turtle_Flipped, A_Turtle_Unflip, A_Turtle_Death
│   │   ├── ABP_Turtle
│   │   ├── MI_Turtle
│   │   └── SM_InvisiblePlatform
│   ├── RollingMonkey/
│   │   ├── SK_RollingMonkey
│   │   ├── A_RollingMonkey_Idle, A_RollingMonkey_Roll, A_RollingMonkey_Death
│   │   ├── ABP_RollingMonkey
│   │   └── MI_RollingMonkey
│   ├── Snake/
│   │   ├── SK_Snake
│   │   ├── SM_SnakeHole
│   │   ├── A_Snake_Emerge, A_Snake_Bob, A_Snake_Lunge, A_Snake_Hide, A_Snake_Death
│   │   ├── ABP_Snake
│   │   ├── MI_Snake
│   │   └── MI_SnakeHole
│   ├── Bat/
│   │   ├── SK_Bat
│   │   ├── A_Bat_Fly, A_Bat_Swoop, A_Bat_Perch, A_Bat_Death
│   │   ├── ABP_Bat
│   │   └── MI_Bat
│   ├── Spider/
│   │   ├── SK_Spider
│   │   ├── SM_SpiderThread
│   │   ├── A_Spider_Hang, A_Spider_Drop, A_Spider_Land, A_Spider_Climb, A_Spider_Jump, A_Spider_Death
│   │   ├── ABP_Spider
│   │   ├── MI_Spider
│   │   └── MI_SpiderThread
│   ├── Lizard/
│   │   ├── SK_LizardGreen
│   │   ├── SK_LizardRed
│   │   ├── A_Lizard_Idle, A_Lizard_Jump, A_Lizard_Land, A_Lizard_Death
│   │   ├── ABP_Lizard                 (shared — same skeleton structure)
│   │   ├── MI_LizardGreen
│   │   └── MI_LizardRed
│   ├── ShieldNative/
│   │   ├── SK_ShieldNative
│   │   ├── A_ShieldNative_Idle, A_ShieldNative_Walk, A_ShieldNative_Block, A_ShieldNative_Death
│   │   ├── ABP_ShieldNative
│   │   └── MI_ShieldNative
│   ├── SpearThrower/
│   │   ├── SK_SpearThrower
│   │   ├── A_SpearThrower_Idle, A_SpearThrower_Windup, A_SpearThrower_Throw, A_SpearThrower_Death
│   │   ├── ABP_SpearThrower
│   │   └── MI_SpearThrower
│   ├── LabAssistant/
│   │   ├── SK_LabAssistant
│   │   ├── A_LabAssistant_Idle, A_LabAssistant_Windup, A_LabAssistant_Throw, A_LabAssistant_Death
│   │   ├── ABP_LabAssistant
│   │   └── MI_LabAssistant
│   ├── ElectricLab/
│   │   ├── SK_ElectricLabAssistant
│   │   ├── A_ElectricLab_Idle, A_ElectricLab_Walk, A_ElectricLab_Pursue, A_ElectricLab_Death
│   │   ├── ABP_ElectricLab
│   │   └── MI_ElectricLab
│   ├── GreenBlob/
│   │   ├── SK_GreenBlob
│   │   ├── A_GreenBlob_Idle, A_GreenBlob_Bounce, A_GreenBlob_Death
│   │   ├── ABP_GreenBlob
│   │   └── MI_GreenBlob
│   └── (all MI_ instances parent to /Game/Shared/Materials/M_MASTER)
├── Hazards/
│   ├── FlyingFish/
│   │   ├── SM_FlyingFish
│   │   └── MI_FlyingFish
│   └── BouncingBarrel/
│       ├── SM_Barrel
│       └── MI_Barrel
├── Projectiles/
│   ├── SM_Spear
│   ├── SM_Beaker
│   ├── SM_LaunchedEnemy
│   ├── SM_ExplosionDisc
│   ├── MI_Spear
│   ├── MI_Beaker_Red
│   ├── MI_Beaker_Green
│   ├── MI_LaunchedEnemy
│   └── MI_ExplosionDisc
├── Blueprints/
│   ├── Enemies/
│   │   ├── BP_Crab
│   │   ├── BP_Skunk
│   │   ├── BP_Tribesman
│   │   ├── BP_VenusFlyTrapGreen
│   │   ├── BP_VenusFlyTrapWhite
│   │   ├── BP_RollingMonkey
│   │   ├── BP_Snake
│   │   ├── BP_BatFlying
│   │   ├── BP_BatSwooping
│   │   ├── BP_SpiderCeiling
│   │   ├── BP_SpiderGround
│   │   ├── BP_LizardGreen
│   │   ├── BP_LizardRed
│   │   ├── BP_ShieldNative
│   │   ├── BP_SpearThrower
│   │   ├── BP_BeakerLabAssistant
│   │   ├── BP_ElectricLabAssistant
│   │   ├── BP_Turtle
│   │   └── BP_GreenBlob
│   ├── Hazards/
│   │   ├── BP_FlyingFish
│   │   └── BP_BouncingBarrel
│   └── Projectiles/
│       ├── BP_Spear
│       ├── BP_BeakerRed
│       └── BP_BeakerGreen
└── Sounds/
    └── Enemies/
        ├── SFX_EnemyDefeat
        ├── SFX_EnemyHit
        └── SFX_ShieldBlock
```

#### Import Sequence (commandlet calls)

Run in order — each step depends on the previous. Process one enemy folder at a time (mesh → animations → material → blueprint).

**Step 1: Import skeletal meshes** (creates Skeleton assets automatically)
```bash
# For each enemy folder — import the SK_ mesh into its own Content folder:
MSYS_NO_PATHCONV=1 "...UnrealEditor-Cmd.exe" "CB.uproject" -run=AssetImport \
  -SourceFile="Assets/Enemies/Crab/SK_Crab.fbx" \
  -DestPath=/Game/Enemies/Crab -Type=SkeletalMesh -Replace
```

**Step 2: Import static meshes** (props and projectiles into their respective folders)
```bash
# Enemy-owned static meshes go in the enemy folder:
MSYS_NO_PATHCONV=1 "...UnrealEditor-Cmd.exe" "CB.uproject" -run=AssetImport \
  -SourceFile="Assets/Enemies/Snake/SM_SnakeHole.fbx" \
  -DestPath=/Game/Enemies/Snake -Type=StaticMesh -Replace

# Hazard/projectile meshes go in their own folders:
MSYS_NO_PATHCONV=1 "...UnrealEditor-Cmd.exe" "CB.uproject" -run=AssetImport \
  -SourceFile="Assets/Hazards/FlyingFish/SM_FlyingFish.fbx" \
  -DestPath=/Game/Hazards/FlyingFish -Type=StaticMesh -Replace
```

**Step 3: Import animations** (reference skeleton from same enemy folder)
```bash
# For each A_ animation — skeleton path points to the enemy's own folder:
MSYS_NO_PATHCONV=1 "...UnrealEditor-Cmd.exe" "CB.uproject" -run=AssetImport \
  -SourceFile="Assets/Enemies/Crab/A_Crab_Idle.fbx" \
  -DestPath=/Game/Enemies/Crab -Type=Animation \
  -Skeleton=/Game/Enemies/Crab/SK_Crab -Replace
```

**Step 4: Import sounds**
```bash
MSYS_NO_PATHCONV=1 "...UnrealEditor-Cmd.exe" "CB.uproject" -run=AssetImport \
  -SourceFile="Assets/Sounds/SFX_EnemyDefeat.wav" \
  -DestPath=/Game/Sounds/Enemies -Replace
```

**Step 5: Create materials** (per-enemy MI_ instances parented to M_MASTER)
```bash
MSYS_NO_PATHCONV=1 "...UnrealEditor-Cmd.exe" "CB.uproject" -run=MaterialCreate \
  -AssetPath=/Game/Enemies/Crab/MI_Crab \
  -ParentMaterial=/Game/Shared/Materials/M_MASTER \
  -SetVector="TintColor=(R=0.9,G=0.5,B=0.1,A=1.0)"
```

**Step 6: Create Blueprints + assign mesh/materials via REST API** (requires editor open)

BPs go in `/Game/Blueprints/Enemies/`, `/Game/Blueprints/Hazards/`, `/Game/Blueprints/Projectiles/`.

```bash
# Create Blueprint (commandlet — works with editor open):
MSYS_NO_PATHCONV=1 "...UnrealEditor-Cmd.exe" "CB.uproject" -run=BlueprintCreate \
  -AssetPath=/Game/Blueprints/Enemies/BP_Crab \
  -ParentClass=/Script/CB.PatrolEnemy

# Import mesh via REST (fire-and-forget, editor must be open):
curl -s --max-time 5 http://localhost:3000/api/call -H "Content-Type: application/json" \
  -d '{"tool":"manage_asset","arguments":{"action":"import","sourcePath":"c:/dev/CrashBandicoot/Assets/Enemies/Crab/SK_Crab.fbx","destinationPath":"/Game/Enemies/Crab"}}'

# Assign mesh to Blueprint via REST (dot-path for inherited C++ components):
curl -s --max-time 5 http://localhost:3000/api/call -H "Content-Type: application/json" \
  -d '{"tool":"manage_blueprint","arguments":{"action":"set_default","requestedPath":"/Game/Blueprints/Enemies/BP_Crab","propertyName":"Mesh.SkeletalMeshAsset","value":"/Game/Enemies/Crab/SK_Crab.SK_Crab"}}'
```

**Note:** Skeletal mesh FBX exports must use `apply_unit_scale=False`, `apply_scale_options='FBX_SCALE_NONE'` to avoid double unit conversion with UE's importer. Static meshes continue using `FBX_SCALE_ALL`.

---

## 0. Naming Convention (Phase 3)

Per CLAUDE.md: the `CB` prefix is reserved for core game framework classes only. Enemy/hazard classes use **no prefix**. The existing `ACBEnemyCharacterBase` and `ACBEnemyAIControllerBase` keep their CB prefix since they are foundation infrastructure. All new subclasses use no prefix.

**Existing (keep CB prefix):**
- `ACBEnemyCharacterBase`, `ACBEnemyAIControllerBase`
- `UCBEnemyPatrolRigComponent`, `ACBEnemyPatrolRigActor`
- `UCBPatrolRigDebugVisualizer`

**New (no prefix):**
- `AEnemyPatrol`, `AEnemyCycling`, `AEnemySnake`, `AEnemyFlying`, `AEnemyCeiling`, `AEnemyJumping`, `AEnemyShielded`, `AEnemyRanged`, `AEnemyElectric`, `AEnemyTurtle`
- `AHazardBase`, `AFlyingFishHazard`, `ABouncingBarrelHazard`
- `AProjectileBase`, `ASpearProjectile`, `ABeakerProjectile`, `AGreenBlobEnemy`
- `ALaunchedEnemyProjectile`

---

## 1. Refactoring the Existing Enemy Infrastructure

### 1.1 Fix: PatrolRig AttackTriggerBox Not Attached

In `UCBEnemyPatrolRigComponent::OnComponentCreated()`, the `AttackTriggerBox` is created but never attached. Add `AttackTriggerBox->SetupAttachment(this)` and add the corresponding destruction in `OnComponentDestroyed()`.

### 1.2 Gut the AI Controller

`ACBEnemyAIControllerBase` currently has only `BlueprintImplementableEvent` stubs designed for behavior trees. Decision: pure C++ state machines, no BTs. The class becomes a minimal `AAIController` subclass:

- Remove `Abstract` from UCLASS
- Remove all `BlueprintImplementableEvent` functions
- Enemies self-drive via their archetype's `Tick()`, not the controller

### 1.3 Add Interface Implementation to Base Class

`ACBEnemyCharacterBase` implements all three interfaces with virtual default behavior. Subclasses override to customize.

```cpp
UCLASS(meta = (PrioritizeCategories = "CB"))
class CB_API ACBEnemyCharacterBase : public ACharacter,
    public ISpinnable, public IStompable, public IExplodable
{
    // ISpinnable — default: kill enemy, launch as projectile
    virtual void OnSpinHit(ACBPlayerCharacter* Player) override;

    // IStompable — default: kill enemy, bounce player
    virtual void OnJumpHit(ACBPlayerCharacter* Player) override;

    // IExplodable — default: kill enemy
    virtual void OnExplosionHit(FVector Origin, float Radius) override;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    TObjectPtr<USoundBase> DefeatSound;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    float SpinLaunchForce = 1500.0f;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    bool bSpinLaunchesAsProjectile = true;

protected:
    virtual void HandleDefeat();
    void SpawnLaunchedProjectile(FVector LaunchDirection);
};
```

**Default behavior:**
- `OnSpinHit`: compute launch direction away from player, `KillCharacter()`, if `bSpinLaunchesAsProjectile` spawn `ALaunchedEnemyProjectile`, then `HandleDefeat()`.
- `OnJumpHit`: bounce player via `LaunchCharacter(FVector(0, 0, Player->StompBounceVelocity))`, `KillCharacter()`, `HandleDefeat()`.
- `OnExplosionHit`: `KillCharacter()`, `HandleDefeat()`.
- `HandleDefeat()`: play `DefeatSound`, disable capsule collision, disable movement, `SetLifeSpan(0.1f)`.

### 1.4 Update HitBeginOverlap for Invincibility Contact Kill

The enemy's `HitBeginOverlap` already fires when player overlaps the capsule. Add the invincibility check there — no changes to `ACBPlayerCharacter` needed:

```cpp
void ACBEnemyCharacterBase::HitBeginOverlap(AActor* OverlappedActor, float Force)
{
    if (IsDead()) return;
    if (ACBPlayerCharacter* Player = Cast<ACBPlayerCharacter>(OverlappedActor))
    {
        if (Player->IsDead()) return;
        if (Player->IsInvincible())
        {
            OnSpinHit(Player); // Invincible player kills on contact
            return;
        }
        Player->OnHit(this);
    }
}
```

### 1.5 Bind Capsule Overlap in C++

Add C++ overlap binding in `BeginPlay()` so new enemies get damage overlap without Blueprint wiring:

```cpp
GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(
    this, &ACBEnemyCharacterBase::OnCapsuleOverlap);
```

### 1.6 Remove Behavior Tree Forwarding

Remove all `Controller->OnXyz()` forwarding calls from `ACBEnemyCharacterBase` (patrol overlap, attack overlap, hit stun callbacks). New enemies do not use the AI controller for logic.

---

## 2. Enemy State Machine Architecture

Each archetype owns an `UENUM` defining its states and a `CurrentState` member. `SetState(NewState)` fires entry/exit logic. `Tick()` dispatches to per-state update functions via switch.

**No shared state machine component or template.** Each archetype has 2–5 states. A generic FSM framework adds abstraction without reducing code. The enum is local to the archetype. This follows KISS and locality-of-change principles.

---

## 3. Archetype Class Designs

### 3.1 AEnemyPatrol (Crab, Skunk, Basic Tribesman)

**States:** `Idle → Patrolling` (player enters PatrolTriggerVolume) `→ Idle` (player exits). `Any → Dead`.

```cpp
UENUM(BlueprintType)
enum class EPatrolEnemyState : uint8 { Idle, Patrolling, Dead };

UCLASS(meta = (PrioritizeCategories = "CB"))
class CB_API AEnemyPatrol : public ACBEnemyCharacterBase
{
    GENERATED_BODY()
public:
    AEnemyPatrol();
    virtual void Tick(float DeltaTime) override;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "CB")
    EPatrolEnemyState CurrentState = EPatrolEnemyState::Idle;

    void SetState(EPatrolEnemyState NewState);
    void TickPatrolling(float DeltaTime);

    FVector CurrentPatrolTarget;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    float PatrolPointReachedThreshold = 50.0f;
};
```

Moves toward `CurrentPatrolTarget` via `AddMovementInput`. On arrival, calls `GetNextPatrolLocation()` (inherited). Rotates via `bOrientRotationToMovement`.

**BP Variants:**
- BP_Crab: slow speed, short patrol, SK_Crab, ABP_Crab
- BP_Skunk: moderate speed, SK_Skunk, ABP_Skunk
- BP_BasicTribesman: moderate speed, wider capsule (club), SK_Tribesman, ABP_Tribesman

### 3.2 AEnemyCycling (Venus Fly Trap green/white, Rolling Monkey)

**States:** `Vulnerable ↔ Attacking` (timer-driven). `Any → Dead`.

```cpp
UENUM(BlueprintType)
enum class ECyclingEnemyState : uint8 { Vulnerable, Attacking, Dead };

UCLASS(meta = (PrioritizeCategories = "CB"))
class CB_API AEnemyCycling : public ACBEnemyCharacterBase
{
    GENERATED_BODY()
public:
    AEnemyCycling();
    virtual void Tick(float DeltaTime) override;
    virtual void OnSpinHit(ACBPlayerCharacter* Player) override;
    virtual void OnJumpHit(ACBPlayerCharacter* Player) override;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "CB")
    ECyclingEnemyState CurrentState = ECyclingEnemyState::Vulnerable;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    float VulnerableDuration = 2.5f;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    float AttackingDuration = 1.0f;

    // If true, cannot be jumped on during any state (white Venus Fly Trap spikes)
    UPROPERTY(EditDefaultsOnly, Category = "CB")
    bool bSpikedTop = false;

    // If true, patrols during Attacking state (Rolling Monkey)
    UPROPERTY(EditDefaultsOnly, Category = "CB")
    bool bPatrolsWhileAttacking = false;

    // If true, contact damages only during Attacking (vs always)
    UPROPERTY(EditDefaultsOnly, Category = "CB")
    bool bOnlyDangerousWhileAttacking = true;

    void SetState(ECyclingEnemyState NewState);
    float StateTimer = 0.0f;
};
```

**Interface overrides:**
- `OnSpinHit`: Rolling Monkey is invulnerable during `Attacking`. Venus Fly Trap is spinnable in both states.
- `OnJumpHit`: If `bSpikedTop`, calls `Player->OnHit(this)` (player takes damage). Otherwise, stomp-kill only during `Vulnerable`.

**BP Variants:**
- BP_VenusFlyTrapGreen: `bSpikedTop = false`, stationary
- BP_VenusFlyTrapWhite: `bSpikedTop = true`, stationary
- BP_RollingMonkey: `bPatrolsWhileAttacking = true`, invulnerable while rolling

### 3.3 AEnemySnake

**States:** `Hidden → Emerging → Lunging → Hidden`. `Any non-Hidden → Dead`.

```cpp
UENUM(BlueprintType)
enum class ESnakeEnemyState : uint8 { Hidden, Emerging, Lunging, Dead };

UCLASS(meta = (PrioritizeCategories = "CB"))
class CB_API AEnemySnake : public ACBEnemyCharacterBase
{
    GENERATED_BODY()
public:
    AEnemySnake();
    virtual void Tick(float DeltaTime) override;
    virtual void OnSpinHit(ACBPlayerCharacter* Player) override;
    virtual void OnJumpHit(ACBPlayerCharacter* Player) override;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "CB")
    ESnakeEnemyState CurrentState = ESnakeEnemyState::Hidden;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    float HiddenDuration = 2.5f;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    float EmergeDuration = 2.0f;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    float LungeDuration = 0.5f;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    float LungeDistance = 200.0f;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    FVector LungeDirection = FVector::ForwardVector;

    FVector HoleLocation; // Spawn location, snake returns here
    float StateTimer = 0.0f;
};
```

**Behavior:**
- `Hidden`: collision disabled, mesh hidden. Timer counts down.
- `Emerging`: mesh visible, collision enabled, sway animation. Vulnerable to spin/stomp.
- `Lunging`: rapid interpolation in `LungeDirection`. Contact damages player. Still vulnerable but tight timing.
- Spin/stomp during `Hidden`: no-op (underground).

### 3.4 AEnemyFlying (Bat normal, Bat swooping)

**States:** Flying variant: `Flying → Dead`. Swooping variant: `Perched → Swooping → Returning → Perched`. `Any → Dead`.

```cpp
UENUM(BlueprintType)
enum class EFlyingEnemyState : uint8 { Flying, Perched, Swooping, Returning, Dead };

UCLASS(meta = (PrioritizeCategories = "CB"))
class CB_API AEnemyFlying : public ACBEnemyCharacterBase
{
    GENERATED_BODY()
public:
    AEnemyFlying();
    virtual void Tick(float DeltaTime) override;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "CB")
    EFlyingEnemyState CurrentState = EFlyingEnemyState::Flying;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    bool bIsSwooper = false;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    float FlySpeed = 300.0f;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    float SwoopDepth = 400.0f;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    float SwoopDuration = 0.8f;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    float ReturnDuration = 1.5f;

    float CurrentSplineDistance = 0.0f;
    FVector PerchLocation;
    float StateAlpha = 0.0f;
};
```

**Flying variant:** `GravityScale = 0`. Uses `SetActorLocation` along spline each tick (not `AddMovementInput` — flying characters shouldn't use walking movement mode).

**Swooping variant:** `AttackTriggerVolume` triggers the swoop. Fixed arc (not homing) — interpolate from perch to `(perch.XY, perch.Z - SwoopDepth)` with ease-in curve, then back.

### 3.5 AEnemyCeiling (Spider ceiling dropper)

**States:** `Hanging → Dropping → Landed → Climbing → Hanging`. `Any → Dead`.

```cpp
UENUM(BlueprintType)
enum class ECeilingEnemyState : uint8 { Hanging, Dropping, Landed, Climbing, Dead };

UCLASS(meta = (PrioritizeCategories = "CB"))
class CB_API AEnemyCeiling : public ACBEnemyCharacterBase
{
    GENERATED_BODY()
public:
    AEnemyCeiling();
    virtual void Tick(float DeltaTime) override;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "CB")
    ECeilingEnemyState CurrentState = ECeilingEnemyState::Hanging;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    float DropSpeed = 2000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    float ClimbSpeed = 200.0f;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    float LandedPauseDuration = 1.0f;

    FVector CeilingLocation; // Spawn point
    FVector GroundLocation;  // Computed via line trace on BeginPlay
    float StateTimer = 0.0f;
};
```

`GravityScale = 0`. Direct `SetActorLocation` interpolation. On `BeginPlay`, line trace downward to find ground. Drop is near-instant, climb is slow. `AttackTriggerVolume` positioned below triggers the drop.

### 3.6 AEnemyJumping (Spider ground jumper, Green Lizard, Red Lizard)

**States:** `Grounded → Jumping → Grounded`. `Any → Dead`.

```cpp
UENUM(BlueprintType)
enum class EJumpingEnemyState : uint8 { Grounded, Jumping, Dead };

UCLASS(meta = (PrioritizeCategories = "CB"))
class CB_API AEnemyJumping : public ACBEnemyCharacterBase
{
    GENERATED_BODY()
public:
    AEnemyJumping();
    virtual void Tick(float DeltaTime) override;
    virtual void Landed(const FHitResult& Hit) override;
    virtual void OnJumpHit(ACBPlayerCharacter* Player) override;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "CB")
    EJumpingEnemyState CurrentState = EJumpingEnemyState::Grounded;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    float GroundedDuration = 2.0f;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    float JumpImpulseZ = 800.0f;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    float JumpImpulseForward = 200.0f;

    // Red Lizard: extra-high bounce on stomp
    UPROPERTY(EditDefaultsOnly, Category = "CB")
    bool bExtraHighBounce = false;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    float ExtraHighBounceVelocity = 1400.0f;

    // Spider ground jumper: contact while airborne damages player
    UPROPERTY(EditDefaultsOnly, Category = "CB")
    bool bDangerousWhileAirborne = false;

    float StateTimer = 0.0f;
};
```

**OnJumpHit override** (Red Lizard extra bounce — hardcoded, no framework):
```cpp
void AEnemyJumping::OnJumpHit(ACBPlayerCharacter* Player)
{
    if (IsDead()) return;
    float BounceVelocity = bExtraHighBounce ? ExtraHighBounceVelocity : Player->StompBounceVelocity;
    Player->LaunchCharacter(FVector(0.f, 0.f, BounceVelocity), false, true);
    KillCharacter();
    HandleDefeat();
}
```

**BP Variants:**
- BP_SpiderGround: `bDangerousWhileAirborne = true`
- BP_LizardGreen: default settings
- BP_LizardRed: `bExtraHighBounce = true`, `ExtraHighBounceVelocity = 1400`

### 3.7 AEnemyShielded (Shield Native)

Same patrol states as `AEnemyPatrol` (`Idle/Patrolling/Dead`) but with frontal shield blocking on `OnSpinHit`.

```cpp
UCLASS(meta = (PrioritizeCategories = "CB"))
class CB_API AEnemyShielded : public ACBEnemyCharacterBase
{
    GENERATED_BODY()
public:
    AEnemyShielded();
    virtual void Tick(float DeltaTime) override;
    virtual void OnSpinHit(ACBPlayerCharacter* Player) override;

protected:
    EPatrolEnemyState CurrentState = EPatrolEnemyState::Idle;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    float ShieldHalfAngle = 90.0f;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    float SpinBouncebackForce = 500.0f;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    TObjectPtr<USoundBase> ShieldBlockSound;

    bool IsSpinBlockedByShield(ACBPlayerCharacter* Player) const;
};
```

**Shield dot product math:**
```cpp
bool AEnemyShielded::IsSpinBlockedByShield(ACBPlayerCharacter* Player) const
{
    FVector ShieldForward = GetActorForwardVector();
    FVector ToPlayer = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
    float Dot = FVector::DotProduct(ShieldForward, ToPlayer);
    float CosHalfAngle = FMath::Cos(FMath::DegreesToRadians(ShieldHalfAngle));
    return Dot > CosHalfAngle; // Player is within frontal cone
}
```

**OnSpinHit override:** If blocked, bounce player back via `LaunchCharacter`, play `ShieldBlockSound`, return. If from behind, `Super::OnSpinHit(Player)` (normal kill).

**Shield does NOT block:**
- Stomp (`OnJumpHit` inherited, always kills)
- Chain-kill projectiles (use `OnExplosionHit`, bypasses shield check)

### 3.8 AEnemyRanged (Spear Thrower, Beaker Lab Assistant)

**States:** `Idle → Throwing` (timer interval) `→ Idle`. `Any → Dead`.

```cpp
UENUM(BlueprintType)
enum class ERangedEnemyState : uint8 { Idle, Throwing, Dead };

UCLASS(meta = (PrioritizeCategories = "CB"))
class CB_API AEnemyRanged : public ACBEnemyCharacterBase
{
    GENERATED_BODY()
public:
    AEnemyRanged();
    virtual void Tick(float DeltaTime) override;

protected:
    ERangedEnemyState CurrentState = ERangedEnemyState::Idle;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    TSubclassOf<AProjectileBase> ProjectileClass;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    float ThrowInterval = 2.5f;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    float ThrowWindupTime = 0.3f;

    // Beaker aims at player position at throw time. Spear fires forward.
    UPROPERTY(EditDefaultsOnly, Category = "CB")
    bool bAimAtPlayer = false;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    FVector ProjectileSpawnOffset = FVector(50.f, 0.f, 50.f);

    void SpawnProjectile();
    float StateTimer = 0.0f;
};
```

**BP Variants:**
- BP_SpearThrower: `bAimAtPlayer = false`, `ProjectileClass = ASpearProjectile`
- BP_BeakerLabAssistant: `bAimAtPlayer = true`, `ProjectileClass = ABeakerProjectile`

### 3.9 AEnemyElectric (Electric Lab Assistant)

**States:** `Idle → Patrolling → Pursuing` (player enters AttackTriggerVolume) `→ Patrolling` (player exits). `Any → Dead`.

```cpp
UENUM(BlueprintType)
enum class EElectricEnemyState : uint8 { Idle, Patrolling, Pursuing, Dead };

UCLASS(meta = (PrioritizeCategories = "CB"))
class CB_API AEnemyElectric : public ACBEnemyCharacterBase
{
    GENERATED_BODY()
public:
    AEnemyElectric();
    virtual void Tick(float DeltaTime) override;
    virtual void OnSpinHit(ACBPlayerCharacter* Player) override;
    virtual void OnJumpHit(ACBPlayerCharacter* Player) override;

protected:
    EElectricEnemyState CurrentState = EElectricEnemyState::Idle;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    float PursuitSpeedMultiplier = 0.7f;

    // Spin only works if player Z is below this offset from enemy center
    UPROPERTY(EditDefaultsOnly, Category = "CB")
    float ElectricBarrierHeight = 40.0f;

    TWeakObjectPtr<ACBPlayerCharacter> PursuitTarget;
};
```

**OnSpinHit override:**
```cpp
void AEnemyElectric::OnSpinHit(ACBPlayerCharacter* Player)
{
    if (IsDead()) return;
    float BarrierZ = GetActorLocation().Z + ElectricBarrierHeight;
    if (Player->GetActorLocation().Z < BarrierZ)
        Super::OnSpinHit(Player); // Below barrier — kill
    else
        Player->OnHit(this); // Above barrier — player takes damage
}
```

**OnJumpHit override:** Electricity on top — `Player->OnHit(this)` (player always takes damage from stomping).

**Pursuit:** `AddMovementInput` toward player at `PursuitSpeedMultiplier * MaxWalkSpeed`. Loses interest when player exits `AttackTriggerVolume`.

### 3.10 AEnemyTurtle

**States:** `Idle → Patrolling → Flipped` (stomped) `→ Patrolling` (5s timer). `Any → Dead` (spin).

```cpp
UENUM(BlueprintType)
enum class ETurtleEnemyState : uint8 { Idle, Patrolling, Flipped, Dead };

UCLASS(meta = (PrioritizeCategories = "CB"))
class CB_API AEnemyTurtle : public ACBEnemyCharacterBase
{
    GENERATED_BODY()
public:
    AEnemyTurtle();
    virtual void Tick(float DeltaTime) override;
    virtual void OnSpinHit(ACBPlayerCharacter* Player) override;
    virtual void OnJumpHit(ACBPlayerCharacter* Player) override;

protected:
    ETurtleEnemyState CurrentState = ETurtleEnemyState::Idle;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    float FlippedDuration = 5.0f;

    UPROPERTY(VisibleAnywhere, Category = "CB")
    TObjectPtr<UStaticMeshComponent> PlatformComponent;

    void FlipUp();
    float FlippedTimer = 0.0f;
};
```

---

## 4. Turtle Platform Implementation (Step by Step)

The turtle uses a **separate invisible `UStaticMeshComponent`** as the platform surface. The capsule stays on the Enemy channel for overlap detection; the platform uses `BlockAll` for the player to stand on.

**Step 1 — Constructor:** Create `PlatformComponent` with `BlockAll` profile, `NoCollision` initially, invisible, `CanCharacterStepUpOn = ECB_Yes`.

**Step 2 — OnJumpHit:** Bounce player, `SetState(Flipped)`. If already flipped: bounce player again, reset flip timer.

**Step 3 — SetState(Flipped):**
- Rotate mesh 180° (shell-up visual)
- `PlatformComponent->SetCollisionEnabled(QueryAndPhysics)` — player can stand on it
- `GetCapsuleComponent()->SetCollisionResponseToChannel(CBCollision::Player, ECR_Ignore)` — disable damage overlap while flipped

**Step 4 — SetState(Patrolling):**
- Restore mesh rotation
- `PlatformComponent->SetCollisionEnabled(NoCollision)` — disable platform
- Re-enable capsule overlap for damage

**Step 5 — TickFlipped:** Increment timer. Continue patrol movement (turtle keeps walking while flipped, player rides it). On `FlippedDuration` elapsed, call `FlipUp()`.

**Step 6 — FlipUp:** `SetState(Patrolling)`. Player falls when platform collision disables. If they land on the now-dangerous capsule, they take damage. This is correct behavior per spec.

**Step 7 — OnSpinHit:** Always kills regardless of state (`Super::OnSpinHit`).

**UE5 moving platform note:** The player's `UCharacterMovementComponent` automatically recognizes `PlatformComponent` as a movement base when its floor sweep hits the `BlockAll` component. The player moves with the turtle. No extra code needed.

**UE5 collision coexistence:** The capsule (Enemy channel, Overlap with Player) and PlatformComponent (BlockAll, WorldStatic) coexist as separate components with separate collision settings. The player's `ECC_Pawn` is blocked by `BlockAll` automatically.

---

## 5. Chain-Kill Launched Enemy System

When an enemy is spin-killed, a **new actor** (`ALaunchedEnemyProjectile`) spawns at the enemy's location, flying in the launch direction. The original enemy is destroyed immediately.

**Why a new actor:** Switching an `ACharacter` to simulated physics mid-game fights `UCharacterMovementComponent`. A simple actor with velocity + gravity is cleaner.

```cpp
UCLASS(meta = (PrioritizeCategories = "CB"))
class CB_API ALaunchedEnemyProjectile : public AActor
{
    GENERATED_BODY()
public:
    ALaunchedEnemyProjectile();
    virtual void Tick(float DeltaTime) override;
    void InitProjectile(UStaticMesh* VisualMesh, FVector Direction, float Speed);

protected:
    TObjectPtr<UStaticMeshComponent> MeshComponent;
    TObjectPtr<USphereComponent> CollisionSphere; // 40-unit radius, OverlapAllDynamic

    float Lifetime = 3.0f;
    float GravityScale = 1.0f;
    FVector Velocity;

    void OnProjectileOverlap(...);
};
```

**Tick:** Apply gravity to velocity, `AddActorWorldOffset` with sweep, destroy on blocking hit, tumble rotation for visual effect.

**OnProjectileOverlap:**
- Skip player (launched enemies don't hurt the player)
- Kill enemies via `OnExplosionHit` (bypasses shields — intentional)
- Trigger `IExplodable` on other actors (TNT chain)
- Self-destruct on contact

**Chain limitation enforcement:** `OnExplosionHit` → `KillCharacter()` → `HandleDefeat()`. `HandleDefeat()` does NOT call `SpawnLaunchedProjectile()`. Only `OnSpinHit` spawns launched projectiles. One level of chain guaranteed.

**Spawning:** `ACBEnemyCharacterBase::SpawnLaunchedProjectile()` creates the projectile using `SM_LaunchedEnemy` (generic tumbling capsule with `MI_LaunchedEnemy` bright red material), sets direction and speed. All enemies share the same launched mesh — no per-enemy static mesh copies needed.

---

## 6. Projectile System

### AProjectileBase

```cpp
UCLASS(Abstract, meta = (PrioritizeCategories = "CB"))
class CB_API AProjectileBase : public AActor
{
    GENERATED_BODY()
public:
    AProjectileBase();
    virtual void Tick(float DeltaTime) override;

    TObjectPtr<USphereComponent> CollisionComponent;
    TObjectPtr<UStaticMeshComponent> MeshComponent;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    float Speed = 800.0f;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    float Lifetime = 5.0f;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    float ArcGravity = 0.0f; // 0 = straight line

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    TObjectPtr<USoundBase> LaunchSound;

    virtual void InitDirection(FVector Direction);

protected:
    FVector Velocity;
    virtual void OnImpact(const FHitResult& Hit);
    void OnOverlap(...); // Damages player via Player->OnHit(this)
};
```

**Tick:** Apply `ArcGravity`, `AddActorWorldOffset` with sweep, `OnImpact` on blocking hit.

### ASpearProjectile

Straight line (`ArcGravity = 0`), `Speed = 1200`. Destroyed on impact with geometry or player.

### ABeakerProjectile

```cpp
UENUM(BlueprintType)
enum class EBeakerType : uint8 { Red, Green };

UCLASS(meta = (PrioritizeCategories = "CB"))
class CB_API ABeakerProjectile : public AProjectileBase
{
    UPROPERTY(EditDefaultsOnly, Category = "CB")
    EBeakerType BeakerType = EBeakerType::Red;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    TSubclassOf<AGreenBlobEnemy> BlobClass;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    float RedExplosionRadius = 100.0f;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    float RedExplosionLingerDuration = 0.5f;
};
```

**OnImpact:**
- Red: shatter sound, spawn brief damaging overlap sphere (timer-based, 0.5s lifetime), destroy.
- Green: shatter sound, spawn `AGreenBlobEnemy` at impact location, destroy.

### AGreenBlobEnemy

1HP enemy extending `ACBEnemyCharacterBase`. Bounces on timer via `LaunchCharacter`. Self-destructs after `BlobLifetime` (5s). `bSpinLaunchesAsProjectile = false` — does not chain.

---

## 7. Hazard System

Hazards are **not** `ACharacter` — simple `AActor` with mesh and damage trigger. No HP, no AI, no movement component. Not defeatable.

```cpp
UCLASS(Abstract, meta = (PrioritizeCategories = "CB"))
class CB_API AHazardBase : public AActor
{
    GENERATED_BODY()
public:
    AHazardBase();

    TObjectPtr<UStaticMeshComponent> MeshComponent;
    TObjectPtr<USphereComponent> DamageVolume;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    TObjectPtr<USoundBase> HazardSound;

protected:
    void OnDamageOverlap(...); // Player->OnHit(this)
};
```

### AFlyingFishHazard

Timer-driven parabolic arc from water surface. Damage volume only active during jump phase. Hidden below surface during rest.

### ABouncingBarrelHazard

Sinusoidal vertical oscillation from ground position via `SetActorLocation`. Always bouncing, always dangerous.

---

## 8. Integration Points with Existing Player Code

| Integration | Change Needed |
|-------------|---------------|
| Invincibility contact kill | Enemy-side only (Section 1.4). No `ACBPlayerCharacter` changes. |
| Stomp detection | Already works — `Landed()` checks `IStompable`, which enemies now implement. |
| Spin detection | Already works — `SpinAttackVolume` checks `ISpinnable`, which enemies now implement. |
| StompBounceVelocity | Enemies read `Player->StompBounceVelocity` (public, 800). Red Lizard overrides. Same pattern as crates. |
| Old HurtBox system | Deprecated but not removed. `IsEnemyJumpValid` / `JumpFromEnemyHurtBox` remain for backward compat. New enemies use `IStompable`. |

---

## 9. Debug Level: Enemy_Debug

Separate level at `Content/Maps/Debug/Enemy_Debug` with one of every enemy type for isolated testing.

| Section | Enemies |
|---------|---------|
| Patrol Lane | Crab, Skunk, Tribesman in sequence along flat corridor |
| Cycling Area | Venus Fly Trap (green), Venus Fly Trap (white), Rolling Monkey |
| Snake Pit | 3 Snake enemies in holes (SM_SnakeHole at each spawn point) |
| Bat Cave | 2 flying bats on spline loops, 1 swooping bat |
| Spider Den | 1 ceiling dropper (SM_SpiderThread visible), 1 ground jumper |
| Lizard Field | 2 Green Lizards, 1 Red Lizard (with high platform reachable via bounce) |
| Shield Alley | 2 Shield Natives patrolling |
| Ranged Range | 1 Spear Thrower, 1 Beaker Lab (red), 1 Beaker Lab (green) |
| Electric Corridor | 1 Electric Lab Assistant |
| Turtle Bridge | 3 Turtles in sequence over gap (test platform riding) |
| Hazard Zone | 3 Flying Fish staggered, 2 Bouncing Barrels |
| Chain Kill Arena | 5 enemies clustered near TNT crate |

---

## 10. AssetGen Additions

### AssetGen.cpp Buttons (new section: "Phase 3 — Enemies")

| Button | Action |
|--------|--------|
| Create Enemy Materials | All MI_ material instances from M_MASTER |
| Create Enemy Blueprints | BPs for all 10 archetypes + green blob, assigns SK_ meshes, MI_ materials, AnimBPs |
| Create Hazard Blueprints | BP_FlyingFish, BP_BouncingBarrel with SM_ meshes and MI_ materials |
| Create Projectile Blueprints | BP_Spear, BP_BeakerRed, BP_BeakerGreen with SM_ meshes and MI_ materials |
| Create Enemy Debug Level | Enemy_Debug level with gauntlet zones |

### generate_meshes.py Entries

16 new skeletal mesh entries (rigged with armatures) + 9 static mesh entries. Skeletal meshes export with armature via `apply_unit_scale=True`, `add_leaf_bones=False`.

### blender_anim.py Entries

~60 animation sequences across 16 skeletons. Each exported as a separate FBX referencing the skeleton's armature. Follows existing convention from `Rig/SKM_Manny_Simple.FBX`.

### generate_sounds.py Entries

3 new sound entries: EnemyDefeat, EnemyHit, ShieldBlock.

### Blueprint Configuration Reference

Blueprints live in `Content/Blueprints/` (the existing Blueprints folder) for easy access. Asset data (meshes, animations, materials) stays in the per-enemy module folders under `Content/Enemies/`.

| Blueprint | Parent Class | BP Path | Asset Folder | Key Config |
|-----------|-------------|---------|--------------|------------|
| BP_Crab | AEnemyPatrol | Blueprints/Enemies/ | Enemies/Crab/ | slow speed |
| BP_Skunk | AEnemyPatrol | Blueprints/Enemies/ | Enemies/Skunk/ | moderate speed |
| BP_Tribesman | AEnemyPatrol | Blueprints/Enemies/ | Enemies/Tribesman/ | moderate speed, wider capsule |
| BP_VenusFlyTrapGreen | AEnemyCycling | Blueprints/Enemies/ | Enemies/VenusFlyTrap/ | bSpikedTop=false |
| BP_VenusFlyTrapWhite | AEnemyCycling | Blueprints/Enemies/ | Enemies/VenusFlyTrap/ | bSpikedTop=true |
| BP_RollingMonkey | AEnemyCycling | Blueprints/Enemies/ | Enemies/RollingMonkey/ | bPatrolsWhileAttacking=true |
| BP_Snake | AEnemySnake | Blueprints/Enemies/ | Enemies/Snake/ | — |
| BP_BatFlying | AEnemyFlying | Blueprints/Enemies/ | Enemies/Bat/ | bIsSwooper=false |
| BP_BatSwooping | AEnemyFlying | Blueprints/Enemies/ | Enemies/Bat/ | bIsSwooper=true |
| BP_SpiderCeiling | AEnemyCeiling | Blueprints/Enemies/ | Enemies/Spider/ | — |
| BP_SpiderGround | AEnemyJumping | Blueprints/Enemies/ | Enemies/Spider/ | bDangerousWhileAirborne=true |
| BP_LizardGreen | AEnemyJumping | Blueprints/Enemies/ | Enemies/Lizard/ | — |
| BP_LizardRed | AEnemyJumping | Blueprints/Enemies/ | Enemies/Lizard/ | bExtraHighBounce=true |
| BP_ShieldNative | AEnemyShielded | Blueprints/Enemies/ | Enemies/ShieldNative/ | — |
| BP_SpearThrower | AEnemyRanged | Blueprints/Enemies/ | Enemies/SpearThrower/ | bAimAtPlayer=false |
| BP_BeakerLabAssistant | AEnemyRanged | Blueprints/Enemies/ | Enemies/LabAssistant/ | bAimAtPlayer=true |
| BP_ElectricLabAssistant | AEnemyElectric | Blueprints/Enemies/ | Enemies/ElectricLab/ | — |
| BP_Turtle | AEnemyTurtle | Blueprints/Enemies/ | Enemies/Turtle/ | — |
| BP_GreenBlob | AGreenBlobEnemy | Blueprints/Enemies/ | Enemies/GreenBlob/ | bSpinLaunchesAsProjectile=false |
| BP_FlyingFish | AFlyingFishHazard | Blueprints/Hazards/ | Hazards/FlyingFish/ | — |
| BP_BouncingBarrel | ABouncingBarrelHazard | Blueprints/Hazards/ | Hazards/BouncingBarrel/ | — |
| BP_Spear | ASpearProjectile | Blueprints/Projectiles/ | Projectiles/ | — |
| BP_BeakerRed | ABeakerProjectile | Blueprints/Projectiles/ | Projectiles/ | BeakerType=Red |
| BP_BeakerGreen | ABeakerProjectile | Blueprints/Projectiles/ | Projectiles/ | BeakerType=Green |

### Animation Notes

Each enemy gets an AnimBlueprint (ABP\_) that lives in its own `Content/Enemies/<Name>/` folder alongside the mesh, materials, and Blueprint. AnimBPs contain a simple state machine graph; all variable computation is in C++ via per-enemy `UAnimInstance` subclasses (following the existing `UCBAnimInstance` pattern). See the UE Content Paths tree above for the complete layout.

---

## 11. Implementation Order

**Group 1: Foundation Refactoring**

1. Fix PatrolRig AttackTriggerBox bug — add `SetupAttachment`, add destruction
2. Gut `ACBEnemyAIControllerBase` — remove Abstract, remove all BlueprintImplementableEvent stubs
3. Add interfaces to `ACBEnemyCharacterBase` — implement `ISpinnable`, `IStompable`, `IExplodable` with virtual defaults. Add `DefeatSound`, `SpinLaunchForce`, `bSpinLaunchesAsProjectile`, `HandleDefeat()`, `SpawnLaunchedProjectile()`.
4. Bind capsule overlap in C++ — `BeginPlay()` binding so enemies get damage overlap without BP wiring
5. Update `HitBeginOverlap` for invincibility — invincible player kills enemy on contact
6. Remove BT forwarding — remove all `Controller->OnXyz()` calls from base class

**Group 2: Core Systems**

7. Implement `ALaunchedEnemyProjectile` — sphere collision, velocity + gravity, kills enemies via `OnExplosionHit`, triggers `IExplodable` on crates
8. Implement `AProjectileBase` — abstract base, speed/arc/lifetime, overlap damages player
9. Implement `ASpearProjectile` — straight line, destroys on impact
10. Implement `ABeakerProjectile` — lobbing arc, red (explosion AoE) and green (spawns blob)
11. Implement `AGreenBlobEnemy` — 1HP, bounces on timer, 5s lifetime, `bSpinLaunchesAsProjectile = false`
12. Implement `AHazardBase` — AActor with mesh + damage volume

**Group 3: Enemy Archetypes (parallelizable)**

13. `AEnemyPatrol` — Crab, Skunk, Tribesman via BP config
14. `AEnemyCycling` — Venus Fly Trap + Rolling Monkey
15. `AEnemySnake` — hidden/emerge/lunge cycle
16. `AEnemyFlying` — flying bat + swooping bat
17. `AEnemyCeiling` — spider dropper
18. `AEnemyJumping` — spider jumper, green/red lizard
19. `AEnemyShielded` — shield dot product, bounce-back
20. `AEnemyRanged` — spear thrower, beaker lab assistant
21. `AEnemyElectric` — electric barrier, pursuit
22. `AEnemyTurtle` — flip, platform component, continues patrol

**Group 4: Hazard Types**

23. `AFlyingFishHazard` — timer-driven arc jump
24. `ABouncingBarrelHazard` — sinusoidal bounce

**Group 5: Asset Generation**

25. Add enemy mesh generators to `generate_meshes.py` — 16 skeletal (rigged) + 5 static entries
26. Add enemy animation generators to `blender_anim.py` — ~60 animation sequences across 16 skeletons
27. Add enemy sound generators to `generate_sounds.py` — 3 entries
28. Add Phase 3 buttons to `AssetGen.cpp` (including animation import and AnimBP creation)
29. Run asset generation pipeline — meshes + animations via Blender, sounds via Python, import via commandlets
30. Create all enemy/hazard/projectile Blueprints and AnimBlueprints via AssetGen or commandlets

**Group 6: Debug Level and Testing**

31. Build Enemy_Debug level with gauntlet zones
32. Integration testing — all stomp/spin/explosion interactions, chain-kill, shield, turtle, electric, projectiles
33. Audio pass — hook up all SFX to trigger points
34. Animation pass — verify state machine drives correct animation per archetype
35. Tuning pass — speeds, timers, radii, bounce heights from playtesting

---

## 12. Edge Cases & Potential Bugs

| Edge Case | Mitigation |
|-----------|------------|
| Enemy killed by invincibility + spin on same frame | `IsDead()` guard at top of all interface implementations |
| Launched projectile hits spawning enemy | Enemy destroyed before projectile spawns (destroy-then-spawn order) |
| Stomp and capsule overlap fire on same frame | `Landed()` fires in movement tick, overlaps in physics step. If stomp kills first, `IsDead()` guard prevents damage. |
| Venus Fly Trap transitions while player is mid-spin inside | `HitBeginOverlap` checks `bOnlyDangerousWhileAttacking` — only damages during Attacking |
| Turtle flips back while player stands on it | Platform collision disables → player falls. Capsule overlap re-enables → damage if player lands on it. Correct per spec. |
| Spider drops on spinning player | Spider enters spin volume → `OnSpinHit` → dies. `IsDead()` guard prevents damage. |
| Snake lunge while hidden | State machine prevents — `SetState` clears previous timers |
| Multiple enemies stomped on same frame | Only one actor from `Landed()` `Hit.GetActor()` |
| Shield Native facing away at patrol end | Shield faces movement direction — spin from player's "front" hits enemy's "behind". Correct. |
| Green blob bounces off level edge | Limited lifetime (5s). If it hits kill volume, destroyed. Otherwise expires. |
| Beaker lands on slope | `OnImpact` fires on any blocking hit regardless of surface angle |
| Chain-kill projectile hits TNT near player | TNT explosion damages player. Working as intended — same as Phase 2. |

### UE5-Specific Gotchas

| Gotcha | Detail |
|--------|--------|
| `ACharacter` + disabled gravity + `SetActorLocation` | For flying/ceiling enemies. Disabling gravity prevents CMC pull-down. `SetActorLocation` bypasses CMC — no collision resolution. Use `AddActorWorldOffset` with sweep for flying enemies that need collision. |
| `NoCollision` on capsule prevents all overlaps | When enemy dies, setting NoCollision prevents pending overlaps. Call `HandleDefeat` before disabling collision if delegates must fire. |
| Launched projectile mesh for skeletal enemies | All enemies use `USkeletalMeshComponent` but `ALaunchedEnemyProjectile` uses `UStaticMeshComponent`. Resolved: all enemies share a single generic `SM_LaunchedEnemy` mesh (bright red capsule). No per-enemy static mesh duplication needed. |
| `Destroy()` during overlap callback | Safe for self. Risky for other actor — use `SetLifeSpan(0.01f)` or guard with `IsActorBeingDestroyed()`. |
| Capsule collision profile | Must ensure overlap events with Player channel. Set in BP or constructor: `SetCollisionResponseToChannel(CBCollision::Player, ECR_Overlap)`. |

---

## 13. Tradeoffs & Decisions Summary

| Decision | Alternative | Rationale |
|----------|-------------|-----------|
| C++ enum state machines | UE Behavior Trees | BTs add complexity for 2–5 state enemies. Enum switch is 20 lines vs 5+ BT assets. KISS. |
| Per-archetype local enum | Generic FSM component | Each archetype has different states. Generic FSM adds abstraction without reducing code. |
| 10 archetype classes, BP config | One class per enemy type (18 classes) | Shared behavioral patterns. Variants differ only by mesh/speed/flags. ~6–8 fewer files. |
| Turtle has its own class | Patrol variant | Flip-to-platform is too unique for a config flag. Clean separation. |
| Launched enemy = new actor | Physics mode switch on ACharacter | Switching ACharacter to physics fights CMC. Simple actor is cleaner. |
| One chain level (no cascade) | Full cascading chains | Exponential complexity. One level matches the original game. |
| Shield blocks spin only | Blocks all attacks from front | Spec: "jump on top always works." Chain-kill uses OnExplosionHit, bypasses shield. |
| Platform component for turtle | Capsule collision swap | Capsule is Enemy channel (overlap). Can't make it BlockAll without breaking damage. Separate component isolates concerns. |
| Hazards as AActor | ACharacter subclass | Hazards have no HP, no movement, no AI. ACharacter overhead unjustified. |
| Green blob extends enemy base | Standalone simple actor | Blob IS an enemy: spinnable, stompable, needs ground movement. Base class gives that free. |
| Invincibility kill on enemy side | Player-side check | Enemy's `HitBeginOverlap` already fires. Check player state there. No player code changes needed. |
| Simple proximity trigger for Electric Lab | NavMesh/perception system | Overkill for "player in box → walk toward." AttackTriggerVolume already exists. |

---

## 14. Deferred to Later Phases

| Topic | Deferred To | Reason |
|-------|-------------|--------|
| Boss fights (Papu Papu through Cortex) | Later phase | Unique arenas, multi-phase HP, escalation patterns — different architecture |
| Enemy placement in real levels | Phase 10 (Level Construction) | Debug level sufficient for validation |
| Polished animations (blend trees, transitions, IK) | Phase 11 (Polish) | Placeholder bone transforms sufficient for Phase 3 |
| Particle effects on death/hit | Phase 11 (Polish) | Play sound + destroy is sufficient. Particles are cosmetic. |
| Enemy-specific ambient sounds | Phase 11 (Polish) | Not gameplay-critical |
| Enemy respawn on checkpoint restart | Phase 5.5 (Checkpoints) | Phase 3 uses "spawn once per level load" |
| NavMesh for pursuit AI | Not needed | `AddMovementInput` toward player sufficient for corridor levels |

---

## 15. Complete File List

### Files to Create (34 C++ files)

**Enemy Archetypes (Public + Private):**
```
Source/CB/Public/Enemy/PatrolEnemy.h
Source/CB/Private/Enemy/PatrolEnemy.cpp
Source/CB/Public/Enemy/CyclingEnemy.h
Source/CB/Private/Enemy/CyclingEnemy.cpp
Source/CB/Public/Enemy/SnakeEnemy.h
Source/CB/Private/Enemy/SnakeEnemy.cpp
Source/CB/Public/Enemy/FlyingEnemy.h
Source/CB/Private/Enemy/FlyingEnemy.cpp
Source/CB/Public/Enemy/CeilingEnemy.h
Source/CB/Private/Enemy/CeilingEnemy.cpp
Source/CB/Public/Enemy/JumpingEnemy.h
Source/CB/Private/Enemy/JumpingEnemy.cpp
Source/CB/Public/Enemy/ShieldedEnemy.h
Source/CB/Private/Enemy/ShieldedEnemy.cpp
Source/CB/Public/Enemy/RangedEnemy.h
Source/CB/Private/Enemy/RangedEnemy.cpp
Source/CB/Public/Enemy/ElectricEnemy.h
Source/CB/Private/Enemy/ElectricEnemy.cpp
Source/CB/Public/Enemy/TurtleEnemy.h
Source/CB/Private/Enemy/TurtleEnemy.cpp
Source/CB/Public/Enemy/GreenBlobEnemy.h
Source/CB/Private/Enemy/GreenBlobEnemy.cpp
```

**Chain-Kill System:**
```
Source/CB/Public/Enemy/LaunchedEnemyProjectile.h
Source/CB/Private/Enemy/LaunchedEnemyProjectile.cpp
```

**Projectile System:**
```
Source/CB/Public/Projectiles/ProjectileBase.h
Source/CB/Private/Projectiles/ProjectileBase.cpp
Source/CB/Public/Projectiles/SpearProjectile.h
Source/CB/Private/Projectiles/SpearProjectile.cpp
Source/CB/Public/Projectiles/BeakerProjectile.h
Source/CB/Private/Projectiles/BeakerProjectile.cpp
```

**Hazard System:**
```
Source/CB/Public/Hazards/HazardBase.h
Source/CB/Private/Hazards/HazardBase.cpp
Source/CB/Public/Hazards/FlyingFishHazard.h
Source/CB/Private/Hazards/FlyingFishHazard.cpp
Source/CB/Public/Hazards/BouncingBarrelHazard.h
Source/CB/Private/Hazards/BouncingBarrelHazard.cpp
```

**Enemy AnimInstances (per-archetype, drive animation variables from C++):**
```
Source/CB/Public/Enemy/Anim/PatrolEnemyAnimInstance.h
Source/CB/Private/Enemy/Anim/PatrolEnemyAnimInstance.cpp
Source/CB/Public/Enemy/Anim/CyclingEnemyAnimInstance.h
Source/CB/Private/Enemy/Anim/CyclingEnemyAnimInstance.cpp
Source/CB/Public/Enemy/Anim/SnakeEnemyAnimInstance.h
Source/CB/Private/Enemy/Anim/SnakeEnemyAnimInstance.cpp
Source/CB/Public/Enemy/Anim/FlyingEnemyAnimInstance.h
Source/CB/Private/Enemy/Anim/FlyingEnemyAnimInstance.cpp
Source/CB/Public/Enemy/Anim/CeilingEnemyAnimInstance.h
Source/CB/Private/Enemy/Anim/CeilingEnemyAnimInstance.cpp
Source/CB/Public/Enemy/Anim/JumpingEnemyAnimInstance.h
Source/CB/Private/Enemy/Anim/JumpingEnemyAnimInstance.cpp
Source/CB/Public/Enemy/Anim/ShieldedEnemyAnimInstance.h
Source/CB/Private/Enemy/Anim/ShieldedEnemyAnimInstance.cpp
Source/CB/Public/Enemy/Anim/RangedEnemyAnimInstance.h
Source/CB/Private/Enemy/Anim/RangedEnemyAnimInstance.cpp
Source/CB/Public/Enemy/Anim/ElectricEnemyAnimInstance.h
Source/CB/Private/Enemy/Anim/ElectricEnemyAnimInstance.cpp
Source/CB/Public/Enemy/Anim/TurtleEnemyAnimInstance.h
Source/CB/Private/Enemy/Anim/TurtleEnemyAnimInstance.cpp
```

Total new files: **54 C++ files** (17 gameplay + 10 anim instance = 27 header + 27 implementation)

### Files to Modify (11 files)

```
Source/CB/Public/Enemy/CBEnemyCharacterBase.h     — ISpinnable/IStompable/IExplodable, new properties
Source/CB/Private/Enemy/CBEnemyCharacterBase.cpp   — Interface defaults, HandleDefeat, invincibility, capsule binding, remove BT forwarding
Source/CB/Public/Enemy/CBEnemyAIControllerBase.h   — Remove Abstract, remove BlueprintImplementableEvent stubs
Source/CB/Private/Enemy/CBEnemyPatrolRigComponent.cpp — Fix AttackTriggerBox attachment + destruction
Plugins/AssetGen/Source/AssetGen/Private/AssetGen.cpp — Phase 3 enemy/hazard/projectile BP + AnimBP buttons
Plugins/AssetGen/Scripts/generate_meshes.py         — 16 skeletal + 9 static mesh entries
Plugins/AssetGen/Scripts/generate_sounds.py         — 3 sound generator entries
Plugins/AssetGen/Scripts/blender_anim.py            — ~60 animation sequence entries for 16 skeletons
CLAUDE.md                                           — Update enemy/projectile/hazard sections, BP layout, animation layout
```

---

## 16. Resolved Questions

| # | Question | Decision |
|---|----------|----------|
| 1 | AI approach | Pure C++ enum state machines. No behavior trees. |
| 2 | Chain kill depth | One level only. Launched enemies don't spawn further projectiles. |
| 3 | Chain kill vs TNT | Launched enemies trigger IExplodable (TNT detonates). No cascade. |
| 4 | Shield blocking scope | Blocks frontal spin only. Not stomp, not chain-kill, not explosion. |
| 5 | Turtle platform mechanism | Separate invisible UStaticMeshComponent with BlockAll, enabled on flip. |
| 6 | Electric barrier detection | Z-height comparison. Spin works only below barrier height. |
| 7 | Flying enemy movement | GravityScale = 0, SetActorLocation along spline. Not CMC walking. |
| 8 | Invincibility kill integration | Enemy-side check in HitBeginOverlap. No player changes needed. |
| 9 | Beaker aim | Aimed at player position at throw time. Fixed arc, not homing. |
| 10 | Green blob lifespan | 5 seconds, then auto-destroy. |
| 11 | Red Lizard extra bounce | Hardcoded `ExtraHighBounceVelocity = 1400`. No framework. |
| 12 | Enemy overlap binding | C++ BeginPlay, not Blueprint event graph. |
| 13 | Old HurtBox system | Deprecated but not removed. New enemies use IStompable. |
| 14 | Hazard base class | AActor, not ACharacter. No HP, no interfaces. |
| 15 | Death effects | Minimal — play sound, destroy actor. Particles/animations deferred to Polish. |
| 16 | Respawn rules | Spawn once per level load. Dead enemies stay dead until reload. |
| 17 | Class hierarchy | 10 archetype classes. Shared BP config, not 18 individual classes. |
| 18 | Non-defeatable hazards | Separate AHazardBase (AActor), not enemy subclass. |
| 19 | Debug level | Separate Enemy_Debug level, not extension of existing Debug level. |
| 20 | Audio | Shared generic sounds (EnemyDefeat, EnemyHit, ShieldBlock). Per-enemy audio deferred. |

---

## 17. Verification

### Build
```bash
"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" CBEditor Win64 Development "c:\dev\CrashBandicoot\CB.uproject" -WaitMutex 2>&1 | tail -15
```

### Asset Pipeline
```bash
blender --background --python Plugins/AssetGen/Scripts/generate_meshes.py -- Content/Assets/Meshes
python Plugins/AssetGen/Scripts/generate_sounds.py Content/Assets/Sounds
```
Then import via AssetImport commandlet and run AssetGen buttons.

### In-Editor Testing
Open Enemy_Debug level. Walk through each gauntlet zone. Verify all checklist items below.

---

## 18. Phase 3 Test Checklist

### Spin Kill
- [ ] Spin near patrol enemy → dies, launched projectile spawned
- [ ] Spin near multiple enemies → all die simultaneously
- [ ] Spin at max range (edge of SpinAttackVolume) → dies
- [ ] Spin near Venus Fly Trap during Vulnerable → dies
- [ ] Spin near Venus Fly Trap during Attacking → dies (still spinnable)
- [ ] Spin near Rolling Monkey during Rolling → no effect (invulnerable)
- [ ] Spin near Rolling Monkey during Paused → dies
- [ ] Spin near Snake during Hidden → no effect
- [ ] Spin near Snake during Emerging → dies
- [ ] Spin near Shield Native from front → blocked, player bounced back, block sound
- [ ] Spin near Shield Native from behind → normal kill
- [ ] Spin near Electric Lab Assistant below barrier → normal kill
- [ ] Spin near Electric Lab Assistant above barrier → player takes damage

### Stomp Kill
- [ ] Jump on patrol enemy → dies, player bounces
- [ ] Jump on white Venus Fly Trap → player takes damage (spiked top)
- [ ] Jump on green Venus Fly Trap during Vulnerable → dies, player bounces
- [ ] Jump on Shield Native → normal kill (shield doesn't block stomp)
- [ ] Jump on Electric Lab Assistant → player takes damage (electric barrier on top)
- [ ] Jump on Turtle → flips, player bounces, becomes platform
- [ ] Jump on Red Lizard → extra-high bounce
- [ ] Jump on Green Lizard → normal bounce

### Chain Kill
- [ ] Spin kill near another enemy → launched projectile kills second enemy
- [ ] Second killed enemy does NOT spawn further projectile (one level)
- [ ] Launched projectile hits TNT → TNT detonates
- [ ] Launched projectile passes over player → no damage
- [ ] Launched projectile hits geometry → destroyed
- [ ] Launched projectile expires → destroyed

### Turtle Platform
- [ ] Stomp → visual flips upside-down
- [ ] Player can stand on flipped turtle
- [ ] Flipped turtle continues patrol (player rides)
- [ ] After 5s, turtle flips back → player falls
- [ ] Spin kills turtle even while flipped
- [ ] Stomping already-flipped turtle → bounces, resets timer

### Shield Native
- [ ] Shield blocks spin from front (0–90° cone)
- [ ] Spin from behind → normal kill
- [ ] Player bounced back on block (no damage)
- [ ] Shield block sound plays
- [ ] Stomp bypasses shield
- [ ] Chain-kill projectile bypasses shield

### Invincibility Contact Kill
- [ ] Invincible player walks into enemy → enemy dies
- [ ] Works for all enemy types including shield and electric

### Cycling Enemies
- [ ] Venus Fly Trap cycles between open and snapping
- [ ] Contact during snapping damages player
- [ ] Rolling Monkey rolls along patrol, pauses, rolls back
- [ ] Contact during rolling damages player
- [ ] Spin/stomp during pause → kills

### Ranged Enemies
- [ ] Spear Thrower throws at intervals in fixed direction
- [ ] Spear damages player, destroyed on geometry hit
- [ ] Beaker Lab throws lobbing arc toward player
- [ ] Red beaker explodes on impact, lingering damage
- [ ] Green beaker spawns blob on impact
- [ ] Blob bounces, damages player, can be spun/stomped

### Hazards
- [ ] Flying Fish jumps from water in arc, damages on contact
- [ ] Flying Fish cannot be defeated
- [ ] Bouncing Barrel bounces in place, damages on contact
- [ ] Bouncing Barrel cannot be defeated

### Audio
- [ ] EnemyDefeat sound on every enemy death
- [ ] ShieldBlock sound on shield block
- [ ] TurtleFlip sound on turtle stomp

### Respawn
- [ ] Killed enemies stay dead for remainder of level
- [ ] Level reload respawns all enemies at original positions
