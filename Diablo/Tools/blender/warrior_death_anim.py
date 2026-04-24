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

# Frame 8: knees buckle, body starts to sag
set_bone_rotation(pb, "L_UpperLeg", 8, (0, math.radians(-20), 0))
set_bone_rotation(pb, "R_UpperLeg", 8, (0, math.radians(-20), 0))
set_bone_rotation(pb, "L_LowerLeg", 8, (0, math.radians(30), 0))
set_bone_rotation(pb, "R_LowerLeg", 8, (0, math.radians(30), 0))
set_bone_rotation(pb, "Spine", 8, (0, math.radians(10), 0))
set_bone_rotation(pb, "Head", 8, (0, math.radians(10), 0))
set_bone_rotation(pb, "L_UpperArm", 8, (0, math.radians(15), 0))
set_bone_rotation(pb, "R_UpperArm", 8, (0, math.radians(15), 0))

# Frame 18: collapsing forward — Root pitches on X axis
set_bone_rotation(pb, "Root", 18, (math.radians(45), 0, 0))
set_bone_rotation(pb, "Spine", 18, (0, math.radians(50), 0))
set_bone_rotation(pb, "Head", 18, (0, math.radians(25), 0))
set_bone_rotation(pb, "L_UpperArm", 18, (0, math.radians(50), 0))
set_bone_rotation(pb, "R_UpperArm", 18, (0, math.radians(50), 0))
set_bone_rotation(pb, "L_LowerArm", 18, (0, math.radians(-15), 0))
set_bone_rotation(pb, "R_LowerArm", 18, (0, math.radians(-15), 0))
set_bone_rotation(pb, "L_UpperLeg", 18, (0, math.radians(-50), 0))
set_bone_rotation(pb, "R_UpperLeg", 18, (0, math.radians(-50), 0))
set_bone_rotation(pb, "L_LowerLeg", 18, (0, math.radians(40), 0))
set_bone_rotation(pb, "R_LowerLeg", 18, (0, math.radians(40), 0))

# Frame 28: on the ground, face-down final pose (held)
set_bone_rotation(pb, "Root", 28, (math.radians(80), 0, 0))
set_bone_rotation(pb, "Spine", 28, (0, math.radians(15), 0))
set_bone_rotation(pb, "Head", 28, (0, math.radians(20), 0))
set_bone_rotation(pb, "L_UpperArm", 28, (0, math.radians(70), 0))
set_bone_rotation(pb, "R_UpperArm", 28, (0, math.radians(70), 0))
set_bone_rotation(pb, "L_LowerArm", 28, (0, math.radians(-30), 0))
set_bone_rotation(pb, "R_LowerArm", 28, (0, math.radians(-30), 0))
set_bone_rotation(pb, "L_UpperLeg", 28, (0, math.radians(-30), 0))
set_bone_rotation(pb, "R_UpperLeg", 28, (0, math.radians(-30), 0))
set_bone_rotation(pb, "L_LowerLeg", 28, (0, math.radians(20), 0))
set_bone_rotation(pb, "R_LowerLeg", 28, (0, math.radians(20), 0))

OUT_FBX = os.path.join(OUT_DIR, "Warrior_Death.fbx")
export_anim_fbx(OUT_FBX)
print(f"[warrior_death_anim] wrote {OUT_FBX}")
