"""
Generate a rigged Turtle enemy skeletal mesh for UE5.

Creates SK_Turtle.fbx — dome shell on flat disc, 4 leg stubs, head bone,
rigged with a simple armature (root, shell, head, leg_fl, leg_fr, leg_bl, leg_br).

Usage:
    blender --background --python generate_turtle.py -- [output_dir]

Output:
    SK_Turtle.fbx — skeletal mesh (mesh + skeleton, bind pose)
    A_Turtle_Idle.fbx — head bobs, legs shift weight
    A_Turtle_Walk.fbx — slow plod cycle, legs alternate
    A_Turtle_Flip.fbx — legs tuck, shell rocks, flips upside-down
    A_Turtle_Flipped.fbx — legs wiggle helplessly (looping)
    A_Turtle_Unflip.fbx — rocks back to upright
    A_Turtle_Death.fbx — shell spins, falls flat
"""

import bpy
import math
import mathutils
import os
import sys

argv = sys.argv
args = argv[argv.index("--") + 1:] if "--" in argv else []
OUTPUT_DIR = args[0] if args else "."

Vector = mathutils.Vector


# ---------------------------------------------------------------------------
# Utilities
# ---------------------------------------------------------------------------

def clear_scene():
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=True)
    for block in [bpy.data.meshes, bpy.data.materials, bpy.data.metaballs,
                  bpy.data.armatures, bpy.data.actions]:
        for item in list(block):
            block.remove(item)


def _sel(obj):
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj


def _apply(obj):
    _sel(obj)
    for mod in list(obj.modifiers):
        if mod.type != 'ARMATURE':
            try:
                bpy.ops.object.modifier_apply(modifier=mod.name)
            except Exception:
                obj.modifiers.remove(mod)
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)


def join(parts):
    for p in parts:
        _apply(p)
    bpy.ops.object.select_all(action='DESELECT')
    for p in parts:
        p.select_set(True)
    bpy.context.view_layer.objects.active = parts[0]
    if len(parts) > 1:
        bpy.ops.object.join()
    return bpy.context.active_object


# ---------------------------------------------------------------------------
# Primitives
# ---------------------------------------------------------------------------

def sphere(loc, radius, segs=16, rings=8):
    bpy.ops.mesh.primitive_uv_sphere_add(
        segments=segs, ring_count=rings, radius=radius, location=loc)
    return bpy.context.active_object


def cone(loc, r1, r2, depth, rot=(0, 0, 0), segs=12):
    bpy.ops.mesh.primitive_cone_add(
        vertices=segs, radius1=r1, radius2=r2, depth=depth,
        location=loc, rotation=rot)
    return bpy.context.active_object


def cylinder(loc, radius, depth, rot=(0, 0, 0), segs=16):
    bpy.ops.mesh.primitive_cylinder_add(
        vertices=segs, radius=radius, depth=depth,
        location=loc, rotation=rot)
    return bpy.context.active_object


def cube(loc, scale, bevel=0):
    bpy.ops.mesh.primitive_cube_add(size=1, location=loc)
    o = bpy.context.active_object
    o.scale = scale
    if bevel > 0:
        _sel(o)
        bpy.ops.object.transform_apply(scale=True)
        mod = o.modifiers.new("B", 'BEVEL')
        mod.width = bevel
        mod.segments = 2
    return o


# ---------------------------------------------------------------------------
# Turtle Mesh — dome shell, flat disc belly, 4 leg stubs, head
# Faces -Y in Blender (UE +X forward after export)
# ---------------------------------------------------------------------------

def build_turtle_mesh():
    single_mat = bpy.data.materials.new("M_Turtle_Placeholder")
    single_mat.use_nodes = True
    bsdf = single_mat.node_tree.nodes["Principled BSDF"]
    bsdf.inputs["Base Color"].default_value = (0.35, 0.5, 0.2, 1.0)

    P = []

    # Shell — dome (top half of a flattened sphere)
    shell = sphere((0, 0, 0.12), 0.22, segs=20, rings=10)
    shell.scale = (1.0, 0.85, 0.6)
    _sel(shell)
    bpy.ops.object.transform_apply(scale=True)
    # Remove bottom half to make a dome
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_all(action='DESELECT')
    bpy.ops.object.mode_set(mode='OBJECT')
    for v in shell.data.vertices:
        if v.co.z < 0.11:
            v.select = True
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.delete(type='VERT')
    bpy.ops.object.mode_set(mode='OBJECT')
    P.append(shell)

    # Belly — flat disc underneath the shell
    belly = cylinder((0, 0, 0.10), 0.20, 0.04, segs=20)
    belly.scale = (1.0, 0.85, 1.0)
    P.append(belly)

    # Head — small sphere extending from front of shell
    head = sphere((0, -0.22, 0.12), 0.07)
    P.append(head)

    # Eyes on the head
    for side in (-1, 1):
        eye = sphere((side * 0.03, -0.27, 0.15), 0.015)
        P.append(eye)

    # 4 leg stubs — short cylinders
    leg_positions = [
        (-0.14, -0.10, 0.04),   # front-left
        (0.14, -0.10, 0.04),    # front-right
        (-0.14, 0.10, 0.04),    # back-left
        (0.14, 0.10, 0.04),     # back-right
    ]
    for pos in leg_positions:
        leg = cylinder(pos, 0.035, 0.08)
        P.append(leg)

    # Small tail at the back
    tail = cone((0, 0.20, 0.10), 0.03, 0.005, 0.08,
                rot=(math.pi / 2, 0, 0))
    P.append(tail)

    result = join(P)
    result.name = "Turtle_Mesh"
    result.data.materials.clear()
    result.data.materials.append(single_mat)

    _sel(result)
    bpy.ops.object.shade_smooth()

    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.uv.smart_project(angle_limit=1.15)
    bpy.ops.object.mode_set(mode='OBJECT')

    return result


# ---------------------------------------------------------------------------
# Armature — 7-bone rig
# ---------------------------------------------------------------------------

# (bone_name, head, tail, parent_name)
BONES = [
    ("root",    (0, 0, 0),          (0, 0, 0.10),       None),
    ("shell",   (0, 0, 0.10),       (0, 0, 0.22),       "root"),
    ("head",    (0, -0.15, 0.12),   (0, -0.25, 0.14),   "shell"),
    ("leg_fl",  (-0.14, -0.10, 0.08), (-0.14, -0.10, 0.0), "shell"),
    ("leg_fr",  (0.14, -0.10, 0.08),  (0.14, -0.10, 0.0),  "shell"),
    ("leg_bl",  (-0.14, 0.10, 0.08),  (-0.14, 0.10, 0.0),  "shell"),
    ("leg_br",  (0.14, 0.10, 0.08),   (0.14, 0.10, 0.0),   "shell"),
]

BONE_RADIUS = {
    "root": 0.30,
    "shell": 0.30,
    "head": 0.12,
    "leg_fl": 0.10,
    "leg_fr": 0.10,
    "leg_bl": 0.10,
    "leg_br": 0.10,
}


def build_armature():
    bpy.ops.object.armature_add(location=(0, 0, 0))
    arm_obj = bpy.context.active_object
    arm_obj.name = "Turtle_Armature"
    arm = arm_obj.data
    arm.name = "Turtle_Skeleton"

    bpy.ops.object.mode_set(mode='EDIT')

    for b in list(arm.edit_bones):
        arm.edit_bones.remove(b)

    bone_map = {}
    for name, head, tail, parent_name in BONES:
        b = arm.edit_bones.new(name)
        b.head = Vector(head)
        b.tail = Vector(tail)
        b.use_connect = False
        bone_map[name] = b

    for name, _, _, parent_name in BONES:
        if parent_name and parent_name in bone_map:
            bone_map[name].parent = bone_map[parent_name]

    bpy.ops.object.mode_set(mode='OBJECT')
    return arm_obj


# ---------------------------------------------------------------------------
# Skinning — distance-based vertex weighting
# ---------------------------------------------------------------------------

def _closest_on_segment(pt, a, b):
    ab = b - a
    d = ab.dot(ab)
    if d < 1e-10:
        return a
    t = max(0.0, min(1.0, (pt - a).dot(ab) / d))
    return a + ab * t


def skin_mesh(mesh_obj, arm_obj):
    bones_info = []
    for bone in arm_obj.data.bones:
        head = arm_obj.matrix_world @ bone.head_local
        tail = arm_obj.matrix_world @ bone.tail_local
        radius = BONE_RADIUS.get(bone.name, 0.25)
        vg = mesh_obj.vertex_groups.new(name=bone.name)
        bones_info.append((head, tail, radius, vg))

    for v in mesh_obj.data.vertices:
        vp = mesh_obj.matrix_world @ v.co
        weights = []
        for head, tail, radius, vg in bones_info:
            cp = _closest_on_segment(vp, head, tail)
            dist = (vp - cp).length
            w = max(0.0, 1.0 - dist / radius) ** 2
            if w > 0.001:
                weights.append((vg, w))
        total = sum(w for _, w in weights)
        if total > 0:
            for vg, w in weights:
                vg.add([v.index], w / total, 'REPLACE')
        else:
            best_vg = min(bones_info,
                          key=lambda b: (vp - _closest_on_segment(vp, b[0], b[1])).length)[3]
            best_vg.add([v.index], 1.0, 'REPLACE')

    mod = mesh_obj.modifiers.new("Armature", 'ARMATURE')
    mod.object = arm_obj
    mesh_obj.parent = arm_obj


# ---------------------------------------------------------------------------
# Animations
# ---------------------------------------------------------------------------

def create_action(arm_obj, name, total_frames, keyframes):
    action = bpy.data.actions.new(name)
    action.use_fake_user = True

    if not arm_obj.animation_data:
        arm_obj.animation_data_create()
    arm_obj.animation_data.action = action

    _sel(arm_obj)
    bpy.ops.object.mode_set(mode='POSE')

    for pb in arm_obj.pose.bones:
        pb.rotation_mode = 'XYZ'
        pb.rotation_euler = (0, 0, 0)
        pb.location = (0, 0, 0)

    for bone_name, frame, rot in keyframes:
        pb = arm_obj.pose.bones.get(bone_name)
        if not pb:
            continue
        pb.rotation_euler = rot
        pb.keyframe_insert(data_path="rotation_euler", frame=frame)

    bpy.ops.object.mode_set(mode='OBJECT')
    return action


def make_idle(arm):
    """A_Turtle_Idle (60 frames): Head bobs, legs shift weight."""
    keys = []

    # Head bobbing up and down
    for cycle in range(3):
        o = cycle * 20
        keys += [
            ("head", 1 + o,  (0.15, 0, 0)),
            ("head", 10 + o, (-0.15, 0, 0)),
        ]
    keys += [("head", 60, (0.15, 0, 0))]

    # Legs shift weight — subtle alternating movement
    for cycle in range(2):
        o = cycle * 30
        keys += [
            ("leg_fl", 1 + o,  (0.1, 0, 0)),
            ("leg_fr", 1 + o,  (-0.1, 0, 0)),
            ("leg_bl", 1 + o,  (-0.1, 0, 0)),
            ("leg_br", 1 + o,  (0.1, 0, 0)),
            ("leg_fl", 15 + o, (-0.1, 0, 0)),
            ("leg_fr", 15 + o, (0.1, 0, 0)),
            ("leg_bl", 15 + o, (0.1, 0, 0)),
            ("leg_br", 15 + o, (-0.1, 0, 0)),
        ]
    keys += [
        ("leg_fl", 60, (0.1, 0, 0)),
        ("leg_fr", 60, (-0.1, 0, 0)),
        ("leg_bl", 60, (-0.1, 0, 0)),
        ("leg_br", 60, (0.1, 0, 0)),
    ]

    # Shell very subtle rock
    keys += [
        ("shell", 1,  (0, 0, 0.02)),
        ("shell", 30, (0, 0, -0.02)),
        ("shell", 60, (0, 0, 0.02)),
    ]

    return create_action(arm, "A_Turtle_Idle", 60, keys)


def make_walk(arm):
    """A_Turtle_Walk (40 frames): Slow plod cycle, legs alternate."""
    keys = []

    # Trot gait — diagonal legs move together
    step_frames = 10
    for step in range(4):
        o = step * step_frames
        phase = 1 if step % 2 == 0 else -1
        swing = 0.45

        # Diagonal pairs
        keys += [
            ("leg_fl", 1 + o, (phase * swing, 0, 0)),
            ("leg_br", 1 + o, (phase * swing, 0, 0)),
            ("leg_fr", 1 + o, (-phase * swing, 0, 0)),
            ("leg_bl", 1 + o, (-phase * swing, 0, 0)),
        ]

        # Shell rocks side-to-side with the gait
        keys += [
            ("shell", 1 + o, (0.03, 0, phase * 0.06)),
        ]

        # Head bobs with steps
        keys += [
            ("head", 1 + o, (phase * 0.12, 0, 0)),
        ]

    # Loop endpoint
    keys += [
        ("leg_fl", 40, (0.45, 0, 0)),
        ("leg_br", 40, (0.45, 0, 0)),
        ("leg_fr", 40, (-0.45, 0, 0)),
        ("leg_bl", 40, (-0.45, 0, 0)),
        ("shell",  40, (0.03, 0, 0.06)),
        ("head",   40, (0.12, 0, 0)),
    ]

    return create_action(arm, "A_Turtle_Walk", 40, keys)


def make_flip(arm):
    """A_Turtle_Flip (20 frames): Legs tuck, shell rocks, flips upside-down."""
    keys = []

    # Frame 1: neutral
    for b in ["shell", "head", "leg_fl", "leg_fr", "leg_bl", "leg_br"]:
        keys.append((b, 1, (0, 0, 0)))

    # Frame 5: legs start tucking in, shell tilts
    keys += [
        ("shell",  5, (0.3, 0, 0)),
        ("head",   5, (0.2, 0, 0)),
        ("leg_fl", 5, (0.5, 0, 0.3)),
        ("leg_fr", 5, (0.5, 0, -0.3)),
        ("leg_bl", 5, (-0.3, 0, 0.3)),
        ("leg_br", 5, (-0.3, 0, -0.3)),
    ]

    # Frame 10: rocking over, legs fully tucked
    keys += [
        ("shell",  10, (0.8, 0, 0)),
        ("head",   10, (0.5, 0, 0)),
        ("leg_fl", 10, (0.8, 0, 0.5)),
        ("leg_fr", 10, (0.8, 0, -0.5)),
        ("leg_bl", 10, (0.6, 0, 0.5)),
        ("leg_br", 10, (0.6, 0, -0.5)),
    ]

    # Frame 15: flipped over (upside-down)
    keys += [
        ("shell",  15, (math.pi, 0, 0)),
        ("head",   15, (0.8, 0, 0)),
        ("leg_fl", 15, (0.6, 0, 0.4)),
        ("leg_fr", 15, (0.6, 0, -0.4)),
        ("leg_bl", 15, (0.6, 0, 0.4)),
        ("leg_br", 15, (0.6, 0, -0.4)),
    ]

    # Frame 20: settle upside-down
    keys += [
        ("shell",  20, (math.pi, 0, 0)),
        ("head",   20, (0.7, 0, 0)),
        ("leg_fl", 20, (0.5, 0, 0.3)),
        ("leg_fr", 20, (0.5, 0, -0.3)),
        ("leg_bl", 20, (0.5, 0, 0.3)),
        ("leg_br", 20, (0.5, 0, -0.3)),
    ]

    return create_action(arm, "A_Turtle_Flip", 20, keys)


def make_flipped(arm):
    """A_Turtle_Flipped (40 frames): Legs wiggle helplessly (looping)."""
    keys = []

    # Shell stays flipped, legs wiggle
    for cycle in range(4):
        o = cycle * 10
        phase = 1 if cycle % 2 == 0 else -1

        keys += [
            ("shell", 1 + o, (math.pi, 0, phase * 0.05)),
            ("head",  1 + o, (0.7, phase * 0.1, 0)),
        ]

        # Legs flail
        keys += [
            ("leg_fl", 1 + o, (0.5 + phase * 0.4, 0, 0.3 + phase * 0.2)),
            ("leg_fr", 1 + o, (0.5 - phase * 0.4, 0, -0.3 + phase * 0.2)),
            ("leg_bl", 1 + o, (0.5 - phase * 0.3, 0, 0.3 - phase * 0.2)),
            ("leg_br", 1 + o, (0.5 + phase * 0.3, 0, -0.3 - phase * 0.2)),
        ]

    # Loop endpoint
    keys += [
        ("shell",  40, (math.pi, 0, 0.05)),
        ("head",   40, (0.7, 0.1, 0)),
        ("leg_fl", 40, (0.9, 0, 0.5)),
        ("leg_fr", 40, (0.1, 0, -0.1)),
        ("leg_bl", 40, (0.2, 0, 0.1)),
        ("leg_br", 40, (0.8, 0, -0.5)),
    ]

    return create_action(arm, "A_Turtle_Flipped", 40, keys)


def make_unflip(arm):
    """A_Turtle_Unflip (20 frames): Rocks back to upright."""
    keys = []

    # Frame 1: upside-down
    keys += [
        ("shell",  1, (math.pi, 0, 0)),
        ("head",   1, (0.7, 0, 0)),
        ("leg_fl", 1, (0.5, 0, 0.3)),
        ("leg_fr", 1, (0.5, 0, -0.3)),
        ("leg_bl", 1, (0.5, 0, 0.3)),
        ("leg_br", 1, (0.5, 0, -0.3)),
    ]

    # Frame 6: rocking, gathering momentum
    keys += [
        ("shell",  6, (math.pi * 0.75, 0, 0.15)),
        ("head",   6, (0.4, 0, 0)),
        ("leg_fl", 6, (0.3, 0, 0.2)),
        ("leg_fr", 6, (0.3, 0, -0.2)),
        ("leg_bl", 6, (0.3, 0, 0.2)),
        ("leg_br", 6, (0.3, 0, -0.2)),
    ]

    # Frame 12: past the tipping point
    keys += [
        ("shell",  12, (math.pi * 0.3, 0, -0.1)),
        ("head",   12, (0.1, 0, 0)),
        ("leg_fl", 12, (0.15, 0, 0.1)),
        ("leg_fr", 12, (0.15, 0, -0.1)),
        ("leg_bl", 12, (-0.15, 0, 0.1)),
        ("leg_br", 12, (-0.15, 0, -0.1)),
    ]

    # Frame 16: almost upright, bounce
    keys += [
        ("shell",  16, (-0.1, 0, 0.05)),
        ("head",   16, (-0.05, 0, 0)),
        ("leg_fl", 16, (0.05, 0, 0)),
        ("leg_fr", 16, (0.05, 0, 0)),
        ("leg_bl", 16, (-0.05, 0, 0)),
        ("leg_br", 16, (-0.05, 0, 0)),
    ]

    # Frame 20: settled upright
    for b in ["shell", "head", "leg_fl", "leg_fr", "leg_bl", "leg_br"]:
        keys.append((b, 20, (0, 0, 0)))

    return create_action(arm, "A_Turtle_Unflip", 20, keys)


def make_death(arm):
    """A_Turtle_Death (30 frames): Shell spins, falls flat."""
    keys = []

    # Frame 1: neutral
    for b in ["shell", "head", "leg_fl", "leg_fr", "leg_bl", "leg_br"]:
        keys.append((b, 1, (0, 0, 0)))

    # Frame 6: shell starts spinning (Z rotation), lifts slightly
    keys += [
        ("shell",  6, (0.1, 0, math.pi * 0.5)),
        ("head",   6, (0.2, 0, 0)),
        ("leg_fl", 6, (0.3, 0, 0.2)),
        ("leg_fr", 6, (0.3, 0, -0.2)),
        ("leg_bl", 6, (-0.2, 0, 0.2)),
        ("leg_br", 6, (-0.2, 0, -0.2)),
    ]

    # Frame 12: spinning faster
    keys += [
        ("shell",  12, (0.2, 0, math.pi * 1.5)),
        ("head",   12, (0.4, 0, 0)),
        ("leg_fl", 12, (0.5, 0, 0.4)),
        ("leg_fr", 12, (0.5, 0, -0.4)),
        ("leg_bl", 12, (0.4, 0, 0.4)),
        ("leg_br", 12, (0.4, 0, -0.4)),
    ]

    # Frame 18: spin slowing, tipping over
    keys += [
        ("shell",  18, (0.6, 0.3, math.pi * 2.5)),
        ("head",   18, (0.5, 0.2, 0)),
        ("leg_fl", 18, (0.5, 0, 0.3)),
        ("leg_fr", 18, (0.5, 0, -0.3)),
        ("leg_bl", 18, (0.4, 0, 0.3)),
        ("leg_br", 18, (0.4, 0, -0.3)),
    ]

    # Frame 24: collapsed on side
    keys += [
        ("shell",  24, (0.5, math.pi / 2, math.pi * 2.8)),
        ("head",   24, (0.6, 0.3, 0)),
        ("leg_fl", 24, (0.3, 0, 0.2)),
        ("leg_fr", 24, (0.3, 0, -0.2)),
        ("leg_bl", 24, (0.3, 0, 0.2)),
        ("leg_br", 24, (0.3, 0, -0.2)),
    ]

    # Frame 30: hold — settled
    keys += [
        ("shell",  30, (0.5, math.pi / 2, math.pi * 2.8)),
        ("head",   30, (0.6, 0.3, 0)),
        ("leg_fl", 30, (0.3, 0, 0.2)),
        ("leg_fr", 30, (0.3, 0, -0.2)),
        ("leg_bl", 30, (0.3, 0, 0.2)),
        ("leg_br", 30, (0.3, 0, -0.2)),
    ]

    return create_action(arm, "A_Turtle_Death", 30, keys)


# ---------------------------------------------------------------------------
# Export
# ---------------------------------------------------------------------------

FBX_COMMON = dict(
    apply_unit_scale=False,
    apply_scale_options='FBX_SCALE_NONE',
    axis_forward='-Y',
    axis_up='Z',
    add_leaf_bones=False,
)


def export_skeletal_mesh(mesh_obj, arm_obj, filepath):
    if arm_obj.animation_data:
        arm_obj.animation_data.action = None

    bpy.ops.object.select_all(action='DESELECT')
    mesh_obj.select_set(True)
    arm_obj.select_set(True)
    bpy.context.view_layer.objects.active = arm_obj

    bpy.ops.export_scene.fbx(
        filepath=filepath,
        use_selection=True,
        object_types={'MESH', 'ARMATURE'},
        mesh_smooth_type='FACE',
        use_mesh_modifiers=True,
        bake_anim=False,
        **FBX_COMMON,
    )


def export_animation(mesh_obj, arm_obj, action, filepath):
    if not arm_obj.animation_data:
        arm_obj.animation_data_create()
    arm_obj.animation_data.action = action

    bpy.context.scene.frame_start = int(action.frame_range[0])
    bpy.context.scene.frame_end = int(action.frame_range[1])

    bpy.ops.object.select_all(action='DESELECT')
    mesh_obj.select_set(True)
    arm_obj.select_set(True)
    bpy.context.view_layer.objects.active = arm_obj

    bpy.ops.export_scene.fbx(
        filepath=filepath,
        use_selection=True,
        object_types={'MESH', 'ARMATURE'},
        mesh_smooth_type='FACE',
        use_mesh_modifiers=True,
        bake_anim=True,
        bake_anim_use_all_bones=True,
        bake_anim_use_nla_strips=False,
        bake_anim_use_all_actions=False,
        bake_anim_force_startend_keying=True,
        bake_anim_step=1.0,
        **FBX_COMMON,
    )


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    bpy.context.scene.render.fps = 30

    clear_scene()

    print("  Building turtle mesh...")
    mesh_obj = build_turtle_mesh()

    print("  Building armature...")
    arm_obj = build_armature()

    print("  Skinning...")
    skin_mesh(mesh_obj, arm_obj)

    print("  Creating animations...")
    actions = [
        make_idle(arm_obj),
        make_walk(arm_obj),
        make_flip(arm_obj),
        make_flipped(arm_obj),
        make_unflip(arm_obj),
        make_death(arm_obj),
    ]

    # Rotate from Blender -Y forward to +X forward for UE
    arm_obj.rotation_euler[2] = math.pi / 2

    sk_path = os.path.join(OUTPUT_DIR, "SK_Turtle.fbx")
    print("  Exporting skeletal mesh...")
    export_skeletal_mesh(mesh_obj, arm_obj, sk_path)
    print(f"  Exported: {sk_path}")

    for action in actions:
        anim_path = os.path.join(OUTPUT_DIR, f"{action.name}.fbx")
        print(f"  Exporting {action.name}...")
        export_animation(mesh_obj, arm_obj, action, anim_path)
        print(f"  Exported: {anim_path}")

    print("Done.")


main()
