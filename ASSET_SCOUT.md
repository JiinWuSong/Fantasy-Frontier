# Asset Scout Rules

## Purpose

The Asset Scout agent analyzes the current sprint and step, then proposes suitable Unreal/Fab/free assets and tools before implementation starts.

The goal is to reduce wasted implementation time by identifying whether assets are:
- directly usable
- usable only through a separate project and migration
- reference/inspiration only
- not suitable for the current sprint

--------------------------------------------------

## Inputs

Always read first:
- AGENTS.md
- ROADMAP.md
- SPRINT_STATE.md
- PROMPT_RUNBOOK.md

--------------------------------------------------

## Required Behavior

1. Read current sprint and step from SPRINT_STATE.md
2. Determine what type of assets or tools are relevant now
3. Search or evaluate candidate assets/tools only in the context of the current sprint
4. Classify every candidate into exactly one category

--------------------------------------------------

## Asset Categories

### A. Directly Usable
Use this if the asset is:
- a direct Unreal plugin
- a direct Unreal asset pack
- a directly importable FBX/mesh/material package
- usable in the current project without creating a separate source project

### B. Requires Separate Project + Migrate
Use this if the asset is:
- a sample project
- a template project
- a world/sample like Project Titan or Content Examples
- best used by creating/opening its own project first, then migrating selected assets

### C. Reference Only
Use this if the asset/tool is:
- useful for visual/style reference
- not practical to integrate directly right now
- better as inspiration than as implementation material

### D. Not Suitable
Use this if the asset/tool is:
- wrong style
- wrong technical path
- too heavy for the current sprint
- too dependent on systems not yet in project
- likely to waste implementation time

--------------------------------------------------

## Output Requirements

For every candidate asset/tool, report:

- Name
- Category (A/B/C/D)
- Why it fits or does not fit the current sprint
- Whether Codex can directly work with it now
- Whether manual user action is needed
- Suggested next action

At the end, summarize:
- Best 3 candidates
- Which one should be used first
- Which one should wait for a later sprint

--------------------------------------------------

## Enforcement

- Do not recommend random assets unrelated to the active sprint
- Do not recommend new assets if the current sprint should be solved with existing assets first
- Prefer free or already-owned assets when possible
- If a candidate requires a separate project, explicitly say so
- If an asset is only reference-quality, say so clearly
