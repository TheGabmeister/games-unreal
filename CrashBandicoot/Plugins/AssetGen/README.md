# AssetGen

UE5 editor plugin for procedural asset generation via AI agents and CLI scripts.

Copy `Plugins/AssetGen/` into any UE5 project's `Plugins/` folder to use.

## Editor Plugin

A Slate window (**Window > AssetGen**) with buttons for one-click Blueprint, Material, Level, Input Action, and GameMode creation. See the source at `Source/AssetGen/`. Dependencies: `UnrealEd`, `ToolMenus`, `LevelEditor`, `EnhancedInput`, `InputCore`.

## Scripts

All scripts live in `Scripts/`. Each follows the same pattern: a Python script is the source of truth, CLI tools do the heavy lifting, and the output is a standard format UE can import (FBX, WAV, PNG).

### generate_sounds.py — Sound Effects

Synthesizes placeholder SFX (jump, coin, hit) as WAV files using only Python stdlib.

```
python Scripts/generate_sounds.py [output_dir] [sound_filter]
```

- **Output:** `SFX_<Name>.wav` (16-bit PCM, 44100 Hz, mono)
- **Dependencies:** None (Python stdlib only)
- **Import:** Drag WAV files into UE Content Browser

### generate_sprites.py — 2D Sprites & Icons

Generates SVG sprites and converts to PNG via Inkscape.

```
python Scripts/generate_sprites.py [output_dir] [sprite_filter] [--size=256]
```

- **Output:** `SPR_<Name>.svg` + `SPR_<Name>.png`
- **Dependencies:** Inkscape
- **Import:** Drag PNG files into UE Content Browser

### generate_music.py — Music

Composes MIDI programmatically and renders to WAV via FluidSynth.

```
python Scripts/generate_music.py [output_dir] [track_filter]
```

- **Output:** `MUS_<Name>.mid` + `MUS_<Name>.wav`
- **Dependencies:** FluidSynth + a GM SoundFont, FFmpeg (optional, for loudness normalization)
- **Import:** Drag WAV files into UE Content Browser

### generate_meshes.py — Static Meshes

Generates 3D meshes procedurally in Blender (primitives, metaballs, recursive branching) and exports as FBX.

```
blender --background --python Scripts/generate_meshes.py -- [output_dir] [mesh_filter]
```

- **Output:** `SM_<Name>.fbx` (one material per mesh, UE scale/axis conventions)
- **Dependencies:** Blender
- **Import:** Drag FBX into UE Content Browser as Static Mesh

### generate_monster_rig.py — Rigged & Animated Skeletal Mesh

Generates a monster mesh (metaball body + detail parts), rigs it with a 26-bone skeleton, skins it with distance-based vertex weights, creates animations, and bakes a procedural texture.

```
blender --background --python Scripts/generate_monster_rig.py -- [output_dir]
```

- **Output:**
  - `SK_Monster.fbx` — Skeletal mesh (mesh + skeleton, bind pose)
  - `A_Monster_Idle.fbx` — 2s idle loop (breathing, head scan, tail sway)
  - `A_Monster_Attack.fbx` — 1.5s right arm overhead smash
  - `A_Monster_Roar.fbx` — 2s battle cry (jaw open, arms spread)
  - `T_Monster.png` — 256x256 baked diffuse texture
- **Dependencies:** Blender (Cycles for texture baking)
- **Import:** Import `SK_Monster.fbx` as Skeletal Mesh, then each `A_Monster_*.fbx` as Animation targeting that skeleton. Create a material from `T_Monster.png` and assign it.

### generate_crab.py — Rigged Crab Enemy with Animations

Generates a crab enemy (flat ellipsoid body, claws, legs, eye stalks), rigs it with a 4-bone skeleton (root, body, claw_l, claw_r), skins with distance-based weights, and creates 3 animations.

```
blender --background --python Scripts/generate_crab.py -- [output_dir]
```

- **Output:**
  - `SK_Crab.fbx` — Skeletal mesh (mesh + skeleton, bind pose)
  - `A_Crab_Idle.fbx` — 2s claw pinch cycle with body sway
  - `A_Crab_Walk.fbx` — 1.3s side-to-side shuffle
  - `A_Crab_Death.fbx` — 1s flip onto back, claws curl
- **Dependencies:** Blender
- **Import:** Import `SK_Crab.fbx` as Skeletal Mesh (creates `SK_Crab_Skeleton` automatically), then each `A_Crab_*.fbx` as Animation targeting that skeleton (`-Type=Animation -Skeleton=/Game/Enemies/Crab/SK_Crab_Skeleton`).

### blender_anim.py — Mannequin Animations

Procedural animation generator for UE's Manny skeleton. Driven by an `ANIMATIONS` dict — edit the script to change animations, then re-run.

```
blender --background --python Scripts/blender_anim.py -- <rig_fbx> [output_dir] [anim_filter]
```

- **Rig:** `Rig/SKM_Manny_Simple.FBX` (88-bone UE Mannequin skeleton, included)
- **Output:** `A_Manny_<Name>.fbx` (animation-only FBX per animation)
- **Dependencies:** Blender
- **Import:** Import each FBX as Animation, point skeleton to project's mannequin skeleton
- **Bone reference:** See `Scripts/ue5-animate.md` for the complete verified axis reference for all 88 bones

## Tool Dependencies

| Tool | Used by | Purpose |
|------|---------|---------|
| Python 3.x | All scripts | Script runtime |
| Blender | generate_meshes, generate_monster_rig, blender_anim | 3D modeling, rigging, animation, texture baking |
| Inkscape | generate_sprites | SVG to PNG conversion |
| FluidSynth + SoundFont | generate_music | MIDI to WAV rendering |
| FFmpeg | generate_music | Audio normalization (optional) |

Tool paths are configured as constants at the top of each script. Update them if your install locations differ.

## FBX Export Conventions

All Blender-to-UE FBX exports model facing **-Y in Blender** (Blender's default forward). Before export, rotate the armature +90° around Z (`arm_obj.rotation_euler[2] = math.pi / 2`, do NOT apply) so models face **+X forward in UE** (UE's default forward).

**Exception:** Humanoid models retargeted to the Unreal Mannequin skip this rotation, since the Mannequin faces +Y.

**Static meshes:**
```python
apply_unit_scale=True
apply_scale_options='FBX_SCALE_ALL'
axis_forward='-Y'
axis_up='Z'
```

**Rigged skeletal meshes** (with armature):
```python
apply_unit_scale=False
apply_scale_options='FBX_SCALE_NONE'
axis_forward='-Y'
axis_up='Z'
add_leaf_bones=False
```

`FBX_SCALE_ALL` causes double unit conversion with UE's skeletal mesh importer — use `FBX_SCALE_NONE` for anything with an armature.

**AssetImport commandlet note:** The commandlet sets `bConvertSceneUnit = true` so Blender's meter-scale FBX files are correctly converted to UE centimeters. Without this, static meshes import 100x too small.

## Other Files

- `Rig/SKM_Manny_Simple.FBX` — UE Mannequin skeleton (source rig for animations, do not rename bones)
- `Scripts/ue5-animate.md` — Complete bone axis reference for the Manny skeleton (verified by visual testing)
- `Scripts/blender_mcp_bridge.py` — MCP bridge for live Blender communication
