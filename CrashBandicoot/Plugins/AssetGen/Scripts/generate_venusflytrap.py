"""
Generate a rigged Venus Fly Trap enemy skeletal mesh for UE5.

Creates SK_VenusFlyTrap.fbx — cylinder stalk with two half-sphere jaws on top,
rigged with a simple armature (root, stalk, jaw_top, jaw_bottom).

Usage:
    blender --background --python generate_venusflytrap.py -- [output_dir]

Output:
    SK_VenusFlyTrap.fbx — skeletal mesh (mesh + skeleton, bind pose)
    A_VenusFlyTrap_Open.fbx — jaws spread wide (vulnerable state)
    A_VenusFlyTrap_Snap.fbx — jaws slam shut rapidly
    A_VenusFlyTrap_Death.fbx — stalk wilts, jaws droop
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
# Venus Fly Trap Mesh — cylinder stalk with two half-sphere jaws
# Faces -Y in Blender (UE +X forward after export)
# ---------------------------------------------------------------------------

def build_venusflytrap_mesh():
    single_mat = bpy.data.materials.new("M_VenusFlyTrap_Placeholder")
    single_mat.use_nodes = True
    bsdf = single_mat.node_tree.nodes["Principled BSDF"]
    bsdf.inputs["Base Color"].default_value = (0.2, 0.6, 0.1, 1.0)

    P = []

    # Stalk — tall cylinder
    stalk = cylinder((0, 0, 0.25), 0.05, 0.50)
    P.append(stalk)

    # Stalk base bulge — slightly wider at the bottom
    base_bulge = cylinder((0, 0, 0.04), 0.08, 0.08)
    P.append(base_bulge)

    # Upper jaw — half-sphere (top), scaled to be flat on the bottom
    jaw_top = sphere((0, -0.02, 0.55), 0.12, segs=16, rings=8)
    jaw_top.scale = (1.0, 1.2, 0.6)
    _sel(jaw_top)
    bpy.ops.object.transform_apply(scale=True)
    # Delete bottom half to make it a half-sphere
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_all(action='DESELECT')
    bpy.ops.object.mode_set(mode='OBJECT')
    # Select vertices below the center
    for v in jaw_top.data.vertices:
        if v.co.z < -0.01:
            v.select = True
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.delete(type='VERT')
    bpy.ops.object.mode_set(mode='OBJECT')
    P.append(jaw_top)

    # Lower jaw — half-sphere (bottom), flipped
    jaw_bottom = sphere((0, -0.02, 0.45), 0.12, segs=16, rings=8)
    jaw_bottom.scale = (1.0, 1.2, 0.6)
    _sel(jaw_bottom)
    bpy.ops.object.transform_apply(scale=True)
    # Delete top half
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_all(action='DESELECT')
    bpy.ops.object.mode_set(mode='OBJECT')
    for v in jaw_bottom.data.vertices:
        if v.co.z > 0.01:
            v.select = True
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.delete(type='VERT')
    bpy.ops.object.mode_set(mode='OBJECT')
    P.append(jaw_bottom)

    # Small leaf-like flaps on the stalk
    for side in (-1, 1):
        leaf = cube((side * 0.08, 0, 0.20), (0.06, 0.02, 0.04))
        leaf.rotation_euler = (0, 0, side * 0.3)
        P.append(leaf)

    result = join(P)
    result.name = "VenusFlyTrap_Mesh"
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
    ("root",       (0, 0, 0),       (0, 0, 0.10),    None),
    ("stalk",      (0, 0, 0.10),    (0, 0, 0.50),    "root"),
    ("jaw_top",    (0, 0, 0.50),    (0, -0.05, 0.60), "stalk"),
    ("jaw_bottom", (0, 0, 0.50),    (0, -0.05, 0.40), "stalk"),
]

BONE_RADIUS = {
    "root": 0.20,
    "stalk": 0.20,
    "jaw_top": 0.18,
    "jaw_bottom": 0.18,
}


def build_armature():
    bpy.ops.object.armature_add(location=(0, 0, 0))
    arm_obj = bpy.context.active_object
    arm_obj.name = "VenusFlyTrap_Armature"
    arm = arm_obj.data
    arm.name = "VenusFlyTrap_Skeleton"

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


def make_open(arm):
    """A_VenusFlyTrap_Open (60 frames): Jaws spread wide, stalk sways gently."""
    keys = []

    # Frame 1: jaws mostly closed
    keys += [
        ("jaw_top",    1, (0.1, 0, 0)),
        ("jaw_bottom", 1, (-0.1, 0, 0)),
        ("stalk",      1, (0, 0, 0)),
    ]

    # Frame 15: jaws start opening
    keys += [
        ("jaw_top",    15, (0.4, 0, 0)),
        ("jaw_bottom", 15, (-0.4, 0, 0)),
        ("stalk",      15, (0, 0, 0.05)),
    ]

    # Frame 30: jaws wide open (vulnerable)
    keys += [
        ("jaw_top",    30, (0.8, 0, 0)),
        ("jaw_bottom", 30, (-0.7, 0, 0)),
        ("stalk",      30, (0, 0, -0.05)),
    ]

    # Frame 45: hold open, stalk sways back
    keys += [
        ("jaw_top",    45, (0.75, 0, 0.05)),
        ("jaw_bottom", 45, (-0.65, 0, -0.05)),
        ("stalk",      45, (0.05, 0, 0.08)),
    ]

    # Frame 60: still open, gentle sway (loop point)
    keys += [
        ("jaw_top",    60, (0.8, 0, 0)),
        ("jaw_bottom", 60, (-0.7, 0, 0)),
        ("stalk",      60, (0, 0, -0.05)),
    ]

    return create_action(arm, "A_VenusFlyTrap_Open", 60, keys)


def make_snap(arm):
    """A_VenusFlyTrap_Snap (20 frames): Jaws slam shut rapidly."""
    keys = []

    # Frame 1: jaws wide open
    keys += [
        ("jaw_top",    1, (0.8, 0, 0)),
        ("jaw_bottom", 1, (-0.7, 0, 0)),
        ("stalk",      1, (0, 0, 0)),
    ]

    # Frame 5: jaws snapping — fast close begins
    keys += [
        ("jaw_top",    5, (0.3, 0, 0)),
        ("jaw_bottom", 5, (-0.25, 0, 0)),
        ("stalk",      5, (-0.1, 0, 0)),
    ]

    # Frame 8: jaws slam shut — overshoot slightly
    keys += [
        ("jaw_top",    8, (-0.15, 0, 0)),
        ("jaw_bottom", 8, (0.15, 0, 0)),
        ("stalk",      8, (-0.15, 0, 0)),
    ]

    # Frame 12: bounce back to closed
    keys += [
        ("jaw_top",    12, (0.05, 0, 0)),
        ("jaw_bottom", 12, (-0.05, 0, 0)),
        ("stalk",      12, (-0.05, 0, 0)),
    ]

    # Frame 20: settled closed
    keys += [
        ("jaw_top",    20, (0, 0, 0)),
        ("jaw_bottom", 20, (0, 0, 0)),
        ("stalk",      20, (0, 0, 0)),
    ]

    return create_action(arm, "A_VenusFlyTrap_Snap", 20, keys)


def make_death(arm):
    """A_VenusFlyTrap_Death (30 frames): Stalk wilts, jaws droop."""
    keys = []

    # Frame 1: neutral upright
    for b in ["stalk", "jaw_top", "jaw_bottom"]:
        keys.append((b, 1, (0, 0, 0)))

    # Frame 8: stalk starts bending forward, jaws loosen
    keys += [
        ("stalk",      8, (0.4, 0, 0.1)),
        ("jaw_top",    8, (0.3, 0, 0.1)),
        ("jaw_bottom", 8, (0.1, 0, -0.1)),
    ]

    # Frame 16: stalk wilting heavily, jaws drooping open
    keys += [
        ("stalk",      16, (1.0, 0, 0.15)),
        ("jaw_top",    16, (0.5, 0.1, 0.15)),
        ("jaw_bottom", 16, (0.3, -0.1, -0.15)),
    ]

    # Frame 22: nearly collapsed
    keys += [
        ("stalk",      22, (1.3, 0, 0.1)),
        ("jaw_top",    22, (0.6, 0.15, 0.1)),
        ("jaw_bottom", 22, (0.4, -0.15, -0.1)),
    ]

    # Frame 30: fully wilted, hold
    keys += [
        ("stalk",      30, (1.4, 0, 0.1)),
        ("jaw_top",    30, (0.7, 0.15, 0.1)),
        ("jaw_bottom", 30, (0.5, -0.15, -0.1)),
    ]

    return create_action(arm, "A_VenusFlyTrap_Death", 30, keys)


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

    print("  Building venus fly trap mesh...")
    mesh_obj = build_venusflytrap_mesh()

    print("  Building armature...")
    arm_obj = build_armature()

    print("  Skinning...")
    skin_mesh(mesh_obj, arm_obj)

    print("  Creating animations...")
    actions = [
        make_open(arm_obj),
        make_snap(arm_obj),
        make_death(arm_obj),
    ]

    # Rotate from Blender -Y forward to +X forward for UE
    arm_obj.rotation_euler[2] = math.pi / 2

    sk_path = os.path.join(OUTPUT_DIR, "SK_VenusFlyTrap.fbx")
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
