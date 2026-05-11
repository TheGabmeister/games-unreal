"""
Generate a rigged Crab enemy skeletal mesh for UE5.

Creates SK_Crab.fbx — a flat ellipsoid body with two claw cones,
rigged with a simple armature (root, body, claw_l, claw_r).

Usage:
    blender --background --python generate_crab.py -- [output_dir]

Output:
    SK_Crab.fbx — skeletal mesh (mesh + skeleton, bind pose)
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
# Crab Mesh — flat ellipsoid body, two claws, small eye stalks
# Faces -Y in Blender (UE +X forward after export)
# ---------------------------------------------------------------------------

def build_crab_mesh():
    single_mat = bpy.data.materials.new("M_Crab_Placeholder")
    single_mat.use_nodes = True
    bsdf = single_mat.node_tree.nodes["Principled BSDF"]
    bsdf.inputs["Base Color"].default_value = (0.9, 0.4, 0.1, 1.0)

    P = []

    # Body — flattened sphere (wide and flat)
    body = sphere((0, 0, 0.12), 0.20)
    body.scale = (1.4, 1.0, 0.5)
    _sel(body)
    bpy.ops.object.transform_apply(scale=True)
    P.append(body)

    # Left claw — cone pointing forward-left
    claw_l = cone((-0.28, -0.12, 0.10), 0.06, 0.01, 0.18,
                  rot=(math.pi/2, 0, math.radians(30)))
    P.append(claw_l)

    # Right claw — cone pointing forward-right
    claw_r = cone((0.28, -0.12, 0.10), 0.06, 0.01, 0.18,
                  rot=(math.pi/2, 0, math.radians(-30)))
    P.append(claw_r)

    # Small legs (4 per side, just bumps for placeholder)
    for side in (-1, 1):
        for i in range(4):
            z_off = 0.08
            y_off = -0.05 + i * 0.06
            x_off = side * (0.22 + i * 0.02)
            leg = cube((x_off, y_off, z_off), (0.03, 0.06, 0.02))
            P.append(leg)

    # Eye stalks
    for side in (-1, 1):
        stalk = cube((side * 0.08, -0.15, 0.22), (0.02, 0.02, 0.06))
        P.append(stalk)
        eye = sphere((side * 0.08, -0.15, 0.26), 0.025)
        P.append(eye)

    result = join(P)
    result.name = "Crab_Mesh"
    result.data.materials.clear()
    result.data.materials.append(single_mat)

    # Smooth shade
    _sel(result)
    bpy.ops.object.shade_smooth()

    # UV unwrap
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.uv.smart_project(angle_limit=1.15)
    bpy.ops.object.mode_set(mode='OBJECT')

    return result


# ---------------------------------------------------------------------------
# Armature — simple 4-bone rig
# ---------------------------------------------------------------------------

# (bone_name, head, tail, parent_name)
BONES = [
    ("root",    (0, 0, 0),       (0, 0, 0.12),    None),
    ("body",    (0, 0, 0.12),    (0, 0, 0.20),    "root"),
    ("claw_l",  (-0.20, -0.05, 0.10), (-0.28, -0.12, 0.10), "body"),
    ("claw_r",  (0.20, -0.05, 0.10),  (0.28, -0.12, 0.10),  "body"),
]

BONE_RADIUS = {
    "root": 0.30,
    "body": 0.30,
    "claw_l": 0.15,
    "claw_r": 0.15,
}


def build_armature():
    bpy.ops.object.armature_add(location=(0, 0, 0))
    arm_obj = bpy.context.active_object
    arm_obj.name = "Crab_Armature"
    arm = arm_obj.data
    arm.name = "Crab_Skeleton"

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
    keys = []
    # Claw pinch cycle — claws open/close subtly
    for cycle in range(2):
        o = cycle * 30
        keys += [
            ("claw_l", 1 + o,  (0, 0, 0.08)),
            ("claw_l", 15 + o, (0, 0, -0.08)),
            ("claw_r", 1 + o,  (0, 0, -0.08)),
            ("claw_r", 15 + o, (0, 0, 0.08)),
        ]
    # Body slight sway
    keys += [
        ("body", 1,  (0, 0.02, 0)),
        ("body", 15, (0, -0.02, 0)),
        ("body", 30, (0, 0.02, 0)),
        ("body", 45, (0, -0.02, 0)),
        ("body", 60, (0, 0.02, 0)),
    ]
    # Loop endpoint
    keys += [
        ("claw_l", 60, (0, 0, 0.08)),
        ("claw_r", 60, (0, 0, -0.08)),
    ]
    return create_action(arm, "A_Crab_Idle", 60, keys)


def make_walk(arm):
    keys = []
    # Side-to-side shuffle — body rocks left/right, claws sway opposite
    for step in range(4):
        o = step * 10
        side = 1 if step % 2 == 0 else -1
        keys += [
            ("body",   1 + o, (0, 0, side * 0.06)),
            ("body",   5 + o, (0, 0, -side * 0.06)),
            ("claw_l", 1 + o, (side * 0.05, 0, 0)),
            ("claw_l", 5 + o, (-side * 0.05, 0, 0)),
            ("claw_r", 1 + o, (-side * 0.05, 0, 0)),
            ("claw_r", 5 + o, (side * 0.05, 0, 0)),
        ]
    # Loop endpoint
    keys += [
        ("body",   40, (0, 0, 0.06)),
        ("claw_l", 40, (0.05, 0, 0)),
        ("claw_r", 40, (-0.05, 0, 0)),
    ]
    return create_action(arm, "A_Crab_Walk", 40, keys)


def make_death(arm):
    keys = []
    # Frame 1: neutral
    for b in ["body", "claw_l", "claw_r"]:
        keys.append((b, 1, (0, 0, 0)))

    # Frame 8: tips backward
    keys += [
        ("body",   8, (0.3, 0, 0)),
        ("claw_l", 8, (0.1, 0, 0.15)),
        ("claw_r", 8, (0.1, 0, -0.15)),
    ]

    # Frame 14: flips onto back
    keys += [
        ("body",   14, (math.pi * 0.9, 0, 0)),
        ("claw_l", 14, (0.3, 0, 0.4)),
        ("claw_r", 14, (0.3, 0, -0.4)),
    ]

    # Frame 20: legs curl, settle
    keys += [
        ("body",   20, (math.pi * 0.95, 0, 0)),
        ("claw_l", 20, (0.5, 0, 0.6)),
        ("claw_r", 20, (0.5, 0, -0.6)),
    ]

    # Frame 30: hold
    keys += [
        ("body",   30, (math.pi * 0.95, 0, 0)),
        ("claw_l", 30, (0.5, 0, 0.6)),
        ("claw_r", 30, (0.5, 0, -0.6)),
    ]

    return create_action(arm, "A_Crab_Death", 30, keys)


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

    print("  Building crab mesh...")
    mesh_obj = build_crab_mesh()

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

    sk_path = os.path.join(OUTPUT_DIR, "SK_Crab.fbx")
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
