"""Generate a Death animation FBX for the Warrior, using the same armature.

Invoke headless:
    "C:\\Program Files\\Blender Foundation\\Blender 5.1\\blender.exe" \
        --background --python Tools/blender/warrior_death_anim.py
"""
import os
import math
import bpy
from mathutils import Vector

OUT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "out")
OUT_FBX = os.path.join(OUT_DIR, "Warrior_Death.fbx")
os.makedirs(OUT_DIR, exist_ok=True)

bpy.ops.wm.read_factory_settings(use_empty=True)

# ---------------------------------------------------------------------------
# 1. Recreate the same armature (must match warrior_mesh.py exactly)
# ---------------------------------------------------------------------------

bpy.ops.object.armature_add(location=(0, 0, 0))
armature_obj = bpy.context.active_object
armature_obj.name = "Warrior_Armature"
armature = armature_obj.data
armature.name = "Warrior_Skeleton"

bpy.ops.object.mode_set(mode='EDIT')
armature.edit_bones.remove(armature.edit_bones[0])

def add_bone(name, head, tail, parent_name=None):
    bone = armature.edit_bones.new(name)
    bone.head = Vector(head)
    bone.tail = Vector(tail)
    if parent_name:
        bone.parent = armature.edit_bones[parent_name]
        bone.use_connect = False
    return bone

add_bone("Root",         (0, 0, 0),       (0, 0, 0.85))
add_bone("Spine",        (0, 0, 0.85),    (0, 0, 1.35),  "Root")
add_bone("Head",         (0, 0, 1.35),    (0, 0, 1.75),  "Spine")
add_bone("L_UpperArm",   (0.4, 0, 1.35),  (0.5, 0, 1.1),  "Spine")
add_bone("L_LowerArm",   (0.5, 0, 1.1),   (0.5, 0, 0.75), "L_UpperArm")
add_bone("R_UpperArm",   (-0.4, 0, 1.35), (-0.5, 0, 1.1), "Spine")
add_bone("R_LowerArm",   (-0.5, 0, 1.1),  (-0.5, 0, 0.75), "R_UpperArm")
add_bone("L_UpperLeg",   (0.18, 0, 0.85), (0.18, 0, 0.45), "Root")
add_bone("L_LowerLeg",   (0.18, 0, 0.45), (0.18, 0, 0.05), "L_UpperLeg")
add_bone("L_Foot",        (0.18, 0, 0.05), (0.18, 0.15, 0.0), "L_LowerLeg")
add_bone("R_UpperLeg",   (-0.18, 0, 0.85), (-0.18, 0, 0.45), "Root")
add_bone("R_LowerLeg",   (-0.18, 0, 0.45), (-0.18, 0, 0.05), "R_UpperLeg")
add_bone("R_Foot",        (-0.18, 0, 0.05), (-0.18, 0.15, 0.0), "R_LowerLeg")

bpy.ops.object.mode_set(mode='OBJECT')

# Rotate armature to face +X (same as warrior_mesh.py)
bpy.ops.object.select_all(action='DESELECT')
armature_obj.select_set(True)
bpy.context.view_layer.objects.active = armature_obj
armature_obj.rotation_euler[2] = math.radians(-90)
bpy.ops.object.transform_apply(rotation=True)

# ---------------------------------------------------------------------------
# 2. Keyframe death animation
# ---------------------------------------------------------------------------

FPS = 30
bpy.context.scene.render.fps = FPS

def set_bone_rotation(pose_bones, bone_name, frame, euler_xyz):
    bone = pose_bones[bone_name]
    bone.rotation_mode = 'XYZ'
    bone.rotation_euler = euler_xyz
    bone.keyframe_insert(data_path="rotation_euler", frame=frame)

death_action = bpy.data.actions.new(name="Death")
armature_obj.animation_data_create()
armature_obj.animation_data.action = death_action

bpy.ops.object.mode_set(mode='POSE')
pb = armature_obj.pose.bones
rest = (0, 0, 0)

for bone_name in pb.keys():
    set_bone_rotation(pb, bone_name, 0, rest)

# Frame 10: knees buckle, spine starts to pitch
set_bone_rotation(pb, "L_UpperLeg", 10, (0, math.radians(-15), 0))
set_bone_rotation(pb, "R_UpperLeg", 10, (0, math.radians(-15), 0))
set_bone_rotation(pb, "L_LowerLeg", 10, (0, math.radians(-30), 0))
set_bone_rotation(pb, "R_LowerLeg", 10, (0, math.radians(-30), 0))
set_bone_rotation(pb, "Spine", 10, (0, math.radians(15), 0))

# Frame 20: collapsing forward
set_bone_rotation(pb, "Spine", 20, (0, math.radians(60), 0))
set_bone_rotation(pb, "Head", 20, (0, math.radians(20), 0))
set_bone_rotation(pb, "L_UpperArm", 20, (0, math.radians(40), 0))
set_bone_rotation(pb, "R_UpperArm", 20, (0, math.radians(40), 0))
set_bone_rotation(pb, "L_UpperLeg", 20, (0, math.radians(-40), 0))
set_bone_rotation(pb, "R_UpperLeg", 20, (0, math.radians(-40), 0))
set_bone_rotation(pb, "Root", 20, (0, math.radians(30), 0))

# Frame 30: on the ground, final pose (held)
set_bone_rotation(pb, "Spine", 30, (0, math.radians(80), 0))
set_bone_rotation(pb, "Head", 30, (0, math.radians(30), 0))
set_bone_rotation(pb, "L_UpperArm", 30, (0, math.radians(60), 0))
set_bone_rotation(pb, "R_UpperArm", 30, (0, math.radians(60), 0))
set_bone_rotation(pb, "L_LowerArm", 30, (0, math.radians(-20), 0))
set_bone_rotation(pb, "R_LowerArm", 30, (0, math.radians(-20), 0))
set_bone_rotation(pb, "L_UpperLeg", 30, (0, math.radians(-60), 0))
set_bone_rotation(pb, "R_UpperLeg", 30, (0, math.radians(-60), 0))
set_bone_rotation(pb, "L_LowerLeg", 30, (0, math.radians(-40), 0))
set_bone_rotation(pb, "R_LowerLeg", 30, (0, math.radians(-40), 0))
set_bone_rotation(pb, "Root", 30, (0, math.radians(50), 0))

bpy.ops.object.mode_set(mode='OBJECT')

# ---------------------------------------------------------------------------
# 3. Export FBX (animation only, no mesh)
# ---------------------------------------------------------------------------

bpy.ops.export_scene.fbx(
    filepath=OUT_FBX,
    use_selection=False,
    global_scale=1.0,
    apply_unit_scale=True,
    bake_space_transform=False,
    axis_forward="-Z",
    axis_up="Y",
    object_types={"ARMATURE"},
    add_leaf_bones=False,
    bake_anim=True,
    bake_anim_use_all_actions=True,
    bake_anim_use_nla_strips=False,
    bake_anim_force_startend_keying=True,
)

print(f"[warrior_death_anim] wrote {OUT_FBX}")
