# Quake — Design Reference

Single-player FPS inspired by original Quake (1996), rebuilt in Unreal 5.7 with primitive shapes. This doc is the **durable "what the game is"** reference — state, formulas, tables, class hierarchy, collision rules. It describes the v1+ target; v1 scoping lives in [ROADMAP.md](ROADMAP.md). HUD layout lives in [HUD.md](HUD.md). Architecture rationale lives in [CLAUDE.md](CLAUDE.md).

**Cross-reference convention.** Code comments use SPEC / DESIGN section numbers (e.g. "SPEC 3.3", "DESIGN 1.5"). Numbers are stable; treat them as interface. [SPEC.md](SPEC.md) is a thin redirect index for historical refs.

## Constraints

- **C++ first, thin BP layer.** All gameplay logic in C++. Blueprints hold only asset slots and per-instance property tuning. Zero event-graph nodes.
- **Primitive shapes.** Cubes, spheres, cylinders, capsules. No skeletal meshes or animations.
- **No audio assets in v1.** Audio system ships with placeholder hooks; sound assets slot in later.
- **Single-player only.** No networking or multiplayer.

---

## 1. Player

### 1.1 Movement

First-person camera with Quake-style physics. Single ground speed cap (no sprint). Strafe-jumping and bunny-hopping emerge from low air acceleration with a dot-product clamp.

| Attribute            | Value | Notes                                        |
|----------------------|-------|----------------------------------------------|
| Max ground speed     | 600   | Horizontal cap on ground                     |
| Ground acceleration  | 6000  | u/sec²                                       |
| Ground friction      | 8     | Velocity decay mult per second               |
| Stop speed           | 100   | Below this, friction doubled                 |
| Air acceleration     | 100   | u/sec² — intentionally low                   |
| Air control          | 0.3   | UE air-control multiplier                    |
| Max air speed gain   | 30    | Per air-strafe tick (dot-product clamp)      |
| Jump velocity        | 420   |                                              |
| Gravity              | 980   | UE default                                   |
| Max walkable slope   | 44°   |                                              |
| Step height          | 45    |                                              |
| Crouch               | —     | Matches original Quake                       |
| Fall damage          | —     | Matches original Quake                       |
| Max HP               | 100   | Overcharges to 200                           |
| Max armor            | 200   | Red tier cap                                 |

**Implementation.** Custom `UQuakeCharacterMovementComponent` overrides `CalcVelocity` (falling branch) with the Quake air-accel formula: clamp `dot(velocity, wishdir)`, **not** velocity magnitude. Stock CMC's magnitude clamp is exactly what breaks strafe-jumping. The dot-product clamp is the single most-important line in the project — see CLAUDE.md "Risk Note: Strafe-Jumping CMC."

**Bunny-hop window:** if Jump is pressed within 100 ms of landing, horizontal velocity is preserved and the next jump executes immediately.

### 1.2 Health and Armor

- Health starts at 100. Megahealth overcharges to 200; decays back to 100 at 1 HP/sec.
- Armor absorbs a fraction of incoming damage. Tiers:

| Tier   | Value | Absorb |
|--------|-------|--------|
| Green  | 100   | 30%    |
| Yellow | 150   | 60%    |
| Red    | 200   | 80%    |

- Armor does **not** stack. New armor replaces current only if (a) new tier absorbs more or (b) current armor value < new pickup's value.
- Absorption formula (pure-static helper `AQuakeCharacter::ApplyArmorAbsorption`):

```
save = min(ceil(absorption * damage), armor)
take = damage - save
newHealth = health - take
newArmor  = armor  - save
```

Subtract `UE_KINDA_SMALL_NUMBER` before the ceil — float imprecision on 0.3×50 = 15.000000596... rounds up to 16 otherwise.

### 1.3 Death and Respawn

- Death screen overlay; restart from last level start.
- No lives system.
- On restart, inventory restores to the **level-entry snapshot** (see 1.4).

### 1.4 Inventory Persistence

| Item     | Across levels (same episode) | On death (level restart)       | Hub return / new episode |
|----------|------------------------------|--------------------------------|--------------------------|
| Weapons  | Carry                         | Restore to snapshot           | Reset to starting loadout |
| Ammo     | Carry                         | Restore to snapshot           | Reset to starting loadout |
| Armor    | Carry                         | Restore to snapshot           | Reset to starting loadout |
| Health   | Reset to 100 on level start   | Reset to 100                  | Reset to 100              |
| Powerups | Expire on level transition    | Cleared                       | Cleared                   |
| Keys     | Reset on level transition     | Reset                         | Reset                     |

**Starting loadout:** Axe, 25 shells, 100 HP. No other weapons, armor, keys, or powerups.

**Ownership.**

| State                              | Home                      | Why                                         |
|------------------------------------|---------------------------|---------------------------------------------|
| Weapons owned, ammo, armor          | `UQuakeGameInstance`      | Must survive `OpenLevel` and pawn respawn   |
| Level-entry snapshot               | `UQuakeGameInstance`      | Death restores from it                      |
| Current-level stats, powerups, keys | `AQuakePlayerState`       | Auto-cleared on `OpenLevel`                 |
| Live HP                            | `AQuakeCharacter`         | Dies with the body; reset per above         |
| Level totals (Kills/SecretsTotal)   | `AQuakeGameMode`          | Computed once at `BeginPlay`                |

**PlayerState lifecycle gotcha.** `OpenLevel` destroys and recreates PlayerState (clears everything automatically). Death-respawn **persists** PlayerState (UE preserves it across pawn recreation), so the death-restart flow in 6.4 must explicitly call `AQuakePlayerState::ClearPerLifeState()` to empty powerups + keys. Kills/Secrets/Time/Deaths stay put — they're score, not life-bound.

### 1.5 Damage Pipeline

All damage flows through `AActor::TakeDamage`. Weapons call `UGameplayStatics::ApplyPointDamage` / `ApplyRadialDamageWithFalloff` / `ApplyDamage`; targets decide the effect. **No code outside `TakeDamage` mutates health.**

**Shared-base damage type.** All Quake damage types inherit from `UQuakeDamageType : UDamageType` (abstract) which owns every Quake-specific UPROPERTY. Leaf subclasses only override defaults in their constructor.

```cpp
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
```

Read metadata via the shared-base CDO cast — **never branch on leaf class identity:**

```cpp
const UQuakeDamageType* DT = Cast<UQuakeDamageType>(
    DamageEvent.DamageTypeClass
        ? DamageEvent.DamageTypeClass->GetDefaultObject()
        : UQuakeDamageType::StaticClass()->GetDefaultObject());
```

Engine-provided fields on `UDamageType` are reused where they fit (`bCausedByWorld`, `bScaleMomentumByMass`, `DamageImpulse`, `DamageFalloff`) — no parallel Quake equivalents.

| Type                    | Overrides                                                                              |
|-------------------------|----------------------------------------------------------------------------------------|
| `_Melee`                | (defaults)                                                                             |
| `_Bullet`               | (defaults)                                                                             |
| `_Nail`                 | (defaults)                                                                             |
| `_Explosive`            | `SelfDamageScale = 0.5`, `KnockbackScale = 4.0`. Splash radius lives on the weapon.    |
| `_Lightning`            | `bIgnoresArmor = true`                                                                 |
| `_Lava`                 | `bSuppressesPain = true`, `bCausedByWorld = true`                                      |
| `_Slime`                | `bCausedByWorld = true`                                                                |
| `_Drown`                | `bIgnoresArmor = true`, `bBypassesBiosuit = true`, `bCausedByWorld = true`             |
| `_Telefrag`             | `bSuppressesPain = true`. Damage amount (10000) is passed by caller, not stored.       |

**Attribution** uses standard `EventInstigator` (controller) + `DamageCauser` (projectile/weapon):

- Self-damage = `DamagedActor == EventInstigator->GetPawn() && DT->bSelfDamage`. Scale by `DT->SelfDamageScale`.
- Infighting: if instigator is another enemy, target switch kicks in (3.3).
- Knockback is computed from `FDamageEvent::GetBestHitInfo` (unifies point vs radial); magnitude = `ScaledDamage × 30 × DT->KnockbackScale`. Do **not** use `ApplyDamageMomentum` — its fixed magnitude loses damage scaling and breaks rocket-jump variability.

Only `AQuakeCharacter::TakeDamage` and `AQuakeEnemyBase::TakeDamage` decrement health.

### 1.6 Collision Model

Player capsule: radius 35, half-height 90. NavMesh agent matches (radius 35, height 180, step 45).

**Custom object channels:**

| Channel      | Used by                                                |
|--------------|--------------------------------------------------------|
| `Pickup`     | `AQuakePickupBase`'s sphere (overlap-only)             |
| `Projectile` | `AQuakeProjectile_*` sphere                            |
| `Corpse`     | Enemy capsule after the 2 s post-death channel flip    |

**Custom trace channel:** `Weapon` — hitscan weapons (Shotgun, SSG, Thunderbolt) and Axe melee.

**Response matrix** (rows = this object's `CollisionObjectType`; columns = response to other's channel). `B`=Block, `O`=Overlap, `I`=Ignore.

|                                          | WorldStatic | WorldDynamic | Pawn | Pickup | Projectile | Corpse | Visibility | Weapon |
|------------------------------------------|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
| **Player capsule** (`Pawn`)              | B | B | B | O | B | I | B | B |
| **Enemy capsule** (`Pawn`)               | B | B | B | I | B | I | B | B |
| **Corpse capsule** (`Corpse`)            | B | B | I | I | I | I | I | I |
| **Projectile sphere** (`Projectile`)      | B | B | B | I | I | B | I | I |
| **Pickup sphere** (`Pickup`)             | I | I | O | I | I | I | I | I |
| **Hazard volume** (`WorldDynamic`)        | I | I | O | I | I | O | I | I |
| **Water volume** (`WorldDynamic`)         | I | I | O | I | I | I | I | I |
| **Trigger (player-only)** (`WorldDynamic`)| I | I | O (filtered in C++) | I | I | I | I | I |
| **Trigger (any pawn)** (`WorldDynamic`)   | I | I | O | I | I | O | I | I |
| **Door closed** (`WorldDynamic`)          | B | B | B | I | B | I | B | B |
| **Door open/opening** (`WorldDynamic`)    | B | B | I | I | I | I | B | I |

**Rules the matrix can't express:**

1. **Projectile muzzle spawn-out.** Projectiles spawn 60u in front of the firer's eye. `AQuakeProjectile::BeginPlay` also calls `IgnoreActorWhenMoving(Instigator, true)` as a belt-and-braces second guard; kept on for the projectile's lifetime. A third guard in `OnSphereHit` bails when `OtherActor == Instigator`.
2. **Corpse channel flip.** On `Die`, capsule stays `Pawn` for 2 s (so fresh corpse still blocks the player and reads as a hit target). Then a timer flips to `Corpse`, which ignores `Projectile`, `Weapon`, `Pawn`, **and `Visibility`** (so splash line-of-sight passes through corpses). Still blocks `WorldStatic` so corpses don't fall through floors.
3. **Gibbed enemies.** Capsule destroyed with the actor; primitive pieces become `PhysicsBody` and don't participate in damage.
4. **Pickup overlap is player-only.** Sphere overlaps `Pawn`; C++ handler casts to `AQuakeCharacter` and bails on null. Enemies trigger the overlap (harmless) but never consume.
5. **Player-only triggers** (`_Teleport`, `_Message`, `ExitTrigger`, `_Secret`) use the same C++-side cast-and-bail pattern.
6. **Hazard volumes** damage any pawn (player and enemies). Pushing enemies into hazards is a valid tactic.
7. **Doors block projectiles when closed.** Open doors are non-blocking for `Pawn`/`Projectile`/`Weapon`.
8. **Water detection.** `AQuakeWaterVolume` overlaps `Pawn`; "submerged" vs "partially in" is computed from camera position relative to volume bounds, not a separate channel.
9. **Hitscan traces** use `Weapon` channel. Block walls/pawns/closed doors; ignore `Pickup` and `Corpse`.
10. **Splash LoS** uses `Visibility` channel (engine default for `ApplyRadialDamageWithFalloff`). Explosions don't penetrate walls.

All channels live in `Config/DefaultEngine.ini` `[/Script/Engine.CollisionProfile]`. Per-actor responses set in C++ constructors. No collision profile assets in the editor. Reference channels via `QuakeCollision::ECC_*` constexpr mirror (see CLAUDE.md) — never raw `ECC_GameTraceChannelN` literals.

---

## 2. Weapons

Weapons are primitives attached to the camera. Projectiles are primitives.

Player always carries the Axe. Other weapons are pickups. Switching is instant via number keys (1-8) or scroll wheel.

### 2.0 Weapon Table

| # | Weapon             | Ammo    | /Shot | RoF    | Dmg                  | Range/Speed       | Spread     | Knockback | Projectile       |
|---|--------------------|---------|-------|--------|----------------------|-------------------|------------|-----------|------------------|
| 1 | Axe                | —       | —     | 2/s    | 20                   | 64u               | —          | 0         | Melee trace      |
| 2 | Shotgun            | Shells  | 1     | 1.5/s  | 4 × 6 pellets = 24   | Hitscan, 4096     | 4° cone    | 50        | Hitscan          |
| 3 | Super Shotgun      | Shells  | 2     | 1/s    | 4 × 14 pellets = 56  | Hitscan, 4096     | 8° cone    | 100       | Hitscan          |
| 4 | Nailgun            | Nails   | 1     | 8/s    | 9                    | 1500 u/s          | 1° cone    | 10        | Projectile       |
| 5 | Super Nailgun      | Nails   | 2     | 8/s    | 18                   | 1500 u/s          | 1° cone    | 20        | Projectile       |
| 6 | Grenade Launcher   | Rockets | 1     | 1.5/s  | 100 direct + splash  | 800 u/s, arc      | —          | 400       | Arc, bouncy      |
| 7 | Rocket Launcher    | Rockets | 1     | 1.5/s  | 100 direct + splash  | 1000 u/s, straight| —          | 400       | Straight         |
| 8 | Thunderbolt        | Cells   | 1/tick| 10/s   | 30/tick              | Hitscan, 600      | —          | 0         | Continuous beam  |

### 2.1 Ammo

| Type    | Max | Small | Large |
|---------|-----|-------|-------|
| Shells  | 100 | 20    | 40    |
| Nails   | 200 | 25    | 50    |
| Rockets | 100 | 5     | 10    |
| Cells   | 100 | 6     | 12    |

### 2.2 General Weapon Rules

- **Weapon switch.** Instant swap. (Animation pass in v1.5: 0.2 s lower + 0.2 s raise with fire-input queueing during raise.)
- **First pickup** grants weapon + ammo and auto-switches.
- **Duplicate pickup** grants only the ammo (no auto-switch).
- **Empty ammo.** Click stub + auto-switch to next-best owned weapon with ammo. Priority: **RL → SNG → SSG → NG → SG → Axe**. Thunderbolt and GL deliberately excluded ("kept manual to avoid accidental switching").
- **Splash damage** (rockets, grenades). Full damage inside a 60u inner plateau, linear falloff to 0 at 120u outer. `ApplyRadialDamageWithFalloff(InnerRadius=60, OuterRadius=120, Falloff=1.0)`. The plateau is an engine workaround — see CLAUDE.md "Radial damage measures to component center."
- **Self-damage scale** = 0.5 (per `UQuakeDamageType_Explosive`). Rocket-jump survivable.
- **Knockback.** Impulse along hit normal (point) or away from explosion center (radial). Magnitude = `ScaledDamage × 30 × DT->KnockbackScale`. Inherits the splash falloff shape naturally.
- **Grenades.** Bounce (restitution 0.4) until **first of:**
  1. Fuse (2.5s) expires — set in `BeginPlay`, never reset by bouncing.
  2. `OnComponentHit` with a Pawn-channel actor → explode immediately + cancel fuse.
  3. Firer's own capsule (after 0.25s grace) → explode.

  World hits (no actor or non-Pawn `OtherActor`) trigger the bounce delegate for sound — do **not** explode.

- **Nails** are physical projectiles, no gravity.
- **Thunderbolt** ticks at 10 Hz while fire is held. Ignores armor. Range 600.

**Projectile implementation.** All projectiles use `UProjectileMovementComponent`:

| Quake property       | PMC property                           |
|----------------------|----------------------------------------|
| Speed                | `InitialSpeed`, `MaxSpeed`             |
| No gravity (nails)   | `ProjectileGravityScale = 0`           |
| Arc (grenades)       | `ProjectileGravityScale = 1`           |
| Bouncy grenades      | `bShouldBounce = true, Bounciness=0.4` |
| Bounce event         | `OnProjectileBounce` delegate          |

**Damage scaling (Quad baked at launch).** Projectiles carry `DamageScale` (default 1.0). The firing weapon's `Fire` sets it to `AQuakeCharacter::GetOutgoingDamageScale()` at spawn so Quad (4×) is frozen on the shot, not re-sampled at impact. Self-damage scale is a separate multiplier applied inside `TakeDamage` via `UQuakeDamageType::SelfDamageScale`. That's why "Quad does not affect splash self-damage scale" (4.3) comes out of the existing pipeline for free.

### 2.3 Underwater Discharge

Firing Thunderbolt while partially submerged in `AQuakeWaterVolume` consumes all remaining cells in one explosion on the player. Damage = `cells × 10`, radius 256u, applied to everything in the water volume regardless of LoS. Usually fatal.

---

## 3. Enemies

Primitive shapes. "Animation" is transform bobbing/rotation/scaling.

### 3.1 Enemy Types

| Enemy    | HP  | Speed | Sight | Hearing | Attack            | Range       | Dmg         | Cooldown | Proj Speed | Notes                                |
|----------|-----|-------|-------|---------|-------------------|-------------|-------------|----------|------------|--------------------------------------|
| Grunt    | 30  | 300   | 2000  | 1500    | Hitscan rifle     | 1500        | 4           | 1.5 s    | —          | Patrols, takes cover                 |
| Knight   | 75  | 400   | 2000  | 1500    | Melee swing       | 80          | 10          | 1.0 s    | —          | Charges                              |
| Ogre     | 200 | 250   | 2500  | 2000    | Grenade + chainsaw| 96 / 1500   | 20 / 40 spl | 2.0 s    | 600 (arc)  | Grenade at range, melee in close     |
| Fiend    | 300 | 500   | 3000  | 2500    | Leap              | leap 800    | 40          | 2.5 s    | —          | Commits leap outside leap range      |
| Shambler | 600 | 200   | 3000  | 2500    | Lightning + claws | 1200 / 96   | 30 / 40     | 2.0 s    | hitscan    | Takes 0.5× rocket/grenade damage     |
| Zombie   | 60  | 200   | 1500  | 1000    | Flesh chunk lob   | 800         | 10          | 2.0 s    | 600 (arc)  | Revives 5 s after non-gib death      |

### 3.2 Drop Tables

Non-gib deaths only. Backpacks despawn after 60 s.

| Enemy    | Drop                 | Chance |
|----------|----------------------|--------|
| Grunt    | Backpack: 5 shells   | 100%   |
| Knight   | —                    | 0%     |
| Ogre     | Backpack: 2 rockets  | 100%   |
| Fiend    | —                    | 0%     |
| Shambler | —                    | 0%     |
| Zombie   | —                    | 0%     |

**Representation:** `TArray<FQuakeDropEntry> DropTable` UPROPERTY on `AQuakeEnemyBase`, filled per-enemy in C++ constructor. BP subclasses only fill the `TSubclassOf<AQuakePickupBase>` slots.

### 3.3 AI Behavior

**Body/brain split.**

| Piece                        | Owns                                                      |
|------------------------------|-----------------------------------------------------------|
| `AQuakeEnemyBase : ACharacter` | Capsule, mesh, movement, health, `TakeDamage`; action methods (`MoveToTarget`, `FireAtTarget`, pain/death reactions) |
| `AQuakeEnemyAIController : AAIController` | FSM, perception, target tracking                |

**FSM** (tick-driven, no Behavior Trees): `Idle → Alert → Chase → Attack → (Pain) → Dead`.

| State  | Behavior                                                                                       |
|--------|------------------------------------------------------------------------------------------------|
| Idle   | Stationary or patrol path. Perception updates evaluated on arrival.                            |
| Alert  | 0.5 s pulse (scale anim + sound stub), then Chase.                                             |
| Chase  | `MoveToActor` toward target. If in range + LoS → Attack.                                       |
| Attack | Execute attack pattern, cooldown, return to Chase. Pawn held still during windup.              |
| Pain   | Brief FSM suspension. Pain chance = `min(0.8, damage / maxHealth × 2)`. Bosses immune.         |
| Dead   | Controller stops; pawn collapses; drops loot; capsule flips to `Corpse` after 2 s; unpossess. |

**Perception (`UAIPerceptionComponent`):**
- `UAISenseConfig_Sight` — radius/lose-sight per enemy (3.1). 90° peripheral, 5 s stimulus aging. `DetectionByAffiliation.bDetectNeutrals = true` is required or the player's neutral team hides them.
- `UAISenseConfig_Hearing` — walls don't block hearing (matches Quake; UE 5.7 default). Noise events from any weapon fire (Axe included) and from taking damage. Do **not** write `bUseLoSHearing` — deprecated in 5.7 and triggers a warning.

**Damage as perception trigger.** `TakeDamage` notifies controller via `OnDamaged(Instigator, Amount)`. Promotes instigator to current target, skips alert pulse, cuts to Chase/Attack.

**Aggro propagation.** On Alert (via sight or damage), the controller iterates `AQuakeEnemyAIController` instances within 600u and forwards the alert. Direct iteration over GameMode's enemy list — no event bus.

**No leash.** Once alerted, pursue indefinitely.

**Infighting.** Enemy B hit by enemy A's splash/stray → B's `TakeDamage` sees non-player `EventInstigator` → controller seeds `GrudgeTarget` with 10 s expiry. Reverts to player when timer expires, grudge target dies, or LoS lost for 5 s. Hazard damage has no instigator and does not trigger infighting. **Emergent — no infighting-specific damage code.**

**Vertical:**
- NavMesh-walkable surfaces by default.
- `NavLinkProxy` actors mark jump-down/jump-up routes.
- Fiend commits leap 300–800u when in Chase, bypasses NavMesh links.
- Flying enemies — v2+.

### 3.4 Gibs

Gib when `overkillDamage >= currentHealth × 2`. Primitives scatter as physics objects, brief red decal stub, fade after 5 s. **No drops, no death state run.**

**Zombies** must be gibbed to die permanently. Non-gib death → `Down` state for 5 s, then revive at full HP + Chase. `IsDead()` returns false during `Down`, true only on permanent kill.

### 3.5 Hit and Death Sounds

Per-enemy `UPROPERTY(EditDefaultsOnly) TObjectPtr<USoundBase>` slots: `PainSound`, `DeathSound`. `PainSound` plays from `TakeDamage` on every non-fatal hit (independent of pain-chance flinch). `DeathSound` plays once from `Die()` before the death/gib branch. Null slots skipped silently.

Phase 14 migrates these direct `PlaySoundAtLocation` calls to `UQuakeSoundManager` and moves the slots into `DT_SoundEvents` rows.

---

## 4. Items and Pickups

`AQuakePickupBase` holds `USphereComponent` (Pickup channel, 64u default), `UStaticMeshComponent`, `UPointLightComponent`. Overlap binding is in `BeginPlay`. Handler casts to `AQuakeCharacter` → `CanBeConsumedBy` gate → `ApplyPickupEffectTo` (PURE_VIRTUAL) → `Destroy`.

### 4.1 Health

| Item         | HP  | Visual                  |
|--------------|-----|-------------------------|
| Small Health | 15  | Small green box         |
| Health Pack  | 25  | Medium green box        |
| Megahealth   | 100 | Gold box (overcharges)  |

Small/Pack refuse at max HP. Megahealth consumes up to overcharge cap (200). Overcharge decays at 1 HP/sec back to max.

### 4.2 Armor

See 1.2 for the tier table. SPEC 1.2 replacement rule lives in `AQuakePickup_Armor::CanBeConsumedBy`: consume if (new absorption > current) OR (current value < new pickup's value). A Green pickup on top of fresh Yellow is refused (stays in world); a Green pickup after the Yellow drained is accepted.

### 4.3 Powerups

| Powerup       | Duration | Effect                                                  | Visual                |
|---------------|----------|---------------------------------------------------------|-----------------------|
| Quad Damage   | 30 s     | 4× outgoing weapon damage. **Does NOT affect splash self-damage scale.** | Blue rotating cube    |
| Pentagram     | 30 s     | Full invulnerability (incoming damage = 0)              | Red pentagonal shape  |
| Ring of Shadows | 30 s   | Enemies can't acquire player via sight/hearing. Already-aggro enemies attack last-known position with 30° projectile error. Melee unaffected. | Yellow transparent sphere |
| Biosuit       | 30 s     | Immune to slime/lava. **Does NOT prevent drowning.**    | Green transparent sphere |

**Stacking.** Different powerups stack (independent entries + timers). Same powerup **refreshes additively, capped at 60 s**:

```
GivePowerup(Type, Duration):
    for each Entry in ActivePowerups:
        if Entry.Type == Type:
            Entry.RemainingTime = min(Entry.RemainingTime + Duration, 60)
            return
    ActivePowerups.Add({Type, min(Duration, 60)})
    EnablePowerupTick()
```

**Storage: `AQuakePlayerState`.** Tick only enabled while entries exist. Cleared automatically on `OpenLevel`; death-restart must call `ClearPerLifeState`.

### 4.4 Keys

- Silver and Gold unlock matching doors in the current level.
- **Not consumed on use** (matches Quake).
- Reset on level transition; reset on death-restart (via `ClearPerLifeState`).
- Re-pickup of a held key is silent (no message, pickup stays in world).

**Storage: `AQuakePlayerState`.** Door check: `Character->HasKey(RequiredKey)` forwards to PlayerState. Key pickup: `Character->GiveKey(Color)` forwards to PlayerState.

---

## 5. Level Structure

### 5.1 Layout

Levels built in the Editor from BSP brushes and/or static meshes. Every level needs:

- `PlayerStart`
- `NavMeshBoundsVolume` + `RecastNavMesh` (Static)
- Lighting
- `AQuakeEnemySpawnPoint` actors (only authoring path for counted enemies)
- Pickup placements
- Doors, buttons, triggers
- `ExitTrigger` (next map name)
- Hazards, water as needed
- `PostProcessVolume` (Unbound, for damage flash + powerup tint)

**`AQuakeEnemySpawnPoint`** is a passive marker. Directly-placed `BP_Enemy_*` actors are **decoration** — they do not count toward `KillsTotal` and do not gate the level-clear check. If you want an enemy to count, place a spawn point.

| Field                 | Purpose                                                           |
|-----------------------|-------------------------------------------------------------------|
| `EnemyClass`          | `TSubclassOf<AQuakeEnemyBase>` — what to spawn                    |
| `MinDifficulty`       | `EQuakeDifficulty` — `CurrentDifficulty >= MinDifficulty` gates   |
| `bDeferredSpawn`      | True = wait for `Activate`; false = spawn in `BeginPlay`          |
| `bIsMarkedKillTarget` | True = counts toward `KillsTotal` and level-clear (default true)  |

Behavior: `BeginPlay` calls `TrySpawn` when `bDeferredSpawn == false`. `Activate` calls `TrySpawn` regardless. `TrySpawn` bails on ineligibility or already-spawned; otherwise spawns `EnemyClass` at actor transform into `SpawnedEnemy`. One-shot per level attempt.

Implements `IQuakeActivatable` so `AQuakeTrigger_Spawn` and relay chains can fire deferred spawns.

### 5.2 Hazards and Water

**`AQuakeHazardVolume`:**

| Hazard | Dmg | Tick  | Notes                                                 |
|--------|-----|-------|-------------------------------------------------------|
| Slime  | 4   | 1.0 s | Greenish glow                                         |
| Lava   | 30  | 1.0 s | Orange glow, 200u horizontal knockback from center on entry |

Hazards damage players and enemies. Biosuit immunes slime/lava but not drowning. First damage tick fires at `TickRate`, not on entry (entry knockback is the on-contact punishment). For on-entry damage (kill volumes), subclass calls `ApplyTickDamage` from `BeginOverlap` directly — `AQuakeTrigger_Hurt` does this.

### 5.3 Water Volumes

**`AQuakeWaterVolume : APhysicsVolume`** with `bWaterVolume = true` in the constructor. UE's `UCharacterMovementComponent` handles all `MOVE_Walking`/`MOVE_Falling` ↔ `MOVE_Swimming` transitions automatically. **Do not drive swim mode from an overlap volume** — `PhysSwimming` re-checks `bWaterVolume` every frame and would fight any manual mode set.

```cpp
AQuakeWaterVolume::AQuakeWaterVolume(const FObjectInitializer& OI) : Super(OI)
{
    bWaterVolume     = true;
    FluidFriction    = 0.3f;
    TerminalVelocity = 350.f;
    Priority         = 1;   // higher than world default
}
```

**Swim movement** (on `UQuakeCharacterMovementComponent`, not the volume):

| Value            | UE value | CMC property                 |
|------------------|----------|------------------------------|
| Max swim speed   | 350      | `MaxSwimSpeed`               |
| Swim accel       | 2000     | `MaxAcceleration` while swim |
| Buoyancy         | neutral  | `Buoyancy = 1.0`             |

**Submerged vs partially-in-water** distinction lives on `AQuakeCharacter` — tests camera (eye) position vs volume bounds, not capsule.

**Drowning:** 12 s air supply; 4 dmg/sec when empty; air refills instantly on surface. Biosuit does not help.

**Water + weapons:** Thunderbolt discharges (2.3). Projectiles move at half speed underwater.

### 5.4 Doors

`AQuakeDoor` is tick-driven linear interpolation, **not** `UTimelineComponent`. Timelines require `.uasset` curves — violates "C++ first." State machine: `Closed → Opening → Open → Closing → Closed`.

| Property         | Default | Notes                                                  |
|------------------|---------|--------------------------------------------------------|
| OpenDistance     | 128     | How far the door slides                                |
| OpenAxis         | +Z      | Local-space direction                                  |
| OpenSpeed        | 200 u/s |                                                        |
| CloseSpeed       | 200 u/s |                                                        |
| AutoCloseDelay   | 4.0 s   | 0 = stay open                                          |
| CrushDamage      | 10000   | Applied to any `ACharacter` caught closing             |
| RequiredKey      | None    | `EQuakeKeyColor::{None, Silver, Gold}`                 |

**Closing safety (two-layer):**
1. **Pre-close gate.** `TryStartClosing` queries `BlockingZone->GetOverlappingActors(..., ACharacter::StaticClass())`. Any occupant re-arms timer 0.5 s. Prevents starting to close around a walking pawn.
2. **Mid-close crush.** `SetRelativeLocation(..., bSweep=true)` fires `OnComponentHit` → `HandleCrushHit` applies 10000 damage with `_Telefrag`. Move still completes (stopping would trap the corpse).

**Keys.** `CanOpenFor(InInstigator)` casts to `AQuakeCharacter` and calls `HasKey(RequiredKey)`. Non-player instigators on a keyed door are refused. Locked feedback: `AQuakeHUD::ShowMessage("You need the silver/gold key.", 2.f)`.

**Activation** is via `IQuakeActivatable` — buttons and triggers fire doors through the interface chain. The door itself has no touch-to-open overlap; use a `BP_Trigger_Relay` around the door if you want proximity-open.

### 5.5 Buttons

`AQuakeButton` is a one-shot or reusable interactable. **Direct actor references, no string-name targeting.**

| Property        | Default              |
|-----------------|----------------------|
| ActivationMode  | `Touch` / `Shoot`    |
| Cooldown        | 0 (one-shot)         |
| Targets         | `TArray<TObjectPtr<AActor>>` — picked per-instance |

On fire, iterate `Targets`, cast each to `IQuakeActivatable`, call `Activate(Instigator)`. Non-implementing entries log a warning.

The button **does not** implement `IQuakeActivatable` (buttons are input sources, not chainables). For "fire on signal" use `AQuakeTrigger_Relay`.

**Collision** supports both touch and shoot (overlap Pawn, block Weapon + Visibility). `ActivationMode` gates which signal calls `Fire`.

### 5.6 Triggers

**`IQuakeActivatable`** — pure C++ virtual, **not** a `BlueprintNativeEvent`. Do not write `Activate_Implementation` — that suffix is `BlueprintNativeEvent`-only and fails to compile.

```cpp
UINTERFACE(MinimalAPI)
class UQuakeActivatable : public UInterface { GENERATED_BODY() };

class IQuakeActivatable
{
    GENERATED_BODY()
public:
    /** Instigator is the pawn that caused activation (may be null for indirect chains). */
    virtual void Activate(AActor* InInstigator) = 0;   // name: InInstigator avoids AActor::Instigator shadow
};
```

**`AQuakeTrigger : AActor, IQuakeActivatable`** is abstract base. Owns `UBoxComponent TriggerVolume` (query-only, overlap Pawn) and `TArray<TObjectPtr<AActor>> Targets`. `BeginPlay` binds overlap → `Activate`. Default `Activate` fires targets via interface; subclasses override and call `Super::Activate` (or omit for self-contained behavior).

| Subclass    | Type-specific fields                                      | Behavior                                                               |
|-------------|-----------------------------------------------------------|------------------------------------------------------------------------|
| `_Relay`    | —                                                         | No-op override. Fan-out only.                                          |
| `_Spawn`    | `TArray<TObjectPtr<AQuakeEnemySpawnPoint>> SpawnPoints`   | Fires typed list (deferred spawns fire now), then base `Targets`.      |
| `_Message`  | `FText Message`, `float Duration = 3.0`                   | `HUD->ShowMessage(Message, Duration)` then base.                       |
| `_Hurt`     | `float DamagePerTick = 10000`, `float TickRate = 0.5`     | Self-contained — no Super. Per-pawn repeating damage timer with `_Telefrag`. Cleared on `EndOverlap`. |
| `_Teleport` | `TObjectPtr<AActor> Destination`                          | Teleports `ACharacter` instigator to `Destination`'s transform. Velocity magnitude preserved, rotated. |
| `_Secret`   | —                                                         | One-shot via `bCredited`. `PlayerState.Secrets += 1`, message "A secret area!"                       |
| `_Exit`     | `FName NextMapName`, `bool bGatedByClearCondition = true`, `float StatsDisplaySeconds = 5` | See 5.9 + 6.3.                                |

All use the player-only overlap filter (cast to `AQuakeCharacter` in the C++ handler).

### 5.7 Progression

- Levels organized in episodes; each episode is a linear sequence.
- Hub level lets player pick episode by walking into a portal.
- Episode end returns to hub.
- Final episode completion → win screen (6.3).

### 5.8 Secrets

Hidden areas accessed by shooting walls, walking through fake geometry, or finding hidden buttons. Contain extra ammo, health, powerups. Each is an `AQuakeTrigger_Secret` volume placed at the hidden-area entrance.

### 5.9 Stat Counting

**Numerator/denominator pair, no duplicate counters.**

| Counter                                 | Home                  | Update                                                |
|-----------------------------------------|-----------------------|-------------------------------------------------------|
| `KillsTotal`                            | `AQuakeGameMode`      | Computed once at `BeginPlay` (count eligible spawn points)    |
| `SecretsTotal`                          | `AQuakeGameMode`      | Computed once at `BeginPlay` (count `_Secret` volumes) |
| `Kills` / `Secrets` / `Deaths`          | `AQuakePlayerState`   | Mutators: `AddKillCredit` / `AddSecretCredit` / `AddDeath` |
| `TimeElapsed`                           | `AQuakePlayerState`   | On-demand from `World->GetTimeSeconds() - LevelStartTime` |

**Level-clear check reads spawn points, not enemies:**

```cpp
bool AQuakeGameMode::IsLevelCleared() const
{
    for (TActorIterator<AQuakeEnemySpawnPoint> It(GetWorld()); It; ++It)
        if (It->IsEligible() && !It->IsSatisfied()) return false;
    return true;
}

bool AQuakeEnemySpawnPoint::IsSatisfied() const
{
    return SpawnedEnemy != nullptr && SpawnedEnemy->IsDead();
}
```

Scanning spawn points handles three edge cases without branching: (a) unfired deferred spawns are unsatisfied; (b) infighting/hazard kills satisfy regardless of player credit; (c) Down-state Zombies report `IsDead() == false`. Pure helper `IsLevelClearedForSet(...)` is the world-free version for tests.

**Kill credit rules** (in `AQuakeEnemyBase::Die` when `bIsMarkedKillTarget == true`):

| Case                        | Credit to Kills? |
|-----------------------------|------------------|
| Player direct kill          | Yes              |
| Player gib                  | Yes (no double)  |
| Infighting                  | No (v1) — level-clear still satisfies |
| Hazard kill, player softened| Yes (`bPlayerHasDamagedMe` flag set in `TakeDamage`) |
| Zombie Down → revive        | No — only on permanent kill |

Secrets: `+1` first entry to each `_Secret` volume (one-shot via `bCredited`). Deaths: `+1` per player death (inside `AQuakeCharacter::TakeDamage` when Health hits 0).

---

## 6. Game Rules

### 6.1 Difficulty

Selected at new-game time, persists for the playthrough, cannot change mid-playthrough.

| Difficulty | Enemy Dmg | Enemy HP | Notes                                                                   |
|------------|-----------|----------|-------------------------------------------------------------------------|
| Easy       | ×0.75     | ×1.0     |                                                                         |
| Normal     | ×1.0      | ×1.0     | Default                                                                 |
| Hard       | ×1.5      | ×1.25    | Extra enemies via spawn-point filtering                                 |
| Nightmare  | ×2.0      | ×1.5     | Even more enemies; Zombies revive 2× faster; all enemies pain-immune    |

**Enemy count scaling is NOT a multiplier.** It's `AQuakeEnemySpawnPoint.MinDifficulty` filtering, authored per-placement.

**Storage.** `EQuakeDifficulty` on `UQuakeGameInstance` (survives `OpenLevel`). `AQuakeGameMode::BeginPlay` reads it and exposes `GetDifficulty()` + `GetDifficultyMultipliers()`.

**Application.** Multipliers are a `UPROPERTY(EditDefaultsOnly) TMap<EQuakeDifficulty, FQuakeDifficultyMultipliers>` on GameMode (tunable via `BP_QuakeGameMode` without C++ recompile). `AQuakeEnemyBase::ApplyDifficultyScaling()` (virtual) baked in `BeginPlay`: `MaxHealth = BaseMaxHealth × M.EnemyHP`, `AttackDamageMultiplier = M.EnemyDamage`. Applied on outgoing attack damage at fire time.

**Per-difficulty quirks:**
- `AQuakeAIController_Zombie::ApplyDifficultyScaling` halves `ReviveTimer` on Nightmare.
- Pain immunity on Nightmare — checked at `OnDamaged` in the base AI controller, not baked.

### 6.2 Saves

`UQuakeSaveGame : USaveGame` via `UGameplayStatics::SaveGameToSlot` / `LoadGameFromSlot`.

**Fields:**
- **Profile:** difficulty, total stats across all levels.
- **Inventory snapshot** (from GameInstance): weapons owned, ammo, armor.
- **Live HP** (from Character): serialized separately — not inventory.
- **Level state:** current level name, player transform.
- **PlayerState snapshot:** Kills, Secrets, Time, Deaths, `ActivePowerups`, Keys.
- **Per-actor state:** array of `FActorSaveRecord` keyed by `AActor::GetFName()`.

```cpp
USTRUCT() struct FActorSaveRecord
{
    GENERATED_BODY()
    UPROPERTY() FName ActorName;      // == owner's GetFName()
    UPROPERTY() TArray<uint8> Payload; // FMemoryWriter blob from the actor's SaveState
};
```

**Participation:** actors with persistent state implement `IQuakeSaveable` with `SaveState(FActorSaveRecord&)` and `LoadState(const FActorSaveRecord&)`.

**Stable identity = `AActor::GetFName()`** for level-placed actors. UE5 assigns unique FNames at placement time, serialized with the `.umap`. Survives save/load, package re-save, editor restart. Not `AActor::Tags` — tags are user-typed strings, the same stringly-typed pattern removed from buttons/triggers.

**Runtime-spawned actors do not persist in v1.** Spawn points re-fire on level reload and reproduce their enemies. Projectiles are too transient to save.

**Load flow:**
1. `LoadGameFromSlot` returns the `UQuakeSaveGame`.
2. GameInstance restores its own fields immediately (before `OpenLevel`).
3. `OpenLevel` with the saved level name.
4. After load, GameMode restores PlayerState fields.
5. GameMode builds `TMap<FName, const FActorSaveRecord*>` and iterates `IQuakeSaveable` actors, calling `LoadState(record)` when FName matches. Unmatched actors fall back to level defaults.
6. Character `BeginPlay` reads inventory from GameInstance (step 2) and HP from its record (step 5).

**Slots:** `auto_<profile>`, `quick_<profile>`. One of each per profile.

**F5 / F9 inputs.** "No mid-air saves" — F5 rejected if `MovementMode != MOVE_Walking` or pain-reacting.

### 6.3 Win Condition

Completing the final level of the final episode:
1. Fade to black.
2. Win screen with total stats (kills, secrets, time, deaths, difficulty).
3. Return to main menu.

Within an episode, last level returns to hub. No score system — stats are informational.

### 6.4 Failure Loop

- Death: 1.5 s camera-tilt anim, then death screen "Press Fire to Restart."
- Fire restores the level-entry snapshot and respawns at `PlayerStart`.
- No game-over. Retry indefinitely.

**Restart sequence (in order):**

1. `PlayerState->AddDeath()`.
2. `PlayerState->ClearPerLifeState()` (empties powerups + keys, leaves Kills/Secrets/Time/Deaths).
3. Restore GameInstance inventory from the level-entry snapshot.
4. Destroy dead pawn, spawn new `AQuakeCharacter` at `PlayerStart`.
5. New pawn `BeginPlay` reads inventory from GameInstance, sets HP to 100 (or snapshot).
6. Time counter keeps running.

---

## 7. HUD

See [HUD.md](HUD.md) for layout, wireframe, and data sources.

---

## 8. Audio System

No audio assets in v1. System provides a clean interface for future integration.

### 8.1 Architecture

- **`UQuakeSoundManager : UGameInstanceSubsystem`** — central audio manager. Auto-instantiated. **Subsystems cannot have Blueprint subclasses.**
- Methods: `PlaySound(ESoundEvent, Location)`, `PlayMusic(EMusicTrack)`, `StopMusic()`.
- `ESoundEvent` enum catalogs every game sound.

**Sound table.** Because subsystems can't be BP-subclassed, the data-table reference lives on `UQuakeGameInstance` (which does have `BP_QuakeGameInstance`) as `UPROPERTY(EditDefaultsOnly) TObjectPtr<UDataTable> SoundEventTable`. `UQuakeSoundManager` pulls it via `GetGameInstance<UQuakeGameInstance>()` on first use and caches.

`DT_SoundEvents.uasset` rows: `FQuakeSoundEvent { ESoundEvent Key; TObjectPtr<USoundBase> Sound; }`. Unmapped or missing rows = `PlaySound` is a no-op (gameplay ships before assets exist).

### 8.2 Sound Events (partial)

- Player: footstep, jump, land, pain, death, pickup_item, pickup_weapon, pickup_powerup
- Weapons: axe_swing, shotgun_fire, nailgun_fire, rocket_fire, grenade_bounce, rocket_explode, thunderbolt_hum
- Enemies: alert, pain, death, attack, idle
- World: door_open, door_close, button_press, teleport, secret_found
- Music: per-level ambient

---

## 9. Technical Architecture

### 9.1 Class Hierarchy

```
AQuakeGameMode                        — level loading, rules, player spawning, denominators
AQuakePlayerController                — input setup, HUD ownership
AQuakePlayerState : APlayerState      — Kills/Secrets/Time/Deaths, ActivePowerups, Keys
AQuakeCharacter                       — player body: movement, HP, weapon mgmt
UQuakeCharacterMovementComponent      — custom CMC, Quake-style air physics
AQuakeHUD : AHUD                      — owns SQuakeHUDOverlay Slate widget
  SQuakeHUDOverlay                    — HP / armor / ammo / weapon bar / keys / powerups / crosshair

AQuakeWeaponBase
  AQuakeWeapon_{Axe, Shotgun, SuperShotgun, Nailgun, SuperNailgun,
                GrenadeLauncher, RocketLauncher, Thunderbolt}

AQuakeProjectile                      — USphereComponent + UProjectileMovementComponent; DamageScale
  AQuakeProjectile_{Nail, Rocket, Grenade}

AQuakeEnemyBase : ACharacter
  AQuakeEnemy_{Grunt, Knight, Ogre, Fiend, Shambler, Zombie}

AQuakeEnemyAIController : AAIController
  AQuakeAIController_{Grunt, Knight, Ogre, Fiend, Shambler, Zombie}

AQuakePickupBase
  AQuakePickup_{Health, Armor, Ammo, Weapon, Key, Powerup}

AQuakeEnemySpawnPoint                 — IQuakeActivatable; canonical authoring path
AQuakeDoor                            — IQuakeActivatable
AQuakeButton                          — fires IQuakeActivatable targets
AQuakeTrigger : IQuakeActivatable
  AQuakeTrigger_{Relay, Spawn, Message, Hurt, Teleport, Secret, Exit}
AQuakeWaterVolume : APhysicsVolume
AQuakeHazardVolume

UQuakeDamageType (abstract)
  UQuakeDamageType_{Melee, Bullet, Nail, Explosive, Lightning, Lava, Slime, Drown, Telefrag}

IQuakeActivatable                     — pure C++ virtual
IQuakeSaveable                        — SaveState/LoadState
UQuakeSaveGame : USaveGame
UQuakeProjectSettings : UDeveloperSettings  — balance + audio DataTable refs
UQuakeSoundManager : UGameInstanceSubsystem
UQuakeGameInstance                    — inventory, snapshot, save/load, difficulty
```

### 9.2 Blueprint Layer

For every C++ class that needs assets or per-variant tuning, create a BP subclass under `Content/Blueprints/`. **Zero nodes in the Event Graph** — only property defaults and asset references.

Categories:
- **Framework:** `BP_QuakeGameMode`, `BP_QuakeGameInstance`, `BP_QuakePlayerController`, `BP_QuakeCharacter`.
- **Weapons / Enemies / Pickups / Projectiles:** per-type BPs filling mesh/material/sound/projectile-class slots.
- **World:** `BP_Door`, `BP_Button`, `BP_HazardVolume_Lava`, `BP_Trigger_*`.

`BP_QuakeGameMode` references BP classes (not C++) via `TSubclassOf<>` properties.

### 9.3 Build Configuration

- Module: `Quake`.
- Public dependencies: `Core`, `CoreUObject`, `Engine`, `InputCore`, `EnhancedInput`, `NavigationSystem`, `AIModule`, `Slate`, `SlateCore`.
- UMG is **not** a dependency — pure Slate HUD.
- `GameplayTags` is **not** a dependency — damage discrimination uses `UDamageType` subclasses.

### 9.4 Ownership Summary

- **Inventory** (weapons, ammo, armor) → `UQuakeGameInstance`. Survives `OpenLevel` and respawn.
- **Live HP** → `AQuakeCharacter`. Body-bound.
- **Per-life** (powerups, keys) + **per-attempt score** (kills/secrets/time/deaths) → `AQuakePlayerState`. Note: powerups/keys cleared by `ClearPerLifeState` on death; score persists across deaths within the attempt.
- **Level totals** (KillsTotal, SecretsTotal) → `AQuakeGameMode`. Computed once.
- **AI** → `AQuakeEnemyAIController`. Pawn exposes action methods only.
- **HUD** → `AQuakeHUD` (polls all of the above via weak pointers in the Slate widget).
- **Damage** → `AActor::TakeDamage` with `UDamageType` subclasses.
- **Saves** → `UQuakeSaveGame` via `SaveGameToSlot`.

---

## 10. Project Layout & Editor Configuration

### 10.1 Content Folder Structure

```
Content/
  Maps/
    MainMenu.umap
    Hub.umap
    E1/E1M1.umap, E1M2.umap, ...
    Tests/PhysSandbox.umap, LevelStructureSandbox.umap, StatsSandbox.umap, ...
  Blueprints/
    Framework/  BP_QuakeGameMode, BP_QuakeGameInstance, BP_QuakePlayerController, BP_QuakeCharacter
    Weapons/    BP_Weapon_Axe, ..._Thunderbolt
    Enemies/    BP_Enemy_Grunt, ..._Zombie
    Pickups/    BP_Pickup_Health*, _Armor_{G,Y,R}, _Ammo_*, _Quad, _KeySilver/Gold, _Weapon_*
    Projectiles/ BP_Projectile_{Nail, Rocket, Grenade}
    World/      BP_Door, BP_Button, BP_HazardVolume_Lava, BP_Trigger_*
  Materials/
    M_QuakeBase.uasset
    Instances/  MI_* per use case
  Data/
    DT_EnemyStats.uasset, DT_WeaponStats.uasset, DT_SoundEvents.uasset
```

### 10.2 Master Material

`M_QuakeBase` drives all visuals:

| Parameter  | Type    | Purpose                                   |
|------------|---------|-------------------------------------------|
| BaseColor  | Vector3 | Flat color                                |
| Emissive   | Scalar  | Glow (0 matte, >1 glow)                   |
| Metallic   | Scalar  | 0 default, 1 for metal pickups            |
| Roughness  | Scalar  | 0.5 default                               |

`MI_*` instances override per use case. Runtime tints (damage flash, powerup overlay) create `UMaterialInstanceDynamic` from the relevant `MI_*`.

### 10.3 NavMesh

**Per level:** `NavMeshBoundsVolume` + auto-spawned `RecastNavMesh` (Runtime Generation = Static).

**Project Settings → Navigation System → Supported Agents:**
- Agent Radius: 35 (matches player)
- Agent Height: 180
- Agent Max Slope: 44
- **Agent Max Step Height: 45 — must equal player's or AI can't path where the player can walk.**

Press **P** in the viewport to visualize (green = walkable). First thing to check if enemies refuse to chase.

### 10.4 Project Settings

Version-controlled in `Config/Default*.ini`.

- **Maps & Modes:** Default GameMode = `BP_QuakeGameMode`; Game Instance Class = `BP_QuakeGameInstance`; Editor Startup Map + Game Default Map = `MainMenu`; Transition Map empty.
- **Input:** `EnhancedPlayerInput` + `EnhancedInputComponent`.
- **Engine:** Smooth Frame Rate true (30–120), Fixed Frame Rate false.
- **Collision:** four custom channels (`Pickup`, `Projectile`, `Corpse`, `Weapon` trace) under `[/Script/Engine.CollisionProfile]`.
- **Packaging:** Use Pak File = true. Explicit list of maps to ship (excludes `Content/Maps/Tests/`).

### 10.5 Per-Level Checklist

Every playable map must include:

- [ ] `PlayerStart`
- [ ] `NavMeshBoundsVolume` covering walkable area
- [ ] `PostProcessVolume` (Unbound) for damage flash + powerup tint
- [ ] At least one light source
- [ ] Floor/wall geometry with `MI_Wall_*` / `MI_Floor`
- [ ] Enemy placements as `AQuakeEnemySpawnPoint` actors (not direct `BP_Enemy_*` drops)
- [ ] Pickup placements
- [ ] Doors, keyed appropriately
- [ ] `ExitTrigger` with `NextMapName` set
- [ ] Hazards, triggers as needed
- [ ] World Partition disabled
- [ ] Level Blueprint event graph: empty
