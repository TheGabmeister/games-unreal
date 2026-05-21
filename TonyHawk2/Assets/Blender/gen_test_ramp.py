import bpy
import os

bpy.ops.wm.read_factory_settings(use_empty=True)

mesh = bpy.data.meshes.new("TestRamp")
obj = bpy.data.objects.new("TestRamp", mesh)
bpy.context.collection.objects.link(obj)

# Wedge/ramp shape: ~4m long, 2m wide, 1.5m tall
# Blender units are meters; FBX export converts to cm for UE
w = 1.0   # half-width (1m each side = 2m total)
l = 2.0   # half-length (2m each side = 4m total)
h = 1.5   # height at back (1.5m)

verts = [
    (-w, -l, 0),   # 0 front-left bottom
    ( w, -l, 0),   # 1 front-right bottom
    ( w,  l, 0),   # 2 back-right bottom
    (-w,  l, 0),   # 3 back-left bottom
    ( w,  l, h),   # 4 back-right top
    (-w,  l, h),   # 5 back-left top
]

faces = [
    (0, 1, 2, 3),  # bottom
    (3, 2, 4, 5),  # back wall
    (0, 3, 5),     # left triangle
    (1, 4, 2),     # right triangle (wound correctly)
    (0, 5, 4, 1),  # ramp surface
]

mesh.from_pydata(verts, [], faces)
mesh.update()

# Simple gray material
mat = bpy.data.materials.new("M_TestRamp")
mat.diffuse_color = (0.5, 0.5, 0.5, 1.0)
obj.data.materials.append(mat)

# UV unwrap
bpy.context.view_layer.objects.active = obj
obj.select_set(True)
bpy.ops.object.mode_set(mode='EDIT')
bpy.ops.mesh.select_all(action='SELECT')
bpy.ops.uv.smart_project()
bpy.ops.mesh.normals_make_consistent(inside=False)
bpy.ops.object.mode_set(mode='OBJECT')

out_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "test_ramp.fbx")
bpy.ops.export_scene.fbx(
    filepath=out_path,
    use_selection=True,
    global_scale=1.0,
    apply_unit_scale=True,
    axis_forward='-Y',
    axis_up='Z',
)
print(f"Exported: {out_path}")
