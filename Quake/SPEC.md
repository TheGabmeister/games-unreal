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

The player character uses a first-person camera with Quake-style physics. There is no sprint — the player has a single ground speed cap. Strafe jumping and bunny hopping emerge from the very low air acceleration.

**Implementation note: custom `UCharacterMovementComponent` required.** UE's stock `UCharacterMovementComponent` clamps air velocity in `PhysFalling` so that `MaxWalkSpeed` is also the air speed cap, regardless of `AirControl`. Quake-style strafe jumping is fundamentally incompatible with stock CMC. The project uses a `UQuakeCharacterMovementComponent : public UCharacterMovementComponent` subclass that overrides `PhysFalling` (and `CalcVelocity` as needed) to implement the Quake air-acceleration formula: clamp the **dot product** of current velocity and wishdir, not the velocity magnitude. The ground movement parameters below map to standard CMC properties; the air parameters drive the custom physics function.

**Core movement parameters:**

| Attribute              | Value | Notes                                       |
|------------------------|-------|---------------------------------------------|
| Max ground speed       | 600   | Hard cap on horizontal velocity on ground   |
| Ground acceleration    | 6000  | Units/sec² — snappy ground control          |
| Ground friction        | 8     | Velocity decay multiplier per second        |
| Stop speed             | 100   | Below this, friction is doubled (hard stop) |
| Air acceleration       | 100   | Units/sec² — intentionally low for strafing |
| Air control            | 0.3   | UE air control multiplier                   |
| Max air speed gain     | 30    | Per air-strafe tick (enables strafe jumping)|
| Jump velocity          | 420   | Initial vertical velocity on jump           |
| Gravity                | 980   | Units/sec² (UE default)                     |
| Max walkable slope     | 44°   | Steeper surfaces are treated as walls       |
| Step height            | 45    | Auto-step over obstacles up to this height  |
| Crouch                 | none  | Matches original Quake                      |
| Fall damage            | none  | Matches original Quake                      |

**Bunny hopping rule:** if Jump is pressed within 100 ms of landing (a "hop window"), horizontal velocity is preserved and the next jump executes immediately. Holding Jump in mid-air queues the next jump for the moment of landing.

**Strafe jumping:** the low air acceleration combined with the air-strafe gain cap means the player can gain speed by holding strafe and turning the mouse in the same direction while airborne. This is intentional and a core skill expression.

**Player stats:**

| Attribute    | Value |
|--------------|-------|
| Health (max) | 100   |
| Armor (max)  | 200   |

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
- On restart, the player's inventory is restored to the **level-entry snapshot** (see 1.4).

### 1.4 Inventory Persistence

The player has a persistent inventory across levels within an episode, with these rules:

| Item       | Across levels (same episode) | On death (level restart)        | Returning to hub / new episode |
|------------|------------------------------|----------------------------------|--------------------------------|
| Weapons    | Carry                         | Restore to level-entry snapshot | Reset to starting loadout       |
| Ammo       | Carry                         | Restore to level-entry snapshot | Reset to starting loadout       |
| Armor      | Carry                         | Restore to level-entry snapshot | Reset to starting loadout       |
| Health     | Reset to 100 on level start   | Reset to 100                    | Reset to 100                    |
| Powerups   | Expire on level transition    | Cleared                         | Cleared                         |
| Keys       | Reset on level transition     | Reset                           | Reset                           |

**Starting loadout** (new game / hub return): Axe, 25 shells (no shotgun yet — first Shotgun pickup grants the weapon), 100 health, no armor, no other weapons.

**Storage location: `UQuakeGameInstance`** for **weapons, ammo, and armor only.** This data lives on the GameInstance, not on `AQuakeCharacter`, because:

- `OpenLevel` destroys and recreates the Character on every level transition.
- Player death also destroys and respawns the Character.
- The level-entry snapshot must survive both events.

`AQuakeCharacter::BeginPlay` reads the current inventory from `UQuakeGameInstance` and applies it to the spawned pawn (via `AQuakeWeaponBase` actors attached to the character). On weapon pickup or ammo change, the character writes back to the GameInstance immediately.

**Storage location: `AQuakePlayerState`** for **keys** and **active powerups.** Although keys are colloquially "inventory," their lifecycle (reset on level transition, reset on death, reset on hub return) is identical to powerups, not to weapons/ammo/armor. See section 4.4 for the full key rules and 4.3 for powerups.

**PlayerState lifecycle — important.** UE's standard PlayerState is owned by the PlayerController and only **partially** auto-resets. The matrix:

| Event             | Happens to PlayerState                    | Powerups / keys behavior                            |
|-------------------|-------------------------------------------|-----------------------------------------------------|
| `OpenLevel`       | Destroyed and recreated with the world    | Cleared automatically (new instance has empty state)|
| Hub return        | Same as `OpenLevel` (it's a level load)   | Cleared automatically                                |
| Death-respawn     | **Persists** (only the Pawn is recreated) | **NOT cleared automatically — explicit reset required** |

The death-respawn case is the gotcha: standard UE keeps the PlayerController and PlayerState across pawn respawn so player identity (name, score, team) is preserved. For Quake's "die and reset" semantics, the death/restart flow in [section 6.4](SPEC.md#L868) must explicitly call `AQuakePlayerState::ClearPerLifeState()` (which empties the powerups array, clears keys, and resets the death-counter increment is **not** included — deaths are cumulative across the level attempt). The `OpenLevel` cases don't need this — they get a fresh PlayerState for free.

```cpp
// AQuakePlayerState.h
void ClearPerLifeState();  // empties ActivePowerups; clears Keys; called from death-restart in 6.4

// AQuakePlayerState.cpp
void AQuakePlayerState::ClearPerLifeState()
{
    ActivePowerups.Empty();
    Keys.Reset();
    // Kills, Secrets, TimeElapsed, Deaths persist across this level's attempts.
}
```

**Level-entry snapshot:** when the player crosses a level boundary, `UQuakeGameInstance` snapshots the inventory state immediately after the transition (i.e., after weapons/ammo carried over but health was topped up to 100). This snapshot is what death restores to. The snapshot is a separate `FQuakeInventorySnapshot` struct field on the GameInstance, distinct from the live inventory.

### 1.5 Damage Pipeline

All damage flows through UE's built-in `AActor::TakeDamage` interface. Custom code never reads health directly on a target; it calls `UGameplayStatics::ApplyPointDamage`, `ApplyRadialDamage`, or `ApplyDamage` and lets the target's `TakeDamage` override decide what happens.

**`UDamageType` subclasses** carry per-source metadata. They are stateless tag classes (the engine itself documents `UDamageType` as "immutable data holders... never stateful"); each one is a `UCLASS()` defined in C++ whose constructor sets default values.

**Shared base.** All Quake damage types inherit from a single abstract base, `UQuakeDamageType : public UDamageType`, which declares **every** Quake-specific field as a `UPROPERTY(EditDefaultsOnly)`. Leaf subclasses add no new properties — they only override defaults in their constructor. This is the consolidation that keeps `TakeDamage` free of `if (IsA<Lightning>) ... else if (IsA<Drown>) ...` ladders: the override does one cast to `UQuakeDamageType` and reads fields uniformly.

```cpp
// QuakeDamageType.h
UCLASS(Abstract)
class UQuakeDamageType : public UDamageType
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly) bool  bIgnoresArmor    = false;
    UPROPERTY(EditDefaultsOnly) bool  bSuppressesPain  = false;
    UPROPERTY(EditDefaultsOnly) bool  bBypassesBiosuit = false;
    UPROPERTY(EditDefaultsOnly) bool  bSelfDamage      = true;
    UPROPERTY(EditDefaultsOnly) float SelfDamageScale  = 1.0f;
    UPROPERTY(EditDefaultsOnly) float KnockbackScale   = 1.0f;
};

// QuakeDamageType_Explosive.h
UCLASS()
class UQuakeDamageType_Explosive : public UQuakeDamageType
{
    GENERATED_BODY()
public:
    UQuakeDamageType_Explosive()
    {
        SelfDamageScale = 0.5f;
        KnockbackScale  = 4.0f;
    }
};
```

Reuse the engine's existing `UDamageType` fields where they fit instead of inventing parallel ones — `bCausedByWorld` (set true on Lava/Slime/Drown), `bScaleMomentumByMass`, `DamageImpulse`, and `DamageFalloff` are already provided by the base.

| DamageType                        | Constructor overrides (delta from `UQuakeDamageType` defaults)                                  |
|-----------------------------------|-------------------------------------------------------------------------------------------------|
| `UQuakeDamageType_Melee`          | (defaults)                                                                                      |
| `UQuakeDamageType_Bullet`         | (defaults)                                                                                      |
| `UQuakeDamageType_Nail`           | (defaults)                                                                                      |
| `UQuakeDamageType_Explosive`      | `SelfDamageScale = 0.5`, `KnockbackScale = 4.0`. Splash radius lives on the weapon, not here.   |
| `UQuakeDamageType_Lightning`      | `bIgnoresArmor = true`                                                                          |
| `UQuakeDamageType_Lava`           | `bSuppressesPain = true`, `bCausedByWorld = true`                                               |
| `UQuakeDamageType_Slime`          | `bCausedByWorld = true`                                                                         |
| `UQuakeDamageType_Drown`          | `bIgnoresArmor = true`, `bBypassesBiosuit = true`, `bCausedByWorld = true`                      |
| `UQuakeDamageType_Telefrag`       | `bSuppressesPain = true`. Damage amount (10000) is passed by the caller, not stored here.       |

Adding a new damage source = adding one ~10-line subclass that overrides defaults in its constructor and calling `ApplyXDamage(..., UMyType::StaticClass(), ...)`. Nothing else changes.

**Standard damage parameters** (`Instigator`, `DamageCauser`) are used to attribute hits:

- **Self-damage** is detected when `DamagedActor == EventInstigator->GetPawn()` and the damage type's `bSelfDamage` flag is true. The damage type also provides the self-damage scalar (0.5 for explosives, 1.0 otherwise).
- **Knockback** is computed in `TakeDamage` from the hit normal (point damage) or the explosion origin (radial damage), using the damage type's `KnockbackScale`.
- **Infighting attribution** uses `EventInstigator` to identify the attacker. When an enemy's `TakeDamage` runs and the instigator is another enemy, target switching kicks in (see 3.3).

In `TakeDamage`, read damage-type fields via the CDO of the shared base — never branch on leaf class identity:

```cpp
const UQuakeDamageType* DT = Cast<UQuakeDamageType>(
    DamageEvent.DamageTypeClass
        ? DamageEvent.DamageTypeClass->GetDefaultObject()
        : UQuakeDamageType::StaticClass()->GetDefaultObject());
```

`AQuakeCharacter::TakeDamage` and `AQuakeEnemyBase::TakeDamage` are the only places where health is decremented. All weapons, projectiles, and hazard volumes call `ApplyPointDamage` / `ApplyRadialDamage` and let the targets handle the result.

### 1.6 Collision Model

Collision configuration is shared between several systems (projectiles, hitscan, pickups, hazards, doors, triggers) and easier to get right once than to rediscover per system. This section defines the channels, the response matrix, and the per-actor rules. All channels are added to `Config/DefaultEngine.ini` and version-controlled.

**Player capsule dimensions:** radius 35, half-height 90. NavMesh agent radius 35 / height 180 matches (see [CLAUDE.md](CLAUDE.md)).

**Custom object channels** (added on top of the engine defaults `WorldStatic`, `WorldDynamic`, `Pawn`, `PhysicsBody`, `Vehicle`, `Destructible`, `Visibility`, `Camera`):

| Channel      | Used by                                                                 |
|--------------|-------------------------------------------------------------------------|
| `Pickup`     | `AQuakePickupBase`'s `USphereComponent` (overlap-only)                  |
| `Projectile` | `AQuakeProjectile_*` actors' `USphereComponent`                         |
| `Corpse`     | `AQuakeEnemyBase` capsule **after** the 2 s post-death channel flip     |

**Custom trace channel:** `Weapon` — used by hitscan weapons (Shotgun, SSG, Thunderbolt) and the Axe melee trace.

**Object response matrix.** Rows are the *source* (this object's `CollisionObjectType`); columns are the *target* (the response on this object to that other object's channel). `B` = Block, `O` = Overlap, `I` = Ignore.

|                | WorldStatic | WorldDynamic | Pawn | Pickup | Projectile | Corpse | Visibility (trace) | Weapon (trace) |
|----------------|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
| **Player capsule** (`Pawn`)        | B | B | B | O | B | I | B | B |
| **Enemy capsule** (`Pawn`)         | B | B | B | I | B | I | B | B |
| **Corpse capsule** (`Corpse`)      | B | B | I | I | I | I | I | I |
| **Projectile sphere** (`Projectile`)| B | B | B | I | I | B | I | I |
| **Pickup sphere** (`Pickup`)       | I | I | O | I | I | I | I | I |
| **Hazard volume** (`WorldDynamic`)  | I | I | O | I | I | O | I | I |
| **Water volume** (`WorldDynamic`)   | I | I | O | I | I | I | I | I |
| **Trigger – player-only** (`WorldDynamic`) | I | I | O (player only — see notes) | I | I | I | I | I |
| **Trigger – any pawn** (`WorldDynamic`)    | I | I | O | I | I | O | I | I |
| **Door (closed)** (`WorldDynamic`)  | B | B | B | I | B | I | B | B |
| **Door (open / opening)** (`WorldDynamic`) | B | B | I | I | I | I | B | I |

**Per-system rules** (the parts the matrix can't express):

1. **Projectile spawn-out distance.** Projectile actors spawn 60 units in front of the firing pawn's muzzle, not at the muzzle itself. This prevents the muzzle-flash-frame self-detonation that breaks the Rocket Launcher otherwise. The first frame the projectile also calls `IgnoreActorWhenMoving(Instigator, true)` as a belt-and-braces second guard; this ignore is left in place for the projectile's lifetime since rockets shouldn't bounce off the firer mid-flight either.
2. **Corpse channel flip.** When `AQuakeEnemyBase::Die()` runs, the capsule keeps its `Pawn` channel for 2 seconds (so a freshly-dead body still blocks the player and reads as a hit target during gib chains). After 2 s, a timer flips the capsule to the `Corpse` channel. The `Corpse` channel ignores `Projectile`, `Weapon` (hitscan), `Pawn`, **and `Visibility`** — meaning rockets, hitscan, the player, and splash-damage line-of-sight queries (rule 10 below) all pass straight through corpses. Without the `Visibility` ignore, a corpse capsule would shield nearby still-living enemies from rocket splash damage, which contradicts the "corpses no longer affect combat" intent. Corpses still block `WorldStatic` so they don't fall through floors.
3. **Gibbed enemies.** On gib, the capsule is destroyed entirely along with the actor; the scattered primitive pieces are physics objects on the `PhysicsBody` channel and don't participate in damage at all.
4. **Pickup overlap is player-only.** The pickup's sphere overlaps the `Pawn` channel, but the C++ overlap handler casts the overlapping actor to `AQuakeCharacter` and bails on null. Enemies *trigger* the overlap (no harm) but never *consume* the pickup. This is simpler than adding a "PlayerPawn" sub-channel and produces the same end behavior.
5. **Player-only triggers.** `AQuakeTrigger_Teleport`, `AQuakeTrigger_Message`, and the level `ExitTrigger` use the same player-only filter pattern as pickups: overlap on `Pawn`, cast to `AQuakeCharacter` in the handler, bail on null. Listed as "player only" in the matrix above for documentation but mechanically it's a C++-side filter, not a channel difference.
6. **Hazard volumes** (`AQuakeHazardVolume`) overlap any pawn (player and enemies) and apply damage on the tick interval defined in [section 5.2](SPEC.md#L411). Pushing enemies into hazards is a valid tactic, so the matrix overlaps both `Pawn` and `Corpse` (lava can still cook a fresh corpse during the 2 s window before it goes non-collidable, which is fine and looks correct).
7. **Doors block projectiles when closed.** A closed door is a solid `WorldDynamic` actor and blocks the `Projectile` channel — rockets explode on it, not through it. While opening or open, the door's collision is set to `Ignore` for `Pawn`, `Projectile`, and `Weapon` so pawns and shots pass through unobstructed. The "door won't close while a pawn is inside" check from [section 5.4](SPEC.md#L469) uses a sweep against the `Pawn` channel.
8. **Water volume detection.** `AQuakeWaterVolume` overlaps `Pawn`. The "submerged" / "partially in water" distinction in [section 5.3](SPEC.md#L437) is computed in C++ from the camera position relative to the volume bounds, not from a separate channel.
9. **Hitscan trace channel.** Shotgun, SSG, and Thunderbolt all trace on the `Weapon` channel. `Weapon` traces block on walls (`WorldStatic`/`WorldDynamic`), pawns, and closed doors; ignore everything else. They specifically ignore `Pickup` (you can't shoot a Quad cube to detonate it) and `Corpse` (so wall-of-bodies doesn't block your shots — same end result as the channel flip but redundant for safety). The Axe melee trace uses the same `Weapon` channel.
10. **Splash damage queries.** `ApplyRadialDamageWithFalloff` uses the `Visibility` channel by default for the line-of-sight checks that determine which actors inside the radius actually take damage. This is the engine default and what we want — explosions don't penetrate walls.

**Where this is configured.** All channels and the default object responses live in `Config/DefaultEngine.ini` under the `[/Script/Engine.CollisionProfile]` section. Per-actor overrides happen in C++ constructors (`Capsule->SetCollisionResponseToChannel(...)`, `Sphere->SetCollisionProfileName(...)`). No collision profile assets are created in the Editor — the C++ constructors are the source of truth, matching the project's "C++ first" rule.

---

## 2. Weapons

All weapons are represented by primitive shapes attached to the camera (viewmodel). Projectiles are also primitive shapes.

The player always carries the Axe. Other weapons are found as pickups. Switching weapons is instant via number keys (1-8) or scroll wheel.

**Class layout.** Each weapon is a C++ subclass of `AQuakeWeaponBase` (`AQuakeWeapon_Shotgun`, `AQuakeWeapon_Nailgun`, etc.) that sets all stats from the table below as `UPROPERTY` defaults in its constructor. Each C++ subclass has a thin BP subclass (`BP_Weapon_Shotgun`, etc.) that holds **only** the asset slot assignments — viewmodel `UStaticMesh*`, `UMaterialInstance*` of `M_QuakeBase`, fire/empty `USoundBase*` slots, and the `TSubclassOf<AQuakeProjectile>` for projectile weapons. **Zero nodes in the BP event graph.** See section 9.2 for the underlying convention.

### 2.0 Weapon Table

| # | Weapon                | Ammo    | /Shot | RoF    | Dmg/Hit             | Range/Speed       | Spread             | Knockback | Projectile        |
|---|-----------------------|---------|-------|--------|---------------------|-------------------|--------------------|-----------|-------------------|
| 1 | Axe                   | —       | —     | 2/sec  | 20                  | 64 units          | —                  | 0         | Melee trace       |
| 2 | Shotgun               | Shells  | 1     | 1.5/s  | 4 × 6 pellets = 24  | Hitscan, 4096     | 4° cone            | 50        | Hitscan           |
| 3 | Double-Barrel Shotgun | Shells  | 2     | 1/sec  | 4 × 14 pellets = 56 | Hitscan, 4096     | 8° cone            | 100       | Hitscan           |
| 4 | Nailgun               | Nails   | 1     | 8/sec  | 9                   | 1500 u/s          | 1° cone            | 10        | Projectile        |
| 5 | Super Nailgun         | Nails   | 2     | 8/sec  | 18                  | 1500 u/s          | 1° cone            | 20        | Projectile        |
| 6 | Grenade Launcher      | Rockets | 1     | 1.5/s  | 100 direct + splash | 800 u/s, arc      | —                  | 400       | Arcing, bouncy    |
| 7 | Rocket Launcher       | Rockets | 1     | 1.5/s  | 100 direct + splash | 1000 u/s, straight| —                  | 400       | Straight          |
| 8 | Thunderbolt           | Cells   | 1/tick| 10/sec | 30 per tick         | Hitscan, 600      | —                  | 0         | Continuous beam   |

### 2.1 Ammo

| Ammo Type | Max Carry | Small Pickup | Large Pickup |
|-----------|-----------|--------------|--------------|
| Shells    | 100       | 20           | 40           |
| Nails     | 200       | 25           | 50           |
| Rockets   | 100       | 5            | 10           |
| Cells     | 100       | 6            | 12           |

### 2.2 General Weapon Rules

- **Weapon switch time:** 0.2 seconds (lower) + 0.2 seconds (raise). During the raise, fire input is queued so the first shot fires the instant the weapon is ready.
- **Auto-switch on first pickup:** picking up a weapon for the first time auto-switches to it.
- **Duplicate weapon pickup:** if the player already owns the weapon, the pickup grants the weapon's ammo (equivalent to a small ammo pickup of the matching type) but does not switch weapons.
- **Empty ammo behavior:** firing with insufficient ammo plays a "click" sound and auto-switches to the next-best owned weapon that has ammo, in priority order: RL → SNG → SSG → NG → SG → Axe. Thunderbolt and GL are skipped by auto-switch (kept manual to avoid accidental switching).
- **Splash damage:** 120 unit radius for rockets and grenades. Damage falls off linearly from full at center to 0 at the radius edge.
- **Self-damage scale:** 0.5 — the player takes 50% of self-inflicted splash damage. This makes rocket jumping survivable while still costing health.
- **Knockback:** applied as an instantaneous impulse along the hit normal (or away from the explosion center). Splash knockback is full at center, falls off linearly to 0 at radius edge. Splash applies to the player as well, enabling rocket jumps.
- **Grenades** bounce off surfaces (restitution 0.4) and explode after 2.5 seconds, or on direct enemy contact, or on direct player contact.
- **Nails** are physical projectiles with travel time and gravity-free flight.
- **Thunderbolt** is a continuous beam that deals damage in 0.1-second ticks while fire is held. Range is short (600 units) but it ignores armor for full damage.

**Projectile implementation: `UProjectileMovementComponent`.** All projectiles (`AQuakeProjectile_Nail`, `_Rocket`, `_Grenade`) use UE's built-in `UProjectileMovementComponent`. The spec values map directly to component properties:

| SPEC value          | `UProjectileMovementComponent` property              |
|---------------------|-------------------------------------------------------|
| Projectile speed    | `InitialSpeed`, `MaxSpeed`                           |
| Gravity-free flight | `ProjectileGravityScale = 0` (nails)                 |
| Arc                 | `ProjectileGravityScale = 1` (grenades, Ogre lobs)   |
| Bouncing grenades   | `bShouldBounce = true`, `Bounciness = 0.4`           |
| Bounce events       | `OnProjectileBounce` delegate (sound, fuse check)    |

Projectile actors carry a `USphereComponent` collision and the `UProjectileMovementComponent`. On hit, the projectile calls `UGameplayStatics::ApplyPointDamage` (direct hit) and/or `ApplyRadialDamageWithFalloff` (splash) using the appropriate `UDamageType` subclass from section 1.5, then destroys itself (or schedules a delayed destroy for explosion VFX).

### 2.3 Underwater Discharge

If the player fires the Thunderbolt while at least partially submerged in a `AQuakeWaterVolume`, the weapon **discharges**: all remaining cells are consumed in a single instantaneous explosion centered on the player. Damage = `cells × 10`, splash radius = 256 units, applied to the player and all enemies in the water volume regardless of line of sight. The player almost always dies. (See section 5.4 for water volumes.)

---

## 3. Enemies

All enemies are built from primitive shapes (capsules for bodies, spheres for heads, boxes for limbs). Enemy "animation" is represented by simple transforms — bobbing, rotation, scaling — rather than skeletal animation.

**Class layout.** Each enemy is a C++ subclass of `AQuakeEnemyBase` (`AQuakeEnemy_Grunt`, `AQuakeEnemy_Knight`, etc.) that sets all stats from the table below — HP, Speed, Sight, Hearing, attack damage, cooldown — as `UPROPERTY` defaults in its constructor, along with `AIControllerClass = AQuakeAIController_<Type>::StaticClass()`. Each C++ pawn subclass has a thin BP subclass (`BP_Enemy_Grunt`, etc.) that holds **only** the asset slot assignments — body/head/limb `UStaticMesh*` primitives, body color `UMaterialInstance*`, pain/death `USoundBase*` slots, and the drop table's `TSubclassOf<AQuakePickupBase>` entries (see 3.2). **Zero nodes in the BP event graph.** AIController subclasses are pure C++ with no BP layer (no asset slots). See section 9.2 for the underlying convention.

### 3.1 Enemy Types

| Enemy    | HP  | Speed | Sight | Hearing | Attack Type        | Range     | Atk Dmg     | Cooldown | Proj Speed | Notes                                   |
|----------|-----|-------|-------|---------|--------------------|-----------|-------------|----------|------------|-----------------------------------------|
| Grunt    | 30  | 300   | 2000  | 1500    | Hitscan rifle      | 1500      | 4           | 1.5 s    | —          | Patrols, takes cover (peeks)            |
| Knight   | 75  | 400   | 2000  | 1500    | Melee swing        | 80        | 10          | 1.0 s    | —          | Charges directly                        |
| Ogre     | 200 | 250   | 2500  | 2000    | Grenade + chainsaw | 96 / 1500 | 20 / 40 spl | 2.0 s    | 600 (arc)  | Lobs grenades; melee in close           |
| Fiend    | 300 | 500   | 3000  | 2500    | Leap                | leap 800  | 40          | 2.5 s    | —          | Leap jump initiates outside leap range  |
| Shambler | 600 | 200   | 3000  | 2500    | Lightning + claws  | 1200 / 96 | 30 / 40     | 2.0 s    | hitscan    | Takes 0.5× rocket/grenade damage        |
| Zombie   | 60  | 200   | 1500  | 1000    | Flesh chunk lob    | 800       | 10          | 2.0 s    | 600 (arc)  | Revives 5 s after non-gib death         |

### 3.2 Drop Tables

When an enemy dies and is not gibbed, it has a chance to drop items:

| Enemy    | Drop                         | Chance |
|----------|------------------------------|--------|
| Grunt    | Backpack: 5 shells           | 100%   |
| Knight   | None                         | 0%     |
| Ogre     | Backpack: 2 rockets          | 100%   |
| Fiend    | None                         | 0%     |
| Shambler | None                         | 0%     |
| Zombie   | None                         | 0%     |

Backpacks despawn after 60 seconds if not picked up.

**Representation.** Drops live as a `UPROPERTY` array on `AQuakeEnemyBase`, not in a `UDataTable`. The roster is small and fixed, the data has no balance interplay across enemies, and keeping it next to the rest of the per-enemy stats matches the project's broader pattern (enemies, weapons, and damage types are all subclass-per-type with `UPROPERTY` defaults — the only `UDataTable` in the project is `DT_SoundEvents` in section 8).

```cpp
// QuakeEnemyBase.h
USTRUCT(BlueprintType)
struct FQuakeDropEntry
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly) TSubclassOf<AQuakePickupBase> PickupClass;
    UPROPERTY(EditDefaultsOnly) int32 Quantity = 1;
    UPROPERTY(EditDefaultsOnly, meta=(ClampMin="0.0", ClampMax="1.0")) float Chance = 1.0f;
};

UCLASS(Abstract)
class AQuakeEnemyBase : public ACharacter
{
    GENERATED_BODY()
protected:
    UPROPERTY(EditDefaultsOnly, Category="Drops")
    TArray<FQuakeDropEntry> DropTable;
};
```

Each leaf enemy subclass (`AQuakeEnemy_Grunt`, `AQuakeEnemy_Ogre`, etc.) sets its `DropTable` defaults in its C++ constructor. The `TSubclassOf<AQuakePickupBase>` slots are assigned in the thin BP subclass (`BP_Grunt`, `BP_Ogre`) — the same asset-slot pattern used elsewhere in the project. Enemies with no drops leave the array empty.

**Evaluation.** On death (non-gibbed only — see section 3.4), `AQuakeEnemyBase` iterates `DropTable`, rolls `FMath::FRand()` once per entry against `Chance`, and for each entry that passes spawns `PickupClass` at the pawn's location with `SetQuantity(Entry.Quantity)`. Multiple drop entries on one enemy are independent rolls. Gibbed enemies skip drops entirely.

### 3.3 AI Behavior

**Architecture: AI lives on `AAIController`, not on the pawn.**

- `AQuakeEnemyBase : public ACharacter` is the **body**: capsule, mesh primitives, movement component, health, `TakeDamage` override. It exposes action methods like `MoveToTarget(FVector)`, `FireAtTarget(AActor*)`, `PlayPainReaction()`, `PlayDeathReaction()` that the controller calls.
- `AQuakeEnemyAIController : public AAIController` is the **brain**: state machine, target tracking, perception. One subclass per enemy type when behavior diverges (`AQuakeAIController_Grunt`, `_Ogre`, `_Fiend`, etc.). The controller is set on the pawn via `AIControllerClass` in the constructor of each enemy subclass.

This split enables the AI debugger (`'` key in PIE), allows the same pawn body to be possessed by different controllers (e.g., a debug "spectate" controller), and follows UE conventions.

**State machine** runs on the AIController in a custom `Tick`-driven FSM (no Behavior Trees): `Idle → Alert → Chase → Attack → (Pain) → Dead`.

**State definitions:**

- **Idle** — stationary or patrolling a placed path. Perception updates evaluated as they arrive.
- **Alert** — triggered by sight, hearing, or damage. The controller commands the pawn to play a 0.5 s alert pulse (scale animation + sound stub), then transitions to Chase.
- **Chase** — controller calls `AAIController::MoveToActor` (or `MoveToLocation`) toward the perceived target. If target is within attack range and has line of sight, transition to Attack.
- **Attack** — execute the enemy's attack pattern via the pawn's action methods. Cooldown. Return to Chase. The pawn is held still during the attack windup.
- **Pain** — on taking damage, the controller may briefly suspend the FSM and call `PlayPainReaction` (pause movement 0.3 s, flash red). Pain chance = `min(0.8, damage / max_health × 2)`. Bosses are immune to pain flinch.
- **Dead** — controller stops; pawn collapses (scale to flat), drops loot per drop table, becomes non-collidable to projectiles after 2 s, and unpossesses the controller.

**Awareness via `UAIPerceptionComponent`:**

The controller owns a `UAIPerceptionComponent` configured with two senses:

- **`UAISenseConfig_Sight`** — sight radius and lose-sight radius set per enemy from section 3.1's Sight column. Peripheral vision angle: 90° (180° FOV cone). Stimulus aging: stimulus is "remembered" for 5 seconds after losing sight, allowing the enemy to chase to last known position.
- **`UAISenseConfig_Hearing`** — hearing range set per enemy from section 3.1's Hearing column. Walls do **not** block hearing (matching Quake). `bUseLoSHearing` was deprecated in UE 5.7 and its default already matches the Quake behavior (walls do not block), so it is not set explicitly. Noise events are reported via `UAISense_Hearing::ReportNoiseEvent` whenever the player fires any weapon (the Axe included, matching original Quake) or takes damage.

The controller binds `OnTargetPerceptionUpdated` to drive transitions: a fresh sight or hearing stimulus on the player promotes the controller from Idle to Alert and stores the target.

**Damage as a perception trigger:** the enemy's `TakeDamage` override notifies its controller via a `OnDamaged(EventInstigator)` call. The controller treats this as an instant high-priority alert, identifies the instigator as the current target, and skips the alert pulse (cuts straight to Chase or Attack).

**Aggro propagation:** when a controller enters Alert via sight or damage, it calls a static helper that queries `AQuakeEnemyAIController` instances within 600 units and forwards the same alert. (No game-wide event bus required — direct iteration over the GameMode's enemy list.)

**No leash:** once alerted, controllers pursue indefinitely. They do not return to spawn.

**Target selection and infighting:**

- The default target is always the player.
- **Friendly fire is enabled at the damage layer.** If enemy A damages enemy B (via splash, stray projectile, or direct hit), B's `TakeDamage` runs, sees `EventInstigator` is another enemy, and asks its controller to switch targets. The grudge lasts 10 seconds (or until A dies or B can no longer see A for 5 s, whichever comes first). After that, B reverts to targeting the player.
- This produces emergent **infighting**: e.g., shooting an Ogre's grenade so it lands near a Shambler causes the Shambler to attack the Ogre.
- **Hazard volumes** (slime, lava, water with damage) damage enemies the same as the player. Pushing enemies into hazards is a valid tactic. Hazard damage uses a hazard `EventInstigator` (none) and does not trigger infighting.

**Vertical handling:**

- AI navigates only across NavMesh-walkable surfaces by default.
- For gaps and ledges, level designers place `NavLinkProxy` actors to mark jump-down or jump-up routes.
- The **Fiend** controller has its own leap logic: when in Chase state and within leap range (300–800 units), with the player above or at the same height, it commits a leap regardless of NavMesh links.
- Flying enemies are out of scope for v1.

### 3.4 Gibs

When an enemy is killed by an attack whose damage exceeds their remaining health by 2× or more, they "gib" — the primitive shapes scatter as physics objects, leave a brief blood splatter (red emissive decal stub), and fade out after 5 seconds. Gibbed enemies do not drop loot and do not run their death state.

**Zombies** must be gibbed to be permanently killed. A non-gib death puts the Zombie into a `Down` state for 5 seconds, after which they revive at full health and re-enter Chase. A gibbed Zombie is permanently dead.

---

## 4. Items and Pickups

All pickups are simple floating/rotating primitive shapes with a colored glow (point light).

**Implementation: `AQuakePickupBase`** holds a `USphereComponent` (overlap-only, no block) sized to the pickup's pickup radius (default 64 units), a `UStaticMeshComponent` for the visual primitive, and a `UPointLightComponent` for the glow. Detection uses `OnComponentBeginOverlap` on the sphere component, bound in `BeginPlay`. The handler checks the overlapping actor is `AQuakeCharacter`, validates the pickup is permitted (e.g., health pickup is only consumed if `Health < Max`), applies the effect, plays the pickup sound stub, and destroys the pickup actor. No tick-based polling.

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

| Powerup                | Duration | Effect                                            | Visual                        |
|------------------------|----------|---------------------------------------------------|-------------------------------|
| Quad Damage            | 30 sec   | 4× outgoing weapon damage (does not affect splash self-damage scale) | Blue rotating cube            |
| Pentagram of Protection| 30 sec   | Full invulnerability — incoming damage = 0        | Red rotating pentagonal shape |
| Ring of Shadows        | 30 sec   | Enemies cannot acquire the player as a target via sight or hearing. Already-aggro'd enemies still attack the player's current position but their projectiles are aimed at the player's last known position with a 30° angular error. Melee attacks are unaffected. | Yellow transparent sphere |
| Biosuit                | 30 sec   | Immune to slime/lava damage. Does **not** prevent drowning. | Green transparent sphere      |

**Stacking and refresh rules:**

- **Different powerups stack.** Quad + Pentagram = full invulnerability while dealing 4× damage. All effects apply simultaneously.
- **Same powerup refreshes additively, capped at 60 seconds.** Picking up a Quad Damage with 20 seconds left grants 30 + 20 = 50 seconds. Picking up another with 50 seconds left grants 60 seconds (capped), not 80.
- Powerups expire on level transition (see 1.4).
- The HUD displays each active powerup's remaining time independently (see section 7).

**Storage: `AQuakePlayerState`.** Active powerups live as a `TArray<FQuakeActivePowerup>` on the PlayerState, where each entry holds the powerup type and remaining time. The PlayerState ticks the timers and removes entries when they reach zero. PlayerState is destroyed on level transition (matching the "expire on level transition" rule) and on death-restart (matching the "cleared on death" rule), so the lifecycle is automatic — no extra cleanup code is needed. Powerups are restored from `UQuakeSaveGame` when loading mid-level.

### 4.4 Keys

- **Silver Key** and **Gold Key** unlock the corresponding colored doors in the current level.
- **Keys are NOT consumed on use.** One Silver Key opens every Silver door in the level for the rest of the level. This matches original Quake behavior.
- Keys reset on level transition — they do not carry between levels.
- Visual: rotating metallic box (gray for Silver, gold for Gold).
- Picking up a key the player already holds is a no-op (no message, no effect).

**Storage: `AQuakePlayerState`.** Keys live on the PlayerState, not on `UQuakeGameInstance`, because their lifecycle (reset on level transition, reset on death, reset on hub return) matches powerups (section 4.3). `OpenLevel` and hub returns clear the PlayerState automatically (the world tears down and a fresh PlayerState is created). **Death-respawn does NOT auto-clear** — UE keeps the PlayerController and PlayerState across pawn respawn — so the death/restart flow must call `AQuakePlayerState::ClearPerLifeState()` explicitly (see section 1.4 for the lifecycle table and section 6.4 for the call site). Door key checks call `Player->GetPlayerState<AQuakePlayerState>()->HasKey(Color)`. Key pickups call `Player->GiveKey(Color)` which forwards to the PlayerState, matching the Character-as-facade pattern used elsewhere for state writes.

---

## 5. Level Structure

### 5.1 Layout

Levels are built in the Unreal Editor using BSP brushes and/or simple static meshes. Each level contains:

- A **Player Start** point.
- Enemy **spawn points** — `AQuakeEnemySpawnPoint` actors (see below).
- **Pickup placements** (placed as actors).
- **Doors** — triggered by proximity, keys, or buttons (see 5.4).
- **Buttons** — interactable actors that fire trigger events (see 5.5).
- **Triggers** — volumes that spawn enemies, open areas, or play events (see 5.6).
- **Exit trigger** — ends the level and loads the next.
- **Hazards** — slime (damage over time) and lava (heavy damage over time) volumes.
- **Water volumes** — swimmable, with drowning timer (see 5.3).

**`AQuakeEnemySpawnPoint`.** A passive marker actor placed in the level that defines where an enemy should appear. **This is the only authoring path for enemies that count toward `KillsTotal`** — directly-placed `BP_Enemy_*` actors in the level are treated as decoration / scripted display and never participate in stat counting or the level-clear check. If you want an enemy to count, you place a spawn point. (See section 10.5 for the per-level checklist.)

Each spawn point holds the enemy class to spawn, the lowest difficulty at which it activates, the kill-target flag, and a flag controlling whether it spawns automatically at level start or waits to be fired by a trigger. Spawn points implement `IQuakeActivatable` so they can be targets of generic buttons / `AQuakeTrigger_Relay` chains in addition to the typed `AQuakeTrigger_Spawn` (see 5.6).

```cpp
UCLASS()
class AQuakeEnemySpawnPoint : public AActor, public IQuakeActivatable
{
    GENERATED_BODY()
public:
    /** What to spawn at this point. */
    UPROPERTY(EditAnywhere, Category="Spawn")
    TSubclassOf<AQuakeEnemyBase> EnemyClass;

    /** Lowest difficulty at which this spawn point activates. Easy = always; Nightmare = nightmare-only. */
    UPROPERTY(EditAnywhere, Category="Spawn")
    EQuakeDifficulty MinDifficulty = EQuakeDifficulty::Easy;

    /** If true, waits for an external Activate() / AQuakeTrigger_Spawn call. If false, spawns in BeginPlay. */
    UPROPERTY(EditAnywhere, Category="Spawn")
    bool bDeferredSpawn = false;

    /** If true, this spawn point's enemy counts toward KillsTotal and the level-clear check (default true). */
    UPROPERTY(EditAnywhere, Category="Spawn")
    bool bIsMarkedKillTarget = true;

    /** True if this spawn point participates in stat / clear counting at the current difficulty. */
    bool IsEligible() const;  // bIsMarkedKillTarget && current difficulty >= MinDifficulty
    /** True if the spawn point's enemy has been spawned AND has died. Unspawned eligible spawn points are NOT satisfied. */
    bool IsSatisfied() const;

    virtual void BeginPlay() override;
    virtual void Activate(AActor* Instigator) override;  // IQuakeActivatable

protected:
    UPROPERTY() TObjectPtr<AQuakeEnemyBase> SpawnedEnemy;  // null until spawned; ensures one-shot
    bool TrySpawn();  // checks IsEligible() + SpawnedEnemy, then spawns
};
```

**Behavior:** `BeginPlay` calls `TrySpawn()` if `bDeferredSpawn == false`. `Activate(Instigator)` calls `TrySpawn()` regardless. `TrySpawn` calls `IsEligible()` (difficulty + marked check), bails if false, bails if already spawned, otherwise spawns `EnemyClass` at the actor's transform and stores the result in `SpawnedEnemy`. Each spawn point spawns at most once per level attempt.

**Why a flag instead of two subclasses:** the level-start vs deferred behaviors share 90% of the code (eligibility check, one-shot guard, actual spawn call). A boolean is simpler than `AQuakeEnemySpawnPoint_LevelStart` / `AQuakeEnemySpawnPoint_Deferred` subclasses, and a designer can flip the flag in the editor without re-placing the actor.

**Why the flag lives on the spawn point, not the enemy.** The "should this count?" decision is a *placement* decision, not a *class* decision — every Grunt class is a real enemy, but a Grunt placed as boss-arena ambient decoration shouldn't gate exit unlock. Putting the flag on the spawn point keeps the placement and the count rule co-located.

### 5.2 Hazards and Water

**Hazard volumes** (`AQuakeHazardVolume`):

| Hazard | Damage | Tick Rate | Notes                                      |
|--------|--------|-----------|--------------------------------------------|
| Slime  | 4      | 1.0 s     | Greenish glow                              |
| Lava   | 30     | 1.0 s     | Orange glow, also applies 200 unit knockback away from center on entry |

Hazards damage both players and enemies. Biosuit makes the player immune to slime and lava but not drowning.

### 5.3 Water Volumes

**`AQuakeWaterVolume : public APhysicsVolume`**, not a custom trigger actor. `APhysicsVolume` is a UE built-in `AVolume` subclass (a brush) whose `bWaterVolume` flag is read by `UCharacterMovementComponent` every tick to decide whether the pawn is swimming. Setting it in the constructor is enough to make the engine handle all `MOVE_Walking`/`MOVE_Falling` ↔ `MOVE_Swimming` transitions automatically — the volume's brush shape (assigned in the editor) defines the water boundary, and any pawn whose `GetPawnPhysicsVolume()` resolves to this volume enters swim mode at the start of the next movement update. **No overlap callbacks, no manual `SetMovementMode` calls.** Driving swim mode from a custom trigger volume's `OnBeginOverlap` would fight the engine because `PhysSwimming` re-checks `Pawn->GetPawnPhysicsVolume()->bWaterVolume` every frame and would immediately snap the pawn back out of swim mode.

```cpp
// QuakeWaterVolume.h
UCLASS()
class AQuakeWaterVolume : public APhysicsVolume
{
    GENERATED_BODY()
public:
    AQuakeWaterVolume(const FObjectInitializer& OI);
};

// QuakeWaterVolume.cpp
AQuakeWaterVolume::AQuakeWaterVolume(const FObjectInitializer& OI) : Super(OI)
{
    bWaterVolume     = true;
    FluidFriction    = 0.3f;   // Quake-feel water drag (tune in playtesting)
    TerminalVelocity = 350.f;  // matches Max swim speed below
    Priority         = 1;      // higher than the world default APhysicsVolume
}
```

**Swimming movement** — these values are set on `UQuakeCharacterMovementComponent` (the per-pawn CMC), not on the volume. The CMC reads them while `MovementMode == MOVE_Swimming`. Stock `PhysSwimming` is used; the project does **not** override it.

| SPEC value             | UE value | `UCharacterMovementComponent` property                |
|------------------------|----------|--------------------------------------------------------|
| Max swim speed         | 350      | `MaxSwimSpeed`                                         |
| Swim acceleration      | 2000     | `MaxAcceleration` (active value while swimming)        |
| Buoyancy               | neutral  | `Buoyancy = 1.0` (the CMC default)                     |
| Vertical control       | —        | Native: `PhysSwimming` reads view pitch + Jump input   |

**Surface detection** — this is computed on `AQuakeCharacter`, not on the volume, because the test cares about the *camera* position, not the capsule, and the engine doesn't know where the camera is:

- A pawn is **submerged** if its camera (eye) is below the water surface.
- A pawn is **partially in water** if its capsule overlaps the volume but the camera is above the surface.
- Drowning only applies when fully submerged.

The character can determine "am I in any water at all" with one call: `GetCharacterMovement()->IsSwimming()` or equivalently `GetPawnPhysicsVolume()->bWaterVolume`. The "is the camera submerged" follow-up is a single line-against-volume-bounds check on tick. The Underwater Discharge from [section 2.3](SPEC.md#L271) and the half-speed projectile rule below both call `IsSwimming()` (or `bWaterVolume`) on the firing pawn — no overlap state to track.

**Drowning:**

- Air supply: 12 seconds, refilled instantly when surfacing.
- Once air runs out: 4 damage per second until the player surfaces or dies.
- Biosuit does NOT extend air or prevent drowning.

**Water and weapons:**

- Firing the Thunderbolt while submerged or partially in water triggers Underwater Discharge (see 2.3).
- Other weapons fire normally underwater. Projectiles (rockets, nails, grenades) move at half speed in water.

### 5.4 Doors

`AQuakeDoor` is an `AActor` with a `UStaticMeshComponent` (the door geometry) driven by a `UTimelineComponent`. The timeline interpolates the mesh's relative location between closed and open positions over the open/close duration. UE5 has no "moving brush" concept — runtime-moving geometry is a regular Actor with component animation.

| Property            | Default | Notes                                              |
|---------------------|---------|----------------------------------------------------|
| Open speed          | 200 u/s | Linear velocity along open direction               |
| Open distance       | 128     | How far the door slides                            |
| Auto-close delay    | 4.0 s   | Set to 0 to stay open permanently                  |
| Close speed         | 200 u/s | Same as open                                       |
| Crush damage        | 10000   | Player or enemy caught in closing door = instant kill (effectively gibbed) |
| Required key        | None / Silver / Gold |                                       |
| Trigger mode        | Proximity / Button / Trigger | How the door is activated      |

**Behavior:**

- Doors open on activation. After the auto-close delay (if > 0), they close.
- A door will not close while a pawn (player or enemy) is inside its swept volume — it stays open until clear.
- A locked door bumps the player back slightly and prints a "You need the [color] key" message at the top of the screen for 2 seconds. A locked-door click sound stub plays.
- Doors block both player and enemy movement and projectiles when closed.
- `AQuakeDoor` implements `IQuakeActivatable` (see 5.5). Buttons and triggers fire it via **direct actor reference**, not by name. The `Required key` field is an `EKeyColor` enum (`None` / `Silver` / `Gold`) checked against `Player->GetPlayerState<AQuakePlayerState>()->HasKey(...)` per [section 4.4](SPEC.md#L432) — also no string lookup.

### 5.5 Buttons

`AQuakeButton` is a one-shot or reusable interactable.

**Activation model: `IQuakeActivatable` interface, direct actor references — no string-name targeting.** Quake's original `targetname`/`target` string lookup is not used. Instead, every fireable actor (`AQuakeDoor`, `AQuakeTrigger` and its subclasses, `AQuakeSecret`, level exit, etc.) implements a small `UInterface` declared in C++. The interface is **C++-only** (not `BlueprintNativeEvent`) — `Activate` is a pure-virtual method that subclasses override directly with `virtual void Activate(...) override`. **Do not use `Activate_Implementation`** — that suffix is only valid for `UFUNCTION(BlueprintNativeEvent)` methods, which this interface intentionally avoids per the project's "no Blueprint logic" rule.

```cpp
// QuakeActivatable.h
UINTERFACE(MinimalAPI)
class UQuakeActivatable : public UInterface { GENERATED_BODY() };

class IQuakeActivatable
{
    GENERATED_BODY()
public:
    /** Fire this actor. Instigator is the pawn that caused the activation (player, usually); may be null for indirect chains. */
    virtual void Activate(AActor* Instigator) = 0;
};
```

Buttons and triggers hold a typed `TArray<TObjectPtr<AActor>>` of targets, set per-instance in the editor via UE5's actor picker (eyedropper UX) on each placed instance. On fire, the source iterates the list, casts each entry to `IQuakeActivatable`, and calls `Activate()`. No name registry, no `TActorIterator` walks, no silent typo failures, no runtime cost beyond a list iteration.

```cpp
// QuakeButton.h
UCLASS()
class AQuakeButton : public AActor
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category="Quake|Button")
    EQuakeButtonActivation ActivationMode = EQuakeButtonActivation::Touch;

    UPROPERTY(EditAnywhere, Category="Quake|Button")
    float Cooldown = 0.0f;   // 0 = one-shot

    /** Actors to fire when this button activates. Each must implement IQuakeActivatable. */
    UPROPERTY(EditInstanceOnly, Category="Quake|Button")
    TArray<TObjectPtr<AActor>> Targets;

protected:
    void Fire(AActor* Instigator);
};
```

| Property        | Default              | Notes                                                |
|-----------------|----------------------|------------------------------------------------------|
| ActivationMode  | Touch / Shoot        | `EQuakeButtonActivation` enum, configurable per button |
| Cooldown        | 0 (one-shot)         | Seconds; 0 means single use                          |
| Targets         | (empty)              | `TArray<TObjectPtr<AActor>>`, picked per-instance in the editor |

**Behavior:**

- On activation, `Fire(Instigator)` iterates `Targets`, casts each to `IQuakeActivatable`, calls `Activate(Instigator)`, and skips entries that don't implement the interface (with a runtime warning so authoring errors are visible in the log).
- One-shot buttons become non-interactable after first use (visual: depressed state). Reusable buttons return to the up state after the cooldown.
- The button itself does **not** implement `IQuakeActivatable`. Buttons are only activated by world input (touch/shoot), not by other buttons or triggers. If you need a "fire on signal" relay, use [`AQuakeTrigger_Relay`](SPEC.md#L558) (see 5.6).

**Per-instance editor authoring.** `AQuakeButton` is a pure C++ class. There is no `BP_Button` thin subclass required for the targeting — `EditInstanceOnly` properties are editable on every level-placed instance regardless of class kind. A thin BP subclass (`BP_Button`) exists only to assign the visible mesh and material asset slots, matching the project convention from [section 9.2](SPEC.md#L744). The `Targets` list is filled per placed instance in the level, not in the BP defaults.

### 5.6 Triggers

`AQuakeTrigger` is the abstract base for trigger volumes and trigger actors. Every trigger subclass **implements `IQuakeActivatable`** (see 5.5), so triggers can be chained — a button fires a trigger, which fires more triggers, doors, secrets, etc.

```cpp
// QuakeTrigger.h
UCLASS(Abstract)
class AQuakeTrigger : public AActor, public IQuakeActivatable
{
    GENERATED_BODY()
public:
    /** Volume component for overlap-based triggers. Optional — relay-style triggers leave it disabled. */
    UPROPERTY(VisibleAnywhere) TObjectPtr<UBoxComponent> TriggerVolume;

    /** Generic targets fired when this trigger activates. Subclasses MAY also have type-specific reference fields. */
    UPROPERTY(EditInstanceOnly, Category="Quake|Trigger")
    TArray<TObjectPtr<AActor>> Targets;

    virtual void Activate(AActor* Instigator) override;  // fires Targets, then runs subclass-specific behavior
};
```

The base class binds `OnComponentBeginOverlap` on `TriggerVolume` (when present) to call `Activate(OverlappingActor)` on itself. Subclasses don't have to think about overlap wiring — they only override `Activate` to add per-type behavior, then call `Super::Activate(Instigator)` to fire the generic `Targets` chain.

**Targeting is direct-reference throughout.** No subclass uses string lookups. Each subclass that needs *typed* references (e.g., spawn points, teleport destination) declares its own additional `TArray<TObjectPtr<T>>` field with the concrete type, so the editor's actor picker filters the dropdown to only valid choices.

**Trigger types:**

| Subclass                    | Type-specific fields                                       | Behavior on `Activate` |
|-----------------------------|------------------------------------------------------------|------------------------|
| **`AQuakeTrigger_Relay`**   | none (uses base `Targets`)                                 | Fires every actor in `Targets` via `IQuakeActivatable::Activate`. Replaces the old "fire by name" pattern; chains buttons → triggers → doors / secrets / level exit. |
| **`AQuakeTrigger_Spawn`**   | `TArray<TObjectPtr<AQuakeEnemySpawnPoint>> SpawnPoints`    | Iterates `SpawnPoints`, asks each one to spawn its enemy class. Then calls `Super::Activate` to fire any base `Targets`. |
| **`AQuakeTrigger_Message`** | `FText Message`, `float Duration = 3.0f`                   | Displays `Message` on the HUD for `Duration` seconds via the existing HUD message API. Then `Super::Activate`. |
| **`AQuakeTrigger_Hurt`**    | `float DamagePerTick = 10000.0f`, `float TickRate = 0.5f`  | **Self-contained — does not fire targets.** Damages every overlapping pawn via `ApplyPointDamage` with `UQuakeDamageType_Telefrag` on a tick interval. Used for kill volumes, crusher pits, and "you walked into the trap" zones. The trigger volume stays active for the duration the actor is inside. |
| **`AQuakeTrigger_Teleport`**| `TObjectPtr<AActor> Destination`                           | Teleports the overlapping pawn to `Destination`'s transform (location + yaw, preserves velocity magnitude rotated to the new yaw). `Destination` is typed `AActor*` so any actor works — typically an engine `ATargetPoint` placed in the level. Then `Super::Activate`. |

**Why every trigger has both overlap-fire and `Activate()`:** the same trigger asset can be either an entry trigger (player walks in → fires targets) or a relay (another trigger fires it → fires its own targets) without changing class. Authoring decision: leave `TriggerVolume`'s collision enabled for entry triggers, disable it for relays. No subclass split needed.

**Per-instance editor authoring.** Same as buttons — `AQuakeTrigger_*` are pure C++ classes. `Targets`, `SpawnPoints`, `Destination`, and `Message` are all `EditInstanceOnly` and editable on every level-placed instance via the picker. Thin BP subclasses exist only if a trigger needs visible meshes (most don't — triggers are typically invisible volumes).

**Why the `IQuakeActivatable` chain replaces named targets cleanly.** Quake's `targetname` model gave you fan-out (one trigger → many actors with the same name) and chaining (trigger A fires trigger B fires trigger C). With direct references, fan-out is just a longer `Targets` list and chaining is just adding the next trigger as a target. No semantic loss, no global name registry, no string typos. The one thing the name model could do that direct references can't is forward-reference an actor that doesn't exist at edit time (e.g., spawned at runtime by another trigger) — the v1 SPEC has no such case, and if one appears later, the right answer is to expose a runtime registration API on the relevant subsystem, not to bring back name-based lookup globally.

### 5.7 Progression

- Levels are organized in episodes. Each episode is a linear sequence of levels.
- A start hub level allows the player to choose an episode (walk into the appropriate portal).
- Completing the last level of an episode returns the player to the hub.
- Completing all episodes triggers the win screen (see section 7).

### 5.8 Secrets

- Hidden areas accessible by shooting walls, walking through fake geometry, or finding hidden buttons.
- Secret areas contain extra ammo, health, or powerups.
- Each secret is implemented as a `AQuakeTrigger_Secret` volume placed at the entrance to the hidden area. Entering the volume credits the secret and prints "A secret area!" on screen.

### 5.9 Stat Counting

Stats split across two actors as a **numerator/denominator pair** — these are not duplicate counts:

- **`AQuakeGameMode`** owns the **denominators**: `KillsTotal` and `SecretsTotal`. Both are computed once at `BeginPlay` and **never updated** afterward.
  - `KillsTotal` = count of `AQuakeEnemySpawnPoint` actors where `IsEligible()` is true at the current difficulty (i.e., `bIsMarkedKillTarget == true && MinDifficulty <= GameMode->GetDifficulty()`). Spawn points are counted regardless of `bDeferredSpawn` — a deferred spawn point still owes its enemy to the level total. **Directly-placed `AQuakeEnemyBase` actors do not contribute** (they're decoration; see [section 5.1](SPEC.md#L446)).
  - `SecretsTotal` = count of `AQuakeTrigger_Secret` volumes in the level.
  - These represent "how many existed (or will exist) in this level," not "how many have happened so far."
- **`AQuakePlayerState`** owns the **numerators**: `Kills`, `Secrets`, `TimeElapsed`, `Deaths`. These run up as the player progresses and represent the player's score for the level attempt.

The HUD displays `Kills / KillsTotal` and `Secrets / SecretsTotal` by reading the numerator from `PlayerState` and the denominator from `GameMode`. The end-of-level stat screen does the same. There is **no third counter** for "how many kills have happened" — the level-clear check is computed on demand (see below).

Game-mode events (an enemy dying, a secret being entered) call into the PlayerState to increment counters where the player has earned credit. The HUD reads PlayerState directly via `PlayerController->GetPlayerState<AQuakePlayerState>()` and reads GameMode via `GetWorld()->GetAuthGameMode<AQuakeGameMode>()`.

**Level-clear check.** "Has the player cleared the level?" is computed on demand by scanning **spawn points** (not enemies) rather than tracked as a separate counter. Scanning spawn points correctly handles the deferred-spawn case: a deferred spawn point that hasn't fired yet is *not* satisfied, so the level cannot clear until that trigger fires AND the resulting enemy dies.

```cpp
bool AQuakeGameMode::IsLevelCleared() const
{
    for (TActorIterator<AQuakeEnemySpawnPoint> It(GetWorld()); It; ++It)
    {
        if (It->IsEligible() && !It->IsSatisfied()) return false;
    }
    return true;
}

// AQuakeEnemySpawnPoint::IsSatisfied
bool AQuakeEnemySpawnPoint::IsSatisfied() const
{
    return SpawnedEnemy != nullptr && SpawnedEnemy->IsDead();
}
```

The exit unlocks when this returns true. This is independent of `PlayerState.Kills` — an enemy that died from infighting without player credit still satisfies the clear condition (its spawn point is satisfied), even though the player doesn't get a point for it on the score screen. It also avoids drift from miscounted edge cases (revived Zombies don't `IsDead()` until permanently killed, so their spawn point isn't satisfied; infighting kills satisfy the spawn point regardless of credit; hazard kills satisfy regardless of credit).

**Counter rules:**

- **Time:** starts ticking when the player gains control after the level loads (post-fade-in). Pauses while paused. Stops the moment the player overlaps the `ExitTrigger`.
- **Kills (player credit, increments `PlayerState.Kills`):** +1 each time a marked kill-target enemy enters its `Dead` state **and the player is credited** by the rules below. Player credit is independent of whether the enemy actually died — credit is purely about score attribution.
  - **Normal death:** +1 if the player dealt the killing blow (`EventInstigator == Player->GetController()`).
  - **Gibbed kill:** +1 (same as normal death — gibbing doesn't double-count).
  - **Zombie revive:** a Zombie that goes Down and revives is **not** dead for level-clear purposes (`IsDead()` returns false during the Down state, true after permanent death/gib only). The player gets +1 only on the permanent kill, never on the temporary Down.
  - **Infighting kill:** +1 to player credit only if a player action started the chain within the last 5 seconds (e.g., the player's rocket bounced an Ogre's grenade into a Shambler). Otherwise the enemy is still dead (counts for level-clear) but the player gets no score credit.
  - **Hazard kill:** +1 to player credit only if the player damaged the enemy at any point during this level attempt. Otherwise, dead but uncredited.
- **Secrets:** `PlayerState.Secrets` increments by 1 the first time the player enters each `AQuakeTrigger_Secret` volume. Re-entering does nothing.
- **Deaths:** number of times the player died on this level attempt. Incremented before restoring the level-entry snapshot.

---

## 6. Game Rules

### 6.1 Difficulty

Difficulty is selected at new-game time from the main menu and persists for the entire playthrough. It cannot be changed mid-playthrough.

| Difficulty | Enemy Damage | Enemy HP | Notes                                                                    |
|------------|--------------|----------|--------------------------------------------------------------------------|
| Easy       | ×0.75        | ×1.0     | Forgiving, for new players                                               |
| Normal     | ×1.0         | ×1.0     | Default                                                                  |
| Hard       | ×1.5         | ×1.25    | Extra enemies via spawn-point filtering (no runtime multiplier)          |
| Nightmare  | ×2.0         | ×1.5     | Even more spawn-point enemies; Zombies revive 2× faster; pain immunity for all enemies |

Damage and HP are flat runtime multipliers applied in `AQuakeEnemyBase::ApplyDifficultyScaling` (see "Application" below). **Enemy count is not a multiplier** — it is implemented entirely by `AQuakeEnemySpawnPoint.MinDifficulty` filtering, which is authored per-placement in the editor. The "extra enemies" wording in the table is descriptive of design intent, not a runtime calculation. See "Enemy count scaling" below for the mechanism.

**Storage and accessor.** `EQuakeDifficulty` is an enum (`Easy`, `Normal`, `Hard`, `Nightmare`) stored on `UQuakeGameInstance` as the source of truth — it must outlive both `OpenLevel` and Character respawn, which is GameInstance's role per [section 1.4](SPEC.md#L70). `AQuakeGameMode` reads it on `BeginPlay` and exposes:

- `EQuakeDifficulty GetDifficulty() const`
- `const FQuakeDifficultyMultipliers& GetDifficultyMultipliers() const`

Gameplay code goes through the **GameMode**, never directly to GameInstance. `GetWorld()->GetAuthGameMode<AQuakeGameMode>()` is the standard one-call lookup the rest of the SPEC already uses.

**Multiplier struct, BP-tunable.** The numbers in the table above live in a `UPROPERTY` `TMap` on `AQuakeGameMode`, marked `EditDefaultsOnly` so values can be tuned without recompiling C++:

```cpp
USTRUCT(BlueprintType)
struct FQuakeDifficultyMultipliers
{
    GENERATED_BODY()
    UPROPERTY(EditDefaultsOnly) float EnemyHP     = 1.0f;
    UPROPERTY(EditDefaultsOnly) float EnemyDamage = 1.0f;
};

UCLASS()
class AQuakeGameMode : public AGameModeBase
{
    GENERATED_BODY()
protected:
    UPROPERTY(EditDefaultsOnly, Category="Difficulty")
    TMap<EQuakeDifficulty, FQuakeDifficultyMultipliers> DifficultyTable;
};
```

`BP_QuakeGameMode` (the existing thin BP subclass per [section 9.2](SPEC.md#L744)) fills in the four entries. Editing the BP to retune Hard from ×1.5 to ×1.4 requires no recompile. `EnemyCount` is intentionally **not** in the struct because count scaling is handled by spawn-point filtering, not a runtime multiplier (see "Enemy count scaling" below).

**Application — enemies bake the scaling in `BeginPlay`.** Each `AQuakeEnemyBase` holds its base values in C++ defaults (`BaseMaxHealth`, `BaseAttackDamage`) and computes the runtime values from the GameMode multipliers in `BeginPlay`. Enemies do **not** store a member reference to the GameMode — they look it up once per spawn:

```cpp
void AQuakeEnemyBase::BeginPlay()
{
    Super::BeginPlay();
    ApplyDifficultyScaling();
    Health = MaxHealth;
}

void AQuakeEnemyBase::ApplyDifficultyScaling()
{
    if (auto* GM = GetWorld()->GetAuthGameMode<AQuakeGameMode>())
    {
        const FQuakeDifficultyMultipliers& M = GM->GetDifficultyMultipliers();
        MaxHealth              = BaseMaxHealth   * M.EnemyHP;
        AttackDamageMultiplier = M.EnemyDamage;
    }
}
```

`AttackDamageMultiplier` is multiplied into the per-attack damage at fire time inside `FireAtTarget` (or wherever the enemy calls `ApplyPointDamage`). HP is baked in once. Both work for the lifetime of the enemy because difficulty cannot change mid-playthrough.

`ApplyDifficultyScaling` is **virtual** — subclasses (or AIController subclasses) override it for per-difficulty quirks. The base only handles the simple stat scaling.

**Per-difficulty quirks.** Things that aren't a flat multiplier go in the quirk slots:

- **Zombie revive timing on Nightmare.** `AQuakeAIController_Zombie::ApplyDifficultyScaling()` overrides to halve `ReviveTimer` when difficulty is Nightmare. Calls `Super::ApplyDifficultyScaling()` first to get the base HP/damage scaling.
- **Pain immunity on Nightmare.** Checked at the moment of the pain decision in `AQuakeEnemyAIController::OnDamaged`, not as a baked value: `if (GM->GetDifficulty() == EQuakeDifficulty::Nightmare) return;` skips the pain reaction. There's no "pain multiplier" because pain is binary — it either fires or it doesn't, and the difficulty rule is "never on Nightmare."

**Enemy count scaling — handled by spawn-point filtering, not multipliers.** Each `AQuakeEnemySpawnPoint` declares a `MinDifficulty` ([section 5.1](SPEC.md#L446)). The check happens in the spawn point's `TrySpawn()` helper:

```cpp
bool AQuakeEnemySpawnPoint::TrySpawn()
{
    if (SpawnedEnemy) return false;  // one-shot
    const auto* GM = GetWorld()->GetAuthGameMode<AQuakeGameMode>();
    if (!GM || GM->GetDifficulty() < MinDifficulty) return false;  // gated
    // ... spawn EnemyClass at GetActorTransform(), store in SpawnedEnemy
    return true;
}
```

Easy and Normal spawn only `MinDifficulty = Easy` placements. Hard adds the `MinDifficulty = Hard` ones. Nightmare adds the `MinDifficulty = Nightmare` ones. The check is local to each spawn point — no central GameMode iteration, no pre-filter pass at level load. The "extra enemies" entries in the multiplier table are advisory only; the actual enemy count is whatever the spawn points add up to per difficulty, which level designers tune by sprinkling Hard+/Nightmare-only placements.

**Why this shape:**

- **GameInstance owns the choice, GameMode owns the application.** Same split as inventory (GameInstance owns the data, Character/PlayerState read it). Difficulty doesn't break the ownership rules.
- **Enemies query, don't subscribe.** Pull-based, consistent with the rest of the codebase (HUD polling, AI perception polling on tick) — no event subscription, no notification delegate, no need for the enemy to react to difficulty *changes* because there are none.
- **Virtual `ApplyDifficultyScaling` is the extension point.** New enemies that need quirky scaling (e.g., a future boss with custom resistance rules) override one method. No central `if (enemy is X) { ... }` ladder in the GameMode.
- **Spawn-point filtering decentralizes the count rule.** The count scaling lives where it's authored (the spawn point in the editor), not in a GameMode pre-pass. This matches the SPEC's preference for derived/local state over cached/central state ([section 5.9 stat counting](SPEC.md#L660) does the same with the level-clear scan).

### 6.2 Saves

Save data is implemented via UE's standard `USaveGame` framework. `UQuakeSaveGame : public USaveGame` holds all persisted state, written and read via `UGameplayStatics::SaveGameToSlot` / `LoadGameFromSlot`.

**`UQuakeSaveGame` fields:**

- **Player profile** — difficulty, total stats across all levels (kills, secrets, time, deaths). Persisted from `UQuakeGameInstance`.
- **Inventory snapshot** (from `UQuakeGameInstance`) — weapons owned, ammo per type, armor type and value. **Health is not in this bullet** (it's not inventory; see below).
- **Live health** (from `AQuakeCharacter`) — current health value at save time. The Character serializes its own health into the save record; on load, the new Character reads it back in `BeginPlay` after the GameInstance inventory restore.
- **Level state** — current level name, player transform (position + rotation).
- **PlayerState snapshot** (from `AQuakePlayerState`) — current-level kills, secrets, time, deaths, the active powerups array (type + remaining duration for each), and **keys held**.
- **Per-actor state** — array of `FActorSaveRecord` keyed by stable per-actor identity (see "Per-actor save participation" below). Captures door state (open/closed/locked), button state (pressed/cooldown), one-shot trigger state (fired/not), secret state (entered/not), spawn point state (spawned/not + spawned-enemy alive/dead), pickup state (consumed/not), and per-enemy state for currently-spawned enemies (alive/dead, current health, AI state).

**Save flow.** On save, the GameInstance, the active `AQuakeCharacter`, and the active `AQuakePlayerState` each serialize their fields into the save game struct. The GameMode then iterates `IQuakeSaveable` actors and asks each one to fill an `FActorSaveRecord`.

**Load flow.** On load:
1. `UGameplayStatics::LoadGameFromSlot` returns the `UQuakeSaveGame`.
2. `UQuakeGameInstance` restores its own fields (player profile, inventory snapshot) immediately, before `OpenLevel`.
3. `OpenLevel` is called with the saved level name. The world tears down and rebuilds.
4. After the new level loads, the GameMode restores the PlayerState fields from the save's PlayerState snapshot.
5. The GameMode builds a `TMap<FName, const FActorSaveRecord*>` from the save's per-actor records and iterates `IQuakeSaveable` actors, looking each one up by its stable identity (see below) and calling `LoadState(record)` if a matching record exists.
6. `AQuakeCharacter::BeginPlay` reads inventory from `UQuakeGameInstance` (already restored in step 2) and reads its `Health` from the save record (loaded in step 5 since the Character implements `IQuakeSaveable`).

**Save slots:**

- `auto_<profile>` — auto-save slot, written at level start immediately after the level-entry snapshot.
- `quick_<profile>` — quick-save slot, written by F5.
- One slot of each kind per player profile.

**Save / load flow:**

- **Auto-save** at level start, immediately after the level-entry snapshot is taken. Overwrites the auto-save slot.
- **Quick save** (F5) — manual snapshot of the current level state. Calls `UGameplayStatics::SaveGameToSlot(SaveGame, "quick_<profile>", 0)`.
- **Quick load** (F9) — calls `LoadGameFromSlot("quick_<profile>", 0)`. If null, falls back to the auto-save. Loading triggers `OpenLevel` to the saved level name, then on `BeginPlay` the GameInstance restores inventory and the level's actors restore state from the save.
- **No mid-air saves** — quick save is rejected if the player is not on the ground or is currently taking damage.
- Save files persist between sessions in `Saved/SaveGames/`.
- Starting a new game on the same profile clears existing slots.

**Per-actor save participation.** Any actor that has persistent state implements `IQuakeSaveable` with `SaveState(FActorSaveRecord&)` and `LoadState(const FActorSaveRecord&)` methods. The save record carries a stable identity field used to match records to actors at load time.

**Stable identity = `AActor::GetFName()` for level-placed actors.** UE5 assigns each level-placed actor a stable, unique `FName` at placement time and serializes it with the `.umap`. This name survives save/load cycles, package re-saves, and editor restarts. It changes only if the actor is duplicated, deleted-and-replaced, or has its name forcibly edited in the level outliner — in which case any existing save records for the old name simply won't match and the actor restores to its level-default state. This is the standard UE5 idiom for persistent identity and matches the SPEC's preference for direct, refactor-traceable references over user-typed strings (the name is editor-assigned, not user-typed; renaming in the outliner is visible in the diff).

```cpp
USTRUCT()
struct FActorSaveRecord
{
    GENERATED_BODY()
    UPROPERTY() FName ActorName;          // == owner's GetFName(); set by SaveState before serialize
    UPROPERTY() TArray<uint8> Payload;    // FMemoryWriter blob written by the actor's SaveState
};
```

**Runtime-spawned actors (not from spawn points) do not persist.** A level reload re-runs `BeginPlay` for spawn points and the chain reproduces the spawn-point-derived enemies. Anything spawned by other runtime systems (e.g., a hypothetical future AI summoner) is intentionally out of scope for v1 saves; if such systems get added later they'll need their own runtime registration mechanism rather than retrofitting names. Currently the only runtime-spawned gameplay actors are spawn-point-derived enemies and their projectiles, and projectiles are too transient to bother saving.

**Why not `AActor::Tags`?** `Tags` is a `TArray<FName>` that requires the user to type a tag string per instance — that's exactly the stringly-typed mechanism the SPEC removed from buttons/triggers in [section 5.5](SPEC.md#L546). `GetFName()` is editor-assigned and refactor-stable, so it's preferred.

### 6.3 Win Condition

- Completing the final level of the **final episode** (defined per episode in the game mode) triggers a win sequence:
  1. Fade to black.
  2. Display win screen with total stats: total kills, total secrets, total time, deaths, difficulty.
  3. Return to main menu.
- Within an episode, completing the last level returns the player to the hub. From the hub, the player can choose another episode or quit.
- **No score system.** Stats are informational only.

### 6.4 Failure Loop

- On player death: 1.5-second death animation (camera tilts to ground), then a death screen with "Press Fire to Restart."
- Pressing fire restores the level-entry snapshot (see 1.4) and respawns the player at the level's PlayerStart.
- There is no game-over screen and no run termination — the player can retry indefinitely.
- The death counter increments per death and is shown on the level-end stats screen.

**Restart sequence (in order).** This is the explicit cleanup path that compensates for UE not auto-resetting PlayerState on pawn respawn (see section 1.4 lifecycle table):

1. Increment `AQuakePlayerState::Deaths`.
2. Call `AQuakePlayerState::ClearPerLifeState()` — empties powerups array and key set. Does **not** touch `Kills`/`Secrets`/`TimeElapsed`/`Deaths` (those persist across the level attempt).
3. Restore `UQuakeGameInstance` inventory from the level-entry snapshot (weapons, ammo, armor).
4. Destroy the dead pawn and spawn a new `AQuakeCharacter` at the level's `PlayerStart`.
5. New pawn's `BeginPlay` reads inventory from GameInstance and sets `Health = 100` (or the snapshot value).
6. Time counter continues running (death does not pause the level timer).

---

## 7. HUD

The HUD is implemented as an `AQuakeHUD : public AHUD` (Actor) owned by the PlayerController. In `BeginPlay`, the HUD constructs an `SQuakeHUDOverlay` Slate widget (`SCompoundWidget` subclass) and adds it to the player's viewport via `GEngine->GameViewport->AddViewportWidgetForPlayer`. No UMG, no Blueprints — pure C++/Slate.

**Data sources** — the HUD reads from three places, in order of update frequency:

- **`AQuakePlayerState`** — kills, secrets, time, deaths, active powerups (with remaining duration), keys held. The HUD's powerup-timer widget polls PlayerState every frame.
- **`AQuakeCharacter`** — live health, current weapon, weapon viewmodel state.
- **`UQuakeGameInstance`** — owned weapons (for the weapon bar), ammo per type, armor type and value.

The Slate widget caches a weak pointer to each source on construction and reads them in its paint callback.

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

### 7.1 Damage Feedback

- Screen flash red on taking damage, intensity scales with damage.
- Screen tint when under powerup effect (blue for Quad, red for Pentagram, etc.).
- Pickup flash (brief gold tint on item pickup).

---

## 8. Audio System

No audio assets exist yet. The system provides a clean interface for future integration.

### 8.1 Architecture

- **`UQuakeSoundManager`** (`UGameInstanceSubsystem`) — central audio manager. Auto-instantiated by the engine; **subsystems cannot have Blueprint subclasses.**
- Exposes functions like `PlaySound(ESoundEvent, Location)`, `PlayMusic(EMusicTrack)`, `StopMusic()`.
- `ESoundEvent` is an enum cataloging every game sound (weapon fire, pickup, enemy alert, door open, etc.).

**Sound table ownership.** Because subsystems can't be Blueprint-subclassed, the data table reference cannot live on `UQuakeSoundManager` itself in a tunable way. Instead:

- The reference lives as a `UPROPERTY(EditDefaultsOnly) TObjectPtr<UDataTable> SoundEventTable` on **`UQuakeGameInstance`**, which **does** have a thin BP subclass (`BP_QuakeGameInstance`) where the slot is filled in.
- `UQuakeSoundManager` reads it via `GetGameInstance<UQuakeGameInstance>()->GetSoundEventTable()` on first use and caches the pointer.
- The sound table is `DT_SoundEvents.uasset` (see [section 10.1](SPEC.md#L1049)) with row type `FQuakeSoundEvent { ESoundEvent Key; TObjectPtr<USoundBase> Sound; }`.
- When a row is unmapped (nullptr) or missing, the `PlaySound` call is a no-op so gameplay code can ship before audio assets exist.

**Why GameInstance and not GameMode.** `AQuakeGameMode` does not own subsystems and is recreated on level transition; the sound manager subsystem persists across levels. The data table reference must live somewhere with the same lifetime as the subsystem, which is `UQuakeGameInstance`.

**`BP_QuakeGameInstance` is set as the Game Instance class in `Project Settings → Maps & Modes → Game Instance Class`.**

This means all gameplay code calls the sound manager, and adding audio later is just filling in the data table rows in the Editor — without touching the subsystem or the GameInstance C++.

### 8.2 Sound Events (partial list)

- Player: footstep, jump, land, pain, death, pickup_item, pickup_weapon, pickup_powerup
- Weapons: axe_swing, shotgun_fire, nailgun_fire, rocket_fire, grenade_bounce, rocket_explode, thunderbolt_hum
- Enemies: alert, pain, death, attack, idle
- World: door_open, door_close, button_press, teleport, secret_found
- Music: per-level ambient tracks

---

## 9. Technical Architecture

### 9.1 Class Hierarchy

```
AQuakeGameMode                       — level loading, rules, player spawning, sets pawn/PC/PlayerState classes
AQuakePlayerController               — input setup, HUD ownership
AQuakePlayerState : public APlayerState  — current-level stats and active powerups
AQuakeCharacter                      — player body: movement, live health, weapon mgmt
UQuakeCharacterMovementComponent     — custom CMC, Quake-style air physics (overrides PhysFalling)
AQuakeHUD : public AHUD              — owns and adds Slate widgets to viewport
  SQuakeHUDOverlay (Slate widget)    — health/armor/ammo/weapon bar/keys/powerups/crosshair

AQuakeWeaponBase                     — base weapon class
  AQuakeWeapon_Axe
  AQuakeWeapon_Shotgun
  AQuakeWeapon_SuperShotgun
  AQuakeWeapon_Nailgun
  AQuakeWeapon_SuperNailgun
  AQuakeWeapon_GrenadeLauncher
  AQuakeWeapon_RocketLauncher
  AQuakeWeapon_Thunderbolt

AQuakeProjectile                     — base projectile, holds UProjectileMovementComponent
  AQuakeProjectile_Nail
  AQuakeProjectile_Rocket
  AQuakeProjectile_Grenade

AQuakeEnemyBase : public ACharacter  — enemy body: mesh, movement, health, action methods
  AQuakeEnemy_Grunt
  AQuakeEnemy_Knight
  AQuakeEnemy_Ogre
  AQuakeEnemy_Fiend
  AQuakeEnemy_Shambler
  AQuakeEnemy_Zombie

AQuakeEnemyAIController : public AAIController  — enemy brain: FSM, perception
  AQuakeAIController_Grunt
  AQuakeAIController_Knight
  AQuakeAIController_Ogre
  AQuakeAIController_Fiend
  AQuakeAIController_Shambler
  AQuakeAIController_Zombie
  (each owns a UAIPerceptionComponent with sight + hearing senses)

AQuakePickupBase                     — base pickup (USphereComponent overlap detection)
  AQuakePickup_Health, _Armor, _Ammo, _Weapon, _Key, _Powerup (subclasses)

AQuakeEnemySpawnPoint                — passive level marker; spawns EnemyClass at BeginPlay (or on Activate when bDeferredSpawn); MinDifficulty + bIsMarkedKillTarget gate stat counting; implements IQuakeActivatable; canonical authoring path for counted enemies

AQuakeDoor                           — UStaticMeshComponent + UTimelineComponent; implements IQuakeActivatable
AQuakeButton                         — touch/shoot interactable; fires IQuakeActivatable targets via direct reference (no name lookup)
AQuakeTrigger                        — abstract base; implements IQuakeActivatable; optional UBoxComponent for entry-overlap fire
  AQuakeTrigger_Relay                — fires base Targets list (relay / external-fire only — old "_Open" name, since it's not door-specific)
  AQuakeTrigger_Spawn                — typed SpawnPoints list of AQuakeEnemySpawnPoint
  AQuakeTrigger_Message              — FText Message + Duration; displays on HUD
  AQuakeTrigger_Hurt                 — DamagePerTick + TickRate; self-contained, no Targets
  AQuakeTrigger_Teleport             — single Destination AActor* (typically ATargetPoint)
  AQuakeTrigger_Secret               — credits a secret on first player overlap
AQuakeWaterVolume : public APhysicsVolume  — bWaterVolume = true; CMC handles MOVE_Swimming transitions automatically
AQuakeHazardVolume                   — slime/lava damage volume

UDamageType subclasses               — see section 1.5 for the full list
  UQuakeDamageType_Melee, _Bullet, _Nail, _Explosive, _Lightning,
  _Lava, _Slime, _Drown, _Telefrag

IQuakeActivatable                    — UInterface; Activate(AActor* Instigator); implemented by Door, Trigger and subclasses, level exit; fired by Button targets and Trigger chains (see 5.5 / 5.6)
IQuakeSaveable                       — interface for actors that persist in saves
UQuakeSaveGame : public USaveGame    — save data container
UQuakeSoundManager : public UGameInstanceSubsystem  — audio
UQuakeGameInstance                   — owns inventory, snapshot, save/load, level transitions
```

**Ownership summary:**

- **Inventory** (weapons owned, ammo, armor) lives on `UQuakeGameInstance`. Survives `OpenLevel` and Character respawn. The Character reads it on `BeginPlay` and writes back on changes.
- **Live health** lives on `AQuakeCharacter`. Tied to the body; reset from the level-entry snapshot on respawn.
- **Current-level stats** (kills, secrets, time, deaths), **active powerups**, and **keys held** live on `AQuakePlayerState`. Recreated on level transition (which clears stats, powerups, and keys for free). **Death-respawn does not auto-clear** PlayerState — `ClearPerLifeState()` is called explicitly from the restart flow in [section 6.4](SPEC.md#L868) to empty powerups and keys while leaving cumulative stats intact. The HUD reads them directly via `PlayerController->GetPlayerState<AQuakePlayerState>()`.
- **AI** lives on `AQuakeEnemyAIController`. The Enemy pawn only exposes action methods (`MoveTo`, `FireAt`, `PlayPainReaction`).
- **HUD** is `AQuakeHUD` (an `AHUD`), owned by the PlayerController. It adds an `SQuakeHUDOverlay` Slate widget to the viewport in `BeginPlay`.
- **Damage** flows through `AActor::TakeDamage` with `UDamageType` subclasses carrying metadata.
- **Saves** are `UQuakeSaveGame` instances written via `UGameplayStatics::SaveGameToSlot`. They serialize the GameInstance inventory + the PlayerState fields + per-actor `IQuakeSaveable` records.

### 9.2 Blueprint Layer

For each C++ class that needs assets or per-variant tuning, create a Blueprint subclass under `Content/Blueprints/`:

- **Weapons** — `BP_Weapon_Shotgun`, `BP_Weapon_Nailgun`, etc. Each holds the weapon's mesh, projectile class reference, muzzle effect, and tunable values (damage, fire rate, ammo cost).
- **Enemies** — `BP_Enemy_Grunt`, `BP_Enemy_Knight`, etc. Each holds the body/head/limb meshes, materials, and stats (health, speed, attack damage, sight range).
- **Pickups** — `BP_Pickup_Health25`, `BP_Pickup_GreenArmor`, etc. Hold mesh + material + value.
- **Projectiles** — `BP_Projectile_Rocket`, `BP_Projectile_Grenade`, etc.

Blueprints contain no nodes in the Event Graph — only property defaults and asset references. The C++ base class drives all behavior.

The game mode references the Blueprint classes (not the C++ classes directly) when spawning, via `TSubclassOf<>` properties assigned in `BP_QuakeGameMode`.

### 9.3 Existing Code

The following classes already exist and should be extended:

- `AQuakeCharacter` — first-person camera, movement parameters
- `AQuakePlayerController` — Enhanced Input setup (Move, Look, Jump actions)
- `AQuakeGameMode` — sets default pawn and controller classes

### 9.4 Build Configuration

- Module: `Quake`
- Public dependencies: `Core`, `CoreUObject`, `Engine`, `InputCore`, `EnhancedInput`, `NavigationSystem`, `AIModule`, `Slate`, `SlateCore`
- Target: Win64 (Development/Shipping)

Notes:
- `NavigationSystem` and `AIModule` are required for `AAIController`, `UAIPerceptionComponent`, and the sight/hearing senses.
- `Slate` and `SlateCore` are required for the C++ Slate HUD (no UMG).
- UMG is intentionally **not** a dependency — the HUD is pure Slate.
- `GameplayTags` is intentionally **not** a dependency — damage discrimination uses `UDamageType` subclasses (see section 1.5), and the project has no other concrete tag use case. Add it back when there is one (faction tags, resistances, status effects).

---

## 10. Project Layout & Editor Configuration

This section documents the on-disk layout, asset conventions, and Editor settings that complement the C++ code.

### 10.1 Content Folder Structure

```
Content/
  Maps/
    MainMenu.umap                  — title screen
    Hub.umap                       — episode portal
    E1/
      E1M1.umap
      E1M2.umap
      ...
    E2/
      E2M1.umap
      ...
  Blueprints/
    Framework/
      BP_QuakeGameMode.uasset      — assigns default pawn, HUD, difficulty multiplier table
      BP_QuakeGameInstance.uasset  — assigns sound event data table; set as Game Instance Class in Project Settings
      BP_QuakePlayerController.uasset — assigns Input Action and Input Mapping Context assets
    Weapons/
      BP_Weapon_Axe.uasset
      BP_Weapon_Shotgun.uasset
      BP_Weapon_SuperShotgun.uasset
      BP_Weapon_Nailgun.uasset
      BP_Weapon_SuperNailgun.uasset
      BP_Weapon_GrenadeLauncher.uasset
      BP_Weapon_RocketLauncher.uasset
      BP_Weapon_Thunderbolt.uasset
    Enemies/
      BP_Enemy_Grunt.uasset
      BP_Enemy_Knight.uasset
      BP_Enemy_Ogre.uasset
      BP_Enemy_Fiend.uasset
      BP_Enemy_Shambler.uasset
      BP_Enemy_Zombie.uasset
    Pickups/
      BP_Pickup_Health15.uasset
      BP_Pickup_Health25.uasset
      BP_Pickup_Megahealth.uasset
      BP_Pickup_Armor_Green.uasset
      BP_Pickup_Armor_Yellow.uasset
      BP_Pickup_Armor_Red.uasset
      BP_Pickup_Ammo_Shells.uasset
      BP_Pickup_Ammo_Nails.uasset
      BP_Pickup_Ammo_Rockets.uasset
      BP_Pickup_Ammo_Cells.uasset
      BP_Pickup_Quad.uasset
      BP_Pickup_Pentagram.uasset
      BP_Pickup_Ring.uasset
      BP_Pickup_Biosuit.uasset
      BP_Pickup_KeySilver.uasset
      BP_Pickup_KeyGold.uasset
    Projectiles/
      BP_Projectile_Nail.uasset
      BP_Projectile_Rocket.uasset
      BP_Projectile_Grenade.uasset
  Materials/
    M_QuakeBase.uasset             — single master material
    Instances/
      MI_Health.uasset
      MI_Armor_Green.uasset
      MI_Armor_Yellow.uasset
      MI_Armor_Red.uasset
      MI_Quad.uasset
      MI_Pentagram.uasset
      MI_Rocket.uasset
      MI_Nail.uasset
      MI_Wall_Brick.uasset
      MI_Wall_Metal.uasset
      MI_Floor.uasset
      MI_Lava.uasset
      MI_Slime.uasset
      MI_Enemy_Grunt.uasset
      MI_Enemy_Knight.uasset
      ...
  Data/
    DT_SoundEvents.uasset          — sound event → USoundBase mapping (rows empty for now)
```

### 10.2 Master Material

A single master material `M_QuakeBase` drives all in-game visuals. Parameters:

| Parameter    | Type    | Purpose                                  |
|--------------|---------|------------------------------------------|
| `BaseColor`  | Vector3 | Flat color                               |
| `Emissive`   | Scalar  | Glow intensity (0 for matte, >1 for glow)|
| `Metallic`   | Scalar  | 0 default, 1 for metal pickups           |
| `Roughness`  | Scalar  | 0.5 default                              |

Material Instances (`MI_*`) override these parameters per use case. Runtime tinting (damage flash, powerup overlay) creates a `UMaterialInstanceDynamic` from the relevant `MI_*` and animates `BaseColor` from C++.

### 10.3 NavMesh

**Per-level setup.** Every playable level must contain:
- A `NavMeshBoundsVolume` scaled to cover the entire walkable area.
- The auto-spawned `RecastNavMesh` actor with `Runtime Generation` set to `Static`.

**Project Settings → Navigation System.** Configure a single Supported Agent matching the player capsule and the player movement parameters in [section 1.1](SPEC.md#L22):
- Agent Radius: 35 (matches player capsule radius)
- Agent Height: 180 (matches player capsule full height)
- Agent Max Slope: 44 (matches player `Max walkable slope`)
- Agent Max Step Height: 45 (matches player `Step height` — must equal player value or AI cannot path through geometry the player can walk over)

Press **P** in the editor viewport to visualize the NavMesh. Green = walkable. If enemies refuse to chase, this is the first thing to check.

### 10.4 Project Settings

All values below are stored in `Config/Default*.ini` and committed to version control.

**Maps & Modes:**
- `Default GameMode` = `BP_QuakeGameMode`
- `Game Instance Class` = `BP_QuakeGameInstance`
- `Editor Startup Map` = `MainMenu`
- `Game Default Map` = `MainMenu`
- `Transition Map` = empty

**Description:**
- `Project Name` = Quake
- `Project Version` = 0.1.0

**Packaging:**
- `List of maps to include in a packaged build` — explicitly list every `.umap` to ship
- `Use Pak File` = true

**Input:**
- `Default Player Input Class` = `EnhancedPlayerInput`
- `Default Input Component Class` = `EnhancedInputComponent`

**Engine - General Settings:**
- `Smooth Frame Rate` = true (30–120 range)
- `Fixed Frame Rate` = false

### 10.5 Per-Level Checklist

When creating a new gameplay level, every map must include:

- [ ] `PlayerStart` actor at the spawn location
- [ ] `NavMeshBoundsVolume` covering all walkable areas
- [ ] `PostProcessVolume` (Unbound, low priority) — used by C++ for damage flash and powerup tint
- [ ] At least one light source (directional or point lights)
- [ ] Floor/wall geometry (BSP brushes or static meshes with `MI_Wall_*` / `MI_Floor` applied)
- [ ] **Enemy spawn placements: `AQuakeEnemySpawnPoint` actors with `EnemyClass` set to a `BP_Enemy_*` class.** Do NOT drag `BP_Enemy_*` actors directly into the level — those won't count toward `KillsTotal` and won't gate the level-clear check. The spawn point is the canonical authoring path (see [section 5.1](SPEC.md#L446)). Set `MinDifficulty`, `bDeferredSpawn`, and `bIsMarkedKillTarget` per placement.
- [ ] Pickup placements (`BP_Pickup_*` actors)
- [ ] Doors (`AQuakeDoor` actors), keyed appropriately
- [ ] An `ExitTrigger` (C++ actor) at the level end, configured with the next map name
- [ ] Optional: `AQuakeHazardVolume` actors for slime/lava areas
- [ ] Optional: `AQuakeTrigger_*` actors for scripted spawns, relays, messages, teleports, kill volumes (see [5.6](SPEC.md#L598))
- [ ] World Partition disabled (uncheck on level creation)
- [ ] Level Blueprint event graph: empty

---

## 11. v1 Scope (First Playable Milestone)

The first milestone targets a small, complete vertical slice rather than the full game. The goal is to prove the core systems end-to-end, then expand in v2+. v1 is delivered in **16 phases** (0–15), each gated on automated tests and manual verification before the next phase begins.

### 11.1 Approach

**Each phase is self-contained.** A phase produces a playable artifact — sandbox map, test level, content level — that can be exercised before the next phase starts. No phase is a pure plumbing layer; even Phase 0 ends with a runnable PIE session.

**Three test layers, used differently per phase:**

| Layer | Tool | What it catches | When to use |
|---|---|---|---|
| **Unit tests** | UE Automation (`IMPLEMENT_SIMPLE_AUTOMATION_TEST`, `EAutomationTestFlags::EditorContext`) | Pure-C++ logic regressions: math formulas, state-machine transitions, struct serialization | Damage scaling, drop table evaluation, powerup timer, save record round-trip |
| **Functional tests** | UE `AFunctionalTest` actor in dedicated test maps under `Content/Maps/Tests/` | Gameplay flow: spawn an actor, drive an input, assert a result | Pickup overlap effects, weapon damage to enemies, save→load round-trip, level-clear scan |
| **Manual smoke tests** | Human in PIE | Feel, polish, anything subjective | Strafe-jump feel, infighting emergence, "does this look right" |

Test maps live under `Content/Maps/Tests/` and are excluded from packaged builds via the section 10.4 packaging settings.

**Exit criteria.** Each phase has an explicit exit criterion. The phase is **not** done until (a) all phase-specific automated tests pass, (b) all manual verification steps succeed, and (c) the previous phase's tests still pass (no regressions). Implementation does not advance to the next phase until exit criteria are met. If a test for a previous phase breaks, fix it before adding new code.

**Phase ordering principle.** Risk first, content last. The highest-risk single item is the strafe-jumping CMC ([CLAUDE.md "Risk Note"](CLAUDE.md)), so it gets its own phase very early (Phase 1) on a sandbox before any combat code exists. Content authoring (the actual E1M1/M2/M3 maps) is the last phase, after every system it depends on is verified.

**No Phase N work in Phase N-1.** It is tempting to "go ahead and add the keys system while I'm in pickups." Don't. Phase boundaries exist so each test pass is targeted; mixing phases makes it harder to localize bugs. If a phase finishes early, run the manual verification a second time and fix anything that feels off — don't pull work forward.

### 11.2 Content Scope

This is what v1 ships, regardless of phasing:

| Category    | v1 Subset                                                          | Deferred to v2+                            |
|-------------|--------------------------------------------------------------------|--------------------------------------------|
| Episodes    | 1 episode (Episode 1)                                              | Episodes 2, 3, 4                           |
| Levels      | 1 hub + 3 gameplay levels: E1M1 (intro), E1M2, E1M3 (boss)         | Additional E1 levels and other episodes    |
| Weapons     | Axe, Shotgun, Nailgun, Rocket Launcher (4)                         | SSG, Super Nailgun, Grenade Launcher, Thunderbolt |
| Enemies     | Grunt, Knight, Ogre (3)                                            | Fiend, Shambler, Zombie                    |
| Boss        | "Greater Ogre" — Ogre variant with 800 HP and faster grenades, at end of E1M3 | True bosses                |
| Pickups     | All health (Small, Pack, Megahealth), all armor tiers, ammo for v1 weapons, Silver/Gold keys | Cells, Quad-related ammo |
| Powerups    | Quad Damage only                                                   | Pentagram, Ring of Shadows, Biosuit        |
| Hazards     | Lava                                                               | Slime, water volumes, drowning             |
| Difficulty  | Easy and Normal                                                    | Hard, Nightmare                            |
| Saves       | Auto-save on level start, quick save / quick load                  | Multiple save slots, save menus            |
| Stats       | Kills, Secrets, Time, Deaths                                       | —                                          |

### 11.3 Out of Scope for v1

- Water and swimming, drowning, Thunderbolt underwater discharge
- Slime hazard
- All powerups except Quad
- Multiple save slots and a save/load UI menu
- Episode hub portal art (a basic teleport-on-touch volume is enough)
- Localization
- Controller / gamepad input (mouse + keyboard only for v1)
- Cutscenes / intermissions

### 11.4 Phase Roadmap

| # | Phase | Primary deliverable | Risk |
|---|---|---|---|
| 0 | Foundation | Build infrastructure, test runner, collision channels, sandbox map loads | Low |
| 1 | Player Movement Sandbox | `UQuakeCharacterMovementComponent` with strafe-jump | **Highest** |
| 2 | Damage Pipeline + Axe | `TakeDamage` works; Axe damages a target dummy | Low |
| 3 | First Enemy (Grunt) | One Grunt patrols, sees, shoots | Medium (AI debugger, perception) |
| 4 | Shotgun + Ammo + First Pickups | Shoot Grunts; pick up shells and health | Low |
| 5 | Rocket Launcher + Splash + Knockback | Rockets explode; rocket-jumping works | Medium (knockback feel) |
| 6 | Nailgun | All 4 v1 weapons working | Low |
| 7 | More Enemies (Knight, Ogre) + Drops + Infighting | 3 enemy types; emergent infighting | Medium (Ogre grenade arc) |
| 8 | Level Structure (doors, buttons, triggers, lava) | A real level can be assembled | Low |
| 9 | Spawn Points + Stats + Level Transitions | Levels chain; exit unlocks on clear | Low |
| 10 | Powerups + Keys + Full HUD | Quad damage; locked doors; complete HUD bar | Low |
| 11 | Save/Load | Quick save/load round-trips correctly | Medium (identity, restore order) |
| 12 | Difficulty | Easy/Normal both playable; multipliers apply | Low |
| 13 | Main Menu / Hub / Win / Death | Full game flow from launch to win | Low |
| 14 | Audio Stubs + Settings | Every sound event reaches sound manager (no-op) | Low |
| 15 | Content Authoring | E1M1 / E1M2 / E1M3 / Hub built and shipped | Medium (level design effort) |

### 11.5 Phase Details

#### Phase 0: Foundation

**Goal.** Build infrastructure exists. Test runner works. Collision channels are configured. A sandbox map loads in PIE.

**Implements:**
- Stub C++ classes (empty `BeginPlay`/constructors): `AQuakeGameMode`, `AQuakePlayerController`, `AQuakePlayerState`, `UQuakeGameInstance`, `AQuakeCharacter`. Most already exist per [section 9.3](SPEC.md#L1027); fill the gaps.
- `UQuakeDamageType` base class with the shared fields from [section 1.5](SPEC.md#L95). No leaf subclasses yet.
- Custom collision channels (`Pickup`, `Projectile`, `Corpse`, `Weapon` trace) added to `Config/DefaultEngine.ini` per [section 1.6](SPEC.md#L165).
- Thin BPs: `BP_QuakeGameMode`, `BP_QuakeGameInstance`, `BP_QuakePlayerController` under `Content/Blueprints/Framework/`.
- Project Settings: `Game Instance Class = BP_QuakeGameInstance`, `Default GameMode = BP_QuakeGameMode`.
- `Content/Maps/Tests/PhysSandbox.umap` — flat plane, ramps at 30/40/45° (one valid, one borderline, one too steep), a 256-unit gap, a few low and high walls. Has a `PlayerStart` and a `NavMeshBoundsVolume`. Will be reused in Phase 1.
- Verify the UE Automation Testing plugin is enabled.

**Automated tests:**
- One trivial `IMPLEMENT_SIMPLE_AUTOMATION_TEST` in `Source/Quake/Tests/QuakeFoundationTest.cpp` that always passes — proves the test runner picks up Quake-module tests under the `Quake.*` filter.
- Build smoke test: `Build.bat` succeeds with no warnings related to new files.

**Manual verification:**
- Editor opens the project without errors.
- `PhysSandbox` map opens and shows the sandbox geometry.
- PIE on `PhysSandbox` spawns a default `AQuakeCharacter` (stock UE movement is fine for now), can look around with the mouse.
- Session Frontend → Automation tab → run `Quake.*` filter → trivial test passes.
- Collision settings panel in Project Settings shows the four custom channels.

**Exit criteria.** Trivial automation test runs green from the editor; PIE on PhysSandbox doesn't crash.

#### Phase 1: Player Movement Sandbox

**Goal.** Player can strafe-jump and bunny-hop on the sandbox map. The dot-product air-acceleration formula is correct. **This is the single highest-risk item in v1** — see [CLAUDE.md "Risk Note"](CLAUDE.md).

**Implements:**
- `UQuakeCharacterMovementComponent : public UCharacterMovementComponent` with `PhysFalling` override implementing the Quake air-acceleration formula (clamp the dot product of velocity and wishdir, NOT the velocity magnitude).
- All movement parameters from [section 1.1](SPEC.md#L21) as `UPROPERTY` defaults: `MaxGroundSpeed`, `GroundAcceleration`, `GroundFriction`, `StopSpeed`, `AirAcceleration`, `MaxAirSpeedGain`, `JumpZVelocity`, `MaxWalkableSlope`, step height 45.
- Bunny-hop window logic (100 ms post-land, jump preserves horizontal velocity).
- `AQuakeCharacter` uses the custom CMC via the constructor's `UCharacterMovementComponent` override.
- `BP_QuakePlayerController` Input Action and Input Mapping Context assets assigned (Move, Look, Jump). Authored in the Editor per the input convention from [CLAUDE.md "Architecture: Input Configuration"](CLAUDE.md).
- A debug HUD overlay showing `Speed`, `Z velocity`, and `MovementMode` as text in the corner — this is the speedometer needed to verify strafe-jump works.

**Automated tests:**
- Unit test: dot-product clamp formula. Given `Velocity = (300, 0, 0)`, `WishDir = (0, 1, 0)`, `MaxAirSpeedGain = 30`, assert post-tick velocity gains exactly 30 in the wish direction (not capped to `MaxGroundSpeed`).
- Functional test (`Tests/FT_StrafeJump.umap`): spawn pawn on flat plane, programmatically apply forward+right input + jump, simulate for 60 ticks, assert horizontal speed > `MaxGroundSpeed × 1.2`. Proves air gain is happening.
- Functional test: bunny hop sequence. Apply jump, simulate to landing, apply jump within 100 ms of landing, assert horizontal speed ≥ pre-landing speed.
- Functional test: walkable slope. Spawn on 30° ramp, apply forward, assert pawn ascends. Spawn on 50° ramp, apply forward, assert pawn does not ascend (treated as wall).

**Manual verification:** (this is the binary feel test — there is no "70% strafe-jump")
- Run around the sandbox. Speedometer shows `~600` on flat ground walking forward.
- Hold strafe + W, turn the mouse smoothly in the strafe direction while jumping. **Speedometer should climb past 600.** If it doesn't, the formula is wrong — do not advance phases until this works.
- Jump in place repeatedly while holding W. Each jump should preserve horizontal speed (bunny hop).
- Jump off the edge of the gap. Mid-air, strafe across to land on the far side. Air control is responsive but limited.
- Walk up the 30° ramp ✓, the 40° ramp ✓, the 45° ramp ✗ (treated as wall).
- Confirm: no fall damage, no crouch.

**Exit criteria.** Manual strafe-jump speed-gain test passes (speedometer climbs past `MaxGroundSpeed`); all functional tests green; `Build.bat` warns are zero.

#### Phase 2: Damage Pipeline + Axe (no enemies)

**Goal.** `TakeDamage` flows correctly. The Axe damages a static target dummy. Health appears on a minimal HUD.

**Implements:**
- `UQuakeDamageType_Melee` leaf subclass per [section 1.5](SPEC.md#L95).
- `AQuakeCharacter::TakeDamage` override: read damage type via shared-base CDO cast, apply armor absorption, apply knockback impulse, decrement health, screen flash via `UMaterialInstanceDynamic` on a `PostProcessVolume`.
- `AQuakeWeaponBase` with `Fire(Instigator)` virtual.
- `AQuakeWeapon_Axe` with melee trace using the `Weapon` trace channel.
- `AQuakeTargetDummy` (test-only `AActor`): static cube with health, override `TakeDamage`, render current HP above its head as a 3D text component for visibility.
- `AQuakeHUD` + minimal `SQuakeHUDOverlay` showing only `Health`. Polls `AQuakeCharacter` on paint.
- `BP_Weapon_Axe` (mesh slot only).

**Automated tests:**
- Functional test: spawn target dummy with 100 HP, fire Axe at it, assert HP = 80.
- Functional test: fire Axe 5 times (with frame delays for cooldown), assert dummy is dead.
- Unit test: armor absorption math. 100 HP + 100 green armor (30%), take 50 damage → assert HP = 65, armor = 85. (Quake formula: `save = ceil(0.3 * 50) = 15` taken by armor, `take = 50 - 15 = 35` taken by HP.)
- Unit test: shared-base CDO cast. Construct an `FDamageEvent` with `UQuakeDamageType_Melee::StaticClass()`, run the cast pattern from [section 1.5](SPEC.md#L155), assert the returned `UQuakeDamageType*` is non-null and has the expected default field values.

**Manual verification:**
- Swing Axe at target dummy in PhysSandbox. See HP text decrease by 20 per swing.
- Reduce Axe cooldown temporarily to verify auto-fire feels right.
- Spawn a target dummy that returns fire (test-only behavior — make it call `ApplyPointDamage` on the player on tick), confirm pain flash on screen and HUD health drops.
- Confirm: Axe range (64 units) is correct — too far and it whiffs.

**Exit criteria.** Damage flows in both directions. HUD health updates. All Phase 0–2 tests still pass.

#### Phase 3: First Enemy (Grunt) + body/brain split

**Goal.** One Grunt patrols, sees the player, shoots the player. The body/brain split works. The AI debugger overlay works.

**Implements:**
- `AQuakeEnemyBase : public ACharacter` with capsule, mesh slots, health, action methods (`MoveToTarget`, `FireAtTarget`, `PlayPainReaction`, `PlayDeathReaction`), `TakeDamage` override per [section 3.3](SPEC.md#L288).
- `AQuakeEnemyAIController : public AAIController` with the FSM (`Idle → Alert → Chase → Attack → Pain → Dead`) and `UAIPerceptionComponent` configured with `UAISenseConfig_Sight` + `UAISenseConfig_Hearing` (`bUseLoSHearing = false`).
- `AQuakeEnemy_Grunt` and `AQuakeAIController_Grunt` (hitscan rifle attack).
- `UQuakeDamageType_Bullet` leaf.
- `BP_Enemy_Grunt` with primitive mesh slots (capsule body + sphere head + box rifle).
- `OnDamaged(EventInstigator)` plumbing from pawn → controller per [section 3.3](SPEC.md#L317).

**Automated tests:**
- Functional test: spawn Grunt + player; tick 1 frame; assert Grunt state = `Idle`. Move player into sight cone; tick 1 frame; assert state = `Alert`. Tick past alert pulse duration; assert state = `Chase`.
- Functional test: spawn Grunt with player in attack range and direct LoS; assert Grunt fires within `Cooldown` seconds.
- Functional test: spawn Grunt (30 HP), apply 30 damage from player instigator, assert state = `Dead` and controller is unpossessed.
- Functional test: spawn Grunt with player out of sight cone but firing the Axe (noise), assert Grunt becomes `Alert` from hearing.
- Unit test: pain chance formula `min(0.8, damage / max_health × 2)` for several values.

**Manual verification:**
- Drop a Grunt in PhysSandbox. Walk into its sight. See it alert (visual scale pulse + sound stub log), then chase, then fire.
- Take damage from the Grunt's hitscan. HUD health drops. Pain flash plays.
- Hide behind a wall. Grunt loses sight after 5 s, stops chasing.
- Press `'` (apostrophe) in PIE for the AI debugger overlay. Verify perception cone, current state, and target are visible.
- Fire the Axe behind a wall — the Grunt should hear it (no LoS) and become Alert.
- Kill the Grunt with the Axe. It enters Dead state, falls flat, becomes non-collidable to projectiles after 2 s (per [section 1.6](SPEC.md#L165) corpse rule).

**Exit criteria.** AI debugger shows correct state transitions; manual hearing-through-walls test passes; Grunt can be killed by Axe.

#### Phase 4: Shotgun + Ammo + First Pickups (health, ammo)

**Goal.** Player can pick up shells and shoot Grunts with the Shotgun. Inventory persists in `UQuakeGameInstance`.

**Implements:**
- Ammo storage on `UQuakeGameInstance`: `TMap<EQuakeAmmoType, int32>` with caps from [section 2.1](SPEC.md#L142).
- `AQuakeWeapon_Shotgun` (hitscan with 6-pellet 4° spread, 4 dmg/pellet, 1.5 s RoF).
- `AQuakePickupBase` with `USphereComponent` overlap detection per [section 4](SPEC.md#L345).
- `AQuakePickup_Health` and `AQuakePickup_Ammo` subclasses.
- `AQuakeCharacter` facade methods: `GiveAmmo(Type, Amount)`, `ConsumeAmmo(Type, Amount)`, `GiveHealth(Amount)` — forward to GameInstance/self per the ownership model.
- HUD adds: current ammo for active weapon, weapon icon.
- `BP_Pickup_AmmoShells`, `BP_Pickup_Health25`, `BP_Pickup_Health15`, `BP_Pickup_Megahealth`, `BP_Weapon_Shotgun`.
- "Click" sound stub call when firing on empty.

**Automated tests:**
- Functional test: spawn shell pickup, walk player over it, assert `GameInstance.Ammo[Shells]` increased by 20.
- Functional test: spawn health pickup at full HP, walk over it, assert NOT consumed (still in world).
- Functional test: spawn health pickup at 50 HP, walk over it, assert HP = 75 and pickup destroyed.
- Functional test: fire Shotgun once with 1 shell, assert ammo consumed and 6 hitscan traces fired.
- Functional test: spawn 5 Grunts in a tight formation 1500 u away, fire Shotgun once, assert at least 4 took damage (cone math).
- Functional test: fire Shotgun with 0 shells, assert no traces fired and the click stub was called.

**Manual verification:**
- Pick up shell pack in PhysSandbox, see HUD ammo count change.
- Switch from Axe to Shotgun (number key 2), see weapon swap. Try to fire — no shells yet → click. Pick up shells, fire — shells deplete.
- Kill multiple Grunts with the Shotgun. Step into spread distance and out — see damage drop with distance/cone.
- Pick up health when at full HP — confirm it stays in world.
- Take damage to 50 HP, pick up Health25 — confirm it's consumed and HP = 75.
- Pick up Megahealth at 100 HP — confirm overcharge.

**Exit criteria.** HUD ammo and health both update correctly; pickup conditional consumption works; Shotgun kills Grunts.

#### Phase 5: Rocket Launcher + Splash + Knockback

**Goal.** Rockets explode with falloff. Splash damages everything in radius. Self-damage is half. Rocket jumping launches the player.

**Implements:**
- `AQuakeProjectile` base with `USphereComponent` + `UProjectileMovementComponent`.
- `AQuakeProjectile_Rocket` (1000 u/s straight, no gravity).
- `AQuakeWeapon_RocketLauncher`.
- `UQuakeDamageType_Explosive` with `SelfDamageScale = 0.5`, `KnockbackScale = 4.0`.
- `OnHit` handler: call `UGameplayStatics::ApplyRadialDamageWithFalloff` (linear falloff, 120 u radius), then destroy.
- Knockback impulse application in `AQuakeCharacter::TakeDamage` per [section 1.5](SPEC.md#L150) and [section 2.2](SPEC.md#L254).
- Muzzle spawn-out: project 60 u in front of pawn at spawn. `IgnoreActorWhenMoving(Instigator, true)` per [section 1.6](SPEC.md#L201) rule 1.
- `AQuakePickup_Ammo` for rockets.
- `BP_Weapon_RocketLauncher`, `BP_Projectile_Rocket`, `BP_Pickup_AmmoRockets`.

**Automated tests:**
- Functional test: detonate rocket at known location with 5 actors at varying distances within 120 u; assert each took damage equal to expected linear falloff.
- Functional test: spawn rocket from a pawn at origin facing +X; assert projectile transform is at `(60, 0, ZOffset)`, not `(0, 0, ZOffset)`.
- Functional test: fire rocket straight down at pawn's feet, assert pawn took 50% of full splash and was knocked upward.
- Unit test: damage falloff formula. Distance 0 → full damage; distance 60 (half radius) → half damage; distance 120 → 0; distance 200 → 0.
- Functional test: fire rocket on first frame after spawn (which previously caused self-detonation), assert no self-damage.

**Manual verification:**
- Fire rockets at Grunts. See them die from imperfect aim due to splash.
- Stand at edge of explosion radius, take partial damage.
- **Rocket jump test (binary feel):** look straight down, fire rocket, jump simultaneously. Player is launched into the air, loses ~25 HP. If the launch height is wrong or self-damage feels off, tune `KnockbackScale` or `SelfDamageScale`.
- Fire a rocket directly forward into a wall 60 u away. Confirm it explodes on the wall, not in your face.
- Fire a rocket at the floor 5 u in front of you. Confirm the muzzle spawn-out prevents instant self-detonation.

**Exit criteria.** Rocket-jump feel test passes; splash damage falloff matches the unit test expectations.

#### Phase 6: Nailgun (final v1 weapon)

**Goal.** All 4 v1 weapons working.

**Implements:**
- `AQuakeProjectile_Nail` (1500 u/s straight, gravity-free).
- `UQuakeDamageType_Nail` leaf.
- `AQuakeWeapon_Nailgun` (8 nails/sec, 1° spread).
- `AQuakePickup_Ammo` for nails.
- `BP_Weapon_Nailgun`, `BP_Projectile_Nail`, `BP_Pickup_AmmoNails`.

**Automated tests:**
- Functional test: hold fire on Nailgun for 1 s with full ammo, assert 8 projectiles spawned and 8 ammo consumed.
- Functional test: nail hits Grunt at 500 u, assert 9 damage applied.
- Functional test: fire 100 nails sequentially, assert all directions are within 1° of the firing forward vector.

**Manual verification:**
- Hose down a Grunt with the Nailgun. See it shred.
- Confirm: ammo decreases at 8/sec while held.
- Switch between all four weapons (1/2/4/7) using number keys — instant swap, no animation locking.
- Try the auto-switch on empty: deplete shotgun shells, fire — should auto-switch to next-best weapon with ammo per the priority in [section 2.2](SPEC.md#L249).

**Exit criteria.** All four v1 weapons fire correctly, swap correctly, and auto-switch on empty.

#### Phase 7: More Enemies (Knight, Ogre) + Drops + Infighting

**Goal.** Three enemy types working. Drop tables work. Infighting emerges from the existing damage pipeline + target switching.

**Implements:**
- `AQuakeEnemy_Knight` + `AQuakeAIController_Knight` (charge melee).
- `AQuakeEnemy_Ogre` + `AQuakeAIController_Ogre` (grenade arc projectile attack).
- `AQuakeProjectile_Grenade` (bouncing, fuse timer, 800 u/s, gravity 1.0).
- `OnProjectileBounce` delegate hook for grenade bounce sound + fuse check.
- Drop table on `AQuakeEnemyBase`: `TArray<FQuakeDropEntry> DropTable` per [section 3.2](SPEC.md#L259).
- Drop spawn logic in `AQuakeEnemyBase::Die` (only on non-gibbed deaths, per [section 3.4](SPEC.md#L382)).
- Friendly fire & infighting target switch in the controller's `TakeDamage`-driven `OnDamaged` flow per [section 3.3](SPEC.md#L323).
- `BP_Enemy_Knight`, `BP_Enemy_Ogre`, `BP_Projectile_Grenade`, `BP_Pickup_BackpackShells`, `BP_Pickup_BackpackRockets`.

**Automated tests:**
- Functional test: spawn Grunt with `DropTable = [{ BP_BackpackShells, 5, 1.0 }]`, kill it with player damage, assert pickup spawned at death location, asset class matches.
- Functional test: spawn Grunt with `Chance = 0.0`, kill it, assert no pickup.
- Functional test: spawn Ogre and Knight 200 u apart, place player such that the Ogre's grenade lands near the Knight, run for N seconds, assert the Knight's `CurrentTarget` is now the Ogre and its grudge timer is non-zero.
- Functional test: Knight spawn 500 u from player, walk player into sight, assert Knight enters `Chase` and moves toward player.
- Functional test: gib an enemy (apply > 2× max HP in one hit), assert no drop spawned, assert capsule is destroyed.

**Manual verification:**
- Fight Knights and Ogres in PhysSandbox.
- Pick up dropped backpacks, see ammo replenish.
- **Infighting test:** lure an Ogre near a Knight, dodge so the Ogre's grenade bounces near the Knight. Watch the Knight charge the Ogre. The Quake-classic moment.
- Gib a Grunt with a direct rocket hit. Confirm scattered pieces, no drop, blood splatter stub.

**Exit criteria.** Infighting emergent behavior works without writing infighting-specific code (it falls out of damage attribution + target switch).

#### Phase 8: Level Structure (doors, buttons, triggers, lava)

**Goal.** A real level can be assembled. The first content-shaped map is built.

**Implements:**
- `IQuakeActivatable` interface per [section 5.5](SPEC.md#L580) (pure C++ virtual).
- `AQuakeDoor` with `UStaticMeshComponent` + `UTimelineComponent`. Implements `IQuakeActivatable`. Closed-door collision blocks per [section 1.6](SPEC.md#L196).
- `AQuakeButton` (touch / shoot activation). Holds `TArray<TObjectPtr<AActor>> Targets`.
- `AQuakeTrigger` abstract base + subclasses: `_Relay`, `_Spawn`, `_Message`, `_Hurt`, `_Teleport`, `_Secret`. (`_Spawn` is implemented but not exercised until Phase 9.)
- `ExitTrigger` actor (subclass of `AQuakeTrigger` or standalone) that loads the next map name on overlap.
- `AQuakeHazardVolume` for lava: 30 dmg / 1 s tick + entry knockback per [section 5.2](SPEC.md#L495).
- `UQuakeDamageType_Lava` (with `bSuppressesPain = true`, `bCausedByWorld = true`) and `UQuakeDamageType_Telefrag`.
- `Content/Maps/Tests/LevelStructureSandbox.umap` — small room with one door, one button, one lava pit, one trigger relay, one exit. NOT a content level — for testing only.

**Automated tests:**
- Functional test: button overlap → assert door's `MovementMode == Opening` then `Open`.
- Functional test: door's swept volume contains a pawn → assert door does not begin closing.
- Functional test: pawn in lava for 1 tick → assert took 30 damage.
- Functional test: `AQuakeTrigger_Relay` with 3 targets (mock `IQuakeActivatable` actors that record calls), fire, assert all 3 received `Activate` exactly once.
- Functional test: `AQuakeTrigger_Hurt` with `DamagePerTick = 100`, overlap a pawn for `TickRate` seconds, assert pawn took 100 damage.
- Functional test: `AQuakeTrigger_Teleport` with `Destination = ATargetPoint`, overlap pawn, assert pawn at destination location.
- Functional test: closed door blocks a fired rocket (rocket explodes on the door, not behind it).

**Manual verification:**
- Walk through `LevelStructureSandbox`. Step on a button — door opens. Walk through. Step into lava — take 30 damage / sec, screen tints orange (no pain flinch from lava per spec).
- Walk into the exit trigger — see "level complete" log message (no transition yet).
- Fire a rocket at the closed door — explodes on the door.

**Exit criteria.** Activation chain works without name lookups; lava damage and door collision behave per spec.

#### Phase 9: Spawn Points + Stats + Level Transitions + Level-Clear

**Goal.** Stats work. Levels chain. Exit unlocks only when all eligible spawn points are satisfied.

**Implements:**
- `AQuakeEnemySpawnPoint` per [section 5.1](SPEC.md#L460) with `EnemyClass`, `MinDifficulty`, `bDeferredSpawn`, `bIsMarkedKillTarget`.
- `AQuakePlayerState` fields: `Kills`, `Secrets`, `TimeElapsed`, `Deaths`.
- `AQuakeGameMode::KillsTotal` and `SecretsTotal` computed at `BeginPlay` by counting eligible spawn points and `AQuakeTrigger_Secret` actors per [section 5.9](SPEC.md#L693).
- `AQuakeGameMode::IsLevelCleared()` scan over spawn points (not enemies) per [section 5.9](SPEC.md#L704).
- `AQuakeEnemySpawnPoint::IsSatisfied()` (spawned + dead).
- `ExitTrigger` checks `IsLevelCleared()` (configurable per-instance: `bGatedByClearCondition`).
- `AQuakeTrigger_Spawn` with `TArray<TObjectPtr<AQuakeEnemySpawnPoint>> SpawnPoints` (typed list).
- HUD additions: kill count `X / KillsTotal`, secret count `X / SecretsTotal`, level time.
- Level-end stats screen Slate widget (shown for 5 s before transition).
- `Content/Maps/Tests/StatsSandbox.umap` — 3 marked spawn points + 1 unmarked + 1 deferred + 2 secrets, with a clear-gated exit.

**Automated tests:**
- Functional test: spawn the StatsSandbox layout, tick 1 frame, assert `KillsTotal == 4` (3 marked + 1 deferred eligible, 1 unmarked excluded).
- Functional test: kill all 4 eligible enemies, assert `IsLevelCleared() == true`.
- Functional test: kill 3/4, assert `false`.
- Functional test: deferred spawn point — confirm spawn point exists at BeginPlay but enemy doesn't; fire the trigger; assert enemy spawns; kill it; assert clear.
- Functional test: secret trigger overlap → assert `PlayerState.Secrets += 1`. Re-enter → no change.
- Functional test: revived Zombie placeholder (spawn an enemy with `IsDead()` returning false during a Down state) → assert `IsSatisfied()` returns false during Down state, true after permanent death. (Even though Zombies aren't in v1, the scan logic should support this for forward compat.)

**Manual verification:**
- Walk through `StatsSandbox`. Kill the 3 marked Grunts. HUD shows `3 / 4`. Walk to the exit — gated. Fire the deferred-spawn trigger, the 4th enemy appears, kill it, HUD shows `4 / 4`, exit unlocks.
- Find both secrets, see the secret message, HUD increments to `2 / 2`.
- Reach the exit, see the level-end stats screen with kills/secrets/time/deaths.

**Exit criteria.** Level-clear gating works for both immediate and deferred spawn points; stats display matches reality.

#### Phase 10: Powerups + Keys + Full HUD

**Goal.** All v1 pickup categories working. Quad damage feels right. Locked doors work.

**Implements:**
- `AQuakePickup_Powerup` and `AQuakePickup_Key` and `AQuakePickup_Weapon` and `AQuakePickup_Armor`.
- `FQuakeActivePowerup { EQuakePowerup Type; float TimeRemaining; }` and `TArray<FQuakeActivePowerup> ActivePowerups` on `AQuakePlayerState`.
- `AQuakePlayerState::Tick` decrements timers, removes expired entries.
- `AQuakePlayerState::ClearPerLifeState()` per [section 1.4](SPEC.md#L93).
- `AQuakePlayerState::HasKey(EKeyColor)` and `Keys` storage.
- `AQuakeCharacter::GetOutgoingDamageScale()` queries PlayerState for Quad → returns 4.0 if active.
- Door key check: `AQuakeDoor::TryOpen(Player)` queries `PlayerState->HasKey(RequiredKey)`.
- Locked-door bump-back + on-screen "You need the [color] key" message for 2 s.
- HUD additions: armor (with tier color), keys (silver/gold icons), powerup timer (with countdown).
- `BP_Pickup_Quad`, `BP_Pickup_KeySilver`, `BP_Pickup_KeyGold`, `BP_Pickup_Armor_Green/Yellow/Red`, `BP_Pickup_Health*`.
- Powerup screen tint via `UMaterialInstanceDynamic` on the level `PostProcessVolume` (blue for Quad).
- Weapon-pickup logic: first pickup grants weapon + ammo and auto-switches; subsequent pickup grants only ammo per [section 2.2](SPEC.md#L246).

**Automated tests:**
- Functional test: pick up Quad, query `Character->GetOutgoingDamageScale()`, assert 4.0. Tick past 30 s, assert 1.0.
- Functional test: pick up Quad twice in succession, assert remaining time capped at 60 s per [section 4.3](SPEC.md#L384).
- Functional test: tick PlayerState with Quad active for 31 s, assert powerup removed from array.
- Functional test: pick up silver key, walk into silver door, assert door opens.
- Functional test: walk into silver door without key, assert door blocks player and the message log fires.
- Functional test: `ClearPerLifeState` — set powerups + keys, call clear, assert both empty, assert kills/secrets/deaths NOT cleared.
- Functional test: pick up Shotgun for the first time, assert `bHasShotgun = true` and active weapon switched. Pick up another Shotgun, assert active weapon NOT switched, ammo increased.

**Manual verification:**
- Pick up Quad, fire a Rocket Launcher at a Grunt. Watch the Grunt explode dramatically. Screen is tinted blue.
- Wait 30 s, see Quad expire, HUD timer countdown reaches 0.
- Pick up silver key — silver key icon appears on HUD top-right. Walk through silver door.
- Try a locked door without the matching key — bump back, message appears for 2 s.
- Pick up green armor — armor value shows on HUD with green color. Take damage — both health and armor decrease per the 30% absorption rule.
- Pick up the Shotgun the first time → auto-switch and full ammo. Pick up another → no switch, ammo only.

**Exit criteria.** All pickup categories work; key-locked doors work; Quad scaling works through the existing `GetOutgoingDamageScale` pull pattern.

#### Phase 11: Save / Load

**Goal.** Quick save and quick load round-trip the world state correctly. Auto-save on level start works.

**Implements:**
- `UQuakeSaveGame : public USaveGame` with all fields per [section 6.2](SPEC.md#L832).
- `IQuakeSaveable` interface with `SaveState(FActorSaveRecord&)` and `LoadState(const FActorSaveRecord&)`.
- `FActorSaveRecord { FName ActorName; TArray<uint8> Payload; }` per [section 6.2](SPEC.md#L857).
- All persistent actors implement `IQuakeSaveable`: doors, buttons, secrets, spawn points, pickups, enemies, character (for health).
- `UQuakeGameInstance::SaveCurrentState()` — gathers all records, writes via `SaveGameToSlot`.
- `UQuakeGameInstance::LoadFromSlot(SlotName)` — reads, restores GameInstance fields, calls `OpenLevel`, then on the new level's `BeginPlay` the GameMode iterates `IQuakeSaveable` actors and applies records by `GetFName()` lookup.
- Auto-save call on level entry (after the level-entry snapshot is taken).
- F5 (quick save) and F9 (quick load) input actions wired through `AQuakePlayerController`.
- "No mid-air saves" guard: F5 rejects if `MovementMode != MOVE_Walking` or if pain reaction active.

**Automated tests:**
- Functional test: open a door, save, reload the level (without restarting PIE), load the save, assert door's saved state restored as Open.
- Functional test: kill an enemy at a spawn point, save, reload level, load, assert spawn point still satisfied (enemy still dead — not respawned).
- Functional test: pick up health to full HP, save, take damage, load, assert HP restored to saved value.
- Functional test: save with active Quad (15 s remaining), load, assert Quad active with ~15 s remaining (within 0.1 s tolerance).
- Functional test: F5 save while jumping, assert save call returned false / save file not written.
- Functional test: F5 save while pain-reacting, assert save call returned false.
- Functional test: save record `FName` round-trip — save a level, deserialize, walk records, assert each `ActorName` is non-empty and matches a level actor's `GetFName()`.
- Functional test: actor with no matching record at load time falls back to its level-default state (no crash).

**Manual verification:**
- Play through half a level, F5. Take damage to near-death. F9 — assert state restored to save point including HP, ammo, powerup timer, opened doors, dead enemies, picked-up items.
- Save in level 2 with weapons from level 1. Restart the editor entirely. Reopen, F9 — assert weapons present and level 2 loaded.
- Try to F5 mid-jump — see message "Cannot save while airborne" or similar.

**Exit criteria.** Quick save/load round-trips state correctly across PIE restarts; identity-by-`GetFName()` works without false matches.

#### Phase 12: Difficulty

**Goal.** Easy and Normal both playable. Multipliers apply correctly to damage and HP. Spawn-point filtering enables/disables placements.

**Implements:**
- `EQuakeDifficulty` enum.
- `Difficulty` field on `UQuakeGameInstance`, set at new-game time.
- `FQuakeDifficultyMultipliers` USTRUCT and `TMap<EQuakeDifficulty, FQuakeDifficultyMultipliers> DifficultyTable` on `AQuakeGameMode` per [section 6.1](SPEC.md#L753).
- `AQuakeEnemyBase::ApplyDifficultyScaling()` (virtual) called from `BeginPlay`.
- `AQuakeEnemySpawnPoint::IsEligible()` checks `MinDifficulty` (already implemented as a stub in Phase 9; this phase makes it functional).
- `AQuakeAIController_Zombie::ApplyDifficultyScaling()` override for revive timing — even though Zombies aren't in v1 content, the override slot exists for v2.
- Pain-immunity check in `AQuakeEnemyAIController::OnDamaged` for Nightmare — same future-proofing.
- `BP_QuakeGameMode` populates the multiplier `TMap` with the four difficulty entries.

**Automated tests:**
- Functional test: spawn Grunt on Easy, assert `MaxHealth == 30 × 1.0`. Spawn on Hard (set GameInstance directly for the test), assert `MaxHealth == 30 × 1.25`.
- Functional test: spawn level with 5 `MinDifficulty=Easy` + 3 `MinDifficulty=Hard` spawn points; set difficulty Easy → assert `KillsTotal == 5`. Set Hard → assert `KillsTotal == 8`.
- Functional test: enemy fires at player on Easy/Normal/Hard, assert damage applied matches the per-difficulty multiplier.
- Functional test: change difficulty between two PIE sessions, verify the spawn-point filtering kicks in correctly per session.

**Manual verification:**
- Start a new game on Easy, fight a few Grunts, note time-to-kill and damage taken.
- Restart with Normal, same room, feel the difference (damage taken roughly +33% per the 0.75 → 1.0 jump).
- (Hard / Nightmare are deferred to v2 per content scope, but the implementation should still support them for tests.)

**Exit criteria.** Per-difficulty multipliers apply at spawn time; spawn-point filtering correctly excludes higher-tier placements.

#### Phase 13: Main Menu / Hub / Win Sequence / Death Restart

**Goal.** The full game flow from launch to win works.

**Implements:**
- `MainMenu.umap` with title screen and main menu Slate widget (`SQuakeMainMenu`).
- New-game flow: difficulty select → load Hub → walk into portal → load E1M1.
- `Hub.umap` with a single `AQuakeTrigger_Teleport` representing the Episode 1 portal (no art).
- Death screen Slate widget with "Press Fire to Restart".
- Restart sequence per [section 6.4](SPEC.md#L894): increment Deaths → `ClearPerLifeState` → restore inventory snapshot → respawn at PlayerStart.
- Win screen Slate widget shown after the final level's exit, with total stats per [section 6.3](SPEC.md#L885).
- Level-end stats screen (already partial in Phase 9) finalized.
- Settings Slate widget with mouse sensitivity (functional) and audio volume sliders (no-op until Phase 14).
- Game Mode flag for "is final level of episode" → triggers win screen instead of next-level transition.

**Automated tests:**
- Functional test: simulate new-game flow programmatically, set difficulty, verify `GameInstance->Difficulty` matches.
- Functional test: kill player programmatically, advance to restart, assert `PlayerState.Deaths += 1`, assert powerups cleared, assert inventory matches the snapshot (NOT pre-death state).
- Functional test: complete the final level (mock by directly calling the win flow), assert win screen widget exists in the viewport.
- Functional test: time pauses while paused → tick the GameMode while paused, assert `PlayerState.TimeElapsed` does not increase.

**Manual verification:**
- Main menu → New Game → Easy → spawn in Hub.
- Walk into portal → loads `E1M1` (a placeholder map for now — Phase 15 fills it in).
- Die in PhysSandbox by jumping into a kill volume (or take Lava damage to 0). See death screen. Press fire → respawn at PlayerStart with snapshot inventory, +1 to Deaths.
- Manually trigger the win flow → see the stats screen and "return to main menu" option.

**Exit criteria.** Full launch-to-win flow works with placeholder content; restart sequence correctly clears per-life state without touching cumulative stats.

#### Phase 14: Audio Stubs + Settings

**Goal.** Every gameplay sound event reaches the (no-op) sound manager. Settings work.

**Implements:**
- `UQuakeSoundManager : public UGameInstanceSubsystem`.
- `ESoundEvent` enum cataloging every sound from [section 8.2](SPEC.md#L928).
- `UQuakeGameInstance::SoundEventTable` `UPROPERTY` and `GetSoundEventTable()` accessor per [section 8.1](SPEC.md#L908).
- `BP_QuakeGameInstance` assigns `DT_SoundEvents.uasset`.
- `DT_SoundEvents.uasset` with row type `FQuakeSoundEvent`, all rows present but `Sound = nullptr`.
- `UQuakeSoundManager::PlaySound(ESoundEvent, FVector)` — looks up the row, calls `UGameplayStatics::PlaySoundAtLocation` if non-null, no-op otherwise. Logs every call at Verbose level for verification.
- Insert `PlaySound` calls at every gameplay event: weapon fire, weapon empty click, weapon pickup, item pickup, enemy alert/pain/death/attack, door open/close, button press, secret found, footstep, jump, land, player pain/death.
- Settings persistence via `UGameUserSettings` subclass with mouse sensitivity + master volume.
- Settings widget reads/writes via `UGameUserSettings`.

**Automated tests:**
- Functional test (with a mock subsystem variant): fire each weapon, assert `PlaySound` was called with the expected `ESoundEvent` for each.
- Functional test: open a door, assert `door_open` was called.
- Functional test: pick up a health pack, assert `pickup_item` was called.
- Functional test: settings — set mouse sensitivity to 2.0, save, reload, assert restored value matches.

**Manual verification:**
- Set log level to Verbose for `LogQuakeSound`. Run through a level and observe the log: every gameplay action logs one `PlaySound` call.
- Open settings, change sensitivity to 2.0 and back to 1.0, confirm the camera responds immediately.
- Confirm volume slider exists and saves (no audible effect yet).

**Exit criteria.** Every gameplay event reaches the sound manager; nothing in the codebase calls `UGameplayStatics::PlaySoundAtLocation` directly.

#### Phase 15: Content Authoring

**Goal.** `E1M1`, `E1M2`, `E1M3`, and `Hub` are real shippable maps that meet the [v1 Definition of Done](#116-v1-definition-of-done).

**Implements:**
- `Content/Maps/Hub.umap` — single room with the Episode 1 portal teleporter and "Quit" trigger. No art beyond primitive shapes.
- `Content/Maps/E1/E1M1.umap` — tutorial map. ~5 rooms, Grunts only, one silver key, one locked door, one secret, one button, lava pit, exit trigger. Estimated 5–10 minutes of gameplay.
- `Content/Maps/E1/E1M2.umap` — full map. Grunts + Knights + Ogres, gold key, multiple secrets, deferred spawns, lava hazards. ~10–15 minutes.
- `Content/Maps/E1/E1M3.umap` — boss map. Leads to a final arena with the Greater Ogre (Ogre subclass with 800 HP and faster grenade RoF). Win triggers on boss death.
- `AQuakeEnemy_GreaterOgre` C++ subclass of `AQuakeEnemy_Ogre` with the boss stat overrides.
- Per-level checklist from [section 10.5](SPEC.md#L1183) followed for each map.

**Automated tests:**
- Functional test per level: programmatically spawn at PlayerStart, run UE Navigation pathfinding to the ExitTrigger location, assert a path exists. Catches "I built a level the AI can't navigate" issues.
- Functional test per level: load the map, tick 1 frame, assert `KillsTotal > 0`, assert at least one `IQuakeActivatable` exit trigger exists.
- Functional test: navigate from MainMenu → Hub → E1M1 → E1M2 → E1M3 → win (drive by simulated player input and direct level-clear scripting), assert the win screen appears at the end.
- Functional test: each map has all the [section 10.5](SPEC.md#L1183) per-level checklist items present (PlayerStart, NavMeshBoundsVolume, PostProcessVolume, light sources, exit trigger).

**Manual verification:**
- The full v1 Definition of Done playthrough (see 11.6 below).

**Exit criteria.** All four maps shippable; the v1 Definition of Done passes manual playthrough end-to-end.

### 11.6 v1 Definition of Done

The milestone is complete when a player can:

1. Launch the game, see the main menu, choose difficulty, and start a new game.
2. Spawn into the hub, walk into the Episode 1 portal, and load E1M1.
3. Fight Grunts, Knights, and Ogres using v1 weapons, with full strafe-jump movement.
4. Pick up keys, open locked doors, find secrets, take damage, take splash damage from rockets, rocket-jump.
5. Die, restart from level entry with the snapshot inventory.
6. Quick save mid-level and quick load successfully.
7. Complete E1M1, transition to E1M2, transition to E1M3, defeat the Greater Ogre boss, and see the win screen with stats.
8. Return to main menu and start a new game on a different difficulty.

**All Phase 0–15 automated tests pass.** All previous-phase tests continue to pass at the time of milestone completion (no regressions).
