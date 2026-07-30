# Fantasy Frontier Highland V84.2 - AAA Grass Art Lock Report

Date: 2026-07-30

## Executive Verdict

V84.2 grass-art scope: **PASS**

Formal project-wide `TESTING.md` completion: **PARTIAL / EXISTING CONTENT BLOCKER**

The final Highland grass now reads as a planted, layered stylized meadow instead
of black-backed cards, broad plastic leaves, or a synchronized technical
prototype. The native LandscapeGrass architecture from V84.1 remains intact.
MapCheck, editor build, native GrassMap generation, LOW/MEDIUM/HIGH runtime,
long movement, package, packaged smoke, packaged visual parity, material scan,
and fatal scan all passed.

The project-wide test checklist still cannot execute the final two unrelated
steps, NPC interaction and enemy combat, because the protected Highland
baseline does not contain approved NPC or enemy actors. Adding those systems
was forbidden by this sprint. This is documented rather than hidden.

## 1. Git And Backup

- Branch: `feature/highland-v842-grass-art-lock`
- V84.1 baseline commit:
  `a446557494f2e272eadb07b189a2a2fe4914b5fb`
- V84.2 final commit: this report is enclosed by the final V84.2 commit; the
  immutable SHA is reported in the live completion output and branch history.
- Remote target:
  `origin/feature/highland-v842-grass-art-lock`
- Main branch push: **not performed**
- Backup:
  `Saved/V842_PreGrassArtLockBackup_20260730_184422`
- Backup size: 209.18 MB
- Backup files: 118
- Backup manifest: present
- No paid content was downloaded.

The backup contains the requested V84.1 map, materials, GrassTypes, commandlet
source, report, and final evidence before V84.2 changes.

## 2. Scope Lock

Implemented:

- final grass backface correction
- final medium-grass silhouette correction
- true timestamped wind proof
- decorrelated patch distribution
- three-layer visual hierarchy
- grass color and lighting balance
- mid-distance and scalability validation
- low-memory validation tooling
- final validation cameras

Not implemented:

- terrain sculpting
- geomorphology
- forest placement
- village placement
- resources
- water or waterfall work
- NPC, quest, combat, UI, or character work
- edits to TitanMain, Kashkeh, CharacterImportLab, or Fab

## 3. Focused Titan / Project Asset Audit

The sprint reused only existing project assets and created derived Fantasy
Frontier materials and GrassTypes. Original reference assets were not modified.

Inspected candidates:

- `/Game/Environment/Foliage/Meshes/SM_GrassBlade`
- `/Game/Environment/Foliage/Grass/Ryegrass_Grass_D`
- `/Game/Environment/Foliage/Grass/Ryegrass_Grass_C`
- `/Game/Environment/Foliage/Grass/Ryegrass_Rye_Tuft`
- `/Game/Environment/Foliage/Grass/Ryegrass_Stalk_Green`
- `/Game/Environment/Clifftop/Materials/Foliage/MI_Clifftop_Ryegrass`
- `/Game/Environment/Clifftop/Textures/Foliage/T_Ryegrass_D`

Adapted principles:

- narrow blade clusters carry the ordinary meadow read
- individual species use different scale, density, and color roles
- bases remain planted while the upper blade carries most motion
- broad gusts are coherent, while per-instance phase avoids mechanical waves
- macro, meso, and micro distribution layers do not share identical borders
- transmission and corrected two-sided normals keep foliage readable in
  backlight without emissive or unlit shortcuts

## 4. Defect Diagnosis

### 4.1 Dark / Black Backsides

Root cause:

- V84.1 was already two-sided, but the backface world normal was not made
  coherent with face orientation.
- Reversed card faces therefore produced an excessively dark response.
- Low-energy subsurface values amplified the problem in backlight.
- Fixed-camera candidate testing ruled out one-sided geometry and global
  lighting as the primary cause.

Correction:

- retained `TwoSided=true`
- retained `TwoSidedFoliage`
- multiplied the source vertex normal by `TwoSidedSign`
- softened the corrected normal toward world up by 0.30-0.33
- raised controlled foliage transmission
- retained matte roughness at 0.96-0.97
- retained zero or near-zero specular at 0.00-0.01

Explicitly rejected:

- emissive workaround
- unlit material
- disabled shadows
- fully flattened normals
- one uniform bright color

Visual result:

- front-lit blades remain green and readable
- side-lit blades preserve depth
- back-lit blades are darker green, but no longer common black cards

Evidence:

- `Saved/V842_GrassArtLock/V842_Lighting_Backface_Comparison.png`
- `Saved/FFSmokeCaptures/V842_Final_LightingProof`

### 4.2 Broad Medium-Grass Cards

Root cause:

- the V84.1 medium candidate used a broad ryegrass card silhouette
- XY scale and close overlap made individual cards read as plastic leaves or
  flat triangles
- adding density could hide gaps but made the silhouette defect worse

Correction:

- selected existing `Ryegrass_Rye_Tuft`
- applied a derived V84.2 two-sided foliage material
- constrained XY scale to 0.22-0.34 / 0.20-0.32
- retained useful Z variation at 0.62-0.94
- lowered density to a deliberate primary-meadow value of 52

Visual result:

- the medium layer reads as narrow clustered blades at normal player distance
- no production density increase was used to conceal the defect

Evidence:

- `Saved/V842_GrassArtLock/V842_Silhouette_BeforeAfter.png`

## 5. Final Materials

### `M_FF_Highland_GrassBlade_V842`

- two-sided foliage shading
- corrected face-aware world normal
- restrained deep, soft, warm, and cool green variation
- matte response
- controlled subsurface transmission
- anchored LOW wind
- full MEDIUM/HIGH gust, sway, and tip flutter

### `M_FF_Highland_Ryegrass_V842`

- retains approved ryegrass opacity texture
- removes white clipping
- corrects backface normals
- creates a readable primary meadow green range
- uses ryegrass-specific planted wind masks

### `M_FF_Highland_Wildgrass_V842`

- uses the proven narrow blade mesh
- warmer stem and tip balance
- clearly taller than filler and medium layers
- sparse accent distribution
- does not flood the full map

The existing V84.1 Highland lighting state was retained:

- directional intensity: 4.6
- skylight intensity: 1.15
- fixed exposure bias: 1.25
- bloom intensity: 0.02

This is not a new global lighting redesign. The same values existed in the
approved V84.1 implementation. V84.2 primarily corrects grass materials.

## 6. Final Mesh And GrassType Configuration

| Layer | Mesh | Density | X scale | Y scale | Z scale | Start cull | End cull | WPO end |
|---|---|---:|---|---|---|---:|---:|---:|
| Short filler | `SM_GrassBlade` | 220.0 | 0.68-1.05 | 0.66-1.02 | 0.22-0.46 | 2500 | 9000 | 5500 |
| Medium meadow | `Ryegrass_Rye_Tuft` | 52.0 | 0.22-0.34 | 0.20-0.32 | 0.62-0.94 | 3000 | 11500 | 7200 |
| Tall wildgrass | `SM_GrassBlade` | 7.5 | 0.38-0.62 | 0.36-0.58 | 0.95-1.32 | 4000 | 14000 | 8000 |

Roles:

- filler: dense, short, quiet ground connection
- medium: dominant ordinary meadow silhouette and strongest common wind read
- tall: sparse warm wildgrass accent

The tall layer is a distinct tall wildgrass treatment, not a bespoke modeled
seed-head asset. Existing seed/stalk candidates produced poorer card reads and
were rejected.

## 7. Wind Implementation And Proof

Preserved architecture:

- planted base
- upper bend
- tip flutter
- broad gust field
- per-instance phase
- distance-based WPO reduction

V84.2 refinements:

- filler base begins moving only above the 0.28-0.48 normalized height band
- medium base begins moving only above the 0.30-0.50 band
- upper masks are squared to protect roots
- tip flutter begins at approximately 0.68 normalized height
- per-instance random phase changes rest direction and timing
- world-position gust terms move across the field coherently
- LOW uses a simplified single-sway path
- MEDIUM/HIGH use full sway, gust, cross motion, and tip flutter

The old repeated `Wind frame 0` labeling was a capture-tool issue, not proof of
real elapsed time. `TP_ThirdPersonPlayerController` now warms the camera and
labels each capture from actual elapsed runtime.

Captured timestamps:

| Frame | Actual timestamp | Normalized diff vs T0 | Base estimate | Upper estimate |
|---|---:|---:|---:|---:|
| T0 | 0.000 s | 0.000000 | 0.000000 | 0.000000 |
| T1 | 0.325 s | 0.030714 | 0.021898 | 0.049779 |
| T2 | 0.662 s | 0.034367 | 0.025437 | 0.054616 |
| T3 | 1.008 s | 0.037192 | 0.028284 | 0.058610 |
| T4 | 1.508 s | 0.040485 | 0.032496 | 0.062959 |
| T5 | 2.007 s | 0.043028 | 0.036751 | 0.065677 |

- mean base estimate: 0.028973
- mean upper estimate: 0.058328
- upper/base ratio: 2.013

Method limitation:

These are fixed image-band motion estimates, not vertex telemetry. The camera,
exposure, resolution, quality, and warmup are fixed. The result reliably proves
that the visible upper region changes about twice as strongly as the base
region without inventing engine timing data.

Evidence:

- `Saved/V842_GrassArtLock/V842_Wind_ContactSheet.png`
- `Saved/V842_GrassArtLock/V842_WindMetrics.json`
- `Saved/FFSmokeCaptures/V842_Final_WindProof_Exact02`

## 8. Natural Patch Distribution

The V84.1 shared patch tendency was replaced with independent V84.2 masks.

Short filler:

- broad warped field at approximately 58,000 units
- meso breakup at approximately 18,500 units
- micro breakup at approximately 6,100 units
- continuous 0.92-1.00 modulation over valid base density

Medium meadow:

- independent seeds, offsets, rotation, and warp
- macro field at approximately 33,500 units
- meso field at approximately 12,300 units
- micro field at approximately 4,700 units
- soft 0.72-0.92 modulation

Tall wildgrass:

- independent seeds, offsets, rotation, and warp
- macro field at approximately 41,000 units
- meso field at approximately 15,700 units
- micro field at approximately 5,900 units
- sparse pocket range of 0.06-0.88

All masks multiply the approved V84 base density, so path, water, gameplay, and
protected-zone zeros remain zeros. The system does not return to uniform
full-map coverage.

Rejected:

- shared patch borders
- hard thresholds
- circular islands
- sine stripes
- checkerboard breakup
- a full-field density carpet

Evidence:

- `Saved/V842_GrassArtLock/V842_Patch_Species_Proof.png`
- `Saved/FFSmokeCaptures/V842_Final_PatchTopDown`

## 9. Mid And Far Distance

The final mid-distance read was compared with a diagnostic
`grass.CullDistanceScale=2.0` capture. The visible smooth distant region did not
change, proving it is the approved gameplay/exclusion density field rather than
a production cull wall.

No diagnostic cull multiplier was retained.
No `PerInstanceFade` workaround was added.

Accepted result:

- no sudden grass disappearance wall in the reviewed player-height cameras
- no repeated circular islands
- no white shimmering tips in final captures
- exposed ground remains readable after grass culls

Evidence:

- `Saved/FFSmokeCaptures/V842_Diagnostic_MidDistance_Cull2x`
- `Saved/FFSmokeCaptures/V842_Final_GrassCore_Authoritative`

## 10. Scalability Lock

| Setting | Density scale | Cull scale | Wind path | Verdict |
|---|---:|---:|---|---|
| LOW | 38% | 65% | simplified anchored sway | PASS |
| MEDIUM | 68% | 82% | full normal wind | PASS |
| HIGH+ | 100% | 100% | full wind and longest range | PASS |

LOW visibly reduces coverage and range but remains readable and contains no
black backface regression. MEDIUM is the intended balanced mid-range setting.
HIGH preserves the complete approved art result.

Evidence:

- `Saved/V842_GrassArtLock/V842_Scalability_Comparison.png`
- `Saved/FFSmokeCaptures/V842_Final_LOW`
- `Saved/FFSmokeCaptures/V842_Final_MEDIUM`
- `Saved/FFSmokeCaptures/V842_Final_HIGH_Scalability`

## 11. Full Highland Runtime Review

Final runtime camera groups:

- PlayerStart
- Village Terrace
- Training Plateau
- Open Plains
- River Basin
- Main Forest Basin
- Cleft Path
- Secondary Shoulder
- South Gorge
- Event Pocket A
- Event Pocket B
- Event Pocket C
- Event Pocket D
- Wide Biome
- Top Down without labels
- Top Down clean route

These inherited V83.2 cameras are primarily topographic and some are high
altitude. Individual blades may become sub-pixel or cull at those heights.
Player-height V84.2 cameras are the authoritative grass-art judgment.

Evidence:

- `Saved/V842_GrassArtLock/V842_Final_16Zone_ContactSheet.png`
- `Saved/V842_GrassArtLock/V842_Labeled_TopDown_Overlay.png`
- `Saved/FFSmokeCaptures/V842_Final_Zones_A`
- `Saved/FFSmokeCaptures/V842_Final_Zones_B`
- `Saved/FFSmokeCaptures/V842_Final_Zones_C`
- `Saved/FFSmokeCaptures/V842_Final_Zones_D_TopDown`

## 12. Protected World Validation

Final commandlet protection report:

- PlayerStart actors: 1
- water actors: 23
- mountain actors: 27
- protected state unchanged: true
- temporary lookdev actors: 0
- final validation cameras: 9
- saved map: true
- saved packages: true

Status:

- PlayerStart transform: unchanged and safe
- mountain ring: unchanged
- river course and water logic: unchanged
- lake geometry and water logic: unchanged
- V83.2 terrain shape: unchanged
- village and forest layout: unchanged
- event logic: unchanged

The map binary changed only because it stores the approved V84.2 grass
bindings, generated GrassMap data, retained V84.1 lighting state, and final
validation cameras. No sculpting code or protected-system edit was executed.

## 13. Validation Results

### MapCheck

- result: PASS
- errors: 0
- warnings: 0
- authoritative log:
  `Saved/V842_GrassArtLock/V842_FinalLightingCameras_Commandlet_Retry.log`

### Editor Build

- result: PASS
- target: FantasyFrontierEditor Win64 Development
- actions: 4
- duration: 27.89 s
- log:
  `Saved/V842_GrassArtLock/V842_EditorBuild_FinalLightingCameras.log`

### Native GrassMap

- result: PASS
- workflow: full editor with real D3D11 SceneProxy, not commandlet/null RHI
- valid components: 110 / 110
- grass raster elements: 1,802,240
- grass weight types: 180
- grass weight bytes: 2,949,120
- non-zero weight samples: 1,454,242
- SaveMap: 1
- validation-only texture pool: 128 MB
- loaded target world reused to avoid duplicate full-map allocation
- report:
  `Saved/V842_EditorGrassMapBuild_Final.txt`
- log:
  `Saved/V842_GrassArtLock/V842_GrassMapBuild_FinalAuthoritative_Retry.log`

### Material And Error Scan

- result: PASS
- authoritative logs scanned: 22
- fatal: 0
- material fallback: 0
- ensure: 0
- known benign editor-only error: one `LogFab: EOS is not initialized`
- Fab was not used or modified
- report:
  `Saved/V842_GrassArtLock/V842_FinalValidation_LogScan.json`

### Package

- result: PASS
- BuildCookRun: 224.57 s
- exit code: 0
- serial safety: `-NoXGE`, one cooker, `MaxParallelActions=1`
- payload after validation, excluding runtime Saved output:
  62 files / 1.126 GiB
- archive:
  `Saved/PackageReady_HighlandWorldbuilding_v842_grass_art_lock`
- log:
  `Saved/V842_GrassArtLock/V842_Package_stdout.log`

The cooker compiled missing cached shader maps as expected and completed
successfully. No fallback material was emitted.

### Packaged Smoke

- result: PASS
- title screen: PASS
- creator open: PASS
- male/female switch: PASS
- preview mode switch: PASS
- preset save/load: PASS
- start game: PASS
- spawn: PASS
- movement: PASS
- dash and light/heavy attack: PASS
- Highland hill grounding: PASS
- Highland top-down capture: PASS
- fatal: 0
- log:
  `Saved/V842_GrassArtLock/V842_PackagedSmoke_stdout.log`

### Packaged Visual Parity

- result: PASS
- packaged close grass visually matches editor HIGH capture
- screenshot:
  `Saved/PackageReady_HighlandWorldbuilding_v842_grass_art_lock/Windows/FantasyFrontier/Saved/FFSmokeCaptures/V842_PackagedClose/00_V842_01_GrassClose.png`

The capture-only run used `-NoSound`; its music warning is expected and is not
the full packaged smoke result. The full packaged smoke separately proved
title, creator, and world music transitions.

## 14. Performance And Memory

HIGH long-run result:

- stationary window: 30 s
- movement window: 60 s
- total process duration: 117.01 s
- movement: PASS, 85.264 m
- actions: PASS
- smoke: PASS
- peak working set: 2.786 GB
- average working set: 1.551 GB
- peak private bytes: 5.405 GB
- average private bytes: 4.140 GB
- free physical RAM at start: 2.840 GB
- minimum free physical RAM: 0.010 GB
- free physical RAM after exit: 2.846 GB
- memory returned after exit: true
- fatal: 0
- Codex working-set trims: 3
- emergency Unreal trims: 1

RAM verdict:

- Unreal returned its memory after exit.
- The desktop still has dangerously low physical headroom under a full HIGH
  validation load.
- Low-memory serialization prevented a machine crash.
- Only one heavy process ran at a time.
- Codex was never terminated.

Reliable FPS, GPU, game-thread, and render-thread averages were not available
from the unattended D3D11 capture. No values were invented.

Evidence:

- `Saved/V842_GrassArtLock/V842_Performance_Memory_Evidence.png`
- `Saved/V842_GrassArtLock/V842_Performance_30Stationary_60Movement_HIGH_Metrics.json`

## 15. Required Screenshot Index

1. Front/back/side lighting:
   `V842_Lighting_Backface_Comparison.png`
2. V84.1 vs V84.2 close grass:
   `V842_Silhouette_BeforeAfter.png`
3. Medium silhouette:
   `V842_Silhouette_BeforeAfter.png`
4. Six-frame wind:
   `V842_Wind_ContactSheet.png`
5. Walking through grass:
   `V842_Final_GrassCore_Authoritative/01_V842_02_GrassWalking.png`
6. Three species:
   `V842_Patch_Species_Proof.png`
7. Patch transition player height:
   `V842_Final_GrassCore_Authoritative/04_V842_05_GrassPatchPlayer.png`
8. Patch transition top down:
   `V842_Final_PatchTopDown`
9-20. PlayerStart through Wide Biome:
   `V842_Final_16Zone_ContactSheet.png`
21. Top Down no labels:
   `V842_Final_16Zone_ContactSheet.png`
22. Top Down labeled:
   `V842_Labeled_TopDown_Overlay.png`
23. LOW/MEDIUM/HIGH:
   `V842_Scalability_Comparison.png`
24. Packaged close:
   package `V842_PackagedClose/00_V842_01_GrassClose.png`
25. Performance:
   `V842_Performance_Memory_Evidence.png`
26. Final contact:
   `V842_Final_ContactSheet.png`

Primary screenshot root:

`Saved/FFSmokeCaptures`

Report evidence root:

`Saved/V842_GrassArtLock`

## 16. Exact Production Files Changed

Tracked or force-added production files:

1. `Content/Maps/FF_Starter_Highland_Blockout.umap`
2. `Content/FantasyFrontier/Blockout/Materials/M_FF_Blockout_Grass_Green.uasset`
3. `Content/FantasyFrontier/Blockout/Materials/M_FF_Highland_GrassBlade_V842.uasset`
4. `Content/FantasyFrontier/Blockout/Materials/M_FF_Highland_Ryegrass_V842.uasset`
5. `Content/FantasyFrontier/Blockout/Materials/M_FF_Highland_Wildgrass_V842.uasset`
6. `Content/FantasyFrontier/Blockout/Materials/GT_FF_Highland_Grass_Filler_V842.uasset`
7. `Content/FantasyFrontier/Blockout/Materials/GT_FF_Highland_Grass_Medium_V842.uasset`
8. `Content/FantasyFrontier/Blockout/Materials/GT_FF_Highland_Grass_Tall_V842.uasset`
9. `Source/TP_ThirdPerson/FFStarterHighlandGrassNaturalizationCommandlet.cpp`
10. `Source/TP_ThirdPerson/TP_ThirdPerson.cpp`
11. `Source/TP_ThirdPerson/TP_ThirdPersonPlayerController.cpp`
12. `Source/TP_ThirdPerson/TP_ThirdPersonPlayerController.h`

Documentation and selected evidence:

13. `Saved/V842_GrassArtLock/Highland_GrassArtLock_v842_Report.md`
14. `Saved/V842_GrassArtLock/V842_FinalValidation_LogScan.json`
15. `Saved/V842_GrassArtLock/V842_WindMetrics.json`
16. `Saved/V842_GrassArtLock/V842_Lighting_Backface_Comparison.png`
17. `Saved/V842_GrassArtLock/V842_Silhouette_BeforeAfter.png`
18. `Saved/V842_GrassArtLock/V842_Wind_ContactSheet.png`
19. `Saved/V842_GrassArtLock/V842_Patch_Species_Proof.png`
20. `Saved/V842_GrassArtLock/V842_Scalability_Comparison.png`
21. `Saved/V842_GrassArtLock/V842_Final_16Zone_ContactSheet.png`
22. `Saved/V842_GrassArtLock/V842_Labeled_TopDown_Overlay.png`
23. `Saved/V842_GrassArtLock/V842_Performance_Memory_Evidence.png`
24. `Saved/V842_GrassArtLock/V842_Final_ContactSheet.png`

Generated logs, package output, backup files, and raw screenshots remain under
`Saved` and are not production source assets.

## 17. Rejected Changes And Iterations

- Candidate A: V84.1 baseline retained black card backs. Rejected.
- Broad leaf candidate: read as plastic foliage rather than meadow grass.
  Rejected.
- Extreme XY scaling: hid one angle and failed another. Rejected.
- Density increase as camouflage: rejected.
- Initial lighting proof after map invalidation: produced no grass and was
  archived at `Saved/FFSmokeCaptures/V842_Rejected_NoGrass_LightingProof`.
- First final commandlet attempt: safely aborted before save when RAM became
  critical. Retry passed.
- `grass.CullDistanceScale=2.0`: diagnostic only; proved the distant smooth
  field was not cull pop-in. Not retained.
- Per-instance fade workaround: unnecessary and not implemented.
- Original Titan asset modification: not performed.
- New paid or external asset: not downloaded.

## 18. Known Issues

1. Unattended D3D11 does not provide reliable average FPS, GPU, game-thread, or
   render-thread telemetry in this workflow.
2. Physical RAM headroom reached 0.010 GB during HIGH validation. Serialization
   and working-set trims prevented a crash, but future heavy work should retain
   the same one-process discipline.
3. High-altitude inherited zone cameras do not represent player-height grass
   quality and can make blades sub-pixel or culled.
4. The tall layer is an accepted wildgrass accent using the narrow blade mesh,
   not a bespoke modeled seed-head mesh.
5. The editor-only GrassMap log contains one benign
   `LogFab: EOS is not initialized`; Fab was not touched.
6. Capture-only packaged visual proof uses `-NoSound`, so its audio warning is
   expected. Full packaged smoke separately passes music.
7. `TESTING.md` NPC interaction and enemy combat remain blocked by absent
   approved actors in the protected Highland baseline.

## 19. NOT DONE

- V85 geomorphology
- terrain sculpting or terrain-shape changes
- forest ecology or tree placement
- village work
- flowers and ferns as a biome pass
- resources
- waterfalls or water-system work
- NPC integration
- quest integration
- enemy integration
- combat-content integration
- UI or character changes
- bespoke seed-head mesh authoring
- reliable GPU/FPS capture unavailable from unattended D3D11
- `TESTING.md` steps 11 and 12

## 20. Placeholder Assets Still In Use

- The wider Highland world remains an active terrain/worldbuilding blockout.
- Existing village, forest, resource, waterfall, NPC, quest, and enemy content
  is not presented as finished because those passes have not been built here.
- The V84.2 grass itself is no longer treated as a temporary lookdev candidate;
  it is the locked meadow foundation.
- Existing project meshes are reused as approved production building blocks,
  and original reference assets remain unmodified.

This sprint does not claim the Highland map is a finished tutorial map or a
finished AAA biome.

## Enforcement

- Rule violations: none in the V84.2 implementation scope.
- No protected world system was intentionally modified.
- No new mountain, water, forest, village, resource, NPC, quest, combat, UI, or
  character work was started.
- No original Titan asset was modified.
- No temporary lookdev actors remain.
- No paid content was downloaded.
- No push to main was performed.
- Output quality below the grass-art target would have been reported as
  PARTIAL; rejected candidates were not promoted.

## Testing Enforcement

`TESTING.md` was read and executed as far as the current protected content
allows.

Passed:

- editor build
- Windows package
- packaged executable launch
- Title Screen to Start
- character creator open
- male/female switching
- preview mode switching
- preset save/load
- start game
- tutorial spawn
- character movement
- dash and light/heavy attack
- Highland grounding and capture smoke

Blocked:

- interact with at least one NPC
- trigger combat with one enemy

Reason:

The approved Highland baseline has no approved NPC or enemy actor available for
those two checks, and V84.2 explicitly forbids adding NPC/combat content.

Testing enforcement verdict:

**PARTIAL due pre-existing content blocker.**

This is the only reason the full project sprint cannot be represented as a
completely closed `TESTING.md` pass. It is not a grass regression.

## Asset Import Enforcement

- No character or creature asset was imported.
- No new external asset was imported.
- `IMPORT_QA.md` was therefore not triggered.
- Existing project foliage meshes were reused without modifying their original
  assets.

## 21. Honest Final Verdict

### Grass Art Lock

**PASS**

The V84.2 grass meets the focused art-lock conditions:

- black backsides substantially eliminated
- medium cards corrected at normal player distance
- bases remain anchored
- upper motion visibly and measurably stronger
- wind phase varies
- patch boundaries softened and decorrelated
- three visual layers readable
- tall accents sparse but visible
- mid-distance acceptable
- LOW acceptable
- MEDIUM viable
- HIGH stable
- exclusions preserved
- PlayerStart safe
- mountain ring unchanged
- river and lake unchanged
- terrain shape unchanged
- MapCheck, build, GrassMap, package, smoke, material scan, and fatal scan pass
- temporary candidates removed

### Whole Sprint Closure Under `AGENTS.md`

**PARTIAL**

The V84.2 implementation and its grass quality are approved, committed, and
safe to use as the grass baseline. Formal project-wide completion remains
partial only because the protected map cannot currently satisfy the unrelated
NPC and enemy checks in `TESTING.md`.

The next art phase may use this grass baseline, but it must not claim that the
entire Highland map or all game systems are finished.
