# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Unreal Engine 5.7 single-player FPS recreating original Quake gameplay with primitive shapes. The full design is in [SPEC.md](SPEC.md) — read it before making non-trivial gameplay changes. Module name: `Quake`. The current scope is the v1 milestone defined in `SPEC.md` section 11.

**SPEC.md is the design source of truth.** When the SPEC and this file disagree about gameplay or data ownership, the SPEC wins — update CLAUDE.md to match. CLAUDE.md is a working-notes file for build/tooling/conventions, not a design doc.

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

A `PostToolUse` hook in `.claude/settings.local.json` runs this automatically after `Write` of any `Source/**.{h,cpp}` file. Manually generated files (created in the IDE) will not trigger it.

The system installs only .NET 10; UnrealBuildTool requires .NET 8. The bundled SDK at the path above is the working version — never invoke a system `dotnet` for UBT.

## IDE Integration Notes

IntelliSense errors like `cannot open source file "InputModifiers.h"` are usually false positives from VS Code's C++ extension when `compileCommands_Quake.json` is stale. The compile-test of truth is a real `Build.bat` run, not the IDE squiggles. Regenerate project files (above) before assuming an include is wrong.

## Running Tests

Automation tests live under `Source/Quake/Tests/` and use `IMPLEMENT_SIMPLE_AUTOMATION_TEST` with `EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter`, guarded by `#if WITH_DEV_AUTOMATION_TESTS`. Run them from the editor via **Session Frontend → Automation tab → filter `Quake.*`**. Tests are grouped by subsystem:

- `Quake.Foundation.*` — smoke test proving the runner is wired up.
- `Quake.Movement.AirAccel.*` — dot-product clamp regression suite for the CMC.
- `Quake.Damage.ArmorAbsorption.*` — armor formula regression suite (Green / Yellow / Underflow / NoArmor).
- `Quake.Damage.DamageType.*` — shared-base CDO cast pattern from SPEC section 1.5.

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

- **`UQuakeGameInstance`** — inventory (weapons owned, ammo, armor), level-entry snapshot, save-game reference, player profile, difficulty. Survives `OpenLevel` and Character respawn. **Note:** keys are *not* here despite being colloquially "inventory" — see PlayerState below.
- **`AQuakePlayerState`** — current-level stats (kills, secrets, deaths, time), active powerups (`TArray<FQuakeActivePowerup>`), and keys held. **Auto-cleared on `OpenLevel` only** — UE preserves PlayerState across pawn death/respawn, so the death-restart path (SPEC section 6.4) calls `ClearPerLifeState()` explicitly to empty powerups and keys. Cumulative stats (kills, secrets, deaths, time) persist across the level attempt. Don't assume "PlayerState resets on death" without that call. Keys live here because their lifecycle matches powerups, not weapons/ammo/armor.
- **`AQuakeCharacter`** — live health, currently-equipped weapon actor. Tied to the body; destroyed on death and respawn.
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

`AQuakeWeaponBase` is the `UCLASS(Abstract)` base for every weapon. The public entry point is `TryFire(InInstigator)`, which enforces `RateOfFire` cooldown via a `LastFireWorldTime` timestamp and then calls the subclass-implemented `Fire`. Subclasses only override `Fire`; cooldown tracking lives on the base so no subclass re-implements it. Concrete weapons ([AQuakeWeapon_Axe](Source/Quake/QuakeWeapon_Axe.h) and onward) set all stats (`Damage`, `Range`, `RateOfFire`, …) as `UPROPERTY` defaults in their C++ constructor per the "C++ first" rule — the thin `BP_Weapon_*` subclass only fills in asset slots.

**`Fire` is declared with `PURE_VIRTUAL`, not C++ `= 0`.** UE's reflection system constructs a Class Default Object for every `UCLASS`, including `UCLASS(Abstract)` ones, and a C++-pure-virtual method (`virtual void Foo() = 0;`) makes the CDO non-instantiable with `error C2259: cannot instantiate abstract class`. The fix is the engine's `PURE_VIRTUAL(AQuakeWeaponBase::Fire, )` macro, which emits a crashing stub body that satisfies the CDO constructor while still firing at runtime if anything calls the base directly. **Any future abstract `UCLASS` virtual needs the same treatment** — this bit us once during Phase 2 and will bite again.

`SpawnActor` for a weapon uses the `(UClass*, FActorSpawnParameters)` overload followed by `AttachToComponent`, not the `(UClass*, FTransform, FActorSpawnParameters)` overload. On UE 5.7 the latter fails template argument deduction when the first argument is a `TSubclassOf<T>` (the compiler can't decide between the `FTransform` and `(FVector, FRotator)` overloads), so spawn at origin and let the attach set the transform. See [QuakeCharacter.cpp SpawnAndEquipDefaultWeapon](Source/Quake/QuakeCharacter.cpp) for the canonical pattern.

## Architecture: AI Split

AI is split between the body and the brain per standard UE convention:

- **`AQuakeEnemyBase : public ACharacter`** (the pawn / body) — capsule, mesh primitives, movement, health, `TakeDamage` override. Exposes action methods like `MoveToTarget`, `FireAtTarget`, `PlayPainReaction`, `PlayDeathReaction`. **No decision-making code lives on the pawn.**
- **`AQuakeEnemyAIController : public AAIController`** (the brain) — state machine (`Idle → Alert → Chase → Attack → Pain → Dead`), target tracking, owns a `UAIPerceptionComponent` with sight (`UAISenseConfig_Sight`) and hearing (`UAISenseConfig_Hearing`, with `bUseLoSHearing = false` for Quake-style hearing through walls). Calls the pawn's action methods to drive behavior.

Per-enemy behavior variations (Fiend leap, Ogre grenade arc, Zombie revive) live on per-enemy AIController subclasses, not on pawn subclasses. This is what makes the AI debugger (`'` key in PIE) work and keeps "what I am" separate from "what I'm doing."

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
- **Enemy placements** — always `AQuakeEnemySpawnPoint` actors with `EnemyClass` set to a `BP_Enemy_*` class. Direct `BP_Enemy_*` placements in a level are decoration and **do not count** toward `KillsTotal` or gate the level-clear scan. This is the canonical authoring path for counted enemies; see SPEC section 5.1.
- **One master material** `M_QuakeBase` with `BaseColor` / `Emissive` / `Metallic` / `Roughness` parameters; everything else is a `MaterialInstance` of it. Runtime tinting (damage flash, powerup overlays) uses `UMaterialInstanceDynamic` from C++.
- **NavMesh** — `NavMeshBoundsVolume` per level, agent radius 35 / height 180 / step height 45 to match the player capsule and movement params (step height MUST equal the player's value or AI cannot path through geometry the player can walk over).
- **Collision channels** — four custom channels (`Pickup`, `Projectile`, `Corpse` object channels + `Weapon` trace channel) defined in `Config/DefaultEngine.ini`. Per-actor responses are set in C++ constructors (`SetCollisionResponseToChannel` / `SetCollisionProfileName`), not via editor profile assets. C++ code references the channels via the named constexpr mirror in [QuakeCollisionChannels.h](Source/Quake/QuakeCollisionChannels.h) (`QuakeCollision::ECC_Weapon`, etc.) — never use raw `ECC_GameTraceChannelN` literals, because the channel→index binding lives in the INI and the mirror is the single source of truth on the C++ side. See SPEC section 1.6 for the full response matrix and per-system rules.
- **Project Settings** in `Config/Default*.ini` — version-controlled.

The per-level checklist in `SPEC.md` section 10.5 is the authoritative reference when creating new maps.

## Auto-Memory

The user has a persistent file-based memory system at `C:\Users\Admin\.claude\projects\c--dev-games-unreal\memory\`. Save user/feedback/project memories there per the auto-memory protocol, and check existing memories before recommending design choices.
