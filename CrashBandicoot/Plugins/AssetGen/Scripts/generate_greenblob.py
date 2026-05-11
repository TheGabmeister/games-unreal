"""
Generate a rigged Green Blob enemy skeletal mesh for UE5.

Creates SK_GreenBlob.fbx — a small sphere with a simple 2-bone rig
(root, body) for squash/stretch animations.

Usage:
    blender --background --python generate_greenblob.py -- [output_dir]

Output:
    SK_GreenBlob.fbx — skeletal mesh (mesh + skeleton, bind pose)
    A_GreenBlob_Idle.fbx — squash/stretch pulse
    A_GreenBlob_Bounce.fbx — compress then spring upward
    A_GreenBlob_Death.fbx — splats flat
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


# ---------------------------------------------------------------------------
# Green Blob Mesh — small sphere, very simple
# Faces -Y in Blender. Radius about 0.08 Blender units.
# ---------------------------------------------------------------------------

def build_greenblob_mesh():
    single_mat = bpy.data.materials.new("M_GreenBlob_Placeholder")
    single_mat.use_nodes = True
    bsdf = single_mat.node_tree.nodes["Principled BSDF"]
    bsdf.inputs["Base Color"].default_value = (0.2, 0.7, 0.1, 1.0)

    P = []

    # Main body — small sphere sitting on the ground
    body = sphere((0, 0, 0.08), 0.08, segs=20, rings=12)
    P.append(body)

    # Small bumps on top for texture (3 small spheres)
    for i in range(3):
        angle = i * math.pi * 2 / 3
        x = math.cos(angle) * 0.03
        y = math.sin(angle) * 0.03
        bump = sphere((x, y, 0.14), 0.025)
        P.append(bump)

    # Flattened base to sit on ground
    base = sphere((0, 0, 0.02), 0.06, segs=16, rings=6)
    base.scale = (1.2, 1.2, 0.4)
    _sel(base)
    bpy.ops.object.transform_apply(scale=True)
    P.append(base)

    result = join(P)
    result.name = "GreenBlob_Mesh"
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
# Armature — simple 2-bone rig
# ---------------------------------------------------------------------------

BONES = [
    ("root", (0, 0, 0),    (0, 0, 0.08),  None),
    ("body", (0, 0, 0.08), (0, 0, 0.16),  "root"),
]

BONE_RADIUS = {
    "root": 0.15,
    "body": 0.15,
}


def build_armature():
    bpy.ops.object.armature_add(location=(0, 0, 0))
    arm_obj = bpy.context.active_object
    arm_obj.name = "GreenBlob_Armature"
    arm = arm_obj.data
    arm.name = "GreenBlob_Skeleton"

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
    """Squash/stretch pulse — body rotates on X axis to simulate breathing."""
    keys = []
    # Two full pulse cycles over 40 frames
    for cycle in range(2):
        o = cycle * 20
        # Squash down (rotate forward to compress top)
        keys += [
            ("body", 1 + o,  (0.3, 0, 0)),
            ("root", 1 + o,  (0, 0, 0.05)),
        ]
        # Stretch up (rotate back to extend)
        keys += [
            ("body", 5 + o,  (-0.35, 0, 0)),
            ("root", 5 + o,  (0, 0, -0.05)),
        ]
        # Back to squash
        keys += [
            ("body", 10 + o, (0.3, 0, 0)),
            ("root", 10 + o, (0, 0, 0.05)),
        ]
        # Stretch again
        keys += [
            ("body", 15 + o, (-0.35, 0, 0)),
            ("root", 15 + o, (0, 0, -0.05)),
        ]

    # Loop endpoint matches frame 1
    keys += [
        ("body", 40, (0.3, 0, 0)),
        ("root", 40, (0, 0, 0.05)),
    ]

    return create_action(arm, "A_GreenBlob_Idle", 40, keys)


def make_bounce(arm):
    """Compress then spring upward."""
    keys = []
    # Frame 1: neutral
    keys += [
        ("body", 1, (0, 0, 0)),
        ("root", 1, (0, 0, 0)),
    ]

    # Frame 5: compress down — squash hard
    keys += [
        ("body", 5, (0.6, 0, 0)),
        ("root", 5, (0.1, 0, 0)),
    ]

    # Frame 8: max compression
    keys += [
        ("body", 8, (0.8, 0, 0)),
        ("root", 8, (0.15, 0, 0)),
    ]

    # Frame 12: spring upward — stretch tall
    keys += [
        ("body", 12, (-0.7, 0, 0)),
        ("root", 12, (-0.3, 0, 0)),
    ]

    # Frame 16: peak of bounce — extended
    keys += [
        ("body", 16, (-0.5, 0, 0)),
        ("root", 16, (-0.2, 0, 0)),
    ]

    # Frame 20: settle back to neutral
    keys += [
        ("body", 20, (0, 0, 0)),
        ("root", 20, (0, 0, 0)),
    ]

    return create_action(arm, "A_GreenBlob_Bounce", 20, keys)


def make_death(arm):
    """Splats flat — body rotates to flatten."""
    keys = []
    # Frame 1: neutral
    keys += [
        ("body", 1, (0, 0, 0)),
        ("root", 1, (0, 0, 0)),
    ]

    # Frame 5: starts to flatten — wobble
    keys += [
        ("body", 5, (0.4, 0.3, 0)),
        ("root", 5, (0.1, 0, 0)),
    ]

    # Frame 10: splats flat — extreme rotation simulates flattening
    keys += [
        ("body", 10, (math.pi * 0.4, 0.5, 0)),
        ("root", 10, (0.3, 0, 0)),
    ]

    # Frame 15: fully flat, slight settling
    keys += [
        ("body", 15, (math.pi * 0.45, 0.5, 0.1)),
        ("root", 15, (0.35, 0, 0)),
    ]

    # Frame 20: hold
    keys += [
        ("body", 20, (math.pi * 0.45, 0.5, 0.1)),
        ("root", 20, (0.35, 0, 0)),
    ]

    return create_action(arm, "A_GreenBlob_Death", 20, keys)


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

    print("  Building green blob mesh...")
    mesh_obj = build_greenblob_mesh()

    print("  Building armature...")
    arm_obj = build_armature()

    print("  Skinning...")
    skin_mesh(mesh_obj, arm_obj)

    print("  Creating animations...")
    actions = [
        make_idle(arm_obj),
        make_bounce(arm_obj),
        make_death(arm_obj),
    ]

    # Rotate from Blender -Y forward to +X forward for UE
    arm_obj.rotation_euler[2] = math.pi / 2

    sk_path = os.path.join(OUTPUT_DIR, "SK_GreenBlob.fbx")
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
