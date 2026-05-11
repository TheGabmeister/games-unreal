"""
Generate a rigged Bat enemy skeletal mesh for UE5.

Creates SK_Bat.fbx — a small sphere body with two flat triangular wings,
rigged with a simple armature (root, body, wing_l, wing_r).

Usage:
    blender --background --python generate_bat.py -- [output_dir]

Output:
    SK_Bat.fbx — skeletal mesh (mesh + skeleton, bind pose)
    A_Bat_Fly.fbx — wing flap cycle (looping)
    A_Bat_Swoop.fbx — wings tuck, dive pose
    A_Bat_Perch.fbx — wings folded, hanging idle
    A_Bat_Death.fbx — wings crumple, tumbles
"""

import bpy
import bmesh
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


def make_wing(origin, side):
    """Create a flat triangular wing mesh.

    side: -1 for left, +1 for right.
    Wing extends outward from the body, flat in XZ plane.
    """
    mesh = bpy.data.meshes.new(f"Wing_{'L' if side < 0 else 'R'}")
    bm = bmesh.new()

    # Wing triangle: root at body, tip far out, trailing edge behind
    sx = side
    v0 = bm.verts.new((sx * 0.08, 0, 0.12))       # wing root (near body)
    v1 = bm.verts.new((sx * 0.38, -0.04, 0.12))    # wing tip (far out)
    v2 = bm.verts.new((sx * 0.22, 0.14, 0.08))     # trailing edge (behind)

    # Add a slight thickness by duplicating verts offset in Z
    v3 = bm.verts.new((sx * 0.08, 0, 0.13))
    v4 = bm.verts.new((sx * 0.38, -0.04, 0.13))
    v5 = bm.verts.new((sx * 0.22, 0.14, 0.09))

    # Top face
    if side < 0:
        bm.faces.new([v0, v1, v2])
        bm.faces.new([v5, v4, v3])
    else:
        bm.faces.new([v2, v1, v0])
        bm.faces.new([v3, v4, v5])

    # Side faces to close the shape
    bm.faces.new([v0, v1, v4, v3])
    bm.faces.new([v1, v2, v5, v4])
    bm.faces.new([v2, v0, v3, v5])

    bm.to_mesh(mesh)
    bm.free()

    obj = bpy.data.objects.new(f"Wing_{'L' if side < 0 else 'R'}", mesh)
    bpy.context.collection.objects.link(obj)
    return obj


# ---------------------------------------------------------------------------
# Bat Mesh — small sphere body, two triangular wings, ears, feet
# Faces -Y in Blender (UE +X forward after export)
# ---------------------------------------------------------------------------

def build_bat_mesh():
    single_mat = bpy.data.materials.new("M_Bat_Placeholder")
    single_mat.use_nodes = True
    bsdf = single_mat.node_tree.nodes["Principled BSDF"]
    bsdf.inputs["Base Color"].default_value = (0.15, 0.1, 0.2, 1.0)

    P = []

    # Body — small sphere
    body = sphere((0, 0, 0.12), 0.08)
    P.append(body)

    # Head — slightly larger sphere, forward and up
    head = sphere((0, -0.06, 0.18), 0.06)
    P.append(head)

    # Ears — two small cones pointing up
    for side in (-1, 1):
        ear = cone((side * 0.035, -0.06, 0.24), 0.015, 0.003, 0.05)
        P.append(ear)

    # Eyes — tiny spheres
    for side in (-1, 1):
        eye = sphere((side * 0.03, -0.10, 0.19), 0.012)
        P.append(eye)

    # Small feet — hanging below body
    for side in (-1, 1):
        foot = cone((side * 0.03, 0, 0.02), 0.015, 0.005, 0.04)
        P.append(foot)

    # Wings — flat triangular meshes
    wing_l = make_wing((0, 0, 0), -1)
    P.append(wing_l)
    wing_r = make_wing((0, 0, 0), 1)
    P.append(wing_r)

    result = join(P)
    result.name = "Bat_Mesh"
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
# Armature — 4-bone rig
# ---------------------------------------------------------------------------

# (bone_name, head, tail, parent_name)
BONES = [
    ("root",    (0, 0, 0),          (0, 0, 0.12),       None),
    ("body",    (0, 0, 0.12),       (0, -0.04, 0.18),   "root"),
    ("wing_l",  (-0.08, 0, 0.12),   (-0.30, -0.02, 0.12), "body"),
    ("wing_r",  (0.08, 0, 0.12),    (0.30, -0.02, 0.12),  "body"),
]

BONE_RADIUS = {
    "root": 0.20,
    "body": 0.18,
    "wing_l": 0.22,
    "wing_r": 0.22,
}


def build_armature():
    bpy.ops.object.armature_add(location=(0, 0, 0))
    arm_obj = bpy.context.active_object
    arm_obj.name = "Bat_Armature"
    arm = arm_obj.data
    arm.name = "Bat_Skeleton"

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


def make_fly(arm):
    """Wing flap cycle — wings go up and down (looping)."""
    keys = []

    # Wing flap — large rotation for visible movement
    flap_up = 0.7      # wings up angle
    flap_down = -0.6    # wings down angle

    for cycle in range(3):
        o = cycle * 10
        keys += [
            ("wing_l", 1 + o,  (flap_up, 0, 0)),    # wings up
            ("wing_l", 5 + o,  (flap_down, 0, 0)),   # wings down
            ("wing_r", 1 + o,  (-flap_up, 0, 0)),    # wings up (mirrored)
            ("wing_r", 5 + o,  (-flap_down, 0, 0)),  # wings down (mirrored)
        ]

    # Loop endpoint matches frame 1
    keys += [
        ("wing_l", 30, (flap_up, 0, 0)),
        ("wing_r", 30, (-flap_up, 0, 0)),
    ]

    # Body bobs slightly with each flap
    for cycle in range(3):
        o = cycle * 10
        keys += [
            ("body", 1 + o,  (0.05, 0, 0)),
            ("body", 5 + o,  (-0.05, 0, 0)),
        ]
    keys += [("body", 30, (0.05, 0, 0))]

    return create_action(arm, "A_Bat_Fly", 30, keys)


def make_swoop(arm):
    """Wings tuck, dive pose."""
    keys = []

    # Frame 1: wings out (flying position)
    keys += [
        ("body",   1, (0, 0, 0)),
        ("wing_l", 1, (0.3, 0, 0)),
        ("wing_r", 1, (-0.3, 0, 0)),
    ]

    # Frame 6: wings begin tucking, body pitches down
    keys += [
        ("body",   6, (0.5, 0, 0)),
        ("wing_l", 6, (0.7, 0, 0.4)),     # wings sweep back
        ("wing_r", 6, (-0.7, 0, -0.4)),
    ]

    # Frame 12: full dive — wings tight against body, steep pitch
    keys += [
        ("body",   12, (0.8, 0, 0)),
        ("wing_l", 12, (0.9, 0, 0.6)),
        ("wing_r", 12, (-0.9, 0, -0.6)),
    ]

    # Frame 20: hold dive pose
    keys += [
        ("body",   20, (0.8, 0, 0)),
        ("wing_l", 20, (0.9, 0, 0.6)),
        ("wing_r", 20, (-0.9, 0, -0.6)),
    ]

    return create_action(arm, "A_Bat_Swoop", 20, keys)


def make_perch(arm):
    """Wings folded, hanging idle (upside-down perch)."""
    keys = []

    # Frame 1: wings folded down along body
    keys += [
        ("body",   1, (0, 0, 0)),
        ("wing_l", 1, (0.8, 0, 0.5)),     # wings folded close
        ("wing_r", 1, (-0.8, 0, -0.5)),
    ]

    # Subtle breathing/sway
    keys += [
        ("body",   10, (0.04, 0, 0.03)),
        ("body",   20, (-0.04, 0, -0.03)),
        ("body",   30, (0.04, 0, 0.03)),
        ("body",   40, (0, 0, 0)),
    ]

    # Wings stay mostly folded with tiny movement
    keys += [
        ("wing_l", 20, (0.85, 0, 0.5)),
        ("wing_l", 40, (0.8, 0, 0.5)),
        ("wing_r", 20, (-0.85, 0, -0.5)),
        ("wing_r", 40, (-0.8, 0, -0.5)),
    ]

    return create_action(arm, "A_Bat_Perch", 40, keys)


def make_death(arm):
    """Wings crumple, tumbles."""
    keys = []

    # Frame 1: neutral flying pose
    keys += [
        ("body",   1, (0, 0, 0)),
        ("wing_l", 1, (0, 0, 0)),
        ("wing_r", 1, (0, 0, 0)),
    ]

    # Frame 6: hit — wings jerk up
    keys += [
        ("body",   6, (-0.3, 0, 0.2)),
        ("wing_l", 6, (-0.5, 0, 0)),
        ("wing_r", 6, (0.5, 0, 0)),
    ]

    # Frame 14: tumbling — body rotating, wings crumpling
    keys += [
        ("body",   14, (0.6, 0.4, 0.8)),
        ("wing_l", 14, (0.9, 0.3, 0.7)),
        ("wing_r", 14, (-0.9, 0.3, -0.7)),
    ]

    # Frame 22: near ground — wings fully crumpled
    keys += [
        ("body",   22, (math.pi * 0.6, 0.5, 1.2)),
        ("wing_l", 22, (1.1, 0.4, 0.9)),
        ("wing_r", 22, (-1.1, 0.4, -0.9)),
    ]

    # Frame 30: hold final crumpled pose
    keys += [
        ("body",   30, (math.pi * 0.6, 0.5, 1.2)),
        ("wing_l", 30, (1.1, 0.4, 0.9)),
        ("wing_r", 30, (-1.1, 0.4, -0.9)),
    ]

    return create_action(arm, "A_Bat_Death", 30, keys)


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

    print("  Building bat mesh...")
    mesh_obj = build_bat_mesh()

    print("  Building armature...")
    arm_obj = build_armature()

    print("  Skinning...")
    skin_mesh(mesh_obj, arm_obj)

    print("  Creating animations...")
    actions = [
        make_fly(arm_obj),
        make_swoop(arm_obj),
        make_perch(arm_obj),
        make_death(arm_obj),
    ]

    # Rotate from Blender -Y forward to +X forward for UE
    arm_obj.rotation_euler[2] = math.pi / 2

    sk_path = os.path.join(OUTPUT_DIR, "SK_Bat.fbx")
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
