"""Generate Walk animation FBX for the Warrior.

Invoke headless:
    "C:\\Program Files\\Blender Foundation\\Blender 5.1\\blender.exe" \
        --background --python Tools/blender/warrior_anim_walk.py
"""
import os, sys, math, bpy

OUT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "out")
os.makedirs(OUT_DIR, exist_ok=True)

bpy.ops.wm.read_factory_settings(use_empty=True)
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from warrior_armature import create_warrior_armature, set_bone_rotation, export_anim_fbx

armature_obj, pb = create_warrior_armature()
rest = (0, 0, 0)

action = bpy.data.actions.new(name="Walk")
armature_obj.animation_data.action = action

leg_swing = math.radians(30)
arm_swing = math.radians(20)
spine_lean = math.radians(3)

for bone_name in pb.keys():
    set_bone_rotation(pb, bone_name, 0, rest)
    set_bone_rotation(pb, bone_name, 30, rest)

set_bone_rotation(pb, "L_UpperLeg", 0, (0, 0, 0))
set_bone_rotation(pb, "L_UpperLeg", 8, (0, leg_swing, 0))
set_bone_rotation(pb, "L_UpperLeg", 15, (0, 0, 0))
set_bone_rotation(pb, "L_UpperLeg", 23, (0, -leg_swing, 0))
set_bone_rotation(pb, "L_UpperLeg", 30, (0, 0, 0))

set_bone_rotation(pb, "R_UpperLeg", 0, (0, 0, 0))
set_bone_rotation(pb, "R_UpperLeg", 8, (0, -leg_swing, 0))
set_bone_rotation(pb, "R_UpperLeg", 15, (0, 0, 0))
set_bone_rotation(pb, "R_UpperLeg", 23, (0, leg_swing, 0))
set_bone_rotation(pb, "R_UpperLeg", 30, (0, 0, 0))

knee_bend = math.radians(15)
set_bone_rotation(pb, "L_LowerLeg", 0, (0, 0, 0))
set_bone_rotation(pb, "L_LowerLeg", 8, (0, -knee_bend, 0))
set_bone_rotation(pb, "L_LowerLeg", 23, (0, -knee_bend, 0))
set_bone_rotation(pb, "L_LowerLeg", 30, (0, 0, 0))

set_bone_rotation(pb, "R_LowerLeg", 0, (0, 0, 0))
set_bone_rotation(pb, "R_LowerLeg", 8, (0, -knee_bend, 0))
set_bone_rotation(pb, "R_LowerLeg", 23, (0, -knee_bend, 0))
set_bone_rotation(pb, "R_LowerLeg", 30, (0, 0, 0))

set_bone_rotation(pb, "L_UpperArm", 0, (0, 0, 0))
set_bone_rotation(pb, "L_UpperArm", 8, (0, -arm_swing, 0))
set_bone_rotation(pb, "L_UpperArm", 15, (0, 0, 0))
set_bone_rotation(pb, "L_UpperArm", 23, (0, arm_swing, 0))
set_bone_rotation(pb, "L_UpperArm", 30, (0, 0, 0))

set_bone_rotation(pb, "R_UpperArm", 0, (0, 0, 0))
set_bone_rotation(pb, "R_UpperArm", 8, (0, arm_swing, 0))
set_bone_rotation(pb, "R_UpperArm", 15, (0, 0, 0))
set_bone_rotation(pb, "R_UpperArm", 23, (0, -arm_swing, 0))
set_bone_rotation(pb, "R_UpperArm", 30, (0, 0, 0))

set_bone_rotation(pb, "Spine", 0, (0, 0, 0))
set_bone_rotation(pb, "Spine", 8, (0, spine_lean, 0))
set_bone_rotation(pb, "Spine", 15, (0, 0, 0))
set_bone_rotation(pb, "Spine", 23, (0, spine_lean, 0))
set_bone_rotation(pb, "Spine", 30, (0, 0, 0))

OUT_FBX = os.path.join(OUT_DIR, "Warrior_Walk.fbx")
export_anim_fbx(OUT_FBX)
print(f"[warrior_anim_walk] wrote {OUT_FBX}")
