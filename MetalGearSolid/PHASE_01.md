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

## Phase 1 Definition Of Done

Phase 1 is complete when pressing Play in the prototype map starts a playable session where:

- The project uses the intended MGS game mode, player controller, and player character.
- The player spawns at a valid player start.
- The player moves on the horizontal plane with keyboard and controller input.
- The camera presents the room from the defined Phase 1 stealth camera angle.
- The player collides with walls and simple props.
- The player can face one test interactable and trigger it with the interact input.
- An interaction prompt state is exposed from C++ and validated through debug output.
- Pause toggles gameplay on and off.
- The prototype can be validated from a single graybox map.

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

Pause while paused:

- Pause input must work both when gameplay is running and when the game is paused.
- `IA_Pause` must execute while paused so it can resume the game.
- Paused gameplay must stop movement and interaction activation.
- Pause must not destroy or reset the current interaction target.

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

Camera stability:

- Camera collision must be disabled by default for Phase 1 to avoid unstable framing in narrow graybox spaces.
- Camera distance, height, and pitch must remain editor-tunable.
- Free-orbit camera behavior is disabled.

Map and game mode defaults:

- The Phase 1 prototype map must explicitly use `BP_MGSGameMode`.
- `BP_MGSGameMode` must use `BP_MGSPlayerController` and `BP_MGSCharacter`.
- The map must not rely on a manually placed possessed pawn for normal validation.
- A manually placed possessed pawn is not used for normal Phase 1 validation.

Prompt and pause UI:

- Prompt UI is not built in Phase 1.
- Pause UI is built as a visible placeholder widget.
- Missing pause UI class must not prevent pause state from functioning, but it fails the full Phase 1 validation checklist.

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

Phase 1 success criteria:

- The player can spawn reliably.
- The player can move in the test map.
- The player can traverse the open area, narrow passage, L-shaped wall, and obstacle layout without camera or collision failure.
- Collision with world geometry is predictable.
- The player can trigger one nearby interactable.

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
- Avoid free-orbit camera behavior.
- Expose enough camera tuning to adjust height, distance, pitch, and lag in the editor.
- Remove free-orbit camera behavior from the default Phase 1 camera path.
- Keep camera collision disabled by default for stable Phase 1 framing.

Phase 1 success criteria:

- The player remains visible while traversing the open area, narrow passage, L-shaped wall, and obstacle layout.
- The camera shows the player, nearby walls, and the test interactable at the same time when the player is near the interactable.

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

Phase 1 success criteria:

- Keyboard, mouse, and controller movement routes are clear.
- Movement has one obvious source of truth.

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

Phase 1 success criteria:

- The player cannot pass through graybox walls.
- Movement along walls behaves predictably.
- Collision setup supports Phase 1 movement and interaction validation.

Required collision checks:

- Walk directly into a wall.
- Slide along a wall while moving diagonally.
- Walk around a corner.
- Approach an interactable without physically blocking the interaction test.

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

Phase 1 success criteria:

- The player can approach a test object and receive a prompt state.
- Pressing the interact input logs `Interactable activated` and toggles the test interactable activated state.

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

Phase 1 success criteria:

- Pressing Play starts in the test map with the prototype player.
- The test map can validate every Phase 1 feature.

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

Phase 1 success criteria:

- Move input works.
- Interact input reaches the interaction system.
- Pause input reaches the pause flow.

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

Phase 1 success criteria:

- Pause input stops gameplay.
- Pause input or menu action resumes gameplay.
- Player input does not leak unexpectedly while paused.

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

Phase 1 success criteria:

- The correct player pawn and controller are used when the test map starts.
- Startup behavior is predictable.
- Phase 1 systems can be tested without manual setup every play session.

Required default classes:

- Default pawn: `BP_MGSCharacter`
- Player controller: `BP_MGSPlayerController`
- Game mode: `BP_MGSGameMode`

## Recommended C++ Types

- `AMGSCharacter` for player pawn behavior.
- `AMGSPlayerController` for input, pause, and player-owned UI flow.
- `AMGSGameMode` for default classes and prototype game startup.
- `AMGSInteractableActor` for simple interactable world objects.

## C++ Ownership Rule

Phase 1 must be implemented with C++ as the behavioral source of truth.

C++ owns:

- Player movement rules.
- Camera behavior.
- Input routing.
- Interaction detection.
- Interaction activation.
- Prompt state.
- Pause state.
- Game mode startup flow.
- Validation logging.

Blueprints own:

- Tunable numeric defaults.
- Meshes, materials, and simple visual placeholders.
- Input action and mapping context asset assignment.
- Widget class assignment.
- Map placement.
- Test-map-only presentation details.

## Logging And Debugging

- Use `LogMGS` for Phase 1 gameplay logs.
- Log when the prototype player initializes.
- Log when interaction target changes.
- Log when an interactable is activated.
- Log pause and resume transitions.

Required debug display:

- Current interaction target name.
- Whether pause is active.

## Phase 1 Validation Checklist

- Project builds successfully.
- Test map opens without errors.
- Play-in-editor spawns the correct player character.
- Player movement works.
- Camera follows the player clearly.
- Player blocks against graybox collision.
- Interact prompt state appears for a test object.
- Interact input triggers the test object.
- Pause toggles on and off.
- Game mode uses the intended pawn and controller.

## Implementation Order

1. Confirm project builds before edits.
2. Stabilize `AMGSCharacter` as the prototype player.
3. Set the Phase 1 movement and camera baseline.
4. Add interaction contract and test interactable.
5. Route Enhanced Input actions.
6. Add pause flow.
7. Configure game mode defaults.
8. Create `L_Phase01_CorePrototype` and configure it as the graybox validation map.
9. Run the Phase 1 validation checklist.

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
