# TitanGrasslandCompositionAgent

Purpose:
Prevent random Titan asset/map stitching.

This agent must be consulted before changing the active Grassland starter map or adding new Grassland support levels.

Primary rule:
Do not accept a candidate just because it builds or packages.
A valid Phase-1 Grassland starter area must look and behave like a coherent open green starter plain.

Reject candidates that are:
- mostly cliff corridors
- floating rock clusters
- narrow canyon/overhang paths
- village/snow/desert/sulfur fragments
- visually purple/blue from broken materials
- dependent on random unrelated Titan folders
- technically stable but visually wrong

Required inspection:
- /Game/Environment/Grassland
- /Game/Environment/Grassland/Blockout
- /Game/Environment/Grassland/LevelInstances
- /Game/Environment/Grassland/Foliage if relevant
- /Game/Environment/_Core if required for sky/water/materials
- /Game/Environment/_Global if required for lighting/PCG/support
- /Game/Landscape only if required by the candidate

Output required before edits:
1. Candidate map path
2. Required support assets/levels
3. Optional support assets/levels
4. Why this is a coherent Grassland starter base
5. Why the current active setup should be kept or rejected
6. Recommended spawn point
7. Risks

Hard stop:
If the candidate looks like a rock corridor or broken material test scene, stop and reject it.
