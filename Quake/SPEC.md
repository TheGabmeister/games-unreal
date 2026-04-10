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

**Storage location: `AQuakePlayerState`** for **keys.** Although keys are colloquially "inventory," their lifecycle (reset on level transition, reset on death, reset on hub return) is identical to powerups, not to weapons/ammo/armor. PlayerState is destroyed automatically on `OpenLevel` and on death-respawn, so storing keys there gives the correct lifecycle for free with no explicit cleanup code. Powerups already live on PlayerState for the same reason (see 4.3). See section 4.4 for the full key rules.

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
| **Corpse capsule** (`Corpse`)      | B | B | I | I | I | I | B | I |
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
2. **Corpse channel flip.** When `AQuakeEnemyBase::Die()` runs, the capsule keeps its `Pawn` channel for 2 seconds (so a freshly-dead body still blocks the player and reads as a hit target during gib chains). After 2 s, a timer flips the capsule to the `Corpse` channel. The `Corpse` channel ignores `Projectile`, `Weapon` (hitscan), and `Pawn` — meaning rockets, hitscan, and the player all pass straight through corpses. Corpses still block `WorldStatic` so they don't fall through floors.
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
- **`UAISenseConfig_Hearing`** — hearing range set per enemy from section 3.1's Hearing column. Walls do **not** block hearing (matching Quake) — set `bUseLoSHearing = false`. Noise events are reported via `UAISense_Hearing::ReportNoiseEvent` whenever the player fires a non-Axe weapon or takes damage.

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

**Storage: `AQuakePlayerState`.** Keys live on the PlayerState, not on `UQuakeGameInstance`, because their lifecycle (reset on level transition, reset on death, reset on hub return) matches PlayerState's destruction lifecycle exactly — same reasoning as powerups in 4.3. PlayerState is destroyed by `OpenLevel` and by death-respawn, so the reset rules in section 1.4 are automatic with no cleanup code. Door key checks call `Player->GetPlayerState<AQuakePlayerState>()->HasKey(Color)`. Key pickups call `Player->GiveKey(Color)` which forwards to the PlayerState, matching the Character-as-facade pattern used elsewhere for state writes.

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

**`AQuakeEnemySpawnPoint`.** A passive marker actor placed in the level that defines where an enemy should appear. Each spawn point holds the enemy class to spawn, the lowest difficulty at which it activates, and a flag controlling whether it spawns automatically at level start or waits to be fired by a trigger. Spawn points implement `IQuakeActivatable` so they can also be targets of generic buttons / `AQuakeTrigger_Relay` chains, in addition to the typed `AQuakeTrigger_Spawn` (see 5.6).

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

    virtual void BeginPlay() override;
    virtual void Activate_Implementation(AActor* Instigator) override;  // IQuakeActivatable

protected:
    UPROPERTY() TObjectPtr<AQuakeEnemyBase> SpawnedEnemy;  // null until spawned; ensures one-shot
    bool TrySpawn();  // checks MinDifficulty + SpawnedEnemy, then spawns
};
```

**Behavior:** `BeginPlay` calls `TrySpawn()` if `bDeferredSpawn == false`. `Activate(Instigator)` calls `TrySpawn()` regardless. `TrySpawn` checks the current difficulty against `MinDifficulty` (via `GameMode->GetDifficulty()`), bails if too low, bails if already spawned, otherwise spawns `EnemyClass` at the actor's transform and stores the result in `SpawnedEnemy`. Each spawn point spawns at most once per level attempt.

**Why a flag instead of two subclasses:** the level-start vs deferred behaviors share 90% of the code (difficulty check, one-shot guard, actual spawn call). A boolean is simpler than `AQuakeEnemySpawnPoint_LevelStart` / `AQuakeEnemySpawnPoint_Deferred` subclasses, and a designer can flip the flag in the editor without re-placing the actor.

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

**Activation model: `IQuakeActivatable` interface, direct actor references — no string-name targeting.** Quake's original `targetname`/`target` string lookup is not used. Instead, every fireable actor (`AQuakeDoor`, `AQuakeTrigger` and its subclasses, `AQuakeSecret`, level exit, etc.) implements a small `UInterface` declared in C++:

```cpp
// QuakeActivatable.h
UINTERFACE(MinimalAPI, BlueprintType)
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

    virtual void Activate_Implementation(AActor* Instigator) override;  // fires Targets, then runs subclass-specific behavior
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

- **`AQuakeGameMode`** owns the **denominators**: `KillsTotal` and `SecretsTotal`. Both are computed once at `BeginPlay` (by counting `AQuakeEnemyBase` actors with `bIsMarkedKillTarget = true` and `AQuakeTrigger_Secret` volumes in the level) and **never updated** afterward. They represent "how many existed in this level," not "how many have happened so far." `bIsMarkedKillTarget` is a `UPROPERTY(EditAnywhere)` flag on `AQuakeEnemyBase` that defaults to `true`; level designers flip it off for decoration enemies, infinitely-spawning enemies, or scripted-display enemies that shouldn't gate the level-clear check.
- **`AQuakePlayerState`** owns the **numerators**: `Kills`, `Secrets`, `TimeElapsed`, `Deaths`. These run up as the player progresses and represent the player's score for the level attempt.

The HUD displays `Kills / KillsTotal` and `Secrets / SecretsTotal` by reading the numerator from `PlayerState` and the denominator from `GameMode`. The end-of-level stat screen does the same. There is **no third counter** for "how many kills have happened" — the level-clear check is computed on demand (see below).

Game-mode events (an enemy dying, a secret being entered) call into the PlayerState to increment counters where the player has earned credit. The HUD reads PlayerState directly via `PlayerController->GetPlayerState<AQuakePlayerState>()` and reads GameMode via `GetWorld()->GetAuthGameMode<AQuakeGameMode>()`.

**Level-clear check.** "Has the player cleared the level?" is computed on demand by scanning the world rather than tracked as a separate counter. This avoids drift from miscounted edge cases (revived Zombies, infighting, hazard kills) and matches the SPEC's preference for derived state over cached state:

```cpp
bool AQuakeGameMode::IsLevelCleared() const
{
    for (TActorIterator<AQuakeEnemyBase> It(GetWorld()); It; ++It)
    {
        if (It->IsMarkedKillTarget() && !It->IsDead()) return false;
    }
    return true;
}
```

The exit unlocks when this returns true. Note this is independent of `PlayerState.Kills` — an enemy that died from infighting without player credit still satisfies the clear condition (it's dead), even though the player doesn't get a point for it on the score screen.

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

| Difficulty | Enemy Damage | Enemy HP | Enemy Count | Notes                                  |
|------------|--------------|----------|-------------|----------------------------------------|
| Easy       | ×0.75        | ×1.0     | ×1.0        | Forgiving, for new players             |
| Normal     | ×1.0         | ×1.0     | ×1.0        | Default                                |
| Hard       | ×1.5         | ×1.25    | ×1.25       | Extra enemies in marked spawn slots    |
| Nightmare  | ×2.0         | ×1.5     | ×1.5        | Zombies revive 2× faster; pain immunity for all enemies |

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

- Player profile: difficulty, total stats across all levels (kills, secrets, time, deaths)
- Inventory snapshot (from `UQuakeGameInstance`): weapons owned, ammo per type, armor type and value, current health
- Level state: current level name, player transform (position + rotation)
- PlayerState snapshot: current-level kills, secrets, time, deaths, and the active powerups array (type + remaining duration for each)
- Actor state: per-level array of `FActorSaveRecord` capturing alive/dead status, current state (for doors and one-shot triggers), and any other per-actor persistent data

On save, the GameInstance and the active `AQuakePlayerState` both serialize their fields into the save game struct. On load, the GameInstance restores the inventory immediately (before `OpenLevel`), and after the new level loads, the GameMode restores the PlayerState fields and then iterates `IQuakeSaveable` actors to apply per-actor records.

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

**Per-actor save participation:** any actor that has persistent state implements `IQuakeSaveable` (a C++ interface) with `SaveState(FActorSaveRecord&)` and `LoadState(const FActorSaveRecord&)` methods. The game mode iterates `IQuakeSaveable` actors at save time and applies records by tag at load time.

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

- **`UQuakeSoundManager`** (Game Instance Subsystem) — central audio manager.
- Exposes functions like `PlaySound(ESoundEvent, Location)`, `PlayMusic(EMusicTrack)`, `StopMusic()`.
- `ESoundEvent` is an enum cataloging every game sound (weapon fire, pickup, enemy alert, door open, etc.).
- Sounds are mapped to `USoundBase*` via a `UDataTable` whose row asset is assigned in a Blueprint subclass of the manager (or referenced from the Game Instance). When a row is unmapped (nullptr), the call is a no-op.
- This means all gameplay code calls the sound manager, and adding audio later is just filling in the data table rows in the Editor.

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
  bIsMarkedKillTarget (UPROPERTY EditAnywhere, default true) — counts toward GameMode.KillsTotal; flip off for decoration / infinitely-spawning enemies
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

AQuakeEnemySpawnPoint                — passive level marker; spawns EnemyClass at level start (or on Activate); MinDifficulty gates; implements IQuakeActivatable

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
- **Current-level stats** (kills, secrets, time, deaths), **active powerups**, and **keys held** live on `AQuakePlayerState`. Recreated on every level transition, which matches the reset semantics — keys live here (rather than on `UQuakeGameInstance` with the rest of "inventory") because their lifecycle is per-level, matching powerups (see 1.4 / 4.4). The HUD reads them directly via `PlayerController->GetPlayerState<AQuakePlayerState>()`.
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
    GameMode/
      BP_QuakeGameMode.uasset      — assigns default pawn, HUD, sound table
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

**Project Settings → Navigation System.** Configure a single Supported Agent matching the player capsule:
- Agent Radius: 35
- Agent Height: 180
- Agent Max Slope: 44
- Agent Max Step Height: 35

Press **P** in the editor viewport to visualize the NavMesh. Green = walkable. If enemies refuse to chase, this is the first thing to check.

### 10.4 Project Settings

All values below are stored in `Config/Default*.ini` and committed to version control.

**Maps & Modes:**
- `Default GameMode` = `BP_QuakeGameMode`
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
- [ ] Enemy spawn placements (`BP_Enemy_*` actors dragged into the level)
- [ ] Pickup placements (`BP_Pickup_*` actors)
- [ ] Doors (`AQuakeDoor` actors), keyed appropriately
- [ ] An `ExitTrigger` (C++ actor) at the level end, configured with the next map name
- [ ] Optional: `AQuakeHazardVolume` actors for slime/lava areas
- [ ] Optional: `AQuakeTrigger` actors for scripted spawns or area events
- [ ] World Partition disabled (uncheck on level creation)
- [ ] Level Blueprint event graph: empty

---

## 11. v1 Scope (First Playable Milestone)

The first milestone targets a small, complete vertical slice rather than the full game. The goal is to prove the core systems end-to-end, then expand in v2+.

### 11.1 Content

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

### 11.2 Systems Required for v1

These must be working for the milestone to be considered complete:

- Player movement with strafe/bunny hop physics (section 1.1)
- Inventory persistence and level-entry snapshot (section 1.4)
- All v1 weapons with full behavior (section 2)
- Enemy AI: state machine, sight/hearing, infighting, friendly fire, drops (section 3)
- Pickup system with all v1 pickups (section 4)
- Doors with key locks, buttons, generic triggers, exit triggers (section 5.4–5.6)
- Lava hazard volumes (section 5.2)
- Stat tracking (section 5.9)
- Difficulty selection at new game (section 6.1)
- Quick save / load and auto-save (section 6.2)
- Win sequence and death/restart loop (section 6.3, 6.4)
- HUD: health, armor, ammo, weapon bar, keys, powerup timer, crosshair, level stats (section 7)
- Audio system stubs in place — every gameplay sound event reaches `UQuakeSoundManager` even though no assets are mapped (section 8)
- Main menu (new game, load, quit) and difficulty select
- Settings: mouse sensitivity, audio volume sliders (no-op until audio assets exist)

### 11.3 Out of Scope for v1

These can be designed and stubbed but should not block the milestone:

- Water and swimming, drowning, Thunderbolt underwater discharge
- Slime hazard
- All powerups except Quad
- Multiple save slots and a save/load UI menu
- Episode hub portal art (a basic teleport-on-touch volume is enough)
- Localization
- Controller / gamepad input (mouse + keyboard only for v1)
- Cutscenes / intermissions

### 11.4 v1 Definition of Done

The milestone is complete when a player can:

1. Launch the game, see the main menu, choose difficulty, and start a new game.
2. Spawn into the hub, walk into the Episode 1 portal, and load E1M1.
3. Fight Grunts, Knights, and Ogres using v1 weapons, with full strafe-jump movement.
4. Pick up keys, open locked doors, find secrets, take damage, take splash damage from rockets, rocket-jump.
5. Die, restart from level entry with the snapshot inventory.
6. Quick save mid-level and quick load successfully.
7. Complete E1M1, transition to E1M2, transition to E1M3, defeat the Greater Ogre boss, and see the win screen with stats.
8. Return to main menu and start a new game on a different difficulty.
