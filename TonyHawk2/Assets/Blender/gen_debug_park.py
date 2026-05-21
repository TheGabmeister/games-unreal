"""
Generate debug skatepark FBX for Tony Hawk's Pro Skater 2 (Phase 1).
Run: blender --background --python gen_debug_park.py

Produces: debug_park.fbx in the same directory as this script.

Pieces:
  - Ground plane (100m x 100m)
  - 2 quarter pipes (3m tall, facing each other)
  - 1 halfpipe / vert ramp (4m tall, 15m wide)
  - 2 kicker ramps (1.5m tall, 30° and 45°)
  - 2 flat banks (gentle slopes)
  - Boundary walls (low perimeter walls)
"""

import bpy
import bmesh
import math
import os

OUT_DIR = os.path.dirname(os.path.abspath(__file__))
OUT_PATH = os.path.join(OUT_DIR, "debug_park.fbx")

# ---- Helpers ----

def clear_scene():
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=False)
    for c in bpy.data.collections:
        if c.name != "Scene Collection":
            bpy.data.collections.remove(c)

def make_material(name, color):
    mat = bpy.data.materials.new(name)
    mat.use_nodes = True
    bsdf = mat.node_tree.nodes.get("Principled BSDF")
    if bsdf:
        bsdf.inputs["Base Color"].default_value = color
        bsdf.inputs["Roughness"].default_value = 0.8
    return mat

def set_origin_to_bottom(obj):
    """Move origin to the bottom of the mesh bounding box."""
    bbox = [obj.matrix_world @ bpy.app.driver_namespace.get('_', __import__('mathutils').Vector(v)) if False else __import__('mathutils').Vector(v) for v in obj.bound_box]
    min_z = min(v.z for v in bbox)
    offset = __import__('mathutils').Vector((0, 0, -min_z))
    for v in obj.data.vertices:
        v.co += offset
    obj.location.z += min_z

def create_curved_ramp(name, width, height, depth, segments, vert_extension=0.0):
    """Create a quarter-pipe shaped ramp with curved transition."""
    mesh = bpy.data.meshes.new(name)
    bm = bmesh.new()

    half_w = width / 2.0
    verts_left = []
    verts_right = []

    for i in range(segments + 1):
        t = i / segments
        angle = t * math.pi / 2.0
        x = depth * math.sin(angle)
        z = height * (1.0 - math.cos(angle))
        # Add vert extension (straight vertical section at top)
        if t > 0.95 and vert_extension > 0:
            extra = (t - 0.95) / 0.05 * vert_extension
            z += extra

        verts_left.append(bm.verts.new((-half_w, -x, z)))
        verts_right.append(bm.verts.new((half_w, -x, z)))

    bm.verts.ensure_lookup_table()

    for i in range(segments):
        face_verts = [verts_left[i], verts_right[i], verts_right[i+1], verts_left[i+1]]
        bm.faces.new(face_verts)

    bm.to_mesh(mesh)
    bm.free()
    mesh.update()

    obj = bpy.data.objects.new(name, mesh)
    bpy.context.collection.objects.link(obj)
    return obj

# ---- Materials ----

mat_concrete = make_material("M_Concrete", (0.6, 0.6, 0.58, 1.0))
mat_asphalt = make_material("M_Asphalt", (0.25, 0.25, 0.27, 1.0))
mat_ramp = make_material("M_Ramp", (0.3, 0.45, 0.65, 1.0))
mat_wall = make_material("M_Wall", (0.4, 0.38, 0.35, 1.0))

# ---- Build Scene ----

clear_scene()

# 1. Ground plane (100m x 100m = 10000 x 10000 cm in UE, but Blender uses meters)
bpy.ops.mesh.primitive_plane_add(size=100, location=(0, 0, 0))
ground = bpy.context.active_object
ground.name = "SM_Ground"
ground.data.materials.append(mat_concrete)

# 2. Quarter pipe A (3m tall, 4m deep, 6m wide) — facing +Y
qp_a = create_curved_ramp("SM_QuarterPipeA", width=6, height=3, depth=4, segments=16)
qp_a.location = (15, -20, 0)
qp_a.rotation_euler = (0, 0, math.pi)  # face toward center
qp_a.data.materials.append(mat_ramp)

# Quarter pipe B — facing -Y (opposite side)
qp_b = create_curved_ramp("SM_QuarterPipeB", width=6, height=3, depth=4, segments=16)
qp_b.location = (15, 20, 0)
qp_b.rotation_euler = (0, 0, 0)
qp_b.data.materials.append(mat_ramp)

# 3. Halfpipe / vert ramp (4m tall, 15m wide, two quarter pipes connected by flat bottom)
# Left wall of halfpipe
hp_left = create_curved_ramp("SM_HalfpipeLeft", width=15, height=4, depth=5, segments=20, vert_extension=0.5)
hp_left.location = (-20, -5, 0)
hp_left.rotation_euler = (0, 0, math.pi)
hp_left.data.materials.append(mat_ramp)

# Right wall of halfpipe
hp_right = create_curved_ramp("SM_HalfpipeRight", width=15, height=4, depth=5, segments=20, vert_extension=0.5)
hp_right.location = (-20, 5, 0)
hp_right.rotation_euler = (0, 0, 0)
hp_right.data.materials.append(mat_ramp)

# Halfpipe flat bottom
bpy.ops.mesh.primitive_plane_add(size=1, location=(-20, 0, 0))
hp_floor = bpy.context.active_object
hp_floor.name = "SM_HalfpipeFloor"
hp_floor.scale = (7.5, 5, 1)
hp_floor.data.materials.append(mat_asphalt)

# 4. Kicker ramps
# Kicker A — 30 degree, 1.5m tall
bpy.ops.mesh.primitive_cube_add(size=1, location=(0, -15, 0.75))
kicker_a = bpy.context.active_object
kicker_a.name = "SM_KickerA"
kicker_a.scale = (2, 2.6, 0.75)
kicker_a.rotation_euler = (math.radians(-30), 0, 0)
kicker_a.location = (0, -15, 0)
kicker_a.data.materials.append(mat_ramp)

# Kicker B — 45 degree, 1.5m tall
bpy.ops.mesh.primitive_cube_add(size=1, location=(10, -15, 0.75))
kicker_b = bpy.context.active_object
kicker_b.name = "SM_KickerB"
kicker_b.scale = (2, 2.1, 0.75)
kicker_b.rotation_euler = (math.radians(-45), 0, 0)
kicker_b.location = (10, -15, 0)
kicker_b.data.materials.append(mat_ramp)

# 5. Flat banks (gentle slopes)
bpy.ops.mesh.primitive_cube_add(size=1, location=(25, 0, 0.5))
bank_a = bpy.context.active_object
bank_a.name = "SM_BankA"
bank_a.scale = (4, 6, 0.5)
bank_a.rotation_euler = (math.radians(-15), 0, 0)
bank_a.location = (25, -10, 0)
bank_a.data.materials.append(mat_asphalt)

bpy.ops.mesh.primitive_cube_add(size=1, location=(25, 10, 0.5))
bank_b = bpy.context.active_object
bank_b.name = "SM_BankB"
bank_b.scale = (4, 6, 0.5)
bank_b.rotation_euler = (math.radians(15), 0, 0)
bank_b.location = (25, 10, 0)
bank_b.data.materials.append(mat_asphalt)

# 6. Boundary walls (low walls around perimeter)
wall_height = 2.0
wall_thickness = 0.3
half_ground = 50.0

wall_positions = [
    ("SM_WallNorth", (0, half_ground, wall_height/2), (half_ground, wall_thickness, wall_height/2)),
    ("SM_WallSouth", (0, -half_ground, wall_height/2), (half_ground, wall_thickness, wall_height/2)),
    ("SM_WallEast", (half_ground, 0, wall_height/2), (wall_thickness, half_ground, wall_height/2)),
    ("SM_WallWest", (-half_ground, 0, wall_height/2), (wall_thickness, half_ground, wall_height/2)),
]

for wname, wloc, wscale in wall_positions:
    bpy.ops.mesh.primitive_cube_add(size=1, location=wloc)
    wall = bpy.context.active_object
    wall.name = wname
    wall.scale = wscale
    wall.data.materials.append(mat_wall)

# ---- Export ----

bpy.ops.object.select_all(action='SELECT')
bpy.ops.export_scene.fbx(
    filepath=OUT_PATH,
    use_selection=True,
    global_scale=1.0,
    apply_unit_scale=True,
    apply_scale_options='FBX_SCALE_ALL',
    axis_forward='-Y',
    axis_up='Z',
    mesh_smooth_type='OFF',
    use_mesh_modifiers=True,
    add_leaf_bones=False,
)

print(f"Exported debug park to: {OUT_PATH}")
