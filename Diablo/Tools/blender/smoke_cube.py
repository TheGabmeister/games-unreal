"""Smoke test: export a 100cm cube as FBX to verify the Blender pipeline.

Invoke headless:
    "C:\\Program Files\\Blender Foundation\\Blender 5.1\\blender.exe" \
        --background --python Tools/blender/smoke_cube.py
"""
import os
import bpy

OUT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "out")
OUT_FBX = os.path.join(OUT_DIR, "cube.fbx")

os.makedirs(OUT_DIR, exist_ok=True)

bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0.5))
cube = bpy.context.active_object
cube.name = "SmokeCube"
cube.scale = (1.0, 1.0, 1.0)

bpy.ops.export_scene.fbx(
    filepath=OUT_FBX,
    use_selection=False,
    global_scale=1.0,
    apply_unit_scale=True,
    bake_space_transform=False,
    axis_forward="-Z",
    axis_up="Y",
    object_types={"MESH"},
)

print(f"[smoke_cube] wrote {OUT_FBX}")
