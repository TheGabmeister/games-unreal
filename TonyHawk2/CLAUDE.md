# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

Tony Hawk's Pro Skater 2 recreation in Unreal Engine 5.7. Single C++ module (`TonyHawk2`). Enhanced Input System is the input framework. Phase 1 (core skating movement) is implemented with a placeholder capsule pawn — SPEC.md has the full gameplay systems spec, PHASES.md has the 17-phase implementation roadmap, PLAN_01.md has the Phase 1 implementation details.

## Build

Engine is installed at `C:\Program Files\Epic Games\UE_5.7`.

```
# Build (Development Editor — the day-to-day target)
"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" TonyHawk2Editor Win64 Development "C:\dev\games-unreal\TonyHawk2\TonyHawk2.uproject" -waitmutex

# Build (Game, various configs: Debug | DebugGame | Development | Test | Shipping)
"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" TonyHawk2 Win64 Development "C:\dev\games-unreal\TonyHawk2\TonyHawk2.uproject" -waitmutex

# Clean
"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Clean.bat" TonyHawk2Editor Win64 Development "C:\dev\games-unreal\TonyHawk2\TonyHawk2.uproject" -waitmutex
```

The .code-workspace file has VS Code tasks for all platform/config combinations (Win64, Android × Debug through Shipping).

**Live Coding mutex issue**: If UBT fails with "Unable to build while Live Coding is active" but the editor is closed, release the stale Windows mutex:
```powershell
$exePath = "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe"
$mutexName = "Global\LiveCoding_" + ($exePath -replace '[\\\/:]', '+')
try { $m = [System.Threading.Mutex]::OpenExisting($mutexName); $m.Close(); $m.Dispose() } catch {}
```

## Module Dependencies

Declared in `Source/TonyHawk2/TonyHawk2.Build.cs`:
- **Public**: Core, CoreUObject, Engine, InputCore, EnhancedInput
- Add Slate/SlateCore when UI work begins; add OnlineSubsystem if multiplayer needs networking

## Architecture

All C++ lives under `Source/TonyHawk2/` (flat structure). Core classes:

- `ATH2SkaterPawn` — player pawn. Capsule + board mesh with state-driven color (no skeletal mesh yet). Manages skater state machine (Idle/Pushing/Crouching/Airborne/Landing/Bailing), ollie charge, advanced launch input buffering (Nollie/No Comply/Boneless), switch stance, and audio playback.
- `UTH2MovementComponent` — custom `UPawnMovementComponent` for skating physics. Handles ground movement with auto-kick, steering, braking, surface alignment via line traces, air physics with hangtime modifier, ramp/vert transitions, wall collision, and Big Drop detection.
- `ATH2GameMode` — sets DefaultPawnClass and PlayerControllerClass. Uses `ConstructorHelpers::FClassFinder` to pick up Blueprint subclasses (`BP_SkaterPawn`, `BP_PlayerController`) at `/Game/Phase1/Blueprints/`.
- `ATH2PlayerController` — Enhanced Input setup. Binds `IMC_Skating` mapping context and routes input actions to the skater pawn.

Target configs: `Source/TonyHawk2.Target.cs` (Game) and `Source/TonyHawk2Editor.Target.cs` (Editor).

## Editor Plugins

- **SoftUEBridge** — CLI-driven editor automation via `soft-ue-cli` (pip package). Connects on port 8080. Use for asset creation, actor spawning, property setting, screenshots. FBX import crashes the bridge unless deferred via `unreal.register_slate_post_tick_callback`. GameMode blueprint compilation also crashes — use C++ `ConstructorHelpers` instead.
- **McpAutomationBridge** — MCP server built into the plugin (Native MCP Transport on port 3000). Configured in `.mcp.json`. No external dependencies needed.
- **PythonScriptPlugin** — enables `run-python-script` via SoftUEBridge for operations the CLI doesn't cover natively.

## Content Layout

- `/Game/Phase1/Meshes/` — debug skatepark static meshes (SM_Ground, SM_QuarterPipeA/B, SM_HalfpipeLeft/Right/Floor, SM_KickerA/B, SM_BankA/B, SM_WallNorth/South/East/West) with materials (M_Concrete, M_Asphalt, M_Ramp, M_Wall)
- `/Game/Phase1/Audio/` — synthesized SFX (SFX_WheelRoll, SFX_OlliePop, SFX_LandingThud, SFX_BailImpact, SFX_WindAir)
- `/Game/Phase1/Input/` — Enhanced Input assets (IMC_Skating, IA_Move, IA_Ollie, IA_Brake, IA_SwitchStance)
- `/Game/Phase1/Blueprints/` — BP_SkaterPawn, BP_PlayerController (with input/audio asset references)
- `/Game/Phase1/Maps/L_DebugPark` — debug skatepark level (default map)

## Asset Pipeline

- **3D models**: Blender Python scripts under `Assets/Blender/` → FBX export → import into UE. Run via `blender --background --python <script>.py`.
- **Audio SFX**: Generated with ffmpeg synthesis filters (`anoisesrc`, `sine`, filters). WAV output at `Assets/Audio/SFX/`, imported as USoundWave.
- **FBX import via bridge**: Use deferred Slate callback pattern to avoid TaskGraph recursion crash. See `Assets/Blender/import_fbx_deferred.py`.

## Design Documents

- **SPEC.md** — full gameplay systems spec (trick system, scoring formulas, stat tables, level goals, economy, multiplayer modes). Source of truth for game mechanics.
- **PHASES.md** — 17-phase implementation plan. Phases 1–6 = vertical slice (movement through special tricks). Phases 7–17 = remaining systems and content expansion.
- **PLAN_01.md** — Phase 1 implementation plan (9 steps covering skating movement, ollie, air physics, camera, debug park).

When implementing a phase, cross-reference both documents: PHASES.md for what to build, SPEC.md for exact values (scoring formulas, stat distributions, thresholds).

## Engine Config Notes

- Enhanced Input is the input system — do not use legacy input. Input Mapping Contexts and Input Actions are editor-authored assets assigned in Blueprint, not constructed at runtime via NewObject.
- Rendering: DX12/SM6, ray tracing enabled, Substrate materials, Virtual Shadow Maps. Target hardware is desktop maximum.
- AnimBPs are manually authored in the editor — do not attempt to construct Animation Blueprint graphs programmatically.
- Live Coding is disabled for this project (`Config/DefaultEditorPerProjectUserSettings.ini`). Use full rebuilds via UBT.
- Default GameMode (`TH2GameMode`) and default map (`L_DebugPark`) are set in `Config/DefaultEngine.ini`.
