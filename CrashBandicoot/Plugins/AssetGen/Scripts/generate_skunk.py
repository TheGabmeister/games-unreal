"""
Generate a rigged Skunk enemy skeletal mesh for UE5.

Creates SK_Skunk.fbx — elongated cylinder body with raised tail cone and 4 leg stubs,
rigged with a simple armature (root, body, tail, leg_fl, leg_fr, leg_bl, leg_br).

Usage:
    blender --background --python generate_skunk.py -- [output_dir]

Output:
    SK_Skunk.fbx — skeletal mesh (mesh + skeleton, bind pose)
    A_Skunk_Idle.fbx — tail sway
    A_Skunk_Walk.fbx — forward walk cycle, legs alternate
    A_Skunk_Death.fbx — falls sideways
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
# Skunk Mesh — elongated body, raised tail cone, 4 leg stubs, small head
# Faces -Y in Blender (UE +X forward after export)
# ---------------------------------------------------------------------------

def build_skunk_mesh():
    single_mat = bpy.data.materials.new("M_Skunk_Placeholder")
    single_mat.use_nodes = True
    bsdf = single_mat.node_tree.nodes["Principled BSDF"]
    bsdf.inputs["Base Color"].default_value = (0.05, 0.05, 0.05, 1.0)

    P = []

    # Body — elongated cylinder, slightly flattened
    body = cylinder((0, 0, 0.12), 0.10, 0.40, rot=(math.pi / 2, 0, 0))
    body.scale = (1.0, 1.0, 0.85)
    _sel(body)
    bpy.ops.object.transform_apply(scale=True)
    P.append(body)

    # Head — small sphere at front
    head = sphere((0, -0.24, 0.14), 0.08)
    P.append(head)

    # Snout — small cone
    snout = cone((0, -0.32, 0.12), 0.04, 0.01, 0.08,
                 rot=(math.pi / 2, 0, 0))
    P.append(snout)

    # Ears
    for side in (-1, 1):
        ear = cone((side * 0.05, -0.22, 0.22), 0.02, 0.005, 0.05,
                   rot=(0, 0, 0))
        P.append(ear)

    # Tail — raised cone curving upward from rear
    tail = cone((0, 0.22, 0.28), 0.05, 0.01, 0.25,
                rot=(math.radians(-30), 0, 0))
    P.append(tail)

    # 4 leg stubs
    leg_positions = [
        (-0.07, -0.12, 0.0),   # front-left
        (0.07, -0.12, 0.0),    # front-right
        (-0.07, 0.12, 0.0),    # back-left
        (0.07, 0.12, 0.0),     # back-right
    ]
    for pos in leg_positions:
        leg = cylinder(pos, 0.025, 0.10)
        P.append(leg)

    # Eyes
    for side in (-1, 1):
        eye = sphere((side * 0.04, -0.28, 0.17), 0.015)
        P.append(eye)

    result = join(P)
    result.name = "Skunk_Mesh"
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
    ("root",    (0, 0, 0),          (0, 0, 0.12),       None),
    ("body",    (0, 0, 0.12),       (0, -0.10, 0.14),   "root"),
    ("tail",    (0, 0.18, 0.14),    (0, 0.28, 0.35),    "body"),
    ("leg_fl",  (-0.07, -0.12, 0.05), (-0.07, -0.12, 0.0), "body"),
    ("leg_fr",  (0.07, -0.12, 0.05),  (0.07, -0.12, 0.0),  "body"),
    ("leg_bl",  (-0.07, 0.12, 0.05),  (-0.07, 0.12, 0.0),  "body"),
    ("leg_br",  (0.07, 0.12, 0.05),   (0.07, 0.12, 0.0),   "body"),
]

BONE_RADIUS = {
    "root": 0.30,
    "body": 0.30,
    "tail": 0.15,
    "leg_fl": 0.08,
    "leg_fr": 0.08,
    "leg_bl": 0.08,
    "leg_br": 0.08,
}


def build_armature():
    bpy.ops.object.armature_add(location=(0, 0, 0))
    arm_obj = bpy.context.active_object
    arm_obj.name = "Skunk_Armature"
    arm = arm_obj.data
    arm.name = "Skunk_Skeleton"

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
    # Tail sway — gentle left/right swing
    for cycle in range(3):
        o = cycle * 20
        keys += [
            ("tail", 1 + o,  (0.05, 0, 0.15)),
            ("tail", 10 + o, (0.05, 0, -0.15)),
        ]
    keys += [("tail", 60, (0.05, 0, 0.15))]

    # Subtle body breathing
    keys += [
        ("body", 1,  (0.01, 0, 0)),
        ("body", 30, (-0.01, 0, 0)),
        ("body", 60, (0.01, 0, 0)),
    ]

    return create_action(arm, "A_Skunk_Idle", 60, keys)


def make_walk(arm):
    keys = []
    # 4-beat walk — diagonal legs move together (trot gait)
    step_frames = 10
    for step in range(4):
        o = step * step_frames
        phase = 1 if step % 2 == 0 else -1
        swing = 0.55

        # Front-left + Back-right move together
        keys += [
            ("leg_fl", 1 + o, (phase * swing, 0, 0)),
            ("leg_br", 1 + o, (phase * swing, 0, 0)),
            ("leg_fr", 1 + o, (-phase * swing, 0, 0)),
            ("leg_bl", 1 + o, (-phase * swing, 0, 0)),
        ]

        # Body rock side-to-side and slight forward bob
        keys += [
            ("body", 1 + o, (0.04, 0, phase * 0.08)),
        ]

        # Tail sways opposite to body
        keys += [
            ("tail", 1 + o, (0.08, 0, -phase * 0.20)),
        ]

    # Loop endpoint
    keys += [
        ("leg_fl", 40, (0.55, 0, 0)),
        ("leg_br", 40, (0.55, 0, 0)),
        ("leg_fr", 40, (-0.55, 0, 0)),
        ("leg_bl", 40, (-0.55, 0, 0)),
        ("body",   40, (0.04, 0, 0.08)),
        ("tail",   40, (0.08, 0, -0.20)),
    ]

    return create_action(arm, "A_Skunk_Walk", 40, keys)


def make_death(arm):
    keys = []
    # Frame 1: neutral
    for b in ["body", "tail", "leg_fl", "leg_fr", "leg_bl", "leg_br"]:
        keys.append((b, 1, (0, 0, 0)))

    # Frame 8: starts tipping sideways
    keys += [
        ("body",   8, (0, 0.2, 0.3)),
        ("tail",   8, (0, 0, 0.2)),
        ("leg_fl", 8, (0, 0, -0.2)),
        ("leg_fr", 8, (0, 0, 0.3)),
        ("leg_bl", 8, (0, 0, -0.2)),
        ("leg_br", 8, (0, 0, 0.3)),
    ]

    # Frame 16: on its side
    keys += [
        ("body",   16, (0, 0.3, math.pi / 2)),
        ("tail",   16, (-0.2, 0, 0.5)),
        ("leg_fl", 16, (0.1, 0, -0.4)),
        ("leg_fr", 16, (0.1, 0, 0.5)),
        ("leg_bl", 16, (0.1, 0, -0.4)),
        ("leg_br", 16, (0.1, 0, 0.5)),
    ]

    # Frame 24: settled, legs slightly curled
    keys += [
        ("body",   24, (0, 0.3, math.pi / 2)),
        ("tail",   24, (-0.3, 0, 0.5)),
        ("leg_fl", 24, (0.2, 0, -0.4)),
        ("leg_fr", 24, (0.2, 0, 0.5)),
        ("leg_bl", 24, (0.2, 0, -0.4)),
        ("leg_br", 24, (0.2, 0, 0.5)),
    ]

    # Frame 30: hold
    keys += [
        ("body",   30, (0, 0.3, math.pi / 2)),
        ("tail",   30, (-0.3, 0, 0.5)),
        ("leg_fl", 30, (0.2, 0, -0.4)),
        ("leg_fr", 30, (0.2, 0, 0.5)),
        ("leg_bl", 30, (0.2, 0, -0.4)),
        ("leg_br", 30, (0.2, 0, 0.5)),
    ]

    return create_action(arm, "A_Skunk_Death", 30, keys)


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

    print("  Building skunk mesh...")
    mesh_obj = build_skunk_mesh()

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

    sk_path = os.path.join(OUTPUT_DIR, "SK_Skunk.fbx")
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
