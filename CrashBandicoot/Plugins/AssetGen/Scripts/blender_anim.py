"""Headless Blender script. Imports the UE mannequin FBX, authors a set of
locomotion animations procedurally, and exports each as an animation-only FBX
(skeleton + curves, no mesh / skin / materials).

These are programmer-art placeholders for wiring up locomotion state
machines — not shippable animations.

Bone-axis convention on this rig — validated by iteration AND by direct
inspection of matrix_local (see inspect_axes.py to dump bone-local axes for
any new rig):

  Spine, neck, head, pelvis (bones whose local Y runs along the bone):
    - X = lateral bend (sway / side tilt).
    - Y = twist (axial rotation around the bone's own length).
    - Z = forward/back pitch. Positive = forward fold; negative = arch
      back. Verified: spine_03 Z=+75° folds the chest fully forward
      over the legs (used in Death); head Z=+30° hangs the head limp.
    - Same L/R sign rule applies if you ever stack multiple bones.

  Upperarms:
    - X = up/down. X=70 puts arms hanging by sides; X=0 leaves the bind
      pose (~A-pose, arms angled out and down). X going negative does NOT
      keep raising — at X=-90 the arms land at horizontal/T-pose.
    - Z = forward/back swing. Z=-100 raises an arm overhead (forward and
      up). Z=+100 swings it backward.
    - L/R sign rule: same sign on both sides → same world direction
      (mirrored motion). Opposite signs → opposite directions
      (alternation, e.g. walk arm swing).

  Thighs and calves:
    - Z = forward/back leg swing. X is side-to-side spread (wrong axis
      for walking). Z=+90° on thighs swings each leg fully horizontal
      forward (used in Death to extend the legs in front of the
      seated character).
    - Same L/R sign rule as upperarms.

Pelvis translation for ground-anchored animations:
  pose_bone.location is in the BONE'S REST-LOCAL frame, not world space.
  For the pelvis on this rig, bone-local +X happens to point along
  armature +Z (world up), so a NEGATIVE cm value on location.x drops the
  whole rig downward. Verified: pelvis location.x = -90 with the rest-pose
  pelvis at armature Z ≈ 95.9 cm puts the character on the floor (used
  in Death). Translating the pelvis cascades to every child bone — no
  per-bone offsets needed. Units are real centimeters (~163 cm tall total),
  matching UE's native skeletal unit.

Debugging tips when porting this script to a new rig:
  - Run inspect_axes.py against the FBX first to confirm which bone-local
    axis runs along each bone (= twist) and which two are perpendicular
    bend axes — the assignment can vary by rig.
  - Validate new axes with LARGE rotations (60°+). Small magnitudes
    (3-10°) all look like subtle weight shifts whether the axis is
    lateral, pitch, or a tiny twist; that ambiguity hid an axis mislabel
    in this script's earlier life until a 75° spine fold made the truth
    obvious.

bake_anim_use_all_bones=True in export_anim_fbx is required: held poses
otherwise get simplified to zero curves and UE rejects the FBX with
"no data to import."

Run from project root:
  blender --background --python Plugins/AssetGen/Scripts/blender_anim.py \\
    -- <input.fbx> [output_dir] [anim_filter]

`anim_filter` is an optional comma-separated list (e.g. `Idle` or `Walk,Run`)
to regenerate only those animations. Omit it for the full set.

Produces in <output_dir> (default = current directory):
  A_Manny_Idle.fbx, A_Manny_Walk.fbx, A_Manny_Run.fbx,
  A_Manny_Jump.fbx, A_Manny_Falling.fbx, A_Manny_Death.fbx
"""
import sys
import os
import math
import bpy


def parse_args():
    if "--" not in sys.argv:
        raise SystemExit("Pass FBX path after `--`: ... -- <input.fbx> [output_dir] [anim_filter]")
    args = sys.argv[sys.argv.index("--") + 1:]
    if len(args) < 1:
        raise SystemExit("Need <input.fbx> [output_dir] [anim_filter]")
    in_fbx = args[0]
    out_dir = args[1] if len(args) > 1 else "."
    anim_filter = [s.strip() for s in args[2].split(",")] if len(args) > 2 else None
    return in_fbx, out_dir, anim_filter


def clear_scene():
    for obj in list(bpy.data.objects):
        bpy.data.objects.remove(obj, do_unlink=True)
    for col in (bpy.data.meshes, bpy.data.armatures, bpy.data.actions, bpy.data.materials):
        for item in list(col):
            col.remove(item)


def find_armature():
    for obj in bpy.data.objects:
        if obj.type == 'ARMATURE':
            return obj
    raise RuntimeError("No armature in imported FBX")


def strip_mesh():
    for obj in list(bpy.data.objects):
        if obj.type == 'MESH':
            bpy.data.objects.remove(obj, do_unlink=True)
    for mesh in list(bpy.data.meshes):
        bpy.data.meshes.remove(mesh)
    for mat in list(bpy.data.materials):
        bpy.data.materials.remove(mat)


def key_rot(pb, frame, xyz_deg):
    pb.rotation_mode = 'XYZ'
    pb.rotation_euler = tuple(math.radians(d) for d in xyz_deg)
    pb.keyframe_insert(data_path="rotation_euler", frame=frame)


def key_loc(pb, frame, xyz):
    pb.location = xyz
    pb.keyframe_insert(data_path="location", frame=frame)


def reset_action(armature, name):
    if not armature.animation_data:
        armature.animation_data_create()
    if armature.animation_data.action:
        bpy.data.actions.remove(armature.animation_data.action)
    armature.animation_data.action = bpy.data.actions.new(name=name)


def export_anim_fbx(out_path):
    bpy.ops.export_scene.fbx(
        filepath=out_path,
        use_selection=False,
        object_types={'ARMATURE'},
        bake_anim=True,
        # use_all_bones=True is required for UE — without it, held poses
        # (constant curves) get simplified out and the FBX exports with zero
        # animation data, which UE rejects as "no data to import."
        bake_anim_use_all_bones=True,
        bake_anim_use_nla_strips=False,
        bake_anim_use_all_actions=False,
        bake_anim_force_startend_keying=True,
        bake_anim_step=1.0,
        bake_anim_simplify_factor=1.0,
        add_leaf_bones=False,
        armature_nodetype='NULL',
        mesh_smooth_type='FACE',
        use_armature_deform_only=False,
        path_mode='AUTO',
    )


# ---- per-animation authoring ------------------------------------------------
# Each function receives:
#   pb(name) -> pose bone or None
#   scene    -> bpy.context.scene (set frame_start/frame_end here)
# Armature is already in POSE mode and the action is freshly empty.

def author_idle(pb, scene):
    """2-second idle loop. Arms hanging by sides with a slow weight-shift sway."""
    scene.frame_start = 1
    scene.frame_end = 60

    # Arms by sides: Y=20 rotates from A-pose to a natural resting position.
    if pb("upperarm_r"):
        key_rot(pb("upperarm_r"), 1,  (0, 20, 0))
        key_rot(pb("upperarm_r"), 60, (0, 20, 0))
    if pb("upperarm_l"):
        key_rot(pb("upperarm_l"), 1,  (0, 20, 0))
        key_rot(pb("upperarm_l"), 60, (0, 20, 0))

    # Slow weight-shift sway on the spine: neutral -> right -> neutral ->
    # left -> neutral (clean loop). Small magnitude so it reads as
    # "shifting," not "dancing."
    SWAY = 3.0
    if pb("spine_03"):
        key_rot(pb("spine_03"), 1,  (0, 0, 0))
        key_rot(pb("spine_03"), 15, (0, 0,  SWAY))
        key_rot(pb("spine_03"), 30, (0, 0, 0))
        key_rot(pb("spine_03"), 45, (0, 0, -SWAY))
        key_rot(pb("spine_03"), 60, (0, 0, 0))


def author_walk(pb, scene):
    """1-second 2-stride loop. 30 frames @ 30 fps."""
    scene.frame_start = 1
    scene.frame_end = 30

    THIGH = 25.0
    CALF_FRONT = 10.0
    CALF_BACK = 30.0
    ARM_SWING = 30.0

    if pb("thigh_r"):
        key_rot(pb("thigh_r"), 1,  (0, 0,  THIGH))
        key_rot(pb("thigh_r"), 15, (0, 0, -THIGH))
        key_rot(pb("thigh_r"), 30, (0, 0,  THIGH))
    if pb("calf_r"):
        key_rot(pb("calf_r"), 1,  (0, 0, -CALF_FRONT))
        key_rot(pb("calf_r"), 8,  (0, 0, 0))
        key_rot(pb("calf_r"), 22, (0, 0, -CALF_BACK))
        key_rot(pb("calf_r"), 30, (0, 0, -CALF_FRONT))

    if pb("thigh_l"):
        key_rot(pb("thigh_l"), 1,  (0, 0, -THIGH))
        key_rot(pb("thigh_l"), 15, (0, 0,  THIGH))
        key_rot(pb("thigh_l"), 30, (0, 0, -THIGH))
    if pb("calf_l"):
        key_rot(pb("calf_l"), 1,  (0, 0, -CALF_BACK))
        key_rot(pb("calf_l"), 7,  (0, 0, -CALF_FRONT))
        key_rot(pb("calf_l"), 15, (0, 0, 0))
        key_rot(pb("calf_l"), 30, (0, 0, -CALF_BACK))

    if pb("upperarm_r"):
        key_rot(pb("upperarm_r"), 1,  (0, 20, -ARM_SWING))
        key_rot(pb("upperarm_r"), 15, (0, 20,  ARM_SWING))
        key_rot(pb("upperarm_r"), 30, (0, 20, -ARM_SWING))
    if pb("upperarm_l"):
        key_rot(pb("upperarm_l"), 1,  (0, 20,  ARM_SWING))
        key_rot(pb("upperarm_l"), 15, (0, 20, -ARM_SWING))
        key_rot(pb("upperarm_l"), 30, (0, 20,  ARM_SWING))


def author_run(pb, scene):
    """0.8-second 2-stride loop, exaggerated walk. 24 frames @ 30 fps."""
    scene.frame_start = 1
    scene.frame_end = 24

    THIGH = 45.0
    CALF_FRONT = 25.0
    CALF_BACK = 60.0
    ARM_SWING = 50.0

    if pb("thigh_r"):
        key_rot(pb("thigh_r"), 1,  (0, 0,  THIGH))
        key_rot(pb("thigh_r"), 12, (0, 0, -THIGH))
        key_rot(pb("thigh_r"), 24, (0, 0,  THIGH))
    if pb("calf_r"):
        key_rot(pb("calf_r"), 1,  (0, 0, -CALF_FRONT))
        key_rot(pb("calf_r"), 6,  (0, 0, -10))
        key_rot(pb("calf_r"), 18, (0, 0, -CALF_BACK))
        key_rot(pb("calf_r"), 24, (0, 0, -CALF_FRONT))
    if pb("thigh_l"):
        key_rot(pb("thigh_l"), 1,  (0, 0, -THIGH))
        key_rot(pb("thigh_l"), 12, (0, 0,  THIGH))
        key_rot(pb("thigh_l"), 24, (0, 0, -THIGH))
    if pb("calf_l"):
        key_rot(pb("calf_l"), 1,  (0, 0, -CALF_BACK))
        key_rot(pb("calf_l"), 6,  (0, 0, -CALF_FRONT))
        key_rot(pb("calf_l"), 12, (0, 0, -10))
        key_rot(pb("calf_l"), 24, (0, 0, -CALF_BACK))

    if pb("upperarm_r"):
        key_rot(pb("upperarm_r"), 1,  (0, 20, -ARM_SWING))
        key_rot(pb("upperarm_r"), 12, (0, 20,  ARM_SWING))
        key_rot(pb("upperarm_r"), 24, (0, 20, -ARM_SWING))
    if pb("lowerarm_r"):
        key_rot(pb("lowerarm_r"), 1,  (0, 0, -70))
        key_rot(pb("lowerarm_r"), 24, (0, 0, -70))
    if pb("upperarm_l"):
        key_rot(pb("upperarm_l"), 1,  (0, 20,  ARM_SWING))
        key_rot(pb("upperarm_l"), 12, (0, 20, -ARM_SWING))
        key_rot(pb("upperarm_l"), 24, (0, 20,  ARM_SWING))
    if pb("lowerarm_l"):
        key_rot(pb("lowerarm_l"), 1,  (0, 0, -70))
        key_rot(pb("lowerarm_l"), 24, (0, 0, -70))


def author_jump(pb, scene):
    """30-frame non-looping start: crouch (1-8) -> takeoff (8-15) -> air (15-30)."""
    scene.frame_start = 1
    scene.frame_end = 30

    # Legs: Z for forward/back swing (matching walk). Crouch = thighs forward
    # + calves bent. Takeoff = thighs extended back + calves nearly straight.
    # Air = slight tuck.
    for thigh in ("thigh_r", "thigh_l"):
        if pb(thigh):
            key_rot(pb(thigh), 1,  (0, 0, 0))
            key_rot(pb(thigh), 8,  (0, 0,  35))    # crouch (knees forward)
            key_rot(pb(thigh), 15, (0, 0, -15))    # extended back at takeoff
            key_rot(pb(thigh), 30, (0, 0,  20))    # tuck
    for calf in ("calf_r", "calf_l"):
        if pb(calf):
            key_rot(pb(calf), 1,  (0, 0, 0))
            key_rot(pb(calf), 8,  (0, 0, -65))     # deep knee bend
            key_rot(pb(calf), 15, (0, 0, -5))      # extended
            key_rot(pb(calf), 30, (0, 0, -30))     # tuck

    # Arms: start by sides (X=70), swing back during crouch (Z=+30 = back),
    # reach overhead at takeoff (Z=-100 = forward+up,
    # settle partially-raised in air. Same Z signs on L/R since both arms do
    # the same motion (mirrored, not alternating).
    if pb("upperarm_r"):
        key_rot(pb("upperarm_r"), 1,  (0, 20,    0))
        key_rot(pb("upperarm_r"), 8,  (0, 20,   30))    # back swing
        key_rot(pb("upperarm_r"), 15, (0,  0, -100))    # overhead reach
        key_rot(pb("upperarm_r"), 30, (0,  0,  -50))    # settle, partially raised
    if pb("upperarm_l"):
        key_rot(pb("upperarm_l"), 1,  (0, 20,    0))
        key_rot(pb("upperarm_l"), 8,  (0, 20,   30))
        key_rot(pb("upperarm_l"), 15, (0,  0, -100))
        key_rot(pb("upperarm_l"), 30, (0,  0,  -50))

    # Forward lean during crouch (X on spine — unverified; could be a side
    # tilt instead of forward).
    if pb("spine_03"):
        key_rot(pb("spine_03"), 1,  (0,  0, 0))
        key_rot(pb("spine_03"), 8,  (10, 0, 0))    # forward
        key_rot(pb("spine_03"), 15, (5,  0, 0))
        key_rot(pb("spine_03"), 30, (2,  0, 0))


def author_falling(pb, scene):
    """1-second loop. Uncontrolled mid-air flail: legs pedal, arms windmill,
    body twists. Layered motion — should read as actually falling, not posing."""
    scene.frame_start = 1
    scene.frame_end = 30

    # Legs pedaling, alternating phase. Less extreme than a real walk so it
    # reads as "kicking in air" not "running in air."
    THIGH_F = 30.0
    THIGH_B = -10.0
    CALF_BENT = -45.0
    CALF_OPEN = -15.0

    if pb("thigh_r"):
        key_rot(pb("thigh_r"), 1,  (0, 0, THIGH_F))
        key_rot(pb("thigh_r"), 15, (0, 0, THIGH_B))
        key_rot(pb("thigh_r"), 30, (0, 0, THIGH_F))
    if pb("calf_r"):
        key_rot(pb("calf_r"), 1,  (0, 0, CALF_OPEN))
        key_rot(pb("calf_r"), 15, (0, 0, CALF_BENT))
        key_rot(pb("calf_r"), 30, (0, 0, CALF_OPEN))
    if pb("thigh_l"):
        key_rot(pb("thigh_l"), 1,  (0, 0, THIGH_B))
        key_rot(pb("thigh_l"), 15, (0, 0, THIGH_F))
        key_rot(pb("thigh_l"), 30, (0, 0, THIGH_B))
    if pb("calf_l"):
        key_rot(pb("calf_l"), 1,  (0, 0, CALF_BENT))
        key_rot(pb("calf_l"), 15, (0, 0, CALF_OPEN))
        key_rot(pb("calf_l"), 30, (0, 0, CALF_BENT))

    # Asymmetric falling silhouette: right arm raised overhead, left arm
    # extended backward — opposite Z signs send each arm in opposite world
    # directions on the upperarm rig. Reads as "tumbling" rather than
    # "reaching" and looks more dynamic than both-arms-overhead.
    ARM_UP = 100.0
    ARM_DRIFT = 10.0
    if pb("upperarm_r"):
        key_rot(pb("upperarm_r"), 1,  (0, 0, -(ARM_UP + ARM_DRIFT)))
        key_rot(pb("upperarm_r"), 15, (0, 0, -(ARM_UP - ARM_DRIFT)))
        key_rot(pb("upperarm_r"), 30, (0, 0, -(ARM_UP + ARM_DRIFT)))
    if pb("upperarm_l"):
        key_rot(pb("upperarm_l"), 1,  (0, 0,  (ARM_UP - ARM_DRIFT)))
        key_rot(pb("upperarm_l"), 15, (0, 0,  (ARM_UP + ARM_DRIFT)))
        key_rot(pb("upperarm_l"), 30, (0, 0,  (ARM_UP - ARM_DRIFT)))

    # Spine twists side to side (Z, the sway axis from idle) plus held slight
    # backward tilt (-X). Larger twist amplitude than idle since this is
    # supposed to feel uncontrolled.
    TWIST = 8.0
    if pb("spine_03"):
        key_rot(pb("spine_03"), 1,  (-3, 0,  TWIST))
        key_rot(pb("spine_03"), 15, (-3, 0, -TWIST))
        key_rot(pb("spine_03"), 30, (-3, 0,  TWIST))

    # Head turning side to side, counter-phase to the spine twist for a more
    # disoriented look. Z on head is unverified — could come out as a nod.
    if pb("head"):
        key_rot(pb("head"), 1,  (0, 0, -10))
        key_rot(pb("head"), 15, (0, 0,  10))
        key_rot(pb("head"), 30, (0, 0, -10))


def author_death(pb, scene):
    """60-frame non-looping collapse: hit reaction (1-5) -> body falls onto
    the ground while legs swing forward (5-35) -> held seated forward-fold
    pose (35-60). The pelvis is translated ~90 cm down so the character
    actually ends up on the ground rather than floating mid-air."""
    scene.frame_start = 1
    scene.frame_end = 60

    # Pelvis: drop ~90 cm so the character lands on the ground (rest-pose
    # pelvis sits at armature Z = 95.9 cm). pose_bone.location is in the
    # bone's rest-local frame; pelvis bone-local +X points along armature
    # +Z (world up), so -X = down. Holds at -90 for the dead pose.
    if pb("pelvis"):
        key_loc(pb("pelvis"), 1,  (  0, 0, 0))
        key_loc(pb("pelvis"), 5,  (  0, 0, 0))
        key_loc(pb("pelvis"), 20, (-30, 0, 0))    # mid-fall
        key_loc(pb("pelvis"), 35, (-90, 0, 0))    # on ground
        key_loc(pb("pelvis"), 60, (-90, 0, 0))    # hold

    # Spine: small backward jerk on impact, then progressive forward pitch
    # as the body collapses. By frame 35 the upper body is folded forward
    # over the extended legs. Z is the forward-pitch axis on this rig
    # (verified by introspecting matrix_local — bone-local Z points along
    # armature -X, perpendicular to the bone, in the sagittal plane).
    # Bone-local Y is twist (along the bone) and bone-local X is the
    # lateral-bend axis.
    if pb("spine_03"):
        key_rot(pb("spine_03"), 1,  (0, 0,   0))
        key_rot(pb("spine_03"), 5,  (0, 0,  -8))   # backward jerk on hit
        key_rot(pb("spine_03"), 20, (0, 0,  30))   # pitching forward
        key_rot(pb("spine_03"), 35, (0, 0,  75))   # folded over legs
        key_rot(pb("spine_03"), 60, (0, 0,  75))   # hold

    # Legs: rotate from "down" (rest) to "horizontal forward" (Z=90) so
    # the legs end up extended in front of the seated character. Calves
    # bend partway through the fall (knees come up as the pelvis drops),
    # then straighten out as the legs reach horizontal.
    for thigh in ("thigh_r", "thigh_l"):
        if pb(thigh):
            key_rot(pb(thigh), 1,  (0, 0,  0))
            key_rot(pb(thigh), 5,  (0, 0,  0))
            key_rot(pb(thigh), 20, (0, 0, 60))    # knees up mid-fall
            key_rot(pb(thigh), 35, (0, 0, 90))    # legs horizontal forward
            key_rot(pb(thigh), 60, (0, 0, 90))    # hold
    for calf in ("calf_r", "calf_l"):
        if pb(calf):
            key_rot(pb(calf), 1,  (0, 0,   0))
            key_rot(pb(calf), 5,  (0, 0,   0))
            key_rot(pb(calf), 20, (0, 0, -60))    # bent mid-fall
            key_rot(pb(calf), 35, (0, 0,   0))    # straight, foot pointed forward
            key_rot(pb(calf), 60, (0, 0,   0))    # hold

    # Arms: start by sides, hang limp through the fall. With the body
    # folded forward, arms hanging "by sides" in body-local project
    # forward in world, ending splayed near the legs in the final pose.
    if pb("upperarm_r"):
        key_rot(pb("upperarm_r"), 1,  (0, 20,   0))
        key_rot(pb("upperarm_r"), 5,  (0, 20,  10))   # tiny recoil
        key_rot(pb("upperarm_r"), 20, (0, 15, -20))   # swinging forward limp
        key_rot(pb("upperarm_r"), 35, (0, 10,   0))   # splayed, settled
        key_rot(pb("upperarm_r"), 60, (0, 10,   0))
    if pb("upperarm_l"):
        key_rot(pb("upperarm_l"), 1,  (0, 20,   0))
        key_rot(pb("upperarm_l"), 5,  (0, 20,  10))
        key_rot(pb("upperarm_l"), 20, (0, 15, -20))
        key_rot(pb("upperarm_l"), 35, (0, 10,   0))
        key_rot(pb("upperarm_l"), 60, (0, 10,   0))

    # Head snaps back on impact, then drops forward and hangs. Same axis
    # convention as the spine: Z = forward/back pitch (nod), X = lateral
    # bend, Y = twist.
    if pb("head"):
        key_rot(pb("head"), 1,  (0, 0,   0))
        key_rot(pb("head"), 5,  (0, 0, -10))   # snaps back on hit
        key_rot(pb("head"), 20, (0, 0,  20))   # falling forward
        key_rot(pb("head"), 35, (0, 0,  30))   # hung limp
        key_rot(pb("head"), 60, (0, 0,  30))


def author_hands_up(pb, scene):
    """2-second loop. Standing with both arms stretched fully vertical overhead."""
    scene.frame_start = 1
    scene.frame_end = 60

    for side in ("_r", "_l"):
        if pb(f"upperarm{side}"):
            key_rot(pb(f"upperarm{side}"), 1,  (0, -40, -140))
            key_rot(pb(f"upperarm{side}"), 60, (0, -40, -140))
        if pb(f"lowerarm{side}"):
            key_rot(pb(f"lowerarm{side}"), 1,  (0, 0, 0))
            key_rot(pb(f"lowerarm{side}"), 60, (0, 0, 0))


def author_seated(pb, scene):
    """2-second loop. Seated on the ground with legs extended forward at 90 degrees."""
    scene.frame_start = 1
    scene.frame_end = 60

    if pb("pelvis"):
        key_loc(pb("pelvis"), 1,  (-90, 0, 0))
        key_loc(pb("pelvis"), 60, (-90, 0, 0))

    for side in ("_r", "_l"):
        if pb(f"thigh{side}"):
            key_rot(pb(f"thigh{side}"), 1,  (0, 0, 90))
            key_rot(pb(f"thigh{side}"), 60, (0, 0, 90))
        if pb(f"calf{side}"):
            key_rot(pb(f"calf{side}"), 1,  (0, 0, 0))
            key_rot(pb(f"calf{side}"), 60, (0, 0, 0))
        if pb(f"upperarm{side}"):
            key_rot(pb(f"upperarm{side}"), 1,  (0, 20, 0))
            key_rot(pb(f"upperarm{side}"), 60, (0, 20, 0))


def author_fist(pb, scene):
    """2-second loop. Standing with both hands clenched into fists."""
    scene.frame_start = 1
    scene.frame_end = 60

    FINGERS = ["index", "middle", "ring", "pinky"]

    for side in ("_r", "_l"):
        if pb(f"upperarm{side}"):
            key_rot(pb(f"upperarm{side}"), 1,  (0, 20, 0))
            key_rot(pb(f"upperarm{side}"), 60, (0, 20, 0))

        for finger in FINGERS:
            for bone, curl in [("_metacarpal", 20), ("_01", 60), ("_02", 70), ("_03", 50)]:
                name = f"{finger}{bone}{side}"
                if pb(name):
                    key_rot(pb(name), 1,  (0, 0, curl))
                    key_rot(pb(name), 60, (0, 0, curl))

        for bone, curl in [("thumb_01", 20), ("thumb_02", 45), ("thumb_03", 35)]:
            name = f"{bone}{side}"
            if pb(name):
                key_rot(pb(name), 1,  (0, 0, curl))
                key_rot(pb(name), 60, (0, 0, curl))


def author_foot_test(pb, scene):
    """2-second loop. Foot/toe axis test: right foot toes down, left foot toes up,
    ball bones curled on both sides."""
    scene.frame_start = 1
    scene.frame_end = 60

    # Raise legs slightly so feet are visible
    for side in ("_r", "_l"):
        if pb(f"thigh{side}"):
            key_rot(pb(f"thigh{side}"), 1,  (0, 0, 30))
            key_rot(pb(f"thigh{side}"), 60, (0, 0, 30))
        if pb(f"upperarm{side}"):
            key_rot(pb(f"upperarm{side}"), 1,  (0, 20, 0))
            key_rot(pb(f"upperarm{side}"), 60, (0, 20, 0))

    # Right foot: toes point down (Z=45)
    if pb("foot_r"):
        key_rot(pb("foot_r"), 1,  (0, 0, 45))
        key_rot(pb("foot_r"), 60, (0, 0, 45))
    # Left foot: toes point up (Z=-45, opposite sign = opposite direction)
    if pb("foot_l"):
        key_rot(pb("foot_l"), 1,  (0, 0, -45))
        key_rot(pb("foot_l"), 60, (0, 0, -45))

    # Ball/toe curl on both sides (Z=30)
    if pb("ball_r"):
        key_rot(pb("ball_r"), 1,  (0, 0, 30))
        key_rot(pb("ball_r"), 60, (0, 0, 30))
    if pb("ball_l"):
        key_rot(pb("ball_l"), 1,  (0, 0, 30))
        key_rot(pb("ball_l"), 60, (0, 0, 30))


def author_head_turn(pb, scene):
    """2-second loop. Head turns left, then right, then back to center."""
    scene.frame_start = 1
    scene.frame_end = 60

    # Arms by sides
    for side in ("_r", "_l"):
        if pb(f"upperarm{side}"):
            key_rot(pb(f"upperarm{side}"), 1,  (0, 20, 0))
            key_rot(pb(f"upperarm{side}"), 60, (0, 20, 0))

    # Head Y = twist (turn left/right) per the skill's inferred convention
    if pb("head"):
        key_rot(pb("head"), 1,  (0, 0, 0))
        key_rot(pb("head"), 15, (60, 0, 0))     # turn left
        key_rot(pb("head"), 30, (0, 0, 0))      # center
        key_rot(pb("head"), 45, (-60, 0, 0))    # turn right
        key_rot(pb("head"), 60, (0, 0, 0))      # back to center


ANIMATIONS = {
    "Idle":    author_idle,
    "Walk":    author_walk,
    "Run":     author_run,
    "Jump":    author_jump,
    "Falling": author_falling,
    "Death":   author_death,
    "HandsUp": author_hands_up,
    "Seated":  author_seated,
    "Fist":    author_fist,
    "FootTest": author_foot_test,
    "HeadTurn": author_head_turn,
}


def author_wrist_test(pb, scene):
    """2-second loop. Wrist bend test: curl palmward then extend backward."""
    scene.frame_start = 1
    scene.frame_end = 60

    for side in ("_r", "_l"):
        if pb(f"upperarm{side}"):
            key_rot(pb(f"upperarm{side}"), 1,  (0, 20, -30))
            key_rot(pb(f"upperarm{side}"), 60, (0, 20, -30))

        if pb(f"hand{side}"):
            key_rot(pb(f"hand{side}"), 1,  (0, 0, 0))
            key_rot(pb(f"hand{side}"), 15, (0, 0, 60))      # curl palmward
            key_rot(pb(f"hand{side}"), 30, (0, 0, 0))       # neutral
            key_rot(pb(f"hand{side}"), 45, (0, 0, -60))     # extend backward
            key_rot(pb(f"hand{side}"), 60, (0, 0, 0))


def author_shrug(pb, scene):
    """2-second loop. Shoulder shrug: up, hold, back down."""
    scene.frame_start = 1
    scene.frame_end = 60

    for side in ("_r", "_l"):
        if pb(f"upperarm{side}"):
            key_rot(pb(f"upperarm{side}"), 1,  (0, 20, 0))
            key_rot(pb(f"upperarm{side}"), 60, (0, 20, 0))

        if pb(f"clavicle{side}"):
            key_rot(pb(f"clavicle{side}"), 1,  (0, 0, 0))
            key_rot(pb(f"clavicle{side}"), 15, (0, -30, 0))
            key_rot(pb(f"clavicle{side}"), 45, (0, -30, 0))
            key_rot(pb(f"clavicle{side}"), 60, (0, 0, 0))


ANIMATIONS["Shrug"] = author_shrug
ANIMATIONS["WristTest"] = author_wrist_test


def author_arms_forward(pb, scene):
    """2-second loop. Arms extended straight forward at shoulder height."""
    scene.frame_start = 1
    scene.frame_end = 60

    for side in ("_r", "_l"):
        if pb(f"upperarm{side}"):
            key_rot(pb(f"upperarm{side}"), 1,  (-30, 0, -90))
            key_rot(pb(f"upperarm{side}"), 60, (-30, 0, -90))
        if pb(f"lowerarm{side}"):
            key_rot(pb(f"lowerarm{side}"), 1,  (0, 0, 30))
            key_rot(pb(f"lowerarm{side}"), 60, (0, 0, 30))


ANIMATIONS["ArmsForward"] = author_arms_forward


def author_spin(pb, scene):
    """0.5-second non-looping spin attack. Full 360-degree rotation via pelvis
    Y-twist, arms extended outward with fists, slight crouch. 15 frames @ 30fps."""
    scene.frame_start = 1
    scene.frame_end = 15

    FINGERS = ["index", "middle", "ring", "pinky"]

    # Pelvis: full 360-degree spin around the vertical axis.
    # Pelvis bone-local +X = world up, so X rotation = vertical spin.
    # Key every 90 degrees to ensure Blender interpolates the correct direction.
    if pb("pelvis"):
        key_rot(pb("pelvis"), 1,  (  0, 0, 0))
        key_rot(pb("pelvis"), 4,  ( 90, 0, 0))
        key_rot(pb("pelvis"), 8,  (180, 0, 0))
        key_rot(pb("pelvis"), 11, (270, 0, 0))
        key_rot(pb("pelvis"), 15, (360, 0, 0))

    # Arms extended outward, slightly below horizontal. Elbows slightly bent.
    for side in ("_r", "_l"):
        if pb(f"upperarm{side}"):
            key_rot(pb(f"upperarm{side}"), 1,  (-70, 0, 0))
            key_rot(pb(f"upperarm{side}"), 15, (-70, 0, 0))
        if pb(f"lowerarm{side}"):
            key_rot(pb(f"lowerarm{side}"), 1,  (0, 0, -20))
            key_rot(pb(f"lowerarm{side}"), 15, (0, 0, -20))

        # Fists clenched
        for finger in FINGERS:
            for bone, curl in [("_metacarpal", 20), ("_01", 60), ("_02", 70), ("_03", 50)]:
                name = f"{finger}{bone}{side}"
                if pb(name):
                    key_rot(pb(name), 1,  (0, 0, curl))
                    key_rot(pb(name), 15, (0, 0, curl))
        for bone, curl in [("thumb_01", 20), ("thumb_02", 45), ("thumb_03", 35)]:
            name = f"{bone}{side}"
            if pb(name):
                key_rot(pb(name), 1,  (0, 0, curl))
                key_rot(pb(name), 15, (0, 0, curl))

    # Slight crouch: knees bent, lower center of gravity
    for thigh in ("thigh_r", "thigh_l"):
        if pb(thigh):
            key_rot(pb(thigh), 1,  (0, 0, 15))
            key_rot(pb(thigh), 15, (0, 0, 15))
    for calf in ("calf_r", "calf_l"):
        if pb(calf):
            key_rot(pb(calf), 1,  (0, 0, -30))
            key_rot(pb(calf), 15, (0, 0, -30))

    # Slight forward lean
    if pb("spine_03"):
        key_rot(pb("spine_03"), 1,  (0, 0, 8))
        key_rot(pb("spine_03"), 15, (0, 0, 8))

    # Drop pelvis slightly to match the crouch
    if pb("pelvis"):
        key_loc(pb("pelvis"), 1,  (-8, 0, 0))
        key_loc(pb("pelvis"), 15, (-8, 0, 0))


ANIMATIONS["Spin"] = author_spin


def author_and_export(in_fbx, out_dir, anim_name, author_fn):
    out_path = os.path.abspath(os.path.join(out_dir, f"A_Manny_{anim_name}.fbx"))
    print(f"[blender_anim] === {anim_name} -> {out_path}")

    clear_scene()
    bpy.ops.import_scene.fbx(filepath=in_fbx)

    armature = find_armature()
    strip_mesh()

    bpy.context.view_layer.objects.active = armature
    bpy.ops.object.mode_set(mode='POSE')

    scene = bpy.context.scene
    scene.render.fps = 30

    reset_action(armature, f"A_Manny_{anim_name}")
    author_fn(lambda n: armature.pose.bones.get(n), scene)

    bpy.ops.object.mode_set(mode='OBJECT')

    armature.select_set(True)
    bpy.context.view_layer.objects.active = armature

    export_anim_fbx(out_path)


def main():
    in_fbx, out_dir, anim_filter = parse_args()
    in_fbx = os.path.abspath(in_fbx)
    out_dir = os.path.abspath(out_dir)
    os.makedirs(out_dir, exist_ok=True)

    if anim_filter:
        unknown = [n for n in anim_filter if n not in ANIMATIONS]
        if unknown:
            raise SystemExit(f"Unknown animations: {unknown}. Known: {list(ANIMATIONS)}")
        selected = {n: ANIMATIONS[n] for n in anim_filter}
    else:
        selected = ANIMATIONS

    print(f"[blender_anim] in:      {in_fbx}")
    print(f"[blender_anim] out_dir: {out_dir}")
    print(f"[blender_anim] anims:   {list(selected)}")

    for anim_name, author_fn in selected.items():
        author_and_export(in_fbx, out_dir, anim_name, author_fn)

    print(f"[blender_anim] DONE. Generated {len(selected)} animations.")


if __name__ == "__main__":
    main()
