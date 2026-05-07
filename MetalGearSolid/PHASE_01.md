# Phase 01 - Core Prototype C++ Plan

This document defines the C++ implementation contract for Phase 1. It is still high-level and contains no code, but it should be specific enough that an implementation pass does not need to invent major behavior.

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
- The camera presents the room from a readable stealth-game angle.
- The player collides with walls and simple props.
- The player can face or approach one test interactable and trigger it.
- An interaction prompt state is exposed to UI or Blueprint.
- Pause toggles gameplay on and off.
- The prototype can be validated from a single graybox map.

## Phase 1 Design Decisions

- The prototype player remains an Unreal `ACharacter`, not a custom pawn.
- Movement is character-movement based and constrained to normal walking on the floor.
- The default camera target is a raised third-person / overhead stealth camera, not a free orbit action camera.
- Camera-relative movement is the default for Phase 1 unless it causes readability problems in the graybox map.
- Jumping is not part of the intended gameplay foundation. It may remain temporarily for template compatibility, but it is not a Phase 1 acceptance requirement.
- Interaction is intentionally simple: one focused interact action, one prompt state, one test response.
- Phase 1 should favor editor-tunable values over hard-coded final-feel values.
- Blueprint assets may configure defaults, but core ownership and behavior should be represented in C++.

## Existing Starting Point

- `AMGSCharacter` already exists as the current player character base.
- `AMGSPlayerController` already manages Enhanced Input mapping contexts.
- `AMGSGameMode` already exists as the current game mode base.
- `LogMGS` already exists as the project log category.
- The project is currently close to the Unreal third-person template, so Phase 1 should reshape these classes toward a stealth prototype foundation.

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
- `UMGSInteractionComponent`
- `UMGSPauseWidget` only if a C++ widget base is useful for pause state ownership

Asset or Blueprint types expected for Phase 1:

- Prototype player Blueprint derived from `AMGSCharacter`
- Prototype game mode Blueprint derived from `AMGSGameMode`, if defaults are easier to assign in editor
- Prototype player controller Blueprint derived from `AMGSPlayerController`, if input assets are assigned in editor
- Prototype map for Phase 1 validation
- Enhanced Input mapping context
- Enhanced Input actions for move, look, interact, and pause
- Minimal prompt / pause widget assets if UI is built in UMG

## Player Character

Primary C++ owner: `AMGSCharacter`

Phase 1 responsibilities:

- Represent the player pawn used in the prototype.
- Own the capsule collision and basic movement component settings.
- Own the camera boom and follow camera components.
- Expose prototype tuning values for movement speed, acceleration, braking, rotation behavior, camera distance, camera height, camera pitch, and camera lag.
- Provide callable movement functions that can be driven by player input or simple test hooks.
- Keep jump support only if useful for template compatibility; it is not a core gameplay goal for this project.
- Provide an entry point for interact input.
- Own or reference the interaction component.

Phase 1 success criteria:

- The player can spawn reliably.
- The player can move in the test map.
- Movement feels controlled enough for future stealth work.
- Collision with world geometry is predictable.
- The player can trigger one nearby interactable.

Required exposed tuning categories:

- `Movement`
- `Camera`
- `Interaction`

Required Blueprint-facing functions:

- Move command
- Look / camera command, if used
- Interact command
- Pause remains controller-owned, not character-owned

## Camera

Primary C++ owner: `AMGSCharacter`

Phase 1 responsibilities:

- Provide a basic top-down / third-person stealth camera mode.
- Keep the camera readable for room-scale navigation.
- Avoid camera behavior that fights fixed-angle or radar-like stealth gameplay later.
- Expose enough camera tuning to adjust height, distance, pitch, and lag in the editor.
- Disable or reduce free-orbit assumptions from the third-person template if they make the game feel like an action template rather than a stealth prototype.
- Keep camera collision enabled unless it causes unstable framing in the graybox map.

Phase 1 success criteria:

- The player remains visible and centered enough during movement.
- The camera gives useful spatial awareness in a simple room.
- The camera setup can evolve into contextual stealth cameras in later phases.

Baseline camera target:

- Raised behind/above the player.
- Angled downward.
- Tunable in Blueprint/editor.
- Stable enough for narrow rooms and wall collision tests.

## Movement

Primary C++ owner: `AMGSCharacter`

Phase 1 responsibilities:

- Convert Enhanced Input movement values into world movement.
- Define whether movement is camera-relative or world-relative for the prototype.
- Configure baseline movement speed and rotation behavior.
- Keep movement simple enough to expand into walk, run, crouch, crawl, and wall actions in Phase 2.
- Prefer character-facing movement over strafing unless the camera-relative prototype feels wrong.
- Keep all Phase 1 movement in a single normal locomotion state.

Phase 1 success criteria:

- Keyboard, mouse, and controller movement routes are clear.
- Movement has one obvious source of truth.
- Later movement states can be added without replacing the entire foundation.

Required input behavior:

- `Move` accepts a 2D input vector.
- Forward/back and right/left movement must work.
- Analog magnitude should affect movement naturally through Enhanced Input / Character Movement.
- No sprint, crouch, crawl, wall press, or combat movement modes in Phase 1.

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
- Leave room for future custom trace channels for sight, sound, interaction, and cover.

Phase 1 success criteria:

- The player cannot pass through graybox walls.
- Movement along walls behaves predictably.
- Collision setup is suitable for later cover, line-of-sight, and interaction traces.

Required collision checks:

- Walk directly into a wall.
- Slide along a wall while moving diagonally.
- Walk around a corner.
- Approach an interactable without physically blocking the interaction test.

## Interaction Prompts

Primary C++ owners:

- `UMGSInteractionComponent`
- `AMGSInteractableActor`
- `AMGSCharacter` as the component owner

Phase 1 responsibilities:

- Define the player-facing concept of an interact action.
- Define a basic way to detect nearby interactable objects.
- Define a basic prompt state that UI can read.
- Support simple test interactables in the graybox map.
- Keep prompt text generic and prototype-only.
- Support one active interactable at a time.
- Avoid building doors, pickups, terminals, or codec behavior yet.

Phase 1 success criteria:

- The player can approach a test object and receive a prompt state.
- Pressing the interact input can trigger a simple response.
- The system is general enough to become doors, elevators, pickups, and codec/save points later.

Required interaction behavior:

- Detect a nearby interactable using either overlap or short trace.
- Track the current best interactable.
- Expose whether an interaction prompt should be visible.
- Expose prompt text or an interaction label.
- Call the interactable when the player presses Interact.
- Log or visibly change the test interactable when activated.

Recommended interaction contract:

- Interactable has enabled / disabled state.
- Interactable exposes prompt text.
- Interactable exposes an activation event.
- Interaction component owns detection and current-target selection.

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
- Optional minimal lighting
- Optional debug labels or simple colored meshes

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
- One narrow passage.
- One corner or L-shaped wall.
- One simple obstacle.
- One test interactable.

## Input Mapping

Primary C++ owner: `AMGSPlayerController`

Supporting asset work:

- Enhanced Input mapping context
- Input actions for movement, look, interact, and pause

Phase 1 responsibilities:

- Register the default input mapping context for the local player.
- Route input actions to the character or controller cleanly.
- Keep input names aligned with future gameplay actions from `SPEC.md`.
- Support keyboard/mouse and controller at a basic level.
- Own pause input in the controller.
- Route movement and interaction to the possessed `AMGSCharacter`.

Phase 1 success criteria:

- Move input works.
- Look or camera-adjust input works if enabled for the prototype.
- Interact input reaches the interaction system.
- Pause input reaches the pause flow.

Required input actions:

- `IA_Move`
- `IA_Look`
- `IA_Interact`
- `IA_Pause`

Required keyboard/mouse bindings:

- Move: `WASD`
- Look: mouse, only if camera look remains enabled
- Interact: `E` or equivalent action key
- Pause: `Esc`

Required controller bindings:

- Move: left stick
- Look: right stick, only if camera look remains enabled
- Interact: face button
- Pause: start/menu button

Input ownership:

- Controller adds mapping contexts.
- Controller handles pause.
- Character handles movement and interaction commands.

## Pause Flow

Primary C++ owner: `AMGSPlayerController`

Supporting UI work:

- Minimal pause widget or placeholder pause state

Phase 1 responsibilities:

- Toggle paused and unpaused gameplay.
- Switch input mode appropriately between gameplay and UI.
- Show or hide a minimal pause overlay.
- Preserve clean handoff to future pause menu and codec systems.
- Keep pause independent from codec; codec is a later system.
- Avoid save/load/options menus in Phase 1.

Phase 1 success criteria:

- Pause input stops gameplay.
- Pause input or menu action resumes gameplay.
- Player input does not leak unexpectedly while paused.

Required pause behavior:

- Pressing pause while unpaused pauses the game.
- Pressing pause while paused resumes the game.
- A minimal visible pause state exists.
- Mouse cursor behavior is intentional while paused.
- Gameplay movement should not continue while paused.

## Game Mode Flow

Primary C++ owner: `AMGSGameMode`

Phase 1 responsibilities:

- Define the default player pawn class.
- Define the default player controller class.
- Own basic prototype startup assumptions.
- Keep future hooks available for mission start, respawn, Game Over, and area transitions.
- Ensure the prototype map does not depend on manually placing a possessed pawn unless explicitly chosen for testing.

Phase 1 success criteria:

- The correct player pawn and controller are used when the test map starts.
- Startup behavior is predictable.
- Phase 1 systems can be tested without manual setup every play session.

Required default classes:

- Default pawn: prototype `AMGSCharacter` class or Blueprint child.
- Player controller: `AMGSPlayerController` class or Blueprint child.
- Game mode: `AMGSGameMode` class or Blueprint child.

Future game flow hooks to leave room for:

- Mission start
- Area transition
- Checkpoint restore
- Player death
- Game Over

## Recommended C++ Types

- `AMGSCharacter` for player pawn behavior.
- `AMGSPlayerController` for input, pause, and player-owned UI flow.
- `AMGSGameMode` for default classes and prototype game startup.
- `AMGSInteractableActor` or similar for simple interactable world objects.
- `UMGSInteractionComponent` for player interaction detection and prompt state.
- `UMGSPauseWidget` or Blueprint widget class for the minimal pause overlay.

## Logging And Debugging

- Use `LogMGS` for Phase 1 gameplay logs.
- Log when the prototype player initializes.
- Log when interaction target changes, if useful during validation.
- Log when an interactable is activated.
- Log pause and resume transitions.

Optional debug display:

- Current interaction target name.
- Whether pause is active.
- Camera mode name.

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
8. Create or configure the graybox validation map.
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
