# Fantasy Frontier - Highland V84 Titan Groundcover + Surface Readability

Date: 2026-07-29

## Verdict

Overall sprint verdict: **PARTIAL**

Technical verdict: **PASS**

Visual verdict: **PARTIAL**

V84 establishes a working native LandscapeGrass foundation, restores visible
meadow coverage in validated areas, adds surface-color and route masks, survives
build/package/smoke, and keeps the protected Highland systems stable.

V84 is not a visual PASS because the final runtime review still shows:

- overly bright/white highlights on grass blades;
- broad pale or overexposed blockout surfaces in several zone views;
- weak low-groundcover diversity outside the dedicated meadow proof;
- surface zones that are more distinct than V83.2 but not yet a cohesive
  Titan/Project Heiken fantasy valley;
- no measured frame-time/FPS capture, despite acceptable runtime memory behavior.

The sprint is therefore not declared final and V85 should not treat this as
finished surface art without a focused V84.1 visual correction.

## Branch And Safety

- Branch: `feature/highland-v84-titan-groundcover-surface`
- Backup:
  `Saved/V84_PreTitanGroundcoverSurfaceBackup_20260729_181110`
- Backup contents: 45 files, 10.93 MB.
- Heavy-process policy: one Unreal/UAT/game process at a time.
- Editor build: `MaxParallelActions=1`.
- Package: `-NoXGE -NumCookersToSpawn=1 -MaxParallelActions=1`.
- No paid content was downloaded.
- No automatic push was performed.

## Audit Findings

### Why Grass Was Missing

- The Highland map had 110 Landscape components.
- `GT_FF_Blockout_LandscapeGrass` existed, but its effective density was zero
  and the active Landscape material did not provide a production
  `LandscapeGrassOutput`.
- The visible historical A/B grass proof contained only 2,070 manual test
  instances. It was not a production carpet.
- V83.2 had no useful painted Landscape weight allocations available for a
  full native rollout.
- A historical full-map manual HISM route could create millions of instances
  and was rejected because it is not memory-safe for this machine.

### Titan Grass Findings

- Titan `LGT_Grass` uses two grass varieties:
  - dense filler: density 200;
  - tall accent: density 25.
- Both use `/Game/Environment/Foliage/Meshes/SM_GrassBlade`.
- Titan culling reference: 2,000-10,000 cm.
- Titan scale reference:
  - filler: X 1.0-1.5, Y 1.0, Z 0.3-0.7;
  - accent: X 1.0-2.0, Y 1.0, Z 0.5-1.2.
- The Titan material depends on Titan RVT context and is unsafe as a direct
  Highland material reuse.

### Roof/Thatch Versus Landscape Grass

- The roof/thatch family is authored roof geometry using the
  Thatch/VolumeFoliage material family.
- It is not the same system as Titan LandscapeGrass.
- Direct landscape reuse would introduce unsuitable orientation, silhouette,
  density and performance behavior.
- It was retained as a quality/material reference only.

## Chosen Method

V84 uses a Highland-owned hybrid native LandscapeGrass solution:

- existing Titan `SM_GrassBlade` mesh;
- new Highland-owned non-RVT blade material;
- two native LandscapeGrass varieties;
- runtime-streamed grass instead of a manual full-map HISM carpet;
- painted layers when available;
- procedural position/slope fallback where V83.2 has no allocations;
- explicit river, lake, pond, route, village, training and event exclusions.

No new external asset was imported.

## Surface Style Contract

- Village Terrace: warm safe green; reduced density in future building pads.
- Training Plateau: drier and clearer; lower grass density in combat center.
- Open Plains: bright meadow with macro green variation.
- River Basin: cooler/wetter edge; no grass in riverbed or water masks.
- Main Forest Basin: richer green groundwork; no forest trees in V84.
- Cleft Path: warmer worn route with reduced center grass.
- Secondary Shoulder: drier foothill support.
- South Gorge: warmer sediment/danger tone.
- Event Pockets: lower center density and stronger edge readability.

The contract is saved at `Saved/V84_SurfaceStyleContract.md`.

## Implemented Changes

### Grass Material

- Two-sided foliage shading.
- Dark root-to-tip color variation.
- Broad, mid-frequency and per-instance color variation.
- Restrained foliage subsurface.
- Roughness 0.92 and specular 0.035.
- Root-fixed two-frequency wind.
- WPO disabled at distance for scalability.

### Grass Type

- Filler density: 200.
- Accent density: 25.
- Start/end cull: 2,000/10,000 cm.
- Filler WPO disable distance: 6,500 cm.
- Accent WPO disable distance: 7,500 cm.
- Density scaling enabled.
- Grid placement, jitter 1.0, random rotation and surface alignment enabled.
- No dynamic/contact shadows from individual blades.

### Landscape Surface

- Preserved the previous V82/V83 base-color graph.
- Added macro cool/warm/rich/open green modulation.
- Added zone-aware village/training/open-plains/forest/shoulder/gorge tinting.
- Added route-wear, wet-bank and water exclusion logic.
- Added native `LandscapeGrassOutput`.
- Added an organic interior mask and slope cutoff.
- Added explicit river/lake/pond exclusions.
- Added route, village, training and event density reductions.

### Validation

- Added V84 grass close and grass distance validation cameras.
- Added optional smoke camera warmup and no-UI screenshot controls.
- Default runtime behavior is unchanged when those validation flags are absent.

## PlayerStart Safety

A real pre-existing spawn safety issue was detected:

- old PlayerStart: `(89682.8, 124267.9, -3203.4)`;
- Landscape ground at the same XY: `Z=3572.3`;
- corrected PlayerStart: `(89682.8, 124267.9, 3682.3)`;
- XY did not change;
- packaged runtime resolved safely to
  `(89682.8, 124267.9, 3670.3)`;
- five runtime Landscape grounding probes passed.

This is the only protected actor adjustment and is allowed by the V84 prompt
because the spawn safety failure was proven and explicitly reported.

## Protected State

- Water actors: 23, unchanged.
- Mountain baseline actors: 27, transform hash unchanged.
- `Water_Lake_1`: unchanged at `(46806.6, 136433.6, -4061.4)`.
- Mountain ring: unchanged.
- River/lake logic: unchanged.
- No TitanMain or Kashkeh map edit.
- No village, forest-tree, resource, boss, waterfall, NPC, quest, combat, UI or
  character-content implementation.

## Zone-By-Zone Visual Review

| Zone | Verdict | Runtime finding |
|---|---|---|
| PlayerStart | PARTIAL | Grass is visible and spawn is safe; pale terrain remains dominant. |
| Village Terrace | PARTIAL | Warmer zone tint exists; future pads remain broad and blockout-like. |
| Training Plateau | PARTIAL | Center is kept readable and less dense; surface identity is still weak. |
| Open Plains | PARTIAL | Meadow color is improved; grass proof is strong locally, not consistently in wide views. |
| River Basin | PARTIAL | Water exclusion works and bank is cooler; white/hard transition remains. |
| Main Forest Basin | PARTIAL | Richer green base exists; no low forest-floor diversity yet. |
| Cleft Path | PARTIAL | Worn route and lower grass center improve direction; transition remains synthetic. |
| Secondary Shoulder | PARTIAL | Drier foothill read exists; view remains empty/overbright. |
| South Gorge | PARTIAL | Warmer sediment cue exists; visual finish is still blockout. |
| Event Pockets | PARTIAL | Density reduction masks exist; boundaries are too subtle without future content. |
| Grass close | PARTIAL | Dense two-layer blades and wind work; highlights are too white/neon. |
| Grass distance | PARTIAL | Coverage and culling work; patterning and lighting need another art pass. |
| Top down | PARTIAL | Macro color zones improve V83.2; the valley still reads as an early blockout. |

## Screenshots

Runtime folder:
`Saved/V84_TitanGroundcoverSurface/RuntimeScreenshots_20260729`

Evidence produced:

- 18 required runtime captures;
- labeled top-down;
- V83.2 versus V84 comparison;
- runtime contact sheet;
- Titan versus V84 grass comparison;
- performance/scalability note.

The final warmed top-down capture does not contain the old grey T-shape.
Testing proved that shape was an early camera/streaming artifact when top-down
was captured first, not persistent map geometry.

## Performance And Scalability

- Native LandscapeGrass; no manual full-map HISM carpet.
- 110 Landscape components have native GrassData saved.
- Manual V84 grass actor count: 0.
- Native instance total is camera/tile dependent and is not stored as a stable
  persistent actor count; no misleading fixed count is reported.
- Runtime quality setting observed: grass density scale 0.68.
- Production cull is 2,000-10,000 cm.
- Screenshot-only distant validation used cull scale 3.0; production values were
  not changed by that flag.
- Four final editor-runtime batches peaked at 2.91-2.99 GB Unreal working set.
- Lowest free system RAM during those batches: 0.87 GB.
- All four batches exited normally with `FFSmoke PASS`.
- Package cook briefly reported internal memory pressure during asset compile,
  completed successfully, and returned RAM after exit.
- No FPS/frame-time capture was produced, so performance verdict remains
  **PARTIAL** rather than claiming an unmeasured frame-rate result.

## Validation Results

| Validation | Result | Evidence |
|---|---|---|
| MapCheck | PASS | 0 errors, 0 warnings |
| Editor build | PASS | `Result: Succeeded`, 19.70 s |
| Native GrassMap build | PASS | 110 components, SaveMap=1 |
| Package | PASS | 123.80 s, ExitCode 0 |
| Packaged smoke | PASS | Clean D3D11 run, ExitCode 0 |
| Runtime camera batches | PASS | 4/4 batches, 18 raw required views |
| Material compile scan | PASS | No material compile/default fallback errors |
| Crash scan | PASS | No fatal/assert/GPU crash/FFSmoke failure |
| Full visual target | PARTIAL | White blades and blockout surface remain |
| FPS/frame-time measurement | NOT DONE | Memory sanity only |

Package:
`Saved/PackageReady_HighlandWorldbuilding_v84_titan_groundcover_surface`

Package size: 1.16 GB.

Packaged smoke captures:
`Saved/PackageReady_HighlandWorldbuilding_v84_titan_groundcover_surface/Windows/FantasyFrontier/Saved/FFSmokeCaptures/V84_PackagedSmoke_Clean`

## Log Scan

No V84 fatal error, assertion, GPU crash, smoke failure, MapCheck error, material
compile error or default-material fallback was found.

Known non-blocking/pre-existing warnings:

- local Oodle Texture DLL is not installed;
- Fab logs `EOS is not initialized` in editor-only runs;
- two optional Dandelion Niagara dependencies are missing;
- packaged runtime skips an optional
  `/WaterAdvanced/Niagara/Systems/Grid2D_OceanPatch` package;
- first packaged run has no user-settings save yet;
- package cook logged memory pressure but completed with ExitCode 0.

Full scan:
`Saved/V84_Final_LogScan.txt`.

## Exact V84 Production Files Changed

- `Content/Maps/FF_Starter_Highland_Blockout.umap`
- `Content/FantasyFrontier/Blockout/Materials/M_FF_Blockout_Grass_Green.uasset`
- `Content/FantasyFrontier/Blockout/Materials/M_FF_Highland_GrassBlade_V84.uasset`
- `Content/FantasyFrontier/Blockout/Materials/GT_FF_Highland_Groundcover_V84.uasset`
- `Source/TP_ThirdPerson/FFStarterHighlandGroundcoverSurfaceCommandlet.h`
- `Source/TP_ThirdPerson/FFStarterHighlandGroundcoverSurfaceCommandlet.cpp`
- `Source/TP_ThirdPerson/TP_ThirdPersonPlayerController.cpp`
  (validation flags only)

The worktree already contained many unrelated dirty/untracked files before V84.
They were neither reverted nor claimed as V84 work.

## V84 Evidence And Support Files

- `Saved/V84_PreImplementation_GrassSurface_Audit.json`
- `Saved/V84_CurrentGrassSurface_Audit.json`
- `Saved/V84_Audit_CurrentGrassSurface.py`
- `Saved/V84_SurfaceStyleContract.md`
- `Saved/V84_EditorGrassMapBuild_Report_Final.txt`
- `Saved/V84_BuildVisualReport.py`
- `Saved/V84_MapCheck_stdout.txt`
- `Saved/package_v84_titan_groundcover_surface.txt`
- `Saved/V84_PackagedSmoke_Clean_stdout.txt`
- `Saved/V84_Final_LogScan.txt`
- `Saved/V84_TitanGroundcoverSurface/RuntimeScreenshots_20260729/*`

## Known Issues

1. Grass blade lighting still produces white/neon highlights.
2. Broad landscape zones remain pale/overexposed and visually blockout-like.
3. The native grass is convincing only in the dedicated meadow views.
4. Route and wet-bank masks are readable but still procedural.
5. No low flower/herb/fern layer was added.
6. No measured FPS/frame-time result exists.
7. Cook approached the machine's physical-memory limit.
8. Optional Dandelion and WaterAdvanced dependencies remain missing.

## NOT DONE

- No V85 forest implementation.
- No village/houses/props.
- No forest trees.
- No resources or ore nodes.
- No NPCs, quests, combat or boss content.
- No waterfall or water-system work.
- No final authored roads.
- No low flower/herb/fern diversity pass.
- No global lighting/exposure correction.
- No FPS/frame-time benchmark.
- `TESTING.md` NPC-interaction and enemy-combat steps were not executed in the
  Highland smoke path; that path intentionally stops after terrain grounding
  and camera validation, and those systems are protected/out of V84 scope.
- No commit or push.

## Placeholder Assets Still In Use

- The Highland interior is still visibly a terrain/surface blockout in several
  zones.
- The default Epic mannequin visible in automated smoke captures is a
  placeholder test character, not a finished hero character.
- Validation cameras and procedural surface masks are test/production-support
  infrastructure, not final authored world dressing.

## Enforcement

- No protected mountain/water/lake/TitanMain/Kashkeh system was modified.
- PlayerStart was changed vertically only after a real ground-safety failure was
  measured; this follows the explicit V84 exception.
- No paid content was downloaded.
- No full-map manual HISM grass rollout was used.
- No visual PASS was claimed below the required quality bar.
- No rule violation was found.

## Testing Enforcement

- `TESTING.md` was read.
- Build, Windows package, executable launch, title flow, creator flow,
  gender/preview switching, preset save/load, game start, spawn, movement,
  dash, light/heavy attacks and Highland terrain grounding passed.
- NPC interaction and enemy combat remain an explicit test gap because the
  Highland smoke path has no approved V84 NPC/enemy implementation and those
  systems were protected.
- Since the full matrix and visual target are incomplete, V84 is reported
  **PARTIAL**, not complete.

## Asset Import Enforcement

- No new external character or creature asset was imported.
- Existing project `SM_GrassBlade` was reused with a Highland-owned material.
- `IMPORT_QA.md` was therefore not triggered.

## Recommended Next Step

Perform a focused V84.1 correction before V85:

- solve grass highlight/lighting response;
- improve low-frequency surface value separation;
- add one restrained existing low-groundcover layer where safe;
- verify path/bank transitions from player height;
- capture frame-time/FPS at production cull settings;
- rerun the same serial build/package/smoke gate.

