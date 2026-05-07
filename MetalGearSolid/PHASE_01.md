# Phase 01 - Core Prototype C++ Plan

This document defines the C++ implementation contract for Phase 1. It is still high-level and contains no code, but it is specific enough that an implementation pass does not need to invent major behavior.

## Phase Goals

- Establish a controllable player character.
- Establish a stealth-friendly camera baseline.
- Establish basic movement and collision.
- Establish a simple interaction foundation.
- Establish a graybox test map flow.
- Establish Enhanced Input bindings.
- Establish pause and game mode flow.

## Required Asset Names

Use these names for Phase 1 assets so setup and validation are predictable:

- Prototype map: `L_Phase01_CorePrototype`
- Player Blueprint: `BP_MGSCharacter`
- Player controller Blueprint: `BP_MGSPlayerController`
- Game mode Blueprint: `BP_MGSGameMode`
- Input mapping context: `IMC_Phase01`
- Move input action: `IA_Move`
- Interact input action: `IA_Interact`
- Pause input action: `IA_Pause`
- Pause widget Blueprint: `WBP_Phase01Pause`

Use these names for Phase 1 C++ types:

- Player character: `AMGSCharacter`
- Player controller: `AMGSPlayerController`
- Game mode: `AMGSGameMode`
- Interactable actor: `AMGSInteractableActor`

## Phase 1 Design Decisions

- The prototype player remains an Unreal `ACharacter`, not a custom pawn.
- Movement is character-movement based and constrained to normal walking on the floor.
- The default camera target is a raised third-person / overhead stealth camera, not a free orbit action camera.
- Movement is camera-relative in Phase 1.
- Jumping is removed from Phase 1 input and acceptance. Existing jump helper functions can remain in C++, but they must not be bound by `IMC_Phase01` and must not be required by validation.
- Interaction is intentionally simple: one focused interact action, one prompt state, one test response.
- Phase 1 uses explicit default tuning values in C++ and exposes them for editor adjustment.
- Implement gameplay behavior in C++ by default.
- Keep Blueprints as a thin layer for adjusting parameters, assigning assets, assigning input references, and placing configured actors in maps.
- Blueprint graphs must not own Phase 1 gameplay rules.
- Do not add abstractions beyond the ones required by Phase 1.
- Do not add hooks, components, interfaces, base classes, or extension points for unimplemented systems.

## Existing Starting Point

- `AMGSCharacter` already exists as the current player character base.
- `AMGSPlayerController` already manages Enhanced Input mapping contexts.
- `AMGSGameMode` already exists as the current game mode base.
- `LogMGS` already exists as the project log category.
- The project is currently close to the Unreal third-person template, so Phase 1 reshapes these classes toward a stealth prototype foundation.
- The existing gameplay classes are currently abstract template-style classes. Phase 1 makes the C++ gameplay classes concrete.

## Expected File / Type Shape

Existing types to keep and evolve:

- `Source/MGS/MGSCharacter.h`
- `Source/MGS/MGSCharacter.cpp`
- `Source/MGS/MGSPlayerController.h`
- `Source/MGS/MGSPlayerController.cpp`
- `Source/MGS/MGSGameMode.h`
- `Source/MGS/MGSGameMode.cpp`

New C++ types expected for Phase 1:

- `AMGSInteractableActor`

Asset or Blueprint types expected for Phase 1:

- Prototype player Blueprint derived from `AMGSCharacter`
- Prototype game mode Blueprint derived from `AMGSGameMode`
- Prototype player controller Blueprint derived from `AMGSPlayerController`
- Prototype map for Phase 1 validation
- Enhanced Input mapping context
- Enhanced Input actions for move, interact, and pause
- Minimal pause widget Blueprint derived from `UUserWidget`

Blueprint layer policy:

- Use Blueprint defaults to tune values exposed by C++.
- Use Blueprint defaults to assign meshes, materials, widgets, input actions, mapping contexts, and test references.
- Use Blueprint child classes only as configured versions of C++ classes.
- Do not put movement, camera, pause, interaction targeting, or activation rules in Blueprint graphs for Phase 1.
- Temporary Blueprint graph gameplay logic is not allowed in Phase 1.

## Edge Cases And Required Decisions

Phase 1 implementation must address these cases up front.

Class instancing:

- `AMGSCharacter`, `AMGSPlayerController`, and `AMGSGameMode` must be concrete C++ classes for Phase 1.
- Blueprint children are used only for defaults and references. The C++ classes remain spawnable and usable without requiring Blueprint graph behavior.

Input asset failure:

- Missing input mapping contexts or input actions must log a clear `LogMGS` warning or error.
- Missing move input must not crash the game.
- Missing interact input must not crash the game.
- Missing pause input must log clearly because it blocks Phase 1 validation.

Possession mismatch:

- `AMGSPlayerController` must safely handle possessing something other than `AMGSCharacter`.
- Movement and interaction routing must no-op safely when the possessed pawn is invalid or the wrong type.
- A wrong pawn type must log once during setup or first failed routing, not every frame.

Interaction detection:

- Phase 1 interaction must use a short forward sphere trace from the player, not an overlap-only system.
- The trace must select the closest valid interactable hit in front of the player.
- Only one active interactable is tracked at a time.
- Disabled interactables must not show prompts and must not activate.
- Destroyed or invalid interaction targets must be cleared safely.
- Repeated interact presses on a valid target are allowed. A single-use target must disable itself after activation.

Prompt fallback:

- Phase 1 does not require a UMG prompt widget. Prompt state is exposed through C++ and validated through logging/debug display.
- A missing prompt widget must not block interaction validation.

## Player Character

Primary C++ owner: `AMGSCharacter`

Phase 1 responsibilities:

- Represent the player pawn used in the prototype.
- Own the capsule collision and basic movement component settings.
- Own the camera boom and follow camera components.
- Expose prototype tuning values for movement speed, acceleration, braking, rotation behavior, camera distance, camera height, camera pitch, and camera lag.
- Provide movement functions driven by player input.
- Do not bind jump input for Phase 1. Existing jump helper functions can remain for template compatibility, but Phase 1 does not call them.
- Provide an entry point for interact input.
- Own the interaction trace, current interactable reference, and prompt state directly.

Required exposed tuning categories:

- `Movement`
- `Camera`
- `Interaction`

Required Blueprint-facing functions:

- Move command
- Interact command
- Pause remains controller-owned, not character-owned

Required default character tuning:

- Walk speed: `350 cm/s`
- Acceleration: `2048 cm/s^2`
- Braking deceleration walking: `2048 cm/s^2`
- Rotation rate yaw: `540 deg/s`
- Orient rotation to movement: enabled
- Use controller yaw rotation: disabled
- Jump input binding: absent from `IMC_Phase01`

## Camera

Primary C++ owner: `AMGSCharacter`

Phase 1 responsibilities:

- Provide a basic top-down / third-person stealth camera mode.
- Keep the camera readable for room-scale navigation.
- Expose enough camera tuning to adjust height, distance, pitch, and lag in the editor.
- Remove free-orbit camera behavior from the default Phase 1 camera path.
- Keep camera collision disabled by default for stable Phase 1 framing.

Baseline camera target:

- Raised behind/above the player.
- Angled downward.
- Tunable in Blueprint/editor.
- Stable enough for narrow rooms and wall collision tests.

Required camera defaults:

- The camera follows the player through the character camera boom.
- The player does not control camera yaw or pitch during Phase 1 gameplay.
- `IA_Look` is not a required Phase 1 input action.
- Existing look functions can remain in C++ for template compatibility, but they are not bound in `IMC_Phase01`.
- Camera collision is disabled.
- Camera lag is disabled.
- Camera boom length: `900 cm`
- Camera boom pitch: `-60 deg`
- Camera boom yaw: `0 deg` relative to the player.
- Camera boom roll: `0 deg`
- Camera uses pawn control rotation: disabled.

## Movement

Primary C++ owner: `AMGSCharacter`

Phase 1 responsibilities:

- Convert Enhanced Input movement values into world movement.
- Convert movement relative to the current camera yaw.
- Configure baseline movement speed and rotation behavior.
- Rotate the character to face movement direction.
- Keep all Phase 1 movement in a single normal locomotion state.

Required input behavior:

- `Move` accepts a 2D input vector.
- Forward/back and right/left movement must work.
- Analog magnitude must affect movement through Enhanced Input and Character Movement.
- No sprint, crouch, crawl, wall press, or combat movement modes in Phase 1.
- No player-controlled camera look is implemented in Phase 1.
- `W` maps to positive move Y.
- `S` maps to negative move Y.
- `D` maps to positive move X.
- `A` maps to negative move X.

## Collision

Primary C++ owner: `AMGSCharacter`

Supporting Unreal systems:

- Character capsule collision
- Static mesh collision in the graybox map
- Basic collision channels and presets

Phase 1 responsibilities:

- Use the character capsule as the main player collision shape.
- Confirm the player blocks against walls, floors, and simple props.
- Keep collision behavior conventional for Unreal Character movement.

## Interaction Prompts

Primary C++ owners:

- `AMGSCharacter`
- `AMGSInteractableActor`

Phase 1 responsibilities:

- Define the player-facing concept of an interact action.
- Define a basic way to detect nearby interactable objects.
- Define a basic prompt state that debug output can read.
- Support simple test interactables in the graybox map.
- Keep prompt text generic and prototype-only.
- Support one active interactable at a time.

Required interaction behavior:

- Detect a nearby interactable using a short forward sphere trace.
- Track the current best interactable.
- Expose whether an interaction prompt should be visible.
- Expose prompt text.
- Call the interactable when the player presses Interact.
- Log activation and toggle an activated state on the test interactable.

Required interaction contract:

- Interactable has enabled / disabled state.
- Interactable exposes prompt text.
- Interactable exposes an activation event.
- `AMGSCharacter` owns detection and current-target selection.
- Interaction trace range is an editor-tunable value on `AMGSCharacter`.
- Interaction trace radius is an editor-tunable value on `AMGSCharacter`.
- Interaction detection runs from the player's view/camera-facing forward direction.
- Interaction detection ignores the owning character.
- Interaction activation is ignored while the game is paused.
- Default interaction trace range: `250 cm`
- Default interaction trace radius: `35 cm`
- Default prompt text: `Interact`
- Default test activation log: `Interactable activated`

## Test Map / Graybox Room

Primary C++ owners:

- `AMGSGameMode`
- `AMGSPlayerController`
- `AMGSCharacter`

Supporting asset work:

- One prototype map
- Basic floor and wall geometry
- Player start
- Simple interactable test object
- Minimal lighting
- Simple colored meshes for player visibility, interactable visibility, and obstacle readability

Phase 1 responsibilities:

- Provide a small controlled space for validating movement, camera, collision, and interaction.
- Keep the map focused on gameplay testing rather than visual fidelity.
- Include enough layout variation to test turning, wall collision, and camera readability.
- Avoid production Shadow Moses layout work in this phase.

Required map contents:

- One player start.
- One open movement area.
- One narrow passage wide enough for the capsule plus movement clearance.
- One L-shaped wall.
- One simple obstacle.
- One test interactable.
- The test interactable must be placed in front of the player start path so sphere-trace interaction can be validated.

## Input Mapping

Primary C++ owner: `AMGSPlayerController`

Supporting asset work:

- Enhanced Input mapping context
- Input actions for movement, interact, and pause

Phase 1 responsibilities:

- Register the default input mapping context for the local player.
- Route input actions to the character or controller cleanly.
- Support keyboard/mouse and controller at a basic level.
- Own pause input in the controller.
- Route movement and interaction to the possessed `AMGSCharacter`.

Required input actions:

- `IA_Move`
- `IA_Interact`
- `IA_Pause`

Required keyboard/mouse bindings:

- Move: `WASD`
- Interact: `E`
- Pause: `Esc`

Required controller bindings:

- Move: left stick
- Interact: gamepad face button bottom
- Pause: start/menu button

Input ownership:

- Controller adds mapping contexts.
- Controller handles pause.
- Character handles movement and interaction commands.

## Pause Flow

Primary C++ owner: `AMGSPlayerController`

Supporting UI work:

- Minimal UMG widget Blueprint derived from `UUserWidget`

Phase 1 responsibilities:

- Toggle paused and unpaused gameplay.
- Switch input mode appropriately between gameplay and UI.
- Show or hide a minimal pause overlay.

Required pause behavior:

- Pressing pause while unpaused pauses the game.
- Pressing pause while paused resumes the game.
- A minimal visible pause state exists.
- Mouse cursor is visible while paused.
- Gameplay movement must not continue while paused.

Pause widget ownership:

- `AMGSPlayerController` owns the pause widget class reference.
- `AMGSPlayerController` creates the pause widget during `BeginPlay`.
- `AMGSPlayerController` adds the pause widget to the viewport.
- `AMGSPlayerController` keeps the pause widget hidden while gameplay is running.
- `AMGSPlayerController` shows the existing pause widget when gameplay pauses.
- `AMGSPlayerController` hides the existing pause widget when gameplay resumes.
- `AMGSCharacter` must not create, own, or directly manipulate the pause widget.
- `AMGSGameMode` must not create, own, or directly manipulate the pause widget.

Pause widget contents:

- A visible paused-state label, such as `PAUSED`.
- No additional visible menu entries.

Pause widget out of scope:

- Save menu
- Load menu
- Options menu
- Codec menu
- Inventory menu
- Restart / continue flow
- Quit confirmation flow

Pause widget behavior:

- The widget is created during `AMGSPlayerController::BeginPlay`.
- The widget is not recreated every pause toggle.
- The widget must not own behavior.
- Resume is handled only by pressing `IA_Pause` again.
- Missing widget class must not crash; the controller still pauses and logs that no widget class is assigned.
- If the widget is missing, Phase 1 validation uses a log or debug display to confirm pause state, but full validation remains failed until `WBP_Phase01Pause` is assigned.

## Game Mode Flow

Primary C++ owner: `AMGSGameMode`

Phase 1 responsibilities:

- Define the default player pawn class.
- Define the default player controller class.
- Own basic prototype startup assumptions.
- Ensure the prototype map does not depend on manually placing a possessed pawn.

Required default classes:

- Default pawn: `BP_MGSCharacter`
- Player controller: `BP_MGSPlayerController`
- Game mode: `BP_MGSGameMode`

## Logging And Debugging

- Use `LogMGS` for Phase 1 gameplay logs.
- Log when the prototype player initializes.
- Log when interaction target changes.
- Log when an interactable is activated.
- Log pause and resume transitions.

Required debug display:

- Current interaction target name.
- Whether pause is active.

## Implementation Order

1. Confirm project builds before edits.
2. Stabilize `AMGSCharacter` as the prototype player.
3. Set the Phase 1 movement and camera baseline.
4. Add interaction contract and test interactable.
5. Route Enhanced Input actions.
6. Add pause flow.
7. Configure game mode defaults.
8. Create `L_Phase01_CorePrototype` and configure it as the graybox validation map.
9. Run the testing checklist.

## Out Of Scope For Phase 1

- Guard AI
- Stealth detection
- Alert / evasion state machine
- Combat
- Inventory
- Soliton Radar
- Codec
- Save / load
- Boss encounters
- Final animation work
- Final UI art
- Production map layout

## Testing Checklist

- Project builds successfully.
- `L_Phase01_CorePrototype` opens without errors.
- `L_Phase01_CorePrototype` uses `BP_MGSGameMode`.
- `BP_MGSGameMode` uses `BP_MGSPlayerController`.
- `BP_MGSGameMode` uses `BP_MGSCharacter`.
- Play-in-editor spawns the player at a valid player start.
- The map does not require a manually placed possessed pawn.
- `AMGSCharacter`, `AMGSPlayerController`, and `AMGSGameMode` are concrete C++ classes.
- Missing input mapping context or input action references log clear `LogMGS` warnings or errors without crashing.
- Possessing a pawn that is not `AMGSCharacter` does not crash and logs once.
- Keyboard `WASD` movement works.
- Controller left-stick movement works.
- Movement is camera-relative.
- Analog movement magnitude affects movement speed.
- The character rotates to face movement direction.
- Jump input is not bound in `IMC_Phase01`.
- Player-controlled camera look is not bound in `IMC_Phase01`.
- Camera follows the player through the character camera boom.
- Camera collision is disabled.
- Camera lag is disabled.
- The player remains visible in the open area, narrow passage, L-shaped wall, and obstacle layout.
- The camera shows the player, nearby walls, and the test interactable when the player is near the interactable.
- The player cannot pass through graybox walls.
- The player slides along a wall while moving diagonally.
- The player can walk around a corner.
- The player can approach the interactable without collision blocking the interaction test.
- The forward sphere trace detects the test interactable.
- Only one active interactable is tracked at a time.
- Disabled interactables do not show prompts and do not activate.
- Destroyed or invalid interaction targets are cleared safely.
- Repeated interact presses on a valid target are allowed.
- Pressing `E` activates the interactable on keyboard.
- Pressing gamepad face button bottom activates the interactable on controller.
- Interaction activation logs `Interactable activated`.
- Interaction activation toggles the test interactable activated state.
- Prompt state is visible in debug output.
- Current interaction target name is visible in debug output.
- Interaction activation is ignored while paused.
- Pressing `Esc` pauses the game.
- Pressing `Esc` again resumes the game.
- Pressing start/menu pauses and resumes the game on controller.
- `IA_Pause` works while paused.
- Gameplay movement stops while paused.
- Interaction activation is blocked while paused.
- Pause does not destroy or reset the current interaction target.
- Mouse cursor is visible while paused.
- `WBP_Phase01Pause` appears while paused.
- `WBP_Phase01Pause` hides while gameplay is running.
- `WBP_Phase01Pause` contains only a visible `PAUSED` label.
- Missing pause widget class does not crash and logs a clear warning.
- Pause state is visible in debug output.
