# TASKS — FF7 implementation roadmap

Phases are ordered for C++/UE learning progression — foundations first, systems and polish later. Each phase implements one or more sections of [SPEC.md](SPEC.md); the SPEC is the source of truth for class names, APIs, data layouts, and architectural choices. This file tracks only phase ordering, learning objectives, tests, and manual verification.

When the SPEC changes, phases inherit the change automatically. When scope changes, update SPEC first, then revise phases here.

**Per-phase conventions:**
- **Scope** — which SPEC sections this phase implements or touches.
- **Concepts** — the UE/C++ concepts the phase teaches.
- **Tests (Session Frontend)** — `DEFINE_SPEC` (pure-C++ unit) or `AFunctionalTest` (in-PIE) tests that gate phase completion.
- **Manual verification** — steps the user runs in-editor.

After each phase is implemented, I will walk through the code and explain every new concept.

---

## Phase 0 — Build pipeline and module sanity

**Scope:** [§2.1 Build and module layout](SPEC.md#21-build-and-module-layout). Creates `Content/Maps/L_TestChamber.umap` with a floor + DirectionalLight.

**Concepts**
- `.Build.cs` structure; `PublicDependencyModuleNames` vs `PrivateDependencyModuleNames`.
- `FF7.Target.cs` vs `FF7Editor.Target.cs` — game vs editor targets.
- Module lifecycle: `StartupModule` / `ShutdownModule`.
- `IMPLEMENT_PRIMARY_GAME_MODULE`.
- Regenerate-project-files workflow.

**Tests**
- `FF7.Smoke.TestHarness` (spec) — trivially asserts true; proves the test module compiled and registered.
- `FF7.Smoke.TestMapLoads` (functional, `L_TestChamber`) — 10-frame tick, finish.

**Manual**
- `FF7Editor Win64 Development` builds cleanly.
- Editor launches directly into `L_TestChamber`.
- Session Frontend → Automation → `FF7.Smoke.*` pass.

---

## Phase 1 — Player pawn and top-down camera

**Scope:** [§2.2 Character representation](SPEC.md#22-character-representation), [§2.3 Player pawn and camera](SPEC.md#23-player-pawn-and-camera). Generate + import the Cloud placeholder mesh ([§3.2](SPEC.md#32-placeholder-3d-model-pipeline)) and reference it from the player pawn's `UStaticMeshComponent`.

**Concepts**
- `UCLASS`, `UPROPERTY`, `UFUNCTION` and the reflection system.
- Component composition: `CreateDefaultSubobject`, `SetupAttachment`, `RootComponent`.
- `APawn` vs `ACharacter` — why we subclass `APawn` for learning.
- Controller/Pawn split; `PossessedBy` / `OnUnPossess`.
- EnhancedInput: IMC, IA, `UEnhancedInputComponent::BindAction`.
- BP/C++ boundary — IMC/IA as assets on the BP controller; binding logic in C++.

**Tests**
- `FF7.Pawn.DefaultComponents` (spec) — spawn into a transient world; assert capsule + camera + spring-arm. (Actors need a World — plain `NewObject` won't register components.)
- `FF7.Pawn.MoveInputTranslates` (functional, blank map) — inject `Move(1,0)` for 0.5 s; X-delta > 0.

**Manual**
- Play `L_TestChamber`; WASD moves the pawn; camera fixed top-down; no orbit.

---

## Phase 2 — NPC and interaction

**Scope:** [§2.4 NPC and dialogue](SPEC.md#24-npc-and-dialogue) + one widget from [§2.8 UI Slate framework](SPEC.md#28-ui-slate-framework) (`SFF7DialoguePopup`). Place one NPC in `L_TestChamber` pointing into `DT_Dialogue`.

**Concepts**
- `UINTERFACE` two-class pattern, `Execute_*` dispatch.
- Collision channels, overlap events, trace vs overlap for interaction.
- First Slate widget: `SCompoundWidget`, `SLATE_BEGIN_ARGS`, `Construct`, `SNew` / `SAssignNew`, slot syntax.
- `TSharedPtr` / `TSharedRef` and why Slate uses them over `UObject`.

**Tests**
- `FF7.Interact.InterfaceDispatch` (spec).
- `FF7.Interact.DialogueAdvances` (functional).

**Manual**
- Walk up to NPC, press E; dialogue advances, closes on last line.

---

## Phase 3 — Game state and party roster

**Scope:** [§2.5 Party and game state](SPEC.md#25-party-and-game-state) — GameInstance, Roster, Gil plumbing, `FPartyMember` shell, exec debug commands. Equipment/materia refs on the struct stay empty until Phase 7. Generate + import placeholder meshes for the remaining 8 party members (Barret, Tifa, Aerith, Red XIII, Yuffie, Cait Sith, Vincent, Cid); Red XIII and Cait Sith need non-humanoid recipes. Stub `UFF7CharacterDefinition` assets point at each mesh.

**Concepts**
- `UGameInstance` lifecycle — why persistent state lives here, not GameMode.
- USTRUCT + UPROPERTY — reflection for serialization + defaults.
- `TArray<T>` vs `TMap<K,V>` — cost and use cases.
- `UFUNCTION(Exec)` for cheat commands.

**Tests**
- `FF7.Party.InitPopulates9` (spec).
- `FF7.Party.SwapActive` (spec).
- `FF7.Party.SwapRejectsDuplicate` (spec).
- `FF7.Gil.AddSpend` (spec).

**Manual**
- `~` → `FF7.Party.Dump` → 9 members.
- `FF7.Party.SwapActive 0 3` → swap visible via `FF7.Party.Dump`.

---

## Phase 4 — Stats and leveling

**Scope:** [§2.6 Stats and leveling](SPEC.md#26-stats-and-leveling). Adds CSV DataTables per character and the `UFF7LevelCurve` library.

**Concepts**
- `UDataTable` + `FTableRowBase` — CSV import path.
- `FDataTableRowHandle` for cross-asset refs.
- `UDataAsset` vs `UDataTable` — when to use each.
- Separating data (values in assets) from behavior (formulas in C++).
- Golden-value tests.

**Tests**
- `FF7.LevelCurve.ExpToLevel` (spec).
- `FF7.LevelCurve.MultiLevelJump` (spec).
- `FF7.LevelCurve.OverflowClamp` (spec).

**Manual**
- `FF7.Party.GrantEXP 0 5000`; confirm Cloud's level + stats via log/debug draw.

---

## Phase 5 — Inventory and items

**Scope:** [§2.7 Inventory and Gil](SPEC.md#27-inventory-and-gil). Gil scalar + plumbing came with Phase 3; this phase adds `UFF7Inventory`, `DT_Items`, and `UFF7ItemEffects`.

**Concepts**
- `UObject` composition vs `UActorComponent` — why GameInstance-owned state is a plain UObject.
- `TMap<FName,int32>` for stackable inventory.
- `DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam` — binding in BP vs C++.
- Static-function dispatch tables.

**Tests**
- `FF7.Inv.AddRemove` (spec).
- `FF7.Inv.UsePotion` (spec).
- `FF7.Inv.PhoenixDownRevivesKO` (spec).
- `FF7.Inv.DelegateFires` (spec).

**Manual**
- `FF7.Inv.Grant potion 3`, `FF7.Inv.Dump`, `FF7.Inv.Use potion 0` restores HP.

---

## Phase 6 — Slate UI foundation and main menu

**Scope:** [§2.8 UI Slate framework](SPEC.md#28-ui-slate-framework) — `SFF7MainMenu` shell, `SFF7StatusScreen`, Item tab functional; other tabs stubbed with "Coming in Phase N".

**Concepts**
- Slate widget anatomy: `SLATE_BEGIN_ARGS`, `SLATE_ARGUMENT`, `SLATE_EVENT`, named slots.
- `TSharedPtr` / `TSharedRef` / `TWeakPtr`.
- Style sets (`FSlateStyleSet`, `ISlateStyle`).
- `TAttribute<T>` for live binding vs one-shot reads.
- Viewport attachment via `GameViewport->AddViewportWidgetContent`.

**Tests**
- `FF7.UI.MenuDefaultTab` (spec).
- `FF7.UI.MenuNavigatesTabs` (spec).
- `FF7.UI.ItemTabShowsInventory` (functional).

**Manual**
- Tab opens menu; field pauses; navigate tabs; Item + Status populate; use a potion, HP updates live on Status.

---

## Phase 7 — Equipment and Materia

**Scope:** [§2.9 Equipment and Materia](SPEC.md#29-equipment-and-materia) + Equip and Materia tabs in `SFF7MainMenu`.

**Concepts**
- UObject instance data (lifetime, polymorphism, Outer ownership).
- `UPrimaryDataAsset` vs plain `UDataAsset` — AssetManager indexing.
- Soft vs hard object references; why the materia *instance* uses a hard ref.
- Bitfield encoding for linked-slot layouts.
- Recompute-on-change stat-modifier pattern.

**Tests**
- `FF7.Equip.EquipAppliesMods` (spec).
- `FF7.Equip.UnequipReverts` (spec).
- `FF7.Materia.APGrowsTier` (spec).
- `FF7.Equip.LinkedPairDetection` (spec).
- `FF7.Materia.AcquireGoesToPool` (spec) — new instance created → lands in `MateriaPool`.
- `FF7.Materia.SocketMovesFromPool` (spec) — socketing removes from pool, inserts into `FEquippedItem.Sockets`.
- `FF7.Materia.UnsocketReturnsToPool` (spec).
- `FF7.Materia.OneLocationInvariant` (spec) — no instance appears in both pool and a socket.

**Manual**
- Equip Buster Sword on Cloud; Attack rises on Status tab.
- Socket Fire + All into a linked pair; link visual shown.

---

## Phase 8 — Combat scaffold (turn-based skeleton)

**Scope:** [§2.10 Combat framework](SPEC.md#210-combat-framework). ATB deferred to Phase 9 — this phase runs sequential turns. Commands: Attack + Escape. Creates `L_BattleArena.umap`. Generate + import placeholder meshes for the starter enemy set (Guard Hound, Grashtrike, MP soldier, Sweeper) via recipes in `tools/generate_meshes.py`; wire `FEnemyRow.PlaceholderMesh` in `DT_Enemies`.

**Concepts**
- Multi-GameMode project — per-level mode selection.
- `UGameplayStatics::OpenLevel` + `PendingReturn` re-apply pattern.
- `UWorldSubsystem` lifecycle.
- State-machine-in-C++ — `EBattleState` enum owned by the subsystem.
- Interface-based targeting (`IFF7Damageable`, `IFF7StatusTarget`).

**Tests**
- `FF7.Battle.TurnOrderRoundRobin` (spec).
- `FF7.Battle.AttackReducesHP` (spec).
- `FF7.Battle.EscapeReturnsToField` (spec).
- `FF7.EnemyAI.AlwaysFallback` (spec) — empty eligible set → picks an `Always` entry.
- `FF7.EnemyAI.ConditionFilter` (spec) — `SelfHPBelowPct 50` gates correctly above/below threshold.
- `FF7.EnemyAI.WeightedPick` (spec) — deterministic RNG seed → expected entry selected.
- `FF7.EnemyAI.MissingAlwaysFailsValidation` (spec) — `FEnemyRow` without any `Always` entry rejected by the data validator.
- `FF7.Battle.VictoryAwardsEXP` (functional).
- `FF7.Battle.FieldToBattleTransition` (functional) — encounter trigger → `PendingEnemies` populated → arena spawns them.

**Manual**
- Walk into encounter trigger → arena loads; 3 allies vs enemies.
- Attack → damage; defeat all enemies → return to field at original position.
- Escape returns to field successfully.

---

## Phase 9 — ATB

**Scope:** [§2.11 ATB](SPEC.md#211-atb). Adds `SFF7BattleHUD` ATB bars and Wait/Active config toggle (Config tab stub wires it).

**Concepts**
- `UTickableWorldSubsystem` vs `AActor::Tick` vs raw `FTickableGameObject` — when to use each.
- `FTimerManager` for delayed callbacks (windups, animations).
- Multicast delegates driving UI instead of polling.
- Deterministic ordering for testability.

**Tests**
- `FF7.ATB.FillRateMatchesDex` (spec).
- `FF7.ATB.WaitModePauses` (spec).
- `FF7.ATB.HasteDoublesFill` (spec).
- `FF7.ATB.QueueOrder` (spec).

**Manual**
- Haste-buffed ally visibly gets more turns.
- Wait mode pauses enemy gauges while menu is open; Active keeps them filling.

---

## Phase 10 — Magic, abilities, status effects

**Scope:** [§2.12 Magic, abilities, status effects](SPEC.md#212-magic-abilities-status-effects). Adds Magic command to battle menu; casts drive placeholder visual flashes.

**Concepts**
- `UBlueprintFunctionLibrary` for pure shared utility.
- Gameplay Tags (`Status.Poison`, etc.) — identity + later resistances.
- Reinforcing the Phase 8 interfaces via `TakePhysicalDamage` vs `TakeMagicalDamage` dispatch.

**Tests**
- `FF7.Formula.PhysicalGolden` (spec).
- `FF7.Formula.MagicalGolden` (spec).
- `FF7.Status.PoisonTicks` (spec).
- `FF7.Status.SleepBreaksOnDamage` (spec).
- `FF7.Magic.MPCostEnforced` (spec).

**Manual**
- Equip Fire materia, cast Fire, damage scales with Magic stat.
- Cast Poison, HP ticks down across turns.

---

## Phase 11 — Limit Breaks

**Scope:** [§2.13 Limit Breaks](SPEC.md#213-limit-breaks). Adds `UFF7CharacterDefinition` assets for at least Cloud + Tifa with their Limit branches.

**Concepts**
- Event-driven UI via dynamic multicast delegates.
- Data-drives-which + code-drives-how (DataAsset + UObject action classes).
- Progression state on GameInstance.

**Tests**
- `FF7.Limit.GaugeFillsFromDamage` (spec).
- `FF7.Limit.FullReadyFires` (spec).
- `FF7.Limit.TierUnlockByKills` (spec).
- `FF7.Limit.BraverDamage` (spec).

**Manual**
- Take damage until gauge fills; Limit replaces Attack; Braver hits hard; gauge resets.

---

## Phase 12 — Save / Load

**Scope:** [§2.14 Save / Load](SPEC.md#214-save--load). Adds an `AFF7SavePoint` to `L_TestChamber`.

**Concepts**
- `USaveGame` + `SaveGame` UPROPERTY flag.
- `UGameplayStatics::SaveGameToSlot` / `LoadGameFromSlot` / `DoesSaveGameExist`.
- Why live UObject pointers don't go into `UPROPERTY(SaveGame)` — the flatten-to-plain-struct pattern (§2.14 save/load procedures).
- `FSoftObjectPath` + AssetManager resolution for asset refs (materia defs).
- `UGameInstanceSubsystem` lifecycle vs singletons.

**Tests**
- `FF7.Save.RoundTrip` (spec).
- `FF7.Save.VersionMismatchRefuses` (spec).
- `FF7.Save.MissingSlotEmptyList` (spec).
- `FF7.Save.PartitionInvariant` (spec) — after save, `PoolMateriaIds ∪ socket ids` partitions `AllMateria`; no duplicates, no orphans.
- `FF7.Save.EndToEnd` (functional) — field state → save → wipe GameInstance → load → state recovered.

**Manual**
- Grant items/EXP, save to slot 0, close PIE, reopen, load, state intact.

---

## Phase 13 — Audio / music stub

**Scope:** [§2.15 Audio](SPEC.md#215-audio).

**Concepts**
- `UGameInstanceSubsystem` over manager-actor or singleton.
- `TSoftObjectPtr` + `FStreamableManager` async load.
- `UAudioComponent` vs `UGameplayStatics::PlaySound2D`.
- Null-safe APIs — the system runs without audio assets.

**Tests**
- `FF7.Audio.NullRowNoCrash` (spec).
- `FF7.Audio.PlayStopsPrevious` (spec).
- `FF7.Audio.EventHooksFire` (spec).

**Manual**
- No audio assets present → no crashes, warning logs.
- Drop a WAV, point `BGM_Battle` row at it, start a battle, hear it play.

---

## Phase 14 — World map and vehicles

**Scope:** [§2.16 World map and vehicles](SPEC.md#216-world-map-and-vehicles). Creates `L_WorldMap.umap`.

**Concepts**
- `APlayerController::Possess` / `UnPossess` full lifecycle.
- Pawn-switching patterns — shared controller, different pawns.
- Level-streaming basics (introduced; deployed later).
- Collision channels for vehicle terrain restriction.

**Tests**
- `FF7.WorldMap.MountChocobo` (functional).
- `FF7.WorldMap.DismountRestoresPawn` (functional).
- `FF7.WorldMap.ChocoboCrossesRivers` (spec).

**Manual**
- World map moves; mount Chocobo changes movement profile; dismount restores player pawn.

---

## Phase 15 — Shops, summons, Enemy Skill, end-to-end

**Scope:** [§2.17 Shops and save points](SPEC.md#217-shops-and-save-points), [§2.18 Summons, Enemy Skill](SPEC.md#218-summons-enemy-skill), additional Limit tiers/branches.

**Concepts**
- Data-flow consolidation (shops touch inventory + gil + UI; save points touch save + PHS + party).
- Integration-test design — the end-to-end test is where separately-phased systems prove they compose.

**Tests**
- `FF7.Shop.BuySellBalancesGil` (spec).
- `FF7.SavePoint.TentRestoresHP` (spec).
- `FF7.Summon.UseCountDecrements` (spec).
- `FF7.EnemySkill.LearnOnHit` (spec).
- `FF7.E2E.FieldBattleLevelSave` (functional, end-to-end).

**Manual**
- Full loop: walk → shop → save-point → rest → PHS swap → save → close → reopen → load → continue.

---

## Phase 16 — Mini-games and Chocobo breeding (stretch, scaffolding only)

**Scope:** [§2.19 Mini-games and Chocobo breeding (stretch)](SPEC.md#219-mini-games-and-chocobo-breeding-stretch). Scaffolding only — content-light.

**Concepts**
- IMC swapping per mini-game.
- Alternate game modes for mini-game levels.
- Data-model-first design when content is deferred.

**Tests**
- `FF7.MiniGame.TransitionAndReturn` (functional).
- `FF7.Breeding.InheritanceFormula` (spec).

**Manual**
- Race trigger → race level → race chocobo → return via console.
- `FF7.Breed.Pair <idA> <idB>` prints offspring preview.

---

## Running Session Frontend tests

1. Open the editor.
2. **Window → Developer Tools → Session Frontend**.
3. Select the running editor session in the left list.
4. Switch to the **Automation** tab.
5. Filter by `FF7.` to see only project tests.
6. Check the boxes, click **Start Tests**. Results appear in the bottom panel.

`DEFINE_SPEC` tests run headless and finish in milliseconds each. `AFunctionalTest` tests spin up PIE and take seconds — they appear under their map name.
