"""
Generate a rigged Snake enemy skeletal mesh for UE5.

Creates SK_Snake.fbx — cylinder tube with tapered head and 3 spine segments,
rigged with a simple armature (root, spine1, spine2, spine3, head).

Usage:
    blender --background --python generate_snake.py -- [output_dir]

Output:
    SK_Snake.fbx — skeletal mesh (mesh + skeleton, bind pose)
    A_Snake_Emerge.fbx — rises from ground, spine segments extend upward
    A_Snake_Bob.fbx — sway side-to-side (emerging idle, looping)
    A_Snake_Lunge.fbx — head snaps forward rapidly, body follows
    A_Snake_Hide.fbx — retracts downward into ground
    A_Snake_Death.fbx — coils and falls limp
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
# Snake Mesh — segmented cylinder tube with tapered head
# Faces -Y in Blender (UE +X forward after export)
# ---------------------------------------------------------------------------

def build_snake_mesh():
    single_mat = bpy.data.materials.new("M_Snake_Placeholder")
    single_mat.use_nodes = True
    bsdf = single_mat.node_tree.nodes["Principled BSDF"]
    bsdf.inputs["Base Color"].default_value = (0.15, 0.45, 0.1, 1.0)

    P = []

    # Spine segment 1 (base) — widest cylinder, at ground level
    seg1 = cylinder((0, 0, 0.06), 0.06, 0.12)
    P.append(seg1)

    # Spine segment 2 — slightly narrower, stacked above
    seg2 = cylinder((0, 0, 0.18), 0.055, 0.12)
    P.append(seg2)

    # Spine segment 3 — narrower still
    seg3 = cylinder((0, 0, 0.30), 0.05, 0.12)
    P.append(seg3)

    # Head — tapered sphere, slightly wider and elongated forward
    head_body = sphere((0, -0.03, 0.42), 0.06, segs=14, rings=8)
    head_body.scale = (0.9, 1.3, 0.8)
    P.append(head_body)

    # Snout — small cone at front of head
    snout = cone((0, -0.12, 0.42), 0.03, 0.005, 0.08,
                 rot=(math.pi / 2, 0, 0))
    P.append(snout)

    # Eyes — small spheres on sides of head
    for side in (-1, 1):
        eye = sphere((side * 0.04, -0.06, 0.45), 0.015)
        P.append(eye)

    # Tongue — thin cone extending from snout (forked look via two small cones)
    tongue = cone((0, -0.17, 0.41), 0.008, 0.001, 0.06,
                  rot=(math.pi / 2, 0, 0))
    P.append(tongue)

    # Small belly scales/ridges (decorative bumps along the body)
    for i in range(3):
        z = 0.08 + i * 0.12
        ridge = cylinder((0, 0.05, z), 0.04, 0.02, segs=8)
        ridge.scale = (1.2, 0.6, 1.0)
        P.append(ridge)

    result = join(P)
    result.name = "Snake_Mesh"
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
# Armature — 5-bone rig
# ---------------------------------------------------------------------------

# (bone_name, head, tail, parent_name)
BONES = [
    ("root",   (0, 0, 0),       (0, 0, 0.06),    None),
    ("spine1", (0, 0, 0.06),    (0, 0, 0.18),    "root"),
    ("spine2", (0, 0, 0.18),    (0, 0, 0.30),    "spine1"),
    ("spine3", (0, 0, 0.30),    (0, 0, 0.40),    "spine2"),
    ("head",   (0, 0, 0.40),    (0, -0.06, 0.44), "spine3"),
]

BONE_RADIUS = {
    "root": 0.15,
    "spine1": 0.15,
    "spine2": 0.14,
    "spine3": 0.13,
    "head": 0.12,
}


def build_armature():
    bpy.ops.object.armature_add(location=(0, 0, 0))
    arm_obj = bpy.context.active_object
    arm_obj.name = "Snake_Armature"
    arm = arm_obj.data
    arm.name = "Snake_Skeleton"

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


def make_emerge(arm):
    """A_Snake_Emerge (20 frames): Rises from ground, spine segments extend upward."""
    keys = []

    # Frame 1: fully retracted — all spine segments bent down
    keys += [
        ("spine1", 1, (0.8, 0, 0)),
        ("spine2", 1, (0.6, 0, 0)),
        ("spine3", 1, (0.5, 0, 0)),
        ("head",   1, (0.4, 0, 0)),
    ]

    # Frame 6: spine1 starts extending
    keys += [
        ("spine1", 6, (0.4, 0, 0)),
        ("spine2", 6, (0.5, 0, 0)),
        ("spine3", 6, (0.4, 0, 0)),
        ("head",   6, (0.3, 0, 0)),
    ]

    # Frame 12: most of the way up
    keys += [
        ("spine1", 12, (0.1, 0, 0)),
        ("spine2", 12, (0.15, 0, 0)),
        ("spine3", 12, (0.1, 0, 0)),
        ("head",   12, (0.05, 0, 0)),
    ]

    # Frame 16: slight overshoot
    keys += [
        ("spine1", 16, (-0.05, 0, 0)),
        ("spine2", 16, (-0.08, 0, 0)),
        ("spine3", 16, (-0.05, 0, 0)),
        ("head",   16, (-0.1, 0, 0)),
    ]

    # Frame 20: settled upright
    keys += [
        ("spine1", 20, (0, 0, 0)),
        ("spine2", 20, (0, 0, 0)),
        ("spine3", 20, (0, 0, 0)),
        ("head",   20, (0, 0, 0)),
    ]

    return create_action(arm, "A_Snake_Emerge", 20, keys)


def make_bob(arm):
    """A_Snake_Bob (60 frames): Sway side-to-side (emerging idle, looping)."""
    keys = []

    # Smooth sinusoidal sway — each spine segment offsets slightly
    for cycle in range(3):
        o = cycle * 20
        phase = 1 if cycle % 2 == 0 else -1

        keys += [
            ("spine1", 1 + o,  (0, 0, phase * 0.15)),
            ("spine2", 1 + o,  (0, 0, phase * 0.25)),
            ("spine3", 1 + o,  (0, 0, phase * 0.35)),
            ("head",   1 + o,  (0, 0, phase * 0.40)),
        ]

        keys += [
            ("spine1", 10 + o, (0, 0, -phase * 0.15)),
            ("spine2", 10 + o, (0, 0, -phase * 0.25)),
            ("spine3", 10 + o, (0, 0, -phase * 0.35)),
            ("head",   10 + o, (0, 0, -phase * 0.40)),
        ]

    # Loop endpoint
    keys += [
        ("spine1", 60, (0, 0, 0.15)),
        ("spine2", 60, (0, 0, 0.25)),
        ("spine3", 60, (0, 0, 0.35)),
        ("head",   60, (0, 0, 0.40)),
    ]

    return create_action(arm, "A_Snake_Bob", 60, keys)


def make_lunge(arm):
    """A_Snake_Lunge (15 frames): Head snaps forward rapidly, body follows."""
    keys = []

    # Frame 1: coiled back (wind-up)
    keys += [
        ("spine1", 1, (0, 0, 0)),
        ("spine2", 1, (-0.15, 0, 0)),
        ("spine3", 1, (-0.3, 0, 0)),
        ("head",   1, (-0.5, 0, 0)),
    ]

    # Frame 4: lunge — head snaps forward-down
    keys += [
        ("spine1", 4, (0.1, 0, 0)),
        ("spine2", 4, (0.3, 0, 0)),
        ("spine3", 4, (0.5, 0, 0)),
        ("head",   4, (0.8, 0, 0)),
    ]

    # Frame 7: full extension — head far forward
    keys += [
        ("spine1", 7, (0.15, 0, 0)),
        ("spine2", 7, (0.4, 0, 0)),
        ("spine3", 7, (0.6, 0, 0)),
        ("head",   7, (0.9, 0, 0)),
    ]

    # Frame 11: recoiling back
    keys += [
        ("spine1", 11, (0.05, 0, 0)),
        ("spine2", 11, (0.1, 0, 0)),
        ("spine3", 11, (0.15, 0, 0)),
        ("head",   11, (0.1, 0, 0)),
    ]

    # Frame 15: settled back to neutral
    keys += [
        ("spine1", 15, (0, 0, 0)),
        ("spine2", 15, (0, 0, 0)),
        ("spine3", 15, (0, 0, 0)),
        ("head",   15, (0, 0, 0)),
    ]

    return create_action(arm, "A_Snake_Lunge", 15, keys)


def make_hide(arm):
    """A_Snake_Hide (20 frames): Retracts downward into ground."""
    keys = []

    # Frame 1: fully upright
    keys += [
        ("spine1", 1, (0, 0, 0)),
        ("spine2", 1, (0, 0, 0)),
        ("spine3", 1, (0, 0, 0)),
        ("head",   1, (0, 0, 0)),
    ]

    # Frame 6: head starts dropping, upper spine bends
    keys += [
        ("spine1", 6, (0.1, 0, 0)),
        ("spine2", 6, (0.15, 0, 0)),
        ("spine3", 6, (0.3, 0, 0)),
        ("head",   6, (0.4, 0, 0)),
    ]

    # Frame 12: retracting quickly
    keys += [
        ("spine1", 12, (0.4, 0, 0)),
        ("spine2", 12, (0.5, 0, 0)),
        ("spine3", 12, (0.6, 0, 0)),
        ("head",   12, (0.6, 0, 0)),
    ]

    # Frame 17: almost fully retracted
    keys += [
        ("spine1", 17, (0.7, 0, 0)),
        ("spine2", 17, (0.6, 0, 0)),
        ("spine3", 17, (0.5, 0, 0)),
        ("head",   17, (0.4, 0, 0)),
    ]

    # Frame 20: fully retracted — everything bent down
    keys += [
        ("spine1", 20, (0.8, 0, 0)),
        ("spine2", 20, (0.6, 0, 0)),
        ("spine3", 20, (0.5, 0, 0)),
        ("head",   20, (0.4, 0, 0)),
    ]

    return create_action(arm, "A_Snake_Hide", 20, keys)


def make_death(arm):
    """A_Snake_Death (30 frames): Coils and falls limp."""
    keys = []

    # Frame 1: neutral upright
    for b in ["spine1", "spine2", "spine3", "head"]:
        keys.append((b, 1, (0, 0, 0)))

    # Frame 6: starts coiling — spine segments twist
    keys += [
        ("spine1", 6, (0.2, 0, 0.3)),
        ("spine2", 6, (0.3, 0, -0.4)),
        ("spine3", 6, (0.2, 0, 0.5)),
        ("head",   6, (0.4, 0.2, -0.3)),
    ]

    # Frame 12: heavy coiling, bending over
    keys += [
        ("spine1", 12, (0.5, 0, 0.5)),
        ("spine2", 12, (0.6, 0, -0.6)),
        ("spine3", 12, (0.5, 0, 0.7)),
        ("head",   12, (0.8, 0.3, -0.5)),
    ]

    # Frame 18: collapsing — all segments falling forward
    keys += [
        ("spine1", 18, (0.8, 0.2, 0.4)),
        ("spine2", 18, (0.9, -0.2, -0.5)),
        ("spine3", 18, (0.7, 0.3, 0.6)),
        ("head",   18, (1.0, 0.4, -0.4)),
    ]

    # Frame 24: nearly limp
    keys += [
        ("spine1", 24, (1.0, 0.2, 0.3)),
        ("spine2", 24, (1.0, -0.2, -0.4)),
        ("spine3", 24, (0.8, 0.3, 0.5)),
        ("head",   24, (1.1, 0.4, -0.3)),
    ]

    # Frame 30: hold — limp on ground
    keys += [
        ("spine1", 30, (1.0, 0.2, 0.3)),
        ("spine2", 30, (1.0, -0.2, -0.4)),
        ("spine3", 30, (0.8, 0.3, 0.5)),
        ("head",   30, (1.1, 0.4, -0.3)),
    ]

    return create_action(arm, "A_Snake_Death", 30, keys)


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

    print("  Building snake mesh...")
    mesh_obj = build_snake_mesh()

    print("  Building armature...")
    arm_obj = build_armature()

    print("  Skinning...")
    skin_mesh(mesh_obj, arm_obj)

    print("  Creating animations...")
    actions = [
        make_emerge(arm_obj),
        make_bob(arm_obj),
        make_lunge(arm_obj),
        make_hide(arm_obj),
        make_death(arm_obj),
    ]

    # Rotate from Blender -Y forward to +X forward for UE
    arm_obj.rotation_euler[2] = math.pi / 2

    sk_path = os.path.join(OUTPUT_DIR, "SK_Snake.fbx")
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
