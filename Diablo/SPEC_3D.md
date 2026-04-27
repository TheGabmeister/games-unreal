# SPEC_3D.md — 3D Asset Workflow Experiments

## Current State

All 3D assets are generated via headless Blender Python scripts (`Tools/blender/`). One character exists: the Warrior, built from primitive boxes and a UV sphere, rigged to a 13-bone skeleton, with 4 baked animations (Idle, Walk, Attack, Death). The mesh script (`warrior_mesh.py`) does everything in one pass — geometry, armature, skinning, animation, FBX export.

### What works

- Fully deterministic, reproducible pipeline (no manual Blender work for meshes)
- Single FBX import into UE with mesh + skeleton + animations
- Armature module (`warrior_armature.py`) shared by standalone animation scripts
- FBX export conventions are stable (axis, scale, bone orientation)

### Pain points

1. **No mesh modularity** — body parts are hardcoded primitives. Adding armor, weapons, or character variants means writing a new script from scratch.
2. **Duplicate armature code** — `warrior_mesh.py` re-implements the bone hierarchy instead of importing `warrior_armature.py`.
3. **Animation is code-only** — every keyframe is a Python constant. Tuning feel requires edit → export → reimport cycles. No visual preview before UE.
4. **No character factory** — each new character (Rogue, Sorcerer, enemies) will need its own full script. Shared structure (biped skeleton, primitive body, FBX export) isn't factored out.
5. **Primitive geometry only** — boxes and spheres with no edge loops. Joint deformation is poor; no topology control for detail.
6. **No equipment meshes** — weapons, shields, helmets are not yet generated. The Warrior fights bare-handed visually.

---

## Experiments

Each experiment is a self-contained investigation. Run them independently; results inform which direction we commit to.

### E1 — Character Factory Module

**Goal:** Extract a reusable `character_factory.py` that takes a character definition (bone lengths, body segment dimensions, mesh style) and produces a rigged mesh + FBX.

**Scope:**
- Factor out from `warrior_mesh.py`: armature creation, primitive body generation, auto-weight parenting, FBX export
- Define a character spec as a Python dict: `{ "name": "Warrior", "height": 1.8, "segments": { "torso": (0.4, 0.25, 0.5), ... } }`
- Rebuild `warrior_mesh.py` as a thin wrapper that passes Warrior-specific params to the factory
- Verify the output FBX is byte-identical (or functionally identical) to the current one

**Success criteria:** Can define a second character (e.g. Skeleton enemy) by writing only a param dict + calling the factory, no armature/export boilerplate.

### E2 — Weapon and Equipment Meshes

**Goal:** Generate standalone weapon/shield meshes as separate FBX files, attachable to sockets in UE.

**Scope:**
- Script `weapon_meshes.py` that generates: short sword (blade + crossguard + grip from boxes), buckler shield (flattened cylinder), staff (long thin box)
- Each exported as a static mesh FBX (no skeleton)
- Verify import into UE and attachment to hand bone socket
- Define socket names in the armature (`R_Hand_Socket`, `L_Hand_Socket`)

**Success criteria:** Sword visually attached to Warrior's right hand in-editor, moves with attack animation.

### E3 — Improved Mesh Topology

**Goal:** Investigate whether adding edge loops at joints improves deformation quality enough to justify the complexity.

**Scope:**
- Create `warrior_mesh_v2.py` with subdivided limb segments (2-3 edge loops at elbow/knee/shoulder)
- Compare animation deformation quality between v1 (pure boxes) and v2 (subdivided) in UE
- Measure vertex count increase and visual improvement tradeoff
- Try `bpy.ops.mesh.subdivide()` on joint regions only

**Success criteria:** Visible improvement in elbow/knee bending during Walk and Attack animations without exceeding 2x vertex count.

### E4 — Animation Helpers

**Goal:** Reduce boilerplate in animation keyframing by building a small animation DSL.

**Scope:**
- Create `anim_helpers.py` with utilities:
  - `pose(bone_name, frame, rotation)` — insert a keyframe
  - `tween(bone_name, frame_start, frame_end, rot_start, rot_end)` — linear interpolation
  - `swing(bone_name, frame_start, frame_end, axis, amplitude)` — oscillating motion (for idle breathing, walk cycles)
  - `mirror(bone_prefix_L, bone_prefix_R, frame_offset)` — mirror left/right limb keyframes with optional phase offset
- Rewrite one animation (Walk) using the helpers as a proof-of-concept
- Compare output to the current hand-keyframed version

**Success criteria:** Walk animation definition is half the line count with equivalent output quality.

### E5 — Enemy Mesh Variants

**Goal:** Generate distinct enemy silhouettes using the character factory (E1 dependency).

**Scope:**
- Define 3 enemy body types via param dicts:
  - **Skeleton** — thinner limbs, taller, no head sphere (skull shape from narrow box)
  - **Fallen** — shorter, wider torso, stubby legs (cowardly goblin archetype)
  - **Goat Demon** — tall, thick limbs, wider stance
- Each uses the same 13-bone armature (animation-compatible with Warrior anims as a starting point)
- Export as separate FBX files
- Verify import and that existing animations play on the new skeletons

**Success criteria:** 3 visually distinct enemy meshes that can share the Warrior's Walk/Idle/Attack/Death animations without re-rigging.

### E6 — Blender Material Baking

**Goal:** Investigate baking simple vertex colors or flat materials in Blender so meshes arrive in UE with basic coloring (not all gray).

**Scope:**
- Assign vertex colors per body segment in Blender (skin tone for head/arms, armor color for torso, dark for legs)
- Export with vertex colors in FBX
- Create a UE material that reads vertex color and applies it
- Compare workflow vs. assigning UE materials manually post-import

**Success criteria:** Warrior imports with distinct body segment colors, no manual UE material assignment needed per mesh region.

---

## Experiment Priority

| Priority | Experiment | Rationale |
|----------|-----------|-----------|
| 1 | E1 — Character Factory | Unblocks E5 (enemy variants) and future classes (M27). Highest reuse value. |
| 2 | E4 — Animation Helpers | Reduces animation authoring cost for all future characters and animations. |
| 3 | E2 — Weapon Meshes | Needed for visual combat feedback (currently fighting bare-handed). |
| 4 | E5 — Enemy Variants | Needed before M24 (unique monsters/bosses). Depends on E1. |
| 5 | E3 — Mesh Topology | Nice-to-have visual improvement. Low priority for D1-style graphics. |
| 6 | E6 — Material Baking | Convenience improvement. UE material assignment works fine as fallback. |
