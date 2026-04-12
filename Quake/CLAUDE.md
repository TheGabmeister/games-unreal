# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Unreal Engine 5.7 single-player FPS recreating original Quake gameplay with primitive shapes. The full design is in [SPEC.md](SPEC.md) — read it before making non-trivial gameplay changes. Module name: `Quake`. The current scope is the v1 milestone defined in `SPEC.md` section 11.

**SPEC.md is the design source of truth.** When the SPEC and this file disagree about gameplay or data ownership, the SPEC wins — update CLAUDE.md to match. CLAUDE.md is a working-notes file for build/tooling/conventions, not a design doc.

**Read SPEC.md surgically.** It's ~1900 lines and reading it top-to-bottom for a scoped task burns tens of thousands of tokens. Start by reading the `## Quick Map — Section Index` block in the first ~90 lines of SPEC.md to decide which sections your task actually needs, then use `Grep "^### <N>\.<M>"` or `Grep "^#### Phase <N>:"` to find exact line offsets, and `Read` with tight `limit=30-50` windows centered on what you need. Cross-references throughout the code and this file use SPEC section numbers (e.g., "SPEC 3.3") — those are stable handles; line numbers drift and should not be hardcoded.

**Implementation phases.** v1 is built in 16 phases (Phase 0–15) defined in [SPEC.md section 11.5](SPEC.md). Each phase has explicit exit criteria — automated tests + manual verification — that must pass before the next phase begins. Before adding gameplay code, find the current phase and stay within its scope. **Phase 1 (the strafe-jumping CMC) is the highest-risk item**; do not build combat code on top of an unverified CMC.

## Build Commands

Build the editor target (the most common command during development):

```bash
"/c/Program Files/Epic Games/UE_5.7/Engine/Build/BatchFiles/Build.bat" QuakeEditor Win64 Development -Project="c:/dev/games-unreal/Quake/Quake.uproject" -WaitMutex -FromMsBuild
```

If a build fails with **"Unable to build while Live Coding is active"**, the user must close the Unreal Editor or press Ctrl+Alt+F11 to apply Live Coding. Do not work around this — ask the user to handle it.

Regenerate VS Code project files (after adding new `.h` / `.cpp` files, so IntelliSense and `compileCommands_Quake.json` pick them up):

```bash
export UE_DOTNET_DIR='/c/Program Files/Epic Games/UE_5.7/Engine/Binaries/ThirdParty/DotNet/8.0.412/win-x64'
export DOTNET_ROOT="$UE_DOTNET_DIR" PATH="$UE_DOTNET_DIR:$PATH" DOTNET_MULTILEVEL_LOOKUP=0 DOTNET_ROLL_FORWARD=LatestMajor
dotnet '/c/Program Files/Epic Games/UE_5.7/Engine/Binaries/DotNET/UnrealBuildTool/UnrealBuildTool.dll' -projectfiles -project='C:/dev/games-unreal/Quake/Quake.uproject' -game -engine -VSCode
```

Run this manually after adding a new `.h` / `.cpp` file. (An earlier `PostToolUse` hook tried to automate it, but it fired once per Write so a phase that created ~11 source files regenerated the project ~11 times back-to-back; removed as of 2026-04-11.)

The system installs only .NET 10; UnrealBuildTool requires .NET 8. The bundled SDK at the path above is the working version — never invoke a system `dotnet` for UBT.

## IDE Integration Notes

IntelliSense errors like `cannot open source file "InputModifiers.h"` are usually false positives from VS Code's C++ extension when `compileCommands_Quake.json` is stale. The compile-test of truth is a real `Build.bat` run, not the IDE squiggles. Regenerate project files (above) before assuming an include is wrong.

## Running Tests

Automation tests live under `Source/Quake/Tests/` and use `IMPLEMENT_SIMPLE_AUTOMATION_TEST` with `EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter`, guarded by `#if WITH_DEV_AUTOMATION_TESTS`. Run them from the editor via **Session Frontend → Automation tab → filter `Quake.*`**. Tests are grouped by subsystem:

- `Quake.Foundation.*` — smoke test proving the runner is wired up.
- `Quake.Movement.AirAccel.*` — dot-product clamp regression suite for the CMC.
- `Quake.Damage.ArmorAbsorption.*` — armor formula regression suite (Green / Yellow / Underflow / NoArmor).
- `Quake.Damage.DamageType.*` — shared-base CDO cast pattern from SPEC section 1.5.
- `Quake.Enemy.PainChance.*` — SPEC 3.3 pain-chance formula (low damage / at-cap / over-cap / zero / zero-maxhp / Grunt-vs-shotgun).
- `Quake.Ammo.*` — Phase 4 ammo inventory: cap table, GiveAmmo clamp, ConsumeAmmo atomic-fail, None short-circuit, Init seeding.
- `Quake.Weapon.RocketSplash.*` — Phase 5 linear splash falloff formula (center / quarter / half / edge / beyond / zero-radius guard).
- `Quake.Weapon.AutoSwitch.*` — Phase 6 SPEC 2.2 empty-ammo auto-switch priority picker (Axe fallback / RL top-priority / NG-beats-Axe / excludes-current / GL+Thunderbolt-skipped / empty-owned-skipped).

A test `.cpp` in `Source/Quake/Tests/` cannot `#include` headers from the module root unless the module adds its source directory to `PrivateIncludePaths`. [Source/Quake/Quake.Build.cs](Source/Quake/Quake.Build.cs) does this via `PrivateIncludePaths.Add(ModuleDirectory)` — leave it; removing it breaks every test file in the subdirectory.

**LWC gotcha in numeric asserts.** UE 5.5+ makes `FVector` components `double` (Large World Coordinates). `TestEqual(TEXT("..."), Vec.X, 30.f, KINDA_SMALL_NUMBER)` is an ambiguous-overload compile error because `Vec.X` is `double` and the literals are `float`. Use double literals (`30.0`) and `UE_KINDA_SMALL_NUMBER` in any test assertion that touches vector components.

Prefer unit tests over map-based functional tests when the logic can be extracted into a pure static function. `UQuakeCharacterMovementComponent::ApplyQuakeAirAccel` is the template: a static function that the CMC override calls, so unit tests can exercise the air-accel math directly without spinning up a world, a pawn, or a physics tick.

## Architecture: C++ First With Thin Blueprint Layer

Per [SPEC.md](SPEC.md) section "Constraints", **all gameplay logic lives in C++**. Blueprints are used **only** as thin subclasses of C++ base classes for two purposes:

1. **Asset references** — assigning meshes, materials, and (future) sound assets to `UPROPERTY` slots.
2. **Per-instance parameter tuning** — tweaking values like enemy health or weapon damage without recompiling.

Blueprints contain **no nodes in event graphs** — only property defaults. Do not propose adding gameplay logic to Blueprints. If a feature requires runtime behavior, add it to the C++ base class.

## Architecture: State Ownership

Data lives in a specific place depending on its lifecycle. Putting it in the wrong place breaks respawn, level transitions, or the HUD. From SPEC.md's ownership summary:

- **`UQuakeGameInstance`** — inventory (weapons owned, ammo, armor), level-entry snapshot, save-game reference, player profile, difficulty. Survives `OpenLevel` and Character respawn. **Note:** keys are *not* here despite being colloquially "inventory" — see PlayerState below. Ammo is a private `TMap<EQuakeAmmoType, int32>` accessed through `GetAmmo` / `GiveAmmo` / `ConsumeAmmo` — caps from SPEC 2.1 are baked into the pure static `GetAmmoCap`. Owned-weapon classes live in an 8-slot `OwnedWeaponClasses` array (index i = SPEC 2.0 weapon number i+1) populated in BP_QuakeGameInstance; the Character reads this at `BeginPlay` and spawns one actor per non-null slot.
- **`AQuakePlayerState`** — current-level stats (kills, secrets, deaths, time) and active powerups (`TArray<FQuakeActivePowerup>`). Auto-cleared on `OpenLevel` only — UE preserves PlayerState across pawn death/respawn, so cumulative stats persist across the level attempt and must NOT be reset by the death-restart path. Keys are planned to live here too (their lifecycle matches powerups, not weapons/ammo/armor) and will land with the key-pickup work in Phase 8; the `ClearPerLifeState()` method that empties powerups + keys on death-restart will land alongside the Phase 6.4 failure-loop implementation. Neither exists yet — adding them preemptively is out of scope, but SPEC section 1.4 is the authoritative reference for where they'll live. Don't assume "PlayerState resets on death" without that call; use `OpenLevel` for level transitions.
- **`AQuakeCharacter`** — live health, `WeaponInstances[8]` (actor pointers for each owned slot), and `CurrentWeapon` / `CurrentWeaponSlot`. Tied to the body; destroyed on death and respawn. Exposes facade methods `GiveAmmo` / `ConsumeAmmo` / `GetAmmo` (forward to the GameInstance) and `GiveHealth(Amount, bOvercharge)` (self-owned state, caps at `MaxHealth` or `GetOverchargeCap()` = 200).
- **`AQuakeGameMode`** — level totals (`KillsTotal`, `SecretsTotal`), spawn rules, win conditions. Server-authoritative (prepared for future multiplayer).
- **`AQuakeHUD`** — reads from all of the above; caches weak pointers in the Slate widget and reads them on paint.

Do not put inventory on the Character. Do not put per-level stats on the Character or the GameMode. Do not put settings (sensitivity, volume) in the PlayerState.

## Architecture: Damage Pipeline

All damage flows through UE's built-in pipeline:

1. Attackers call `UGameplayStatics::ApplyPointDamage` / `ApplyRadialDamage` / `ApplyDamage`.
2. Targets override `AActor::TakeDamage` to actually decrement health, apply armor, knockback, etc.
3. **No code outside `TakeDamage` mutates health directly.** If you see `Target->Health -= X` anywhere else, it's a bug.

Damage metadata (self-damage scale, armor bypass, pain suppression, knockback strength) lives on `UDamageType` subclasses defined in SPEC section 1.5 — not on the attacker, not on the target. All Quake damage types inherit from a single abstract base `UQuakeDamageType` that owns every Quake-specific `UPROPERTY`; leaf subclasses add no new properties and only override defaults in their constructor. `TakeDamage` reads these via the CDO of the shared base — never branch on leaf class identity. Adding a new damage source means adding a ~10-line subclass that overrides defaults in its constructor and calling the appropriate `ApplyXDamage` helper with its `StaticClass()`. Splash radius lives on the weapon, not the damage type.

Attribution uses UE's built-in `EventInstigator` (the controller) and `DamageCauser` (the projectile/weapon). Self-damage detection, infighting, and kill credit all derive from `EventInstigator` — no custom event bus needed.

**Armor absorption is a pure static helper** `AQuakeCharacter::ApplyArmorAbsorption` — extracted from `TakeDamage` so the Quake formula (`save = ceil(absorption * damage)`) can be unit-tested without a world. **Float-precision gotcha:** because `0.3f` is not exactly representable in IEEE 754, `0.3f * 50.0f` evaluates to `15.000000596...`, which `FMath::CeilToFloat` rounds up to 16 instead of 15. The helper subtracts `UE_KINDA_SMALL_NUMBER` before the ceil to snap "essentially integer" results back — any rewrite must preserve this epsilon or the Green/Yellow armor tests go off by one. SPEC section 1.2 prose + the Quake-canonical formula give HP=65, armor=85 for 100 HP + 100 green armor + 50 damage; an earlier draft of the SPEC had these swapped and was corrected during Phase 2.

**Knockback uses Quake's damage-scaled formula** (`ScaledDamage * 30 * DT->KnockbackScale`), NOT UE's stock `ACharacter::ApplyDamageMomentum` + `DamageImpulse` (which is a fixed magnitude per damage type and would lose the damage scaling — rocket-jump height depends on the damage the rocket dealt, not a constant). The override reuses the engine's `FDamageEvent::GetBestHitInfo` helper to unify point and radial damage directions in one call, so there is no manual `FPointDamageEvent` / `FRadialDamageEvent` branching — copy that pattern for any future `TakeDamage` override.

**`AQuakeCharacter::Health` is `protected`**; external code reads it via `GetHealth()`. This is compile-time enforcement of the "no code outside `TakeDamage` mutates health directly" rule.

## Architecture: Weapons

`AQuakeWeaponBase` is the `UCLASS(Abstract)` base for every weapon. The public entry point is `TryFire(InInstigator)`, which enforces `RateOfFire` cooldown via a `LastFireWorldTime` timestamp, runs the SPEC 2.1 ammo gate (`ConsumeAmmo(AmmoType, AmmoPerShot)` on the Instigator's `UQuakeGameInstance`), and then calls the subclass-implemented `Fire`. Weapons with `AmmoType == None` (the Axe) bypass the ammo gate — `ConsumeAmmo(None, ...)` always returns true. On empty ammo the base plays `PlayEmptyClick`, arms the cooldown, and attempts an auto-switch — see the dedicated paragraph below. Subclasses only override `Fire`; cooldown + ammo gating live on the base. The four v1 weapons ([AQuakeWeapon_Axe](Source/Quake/QuakeWeapon_Axe.h), [AQuakeWeapon_Shotgun](Source/Quake/QuakeWeapon_Shotgun.h), [AQuakeWeapon_Nailgun](Source/Quake/QuakeWeapon_Nailgun.h), [AQuakeWeapon_RocketLauncher](Source/Quake/QuakeWeapon_RocketLauncher.h)) set all stats (`RateOfFire`, `AmmoType`, `AmmoPerShot`, `DisplayName`, plus any subclass-specific `Damage` / `Range` / `ProjectileClass` / `SpreadHalfAngleDegrees`) as `UPROPERTY` defaults in their C++ constructor per the "C++ first" rule — the thin `BP_Weapon_*` subclass only fills in asset slots and (for projectile weapons) the `TSubclassOf<AQuakeProjectile> ProjectileClass`.

**`Fire` is declared with `PURE_VIRTUAL`, not C++ `= 0`.** UE's reflection system constructs a Class Default Object for every `UCLASS`, including `UCLASS(Abstract)` ones, and a C++-pure-virtual method (`virtual void Foo() = 0;`) makes the CDO non-instantiable with `error C2259: cannot instantiate abstract class`. The fix is the engine's `PURE_VIRTUAL(AQuakeWeaponBase::Fire, )` macro, which emits a crashing stub body that satisfies the CDO constructor while still firing at runtime if anything calls the base directly. **Any future abstract `UCLASS` virtual needs the same treatment** — this bit us once during Phase 2 and will bite again. `AQuakeEnemyBase::FireAtTarget` and `AQuakePickupBase::ApplyPickupEffectTo` use the same pattern.

`SpawnActor` for a weapon uses the `(UClass*, FActorSpawnParameters)` overload followed by `AttachToComponent`, not the `(UClass*, FTransform, FActorSpawnParameters)` overload. On UE 5.7 the latter fails template argument deduction when the first argument is a `TSubclassOf<T>` (the compiler can't decide between the `FTransform` and `(FVector, FRotator)` overloads), so spawn at origin and let the attach set the transform. See [QuakeCharacter.cpp SpawnOwnedWeapons](Source/Quake/QuakeCharacter.cpp) for the canonical pattern — Phase 4 extended it to spawn every owned weapon slot at BeginPlay (hiding all but the active one via `SetActorHiddenInGame`).

**`TSubclassOf<T>` in .cpp files requires the full `T` header, not a forward decl.** `TSubclassOf<AQuakeWeaponBase>` instantiation (from `operator*` or `IsChildOf`) needs `AQuakeWeaponBase::StaticClass()` visible, so any .cpp that touches a `TSubclassOf` member must `#include "QuakeWeaponBase.h"` even when the header only forward-declares it. Hit us once in [QuakeGameInstance.cpp](Source/Quake/QuakeGameInstance.cpp) during Phase 4.

**Empty-ammo auto-switch** is wired in [AQuakeWeaponBase::TryFire](Source/Quake/QuakeWeaponBase.cpp). When the ammo gate fails, the base calls `PlayEmptyClick` (so the player hears the dry-fire), arms the cooldown, and then asks the firing character `AutoSwitchFromEmptyWeapon()` to swap to the next best owned weapon. The priority order is implemented in the pure static helper [AQuakeCharacter::PickAutoSwitchWeaponSlot](Source/Quake/QuakeCharacter.cpp) which walks SPEC 2.2's `RL → SNG → SSG → NG → SG → Axe` sequence. **Thunderbolt (slot index 7) and GL (slot index 5) are intentionally NOT in the priority list** — SPEC 2.2 keeps them manual-only so the player can't accidentally burn through their last Cells / Rockets on a reflex fire. The runtime wrapper builds parallel length-8 ownership + ammo bool masks from `WeaponInstances` + `GameInstance->GetAmmo`; the picker takes only those masks + an exclude slot, so the `Quake.Weapon.AutoSwitch.*` test suite can regress the priority table without a world. The Axe (`AmmoType::None`) is the terminal fallback — it is always "has ammo" in the mask, so auto-switch only returns -1 when the player genuinely owns nothing.

## Architecture: Projectiles

[AQuakeProjectile](Source/Quake/QuakeProjectile.h) is the `UCLASS(Abstract)` base for every projectile (Rocket — Phase 5; Nail — Phase 6; Grenade — Phase 7). Owns a `USphereComponent` root (collision on the `Projectile` object channel, block `WorldStatic`/`WorldDynamic`/`Pawn`/`Corpse`, ignore everything else), a non-colliding `UStaticMeshComponent` (visual), and a `UProjectileMovementComponent` (flight). The sphere's `OnComponentHit` is bound **in the constructor** (not BeginPlay) so BP subclasses inherit the wiring, and forwards to a virtual `HandleImpact(Hit, OtherActor)` that subclasses override. The base implementation of `HandleImpact` is a no-op — grenades want to bounce on world hits instead of exploding, so the base deliberately does not call `Destroy()`. Rockets/Nails override and destroy themselves at the end of their handler.

**Muzzle spawn-out is a two-layer guard** per SPEC 1.6 rule 1. The weapon's `Fire` projects the spawn location `60 u` in front of the firer's eye viewpoint (configurable via `MuzzleSpawnForwardOffset` on the weapon), so the rocket never spawns inside the capsule. [AQuakeProjectile::BeginPlay](Source/Quake/QuakeProjectile.cpp) then calls `CollisionSphere->IgnoreActorWhenMoving(GetInstigator(), true)` as a second guard — the ignore stays in place for the projectile's lifetime since rockets shouldn't bounce off the firer mid-flight either. `OnSphereHit` additionally bails if `OtherActor == GetInstigator()` as a third belt to catch pathological corner cases. All three layers are redundant by design — lose any one and a tight wall corner can still cause self-detonation.

**Splash damage lives on the projectile subclass, not on the damage type.** SPEC section 1.5 explicitly calls this out: `UQuakeDamageType_Explosive` carries the self-damage scale (0.5) and knockback scale (4.0), but `BaseDamage` and `SplashRadius` are `UPROPERTY` defaults on [AQuakeProjectile_Rocket](Source/Quake/QuakeProjectile_Rocket.h). The rocket is the thing that knows where it exploded. `HandleImpact` applies splash via `UGameplayStatics::ApplyRadialDamageWithFalloff` with `DamageInnerRadius = 0`, `DamageOuterRadius = 120`, `DamageFalloff = 1.0` (linear) — do not call `ApplyPointDamage` separately for the direct-hit victim, since at `InnerRadius = 0` they are already inside the splash and take full `BaseDamage` from the radial call. Adding a direct-hit `ApplyPointDamage` would double the direct-hit damage (regression trap).

**The linear falloff formula is extracted as a pure static helper** `AQuakeProjectile_Rocket::ComputeLinearFalloffDamage(BaseDamage, Distance, Radius)` so it can be unit-tested without spinning up a world — same pattern as `ApplyArmorAbsorption`, `ApplyQuakeAirAccel`, and `ComputePainChance`. The helper mirrors the engine's `ApplyRadialDamageWithFalloff` math (InnerRadius 0, linear): full damage at `d=0`, zero at `d>=r`, linear interpolation in between. The `Quake.Weapon.RocketSplash.*` test suite regresses the four canonical distances from SPEC (0 / half / edge / beyond) plus a zero-radius guard.

**Rocket-jumping works with zero extra code in TakeDamage** because the knockback path in [AQuakeCharacter::TakeDamage](Source/Quake/QuakeCharacter.cpp) already reads `DT->SelfDamageScale` and `DT->KnockbackScale` via the shared-base CDO cast. Adding a new splash weapon only requires a new `UQuakeDamageType_*` subclass with appropriate constructor overrides plus a new `AQuakeProjectile_*` subclass — `TakeDamage` is closed to modification for damage-type metadata.

**Nails are point-damage, not splash** — [AQuakeProjectile_Nail::HandleImpact](Source/Quake/QuakeProjectile_Nail.cpp) calls `ApplyPointDamage` on the hit actor with the nail's velocity-normalized direction as the hit axis, then destroys. World geometry hits skip the damage call (no actor to damage) but still destroy the nail. Knockback stacks naturally via the damage-scaled formula in `TakeDamage` at `ScaledDamage * 30 * 1.0` per hit, so sustained fire at 8/sec compounds the impulse without any special weapon-level code.

## Architecture: Pickups

`AQuakePickupBase` is the `UCLASS(Abstract)` base for every pickup (health, ammo, and — in later phases — armor, powerup, key). Owns a `USphereComponent` (root, overlap-only on the `Pickup` object channel, 64 u default), a `UStaticMeshComponent` (visual, no collision), and a `UPointLightComponent` (glow). `BeginPlay` binds `OnComponentBeginOverlap` on the sphere to a handler that casts the overlapping actor to `AQuakeCharacter` (per SPEC 1.6 rule 4 — enemies trigger the overlap but the cast bails), calls the virtual `CanBeConsumedBy` gate, then `ApplyPickupEffectTo`, then `Destroy()`. Subclasses override `CanBeConsumedBy` (optional, default true) and `ApplyPickupEffectTo` (`PURE_VIRTUAL`).

- **[AQuakePickup_Health](Source/Quake/QuakePickup_Health.h)** — `HealthAmount` + `bIsOvercharge`. Non-overcharge variants refuse consume when HP is already at max (SPEC 4.1); Megahealth consumes up to the 200 HP overcharge ceiling (`AQuakeCharacter::GetOverchargeCap()`).
- **[AQuakePickup_Ammo](Source/Quake/QuakePickup_Ammo.h)** — `AmmoType` + `AmmoAmount`. Always consumed, even when already at cap, matching original Quake (excess wasted). If "no waste" is ever wanted, override `CanBeConsumedBy`.

All three rules above are C++ defaults; BP subclasses (`BP_Pickup_Health15`, `BP_Pickup_Health25`, `BP_Pickup_Megahealth`, `BP_Pickup_AmmoShells`, etc.) only fill mesh + material + light color asset slots.

## Architecture: AI Split

AI is split between the body and the brain per standard UE convention:

- **[AQuakeEnemyBase](Source/Quake/QuakeEnemyBase.h) : public ACharacter** (the pawn / body) — capsule, primitive mesh component slots (`BodyMesh`/`HeadMesh`/`WeaponMesh`), movement, health, `TakeDamage` override. Exposes action methods `MoveToTarget`, `FireAtTarget` (PURE_VIRTUAL — same CDO gotcha as `AQuakeWeaponBase::Fire`, use the engine macro), `PlayPainReaction`, `PlayDeathReaction`, plus `Die(Killer)` that handles the 2 s corpse channel flip + unpossess. All SPEC section 3.1 stats (`MaxHealth`, `WalkSpeed`, `SightRadius`/`LoseSightRadius`/`PeripheralVisionAngleDegrees`/`HearingRadius`/`SightMaxAge`, `AttackRange`/`AttackDamage`/`AttackCooldown`) live on the pawn as `UPROPERTY` defaults, set in each leaf subclass's constructor. **No decision-making code lives on the pawn.**
- **[AQuakeEnemyAIController](Source/Quake/QuakeEnemyAIController.h) : public AAIController** (the brain) — tick-driven state machine (`EQuakeEnemyState::Idle → Alert → Chase → Attack → Pain → Dead`, no Behavior Trees), target tracking, owns a `UAIPerceptionComponent` with `UAISenseConfig_Sight` and `UAISenseConfig_Hearing`. The sight config must set `DetectionByAffiliation.bDetectNeutrals = true` (plus Enemies/Friendlies) or the player's neutral team attitude hides it from the perception sweep. On `OnPossess`, the controller re-reads the per-enemy sight/hearing tuning from the pawn's `UPROPERTY`s and calls `ConfigureSense` on each config so a single controller class works for every enemy type. `OnDamaged(Instigator, Amount)` is the pawn → controller entry point called from `TakeDamage`; it promotes the instigator to the current target, rolls pain chance, and cuts straight to Chase (skipping the Alert pulse). `NotifyPawnDied` is called from `Die` before the unpossess, so tests can observe `GetCurrentState() == Dead` after the kill.
- **[AQuakeEnemy_Grunt](Source/Quake/QuakeEnemy_Grunt.h)** / **AQuakeAIController_Grunt** is the Phase 3 reference pair. The Grunt overrides `FireAtTarget` for the hitscan rifle (trace on the `Weapon` channel with `UQuakeDamageType_Bullet`). `AQuakeAIController_Grunt` is an empty marker subclass — the base controller's FSM is sufficient. Follow this split for every new enemy type: the leaf pawn sets stats + overrides its attack, the leaf controller only exists when there is per-type FSM behavior to add (Fiend leap, Ogre arc lob, Zombie revive).

**`bUseLoSHearing` is deprecated in UE 5.7.** The default hearing behavior is already walls-don't-block (matching Quake), so the controller does not set it — writing to it triggers a deprecation warning that violates the "zero warnings" exit criterion. If a future UE upgrade needs the new API, update [QuakeEnemyAIController.cpp](Source/Quake/QuakeEnemyAIController.cpp) and leave a breadcrumb in this section.

**Weapon noise events.** SPEC 3.3: firing *any* weapon (Axe included — this matches original Quake, and Phase 3's manual verification explicitly tests it) calls `UAISense_Hearing::ReportNoiseEvent` at the swing/shot origin. [QuakeWeapon_Axe.cpp Fire](Source/Quake/QuakeWeapon_Axe.cpp) is the canonical call site; copy the pattern in future weapons. The noise tag is `QuakeWeaponFire`; range 0 means the sense config's `HearingRange` is the authoritative cutoff.

**Pain chance is a pure static helper** `AQuakeEnemyBase::ComputePainChance(Damage, MaxHealth)` — `min(0.8, (damage / max_health) * 2)`, extracted so it can be unit-tested without a world or a pawn. Same pattern as `AQuakeCharacter::ApplyArmorAbsorption` and `UQuakeCharacterMovementComponent::ApplyQuakeAirAccel`. The cap of 0.8 is the regression target; an earlier spec-fidelity pass missed the `min()` and every hit produced a flinch.

Per-enemy behavior variations live on per-enemy AIController subclasses, not on pawn subclasses. This is what makes the AI debugger (`'` key in PIE) work and keeps "what I am" separate from "what I'm doing."

## Architecture: Activation Chains

Buttons, triggers, doors, spawn points, and the level exit communicate via the `IQuakeActivatable` interface — a **pure C++ virtual** `Activate(AActor* Instigator)`, **not** a `BlueprintNativeEvent`. Do not write `Activate_Implementation` (that suffix is only valid for `BlueprintNativeEvent` methods, and using it with this interface will not compile). Sources hold typed `TArray<TObjectPtr<AActor>>` slots filled per-instance in the editor via the actor picker (eyedropper UX). **No string-name targeting** — Quake's original `targetname`/`target` lookup is intentionally not used because it loses refactor traceability and editor pick-list filtering. See SPEC sections 5.5 and 5.6.

## Risk Note: Strafe-Jumping CMC

[UQuakeCharacterMovementComponent](Source/Quake/QuakeCharacterMovementComponent.h) is **the single biggest risk in the project**. It overrides `CalcVelocity` (not `PhysFalling`) and routes the `MOVE_Falling` branch through `ApplyQuakeAirAccel`, which implements the original Quake `PM_AirAccelerate` formula: clamp the **dot product** of velocity and wishdir to `MaxAirSpeedGain`, NOT the velocity magnitude. Stock `UCharacterMovementComponent::CalcVelocity` ends with `Velocity.GetClampedToMaxSize(MaxInputSpeed)` — that magnitude clamp is exactly what breaks Quake strafe jumping, which is why the falling branch must not call `Super::CalcVelocity`. `PhysFalling` zeroes `Velocity.Z` before calling `CalcVelocity` and restores it afterward (gravity is applied separately via `NewFallVelocity`), so `ApplyQuakeAirAccel` only touches horizontal components.

`ApplyQuakeAirAccel` is a pure static function so the formula can be unit-tested without spinning up a world. [Source/Quake/Tests/QuakeCharacterMovementComponentTest.cpp](Source/Quake/Tests/QuakeCharacterMovementComponentTest.cpp) has the canonical regression: velocity `(300,0,0)` + wishdir `(0,1,0)` + `MaxAirSpeedGain=30` must yield a Y gain of exactly 30 with X untouched. Any change to the formula must keep these tests green — the `Quake.Movement.AirAccel.HighSpeedStrafe` test specifically guards against the stock-CMC `MaxWalkSpeed` clamp being accidentally reintroduced.

**Bunny-hop window.** `ProcessLanded` captures pre-landing horizontal velocity; `DoJump` restores it to `Velocity.X/Y` if the jump fires within `BunnyHopWindow` seconds (default 100 ms) and then consumes the window by setting `LastLandedWorldTime = -1`. `Super::DoJump` only touches `Velocity.Z`, so overwriting X/Y after calling it is safe.

**All movement parameters from SPEC 1.1 live in `UQuakeCharacterMovementComponent`'s constructor**, not on `AQuakeCharacter`. `AQuakeCharacter`'s constructor uses `ObjectInitializer.SetDefaultSubobjectClass<UQuakeCharacterMovementComponent>(ACharacter::CharacterMovementComponentName)` to swap the CMC type; BP subclasses inherit this automatically (BPs cannot override component class choice, only property defaults).

If asked to tune movement:

- **Feel is binary.** Either the dot-product clamp is right and speed climbs past `MaxWalkSpeed` when air-strafing on `Content/Maps/Tests/PhysSandbox.umap`, or the whole game stops feeling like Quake. There is no "70% strafe-jumping."
- Use the debug speedometer in [QuakeHUD](Source/Quake/QuakeHUD.h) (Speed / Z vel / MovementMode in the top-left) to verify the feel test at a glance. It's wired via `AQuakeGameMode::HUDClass`.
- **BP property overrides shadow C++ defaults.** If you change a CMC default in the constructor but `BP_QuakeCharacter` has a stored override for that property, the BP wins and your C++ change is silently ignored. Open the BP, select the `CharacterMovement` component in the Details panel, and right-click → Reset to Default on any overridden values after editing C++ defaults. This is the most common reason a "tuning" change appears to do nothing.

Multiplayer would additionally require `FSavedMove_Character` / `FNetworkPredictionData_Client_Character` overrides for client-side prediction. This is famously the part where "I added Quake movement to UE" projects break, and it's why this project is single-player only (SPEC constraint).

## Architecture: Input Configuration

Enhanced Input uses Editor-authored assets (Input Actions and Input Mapping Context), assigned to `UPROPERTY(EditDefaultsOnly)` slots on `AQuakePlayerController` via a Blueprint subclass (`BP_QuakePlayerController`). The PlayerController adds the mapping context to the Enhanced Input subsystem in `BeginPlay`. Pawns access actions via `GetController<AQuakePlayerController>()->MoveAction` etc. and bind handlers in `SetupPlayerInputComponent`.

When adding a new input action, add a new `UPROPERTY(EditDefaultsOnly)` slot on `AQuakePlayerController`, create the IA asset in the Editor, map it in IMC_Default, then assign it in the BP subclass. Do not create IA/IMC objects at runtime with `NewObject<>()` — that approach was tried and reverted.

## Architecture: Editor-Only Pieces

The following must exist in the Editor; they cannot be created from C++ alone (per `SPEC.md` section 10):

- **Levels** (`.umap`) under `Content/Maps/` — World Partition disabled per level.
- **BP framework subclasses** under `Content/Blueprints/Framework/` — thin Blueprint subclasses of the C++ framework classes that hold asset + slot defaults C++ can't reference directly:
  - `BP_QuakeGameMode` — wired via Project Settings as the default GameMode.
  - `BP_QuakeGameInstance` — wired via Project Settings as the Game Instance Class. Owns the `OwnedWeaponClasses[8]` slot array (SPEC 2.0 weapon numbers) that the Character reads at `BeginPlay`. Unpopulated slots mean "not owned"; if every slot is empty the player spawns weaponless and you'll see a warning in the output log.
  - `BP_QuakePlayerController` — owns the Input Action + Input Mapping Context asset-reference slots (`MoveAction`, `LookAction`, `JumpAction`, `FireAction`, `Weapon1Action`, `Weapon2Action`, `Weapon4Action`, `Weapon7Action`, `InputMappingContext`, …). Adding a new input action = add a new `UPROPERTY(EditDefaultsOnly)` slot on `AQuakePlayerController`, create the IA asset, map it in `IMC_Default`, then assign it in this BP.
  - `BP_QuakeCharacter` — holds the mesh / material asset slots (future) and any per-BP movement overrides. Note the "BP property overrides shadow C++ defaults" gotcha in the CMC Risk Note section.
- **Enemy placements** — always `AQuakeEnemySpawnPoint` actors with `EnemyClass` set to a `BP_Enemy_*` class. Direct `BP_Enemy_*` placements in a level are decoration and **do not count** toward `KillsTotal` or gate the level-clear scan. This is the canonical authoring path for counted enemies; see SPEC section 5.1.
- **One master material** `M_QuakeBase` with `BaseColor` / `Emissive` / `Metallic` / `Roughness` parameters; everything else is a `MaterialInstance` of it. Runtime tinting (damage flash, powerup overlays) uses `UMaterialInstanceDynamic` from C++.
- **NavMesh** — `NavMeshBoundsVolume` per level, agent radius 35 / height 180 / step height 45 to match the player capsule and movement params (step height MUST equal the player's value or AI cannot path through geometry the player can walk over).
- **Collision channels** — four custom channels (`Pickup`, `Projectile`, `Corpse` object channels + `Weapon` trace channel) defined in `Config/DefaultEngine.ini`. Per-actor responses are set in C++ constructors (`SetCollisionResponseToChannel` / `SetCollisionProfileName`), not via editor profile assets. C++ code references the channels via the named constexpr mirror in [QuakeCollisionChannels.h](Source/Quake/QuakeCollisionChannels.h) (`QuakeCollision::ECC_Weapon`, etc.) — never use raw `ECC_GameTraceChannelN` literals, because the channel→index binding lives in the INI and the mirror is the single source of truth on the C++ side. See SPEC section 1.6 for the full response matrix and per-system rules.
- **Project Settings** in `Config/Default*.ini` — version-controlled.

The per-level checklist in `SPEC.md` section 10.5 is the authoritative reference when creating new maps.

## Auto-Memory

The user has a persistent file-based memory system at `C:\Users\Admin\.claude\projects\c--dev-games-unreal\memory\`. Save user/feedback/project memories there per the auto-memory protocol, and check existing memories before recommending design choices.
