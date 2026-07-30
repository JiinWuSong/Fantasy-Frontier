# Fantasy Frontier Highland Worldbuilding V84.1

## AAA Grass Naturalization, Lighting and Species Diversity Pass

Date: 2026-07-30

Branch: `feature/highland-v841-grass-naturalization`

V84 baseline commit: `577254f`

Backup:
`Saved/V841_PreGrassNaturalizationBackup_20260730_131904`

Final verdict: **PARTIAL**

Technical gate verdict: **PASS**

Visual direction verdict: **PARTIAL**

## Executive Verdict

V84.1 keeps the validated V84 native LandscapeGrass foundation and changes it
into a three-layer, quality-scaled meadow system. It adds differentiated short,
medium and tall species, layered wind, restrained two-sided foliage lighting,
organic multi-scale patch masks, warmed runtime validation, LOW/HIGH evidence,
a 30 second stationary plus 60 second movement test, and a fresh packaged build.

The pass materially corrects the largest V84 failures:

- the exposed landscape no longer clips to white through an old emissive chain;
- the grass no longer reads as one globally uniform species;
- the original parallel density bands were replaced with smooth rotated
  multi-scale value-noise masks;
- the grass has visible per-instance phase and four-frame runtime motion;
- highlights are green and restrained instead of white/neon;
- LOW, MEDIUM and HIGH density/cull settings are production-authored;
- river, lake, routes, PlayerStart and the mountain ring remain protected;
- the production map contains no temporary look-development actors.

V84.1 is not called a visual PASS because the close runtime view still exposes
stylized card-like blade silhouettes and dark card backs. The result is a
stronger living meadow foundation, but it is not yet Titan-quality final grass.
Player interaction was also rejected for this sprint because Highland does not
have a proven runtime updater for Titan's MPC/RT interaction chain.

## Gate Summary

| Gate | Result | Evidence |
| --- | --- | --- |
| V84 baseline safety | PASS | commit `577254f`, pushed before V84.1 |
| V84.1 backup | PASS | timestamped map/material/source/report backup |
| Titan grass audit | PASS | focused mesh/material/dependency/parameter logs |
| A/B/C runtime lookdev | PASS | 15 internal iterations, selected comparison |
| Lookdev cleanup | PASS | zero production lookdev actors; rejected MI archived |
| Final native rollout | PASS | three LandscapeGrass inputs |
| MapCheck | PASS | 0 errors, 0 warnings |
| Editor build | PASS | `Result: Succeeded`, 31.07 s |
| Native GrassMap | PASS | 110/110 components, SaveMap=1 |
| HIGH runtime | PASS | 19 requested cameras, FFSmoke PASS |
| LOW runtime | PASS | 9 requested cameras, FFSmoke PASS |
| Wind proof | PASS | four warmed frames with measured image motion |
| Long performance smoke | PASS | 30 s stationary plus 60 s movement |
| Windows package | PASS | BuildCookRun ExitCode 0 |
| Packaged smoke | PASS | complete protected Highland smoke path |
| Material/crash scan | PASS | no blocking material/fatal/assert/GPU/smoke hit |
| NPC interaction/enemy combat | NOT DONE | protected Highland has no approved actors |
| Reliable FPS/frame timing | NOT DONE | unattended D3D11 timing was not trustworthy |
| Full Titan visual parity | PARTIAL | card silhouette remains visible close-up |

## 1. Git, Baseline and Backup

- Confirmed the starting V84 branch:
  `feature/highland-v84-titan-groundcover-surface`.
- Committed the validated baseline:
  `V84 establish native Highland groundcover foundation`.
- Baseline commit: `577254f`.
- Pushed the V84 baseline branch.
- Created and switched to:
  `feature/highland-v841-grass-naturalization`.
- Created:
  `Saved/V841_PreGrassNaturalizationBackup_20260730_131904`.
- No work was pushed to `main`.

## 2. Titan Reverse-Engineering Findings

Focused references inspected:

- `/Game/Landscape/LGT/LGT_Grass`
- `/Game/Environment/Foliage/Meshes/SM_GrassBlade`
- `/Game/Environment/Foliage/Materials/M_GrassBlade`
- `/Game/Environment/Foliage/Materials/MI_GrassBlade*`
- `/Game/Environment/Foliage/Grass/Ryegrass_Grass_A`
- `/Game/Environment/Foliage/Grass/Ryegrass_Grass_C`
- `/Game/Environment/Foliage/Grass/Ryegrass_Grass_D`
- `/Game/Environment/Foliage/Grass/Ryegrass_Rye_Tuft`
- `/Game/Environment/Foliage/Grass/Ryegrass_Stalk_Green`
- `/Game/Environment/Foliage/Grass/Titan_Lilium_Reedgrass`
- `/Game/Environment/Clifftop/Materials/Foliage/MI_Clifftop_Ryegrass`
- `/Game/Environment/_Core/Materials/Functions/MF_WindGerst`
- `/Game/Environment/_Core/Materials/Functions/MF_WindMovement`
- `/Game/Environment/_Core/Materials/Functions/MF_FoliageInteraction`
- `/Game/Blueprint/FoliageInteraction/MPC_Player`
- `/Game/Blueprint/FoliageInteraction/RT_Player`
- `/Game/Landscape/RVT/RVT_Titan_D`

Titan `LGT_Grass` uses two `SM_GrassBlade` varieties at densities 25 and 200,
grid placement, full jitter, random rotation, surface alignment, and culling
from 2,000 to 10,000 cm. This proves that the Titan base read comes from a
dense-short plus sparse-taller hierarchy, not one grass card scaled globally.

Titan `M_GrassBlade` depends on landscape RVT color, two wind functions and the
MPC/RT player interaction chain. Its useful transferable principles are:

- grounded color coupling;
- stable roots and height-based bending;
- coherent wind direction with phase variation;
- separate whole-blade and higher-frequency motion;
- quality and distance controls.

The complete Titan dependency graph was not copied into Highland. The original
Titan assets were not modified. V84.1 creates Fantasy Frontier-owned materials
that adapt the safe visual principles without requiring TitanMain runtime state.

## 3. Roof and Thatch Conclusion

The roof/thatch families are authored to conform to roof surfaces and silhouettes.
Their geometry, density and orientation are not safe landscape-ground grass.
They were rejected as production meshes.

Only general principles were retained:

- anchored roots;
- flexible upper portions;
- random phase;
- restrained layered wind;
- silhouette variation through mixed heights.

No roof-specific mesh was rolled out as Highland groundcover.

## 4. Candidate A Findings

Candidate A was the closest V84/Titan baseline:

- mesh: `SM_GrassBlade`;
- material: V84 blade material;
- 1,650 controlled lookdev instances;
- broad scale variation;
- close, player, mid, far and wind views captured.

Strengths:

- stable and inexpensive;
- clear Titan baseline relationship;
- good ground fill.

Weaknesses:

- too upright;
- too uniform;
- bright/rigid card read remained;
- insufficient species hierarchy.

Verdict: rejected as the final standalone method.

## 5. Candidate B Findings

Candidate B combined:

- curved Ryegrass soft mesh;
- new V84.1 ryegrass material;
- 750 curved instances;
- 650 short naturalized blade instances.

Strengths:

- visibly softer upper silhouette;
- darker foliage response;
- better wind deformation than Candidate A.

Weaknesses:

- too coarse and gappy at the tested spacing;
- dominant broad cards;
- weak distance continuity.

Verdict: useful wind/curvature reference, rejected as standalone rollout.

## 6. Candidate C Findings

Candidate C combined:

- 1,400 short naturalized `SM_GrassBlade` instances;
- 260 `Ryegrass_Rye_Tuft` medium instances;
- 14 `Ryegrass_Stalk_Green` tall accent instances.

Strengths:

- strongest short/medium/tall hierarchy;
- best patch and silhouette diversity;
- strongest meadow rhythm;
- retained readable ground;
- acceptable runtime behavior.

Weaknesses:

- some card backs remain dark close-up;
- tall accents require sparse masks to avoid a field-wide reed read.

Verdict: selected as a controlled B/C hybrid and translated into native
LandscapeGrass production assets.

Candidate evidence:
`Saved/V841_GrassNaturalization/V841_Candidate_Comparison_Selected.png`

## 7. Selected Final Method

The final system is native LandscapeGrass, not a manual HISM carpet.

The existing V84 base density remains responsible for:

- water exclusion;
- exposed riverbed exclusion;
- slope exclusion;
- route and protected-center exclusion;
- approved Highland gameplay masks.

V84.1 layers three organic patch masks on top of that base. The masks use
rotated, smooth two-scale value noise. This replaced the rejected sinusoidal
version that created visible parallel grass bands.

The final LandscapeGrass output contains:

- `FF_V841_Filler`
- `FF_V841_Medium`
- `FF_V841_Tall`

## 8. Meshes Used

Short filler:
`/Game/Environment/Foliage/Meshes/SM_GrassBlade`

Medium meadow:
`/Game/Environment/Foliage/Grass/Ryegrass_Rye_Tuft`

Tall accents:
`/Game/Environment/Foliage/Grass/Ryegrass_Stalk_Green`

These are existing licensed project/Titan source meshes. The original meshes
were not modified.

## 9. Materials Used

New Fantasy Frontier materials:

- `M_FF_Highland_GrassBlade_V841`
- `M_FF_Highland_Ryegrass_V841`

Landscape integration:

- modified `M_FF_Blockout_Grass_Green`;
- disconnected the obsolete landscape emissive output that caused white ground;
- preserved the validated V84 density/exclusion chain;
- added three V84.1 LandscapeGrass patch inputs.

Blade material:

- opaque two-sided foliage;
- roughness 0.97;
- specular 0.00;
- AO 0.96;
- restrained subsurface color;
- dark/soft/warm/cool green variation;
- no emissive contribution.

Ryegrass material:

- masked two-sided foliage;
- opacity clip 0.32;
- roughness 0.96;
- specular 0.01;
- AO 0.97;
- restrained subsurface transmission;
- existing Ryegrass albedo used through a Fantasy Frontier material.

## 10. Wind Implementation

Both materials use a material-quality switch.

LOW:

- simplified low-frequency sway;
- anchored height mask;
- reduced motion detail.

MEDIUM/HIGH/EPIC:

- stable lower blade;
- random resting lean;
- whole-blade low-frequency sway;
- stronger upper-half bend;
- tip flutter;
- coherent gust field;
- crosswind contribution;
- `PerInstanceRandom` phase/amplitude variation.

Production WPO disable distances:

- filler: 5,500 cm;
- medium: 7,200 cm;
- tall: 8,000 cm.

Four-frame wind evidence measured normalized image RMSE of approximately:

- frame 2 vs frame 1: 0.0570;
- frame 3 vs frame 1: 0.0736;
- frame 4 vs frame 1: 0.0762.

This proves runtime motion and non-identical frames.

Wind sheet:
`Saved/V841_GrassNaturalization/V841_FinalNatural_WindSequence.png`

## 11. Blade Curvature

The base remains planted through a height mask. Resting lean increases toward
the upper blade, and dynamic sway is multiplied by the anchored/upper masks.
The tip receives the strongest additional flutter.

The final result is substantially less rigid than V84, but the close screenshot
still exposes broad card silhouettes. This is the main visual reason for the
PARTIAL verdict.

## 12. Lighting Corrections

Final production lookdev balance:

- directional light intensity: 4.6;
- skylight intensity: 1.15;
- unbound V84.1 post-process volume;
- auto exposure min/max: 1.0/1.0;
- exposure bias: 1.25;
- bloom: 0.02.

The old landscape emissive connection was removed. Grass color, roughness,
specular and two-sided subsurface were rebalanced together.

Runtime verdict:

- white/neon clipping substantially corrected;
- no emissive-looking white field;
- sun-facing blades remain green;
- shadow faces are dark but not fully black.

## 13. Species Hierarchy and Production Values

| Layer | Density | Scale X | Scale Y | Scale Z | Cull start/end | WPO off |
| --- | ---: | --- | --- | --- | --- | ---: |
| Filler | 220 | 0.68-1.05 | 0.66-1.02 | 0.22-0.46 | 2,500/9,000 | 5,500 |
| Medium | 54 | 0.42-0.72 | 0.40-0.70 | 0.52-0.85 | 3,000/11,500 | 7,200 |
| Tall | 5 | 0.22-0.36 | 0.21-0.35 | 0.40-0.65 | 4,000/14,000 | 8,000 |

All layers use grid placement, full jitter, random rotation, free XYZ scaling,
surface alignment, no dynamic/contact shadows and density scaling.

## 14. Scalability

LOW:

- density: 38 percent;
- cull distance: 65 percent;
- low material quality wind.

MEDIUM:

- density: 68 percent;
- cull distance: 82 percent;
- full normal sway with balanced distance.

HIGH/EPIC/CINEMATIC:

- density: 100 percent;
- cull distance: 100 percent;
- full layered wind.

The first LOW test that changed console values after load was rejected. The
authoritative LOW capture applies quality overrides before map load.

Validated comparison:
`Saved/V841_GrassNaturalization/V841_LOW_vs_HIGH_Comparison_Validated.png`

## 15. Zone Distribution

The V84 base mask continues to protect water, riverbed, steep slopes, routes,
PlayerStart and gameplay centers. V84.1 adds layer-specific patch behavior.

- PlayerStart remains readable and grounded.
- Village Terrace and Training Plateau retain lower/shorter playable centers.
- Open Plains is the primary mixed meadow showcase.
- River Basin keeps water and exposed bed clear while bank vegetation remains.
- Main Forest Basin uses lower/darker meadow coverage.
- Cleft Path center remains readable.
- Secondary Shoulder and South Gorge expose more ground.
- Event pockets retain playable centers and denser natural edges.

The required camera set confirms the protected route and zone layout remained
unchanged. At long distance the grass intentionally culls, so some wide views
show the still-blockout landscape rather than a full-map carpet.

## 16. Patch and Distance Correction

The initial full rollout used sinusoidal macro masks and produced obvious
parallel bands. Screenshot review failed that iteration.

Correction:

- replaced sine bands with smoothed value noise;
- rotated the second noise octave independently per layer;
- used different scales and thresholds for filler, medium and tall;
- kept overlapping transitions;
- rebuilt all native GrassMaps;
- recaptured close, mid, wide, LOW and HIGH views.

Final verdict:

- obvious linear stripes removed;
- patch boundaries are substantially more organic;
- no checkerboard or uniform tall-grass carpet;
- the close patch rhythm remains visibly procedural at proof-level fidelity,
  but no longer fails as a repeated stripe mask.

## 17. Native GrassMap Result

Full-editor GrassMap build:

- Landscape components: 110;
- registered/render-state/scene-proxy/material-grass: 110/110;
- valid GrassData: 110/110;
- grass elements: 1,802,240;
- grass weight types: 142;
- grass weight bytes: 2,326,528;
- non-zero weight samples: 1,097,912;
- outdated GrassMaps: 0;
- SaveMap: 1.

Result:
`PASS - Full-editor SceneProxy gate opened and native GrassMap data generated/saved`.

## 18. Performance Results

HIGH runtime, 19 cameras:

- duration: 52.61 s;
- peak Unreal working set: 2.992 GB;
- average working set: 2.235 GB;
- lowest available physical RAM: 0.015 GB;
- exit code: 0;
- FFSmoke: PASS.

LOW validated runtime, 9 cameras:

- duration: 34.56 s;
- peak Unreal working set: 2.585 GB;
- average working set: 2.054 GB;
- lowest available physical RAM: 0.163 GB;
- exit code: 0;
- FFSmoke: PASS.

Long performance smoke:

- stationary: 30.0 s;
- movement: 60.0 s;
- Move Character: PASS;
- Dash plus Light/Heavy Attack: PASS;
- overall FFSmoke: PASS;
- observed spot sample: 1.73 GB working set, 4.27 GB private, 0.60 GB
  physical RAM free;
- all Unreal memory returned after normal exit.

No reliable GPU/game/render thread timing was produced by the unattended D3D11
workflow. FPS and frame time are reported as NOT DONE rather than invented.

Performance sheet:
`Saved/V841_GrassNaturalization/V841_Performance_Evidence.png`

## 19. RAM and Process Safety

- Used one heavy workflow at a time.
- No editor/package/smoke overlap.
- Editor build used `MaxParallelActions=1`.
- Package used `-NoXGE -NumCookersToSpawn=1 -MaxParallelActions=1`.
- Unreal's cook internally spawned shader workers; no second user workflow ran.
- Package logged asset-compile memory pressure, but completed with ExitCode 0.
- No crash reporter or orphan Unreal process remained.
- RAM returned after every completed Unreal/UAT/game process.
- No paid content was downloaded.

The desktop baseline itself remains memory-heavy. During audit, Codex/ChatGPT
processes and large Windows kernel/driver pools accounted for a substantial
portion of the non-Unreal baseline. NVIDIA overlay helpers respawned when closed,
so repeated termination was rejected as ineffective.

## 20. PlayerStart and Protected World Status

Final commandlet protection audit:

- PlayerStart actors: 1;
- water actors: 23;
- mountain actors: 27;
- protected state unchanged: true.

Runtime spawn:
`(89682.8, 124267.9, 3670.3)`

PlayerStart was not moved in V84.1.

Mountain ring:
unchanged.

River and lake:
unchanged and grass-excluded.

Macro terrain/traversal:
unchanged.

No V85 geomorphology work was started.

## 21. Validation Results

### MapCheck

PASS:
0 errors, 0 warnings.

### Editor Build

PASS:
`FantasyFrontierEditor Win64 Development`, 31.07 s, `Result: Succeeded`.

### Package

PASS:

- BuildCookRun: 187.62 s;
- cooked packages: 1,516;
- UnrealPak/IoStore: success;
- AutomationTool ExitCode: 0;
- archive:
  `Saved/PackageReady_HighlandWorldbuilding_v841_grass_naturalization`;
- archive size: 1.141 GB.

### Packaged Smoke

PASS:

- title screen;
- title music;
- character creator;
- creator music;
- female/male switch;
- preview modes;
- preset save/load;
- start game;
- spawn;
- world music transition;
- movement;
- dash plus light/heavy attack;
- Highland grounding;
- requested V84.1 grass camera;
- overall FFSmoke PASS.

### Material and Crash Scan

PASS:

- no fatal error;
- no assertion;
- no GPU crash/device loss;
- no FFSmoke failure;
- no material compile failure;
- no default/fallback material blocker.

Non-blocking warnings:

- editor-only Fab `EOS is not initialized`;
- package cook memory-pressure warnings;
- previously known optional content warnings remain unrelated to V84.1.

Full scan:
`Saved/V841_GrassNaturalization/V841_Final_LogScan.txt`

## 22. Screenshots

Raw final HIGH runtime:
`Saved/FFSmokeCaptures/V841_FinalNatural_HIGH`

Raw validated LOW runtime:
`Saved/FFSmokeCaptures/V841_FinalNatural_LOW_Validated`

Raw final wind:
`Saved/FFSmokeCaptures/V841_FinalNatural_WindSequence`

Raw warmed top-down:
`Saved/FFSmokeCaptures/V841_FinalNatural_TopDownWarm`

Raw packaged smoke:
`Saved/PackageReady_HighlandWorldbuilding_v841_grass_naturalization/Windows/FantasyFrontier/Saved/FFSmokeCaptures/V841_PackagedSmoke`

Key sheets:

- `V841_Candidate_Comparison_Selected.png`
- `V841_FinalNatural_WindSequence.png`
- `V841_FinalNatural_Validated_ContactSheet.png`
- `V841_LOW_vs_HIGH_Comparison_Validated.png`
- `V84_vs_V841_Runtime_Comparison.png`
- `V841_Runtime_TopDown_Labeled_Warm.png`
- `V841_Performance_Evidence.png`

The grey rectangles in late captures from the 19-camera HIGH run were streaming
artifacts under near-zero available RAM. Fresh isolated top-down captures with
five-second warmup contain no rectangles and are the authoritative images.

## 23. Exact Files Changed

Production assets:

- `Content/Maps/FF_Starter_Highland_Blockout.umap`
- `Content/FantasyFrontier/Blockout/Materials/M_FF_Blockout_Grass_Green.uasset`
- `Content/FantasyFrontier/Blockout/Materials/M_FF_Highland_GrassBlade_V841.uasset`
- `Content/FantasyFrontier/Blockout/Materials/M_FF_Highland_Ryegrass_V841.uasset`
- `Content/FantasyFrontier/Blockout/Materials/GT_FF_Highland_Grass_Filler_V841.uasset`
- `Content/FantasyFrontier/Blockout/Materials/GT_FF_Highland_Grass_Medium_V841.uasset`
- `Content/FantasyFrontier/Blockout/Materials/GT_FF_Highland_Grass_Tall_V841.uasset`

Source:

- `Source/TP_ThirdPerson/FFStarterHighlandGrassNaturalizationCommandlet.h`
- `Source/TP_ThirdPerson/FFStarterHighlandGrassNaturalizationCommandlet.cpp`
- `Source/TP_ThirdPerson/FFTitanGrasslandGrassAuditCommandlet.cpp`
- `Source/TP_ThirdPerson/TP_ThirdPersonPlayerController.cpp`

Persistent report:

- `Saved/V841_GrassNaturalization/Highland_GrassNaturalizationPass_v841_Report.md`

Generated validation artifacts:

- V84.1 audit, commandlet, build, GrassMap, runtime, package and smoke logs;
- V84.1 screenshot folders and comparison sheets;
- packaged V84.1 archive.

Rejected lookdev artifact:

- `MI_FF_Highland_MeadowCluster_V841.uasset` was never referenced or cooked;
- it was removed from `Content` and preserved only under
  `Saved/V841_GrassNaturalization/RejectedLookdev`.

## 24. Known Issues

- Close blades still reveal broad stylized cards and dark backs.
- The final meadow is visibly improved but not Titan-quality final vegetation.
- Reliable FPS, game-thread, render-thread and GPU timing were not captured.
- Player interaction is not implemented.
- Full zone-specific moisture/material coupling remains based on V84 masks plus
  V84.1 procedural patching, not a dedicated authored ecology map.
- Wide zone views still expose the intentionally unfinished terrain blockout
  after grass culls.
- Package cook runs very close to the machine's physical RAM limit.
- The full HIGH 19-camera top-down images suffered streaming artifacts under
  memory pressure; isolated warmed captures are clean.

## 25. NOT DONE

- No player foliage displacement/return system.
- No new external or paid grass asset import.
- No final custom curved filler mesh re-authoring.
- No reliable FPS/frame-time/GPU benchmark.
- No NPC interaction test in Highland.
- No enemy combat trigger test in Highland.
- No forest pass.
- No village pass.
- No resource pass.
- No waterfall/water-system pass.
- No terrain/mountain/geomorphology work.
- No V85 work.

## 26. Placeholder Assets Still In Use

- The default Epic mannequin in automated captures is a placeholder test
  character, not a finished hero character.
- Several Highland interior zones remain terrain/surface blockout, not a
  finished tutorial map.
- Validation cameras are test infrastructure, not world art.
- The V84.1 grass is a production-capable foundation but the close blade shape
  remains proof-level relative to the final Titan visual target.

## Enforcement

- No protected terrain, mountain, river, lake, PlayerStart, gameplay, UI, NPC,
  quest, combat, TitanMain, Kashkeh, Fab or CharacterImportLab work was started.
- No original Titan source asset was modified.
- No paid asset was downloaded.
- No full-map manual HISM carpet was created.
- Temporary lookdev actors were removed before final rollout.
- The rejected lookdev material was removed from production `Content`.
- The visual quality target is below full Titan parity, so this report explicitly
  marks V84.1 PARTIAL.
- No project rule was silently bypassed.

## Testing Enforcement

- `TESTING.md` was read.
- Editor build, Windows package, executable launch, title flow, creator flow,
  gender/preview switching, preset save/load, start, spawn, movement, dash,
  light/heavy attacks and Highland grounding passed.
- NPC interaction and enemy combat were not run because the protected Highland
  smoke route contains no approved V84.1 NPC/enemy implementation and explicitly
  stops after terrain validation.
- Therefore the full `TESTING.md` matrix is not complete, and the sprint is not
  declared a full PASS.

## Asset Import Enforcement

- No external character or creature asset was imported.
- No paid or unlicensed asset was downloaded.
- Existing project/Titan grass meshes were referenced without modifying their
  source assets.
- New Fantasy Frontier assets are materials and GrassTypes created inside the
  project namespace.
- `IMPORT_QA.md` character/creature integration gates were not triggered.

## Final Recommendation

Keep V84.1 as the stable native meadow foundation.

Before calling the grass final, run a narrow follow-up dedicated to:

- authoring or selecting a better curved short-filler silhouette;
- reducing dark card-back read at close distance;
- measuring GPU/game/render frame time with a stable profiler setup;
- testing player interaction only after a safe Highland runtime updater exists.

Do not restart the V84 pipeline and do not begin V85 inside this sprint.
