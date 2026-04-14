# AGENTS.md

This file gives coding agents a fast, practical map of the `FF7` repository.

## Project Overview

- Engine: Unreal Engine `5.7`
- Project file: `FF7.uproject`
- Primary language: C++
- Current game module: `FF7` (`Runtime`)
- Workspace file: `FF7.code-workspace`

This repo currently looks like a freshly scaffolded Unreal project with one runtime module and the usual generated build/output folders already present.

## Repo Layout

- `Source/`
  - `FF7/` contains the main runtime game module.
  - `FF7.Target.cs` and `FF7Editor.Target.cs` define the game/editor targets.
- `Config/` holds Unreal config files.
- `Content/` holds assets and editor-authored content.
- `Binaries/`, `Intermediate/`, `DerivedDataCache/`, and `Saved/` are generated output/cache folders.
- `.vscode/` and `FF7.code-workspace` contain local editor tasks and launch configs.

## Files Agents Should Prefer Editing

- C++ gameplay/module code under `Source/`
- Project settings under `Config/` when the task explicitly requires config changes
- Documentation files at the repo root, including this file

## Files And Folders Agents Should Avoid Editing

- `Binaries/`
- `Intermediate/`
- `DerivedDataCache/`
- `Saved/`
- `.vs/`, `.vscode/`, and `*.code-workspace` unless the task is specifically about tooling/editor setup

Treat those folders as generated or local-environment state. Prefer changing source inputs instead of build outputs.

## Build And Run Guidance

Prefer the existing VS Code tasks in `FF7.code-workspace` instead of inventing custom build commands.

Common tasks already defined there include:

- `FF7Editor Win64 Development Build`
- `FF7 Win64 Development Build`
- `Generate Project Files`

Those tasks call Unreal's standard batch files from the UE `5.7` install at `C:\Program Files\Epic Games\UE_5.7`.

## Unreal-Specific Guardrails

- Do not hand-edit generated project files or compiled outputs.
- Expect many content changes to be binary `.uasset` files; avoid modifying binary assets unless the task explicitly requires it.
- If a task needs new gameplay code, add it under `Source/FF7/` and keep the module structure coherent.
- If new modules or plugins are introduced, update the relevant `.Build.cs`, target files, and `.uproject` metadata together.

## Git And Workspace Notes

- The repo's `.gitignore` excludes generated Unreal outputs, local editor state, and plugin binaries/intermediates.
- The entire `Plugins/` directory is currently ignored. If plugin source needs to become versioned, revisit `.gitignore` first instead of adding files blindly.
- Assume the worktree may contain user changes. Never revert unrelated edits.

## Suggested Agent Workflow

1. Inspect `Source/`, `Config/`, and `FF7.uproject` before making assumptions.
2. Prefer small, source-level changes over regenerating project state.
3. Use the workspace build tasks to validate C++ changes when available.
4. Summarize any Unreal-specific follow-up steps if validation cannot be run in-session.

## Current Known State

- One runtime module: `FF7`
- One editor-only enabled plugin in the project file: `ModelingToolsEditorMode`
- No repo-root `.ignore` file is present at the moment
