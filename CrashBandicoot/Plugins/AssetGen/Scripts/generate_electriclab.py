"""
Generate a rigged Electric Lab Assistant enemy skeletal mesh for UE5.

Creates SK_ElectricLabAssistant.fbx — capsule torso, arm bones, wire cones
on top (electrodes), rigged with armature (root, spine, head, arm_l, arm_r,
electrode_l, electrode_r, leg_l, leg_r).

Usage:
    blender --background --python generate_electriclab.py -- [output_dir]

Output:
    SK_ElectricLabAssistant.fbx — skeletal mesh (mesh + skeleton, bind pose)
    A_ElectricLab_Idle.fbx — electrodes spark, slight sway
    A_ElectricLab_Walk.fbx — forward march, arms slightly raised
    A_ElectricLab_Pursue.fbx — faster walk, leaning forward
    A_ElectricLab_Death.fbx — electrodes short out, collapse
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
# Electric Lab Assistant Mesh — capsule torso, head with electrode cones,
# arms, legs. Faces -Y in Blender. About 0.70 Blender units tall.
# ---------------------------------------------------------------------------

def build_electriclab_mesh():
    single_mat = bpy.data.materials.new("M_ElectricLab_Placeholder")
    single_mat.use_nodes = True
    bsdf = single_mat.node_tree.nodes["Principled BSDF"]
    bsdf.inputs["Base Color"].default_value = (0.80, 0.82, 0.85, 1.0)

    P = []

    # Torso — capsule (cylinder + half-spheres top/bottom)
    torso = cylinder((0, 0, 0.32), 0.10, 0.26)
    P.append(torso)
    torso_top = sphere((0, 0, 0.45), 0.10)
    P.append(torso_top)
    torso_bot = sphere((0, 0, 0.19), 0.10)
    P.append(torso_bot)

    # Head
    head = sphere((0, -0.02, 0.56), 0.09)
    P.append(head)

    # Eyes
    for side in (-1, 1):
        eye = sphere((side * 0.04, -0.07, 0.58), 0.02)
        P.append(eye)

    # Headgear base — band around head
    headband = cylinder((0, 0, 0.62), 0.10, 0.02, segs=16)
    P.append(headband)

    # Left electrode — cone pointing upward on left side of head
    electrode_l_base = cylinder((-0.09, 0, 0.64), 0.025, 0.04)
    P.append(electrode_l_base)
    electrode_l_tip = cone((-0.09, 0, 0.70), 0.02, 0.005, 0.08,
                           rot=(0, 0, 0), segs=8)
    P.append(electrode_l_tip)
    # Wire from headband to electrode
    wire_l = cylinder((-0.09, 0, 0.63), 0.008, 0.02)
    P.append(wire_l)

    # Right electrode — cone pointing upward on right side of head
    electrode_r_base = cylinder((0.09, 0, 0.64), 0.025, 0.04)
    P.append(electrode_r_base)
    electrode_r_tip = cone((0.09, 0, 0.70), 0.02, 0.005, 0.08,
                           rot=(0, 0, 0), segs=8)
    P.append(electrode_r_tip)
    # Wire from headband to electrode
    wire_r = cylinder((0.09, 0, 0.63), 0.008, 0.02)
    P.append(wire_r)

    # Left arm
    arm_l_upper = cylinder((-0.15, 0, 0.40), 0.035, 0.14,
                           rot=(0, 0, math.radians(12)))
    P.append(arm_l_upper)
    arm_l_lower = cylinder((-0.17, 0, 0.26), 0.03, 0.12,
                           rot=(0, 0, math.radians(8)))
    P.append(arm_l_lower)
    hand_l = sphere((-0.18, 0, 0.19), 0.035)
    P.append(hand_l)

    # Right arm
    arm_r_upper = cylinder((0.15, 0, 0.40), 0.035, 0.14,
                           rot=(0, 0, math.radians(-12)))
    P.append(arm_r_upper)
    arm_r_lower = cylinder((0.17, 0, 0.26), 0.03, 0.12,
                           rot=(0, 0, math.radians(-8)))
    P.append(arm_r_lower)
    hand_r = sphere((0.18, 0, 0.19), 0.035)
    P.append(hand_r)

    # Legs
    for side in (-1, 1):
        x = side * 0.06
        thigh = cylinder((x, 0, 0.12), 0.04, 0.12)
        P.append(thigh)
        shin = cylinder((x, 0, 0.02), 0.03, 0.10)
        P.append(shin)
        foot = cube((x, -0.03, -0.04), (0.035, 0.055, 0.015))
        P.append(foot)

    # Lab coat flaps
    for i in range(8):
        angle = i * math.pi * 2 / 8
        x = math.cos(angle) * 0.11
        y = math.sin(angle) * 0.11
        flap = cone((x, y, 0.16), 0.03, 0.01, 0.06)
        P.append(flap)

    result = join(P)
    result.name = "ElectricLab_Mesh"
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
# Armature — 9-bone rig
# ---------------------------------------------------------------------------

BONES = [
    ("root",          (0, 0, 0),            (0, 0, 0.19),        None),
    ("spine",         (0, 0, 0.19),         (0, 0, 0.45),        "root"),
    ("head",          (0, 0, 0.45),         (0, 0, 0.62),        "spine"),
    ("arm_l",         (-0.12, 0, 0.42),     (-0.18, 0, 0.19),    "spine"),
    ("arm_r",         (0.12, 0, 0.42),      (0.18, 0, 0.19),     "spine"),
    ("electrode_l",   (-0.09, 0, 0.62),     (-0.09, 0, 0.74),    "head"),
    ("electrode_r",   (0.09, 0, 0.62),      (0.09, 0, 0.74),     "head"),
    ("leg_l",         (-0.06, 0, 0.19),     (-0.06, 0, -0.04),   "root"),
    ("leg_r",         (0.06, 0, 0.19),      (0.06, 0, -0.04),    "root"),
]

BONE_RADIUS = {
    "root": 0.25,
    "spine": 0.20,
    "head": 0.15,
    "arm_l": 0.12,
    "arm_r": 0.12,
    "electrode_l": 0.07,
    "electrode_r": 0.07,
    "leg_l": 0.10,
    "leg_r": 0.10,
}


def build_armature():
    bpy.ops.object.armature_add(location=(0, 0, 0))
    arm_obj = bpy.context.active_object
    arm_obj.name = "ElectricLab_Armature"
    arm = arm_obj.data
    arm.name = "ElectricLab_Skeleton"

    bpy.ops.object.mode_set(mode='EDIT')

    for b in list(arm.edit_bones):
        arm.edit_bones.remove(b)

    bone_map = {}
    for name, head_pos, tail_pos, parent_name in BONES:
        b = arm.edit_bones.new(name)
        b.head = Vector(head_pos)
        b.tail = Vector(tail_pos)
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
        head_pos = arm_obj.matrix_world @ bone.head_local
        tail_pos = arm_obj.matrix_world @ bone.tail_local
        radius = BONE_RADIUS.get(bone.name, 0.25)
        vg = mesh_obj.vertex_groups.new(name=bone.name)
        bones_info.append((head_pos, tail_pos, radius, vg))

    for v in mesh_obj.data.vertices:
        vp = mesh_obj.matrix_world @ v.co
        weights = []
        for head_pos, tail_pos, radius, vg in bones_info:
            cp = _closest_on_segment(vp, head_pos, tail_pos)
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
    keys = []
    # Electrodes spark — jittery twitching
    for cycle in range(4):
        o = cycle * 15
        keys += [
            ("electrode_l", 1 + o,  (0.08, 0.05, 0)),
            ("electrode_l", 4 + o,  (-0.10, -0.06, 0.04)),
            ("electrode_l", 8 + o,  (0.06, 0.03, -0.03)),
            ("electrode_r", 1 + o,  (-0.06, 0.04, 0)),
            ("electrode_r", 4 + o,  (0.09, -0.05, -0.03)),
            ("electrode_r", 8 + o,  (-0.07, 0.03, 0.04)),
        ]
    keys += [
        ("electrode_l", 60, (0.08, 0.05, 0)),
        ("electrode_r", 60, (-0.06, 0.04, 0)),
    ]

    # Subtle arm twitch from electrical current
    for cycle in range(3):
        o = cycle * 20
        keys += [
            ("arm_l", 1 + o,  (0.04, 0, 0.03)),
            ("arm_l", 5 + o,  (-0.03, 0, -0.04)),
            ("arm_l", 10 + o, (0.05, 0, 0.02)),
            ("arm_r", 1 + o,  (-0.03, 0, -0.03)),
            ("arm_r", 5 + o,  (0.04, 0, 0.04)),
            ("arm_r", 10 + o, (-0.04, 0, -0.02)),
        ]
    keys += [
        ("arm_l", 60, (0.04, 0, 0.03)),
        ("arm_r", 60, (-0.03, 0, -0.03)),
    ]

    # Body slight sway
    keys += [
        ("spine", 1,  (0, 0, 0.03)),
        ("spine", 30, (0, 0, -0.03)),
        ("spine", 60, (0, 0, 0.03)),
    ]

    # Head
    keys += [
        ("head", 1,  (0, 0, 0.04)),
        ("head", 30, (0, 0, -0.04)),
        ("head", 60, (0, 0, 0.04)),
    ]

    return create_action(arm, "A_ElectricLab_Idle", 60, keys)


def make_walk(arm):
    keys = []
    step_frames = 10
    for step in range(4):
        o = step * step_frames
        phase = 1 if step % 2 == 0 else -1
        swing = 0.35

        # Legs alternate
        keys += [
            ("leg_l", 1 + o, (phase * swing, 0, 0)),
            ("leg_r", 1 + o, (-phase * swing, 0, 0)),
        ]

        # Arms slightly raised, swing opposite to legs
        keys += [
            ("arm_l", 1 + o, (-phase * 0.25 - 0.15, 0, -0.1)),
            ("arm_r", 1 + o, (phase * 0.25 - 0.15, 0, 0.1)),
        ]

        # Electrodes bob
        keys += [
            ("electrode_l", 1 + o, (phase * 0.06, 0, 0)),
            ("electrode_r", 1 + o, (-phase * 0.06, 0, 0)),
        ]

        # Spine slight twist
        keys += [
            ("spine", 1 + o, (0, 0, phase * 0.04)),
        ]

        # Head stays stable
        keys += [
            ("head", 1 + o, (0, 0, -phase * 0.03)),
        ]

    # Loop endpoint
    keys += [
        ("leg_l", 40, (0.35, 0, 0)),
        ("leg_r", 40, (-0.35, 0, 0)),
        ("arm_l", 40, (-0.25 - 0.15, 0, -0.1)),
        ("arm_r", 40, (0.25 - 0.15, 0, 0.1)),
        ("electrode_l", 40, (0.06, 0, 0)),
        ("electrode_r", 40, (-0.06, 0, 0)),
        ("spine", 40, (0, 0, 0.04)),
        ("head",  40, (0, 0, -0.03)),
    ]

    return create_action(arm, "A_ElectricLab_Walk", 40, keys)


def make_pursue(arm):
    keys = []
    step_frames = 7  # faster cadence than walk
    for step in range(4):
        o = step * step_frames
        phase = 1 if step % 2 == 0 else -1
        swing = 0.45  # wider stride

        # Legs alternate — bigger swings
        keys += [
            ("leg_l", 1 + o, (phase * swing, 0, 0)),
            ("leg_r", 1 + o, (-phase * swing, 0, 0)),
        ]

        # Arms more raised, more swing
        keys += [
            ("arm_l", 1 + o, (-phase * 0.35 - 0.25, 0, -0.15)),
            ("arm_r", 1 + o, (phase * 0.35 - 0.25, 0, 0.15)),
        ]

        # Electrodes bounce more aggressively
        keys += [
            ("electrode_l", 1 + o, (phase * 0.12, 0.05, 0)),
            ("electrode_r", 1 + o, (-phase * 0.12, -0.05, 0)),
        ]

        # Spine leaning forward + twist
        keys += [
            ("spine", 1 + o, (-0.15, 0, phase * 0.06)),
        ]

        # Head leaning forward
        keys += [
            ("head", 1 + o, (-0.1, 0, -phase * 0.04)),
        ]

    # Loop endpoint (at frame 30, step 0 phase)
    keys += [
        ("leg_l", 30, (0.45, 0, 0)),
        ("leg_r", 30, (-0.45, 0, 0)),
        ("arm_l", 30, (-0.35 - 0.25, 0, -0.15)),
        ("arm_r", 30, (0.35 - 0.25, 0, 0.15)),
        ("electrode_l", 30, (0.12, 0.05, 0)),
        ("electrode_r", 30, (-0.12, -0.05, 0)),
        ("spine", 30, (-0.15, 0, 0.06)),
        ("head",  30, (-0.1, 0, -0.04)),
    ]

    return create_action(arm, "A_ElectricLab_Pursue", 30, keys)


def make_death(arm):
    keys = []
    # Frame 1: neutral
    for b in ["spine", "head", "arm_l", "arm_r", "electrode_l", "electrode_r",
              "leg_l", "leg_r"]:
        keys.append((b, 1, (0, 0, 0)))

    # Frame 5: electrodes short out — wild jitter
    keys += [
        ("electrode_l", 5, (0.4, 0.3, -0.2)),
        ("electrode_r", 5, (-0.5, -0.3, 0.3)),
        ("head",        5, (0.1, 0, 0.15)),
        ("arm_l",       5, (-0.2, 0, -0.3)),
        ("arm_r",       5, (-0.2, 0, 0.3)),
        ("spine",       5, (0.1, 0, 0)),
    ]

    # Frame 10: arms flail outward, stumbling
    keys += [
        ("electrode_l", 10, (-0.3, -0.5, 0.4)),
        ("electrode_r", 10, (0.4, 0.5, -0.3)),
        ("arm_l",       10, (-0.5, 0, -0.6)),
        ("arm_r",       10, (-0.5, 0, 0.6)),
        ("spine",       10, (0.3, 0, 0.1)),
        ("head",        10, (0.3, 0, -0.1)),
        ("leg_l",       10, (-0.1, 0, 0)),
        ("leg_r",       10, (-0.1, 0, 0)),
    ]

    # Frame 18: collapsing — falling backward
    keys += [
        ("electrode_l", 18, (0.6, 0.2, -0.3)),
        ("electrode_r", 18, (-0.5, -0.2, 0.4)),
        ("arm_l",       18, (-0.7, 0, -0.8)),
        ("arm_r",       18, (-0.6, 0, 0.8)),
        ("spine",       18, (0.6, 0, 0)),
        ("head",        18, (0.5, 0, 0.1)),
        ("leg_l",       18, (-0.3, 0, 0.05)),
        ("leg_r",       18, (-0.25, 0, -0.05)),
    ]

    # Frame 24: on the ground
    keys += [
        ("electrode_l", 24, (0.8, 0.1, -0.2)),
        ("electrode_r", 24, (-0.7, -0.1, 0.3)),
        ("arm_l",       24, (-0.8, 0, -0.7)),
        ("arm_r",       24, (-0.7, 0, 0.8)),
        ("spine",       24, (math.pi * 0.45, 0, 0)),
        ("head",        24, (0.5, 0, 0)),
        ("leg_l",       24, (-0.5, 0, 0.1)),
        ("leg_r",       24, (-0.4, 0, -0.1)),
    ]

    # Frame 30: hold
    keys += [
        ("electrode_l", 30, (0.8, 0.1, -0.2)),
        ("electrode_r", 30, (-0.7, -0.1, 0.3)),
        ("arm_l",       30, (-0.8, 0, -0.7)),
        ("arm_r",       30, (-0.7, 0, 0.8)),
        ("spine",       30, (math.pi * 0.45, 0, 0)),
        ("head",        30, (0.5, 0, 0)),
        ("leg_l",       30, (-0.5, 0, 0.1)),
        ("leg_r",       30, (-0.4, 0, -0.1)),
    ]

    return create_action(arm, "A_ElectricLab_Death", 30, keys)


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

    print("  Building electric lab assistant mesh...")
    mesh_obj = build_electriclab_mesh()

    print("  Building armature...")
    arm_obj = build_armature()

    print("  Skinning...")
    skin_mesh(mesh_obj, arm_obj)

    print("  Creating animations...")
    actions = [
        make_idle(arm_obj),
        make_walk(arm_obj),
        make_pursue(arm_obj),
        make_death(arm_obj),
    ]

    # Rotate from Blender -Y forward to +X forward for UE
    arm_obj.rotation_euler[2] = math.pi / 2

    sk_path = os.path.join(OUTPUT_DIR, "SK_ElectricLabAssistant.fbx")
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
