"""Generate Attack animation FBX for the Warrior.

Invoke headless:
    "C:\\Program Files\\Blender Foundation\\Blender 5.1\\blender.exe" \
        --background --python Tools/blender/warrior_anim_attack.py
"""
import os, sys, math, bpy

OUT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "out")
os.makedirs(OUT_DIR, exist_ok=True)

bpy.ops.wm.read_factory_settings(use_empty=True)
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from warrior_armature import create_warrior_armature, set_bone_rotation, export_anim_fbx

armature_obj, pb = create_warrior_armature()
rest = (0, 0, 0)

action = bpy.data.actions.new(name="Attack")
armature_obj.animation_data.action = action

for bone_name in pb.keys():
    set_bone_rotation(pb, bone_name, 0, rest)
    set_bone_rotation(pb, bone_name, 20, rest)

wind_up = math.radians(-45)
swing_fwd = math.radians(60)
follow_through = math.radians(40)
forearm_extend = math.radians(-20)
spine_fwd = math.radians(8)

set_bone_rotation(pb, "R_UpperArm", 5, (0, wind_up, 0))
set_bone_rotation(pb, "R_UpperArm", 10, (0, swing_fwd, 0))
set_bone_rotation(pb, "R_UpperArm", 15, (0, follow_through, 0))

set_bone_rotation(pb, "R_LowerArm", 5, (0, math.radians(-10), 0))
set_bone_rotation(pb, "R_LowerArm", 10, (0, forearm_extend, 0))
set_bone_rotation(pb, "R_LowerArm", 15, (0, math.radians(-10), 0))

set_bone_rotation(pb, "Spine", 5, (0, math.radians(-3), 0))
set_bone_rotation(pb, "Spine", 10, (0, spine_fwd, 0))
set_bone_rotation(pb, "Spine", 15, (0, math.radians(4), 0))

set_bone_rotation(pb, "L_UpperArm", 5, (0, math.radians(10), 0))
set_bone_rotation(pb, "L_UpperArm", 10, (0, math.radians(-15), 0))
set_bone_rotation(pb, "L_UpperArm", 15, (0, math.radians(-5), 0))

OUT_FBX = os.path.join(OUT_DIR, "Warrior_Attack.fbx")
export_anim_fbx(OUT_FBX)
print(f"[warrior_anim_attack] wrote {OUT_FBX}")
