# Tooling Classifier Rules

## Purpose

The Tooling Classifier determines what kind of Unreal/Fab item a candidate actually is, and how it must be used.

This prevents wasted time caused by confusing:
- plugins
- sample projects
- direct asset imports
- migrate-only packs
- reference-only tools

--------------------------------------------------

## Inputs

Always read first:
- AGENTS.md
- ROADMAP.md
- SPRINT_STATE.md
- ASSET_SCOUT.md

--------------------------------------------------

## Classification Types

### 1. Engine Plugin
Examples:
- AI SDKs
- utility plugins
- editor plugins

How to use:
- install in engine
- enable in project plugins
- then configure

### 2. Project Template / Sample Project
Examples:
- Project Titan
- Content Examples
- Valley of the Ancient

How to use:
- create/open separate project
- inspect assets/maps there
- migrate selected content into target project

### 3. Direct Unreal Asset Pack
Examples:
- UE-ready environment pack
- direct character pack with Add to Project support

How to use:
- add directly to project
- then run Import QA or integration QA if needed

### 4. External Import Asset
Examples:
- FBX
- OBJ
- standalone mesh/material downloads

How to use:
- import manually or through controlled import workflow
- run IMPORT_QA before integration

### 5. Reference / Inspiration Asset
Examples:
- useful for visual direction only
- not worth integrating directly now

How to use:
- reference only
- do not treat as implementation dependency

--------------------------------------------------

## Required Output

For every tool/asset:
- Name
- Classification Type
- Directly usable now: YES / NO
- Requires separate project: YES / NO
- Requires manual user action: YES / NO
- Suitable for current sprint: YES / NO
- Risk level: LOW / MEDIUM / HIGH
- Recommended workflow

--------------------------------------------------

## Enforcement

- Do not confuse sample projects with direct add-to-project assets
- Do not treat engine plugins as gameplay-ready systems by default
- Do not recommend import-heavy tooling if the sprint does not need it
- If the tool requires manual steps, state them explicitly
- If the tool is likely to create integration chaos, say so clearly
