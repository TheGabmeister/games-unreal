"""
Generate a rigged Tribesman enemy skeletal mesh for UE5.

Creates SK_Tribesman.fbx — capsule torso, arm bones, club on right hand,
rigged with armature (root, spine, head, arm_l, arm_r, club, leg_l, leg_r).

Usage:
    blender --background --python generate_tribesman.py -- [output_dir]

Output:
    SK_Tribesman.fbx — skeletal mesh (mesh + skeleton, bind pose)
    A_Tribesman_Idle.fbx — club on shoulder, slight sway
    A_Tribesman_Walk.fbx — forward march, club bobs
    A_Tribesman_Death.fbx — drops club, falls backward
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
# Tribesman Mesh — capsule torso, head, arms, legs, club
# Faces -Y in Blender (rotated to +X for UE before export)
# ---------------------------------------------------------------------------

def build_tribesman_mesh():
    single_mat = bpy.data.materials.new("M_Tribesman_Placeholder")
    single_mat.use_nodes = True
    bsdf = single_mat.node_tree.nodes["Principled BSDF"]
    bsdf.inputs["Base Color"].default_value = (0.55, 0.35, 0.20, 1.0)

    P = []

    # Torso — capsule (cylinder + half-spheres top/bottom)
    torso = cylinder((0, 0, 0.30), 0.10, 0.28)
    P.append(torso)
    torso_top = sphere((0, 0, 0.44), 0.10)
    P.append(torso_top)
    torso_bot = sphere((0, 0, 0.16), 0.10)
    P.append(torso_bot)

    # Head
    head = sphere((0, -0.02, 0.55), 0.09)
    P.append(head)

    # Eyes
    for side in (-1, 1):
        eye = sphere((side * 0.04, -0.07, 0.57), 0.02)
        P.append(eye)

    # Nose
    nose = cone((0, -0.10, 0.54), 0.02, 0.005, 0.04,
                rot=(math.pi / 2, 0, 0))
    P.append(nose)

    # Left arm
    arm_l_upper = cylinder((-0.15, 0, 0.38), 0.04, 0.14,
                           rot=(0, 0, math.radians(15)))
    P.append(arm_l_upper)
    arm_l_lower = cylinder((-0.18, 0, 0.24), 0.035, 0.12,
                           rot=(0, 0, math.radians(10)))
    P.append(arm_l_lower)
    hand_l = sphere((-0.19, 0, 0.17), 0.04)
    P.append(hand_l)

    # Right arm (raised, holding club on shoulder)
    arm_r_upper = cylinder((0.15, 0, 0.38), 0.04, 0.14,
                           rot=(0, 0, math.radians(-15)))
    P.append(arm_r_upper)
    arm_r_lower = cylinder((0.17, -0.02, 0.46), 0.035, 0.12,
                           rot=(math.radians(-40), 0, math.radians(-10)))
    P.append(arm_r_lower)
    hand_r = sphere((0.17, -0.05, 0.52), 0.04)
    P.append(hand_r)

    # Club — horizontal bar on right hand
    club_handle = cylinder((0.17, -0.05, 0.60), 0.02, 0.16)
    P.append(club_handle)
    club_head = sphere((0.17, -0.05, 0.70), 0.05)
    P.append(club_head)

    # Legs
    for side in (-1, 1):
        x = side * 0.06
        thigh = cylinder((x, 0, 0.10), 0.045, 0.12)
        P.append(thigh)
        shin = cylinder((x, 0, 0.0), 0.035, 0.10)
        P.append(shin)
        foot = cube((x, -0.03, -0.06), (0.04, 0.06, 0.02))
        P.append(foot)

    # Grass skirt (ring of cones around waist)
    for i in range(10):
        angle = i * math.pi * 2 / 10
        x = math.cos(angle) * 0.11
        y = math.sin(angle) * 0.11
        skirt = cone((x, y, 0.14), 0.03, 0.01, 0.08)
        P.append(skirt)

    result = join(P)
    result.name = "Tribesman_Mesh"
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
# Armature — 8-bone rig
# ---------------------------------------------------------------------------

BONES = [
    ("root",    (0, 0, 0),          (0, 0, 0.16),      None),
    ("spine",   (0, 0, 0.16),       (0, 0, 0.44),      "root"),
    ("head",    (0, 0, 0.44),       (0, 0, 0.58),      "spine"),
    ("arm_l",   (-0.12, 0, 0.40),   (-0.19, 0, 0.17),  "spine"),
    ("arm_r",   (0.12, 0, 0.40),    (0.17, -0.05, 0.52), "spine"),
    ("club",    (0.17, -0.05, 0.52),(0.17, -0.05, 0.70), "arm_r"),
    ("leg_l",   (-0.06, 0, 0.16),   (-0.06, 0, -0.06),  "root"),
    ("leg_r",   (0.06, 0, 0.16),    (0.06, 0, -0.06),   "root"),
]

BONE_RADIUS = {
    "root": 0.25,
    "spine": 0.20,
    "head": 0.15,
    "arm_l": 0.12,
    "arm_r": 0.12,
    "club": 0.08,
    "leg_l": 0.10,
    "leg_r": 0.10,
}


def build_armature():
    bpy.ops.object.armature_add(location=(0, 0, 0))
    arm_obj = bpy.context.active_object
    arm_obj.name = "Tribesman_Armature"
    arm = arm_obj.data
    arm.name = "Tribesman_Skeleton"

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
    # Club resting on shoulder — slight arm sway
    for cycle in range(3):
        o = cycle * 20
        keys += [
            ("arm_r", 1 + o,  (0.05, 0, -0.03)),
            ("arm_r", 10 + o, (-0.05, 0, 0.03)),
            ("club",  1 + o,  (0.03, 0, 0)),
            ("club",  10 + o, (-0.03, 0, 0)),
        ]
    keys += [
        ("arm_r", 60, (0.05, 0, -0.03)),
        ("club",  60, (0.03, 0, 0)),
    ]

    # Body slight sway
    keys += [
        ("spine", 1,  (0, 0, 0.02)),
        ("spine", 30, (0, 0, -0.02)),
        ("spine", 60, (0, 0, 0.02)),
    ]

    # Head look around
    keys += [
        ("head", 1,  (0, 0, 0.05)),
        ("head", 20, (0, 0, -0.08)),
        ("head", 40, (0, 0, 0.05)),
        ("head", 60, (0, 0, 0.05)),
    ]

    # Left arm hangs, slight swing
    keys += [
        ("arm_l", 1,  (0.02, 0, 0)),
        ("arm_l", 30, (-0.02, 0, 0)),
        ("arm_l", 60, (0.02, 0, 0)),
    ]

    return create_action(arm, "A_Tribesman_Idle", 60, keys)


def make_walk(arm):
    keys = []
    step_frames = 10
    for step in range(4):
        o = step * step_frames
        phase = 1 if step % 2 == 0 else -1
        swing = 0.4

        # Legs alternate
        keys += [
            ("leg_l", 1 + o, (phase * swing, 0, 0)),
            ("leg_r", 1 + o, (-phase * swing, 0, 0)),
        ]

        # Arms swing opposite to legs
        keys += [
            ("arm_l", 1 + o, (-phase * 0.3, 0, 0)),
            ("arm_r", 1 + o, (phase * 0.15, 0, 0)),
        ]

        # Club bobs with arm
        keys += [
            ("club", 1 + o, (phase * 0.1, 0, 0)),
        ]

        # Spine slight twist
        keys += [
            ("spine", 1 + o, (0, 0, phase * 0.04)),
        ]

        # Head stays relatively stable
        keys += [
            ("head", 1 + o, (0, 0, -phase * 0.03)),
        ]

    # Loop endpoint
    keys += [
        ("leg_l", 40, (0.4, 0, 0)),
        ("leg_r", 40, (-0.4, 0, 0)),
        ("arm_l", 40, (-0.3, 0, 0)),
        ("arm_r", 40, (0.15, 0, 0)),
        ("club",  40, (0.1, 0, 0)),
        ("spine", 40, (0, 0, 0.04)),
        ("head",  40, (0, 0, -0.03)),
    ]

    return create_action(arm, "A_Tribesman_Walk", 40, keys)


def make_death(arm):
    keys = []
    # Frame 1: neutral
    for b in ["spine", "head", "arm_l", "arm_r", "club", "leg_l", "leg_r"]:
        keys.append((b, 1, (0, 0, 0)))

    # Frame 6: drops club, starts falling back
    keys += [
        ("club",  6, (0.8, 0, 0.3)),
        ("arm_r", 6, (0.3, 0, 0.2)),
        ("arm_l", 6, (-0.1, 0, -0.2)),
        ("spine", 6, (0.15, 0, 0)),
        ("head",  6, (0.2, 0, 0)),
    ]

    # Frame 14: falling backward
    keys += [
        ("spine", 14, (0.6, 0, 0)),
        ("head",  14, (0.4, 0, 0)),
        ("arm_l", 14, (-0.5, 0, -0.4)),
        ("arm_r", 14, (-0.4, 0, 0.5)),
        ("club",  14, (1.5, 0, 0.5)),
        ("leg_l", 14, (-0.3, 0, 0)),
        ("leg_r", 14, (-0.2, 0, 0)),
    ]

    # Frame 22: on the ground
    keys += [
        ("spine", 22, (math.pi * 0.45, 0, 0)),
        ("head",  22, (0.5, 0, 0)),
        ("arm_l", 22, (-0.8, 0, -0.6)),
        ("arm_r", 22, (-0.6, 0, 0.7)),
        ("club",  22, (2.0, 0, 0.3)),
        ("leg_l", 22, (-0.5, 0, 0.1)),
        ("leg_r", 22, (-0.4, 0, -0.1)),
    ]

    # Frame 30: hold
    keys += [
        ("spine", 30, (math.pi * 0.45, 0, 0)),
        ("head",  30, (0.5, 0, 0)),
        ("arm_l", 30, (-0.8, 0, -0.6)),
        ("arm_r", 30, (-0.6, 0, 0.7)),
        ("club",  30, (2.0, 0, 0.3)),
        ("leg_l", 30, (-0.5, 0, 0.1)),
        ("leg_r", 30, (-0.4, 0, -0.1)),
    ]

    return create_action(arm, "A_Tribesman_Death", 30, keys)


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

    print("  Building tribesman mesh...")
    mesh_obj = build_tribesman_mesh()

    print("  Building armature...")
    arm_obj = build_armature()

    print("  Skinning...")
    skin_mesh(mesh_obj, arm_obj)

    print("  Creating animations...")
    actions = [
        make_idle(arm_obj),
        make_walk(arm_obj),
        make_death(arm_obj),
    ]

    # Rotate from Blender -Y forward to +X forward for UE
    arm_obj.rotation_euler[2] = math.pi / 2

    sk_path = os.path.join(OUTPUT_DIR, "SK_Tribesman.fbx")
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
