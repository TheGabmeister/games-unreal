"""Generate Death animation FBX for the Warrior.

Invoke headless:
    "C:\\Program Files\\Blender Foundation\\Blender 5.1\\blender.exe" \
        --background --python Tools/blender/warrior_death_anim.py
"""
import os, sys, math, bpy

OUT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "out")
os.makedirs(OUT_DIR, exist_ok=True)

bpy.ops.wm.read_factory_settings(use_empty=True)
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from warrior_armature import create_warrior_armature, set_bone_rotation, export_anim_fbx

armature_obj, pb = create_warrior_armature()
rest = (0, 0, 0)

action = bpy.data.actions.new(name="Death")
armature_obj.animation_data.action = action

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

OUT_FBX = os.path.join(OUT_DIR, "Warrior_Death.fbx")
export_anim_fbx(OUT_FBX)
print(f"[warrior_death_anim] wrote {OUT_FBX}")
