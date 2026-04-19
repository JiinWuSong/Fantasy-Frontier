# Prompt Runbook

## Decision Flow

1. If new asset imported:
   - run IMPORT_QA first
   - do not integrate before PASS

2. If build/package/smoke pass but visual QA fails:
   - remain on current step
   - try a different route
   - max 2 meaningful attempts on one route

3. If 2 attempts fail on one route:
   - stop
   - report exact blocker
   - do not silently continue

4. If step passes:
   - update SPRINT_STATE.md
   - start next step from ROADMAP.md

5. If desktop launcher or validation path is unstable:
   - keep one stable executable path
   - avoid repeated firewall prompts from changing package paths
