# Quake — v1 Roadmap

v1 is the first playable milestone: 1 hub + 3 levels, 4 weapons, 3 enemies. Delivered in 16 phases (0–15), each gated on automated tests + manual verification. See [DESIGN.md](DESIGN.md) for the durable "what the game is" reference.

## Principles

- **Each phase produces a runnable artifact.** Sandbox map, test level, or content level — never a pure plumbing layer.
- **Risk first, content last.** Strafe-jumping CMC is the single highest-risk item; it gets its own phase (1) before any combat code.
- **No cross-phase work.** Don't add keys while in pickups. Phase boundaries exist so each test pass is targeted.
- **Exit criteria are absolute.** A phase is done when (a) its automated tests pass, (b) manual verification passes, (c) prior phases' tests still pass.
- **Test layers:**
  - Unit tests (`IMPLEMENT_SIMPLE_AUTOMATION_TEST`, `EditorContext | EngineFilter`): pure-C++ formulas, state transitions, struct round-trip.
  - Functional tests (`AFunctionalTest` in `Content/Maps/Tests/`): spawn an actor, drive input, assert a result.
  - Manual smoke (human in PIE): feel, polish, emergent behavior.

**Test-style convention.** Per [CLAUDE.md](CLAUDE.md), prefer pure-static helpers over world-spinup tests. Phase test lists below call out "Unit" vs "Functional" explicitly; functional tests requiring a live world that don't yet exist are deferred to manual sandbox-map verification.

## Content Scope

| Category   | v1                                                                 | Deferred                             |
|------------|--------------------------------------------------------------------|--------------------------------------|
| Episodes   | 1 (Episode 1)                                                      | 2, 3, 4                              |
| Levels     | 1 hub + E1M1 (intro) + E1M2 + E1M3 (boss)                          | More E1 maps, other episodes         |
| Weapons    | Axe, Shotgun, Nailgun, Rocket Launcher                             | SSG, SNG, GL, Thunderbolt            |
| Enemies    | Grunt, Knight, Ogre                                                | Fiend, Shambler, Zombie              |
| Boss       | Greater Ogre (800 HP Ogre variant) at end of E1M3                  | True bosses                          |
| Pickups    | All health, all armor tiers, ammo for v1 weapons, Silver + Gold keys | Cells, cells pickups                 |
| Powerups   | Quad                                                               | Pentagram, Ring, Biosuit             |
| Hazards    | Lava                                                               | Slime, water, drowning               |
| Difficulty | Easy, Normal                                                       | Hard, Nightmare                      |
| Saves      | Auto-save on level start, quick save/load                          | Multiple slots, save menu UI         |

## Out of Scope (v1)

- Water / swimming / drowning / Thunderbolt discharge
- Slime hazard
- All powerups except Quad
- Save/load UI menu
- Episode hub portal art (teleport-on-touch volume is enough)
- Localization
- Gamepad input (M+KB only)
- Cutscenes / intermissions

## Phase Table

| #  | Phase                          | Deliverable                                                              | Risk     |
|----|--------------------------------|--------------------------------------------------------------------------|----------|
| 0  | Foundation                     | Build, test runner, collision channels, sandbox loads                    | Low      |
| 1  | Player Movement Sandbox        | `UQuakeCharacterMovementComponent` with strafe-jump                      | **High** |
| 2  | Damage Pipeline + Axe          | `TakeDamage`; Axe damages a dummy                                        | Low      |
| 3  | First Enemy (Grunt)            | Grunt patrols, sees, shoots                                              | Medium   |
| 4  | Shotgun + Ammo + First Pickups | Shells, Shotgun, health pickups                                          | Low      |
| 5  | Rocket Launcher + Splash       | Rockets, splash, self-damage, rocket-jump                                | Medium   |
| 6  | Nailgun                        | All 4 v1 weapons                                                         | Low      |
| 7  | More Enemies + Drops + Infighting | Knight, Ogre, drops, emergent infighting                               | Medium   |
| 8  | Level Structure                | Doors, buttons, triggers, lava                                           | Low      |
| 9  | Spawn Points + Stats           | Spawn points, Kills/Secrets/Time/Deaths, level-clear gating              | Low      |
| 10 | Powerups + Keys + Full HUD     | Quad, locked doors, full HUD bar                                         | Low      |
| 11 | Save / Load                    | Quick save/load round-trip                                               | Medium   |
| 12 | Difficulty                     | Easy/Normal; multipliers; spawn-point filtering                          | Low      |
| 13 | Main Menu / Hub / Win / Death  | Full game flow launch-to-win                                             | Low      |
| 14 | Audio Stubs + Settings         | Sound manager no-ops; sensitivity/volume save                            | Low      |
| 15 | Content Authoring              | E1M1, E1M2, E1M3, Hub shipped                                            | Medium   |

---

## Phase 0 — Foundation

**Goal.** Build works. Test runner discovers `Quake.*`. Collision channels configured. Sandbox loads in PIE.

**Implements:**
- Stub classes: `AQuakeGameMode`, `AQuakePlayerController`, `AQuakePlayerState`, `UQuakeGameInstance`, `AQuakeCharacter`.
- `UQuakeDamageType` abstract base (DESIGN 1.5) — no leaves yet.
- Custom collision channels in `Config/DefaultEngine.ini` (DESIGN 1.6).
- Thin BPs: `BP_QuakeGameMode`, `BP_QuakeGameInstance`, `BP_QuakePlayerController`.
- Project Settings: Game Instance Class, Default GameMode.
- `Content/Maps/Tests/PhysSandbox.umap` — flat plane, 30/40/45° ramps, 256u gap, walls, `PlayerStart`, `NavMeshBoundsVolume`.
- Enable UE Automation Testing plugin.

**Tests:**
- [x] Unit: trivial `Quake.Foundation.Smoke` always-passes test proves the runner picks up the module.
- [x] Build smoke: `Build.bat` succeeds with zero warnings on new files.

**Manual:**
- [x] Editor opens without errors.
- [x] `PhysSandbox` opens.
- [x] PIE spawns a default Character; mouse look works.
- [x] Session Frontend → `Quake.*` filter → trivial test green.
- [x] Project Settings shows the 4 custom channels.

**Exit.** Trivial test green; PIE on `PhysSandbox` doesn't crash.

---

## Phase 1 — Player Movement Sandbox

**Goal.** Strafe-jump and bunny-hop on the sandbox. **Highest-risk item in v1** — see [CLAUDE.md "Risk Note: Strafe-Jumping CMC"](CLAUDE.md).

**Implements:**
- `UQuakeCharacterMovementComponent : UCharacterMovementComponent`. Override `CalcVelocity` falling branch with Quake air-accel (clamp `dot(velocity, wishdir)`, NOT magnitude). **Do not** call `Super::CalcVelocity` on the falling branch — its `GetClampedToMaxSize(MaxInputSpeed)` is exactly what breaks strafe-jumping.
- All movement params (DESIGN 1.1) as UPROPERTY defaults on the CMC.
- Bunny-hop window (100 ms post-land preserves horizontal velocity).
- `AQuakeCharacter` installs the custom CMC via `ObjectInitializer.SetDefaultSubobjectClass`.
- `BP_QuakePlayerController` IA/IMC assets for Move/Look/Jump.
- Debug HUD speedometer (`Speed`, `Z vel`, `MovementMode`) — top-left, `DrawHUD` path.

**Tests:**
- [x] Unit: `ApplyQuakeAirAccel` canonical case — velocity (300,0,0), wishdir (0,1,0), MaxAirSpeedGain 30 → Y gain = 30, X untouched.
- [x] Unit: dot-product clamp does NOT re-introduce magnitude clamp (velocity (300,0,0) + strafe must still allow sideways gain).
- [x] Functional: bunny-hop window — apply jump, land, jump within 100 ms → horizontal speed ≥ pre-landing.
- [x] Functional: 30° ramp ascends; 50° ramp rejected.

**Manual:** (binary feel test — no "70% strafe-jump")
- [x] Speedometer shows ~600 on flat ground.
- [x] Hold strafe + W, turn mouse with strafe direction while jumping → **speed climbs past 600.** If not, formula is wrong — do not advance.
- [x] Bunny-hop in place preserves speed.
- [x] Walk up 30° ✓, 40° ✓, 45° ✗.
- [x] No fall damage, no crouch.

**Exit.** Manual speed-gain passes; functional tests green; zero build warnings.

---

## Phase 2 — Damage Pipeline + Axe

**Goal.** `TakeDamage` flows. Axe damages a target dummy. HP on a minimal HUD.

**Implements:**
- `UQuakeDamageType_Melee` leaf.
- `AQuakeCharacter::TakeDamage` — CDO cast, armor absorption helper, knockback via `GetBestHitInfo`, screen flash stub.
- `AQuakeWeaponBase` with `TryFire(Instigator)` + abstract `Fire(Instigator)` (PURE_VIRTUAL — `= 0` fails CDO construction on Abstract UCLASSes).
- `AQuakeWeapon_Axe` — melee trace on `Weapon` channel, 64u range.
- `AQuakeTargetDummy` (test-only): static cube with HP, text readout above head.
- `AQuakeHUD` + minimal `SQuakeHUDOverlay` with HP only.
- `BP_Weapon_Axe` (mesh slot).

**Tests:**
- [x] Unit: `ApplyArmorAbsorption` — 100 HP + 100 Green armor + 50 dmg → HP 65, armor 85. Include the 0.3f×50 float-epsilon case.
- [x] Unit: shared-base CDO cast returns non-null for `_Melee` subclass.
- [x] Functional: dummy at 100 HP, Axe hit → HP 80; five hits → dead.

**Manual:**
- [x] Axe swing in `PhysSandbox` reduces dummy HP by 20.
- [x] Dummy returns fire → pain flash, HUD HP drops.
- [x] Axe 64u range — too far whiffs.

**Exit.** Damage flows both directions; Phase 0–2 tests green.

---

## Phase 3 — First Enemy (Grunt)

**Goal.** One Grunt patrols, sees, shoots. Body/brain split works. AI debugger shows transitions.

**Implements:**
- `AQuakeEnemyBase : ACharacter` with capsule + mesh slots + health + action methods + `TakeDamage`.
- `AQuakeEnemyAIController : AAIController` with FSM + `UAIPerceptionComponent` (Sight + Hearing senses).
- `AQuakeEnemy_Grunt` + `AQuakeAIController_Grunt` (hitscan rifle).
- `UQuakeDamageType_Bullet` leaf.
- `BP_Enemy_Grunt` (capsule body + sphere head + box rifle).
- `OnDamaged(Instigator, Amount)` plumbing pawn → controller.

**Tests:**
- [x] Unit: `ComputePainChance(damage, maxHealth) = min(0.8, (dmg/maxHP) × 2)`. Regress the 0.8 cap.
- [x] Functional: Grunt in `Idle`; player steps into sight cone → `Alert`; pulse elapses → `Chase`.
- [x] Functional: Grunt at 30 HP + 30 dmg from player → `Dead` + unpossessed.
- [x] Functional: Axe fire behind wall (noise) → Grunt becomes `Alert` from hearing.

**Manual:**
- [x] Drop a Grunt in `PhysSandbox`. It alerts, chases, fires.
- [x] Hide behind wall → loses sight after 5 s.
- [x] `'` key → AI debugger shows state, cone, target.
- [x] Grunt killed by Axe enters `Dead`, collapses, turns corpse-channel after 2 s.

**Exit.** AI debugger transitions visible; hearing-through-walls works; Grunt killable.

---

## Phase 4 — Shotgun + Ammo + First Pickups

**Goal.** Ammo inventory on GameInstance. Shotgun kills Grunts. Health/ammo pickups.

**Implements:**
- `UQuakeGameInstance` ammo `TMap` + caps (DESIGN 2.1).
- `AQuakeWeapon_Shotgun` — 6-pellet 4° spread, 4 dmg/pellet, 1.5 Hz.
- `AQuakePickupBase` + `_Health` + `_Ammo` subclasses.
- `AQuakeCharacter` facades: `GiveAmmo`/`ConsumeAmmo`/`GetAmmo` → GameInstance; `GiveHealth` self-owned.
- HUD adds: ammo for active weapon, weapon name.
- BPs: `BP_Pickup_AmmoShells`, `BP_Pickup_Health15/25/Megahealth`, `BP_Weapon_Shotgun`.
- Click stub on empty fire.

**Tests:**
- [x] Unit: `GetAmmoCap` table (Shells 100, Nails 200, Rockets 100, Cells 100).
- [x] Unit: `GiveAmmo` below/at/over cap; return = delta applied.
- [x] Unit: `ConsumeAmmo` success vs insufficient; None short-circuit (Axe).
- [x] Functional: shell pickup adds 20; health pickup at full HP refused.

**Manual:**
- [x] Pick up shell pack, HUD ammo updates.
- [x] Switch 1↔2, fire empty click, fire with ammo.
- [x] Health pickup at full HP stays in world; at 50 HP → 75 + destroyed.
- [x] Megahealth at 100 HP overcharges.

**Exit.** HUD ammo + HP both update; conditional consumption works; Shotgun kills Grunts.

---

## Phase 5 — Rocket Launcher + Splash + Knockback

**Goal.** Rockets explode with falloff. Self-damage 50%. Rocket-jump launches.

**Implements:**
- `AQuakeProjectile` base with sphere + `UProjectileMovementComponent`. `OnComponentHit` bound in constructor → virtual `HandleImpact`.
- `AQuakeProjectile_Rocket` — 1000 u/s straight, no gravity, 8u sphere, `DamageInnerRadius=60`, `SplashRadius=120`.
- `AQuakeWeapon_RocketLauncher`.
- `UQuakeDamageType_Explosive` with `SelfDamageScale=0.5`, `KnockbackScale=4.0`.
- `HandleImpact` → `ApplyRadialDamageWithFalloff` (linear). No separate `ApplyPointDamage` for direct hit (would double-count).
- Knockback in `AQuakeCharacter::TakeDamage` via `GetBestHitInfo`. `ScaledDamage × 30 × KnockbackScale`.
- Muzzle spawn-out (60u forward + `IgnoreActorWhenMoving(Instigator)` + handler bail on `OtherActor == Instigator`).
- `AQuakePickup_Ammo` Rockets variant.
- BPs: `BP_Weapon_RocketLauncher`, `BP_Projectile_Rocket`, `BP_Pickup_AmmoRockets`.

**Tests:**
- [x] Unit: `ComputeLinearFalloffDamage` — d=0 full; d=r/2 half; d≥r zero; d>r zero.
- [x] Functional: detonate at known origin with pawns at varying distances; each takes expected falloff.
- [x] Functional: rocket spawn transform is 60u in front of pawn.
- [x] Functional: rocket fired on spawn frame → no self-damage.

**Manual:**
- [x] Rockets kill Grunts on imperfect aim.
- [x] **Rocket-jump feel test (binary):** look down, fire, jump → launched up, ~25 HP loss.
- [x] Rocket into wall 60u away explodes on wall, not face.
- [x] Rocket at floor 5u away doesn't self-detonate instantly.

**Exit.** Rocket-jump feels right; falloff matches unit test.

---

## Phase 6 — Nailgun

**Goal.** All 4 v1 weapons working. Auto-switch on empty works.

**Implements:**
- `AQuakeProjectile_Nail` — 1500 u/s straight, gravity-free, 2u sphere.
- `UQuakeDamageType_Nail` leaf.
- `AQuakeWeapon_Nailgun` — 8 Hz, 1° spread.
- `AQuakePickup_Ammo` Nails variant.
- BPs: `BP_Weapon_Nailgun`, `BP_Projectile_Nail`, `BP_Pickup_AmmoNails`.
- **Auto-switch on empty.** `AQuakeCharacter::AutoSwitchFromEmptyWeapon()` + pure-static `PickAutoSwitchWeaponSlot` implementing DESIGN 2.2 priority (RL → SNG → SSG → NG → SG → Axe; Thunderbolt and GL excluded).

**Tests:**
- [x] Unit: `PickAutoSwitchWeaponSlot` priority — given various ownership + ammo masks, returns correct slot. Verify Thunderbolt/GL excluded even when owned + fueled. Axe is terminal fallback.
- [x] Functional: hold-fire 1 s with full ammo → 8 nails spawned + 8 ammo consumed.
- [x] Functional: nail at 500u → 9 dmg applied.

**Manual:**
- [x] Hose a Grunt with Nailgun.
- [x] Ammo depletes at 8/sec.
- [x] Switch 1/2/4/7 — instant swap.
- [x] Deplete shells, fire SG → auto-switches per priority.

**Exit.** All 4 weapons fire, swap, auto-switch correctly.

---

## Phase 7 — Knight + Ogre + Drops + Infighting

**Goal.** 3 enemy types. Drop tables. Emergent infighting.

**Implements:**
- `AQuakeEnemy_Knight` + `AQuakeAIController_Knight` (charge melee).
- `AQuakeEnemy_Ogre` + `AQuakeAIController_Ogre` (grenade at range, chainsaw close; `Tick` picks based on `DistToTarget <= MeleeThreshold`).
- `AQuakeProjectile_Grenade` (DESIGN 2.2 lifecycle: 2.5 s fuse via `FTimerHandle` set in `BeginPlay`, never reset by bouncing; explode on Pawn contact; 0.25 s firer-grace; `OnProjectileBounce` delegate for sound stub).
- `DropTable` UPROPERTY on `AQuakeEnemyBase` + `FQuakeDropEntry`. Spawn drops in `Die` (non-gib only).
- Infighting via `OnDamaged` — seed `GrudgeTarget`, expire after 10 s. `OnTargetPerceptionUpdated` accepts stimuli from player OR active grudge target.
- Per-state tick pattern: subclass intercepting one FSM state calls grandparent `AAIController::Tick`, **not** `Super::Tick` (base FSM would double-fire).
- BPs: `BP_Enemy_Knight`, `BP_Enemy_Ogre`, `BP_Projectile_Grenade`, `BP_Pickup_BackpackShells`, `BP_Pickup_BackpackRockets`.

**Tests:**
- [x] Functional: Grunt with `DropTable=[{BackpackShells, 5, 1.0}]` + player kill → pickup spawned at death location.
- [x] Functional: Grunt with `Chance = 0.0` → no pickup.
- [x] Functional: Ogre grenade lands near Knight → Knight's `CurrentTarget` switches to Ogre; grudge timer > 0.
- [x] Functional: gib with overkill ≥ 2× HP → capsule destroyed, no drop.

**Manual:**
- [x] Fight all 3 types in sandbox.
- [x] Pick up backpack drops.
- [x] **Infighting.** Lure Ogre near Knight, dodge so grenade bounces on Knight → Knight charges Ogre.
- [x] Gib a Grunt with direct rocket → scattered pieces, no drop.

**Exit.** Infighting is emergent (no infighting-specific damage code).

---

## Phase 8 — Level Structure

**Goal.** Assemble a real level. Doors, buttons, triggers, lava.

**Implements:**
- `IQuakeActivatable` interface (pure C++ virtual; parameter name `InInstigator` to avoid `AActor::Instigator` shadow).
- `AQuakeDoor` — tick-driven linear interp (no `UTimelineComponent`). Two-layer closing safety. Implements `IQuakeActivatable`.
- `AQuakeButton` — touch + shoot activation, `TArray<TObjectPtr<AActor>> Targets`. Does NOT implement `IQuakeActivatable`.
- `AQuakeTrigger` abstract base + subclasses: `_Relay`, `_Spawn` (stub until Phase 9), `_Message`, `_Hurt`, `_Teleport`, `_Secret`, `_Exit`.
- `AQuakeHazardVolume` (lava: 30 dmg / 1 s + 200u entry knockback; per-pawn `FTimerHandle` map).
- `UQuakeDamageType_Lava` (`bSuppressesPain = true`, `bCausedByWorld = true`); `UQuakeDamageType_Telefrag`.
- `Content/Maps/Tests/LevelStructureSandbox.umap` — small room: door + button + lava + relay + exit.

**Tests:**
- [x] Functional: button overlap → door transitions Opening then Open.
- [x] Functional: pawn in swept volume → door doesn't start Closing.
- [x] Functional: pawn in lava for 1 tick → 30 dmg.
- [x] Functional: `_Relay` with 3 mock targets → all 3 `Activate` calls received exactly once.
- [x] Functional: `_Hurt` with `DamagePerTick=100` over `TickRate` → 100 dmg.
- [x] Functional: `_Teleport` with `Destination=ATargetPoint` → pawn at destination.
- [x] Functional: closed door blocks rocket (explodes on door, not through).

**Manual:**
- [x] Walk `LevelStructureSandbox`. Button → door opens. Lava → 30/sec. Exit trigger logs "level complete." Rocket on closed door explodes on the face.

**Exit.** Activation chains work without name lookups; lava + doors behave per spec.

---

## Phase 9 — Spawn Points + Stats + Level Transitions

**Goal.** Stats numerator/denominator. Exit unlocks only on level clear. Level-end stats screen.

**Implements:**
- `AQuakeEnemySpawnPoint` (DESIGN 5.1).
- `AQuakePlayerState` fields: `Kills`, `Secrets`, `Deaths`, `GetTimeElapsed()`.
- `AQuakeGameMode::KillsTotal` / `SecretsTotal` computed in `BeginPlay` via `TActorIterator`.
- `AQuakeGameMode::IsLevelCleared()` + pure helper `IsLevelClearedForSet(TArray<const AQuakeEnemySpawnPoint*>&)` for world-free tests.
- `AQuakeEnemySpawnPoint::IsSatisfied()`, `IsEligible()`, `IsEligibleForDifficulty(EQuakeDifficulty)`.
- `AQuakeTrigger_Exit` gates on `IsLevelCleared` when `bGatedByClearCondition == true`. On unlock: fire base Targets, show stats screen for `StatsDisplaySeconds`, `OpenLevel(NextMapName)` via timer. `bTransitionInFlight` prevents re-fire.
- `AQuakeTrigger_Spawn` with typed `TArray<TObjectPtr<AQuakeEnemySpawnPoint>> SpawnPoints`.
- HUD adds: `Kills X / N`, `Secrets X / N`, `Time mm:ss`; level-end Slate overlay with visibility gate.
- Kill-credit rules in `AQuakeEnemyBase::Die` (DESIGN 5.9 table).
- `Content/Maps/Tests/StatsSandbox.umap` — 3 marked + 1 unmarked + 1 deferred spawn point, 2 secrets, clear-gated exit.

**Tests:**
- [x] Unit: `IsEligibleForDifficulty` — marked Easy eligible at Normal/Nightmare; Nightmare-only excluded at Easy/Hard.
- [x] Unit: `IsEligible` — unmarked always excluded; deferred still counts.
- [x] Unit: `IsSatisfied` — unspawned = no; spawned + alive = no; spawned + dead = yes.
- [x] Unit: `IsLevelClearedForSet` — empty = clear; all-null = clear; unmarked unsatisfied = clear; marked unsatisfied = block; deferred-unfired = block.
- [x] Unit: forward-compat — revived-zombie (`Health > 0` on marked point) = unsatisfied; permanent kill = satisfied.
- [x] Unit: `AQuakePlayerState::AddKillCredit`/`AddSecretCredit`/`AddDeath`; `ClearPerLifeState` preserves score.

**Manual:**
- [x] `StatsSandbox` — kill 3 marked Grunts → HUD `3/4`. Exit blocked. Fire deferred trigger → 4th spawns. Kill → `4/4`. Exit unlocks.
- [x] Find both secrets → `2/2`, message shown.
- [x] Level-end stats screen shows kills/secrets/time/deaths.

**Exit.** Clear gating works for immediate + deferred; stats match reality.

---

## Phase 10 — Powerups + Keys + Full HUD

**Goal.** All pickup categories working. Quad feels right. Locked doors work.

**Implements:**
- Enums: `EQuakePowerup` ([QuakePowerup.h](Source/Quake/QuakePowerup.h)), `EQuakeKeyColor` ([QuakeKeyColor.h](Source/Quake/QuakeKeyColor.h)).
- `FQuakeActivePowerup { EQuakePowerup Type; float RemainingTime; }` + `ActivePowerups` on `AQuakePlayerState`.
- PlayerState: `GivePowerup(Type, Duration)` (DESIGN 4.3 additive-capped-at-60), `HasPowerup`, `GetPowerupRemaining`, `HasKey`/`GiveKey`, `Keys` storage, `ClearPerLifeState` empties both. `Tick` decrements + prunes + disables itself when empty.
- `AQuakeCharacter::GetOutgoingDamageScale()` — returns 4.0 when Quad active else 1.0.
- `AQuakeCharacter::GiveKey`/`HasKey` facades → PlayerState.
- `AQuakeCharacter::GiveWeaponPickup(Slot, Class)` — idempotent; returns true only on first pickup (weapon spawned + auto-switched).
- `AQuakeProjectile::DamageScale` (default 1.0). Nailgun + RL set it at launch via `GetOutgoingDamageScale`. Rocket/Nail/Grenade multiply `BaseDamage * DamageScale` in `HandleImpact`.
- Axe + Shotgun scale damage via `GetOutgoingDamageScale` at shot time.
- Pickups: `AQuakePickup_Armor` (tier table + DESIGN 1.2 replace rule), `_Powerup` (always consumed — refresh cap lives on PlayerState), `_Key` (silent re-pickup — stays in world), `_Weapon` (ammo always; weapon first-time only).
- `AQuakeDoor::CanOpenFor` queries PlayerState keys via Character facade. Locked feedback: `AQuakeHUD::ShowMessage("You need the silver/gold key.", 2.f)`.
- HUD additions: armor (tier-colored, bottom-left), silver/gold key indicators (bottom-left), Quad countdown (top-center).
- Powerup post-process tint (blue for Quad) via `UMaterialInstanceDynamic` on the level `PostProcessVolume`.
- BPs: `BP_Pickup_Quad`, `BP_Pickup_KeySilver`/`Gold`, `BP_Pickup_Armor_Green`/`Yellow`/`Red`, `BP_Pickup_Weapon_Shotgun`.

**Tests:**
- [x] Unit: `Quake.Powerup.Grant.FreshEntry` — fresh grant creates entry at exact duration.
- [x] Unit: `Quake.Powerup.Grant.AdditiveCapAt60` — two 30 s grants = 60 s (not 2 entries; third no-overflow).
- [x] Unit: `Quake.Powerup.Grant.DifferentTypesStack` — Quad + Pentagram = 2 entries.
- [x] Unit: `Quake.Powerup.Expiry.TickPrunes` — `Tick(31)` removes entry.
- [x] Unit: `Quake.Powerup.Grant.RejectsNoneAndZeroDuration`.
- [x] Unit: `Quake.Key.GiveAndHas` — re-grant no-op; None sentinel rejected.
- [x] Unit: `Quake.Key.ClearPerLifeStateEmptiesKeys` — powerups + keys empty; score preserved.
- [x] Unit: `Quake.Armor.TierTable` — amount + absorption for all 3 tiers.

**Manual:** see [HUD.md](HUD.md) for layout reference + [Phase 10 manual test setup](#phase-10-manual-setup) below.

**Phase 10 manual setup.** Duplicate `LevelStructureSandbox` → `Phase10Sandbox.umap`. Place one of each pickup; a `BP_Door` with `RequiredKey=Silver` beside a silver key; a second `BP_Door` with `RequiredKey=Gold` (no gold key); an Ogre spawn point near Green armor + RL pickup.

- [x] Quad + RL on Ogre: top-center shows `QUAD 30` blue; rocket deals ~400 splash, gibs Ogre.
- [x] Stand still 30 s → Quad timer reaches 0 and disappears.
- [x] Silver key pickup → bottom-left `[SILVER]` indicator. Silver door opens.
- [x] Gold door without gold key → "You need the gold key." top-center for 2 s, door stays shut.
- [x] Green armor → bottom-left `AR 100` green. Take 5 dmg in `_Hurt` → HP -4, armor -2 (30% absorb, `ceil(0.3×5)=2`).
- [x] First Shotgun pickup → auto-switch + ammo grant. Second → shells increase, no switch.

*Note: the blue post-process tint is a content task (PostProcessVolume + MID) and not strictly required for test 1 verification — the timer + damage do the work.*

**Exit.** All pickup categories work; key-locked doors work; Quad scales via `GetOutgoingDamageScale`.

---

## Phase 11 — Save / Load

**Goal.** Quick save / quick load round-trip. Auto-save on level start.

**Implements:**
- `UQuakeSaveGame : USaveGame` (DESIGN 6.2 fields).
- `IQuakeSaveable { SaveState(FActorSaveRecord&); LoadState(const FActorSaveRecord&); }`.
- `FActorSaveRecord { FName ActorName; TArray<uint8> Payload; }`.
- Actors with persistent state implement `IQuakeSaveable`: doors, buttons, secrets, spawn points, pickups, enemies, Character (HP).
- `UQuakeGameInstance::SaveCurrentState()` / `LoadFromSlot(SlotName)`.
- Auto-save on level entry (after snapshot taken).
- F5 / F9 input actions via `AQuakePlayerController`.
- "No mid-air saves" — F5 rejects if `MovementMode != MOVE_Walking` OR pain-reacting.

**Tests:**
- [ ] Functional: open door, save, reload level, load → door Open.
- [ ] Functional: kill enemy at spawn point, save, reload, load → spawn point still satisfied.
- [ ] Functional: pick up health to full, save, take damage, load → HP restored to saved value.
- [ ] Functional: save with Quad (15 s remaining), load → Quad active ≈15 s (±0.1 s).
- [ ] Functional: F5 mid-jump → save rejected.
- [ ] Functional: F5 pain-reacting → save rejected.
- [ ] Unit: `FActorSaveRecord` round-trip — save a level, deserialize, every `ActorName` matches a level actor's `GetFName()`.
- [ ] Functional: actor with no matching record on load → level-default state, no crash.

**Manual:**
- [ ] Mid-level F5. Take damage. F9 → state restored (HP, ammo, Quad timer, doors, dead enemies, pickups).
- [ ] Save in E1M2, restart editor, F9 → E1M2 loads with weapons from E1M1.
- [ ] F5 mid-jump → rejection message.

**Exit.** Round-trip works across PIE restarts; `GetFName()` identity is stable.

---

## Phase 12 — Difficulty

**Goal.** Easy / Normal both playable. Multipliers apply. Spawn-point filtering works.

**Implements:**
- `EQuakeDifficulty` enum + field on `UQuakeGameInstance`.
- `FQuakeDifficultyMultipliers` + `TMap<EQuakeDifficulty, ...> DifficultyTable` on `AQuakeGameMode` (UPROPERTY, BP-tunable).
- `AQuakeEnemyBase::ApplyDifficultyScaling()` (virtual) in `BeginPlay`: `MaxHealth = Base × M.EnemyHP`, `AttackDamageMultiplier = M.EnemyDamage`.
- `AQuakeEnemySpawnPoint::IsEligibleForDifficulty` — already stubbed Phase 9; this phase makes the GameInstance→GameMode plumbing functional.
- `AQuakeAIController_Zombie::ApplyDifficultyScaling` override for revive timing (slot for v2).
- Pain-immunity on Nightmare checked at `OnDamaged`.
- `BP_QuakeGameMode` populates multiplier map.

**Tests:**
- [ ] Functional: Grunt on Easy → `MaxHealth == 30`; on Hard → `MaxHealth == 37.5`.
- [ ] Functional: 5 `MinDifficulty=Easy` + 3 `MinDifficulty=Hard` spawn points → `KillsTotal == 5` on Easy, `== 8` on Hard.
- [ ] Functional: enemy fire at player — damage applied matches per-difficulty multiplier.

**Manual:**
- [ ] Easy: fight Grunts, note TTK + damage taken.
- [ ] Restart Normal, same room → perceptibly harder (0.75 → 1.0).

**Exit.** Multipliers apply at spawn; filtering excludes higher-tier placements.

---

## Phase 13 — Main Menu / Hub / Win / Death

**Goal.** Full launch-to-win flow.

**Implements:**
- `MainMenu.umap` + `SQuakeMainMenu` Slate widget.
- New-game flow: difficulty → Hub → portal → E1M1.
- `Hub.umap` with one `AQuakeTrigger_Teleport` as the Episode 1 portal (no art).
- Death screen Slate widget with "Press Fire to Restart."
- Restart sequence per DESIGN 6.4.
- Win screen Slate (total stats).
- Level-end stats screen finalized.
- Settings Slate: mouse sensitivity (functional), audio volume (no-op until Phase 14).
- GameMode flag "is final level of episode" → win screen instead of next-level transition.

**Tests:**
- [ ] Functional: new-game flow sets `GameInstance->Difficulty` correctly.
- [ ] Functional: kill player → `PlayerState.Deaths += 1`, powerups cleared, inventory matches snapshot.
- [ ] Functional: final-level exit → win screen widget in viewport.
- [ ] Functional: tick while paused → `TimeElapsed` does not advance.

**Manual:**
- [ ] Main Menu → New Game → Easy → Hub. Portal → E1M1 (placeholder).
- [ ] Die in lava → death screen → fire → respawn at `PlayerStart`, +1 Deaths.
- [ ] Manually trigger win → stats screen + return to menu.

**Exit.** Launch-to-win works with placeholder content.

---

## Phase 14 — Audio Stubs + Settings

**Goal.** Every gameplay event reaches the (no-op) sound manager. Settings persist.

**Implements:**
- `UQuakeSoundManager : UGameInstanceSubsystem`.
- `ESoundEvent` enum (DESIGN 8.2).
- `SoundEventTable` UPROPERTY on `UQuakeGameInstance`; `BP_QuakeGameInstance` assigns `DT_SoundEvents.uasset`.
- `DT_SoundEvents` with `FQuakeSoundEvent` rows (all `Sound = nullptr`).
- `UQuakeSoundManager::PlaySound(Event, Location)` → no-op if row missing; logs at Verbose.
- Insert `PlaySound` calls at every event: weapon fire, empty click, pickup, enemy alert/pain/death/attack, door open/close, button press, secret, footstep, jump, land, player pain/death.
- `UGameUserSettings` subclass with mouse sensitivity + master volume.
- Settings widget reads/writes.
- **Migration:** the pre-Phase-14 `AQuakeEnemyBase::PainSound`/`DeathSound` direct `PlaySoundAtLocation` calls migrate to `ESoundEvent::EnemyPain`/`EnemyDeath` rows.

**Tests:**
- [ ] Functional: fire each weapon → expected `ESoundEvent` logged.
- [ ] Functional: door open → `door_open` logged.
- [ ] Functional: pickup → `pickup_item` logged.
- [ ] Functional: set sensitivity 2.0, save, reload, restored.

**Manual:**
- [ ] Log level Verbose for `LogQuakeSound`. Run a level → every action logs one `PlaySound`.
- [ ] Change sensitivity in settings → camera responds immediately.
- [ ] Volume slider saves (no audible effect yet).

**Exit.** Every gameplay event reaches sound manager; no direct `PlaySoundAtLocation` calls remain.

---

## Phase 15 — Content Authoring

**Goal.** E1M1, E1M2, E1M3, Hub are shippable.

**Implements:**
- `Content/Maps/Hub.umap` — Episode 1 portal + Quit. No art beyond primitives.
- `Content/Maps/E1/E1M1.umap` — tutorial, ~5 rooms, Grunts only, 1 silver key + lock, 1 secret, 1 button, lava pit, exit. 5–10 min.
- `Content/Maps/E1/E1M2.umap` — full map with all 3 enemies, gold key, multiple secrets, deferred spawns, lava. 10–15 min.
- `Content/Maps/E1/E1M3.umap` — boss arena; `AQuakeEnemy_GreaterOgre` (800 HP Ogre subclass + faster grenades). Win on boss death.
- DESIGN 10.5 per-level checklist followed for each map.

**Tests:**
- [ ] Functional per level: pathfind from `PlayerStart` to `ExitTrigger` — path exists. Catches "level the AI can't navigate."
- [ ] Functional per level: `KillsTotal > 0`, at least one `IQuakeActivatable` exit exists.
- [ ] Functional: MainMenu → Hub → E1M1 → E1M2 → E1M3 → win. Drive via simulated input + direct level-clear scripting.
- [ ] Functional: per-level checklist items present (`PlayerStart`, `NavMeshBoundsVolume`, `PostProcessVolume`, lights, exit).

**Manual:** the [Definition of Done](#v1-definition-of-done) playthrough.

**Exit.** All four maps shippable; Definition of Done passes end-to-end.

---

## v1 Definition of Done

Player can:

1. Launch, see main menu, choose difficulty, start new game.
2. Spawn in Hub, walk into Episode 1 portal, load E1M1.
3. Fight Grunts/Knights/Ogres with v1 weapons and full strafe-jump.
4. Pick up keys, open locked doors, find secrets, take damage, take splash, rocket-jump.
5. Die, restart from level entry with snapshot inventory.
6. Quick save + quick load successfully.
7. Complete E1M1 → E1M2 → E1M3, defeat Greater Ogre, see win screen.
8. Return to main menu and start again on a different difficulty.

All Phase 0–15 automated tests pass with zero regressions.
