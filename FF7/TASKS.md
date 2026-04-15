# TASKS — FF7 implementation roadmap

Phases are ordered to teach C++ in Unreal from the ground up. Each phase builds on the previous. Do not skip phases — the learning value comes from the progression.

**Conventions for every phase:**
- **Features** — what ships at the end of the phase.
- **Concepts** — the UE/C++ concepts the phase teaches.
- **Automated tests** — runnable via **Session Frontend** (Window → Developer Tools → Session Frontend → Automation tab). Two flavors:
  - `DEFINE_SPEC` — pure-C++ unit-style tests under `Source/FF7/Tests/` (no PIE, fastest feedback).
  - `AFunctionalTest`-based tests — actor placed in a test map; runs in PIE; asserts gameplay-level behavior.
- **Manual verification** — steps to exercise the feature in-editor. The user runs these and reports back.

After each phase is implemented, I (Claude) will walk through the code with you and explain what every new concept does and why.

---

## Phase 0 — Build pipeline & module sanity

**Features**
- `Content/Maps/L_TestChamber.umap` — empty test map with a floor and a DirectionalLight.
- `DefaultEngine.ini` set to open `L_TestChamber` by default.
- `FF7` module confirmed to compile + load cleanly.

**Concepts**
- `.Build.cs` structure; `PublicDependencyModuleNames` vs `PrivateDependencyModuleNames`.
- `FF7.Target.cs` and `FF7Editor.Target.cs` — what game vs editor targets mean.
- Module lifecycle: `StartupModule` / `ShutdownModule` in [Source/FF7/FF7.cpp](Source/FF7/FF7.cpp).
- `IMPLEMENT_PRIMARY_GAME_MODULE` macro.
- Regenerate project files workflow.

**Automated tests (Session Frontend)**
- `FF7.Smoke.TestHarness` (`DEFINE_SPEC`) — trivially asserts `true`; proves the test module compiled and registered (if the FF7 module fails to load, this test won't appear in Session Frontend, which is itself the signal).
- `FF7.Smoke.TestMapLoads` (`AFunctionalTest` in `L_TestChamber`) — ticks 10 frames then finishes; proves PIE + world + default GameMode come up.

**Manual verification**
- Build `FF7Editor Win64 Development` succeeds with no warnings.
- Launch editor, confirm `L_TestChamber` opens.
- Open Session Frontend → Automation → run `FF7.Smoke.*`, both pass.

---

## Phase 1 — Player pawn & top-down camera

**Features**
- `AFF7CharacterBase` — capsule collision + visual root (`USceneComponent`) with a primitive `UStaticMeshComponent` child (cube for body, smaller cube for head, visible direction indicator).
- `AFF7PlayerPawn : AFF7CharacterBase` — adds `USpringArmComponent` + `UCameraComponent` configured for fixed top-down angle (~60° pitch, fixed arm length).
- `AFF7PlayerController` + `BP_FF7PlayerController` — controller holds soft refs to `UInputMappingContext` and `UInputAction` assets (authored in editor), applies the IMC on possession.
- Input: `IA_Move` (Vector2D, WASD + stick), `IA_Interact` (bool, E/A), `IA_MenuToggle` (bool, Tab/Start).
- `AFF7FieldGameMode` — default pawn/controller set via `BP_FF7FieldGameMode`.

**Concepts**
- `UCLASS`, `UPROPERTY`, `UFUNCTION` specifiers and why the reflection system needs them.
- Component composition: `CreateDefaultSubobject`, `SetupAttachment`, `RootComponent`.
- `APawn` vs `ACharacter` — we inherit from `APawn` + add movement ourselves (cleaner for learning; swap to ACharacter later if needed).
- Controller/Pawn split: Controller = "who's driving", Pawn = "what's being driven". Possession lifecycle (`PossessedBy` / `OnUnPossess`).
- EnhancedInput: `UInputMappingContext`, `UInputAction`, `UEnhancedInputComponent::BindAction`. Why assets beat hardcoded keys.
- BP-vs-C++ boundary: IMC/IA live as assets referenced by the BP controller; binding logic lives in C++.

**Automated tests**
- `FF7.Pawn.DefaultComponents` (spec) — spawn `AFF7PlayerPawn` into a transient world via `FAutomationEditorCommonUtils::CreateNewMap` / `World->SpawnActor`, assert capsule + camera + spring-arm present and attached correctly. (Actors need a World to construct properly; plain `NewObject` doesn't run `OnConstruction`/component registration.)
- `FF7.Pawn.MoveInputTranslates` (functional test in a blank map) — spawn pawn, inject a `Move(1,0)` input for 0.5s, assert X-position delta > 0.

**Manual verification**
- Hit Play in `L_TestChamber`. Character visible from top-down.
- WASD moves the character in world-axis directions (not camera-relative for now).
- Camera stays fixed over the pawn; no orbit.

---

## Phase 2 — NPC & interaction

**Features**
- `IFF7Interactable` (UINTERFACE) — single method `Interact(AFF7PlayerController* By)`.
- `AFF7NPCActor : AActor` — capsule + placeholder mesh + `USphereComponent` interact volume; implements `IFF7Interactable`.
- `SFF7DialoguePopup` (`SCompoundWidget`) — bottom-of-screen panel showing speaker name + text, advances on Interact, closes on final line.
- `AFF7PlayerController` traces/overlaps to find the nearest interactable on `IA_Interact`; calls `Interact`.
- `FDialogueLineRow : FTableRowBase { FName SpeakerId; FText Line; FName NextId; }` — dialogue is DataTable-driven from day one so Phase 15's dialogue work is just more rows. `L_TestChamber` gets one NPC whose line-chain id points into `DT_Dialogue`.

**Concepts**
- `UINTERFACE` declaration pattern (why two classes — the `U*` and `I*`) and `Execute_*` dispatch.
- Collision channels, overlap events (`OnComponentBeginOverlap`), and the difference between overlap and line trace for interaction.
- First Slate widget: `SCompoundWidget`, `SLATE_BEGIN_ARGS`, `Construct`, `SNew` + `SAssignNew`, slot syntax (`SHorizontalBox::Slot`).
- `TSharedPtr` / `TSharedRef` ownership model and why Slate uses it (not `UObject`).

**Automated tests**
- `FF7.Interact.InterfaceDispatch` (spec) — mock interactable class, call `IFF7Interactable::Execute_Interact`, assert handler ran.
- `FF7.Interact.DialogueAdvances` (functional test) — spawn pawn + NPC, simulate Interact input, assert popup visible; simulate Interact again, assert text advanced; final Interact, assert popup closed.

**Manual verification**
- Walk up to the NPC, press E. Popup appears with first line.
- Press E to advance; final press closes the popup.
- Walk away from the NPC: Interact does nothing.

---

## Phase 3 — Game state & party roster

**Features**
- `UFF7GameInstance` — holds `TArray<FPartyMember> Roster` (size 9) and `TArray<int32> ActivePartyIndices` (size 3).
- `FPartyMember` (USTRUCT) — FName CharacterId, current HP/MP, equipment refs (empty for now), materia refs (empty for now).
- `UFF7GameInstance` registered as the project's default via `DefaultEngine.ini` → `[/Script/EngineSettings.GameMapsSettings]` → `GameInstanceClass=/Script/FF7.FF7GameInstance` (the GameInstance class is a project-level setting, not a per-GameMode one).
- Console command `FF7.Party.Dump` — prints roster + active indices.
- Console command `FF7.Party.SwapActive <slotIdx> <rosterIdx>` — swaps an active slot.

**Concepts**
- `UGameInstance` lifecycle — created once at launch, survives level transitions (this is why it holds persistent state, not GameMode).
- `USTRUCT` with `UPROPERTY` — why reflection matters for serialization + defaults.
- `TArray<T>` vs `TMap<K,V>` — cost and use cases.
- Default subobjects in `UGameInstance::Init`.
- Exec functions (`UFUNCTION(Exec)`) for cheat/debug commands.

**Automated tests**
- `FF7.Party.InitPopulates9` (spec) — construct GameInstance, `Init()`, assert Roster.Num() == 9 and first 3 active indices are 0,1,2.
- `FF7.Party.SwapActive` (spec) — swap active slot 0 to roster index 5, assert ActivePartyIndices == [5,1,2].
- `FF7.Party.SwapRejectsDuplicate` (spec) — swapping to an already-active roster index is rejected.

**Manual verification**
- Launch PIE, open console (`~`), `FF7.Party.Dump` prints 9 members.
- `FF7.Party.SwapActive 0 3` then `FF7.Party.Dump` confirms swap.

---

## Phase 4 — Stats & leveling (data-driven)

**Features**
- `FCharacterStats` (USTRUCT) — HP, MaxHP, MP, MaxMP, Str, Vit, Mag, Spr, Dex, Lck, Level, EXP.
- `FLevelCurveRow : FTableRowBase` — Level, EXPToNext, HP, MP, Str, Vit, Mag, Spr, Dex, Lck (one row per level per character).
- `Content/Data/DT_LevelCurve_Cloud.csv` and friends — CSV-imported DataTable per character.
- `UFF7LevelCurve` static library — `ApplyLevelUp(FCharacterStats&, const UDataTable* Curve)` and `AwardEXP(FCharacterStats&, int32 EXP, const UDataTable* Curve)`.
- Console command `FF7.Party.GrantEXP <slotIdx> <amount>`.

**Concepts**
- `UDataTable` + `FTableRowBase` — how CSV imports map to structs.
- Row handles (`FDataTableRowHandle`) for safe references from other data.
- `UDataAsset` vs `UDataTable` — when to use each (DataTable = tabular repeated rows; DataAsset = one-off tuning).
- Separating *data* (values in assets) from *behavior* (formulas in C++) for testability.
- Writing "golden-value" tests.

**Automated tests**
- `FF7.LevelCurve.ExpToLevel` (spec) — inject a known curve, feed `AwardEXP(0, 100)`, assert expected level and stats (golden table of inputs → outputs).
- `FF7.LevelCurve.MultiLevelJump` (spec) — single huge EXP award crosses multiple levels, assert stats match terminal level.
- `FF7.LevelCurve.OverflowClamp` (spec) — EXP past max level clamps, does not crash.

**Manual verification**
- `FF7.Party.GrantEXP 0 5000`, then inspect (temporary debug draw or log) that Cloud's level and stats reflect the curve.

---

## Phase 5 — Inventory & items

**Features**
- `FItemRow : FTableRowBase` — Id, DisplayName, ItemType enum (Consumable/Key/Misc), GilValue, EffectTag.
- `UFF7Inventory` — plain `UObject` owned by `UFF7GameInstance` (not a `UActorComponent` — GameInstance isn't an `AActor`). Holds `TMap<FName, int32>` row-id → count.
- `UFF7Inventory::AddItem(FName, int32)`, `RemoveItem`, `GetCount`, broadcast `OnInventoryChanged` delegate.
- `int32 Gil` on `UFF7GameInstance` with `Add`/`Spend` helpers and `OnGilChanged` delegate.
- Consumable effect dispatcher: `UFF7ItemEffects::ApplyPotion`, `ApplyPhoenixDown` (operate on `FCharacterStats`).
- Console commands: `FF7.Inv.Grant <id> <n>`, `FF7.Inv.Use <id> <slotIdx>`, `FF7.Inv.Dump`.

**Concepts**
- `UObject` composition vs `UActorComponent` — why GameInstance-owned state uses UObjects.
- `TMap<FName,int32>` for stackable inventory.
- Broadcast delegates: `DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam`, binding in BP vs C++.
- Static-function dispatch tables for effects (simple alternative to a full effect-object hierarchy for now).

**Automated tests**
- `FF7.Inv.AddRemove` (spec) — add 5 potions, remove 3, count == 2; remove 10, count == 0 (clamped).
- `FF7.Inv.UsePotion` (spec) — damage a stats struct, apply Potion, assert HP +100 (capped at MaxHP).
- `FF7.Inv.PhoenixDownRevivesKO` (spec) — stats at 0 HP, apply Phoenix Down, assert HP > 0 and not-KO'd.
- `FF7.Inv.DelegateFires` (spec) — bind lambda to `OnInventoryChanged`, add item, assert lambda fired once.

**Manual verification**
- `FF7.Inv.Grant potion 3`, `FF7.Inv.Dump`, see 3 potions.
- `FF7.Inv.Use potion 0` on damaged Cloud, HP restored.

---

## Phase 6 — Slate UI foundation & main menu

**Features**
- `SFF7MainMenu` — tabbed layout: **Item, Magic, Materia, Equip, Status, Config, Save, PHS**. Only **Item** and **Status** are functional this phase; others show a stub panel "Coming in Phase N".
- `Item` tab: list inventory with counts, target selector, Use button → routes to Phase 5 dispatcher.
- `Status` tab: shows selected party member's stats from Phase 4.
- `SFF7MenuStyle` — a minimal `FSlateStyleSet` for fonts, colors, padding so later menus reuse tokens.
- Controller opens/closes the menu on `IA_MenuToggle`, pausing field time while open.

**Concepts**
- Slate widget anatomy in depth: `SLATE_BEGIN_ARGS`, `SLATE_ARGUMENT`, `SLATE_EVENT`, named slots.
- `TSharedPtr` / `TSharedRef` / `TWeakPtr` — what each guarantees, how to pick.
- Style sets (`FSlateStyleSet`, `ISlateStyle`) — why you centralize tokens.
- Widget → game-state binding via attribute functions (`TAttribute<FText>`) vs one-shot reads.
- Viewport attachment via `GameViewport->AddViewportWidgetContent`.

**Automated tests**
- `FF7.UI.MenuDefaultTab` (spec) — construct `SFF7MainMenu`, assert selected tab is Item.
- `FF7.UI.MenuNavigatesTabs` (spec) — simulate tab-right input, assert selection advances.
- `FF7.UI.ItemTabShowsInventory` (functional test) — grant items, open menu, assert visible rows equal inventory size.

**Manual verification**
- Press Tab in PIE, menu opens, field pauses.
- Navigate tabs, Item + Status populate, others show stub.
- Use a potion from the Item tab — HP updates in real time on Status tab.

---

## Phase 7 — Equipment & Materia slots

**Features**
- `FEquipmentRow : FTableRowBase` — Id, Slot (Weapon/Armor/Accessory), stat modifiers, `uint8 SocketCount`, `uint16 LinkMask` (bit N set = slot N linked to N+1).
- `UFF7MateriaDataAsset : UPrimaryDataAsset` — Family, Name, `TArray<FMateriaTier>` (AP threshold → granted ability id).
- `UFF7MateriaInstance : UObject` — hard `UFF7MateriaDataAsset* Def`, `int32 CurrentAP`, computed `int32 Tier`. (Hard ref because at runtime the instance needs the def resolved; SaveGame persists the def by `FSoftObjectPath` and re-resolves on load.)
- `UFF7Equipment : UObject` — per-character; three `FEquippedItem { FName EquipRowId; TArray<UFF7MateriaInstance*> Sockets; }`.
- **Equip** screen in `SFF7MainMenu` — select weapon/armor/accessory from inventory, preview stat deltas, commit.
- **Materia** screen — socket/unsocket materia into equipped gear, visualize linked pairs.
- `FF7.Inv.GrantMateria <id>` console command.

**Concepts**
- `UObject` subclass instance data — when you need it over structs (lifetime, polymorphism, outer ownership).
- `UPrimaryDataAsset` vs plain `UDataAsset` — why primary assets get indexed by the AssetManager.
- Soft vs hard object references (`TSoftObjectPtr` — loaded on demand).
- Bitfield encoding for linked-slot layouts; helper inline methods for readability.
- Equip → stat-modifier application pattern (recompute-on-change, not tick).

**Automated tests**
- `FF7.Equip.EquipAppliesMods` (spec) — equip a weapon with Str+5, assert derived Attack increases by 5.
- `FF7.Equip.UnequipReverts` (spec) — unequip, assert stats restored.
- `FF7.Materia.APGrowsTier` (spec) — award AP to a materia instance, assert `Tier` increments at threshold.
- `FF7.Equip.LinkedPairDetection` (spec) — build equipment with LinkMask = 0b01, query `IsSocketLinkedTo(0) == 1`.

**Manual verification**
- Grant Cloud a Buster Sword, equip via menu, see Attack go up in Status tab.
- Grant Fire + All materia, socket them into linked pair, confirm linked visual.

---

## Phase 8 — Combat scaffold (turn-based skeleton)

**Features**
- `L_BattleArena.umap` — empty arena level with spawn points for 3 allies + 4 enemies.
- `AFF7EncounterTrigger` (AActor) — overlap volume; on player overlap, stores `FReturnContext { FName LevelName; FTransform PlayerTransform; }` on GameInstance, then `OpenLevel(L_BattleArena)`.
- `AFF7BattleGameMode` + `BP_FF7BattleGameMode` — spawns active party + encounter enemies on BeginPlay.
- Field GameMode on BeginPlay checks GameInstance for a pending `FReturnContext`; if present, teleports the possessed pawn to the stored transform and clears it. (`OpenLevel` doesn't carry a transform, so this re-apply step is required.)
- `UFF7BattleSubsystem : UWorldSubsystem` in the battle world — round-robin turn manager (no ATB yet, just sequential turns).
- `IFF7Damageable` / `IFF7StatusTarget` interfaces introduced here (party + enemy actors implement both) so Attack resolution is side-agnostic.
- Commands this phase: **Attack** and **Escape** (Escape rolls against an always-flee-succeeds placeholder formula, returns to field). Targeting via a simple selection widget (`SFF7TargetPicker`).
- Multicast delegates fired on the BattleSubsystem: `OnBattleStart`, `OnBattleVictory`, `OnBattleDefeat`, `OnCombatantActed`. Phase 13 audio binds to these; Phase 11 Limit gauges bind to `OnCombatantActed`.
- Victory condition: all enemies KO'd → load return level, re-apply stored transform, award EXP/gil.
- Defeat condition: all allies KO'd → "Game Over" placeholder, return to save point placeholder.

**Concepts**
- Multi-GameMode project — which mode the World uses is set per-level.
- `UGameplayStatics::OpenLevel` — caveats about cleanup and the GameInstance surviving.
- `UWorldSubsystem` lifecycle (created per-world, gone when the world tears down).
- State machine in C++ — enum `EBattleState { SelectAction, Executing, Resolving, Victory, Defeat }` with transitions owned by the subsystem.

**Automated tests**
- `FF7.Battle.TurnOrderRoundRobin` (spec) — 3 allies + 2 enemies, step turns, assert order by dex.
- `FF7.Battle.AttackReducesHP` (spec) — apply an Attack action, assert target HP decreased by formula output.
- `FF7.Battle.VictoryAwardsEXP` (functional test) — script-defeat enemies, assert GameInstance EXP increased and return-level requested.
- `FF7.Battle.FieldToBattleTransition` (functional test) — overlap `AFF7EncounterTrigger`, assert `L_BattleArena` loaded.

**Manual verification**
- Walk into the encounter trigger in `L_TestChamber`.
- Arena loads, 3 party members on one side, enemies on the other.
- Select Attack → target → damage applied; enemies take turns attacking back.
- Defeat all enemies → return to `L_TestChamber` at the original position.

---

## Phase 9 — ATB system

**Features**
- `FATBGauge` per combatant — 0..1000 fill, rate derived from Dex + Haste/Slow modifier.
- `UFF7BattleSubsystem` is changed to extend `UTickableWorldSubsystem` (UE5's dedicated base for tickable world subsystems — cleaner than hand-implementing `FTickableGameObject` on a plain `UWorldSubsystem`). Its `Tick(DeltaTime)` advances all gauges.
- **Wait mode** / **Active mode** config toggle in `UFF7GameInstance` (Config menu stub wires it this phase). Wait mode pauses all gauges while a menu is open.
- Action queue — full gauges enqueue in order; ties break by Luck then stable-order.
- Battle HUD (`SFF7BattleHUD`) draws ATB bars per combatant.

**Concepts**
- `UTickableWorldSubsystem` vs `AActor::Tick` vs raw `FTickableGameObject` — when to use each; why world subsystems are the right home for battle-wide state.
- `FTimerManager` for delayed callbacks (action windups, animations).
- Multicast delegates (`DECLARE_DYNAMIC_MULTICAST_DELEGATE_*`) for `OnGaugeFull`, `OnActionComplete`, `OnATBModeChanged` — HUD binds to these instead of polling.
- Deterministic ordering for testability.

**Automated tests**
- `FF7.ATB.FillRateMatchesDex` (spec) — simulate 1s of tick, assert gauge delta proportional to Dex.
- `FF7.ATB.WaitModePauses` (spec) — set Wait + open menu, tick 1s, assert no fill.
- `FF7.ATB.HasteDoublesFill` (spec) — apply Haste status, assert fill rate doubled.
- `FF7.ATB.QueueOrder` (spec) — three combatants fill at different times, assert action order.

**Manual verification**
- Fight with one haste-buffed ally — it gets noticeably more turns.
- Open the in-battle menu in Wait mode — enemy gauges stop; in Active mode — they keep filling.

---

## Phase 10 — Magic, abilities, status effects

**Features**
- `FSpellRow : FTableRowBase` — Id, MPCost, Power, Element, TargetType, StatusApplied (optional).
- `UFF7DamageCalc` (static library) — FF7-style magic and physical damage formulas (documented inline; not pixel-accurate to the original but readable).
- `FStatusEffect` + `UFF7StatusLibrary` — apply/tick/remove; poison DOT, regen HOT, haste/slow speed mod, sleep cured by damage.
- **Magic** command in battle — opens the player's known spells (from equipped materia), MP-gated.
- Visual stubs for effects (primitive spheres / color flashes on target).

**Concepts**
- Static utility classes (`UFUNCTION(BlueprintCallable)` on a `UBlueprintFunctionLibrary`) — shared pure functions callable from BP and C++.
- Gameplay Tags (introduction) — tagging statuses as `Status.Poison`, `Status.Haste`, etc.; later used for resistances and counters.
- Reinforcing the interface-based targeting pattern from Phase 8 — now that damage actually varies by spell element, `IFF7Damageable::TakeMagicalDamage` vs `TakePhysicalDamage` shows why one interface can have multiple dispatch paths.

**Automated tests**
- `FF7.Formula.PhysicalGolden` (spec) — table of known (attacker stats, defender stats, weapon power) → expected damage.
- `FF7.Formula.MagicalGolden` (spec) — same for magic.
- `FF7.Status.PoisonTicks` (spec) — apply Poison, tick 5 turns, assert HP decrements each tick.
- `FF7.Status.SleepBreaksOnDamage` (spec) — apply Sleep, take damage, assert Sleep removed.
- `FF7.Magic.MPCostEnforced` (spec) — target under MP threshold cannot cast.

**Manual verification**
- Equip Fire materia, cast Fire in battle, enemy takes damage proportional to Magic stat.
- Cast Poison on an enemy, watch HP tick down across turns.

---

## Phase 11 — Limit Breaks

**Features**
- `FLimitGauge` per party member — 0..255 fill, increments on damage taken.
- `UFF7CharacterDefinition : UPrimaryDataAsset` — per-character Limit branches and tier unlock conditions (kill counts, branch usage). Also the home for starting stats and weapon restriction referenced by earlier phases.
- **Limit** command replaces Attack when the gauge is full.
- `UFF7LimitAction` subclasses — Braver, Cross-Slash (Cloud); Beat Rush (Tifa); per-character unique effects.
- Limit UI indicator on battle HUD glows when ready.

**Concepts**
- Event-driven UI: dynamic multicast `FOnLimitGaugeChanged` / `FOnLimitReady` wire the HUD directly.
- Per-character behavior via DataAsset + UObject action classes (data drives *which* actions are available, code drives *how*).
- Unlock conditions as progression state on the GameInstance.

**Automated tests**
- `FF7.Limit.GaugeFillsFromDamage` (spec) — apply damage, assert gauge delta.
- `FF7.Limit.FullReadyFires` (spec) — fill gauge, assert `OnLimitReady` fires exactly once.
- `FF7.Limit.TierUnlockByKills` (spec) — increment kill count past threshold, assert tier 1-2 unlocked.
- `FF7.Limit.BraverDamage` (spec) — golden damage value for Cloud's Braver vs a known target.

**Manual verification**
- Let Cloud take damage until gauge fills; Limit command replaces Attack; select Braver, enemy takes large damage; gauge resets.

---

## Phase 12 — Save / Load

**Features**
- `UFF7SaveGame : USaveGame` — all party/inventory/equipment/flags/gil/materia-AP fields tagged `UPROPERTY(SaveGame)`.
- Materia instances serialize the *def asset path* (`FSoftObjectPath`), not the live UObject pointer — on load, paths re-resolve to the current `UFF7MateriaDataAsset` via the AssetManager. This is why we chose `UPrimaryDataAsset` back in Phase 7.
- `UFF7SaveSubsystem : UGameInstanceSubsystem` — `Save(int32 Slot)`, `Load(int32 Slot)`, `ListSlots()`, returns `FSaveSlotSummary` for UI.
- `SFF7SaveMenu` — 3 slots, each showing summary (character portraits stub, play time, location name).
- Save-point actor in `L_TestChamber` opens the save menu via interact.
- Version header (`int32 SaveVersion = 1`) — mismatch logs and refuses load.

**Concepts**
- `USaveGame` + the `SaveGame` UPROPERTY flag — what's serialized and what isn't.
- `UGameplayStatics::SaveGameToSlot` / `LoadGameFromSlot` / `DoesSaveGameExist`.
- `FArchive` basics for custom fields (not needed yet, but introduced).
- `UGameInstanceSubsystem` lifecycle and why subsystems beat singletons.

**Automated tests**
- `FF7.Save.RoundTrip` (spec) — populate GameInstance, save to slot 0, wipe, load, assert equality.
- `FF7.Save.VersionMismatchRefuses` (spec) — save with version 1, force version 2, load, assert rejection + error.
- `FF7.Save.MissingSlotEmptyList` (spec) — no saves, `ListSlots` returns empty.
- `FF7.Save.EndToEnd` (functional test) — grant items + EXP, save, restart PIE, load, verify state.

**Manual verification**
- Play, acquire items and EXP, save to slot 0 via save-point NPC.
- Close PIE, reopen, load slot 0, state intact.

---

## Phase 13 — Audio / music stub system

**Features**
- `FAudioTrackRow : FTableRowBase` — Id, `TSoftObjectPtr<USoundBase>` Asset, Volume, bLooping.
- `UFF7AudioSubsystem : UGameInstanceSubsystem` — `PlayBGM(FName)`, `StopBGM(float FadeSeconds)`, `PlaySFX(FName)`, `PlayJingle(FName)` (pauses BGM, plays jingle, resumes).
- Two `UAudioComponent` slots for BGM crossfade.
- Null-asset safe — missing row or null soft ref logs once, plays nothing.
- Hooks fired by gameplay: `OnBattleStart` → `PlayBGM("BGM_Battle")`, `OnBattleVictory` → `PlayJingle("Jingle_Victory")`, `OnLevelUp`, `OnMenuOpen`.

**Concepts**
- `UGameInstanceSubsystem` — when to use over a manager actor or singleton.
- Soft object references (`TSoftObjectPtr`) + async load (`FStreamableManager`) — asset may load on demand.
- `UAudioComponent` per-slot vs `UGameplayStatics::PlaySound2D` — tradeoffs.
- Null-safe APIs — the system works when rows are empty so the project runs without audio assets today.

**Automated tests**
- `FF7.Audio.NullRowNoCrash` (spec) — resolve missing id, asserts no crash and warning logged.
- `FF7.Audio.PlayStopsPrevious` (spec) — call PlayBGM twice, assert old component stopped, new one playing.
- `FF7.Audio.EventHooksFire` (spec) — fire `OnBattleStart` delegate, assert `PlayBGM` called with expected id.

**Manual verification**
- No audio assets present: play works without error, log shows silent-skip warnings.
- Drop a WAV asset into the project, point the BGM_Battle row at it, start a battle, hear it play.

---

## Phase 14 — World map & vehicles

**Features**
- `L_WorldMap.umap` — large open level with placeholder terrain.
- `AFF7WorldMapPawn` — faster movement profile than field pawn.
- `AFF7ChocoboPawn`, `AFF7BuggyPawn` — alt pawns with different speed/terrain restrictions.
- Vehicle mount/dismount via interact with parked-vehicle actors; `AFF7PlayerController::Possess` swaps active pawn.
- Encounter triggers carry over from Phase 8, scaled for overworld.

**Concepts**
- `APlayerController::Possess` / `UnPossess` — the full possession lifecycle.
- Pawn switching patterns — shared controller, different pawns.
- Level streaming basics (single persistent level + streaming sublevels) — introduced, not yet deployed at scale.
- Terrain/collision channel filtering for vehicle restrictions.

**Automated tests**
- `FF7.WorldMap.MountChocobo` (functional test) — overlap Chocobo, interact, assert controller possesses `AFF7ChocoboPawn`.
- `FF7.WorldMap.DismountRestoresPawn` (functional test) — dismount, assert controller possesses `AFF7WorldMapPawn`.
- `FF7.WorldMap.ChocoboCrossesRivers` (spec) — query terrain restriction flags for chocobo vs buggy.

**Manual verification**
- Launch world map, player pawn moves.
- Walk to parked Chocobo, interact, mount; movement profile changes.
- Dismount, player pawn resumes.

---

## Phase 15 — Polish (shops, save points, summons, EnemySkill, end-to-end)

**Features**
- `FShopRow : FTableRowBase` — shop id + stock list. `SFF7ShopMenu` for buy/sell.
- `AFF7SavePoint` actor — Tent (HP/MP restore + save menu + PHS).
- Summons as a Phase-10 materia family: `UFF7SummonAction` subclasses (Ifrit, Shiva, Ramuh) with per-battle use caps.
- **Enemy Skill** materia family — learned when a registered skill hits the character.
- Additional Limit tiers and branches per character.
- End-to-end functional test.

**Concepts**
- Consolidating data flows — shops touch inventory, gil, and UI; save points touch save system, PHS, and party state.
- Reference refactor — now that many systems are built, revisit modularity and split modules if compile times hurt.

**Automated tests**
- `FF7.Shop.BuySellBalancesGil` (spec).
- `FF7.SavePoint.TentRestoresHP` (spec).
- `FF7.Summon.UseCountDecrements` (spec).
- `FF7.EnemySkill.LearnOnHit` (spec) — enemy casts a learnable skill on the party, assert skill registered.
- `FF7.E2E.FieldBattleLevelSave` (functional test) — field → trigger encounter → battle → victory → level-up → save → load → field with state intact.

**Manual verification**
- Full loop: walk around field, enter shop, buy potion, talk to save-point NPC, rest, swap party via PHS, save, close PIE, reopen, load, continue.

---

## Phase 16 — Mini-games & Chocobo breeding (stretch, scaffolding only)

**Features**
- `AFF7MiniGameTrigger` — transitions to a dedicated mini-game level the same way encounter triggers do.
- `AFF7ChocoboRaceTrack` + `AFF7RaceChocoboPawn` — alternate input mapping (hold to sprint, release to regen stamina).
- `UChocoboBreedingSubsystem` — data model for parent Chocobo refs, offspring rating, stat inheritance formula; no UI beyond console commands.
- No full mini-game content — validates that the architecture supports these features without blocking earlier phases.

**Concepts**
- Input mapping contexts swapped per mini-game.
- Alternate game modes for mini-game levels.
- Data-model-first design when content is deferred.

**Automated tests**
- `FF7.MiniGame.TransitionAndReturn` (functional test).
- `FF7.Breeding.InheritanceFormula` (spec) — golden values for offspring stats given parent stats.

**Manual verification**
- Walk into race entrance, transition to race track level, take control of race Chocobo, return to overworld via console.
- `FF7.Breed.Pair <idA> <idB>` console command prints offspring preview.

---

## Running Session Frontend tests

1. Open the editor.
2. **Window → Developer Tools → Session Frontend**.
3. Select the running editor session in the left list.
4. Switch to the **Automation** tab.
5. Filter by `FF7.` to see only project tests.
6. Check the boxes, click **Start Tests**. Results appear in the bottom panel.

`DEFINE_SPEC` tests run headless and finish in milliseconds each. `AFunctionalTest` tests spin up PIE and take seconds — they appear under their map name.
