# \# Fantasy Frontier agent rules

* Never describe stock Epic mannequin visuals as finished hero characters.
* Never describe runtime dressing of a default sample level as a finished tutorial map.
* If something is placeholder, label it explicitly as placeholder.
* A sprint is only "playable and presentable" if the user-facing result is visibly distinct from default Unreal starter content.
* Do not defer blocking visible issues to the next sprint unless explicitly instructed.
* Every sprint report must include:

  1. exact files changed
  2. build/package validation
  3. known issues
  4. NOT DONE items
  5. any placeholder assets still in use
  6. \## Enforcement
  7. 
  8. \- If a rule is violated, explicitly state it in the report.
  9. \- Do not proceed silently if output quality is below these standards.
  10. \## Testing Enforcement
  11. 
  12. \- Always run TESTING.md after implementing features.
  13. \- Never consider a sprint complete without passing tests.
  14. \## Asset Import Enforcement
  15. 
  16. \- Any newly imported character or creature asset must pass IMPORT\_QA.md before integration.
  17. \- Never integrate a newly imported asset into creator, gameplay, NPC, or enemy systems before confirming:
  18. &#x20; - asset path resolves
  19. &#x20; - materials are correct
  20. &#x20; - textures are correct
  21. &#x20; - no default/fallback material is visible
  22. &#x20; - Unreal preview visually matches the source asset well enough
  23. \- If import QA fails, stop integration and fix import/setup first.

