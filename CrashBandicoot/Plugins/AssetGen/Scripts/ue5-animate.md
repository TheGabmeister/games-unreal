# Animate UE Mannequin

You are authoring procedural animations for the UE5 Mannequin skeleton via `blender_anim.py`. The user will describe a movement, pose, or animation — your job is to write or modify the Python `author_*` function that produces it.

User's request: $ARGUMENTS

---

## Script location and how to run

Script: `Plugins/AssetGen/Scripts/blender_anim.py`
Source FBX: `Plugins/AssetGen/Rig/SKM_Manny_Simple.FBX`

```
"C:\Program Files\Blender Foundation\Blender 5.1\blender.exe" --background --python Plugins/AssetGen/Scripts/blender_anim.py -- Plugins/AssetGen/Rig/SKM_Manny_Simple.FBX . <AnimName>
```

The third arg is an optional comma-separated filter (e.g. `Walk,Run`). Omit to regenerate all.

Output: `A_Manny_<Name>.fbx` per animation in the output directory (default `.`).

## Script architecture

Each animation is an `author_<name>(pb, scene)` function registered in the `ANIMATIONS` dict at module level. The function receives:
- `pb(name)` — returns a pose bone by name, or `None` if not found. Always guard with `if pb("bone_name"):`.
- `scene` — `bpy.context.scene`. Set `scene.frame_start` and `scene.frame_end` here. FPS is 30.

Available keyframing helpers:
- `key_rot(pb, frame, (x_deg, y_deg, z_deg))` — keys rotation_euler in XYZ order (degrees).
- `key_loc(pb, frame, (x, y, z))` — keys location in bone-local space (centimeters).

To add a new animation:
1. Write `def author_<name>(pb, scene):` following the pattern of existing functions.
2. Add `"Name": author_<name>` to the `ANIMATIONS` dict.
3. Run the script with `<Name>` as the filter.

After running, offer to run the script for the user so they can import into UE.

## Critical export rules

- **Animation-only FBX — no mesh, no skin, no materials.** The UE project already has the mannequin mesh and skeleton imported. The FBX files produced here contain ONLY skeleton + animation curves. `strip_mesh()` is called automatically before export to enforce this. Never add mesh export options — it would bloat the project with duplicate geometry. When importing into UE, use "Import Only Animations" and point "Skeleton" at the existing mannequin skeleton.
- `bake_anim_use_all_bones=True` MUST stay set. Without it, held/static poses export with zero animation data and UE rejects the FBX.
- `add_leaf_bones=False` — leaf bones break the skeleton match in UE.

---

## Bind pose (default, all rotations at zero)

The rest/bind pose is an **A-pose**: the character stands upright with arms angled out and slightly down (roughly 20-30 degrees below horizontal), legs straight, fingers slightly splayed in a relaxed open hand. This is what the mesh looks like with no animation applied — all bone rotations at `(0, 0, 0)`.

Key reference points at rest:
- **Arms**: angled out and slightly down (A-pose, not T-pose). Upperarm `(0,0,0)` = this pose. **`(0,20,0)` brings them to a natural arms-by-sides resting position** (Y=20 rotates them inward). This replaces the old `X=70` convention which was rotating the wrong axis.
- **Legs**: straight, standing. Thigh/calf all at `(0,0,0)`.
- **Spine**: upright. All spine bones at `(0,0,0)`.
- **Pelvis**: at Z=95.9 cm above ground. `location = (0,0,0)`.
- **Total height**: ~163 cm.

All animation is relative to this pose. "Neutral" in an `author_*` function means returning to these values, not to some other pose.

---

## Skeleton reference — 88 bones

### Hierarchy (indentation = parent-child)

```
pelvis                          (root, head at Z=95.9cm)
  spine_01
    spine_02
      spine_03
        spine_04
          spine_05
            clavicle_l
              upperarm_l
                lowerarm_l
                  hand_l
                    index_metacarpal_l → index_01_l → index_02_l → index_03_l
                    middle_metacarpal_l → middle_01_l → middle_02_l → middle_03_l
                    ring_metacarpal_l → ring_01_l → ring_02_l → ring_03_l
                    pinky_metacarpal_l → pinky_01_l → pinky_02_l → pinky_03_l
                    thumb_01_l → thumb_02_l → thumb_03_l
                  lowerarm_twist_01_l, lowerarm_twist_02_l
                upperarm_twist_01_l, upperarm_twist_02_l
            clavicle_r
              upperarm_r
                lowerarm_r
                  hand_r
                    (same finger layout as L side, with _r suffix)
                  lowerarm_twist_01_r, lowerarm_twist_02_r
                upperarm_twist_01_r, upperarm_twist_02_r
            neck_01
              neck_02
                head
  thigh_l
    calf_l
      foot_l
        ball_l
      calf_twist_01_l, calf_twist_02_l
    thigh_twist_01_l, thigh_twist_02_l
  thigh_r
    calf_r
      foot_r
        ball_r
      calf_twist_01_r, calf_twist_02_r
    thigh_twist_01_r, thigh_twist_02_r
```

Virtual/IK bones (don't keyframe these for FK animation):
`center_of_mass`, `interaction`, `ik_foot_root`, `ik_foot_l`, `ik_foot_r`, `ik_hand_root`, `ik_hand_gun`, `ik_hand_l`, `ik_hand_r`

### Bone groups for code

```python
SPINE   = ["spine_01", "spine_02", "spine_03", "spine_04", "spine_05"]
NECK    = ["neck_01", "neck_02"]
ARMS_L  = ["clavicle_l", "upperarm_l", "lowerarm_l", "hand_l"]
ARMS_R  = ["clavicle_r", "upperarm_r", "lowerarm_r", "hand_r"]
LEGS_L  = ["thigh_l", "calf_l", "foot_l", "ball_l"]
LEGS_R  = ["thigh_r", "calf_r", "foot_r", "ball_r"]
FINGERS = ["thumb", "index", "middle", "ring", "pinky"]  # each has _metacarpal, _01, _02, _03 + _l/_r
```

---

## Axis conventions (Euler XYZ, degrees)

All rotations are `rotation_mode = 'XYZ'` (set automatically by `key_rot`). Axes below refer to what happens when you put a positive value on that Euler channel.

### Spine chain: pelvis, spine_01 through spine_05

All spine bones share the same local frame orientation:
- **X = lateral bend** (side tilt). Positive X on spine tilts the torso to one side.
- **Y = twist** (axial rotation around the bone's own length). Positive Y twists/rotates the torso.
- **Z = forward/back pitch**. **Positive Z = forward fold** (lean forward / hunch); negative Z = arch backward.

Verified at large magnitude: `spine_03 Z=+75` folds the chest fully forward over the legs (used in Death).

The spine has 5 bones (spine_01 through spine_05). For subtle motion, keying just `spine_03` is fine. For more natural bends, distribute rotation across 2-3 spine bones. spine_01/02 are lower back, spine_03 is mid, spine_04/05 are upper chest.

### Neck: neck_01, neck_02

Same convention as spine:
- **X = lateral tilt** (ear toward shoulder).
- **Y = twist** (turn the neck without moving the head).
- **Z = forward/back nod**. Positive Z = neck bends forward.

### Head

**VERIFIED — axes are NOT the same as spine. All three tested independently.**

- **X = face turn** (look left / look right). Positive X = turn left; negative X = turn right. Verified at X=60.
- **Y = lateral tilt** (ear toward shoulder, sideways head tilt). Verified at Y=60.
- **Z = forward/back nod**. Positive Z = chin drops toward chest; negative Z = look up. Verified: `head Z=+30` hangs the head limp forward (used in Death).

### Clavicles: clavicle_l, clavicle_r

**VERIFIED — all three axes tested.**

- **X = protraction/retraction** (shoulder forward/back). Verified at X=30.
- **Y = shrug (elevation/depression)**. **Negative Y = shoulder up (shrug)**; positive Y = shoulder down. Verified at Y=-30.
  - **Same sign on both clavicles = mirrored motion** (both shoulders shrug up together).
- **Z = other axis** (tested at Z=30, not shrug or forward/back — likely twist along the collarbone).

### Upperarms: upperarm_l, upperarm_r

**VERIFIED — the most-used bones in the script.**

- **X = up/down (abduction/adduction)**.
  - `X=0` = bind pose (~A-pose, arms angled out and slightly down).
  - `X=-90` = arms at horizontal / T-pose (can't go higher via X alone).
- **Y = arms by sides / forward-back correction**. **Positive Y=20 = natural arms-by-sides resting position** (replaces the old incorrect `X=70` convention). When arms are raised overhead via Z, Y corrects forward/back lean: **negative Y pulls backward toward true vertical; positive Y pushes forward.**
  - `Y=20` = arms by sides (idle/standing). Verified.
  - `Y=-40` with `Z=-140` = arms perfectly vertical overhead. Verified.
- **Z = forward/back swing**.
  - `Z=-100` = arm raised overhead (forward and up).
  - `Z=-140` = arm past vertical (slightly backward without Y correction).
  - `Z=+100` = arm swung fully backward.
  - `Z=0` = neutral.

**Verified key poses:**
- `(0, 20, 0)` = arms by sides, natural idle. Use this as the default resting position.
- `(0, -40, -140)` = arms straight up, fully vertical (HandsUp).
- `(-30, 0, -90)` + lowerarm `(0, 0, 30)` = arms extended straight forward at shoulder height.
- `(0, 0, -100)` = overhead reach, angled forward (Jump takeoff).

**L/R sign rule**: same sign on both sides = same world direction (mirrored). Opposite signs = alternating (e.g. walk arm swing where arms go opposite directions).

### Lowerarms (forearms): lowerarm_l, lowerarm_r

- **X = lateral deviation** (ulnar/radial — wrist bending side to side). Rarely needed.
- **Y = twist** (pronation/supination — palm up vs. palm down). Useful for gestures.
- **Z = elbow bend (flexion)**. When arms hang by sides: **negative Z = bend the elbow**. `Z=0` = arm straight. `Z=-70` = moderately bent (used in Run).

Verified: `lowerarm_r Z=-70` produces a natural running arm bend.

**Important: Z sign depends on upperarm orientation.** When the upperarm is rotated forward (e.g. arms-forward pose), the lowerarm's local frame rotates with it. In a forward-reaching pose (`upperarm (-30, 0, -90)`), **positive Z=30 on lowerarm** brings the forearm down to horizontal. Always think about what direction "bend" means relative to the current arm orientation, not just the rest pose.

**Verified arms-forward pose:** upperarm `(-30, 0, -90)` + lowerarm `(0, 0, 30)` = arms extended straight forward at shoulder height.

### Hands: hand_l, hand_r

**VERIFIED — Z is the wrist bend axis.**

- **X = ulnar/radial deviation** (wrist bending sideways, toward pinky or thumb).
- **Y = twist** (wrist rotation, continuing the forearm's pronation/supination).
- **Z = wrist flexion/extension**. **Positive Z = wrist curls palmward (flexion); negative Z = wrist bends backward (extension).** Verified at Z=60 / Z=-60.

### Thighs: thigh_l, thigh_r

**VERIFIED.**

- **X = side-to-side spread** (abduction/adduction). Not used for standard locomotion.
- **Y = twist** (internal/external rotation of the leg).
- **Z = forward/back leg swing (hip flexion/extension)**.
  - `Z=+90` = leg horizontal forward (used in Death seated pose).
  - `Z=+25` = moderate forward step (Walk).
  - `Z=0` = neutral (standing).
  - `Z=-25` = moderate backward extension (Walk back-leg phase).

**L/R sign rule**: same as upperarms. Same sign = mirrored. Opposite signs = alternation (standard walking).

### Calves: calf_l, calf_r

**VERIFIED.**

- **X = lateral** (not useful — knees are hinges).
- **Y = twist** (tibia rotation — minimal use).
- **Z = knee bend (flexion)**. **Negative Z = bend the knee**. `Z=0` = straight leg. `Z=-65` = deep bend (Jump crouch). `Z=-60` = mid-fall tuck (Death).

Like the elbow: knees only bend one way. Always use negative Z.

### Feet: foot_l, foot_r

**VERIFIED — Z is the ankle flex axis, not X.**

- **X = inversion/eversion** (tilting the sole inward or outward, sideways tilt).
- **Y = twist** (foot rotation inward/outward around the shin axis).
- **Z = ankle dorsiflexion/plantarflexion** (toe up / toe down).
  - foot_r: positive Z = toes point down (plantarflexion). Verified at Z=45.
  - foot_l: negative Z = toes point up (dorsiflexion). Verified at Z=-45.
  - **Opposite signs on L/R = same world direction** (both toes up or both down).

### Ball (toe base): ball_l, ball_r

**VERIFIED — Z is the toe curl axis, not X.**

- **X = lateral** (minimal use).
- **Z = toe curl/extension**. Bending the toes at the ball of the foot.
  - Positive Z = toes curl down. Verified at Z=30.
- **Y = twist** (minimal use).

### Fingers

Each finger has: `<finger>_metacarpal_<side>`, `<finger>_01_<side>`, `<finger>_02_<side>`, `<finger>_03_<side>`.
Fingers: `thumb`, `index`, `middle`, `ring`, `pinky`. Sides: `_l`, `_r`.

**VERIFIED — positive Z = curl closed (fist), negative Z = extend/spread open.**

For all non-thumb finger bones:
- **Z = curl (flexion/extension)** — the primary animation axis. **Positive Z = curl the finger closed.** Negative Z = extend/spread.
- **X = lateral splay** (spread fingers apart). Small values only.
- **Y = twist** (minimal use).

For thumb bones (`thumb_01/02/03`):
- **Z = curl** applies to all three. **thumb_01** (the base) also has significant X range for opposition (bringing the thumb across the palm).

**Verified fist preset (max values before fingers intersect):**

```python
# Fist (verified — do not exceed these or fingers clip through each other)
FIST_METACARPAL = 20   # Z on _metacarpal bones
FIST_01 = 60           # Z on _01 bones
FIST_02 = 70           # Z on _02 bones
FIST_03 = 50           # Z on _03 bones
THUMB_01 = 20          # Z on thumb_01
THUMB_02 = 45          # Z on thumb_02
THUMB_03 = 35          # Z on thumb_03

# Relaxed open hand (slight natural curl)
FINGERS_RELAXED = 10   # Z on all _01, _02, _03

# Pointing (index extended, others curled)
# Set index_01/02/03 Z=0, all others to fist values
```

### Twist bones (deformation helpers)

`upperarm_twist_01/02`, `lowerarm_twist_01/02`, `thigh_twist_01/02`, `calf_twist_01/02` (all with `_l`/`_r`).

These exist for smooth skin deformation when limbs twist. They share their parent's Y axis (twist). **Do not keyframe these directly** — they're meant to be driven by constraints or left at rest. Keying the main bone's Y rotation already produces the twist; the twist bones just distribute it for better mesh deformation.

---

## Pelvis translation (ground anchoring)

`pose_bone.location` on the pelvis is in **bone-local space**, not world space.

- Pelvis bone-local **+X = world up** (armature +Z). So **negative X = drop the character down**.
- Rest-pose pelvis sits at Z = 95.9 cm above ground.
- `location.x = -90` drops the rig ~90 cm, putting it on the floor.
- `location.x = -50` = roughly a deep crouch / knees-on-ground height.
- The whole skeleton inherits the pelvis translation. No per-bone offsets needed.
- Units are centimeters (~163 cm total character height).

Use `key_loc(pb("pelvis"), frame, (-amount, 0, 0))` to drop. Y and Z are rarely needed (Y = forward/back shift, Z = lateral shift, both in bone-local frame).

---

## L/R sign rules (summary)

This matters for every paired bone. The rig is mirrored, so the local frames of `_l` and `_r` bones are reflections of each other. This means:

- **Same sign on both sides = mirrored motion** (same world direction). Both arms up, both legs forward, both shoulders shrug.
- **Opposite signs = alternating motion** (opposite world directions). Walk arm swing, walk leg swing.

This rule holds consistently across ALL paired bones: upperarms, lowerarms, hands, thighs, calves, feet, clavicles, and fingers.

---

## Animation authoring guidelines

### Frame rates and durations
- FPS is always 30.
- Looping animations: make frame 1 and the last frame identical (the engine blends them).
- Common durations: Idle = 60 frames (2s), Walk = 30 frames (1s), Run = 24 frames (0.8s).
- Non-looping: Jump = 30 frames, Death = 60 frames.

### Keyframing patterns
- **Held pose**: key the same values at frame_start and frame_end. Every bone must still be keyed (Blender will bake all bones on export anyway due to `bake_anim_use_all_bones`).
- **Cyclic motion**: key at frame 1, mid-cycle, and last frame (= frame 1 values) for a clean loop.
- **Transitions**: key the start pose, one or more breakdown poses, and the final pose. Use more keys for complex arcs.

### Bone keying best practices
- Always check `if pb("bone_name"):` before keying — this makes the script resilient to rig changes.
- For symmetric motion (both arms do the same thing), use a loop:
  ```python
  for side in ("_r", "_l"):
      if pb(f"upperarm{side}"):
          key_rot(pb(f"upperarm{side}"), 1, (70, 0, 0))
  ```
- For alternating motion (walk), key each side explicitly with opposite Z signs.
- When animating a chain (spine, fingers), distribute rotation across multiple bones for natural arcs rather than hinging on a single bone.

### Verifying uncharted axes
Several bone groups have axis conventions inferred from `matrix_local` but not yet verified by visual inspection at large magnitudes. Before building an animation that relies heavily on an unverified axis:

1. Write a short test animation that rotates the bone 45-60 degrees on the axis in question.
2. Run the script, import into UE (or check in Blender viewport), and confirm the direction matches expectations.
3. If it doesn't match, check the perpendicular axes — the actual direction is one of the other two.

Verified axes are marked "VERIFIED" above. Unverified axes say "inferred from matrix_local."

---

## Existing animations (for reference, do not duplicate)

The script currently contains these `author_*` functions:
- `author_idle` — 60-frame loop, arms by sides (X=70), subtle spine sway on Z.
- `author_walk` — 30-frame 2-stride loop, legs alternate on Z, arms counter-swing.
- `author_run` — 24-frame 2-stride loop, exaggerated walk + bent elbows (lowerarm Z=-70).
- `author_jump` — 30-frame non-looping: crouch → takeoff → air pose.
- `author_falling` — 30-frame loop, legs pedal, arms asymmetric, spine twists.
- `author_death` — 60-frame non-looping: hit reaction → collapse → seated forward fold with pelvis drop.

When the user asks for a new animation, check whether it should replace or supplement one of these. When modifying an existing animation, read the current function first.
