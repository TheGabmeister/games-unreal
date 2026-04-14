# Quake — Spec Index

The original 1940-line SPEC has been split into three focused docs. Code comments and CLAUDE.md reference "SPEC N.M" — section numbers are stable; they live in [DESIGN.md](DESIGN.md).

## Docs

- **[DESIGN.md](DESIGN.md)** — durable "what the game is" reference. Movement formulas, damage pipeline, class hierarchy, collision model, stats, saves, game rules. **When in doubt, this is the source of truth.**
- **[ROADMAP.md](ROADMAP.md)** — v1 scope and 16-phase plan (0–15). Per-phase checklists for implementation, tests, and manual verification. Goes stale as phases ship.
- **[HUD.md](HUD.md)** — HUD layout with wireframe, element table, and data sources.

## Section Number Map (SPEC N.M → DESIGN N.M)

Section numbers are stable interface. Code comments like `// SPEC 3.3` refer to these:

| Section | Topic                               | Lives in           |
|---------|-------------------------------------|--------------------|
| 1.1     | Movement                            | DESIGN.md § 1.1    |
| 1.2     | Health and Armor                    | DESIGN.md § 1.2    |
| 1.3     | Death and Respawn                   | DESIGN.md § 1.3    |
| 1.4     | Inventory Persistence               | DESIGN.md § 1.4    |
| 1.5     | Damage Pipeline                     | DESIGN.md § 1.5    |
| 1.6     | Collision Model                     | DESIGN.md § 1.6    |
| 2.0     | Weapon Table                        | DESIGN.md § 2.0    |
| 2.1     | Ammo                                | DESIGN.md § 2.1    |
| 2.2     | General Weapon Rules                | DESIGN.md § 2.2    |
| 2.3     | Underwater Discharge                | DESIGN.md § 2.3    |
| 3.1–3.5 | Enemies                             | DESIGN.md § 3      |
| 4.1–4.4 | Items and Pickups                   | DESIGN.md § 4      |
| 5.1–5.9 | Level Structure                     | DESIGN.md § 5      |
| 6.1–6.4 | Game Rules (difficulty, saves, win, failure) | DESIGN.md § 6 |
| 7       | HUD                                 | **HUD.md**         |
| 8.1–8.2 | Audio System                        | DESIGN.md § 8      |
| 9.1–9.4 | Technical Architecture              | DESIGN.md § 9      |
| 10.1–10.5 | Project Layout & Editor Config    | DESIGN.md § 10     |
| 11      | v1 Scope + Phases                   | **ROADMAP.md**     |

## Conflict Rules

- When DESIGN and [CLAUDE.md](CLAUDE.md) disagree on *what the game is*: **DESIGN wins.**
- When DESIGN and CLAUDE.md disagree on *how the code is structured or built*: **CLAUDE.md wins** (it's the working-notes / conventions layer).
- When ROADMAP and DESIGN disagree: **DESIGN wins** (ROADMAP is a delivery plan, not a spec).

## Reading Guide

- Implementing a feature: grep `^### N\.M` in DESIGN.md for the section, read that + the table at its top.
- Picking up a phase: read the whole phase block in ROADMAP.md (one phase is ~30 lines) + any DESIGN.md sections it references.
- Editing HUD: HUD.md is the single source for layout and data sources.
- Architecture question: CLAUDE.md for the "why"; DESIGN.md for the "what."
