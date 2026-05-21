"""Import FBX via deferred Slate tick callback to avoid TaskGraph recursion."""
import unreal

handle = None

def do_import(dt):
    global handle
    unreal.unregister_slate_post_tick_callback(handle)

    task = unreal.AssetImportTask()
    task.filename = r"C:\dev\games-unreal\TonyHawk2\Assets\Blender\debug_park.fbx"
    task.destination_path = "/Game/Phase1/Meshes"
    task.automated = True
    task.replace_existing = True
    task.save = True

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    unreal.log("FBX import complete")

handle = unreal.register_slate_post_tick_callback(do_import)
print("deferred FBX import scheduled - will run next frame")
