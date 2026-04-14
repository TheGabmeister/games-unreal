# AGENTS.md

This file gives repo-specific guidance to coding agents working in this project.

## Project Summary

- Unreal Engine 5.7 single-player FPS inspired by Quake, implemented in C++.
- `DESIGN.md` is the durable gameplay/design source of truth. `HUD.md` covers HUD layout/data flow, `CLAUDE.md` captures architecture/tooling notes, and `TODO.md` tracks v2+ backlog and remaining content/editor work.
- Read `DESIGN.md` before making non-trivial gameplay changes. Read `CLAUDE.md` before touching movement, AI, save/load, menu/settings, or other high-churn architecture.
- v1 gameplay code is feature-complete. Most remaining work is content/editor-side authoring plus selective polish. Do not quietly pull `TODO.md` backlog items into the current task unless the user asks for them.
- Historical `SPEC`, `ROADMAP`, and `Phase N` references still appear in comments/tests. Treat them as legacy labels that usually map back to `DESIGN.md` sections and nearby code, not as root docs you should expect to find.
- Module name: `Quake`.
- Current code lives under `Source/Quake/`.

## Core Rules

- Keep gameplay logic in C++.
- Use Blueprints only as thin asset/tuning layers when needed.
- Do not add Blueprint event graph gameplay logic.
- Keep the project single-player only. Do not introduce replication or networking systems unless explicitly requested.
- Preserve the primitive-shape visual direction unless the user asks to expand scope.

## Repo Map

- `Source/Quake/` - gameplay module source.
- `Config/` - version-controlled Unreal project settings.
- `Content/` - editor-authored assets and maps.
- `DESIGN.md` - durable gameplay/design reference and system rules.
- `HUD.md` - HUD wireframe, data sources, and HUD-specific behavior.
- `CLAUDE.md` - additional architecture guidance and implementation notes.
- `TODO.md` - v2+ backlog plus remaining editor/content work.

## Existing Gameplay Architecture

- `AQuakeCharacter` owns first-person movement, live health, and active weapon instances. Ammo and owned-weapon persistence route through `UQuakeGameInstance`; key checks and powerup-derived combat queries route through `AQuakePlayerState`.
- `AQuakePlayerController` owns Enhanced Input setup.
- `AQuakeGameMode` sets core game classes and rules.
- `AQuakeHUD` owns the pure-Slate HUD and transient message / level-end overlays. Keep HUD behavior in C++/Slate, not UMG.
- `AQuakePlayerState` owns current-level stats plus per-level state such as keys and active powerups. `GivePowerup` refreshes additively with a 60 s cap, and `ClearPerLifeState()` clears only per-life state while preserving attempt stats.
- `UQuakeGameInstance` owns persistent inventory, profile-level data, and cross-level state.
- `UQuakeSoundManager` is the gameplay audio entry point and lives as a `UGameInstanceSubsystem`.
- `UQuakeGameUserSettings` persists mouse sensitivity and master-volume settings via `GameUserSettings.ini`.
- `AQuakeCharacter::GiveWeaponPickup` is the first-pickup path for weapons: first pickup grants the weapon and auto-switches, repeat pickups grant only ammo.
- New shared input actions should be added in `AQuakePlayerController` rather than created as editor-only input assets.
- New input wiring is a 4-part change: add the `UPROPERTY(EditDefaultsOnly)` slot on `AQuakePlayerController`, create the `IA_*` asset, map it in `IMC_Default`, and assign it in `BP_QuakePlayerController`. Do not synthesize IA/IMC objects at runtime as a shortcut.

## Doc Alignment Notes

- Treat `DESIGN.md` as the gameplay/design source of truth. When `DESIGN.md`, `AGENTS.md`, `HUD.md`, and `CLAUDE.md` disagree on gameplay or state ownership, follow `DESIGN.md` and then update the secondary docs to match.
- Treat `HUD.md` as the HUD layout/source-of-data reference. When HUD presentation, placement, or always-visible data rules are in question, follow `HUD.md`.
- Treat `TODO.md` as backlog, not as automatic scope. Items there are opt-in future work.
- Historical `SPEC` / `ROADMAP` references in code comments and tests should usually be resolved back to `DESIGN.md` section numbers or the local implementation/test they sit next to.
- Do not assume `AQuakePlayerState` is recreated on death. UE keeps `PlayerController` and `PlayerState` across pawn respawn, so Quake's death-restart flow must explicitly call `AQuakePlayerState::ClearPerLifeState()` to clear keys and active powerups while preserving cumulative level-attempt stats.
- Keep live health on `AQuakeCharacter` or a shared health component tied to the pawn. Inventory lives on `UQuakeGameInstance`, but save/load and level-entry restore must serialize and restore health explicitly.
- `AQuakeCharacter` input handlers intentionally early-return when `bAwaitingRestart` so the death screen can consume Fire for restart flow. Preserve that gate when touching player input or death handling.
- Key state is shared across door, pickup, HUD, and player-state code via `EQuakeKeyColor` in `QuakeKeyColor.h`. Reuse the shared enum instead of re-declaring key colors on individual gameplay classes.
- Pickups depend on collision setup in C++, not just BP placement: `AQuakePickupBase` overlaps the custom `Pickup` channel, and the player capsule must explicitly overlap `QuakeCollision::ECC_Pickup` or shell/health/powerup pickups will never fire `OnPickupBeginOverlap`.
- Pickup rules are split by subclass, not ad hoc branches in placed Blueprints: `AQuakePickup_Key` rejects duplicate keys so the actor stays in-world, `AQuakePickup_Powerup` is always consumed and leaves refresh-cap logic to `AQuakePlayerState`, `AQuakePickup_Armor` owns the tier/value lookup and replacement rule, and `AQuakePickup_Weapon` grants ammo first then delegates first-time weapon ownership to `AQuakeCharacter::GiveWeaponPickup()`.
- Counted enemies are authored through `AQuakeEnemySpawnPoint`, not by dragging `BP_Enemy_*` actors directly into a map. Direct enemy placements are decoration/scripted display unless the user explicitly changes that rule.
- Stats and level-clear logic must follow spawn points, difficulty gating, and spawn-point satisfaction state. Do not build systems that only count already-spawned `AQuakeEnemyBase` actors.
- Save/load identity for level-placed actors uses stable actor identity via `GetFName()`/`FActorSaveRecord::ActorName`, not ad hoc Actor Tags or string registries.
- `IQuakeActivatable` is a pure C++ interface method: implement `Activate(AActor* Instigator)`. Do not use `Activate_Implementation` unless the interface is explicitly changed to `BlueprintNativeEvent`.
- `UQuakeSoundManager` is a `UGameInstanceSubsystem`. Keep sound-table ownership aligned with the GameInstance/subsystem lifecycle rather than hanging audio configuration off unrelated gameplay actors.
- `UQuakeGameUserSettings` only works when `Config/DefaultEngine.ini` keeps `GameUserSettingsClassName=/Script/Quake.QuakeGameUserSettings`. Do not remove or bypass that wiring when touching menu/settings code.
- `UQuakeProjectSettings` is the project-level home for balance/audio DataTable references. Prefer extending project settings over scattering new global tuning assets across unrelated classes.

## Damage Guidance

- Follow Unreal's built-in damage pipeline: attackers call `UGameplayStatics::ApplyPointDamage`, `ApplyRadialDamage`, or `ApplyDamage`; receivers own the final response in `TakeDamage`.
- Use `UDamageType` subclasses as immutable metadata holders for semantic damage categories such as bullet, explosive, lightning, or world hazards.
- Keep `UDamageType` subclasses lightweight. They should expose defaults and flags, not store runtime state or become a deep per-weapon class tree.
- Do not branch on damage type leaf class identity when a shared base such as `UQuakeDamageType` can expose the needed fields uniformly.
- Avoid duplicating health/armor resolution logic in both `AQuakeCharacter` and enemy classes. Prefer a shared resolver or `UQuakeHealthComponent` that handles armor absorption, self-damage scaling, knockback inputs, death checks, and related bookkeeping.
- Actor-specific `TakeDamage` overrides should stay thin: gather context from `FDamageEvent`, hand the calculation to shared code, then perform actor-specific reactions such as HUD feedback, AI aggro, pain reactions, or death presentation.
- Not every health change belongs in the damage pipeline. Timed effects like megahealth decay should use explicit health-management code rather than being faked as combat damage.
- Megahealth overcharge/ceiling rules live with the Character/health system, but the time-based decay is separate follow-up behavior. When implementing decay, add explicit health-management logic rather than routing it through `TakeDamage`.

## Build And Tooling

Build the editor target with:

```powershell
& "C:/Program Files/Epic Games/UE_5.7/Engine/Build/BatchFiles/Build.bat" QuakeEditor Win64 Development -Project="c:/dev/games-unreal/Quake/Quake.uproject" -WaitMutex -FromMsBuild
```

If the build fails with `Unable to build while Live Coding is active`, stop and ask the user to close Unreal Editor or press `Ctrl+Alt+F11`. Do not try to work around Live Coding from the CLI.

When adding new `.h` or `.cpp` files, regenerate project files so IntelliSense and compile commands stay current. Use Unreal's bundled .NET SDK, not the system `dotnet`.

```powershell
$env:UE_DOTNET_DIR = "C:/Program Files/Epic Games/UE_5.7/Engine/Binaries/ThirdParty/DotNet/8.0.412/win-x64"
$env:DOTNET_ROOT = $env:UE_DOTNET_DIR
$env:PATH = "$env:UE_DOTNET_DIR;$env:PATH"
$env:DOTNET_MULTILEVEL_LOOKUP = "0"
$env:DOTNET_ROLL_FORWARD = "LatestMajor"
& "$env:UE_DOTNET_DIR/dotnet.exe" "C:/Program Files/Epic Games/UE_5.7/Engine/Binaries/DotNET/UnrealBuildTool/UnrealBuildTool.dll" -projectfiles -project="C:/dev/games-unreal/Quake/Quake.uproject" -game -engine -VSCode
```

## Tests

- Automation tests live under `Source/Quake/Tests/` and use Unreal's automation framework.
- Prefer unit tests for pure gameplay logic and pure static helpers over world-spinup tests when possible.
- Run automation tests from the editor via Session Frontend -> Automation and filter on `Quake.*`.
- Keep `Source/Quake/Quake.Build.cs` configured so tests under `Source/Quake/Tests/` can include module headers. Do not remove include-path support without replacing it with an equivalent solution.
- When a subsystem already has focused automation coverage, run the closest matching `Quake.*` test(s) after changing it. For map/content/editor-only work, call out manual verification if automation is not practical yet.

## Unreal-Specific Guidance

- Prefer C++ base classes plus editor-assigned defaults over gameplay implemented in Blueprints.
- Treat VS Code IntelliSense errors as secondary to a real Unreal build. Stale compile commands can produce false squiggles.
- Keep project settings changes in `Config/Default*.ini` under version control.
- Avoid editing generated directories such as `Binaries/`, `DerivedDataCache/`, `Intermediate/`, or `Saved/` unless the task explicitly requires it.
- Keep the custom collision channels (`Pickup`, `Projectile`, `Corpse`, and the `Weapon` trace channel) in sync with `DESIGN.md` section 1.6 when touching collision.
- Reference custom channels through `QuakeCollision::ECC_*` in `QuakeCollisionChannels.h`; do not scatter raw `ECC_GameTraceChannelN` literals through gameplay code.
- Enemy navigation settings must stay aligned with player traversal assumptions. In particular, NavMesh step height should match the movement spec rather than drifting lower than the player's step-up capability.

## Implementation Preferences

- Favor data-driven tuning for weapons, enemies, pickups, and other balance values even when behavior stays in C++.
- Keep HUD work in C++/Slate unless the user explicitly changes that direction.
- Keep the HUD primarily pull-based for always-visible state and reserve event-driven logic for transient messages or feedback effects. Use `HUD.md` when changing element placement, visibility rules, or which owner provides a HUD datum.
- When adding new gameplay systems, match the naming/style of the existing `AQuake*` and `UQuake*` classes.
- Use `AQuakeCharacter::NumWeaponSlots` instead of hardcoding `8` for slot-count logic.
- For abstract `UCLASS` gameplay bases, use Unreal's `PURE_VIRTUAL(...)` macro rather than C++ `= 0` so the class default object remains constructible.
- For non-trivial gameplay changes, update the right design doc when the implementation changes the contract: `DESIGN.md` for gameplay/system rules, `HUD.md` for HUD layout/data-flow changes, `CLAUDE.md` for architecture/tooling guidance, and `TODO.md` when you intentionally add or retire backlog work.
- If touching movement, AI, or save/load architecture, read the matching section in `CLAUDE.md` before editing; those are the highest-churn areas.

## Verification

- After meaningful C++ changes, run a real `Build.bat` compile when feasible.
- When a task touches an existing tested subsystem, run the closest relevant automation test(s) under `Source/Quake/Tests/` and note any manual map verification that still remains.
- If you could not build or test, say so clearly in your final handoff.
