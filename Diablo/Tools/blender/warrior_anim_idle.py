"""Generate Idle animation FBX for the Warrior.

Invoke headless:
    "C:\\Program Files\\Blender Foundation\\Blender 5.1\\blender.exe" \
        --background --python Tools/blender/warrior_anim_idle.py
"""
import os, sys, math, bpy

OUT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "out")
os.makedirs(OUT_DIR, exist_ok=True)

bpy.ops.wm.read_factory_settings(use_empty=True)
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from warrior_armature import create_warrior_armature, set_bone_rotation, export_anim_fbx

armature_obj, pb = create_warrior_armature()
rest = (0, 0, 0)

action = bpy.data.actions.new(name="Idle")
armature_obj.animation_data.action = action

for bone_name in pb.keys():
    set_bone_rotation(pb, bone_name, 0, rest)
    set_bone_rotation(pb, bone_name, 60, rest)

bob_angle = math.radians(2)
set_bone_rotation(pb, "Spine", 15, (0, bob_angle, 0))
set_bone_rotation(pb, "Spine", 45, (0, -bob_angle, 0))
set_bone_rotation(pb, "Spine", 0, (0, 0, 0))
set_bone_rotation(pb, "Spine", 30, (0, 0, 0))
set_bone_rotation(pb, "Spine", 60, (0, 0, 0))

arm_sway = math.radians(3)
set_bone_rotation(pb, "L_UpperArm", 15, (0, arm_sway, 0))
set_bone_rotation(pb, "L_UpperArm", 45, (0, -arm_sway, 0))
set_bone_rotation(pb, "R_UpperArm", 15, (0, -arm_sway, 0))
set_bone_rotation(pb, "R_UpperArm", 45, (0, arm_sway, 0))

OUT_FBX = os.path.join(OUT_DIR, "Warrior_Idle.fbx")
export_anim_fbx(OUT_FBX)
print(f"[warrior_anim_idle] wrote {OUT_FBX}")
