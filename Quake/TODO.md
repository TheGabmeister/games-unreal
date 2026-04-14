# TODO — v1.5, v2, and beyond

Captured from [HUD.md](HUD.md) "v1 vs v2" and inline `// later phase` /
`// future` / `// v2` markers in the codebase. v1 is feature-complete in
code; this file is the next-milestone backlog.

---

## v1.5 — Multiplayer-Readiness Refactor

**Goal:** restructure the project so dropping in MP later is a focused, contained engineering effort instead of a rewrite. The game stays single-player at the end of this phase. No replication actually fires (we stay `NM_Standalone`), but every code path that *would* need to be authority-gated, replicated, or RPC-routed in MP is structured correctly. Each step is independently verifiable in SP — if `Quake.*` automation tests stay green and feel-test on `PhysSandbox.umap` is unchanged, the step is safe.

**Why now:** the inventory-component refactor already established the pattern (component on the pawn, mailbox on GameInstance). Capitalize on that momentum before more SP-only state accumulates.

**Out of scope for v1.5:** actually testing MP gameplay (no listen server, no PIE multi-client). Sound multicast routing — needs MP runtime to validate. UI for joining games. Anti-cheat. Lag compensation.

### A. Authority-aware mutation contracts

The contract: *gameplay-state mutation only happens on the authority*. In SP that's always true so the guards are no-ops, but they document the boundary and crash loudly if MP ever calls them from the wrong side.

- **A1. Add `HasAuthority()` guards at mutation entry points.** Top of every override that changes shared state: `AQuakeCharacter::TakeDamage`, `AQuakeEnemyBase::TakeDamage`, `AQuakePickupBase::OnPickupBeginOverlap` consumption branch, `AQuakeDoor::Activate`, `AQuakeButton::Fire`, `AQuakeTrigger::Activate` (and subclasses), `AQuakeHazardVolume::ApplyTickDamage`, `AQuakeEnemySpawnPoint::TrySpawn`. Pattern: `if (!HasAuthority()) return ...;` at top, with a `Server_*` RPC stub forwarding to the same body. Stubs can be `unimplemented()` until MP lands — what matters is the structural commitment.
- **A2. Route input → mutation through `Server_*` RPCs.** `AQuakeCharacter::OnFirePressed`, `OnFireReleased`, `OnWeaponSlotPressed`, `OnQuickSavePressed`, `OnQuickLoadPressed`. Local handler validates locally-needed UI feedback (empty-click sound is fine local), then calls a `Server_*` UFUNCTION marked `Server, Reliable, WithValidation`. Body lives in the Server function. SP path: `Server_*` calls execute immediately because we're authority.
- **A3. Disable F5/F9 outside `NM_Standalone`.** `UQuakeGameInstance::SaveCurrentState` / `LoadFromSlot` early-return with `LogQuakeSave` warning if `GetWorld()->GetNetMode() != NM_Standalone`. Quicksave is fundamentally SP-only; this prevents future MP code from accidentally invoking it.

### B. Replication-ready state markers

Add `UPROPERTY(Replicated)` (or `ReplicatedUsing=OnRep_*` where the client needs a callback) + `GetLifetimeReplicatedProps` to every gameplay-visible field. In SP these markers are inert — the actor never replicates because `bReplicates=false`. In MP, flipping `bReplicates=true` on the relevant actor classes is the second drop-in step.

- **B1. `UQuakeInventoryComponent`**: `Armor`, `ArmorAbsorption`, `AmmoCounts`, `OwnedWeaponClasses`. Add `SetIsReplicatedByDefault(true)` in the constructor. CLAUDE.md already calls this out as the planned MP follow-up — finish the wiring.
- **B2. `AQuakeCharacter`**: `Health`, `CurrentWeapon`, `CurrentWeaponSlot`, `bIsDead`, `bIsInPain`. Use `OnRep_Health` to drive HUD damage flash so the indicator survives in MP without an RPC.
- **B3. `AQuakePlayerState`**: `ActivePowerups`, `Keys`, `Kills`, `Secrets`, `Deaths`. PlayerState is already replicated by UE — these properties just need the marker.
- **B4. `AQuakeDoor`**: `State` (enum), `CurrentRelativeLocation`. Mid-animation interpolation is OK to be best-effort on clients; `OnRep_State` snaps to endpoints.
- **B5. `AQuakeButton`**: `bFired`, `LastFireTime`. Idempotency guards prevent double-fire on replication.
- **B6. `AQuakeEnemySpawnPoint`**: `bHasFired`, `SpawnedEnemy`. Already saveable; add the replication marker so a late-joining client sees the correct world state.
- **B7. `AQuakeEnemyBase`**: `Health`, `bIsDead`. AI controller stays server-only (correct architecture per the audit).
- **B8. Set `bReplicates = true` explicitly** in constructors of `AQuakeCharacter`, `AQuakeEnemyBase`, `AQuakeProjectile`, `AQuakePickupBase`, `AQuakeDoor`, `AQuakeButton`. Default-on for `ACharacter` already, but explicit removes ambiguity.

### C. Remove single-player singleton assumptions

Code that hardcodes "the player" breaks the moment a second one exists.

- **C1. `AQuakeEnemyAIController.cpp:362`** — grudge-revert uses `GetFirstPlayerController()`. Change to: revert to last-known player target tracked per-controller, or null-out the target and fall back to the regular perception sweep on next tick.
- **C2. `AQuakeDoor.cpp:60`** — locked-key HUD message uses `GetPlayerController(this, 0)`. Change to: route the message to the activator's controller (`Cast<APlayerController>(InInstigator->GetInstigatorController())`).
- **C3. `UQuakeGameInstance::SaveCurrentState`** — captures inventory via `GetPlayerController(World, 0)`. Refactor to take an explicit `APlayerController*` parameter passed by the caller (`OnQuickSavePressed` knows who pressed F5).
- **C4. Audit-grep for remaining offenders:** `GetFirstPlayerController`, `GetPlayerController(.*, 0)`, `GetPlayerCharacter(.*, 0)`, `GetPlayerPawn(.*, 0)`. Each survivor needs a context-aware replacement or a comment justifying SP-only intent.

### D. CMC client-side prediction

The single highest-risk piece — `UQuakeCharacterMovementComponent::CalcAirVelocity` is deterministic but UE's prediction system needs the saved-move plumbing or every air-strafe correction snaps the player.

- **D1. Implement `FSavedMove_Quake : FSavedMove_Character`** with `Clear` / `SetMoveFor` / `PrepMoveFor` overrides capturing whatever state `CalcAirVelocity` reads. Today that's just `Velocity` + `Acceleration` (already in the base saved move) + the bunny-hop window timer (`LastLandedWorldTime`). Confirm by grep.
- **D2. Implement `FNetworkPredictionData_Client_Quake : FNetworkPredictionData_Client_Character`** that allocates `FSavedMove_Quake`. Override `UQuakeCharacterMovementComponent::GetPredictionData_Client` to return it.
- **D3. SP regression test.** `Quake.Movement.AirAccel.HighSpeedStrafe` already locks the dot-product clamp; add `Quake.Movement.Prediction.SavedMoveRoundTrip` that constructs an `FSavedMove_Quake`, calls `SetMoveFor` then `PrepMoveFor` on a fresh CMC, and asserts `Velocity` / bunny-hop window survive the round trip. Pure-static, world-free.
- **D4. Document the contract** in [Source/Quake/QuakeCharacterMovementComponent.h](Source/Quake/QuakeCharacterMovementComponent.h): "any new field read by `CalcAirVelocity` MUST be added to `FSavedMove_Quake::SetMoveFor` and `PrepMoveFor` or MP prediction will mispredict." Risk note in CLAUDE.md updates accordingly.

### E. Architecture tests + docs

Lock the contracts in so future code doesn't drift back.

- **E1. `Quake.Architecture.NoSingletonPlayerLookups`** — automation test that scans `Source/Quake/**/*.cpp` (excluding `Tests/`) for `GetFirstPlayerController` / `GetPlayerController(*, 0)` / `GetPlayerCharacter(*, 0)`. Fails the build if any match. Allow-list specific files by exact match if a true SP-only path is justified.
- **E2. `Quake.Architecture.MutationGuards`** — manual checklist (or a reflection-based scan if cheap) verifying every `IQuakeActivatable::Activate` and `TakeDamage` override starts with an authority guard. Cheaper alternative: add a `QUAKE_REQUIRE_AUTHORITY()` macro that the linter can grep for.
- **E3. Update [CLAUDE.md](CLAUDE.md) and [DESIGN.md](DESIGN.md)** "Risk Note: Strafe-Jumping CMC" + "Architecture: Inventory Component" + §9.4 to describe the v1.5 state — replication markers are present, RPC scaffolding is wired, prediction works, but `bReplicates` flips and listen-server bring-up are still v2.
- **E4. Sweep the codebase for `// MP:` / `// future` / `// v2` markers** added during v1.5 and consolidate any remaining ones into this TODO.

### Acceptance criteria

- All `Quake.*` automation tests pass.
- `PhysSandbox.umap` feel-test: strafe-jumping, bunny-hopping, rocket-jumping unchanged at the joystick.
- Grep clean: zero `GetFirstPlayerController` / `PlayerIndex 0` matches outside SP-only allow-list.
- Every `TakeDamage` override + `IQuakeActivatable::Activate` starts with an authority guard.
- CMC saved-move round-trip test passes.
- CLAUDE.md / DESIGN.md sections updated.

---

## Content

- **Episodes 2 / 3 / 4** — additional `Content/Maps/E{2,3,4}/` map sets following [DESIGN.md](DESIGN.md) §10.5 per-level checklist.
- **More E1 maps** — secret level + additional regular maps between E1M1 and E1M3.
- **True bosses** — distinct `AQuakeEnemy_*` boss class per episode (E1 Greater Ogre is the v1 stand-in).
- **Audio assets** — the Phase 14 manager + `DT_SoundEvents` are wired and waiting for `USoundBase` assignments. See [Source/Quake/QuakeSoundEvent.h](Source/Quake/QuakeSoundEvent.h) for the catalog.
- **Music tracks** — `UQuakeSoundManager::PlayMusic` / `StopMusic` are stubs that log Verbose. Wire to a `UAudioComponent` with crossfade.

## Weapons

- **Super Shotgun (slot 3)** — second-barrel variant of `AQuakeWeapon_Shotgun`. Ammo `Shells`, 2/shot.
- **Super Nailgun (slot 5)** — high-RoF variant of `AQuakeWeapon_Nailgun`. Ammo `Nails`, 2/shot.
- **Grenade Launcher (slot 6)** — player-fired grenade weapon reusing `AQuakeProjectile_Grenade`.
- **Thunderbolt (slot 8)** — beam weapon, ammo `Cells`. SPEC 2.0 row exists; `EQuakeSoundEvent::WeaponThunderboltHum` already in the catalog.
- **Cells ammo + pickups** — `EQuakeAmmoType::Cells` is in the enum but no pickup BP exists yet.
- **Auto-switch table** — extend `AQuakeCharacter::PickAutoSwitchWeaponSlot` priority list once SSG / SNG / GL exist (currently `RL → NG → SG → Axe`; SPEC 2.2 wants `RL → SNG → SSG → NG → SG → Axe`).
- **Weapon-switch animation** — SPEC 2.2's 0.2 s lower + 0.2 s raise. See [Source/Quake/QuakeCharacter.h:76](Source/Quake/QuakeCharacter.h#L76).

## Enemies

- **Fiend** — leaping melee enemy. Per-type AIController for the leap state.
- **Shambler** — high-HP heavy with ranged lightning attack. Add `EQuakeSoundEvent::EnemyPain_Shambler` row if a unique asset is needed (per CLAUDE.md migration note).
- **Zombie** — Down-state revive timer. `FQuakeDifficultyMultipliers::ZombieReviveScale` exists but unused; see [Source/Quake/QuakeDifficultyMultipliers.h:37](Source/Quake/QuakeDifficultyMultipliers.h#L37) — needs an `AQuakeAIController_Zombie::ApplyDifficultyScaling` override to consume it.
- **Flying enemies** — DESIGN explicitly defers (`Wizard`, `Scrag`). Will require pathfinding-on-flight-volume work.
- **Enforcer** (future v2) — referenced in [Source/Quake/QuakeWeapon_Shotgun.cpp:61](Source/Quake/QuakeWeapon_Shotgun.cpp#L61) as the example of an enemy reading the firer's `GetOutgoingDamageScale()`.
- **Infighting 5-second chain credit rule** — currently deferred; infighting kills credit nothing. See [Source/Quake/QuakeEnemyBase.cpp:212](Source/Quake/QuakeEnemyBase.cpp#L212).

## Powerups

- **Pentagram of Protection** — invulnerability. Red HUD tint per [HUD.md:77](HUD.md#L77).
- **Ring of Shadows** — invisibility / detection radius reduction. Yellow transparency tint.
- **Biosuit** — hazard immunity (lava + future slime/drown). Green tint.
- **HUD active-powerup tints** — three additions to `SQuakeHUDOverlay`'s post-process pipeline.

## Hazards + World

- **Slime hazard** — `AQuakeHazardVolume` already supports the response matrix; need a `BP_HazardVolume_Slime` + `UQuakeDamageType_Slime` (already declared in DESIGN class hierarchy).
- **Water + swimming** — `AQuakeWaterVolume : APhysicsVolume` shell exists in DESIGN. Need swim CMC mode + buoyancy.
- **Drowning** — air-supply timer + `UQuakeDamageType_Drown`.
- **Gibs channel** — referenced in [Source/Quake/QuakeProjectile.cpp:29](Source/Quake/QuakeProjectile.cpp#L29). Adds a separate collision channel for scattered limb props.

## Polish (cosmetic, can ship without)

- **Damage flash post-process** — `AQuakeCharacter::TriggerDamageFlash` is a stub. Wire to a `UMaterialInstanceDynamic` parameter on the `DamageFlashPostProcess` component. See [Source/Quake/QuakeCharacter.h:275](Source/Quake/QuakeCharacter.h#L275).
- **Real enemy death animations** — collapse / tip-over instead of the current `StopMovementImmediately` + Hide. See [Source/Quake/QuakeEnemyBase.cpp:193](Source/Quake/QuakeEnemyBase.cpp#L193).
- **Real gib reaction** — scatter primitives, blood decal, fade-out. See [Source/Quake/QuakeEnemyBase.cpp:312](Source/Quake/QuakeEnemyBase.cpp#L312).
- **Megahealth decay** — overcharge HP currently sits at 200; SPEC says decay back to 100 at 1 HP/s. See [Source/Quake/QuakePickup_Health.h:32](Source/Quake/QuakePickup_Health.h#L32).
- **Crosshair** — flagged as v2 polish in [HUD.md:47](HUD.md#L47).
- **Per-enemy pain-sound dedupe** — currently the `TakeDamage` non-fatal path emits `EnemyPain` even if the same enemy is mid-flinch. Add a per-pawn cooldown.

## UX

- **Save menu UI + multiple slots** — v1 has only quick / auto. ROADMAP "Content Scope" defers a slot picker. New widget that lists `UGameplayStatics::DoesSaveGameExist` slots.
- **Pause menu** — currently only the main menu has Settings. Pause + Settings + Quit-to-Menu overlay during gameplay.
- **Per-binding rebind UI** — exposes `UInputMappingContext` to the player (Enhanced Input "remap" path).
- **Master volume audio routing** — `UQuakeGameUserSettings::MasterVolume` is stored but not applied. Needs a `USoundMix` + `USoundClass` hooked at `Initialize`.
- **Localization** — `NSLOCTEXT` is already used everywhere; missing piece is .pak language sets. Out of v1 scope per ROADMAP.

## Engineering

- **Functional / world-spinup tests** — convert "deferred to manual" checks (PIE-walkthrough verifications) into automation tests once the v1 maps exist. Especially per-map Phase 15 coverage: load `.umap` → assert PlayerStart, NavMeshBoundsVolume, exit, pathfind.
- **Per-map automation tests** for E1M1 / E1M2 / E1M3 / Hub once the maps exist (see Phase 15 checklist).
- **Gamepad input** — Enhanced Input mapping context for controller. Out of v1 scope.

## Stretch (would change project shape — discuss before starting)

- **Multiplayer (full bring-up)** — assumes v1.5 above is complete (replication markers in place, RPC scaffolding wired, CMC prediction working). Remaining v2 work: flip `bReplicates = true` on the relevant actor classes, stand up a listen-server smoke test in PIE, multicast routing for non-local sounds via `UQuakeSoundManager`, lag-compensation pass on hitscan weapons, anti-cheat / server validation pass on the `Server_*` RPC bodies, MP-aware respawn (skip `TransitSnapshot` / `LevelEntrySnapshot`), join-in-progress UX.
- **Mod / map workshop** — Steam Workshop or similar.
- **Speedrun timer + leaderboards** — `AQuakePlayerState::GetTimeElapsed` is the data source; needs a server.
