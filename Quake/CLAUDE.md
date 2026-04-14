# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Unreal Engine 5.7 single-player FPS recreating original Quake gameplay with primitive shapes. v1 is feature-complete in code; remaining work is editor-side map authoring (Phase 15) tracked in [TODO.md](TODO.md).

**C++ first, thin BP layer.** All gameplay logic in C++. Blueprints are thin subclasses with **zero event-graph nodes** — asset-reference slots and per-instance property tuning only. Runtime behavior goes in the C++ base.

- **[DESIGN.md](DESIGN.md)** — durable "what the game is" (movement/damage formulas, class hierarchy, collision, game rules). Read this before non-trivial gameplay changes.
- **[HUD.md](HUD.md)** — HUD layout, wireframe, data sources.
- **[TODO.md](TODO.md)** — v2 backlog.

Module name: `Quake`.

**DESIGN is the design source of truth.** When DESIGN and this file disagree on gameplay or data ownership, DESIGN wins — update CLAUDE.md to match. CLAUDE.md is working notes for build/tooling/conventions, not a design doc.

**Read surgically.** Code comments use section numbers like "SPEC 3.3" / "DESIGN 3.3" interchangeably — they're stable; grep `^### <N>\.<M>` in DESIGN.md for the section. Read with tight `limit=30-50` windows — prefer targeted reads over top-to-bottom.

## Build Commands

Build the editor target:

```bash
"/c/Program Files/Epic Games/UE_5.7/Engine/Build/BatchFiles/Build.bat" QuakeEditor Win64 Development -Project="c:/dev/games-unreal/Quake/Quake.uproject" -WaitMutex -FromMsBuild
```

If the build fails with **"Unable to build while Live Coding is active"**, ask the user to close the Editor or press Ctrl+Alt+F11. Do not work around it.

Regenerate VS Code project files manually after adding new `.h`/`.cpp` files:

```bash
export UE_DOTNET_DIR='/c/Program Files/Epic Games/UE_5.7/Engine/Binaries/ThirdParty/DotNet/8.0.412/win-x64'
export DOTNET_ROOT="$UE_DOTNET_DIR" PATH="$UE_DOTNET_DIR:$PATH" DOTNET_MULTILEVEL_LOOKUP=0 DOTNET_ROLL_FORWARD=LatestMajor
dotnet '/c/Program Files/Epic Games/UE_5.7/Engine/Binaries/DotNET/UnrealBuildTool/UnrealBuildTool.dll' -projectfiles -project='C:/dev/games-unreal/Quake/Quake.uproject' -game -engine -VSCode
```

The system has only .NET 10; UnrealBuildTool needs .NET 8. The bundled SDK at the path above is the working version — never invoke a system `dotnet` for UBT.

## IDE Integration

IntelliSense errors like `cannot open source file "InputModifiers.h"` are usually false positives from a stale `compileCommands_Quake.json`. The truth is a real `Build.bat` run, not IDE squiggles — regenerate project files before assuming an include is wrong.

## Running Tests

Tests live under `Source/Quake/Tests/` and use `IMPLEMENT_SIMPLE_AUTOMATION_TEST` with `EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter`, guarded by `#if WITH_DEV_AUTOMATION_TESTS`. Run via **Session Frontend → Automation tab → filter `Quake.*`** — the runtime discovery is the test inventory; one test file per phase / subsystem.

**Single test / suite.** The Automation filter is substring-matched against the test name declared in `IMPLEMENT_SIMPLE_AUTOMATION_TEST`. Type the full name for one test (`Quake.Movement.AirAccel.HighSpeedStrafe`) or a prefix for a suite (`Quake.Ammo` runs every ammo-inventory test). Click *Start Tests*.

**Prefer pure static helpers over world-spinup tests.** `ApplyQuakeAirAccel`, `ApplyArmorAbsorption`, `ComputePainChance`, `ComputeLinearFalloffDamage`, `PickAutoSwitchWeaponSlot`, `IsLevelClearedForSet`, `ComputeScaledEnemyStats`, `ComputeConsumedNames`, `CanQuickSave`, `ShouldRouteToWinScreen`, `UQuakeSoundManager::ResolveRowName` are the templates. Functional tests requiring a world are deferred to manual sandbox-map verification.

**Test seams.** Test-only setters (e.g., `AQuakeEnemyBase::SetHealthForTest`) wrap in `#if WITH_DEV_AUTOMATION_TESTS` so they don't ship.

**Module include path.** Tests `#include` headers from the module root because [Quake.Build.cs](Source/Quake/Quake.Build.cs) sets `PrivateIncludePaths.Add(ModuleDirectory)`. Don't remove it.

**LWC numeric-assert gotcha.** UE 5.5+ `FVector` components are `double`. `TestEqual(TEXT("..."), Vec.X, 30.f, KINDA_SMALL_NUMBER)` is an ambiguous-overload error. Use `30.0` and `UE_KINDA_SMALL_NUMBER` for any vector-component assertion.

**DataTable rows.** Tests exercise the C++-default fallback only, world-free. Row-name conventions live next to the row structs in [QuakeBalanceRows.h](Source/Quake/QuakeBalanceRows.h).

## Architecture: State Ownership

Where data lives is determined by lifecycle. Wrong placement breaks respawn, level transitions, or the HUD.

- **`UQuakeGameInstance`** — passive mailbox (`TransitSnapshot`, `LevelEntrySnapshot`), `PendingLoad`, `CurrentDifficulty`, `SoundEventTable`, save plumbing. No live inventory. Survives `OpenLevel` and Character respawn. Paths that need it resolve via `UQuakeGameInstance::GetChecked(this)` — `checkf` fires if the project's `GameInstanceClass` isn't set to `BP_QuakeGameInstance` (or a subclass). Null-tolerant `GetGameInstance<>()` is reserved for HUD/menu polling paths that can legitimately run outside a configured game.
- **`UQuakeInventoryComponent`** — ammo counts, armor, owned weapon classes. Attached to `AQuakeCharacter` as `CreateDefaultSubobject`. MP-ready: living on the pawn means replication is a drop-in follow-up (add `UPROPERTY(Replicated)` + `GetLifetimeReplicatedProps`).
- **`AQuakePlayerState`** — per-attempt stats (Kills/Secrets/Deaths/Time), powerups, keys. Auto-cleared on `OpenLevel`; **NOT** cleared on pawn respawn (UE preserves PlayerState across death) — death-restart must call `ClearPerLifeState()`, which deliberately preserves the score counters.
- **`AQuakeCharacter`** — live health, weapon instances, pain/death flags. Dies with the body. Facade methods forward to `InventoryComponent` (ammo, armor, weapons) and `PlayerState` (keys, powerups). **`Move`/`Look`/`OnFirePressed` early-return when `bAwaitingRestart`** so the death screen can consume Fire for the restart prompt. Use `static constexpr NumWeaponSlots` everywhere — never hardcode `8`; `static_assert` guards in [QuakePickup_Weapon.h](Source/Quake/QuakePickup_Weapon.h) and `PickAutoSwitchWeaponSlot` catch drift against that constant at compile time.
- **`AQuakeGameMode`** — level denominators, spawn rules, win routing, restart orchestration. `GetDifficulty()` forwards to GameInstance — GameMode does not own the field.
- **`AQuakeHUD`** — paints from all of the above; Slate widget caches weak pointers and reads them on paint. Three viewport overlays: main HUD (z=10), death (z=20), win (z=30).

Do not put live inventory on the GameInstance (that was the pre-refactor design; no MP analog). Do not put per-level stats on Character or GameMode. Do not put settings (sensitivity, volume) on PlayerState.

## Architecture: Inventory Component

`UQuakeInventoryComponent` attached to `AQuakeCharacter` owns live ammo, armor, and `OwnedWeaponClasses`. The pawn dies on `OpenLevel`; the GameInstance survives and holds `FQuakeInventorySnapshot TransitSnapshot` as a **one-shot mailbox**. The next-spawned pawn's component consumes the snapshot in `InitializeComponent` (pre-BeginPlay, so GameMode::BeginPlay's auto-save + level-entry snapshot see the hydrated state) and clears `bValid`. Matches Quake 1's `parm1..parm16` globals pattern.

**Mailbox writers:**
- `AQuakeTrigger_Exit::OnStatsScreenTimeout` → serializes live component into `GI->TransitSnapshot` just before `OpenLevel` (level-to-level handoff).
- `UQuakeGameInstance::ApplyInventorySnapshot` (called by `LoadFromSlot`) → copies from `UQuakeSaveGame::InventorySnapshot` (quickload).
- `UQuakeGameInstance::RestoreFromLevelEntrySnapshot` (called by `RequestRestartFromDeath`) → copies from `LevelEntrySnapshot` (death-restart).

**Mailbox reader**: `UQuakeInventoryComponent::InitializeComponent`. If invalid, falls back to UPROPERTY defaults (the `BP_QuakeCharacter` authored starting loadout + `StartingShells`).

**BP authoring point:** `BP_QuakeCharacter → InventoryComponent → OwnedWeaponClasses` slots. Pre-refactor these lived on `BP_QuakeGameInstance`; the migration step is to copy the slot references over. An empty array at runtime logs a `LogQuakeInventory` warning and the player spawns weaponless until authored.

**Tests** construct the component standalone via `NewObject<UQuakeInventoryComponent>()` — same shape as the pre-refactor GameInstance tests. Snapshot round-trip uses `SerializeTo` / `DeserializeFrom` and needs no world.

**MP future**: add `UPROPERTY(Replicated)` on `Armor` / `ArmorAbsorption` / `OwnedWeaponClasses` / `AmmoCounts` + `GetLifetimeReplicatedProps` + `SetIsReplicated(true)` in ctor. `TransitSnapshot` and `LevelEntrySnapshot` go unused (MP respawn = default loadout per match), harmlessly.

## Architecture: Damage Pipeline

All damage flows through UE's `TakeDamage`:

1. Attackers call `UGameplayStatics::ApplyPointDamage`/`ApplyRadialDamage`/`ApplyDamage`.
2. Targets override `AActor::TakeDamage` to decrement health, apply armor, knockback.
3. **No code outside `TakeDamage` mutates health.** `Health -= X` anywhere else is a bug. `AQuakeCharacter::Health` and `AQuakeEnemyBase::Health` are both `private` with a single `SetHealth` chokepoint — the compiler enforces the invariant, and every legitimate writer (BeginPlay seed, GiveHealth, TakeDamage, Die) routes through the chokepoint. Save-archive reflection bypasses access by design.

Damage metadata (self-damage scale, armor bypass, pain suppression, knockback) lives on `UDamageType` subclasses (SPEC 1.5), not on the attacker or target. All Quake damage types inherit from abstract `UQuakeDamageType` which owns every Quake-specific UPROPERTY; leaves only override defaults in their constructor. Read fields in `TakeDamage` via the shared-base CDO cast — never branch on leaf class identity. Adding a damage source = ~10-line subclass overriding constructor defaults + calling the appropriate `Apply...Damage` helper. **Splash radius lives on the weapon, not the damage type.**

Attribution uses UE's built-in `EventInstigator` (controller) and `DamageCauser` (projectile/weapon). Self-damage detection, infighting, kill credit all derive from `EventInstigator`.

**Armor absorption: pure static helper** `AQuakeCharacter::ApplyArmorAbsorption`. Formula: `save = ceil(absorption * damage); save = min(save, armor); take = damage - save`. **Float-precision gotcha:** `0.3f * 50.0f` evaluates to `15.000000596...`, which `FMath::CeilToFloat` rounds up to 16 instead of 15. The helper subtracts `UE_KINDA_SMALL_NUMBER` before the ceil — preserve this epsilon or Green/Yellow armor tests go off by one.

**Knockback uses Quake's damage-scaled formula** (`ScaledDamage * 30 * DT->KnockbackScale`), NOT UE's `ApplyDamageMomentum`/`DamageImpulse` (fixed magnitude — would lose damage scaling, breaking rocket-jump variability). Use `FDamageEvent::GetBestHitInfo` for the impulse direction; it unifies point and radial damage in one call. Copy this pattern for any future `TakeDamage` override.

## Architecture: Weapons

`AQuakeWeaponBase` is the `UCLASS(Abstract)` base. Public entry: `TryFire(InInstigator)`. It enforces `RateOfFire` cooldown via `LastFireWorldTime`, runs the SPEC 2.1 ammo gate through the firer's `UQuakeInventoryComponent::ConsumeAmmo` (the instigator is cast to `AQuakeCharacter`; a `checkf` fires if something non-player tries to use an ammo weapon), then calls subclass `Fire`. Weapons with `AmmoType == None` (Axe) bypass the ammo gate. Subclasses only override `Fire`. Stats live on the C++ class as `UPROPERTY` defaults; thin `BP_Weapon_*` only fills asset slots and `ProjectileClass`.

**`Fire` preamble is centralized.** Every subclass `Fire` starts with `if (!GetFireContext(InInstigator, PawnInstigator, World)) return;`. Don't reimplement the boilerplate.

**Abstract `UCLASS` virtuals must use the engine's `PURE_VIRTUAL` macro, not C++ `= 0`.** Reflection constructs a CDO for every UCLASS, including Abstract ones; C++ pure-virtual fails CDO construction with `error C2259: cannot instantiate abstract class`. Use `PURE_VIRTUAL(MyClass::Method, )` — emits a crashing stub that satisfies the CDO. Applies to `AQuakeWeaponBase::Fire`, `AQuakeEnemyBase::FireAtTarget`, `AQuakePickupBase::ApplyPickupEffectTo`, and any future abstract UCLASS virtual.

**`SpawnActor` for a weapon uses the `(UClass*, FActorSpawnParameters)` overload + `AttachToComponent`,** NOT the `(UClass*, FTransform, FActorSpawnParameters)` overload. The transform-overload fails template argument deduction with a `TSubclassOf<T>` first arg (compiler can't pick between FTransform and `(FVector, FRotator)` overloads). Spawn at origin, attach to set transform. See `AQuakeCharacter::SpawnOwnedWeapons` — owned weapons spawn at `BeginPlay`, all but the active one hidden via `SetActorHiddenInGame`.

**`TSubclassOf<T>` in .cpp requires the full `T` header, not a forward decl.** `operator*` and `IsChildOf` need `T::StaticClass()` visible. Any .cpp touching a `TSubclassOf` member must `#include` the concrete header even when the .h forward-declares.

**Empty-ammo auto-switch** is in `AQuakeWeaponBase::TryFire`. On ammo-gate failure: `PlayEmptyClick`, arm cooldown, call `AutoSwitchFromEmptyWeapon()`. The picker is pure-static `AQuakeCharacter::PickAutoSwitchWeaponSlot` walking SPEC 2.2's `RL → SNG → SSG → NG → SG → Axe`. **Thunderbolt and GL are intentionally NOT in the priority list** — keeping them manual prevents a reflex fire from burning the last Cells/Rockets. Axe is the terminal fallback (its `AmmoType::None` is always "has ammo" in the mask). The pure helper takes parallel length-`NumWeaponSlots` bool masks, so the test suite regresses the priority table without a world.

## Architecture: Projectiles

`AQuakeProjectile` is the `UCLASS(Abstract)` base. Owns `USphereComponent` root (Projectile channel, blocks WorldStatic/WorldDynamic/Pawn/Corpse, ignores rest), non-colliding `UStaticMeshComponent`, `UProjectileMovementComponent`. `OnComponentHit` is bound **in the constructor** (so BP subclasses inherit) and forwards to virtual `HandleImpact`. Base `HandleImpact` is no-op — grenades bounce on world hits, so the base doesn't `Destroy()`. Rockets/Nails destroy themselves at the end of their handler.

**Muzzle spawn-out is a three-layer guard** (SPEC 1.6 rule 1): (1) the weapon's `Fire` projects the spawn `MuzzleSpawnForwardOffset = 60u` ahead of the firer's eye; (2) `AQuakeProjectile::BeginPlay` calls `IgnoreActorWhenMoving(GetInstigator())`; (3) `OnSphereHit` bails on `OtherActor == GetInstigator()`. All three redundant by design — lose any one and a tight wall corner can self-detonate.

**Splash damage lives on the projectile subclass, not the damage type.** `UQuakeDamageType_Explosive` carries `SelfDamageScale = 0.5` and `KnockbackScale = 4.0`; `BaseDamage`/`DamageInnerRadius`/`SplashRadius` are UPROPERTY defaults on `AQuakeProjectile_Rocket`. `HandleImpact` uses `ApplyRadialDamageWithFalloff` with `InnerRadius = 60`, `OuterRadius = 120`, `Falloff = 1.0`. **Do not call `ApplyPointDamage` separately for the direct hit** — it would double-count.

**Radial damage measures to the victim's component center, not its surface — this is why `DamageInnerRadius` is non-zero.** `ApplyRadialDamageWithFalloff` line-traces from `Origin` to `VictimComp->Bounds.Origin` via `ComponentIsDamageableFrom`; on a direct hit (origin inside collision), the trace produces no blocking hit and falls back to `ImpactPoint = ComponentLocation`, giving `Distance ≈ sqrt(35² + z_offset²) ≈ 37u` against a player capsule. With `InnerRadius = 0`, that scales `BaseDamage` by `(1 - 37/120) ≈ 0.69`. Plain consequence: **`InnerRadius = 0` with `ApplyRadialDamageWithFalloff` never delivers full `BaseDamage` to a live pawn, even on a dead-center hit.** Rocket sets `InnerRadius = 60` so the ~37u measurement lands inside the full-damage plateau. `AQuakeProjectile_Grenade::HandleImpact` still uses `InnerRadius = 0` — adopt the same plateau next time it's touched.

**`ComputeLinearFalloffDamage` is a pure static helper** describing the falloff *shape* outside the inner plateau (full at d=0, zero at d>=r, linear between). Deliberately doesn't model `DamageInnerRadius` — `Quake.Weapon.RocketSplash.*` regresses the formula, not the in-game damage curve.

**Rocket-jumping needs zero extra TakeDamage code** because the knockback path already reads `DT->SelfDamageScale` and `DT->KnockbackScale` via the CDO cast. New splash weapon = new damage-type subclass + new projectile subclass. `TakeDamage` is closed to modification for damage-type metadata.

**Quad damage is frozen at launch, not impact.** `AQuakeProjectile::DamageScale` (default 1.0) is set by the firing weapon's `Fire` to `AQuakeCharacter::GetOutgoingDamageScale()` — the multiplier baked on the shot. Rocket / Nail / Grenade multiply `BaseDamage * DamageScale` inside their `HandleImpact`. Matches Quake: a rocket fired during Quad lands at 4× even if the timer expires mid-flight. Self-damage scale is applied on top inside `TakeDamage` via `UQuakeDamageType::SelfDamageScale`, so SPEC 4.3's "Quad does not affect splash self-damage scale" comes out of the existing pipeline for free. Enemy projectiles (Ogre grenades) default `DamageScale = 1.0` — enemies don't read PlayerState.

**Nails are point-damage** — `AQuakeProjectile_Nail::HandleImpact` calls `ApplyPointDamage` with the velocity-normalized direction, then destroys. World hits skip damage but still destroy. Damage-scaled knockback compounds naturally with sustained fire.

**Grenades** explode on `ACharacter` contact, no-op on world hits (letting `bShouldBounce = true`, `Bounciness = 0.4` handle the bounce). 2.5s fuse via `FTimerHandle` set in `BeginPlay`, never reset by bouncing. 0.25s firer-grace via `FirerGraceEndTime` — after that, walking into your own grenade detonates it. Splash uses `UQuakeDamageType_Explosive` and the same `ApplyRadialDamageWithFalloff` call as the rocket.

## Architecture: Pickups

`AQuakePickupBase` is the `UCLASS(Abstract)` base. Owns `USphereComponent` (Pickup channel, overlap-only), non-colliding mesh + light. `BeginPlay` binds overlap → cast to `AQuakeCharacter` (SPEC 1.6 rule 4) → virtual `CanBeConsumedBy` gate → `ApplyPickupEffectTo` (PURE_VIRTUAL) → `Destroy()`.

**Subclass placement rules:**
- **Refresh / cap logic (powerup duration, armor tier replacement) lives on PlayerState/Character, not the pickup.** Pickup decides "can I be consumed?"; the recipient owns "how does this combine with what I have?"
- **`AQuakePickup_Weapon`: ammo always granted first**, then `Character->GiveWeaponPickup` which is idempotent. SPEC 2.2 "first pickup auto-switches" is inside `GiveWeaponPickup` — don't re-implement in the pickup.

BP subclasses (`BP_Pickup_*`) only fill mesh/material/light asset slots + UPROPERTY defaults.

## Architecture: Balance DataTables

Stats centralized in `DT_EnemyStats` / `DT_WeaponStats`, configured via `UQuakeProjectSettings` (**Project Settings > Game > Quake**). Conventions and adding-a-new-enemy walkthrough live next to the row structs in [QuakeBalanceRows.h](Source/Quake/QuakeBalanceRows.h).

**Loading order gotcha.** `AQuakeEnemyBase::PostInitializeComponents` loads DataTable stats *before* `Super` because Super triggers `SpawnDefaultController` → `OnPossess`, which reads perception stats. Loading in `BeginPlay` would leave the controller seeing stale C++ defaults. Weapons can load in `BeginPlay` — no auto-possess timing constraint.

## Architecture: AI Split

Body/brain split per UE convention:

- **`AQuakeEnemyBase` : ACharacter** (body) — capsule, mesh slots, movement, health, `TakeDamage` override. Action methods: `MoveToTarget`, `FireAtTarget` (PURE_VIRTUAL), `PlayPainReaction`, `PlayDeathReaction`, `Die(Killer, bGibbed)` (handles 2s corpse-channel flip + unpossess). SPEC 3.1 stats live as UPROPERTY defaults, set per leaf. **No decision-making code on the pawn.**
- **`AQuakeEnemyAIController` : AAIController** (brain) — tick-driven FSM (`Idle → Alert → Chase → Attack → Pain → Dead`, no Behavior Trees), perception via `UAIPerceptionComponent` with `UAISenseConfig_Sight` + `UAISenseConfig_Hearing`. The sight config **must set `DetectionByAffiliation.bDetectNeutrals = true`** (plus Enemies/Friendlies) or the player's neutral team hides them from the sweep. On `OnPossess` the controller reads per-pawn tuning and calls `ConfigureSense`, so one controller class works for every enemy. `OnDamaged(Instigator, Amount)` is the pawn → controller entry from `TakeDamage`; promotes the instigator to current target and cuts to Chase. `NotifyPawnDied` is called from `Die` before unpossess so tests can observe `Dead` state.
- **Per-type controller policy** — only subclass when there's per-type FSM behavior. `_Grunt`/`_Knight` are empty marker subclasses; `_Ogre` is the per-type FSM template (two attacks: grenade lob at range, chainsaw in close — `Tick` override picks one based on `DistToTarget <= MeleeThreshold`). The pawn's `AttackRange` is set to the longer (grenade) range so the base Chase→Attack transition fires early enough to reach melee.

**Per-state Tick override gotcha.** A controller subclass intercepting one FSM state must **not** call `Super::Tick` for that state — the base's switch would also run its branch (e.g., base Attack calls `Enemy->FireAtTarget`, double-firing). Call the grandparent `AAIController::Tick` and manually replicate the pre-switch bookkeeping (`TimeInState += DeltaTime`, Dead/null-pawn check, grudge expiry). `AQuakeAIController_Ogre::Tick` is the canonical pattern.

**Infighting (SPEC 3.3).** When `TakeDamage` runs with an `EventInstigator` whose pawn is another enemy, `OnDamaged` seeds a grudge (`GrudgeTarget`, `GrudgeExpireTime = Now + 10s`). `Tick` clears the grudge and reverts to the player when the timer expires or the grudge target dies. `OnTargetPerceptionUpdated` accepts stimuli from the player OR active grudge target. No infighting-specific damage code — emergent from `EventInstigator`-driven target switching alone.

**`bUseLoSHearing` is deprecated in UE 5.2.** Default is `false`, which already means walls don't block hearing (matches Quake). Don't write to it — triggers a deprecation warning that violates the zero-warnings exit criterion. A `checkf(!HearingConfig->bUseLoSHearing, ...)` guard in the controller constructor fires at runtime if a future engine upgrade ever flips the default; when the field is eventually removed, that block stops compiling and the maintainer has to confirm the new default still matches Quake.

**Weapon noise events.** SPEC 3.3: every weapon (Axe included — matches original Quake) calls `UAISense_Hearing::ReportNoiseEvent` at the swing/shot origin. Tag is `QuakeWeaponFire`; range 0 means the sense config's `HearingRange` is the cutoff. See `AQuakeWeapon_Axe::Fire`.

**Pain chance is pure-static** `AQuakeEnemyBase::ComputePainChance(Damage, MaxHealth) = min(0.8, (damage/maxhealth) * 2)`. The 0.8 cap is the regression target.

Per-enemy variations live on AIController subclasses, not pawn subclasses — keeps "what I am" separate from "what I'm doing" and makes the AI debugger (`'` in PIE) work.

## Architecture: Drops and Gibs

**Drop table** is a `TArray<FQuakeDropEntry> DropTable` UPROPERTY on `AQuakeEnemyBase`. Each entry: `{ TSubclassOf<AQuakePickupBase> PickupClass, int32 Quantity, float Chance }`. Filled per-enemy-type in BP. `SpawnDrops` rolls `FMath::FRand()` per entry against `Chance`. Drops only spawn on non-gib deaths.

**`TSubclassOf<AQuakePickupBase>` in `FQuakeDropEntry` requires the full `AQuakePickupBase` header in `QuakeEnemyBase.h`** — same gotcha as `TSubclassOf<AQuakeWeaponBase>` in [QuakeInventoryComponent.h](Source/Quake/QuakeInventoryComponent.h). The include must come *before* `QuakeEnemyBase.generated.h` (UHT rejects includes after generated.h).

**Gib detection** at the end of `TakeDamage` before `Die`: `Overkill = ScaledDamage - HealthBefore`; gib when `Overkill >= HealthBefore * 2`. `HealthBefore = Health + ScaledDamage` since Health is already decremented. `Die(Killer, bGibbed)` dispatches to `PlayGibReaction` vs `PlayDeathReaction` and skips drops on gib. Player death is **not** gib-aware.

## Architecture: Stats and Level-Clear

Per SPEC 5.9, stats split as a **numerator/denominator pair**, never duplicated: denominators (`KillsTotal`, `SecretsTotal`) on `AQuakeGameMode`, numerators (`Kills`, `Secrets`, `Deaths`, time-elapsed) on `AQuakePlayerState`. Denominators computed once in `BeginPlay` via `TActorIterator` and never updated — a deferred spawn point still owes its enemy.

**Level-clear scan reads spawn points, not enemies.** `AQuakeGameMode::IsLevelCleared` iterates spawn points and bails on any eligible-but-unsatisfied entry. This handles three edge cases without branching: (1) unfired deferred spawns are unsatisfied; (2) infighting/hazard kills satisfy regardless of player credit; (3) Down-state Zombies (forward-compat) report `IsDead() = false`. Pure helper `IsLevelClearedForSet` is the world-free test target.

**`AQuakeEnemySpawnPoint` is the only authoring path for counted enemies.** Direct `BP_Enemy_*` placements are decoration — `bIsMarkedKillTarget` defaults false; only `TrySpawn` flips it true. Death credit (`Die`) only routes to PlayerState when marked.

**Kill credit rules** (in `Die`): `PlayerState.Kills` increments only when marked AND one of: `Killer && Killer->IsPlayerController()` (direct), or `Killer == nullptr && bPlayerHasDamagedMe` (hazard kill where the player softened first). Infighting kills grant no credit; the spawn point still satisfies.

**`bTransitionInFlight` blocks a second exit-trigger overlap from re-firing.** Phase 13: final-level exits show `ShowWinScreen()` and route to `MainMenuMapName` via the pure predicate `ShouldRouteToWinScreen`.

## Architecture: Activation Chains

Buttons, triggers, doors, spawn points, and the level exit communicate via `IQuakeActivatable` — a **pure C++ virtual** `Activate(AActor* InInstigator)`, **NOT** a `BlueprintNativeEvent`. Do not write `Activate_Implementation` (that suffix is `BlueprintNativeEvent`-only and won't compile). Sources hold typed `TArray<TObjectPtr<AActor>>` slots filled per-instance via the editor picker (eyedropper UX). **No string-name targeting** — see SPEC 5.5/5.6.

**Always name the interface parameter `InInstigator`, not `Instigator`.** `AActor` has a protected `TObjectPtr<APawn> Instigator` member, so any override using `Instigator` trips `error C4458: declaration hides class member` under warnings-as-errors.

**Trigger hierarchy.** `AQuakeTrigger` is the `UCLASS(Abstract)` base with `UBoxComponent TriggerVolume` (query-only, overlap Pawn) and `TArray<TObjectPtr<AActor>> Targets`. Default `Activate` calls `FireTargets` (iterate, cast to `IQuakeActivatable`, log on null/non-impl). `BeginPlay` binds `OnComponentBeginOverlap` → `Activate`. Subclasses override `Activate` and call `Super::Activate(InInstigator)` to fire the chain (or omit Super for self-contained triggers like `_Hurt`).

**Overlap filter: player-only by default** (Quake `SF_TRIGGER_ALLOWMONSTERS` = off). `AQuakeTrigger::OnTriggerBeginOverlap` rejects any actor that isn't `AQuakeCharacter` unless `bAllowMonsters` is true, so an enemy chasing the player can't open the exit, burn a secret, or fire a message. Type-based check (`Cast<AQuakeCharacter>`) rather than controller-based so the filter is stable across possession timing — enemies subclass `AQuakeEnemyBase`, not `AQuakeCharacter`. `_Hurt` and `_Teleport` flip `bAllowMonsters = true` in their constructors (kill floors and teleporters affect monsters in original Quake). `Activate()` calls from a chain bypass the filter — a relay fired by any source still propagates.

| Subclass | Behavior |
|----------|----------|
| `_Relay` | No-op override — fan-out only. |
| `_Hurt` | Self-contained, no Super. Per-pawn repeating damage timer with `UQuakeDamageType_Telefrag`. Cleared on `EndOverlap`. |
| `_Teleport` | Teleports a `ACharacter` instigator to `Destination`'s transform. Velocity magnitude preserved, rotated to destination yaw. |
| `_Message` | `AQuakeHUD::ShowMessage(FText, float)` then Super. |
| `_Secret` | One-shot via `bCredited`. Increments `PlayerState->Secrets`, shows "A secret area!". |
| `_Spawn` | Fires the typed `SpawnPoints` list (deferred spawn points spawn now), then Super. |
| `_Exit` | See **Stats and Level-Clear** above for gating + stats-screen flow. |

**Buttons are input sources, not chainables.** `AQuakeButton` does NOT implement `IQuakeActivatable`. It holds its own `Targets` and fires on Touch-overlap or Shoot-hit. For "fire on signal" relays use `AQuakeTrigger_Relay`. Collision supports both touch and shoot simultaneously (overlap Pawn, block Weapon + Visibility); `ActivationMode` gates which signal calls `Fire`. Same player-only default as triggers via its own `bAllowMonsters` flag — Touch-overlap and Shoot-damage paths both route the instigator through `AQuakeTrigger::IsPlayerPawn` before firing.

## Architecture: Doors

`AQuakeDoor` is **tick-driven linear interpolation, not `UTimelineComponent`.** Timeline curves require a `.uasset` authored in the editor — pulls gameplay data out of C++ and violates the "C++ first" rule. State machine `Closed → Opening → Open → Closing → Closed`; `Opening`/`Closing` step the relative location by `speed * DeltaSeconds` toward the target. **Do not resurrect `UTimelineComponent`** for doors without a strong reason.

**Closing safety is two-layer.** (1) Before transitioning `Open → Closing`, `TryStartClosing()` queries `BlockingZone->GetOverlappingActors(..., ACharacter::StaticClass())`; if anyone's in the doorway, re-arm the auto-close timer for 0.5s and retry. Prevents starting to close around a walking pawn. (2) Once `Closing`, the mesh's `SetRelativeLocation(..., bSweep=true)` fires `OnComponentHit` → `HandleCrushHit` applies 10000 damage with `UQuakeDamageType_Telefrag`; the move completes (stopping would trap the corpse). Pre-close = gate, mid-close = crush; both needed.

**Keys.** `CanOpenFor(AActor*)` casts the instigator to `AQuakeCharacter` and calls `HasKey(RequiredKey)`. A non-player activator (trigger relay, enemy overlap) on a locked door is refused — keeps "a dying Grunt brushed the door open" from happening. Locked feedback uses `AQuakeHUD::ShowMessage` ("You need the silver/gold key." for 2 s) per SPEC 10.

## Architecture: Hazard Volumes

`AQuakeHazardVolume` (lava/slime) uses a **per-pawn `FTimerHandle` map** keyed by weak pointer. `BeginOverlap`: launch character horizontally away from center (200u for lava, 0 for slime), register repeating `TickRate` timer applying `DamagePerTick` via `ApplyPointDamage`. `EndOverlap`: clear that pawn's timer. `EndPlay` sweeps all to avoid dangling callbacks.

**First damage tick fires at `TickRate`, not on entry.** Entry knockback is the on-contact punishment; damage is sustained. For on-entry damage (crusher pits), call `ApplyTickDamage` directly from `BeginOverlap` — `AQuakeTrigger_Hurt` does this explicitly because kill volumes should bite the moment you step in.

**`UQuakeDamageType_Lava` carries `bSuppressesPain = true`** (no flinch animation per tick) and `bCausedByWorld = true` (attributes to `<world>`, prevents enemy-vs-enemy grudge seeding when lava kills an enemy).

## Architecture: HUD

`AQuakeHUD` constructs `SQuakeHUDOverlay` (pure Slate, no UMG) in `BeginPlay` and adds it to the viewport. The widget caches a weak `APlayerController` pointer and resolves the pawn on every paint — survives pawn replacement (death/respawn, teleport).

**Transient messages.** `ShowMessage(FText, float)` stores one active message with a world-time expiry; `DrawHUD` paints it centered near the top. Later calls replace any visible message (no queue — "latest wins" matches original Quake's single-line message area). The width calc uses `Canvas->ClipX`, which **requires `#include "Engine/Canvas.h"`** — `Canvas` is a `TObjectPtr<UCanvas>` forward-declared in `AHUD.h`, so accessing `->ClipX` without the full header errors with `error C2027: use of undefined type 'UCanvas'`.

**Level-end stats screen.** `ShowLevelEndStats(Duration)` flips a world-time visibility gate on the Slate overlay. The widget polls PlayerState/GameMode every paint — no refresh needed.

## Architecture: Save/Load (Phase 11)

`IQuakeSaveable` is a **pure C++ virtual** interface (same rule as `IQuakeActivatable` — no `BlueprintNativeEvent`, no `_Implementation` suffix). `SaveState(FActorSaveRecord&)` writes `GetFName()` + an `FMemoryWriter` blob; `LoadState(const FActorSaveRecord&)` reads it. Stable identity is `AActor::GetFName()` (editor-assigned, serialized with the `.umap`); runtime-spawned actors are intentionally excluded — their FNames are unstable.

**Archive helpers** in [QuakeSaveArchive.h](Source/Quake/QuakeSaveArchive.h): `WriteSaveProperties` / `ReadSaveProperties` wrap `FMemoryWriter` + `FObjectAndNameAsStringProxyArchive` with `ArIsSaveGame = true`. Any `UPROPERTY(meta = (SaveGame))` round-trips; everything else is filtered out. **Forgetting `ArIsSaveGame = true` serializes everything**, including transient fields, and bloats payloads.

**Save flow** (`UQuakeGameInstance::SaveCurrentState`): capture inventory snapshot → `GameMode->CaptureWorldSnapshot` (level name, pawn transform, PlayerState, per-actor records via `TActorIterator<AActor>` + `Implements<UQuakeSaveable>()`, consumed-pickup set difference) → `UGameplayStatics::SaveGameToSlot`.

**Load flow** (DESIGN 6.2): `LoadFromSlot` restores GameInstance fields BEFORE `OpenLevel`, stashes `PendingLoad`. The new world's `GameMode::BeginPlay` consumes it: restore PlayerState (with time-base translation), teleport pawn, dispatch `LoadState` to FName-matched actors, destroy consumed pickups. **`TActorIterator<IQuakeSaveable>` does NOT work** — interfaces aren't AActor types; iterate `TActorIterator<AActor>` and gate on `Implements<UQuakeSaveable>()`.

**Consumed pickups** use a global `TSet<FName> ConsumedPickupNames` on the save, not per-pickup records — a `Destroy()`-ed pickup has no `this` to author a record. GameMode caches `InitialPickupNames` at `BeginPlay`; save time computes `Initial \ Live` via pure-static `ComputeConsumedNames`.

**F5 gate** (`CanQuickSave` pure-static): `MovementMode == MOVE_Walking && !bIsInPain && !bIsDead`. The pain flag is set in `TakeDamage` whenever the damage type doesn't have `bSuppressesPain` (Lava ticks intentionally don't block saves), cleared 0.5s later via `PainClearTimer`.

**Door state on load.** Mid-animation states (`Opening`/`Closing`) snap to nearest static endpoint (`Open`/`Closed`) — we don't persist intermediate mesh positions. `EQuakeDoorState` is a top-level `UENUM` (promoted from a nested enum so UHT can reflect it for `meta = (SaveGame)`).

**Spawn point on load.** `bHasFired` is the persisted truth. `IsSatisfied()` returns true when `bHasFired && SpawnedEnemy == nullptr` (LoadState destroys the fresh re-spawned pawn so the level reflects the saved "this wave is dead" state). `TrySpawn` short-circuits on `bHasFired` so deferred-spawn `Activate` calls are no-ops on a loaded save.

## Architecture: Difficulty (Phase 12)

Difficulty stored on `UQuakeGameInstance` (survives `OpenLevel`); `AQuakeGameMode::GetDifficulty` forwards. Multipliers `TMap` on GameMode is BP-tunable, seeded with DESIGN 6.1 defaults in the constructor.

**Damage scaling is frozen at launch, matching the Quad pattern.** Hitscan `Fire` paths multiply by `AttackDamageMultiplier`; the Ogre grenade bakes `Projectile->DamageScale` at spawn — a difficulty change mid-flight does not affect in-flight grenades.

**Nightmare pain immunity.** `AQuakeEnemyAIController::OnDamaged` skips the pain roll when `GetDifficultyMultipliers().bSuppressPain` is true.

## Architecture: Failure + Win Flow (Phase 13)

Inventory handoff on death-restart: see **Architecture: Inventory Component** above. Health is intentionally NOT in the snapshot (DESIGN 6.4: death always restores full HP).

**Win routing is the pure predicate** `AQuakeGameMode::ShouldRouteToWinScreen(bIsFinal, NextMapName)` — final-level always wins regardless of NextMapName; non-final never does. Test target.

## Architecture: Main Menu (Phase 13)

**Cursor / input-mode reset gotcha.** `LocalPlayer` and Slate viewport state survive `OpenLevel` even though the `PlayerController` is destroyed and recreated. Without an explicit reset, the new in-game PC inherits `UIOnly` + cursor-on from the menu. `AQuakePlayerController::BeginPlay` therefore calls `SetInputMode(FInputModeGameOnly())` and `bShowMouseCursor = false` unconditionally — preserve this or transitioning MainMenu → Hub leaves controls dead.

## Architecture: Audio + Settings (Phase 14)

**Audio entry point** is `UQuakeSoundManager : UGameInstanceSubsystem`. Every gameplay sound flows through `UQuakeSoundManager::PlaySoundEvent(WorldContext, EQuakeSoundEvent, Location)` — the static convenience that finds the manager off any UObject and forwards. The enum catalogs DESIGN 8.2 events; rows in `DT_SoundEvents` (`FQuakeSoundEvent { Sound, VolumeMultiplier, PitchMultiplier }`) drive the actual playback. Phase 14 ships with every row's `Sound = nullptr` so gameplay paths exercise the system end-to-end before any audio assets exist; missing/null rows = `LogQuakeSound` Verbose log + no-op.

**`SoundEventTable` lives on `UQuakeGameInstance`, not on the subsystem,** because UE forbids Blueprint subclasses of `UGameInstanceSubsystem` and we need a BP slot for the asset reference. The manager calls `Cast<UQuakeGameInstance>(GetGameInstance())` on first use and caches the resolved table. Use `Cast<...>(GetGameInstance())` — the templated `GetGameInstance<T>()` overload is not available on subsystems in UE 5.7.

**Row-name lookup is a compile-time switch** over every `EQuakeSoundEvent` value — NOT `UEnum::GetNameStringByValue` reflection (the reflection approach silently re-keyed every row when an enum value was renamed). DT_SoundEvents row keys must still match the UENUM value names verbatim (`PlayerJump`, `DoorOpen`, `WeaponShotgunFire`, …), but the switch now fails to compile on an unhandled enum, so a rename forces a matching case update. `UQuakeSoundManager::ResolveRowName` is the pure helper exposed for `Quake.Phase14.Sound.ResolveRowName`.

**Per-enemy sound variation lives on DT_SoundEvents rows (volume/pitch), not on the pawn.** Legacy `PainSound`/`DeathSound` UPROPERTY slots were removed from `AQuakeEnemyBase`. If a future enemy needs a unique sound asset, add a new `EQuakeSoundEvent::EnemyPain_Shambler` row rather than re-introducing per-pawn slots.

**Pain sound is in `TakeDamage`'s non-fatal branch only.** `PlayPainReaction` is the visual-flinch hook — emitting sound there double-plays on non-fatal hits and overlaps `EnemyDeath` on the killing blow.

**Settings persistence: `UQuakeGameUserSettings : UGameUserSettings`** holds `MouseSensitivity` and `MasterVolume` as `Config` UPROPERTYs (auto-persisted to `GameUserSettings.ini` on `SaveSettings()`). Wired in `Config/DefaultEngine.ini` via `[/Script/Engine.Engine] GameUserSettingsClassName=/Script/Quake.QuakeGameUserSettings` — without that line, `UQuakeGameUserSettings::Get()` returns null and the settings menu falls back to the live pawn's `LookSensitivity`. Both setters clamp; `SetToDefaults` resets to 0.5 / 1.0. `AQuakeCharacter::BeginPlay` pulls the saved sensitivity into `LookSensitivity` so death-respawn / level-load applies the persisted value.

**Volume slider is "stored only" in v1.** No `USoundMix` routing yet — Phase 14 scope is "settings persist". Hooking the slider to a master sound class happens once the first real audio asset lands.

## Risk Note: Strafe-Jumping CMC

`UQuakeCharacterMovementComponent` is **the single biggest risk in the project**. It overrides `CalcVelocity` (marked `final`) and routes `MOVE_Falling` through the `CalcAirVelocity` seam, which calls `ApplyQuakeAirAccel` — original Quake's `PM_AirAccelerate`: clamp the **dot product** of velocity and wishdir to `MaxAirSpeedGain`, NOT the velocity magnitude. Stock `UCharacterMovementComponent::CalcVelocity` ends with `Velocity.GetClampedToMaxSize(MaxInputSpeed)` — that magnitude clamp is exactly what breaks Quake strafe jumping, so the falling branch must NOT call `Super::CalcVelocity`. `final` on `CalcVelocity` + the dedicated `CalcAirVelocity` seam is how the "don't call Super here" contract is enforced structurally; subclasses that need to tweak airborne behavior override `CalcAirVelocity` instead. `PhysFalling` zeroes `Velocity.Z` before calling `CalcVelocity` and restores it after (gravity applied via `NewFallVelocity`); `CalcAirVelocity` asserts both preconditions (`MovementMode == MOVE_Falling`, `Velocity.Z ≈ 0`) on entry so an engine-upgrade regression fires here instead of silently re-enabling a vertical clamp.

`ApplyQuakeAirAccel` is pure-static so the formula is unit-tested without a world. Canonical regression: velocity `(300,0,0)` + wishdir `(0,1,0)` + `MaxAirSpeedGain=30` must yield Y gain of exactly 30 with X untouched. `Quake.Movement.AirAccel.HighSpeedStrafe` specifically guards against accidentally re-introducing the stock `MaxWalkSpeed` clamp.

**Bunny-hop window.** `ProcessLanded` captures pre-landing horizontal velocity; `DoJump` restores it to `Velocity.X/Y` if jump fires within `BunnyHopWindow` (default 100ms), then consumes the window via `LastLandedWorldTime = -1`. `Super::DoJump` only touches `Velocity.Z`, so overwriting X/Y after Super is safe.

**All movement parameters live in `UQuakeCharacterMovementComponent`'s constructor**, not on `AQuakeCharacter`. Character constructor uses `ObjectInitializer.SetDefaultSubobjectClass<UQuakeCharacterMovementComponent>(ACharacter::CharacterMovementComponentName)` to swap the CMC type; BP subclasses inherit (BPs cannot override component class, only property defaults).

If asked to tune movement:

- **Feel is binary.** Either the dot-product clamp is right and speed climbs past `MaxWalkSpeed` while air-strafing on `Content/Maps/Tests/PhysSandbox.umap`, or the game stops feeling like Quake. There is no "70% strafe-jumping."
- The debug speedometer in `AQuakeHUD::DrawHUD` (Speed / Z vel / MovementMode top-left) verifies the feel test at a glance.
- **BP property overrides shadow C++ defaults.** Changing a CMC default in C++ when `BP_QuakeCharacter` has a stored override does nothing — the BP wins silently. After editing C++ defaults, open the BP, select the CharacterMovement component, right-click → Reset to Default on overridden values. Most common reason a tuning change "appears to do nothing."

Multiplayer would additionally require `FSavedMove_Character` / `FNetworkPredictionData_Client_Character` overrides for client-side prediction. This is famously where "I added Quake movement to UE" projects break — and why this project is single-player only.

## Architecture: Input Configuration

Enhanced Input uses **Editor-authored** Input Action and Input Mapping Context assets, assigned via `UPROPERTY(EditDefaultsOnly)` slots on `AQuakePlayerController` through `BP_QuakePlayerController`. The PlayerController adds the mapping context to the Enhanced Input subsystem in `BeginPlay`. Pawns access actions via `GetController<AQuakePlayerController>()->MoveAction` and bind in `SetupPlayerInputComponent`.

Adding a new input action: add a `UPROPERTY(EditDefaultsOnly)` slot on `AQuakePlayerController`, create the IA asset, map it in `IMC_Default`, assign in `BP_QuakePlayerController`. **Do not create IA/IMC at runtime with `NewObject<>()`** — that approach was tried and reverted.

**Weapon-switch bindings use Enhanced Input's variadic `BindAction`** so a single `OnWeaponSlotPressed(const FInputActionValue&, int32 SlotIndex)` covers all slots (slot index passed as a bound extra arg). See the `WeaponBindings` table in `AQuakeCharacter::SetupPlayerInputComponent`. New slot = add the IA on the PlayerController, add `{ Action, SlotIndex }` to the table. Don't add per-slot handlers.

## Architecture: Editor-Only Pieces

Per SPEC section 10, these can't be created from C++ alone:

- **Levels** (`.umap`) under `Content/Maps/` — World Partition disabled per level. `MainMenu.umap` (Phase 13) uses `BP_QuakeMenuGameMode` as its World Settings GameMode override; pick it as the project default in **Project Settings → Maps & Modes → Game Default Map**. `Hub.umap` is a thin level holding one `BP_Trigger_Teleport` per episode portal.
- **BP framework subclasses** under `Content/Blueprints/Framework/` — thin BPs holding asset/slot defaults C++ can't reference directly:
  - `BP_QuakeGameMode`, `BP_QuakeGameInstance` (no more `OwnedWeaponClasses` slots post inventory-component refactor — `SoundEventTable` + BP-subclass presence for `GetChecked` assertion is all that remains), `BP_QuakePlayerController` (owns IA/IMC slots, including Phase 11 `IA_QuickSave`/`IA_QuickLoad` mapped to F5/F9 in `IMC_Default`), `BP_QuakeCharacter` (mesh slots + `InventoryComponent->OwnedWeaponClasses` slots — empty all-slots logs warning and spawns weaponless; watch the BP-shadows-C++ gotcha).
  - `BP_QuakeMenuGameMode` + `BP_QuakeMenuHUD` (Phase 13) for `MainMenu.umap`. The HUD BP sets `HubMapName` — the map opened after difficulty selection. **Setting it on the C++ class doesn't expose it**, so a BP subclass is required.
  - For an episode-final level: a `BP_QuakeGameMode_*` with `bIsFinalLevel = true` and `MainMenuMapName = "MainMenu"`. Used as the level's World Settings GameMode override.
- **Enemy placements** — always `AQuakeEnemySpawnPoint` actors with `EnemyClass` set to a `BP_Enemy_*` (Grunt/Knight/Ogre). Direct `BP_Enemy_*` placements are decoration and don't count toward `KillsTotal` (SPEC 5.1). `BP_Enemy_Ogre` must have `GrenadeClass` set to `BP_Projectile_Grenade` or grenade lobs no-op.
- **World-element BPs** under `Content/Blueprints/World/` — `BP_Door`, `BP_Button`, `BP_HazardVolume_Lava`, `BP_Trigger_*` (Relay/Hurt/Teleport/Message/Secret/Spawn/Exit). Each fills mesh/material/collision-extent slots only — zero event-graph nodes. Per-instance fields (`Targets`, `SpawnPoints`, `Destination`, `Message`, `NextMapName`, `RequiredKey`, `bGatedByClearCondition`, `StatsDisplaySeconds`, etc.) are set on the placed instance, not the BP. `_Hurt` is self-contained — `Targets` is ignored. `_Exit` defaults `bGatedByClearCondition = true`; flip off for hub exits / escape sequences. `BP_HazardVolume_Lava`'s C++ defaults already match lava — the visible lava surface is a separate `StaticMeshActor` with `MI_Lava` placed adjacent (the volume itself is invisible by design).
- **DataTables** under `Content/Data/` — `DT_EnemyStats`, `DT_WeaponStats`, both assigned in **Project Settings > Game > Quake**. Row names must match `StatsRowName` per leaf C++ constructor.
- **Master material** `M_QuakeBase` with `BaseColor`/`Emissive`/`Metallic`/`Roughness` parameters; everything else is a `MaterialInstance`. Runtime tinting via `UMaterialInstanceDynamic`.
- **NavMesh** — `NavMeshBoundsVolume` per level, agent radius 35 / height 180 / **step height 45 — MUST equal player's** or AI can't path through geometry the player can walk over.
- **Collision channels** — four custom channels (`Pickup`, `Projectile`, `Corpse` + `Weapon` trace) in `Config/DefaultEngine.ini`. Per-actor responses set in C++ constructors (never via editor profile assets). Reference channels via the `QuakeCollision::ECC_*` constexpr mirror in [QuakeCollisionChannels.h](Source/Quake/QuakeCollisionChannels.h) — never raw `ECC_GameTraceChannelN` literals (the channel→index binding lives in the INI; the mirror is the C++-side source of truth). See SPEC 1.6 for the response matrix.
- **Project Settings** in `Config/Default*.ini` — version-controlled.

Per-level checklist in SPEC section 10.5 is the authoritative reference for new maps.

## Auto-Memory

The user has a persistent file-based memory system at `C:\Users\Admin\.claude\projects\c--dev-games-unreal\memory\`. Save user/feedback/project memories there per the auto-memory protocol; check existing memories before recommending design choices.
