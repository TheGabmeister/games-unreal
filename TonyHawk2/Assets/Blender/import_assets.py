"""Import Phase 1 assets into UE: debug park FBX + audio WAVs."""
import unreal

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

def import_file(filepath, dest_path, fbx_opts=None):
    task = unreal.AssetImportTask()
    task.filename = filepath
    task.destination_path = dest_path
    task.automated = True
    task.replace_existing = True
    task.save = True
    if fbx_opts:
        task.options = fbx_opts
    asset_tools.import_asset_tasks([task])

# --- Import WAVs ---
wav_dir = r"C:\dev\games-unreal\TonyHawk2\Assets\Audio\SFX"
for name in ["SFX_WheelRoll", "SFX_LandingThud", "SFX_BailImpact", "SFX_WindAir"]:
    import_file(wav_dir + "\\" + name + ".wav", "/Game/Phase1/Audio")
    print("Imported " + name)

# --- Import FBX ---
opts = unreal.FbxImportUI()
opts.import_mesh = True
opts.import_as_skeletal = False
opts.import_materials = True
opts.import_textures = False
opts.import_animations = False
opts.static_mesh_import_data.combine_meshes = False

import_file(r"C:\dev\games-unreal\TonyHawk2\Assets\Blender\debug_park.fbx", "/Game/Phase1/Meshes", opts)
print("FBX import complete")

print("All Phase 1 assets imported")
