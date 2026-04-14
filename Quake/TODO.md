# TODO — v2 and beyond

Captured from [HUD.md](HUD.md) "v1 vs v2" and inline `// later phase` /
`// future` / `// v2` markers in the codebase. v1 is feature-complete in
code; this file is the next-milestone backlog.

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

- **Multiplayer** — would require `FSavedMove_Character` / `FNetworkPredictionData_Client_Character` overrides on the CMC for client-side prediction of strafe-jumping. CLAUDE.md "Risk Note" calls this out as the place "I added Quake movement to UE" projects famously break.
- **Mod / map workshop** — Steam Workshop or similar.
- **Speedrun timer + leaderboards** — `AQuakePlayerState::GetTimeElapsed` is the data source; needs a server.
