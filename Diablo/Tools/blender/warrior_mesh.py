"""Generate a low-poly Warrior mesh with idle and walk animations.

Invoke headless:
    "C:\\Program Files\\Blender Foundation\\Blender 5.1\\blender.exe" \
        --background --python Tools/blender/warrior_mesh.py
"""
import os
import math
import bpy
from mathutils import Vector, Matrix

OUT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "out")
OUT_FBX = os.path.join(OUT_DIR, "Warrior.fbx")
os.makedirs(OUT_DIR, exist_ok=True)

bpy.ops.wm.read_factory_settings(use_empty=True)

# ---------------------------------------------------------------------------
# 1. Build mesh from primitives  (character faces +Y in Blender)
# ---------------------------------------------------------------------------

def add_box(name, location, scale):
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=location)
    obj = bpy.context.active_object
    obj.name = name
    obj.scale = scale
    bpy.ops.object.transform_apply(scale=True)
    return obj

def add_sphere(name, location, radius, segments=12, rings=8):
    bpy.ops.mesh.primitive_uv_sphere_add(
        radius=radius, segments=segments, ring_count=rings, location=location)
    obj = bpy.context.active_object
    obj.name = name
    return obj

parts = []

# Torso
parts.append(add_box("Torso", (0, 0, 1.1), (0.4, 0.25, 0.5)))
# Head
parts.append(add_sphere("Head", (0, 0, 1.65), 0.15))
# Upper arms
parts.append(add_box("L_UpperArm", (0.5, 0, 1.25), (0.12, 0.12, 0.3)))
parts.append(add_box("R_UpperArm", (-0.5, 0, 1.25), (0.12, 0.12, 0.3)))
# Lower arms
parts.append(add_box("L_LowerArm", (0.5, 0, 0.88), (0.1, 0.1, 0.25)))
parts.append(add_box("R_LowerArm", (-0.5, 0, 0.88), (0.1, 0.1, 0.25)))
# Upper legs
parts.append(add_box("L_UpperLeg", (0.18, 0, 0.55), (0.15, 0.15, 0.4)))
parts.append(add_box("R_UpperLeg", (-0.18, 0, 0.55), (0.15, 0.15, 0.4)))
# Lower legs
parts.append(add_box("L_LowerLeg", (0.18, 0, 0.2), (0.12, 0.12, 0.3)))
parts.append(add_box("R_LowerLeg", (-0.18, 0, 0.2), (0.12, 0.12, 0.3)))
# Feet
parts.append(add_box("L_Foot", (0.18, 0.05, 0.025), (0.12, 0.2, 0.05)))
parts.append(add_box("R_Foot", (-0.18, 0.05, 0.025), (0.12, 0.2, 0.05)))

# Join all into one mesh
bpy.ops.object.select_all(action='DESELECT')
for p in parts:
    p.select_set(True)
bpy.context.view_layer.objects.active = parts[0]
bpy.ops.object.join()
mesh_obj = bpy.context.active_object
mesh_obj.name = "Warrior"

# ---------------------------------------------------------------------------
# 2. Create armature
# ---------------------------------------------------------------------------

bpy.ops.object.select_all(action='DESELECT')
bpy.ops.object.armature_add(location=(0, 0, 0))
armature_obj = bpy.context.active_object
armature_obj.name = "Warrior_Armature"
armature = armature_obj.data
armature.name = "Warrior_Skeleton"

bpy.ops.object.mode_set(mode='EDIT')

# Remove default bone
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

# ---------------------------------------------------------------------------
# 2b. Rotate mesh + armature to face +X, then apply transform
# ---------------------------------------------------------------------------
# Both objects are rotated and the transform is applied BEFORE parenting,
# so auto-weights see the final geometry with no residual rotation.

for obj in (mesh_obj, armature_obj):
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    obj.rotation_euler[2] = math.radians(-90)
    bpy.ops.object.transform_apply(rotation=True)

# ---------------------------------------------------------------------------
# 3. Parent mesh to armature with automatic weights
# ---------------------------------------------------------------------------

bpy.ops.object.select_all(action='DESELECT')
mesh_obj.select_set(True)
armature_obj.select_set(True)
bpy.context.view_layer.objects.active = armature_obj
bpy.ops.object.parent_set(type='ARMATURE_AUTO')

# ---------------------------------------------------------------------------
# 4. Keyframe animations
# ---------------------------------------------------------------------------

FPS = 30
bpy.context.scene.render.fps = FPS

def set_bone_rotation(pose_bones, bone_name, frame, euler_xyz):
    bone = pose_bones[bone_name]
    bone.rotation_mode = 'XYZ'
    bone.rotation_euler = euler_xyz
    bone.keyframe_insert(data_path="rotation_euler", frame=frame)

# --- Idle animation (frames 0-60, 2 seconds at 30fps) ---
idle_action = bpy.data.actions.new(name="Idle")
armature_obj.animation_data_create()
armature_obj.animation_data.action = idle_action

bpy.ops.object.select_all(action='DESELECT')
armature_obj.select_set(True)
bpy.context.view_layer.objects.active = armature_obj
bpy.ops.object.mode_set(mode='POSE')
pb = armature_obj.pose.bones

rest = (0, 0, 0)
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

bpy.ops.object.mode_set(mode='OBJECT')

# Push idle to NLA
track_idle = armature_obj.animation_data.nla_tracks.new()
track_idle.name = "IdleTrack"
strip_idle = track_idle.strips.new("Idle", 0, idle_action)
strip_idle.frame_end = 60
armature_obj.animation_data.action = None

# --- Walk animation (frames 0-30, 1 second at 30fps) ---
walk_action = bpy.data.actions.new(name="Walk")
armature_obj.animation_data.action = walk_action

bpy.ops.object.mode_set(mode='POSE')

leg_swing = math.radians(30)
arm_swing = math.radians(20)
spine_lean = math.radians(3)

for bone_name in pb.keys():
    set_bone_rotation(pb, bone_name, 0, rest)
    set_bone_rotation(pb, bone_name, 30, rest)

# Frame 0 & 30: neutral (loop point)
# Frame 8: left leg forward, right leg back
# Frame 15: passing (neutral legs)
# Frame 23: right leg forward, left leg back

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

# Opposing arm swing
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

# Spine lean
set_bone_rotation(pb, "Spine", 0, (0, 0, 0))
set_bone_rotation(pb, "Spine", 8, (0, spine_lean, 0))
set_bone_rotation(pb, "Spine", 15, (0, 0, 0))
set_bone_rotation(pb, "Spine", 23, (0, spine_lean, 0))
set_bone_rotation(pb, "Spine", 30, (0, 0, 0))

bpy.ops.object.mode_set(mode='OBJECT')

# Push walk to NLA
track_walk = armature_obj.animation_data.nla_tracks.new()
track_walk.name = "WalkTrack"
strip_walk = track_walk.strips.new("Walk", 0, walk_action)
strip_walk.frame_end = 30
armature_obj.animation_data.action = None

# --- Attack animation (frames 0-20, ~0.67 second at 30fps) ---
attack_action = bpy.data.actions.new(name="Attack")
armature_obj.animation_data.action = attack_action

bpy.ops.object.mode_set(mode='POSE')

for bone_name in pb.keys():
    set_bone_rotation(pb, bone_name, 0, rest)
    set_bone_rotation(pb, bone_name, 20, rest)

wind_up = math.radians(-45)
swing_fwd = math.radians(60)
follow_through = math.radians(40)
forearm_extend = math.radians(-20)
spine_fwd = math.radians(8)

# Right arm overhead slash
set_bone_rotation(pb, "R_UpperArm", 5, (0, wind_up, 0))
set_bone_rotation(pb, "R_UpperArm", 10, (0, swing_fwd, 0))
set_bone_rotation(pb, "R_UpperArm", 15, (0, follow_through, 0))

set_bone_rotation(pb, "R_LowerArm", 5, (0, math.radians(-10), 0))
set_bone_rotation(pb, "R_LowerArm", 10, (0, forearm_extend, 0))
set_bone_rotation(pb, "R_LowerArm", 15, (0, math.radians(-10), 0))

# Spine leans into the swing
set_bone_rotation(pb, "Spine", 5, (0, math.radians(-3), 0))
set_bone_rotation(pb, "Spine", 10, (0, spine_fwd, 0))
set_bone_rotation(pb, "Spine", 15, (0, math.radians(4), 0))

# Left arm counters slightly
set_bone_rotation(pb, "L_UpperArm", 5, (0, math.radians(10), 0))
set_bone_rotation(pb, "L_UpperArm", 10, (0, math.radians(-15), 0))
set_bone_rotation(pb, "L_UpperArm", 15, (0, math.radians(-5), 0))

bpy.ops.object.mode_set(mode='OBJECT')

# Push attack to NLA
track_attack = armature_obj.animation_data.nla_tracks.new()
track_attack.name = "AttackTrack"
strip_attack = track_attack.strips.new("Attack", 0, attack_action)
strip_attack.frame_end = 20
armature_obj.animation_data.action = None

# Keep NLA tracks active so they export as separate FBX animation stacks
track_idle.mute = False
track_walk.mute = False
track_attack.mute = False

# ---------------------------------------------------------------------------
# 5. Export FBX
# ---------------------------------------------------------------------------
# Mesh + armature were rotated to face +X before parenting.
# Standard Blender→UE FBX axis convention.

bpy.ops.export_scene.fbx(
    filepath=OUT_FBX,
    use_selection=False,
    global_scale=1.0,
    apply_unit_scale=True,
    bake_space_transform=False,
    axis_forward="-Z",
    axis_up="Y",
    object_types={"ARMATURE", "MESH"},
    add_leaf_bones=False,
    bake_anim=True,
    bake_anim_use_all_actions=False,
    bake_anim_use_nla_strips=True,
    bake_anim_force_startend_keying=True,
)

print(f"[warrior_mesh] wrote {OUT_FBX}")
