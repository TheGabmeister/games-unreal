"""
Generate a rigged Spider enemy skeletal mesh for UE5.

Creates SK_Spider.fbx — a sphere body with 8 thin cylinder legs (4 bone pairs),
rigged with a simple armature (root, body, leg_fl, leg_fr, leg_bl, leg_br).

Usage:
    blender --background --python generate_spider.py -- [output_dir]

Output:
    SK_Spider.fbx — skeletal mesh (mesh + skeleton, bind pose)
    A_Spider_Hang.fbx — legs slightly curled, idle dangle (ceiling)
    A_Spider_Drop.fbx — legs splay outward on descent
    A_Spider_Land.fbx — legs compress on impact
    A_Spider_Climb.fbx — legs alternate upward pull (ceiling return)
    A_Spider_Jump.fbx — legs compress then extend upward
    A_Spider_Death.fbx — legs curl inward
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


def cylinder(loc, radius, depth, rot=(0, 0, 0), segs=8):
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
# Spider Mesh — sphere body, 8 thin cylinder legs (paired), small head, eyes
# Faces -Y in Blender (UE +X forward after export)
# Each leg bone drives a pair of 2 visual legs (front-left pair, etc.)
# ---------------------------------------------------------------------------

def build_spider_mesh():
    single_mat = bpy.data.materials.new("M_Spider_Placeholder")
    single_mat.use_nodes = True
    bsdf = single_mat.node_tree.nodes["Principled BSDF"]
    bsdf.inputs["Base Color"].default_value = (0.05, 0.05, 0.05, 1.0)

    P = []

    # Body — sphere, slightly flattened
    body = sphere((0, 0, 0.12), 0.10)
    body.scale = (1.0, 0.9, 0.7)
    _sel(body)
    bpy.ops.object.transform_apply(scale=True)
    P.append(body)

    # Head — smaller sphere at front
    head = sphere((0, -0.10, 0.14), 0.06)
    P.append(head)

    # Eyes — 4 small spheres in pairs (spider-like)
    eye_positions = [
        (-0.03, -0.14, 0.16),
        (0.03, -0.14, 0.16),
        (-0.05, -0.12, 0.17),
        (0.05, -0.12, 0.17),
    ]
    for pos in eye_positions:
        eye = sphere(pos, 0.012)
        P.append(eye)

    # Fangs — two small cones pointing down from head
    for side in (-1, 1):
        fang = cone((side * 0.02, -0.15, 0.08), 0.008, 0.002, 0.05,
                    rot=(0, 0, 0))
        P.append(fang)

    # 8 legs — 2 per bone group, arranged around the body
    # Each pair: one slightly forward, one slightly behind within the group
    # Legs angle outward and down with a knee bend (two segments each)

    leg_configs = [
        # (label, side_x, y_offset, angle_z) — upper and lower segment
        # Front-left pair
        ("fl_a", -1, -0.06, math.radians(30)),
        ("fl_b", -1, -0.02, math.radians(50)),
        # Front-right pair
        ("fr_a",  1, -0.06, math.radians(-30)),
        ("fr_b",  1, -0.02, math.radians(-50)),
        # Back-left pair
        ("bl_a", -1,  0.06, math.radians(150)),
        ("bl_b", -1,  0.02, math.radians(130)),
        # Back-right pair
        ("br_a",  1,  0.06, math.radians(-150)),
        ("br_b",  1,  0.02, math.radians(-130)),
    ]

    for label, side_x, y_off, angle_z in leg_configs:
        # Upper leg segment — from body outward
        ux = side_x * 0.08
        uy = y_off
        uz = 0.12
        leg_len = 0.14
        # Direction: outward and slightly down
        dx = math.sin(angle_z) * leg_len
        dy = math.cos(angle_z) * leg_len
        mid_x = ux + dx * 0.5
        mid_y = uy + dy * 0.5
        mid_z = uz - 0.02

        rot_angle = math.atan2(abs(dx), 0.02)
        upper = cylinder(
            (mid_x, mid_y, mid_z),
            0.012, leg_len,
            rot=(0, rot_angle * (-1 if side_x > 0 else 1), angle_z),
            segs=6,
        )
        P.append(upper)

        # Lower leg segment — angling down to ground
        tip_x = ux + dx
        tip_y = uy + dy
        lower_len = 0.12
        lower = cylinder(
            (tip_x, tip_y, 0.04),
            0.010, lower_len,
            rot=(math.radians(10), 0, 0),
            segs=6,
        )
        P.append(lower)

    # Abdomen — larger sphere behind body
    abdomen = sphere((0, 0.10, 0.10), 0.08)
    abdomen.scale = (0.9, 1.2, 0.8)
    _sel(abdomen)
    bpy.ops.object.transform_apply(scale=True)
    P.append(abdomen)

    result = join(P)
    result.name = "Spider_Mesh"
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
# Armature — 6-bone rig (root, body, 4 leg pairs)
# ---------------------------------------------------------------------------

# (bone_name, head, tail, parent_name)
BONES = [
    ("root",    (0, 0, 0),          (0, 0, 0.12),       None),
    ("body",    (0, 0, 0.12),       (0, -0.06, 0.14),   "root"),
    ("leg_fl",  (-0.08, -0.04, 0.12), (-0.22, -0.08, 0.0), "body"),
    ("leg_fr",  (0.08, -0.04, 0.12),  (0.22, -0.08, 0.0),  "body"),
    ("leg_bl",  (-0.08, 0.04, 0.12),  (-0.22, 0.08, 0.0),  "body"),
    ("leg_br",  (0.08, 0.04, 0.12),   (0.22, 0.08, 0.0),   "body"),
]

BONE_RADIUS = {
    "root": 0.25,
    "body": 0.22,
    "leg_fl": 0.18,
    "leg_fr": 0.18,
    "leg_bl": 0.18,
    "leg_br": 0.18,
}


def build_armature():
    bpy.ops.object.armature_add(location=(0, 0, 0))
    arm_obj = bpy.context.active_object
    arm_obj.name = "Spider_Armature"
    arm = arm_obj.data
    arm.name = "Spider_Skeleton"

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


def make_hang(arm):
    """Legs slightly curled, idle dangle from ceiling."""
    keys = []

    # Body gentle sway
    keys += [
        ("body", 1,  (0.05, 0, 0.03)),
        ("body", 10, (-0.03, 0, -0.04)),
        ("body", 20, (0.04, 0, 0.02)),
        ("body", 30, (-0.03, 0, -0.03)),
        ("body", 40, (0.05, 0, 0.03)),
    ]

    # Legs slightly curled inward (hanging pose)
    curl = 0.35
    keys += [
        ("leg_fl", 1,  (curl, 0, 0.15)),
        ("leg_fr", 1,  (curl, 0, -0.15)),
        ("leg_bl", 1,  (curl, 0, 0.15)),
        ("leg_br", 1,  (curl, 0, -0.15)),
    ]

    # Subtle leg twitch
    keys += [
        ("leg_fl", 15, (curl + 0.1, 0, 0.15)),
        ("leg_fl", 25, (curl - 0.05, 0, 0.15)),
        ("leg_fl", 40, (curl, 0, 0.15)),
        ("leg_fr", 20, (curl + 0.08, 0, -0.15)),
        ("leg_fr", 35, (curl - 0.05, 0, -0.15)),
        ("leg_fr", 40, (curl, 0, -0.15)),
        ("leg_bl", 10, (curl + 0.06, 0, 0.15)),
        ("leg_bl", 30, (curl - 0.04, 0, 0.15)),
        ("leg_bl", 40, (curl, 0, 0.15)),
        ("leg_br", 18, (curl + 0.1, 0, -0.15)),
        ("leg_br", 32, (curl - 0.06, 0, -0.15)),
        ("leg_br", 40, (curl, 0, -0.15)),
    ]

    return create_action(arm, "A_Spider_Hang", 40, keys)


def make_drop(arm):
    """Legs splay outward on descent."""
    keys = []

    # Frame 1: curled hang pose
    curl = 0.35
    keys += [
        ("body",   1, (0, 0, 0)),
        ("leg_fl", 1, (curl, 0, 0.15)),
        ("leg_fr", 1, (curl, 0, -0.15)),
        ("leg_bl", 1, (curl, 0, 0.15)),
        ("leg_br", 1, (curl, 0, -0.15)),
    ]

    # Frame 5: legs begin to splay as spider releases
    keys += [
        ("body",   5, (0.1, 0, 0)),
        ("leg_fl", 5, (0.1, 0, -0.3)),
        ("leg_fr", 5, (0.1, 0, 0.3)),
        ("leg_bl", 5, (0.1, 0, -0.3)),
        ("leg_br", 5, (0.1, 0, 0.3)),
    ]

    # Frame 10: fully splayed — legs spread wide, body tilted
    keys += [
        ("body",   10, (0.15, 0, 0)),
        ("leg_fl", 10, (-0.4, 0, -0.5)),
        ("leg_fr", 10, (-0.4, 0, 0.5)),
        ("leg_bl", 10, (-0.4, 0, -0.5)),
        ("leg_br", 10, (-0.4, 0, 0.5)),
    ]

    # Frame 15: hold splayed pose
    keys += [
        ("body",   15, (0.15, 0, 0)),
        ("leg_fl", 15, (-0.4, 0, -0.5)),
        ("leg_fr", 15, (-0.4, 0, 0.5)),
        ("leg_bl", 15, (-0.4, 0, -0.5)),
        ("leg_br", 15, (-0.4, 0, 0.5)),
    ]

    return create_action(arm, "A_Spider_Drop", 15, keys)


def make_land(arm):
    """Legs compress on impact."""
    keys = []

    # Frame 1: splayed from drop
    keys += [
        ("body",   1, (0.15, 0, 0)),
        ("leg_fl", 1, (-0.4, 0, -0.5)),
        ("leg_fr", 1, (-0.4, 0, 0.5)),
        ("leg_bl", 1, (-0.4, 0, -0.5)),
        ("leg_br", 1, (-0.4, 0, 0.5)),
    ]

    # Frame 4: impact — legs compress hard, body drops
    keys += [
        ("body",   4, (-0.2, 0, 0)),
        ("leg_fl", 4, (0.6, 0, -0.2)),
        ("leg_fr", 4, (0.6, 0, 0.2)),
        ("leg_bl", 4, (0.6, 0, -0.2)),
        ("leg_br", 4, (0.6, 0, 0.2)),
    ]

    # Frame 7: spring back to standing
    keys += [
        ("body",   7, (-0.05, 0, 0)),
        ("leg_fl", 7, (0.15, 0, -0.15)),
        ("leg_fr", 7, (0.15, 0, 0.15)),
        ("leg_bl", 7, (0.15, 0, -0.15)),
        ("leg_br", 7, (0.15, 0, 0.15)),
    ]

    # Frame 10: settled neutral
    keys += [
        ("body",   10, (0, 0, 0)),
        ("leg_fl", 10, (0.1, 0, -0.1)),
        ("leg_fr", 10, (0.1, 0, 0.1)),
        ("leg_bl", 10, (0.1, 0, -0.1)),
        ("leg_br", 10, (0.1, 0, 0.1)),
    ]

    return create_action(arm, "A_Spider_Land", 10, keys)


def make_climb(arm):
    """Legs alternate upward pull — climbing back to ceiling."""
    keys = []

    # Alternating leg pulls: front-left + back-right, then front-right + back-left
    stride = 0.5

    for step in range(4):
        o = step * 7  # ~7 frames per step
        phase = 1 if step % 2 == 0 else -1

        # Diagonal pair reaches up, other pair pushes
        keys += [
            ("leg_fl", 1 + o, (phase * stride, 0, -0.15)),
            ("leg_br", 1 + o, (phase * stride, 0, 0.15)),
            ("leg_fr", 1 + o, (-phase * stride, 0, 0.15)),
            ("leg_bl", 1 + o, (-phase * stride, 0, -0.15)),
        ]

        # Body rocks with the pull
        keys += [
            ("body", 1 + o, (0.06, 0, phase * 0.08)),
        ]

    # Loop endpoint
    keys += [
        ("leg_fl", 30, (stride, 0, -0.15)),
        ("leg_br", 30, (stride, 0, 0.15)),
        ("leg_fr", 30, (-stride, 0, 0.15)),
        ("leg_bl", 30, (-stride, 0, -0.15)),
        ("body",   30, (0.06, 0, 0.08)),
    ]

    return create_action(arm, "A_Spider_Climb", 30, keys)


def make_jump(arm):
    """Legs compress then extend upward — ground jumper."""
    keys = []

    # Frame 1: neutral standing
    keys += [
        ("body",   1, (0, 0, 0)),
        ("leg_fl", 1, (0.1, 0, -0.1)),
        ("leg_fr", 1, (0.1, 0, 0.1)),
        ("leg_bl", 1, (0.1, 0, -0.1)),
        ("leg_br", 1, (0.1, 0, 0.1)),
    ]

    # Frame 6: crouch — legs compress, body drops
    keys += [
        ("body",   6, (-0.25, 0, 0)),
        ("leg_fl", 6, (0.7, 0, -0.2)),
        ("leg_fr", 6, (0.7, 0, 0.2)),
        ("leg_bl", 6, (0.7, 0, -0.2)),
        ("leg_br", 6, (0.7, 0, 0.2)),
    ]

    # Frame 10: launch — legs extend fully, body lifts
    keys += [
        ("body",   10, (0.3, 0, 0)),
        ("leg_fl", 10, (-0.5, 0, -0.4)),
        ("leg_fr", 10, (-0.5, 0, 0.4)),
        ("leg_bl", 10, (-0.5, 0, -0.4)),
        ("leg_br", 10, (-0.5, 0, 0.4)),
    ]

    # Frame 15: airborne — legs tucked slightly
    keys += [
        ("body",   15, (0.15, 0, 0)),
        ("leg_fl", 15, (-0.2, 0, -0.3)),
        ("leg_fr", 15, (-0.2, 0, 0.3)),
        ("leg_bl", 15, (-0.2, 0, -0.3)),
        ("leg_br", 15, (-0.2, 0, 0.3)),
    ]

    # Frame 20: hold airborne pose
    keys += [
        ("body",   20, (0.15, 0, 0)),
        ("leg_fl", 20, (-0.2, 0, -0.3)),
        ("leg_fr", 20, (-0.2, 0, 0.3)),
        ("leg_bl", 20, (-0.2, 0, -0.3)),
        ("leg_br", 20, (-0.2, 0, 0.3)),
    ]

    return create_action(arm, "A_Spider_Jump", 20, keys)


def make_death(arm):
    """Legs curl inward — death animation."""
    keys = []

    # Frame 1: neutral
    for b in ["body", "leg_fl", "leg_fr", "leg_bl", "leg_br"]:
        keys.append((b, 1, (0, 0, 0)))

    # Frame 6: hit reaction — body jolts
    keys += [
        ("body",   6, (-0.2, 0.15, 0)),
        ("leg_fl", 6, (0.2, 0, 0.1)),
        ("leg_fr", 6, (0.2, 0, -0.1)),
        ("leg_bl", 6, (0.2, 0, 0.1)),
        ("leg_br", 6, (0.2, 0, -0.1)),
    ]

    # Frame 14: legs curling inward, body tipping
    keys += [
        ("body",   14, (-0.1, 0.3, 0.4)),
        ("leg_fl", 14, (0.6, 0, 0.4)),
        ("leg_fr", 14, (0.6, 0, -0.4)),
        ("leg_bl", 14, (0.6, 0, 0.4)),
        ("leg_br", 14, (0.6, 0, -0.4)),
    ]

    # Frame 22: fully curled — legs pulled tight under body
    keys += [
        ("body",   22, (0, 0.3, math.pi * 0.4)),
        ("leg_fl", 22, (0.9, 0, 0.6)),
        ("leg_fr", 22, (0.9, 0, -0.6)),
        ("leg_bl", 22, (0.9, 0, 0.6)),
        ("leg_br", 22, (0.9, 0, -0.6)),
    ]

    # Frame 30: hold curled pose
    keys += [
        ("body",   30, (0, 0.3, math.pi * 0.4)),
        ("leg_fl", 30, (0.9, 0, 0.6)),
        ("leg_fr", 30, (0.9, 0, -0.6)),
        ("leg_bl", 30, (0.9, 0, 0.6)),
        ("leg_br", 30, (0.9, 0, -0.6)),
    ]

    return create_action(arm, "A_Spider_Death", 30, keys)


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

    print("  Building spider mesh...")
    mesh_obj = build_spider_mesh()

    print("  Building armature...")
    arm_obj = build_armature()

    print("  Skinning...")
    skin_mesh(mesh_obj, arm_obj)

    print("  Creating animations...")
    actions = [
        make_hang(arm_obj),
        make_drop(arm_obj),
        make_land(arm_obj),
        make_climb(arm_obj),
        make_jump(arm_obj),
        make_death(arm_obj),
    ]

    # Rotate from Blender -Y forward to +X forward for UE
    arm_obj.rotation_euler[2] = math.pi / 2

    sk_path = os.path.join(OUTPUT_DIR, "SK_Spider.fbx")
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
