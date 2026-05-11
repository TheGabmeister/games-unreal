---
name: ue5-assetgen
description: Create, import, and manage Unreal Engine 5 assets using the AssetGen pipeline. Use when creating/importing meshes, sounds, textures, materials, or blueprints into UE; wiring up Blueprint properties; inspecting Blueprint contents. Supports both commandlet (editor closed) and REST API (editor open) workflows.
---

# UE5 AssetGen Pipeline

Two asset management interfaces, depending on whether the editor is open:

1. **REST API** (editor open) — `POST http://localhost:3000/api/call` via curl. Preferred for all operations when the editor is running.
2. **Commandlets** (editor closed or headless/CI) — `UnrealEditor-Cmd.exe` with `-run=<Commandlet>`.

## REST API (Editor Open)

The McpAutomationBridge plugin exposes a direct REST endpoint. Requires `bEnableNativeMCP=True` in `DefaultGame.ini` (already configured).

### Request/Response Format
```bash
curl -s --max-time <timeout> http://localhost:3000/api/call \
  -H "Content-Type: application/json" \
  -d '{"tool":"<tool_name>","arguments":{...}}'
```
Response: `{"success": true, "message": "...", "data": {...}}`

### Timeouts
- **Fast operations** (set property, get SCS, compile): `--max-time 5`
- **Mesh/texture import** (FBX, WAV, PNG): `--max-time 5` (fire-and-forget — import handler doesn't respond but operation succeeds)

### Animation Imports — Use Commandlet, Not REST
The REST API's import handler crashes on animation FBX files (no skeleton picker). Always use the `AssetImport` commandlet for animations:
```bash
MSYS_NO_PATHCONV=1 "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" \
  "c:\dev\CrashBandicoot\CB.uproject" -run=AssetImport \
  -SourceFile="path/to/A_Name.fbx" -DestPath=/Game/Enemies/Name \
  -Type=Animation -Skeleton=/Game/Enemies/Name/SK_Name_Skeleton -Replace \
  -stdout -unattended -nosplash
```
This works with the editor closed. The `-Skeleton` parameter is required.

### Blueprint Operations (manage_blueprint)

**Set inherited C++ component property** (e.g., assign mesh from ACharacter::Mesh):
```bash
curl -s --max-time 30 http://localhost:3000/api/call -H "Content-Type: application/json" \
  -d '{"tool":"manage_blueprint","arguments":{"action":"set_default","requestedPath":"/Game/Path/BP_Name","propertyName":"Mesh.SkeletalMeshAsset","value":"/Game/Path/SK_Name.SK_Name"}}'
```

**Set CDO property** (e.g., HitPoints, KnockbackForce):
```bash
curl -s --max-time 30 http://localhost:3000/api/call -H "Content-Type: application/json" \
  -d '{"tool":"manage_blueprint","arguments":{"action":"set_default","requestedPath":"/Game/Path/BP_Name","propertyName":"HitPoints","value":"3"}}'
```

**Get SCS component hierarchy:**
```bash
curl -s --max-time 30 http://localhost:3000/api/call -H "Content-Type: application/json" \
  -d '{"tool":"manage_blueprint","arguments":{"action":"get_scs","blueprintPath":"/Game/Path/BP_Name.BP_Name"}}'
```

**Set SCS component property** (Blueprint-added components only):
```bash
curl -s --max-time 30 http://localhost:3000/api/call -H "Content-Type: application/json" \
  -d '{"tool":"manage_blueprint","arguments":{"action":"set_scs_property","blueprintPath":"/Game/Path/BP_Name.BP_Name","componentName":"MyComp","propertyName":"PropName","value":"PropValue"}}'
```

### Asset Operations (manage_asset)

**Import FBX/WAV/PNG:**
```bash
curl -s --max-time 300 http://localhost:3000/api/call -H "Content-Type: application/json" \
  -d '{"tool":"manage_asset","arguments":{"action":"import","sourcePath":"c:/path/to/file.fbx","destinationPath":"/Game/Target/Folder"}}'
```

**Search assets:**
```bash
curl -s --max-time 30 http://localhost:3000/api/call -H "Content-Type: application/json" \
  -d '{"tool":"manage_asset","arguments":{"action":"search_assets","search_path":"/Game/Folder","filter":"SearchTerm"}}'
```

### Key Concepts
- **Inherited C++ components** (Mesh from ACharacter, CapsuleComponent, etc.): Use `set_default` with dot-path (`Mesh.SkeletalMeshAsset`). This sets the CDO property and persists through recompile.
- **SCS components** (added in Blueprint editor): Use `get_scs` / `set_scs_property`.
- **Asset paths**: Use `/Game/Folder/AssetName.AssetName` (with object name after dot) for asset references in values. Use `/Game/Folder/AssetName` (no dot suffix) for `requestedPath`.

## Commandlets (Editor Closed)

All commands require `MSYS_NO_PATHCONV=1` prefix and must use Bash (not PowerShell).

### Command Template
```bash
MSYS_NO_PATHCONV=1 "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "c:\dev\CrashBandicoot\CB.uproject" -run=<Commandlet> <params> -stdout -unattended -nosplash
```

### 1. AssetImport
```
-run=AssetImport
  -SourceFile="C:\path\to\file.fbx"
  -DestPath=/Game/Target/Folder
  -DestName=AssetName              # optional, defaults to filename
  -Type=StaticMesh|SkeletalMesh|Animation|Auto
  -Skeleton=/Game/Path/To/Skeleton # required for Animation type
  -Replace                         # optional: overwrite existing
```

### 2. BlueprintCreate
```
-run=BlueprintCreate
  -AssetPath=/Game/Blueprints/BP_Name
  -ParentClass=/Script/Module.ClassName
```

### 3. BlueprintEdit
```
-run=BlueprintEdit
  -AssetPath=/Game/Blueprints/BP_Name
  -Set="PropertyName=Value"        # repeatable
```
**NOTE**: CDO-level properties only. Component sub-properties (dot-path) do NOT persist through recompile. Use the REST API `set_default` action instead (works with editor open).

### 4. BlueprintDump
```
-run=BlueprintDump
  -AssetPath=/Game/Blueprints/BP_Name
```

### 5. MaterialCreate
```
-run=MaterialCreate
  -AssetPath=/Game/Materials/MI_Name
  -ParentMaterial=/Engine/BasicShapes/BasicShapeMaterial
  -SetScalar="ParamName=0.5"
  -SetVector="ParamName=(R=0.5,G=0.1,B=0.05,A=1.0)"
  -SetTexture="ParamName=/Game/Textures/T_Name"
```

## Asset Generation Scripts (pre-import)

Located at `Plugins/AssetGen/Scripts/`:
- `generate_meshes.py` — Blender, static mesh FBX. Run: `"C:\Program Files\Blender Foundation\Blender 5.1\blender.exe" --background --python Scripts/generate_meshes.py -- [output_dir] [mesh_filter]`
- `generate_crab.py` — Blender, rigged skeletal mesh + animations (Idle, Walk, Death). Run: `"C:\Program Files\Blender Foundation\Blender 5.1\blender.exe" --background --python Scripts/generate_crab.py -- [output_dir]`
- `generate_monster_rig.py` — Blender, rigged monster + animations. Run: `"C:\Program Files\Blender Foundation\Blender 5.1\blender.exe" --background --python Scripts/generate_monster_rig.py -- [output_dir]`
- `generate_sounds.py` — Python, WAV. Run: `python Scripts/generate_sounds.py [output_dir] [sound_filter]`
- `generate_music.py` — Python + FluidSynth, WAV. Run: `python Scripts/generate_music.py [output_dir] [track_filter]`
- `generate_sprites.py` — SVG + Inkscape PNG. Run: `python Scripts/generate_sprites.py [output_dir] [sprite_filter]`
- `blender_anim.py` — Mannequin animations. Run: `"C:\Program Files\Blender Foundation\Blender 5.1\blender.exe" --background --python Scripts/blender_anim.py -- <input.fbx> [output_dir] [anim_filter]`

## Typical Pipeline (Editor Open)

1. Generate files with Python/Blender scripts (headless)
2. **REST: import mesh** — `manage_asset` `import` action (`--max-time 5`, fire-and-forget), then `sleep 2`
3. **Commandlet: import animations** — `AssetImport` with `-Type=Animation -Skeleton=...` (REST crashes for animations)
4. **Commandlet or REST: create material** — `MaterialCreate` with `-ParentMaterial=/Game/Shared/Materials/M_MASTER`
5. **Commandlet: create Blueprint** — `BlueprintCreate` (works with editor open)
6. **REST: assign mesh/materials** — `manage_blueprint` `set_default` with dot-path (e.g., `Mesh.SkeletalMeshAsset`)
7. **REST: set CDO properties** — `manage_blueprint` `set_default` (e.g., `HitPoints`, `KnockbackForce`)

## Editor vs Headless Compatibility

| Operation | REST (editor open) | Commandlet |
|-----------|:---:|:---:|
| Import mesh/texture/sound | Yes (fire-and-forget) | Yes (editor closed) |
| Import animation | **No** (crashes) | **Yes** (with `-Skeleton`, editor closed) |
| Create Blueprint | — | Yes (editor open or closed) |
| Set CDO properties | Yes | Yes (CDO-level only) |
| Set component properties | **Yes** (dot-path works) | **No** (lost on recompile) |
| Read Blueprint | Yes | Yes (BlueprintDump) |
| Create Material | — | Yes (editor open or closed) |
| Build CBEditor | — | Yes (editor must be closed) |
| Build CB (game) | — | Yes |

## Important Notes
- Always use Bash, not PowerShell
- Commandlets require `MSYS_NO_PATHCONV=1` prefix
- Asset paths use UE format: `/Game/Folder/AssetName` (maps to `Content/Folder/AssetName.uasset`)
- Blender FBX export: model facing -Y in Blender. Use `axis_forward='-Y'`, `axis_up='Z'`, `add_leaf_bones=False`. Before export, rotate the armature +90° around Z (`arm_obj.rotation_euler[2] = math.pi / 2`, do NOT apply) so models face **+X forward in UE**. Exception: humanoid models retargeted to the Mannequin skip this rotation (Mannequin faces +Y).
- For rigged skeletal meshes: use `apply_unit_scale=False`, `apply_scale_options='FBX_SCALE_NONE'` (NOT `FBX_SCALE_ALL` — causes double unit conversion with UE's importer).
- For static meshes: `apply_unit_scale=True`, `apply_scale_options='FBX_SCALE_ALL'`. The `AssetImport` commandlet sets `bConvertSceneUnit=true` to convert Blender meters to UE centimeters. Without this flag, static meshes import 100x too small.
