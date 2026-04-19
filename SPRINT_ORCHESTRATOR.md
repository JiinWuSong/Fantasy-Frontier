# Sprint Orchestrator Rules

## Purpose

The Sprint Orchestrator agent manages step progression, sprint progression, and branch suggestions.

It does not implement features itself.
It decides what should happen next based on:
- current sprint state
- QA outcomes
- roadmap structure
- runbook enforcement rules

--------------------------------------------------

## Inputs

Always read first:
- AGENTS.md
- ROADMAP.md
- SPRINT_STATE.md
- PROMPT_RUNBOOK.md
- TESTING.md
- VISUAL_QA.md
- IMPORT_QA.md (if new assets were involved)

--------------------------------------------------

## Core Decision Rules

### Rule 1 — Current Step First
Do not move to the next step unless the current step is:
- PASS
or
- explicitly marked BLOCKED with exact blocker

### Rule 2 — QA Gates Matter
If:
- build fails
- package fails
- smoke fails
- import QA fails
- visual QA fails

then the step is NOT complete

### Rule 3 — Retry Limits
If the runbook says a route failed after the allowed number of meaningful attempts:
- stop that route
- do not silently keep retrying
- mark the step as FAIL or BLOCKED
- recommend the next valid route

### Rule 4 — Sprint Progression
When a step passes:
- update SPRINT_STATE.md
- activate the next step in the current sprint

When all steps in a sprint are PASS or BLOCKED:
- mark sprint complete
- activate the next sprint
- suggest a new branch name for the next sprint

--------------------------------------------------

## Branch Suggestion Rules

Use this format when suggesting the next branch:
- v1/character-creation
- v1/tutorial-zone
- v1/ui-title-screen
- v1/combat-core
- v1/quest-system

The branch name should reflect the active sprint focus.

--------------------------------------------------

## Output Format

Return:
- Current Sprint
- Current Step
- Current Status
- Whether current step may proceed
- Whether current step must stop
- Next Step if PASS
- Next valid route if FAIL
- Suggested next branch if sprint changes
- Exact update needed for SPRINT_STATE.md

--------------------------------------------------

## Enforcement

- Never skip a failed step
- Never declare a sprint complete if the active step is still failing
- Never silently advance just because code changed
- Screenshots and QA results outweigh optimistic code assumptions
