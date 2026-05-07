# Metal Gear Solid Recreation - Development Phases

This document lists the major gameplay features and systems to implement in order. The goal is not a perfect recreation, but a gameplay-focused Unreal Engine version inspired by the original Metal Gear Solid.

## Phase 1 - Core Prototype

- Player character
- Basic top-down / third-person stealth camera
- Basic movement
- Basic collision
- Basic interaction prompts
- Test map / graybox room
- Input mapping
- Basic pause flow
- Basic game mode flow

## Phase 2 - Player Movement And Actions

- Walk and run movement
- Crouch
- Crawl
- Wall press
- Wall sliding
- Corner peeking
- Wall knocking
- First-person look mode
- Contextual ladder use
- Rappelling
- Contextual crawl spaces / vents
- Basic swimming / water movement
- Weapon-equipped movement behavior
- Basic player animation states

## Phase 3 - Stealth Detection Foundation

- Guard vision cones
- Guard hearing
- Suspicious sound investigation
- Noisy surface detection
- Loud action detection
- Player visibility states
- Cover and line-of-sight blocking
- Crawl hiding behavior
- Cardboard box hiding behavior
- Hiding spot detection
- Environmental detection traces
- Footprints
- Blood trails
- Cold / sneezing alerts
- Detection indicators

## Phase 4 - Guard AI And Alert Loop

- Patrol routes
- Genome soldier enemy type
- Idle guard behavior
- Suspicion state
- Alert state
- Evasion state
- Search behavior
- Reinforcement behavior
- Radio call behavior
- HQ response behavior
- Room lockdown behavior
- Guard return-to-patrol behavior
- Guard awareness scaling by difficulty
- Forced alert encounters
- Wolf enemy behavior
- Enemy perception debugging tools

## Phase 5 - Combat Foundation

- Player health
- Max health progression
- Enemy health
- Hand-to-hand attacks
- Punch combo
- Grab / choke
- Throw
- Basic weapon aiming
- Basic weapon firing
- Soft auto-aim
- Enemy damage reactions
- Player damage reactions
- Knockout state
- Death state
- Ration auto-use
- Boss health bars
- Special Game Over conditions
- Game over flow

## Phase 6 - Weapons, Items, And Equipment

- Weapon inventory
- Item inventory
- Weapon quick-cycle
- Item quick-cycle
- Weapon selection menu
- Item selection menu
- Ammo counts
- Item counts
- Ammo capacity progression
- Key items
- Rations
- Cigarettes
- Scope
- Thermal goggles
- Night vision goggles
- Body armor
- Gas mask
- Mine detector
- Camera
- Bandage
- Cold medicine
- Diazepam
- Ketchup
- PAL key
- Rope
- Cardboard boxes
- SOCOM
- FAMAS
- PSG-1
- Stinger
- Nikita
- Chaff grenades
- Stun grenades
- Grenades
- C4
- Claymores
- Mine detection and collection
- Frozen ration behavior
- New Game+ special items

## Phase 7 - HUD And Menus

- Life gauge
- O2 gauge
- Soliton Radar
- Radar enemy dots
- Radar vision cones
- Radar jammed state
- Weapon display
- Item display
- Alert countdown
- Evasion countdown
- In-game indicators
- Enemy alert icons
- Boss life gauge display
- Interaction prompts
- Inventory overlays
- Codec screen
- Codec memory window
- Pause menu
- Game over screen

## Phase 8 - Security And Environment Systems

- Surveillance cameras
- Gun cameras
- Infrared laser tripwires
- Doors and keycard access
- Elevators
- Cargo truck travel
- Gas rooms
- Water / drowning areas
- Electric floors
- Claymore mine hazards
- Timer bomb hazard
- Security lockdowns
- Room transitions
- Area-specific radar restrictions
- Environmental hazards

## Phase 9 - World Progression

- Shadow Moses area structure
- Area connections
- Cargo Dock
- Heliport
- Tank Hangar 1F
- Tank Hangar B1 / Cells
- Tank Hangar B2 / Armory
- Armory
- Canyon
- Nuclear Warhead Storage Building 1F
- Nuclear Warhead Storage Building B1
- Nuclear Warhead Storage Building B2
- Commander Room
- Caves
- Underground Passage
- Communications Tower A
- Communications Tower roof
- Communications Tower wall
- Communications Tower B
- Snowfield
- Blast Furnace
- Cargo Elevator
- Warehouse
- Underground Base
- Escape Route
- Progression gates
- Backtracking routes
- PAL key temperature puzzle
- Story-triggered access changes

## Phase 10 - Codec, Dialogue, And Save Systems

- Codec contact list
- Codec frequencies
- Manual codec dialing
- Incoming codec calls
- Context-sensitive codec calls
- Codec pause behavior
- Hidden codec calls
- Mei Ling save flow
- Manual save system
- Load / continue flow
- Checkpoint tracking
- Mission log
- Story dialogue triggers
- Cutscene trigger flow
- Cutscene input lock
- Dialogue portraits / presentation
- Voice acting support
- Tutorial and hint calls
- Special codec events

## Phase 11 - Boss Encounters

- Revolver Ocelot
- M1 Tank
- Cyborg Ninja
- Psycho Mantis
- Sniper Wolf first fight
- Hind D
- Sniper Wolf second fight
- Vulcan Raven second fight
- Metal Gear REX
- Liquid Snake fistfight
- Jeep chase finale
- Boss health rules
- Boss phase transitions
- Boss rewards
- Boss-specific arena hazards

## Phase 12 - Story Progression And Special Sequences

- Opening infiltration flow
- Major story beats
- Capture sequence
- Torture sequence
- Torture submit / resist branch
- Torture hard Game Over
- Cell escape options
- Hostage protection fail states
- Meryl / Otacon ending branch
- Key character encounters
- Fourth-wall-inspired moments
- New Game+ unlocks
- Ending flow
- Credits flow

## Phase 13 - Difficulty, Ranking, And Replay Systems

- Difficulty presets
- Detection sensitivity scaling
- Enemy damage scaling
- Player damage scaling
- Item carry limits
- Boss difficulty differences
- Ranking system
- Clear-time tracking
- Kills tracking
- Alerts tracking
- Continues tracking
- Saves tracking
- Special unlock rewards
- VR Training mode
- VR Missions-inspired challenge mode
- Camera photo mode
- Ghost photo collectibles
- Photo viewer

## Phase 14 - Presentation And Polish

- Final camera tuning
- Fixed camera angle polish
- Wall-press camera polish
- First-person camera polish
- Final animation polish
- Sound effects
- Music states
- Alert sting
- Codec voice presentation
- Cutscene presentation
- Combat feedback
- UI polish
- Environmental ambience
- Visual effects
- Screen transitions
- Loading flow
- Accessibility options
- Controller polish
- Keyboard and mouse polish

## Phase 15 - Testing And Release Readiness

- Full gameplay smoke pass
- Area-by-area progression pass
- Stealth system validation
- Combat system validation
- Boss encounter validation
- Inventory and item validation
- Save / load validation
- Difficulty validation
- Performance pass
- Packaging pass
- Known issues list
- Release candidate checklist
