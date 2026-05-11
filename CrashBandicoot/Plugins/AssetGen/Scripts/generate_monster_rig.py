"""
Generate a rigged and animated monster for UE5.

Creates a skeletal mesh with armature and three animations:
  - Idle (2s loop): heavy breathing, head scanning, tail sway
  - Attack (1.5s): right arm overhead smash
  - Roar (2s): head-back battle cry with jaw open

Usage:
    blender --background --python generate_monster_rig.py -- [output_dir]

Output:
    SK_Monster.fbx         — skeletal mesh (mesh + skeleton, bind pose)
    A_Monster_Idle.fbx     — idle animation
    A_Monster_Attack.fbx   — attack animation
    A_Monster_Roar.fbx     — roar animation
"""

import bpy
import math
import mathutils
import os
import random
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
# Primitives (no material — single mat assigned after join)
# ---------------------------------------------------------------------------

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


def cone(loc, r1, r2, depth, rot=(0, 0, 0), segs=12):
    bpy.ops.mesh.primitive_cone_add(
        vertices=segs, radius1=r1, radius2=r2, depth=depth,
        location=loc, rotation=rot)
    return bpy.context.active_object


def sphere(loc, radius, segs=16, rings=8):
    bpy.ops.mesh.primitive_uv_sphere_add(
        segments=segs, ring_count=rings, radius=radius, location=loc)
    return bpy.context.active_object


# ---------------------------------------------------------------------------
# Monster mesh (same geometry as generate_meshes.py)
# ---------------------------------------------------------------------------

def build_monster_mesh():
    random.seed(2)
    P = []

    mball = bpy.data.metaballs.new("DemonMeta")
    mball.resolution = 0.10
    mball.threshold = 0.6

    def mb(co, radius):
        e = mball.elements.new()
        e.co = co
        e.radius = radius
        e.stiffness = 2.0

    mb((0, 0, 1.0), 0.55)
    mb((0, -0.15, 1.35), 0.40)
    mb((0, 0.08, 0.65), 0.45)
    mb((0, -0.35, 1.55), 0.28)
    mb((0, -0.52, 1.48), 0.15)
    mb((0, -0.46, 1.38), 0.14)

    for s in (-1, 1):
        mb((s * 0.50, -0.05, 1.15), 0.22)
        mb((s * 0.72, 0.02, 0.88), 0.17)
        mb((s * 0.88, 0.08, 0.62), 0.14)
        mb((s * 0.95, 0.12, 0.42), 0.12)

    for s in (-1, 1):
        mb((s * 0.22, 0.05, 0.50), 0.20)
        mb((s * 0.24, 0.10, 0.25), 0.14)
        mb((s * 0.26, 0.15, 0.08), 0.12)

    for i in range(6):
        t = i / 5
        mb((0, 0.35 + t * 0.6, 0.65 - t * 0.35), 0.10 - t * 0.012)

    body = bpy.data.objects.new("DemonBody", mball)
    bpy.context.collection.objects.link(body)
    _sel(body)
    bpy.ops.object.convert(target='MESH')
    body = bpy.context.active_object
    P.append(body)

    for s in (-1, 1):
        pos = Vector((s * 0.12, -0.35, 1.72))
        d = Vector((s * 0.3, -0.15, 1.0)).normalized()
        for i in range(7):
            t = i / 7
            r1 = 0.045 * (1 - t * 0.85)
            r2 = 0.045 * (1 - (t + 1 / 7) * 0.85)
            end = pos + d * 0.07
            mid = (pos + end) / 2
            seg = cone(tuple(mid), r1, r2, 0.07, segs=8)
            q = Vector((0, 0, 1)).rotation_difference(d)
            seg.rotation_mode = 'QUATERNION'
            seg.rotation_quaternion = q
            P.append(seg)
            pos = end
            d = (d + Vector((s * 0.06, 0.04, -0.05))).normalized()

    for s in (-1, 1):
        P.append(sphere((s * 0.10, -0.58, 1.55), 0.035, segs=12, rings=6))

    for i in range(8):
        t = i / 7
        P.append(cone((0, 0.12 + t * 0.08, 1.45 - t * 0.80),
                       0.03, 0.005, 0.14 - t * 0.010, rot=(-0.5, 0, 0), segs=6))

    for s in (-1, 1):
        for i in range(4):
            cx = s * 0.95 + s * (i - 1.5) * 0.035
            P.append(cone((cx, 0.06, 0.33 - i * 0.015), 0.014, 0.003, 0.10,
                          rot=(0.7, 0, s * 0.08 * (i - 1.5)), segs=6))
        for i in range(3):
            fx = s * 0.26 + (i - 1) * 0.04
            P.append(cone((fx, 0.08, 0.0), 0.012, 0.003, 0.07,
                          rot=(0.8, 0, 0), segs=6))

    for s in (-1, 1):
        P.append(cone((s * 0.07, -0.60, 1.38), 0.018, 0.004, 0.10,
                       rot=(0.2, 0, s * 0.1), segs=6))

    P.append(cone((0, 0.95, 0.30), 0.04, 0.005, 0.18, rot=(-1.0, 0, 0), segs=6))

    for s in (-1, 1):
        brow = cube((s * 0.10, -0.52, 1.62), (0.10, 0.05, 0.03), bevel=0.008)
        brow.rotation_euler = (0.2, s * 0.15, 0)
        P.append(brow)

    mesh_obj = join(P)
    mesh_obj.name = "Monster"

    _sel(mesh_obj)
    bpy.ops.object.shade_smooth()
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.uv.smart_project(angle_limit=1.15)
    bpy.ops.object.mode_set(mode='OBJECT')

    m = build_procedural_material()
    mesh_obj.data.materials.append(m)

    return mesh_obj


def build_procedural_material():
    """Procedural demon skin material — baked to texture later."""
    m = bpy.data.materials.new("M_Monster")
    m.use_nodes = True
    tree = m.node_tree
    nodes = tree.nodes
    links = tree.links
    nodes.clear()

    # Output
    out = nodes.new("ShaderNodeOutputMaterial")
    out.location = (800, 0)

    bsdf = nodes.new("ShaderNodeBsdfPrincipled")
    bsdf.location = (500, 0)
    bsdf.inputs["Roughness"].default_value = 0.6
    links.new(bsdf.outputs["BSDF"], out.inputs["Surface"])

    # --- Base color from world-Z height gradient ---
    # Dark red-brown body, lighter belly, darker extremities
    tex_coord = nodes.new("ShaderNodeTexCoord")
    tex_coord.location = (-800, 200)

    separate = nodes.new("ShaderNodeSeparateXYZ")
    separate.location = (-600, 200)
    links.new(tex_coord.outputs["Object"], separate.inputs["Vector"])

    # Height ramp: Z position drives color
    height_ramp = nodes.new("ShaderNodeMapRange")
    height_ramp.location = (-400, 200)
    height_ramp.inputs["From Min"].default_value = 0.0
    height_ramp.inputs["From Max"].default_value = 1.8
    height_ramp.inputs["To Min"].default_value = 0.0
    height_ramp.inputs["To Max"].default_value = 1.0
    links.new(separate.outputs["Z"], height_ramp.inputs["Value"])

    color_ramp = nodes.new("ShaderNodeValToRGB")
    color_ramp.location = (-150, 300)
    cr = color_ramp.color_ramp
    cr.elements[0].position = 0.0
    cr.elements[0].color = (0.05, 0.02, 0.01, 1)    # feet: near black
    cr.elements[1].position = 1.0
    cr.elements[1].color = (0.12, 0.04, 0.03, 1)    # top: dark brown-red
    e = cr.elements.new(0.35)
    e.color = (0.22, 0.10, 0.06, 1)                  # belly: lighter warm tone
    e = cr.elements.new(0.7)
    e.color = (0.15, 0.05, 0.04, 1)                  # chest: mid dark
    links.new(height_ramp.outputs["Result"], color_ramp.inputs["Fac"])

    # --- Noise for skin texture variation ---
    noise1 = nodes.new("ShaderNodeTexNoise")
    noise1.location = (-400, -100)
    noise1.inputs["Scale"].default_value = 18.0
    noise1.inputs["Detail"].default_value = 8.0
    noise1.inputs["Roughness"].default_value = 0.7
    links.new(tex_coord.outputs["Object"], noise1.inputs["Vector"])

    noise_ramp = nodes.new("ShaderNodeValToRGB")
    noise_ramp.location = (-150, -100)
    nr = noise_ramp.color_ramp
    nr.elements[0].position = 0.3
    nr.elements[0].color = (0.03, 0.01, 0.005, 1)   # dark veins/cracks
    nr.elements[1].position = 0.6
    nr.elements[1].color = (0.08, 0.04, 0.03, 1)     # lighter patches
    links.new(noise1.outputs["Fac"], noise_ramp.inputs["Fac"])

    # --- Fine detail noise (scales/bumps) ---
    noise2 = nodes.new("ShaderNodeTexNoise")
    noise2.location = (-400, -350)
    noise2.inputs["Scale"].default_value = 45.0
    noise2.inputs["Detail"].default_value = 4.0
    noise2.inputs["Roughness"].default_value = 0.5
    links.new(tex_coord.outputs["Object"], noise2.inputs["Vector"])

    # --- Mix height color with noise ---
    mix1 = nodes.new("ShaderNodeMix")
    mix1.location = (100, 200)
    mix1.data_type = 'RGBA'
    mix1.inputs["Factor"].default_value = 0.5
    links.new(color_ramp.outputs["Color"], mix1.inputs[6])    # A
    links.new(noise_ramp.outputs["Color"], mix1.inputs[7])    # B

    # --- Overlay fine detail ---
    mix2 = nodes.new("ShaderNodeMix")
    mix2.location = (280, 100)
    mix2.data_type = 'RGBA'
    mix2.inputs["Factor"].default_value = 0.15
    links.new(mix1.outputs[2], mix2.inputs[6])                # A: base mix
    links.new(noise2.outputs["Fac"], mix2.inputs[7])          # B: fine detail

    # --- Glowing eyes: use Y position to detect front-facing head area ---
    # Vertices very far forward (Y < -0.5) and at head height (Z > 1.4) get orange glow
    forward_check = nodes.new("ShaderNodeMapRange")
    forward_check.location = (-400, -550)
    forward_check.inputs["From Min"].default_value = -0.62
    forward_check.inputs["From Max"].default_value = -0.54
    forward_check.inputs["To Min"].default_value = 1.0
    forward_check.inputs["To Max"].default_value = 0.0
    forward_check.clamp = True
    links.new(separate.outputs["Y"], forward_check.inputs["Value"])

    height_check = nodes.new("ShaderNodeMapRange")
    height_check.location = (-400, -750)
    height_check.inputs["From Min"].default_value = 1.48
    height_check.inputs["From Max"].default_value = 1.58
    height_check.inputs["To Min"].default_value = 0.0
    height_check.inputs["To Max"].default_value = 1.0
    height_check.clamp = True
    links.new(separate.outputs["Z"], height_check.inputs["Value"])

    eye_mult = nodes.new("ShaderNodeMath")
    eye_mult.location = (-150, -650)
    eye_mult.operation = 'MULTIPLY'
    links.new(forward_check.outputs["Result"], eye_mult.inputs[0])
    links.new(height_check.outputs["Result"], eye_mult.inputs[1])

    eye_color = nodes.new("ShaderNodeMix")
    eye_color.location = (350, 0)
    eye_color.data_type = 'RGBA'
    links.new(eye_mult.outputs["Value"], eye_color.inputs["Factor"])
    links.new(mix2.outputs[2], eye_color.inputs[6])                       # body
    eye_color.inputs[7].default_value = (1.0, 0.4, 0.05, 1.0)            # orange glow

    links.new(eye_color.outputs[2], bsdf.inputs["Base Color"])

    return m


def bake_texture(mesh_obj, filepath, size=1024):
    """Bake the procedural material's base color to a texture image."""
    bpy.context.scene.render.engine = 'CYCLES'
    bpy.context.scene.cycles.device = 'CPU'
    bpy.context.scene.cycles.samples = 4
    bpy.context.scene.cycles.bake_type = 'DIFFUSE'
    bpy.context.scene.render.bake.use_pass_direct = False
    bpy.context.scene.render.bake.use_pass_indirect = False
    bpy.context.scene.render.bake.use_pass_color = True
    bpy.context.scene.render.bake.margin = 4

    img = bpy.data.images.new("T_Monster", width=size, height=size, alpha=False)

    mat = mesh_obj.data.materials[0]
    tree = mat.node_tree
    tex_node = tree.nodes.new("ShaderNodeTexImage")
    tex_node.image = img
    tex_node.location = (500, -300)
    tree.nodes.active = tex_node

    _sel(mesh_obj)
    bpy.ops.object.bake(type='DIFFUSE')

    img.filepath_raw = filepath
    img.file_format = 'PNG'
    img.save()

    # Rewire material to use the baked texture instead of procedural nodes
    nodes = tree.nodes
    links = tree.links
    bsdf = nodes.get("Principled BSDF")

    for link in list(links):
        if link.to_node == bsdf and link.to_socket.name == "Base Color":
            links.remove(link)

    links.new(tex_node.outputs["Color"], bsdf.inputs["Base Color"])

    return img


# ---------------------------------------------------------------------------
# Skeleton
# ---------------------------------------------------------------------------

BONES = [
    # (name, head, tail, parent)
    ("root",        (0, 0, 0),             (0, 0, 0.30),          None),
    ("pelvis",      (0, 0.05, 0.62),       (0, 0, 0.78),          "root"),
    ("spine_01",    (0, 0, 0.78),          (0, -0.04, 0.98),      "pelvis"),
    ("spine_02",    (0, -0.04, 0.98),      (0, -0.10, 1.18),      "spine_01"),
    ("chest",       (0, -0.10, 1.18),      (0, -0.15, 1.35),      "spine_02"),
    ("neck",        (0, -0.15, 1.35),      (0, -0.28, 1.50),      "chest"),
    ("head",        (0, -0.28, 1.50),      (0, -0.42, 1.56),      "neck"),
    ("jaw",         (0, -0.38, 1.42),      (0, -0.56, 1.35),      "head"),

    ("upperarm_l",  (-0.38, -0.05, 1.18),  (-0.68, 0.01, 0.92),   "chest"),
    ("forearm_l",   (-0.68, 0.01, 0.92),   (-0.88, 0.08, 0.62),   "upperarm_l"),
    ("hand_l",      (-0.88, 0.08, 0.62),   (-0.96, 0.12, 0.42),   "forearm_l"),

    ("upperarm_r",  (0.38, -0.05, 1.18),   (0.68, 0.01, 0.92),    "chest"),
    ("forearm_r",   (0.68, 0.01, 0.92),    (0.88, 0.08, 0.62),    "upperarm_r"),
    ("hand_r",      (0.88, 0.08, 0.62),    (0.96, 0.12, 0.42),    "forearm_r"),

    ("thigh_l",     (-0.20, 0.05, 0.52),   (-0.23, 0.10, 0.26),   "pelvis"),
    ("shin_l",      (-0.23, 0.10, 0.26),   (-0.26, 0.14, 0.08),   "thigh_l"),
    ("foot_l",      (-0.26, 0.14, 0.08),   (-0.26, 0.22, 0.00),   "shin_l"),

    ("thigh_r",     (0.20, 0.05, 0.52),    (0.23, 0.10, 0.26),    "pelvis"),
    ("shin_r",      (0.23, 0.10, 0.26),    (0.26, 0.14, 0.08),    "thigh_r"),
    ("foot_r",      (0.26, 0.14, 0.08),    (0.26, 0.22, 0.00),    "shin_r"),

    ("tail_01",     (0, 0.18, 0.60),       (0, 0.38, 0.52),       "pelvis"),
    ("tail_02",     (0, 0.38, 0.52),       (0, 0.56, 0.44),       "tail_01"),
    ("tail_03",     (0, 0.56, 0.44),       (0, 0.74, 0.36),       "tail_02"),
    ("tail_04",     (0, 0.74, 0.36),       (0, 0.95, 0.28),       "tail_03"),
]


def build_armature():
    bpy.ops.object.armature_add(location=(0, 0, 0))
    arm_obj = bpy.context.active_object
    arm_obj.name = "Monster_Armature"
    arm = arm_obj.data
    arm.name = "Monster_Skeleton"

    bpy.ops.object.mode_set(mode='EDIT')

    # Remove default bone
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
# Skinning
# ---------------------------------------------------------------------------

def _closest_on_segment(pt, a, b):
    ab = b - a
    d = ab.dot(ab)
    if d < 1e-10:
        return a
    t = max(0.0, min(1.0, (pt - a).dot(ab) / d))
    return a + ab * t


BONE_RADIUS = {
    "root": 0.30, "pelvis": 0.35,
    "spine_01": 0.30, "spine_02": 0.30, "chest": 0.30,
    "neck": 0.20, "head": 0.25, "jaw": 0.15,
    "upperarm_l": 0.25, "forearm_l": 0.20, "hand_l": 0.18,
    "upperarm_r": 0.25, "forearm_r": 0.20, "hand_r": 0.18,
    "thigh_l": 0.25, "shin_l": 0.20, "foot_l": 0.15,
    "thigh_r": 0.25, "shin_r": 0.20, "foot_r": 0.15,
    "tail_01": 0.18, "tail_02": 0.16, "tail_03": 0.14, "tail_04": 0.12,
}


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
            best_vg = min(bones_info, key=lambda b: (vp - _closest_on_segment(vp, b[0], b[1])).length)[3]
            best_vg.add([v.index], 1.0, 'REPLACE')

    mod = mesh_obj.modifiers.new("Armature", 'ARMATURE')
    mod.object = arm_obj
    mesh_obj.parent = arm_obj


# ---------------------------------------------------------------------------
# Ground everything (move bones + mesh so lowest point = Z=0)
# ---------------------------------------------------------------------------

def ground_rig(mesh_obj, arm_obj):
    min_z = min((mesh_obj.matrix_world @ v.co).z for v in mesh_obj.data.vertices)
    if abs(min_z) < 0.001:
        return

    _sel(arm_obj)
    bpy.ops.object.mode_set(mode='EDIT')
    for bone in arm_obj.data.edit_bones:
        bone.head.z -= min_z
        bone.tail.z -= min_z
    bpy.ops.object.mode_set(mode='OBJECT')

    for v in mesh_obj.data.vertices:
        v.co.z -= min_z


# ---------------------------------------------------------------------------
# Animations
# ---------------------------------------------------------------------------

def create_action(arm_obj, name, frame_count, keyframes):
    action = bpy.data.actions.new(f"A_Monster_{name}")

    if not arm_obj.animation_data:
        arm_obj.animation_data_create()
    arm_obj.animation_data.action = action

    bpy.context.scene.frame_start = 1
    bpy.context.scene.frame_end = frame_count

    _sel(arm_obj)
    bpy.ops.object.mode_set(mode='POSE')

    # Rest all bones
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

    # Breathing cycle (spine oscillation, 2 full breaths in 60 frames)
    for cycle in range(2):
        o = cycle * 30
        keys += [
            ("spine_01", 1 + o,  (0.03, 0, 0)),
            ("spine_01", 15 + o, (-0.04, 0, 0)),
            ("spine_02", 1 + o,  (0.02, 0, 0)),
            ("spine_02", 15 + o, (-0.03, 0, 0)),
            ("chest",    1 + o,  (0.01, 0, 0)),
            ("chest",    15 + o, (-0.02, 0, 0)),
        ]
    keys += [("spine_01", 60, (0.03, 0, 0)),
             ("spine_02", 60, (0.02, 0, 0)),
             ("chest", 60, (0.01, 0, 0))]

    # Head scanning left-right
    keys += [
        ("head", 1,  (0, 0, 0.10)),
        ("head", 20, (-0.05, 0, -0.12)),
        ("head", 40, (0.03, 0, 0.08)),
        ("head", 60, (0, 0, 0.10)),
        ("neck", 1,  (0, 0, 0.05)),
        ("neck", 20, (0, 0, -0.06)),
        ("neck", 40, (0, 0, 0.04)),
        ("neck", 60, (0, 0, 0.05)),
    ]

    # Jaw subtle open/close
    keys += [
        ("jaw", 1,  (0, 0, 0)),
        ("jaw", 12, (0.06, 0, 0)),
        ("jaw", 25, (0, 0, 0)),
        ("jaw", 42, (0.04, 0, 0)),
        ("jaw", 55, (0, 0, 0)),
        ("jaw", 60, (0, 0, 0)),
    ]

    # Tail sway (increasing amplitude down the chain)
    for bone_i, tail in enumerate(["tail_01", "tail_02", "tail_03", "tail_04"]):
        amp = 0.06 + bone_i * 0.04
        phase = bone_i * 3
        for f in range(0, 61, 10):
            angle = math.sin((f + phase) / 60 * 2 * math.pi * 2) * amp
            keys.append((tail, max(1, f), (0, 0, angle)))

    # Arms subtle sway (counter to breathing)
    for side, bone in [(-1, "upperarm_l"), (1, "upperarm_r")]:
        keys += [
            (bone, 1,  (0, 0, side * 0.04)),
            (bone, 30, (0, 0, side * -0.03)),
            (bone, 60, (0, 0, side * 0.04)),
        ]

    # Pelvis subtle bob
    keys += [
        ("pelvis", 1,  (0.01, 0, 0)),
        ("pelvis", 15, (-0.01, 0, 0)),
        ("pelvis", 30, (0.01, 0, 0)),
        ("pelvis", 45, (-0.01, 0, 0)),
        ("pelvis", 60, (0.01, 0, 0)),
    ]

    return create_action(arm, "Idle", 60, keys)


def make_attack(arm):
    keys = []

    # Frame 1: neutral
    neutral_bones = ["spine_01", "spine_02", "chest", "head", "neck", "jaw",
                     "upperarm_r", "forearm_r", "hand_r",
                     "upperarm_l", "forearm_l", "pelvis",
                     "tail_01", "tail_02"]
    for b in neutral_bones:
        keys.append((b, 1, (0, 0, 0)))

    # Frame 10: wind up — lean back, raise right arm
    keys += [
        ("pelvis",      10, (-0.06, 0, 0)),
        ("spine_01",    10, (-0.12, 0, 0.03)),
        ("spine_02",    10, (-0.10, 0, 0.02)),
        ("chest",       10, (-0.08, 0, 0)),
        ("neck",        10, (-0.06, 0, 0.05)),
        ("head",        10, (-0.08, 0, 0.06)),
        ("jaw",         10, (0.04, 0, 0)),
        ("upperarm_r",  10, (-0.6, -0.2, -0.4)),
        ("forearm_r",   10, (-0.4, 0, -0.2)),
        ("hand_r",      10, (-0.2, 0, 0)),
        ("upperarm_l",  10, (0.05, 0, 0.1)),
        ("tail_01",     10, (-0.08, 0, 0)),
        ("tail_02",     10, (-0.06, 0, 0)),
    ]

    # Frame 16: strike — explosive forward smash
    keys += [
        ("pelvis",      16, (0.08, 0, 0)),
        ("spine_01",    16, (0.18, 0, -0.02)),
        ("spine_02",    16, (0.14, 0, -0.01)),
        ("chest",       16, (0.12, 0, 0)),
        ("neck",        16, (0.08, 0, -0.04)),
        ("head",        16, (0.10, 0, -0.06)),
        ("jaw",         16, (0.18, 0, 0)),
        ("upperarm_r",  16, (0.5, 0.15, 0.3)),
        ("forearm_r",   16, (0.3, 0, 0.1)),
        ("hand_r",      16, (0.15, 0, 0)),
        ("upperarm_l",  16, (-0.08, 0, -0.06)),
        ("tail_01",     16, (0.10, 0, 0.06)),
        ("tail_02",     16, (0.08, 0, 0.04)),
    ]

    # Frame 22: impact hold
    keys += [
        ("pelvis",      22, (0.05, 0, 0)),
        ("spine_01",    22, (0.12, 0, 0)),
        ("spine_02",    22, (0.10, 0, 0)),
        ("chest",       22, (0.08, 0, 0)),
        ("head",        22, (0.06, 0, 0)),
        ("jaw",         22, (0.10, 0, 0)),
        ("upperarm_r",  22, (0.4, 0.1, 0.2)),
        ("forearm_r",   22, (0.2, 0, 0.05)),
    ]

    # Frame 45: recovery to neutral
    for b in neutral_bones:
        keys.append((b, 45, (0, 0, 0)))

    return create_action(arm, "Attack", 45, keys)


def make_roar(arm):
    keys = []

    # Frame 1: neutral
    all_bones = ["spine_01", "spine_02", "chest", "head", "neck", "jaw",
                 "upperarm_l", "forearm_l", "upperarm_r", "forearm_r",
                 "pelvis", "tail_01", "tail_02", "tail_03"]
    for b in all_bones:
        keys.append((b, 1, (0, 0, 0)))

    # Frame 8: crouch (anticipation)
    keys += [
        ("pelvis",   8, (0.06, 0, 0)),
        ("spine_01", 8, (0.10, 0, 0)),
        ("spine_02", 8, (0.08, 0, 0)),
        ("head",     8, (0.12, 0, 0)),
        ("neck",     8, (0.06, 0, 0)),
        ("jaw",      8, (0, 0, 0)),
    ]

    # Frame 16: ROAR — head thrown back, jaw open, arms wide
    keys += [
        ("pelvis",      16, (-0.06, 0, 0)),
        ("spine_01",    16, (-0.14, 0, 0)),
        ("spine_02",    16, (-0.12, 0, 0)),
        ("chest",       16, (-0.10, 0, 0)),
        ("neck",        16, (-0.15, 0, 0)),
        ("head",        16, (-0.22, 0, 0)),
        ("jaw",         16, (0.28, 0, 0)),
        ("upperarm_l",  16, (-0.15, 0, 0.45)),
        ("forearm_l",   16, (-0.10, 0, 0.15)),
        ("upperarm_r",  16, (-0.15, 0, -0.45)),
        ("forearm_r",   16, (-0.10, 0, -0.15)),
        ("tail_01",     16, (-0.12, 0, 0)),
        ("tail_02",     16, (-0.10, 0, 0)),
        ("tail_03",     16, (-0.08, 0, 0)),
    ]

    # Frames 22-38: sustain roar with vibration
    for f in range(20, 40, 4):
        vibrate = 0.03 * (1 if (f // 4) % 2 == 0 else -1)
        keys += [
            ("head", f, (-0.22 + vibrate, vibrate * 0.5, vibrate)),
            ("jaw",  f, (0.26 + abs(vibrate), 0, 0)),
        ]

    # Frame 50-60: recovery
    for b in all_bones:
        keys.append((b, 52, (0, 0, 0)))

    return create_action(arm, "Roar", 60, keys)


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

    print("  Building mesh...")
    mesh_obj = build_monster_mesh()

    tex_path = os.path.join(OUTPUT_DIR, "T_Monster.png")
    print("  Baking texture (256x256)...")
    bake_texture(mesh_obj, tex_path, size=256)
    print(f"  Exported: {tex_path}")

    print("  Building armature...")
    arm_obj = build_armature()

    print("  Skinning (automatic weights)...")
    skin_mesh(mesh_obj, arm_obj)

    print("  Grounding rig...")
    ground_rig(mesh_obj, arm_obj)

    print("  Creating animations...")
    actions = [
        make_idle(arm_obj),
        make_attack(arm_obj),
        make_roar(arm_obj),
    ]

    sk_path = os.path.join(OUTPUT_DIR, "SK_Monster.fbx")
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
