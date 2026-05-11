"""
Procedural 3D mesh generator for UE5 placeholder assets.

Generates meshes in Blender and exports as FBX with UE conventions.

Usage (run via Blender):
    blender --background --python generate_meshes.py -- [output_dir] [mesh_filter]

    output_dir  — directory for FBX output (default: current directory)
    mesh_filter — comma-separated names to generate (default: all)

Output: SM_<Name>.fbx per mesh.
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
MESH_FILTER = args[1].split(",") if len(args) > 1 else None

Vector = mathutils.Vector


# ---------------------------------------------------------------------------
# Utilities
# ---------------------------------------------------------------------------

def clear_scene():
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=True)
    for block in [bpy.data.meshes, bpy.data.materials, bpy.data.metaballs]:
        for item in list(block):
            block.remove(item)


def mat(name, color, metallic=0.0, roughness=0.5, emission=0.0):
    m = bpy.data.materials.new(name)
    m.use_nodes = True
    bsdf = m.node_tree.nodes["Principled BSDF"]
    bsdf.inputs["Base Color"].default_value = (*color, 1.0)
    bsdf.inputs["Metallic"].default_value = metallic
    bsdf.inputs["Roughness"].default_value = roughness
    if emission > 0:
        if "Emission Color" in bsdf.inputs:
            bsdf.inputs["Emission Color"].default_value = (*color, 1.0)
        if "Emission Strength" in bsdf.inputs:
            bsdf.inputs["Emission Strength"].default_value = emission
    return m


def _sel(obj):
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj


def _apply(obj):
    _sel(obj)
    for mod in list(obj.modifiers):
        try:
            bpy.ops.object.modifier_apply(modifier=mod.name)
        except Exception:
            obj.modifiers.remove(mod)
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)


def join(parts):
    if not parts:
        return None
    for p in parts:
        _apply(p)
    bpy.ops.object.select_all(action='DESELECT')
    for p in parts:
        p.select_set(True)
    bpy.context.view_layer.objects.active = parts[0]
    if len(parts) > 1:
        bpy.ops.object.join()
    return bpy.context.active_object


def finalize(obj, name, material):
    obj.name = name
    # Single material for the whole mesh
    obj.data.materials.clear()
    obj.data.materials.append(material)
    _sel(obj)
    bpy.ops.object.shade_smooth()
    # Move lowest vertex to Z=0
    min_z = min(v.co.z for v in obj.data.vertices)
    if abs(min_z) > 0.0001:
        for v in obj.data.vertices:
            v.co.z -= min_z
    # UV unwrap
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.uv.smart_project(angle_limit=1.15)
    bpy.ops.object.mode_set(mode='OBJECT')


def export_fbx(obj, filepath):
    _sel(obj)
    bpy.ops.export_scene.fbx(
        filepath=filepath,
        use_selection=True,
        apply_unit_scale=True,
        apply_scale_options='FBX_SCALE_ALL',
        axis_forward='-Y',
        axis_up='Z',
        mesh_smooth_type='FACE',
        use_mesh_modifiers=True,
        object_types={'MESH'},
    )


# ---------------------------------------------------------------------------
# Primitive helpers
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


def cyl(loc, radius, depth, rot=(0, 0, 0), segs=12):
    bpy.ops.mesh.primitive_cylinder_add(
        vertices=segs, radius=radius, depth=depth, location=loc, rotation=rot)
    return bpy.context.active_object


def sphere(loc, radius, segs=16, rings=8):
    bpy.ops.mesh.primitive_uv_sphere_add(
        segments=segs, ring_count=rings, radius=radius, location=loc)
    return bpy.context.active_object


def cone(loc, r1, r2, depth, rot=(0, 0, 0), segs=12):
    bpy.ops.mesh.primitive_cone_add(
        vertices=segs, radius1=r1, radius2=r2, depth=depth, location=loc, rotation=rot)
    return bpy.context.active_object


def ico(loc, radius, subdiv=1):
    bpy.ops.mesh.primitive_ico_sphere_add(
        subdivisions=subdiv, radius=radius, location=loc)
    return bpy.context.active_object


# ---------------------------------------------------------------------------
# Character — Dark Fantasy Knight (~1.8m)
# ---------------------------------------------------------------------------

def make_character():
    random.seed(1)
    single_mat = mat("M_Character", (0.5, 0.5, 0.5), roughness=0.5)
    P = []

    # ---- Legs ----
    for s in (-1, 1):
        x = s * 0.15
        P.append(cube((x, 0.02, 0.06), (0.12, 0.20, 0.12), bevel=0.012))
        P.append(cyl((x, 0, 0.28), 0.065, 0.30))
        P.append(cube((x, -0.035, 0.48), (0.09, 0.07, 0.10), bevel=0.008))
        P.append(cyl((x, 0, 0.68), 0.06, 0.28))

    # ---- Waist ----
    P.append(cube((0, 0, 0.87), (0.40, 0.22, 0.07), bevel=0.008))
    P.append(cube((0, -0.12, 0.87), (0.07, 0.02, 0.07), bevel=0.004))
    for s in (-1, 1):
        P.append(cube((s * 0.13, -0.02, 0.76), (0.13, 0.04, 0.16), bevel=0.008))

    # ---- Torso ----
    P.append(cube((0, 0, 1.10), (0.40, 0.22, 0.40), bevel=0.018))
    P.append(cube((0, -0.07, 1.12), (0.34, 0.05, 0.32), bevel=0.012))
    P.append(cube((0, -0.10, 1.12), (0.04, 0.02, 0.28), bevel=0.004))

    # ---- Pauldrons ----
    for s in (-1, 1):
        x = s * 0.28
        p = cube((x, 0, 1.34), (0.18, 0.14, 0.10), bevel=0.01)
        p.rotation_euler.y = s * 0.15
        P.append(p)
        P.append(cone((x, 0, 1.42), 0.03, 0.0, 0.10))
        P.append(cube((x, 0, 1.29), (0.16, 0.12, 0.02), bevel=0.004))

    # ---- Arms ----
    for s in (-1, 1):
        x = s * 0.32
        P.append(cyl((x, 0, 1.15), 0.045, 0.22))
        P.append(sphere((x, 0, 1.01), 0.04))
        P.append(cyl((x, 0, 0.88), 0.05, 0.18))
        P.append(cone((x, 0, 0.79), 0.06, 0.04, 0.06))
        P.append(cube((x, -0.01, 0.72), (0.04, 0.035, 0.06)))

    # ---- Neck & gorget ----
    P.append(cyl((0, 0, 1.33), 0.04, 0.04))
    P.append(cone((0, 0, 1.31), 0.10, 0.05, 0.06))

    # ---- Helmet ----
    h = sphere((0, 0, 1.46), 0.12)
    h.scale = (1, 0.92, 1.08)
    P.append(h)
    P.append(cube((0, -0.10, 1.44), (0.10, 0.04, 0.04), bevel=0.004))
    P.append(cube((0, -0.135, 1.45), (0.08, 0.008, 0.012)))
    P.append(cube((0, 0, 1.56), (0.02, 0.10, 0.10), bevel=0.004))
    P.append(cube((0, 0, 1.535), (0.13, 0.018, 0.018), bevel=0.003))

    # ---- Cape ----
    bpy.ops.mesh.primitive_plane_add(size=1, location=(0, 0.14, 0.92))
    cape = bpy.context.active_object
    cape.rotation_euler.x = math.pi / 2
    cape.scale = (0.50, 0.80, 1)
    _sel(cape)
    bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.mesh.subdivide(number_cuts=6)
    bpy.ops.object.mode_set(mode='OBJECT')
    for v in cape.data.vertices:
        z_t = (0.40 - v.co.z) / 0.80
        v.co.x *= 1 + z_t * 0.5
        v.co.y += z_t * 0.10
        v.co.y += math.sin(v.co.x * 10) * 0.012 * z_t
        v.co.x += math.sin(v.co.z * 6) * 0.008
    P.append(cape)

    # ---- Sword on back ----
    P.append(cube((0.10, 0.14, 1.24), (0.035, 0.012, 0.42), bevel=0.002))
    P.append(cube((0.10, 0.14, 0.99), (0.10, 0.018, 0.02), bevel=0.004))
    P.append(cyl((0.10, 0.14, 0.92), 0.014, 0.10))
    P.append(sphere((0.10, 0.14, 0.86), 0.02))

    result = join(P)
    finalize(result, "SM_Character", single_mat)
    return result


# ---------------------------------------------------------------------------
# Monster — Hulking Demon (~2.5m, metaball body)
# ---------------------------------------------------------------------------

def make_monster():
    random.seed(2)
    single_mat = mat("M_Monster", (0.5, 0.5, 0.5), roughness=0.5)
    P = []

    # ---- Organic body via metaballs ----
    mball = bpy.data.metaballs.new("DemonMeta")
    mball.resolution = 0.10
    mball.threshold = 0.6

    def mb(co, radius):
        e = mball.elements.new()
        e.co = co
        e.radius = radius
        e.stiffness = 2.0

    # Torso (hunched)
    mb((0, 0, 1.0), 0.55)
    mb((0, -0.15, 1.35), 0.40)
    mb((0, 0.08, 0.65), 0.45)

    # Head + jaw
    mb((0, -0.35, 1.55), 0.28)
    mb((0, -0.52, 1.48), 0.15)
    mb((0, -0.46, 1.38), 0.14)

    # Arms (long, powerful)
    for s in (-1, 1):
        mb((s * 0.50, -0.05, 1.15), 0.22)
        mb((s * 0.72, 0.02, 0.88), 0.17)
        mb((s * 0.88, 0.08, 0.62), 0.14)
        mb((s * 0.95, 0.12, 0.42), 0.12)

    # Legs (thick, short)
    for s in (-1, 1):
        mb((s * 0.22, 0.05, 0.50), 0.20)
        mb((s * 0.24, 0.10, 0.25), 0.14)
        mb((s * 0.26, 0.15, 0.08), 0.12)

    # Tail
    for i in range(6):
        t = i / 5
        mb((0, 0.35 + t * 0.6, 0.65 - t * 0.35), 0.10 - t * 0.012)

    body = bpy.data.objects.new("DemonBody", mball)
    bpy.context.collection.objects.link(body)
    _sel(body)
    bpy.ops.object.convert(target='MESH')
    body = bpy.context.active_object
    P.append(body)

    # ---- Curved horns ----
    for s in (-1, 1):
        pos = Vector((s * 0.12, -0.35, 1.72))
        d = Vector((s * 0.3, -0.15, 1.0)).normalized()
        for i in range(7):
            t = i / 7
            seg_len = 0.07
            r1 = 0.045 * (1 - t * 0.85)
            r2 = 0.045 * (1 - (t + 1 / 7) * 0.85)
            end = pos + d * seg_len
            mid = (pos + end) / 2
            seg = cone(tuple(mid), r1, r2, seg_len, segs=8)
            q = Vector((0, 0, 1)).rotation_difference(d)
            seg.rotation_mode = 'QUATERNION'
            seg.rotation_quaternion = q
            P.append(seg)
            pos = end
            d = (d + Vector((s * 0.06, 0.04, -0.05))).normalized()

    # ---- Eyes ----
    for s in (-1, 1):
        P.append(sphere((s * 0.10, -0.58, 1.55), 0.035, segs=12, rings=6))

    # ---- Back spines ----
    for i in range(8):
        t = i / 7
        z = 1.45 - t * 0.80
        y = 0.12 + t * 0.08
        h = 0.14 - t * 0.010
        P.append(cone((0, y, z), 0.03, 0.005, h, rot=(-0.5, 0, 0), segs=6))

    # ---- Claws ----
    for s in (-1, 1):
        for i in range(4):
            cx = s * 0.95 + s * (i - 1.5) * 0.035
            P.append(cone((cx, 0.06, 0.33 - i * 0.015), 0.014, 0.003, 0.10,
                          rot=(0.7, 0, s * 0.08 * (i - 1.5)), segs=6))

    # Foot claws
    for s in (-1, 1):
        for i in range(3):
            fx = s * 0.26 + (i - 1) * 0.04
            P.append(cone((fx, 0.08, 0.0), 0.012, 0.003, 0.07,
                          rot=(0.8, 0, 0), segs=6))

    # ---- Tusks ----
    for s in (-1, 1):
        P.append(cone((s * 0.07, -0.60, 1.38), 0.018, 0.004, 0.10,
                       rot=(0.2, 0, s * 0.1), segs=6))

    # ---- Tail spike ----
    P.append(cone((0, 0.95, 0.30), 0.04, 0.005, 0.18,
                   rot=(-1.0, 0, 0), segs=6))

    # ---- Brow ridges ----
    for s in (-1, 1):
        brow = cube((s * 0.10, -0.52, 1.62), (0.10, 0.05, 0.03), bevel=0.008)
        brow.rotation_euler = (0.2, s * 0.15, 0)
        P.append(brow)

    result = join(P)
    finalize(result, "SM_Monster", single_mat)
    return result


# ---------------------------------------------------------------------------
# Tree — Gnarled Ancient Oak (~6m, recursive branching)
# ---------------------------------------------------------------------------

def make_tree():
    random.seed(3)
    single_mat = mat("M_Tree", (0.5, 0.5, 0.5), roughness=0.5)
    P = []

    def branch(origin, direction, length, radius, depth, max_depth):
        end = origin + direction.normalized() * length
        mid = (origin + end) / 2
        tip_r = radius * 0.60

        seg = cone(tuple(mid), radius, tip_r, length, segs=8)

        q = Vector((0, 0, 1)).rotation_difference(direction.normalized())
        seg.rotation_mode = 'QUATERNION'
        seg.rotation_quaternion = q
        _sel(seg)
        bpy.ops.object.transform_apply(rotation=True)

        noise_s = radius * 0.15
        for v in seg.data.vertices:
            v.co.x += random.uniform(-noise_s, noise_s)
            v.co.y += random.uniform(-noise_s, noise_s)
        P.append(seg)

        if depth < max_depth:
            n_sub = random.randint(2, 3) if depth < 2 else random.randint(1, 3)
            for _ in range(n_sub):
                spread = 0.35 + depth * 0.18
                nd = Vector((
                    direction.x + random.uniform(-spread, spread),
                    direction.y + random.uniform(-spread, spread),
                    direction.z + random.uniform(0.05, 0.45),
                )).normalized()
                nl = length * random.uniform(0.50, 0.72)
                nr = radius * random.uniform(0.40, 0.62)
                branch(end, nd, nl, nr, depth + 1, max_depth)
        else:
            for _ in range(random.randint(3, 5)):
                lp = end + Vector((
                    random.uniform(-0.35, 0.35),
                    random.uniform(-0.35, 0.35),
                    random.uniform(-0.15, 0.40),
                ))
                lf = ico(tuple(lp), random.uniform(0.18, 0.38), subdiv=1)
                lf.scale.z = random.uniform(0.45, 0.75)
                P.append(lf)

    # ---- Trunk ----
    trunk_h = 2.8
    trunk_r = 0.30
    for i in range(4):
        t = i / 4
        seg_h = trunk_h / 4
        r_bot = trunk_r * (1 - t * 0.35)
        r_top = trunk_r * (1 - (t + 0.25) * 0.35)
        z_mid = t * trunk_h + seg_h / 2
        lean_x = math.sin(t * 1.2) * 0.06
        lean_y = math.cos(t * 0.8) * 0.04

        seg = cone((lean_x, lean_y, z_mid), r_bot, r_top, seg_h, segs=12)
        _sel(seg)
        bpy.ops.object.transform_apply(scale=True)
        noise_s = r_bot * 0.10
        for v in seg.data.vertices:
            v.co.x += random.uniform(-noise_s, noise_s)
            v.co.y += random.uniform(-noise_s, noise_s)
        P.append(seg)

    # ---- Roots ----
    for i in range(6):
        angle = i * 2 * math.pi / 6 + random.uniform(-0.25, 0.25)
        rd = Vector((math.cos(angle), math.sin(angle), -0.35)).normalized()
        rl = random.uniform(0.5, 0.9)
        rr = random.uniform(0.06, 0.11)
        rmid = Vector((0, 0, 0.12)) + rd * rl / 2

        seg = cone(tuple(rmid), rr, rr * 0.25, rl, segs=6)
        q = Vector((0, 0, 1)).rotation_difference(rd)
        seg.rotation_mode = 'QUATERNION'
        seg.rotation_quaternion = q
        P.append(seg)

    # ---- Main branches from trunk top ----
    trunk_top = Vector((0.06, 0.04, trunk_h))
    n_main = random.randint(4, 5)
    for i in range(n_main):
        angle = i * 2 * math.pi / n_main + random.uniform(-0.25, 0.25)
        bd = Vector((
            math.cos(angle) * 0.55,
            math.sin(angle) * 0.55,
            random.uniform(0.55, 0.85),
        )).normalized()
        bl = random.uniform(1.2, 1.9)
        br = random.uniform(0.07, 0.13)
        branch(trunk_top, bd, bl, br, 1, 3)

    # ---- Trunk knots ----
    for _ in range(5):
        angle = random.uniform(0, 2 * math.pi)
        z = random.uniform(0.4, 2.2)
        r = trunk_r * 0.65
        kp = (math.cos(angle) * r, math.sin(angle) * r, z)
        k = sphere(kp, random.uniform(0.04, 0.09), segs=8, rings=4)
        k.scale = (0.8, 0.8, random.uniform(0.5, 1.3))
        P.append(k)

    # ---- Hanging moss strands ----
    for _ in range(8):
        angle = random.uniform(0, 2 * math.pi)
        dist = random.uniform(0.4, 1.8)
        z = random.uniform(3.2, 5.5)
        ml = random.uniform(0.25, 0.70)
        P.append(cyl((math.cos(angle) * dist, math.sin(angle) * dist, z - ml / 2),
                      0.008, ml, segs=4))

    result = join(P)
    finalize(result, "SM_Tree", single_mat)
    return result


# ---------------------------------------------------------------------------
# Crate — Simple box (~70cm, beveled edges)
# ---------------------------------------------------------------------------

def make_crate():
    single_mat = mat("M_Crate", (0.5, 0.5, 0.5), roughness=0.5)
    body = cube((0, 0, 0), (0.35, 0.35, 0.35), bevel=0.01)
    result = join([body])
    finalize(result, "SM_Crate", single_mat)
    return result


# ---------------------------------------------------------------------------
# Wumpa Fruit — Mango/apple shape (~20cm)
# ---------------------------------------------------------------------------

def make_wumpa():
    single_mat = mat("M_Wumpa", (0.5, 0.5, 0.5), roughness=0.5)
    P = []

    # Main body — slightly tall sphere, pinched at bottom
    bpy.ops.mesh.primitive_uv_sphere_add(segments=16, ring_count=12, radius=0.09, location=(0, 0, 0.09))
    body = bpy.context.active_object
    body.scale = (1.0, 1.0, 1.15)
    _sel(body)
    bpy.ops.object.transform_apply(scale=True)
    # Taper bottom half for mango/apple shape
    for v in body.data.vertices:
        if v.co.z < 0.09:
            t = (0.09 - v.co.z) / 0.09
            shrink = 1.0 - t * 0.35
            v.co.x *= shrink
            v.co.y *= shrink
    # Slight dimple at top
    for v in body.data.vertices:
        if v.co.z > 0.18:
            t = (v.co.z - 0.18) / 0.04
            v.co.z -= t * 0.008
    P.append(body)

    # Stem
    P.append(cyl((0, 0, 0.21), 0.006, 0.04, segs=6))

    # Two small leaves at stem
    for s in (-1, 1):
        bpy.ops.mesh.primitive_plane_add(size=0.05, location=(s * 0.02, 0, 0.22))
        leaf = bpy.context.active_object
        leaf.scale = (0.6, 1.0, 1.0)
        leaf.rotation_euler = (0.4 * s, 0.3, s * 0.5)
        P.append(leaf)

    result = join(P)
    finalize(result, "SM_WumpaFruit", single_mat)
    return result


# ---------------------------------------------------------------------------
# Aku Aku Mask — Tiki mask (~40cm tall)
# ---------------------------------------------------------------------------

def make_akuaku():
    single_mat = mat("M_AkuAku", (0.5, 0.5, 0.5), roughness=0.5)
    P = []

    # Main mask face — flattened elongated hexagon shape
    bpy.ops.mesh.primitive_uv_sphere_add(segments=12, ring_count=8, radius=0.15, location=(0, 0, 0.20))
    face = bpy.context.active_object
    face.scale = (1.0, 0.35, 1.3)
    _sel(face)
    bpy.ops.object.transform_apply(scale=True)
    # Flatten the back
    for v in face.data.vertices:
        if v.co.y > 0.02:
            v.co.y = 0.02
    # Widen the cheeks, narrow the chin and forehead
    for v in face.data.vertices:
        z = v.co.z - 0.20
        if z < -0.08:
            t = (-0.08 - z) / 0.12
            v.co.x *= 1.0 - t * 0.4
        elif z > 0.10:
            t = (z - 0.10) / 0.10
            v.co.x *= 1.0 - t * 0.25
    P.append(face)

    # Brow ridge
    P.append(cube((0, -0.04, 0.32), (0.13, 0.025, 0.02), bevel=0.005))

    # Eye sockets (recessed areas)
    for s in (-1, 1):
        eye_socket = sphere((s * 0.055, -0.045, 0.28), 0.025, segs=8, rings=6)
        P.append(eye_socket)

    # Eyeballs (slightly protruding)
    for s in (-1, 1):
        P.append(sphere((s * 0.055, -0.055, 0.28), 0.018, segs=8, rings=6))

    # Nose — triangular wedge
    P.append(cone((0, -0.06, 0.22), 0.02, 0.008, 0.05, rot=(0.3, 0, 0), segs=4))

    # Mouth — wide open rectangle with teeth
    P.append(cube((0, -0.05, 0.12), (0.08, 0.02, 0.025), bevel=0.004))

    # Upper teeth
    for i in range(5):
        x = -0.05 + i * 0.025
        P.append(cube((x, -0.06, 0.135), (0.008, 0.012, 0.012)))

    # Lower teeth
    for i in range(5):
        x = -0.05 + i * 0.025
        P.append(cube((x, -0.06, 0.105), (0.008, 0.012, 0.012)))

    # Feathers on top (3 tall feather shapes)
    for i, s in enumerate((-1, 0, 1)):
        x = s * 0.04
        h = 0.14 + (1 - abs(s)) * 0.06
        feather = cube((x, 0, 0.40 + h / 2), (0.015, 0.005, h / 2), bevel=0.003)
        feather.rotation_euler.y = s * 0.15
        P.append(feather)
        # Feather tip
        tip = cone((x + s * 0.01, 0, 0.40 + h), 0.015, 0.003, 0.03, segs=6)
        tip.rotation_euler.y = s * 0.15
        P.append(tip)

    # Side decorations — small tiki ear wings
    for s in (-1, 1):
        P.append(cube((s * 0.14, -0.01, 0.25), (0.025, 0.015, 0.05), bevel=0.004))
        P.append(cube((s * 0.16, -0.01, 0.22), (0.015, 0.01, 0.03), bevel=0.003))

    # Chin beard / bottom decoration
    P.append(cone((0, -0.03, 0.06), 0.035, 0.015, 0.06, segs=6))

    result = join(P)
    # Rotate so face points +X (UE forward) instead of -Y (Blender front)
    rot = mathutils.Matrix.Rotation(math.pi / 2, 4, 'Z')
    for v in result.data.vertices:
        v.co = rot @ v.co
    finalize(result, "SM_AkuAku", single_mat)
    return result


# ---------------------------------------------------------------------------
# Registry & main
# ---------------------------------------------------------------------------

def make_spear():
    random.seed(20)
    single_mat = mat("M_Spear", (0.4, 0.3, 0.2), roughness=0.7)
    P = []
    P.append(cyl((0, 0, 0.5), 0.015, 0.9))
    P.append(cone((0, 0, 0.95), 0.025, 0.003, 0.12, segs=8))
    result = join(P)
    finalize(result, "SM_Spear", single_mat)
    return result


def make_beaker():
    random.seed(21)
    single_mat = mat("M_Beaker", (0.6, 0.15, 0.1), roughness=0.3)
    P = []
    P.append(cyl((0, 0, 0.06), 0.035, 0.1))
    P.append(sphere((0, 0, 0.14), 0.03, segs=10, rings=8))
    result = join(P)
    finalize(result, "SM_Beaker", single_mat)
    return result


def make_launched_enemy():
    random.seed(22)
    single_mat = mat("M_LaunchedEnemy", (0.9, 0.1, 0.1), roughness=0.4)
    P = []
    P.append(cyl((0, 0, 0.15), 0.06, 0.25, rot=(math.pi / 6, 0, 0)))
    P.append(sphere((0, 0, 0.28), 0.06, segs=10, rings=8))
    result = join(P)
    finalize(result, "SM_LaunchedEnemy", single_mat)
    return result


def make_flying_fish():
    random.seed(23)
    single_mat = mat("M_FlyingFish", (0.5, 0.6, 0.8), metallic=0.3, roughness=0.3)
    P = []
    P.append(sphere((0, 0, 0.15), 0.15, segs=12, rings=8))
    P.append(cube((0, 0, 0.15), (0.08, 0.30, 0.08), bevel=0.015))
    P.append(cone((0, -0.25, 0.15), 0.08, 0.02, 0.12, segs=6))
    for s in (-1, 1):
        P.append(cube((s * 0.18, 0.04, 0.18), (0.12, 0.08, 0.02), bevel=0.005))
    result = join(P)
    finalize(result, "SM_FlyingFish", single_mat)
    return result


def make_barrel():
    random.seed(24)
    single_mat = mat("M_Barrel", (0.45, 0.3, 0.15), roughness=0.8)
    P = []
    P.append(cyl((0, 0, 0.45), 0.22, 0.85))
    P.append(cyl((0, 0, 0.12), 0.235, 0.06))
    P.append(cyl((0, 0, 0.78), 0.235, 0.06))
    result = join(P)
    finalize(result, "SM_Barrel", single_mat)
    return result


def make_invisible_platform():
    random.seed(25)
    single_mat = mat("M_InvisiblePlatform", (0.3, 0.3, 0.3), roughness=0.5)
    P = []
    P.append(cyl((0, 0, 0.02), 0.2, 0.03))
    result = join(P)
    finalize(result, "SM_InvisiblePlatform", single_mat)
    return result


def make_snake_hole():
    random.seed(26)
    single_mat = mat("M_SnakeHole", (0.1, 0.08, 0.05), roughness=0.9)
    P = []
    P.append(cyl((0, 0, 0.01), 0.12, 0.02))
    result = join(P)
    finalize(result, "SM_SnakeHole", single_mat)
    return result


def make_spider_thread():
    random.seed(27)
    single_mat = mat("M_SpiderThread", (0.9, 0.9, 0.9), roughness=0.3)
    P = []
    P.append(cyl((0, 0, 0.5), 0.005, 1.0))
    result = join(P)
    finalize(result, "SM_SpiderThread", single_mat)
    return result


MESHES = {
    "Character": make_character,
    "Monster": make_monster,
    "Tree": make_tree,
    "Crate": make_crate,
    "WumpaFruit": make_wumpa,
    "AkuAku": make_akuaku,
    "Spear": make_spear,
    "Beaker": make_beaker,
    "LaunchedEnemy": make_launched_enemy,
    "FlyingFish": make_flying_fish,
    "Barrel": make_barrel,
    "InvisiblePlatform": make_invisible_platform,
    "SnakeHole": make_snake_hole,
    "SpiderThread": make_spider_thread,
}


def main():
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    for name, generator in MESHES.items():
        if MESH_FILTER and name not in MESH_FILTER:
            continue

        clear_scene()
        obj = generator()
        filepath = os.path.join(OUTPUT_DIR, f"SM_{name}.fbx")
        export_fbx(obj, filepath)
        print(f"  Exported: {filepath}")

    print("Done.")


main()
