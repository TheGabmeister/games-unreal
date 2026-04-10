# Quake — Game Specification

A single-player FPS inspired by the original Quake (1996), rebuilt in Unreal Engine 5.7 using C++. The game recreates the core gameplay loop — fast movement, weapon switching, room-clearing combat, and key-locked exploration — with modernized controls and primitive-shape visuals.

## Constraints

- **C++ first, with thin Blueprint layer.** All gameplay logic lives in C++. Blueprints are used only as thin subclasses of C++ base classes for two purposes:
  1. **Asset references** — assigning meshes, materials, and (future) sound assets to `UPROPERTY` slots on C++ classes.
  2. **Per-instance parameter tuning** — tweaking values like enemy health, weapon damage, or fire rate without recompiling. Each enemy and weapon variant is a Blueprint subclass of its C++ base.

  Blueprints contain **no logic** — only property values and asset references. All behavior is in C++.
- **Primitive shapes.** All characters, enemies, weapons, and pickups are built from cubes, spheres, cylinders, and capsules. No skeletal meshes or animations.
- **No audio assets.** An audio system will be implemented with placeholder hooks so sound effects and music can be plugged in later.
- **Single player only.** No networking or multiplayer.

---

## 1. Player

### 1.1 Movement

The player character uses a first-person camera with modernized FPS controls.

| Attribute         | Value   |
|-------------------|---------|
| Walk speed        | 600     |
| Run speed         | 900     |
| Jump velocity     | 420     |
| Air control       | 0.3     |
| Health (max)      | 100     |
| Armor (max)       | 200     |

- WASD movement, mouse look, space to jump.
- No crouch (matching original Quake).
- Strafe jumping and bunny hopping should be possible via air control and momentum preservation.
- Fall damage: none (matching original Quake).

### 1.2 Health and Armor

- Health starts at 100. Can be overcharged to 200 via Megahealth, but decays back to 100 over time (1 HP/sec).
- Armor absorbs a percentage of incoming damage:
  - **Green Armor (100):** absorbs 30% of damage.
  - **Yellow Armor (150):** absorbs 60% of damage.
  - **Red Armor (200):** absorbs 80% of damage.
- Armor does not stack — picking up a new armor replaces the current one only if the new tier is higher or current armor is depleted below the new pickup's value.

### 1.3 Death and Respawn

- On death, display a death screen overlay. Player can restart from the last level start.
- No lives system.

---

## 2. Weapons

All weapons are represented by primitive shapes attached to the camera (viewmodel). Projectiles are also primitive shapes.

The player always carries the Axe. Other weapons are found as pickups. Switching weapons is instant via number keys (1-8) or scroll wheel.

| # | Weapon              | Ammo Type | Ammo/Shot | Fire Rate  | Damage     | Projectile      |
|---|---------------------|-----------|-----------|------------|------------|------------------|
| 1 | Axe                 | —         | —         | 2/sec      | 20         | Melee trace      |
| 2 | Shotgun             | Shells    | 1         | 1.5/sec    | 4x6 = 24  | Hitscan (spread) |
| 3 | Double-Barrel Shotgun| Shells   | 2         | 1/sec      | 4x14 = 56 | Hitscan (spread) |
| 4 | Nailgun             | Nails     | 1         | 8/sec      | 9          | Projectile       |
| 5 | Super Nailgun       | Nails     | 2         | 8/sec      | 18         | Projectile       |
| 6 | Grenade Launcher    | Rockets   | 1         | 1.5/sec    | 100 (direct) / splash | Arcing projectile |
| 7 | Rocket Launcher     | Rockets   | 1         | 1.5/sec    | 100 (direct) / splash | Straight projectile |
| 8 | Thunderbolt         | Cells     | 1/tick    | Continuous | 30/sec     | Trace beam       |

- **Splash damage** radius: 120 units for rockets/grenades. Damage falls off linearly with distance.
- **Rocket jumping** is supported — the player takes self-splash damage but gains momentum.
- **Grenades** bounce off surfaces and explode after 2.5 seconds, or on direct enemy contact.
- **Nails** are physical projectiles with travel time.
- **Thunderbolt** discharges all remaining cells and kills the player if used underwater.

### 2.1 Ammo

| Ammo Type | Max Carry | Small Pickup | Large Pickup |
|-----------|-----------|--------------|--------------|
| Shells    | 100       | 20           | 40           |
| Nails     | 200       | 25           | 50           |
| Rockets   | 100       | 5            | 10           |
| Cells     | 100       | 6            | 12           |

- Auto-switch: when picking up a weapon for the first time, auto-switch to it. Otherwise, stay on current weapon.

---

## 3. Enemies

All enemies are built from primitive shapes (capsules for bodies, spheres for heads, boxes for limbs). Enemy "animation" is represented by simple transforms — bobbing, rotation, scaling — rather than skeletal animation.

### 3.1 Enemy Types

| Enemy        | Health | Speed | Attack            | Damage | Behavior              |
|--------------|--------|-------|-------------------|--------|-----------------------|
| Grunt        | 30     | 300   | Hitscan (rifle)   | 4      | Ranged, patrols       |
| Knight       | 75     | 400   | Melee (sword)     | 10     | Charges player        |
| Ogre         | 200    | 250   | Grenade + Melee   | 40/20  | Ranged + melee combo  |
| Fiend        | 300    | 500   | Leap attack       | 40     | Jumps at player       |
| Shambler     | 600    | 200   | Lightning + Melee | 30/40  | Tank, half rocket dmg |
| Zombie       | 60     | 200   | Flesh throw       | 10     | Revives unless gibbed |

### 3.2 AI Behavior

Simple state-machine AI:

- **Idle** — stationary or patrolling a path.
- **Alert** — triggered by seeing or hearing the player (or being damaged). Plays alert "animation" (scale pulse), then transitions to Chase.
- **Chase** — navigates toward the player using UE NavMesh.
- **Attack** — within attack range, execute attack. Cooldown, then return to Chase.
- **Pain** — on taking damage, brief stagger (pause movement, flash red). Chance to flinch scales with damage dealt.
- **Dead** — ragdoll-style collapse (scale to flat), drop ammo/items.

Line of sight and hearing radius determine awareness. Enemies that hear combat nearby also become alert.

### 3.3 Gibs

When an enemy is killed by damage that exceeds their remaining health by 2x or more, they "gib" — the primitive shapes scatter as physics objects and fade out. Zombie gibs are the only way to permanently kill Zombies.

---

## 4. Items and Pickups

All pickups are simple floating/rotating primitive shapes with a colored glow (point light).

### 4.1 Health Pickups

| Item          | Health Restored | Visual                   |
|---------------|-----------------|--------------------------|
| Small Health  | 15              | Small green box           |
| Health Pack   | 25              | Medium green box          |
| Megahealth    | 100 (overcharge)| Large rotating gold box   |

### 4.2 Armor Pickups

| Item         | Armor Value | Absorb % | Visual              |
|--------------|-------------|----------|----------------------|
| Green Armor  | 100         | 30%      | Green shield shape    |
| Yellow Armor | 150         | 60%      | Yellow shield shape   |
| Red Armor    | 200         | 80%      | Red shield shape      |

### 4.3 Powerups

| Powerup                | Duration | Effect                                  | Visual                    |
|------------------------|----------|-----------------------------------------|---------------------------|
| Quad Damage            | 30 sec   | 4x damage multiplier                    | Blue rotating cube        |
| Pentagram of Protection| 30 sec   | Invulnerability                         | Red rotating pentagonal shape |
| Ring of Shadows        | 30 sec   | Enemies can't target player (inaccurate attacks) | Yellow transparent sphere |
| Biosuit                | 30 sec   | Immune to slime/lava damage             | Green transparent sphere  |

### 4.4 Keys

- **Silver Key** and **Gold Key** are per-level items that unlock corresponding doors.
- Consumed on use. Do not carry between levels.
- Visual: rotating metallic box (gray or gold).

---

## 5. Level Structure

### 5.1 Layout

Levels are built in the Unreal Editor using BSP brushes and/or simple static meshes. Each level contains:

- A **Player Start** point.
- Enemy **spawn points** (placed as actors in the editor).
- **Pickup placements** (placed as actors).
- **Doors** — triggered by proximity, keys, or buttons.
- **Triggers** — volumes that spawn enemies, open areas, or play events.
- **Exit trigger** — ends the level and loads the next.
- **Hazards** — slime (damage over time) and lava (heavy damage over time) floor volumes.

### 5.2 Progression

- Levels are organized in episodes. Each episode is a linear sequence of levels.
- A start hub level allows the player to choose an episode (walk into the appropriate portal).
- Completing the last level of an episode returns the player to the hub.

### 5.3 Secrets

- Hidden areas accessible by shooting walls, walking through fake geometry, or finding hidden buttons.
- Secret areas contain extra ammo, health, or powerups.
- Track secrets found vs total per level.

---

## 6. HUD

The HUD is implemented in C++ using Slate (no UMG Blueprints).

| Element           | Position       | Info                              |
|-------------------|----------------|-----------------------------------|
| Health            | Bottom-left    | Numeric display + icon            |
| Armor             | Bottom-left    | Numeric display + icon (colored by tier) |
| Current Ammo      | Bottom-right   | Numeric display for active weapon |
| Weapon Bar        | Bottom-center  | Icons for weapons 1-8, highlight active |
| Keys              | Top-right      | Silver/Gold key icons when held   |
| Powerup Timer     | Top-center     | Active powerup icon + countdown   |
| Crosshair         | Center         | Simple dot or cross               |
| Level Stats       | End-of-level   | Kills, secrets, time              |

### 6.1 Damage Feedback

- Screen flash red on taking damage, intensity scales with damage.
- Screen tint when under powerup effect (blue for Quad, red for Pentagram, etc.).
- Pickup flash (brief gold tint on item pickup).

---

## 7. Audio System

No audio assets exist yet. The system provides a clean interface for future integration.

### 7.1 Architecture

- **`UQuakeSoundManager`** (Game Instance Subsystem) — central audio manager.
- Exposes functions like `PlaySound(ESoundEvent, Location)`, `PlayMusic(EMusicTrack)`, `StopMusic()`.
- `ESoundEvent` is an enum cataloging every game sound (weapon fire, pickup, enemy alert, door open, etc.).
- Sounds are mapped to `USoundBase*` via a `UDataTable` whose row asset is assigned in a Blueprint subclass of the manager (or referenced from the Game Instance). When a row is unmapped (nullptr), the call is a no-op.
- This means all gameplay code calls the sound manager, and adding audio later is just filling in the data table rows in the Editor.

### 7.2 Sound Events (partial list)

- Player: footstep, jump, land, pain, death, pickup_item, pickup_weapon, pickup_powerup
- Weapons: axe_swing, shotgun_fire, nailgun_fire, rocket_fire, grenade_bounce, rocket_explode, thunderbolt_hum
- Enemies: alert, pain, death, attack, idle
- World: door_open, door_close, button_press, teleport, secret_found
- Music: per-level ambient tracks

---

## 8. Technical Architecture

### 8.1 Class Hierarchy

```
AQuakeGameMode          — level loading, rules, player spawning
AQuakePlayerController  — input setup, HUD ownership
AQuakeCharacter         — movement, health/armor, weapon management
AQuakeWeaponBase        — base weapon class
  AQuakeWeapon_Axe
  AQuakeWeapon_Shotgun
  AQuakeWeapon_SuperShotgun
  AQuakeWeapon_Nailgun
  AQuakeWeapon_SuperNailgun
  AQuakeWeapon_GrenadeLauncher
  AQuakeWeapon_RocketLauncher
  AQuakeWeapon_Thunderbolt
AQuakeProjectile        — base projectile (nail, rocket, grenade)
AQuakeEnemyBase         — base enemy with state machine AI
  AQuakeEnemy_Grunt
  AQuakeEnemy_Knight
  AQuakeEnemy_Ogre
  AQuakeEnemy_Fiend
  AQuakeEnemy_Shambler
  AQuakeEnemy_Zombie
AQuakePickupBase        — base pickup (health, armor, ammo, weapon, key, powerup)
AQuakeDoor              — door actor with key/trigger logic
AQuakeTrigger           — generic trigger volume
AQuakeHazardVolume      — slime/lava damage volume
UQuakeHUD               — Slate-based HUD
UQuakeSoundManager      — audio subsystem
UQuakeGameInstance       — persistent state across levels
```

### 8.2 Blueprint Layer

For each C++ class that needs assets or per-variant tuning, create a Blueprint subclass under `Content/Blueprints/`:

- **Weapons** — `BP_Weapon_Shotgun`, `BP_Weapon_Nailgun`, etc. Each holds the weapon's mesh, projectile class reference, muzzle effect, and tunable values (damage, fire rate, ammo cost).
- **Enemies** — `BP_Enemy_Grunt`, `BP_Enemy_Knight`, etc. Each holds the body/head/limb meshes, materials, and stats (health, speed, attack damage, sight range).
- **Pickups** — `BP_Pickup_Health25`, `BP_Pickup_GreenArmor`, etc. Hold mesh + material + value.
- **Projectiles** — `BP_Projectile_Rocket`, `BP_Projectile_Grenade`, etc.

Blueprints contain no nodes in the Event Graph — only property defaults and asset references. The C++ base class drives all behavior.

The game mode references the Blueprint classes (not the C++ classes directly) when spawning, via `TSubclassOf<>` properties assigned in `BP_QuakeGameMode`.

### 8.3 Existing Code

The following classes already exist and should be extended:

- `AQuakeCharacter` — first-person camera, movement parameters
- `AQuakePlayerController` — Enhanced Input setup (Move, Look, Jump actions)
- `AQuakeGameMode` — sets default pawn and controller classes

### 8.4 Build Configuration

- Module: `Quake`
- Dependencies: Core, CoreUObject, Engine, InputCore, EnhancedInput, NavigationSystem, AIModule, SlateCore, Slate, UMG
- Target: Win64 (Development/Shipping)
