"""Generate a simple healing potion static mesh.

Invoke headless:
    "C:\\Program Files\\Blender Foundation\\Blender 5.1\\blender.exe" \
        --background --python Tools/blender/potion_mesh.py
"""
import os
import math
import bpy

OUT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "out")
OUT_FBX = os.path.join(OUT_DIR, "HealingPotion.fbx")
os.makedirs(OUT_DIR, exist_ok=True)

bpy.ops.wm.read_factory_settings(use_empty=True)

# ---------------------------------------------------------------------------
# 1. Build mesh
# ---------------------------------------------------------------------------

# Bottle body (cylinder)
bpy.ops.mesh.primitive_cylinder_add(
    radius=0.06, depth=0.2, location=(0, 0, 0.1))
body = bpy.context.active_object
body.name = "PotionBody"

# Bottle neck (narrower cylinder)
bpy.ops.mesh.primitive_cylinder_add(
    radius=0.03, depth=0.06, location=(0, 0, 0.23))
neck = bpy.context.active_object
neck.name = "PotionNeck"

# Cork (sphere on top)
bpy.ops.mesh.primitive_uv_sphere_add(
    radius=0.035, segments=12, ring_count=8, location=(0, 0, 0.28))
cork = bpy.context.active_object
cork.name = "PotionCork"

# ---------------------------------------------------------------------------
# 2. Materials
# ---------------------------------------------------------------------------

red_mat = bpy.data.materials.new(name="PotionRed")
red_mat.use_nodes = True
red_mat.node_tree.nodes["Principled BSDF"].inputs["Base Color"].default_value = (0.8, 0.1, 0.1, 1.0)

brown_mat = bpy.data.materials.new(name="CorkBrown")
brown_mat.use_nodes = True
brown_mat.node_tree.nodes["Principled BSDF"].inputs["Base Color"].default_value = (0.45, 0.3, 0.15, 1.0)

body.data.materials.append(red_mat)
neck.data.materials.append(red_mat)
cork.data.materials.append(brown_mat)

# ---------------------------------------------------------------------------
# 3. Join into one mesh
# ---------------------------------------------------------------------------

bpy.ops.object.select_all(action='DESELECT')
body.select_set(True)
neck.select_set(True)
cork.select_set(True)
bpy.context.view_layer.objects.active = body
bpy.ops.object.join()
mesh_obj = bpy.context.active_object
mesh_obj.name = "HealingPotion"

# ---------------------------------------------------------------------------
# 4. Export FBX (static mesh, no armature)
# ---------------------------------------------------------------------------

bpy.ops.export_scene.fbx(
    filepath=OUT_FBX,
    use_selection=False,
    global_scale=1.0,
    apply_unit_scale=True,
    bake_space_transform=False,
    axis_forward="-Z",
    axis_up="Y",
    object_types={"MESH"},
    add_leaf_bones=False,
    bake_anim=False,
)

print(f"[potion_mesh] wrote {OUT_FBX}")
