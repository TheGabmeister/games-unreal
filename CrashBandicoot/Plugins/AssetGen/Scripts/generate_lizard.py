"""
Generate a rigged Lizard enemy skeletal mesh for UE5.

Creates SK_LizardGreen.fbx and SK_LizardRed.fbx — low wide capsule body with
four leg stubs and a tail, rigged with armature (root, body, leg_fl, leg_fr,
leg_bl, leg_br, tail).

Usage:
    blender --background --python generate_lizard.py -- [output_dir]

Output:
    SK_LizardGreen.fbx — skeletal mesh (mesh + skeleton, bind pose)
    SK_LizardRed.fbx   — same geometry, different file name
    A_Lizard_Idle.fbx  — low crouch, slight tail sway
    A_Lizard_Jump.fbx  — legs push off, body arcs upward
    A_Lizard_Land.fbx  — legs absorb impact, body compresses
    A_Lizard_Death.fbx — rolls onto side
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
# Lizard Mesh — low wide capsule body, four leg stubs, tail
# Faces -Y in Blender (UE +X forward after export)
# ---------------------------------------------------------------------------

def build_lizard_mesh():
    single_mat = bpy.data.materials.new("M_Lizard_Placeholder")
    single_mat.use_nodes = True
    bsdf = single_mat.node_tree.nodes["Principled BSDF"]
    bsdf.inputs["Base Color"].default_value = (0.2, 0.55, 0.15, 1.0)

    P = []

    # Body — wide flat ellipsoid (low to the ground)
    body = sphere((0, 0, 0.10), 0.14)
    body.scale = (1.0, 1.6, 0.5)
    _sel(body)
    bpy.ops.object.transform_apply(scale=True)
    P.append(body)

    # Hip joint — slightly wider mid-section for crouch deformation
    hip = sphere((0, 0.04, 0.10), 0.10)
    hip.scale = (0.9, 0.8, 0.45)
    _sel(hip)
    bpy.ops.object.transform_apply(scale=True)
    P.append(hip)

    # Head — small sphere at front
    head = sphere((0, -0.22, 0.12), 0.07)
    head.scale = (0.9, 1.1, 0.7)
    _sel(head)
    bpy.ops.object.transform_apply(scale=True)
    P.append(head)

    # Eyes
    for side in (-1, 1):
        eye = sphere((side * 0.04, -0.26, 0.15), 0.02)
        P.append(eye)

    # Front-left leg
    leg_fl = cylinder((-0.12, -0.10, 0.04), 0.03, 0.10,
                      rot=(0, math.radians(20), 0))
    P.append(leg_fl)
    foot_fl = sphere((-0.14, -0.10, -0.01), 0.025)
    P.append(foot_fl)

    # Front-right leg
    leg_fr = cylinder((0.12, -0.10, 0.04), 0.03, 0.10,
                      rot=(0, math.radians(-20), 0))
    P.append(leg_fr)
    foot_fr = sphere((0.14, -0.10, -0.01), 0.025)
    P.append(foot_fr)

    # Back-left leg
    leg_bl = cylinder((-0.12, 0.12, 0.04), 0.03, 0.10,
                      rot=(0, math.radians(20), 0))
    P.append(leg_bl)
    foot_bl = sphere((-0.14, 0.12, -0.01), 0.025)
    P.append(foot_bl)

    # Back-right leg
    leg_br = cylinder((0.12, 0.12, 0.04), 0.03, 0.10,
                      rot=(0, math.radians(-20), 0))
    P.append(leg_br)
    foot_br = sphere((0.14, 0.12, -0.01), 0.025)
    P.append(foot_br)

    # Tail — tapered cylinder extending backward
    tail_base = cylinder((0, 0.24, 0.09), 0.04, 0.14)
    P.append(tail_base)
    tail_mid = cylinder((0, 0.36, 0.08), 0.025, 0.12)
    P.append(tail_mid)
    tail_tip = cone((0, 0.46, 0.07), 0.02, 0.005, 0.10, segs=8)
    P.append(tail_tip)

    result = join(P)
    result.name = "Lizard_Mesh"
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

BONES = [
    ("root",    (0, 0, 0),        (0, 0, 0.10),     None),
    ("body",    (0, 0, 0.10),     (0, -0.15, 0.12),  "root"),
    ("leg_fl",  (-0.10, -0.10, 0.08), (-0.14, -0.10, -0.01), "body"),
    ("leg_fr",  (0.10, -0.10, 0.08),  (0.14, -0.10, -0.01),  "body"),
    ("leg_bl",  (-0.10, 0.12, 0.08),  (-0.14, 0.12, -0.01),  "root"),
    ("leg_br",  (0.10, 0.12, 0.08),   (0.14, 0.12, -0.01),   "root"),
    ("tail",    (0, 0.18, 0.09),  (0, 0.46, 0.07),  "root"),
]

BONE_RADIUS = {
    "root": 0.25,
    "body": 0.22,
    "leg_fl": 0.10,
    "leg_fr": 0.10,
    "leg_bl": 0.10,
    "leg_br": 0.10,
    "tail": 0.08,
}


def build_armature():
    bpy.ops.object.armature_add(location=(0, 0, 0))
    arm_obj = bpy.context.active_object
    arm_obj.name = "Lizard_Armature"
    arm = arm_obj.data
    arm.name = "Lizard_Skeleton"

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
    """Low crouch, slight tail sway — 60 frames."""
    keys = []

    # Body slight breathing/crouch
    keys += [
        ("body", 1,  (0.05, 0, 0)),
        ("body", 15, (-0.03, 0, 0)),
        ("body", 30, (0.05, 0, 0)),
        ("body", 45, (-0.03, 0, 0)),
        ("body", 60, (0.05, 0, 0)),
    ]

    # Tail sway side to side
    keys += [
        ("tail", 1,  (0, 0, 0.15)),
        ("tail", 15, (0, 0, -0.15)),
        ("tail", 30, (0, 0, 0.15)),
        ("tail", 45, (0, 0, -0.15)),
        ("tail", 60, (0, 0, 0.15)),
    ]

    # Legs shift weight subtly
    for cycle in range(2):
        o = cycle * 30
        keys += [
            ("leg_fl", 1 + o,  (0.04, 0, 0)),
            ("leg_fl", 15 + o, (-0.04, 0, 0)),
            ("leg_fr", 1 + o,  (-0.04, 0, 0)),
            ("leg_fr", 15 + o, (0.04, 0, 0)),
            ("leg_bl", 1 + o,  (-0.03, 0, 0)),
            ("leg_bl", 15 + o, (0.03, 0, 0)),
            ("leg_br", 1 + o,  (0.03, 0, 0)),
            ("leg_br", 15 + o, (-0.03, 0, 0)),
        ]

    # Loop endpoints
    keys += [
        ("leg_fl", 60, (0.04, 0, 0)),
        ("leg_fr", 60, (-0.04, 0, 0)),
        ("leg_bl", 60, (-0.03, 0, 0)),
        ("leg_br", 60, (0.03, 0, 0)),
    ]

    return create_action(arm, "A_Lizard_Idle", 60, keys)


def make_jump(arm):
    """Legs push off, body arcs upward — 20 frames."""
    keys = []

    # Frame 1: crouched low (pre-jump)
    keys += [
        ("body",   1, (0.15, 0, 0)),
        ("leg_fl", 1, (0.3, 0, 0)),
        ("leg_fr", 1, (0.3, 0, 0)),
        ("leg_bl", 1, (0.3, 0, 0)),
        ("leg_br", 1, (0.3, 0, 0)),
        ("tail",   1, (-0.1, 0, 0)),
    ]

    # Frame 6: explosive push — legs extend, body tilts up
    keys += [
        ("body",   6, (-0.4, 0, 0)),
        ("leg_fl", 6, (-0.5, 0, 0)),
        ("leg_fr", 6, (-0.5, 0, 0)),
        ("leg_bl", 6, (-0.6, 0, 0)),
        ("leg_br", 6, (-0.6, 0, 0)),
        ("tail",   6, (0.3, 0, 0)),
    ]

    # Frame 12: airborne — legs tucked, body level
    keys += [
        ("body",   12, (-0.2, 0, 0)),
        ("leg_fl", 12, (0.4, 0, 0.2)),
        ("leg_fr", 12, (0.4, 0, -0.2)),
        ("leg_bl", 12, (0.3, 0, 0.15)),
        ("leg_br", 12, (0.3, 0, -0.15)),
        ("tail",   12, (0.15, 0, 0.1)),
    ]

    # Frame 20: peak — body slightly nose-up, legs spread
    keys += [
        ("body",   20, (-0.15, 0, 0)),
        ("leg_fl", 20, (0.35, 0, 0.25)),
        ("leg_fr", 20, (0.35, 0, -0.25)),
        ("leg_bl", 20, (0.25, 0, 0.2)),
        ("leg_br", 20, (0.25, 0, -0.2)),
        ("tail",   20, (0.2, 0, 0.15)),
    ]

    return create_action(arm, "A_Lizard_Jump", 20, keys)


def make_land(arm):
    """Legs absorb impact, body compresses — 15 frames."""
    keys = []

    # Frame 1: descending, legs reaching for ground
    keys += [
        ("body",   1, (-0.1, 0, 0)),
        ("leg_fl", 1, (-0.3, 0, 0)),
        ("leg_fr", 1, (-0.3, 0, 0)),
        ("leg_bl", 1, (-0.3, 0, 0)),
        ("leg_br", 1, (-0.3, 0, 0)),
        ("tail",   1, (0.2, 0, 0)),
    ]

    # Frame 5: impact — body squashes down, legs compress
    keys += [
        ("body",   5, (0.35, 0, 0)),
        ("leg_fl", 5, (0.5, 0, 0.1)),
        ("leg_fr", 5, (0.5, 0, -0.1)),
        ("leg_bl", 5, (0.5, 0, 0.1)),
        ("leg_br", 5, (0.5, 0, -0.1)),
        ("tail",   5, (-0.15, 0, 0.2)),
    ]

    # Frame 10: recovery — body rising back
    keys += [
        ("body",   10, (0.1, 0, 0)),
        ("leg_fl", 10, (0.15, 0, 0)),
        ("leg_fr", 10, (0.15, 0, 0)),
        ("leg_bl", 10, (0.15, 0, 0)),
        ("leg_br", 10, (0.15, 0, 0)),
        ("tail",   10, (0.05, 0, 0.05)),
    ]

    # Frame 15: settled
    keys += [
        ("body",   15, (0, 0, 0)),
        ("leg_fl", 15, (0, 0, 0)),
        ("leg_fr", 15, (0, 0, 0)),
        ("leg_bl", 15, (0, 0, 0)),
        ("leg_br", 15, (0, 0, 0)),
        ("tail",   15, (0, 0, 0)),
    ]

    return create_action(arm, "A_Lizard_Land", 15, keys)


def make_death(arm):
    """Rolls onto side — 30 frames."""
    keys = []

    # Frame 1: neutral
    for b in ["body", "leg_fl", "leg_fr", "leg_bl", "leg_br", "tail"]:
        keys.append((b, 1, (0, 0, 0)))

    # Frame 6: starts tipping to the side
    keys += [
        ("body",   6, (0.2, 0.3, 0)),
        ("leg_fl", 6, (0.1, 0, 0.3)),
        ("leg_fr", 6, (-0.1, 0, -0.2)),
        ("leg_bl", 6, (0.1, 0, 0.2)),
        ("leg_br", 6, (-0.1, 0, -0.15)),
        ("tail",   6, (0, 0, 0.3)),
    ]

    # Frame 14: rolling onto side
    keys += [
        ("body",   14, (0.4, 0.8, 0)),
        ("leg_fl", 14, (0.3, 0, 0.6)),
        ("leg_fr", 14, (-0.3, 0, -0.5)),
        ("leg_bl", 14, (0.2, 0, 0.5)),
        ("leg_br", 14, (-0.2, 0, -0.4)),
        ("tail",   14, (0.2, 0, 0.5)),
    ]

    # Frame 22: on side, legs curling
    keys += [
        ("body",   22, (0.5, math.pi * 0.45, 0)),
        ("leg_fl", 22, (0.5, 0, 0.8)),
        ("leg_fr", 22, (-0.5, 0, -0.7)),
        ("leg_bl", 22, (0.4, 0, 0.7)),
        ("leg_br", 22, (-0.4, 0, -0.6)),
        ("tail",   22, (0.3, 0, 0.6)),
    ]

    # Frame 30: hold
    keys += [
        ("body",   30, (0.5, math.pi * 0.45, 0)),
        ("leg_fl", 30, (0.5, 0, 0.8)),
        ("leg_fr", 30, (-0.5, 0, -0.7)),
        ("leg_bl", 30, (0.4, 0, 0.7)),
        ("leg_br", 30, (-0.4, 0, -0.6)),
        ("tail",   30, (0.3, 0, 0.6)),
    ]

    return create_action(arm, "A_Lizard_Death", 30, keys)


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

    print("  Building lizard mesh...")
    mesh_obj = build_lizard_mesh()

    print("  Building armature...")
    arm_obj = build_armature()

    print("  Skinning...")
    skin_mesh(mesh_obj, arm_obj)

    print("  Creating animations...")
    actions = [
        make_idle(arm_obj),
        make_jump(arm_obj),
        make_land(arm_obj),
        make_death(arm_obj),
    ]

    # Rotate from Blender -Y forward to +X forward for UE
    arm_obj.rotation_euler[2] = math.pi / 2

    # Export two skeletal meshes (same geometry, different file names)
    for sk_name in ("SK_LizardGreen", "SK_LizardRed"):
        sk_path = os.path.join(OUTPUT_DIR, f"{sk_name}.fbx")
        print(f"  Exporting {sk_name}...")
        export_skeletal_mesh(mesh_obj, arm_obj, sk_path)
        print(f"  Exported: {sk_path}")

    for action in actions:
        anim_path = os.path.join(OUTPUT_DIR, f"{action.name}.fbx")
        print(f"  Exporting {action.name}...")
        export_animation(mesh_obj, arm_obj, action, anim_path)
        print(f"  Exported: {anim_path}")

    print("Done.")


main()
