"""Shared Warrior armature setup for animation-only FBX exports.

Usage from other scripts:
    from warrior_armature import create_warrior_armature, set_bone_rotation
    armature_obj, pb = create_warrior_armature()
"""
import math
import bpy
from mathutils import Vector


def create_warrior_armature():
    """Create the Warrior armature matching warrior_mesh.py's bone hierarchy.
    Returns (armature_obj, pose_bones).
    """
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

    bpy.context.scene.render.fps = 30

    armature_obj.animation_data_create()
    bpy.ops.object.mode_set(mode='POSE')
    pb = armature_obj.pose.bones

    return armature_obj, pb


def set_bone_rotation(pose_bones, bone_name, frame, euler_xyz):
    bone = pose_bones[bone_name]
    bone.rotation_mode = 'XYZ'
    bone.rotation_euler = euler_xyz
    bone.keyframe_insert(data_path="rotation_euler", frame=frame)


def export_anim_fbx(filepath):
    bpy.ops.object.mode_set(mode='OBJECT')
    bpy.ops.export_scene.fbx(
        filepath=filepath,
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
