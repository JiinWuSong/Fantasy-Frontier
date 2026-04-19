# Fantasy Frontier Visual QA Rules

## Core Principle

Screenshots are the source of truth for all player-facing quality.

If the screenshots look bad, broken, or low-quality, the task is FAIL,
even if build, package, and smoke tests pass.

--------------------------------------------------

## Mandatory Screenshot Checks

The following screenshots must be generated and evaluated:

1. Title Screen
2. Character Creator – Race/Gender Step
3. Character Creator – Appearance Step
4. Ingame Spawn (Player visible + controllable)

--------------------------------------------------

## FAIL CONDITIONS (CRITICAL)

Any of the following = automatic FAIL:

### UI / Layout
- overlapping text
- unreadable text
- broken spacing
- misaligned panels
- debug-style UI

### Character
- broken mesh
- mannequin-grade presentation
- visible primitive kitbash parts
- missing or ugly materials
- non-human proportions

### Title Screen
- looks like prototype
- flat or low-quality background
- broken motion (glitchy pan, jitter)
- bad logo placement
- poor composition

### Creator
- layout not clean
- steps missing or incorrect
- preview not centered or badly lit

### Gameplay
- movement not working
- camera not working
- input not responding

--------------------------------------------------

## PASS CONDITIONS

A sprint can only PASS if:

- all 4 screenshots look clean and intentional
- no FAIL conditions are present
- presentation is at least "acceptable for a game prototype"
- no obvious placeholder trash in main player path

--------------------------------------------------

## REPORTING RULES

- Do NOT claim "improved", "premium", or "clean" unless screenshots clearly support it
- If something still looks bad → explicitly say it
- If uncertain → mark as PARTIAL, not PASS
- Always include screenshot references in the report

--------------------------------------------------

## ENFORCEMENT

If any FAIL condition is visible in screenshots:

- The sprint is NOT complete
- Do NOT proceed to next sprint
- Fix issues before continuing

--------------------------------------------------
