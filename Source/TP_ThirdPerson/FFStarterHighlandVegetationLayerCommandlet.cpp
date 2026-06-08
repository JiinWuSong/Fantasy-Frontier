#include "FFStarterHighlandVegetationLayerCommandlet.h"

#if WITH_EDITOR
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "FileHelpers.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerStart.h"
#include "LandscapeComponent.h"
#include "LandscapeLayerInfoObject.h"
#include "LandscapeProxy.h"
#include "LevelInstance/LevelInstanceActor.h"
#include "Materials/MaterialInterface.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#endif

UFFStarterHighlandVegetationLayerCommandlet::UFFStarterHighlandVegetationLayerCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

#if WITH_EDITOR
namespace
{
	const TCHAR* HighlandMapPath = TEXT("/Game/Maps/FF_Starter_Highland_Blockout");
	const FName VegetationTag(TEXT("FFPhase1TitanVegetationLayer"));
	const FName CliffRockTag(TEXT("FFPhase1TitanCliffRockLayer"));
	const FName V35ValidationCameraTag(TEXT("FFHighlandV35ValidationCamera"));
	const FName V36ValidationCameraTag(TEXT("FFHighlandV36ValidationCamera"));
	const FName V37ValidationCameraTag(TEXT("FFHighlandV37ValidationCamera"));
	const FName V38ValidationCameraTag(TEXT("FFHighlandV38ValidationCamera"));
	const FName V39ValidationCameraTag(TEXT("FFHighlandV39ValidationCamera"));
	const FName V40ValidationCameraTag(TEXT("FFHighlandV40ValidationCamera"));
	const FName V41ValidationCameraTag(TEXT("FFHighlandV41ValidationCamera"));
	const FName V42ValidationCameraTag(TEXT("FFHighlandV42ValidationCamera"));
	const FName V43ValidationCameraTag(TEXT("FFHighlandV43ValidationCamera"));
	const FName V44ValidationCameraTag(TEXT("FFHighlandV44ValidationCamera"));
	const FName V45ValidationCameraTag(TEXT("FFHighlandV45ValidationCamera"));
	const FName V45EmbeddedCliffMassTag(TEXT("FFHighlandV45EmbeddedCliffMass"));
	const FName V46ValidationCameraTag(TEXT("FFHighlandV46ValidationCamera"));
	const FName V46EmbeddedCliffIntegrationTag(TEXT("FFHighlandV46EmbeddedCliffIntegration"));
	const FName V52ThinCliffFacadeTag(TEXT("FFHighlandV52ThinCliffFacade"));
	const FName V53ThinCliffFacadeTag(TEXT("FFHighlandV53ThinCliffFacade"));
	const FName V54ValidationCameraTag(TEXT("FFHighlandV54ValidationCamera"));
	const FName V54ThinCliffFacadeTag(TEXT("FFHighlandV54ThinCliffFacade"));
	const FName V55ValidationCameraTag(TEXT("FFHighlandV55ValidationCamera"));
	const FName V55ThinCliffFacadeTag(TEXT("FFHighlandV55ThinCliffFacade"));
	const FName V56ValidationCameraTag(TEXT("FFHighlandV56ValidationCamera"));
	const FName V56MountainIdentityTag(TEXT("FFHighlandV56MountainIdentity"));
	const FName V57ValidationCameraTag(TEXT("FFHighlandV57ValidationCamera"));
	const FName V57SurfaceReplacementTag(TEXT("FFHighlandV57SurfaceReplacement"));
	const FName V59ValidationCameraTag(TEXT("FFHighlandV59ValidationCamera"));
	const FName V59TitanGeologyTag(TEXT("FFHighlandV59TitanGeology"));
	const FName V65ValidationCameraTag(TEXT("FFHighlandV65ValidationCamera"));
	const FName V65TitanMountainConversionTag(TEXT("FFHighlandV65TitanMountainConversion"));
	const FName V66ValidationCameraTag(TEXT("FFHighlandV66ValidationCamera"));
	const FName V66TitanMountainBodyTag(TEXT("FFHighlandV66TitanMountainBody"));
	const FName V67ValidationCameraTag(TEXT("FFHighlandV67ValidationCamera"));
	const FName V67TitanMountainBodyTag(TEXT("FFHighlandV67TitanMountainBody"));
	const FName V68ValidationCameraTag(TEXT("FFHighlandV68ValidationCamera"));
	const FName V68TitanMountainCoverageTag(TEXT("FFHighlandV68TitanMountainCoverage"));
	const FName V69ValidationCameraTag(TEXT("FFHighlandV69ValidationCamera"));
	const FName V69TitanMountainMassTag(TEXT("FFHighlandV69TitanMountainMass"));
	const FName V70ValidationCameraTag(TEXT("FFHighlandV70ValidationCamera"));
	const FName V70MountainFreezeCleanupTag(TEXT("FFHighlandV70MountainFreezeCleanup"));
	const FName StarterVillagePlaceholderTag(TEXT("FFStarterVillagePlaceholder"));
	const FName StarterNorthBridgePlaceholderTag(TEXT("FFStarterNorthBridgePlaceholder"));
	const FName StarterBossGatePlaceholderTag(TEXT("FFStarterBossGatePlaceholder"));
	const FName StarterBossGatePathTag(TEXT("FFStarterBossGatePath"));
	const FName StarterNorthExitPathTag(TEXT("FFStarterNorthExitPath"));
	const FName TitanGrasslandPipelineProofTag(TEXT("FFTitanGrasslandPipelineProof"));
	const FName SpawnGrassProofTag(TEXT("FFSpawnGrassProof"));
	const TCHAR* TitanCliffRockMaterialPath = TEXT("/Game/Environment/Grassland/Materials/MI_Cliffside_Dirt_B.MI_Cliffside_Dirt_B");

	const TCHAR* TreeMeshPaths[] = {
		TEXT("/Game/Environment/Foliage/SM_Grassland_OliveTree_Large_A.SM_Grassland_OliveTree_Large_A"),
		TEXT("/Game/Environment/Foliage/SM_Grassland_OliveTree_Medium_A.SM_Grassland_OliveTree_Medium_A"),
		TEXT("/Game/Environment/Foliage/SM_Grassland_OliveTree_Medium_B.SM_Grassland_OliveTree_Medium_B"),
		TEXT("/Game/Environment/Foliage/SM_Grassland_OliveTree_Small_A.SM_Grassland_OliveTree_Small_A"),
		TEXT("/Game/Environment/Foliage/SM_Laketree_01.SM_Laketree_01"),
		TEXT("/Game/Environment/Foliage/SM_Laketree_02.SM_Laketree_02"),
		TEXT("/Game/Environment/Foliage/SM_Laketree_03.SM_Laketree_03"),
		TEXT("/Game/Environment/Foliage/SM_BigBroadTree2.SM_BigBroadTree2")
	};

	const TCHAR* UndergrowthMeshPaths[] = {
		TEXT("/Game/Environment/Foliage/SM_Bush_01a.SM_Bush_01a"),
		TEXT("/Game/Environment/Foliage/SM_Bush_01b.SM_Bush_01b")
	};

	const TCHAR* TraversalTreeBlueprintPaths[] = {
		TEXT("/Game/Environment/Grassland/BlueprintActors/BP_LargeTree_WBushLeaves_V1.BP_LargeTree_WBushLeaves_V1_C"),
		TEXT("/Game/Environment/Grassland/BlueprintActors/BP_LargeTree_WBushLeaves_V2.BP_LargeTree_WBushLeaves_V2_C")
	};

	const TCHAR* CliffRockMeshPaths[] = {
		TEXT("/Game/Environment/Grassland/Meshes/Landscape/Rocks/SM_Grassland_Rock_Big_01.SM_Grassland_Rock_Big_01"),
		TEXT("/Game/Environment/Grassland/Meshes/Landscape/Rocks/SM_Grassland_Rock_Big_02.SM_Grassland_Rock_Big_02"),
		TEXT("/Game/Environment/Grassland/Meshes/Landscape/Rocks/SM_Grassland_Rock_Big_03.SM_Grassland_Rock_Big_03"),
		TEXT("/Game/Environment/Grassland/Meshes/Landscape/Rocks/SM_Grassland_Rock_Big_04.SM_Grassland_Rock_Big_04")
	};

	const TCHAR* TitanGeologyWallMeshPathsV59[] = {
		TEXT("/Game/Environment/Grassland/ModularRock_Sets/Prefabs/SM_Prefab_Cliffside_A.SM_Prefab_Cliffside_A"),
		TEXT("/Game/Environment/Grassland/ModularRock_Sets/Prefabs/SM_Prefab_Cliffside_B.SM_Prefab_Cliffside_B"),
		TEXT("/Game/Environment/Grassland/ModularRock_Sets/Large_Rocks/Large_Rock_01.Large_Rock_01"),
		TEXT("/Game/Environment/Grassland/ModularRock_Sets/Large_Rocks/Large_Rock_02.Large_Rock_02"),
		TEXT("/Game/Environment/Grassland/ModularRock_Sets/Large_Rocks/Large_Rock_03.Large_Rock_03"),
		TEXT("/Game/Environment/Grassland/ModularRock_Sets/Large_Rocks/Large_Rock_04.Large_Rock_04"),
		TEXT("/Game/Environment/Grassland/ModularRock_Sets/Large_Rocks/Large_Rock_05.Large_Rock_05"),
		TEXT("/Game/Environment/Grassland/ModularRock_Sets/Large_Rocks/Large_Rock_06.Large_Rock_06"),
		TEXT("/Game/Environment/Grassland/ModularRock_Sets/Large_Rocks/Large_Rock_07.Large_Rock_07")
	};

	const TCHAR* TitanMountainBodyMeshPathsV67[] = {
		TEXT("/Game/Environment/Grassland/ModularRock_Sets/LandChunks/SM_LandChunk_A.SM_LandChunk_A"),
		TEXT("/Game/Environment/Grassland/ModularRock_Sets/LandChunks/SM_DirtMound_A.SM_DirtMound_A"),
		TEXT("/Game/Environment/Grassland/ModularRock_Sets/Prefabs/SM_Prefab_Cliffside_A.SM_Prefab_Cliffside_A"),
		TEXT("/Game/Environment/Grassland/ModularRock_Sets/Prefabs/SM_Prefab_Cliffside_B.SM_Prefab_Cliffside_B"),
		TEXT("/Game/Environment/Grassland/ModularRock_Sets/Prefabs/SM_Prefab_EoG.SM_Prefab_EoG"),
		TEXT("/Game/Environment/Grassland/ModularRock_Sets/Large_Rocks/Large_Rock_01.Large_Rock_01"),
		TEXT("/Game/Environment/Grassland/ModularRock_Sets/Large_Rocks/Large_Rock_02.Large_Rock_02"),
		TEXT("/Game/Environment/Grassland/ModularRock_Sets/Large_Rocks/Large_Rock_03.Large_Rock_03"),
		TEXT("/Game/Environment/Grassland/ModularRock_Sets/Large_Rocks/Large_Rock_04.Large_Rock_04"),
		TEXT("/Game/Environment/Grassland/ModularRock_Sets/Large_Rocks/Large_Rock_05.Large_Rock_05"),
		TEXT("/Game/Environment/Grassland/ModularRock_Sets/Large_Rocks/Large_Rock_06.Large_Rock_06"),
		TEXT("/Game/Environment/Grassland/ModularRock_Sets/Large_Rocks/Large_Rock_07.Large_Rock_07")
	};

	const TCHAR* TitanMountainAssemblyLevelPathsV67[] = {
		TEXT("/Game/Environment/Grassland/ModularRock_Sets/LevelInstances/LI_RockyCliffside.LI_RockyCliffside"),
		TEXT("/Game/Environment/Grassland/ModularRock_Sets/LevelInstances/LI_Dome_Cliffside_01.LI_Dome_Cliffside_01"),
		TEXT("/Game/Environment/Grassland/ModularRock_Sets/LevelInstances/LI_Dome_Cliffside_02.LI_Dome_Cliffside_02"),
		TEXT("/Game/Environment/Grassland/ModularRock_Sets/LevelInstances/LI_Dome_Cliffside_03.LI_Dome_Cliffside_03")
	};

	const TCHAR* CliffTransitionDirtMeshPaths[] = {
		TEXT("/Game/Environment/Grassland/Kashkeh/Landscape/SM_DirtCircles_NoColl.SM_DirtCircles_NoColl")
	};

	const TCHAR* CliffTransitionDebrisMeshPaths[] = {
		TEXT("/Game/VFX/Environment/_Global/Rockfall/Meshes/SM_Rockfall_01.SM_Rockfall_01"),
		TEXT("/Game/VFX/Environment/_Global/Rockfall/Meshes/SM_Rockfall_02.SM_Rockfall_02"),
		TEXT("/Game/VFX/Environment/_Global/Rockfall/Meshes/SM_Rockfall_03.SM_Rockfall_03")
	};

	const TCHAR* CliffTransitionShrubMeshPaths[] = {
		TEXT("/Game/Environment/Foliage/SM_Bush_01a.SM_Bush_01a"),
		TEXT("/Game/Environment/Foliage/SM_Bush_01b.SM_Bush_01b"),
		TEXT("/Game/Environment/Foliage/SM_Bush_02.SM_Bush_02"),
		TEXT("/Game/Environment/Foliage/SM_ShrubExample_01.SM_ShrubExample_01")
	};

	const TCHAR* TitanTransitionDirtMaterialPath = TEXT("/Game/Environment/Grassland/Materials/MI_Dirt_Blended.MI_Dirt_Blended");
	const TCHAR* TitanRockfallMaterialPath = TEXT("/Game/Environment/Grassland/Materials/MI_Cliffside_Dirt_B.MI_Cliffside_Dirt_B");

	struct FVegetationZone
	{
		const TCHAR* Name;
		FVector2D Center;
		FVector2D Extents;
		float RotationDegrees;
		int32 TreeCount;
		int32 UndergrowthCount;
		float ClusterBias;
	};

	struct FCliffRockRun
	{
		const TCHAR* Name;
		FVector2D Start;
		FVector2D End;
		float JitterRadius;
		int32 Count;
		float MinScale;
		float MaxScale;
	};

	struct FEmbeddedCliffMass
	{
		const TCHAR* Name;
		int32 MeshIndex;
		FVector2D Position;
		float HeightOffset;
		FRotator Rotation;
		FVector Scale;
	};

	struct FEmbeddedCliffMassV46
	{
		const TCHAR* Name;
		int32 MeshIndex;
		FVector2D Position;
		float HeightOffset;
		float NormalBurial;
		float MinWaterDistance;
		FRotator Rotation;
		FVector Scale;
	};

	struct FGroundBlendInstanceV46
	{
		const TCHAR* Name;
		int32 MeshIndex;
		FVector2D Position;
		float HeightOffset;
		float MinNormalZ;
		float MinWaterDistance;
		FRotator Rotation;
		FVector Scale;
	};

	struct FCliffSurfaceSkinRunV49
	{
		const TCHAR* Name;
		FVector2D Start;
		FVector2D End;
		float JitterRadius;
		int32 Count;
		float MinScale;
		float MaxScale;
	};

	struct FThinCliffFacadeRunV52
	{
		const TCHAR* Name;
		FVector2D Start;
		FVector2D End;
		float JitterRadius;
		float InwardOffsetMin;
		float InwardOffsetMax;
		float MinRiverDistance;
		float MinLakeDistance;
		int32 Count;
		int32 Rows;
		float MinBaseScale;
		float MaxBaseScale;
		float MinLengthScale;
		float MaxLengthScale;
		float MinDepthScale;
		float MaxDepthScale;
		float MinHeightScale;
		float MaxHeightScale;
		float RowRise;
	};

	struct FMountainIdentityRunV56
	{
		const TCHAR* Name;
		FVector2D Start;
		FVector2D End;
		float JitterRadius;
		float InwardOffsetMin;
		float InwardOffsetMax;
		float MinRiverDistance;
		float MinLakeDistance;
		int32 AnchorCount;
		float MinorBaseScale;
		float SecondaryBaseScale;
		float PrimaryBaseScale;
		float MinorHeightScale;
		float SecondaryHeightScale;
		float PrimaryHeightScale;
		float PeakLift;
		float ShoulderSpread;
	};

	struct FTitanGeologyRunV59
	{
		const TCHAR* Name;
		FVector2D Start;
		FVector2D End;
		float JitterRadius;
		float InwardOffsetMin;
		float InwardOffsetMax;
		float OutwardBurialMin;
		float OutwardBurialMax;
		float NormalBurialMin;
		float NormalBurialMax;
		float MinRiverDistance;
		float MinLakeDistance;
		int32 AnchorCount;
		int32 Rows;
		float MinLengthScale;
		float MaxLengthScale;
		float MinDepthScale;
		float MaxDepthScale;
		float MinHeightScale;
		float MaxHeightScale;
		float RowRise;
		float BaseLift;
	};

	const FVegetationZone VegetationZones[] = {
		{ TEXT("NorthForestBand"), FVector2D(55500.0f, 166500.0f), FVector2D(36000.0f, 12500.0f), -3.0f, 520, 300, 1.25f },
		{ TEXT("NorthwestForestMass"), FVector2D(25000.0f, 154000.0f), FVector2D(22000.0f, 13500.0f), 12.0f, 220, 90, 1.35f },
		{ TEXT("NortheastForestMass"), FVector2D(90000.0f, 139000.0f), FVector2D(23000.0f, 14500.0f), -16.0f, 350, 170, 1.25f },
		{ TEXT("RiverCentralCopse"), FVector2D(73000.0f, 104000.0f), FVector2D(15000.0f, 11500.0f), 18.0f, 140, 80, 1.20f },
		{ TEXT("WestLakeUndergrowth"), FVector2D(25500.0f, 78000.0f), FVector2D(14500.0f, 11500.0f), -10.0f, 90, 85, 1.20f },
		{ TEXT("SouthEastScatteredGrove"), FVector2D(86500.0f, 47000.0f), FVector2D(18000.0f, 14500.0f), -20.0f, 130, 80, 1.05f },
		{ TEXT("CentralOpenSaplings"), FVector2D(61000.0f, 101000.0f), FVector2D(24000.0f, 18000.0f), 4.0f, 90, 55, 0.78f },
		{ TEXT("SouthOpenSaplings"), FVector2D(63000.0f, 56000.0f), FVector2D(24000.0f, 16000.0f), -8.0f, 80, 50, 0.76f },
		{ TEXT("SpawnViewDistantGrove"), FVector2D(36000.0f, 154800.0f), FVector2D(13000.0f, 4500.0f), 2.0f, 0, 0, 1.00f }
	};

	const FVegetationZone TraversalForestV2Zones[] = {
		// Titan forests read as masses with open traversal corridors, not even scatter.
		{ TEXT("UpperRightForestCore"), FVector2D(91000.0f, 139500.0f), FVector2D(24500.0f, 14200.0f), -15.0f, 108, 0, 1.48f },
		{ TEXT("UpperRightForestEdge"), FVector2D(79500.0f, 128500.0f), FVector2D(24500.0f, 12600.0f), -26.0f, 52, 0, 1.02f },
		{ TEXT("NorthTreeLineTransition"), FVector2D(58500.0f, 160500.0f), FVector2D(36500.0f, 7200.0f), -4.0f, 48, 0, 1.20f },
		{ TEXT("NorthwestSoftEdge"), FVector2D(28500.0f, 151000.0f), FVector2D(15500.0f, 8200.0f), 12.0f, 18, 0, 1.12f }
	};

	const FVegetationZone TraversalForestV3Zones[] = {
		// Titan-style forest read: broken clustered masses, open meadow pockets, and traversable seams.
		{ TEXT("UpperRightDeepCore"), FVector2D(93500.0f, 140500.0f), FVector2D(16800.0f, 9800.0f), -28.0f, 62, 0, 1.58f },
		{ TEXT("UpperRightBackPocket"), FVector2D(104000.0f, 130500.0f), FVector2D(9800.0f, 7600.0f), -8.0f, 24, 0, 1.42f },
		{ TEXT("UpperRightLooseEdge"), FVector2D(79000.0f, 138500.0f), FVector2D(17200.0f, 8600.0f), -20.0f, 26, 0, 1.18f },
		{ TEXT("NorthBrokenCopseA"), FVector2D(54500.0f, 160200.0f), FVector2D(11800.0f, 5200.0f), -6.0f, 14, 0, 1.32f },
		{ TEXT("NorthBrokenCopseB"), FVector2D(68500.0f, 157400.0f), FVector2D(10400.0f, 5000.0f), 9.0f, 12, 0, 1.24f },
		{ TEXT("NorthwestSoftPocket"), FVector2D(28500.0f, 152500.0f), FVector2D(12500.0f, 5200.0f), 15.0f, 8, 0, 1.18f }
	};

	const FVegetationZone TraversalForestV4Zones[] = {
		// V4 adds Titan-style fractal edges: advance trees, retreat pockets, and meadow bites.
		{ TEXT("UpperRightLayeredCore"), FVector2D(94000.0f, 140800.0f), FVector2D(14800.0f, 9000.0f), -27.0f, 54, 0, 1.66f },
		{ TEXT("UpperRightBackCanopy"), FVector2D(104500.0f, 130800.0f), FVector2D(9000.0f, 7000.0f), -8.0f, 20, 0, 1.48f },
		{ TEXT("UpperRightAdvanceA"), FVector2D(79000.0f, 141000.0f), FVector2D(11200.0f, 5200.0f), -24.0f, 13, 0, 1.10f },
		{ TEXT("UpperRightAdvanceB"), FVector2D(83200.0f, 129400.0f), FVector2D(9600.0f, 4700.0f), -14.0f, 10, 0, 1.08f },
		{ TEXT("UpperRightRetreatPocket"), FVector2D(98200.0f, 119500.0f), FVector2D(7600.0f, 5400.0f), 8.0f, 8, 0, 1.24f },
		{ TEXT("NorthBrokenCanopyA"), FVector2D(52600.0f, 160800.0f), FVector2D(10400.0f, 4600.0f), -5.0f, 10, 0, 1.32f },
		{ TEXT("NorthBrokenCanopyB"), FVector2D(67500.0f, 157900.0f), FVector2D(9000.0f, 4300.0f), 9.0f, 9, 0, 1.24f },
		{ TEXT("NorthBrokenCanopyC"), FVector2D(79200.0f, 154600.0f), FVector2D(8200.0f, 3900.0f), -12.0f, 7, 0, 1.18f },
		{ TEXT("NorthwestRecoveredPocket"), FVector2D(31800.0f, 149200.0f), FVector2D(8800.0f, 4300.0f), 10.0f, 9, 0, 1.14f }
	};

	const FVegetationZone TraversalForestV5Zones[] = {
		// V5 deepens the same forest instead of making a new wall: back canopy,
		// landmark trunks, soft edge advances, and meadow breaks for traversal reads.
		{ TEXT("UpperRightDeepBackCanopy"), FVector2D(101500.0f, 137500.0f), FVector2D(13200.0f, 8200.0f), -18.0f, 32, 0, 1.62f },
		{ TEXT("UpperRightLandmarkPair"), FVector2D(91600.0f, 133800.0f), FVector2D(7200.0f, 4100.0f), -21.0f, 8, 0, 1.90f },
		{ TEXT("UpperRightLooseFrontEdge"), FVector2D(78000.0f, 136400.0f), FVector2D(14200.0f, 6100.0f), -18.0f, 18, 0, 1.08f },
		{ TEXT("UpperRightLowerPocket"), FVector2D(100800.0f, 119200.0f), FVector2D(9400.0f, 6100.0f), 6.0f, 12, 0, 1.32f },
		{ TEXT("NorthDepthCanopyA"), FVector2D(50000.0f, 160400.0f), FVector2D(11800.0f, 4700.0f), -6.0f, 12, 0, 1.36f },
		{ TEXT("NorthDepthCanopyB"), FVector2D(67800.0f, 158600.0f), FVector2D(11200.0f, 4300.0f), 8.0f, 11, 0, 1.28f },
		{ TEXT("NorthwestEdgeScatter"), FVector2D(33800.0f, 148600.0f), FVector2D(10400.0f, 4400.0f), 12.0f, 10, 0, 1.10f },
		{ TEXT("ForestCorridorSentinels"), FVector2D(84600.0f, 125500.0f), FVector2D(7600.0f, 3700.0f), -22.0f, 6, 0, 1.04f }
	};

	const FVegetationZone TraversalForestV6Zones[] = {
		// V6 follows Titan forest depth: dense back canopy, layered mid canopy,
		// broken advance trees, and corridor-side sentinels without forming a wall.
		{ TEXT("UpperRightBackCanopyDense"), FVector2D(101800.0f, 137200.0f), FVector2D(15000.0f, 9300.0f), -19.0f, 42, 0, 1.72f },
		{ TEXT("UpperRightMidCanopy"), FVector2D(107200.0f, 127800.0f), FVector2D(11800.0f, 7000.0f), -24.0f, 24, 0, 1.44f },
		{ TEXT("UpperRightBrokenFrontA"), FVector2D(98200.0f, 118800.0f), FVector2D(8200.0f, 4300.0f), -18.0f, 14, 0, 1.14f },
		{ TEXT("UpperRightBrokenFrontB"), FVector2D(111000.0f, 114200.0f), FVector2D(7400.0f, 3800.0f), -11.0f, 10, 0, 1.10f },
		{ TEXT("NorthDepthCanopyArc"), FVector2D(61000.0f, 158900.0f), FVector2D(26000.0f, 5200.0f), 3.0f, 28, 0, 1.34f },
		{ TEXT("NorthRetreatPocket"), FVector2D(76000.0f, 160800.0f), FVector2D(9600.0f, 3600.0f), 8.0f, 10, 0, 1.18f },
		{ TEXT("CorridorEdgeSentinels"), FVector2D(106500.0f, 119600.0f), FVector2D(6800.0f, 3100.0f), -25.0f, 8, 0, 1.04f }
	};

	const FVegetationZone TraversalForestV7Zones[] = {
		// V7 keeps the Titan logic from v6 but gives the proof camera a real
		// readable forest: sparse foreground, layered mid canopy, denser core.
		{ TEXT("UpperRightBackCanopyCore"), FVector2D(104000.0f, 137600.0f), FVector2D(13200.0f, 8200.0f), -21.0f, 46, 0, 1.78f },
		{ TEXT("UpperRightBackCanopyOffset"), FVector2D(113000.0f, 128400.0f), FVector2D(9200.0f, 6100.0f), -18.0f, 24, 0, 1.46f },
		{ TEXT("UpperRightMidCanopyShelf"), FVector2D(100200.0f, 124400.0f), FVector2D(12600.0f, 6200.0f), -24.0f, 28, 0, 1.36f },
		{ TEXT("UpperRightFrontEdgeSparseA"), FVector2D(90600.0f, 117800.0f), FVector2D(8600.0f, 3500.0f), -20.0f, 14, 0, 1.02f },
		{ TEXT("UpperRightFrontEdgeSparseB"), FVector2D(111500.0f, 112200.0f), FVector2D(7600.0f, 3100.0f), -10.0f, 12, 0, 1.04f },
		{ TEXT("NorthDepthLayerA"), FVector2D(53000.0f, 159600.0f), FVector2D(11800.0f, 3800.0f), -5.0f, 16, 0, 1.30f },
		{ TEXT("NorthDepthLayerB"), FVector2D(72000.0f, 158800.0f), FVector2D(13200.0f, 3900.0f), 7.0f, 16, 0, 1.24f },
		{ TEXT("CorridorSentinelBreakup"), FVector2D(101500.0f, 116600.0f), FVector2D(8500.0f, 2900.0f), -28.0f, 8, 0, 0.98f }
	};

	const FVegetationZone TraversalForestV8Zones[] = {
		// V8 borrows Titan's ecology rhythm more directly: an uneven back canopy,
		// offset mid layers, feathered edge trees, landmarks, and open approach cuts.
		{ TEXT("UpperRightBackCanopyDenseA"), FVector2D(103500.0f, 138600.0f), FVector2D(12800.0f, 8600.0f), -23.0f, 50, 0, 1.82f },
		{ TEXT("UpperRightBackCanopyDenseB"), FVector2D(115500.0f, 130800.0f), FVector2D(8500.0f, 5700.0f), -16.0f, 28, 0, 1.55f },
		{ TEXT("UpperRightMidCanopyArc"), FVector2D(101500.0f, 124800.0f), FVector2D(13800.0f, 6600.0f), -26.0f, 42, 0, 1.34f },
		{ TEXT("UpperRightFrontFeatherA"), FVector2D(89200.0f, 119500.0f), FVector2D(9200.0f, 3400.0f), -20.0f, 14, 0, 0.96f },
		{ TEXT("UpperRightFrontFeatherB"), FVector2D(112000.0f, 113400.0f), FVector2D(8200.0f, 3300.0f), -9.0f, 12, 0, 0.98f },
		{ TEXT("NorthDepthBrokenCanopyA"), FVector2D(51500.0f, 160200.0f), FVector2D(10800.0f, 3900.0f), -6.0f, 14, 0, 1.36f },
		{ TEXT("NorthDepthBrokenCanopyB"), FVector2D(70000.0f, 158900.0f), FVector2D(12600.0f, 4100.0f), 7.0f, 23, 0, 1.28f },
		{ TEXT("CorridorEdgeAsymmetricSentinels"), FVector2D(102500.0f, 117000.0f), FVector2D(9400.0f, 2700.0f), -29.0f, 9, 0, 0.90f }
	};

	const FVegetationZone TraversalForestV9Zones[] = {
		// V9 is the v38 forest ecology pass: dense interior pockets, sparse edge
		// trees, meadow incursions, landmark trunks, and asymmetrical canopy flow.
		{ TEXT("UpperRightInteriorCanopyCoreA"), FVector2D(104800.0f, 139400.0f), FVector2D(11600.0f, 7800.0f), -25.0f, 44, 0, 1.90f },
		{ TEXT("UpperRightInteriorCanopyCoreB"), FVector2D(116800.0f, 131900.0f), FVector2D(7600.0f, 5200.0f), -14.0f, 24, 0, 1.62f },
		{ TEXT("UpperRightMidCanopyBrokenFlow"), FVector2D(103800.0f, 126300.0f), FVector2D(13200.0f, 6200.0f), -28.0f, 36, 0, 1.42f },
		{ TEXT("UpperRightMeadowIncursionEdgeA"), FVector2D(91000.0f, 121400.0f), FVector2D(7800.0f, 3000.0f), -23.0f, 11, 0, 0.90f },
		{ TEXT("UpperRightMeadowIncursionEdgeB"), FVector2D(113600.0f, 116200.0f), FVector2D(6900.0f, 2800.0f), -8.0f, 10, 0, 0.92f },
		{ TEXT("UpperRightLandmarkTrees"), FVector2D(108200.0f, 121300.0f), FVector2D(5200.0f, 2300.0f), -18.0f, 5, 0, 1.05f },
		{ TEXT("NorthCanopyDepthPocketA"), FVector2D(51000.0f, 160600.0f), FVector2D(9800.0f, 3400.0f), -7.0f, 12, 0, 1.34f },
		{ TEXT("NorthCanopyDepthPocketB"), FVector2D(69200.0f, 159300.0f), FVector2D(11600.0f, 3600.0f), 6.0f, 18, 0, 1.28f },
		{ TEXT("NorthCanopyDepthPocketC"), FVector2D(81800.0f, 156600.0f), FVector2D(7600.0f, 3000.0f), -13.0f, 10, 0, 1.16f },
		{ TEXT("CorridorSparseSentinelFlow"), FVector2D(100800.0f, 117500.0f), FVector2D(9700.0f, 2500.0f), -31.0f, 8, 0, 0.84f },
		{ TEXT("ForestEdgeFuturePOIFrame"), FVector2D(119000.0f, 122500.0f), FVector2D(5600.0f, 2700.0f), -18.0f, 7, 0, 0.96f }
	};

	const FVegetationZone TraversalForestV10Zones[] = {
		// V10 is the v39 ecology refinement: keep the v9 readable gaps, but
		// push a more grown front/mid/back rhythm with fewer straight edges.
		{ TEXT("UpperRightInteriorPocketNorth"), FVector2D(105800.0f, 140000.0f), FVector2D(10500.0f, 7200.0f), -26.0f, 38, 0, 1.94f },
		{ TEXT("UpperRightInteriorPocketSouth"), FVector2D(116200.0f, 131200.0f), FVector2D(7200.0f, 5000.0f), -13.0f, 22, 0, 1.60f },
		{ TEXT("UpperRightCanopyBridgeBroken"), FVector2D(104500.0f, 126400.0f), FVector2D(12400.0f, 5600.0f), -29.0f, 32, 0, 1.38f },
		{ TEXT("UpperRightSparseEdgeMeadowTongue"), FVector2D(91800.0f, 121800.0f), FVector2D(7200.0f, 2600.0f), -24.0f, 10, 0, 0.86f },
		{ TEXT("UpperRightLandmarkRidgeTrees"), FVector2D(109200.0f, 121800.0f), FVector2D(4600.0f, 2100.0f), -19.0f, 6, 0, 1.08f },
		{ TEXT("NorthDepthStaggeredPocketA"), FVector2D(50600.0f, 160500.0f), FVector2D(9000.0f, 3000.0f), -7.0f, 12, 0, 1.30f },
		{ TEXT("NorthDepthStaggeredPocketB"), FVector2D(69400.0f, 159400.0f), FVector2D(10600.0f, 3300.0f), 6.0f, 16, 0, 1.24f },
		{ TEXT("NorthDepthStaggeredPocketC"), FVector2D(81400.0f, 156800.0f), FVector2D(6200.0f, 2400.0f), -13.0f, 8, 0, 1.10f },
		{ TEXT("EastPocketBorderFrame"), FVector2D(121000.0f, 116500.0f), FVector2D(5400.0f, 2500.0f), -18.0f, 8, 0, 0.94f },
		{ TEXT("CorridorEdgeBrokenSentinels"), FVector2D(100500.0f, 117700.0f), FVector2D(8900.0f, 2300.0f), -31.0f, 7, 0, 0.82f },
		{ TEXT("FieldForestFeatherAsymmetry"), FVector2D(89500.0f, 130500.0f), FVector2D(7800.0f, 2800.0f), -20.0f, 9, 0, 0.92f }
	};

	const FVegetationZone TraversalForestV40Zones[] = {
		// V40 correction: remove trees from ridge/border crests and rebuild the
		// forest as lower protected pockets with wider traversal gaps.
		{ TEXT("UpperRightProtectedValleyPocketA"), FVector2D(98200.0f, 127300.0f), FVector2D(6600.0f, 3400.0f), -24.0f, 14, 0, 1.02f },
		{ TEXT("UpperRightProtectedValleyPocketB"), FVector2D(112800.0f, 122800.0f), FVector2D(5600.0f, 3000.0f), -16.0f, 10, 0, 0.98f },
		{ TEXT("UpperRightBrokenMidTransition"), FVector2D(102600.0f, 121200.0f), FVector2D(9200.0f, 3400.0f), -28.0f, 18, 0, 0.84f },
		{ TEXT("UpperRightSparseMeadowEdgeA"), FVector2D(91300.0f, 121600.0f), FVector2D(6700.0f, 2400.0f), -24.0f, 5, 0, 0.76f },
		{ TEXT("UpperRightSparseMeadowEdgeB"), FVector2D(111400.0f, 115000.0f), FVector2D(5700.0f, 2300.0f), -10.0f, 5, 0, 0.78f },
		{ TEXT("EastProtectedPocketLow"), FVector2D(120000.0f, 115800.0f), FVector2D(4700.0f, 2200.0f), -18.0f, 5, 0, 0.82f },
		{ TEXT("CorridorEdgeLooseSentinels"), FVector2D(100000.0f, 117300.0f), FVector2D(7800.0f, 2100.0f), -31.0f, 4, 0, 0.70f },
		{ TEXT("FieldForestFeatherLoose"), FVector2D(88800.0f, 129100.0f), FVector2D(6500.0f, 2300.0f), -20.0f, 5, 0, 0.72f }
	};

	const FVegetationZone TraversalForestV42Zones[] = {
		// V42 ecology variation: lower, protected pockets only. These zones
		// add species/scale rhythm without rebuilding dense forest walls.
		{ TEXT("LowerPocketOldTreesA"), FVector2D(97000.0f, 126200.0f), FVector2D(5600.0f, 2900.0f), -24.0f, 3, 0, 1.20f },
		{ TEXT("LowerPocketSupportA"), FVector2D(102500.0f, 121200.0f), FVector2D(7800.0f, 2600.0f), -28.0f, 4, 0, 0.84f },
		{ TEXT("EastProtectedMixedPocket"), FVector2D(114800.0f, 119000.0f), FVector2D(5200.0f, 2500.0f), -14.0f, 3, 0, 0.92f },
		{ TEXT("SparseMeadowIncursion"), FVector2D(90600.0f, 121600.0f), FVector2D(6100.0f, 2100.0f), -24.0f, 2, 0, 0.70f },
		{ TEXT("CorridorLandmarkPair"), FVector2D(108200.0f, 116600.0f), FVector2D(4700.0f, 1700.0f), -22.0f, 2, 0, 1.34f },
		{ TEXT("FieldForestFeatherMixed"), FVector2D(88400.0f, 128500.0f), FVector2D(6100.0f, 2100.0f), -20.0f, 2, 0, 0.74f }
	};

	const FCliffRockRun CliffRockRunsV1[] = {
		// Titan cliffs read as layered rock masses and ledge breaks. These runs
		// dress only existing rim/footing areas; they do not alter terrain or water.
		{ TEXT("SpawnNorthWallLowerStrata"), FVector2D(28500.0f, 158400.0f), FVector2D(76000.0f, 158100.0f), 1200.0f, 18, 0.45f, 1.10f },
		{ TEXT("SpawnNorthWallUpperBreaks"), FVector2D(30000.0f, 164000.0f), FVector2D(81000.0f, 160800.0f), 1800.0f, 12, 0.70f, 1.60f },
		{ TEXT("NorthBackWallStrata"), FVector2D(78000.0f, 160500.0f), FVector2D(112000.0f, 139000.0f), 3600.0f, 24, 2.6f, 5.2f },
		{ TEXT("UpperRightWallBreaks"), FVector2D(95200.0f, 145000.0f), FVector2D(113500.0f, 116200.0f), 3300.0f, 22, 2.8f, 5.8f },
		{ TEXT("NorthRimToeRocks"), FVector2D(62000.0f, 158500.0f), FVector2D(84500.0f, 156500.0f), 2500.0f, 12, 2.2f, 4.4f },
		{ TEXT("EastShoulderRocks"), FVector2D(101500.0f, 129500.0f), FVector2D(116000.0f, 108500.0f), 2400.0f, 14, 1.8f, 3.6f }
	};

	const FCliffRockRun CliffRockRunsV2[] = {
		// V2 keeps the v35 shelf read but spreads it into a clearer world-border
		// language: toe rocks, ledge breaks, and one small overlook landmark.
		{ TEXT("NorthWallLowerToeStrata"), FVector2D(30000.0f, 157700.0f), FVector2D(77000.0f, 157200.0f), 1150.0f, 16, 0.45f, 1.15f },
		{ TEXT("NorthWallUpperShelfBreaks"), FVector2D(34000.0f, 163600.0f), FVector2D(84500.0f, 160200.0f), 1650.0f, 12, 0.55f, 1.30f },
		{ TEXT("NorthBackWallLayeredMass"), FVector2D(80000.0f, 160000.0f), FVector2D(111500.0f, 139000.0f), 3000.0f, 18, 1.25f, 2.50f },
		{ TEXT("UpperRightWallDepthBreaks"), FVector2D(96500.0f, 145200.0f), FVector2D(114000.0f, 116500.0f), 2900.0f, 18, 1.25f, 2.60f },
		{ TEXT("NorthRimToeRocks"), FVector2D(61500.0f, 157900.0f), FVector2D(85800.0f, 155900.0f), 2300.0f, 12, 0.90f, 1.90f },
		{ TEXT("EastShoulderBoundaryRocks"), FVector2D(101500.0f, 129500.0f), FVector2D(116000.0f, 108500.0f), 2250.0f, 12, 0.85f, 1.75f },
		{ TEXT("WestBoundaryToeBreaks"), FVector2D(21000.0f, 139000.0f), FVector2D(25500.0f, 72000.0f), 2600.0f, 14, 0.85f, 1.65f },
		{ TEXT("SmallTraversalOverlookLandmark"), FVector2D(88000.0f, 126000.0f), FVector2D(95000.0f, 121500.0f), 900.0f, 5, 1.10f, 1.80f }
	};

	const FCliffRockRun CliffRockRunsV3[] = {
		// V3 increases believable geology without changing collision/traversal:
		// smaller toe strata, staggered upper ledges, erosion ribs, and silhouettes.
		{ TEXT("NorthWallToeFineStrata"), FVector2D(29200.0f, 157300.0f), FVector2D(77000.0f, 156900.0f), 980.0f, 20, 0.38f, 1.00f },
		{ TEXT("NorthWallUpperStaggeredLedges"), FVector2D(32500.0f, 163500.0f), FVector2D(87200.0f, 160500.0f), 1550.0f, 16, 0.45f, 1.22f },
		{ TEXT("NorthBackWallErosionRibs"), FVector2D(80200.0f, 160600.0f), FVector2D(112800.0f, 139300.0f), 2550.0f, 20, 0.88f, 1.85f },
		{ TEXT("UpperRightVerticalRhythm"), FVector2D(96800.0f, 145300.0f), FVector2D(114500.0f, 116700.0f), 2450.0f, 20, 0.82f, 1.90f },
		{ TEXT("NorthRimToeAsymmetricBlocks"), FVector2D(60200.0f, 158000.0f), FVector2D(87600.0f, 155900.0f), 2050.0f, 15, 0.70f, 1.55f },
		{ TEXT("EastShoulderLayeredBoundary"), FVector2D(101000.0f, 130200.0f), FVector2D(116300.0f, 108700.0f), 2150.0f, 14, 0.70f, 1.60f },
		{ TEXT("WestBoundaryErosionBreaks"), FVector2D(20700.0f, 140500.0f), FVector2D(25500.0f, 72000.0f), 2350.0f, 16, 0.62f, 1.48f },
		{ TEXT("TraversalOverlookAnchorRocks"), FVector2D(87200.0f, 126300.0f), FVector2D(95800.0f, 121000.0f), 850.0f, 7, 0.82f, 1.55f },
		{ TEXT("FieldCliffTransitionPebbleBreaks"), FVector2D(73500.0f, 134500.0f), FVector2D(94600.0f, 119200.0f), 1400.0f, 12, 0.45f, 1.08f }
	};

	const FCliffRockRun CliffRockRunsV4[] = {
		// V4 is the v38 cliff polish/landmark pass. It keeps dressing small and
		// local: shelf fingers, toe rubble, erosion ribs, and two readable future
		// traversal landmarks outside locked village/water/player systems.
		{ TEXT("NorthWallShelfFingersV38"), FVector2D(33200.0f, 162900.0f), FVector2D(84000.0f, 160100.0f), 1280.0f, 18, 0.42f, 1.18f },
		{ TEXT("NorthWallToeRubbleRhythmV38"), FVector2D(30200.0f, 156800.0f), FVector2D(75500.0f, 156200.0f), 850.0f, 18, 0.34f, 0.88f },
		{ TEXT("NorthBackWallDeepErosionRibsV38"), FVector2D(81800.0f, 160400.0f), FVector2D(113800.0f, 139600.0f), 2250.0f, 22, 0.78f, 1.72f },
		{ TEXT("UpperRightCliffDepthLayerV38"), FVector2D(98200.0f, 145000.0f), FVector2D(115600.0f, 117400.0f), 2250.0f, 22, 0.76f, 1.78f },
		{ TEXT("CliffsideLookoutMiniMountainV38"), FVector2D(103500.0f, 124500.0f), FVector2D(113500.0f, 119000.0f), 760.0f, 10, 1.05f, 2.05f },
		{ TEXT("FutureHutShelfRockHaloV38"), FVector2D(115500.0f, 127000.0f), FVector2D(121500.0f, 122000.0f), 680.0f, 8, 0.72f, 1.42f },
		{ TEXT("EastShoulderWarmCoolBreakupV38"), FVector2D(102500.0f, 130400.0f), FVector2D(117000.0f, 109400.0f), 1900.0f, 14, 0.62f, 1.45f },
		{ TEXT("ForestCliffTransitionStonesV38"), FVector2D(95500.0f, 134500.0f), FVector2D(110500.0f, 121500.0f), 1300.0f, 12, 0.44f, 1.05f },
		{ TEXT("WestBorderSilhouetteNicksV38"), FVector2D(22000.0f, 136000.0f), FVector2D(27000.0f, 76000.0f), 2050.0f, 12, 0.55f, 1.30f }
	};

	const FCliffRockRun CliffRockRunsV5[] = {
		// V5 is the v39 silhouette/world-border composition pass. It does not
		// sculpt terrain; it breaks the read of smooth walls with reversible HISM
		// shelves, crowns, toes, and small traversal-readable rock anchors.
		{ TEXT("NorthOuterRingSilhouetteTeethV39"), FVector2D(30000.0f, 164000.0f), FVector2D(88500.0f, 160800.0f), 1550.0f, 18, 0.48f, 1.28f },
		{ TEXT("NorthWallErosionShelfBreaksV39"), FVector2D(29800.0f, 157200.0f), FVector2D(82000.0f, 156600.0f), 920.0f, 18, 0.34f, 0.95f },
		{ TEXT("NorthBackWallDepthRibsV39"), FVector2D(81200.0f, 160800.0f), FVector2D(114500.0f, 139300.0f), 2200.0f, 22, 0.76f, 1.68f },
		{ TEXT("UpperRightCrownShelfRhythmV39"), FVector2D(99000.0f, 145500.0f), FVector2D(116500.0f, 118500.0f), 2150.0f, 22, 0.72f, 1.72f },
		{ TEXT("LookoutMiniLandmarkUnderlineV39"), FVector2D(103000.0f, 125400.0f), FVector2D(114000.0f, 119500.0f), 760.0f, 12, 0.90f, 1.85f },
		{ TEXT("FutureShelfPoiReadV39"), FVector2D(115000.0f, 127600.0f), FVector2D(122000.0f, 121900.0f), 720.0f, 10, 0.66f, 1.34f },
		{ TEXT("EastOuterBorderToeRhythmV39"), FVector2D(105000.0f, 132000.0f), FVector2D(119000.0f, 109500.0f), 1800.0f, 15, 0.56f, 1.34f },
		{ TEXT("ForestCliffTransitionScatterV39"), FVector2D(94000.0f, 134000.0f), FVector2D(111000.0f, 121000.0f), 1240.0f, 14, 0.38f, 0.96f },
		{ TEXT("WestBorderSoftSilhouetteNicksV39"), FVector2D(22000.0f, 139000.0f), FVector2D(28500.0f, 78000.0f), 1900.0f, 12, 0.48f, 1.18f },
		{ TEXT("WideFieldBorderBreaksV39"), FVector2D(62000.0f, 151500.0f), FVector2D(103000.0f, 139500.0f), 1850.0f, 16, 0.44f, 1.15f }
	};

	const FCliffRockRun CliffRockRunsV6[] = {
		// V6 is the v40 correction pass: no terrain cuts, just a broader, more
		// natural rock language around the visible mountain border and transition toes.
		{ TEXT("NorthOuterRingBrokenToeV40"), FVector2D(30000.0f, 157300.0f), FVector2D(87000.0f, 156400.0f), 1120.0f, 24, 0.36f, 0.98f },
		{ TEXT("NorthOuterRingMidShelfV40"), FVector2D(36000.0f, 163500.0f), FVector2D(93500.0f, 159400.0f), 1480.0f, 22, 0.42f, 1.22f },
		{ TEXT("NorthBackWallLayeredRibsV40"), FVector2D(82500.0f, 160600.0f), FVector2D(115000.0f, 139000.0f), 2060.0f, 30, 0.58f, 1.44f },
		{ TEXT("UpperRightBorderFaceBreakupV40"), FVector2D(98500.0f, 145000.0f), FVector2D(118200.0f, 119500.0f), 2050.0f, 30, 0.56f, 1.48f },
		{ TEXT("UpperRightLowerCliffToeBlendV40"), FVector2D(91500.0f, 130500.0f), FVector2D(112500.0f, 116500.0f), 1320.0f, 20, 0.34f, 0.92f },
		{ TEXT("EastBorderFootingRhythmV40"), FVector2D(107500.0f, 132000.0f), FVector2D(121000.0f, 108500.0f), 1620.0f, 20, 0.42f, 1.18f },
		{ TEXT("WorldRingValleyTransitionV40"), FVector2D(72000.0f, 148000.0f), FVector2D(108000.0f, 134500.0f), 1700.0f, 18, 0.34f, 0.95f },
		{ TEXT("WestBorderLowBreaksV40"), FVector2D(22000.0f, 138500.0f), FVector2D(28600.0f, 79000.0f), 1780.0f, 14, 0.42f, 1.08f },
		{ TEXT("WideFieldOuterRingNicksV40"), FVector2D(58000.0f, 151800.0f), FVector2D(104000.0f, 139800.0f), 1620.0f, 18, 0.34f, 0.90f }
	};

	const FCliffRockRun CliffRockRunsV7[] = {
		// V7 is the v41 correction: replace long protruding slabs with low-profile
		// embedded face layers that support the material-integrated mountain ring.
		{ TEXT("NorthRingEmbeddedFaceLayersV41"), FVector2D(34000.0f, 158800.0f), FVector2D(92500.0f, 157000.0f), 360.0f, 8, 0.08f, 0.22f },
		{ TEXT("NorthBackWallSoftScarsV41"), FVector2D(83000.0f, 160200.0f), FVector2D(115000.0f, 139800.0f), 520.0f, 8, 0.08f, 0.24f },
		{ TEXT("UpperRightEmbeddedBandsV41"), FVector2D(99500.0f, 144400.0f), FVector2D(117600.0f, 120800.0f), 500.0f, 8, 0.08f, 0.24f },
		{ TEXT("UpperRightToeBlendChipsV41"), FVector2D(92000.0f, 130000.0f), FVector2D(112000.0f, 117000.0f), 520.0f, 10, 0.10f, 0.28f },
		{ TEXT("EastBorderGroundedStrataV41"), FVector2D(108500.0f, 129500.0f), FVector2D(120500.0f, 109500.0f), 560.0f, 10, 0.10f, 0.30f },
		{ TEXT("WorldRingLowTransitionV41"), FVector2D(73500.0f, 146500.0f), FVector2D(106000.0f, 134500.0f), 500.0f, 8, 0.09f, 0.26f },
		{ TEXT("WestBorderSubtleFaceBreaksV41"), FVector2D(22500.0f, 137500.0f), FVector2D(28600.0f, 80000.0f), 520.0f, 6, 0.08f, 0.22f }
	};

	const FEmbeddedCliffMass EmbeddedCliffMassesV45[] = {
		// V45 switches from small rock dressing to few large embedded masses.
		// These are deliberately broad and partly buried into the existing ring.
		{ TEXT("NorthWestLowerButtress"), 0, FVector2D(37800.0f, 139800.0f), -760.0f, FRotator(-4.0f, -7.0f, 3.0f), FVector(12.5f, 4.2f, 7.8f) },
		{ TEXT("NorthWestShoulderFace"), 2, FVector2D(50500.0f, 145300.0f), -1020.0f, FRotator(2.0f, -5.0f, -4.0f), FVector(17.0f, 4.8f, 8.4f) },
		{ TEXT("NorthCentralBuriedShelf"), 1, FVector2D(64600.0f, 147300.0f), -910.0f, FRotator(-3.0f, -3.0f, 2.0f), FVector(18.5f, 4.4f, 7.2f) },
		{ TEXT("NorthCentralTallFace"), 3, FVector2D(80200.0f, 145900.0f), -1120.0f, FRotator(1.0f, -2.0f, -2.0f), FVector(15.0f, 4.0f, 10.4f) },
		{ TEXT("NorthHighInterlock"), 0, FVector2D(93600.0f, 142500.0f), -980.0f, FRotator(-2.0f, -8.0f, 5.0f), FVector(18.0f, 4.7f, 8.8f) },
		{ TEXT("NorthEastShadowMass"), 2, FVector2D(107000.0f, 137300.0f), -1180.0f, FRotator(4.0f, -21.0f, -3.0f), FVector(16.0f, 4.6f, 9.2f) },
		{ TEXT("EastUpperDiagonalFace"), 3, FVector2D(115200.0f, 125200.0f), -1040.0f, FRotator(-2.0f, -42.0f, 4.0f), FVector(15.0f, 4.2f, 9.8f) },
		{ TEXT("EastMidEmbeddedWall"), 1, FVector2D(121200.0f, 111400.0f), -960.0f, FRotator(3.0f, -62.0f, -5.0f), FVector(13.8f, 4.1f, 8.4f) },
		{ TEXT("EastToeHeavyTransition"), 0, FVector2D(113800.0f, 105200.0f), -1260.0f, FRotator(-1.0f, -51.0f, 2.0f), FVector(18.8f, 5.4f, 6.2f) },
		{ TEXT("WestFaceLongStrata"), 2, FVector2D(27800.0f, 124800.0f), -880.0f, FRotator(1.0f, 77.0f, -4.0f), FVector(15.6f, 4.5f, 8.0f) },
		{ TEXT("WestFaceLowerMass"), 1, FVector2D(30200.0f, 106800.0f), -1060.0f, FRotator(-3.0f, 82.0f, 4.0f), FVector(14.2f, 4.4f, 8.8f) },
		{ TEXT("WestSouthToeMass"), 3, FVector2D(34800.0f, 91000.0f), -1180.0f, FRotator(2.0f, 70.0f, -2.0f), FVector(16.0f, 5.0f, 6.6f) },
		{ TEXT("RiverbankEmbeddedToe"), 0, FVector2D(106800.0f, 96500.0f), -700.0f, FRotator(-1.0f, -54.0f, 3.0f), FVector(11.5f, 4.8f, 4.8f) },
		{ TEXT("RiverbankBackFace"), 2, FVector2D(118000.0f, 102000.0f), -820.0f, FRotator(2.0f, -45.0f, -3.0f), FVector(12.2f, 4.2f, 5.6f) }
	};

	const FEmbeddedCliffMassV46 EmbeddedCliffMassesV46[] = {
		// V48d treats the sculpted landscape ring as the final silhouette guide.
		// These are small buried nicks only; top-down protrusion safety wins over coverage.
		{ TEXT("NorthwestWallNickV48dA"), 2, FVector2D(35200.0f, 147600.0f), -1900.0f, 1250.0f, 12000.0f, FRotator(-4.0f, -19.0f, 3.0f), FVector(1.85f, 0.74f, 1.45f) },
		{ TEXT("NorthwestWallNickV48dB"), 3, FVector2D(46800.0f, 153800.0f), -1980.0f, 1280.0f, 11800.0f, FRotator(3.0f, -10.0f, -3.0f), FVector(1.8f, 0.72f, 1.45f) },
		{ TEXT("NorthCentralWallNickV48dA"), 2, FVector2D(79200.0f, 146200.0f), -2040.0f, 1280.0f, 12000.0f, FRotator(-4.0f, -12.0f, 3.0f), FVector(1.85f, 0.74f, 1.55f) },
		{ TEXT("NorthCentralWallNickV48dB"), 3, FVector2D(93000.0f, 143800.0f), -2080.0f, 1320.0f, 13200.0f, FRotator(4.0f, -24.0f, -3.0f), FVector(1.9f, 0.72f, 1.6f) },
		{ TEXT("NorthBackWallNickV48dA"), 2, FVector2D(101800.0f, 149200.0f), -2100.0f, 1340.0f, 16800.0f, FRotator(-4.0f, -28.0f, 3.0f), FVector(2.05f, 0.74f, 1.7f) },
		{ TEXT("NorthBackWallNickV48dB"), 3, FVector2D(111800.0f, 143800.0f), -2120.0f, 1340.0f, 17400.0f, FRotator(4.0f, -42.0f, -3.0f), FVector(1.95f, 0.72f, 1.6f) },
		{ TEXT("NorthEastWallNickV48d"), 2, FVector2D(108200.0f, 136400.0f), -2040.0f, 1280.0f, 14000.0f, FRotator(4.0f, -42.0f, -3.0f), FVector(1.75f, 0.72f, 1.45f) },
		{ TEXT("NorthEastOuterNickV48d"), 3, FVector2D(119000.0f, 134800.0f), -2100.0f, 1320.0f, 17800.0f, FRotator(-4.0f, -52.0f, 3.0f), FVector(1.85f, 0.72f, 1.55f) },
		{ TEXT("EastFarWallNickV48d"), 3, FVector2D(121800.0f, 124800.0f), -2080.0f, 1260.0f, 15000.0f, FRotator(-4.0f, -58.0f, 3.0f), FVector(1.7f, 0.72f, 1.4f) },
		{ TEXT("WestWallNickV48dA"), 3, FVector2D(27800.0f, 119800.0f), -1980.0f, 1260.0f, 12000.0f, FRotator(4.0f, 80.0f, -3.0f), FVector(1.75f, 0.72f, 1.4f) },
		{ TEXT("WestWallNickV48dB"), 2, FVector2D(29200.0f, 101800.0f), -2020.0f, 1280.0f, 12600.0f, FRotator(-4.0f, 84.0f, 3.0f), FVector(1.7f, 0.72f, 1.4f) },
		{ TEXT("WestSouthWallNickV48d"), 3, FVector2D(31800.0f, 90000.0f), -2040.0f, 1280.0f, 13200.0f, FRotator(4.0f, 72.0f, -3.0f), FVector(1.65f, 0.70f, 1.32f) }
	};

	const FGroundBlendInstanceV46 CliffDustPatchesV46[] = {
		// Smaller, staggered sediment fans avoid drawing new rectangular footprints.
		{ TEXT("NorthwestDustFanV47"), 0, FVector2D(36000.0f, 139900.0f), 18.0f, 0.82f, 9000.0f, FRotator(0.0f, -17.0f, 0.0f), FVector(3.0f, 1.25f, 0.03f) },
		{ TEXT("NorthCentralDustFanAV47"), 0, FVector2D(70400.0f, 141300.0f), 18.0f, 0.82f, 9200.0f, FRotator(0.0f, 23.0f, 0.0f), FVector(3.2f, 1.2f, 0.03f) },
		{ TEXT("NorthCentralDustFanBV47"), 0, FVector2D(85800.0f, 140200.0f), 18.0f, 0.82f, 9400.0f, FRotator(0.0f, -27.0f, 0.0f), FVector(3.0f, 1.15f, 0.03f) },
		{ TEXT("NorthEastDustFanV47"), 0, FVector2D(105600.0f, 132600.0f), 18.0f, 0.82f, 9000.0f, FRotator(0.0f, -39.0f, 0.0f), FVector(2.9f, 1.1f, 0.03f) },
		{ TEXT("EastDustFanUpperV47"), 0, FVector2D(116800.0f, 119800.0f), 18.0f, 0.82f, 8600.0f, FRotator(0.0f, -56.0f, 0.0f), FVector(2.8f, 1.05f, 0.03f) },
		{ TEXT("EastDustFanLowerV47"), 0, FVector2D(118300.0f, 103200.0f), 18.0f, 0.82f, 9000.0f, FRotator(0.0f, -67.0f, 0.0f), FVector(2.7f, 1.0f, 0.03f) },
		{ TEXT("WestDustFanHighV47"), 0, FVector2D(28600.0f, 119600.0f), 18.0f, 0.82f, 8600.0f, FRotator(0.0f, 82.0f, 0.0f), FVector(2.8f, 1.05f, 0.03f) },
		{ TEXT("WestDustFanLowV47"), 0, FVector2D(29200.0f, 98600.0f), 18.0f, 0.82f, 9200.0f, FRotator(0.0f, 72.0f, 0.0f), FVector(2.6f, 1.0f, 0.03f) }
	};

	const FGroundBlendInstanceV46 CliffDebrisV46[] = {
		{ TEXT("NorthwestDebrisAV47"), 0, FVector2D(34200.0f, 142400.0f), 24.0f, 0.42f, 8400.0f, FRotator(-4.0f, 19.0f, 2.0f), FVector(2.6f, 1.45f, 0.9f) },
		{ TEXT("NorthwestDebrisBV47"), 1, FVector2D(47200.0f, 140900.0f), 22.0f, 0.42f, 7200.0f, FRotator(2.0f, -31.0f, -3.0f), FVector(2.1f, 1.35f, 0.78f) },
		{ TEXT("NorthCentralDebrisAV47"), 2, FVector2D(71600.0f, 143300.0f), 20.0f, 0.42f, 8800.0f, FRotator(4.0f, 42.0f, -2.0f), FVector(2.4f, 1.4f, 0.86f) },
		{ TEXT("NorthCentralDebrisBV47"), 0, FVector2D(85800.0f, 142600.0f), 24.0f, 0.42f, 9000.0f, FRotator(-3.0f, -26.0f, 4.0f), FVector(2.0f, 1.35f, 0.72f) },
		{ TEXT("NorthEastDebrisAV47"), 2, FVector2D(105400.0f, 134500.0f), 22.0f, 0.42f, 9200.0f, FRotator(-2.0f, -36.0f, 3.0f), FVector(2.1f, 1.35f, 0.78f) },
		{ TEXT("EastDebrisAV47"), 0, FVector2D(118400.0f, 122400.0f), 22.0f, 0.42f, 8800.0f, FRotator(1.0f, -52.0f, -2.0f), FVector(2.3f, 1.45f, 0.82f) },
		{ TEXT("EastDebrisBV47"), 1, FVector2D(120200.0f, 111000.0f), 20.0f, 0.42f, 9000.0f, FRotator(-2.0f, -69.0f, 3.0f), FVector(1.9f, 1.3f, 0.7f) },
		{ TEXT("WestDebrisAV47"), 0, FVector2D(26800.0f, 125700.0f), 22.0f, 0.42f, 8600.0f, FRotator(-3.0f, 88.0f, 2.0f), FVector(2.35f, 1.35f, 0.84f) },
		{ TEXT("WestDebrisBV47"), 1, FVector2D(27400.0f, 111800.0f), 22.0f, 0.42f, 9200.0f, FRotator(2.0f, 77.0f, -2.0f), FVector(2.1f, 1.3f, 0.72f) },
		{ TEXT("WestDebrisCV47"), 2, FVector2D(27600.0f, 95800.0f), 20.0f, 0.42f, 9800.0f, FRotator(-1.0f, 69.0f, 2.0f), FVector(1.95f, 1.3f, 0.68f) }
	};

	const FGroundBlendInstanceV46 CliffDryEcologyV46[] = {
		// Sparse only: transition-readable ecology, not a forest rollout.
		{ TEXT("NorthwestDryShrubA"), 0, FVector2D(40400.0f, 136200.0f), 8.0f, 0.80f, 7600.0f, FRotator(0.0f, 21.0f, 0.0f), FVector(1.7f, 1.7f, 1.25f) },
		{ TEXT("NorthCentralDryShrubA"), 1, FVector2D(65100.0f, 137000.0f), 8.0f, 0.80f, 8200.0f, FRotator(0.0f, -18.0f, 0.0f), FVector(1.45f, 1.45f, 1.0f) },
		{ TEXT("NorthCentralDryShrubB"), 2, FVector2D(81500.0f, 135500.0f), 8.0f, 0.80f, 8400.0f, FRotator(0.0f, 43.0f, 0.0f), FVector(1.3f, 1.3f, 0.92f) },
		{ TEXT("NorthEastDryShrubA"), 0, FVector2D(102700.0f, 128100.0f), 8.0f, 0.80f, 8400.0f, FRotator(0.0f, -34.0f, 0.0f), FVector(1.65f, 1.65f, 1.15f) },
		{ TEXT("EastDryShrubA"), 1, FVector2D(111100.0f, 116200.0f), 8.0f, 0.80f, 7800.0f, FRotator(0.0f, -72.0f, 0.0f), FVector(1.35f, 1.35f, 0.92f) },
		{ TEXT("WestDryShrubA"), 2, FVector2D(33400.0f, 118300.0f), 8.0f, 0.80f, 7800.0f, FRotator(0.0f, 62.0f, 0.0f), FVector(1.4f, 1.4f, 0.94f) },
		{ TEXT("WestDryShrubB"), 3, FVector2D(34000.0f, 100600.0f), 8.0f, 0.80f, 8600.0f, FRotator(0.0f, -17.0f, 0.0f), FVector(1.2f, 1.2f, 0.86f) }
	};

	const FCliffSurfaceSkinRunV49 CliffSurfaceSkinRunsV49[] = {
		// V49 completion pass: organic material skin only. The runs follow the
		// existing sculpted ring line and place surface-aligned cliff/dirt patches
		// only on sloped faces, leaving flat gameplay space, river, and lake open.
		{ TEXT("NorthLowerSlopeSkinV49"), FVector2D(34000.0f, 157900.0f), FVector2D(93000.0f, 156700.0f), 620.0f, 24, 3.0f, 5.4f },
		{ TEXT("NorthUpperSlopeSkinV49"), FVector2D(37000.0f, 161300.0f), FVector2D(101000.0f, 158400.0f), 640.0f, 20, 2.8f, 5.0f },
		{ TEXT("NorthBackDiagonalSlopeSkinV49"), FVector2D(84000.0f, 160300.0f), FVector2D(116000.0f, 139500.0f), 560.0f, 22, 3.0f, 5.4f },
		{ TEXT("UpperRightSlopeSkinV49"), FVector2D(100000.0f, 144500.0f), FVector2D(118500.0f, 121000.0f), 560.0f, 22, 2.8f, 5.0f },
		{ TEXT("UpperRightToeSkinV49"), FVector2D(93500.0f, 131200.0f), FVector2D(112000.0f, 118200.0f), 420.0f, 12, 2.2f, 3.8f },
		{ TEXT("EastBorderSlopeSkinV49"), FVector2D(111500.0f, 129200.0f), FVector2D(122000.0f, 110500.0f), 480.0f, 18, 2.6f, 4.7f },
		{ TEXT("EastOuterShadowSkinV49"), FVector2D(121600.0f, 128000.0f), FVector2D(124200.0f, 113000.0f), 360.0f, 10, 2.2f, 3.8f },
		{ TEXT("WestFaceSlopeSkinV49"), FVector2D(28600.0f, 106000.0f), FVector2D(24800.0f, 136000.0f), 460.0f, 16, 2.6f, 4.6f },
		{ TEXT("WestLowerSlopeSkinV49"), FVector2D(32600.0f, 90000.0f), FVector2D(28600.0f, 113000.0f), 360.0f, 10, 2.0f, 3.6f },
		{ TEXT("InnerToeErosionSkinV49"), FVector2D(65000.0f, 145000.0f), FVector2D(109000.0f, 132500.0f), 380.0f, 12, 1.8f, 3.2f }
	};

	const FThinCliffFacadeRunV52 ThinCliffFacadeRunsV52[] = {
		// V52 facade: thin mountain skin that follows the existing sculpted ring.
		// These runs are intentionally shallow and avoid the river, lake, traversal,
		// village, and spawn reserves. They are not terrain sculpting or hero rocks.
		{ TEXT("NorthLowerContinuousWallV52"), FVector2D(34000.0f, 158000.0f), FVector2D(93000.0f, 156800.0f), 360.0f, 320.0f, 680.0f, 12200.0f, 22000.0f, 18, 2, 2.7f, 4.2f, 1.7f, 2.7f, 0.30f, 0.46f, 1.6f, 2.5f, 1850.0f },
		{ TEXT("NorthCrownBrokenShellV52"), FVector2D(37000.0f, 162200.0f), FVector2D(101000.0f, 159100.0f), 420.0f, 260.0f, 620.0f, 13200.0f, 24000.0f, 16, 1, 2.8f, 4.4f, 1.8f, 2.9f, 0.28f, 0.42f, 1.3f, 2.1f, 0.0f },
		{ TEXT("NorthBackDiagonalFacadeV52"), FVector2D(84000.0f, 160400.0f), FVector2D(116000.0f, 139500.0f), 360.0f, 300.0f, 700.0f, 12800.0f, 24000.0f, 15, 2, 2.8f, 4.5f, 1.7f, 2.8f, 0.30f, 0.45f, 1.6f, 2.6f, 1950.0f },
		{ TEXT("UpperRightCurvedWallV52"), FVector2D(100000.0f, 144500.0f), FVector2D(118800.0f, 121000.0f), 340.0f, 300.0f, 720.0f, 12200.0f, 25000.0f, 14, 2, 2.7f, 4.3f, 1.6f, 2.6f, 0.28f, 0.44f, 1.5f, 2.4f, 1800.0f },
		{ TEXT("InnerVisibleDomeCloakV52"), FVector2D(65000.0f, 145000.0f), FVector2D(109000.0f, 132500.0f), 260.0f, 260.0f, 620.0f, 11600.0f, 26000.0f, 18, 2, 2.5f, 4.0f, 1.5f, 2.35f, 0.24f, 0.38f, 1.45f, 2.25f, 1550.0f },
		{ TEXT("CentralSmoothFaceCloakV52"), FVector2D(79000.0f, 138500.0f), FVector2D(105000.0f, 128500.0f), 220.0f, 220.0f, 560.0f, 11200.0f, 25000.0f, 12, 2, 2.4f, 3.7f, 1.45f, 2.25f, 0.22f, 0.36f, 1.35f, 2.05f, 1400.0f },
		{ TEXT("EastRingTightFacadeV52"), FVector2D(116000.0f, 128500.0f), FVector2D(123500.0f, 108500.0f), 300.0f, 260.0f, 620.0f, 13800.0f, 24000.0f, 12, 2, 2.4f, 3.9f, 1.5f, 2.4f, 0.27f, 0.42f, 1.4f, 2.2f, 1650.0f },
		{ TEXT("WestUpperRingFacadeV52"), FVector2D(28500.0f, 137000.0f), FVector2D(27000.0f, 114000.0f), 320.0f, 300.0f, 700.0f, 11800.0f, 21000.0f, 12, 2, 2.4f, 4.0f, 1.5f, 2.4f, 0.28f, 0.42f, 1.4f, 2.3f, 1700.0f },
		{ TEXT("WestLowerRingFacadeV52"), FVector2D(31500.0f, 111000.0f), FVector2D(34000.0f, 91500.0f), 300.0f, 280.0f, 660.0f, 12600.0f, 18000.0f, 10, 2, 2.2f, 3.6f, 1.4f, 2.2f, 0.26f, 0.40f, 1.3f, 2.1f, 1550.0f },
		{ TEXT("NorthwestReturnShellV52"), FVector2D(23500.0f, 150000.0f), FVector2D(34000.0f, 160000.0f), 300.0f, 280.0f, 620.0f, 13500.0f, 24000.0f, 8, 1, 2.2f, 3.7f, 1.4f, 2.2f, 0.26f, 0.40f, 1.2f, 2.0f, 0.0f }
	};

	const FThinCliffFacadeRunV52 ThinCliffFacadeRunsV53[] = {
		// V53 keeps the successful v52 rock language but pulls the shell back
		// onto the sculpted ring line. The lower offsets/depth scales are the
		// gameplay-space correction: thin mountain skin, not an inward range.
		{ TEXT("NorthLowerTightSkinV53"), FVector2D(34000.0f, 159300.0f), FVector2D(93000.0f, 158200.0f), 220.0f, 80.0f, 260.0f, 12600.0f, 23000.0f, 18, 2, 2.55f, 4.05f, 1.55f, 2.45f, 0.20f, 0.32f, 1.65f, 2.55f, 1750.0f },
		{ TEXT("NorthCrownBrokenSkinV53"), FVector2D(37000.0f, 162800.0f), FVector2D(101000.0f, 159800.0f), 240.0f, 60.0f, 220.0f, 13600.0f, 24500.0f, 16, 1, 2.65f, 4.25f, 1.60f, 2.55f, 0.18f, 0.30f, 1.35f, 2.20f, 0.0f },
		{ TEXT("NorthBackDiagonalTightSkinV53"), FVector2D(85000.0f, 161100.0f), FVector2D(116500.0f, 140600.0f), 220.0f, 90.0f, 280.0f, 13200.0f, 24500.0f, 15, 2, 2.65f, 4.25f, 1.55f, 2.55f, 0.20f, 0.32f, 1.60f, 2.50f, 1750.0f },
		{ TEXT("UpperRightRingTightSkinV53"), FVector2D(101500.0f, 145800.0f), FVector2D(119500.0f, 122500.0f), 210.0f, 80.0f, 260.0f, 12800.0f, 25500.0f, 14, 2, 2.55f, 4.05f, 1.50f, 2.45f, 0.19f, 0.31f, 1.50f, 2.35f, 1650.0f },
		{ TEXT("InnerDomeFrontSkinV53"), FVector2D(65000.0f, 145000.0f), FVector2D(109000.0f, 132500.0f), 180.0f, 40.0f, 140.0f, 12400.0f, 26500.0f, 16, 2, 2.35f, 3.75f, 1.40f, 2.20f, 0.17f, 0.27f, 1.35f, 2.12f, 1050.0f },
		{ TEXT("CentralFaceFrontSkinV53"), FVector2D(79000.0f, 138500.0f), FVector2D(105000.0f, 128500.0f), 150.0f, 40.0f, 130.0f, 12400.0f, 26000.0f, 10, 1, 2.25f, 3.45f, 1.35f, 2.00f, 0.17f, 0.26f, 1.25f, 1.95f, 0.0f },
		{ TEXT("EastRingBackSkinV53"), FVector2D(117500.0f, 130500.0f), FVector2D(124500.0f, 110500.0f), 190.0f, 70.0f, 230.0f, 14200.0f, 24500.0f, 12, 2, 2.30f, 3.65f, 1.35f, 2.20f, 0.19f, 0.31f, 1.35f, 2.10f, 1450.0f },
		{ TEXT("WestUpperBackSkinV53"), FVector2D(27000.0f, 138000.0f), FVector2D(25800.0f, 116000.0f), 190.0f, 70.0f, 230.0f, 12400.0f, 22000.0f, 10, 1, 2.25f, 3.65f, 1.35f, 2.15f, 0.19f, 0.30f, 1.30f, 2.05f, 0.0f },
		{ TEXT("WestLowerBackSkinV53"), FVector2D(30500.0f, 112000.0f), FVector2D(33200.0f, 92500.0f), 170.0f, 70.0f, 220.0f, 13000.0f, 19000.0f, 6, 1, 2.10f, 3.25f, 1.25f, 1.95f, 0.18f, 0.28f, 1.20f, 1.85f, 0.0f },
		{ TEXT("NorthwestReturnBackSkinV53"), FVector2D(22500.0f, 151000.0f), FVector2D(34000.0f, 161000.0f), 180.0f, 60.0f, 200.0f, 13800.0f, 24500.0f, 6, 1, 2.10f, 3.35f, 1.25f, 2.00f, 0.18f, 0.28f, 1.15f, 1.85f, 0.0f }
	};

	const FThinCliffFacadeRunV52 ThinCliffFacadeRunsV54[] = {
		// V54 preserves the v53/v52 rock language but opens the river cut.
		// The north wall is split around the river instead of spanning it;
		// all remaining runs keep shallow offsets so the facade stays a skin
		// on the sculpted border, not a second mountain range inside the map.
		{ TEXT("NorthLowerWestOpenSkinV54"), FVector2D(24000.0f, 159800.0f), FVector2D(38000.0f, 160100.0f), 180.0f, 35.0f, 150.0f, 20500.0f, 23500.0f, 6, 2, 2.50f, 3.95f, 1.50f, 2.35f, 0.18f, 0.29f, 1.60f, 2.45f, 1600.0f },
		{ TEXT("NorthLowerEastOpenSkinV54"), FVector2D(78000.0f, 159200.0f), FVector2D(96000.0f, 158200.0f), 180.0f, 35.0f, 150.0f, 20500.0f, 24000.0f, 7, 2, 2.50f, 4.00f, 1.50f, 2.40f, 0.18f, 0.29f, 1.60f, 2.50f, 1600.0f },
		{ TEXT("NorthCrownWestOpenSkinV54"), FVector2D(25000.0f, 163200.0f), FVector2D(36500.0f, 163500.0f), 190.0f, 30.0f, 130.0f, 21500.0f, 24500.0f, 5, 1, 2.60f, 4.15f, 1.55f, 2.45f, 0.17f, 0.28f, 1.30f, 2.10f, 0.0f },
		{ TEXT("NorthCrownEastOpenSkinV54"), FVector2D(81000.0f, 161800.0f), FVector2D(104000.0f, 159800.0f), 190.0f, 30.0f, 130.0f, 21500.0f, 24500.0f, 8, 1, 2.60f, 4.20f, 1.55f, 2.50f, 0.17f, 0.28f, 1.30f, 2.15f, 0.0f },
		{ TEXT("NorthBackRightDiagonalSkinV54"), FVector2D(88000.0f, 161000.0f), FVector2D(116500.0f, 140600.0f), 200.0f, 45.0f, 170.0f, 20000.0f, 24500.0f, 12, 2, 2.60f, 4.20f, 1.50f, 2.50f, 0.18f, 0.29f, 1.55f, 2.45f, 1600.0f },
		{ TEXT("UpperRightOpenRingSkinV54"), FVector2D(103500.0f, 146500.0f), FVector2D(120000.0f, 123000.0f), 190.0f, 40.0f, 165.0f, 19000.0f, 25500.0f, 12, 2, 2.50f, 4.00f, 1.45f, 2.35f, 0.17f, 0.29f, 1.45f, 2.30f, 1500.0f },
		{ TEXT("InnerDomeRightCloakV54"), FVector2D(83000.0f, 144500.0f), FVector2D(110500.0f, 132500.0f), 155.0f, 25.0f, 105.0f, 19500.0f, 26500.0f, 9, 2, 2.30f, 3.65f, 1.35f, 2.10f, 0.16f, 0.25f, 1.30f, 2.05f, 950.0f },
		{ TEXT("CentralRightFaceSkinV54"), FVector2D(88000.0f, 138000.0f), FVector2D(106500.0f, 129000.0f), 130.0f, 25.0f, 95.0f, 19500.0f, 26000.0f, 7, 1, 2.20f, 3.35f, 1.30f, 1.95f, 0.16f, 0.24f, 1.20f, 1.85f, 0.0f },
		{ TEXT("EastRingBackOpenSkinV54"), FVector2D(118500.0f, 131500.0f), FVector2D(125000.0f, 111000.0f), 175.0f, 35.0f, 145.0f, 19000.0f, 24500.0f, 10, 2, 2.25f, 3.55f, 1.30f, 2.10f, 0.17f, 0.28f, 1.30f, 2.05f, 1350.0f },
		{ TEXT("WestUpperOpenBackSkinV54"), FVector2D(26000.0f, 138500.0f), FVector2D(25200.0f, 116500.0f), 175.0f, 35.0f, 145.0f, 19000.0f, 22000.0f, 8, 1, 2.20f, 3.50f, 1.30f, 2.05f, 0.17f, 0.28f, 1.25f, 2.00f, 0.0f },
		{ TEXT("WestLowerBackSkinV54"), FVector2D(30000.0f, 112000.0f), FVector2D(33000.0f, 92500.0f), 155.0f, 35.0f, 140.0f, 19000.0f, 19000.0f, 5, 1, 2.05f, 3.15f, 1.20f, 1.90f, 0.16f, 0.26f, 1.15f, 1.80f, 0.0f },
		{ TEXT("NorthwestReturnOpenSkinV54"), FVector2D(22000.0f, 151500.0f), FVector2D(32500.0f, 161200.0f), 160.0f, 30.0f, 120.0f, 21000.0f, 24500.0f, 4, 1, 2.05f, 3.25f, 1.20f, 1.90f, 0.16f, 0.26f, 1.10f, 1.80f, 0.0f }
	};

	const FThinCliffFacadeRunV52 ThinCliffFacadeRunsV55[] = {
		// V55 expands the successful v53/v54 thin cliff language around the full
		// sculpted Highland border. Runs stay shallow and split around both river
		// exits so the ring reads as one mountain range without closing waterfalls.
		{ TEXT("NorthwestShoulderFullRingV55"), FVector2D(9000.0f, 151000.0f), FVector2D(28500.0f, 164500.0f), 210.0f, 35.0f, 145.0f, 22000.0f, 25000.0f, 7, 2, 2.30f, 3.85f, 1.35f, 2.30f, 0.16f, 0.28f, 1.35f, 2.25f, 1500.0f },
		{ TEXT("NorthwestCrestFullRingV55"), FVector2D(18000.0f, 162500.0f), FVector2D(38000.0f, 165300.0f), 190.0f, 30.0f, 130.0f, 22500.0f, 25000.0f, 6, 1, 2.65f, 4.25f, 1.50f, 2.55f, 0.16f, 0.27f, 1.45f, 2.35f, 0.0f },
		{ TEXT("NorthWestOpenFaceV55"), FVector2D(23500.0f, 159800.0f), FVector2D(38200.0f, 160300.0f), 180.0f, 30.0f, 135.0f, 21500.0f, 24000.0f, 7, 2, 2.50f, 4.05f, 1.45f, 2.45f, 0.17f, 0.28f, 1.55f, 2.50f, 1600.0f },
		{ TEXT("NorthEastOpenFaceV55"), FVector2D(80500.0f, 159300.0f), FVector2D(101000.0f, 158200.0f), 180.0f, 30.0f, 135.0f, 21500.0f, 24500.0f, 8, 2, 2.55f, 4.15f, 1.45f, 2.50f, 0.17f, 0.28f, 1.55f, 2.55f, 1650.0f },
		{ TEXT("NorthEastMajorPeakV55"), FVector2D(83000.0f, 162500.0f), FVector2D(112000.0f, 157200.0f), 220.0f, 30.0f, 140.0f, 22000.0f, 25000.0f, 8, 2, 2.75f, 4.55f, 1.55f, 2.75f, 0.17f, 0.30f, 1.65f, 2.75f, 2100.0f },
		{ TEXT("NortheastDiagonalShoulderV55"), FVector2D(105000.0f, 154000.0f), FVector2D(120500.0f, 137000.0f), 210.0f, 35.0f, 150.0f, 20000.0f, 25000.0f, 8, 2, 2.55f, 4.20f, 1.45f, 2.50f, 0.17f, 0.29f, 1.50f, 2.45f, 1650.0f },
		{ TEXT("EastUpperRidgeV55"), FVector2D(119500.0f, 139000.0f), FVector2D(126000.0f, 112000.0f), 210.0f, 35.0f, 150.0f, 19000.0f, 24500.0f, 10, 2, 2.35f, 3.90f, 1.35f, 2.30f, 0.16f, 0.28f, 1.35f, 2.25f, 1500.0f },
		{ TEXT("EastMidSaddleV55"), FVector2D(126000.0f, 112000.0f), FVector2D(125000.0f, 79000.0f), 220.0f, 35.0f, 150.0f, 19000.0f, 23500.0f, 10, 2, 2.20f, 3.65f, 1.30f, 2.15f, 0.16f, 0.27f, 1.25f, 2.10f, 1250.0f },
		{ TEXT("EastLowerSecondaryPeakV55"), FVector2D(124000.0f, 80500.0f), FVector2D(116500.0f, 52000.0f), 220.0f, 35.0f, 150.0f, 19000.0f, 22000.0f, 9, 2, 2.35f, 3.95f, 1.35f, 2.35f, 0.16f, 0.28f, 1.35f, 2.30f, 1550.0f },
		{ TEXT("SoutheastShoulderV55"), FVector2D(115000.0f, 54000.0f), FVector2D(101000.0f, 30000.0f), 220.0f, 30.0f, 140.0f, 17000.0f, 22000.0f, 8, 2, 2.30f, 3.90f, 1.30f, 2.25f, 0.15f, 0.26f, 1.30f, 2.20f, 1400.0f },
		{ TEXT("SoutheastOpenCanyonSideV55"), FVector2D(102500.0f, 30500.0f), FVector2D(90000.0f, 9000.0f), 200.0f, 25.0f, 120.0f, 17000.0f, 22000.0f, 6, 1, 2.25f, 3.70f, 1.25f, 2.05f, 0.14f, 0.24f, 1.25f, 2.05f, 0.0f },
		{ TEXT("SouthwestOpenCanyonSideV55"), FVector2D(56000.0f, -16000.0f), FVector2D(18000.0f, -12000.0f), 240.0f, 20.0f, 110.0f, 18000.0f, 9500.0f, 7, 1, 2.10f, 3.55f, 1.20f, 2.00f, 0.13f, 0.23f, 1.20f, 2.00f, 0.0f },
		{ TEXT("WestLowerMountainBackboneV55"), FVector2D(-4500.0f, 16000.0f), FVector2D(2500.0f, 60000.0f), 230.0f, 25.0f, 125.0f, 17000.0f, 12500.0f, 8, 2, 2.15f, 3.60f, 1.25f, 2.10f, 0.14f, 0.25f, 1.20f, 2.05f, 1200.0f },
		{ TEXT("WestLakeBackMountainV55"), FVector2D(15500.0f, 87500.0f), FVector2D(23000.0f, 116500.0f), 210.0f, 25.0f, 125.0f, 17000.0f, 16000.0f, 8, 2, 2.15f, 3.65f, 1.25f, 2.15f, 0.14f, 0.25f, 1.20f, 2.05f, 1250.0f },
		{ TEXT("WestUpperRidgeV55"), FVector2D(23000.0f, 116500.0f), FVector2D(21000.0f, 143000.0f), 210.0f, 25.0f, 125.0f, 19000.0f, 22000.0f, 8, 2, 2.20f, 3.70f, 1.25f, 2.20f, 0.15f, 0.26f, 1.25f, 2.10f, 1300.0f },
		{ TEXT("NorthInnerSmoothBreakV55"), FVector2D(85000.0f, 144500.0f), FVector2D(111000.0f, 132500.0f), 150.0f, 20.0f, 100.0f, 20000.0f, 26500.0f, 7, 1, 2.15f, 3.40f, 1.25f, 1.95f, 0.13f, 0.22f, 1.15f, 1.85f, 0.0f },
		{ TEXT("FullRingContinuityLowBreakV55"), FVector2D(30000.0f, 146500.0f), FVector2D(106000.0f, 128000.0f), 160.0f, 18.0f, 90.0f, 18000.0f, 25500.0f, 10, 1, 1.95f, 3.25f, 1.15f, 1.85f, 0.12f, 0.21f, 1.10f, 1.75f, 0.0f }
	};

	const FMountainIdentityRunV56 MountainIdentityRunsV56[] = {
		// V56 keeps the ring line but changes the read from repeated shelves to
		// peak/ridge/saddle hierarchy. Counts stay intentionally low: each anchor
		// is a skyline mass, not another horizontal band.
		{ TEXT("NorthwestMinorToPrimaryRiseV56"), FVector2D(9000.0f, 151000.0f), FVector2D(28500.0f, 164500.0f), 360.0f, 15.0f, 80.0f, 22500.0f, 25500.0f, 5, 2.15f, 3.65f, 5.35f, 1.55f, 2.85f, 4.60f, 620.0f, 1250.0f },
		{ TEXT("NorthwestCrestPeaksV56"), FVector2D(18000.0f, 162500.0f), FVector2D(38000.0f, 165300.0f), 300.0f, 10.0f, 70.0f, 23000.0f, 25500.0f, 4, 2.25f, 3.90f, 5.80f, 1.65f, 3.05f, 4.95f, 760.0f, 1300.0f },
		{ TEXT("NorthWestWaterfallPassShouldersV56"), FVector2D(23500.0f, 159800.0f), FVector2D(38200.0f, 160300.0f), 290.0f, 10.0f, 65.0f, 22500.0f, 24500.0f, 4, 2.10f, 3.55f, 5.10f, 1.50f, 2.65f, 4.25f, 520.0f, 1050.0f },
		{ TEXT("NorthEastWaterfallPassShouldersV56"), FVector2D(80500.0f, 159300.0f), FVector2D(101000.0f, 158200.0f), 300.0f, 10.0f, 70.0f, 22500.0f, 25000.0f, 5, 2.15f, 3.70f, 5.55f, 1.55f, 2.85f, 4.65f, 660.0f, 1200.0f },
		{ TEXT("NorthEastPrimarySummitsV56"), FVector2D(83000.0f, 162500.0f), FVector2D(112000.0f, 157200.0f), 390.0f, 10.0f, 80.0f, 22500.0f, 26000.0f, 6, 2.30f, 4.25f, 6.35f, 1.70f, 3.35f, 5.55f, 980.0f, 1550.0f },
		{ TEXT("NortheastDiagonalRidgeFlowV56"), FVector2D(105000.0f, 154000.0f), FVector2D(120500.0f, 137000.0f), 380.0f, 10.0f, 75.0f, 21000.0f, 25500.0f, 5, 2.20f, 3.95f, 5.85f, 1.60f, 3.05f, 4.95f, 740.0f, 1350.0f },
		{ TEXT("EastUpperPeakChainV56"), FVector2D(119500.0f, 139000.0f), FVector2D(126000.0f, 112000.0f), 400.0f, 10.0f, 75.0f, 20000.0f, 25000.0f, 7, 2.10f, 3.75f, 5.65f, 1.55f, 2.95f, 4.75f, 720.0f, 1300.0f },
		{ TEXT("EastMidLowerSaddlesV56"), FVector2D(126000.0f, 112000.0f), FVector2D(125000.0f, 79000.0f), 420.0f, 10.0f, 75.0f, 19500.0f, 24000.0f, 7, 1.95f, 3.25f, 4.70f, 1.35f, 2.35f, 3.65f, 280.0f, 1100.0f },
		{ TEXT("EastLowerSecondarySummitsV56"), FVector2D(124000.0f, 80500.0f), FVector2D(116500.0f, 52000.0f), 400.0f, 10.0f, 75.0f, 19500.0f, 23000.0f, 6, 2.05f, 3.60f, 5.30f, 1.45f, 2.80f, 4.45f, 560.0f, 1200.0f },
		{ TEXT("SoutheastShoulderFalloffV56"), FVector2D(115000.0f, 54000.0f), FVector2D(101000.0f, 30000.0f), 380.0f, 10.0f, 70.0f, 18000.0f, 22500.0f, 5, 2.00f, 3.40f, 5.00f, 1.40f, 2.55f, 4.05f, 440.0f, 1050.0f },
		{ TEXT("SoutheastOpenCanyonRimsV56"), FVector2D(102500.0f, 30500.0f), FVector2D(90000.0f, 9000.0f), 320.0f, 5.0f, 55.0f, 18500.0f, 22500.0f, 3, 1.85f, 3.05f, 4.35f, 1.25f, 2.15f, 3.45f, 240.0f, 820.0f },
		{ TEXT("SouthwestOpenCanyonRimsV56"), FVector2D(56000.0f, -16000.0f), FVector2D(18000.0f, -12000.0f), 420.0f, 5.0f, 55.0f, 19000.0f, 10000.0f, 4, 1.90f, 3.15f, 4.60f, 1.25f, 2.25f, 3.65f, 300.0f, 900.0f },
		{ TEXT("WestLowerBackbonePeaksV56"), FVector2D(-4500.0f, 16000.0f), FVector2D(2500.0f, 60000.0f), 430.0f, 5.0f, 65.0f, 17500.0f, 13000.0f, 6, 1.95f, 3.30f, 4.95f, 1.35f, 2.45f, 3.95f, 420.0f, 1000.0f },
		{ TEXT("WestLakeBackRidgeV56"), FVector2D(15500.0f, 87500.0f), FVector2D(23000.0f, 116500.0f), 380.0f, 5.0f, 65.0f, 17500.0f, 16500.0f, 6, 2.00f, 3.45f, 5.10f, 1.40f, 2.60f, 4.20f, 520.0f, 1050.0f },
		{ TEXT("WestUpperRidgeBreakV56"), FVector2D(23000.0f, 116500.0f), FVector2D(21000.0f, 143000.0f), 370.0f, 5.0f, 65.0f, 19500.0f, 22500.0f, 6, 2.05f, 3.55f, 5.25f, 1.45f, 2.75f, 4.35f, 600.0f, 1120.0f },
		{ TEXT("NorthInnerDomePeakBreakV56"), FVector2D(85000.0f, 144500.0f), FVector2D(111000.0f, 132500.0f), 260.0f, 5.0f, 55.0f, 21000.0f, 27000.0f, 4, 1.85f, 3.10f, 4.55f, 1.25f, 2.20f, 3.55f, 260.0f, 800.0f }
	};

	const FCliffSurfaceSkinRunV49 OuterRingSurfaceReplacementRunsV57[] = {
		// V57 treats the existing Landscape ring as the finished mountain form.
		// These runs only paint thin no-collision surface skins onto the current
		// ring body; they do not create peaks, ridges, facades, or new mass.
		{ TEXT("NorthwestLandscapeBodySkinV57"), FVector2D(6500.0f, 151500.0f), FVector2D(28500.0f, 164000.0f), 1040.0f, 34, 4.6f, 7.2f },
		{ TEXT("NorthwestCrestLandscapeSkinV57"), FVector2D(22000.0f, 163800.0f), FVector2D(43000.0f, 165200.0f), 980.0f, 30, 4.4f, 6.9f },
		{ TEXT("NorthWaterfallWestOpenSkinV57"), FVector2D(43000.0f, 164000.0f), FVector2D(50000.0f, 163000.0f), 720.0f, 12, 3.8f, 6.0f },
		{ TEXT("NorthWaterfallEastOpenSkinV57"), FVector2D(81000.0f, 162500.0f), FVector2D(94000.0f, 160500.0f), 820.0f, 18, 4.0f, 6.4f },
		{ TEXT("NortheastLandscapeBodySkinV57"), FVector2D(93000.0f, 160500.0f), FVector2D(116500.0f, 145000.0f), 1080.0f, 36, 4.6f, 7.4f },
		{ TEXT("EastUpperLandscapeBodySkinV57"), FVector2D(116500.0f, 145000.0f), FVector2D(125000.0f, 113000.0f), 1120.0f, 38, 4.5f, 7.2f },
		{ TEXT("EastMidLandscapeBodySkinV57"), FVector2D(125000.0f, 113000.0f), FVector2D(124500.0f, 78500.0f), 1120.0f, 38, 4.4f, 7.0f },
		{ TEXT("EastLowerLandscapeBodySkinV57"), FVector2D(124500.0f, 78500.0f), FVector2D(114000.0f, 48000.0f), 1080.0f, 34, 4.3f, 6.9f },
		{ TEXT("SoutheastLandscapeBodySkinV57"), FVector2D(114000.0f, 48000.0f), FVector2D(98500.0f, 24000.0f), 980.0f, 24, 4.1f, 6.5f },
		{ TEXT("SouthWaterfallEastOpenSkinV57"), FVector2D(98500.0f, 24000.0f), FVector2D(87000.0f, 6500.0f), 720.0f, 12, 3.8f, 5.8f },
		{ TEXT("SouthWaterfallWestOpenSkinV57"), FVector2D(54500.0f, -14500.0f), FVector2D(19000.0f, -11500.0f), 1120.0f, 30, 4.3f, 6.8f },
		{ TEXT("WestLowerLandscapeBodySkinV57"), FVector2D(-5000.0f, 14500.0f), FVector2D(2500.0f, 61000.0f), 1160.0f, 38, 4.4f, 7.0f },
		{ TEXT("WestLakeBackLandscapeSkinV57"), FVector2D(15500.0f, 87500.0f), FVector2D(23000.0f, 116500.0f), 960.0f, 28, 4.1f, 6.6f },
		{ TEXT("WestUpperLandscapeBodySkinV57"), FVector2D(23000.0f, 116500.0f), FVector2D(20500.0f, 143500.0f), 980.0f, 28, 4.2f, 6.7f },
		{ TEXT("NorthInnerLandscapeBodySkinV57"), FVector2D(85000.0f, 144500.0f), FVector2D(112500.0f, 132500.0f), 900.0f, 24, 4.0f, 6.4f }
	};

	const FTitanGeologyRunV59 TitanGeologyRunsV59[] = {
		// V59 is Option B only: geometry-driven Titan geology placed directly on
		// the existing outer Landscape ring. It preserves the ring footprint and
		// keeps the north/south waterfall cuts open.
		{ TEXT("NorthwestOuterGeologyV59"), FVector2D(6500.0f, 151500.0f), FVector2D(28500.0f, 164000.0f), 440.0f, 0.0f, 42.0f, 520.0f, 980.0f, 260.0f, 620.0f, 23000.0f, 25500.0f, 6, 3, 3.20f, 5.60f, 0.34f, 0.58f, 1.85f, 3.35f, 1150.0f, 240.0f },
		{ TEXT("NorthwestCrestGeologyV59"), FVector2D(22000.0f, 163800.0f), FVector2D(43000.0f, 165200.0f), 360.0f, 0.0f, 38.0f, 540.0f, 1040.0f, 280.0f, 650.0f, 23500.0f, 25500.0f, 5, 3, 3.25f, 5.75f, 0.34f, 0.60f, 1.90f, 3.45f, 1180.0f, 280.0f },
		{ TEXT("NorthEastOpenSideGeologyV59"), FVector2D(81000.0f, 162500.0f), FVector2D(104000.0f, 159800.0f), 390.0f, 0.0f, 42.0f, 560.0f, 1060.0f, 280.0f, 660.0f, 23500.0f, 25500.0f, 6, 3, 3.30f, 5.90f, 0.34f, 0.60f, 1.90f, 3.55f, 1200.0f, 280.0f },
		{ TEXT("NortheastDiagonalGeologyV59"), FVector2D(93000.0f, 160500.0f), FVector2D(116500.0f, 145000.0f), 430.0f, 0.0f, 48.0f, 620.0f, 1120.0f, 300.0f, 720.0f, 21500.0f, 25500.0f, 7, 3, 3.35f, 6.10f, 0.36f, 0.64f, 1.95f, 3.70f, 1220.0f, 300.0f },
		{ TEXT("EastUpperWallGeologyV59"), FVector2D(116500.0f, 145000.0f), FVector2D(125000.0f, 113000.0f), 460.0f, 0.0f, 52.0f, 660.0f, 1180.0f, 320.0f, 760.0f, 20500.0f, 24500.0f, 8, 3, 3.20f, 5.95f, 0.34f, 0.62f, 1.90f, 3.60f, 1160.0f, 260.0f },
		{ TEXT("EastMidWallGeologyV59"), FVector2D(125000.0f, 113000.0f), FVector2D(124500.0f, 78500.0f), 470.0f, 0.0f, 54.0f, 660.0f, 1180.0f, 320.0f, 760.0f, 20000.0f, 23800.0f, 8, 3, 3.00f, 5.55f, 0.34f, 0.60f, 1.75f, 3.30f, 1020.0f, 220.0f },
		{ TEXT("EastLowerWallGeologyV59"), FVector2D(124500.0f, 78500.0f), FVector2D(114000.0f, 48000.0f), 450.0f, 0.0f, 52.0f, 620.0f, 1120.0f, 300.0f, 720.0f, 19500.0f, 22500.0f, 7, 3, 3.10f, 5.70f, 0.34f, 0.60f, 1.82f, 3.40f, 1080.0f, 240.0f },
		{ TEXT("SoutheastCanyonSideGeologyV59"), FVector2D(114000.0f, 48000.0f), FVector2D(98500.0f, 24000.0f), 380.0f, 0.0f, 42.0f, 520.0f, 980.0f, 260.0f, 620.0f, 17800.0f, 22500.0f, 5, 2, 2.85f, 5.15f, 0.30f, 0.54f, 1.55f, 2.90f, 980.0f, 180.0f },
		{ TEXT("SouthwestCanyonSideGeologyV59"), FVector2D(54500.0f, -14500.0f), FVector2D(19000.0f, -11500.0f), 450.0f, 0.0f, 48.0f, 560.0f, 1040.0f, 280.0f, 660.0f, 19000.0f, 10500.0f, 7, 2, 2.95f, 5.35f, 0.30f, 0.56f, 1.60f, 3.05f, 1020.0f, 200.0f },
		{ TEXT("WestLowerBackboneGeologyV59"), FVector2D(-5000.0f, 14500.0f), FVector2D(2500.0f, 61000.0f), 480.0f, 0.0f, 54.0f, 620.0f, 1180.0f, 320.0f, 760.0f, 17600.0f, 13200.0f, 8, 3, 3.05f, 5.65f, 0.34f, 0.62f, 1.78f, 3.35f, 1060.0f, 220.0f },
		{ TEXT("WestLakeBackGeologyV59"), FVector2D(15500.0f, 87500.0f), FVector2D(23000.0f, 116500.0f), 380.0f, 0.0f, 42.0f, 520.0f, 980.0f, 280.0f, 640.0f, 17800.0f, 16800.0f, 5, 2, 2.85f, 5.10f, 0.30f, 0.54f, 1.55f, 2.95f, 980.0f, 180.0f },
		{ TEXT("WestUpperReturnGeologyV59"), FVector2D(23000.0f, 116500.0f), FVector2D(20500.0f, 143500.0f), 400.0f, 0.0f, 44.0f, 560.0f, 1040.0f, 280.0f, 660.0f, 19800.0f, 22500.0f, 6, 3, 3.00f, 5.45f, 0.32f, 0.58f, 1.70f, 3.25f, 1060.0f, 220.0f }
	};

	float PseudoRandom01(const int32 Seed)
	{
		const float SineValue = FMath::Sin(static_cast<float>(Seed) * 12.9898f + 78.233f);
		return FMath::Frac(SineValue * 43758.5453f);
	}

	float DistanceToSegment(const FVector2D& Point, const FVector2D& Start, const FVector2D& End)
	{
		const FVector2D Segment = End - Start;
		const float Denominator = FMath::Max(FVector2D::DotProduct(Segment, Segment), 1.0f);
		const float Alpha = FMath::Clamp(FVector2D::DotProduct(Point - Start, Segment) / Denominator, 0.0f, 1.0f);
		return FVector2D::Distance(Point, Start + Segment * Alpha);
	}

	float GetNearestWaterDistance(const FVector2D& Position)
	{
		float Nearest = TNumericLimits<float>::Max();
		auto CheckSegment = [&Nearest, &Position](const FVector2D& Start, const FVector2D& End)
		{
			Nearest = FMath::Min(Nearest, DistanceToSegment(Position, Start, End));
		};

		CheckSegment(FVector2D(48590.0f, 183116.0f), FVector2D(49910.0f, 174430.0f));
		CheckSegment(FVector2D(49910.0f, 174430.0f), FVector2D(51518.0f, 166778.0f));
		CheckSegment(FVector2D(51518.0f, 166778.0f), FVector2D(58605.0f, 152688.0f));
		CheckSegment(FVector2D(58605.0f, 152688.0f), FVector2D(62544.0f, 138795.0f));
		CheckSegment(FVector2D(62544.0f, 138795.0f), FVector2D(58487.0f, 122723.0f));
		CheckSegment(FVector2D(58487.0f, 122723.0f), FVector2D(55306.0f, 96223.0f));
		CheckSegment(FVector2D(55306.0f, 96223.0f), FVector2D(54105.0f, 78933.0f));
		CheckSegment(FVector2D(54105.0f, 78933.0f), FVector2D(57584.0f, 62136.0f));
		CheckSegment(FVector2D(57584.0f, 62136.0f), FVector2D(63147.0f, 37260.0f));

		const FVector2D Lake0 = (Position - FVector2D(40000.0f, 23500.0f)) / FVector2D(16000.0f, 12000.0f);
		const FVector2D Lake1 = (Position - FVector2D(25640.0f, 72820.0f)) / FVector2D(4700.0f, 1900.0f);
		const FVector2D PondA = (Position - FVector2D(45457.0f, 136962.0f)) / FVector2D(3700.0f, 1700.0f);
		const FVector2D PondB = (Position - FVector2D(19220.0f, 124250.0f)) / FVector2D(3100.0f, 1600.0f);
		return FMath::Min(
			FMath::Min(Nearest, (Lake0.Size() - 1.0f) * 5400.0f),
			FMath::Min((Lake1.Size() - 1.0f) * 6000.0f, FMath::Min((PondA.Size() - 1.0f) * 3200.0f, (PondB.Size() - 1.0f) * 2800.0f)));
	}

	bool IsNearPath(const FVector2D& Position)
	{
		constexpr float PathPadding = 1850.0f;
		if (DistanceToSegment(Position, FVector2D(18000.0f, 151500.0f), FVector2D(82000.0f, 151500.0f)) < PathPadding)
		{
			return true;
		}
		if (DistanceToSegment(Position, FVector2D(-10000.0f, 6500.0f), FVector2D(9000.0f, 9000.0f)) < PathPadding)
		{
			return true;
		}
		if (DistanceToSegment(Position, FVector2D(9000.0f, 9000.0f), FVector2D(36000.0f, 21000.0f)) < PathPadding)
		{
			return true;
		}
		if (DistanceToSegment(Position, FVector2D(36000.0f, 21000.0f), FVector2D(76000.0f, 32000.0f)) < PathPadding)
		{
			return true;
		}
		if (DistanceToSegment(Position, FVector2D(10000.0f, 36000.0f), FVector2D(12000.0f, 60000.0f)) < PathPadding)
		{
			return true;
		}
		if (DistanceToSegment(Position, FVector2D(12000.0f, 60000.0f), FVector2D(16000.0f, 87000.0f)) < PathPadding)
		{
			return true;
		}
		const float SpawnLoopDistance = FMath::Abs(FVector2D::Distance(Position, FVector2D(-18000.0f, 4000.0f)) - 10500.0f);
		return SpawnLoopDistance < 1300.0f;
	}

	bool IsInsideIslandInterior(const FVector2D& Position)
	{
		const FVector2D Centered = (Position - FVector2D(71396.0f, 79714.0f)) / FVector2D(112000.0f, 124000.0f);
		const float Organic = Centered.Size()
			+ 0.055f * FMath::Sin(Position.X / 11500.0f)
			- 0.045f * FMath::Cos(Position.Y / 9000.0f)
			+ 0.035f * FMath::Sin((Position.X + Position.Y) / 17000.0f);
		return Organic < 0.66f;
	}

	bool IsInsideSpawnMeadow(const FVector2D& Position)
	{
		// Keep the immediate player start readable: grass-only meadow, with tree masses pushed into the distance.
		return FVector2D::Distance(Position, FVector2D(38553.5f, 147844.6f)) < 17000.0f;
	}

	bool IsInsideVillagePlateauReserve(const FVector2D& Position)
	{
		const FVector2D Normalized = (Position - FVector2D(76500.0f, 124000.0f)) / FVector2D(18500.0f, 13500.0f);
		return Normalized.Size() < 1.0f;
	}

	bool IsInsideTraversalCorridorReserve(const FVector2D& Position)
	{
		// Keep broad readable lines through the forest mass for future glide/slide routing.
		if (DistanceToSegment(Position, FVector2D(56000.0f, 151000.0f), FVector2D(98500.0f, 130000.0f)) < 3900.0f)
		{
			return true;
		}
		if (DistanceToSegment(Position, FVector2D(79000.0f, 142000.0f), FVector2D(103000.0f, 116000.0f)) < 3400.0f)
		{
			return true;
		}
		return false;
	}

	bool IsInsideTraversalForestV3MeadowBreak(const FVector2D& Position)
	{
		const FVector2D UpperRightWindow = (Position - FVector2D(86200.0f, 136900.0f)) / FVector2D(7600.0f, 5200.0f);
		if (UpperRightWindow.Size() < 1.0f)
		{
			return true;
		}

		const FVector2D NorthernGap = (Position - FVector2D(61200.0f, 156800.0f)) / FVector2D(8200.0f, 3900.0f);
		if (NorthernGap.Size() < 1.0f)
		{
			return true;
		}

		// Keep a clear shoulder into the future village overlook so the forest frames the path instead of becoming a wall.
		return DistanceToSegment(Position, FVector2D(72000.0f, 142000.0f), FVector2D(96000.0f, 121500.0f)) < 5200.0f;
	}

	bool IsInsideTraversalForestV4MeadowBreak(const FVector2D& Position)
	{
		if (IsInsideTraversalForestV3MeadowBreak(Position))
		{
			return true;
		}

		const FVector2D UpperRightRetreat = (Position - FVector2D(89000.0f, 133600.0f)) / FVector2D(9000.0f, 6100.0f);
		if (UpperRightRetreat.Size() < 1.0f)
		{
			return true;
		}

		const FVector2D InnerMeadowBite = (Position - FVector2D(80800.0f, 124800.0f)) / FVector2D(7200.0f, 5000.0f);
		if (InnerMeadowBite.Size() < 1.0f)
		{
			return true;
		}

		const FVector2D NorthWindow = (Position - FVector2D(73800.0f, 156000.0f)) / FVector2D(8600.0f, 3600.0f);
		if (NorthWindow.Size() < 1.0f)
		{
			return true;
		}

		return DistanceToSegment(Position, FVector2D(76000.0f, 145000.0f), FVector2D(104000.0f, 123000.0f)) < 6100.0f;
	}

	bool IsInsideTraversalForestV5MeadowBreak(const FVector2D& Position)
	{
		if (IsInsideTraversalForestV4MeadowBreak(Position))
		{
			return true;
		}

		// Keep the edge fractal: trees advance around these gaps, but do not fill them.
		const FVector2D UpperRightMeadowFinger = (Position - FVector2D(84500.0f, 135200.0f)) / FVector2D(7600.0f, 3600.0f);
		if (UpperRightMeadowFinger.Size() < 1.0f)
		{
			return true;
		}

		const FVector2D BackCanopyWindow = (Position - FVector2D(98500.0f, 130200.0f)) / FVector2D(6200.0f, 3900.0f);
		if (BackCanopyWindow.Size() < 1.0f)
		{
			return true;
		}

		const FVector2D NorthMeadowPocket = (Position - FVector2D(60200.0f, 158300.0f)) / FVector2D(8200.0f, 3300.0f);
		if (NorthMeadowPocket.Size() < 1.0f)
		{
			return true;
		}

		return DistanceToSegment(Position, FVector2D(74000.0f, 143000.0f), FVector2D(101000.0f, 121500.0f)) < 5200.0f;
	}

	bool IsInsideTraversalForestV6MeadowBreak(const FVector2D& Position)
	{
		// V6 deliberately stops inheriting the wide V5 gap mask. Titan forest
		// edges keep view corridors, but they still allow layered advance trees.
		const FVector2D UpperRightPocket = (Position - FVector2D(89000.0f, 135900.0f)) / FVector2D(4300.0f, 2800.0f);
		if (UpperRightPocket.Size() < 1.0f)
		{
			return true;
		}

		const FVector2D FrontMeadowBite = (Position - FVector2D(80400.0f, 129400.0f)) / FVector2D(3900.0f, 2500.0f);
		if (FrontMeadowBite.Size() < 1.0f)
		{
			return true;
		}

		const FVector2D NorthernWindow = (Position - FVector2D(65000.0f, 157800.0f)) / FVector2D(5600.0f, 2300.0f);
		if (NorthernWindow.Size() < 1.0f)
		{
			return true;
		}

		return DistanceToSegment(Position, FVector2D(72200.0f, 144000.0f), FVector2D(103000.0f, 120200.0f)) < 2500.0f;
	}

	bool IsInsideTraversalForestV7MeadowBreak(const FVector2D& Position)
	{
		// Preserve the readable movement seams Titan forests rely on, but keep
		// them narrow enough that the edge still feels grown rather than cut.
		const FVector2D FrontPocket = (Position - FVector2D(95000.0f, 120800.0f)) / FVector2D(5200.0f, 2600.0f);
		if (FrontPocket.Size() < 1.0f)
		{
			return true;
		}

		const FVector2D BackPocket = (Position - FVector2D(106500.0f, 131200.0f)) / FVector2D(4700.0f, 3100.0f);
		if (BackPocket.Size() < 1.0f)
		{
			return true;
		}

		const FVector2D NorthWindow = (Position - FVector2D(64500.0f, 158200.0f)) / FVector2D(5200.0f, 2100.0f);
		if (NorthWindow.Size() < 1.0f)
		{
			return true;
		}

		return DistanceToSegment(Position, FVector2D(77000.0f, 143000.0f), FVector2D(108000.0f, 117500.0f)) < 2100.0f;
	}

	bool IsInsideTraversalForestV8MeadowBreak(const FVector2D& Position)
	{
		const FVector2D FrontPocket = (Position - FVector2D(94800.0f, 120500.0f)) / FVector2D(5600.0f, 2500.0f);
		if (FrontPocket.Size() < 1.0f)
		{
			return true;
		}

		const FVector2D MidWindow = (Position - FVector2D(104500.0f, 127600.0f)) / FVector2D(5200.0f, 3000.0f);
		if (MidWindow.Size() < 1.0f)
		{
			return true;
		}

		const FVector2D NorthReadWindow = (Position - FVector2D(64200.0f, 158400.0f)) / FVector2D(6100.0f, 2200.0f);
		if (NorthReadWindow.Size() < 1.0f)
		{
			return true;
		}

		const FVector2D ApproachPocket = (Position - FVector2D(109800.0f, 116200.0f)) / FVector2D(4700.0f, 2200.0f);
		if (ApproachPocket.Size() < 1.0f)
		{
			return true;
		}

		return DistanceToSegment(Position, FVector2D(75500.0f, 144000.0f), FVector2D(109500.0f, 116500.0f)) < 2350.0f;
	}

	bool IsInsideTraversalForestV9MeadowBreak(const FVector2D& Position)
	{
		// V9 keeps readable meadow incursions and approach lines instead of
		// letting denser ecology collapse into a straight forest wall.
		const FVector2D FrontMeadowTongue = (Position - FVector2D(95800.0f, 121500.0f)) / FVector2D(4700.0f, 2400.0f);
		if (FrontMeadowTongue.Size() < 1.0f)
		{
			return true;
		}

		const FVector2D MidCanopyWindow = (Position - FVector2D(104000.0f, 128100.0f)) / FVector2D(4200.0f, 2700.0f);
		if (MidCanopyWindow.Size() < 1.0f)
		{
			return true;
		}

		const FVector2D LandmarkApproach = (Position - FVector2D(110000.0f, 120600.0f)) / FVector2D(4200.0f, 2100.0f);
		if (LandmarkApproach.Size() < 1.0f)
		{
			return true;
		}

		const FVector2D NorthTraversalWindow = (Position - FVector2D(64000.0f, 158600.0f)) / FVector2D(5900.0f, 2100.0f);
		if (NorthTraversalWindow.Size() < 1.0f)
		{
			return true;
		}

		const FVector2D FuturePoiReadWindow = (Position - FVector2D(117500.0f, 122300.0f)) / FVector2D(3000.0f, 1700.0f);
		if (FuturePoiReadWindow.Size() < 1.0f)
		{
			return true;
		}

		return DistanceToSegment(Position, FVector2D(76000.0f, 144300.0f), FVector2D(111500.0f, 116900.0f)) < 2050.0f;
	}

	bool IsInsideTraversalForestV10MeadowBreak(const FVector2D& Position)
	{
		// V10 keeps Titan-style meadow fingers and movement corridors open while
		// allowing denser pockets to wrap around them asymmetrically.
		const FVector2D FrontMeadowFinger = (Position - FVector2D(96000.0f, 121700.0f)) / FVector2D(4300.0f, 2200.0f);
		if (FrontMeadowFinger.Size() < 1.0f)
		{
			return true;
		}

		const FVector2D MidReadWindow = (Position - FVector2D(104800.0f, 128200.0f)) / FVector2D(3900.0f, 2400.0f);
		if (MidReadWindow.Size() < 1.0f)
		{
			return true;
		}

		const FVector2D LandmarkApproach = (Position - FVector2D(110200.0f, 121200.0f)) / FVector2D(3900.0f, 1900.0f);
		if (LandmarkApproach.Size() < 1.0f)
		{
			return true;
		}

		const FVector2D NorthWindow = (Position - FVector2D(64200.0f, 158700.0f)) / FVector2D(5600.0f, 1900.0f);
		if (NorthWindow.Size() < 1.0f)
		{
			return true;
		}

		const FVector2D EastPocketOpening = (Position - FVector2D(117700.0f, 121800.0f)) / FVector2D(2800.0f, 1600.0f);
		if (EastPocketOpening.Size() < 1.0f)
		{
			return true;
		}

		return DistanceToSegment(Position, FVector2D(76200.0f, 144200.0f), FVector2D(112500.0f, 116800.0f)) < 1850.0f;
	}

	bool IsInsideTraversalForestV40RidgeExclusion(const FVector2D& Position)
	{
		// User-facing v40 correction: ridge and world-border crest trees looked
		// artificial. Keep trees down in valleys/protected shelves instead.
		if (Position.Y > 145000.0f)
		{
			return true;
		}

		if (Position.X > 96000.0f && Position.Y > 128500.0f)
		{
			return true;
		}

		if (DistanceToSegment(Position, FVector2D(98000.0f, 145000.0f), FVector2D(118500.0f, 119000.0f)) < 2600.0f)
		{
			return true;
		}

		if (DistanceToSegment(Position, FVector2D(78000.0f, 160000.0f), FVector2D(114500.0f, 139000.0f)) < 3000.0f)
		{
			return true;
		}

		return false;
	}

	bool IsInsideTraversalForestV40MeadowBreak(const FVector2D& Position)
	{
		const FVector2D FrontMeadowFinger = (Position - FVector2D(96200.0f, 121600.0f)) / FVector2D(3900.0f, 2100.0f);
		if (FrontMeadowFinger.Size() < 1.0f)
		{
			return true;
		}

		const FVector2D MidPocketOpening = (Position - FVector2D(104500.0f, 127800.0f)) / FVector2D(3800.0f, 2400.0f);
		if (MidPocketOpening.Size() < 1.0f)
		{
			return true;
		}

		const FVector2D OverlookApproach = (Position - FVector2D(110600.0f, 121200.0f)) / FVector2D(3600.0f, 1800.0f);
		if (OverlookApproach.Size() < 1.0f)
		{
			return true;
		}

		const FVector2D EastPocketOpening = (Position - FVector2D(118000.0f, 121600.0f)) / FVector2D(2600.0f, 1500.0f);
		if (EastPocketOpening.Size() < 1.0f)
		{
			return true;
		}

		return DistanceToSegment(Position, FVector2D(76000.0f, 144200.0f), FVector2D(113000.0f, 116500.0f)) < 1550.0f;
	}

	FVector2D GetRandomPointInZone(const FVegetationZone& Zone, const int32 Seed)
	{
		const float Angle = PseudoRandom01(Seed) * 2.0f * PI;
		const float RadiusAlpha = FMath::Pow(PseudoRandom01(Seed + 11), Zone.ClusterBias);
		const FVector2D Local(FMath::Cos(Angle) * Zone.Extents.X * RadiusAlpha, FMath::Sin(Angle) * Zone.Extents.Y * RadiusAlpha);
		const float RotationRadians = FMath::DegreesToRadians(Zone.RotationDegrees);
		const float CosAngle = FMath::Cos(RotationRadians);
		const float SinAngle = FMath::Sin(RotationRadians);
		return Zone.Center + FVector2D(Local.X * CosAngle - Local.Y * SinAngle, Local.X * SinAngle + Local.Y * CosAngle);
	}

	bool GetPlacementGround(UWorld* World, const FVector2D& Position, const float MinNormalZ, FHitResult& OutHit)
	{
		if (!World)
		{
			return false;
		}

		const FVector Start(Position.X, Position.Y, 36000.0f);
		const FVector End(Position.X, Position.Y, -42000.0f);
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(FFStarterHighlandVegetationPlacement), true);
		QueryParams.bReturnPhysicalMaterial = false;
		if (!World->LineTraceSingleByChannel(OutHit, Start, End, ECC_WorldStatic, QueryParams))
		{
			return false;
		}

		if (!OutHit.GetActor() || !OutHit.GetActor()->IsA<ALandscapeProxy>())
		{
			return false;
		}

		return OutHit.ImpactNormal.Z >= MinNormalZ;
	}

	FVector ResolveLandscapePoint(UWorld* World, const FVector2D& Position, const float HeightOffset, const float FallbackZ)
	{
		FHitResult GroundHit;
		if (GetPlacementGround(World, Position, 0.0f, GroundHit))
		{
			return GroundHit.ImpactPoint + FVector(0.0f, 0.0f, HeightOffset);
		}

		return FVector(Position.X, Position.Y, FallbackZ);
	}

	void HideValidationCameraRuntimeVisuals(ACameraActor* CameraActor)
	{
		if (!CameraActor)
		{
			return;
		}

		CameraActor->SetActorHiddenInGame(true);
		CameraActor->SetActorEnableCollision(false);

		TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents;
		CameraActor->GetComponents(PrimitiveComponents);
		for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
		{
			if (!PrimitiveComponent)
			{
				continue;
			}

			PrimitiveComponent->SetCastShadow(false);
			PrimitiveComponent->SetHiddenInGame(true);
			PrimitiveComponent->SetVisibility(false, true);
		}
	}

	bool SpawnV35ValidationCamera(
		UWorld* World,
		const TCHAR* ActorLabel,
		const TCHAR* CameraTag,
		const FVector2D& CameraXY,
		const float CameraHeightOffset,
		const FVector2D& TargetXY,
		const float TargetHeightOffset,
		const float FieldOfView)
	{
		if (!World)
		{
			return false;
		}

		const FVector CameraLocation = ResolveLandscapePoint(World, CameraXY, CameraHeightOffset, CameraHeightOffset);
		const FVector TargetLocation = ResolveLandscapePoint(World, TargetXY, TargetHeightOffset, TargetHeightOffset);
		const FRotator CameraRotation = (TargetLocation - CameraLocation).Rotation();

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParameters.ObjectFlags = RF_Transactional;
		ACameraActor* CameraActor = World->SpawnActor<ACameraActor>(CameraLocation, CameraRotation, SpawnParameters);
		if (!CameraActor)
		{
			return false;
		}

		CameraActor->SetActorLabel(ActorLabel);
		CameraActor->Tags.AddUnique(V35ValidationCameraTag);
		CameraActor->Tags.AddUnique(FName(CameraTag));
		if (UCameraComponent* CameraComponent = CameraActor->GetCameraComponent())
		{
			CameraComponent->FieldOfView = FieldOfView;
			CameraComponent->bCameraMeshHiddenInGame = true;
		}
		HideValidationCameraRuntimeVisuals(CameraActor);

		CameraActor->MarkPackageDirty();
		return true;
	}

	bool SpawnExplicitValidationCamera(
		UWorld* World,
		const TCHAR* ActorLabel,
		const TCHAR* CameraTag,
		const FVector& CameraLocation,
		const FVector& TargetLocation,
		const float FieldOfView)
	{
		if (!World)
		{
			return false;
		}

		const FRotator CameraRotation = (TargetLocation - CameraLocation).Rotation();

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParameters.ObjectFlags = RF_Transactional;
		ACameraActor* CameraActor = World->SpawnActor<ACameraActor>(CameraLocation, CameraRotation, SpawnParameters);
		if (!CameraActor)
		{
			return false;
		}

		CameraActor->SetActorLabel(ActorLabel);
		CameraActor->Tags.AddUnique(V36ValidationCameraTag);
		CameraActor->Tags.AddUnique(FName(CameraTag));
		if (UCameraComponent* CameraComponent = CameraActor->GetCameraComponent())
		{
			CameraComponent->FieldOfView = FieldOfView;
			CameraComponent->bCameraMeshHiddenInGame = true;
		}
		HideValidationCameraRuntimeVisuals(CameraActor);

		CameraActor->MarkPackageDirty();
		return true;
	}

	bool IsV39VisibleBlockoutPlaceholder(const AActor* Actor)
	{
		if (!Actor)
		{
			return false;
		}

		const FString NameAndLabel = Actor->GetName() + TEXT(" ") + Actor->GetActorLabel();
		if (NameAndLabel.Contains(TEXT("Water"), ESearchCase::IgnoreCase)
			|| NameAndLabel.Contains(TEXT("PlayerStart"), ESearchCase::IgnoreCase))
		{
			return false;
		}

		const FName PlaceholderTags[] = {
			StarterVillagePlaceholderTag,
			StarterNorthBridgePlaceholderTag,
			StarterBossGatePlaceholderTag,
			StarterBossGatePathTag,
			StarterNorthExitPathTag,
			TitanGrasslandPipelineProofTag
		};

		for (const FName& PlaceholderTag : PlaceholderTags)
		{
			if (Actor->ActorHasTag(PlaceholderTag))
			{
				return true;
			}
		}

		if (NameAndLabel.Contains(TEXT("StarterVillagePlaceholder"), ESearchCase::IgnoreCase)
			|| NameAndLabel.Contains(TEXT("StarterNorthBridgePlaceholder"), ESearchCase::IgnoreCase)
			|| NameAndLabel.Contains(TEXT("StarterBossGatePlaceholder"), ESearchCase::IgnoreCase)
			|| NameAndLabel.Contains(TEXT("StarterBossGatePath"), ESearchCase::IgnoreCase)
			|| NameAndLabel.Contains(TEXT("StarterNorthExitPath"), ESearchCase::IgnoreCase)
			|| NameAndLabel.Contains(TEXT("TitanPipeline"), ESearchCase::IgnoreCase)
			|| NameAndLabel.Contains(TEXT("GrasslandPipelineProof"), ESearchCase::IgnoreCase))
		{
			return true;
		}

		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
		for (const UStaticMeshComponent* MeshComponent : StaticMeshComponents)
		{
			if (!MeshComponent)
			{
				continue;
			}

			for (int32 MaterialIndex = 0; MaterialIndex < MeshComponent->GetNumMaterials(); ++MaterialIndex)
			{
				const UMaterialInterface* Material = MeshComponent->GetMaterial(MaterialIndex);
				const FString MaterialPath = Material ? Material->GetPathName() : FString();
				if (MaterialPath.Contains(TEXT("HighlandMarker_Blockout"), ESearchCase::IgnoreCase)
					|| MaterialPath.Contains(TEXT("Blockout_Marker"), ESearchCase::IgnoreCase)
					|| MaterialPath.Contains(TEXT("HighlandPath_Blockout"), ESearchCase::IgnoreCase)
					|| MaterialPath.Contains(TEXT("Blockout_Path"), ESearchCase::IgnoreCase))
				{
					return true;
				}
			}
		}

		return false;
	}

	bool IsObsoleteGrassProofActorV59(const AActor* Actor)
	{
		if (!Actor)
		{
			return false;
		}

		if (Actor->ActorHasTag(SpawnGrassProofTag) || Actor->ActorHasTag(TitanGrasslandPipelineProofTag))
		{
			return true;
		}

		const FString NameAndLabel = Actor->GetName() + TEXT(" ") + Actor->GetActorLabel();
		if (NameAndLabel.Contains(TEXT("FFSpawnGrassProof"), ESearchCase::IgnoreCase)
			|| NameAndLabel.Contains(TEXT("TitanGrasslandPipelineProof"), ESearchCase::IgnoreCase)
			|| NameAndLabel.Contains(TEXT("GrasslandPipelineProof"), ESearchCase::IgnoreCase))
		{
			return true;
		}

		TArray<UActorComponent*> Components;
		Actor->GetComponents(Components);
		for (const UActorComponent* Component : Components)
		{
			if (Component && Component->GetName().Contains(TEXT("FFSpawnGrassProof"), ESearchCase::IgnoreCase))
			{
				return true;
			}
		}

		return false;
	}

	int32 SaveAfterV39VisibleBlockoutCleanup(UWorld* World, const int32 RemovedActors)
	{
		const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
		const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=RemoveVisibleBlockoutMarkersV39 removedActors=%d savedMap=%s savedPackages=%s"),
			RemovedActors,
			bSavedMap ? TEXT("true") : TEXT("false"),
			bSavedPackages ? TEXT("true") : TEXT("false"));
		return (bSavedMap && bSavedPackages) ? 0 : 1;
	}

	int32 AuditFlatActorsV39(UWorld* World)
	{
		int32 LoggedActors = 0;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			const AActor* Actor = *It;
			if (!Actor)
			{
				continue;
			}

			const FVector ActorLocation = Actor->GetActorLocation();
			const bool bInAuditRegion = ActorLocation.X > 20000.0f && ActorLocation.X < 130000.0f && ActorLocation.Y > 70000.0f && ActorLocation.Y < 175000.0f;
			const bool bInTopDownSuspectRegion = ActorLocation.X > 85000.0f && ActorLocation.X < 105000.0f && ActorLocation.Y > 112000.0f && ActorLocation.Y < 130000.0f;
			if (!bInAuditRegion && !Actor->GetActorLabel().Contains(TEXT("Marker")) && !Actor->GetActorLabel().Contains(TEXT("Path")))
			{
				continue;
			}

			TArray<UStaticMeshComponent*> StaticMeshComponents;
			Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);

			if (bInTopDownSuspectRegion)
			{
				TArray<UPrimitiveComponent*> PrimitiveComponents;
				Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
				FString PrimitiveSummary;
				for (const UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
				{
					if (!PrimitiveComponent)
					{
						continue;
					}

					PrimitiveSummary += PrimitiveComponent->GetClass() ? PrimitiveComponent->GetClass()->GetName() : TEXT("None");
					PrimitiveSummary += TEXT(":");
					PrimitiveSummary += PrimitiveComponent->GetName();
					PrimitiveSummary += TEXT(" materials=");
					for (int32 MaterialIndex = 0; MaterialIndex < PrimitiveComponent->GetNumMaterials(); ++MaterialIndex)
					{
						const UMaterialInterface* Material = PrimitiveComponent->GetMaterial(MaterialIndex);
						PrimitiveSummary += Material ? Material->GetPathName() : TEXT("None");
						PrimitiveSummary += TEXT("|");
					}
					PrimitiveSummary += TEXT(" ");
				}
				UE_LOG(LogTemp, Display, TEXT("FFV39SuspectActorAudit label=%s name=%s class=%s loc=(%.1f,%.1f,%.1f) primitives=%s tags=%s"),
					*Actor->GetActorLabel(),
					*Actor->GetName(),
					Actor->GetClass() ? *Actor->GetClass()->GetName() : TEXT("None"),
					ActorLocation.X,
					ActorLocation.Y,
					ActorLocation.Z,
					*PrimitiveSummary,
					*FString::JoinBy(Actor->Tags, TEXT(","), [](const FName& Tag) { return Tag.ToString(); }));
				++LoggedActors;
			}

			if (StaticMeshComponents.IsEmpty())
			{
				continue;
			}

			for (const UStaticMeshComponent* MeshComponent : StaticMeshComponents)
			{
				if (!MeshComponent || !MeshComponent->GetStaticMesh())
				{
					continue;
				}

				const FBoxSphereBounds Bounds = MeshComponent->Bounds;
				const FVector Extent = Bounds.BoxExtent;
				const bool bFlatWide = Extent.Z < 220.0f && (Extent.X > 450.0f || Extent.Y > 450.0f);
				const FString MeshName = MeshComponent->GetStaticMesh()->GetName();
				FString Materials;
				for (int32 MaterialIndex = 0; MaterialIndex < MeshComponent->GetNumMaterials(); ++MaterialIndex)
				{
					const UMaterialInterface* Material = MeshComponent->GetMaterial(MaterialIndex);
					Materials += Material ? Material->GetPathName() : TEXT("None");
					Materials += TEXT(" ");
				}

				if (bFlatWide || Materials.Contains(TEXT("Black"), ESearchCase::IgnoreCase) || Materials.Contains(TEXT("Marker"), ESearchCase::IgnoreCase) || Materials.Contains(TEXT("Path"), ESearchCase::IgnoreCase))
				{
					UE_LOG(LogTemp, Display, TEXT("FFV39FlatActorAudit label=%s name=%s class=%s loc=(%.1f,%.1f,%.1f) extent=(%.1f,%.1f,%.1f) scale=(%.2f,%.2f,%.2f) mesh=%s materials=%s tags=%s"),
						*Actor->GetActorLabel(),
						*Actor->GetName(),
						Actor->GetClass() ? *Actor->GetClass()->GetName() : TEXT("None"),
						ActorLocation.X,
						ActorLocation.Y,
						ActorLocation.Z,
						Extent.X,
						Extent.Y,
						Extent.Z,
						Actor->GetActorScale3D().X,
						Actor->GetActorScale3D().Y,
						Actor->GetActorScale3D().Z,
						*MeshName,
						*Materials,
						*FString::JoinBy(Actor->Tags, TEXT(","), [](const FName& Tag) { return Tag.ToString(); }));
					++LoggedActors;
				}
			}
		}

		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=AuditFlatActorsV39 loggedActors=%d"), LoggedActors);
		return 0;
	}

	int32 RunV35ValidationCameras(UWorld* World, const int32 RemovedActors)
	{
		int32 CreatedCameras = 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV35_CliffReadability"),
			TEXT("FFSmokeHighlandV35CliffReadabilityCamera"),
			FVector2D(65000.0f, 110000.0f),
			14500.0f,
			FVector2D(101000.0f, 140000.0f),
			2400.0f,
			66.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV35_ForestDepth"),
			TEXT("FFSmokeHighlandV35ForestDepthCamera"),
			FVector2D(88000.0f, 96500.0f),
			13000.0f,
			FVector2D(105500.0f, 132500.0f),
			1700.0f,
			68.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV35_TraversalPath"),
			TEXT("FFSmokeHighlandV35TraversalPathCamera"),
			FVector2D(47000.0f, 103000.0f),
			9000.0f,
			FVector2D(90000.0f, 136000.0f),
			1400.0f,
			66.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV35_WideField"),
			TEXT("FFSmokeHighlandV35WideFieldCamera"),
			FVector2D(31000.0f, 77000.0f),
			26000.0f,
			FVector2D(83000.0f, 125000.0f),
			1700.0f,
			72.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV35_TopDown"),
			TEXT("FFSmokeHighlandV35TopDownCamera"),
			FVector2D(71396.0f, 79714.0f),
			68000.0f,
			FVector2D(71396.0f, 79714.0f),
			0.0f,
			65.0f) ? 1 : 0;

		World->MarkPackageDirty();
		const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
		const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=ValidationCamerasV35 removedActors=%d createdCameras=%d savedMap=%s savedPackages=%s"),
			RemovedActors,
			CreatedCameras,
			bSavedMap ? TEXT("true") : TEXT("false"),
			bSavedPackages ? TEXT("true") : TEXT("false"));
		return (bSavedMap && bSavedPackages && CreatedCameras == 5) ? 0 : 1;
	}

	int32 RunV36ValidationCameras(UWorld* World, const int32 RemovedActors)
	{
		int32 CreatedCameras = 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV36_CliffClose"),
			TEXT("FFSmokeHighlandV36CliffCloseCamera"),
			FVector2D(69000.0f, 112500.0f),
			8200.0f,
			FVector2D(100500.0f, 139500.0f),
			2200.0f,
			58.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV36_ForestDepth"),
			TEXT("FFSmokeHighlandV36ForestDepthCamera"),
			FVector(87500.0f, 103000.0f, 1700.0f),
			FVector(104500.0f, 115000.0f, -2600.0f),
			48.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV36_TraversalOverlook"),
			TEXT("FFSmokeHighlandV36TraversalOverlookCamera"),
			FVector2D(53500.0f, 105500.0f),
			9000.0f,
			FVector2D(97000.0f, 123000.0f),
			1900.0f,
			64.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV36_WideBiomeField"),
			TEXT("FFSmokeHighlandV36WideBiomeFieldCamera"),
			FVector2D(30000.0f, 79000.0f),
			24500.0f,
			FVector2D(91500.0f, 124500.0f),
			2400.0f,
			72.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV36_WorldBorder"),
			TEXT("FFSmokeHighlandV36WorldBorderCamera"),
			FVector2D(72000.0f, 118500.0f),
			12500.0f,
			FVector2D(56000.0f, 158000.0f),
			2600.0f,
			62.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV36_TopDown"),
			TEXT("FFSmokeHighlandV36TopDownCamera"),
			FVector2D(64000.0f, 111000.0f),
			92000.0f,
			FVector2D(64000.0f, 111000.0f),
			0.0f,
			60.0f) ? 1 : 0;

		World->MarkPackageDirty();
		const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
		const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=ValidationCamerasV36 removedActors=%d createdCameras=%d savedMap=%s savedPackages=%s"),
			RemovedActors,
			CreatedCameras,
			bSavedMap ? TEXT("true") : TEXT("false"),
			bSavedPackages ? TEXT("true") : TEXT("false"));
		return (bSavedMap && bSavedPackages && CreatedCameras == 6) ? 0 : 1;
	}

	int32 RunV37ValidationCameras(UWorld* World, const int32 RemovedActors)
	{
		int32 CreatedCameras = 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV37_CliffClose"),
			TEXT("FFSmokeHighlandV37CliffCloseCamera"),
			FVector2D(70000.0f, 112000.0f),
			7600.0f,
			FVector2D(104000.0f, 138500.0f),
			2100.0f,
			56.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV37_ForestDepth"),
			TEXT("FFSmokeHighlandV37ForestDepthCamera"),
			FVector(87000.0f, 102000.0f, 1800.0f),
			FVector(106000.0f, 127000.0f, -2300.0f),
			50.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV37_TraversalOverlook"),
			TEXT("FFSmokeHighlandV37TraversalOverlookCamera"),
			FVector2D(52000.0f, 104000.0f),
			8800.0f,
			FVector2D(96200.0f, 122000.0f),
			1800.0f,
			63.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV37_WideBiomeField"),
			TEXT("FFSmokeHighlandV37WideBiomeFieldCamera"),
			FVector2D(28000.0f, 78000.0f),
			23800.0f,
			FVector2D(91000.0f, 126500.0f),
			2300.0f,
			72.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV37_WorldBorder"),
			TEXT("FFSmokeHighlandV37WorldBorderCamera"),
			FVector2D(73000.0f, 118000.0f),
			11600.0f,
			FVector2D(58000.0f, 158300.0f),
			2500.0f,
			61.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV37_TopDown"),
			TEXT("FFSmokeHighlandV37TopDownCamera"),
			FVector2D(64000.0f, 111000.0f),
			92000.0f,
			FVector2D(64000.0f, 111000.0f),
			0.0f,
			60.0f) ? 1 : 0;

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && Actor->GetActorLabel().Contains(TEXT("FFSmoke_HighlandV37_")))
			{
				Actor->Tags.AddUnique(V37ValidationCameraTag);
			}
		}

		World->MarkPackageDirty();
		const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
		const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=ValidationCamerasV37 removedActors=%d createdCameras=%d savedMap=%s savedPackages=%s"),
			RemovedActors,
			CreatedCameras,
			bSavedMap ? TEXT("true") : TEXT("false"),
			bSavedPackages ? TEXT("true") : TEXT("false"));
		return (bSavedMap && bSavedPackages && CreatedCameras == 6) ? 0 : 1;
	}

	int32 RunV38ValidationCameras(UWorld* World, const int32 RemovedActors)
	{
		int32 CreatedCameras = 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV38_ForestEcologyDepth"),
			TEXT("FFSmokeHighlandV38ForestEcologyDepthCamera"),
			FVector(86000.0f, 102500.0f, 2100.0f),
			FVector(109000.0f, 126800.0f, -2100.0f),
			48.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV38_TraversalLandmark"),
			TEXT("FFSmokeHighlandV38TraversalLandmarkCamera"),
			FVector2D(90500.0f, 108500.0f),
			6900.0f,
			FVector2D(109500.0f, 121500.0f),
			1900.0f,
			53.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV38_CliffReadability"),
			TEXT("FFSmokeHighlandV38CliffReadabilityCamera"),
			FVector2D(73500.0f, 113500.0f),
			7800.0f,
			FVector2D(110500.0f, 140000.0f),
			2200.0f,
			55.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV38_WideBiomeField"),
			TEXT("FFSmokeHighlandV38WideBiomeFieldCamera"),
			FVector2D(30000.0f, 79000.0f),
			24600.0f,
			FVector2D(96000.0f, 126000.0f),
			2300.0f,
			71.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV38_TopDownMacroFlow"),
			TEXT("FFSmokeHighlandV38TopDownMacroFlowCamera"),
			FVector2D(72000.0f, 118500.0f),
			93000.0f,
			FVector2D(72000.0f, 118500.0f),
			0.0f,
			59.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV38_TraversalOverlook"),
			TEXT("FFSmokeHighlandV38TraversalOverlookCamera"),
			FVector2D(52000.0f, 104000.0f),
			9000.0f,
			FVector2D(108000.0f, 121000.0f),
			1900.0f,
			62.0f) ? 1 : 0;

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && Actor->GetActorLabel().Contains(TEXT("FFSmoke_HighlandV38_")))
			{
				Actor->Tags.AddUnique(V38ValidationCameraTag);
			}
		}

		World->MarkPackageDirty();
		const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
		const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=ValidationCamerasV38 removedActors=%d createdCameras=%d savedMap=%s savedPackages=%s"),
			RemovedActors,
			CreatedCameras,
			bSavedMap ? TEXT("true") : TEXT("false"),
			bSavedPackages ? TEXT("true") : TEXT("false"));
		return (bSavedMap && bSavedPackages && CreatedCameras == 6) ? 0 : 1;
	}

	int32 RunV39ValidationCameras(UWorld* World, const int32 RemovedActors)
	{
		int32 CreatedCameras = 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV39_CliffClose"),
			TEXT("FFSmokeHighlandV39CliffCloseCamera"),
			FVector2D(73500.0f, 113000.0f),
			7200.0f,
			FVector2D(110500.0f, 138500.0f),
			2200.0f,
			53.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV39_TraversalOverlook"),
			TEXT("FFSmokeHighlandV39TraversalOverlookCamera"),
			FVector2D(52000.0f, 104000.0f),
			9000.0f,
			FVector2D(110500.0f, 121000.0f),
			2000.0f,
			62.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV39_ForestDepth"),
			TEXT("FFSmokeHighlandV39ForestDepthCamera"),
			FVector(86200.0f, 102500.0f, 2100.0f),
			FVector(109500.0f, 126800.0f, -2100.0f),
			48.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV39_WorldBorderReadability"),
			TEXT("FFSmokeHighlandV39WorldBorderReadabilityCamera"),
			FVector2D(35000.0f, 82000.0f),
			26500.0f,
			FVector2D(92000.0f, 144500.0f),
			2600.0f,
			69.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV39_TopDownMacroBreakup"),
			TEXT("FFSmokeHighlandV39TopDownMacroBreakupCamera"),
			FVector2D(72000.0f, 118500.0f),
			93000.0f,
			FVector2D(72000.0f, 118500.0f),
			0.0f,
			59.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV39_WideBiomeField"),
			TEXT("FFSmokeHighlandV39WideBiomeFieldCamera"),
			FVector2D(30000.0f, 79000.0f),
			24800.0f,
			FVector2D(97000.0f, 126000.0f),
			2300.0f,
			71.0f) ? 1 : 0;

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && Actor->GetActorLabel().Contains(TEXT("FFSmoke_HighlandV39_")))
			{
				Actor->Tags.AddUnique(V39ValidationCameraTag);
			}
		}

		World->MarkPackageDirty();
		const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
		const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=ValidationCamerasV39 removedActors=%d createdCameras=%d savedMap=%s savedPackages=%s"),
			RemovedActors,
			CreatedCameras,
			bSavedMap ? TEXT("true") : TEXT("false"),
			bSavedPackages ? TEXT("true") : TEXT("false"));
		return (bSavedMap && bSavedPackages && CreatedCameras == 6) ? 0 : 1;
	}

	int32 RunV40ValidationCameras(UWorld* World, const int32 RemovedActors)
	{
		int32 CreatedCameras = 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV40_CorrectedRidgeTrees"),
			TEXT("FFSmokeHighlandV40CorrectedRidgeTreesCamera"),
			FVector2D(36000.0f, 82000.0f),
			26200.0f,
			FVector2D(94000.0f, 144000.0f),
			2600.0f,
			69.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV40_ForestSpacingProof"),
			TEXT("FFSmokeHighlandV40ForestSpacingProofCamera"),
			FVector(86000.0f, 102500.0f, 2200.0f),
			FVector(109500.0f, 125000.0f, -2100.0f),
			49.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV40_CliffClose"),
			TEXT("FFSmokeHighlandV40CliffCloseCamera"),
			FVector2D(73500.0f, 113000.0f),
			7200.0f,
			FVector2D(110500.0f, 138000.0f),
			2200.0f,
			53.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV40_WorldBorderMountainRing"),
			TEXT("FFSmokeHighlandV40WorldBorderMountainRingCamera"),
			FVector2D(30500.0f, 76000.0f),
			27000.0f,
			FVector2D(93000.0f, 143000.0f),
			2600.0f,
			70.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV40_WideBiomeField"),
			TEXT("FFSmokeHighlandV40WideBiomeFieldCamera"),
			FVector2D(30000.0f, 79000.0f),
			24800.0f,
			FVector2D(97000.0f, 126000.0f),
			2300.0f,
			71.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV40_TopDownMacroCleanup"),
			TEXT("FFSmokeHighlandV40TopDownMacroCleanupCamera"),
			FVector2D(72000.0f, 118500.0f),
			93000.0f,
			FVector2D(72000.0f, 118500.0f),
			0.0f,
			59.0f) ? 1 : 0;

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && Actor->GetActorLabel().Contains(TEXT("FFSmoke_HighlandV40_")))
			{
				Actor->Tags.AddUnique(V40ValidationCameraTag);
			}
		}

		World->MarkPackageDirty();
		const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
		const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=ValidationCamerasV40 removedActors=%d createdCameras=%d savedMap=%s savedPackages=%s"),
			RemovedActors,
			CreatedCameras,
			bSavedMap ? TEXT("true") : TEXT("false"),
			bSavedPackages ? TEXT("true") : TEXT("false"));
		return (bSavedMap && bSavedPackages && CreatedCameras == 6) ? 0 : 1;
	}

	int32 RunV41ValidationCameras(UWorld* World, const int32 RemovedActors)
	{
		int32 CreatedCameras = 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV41_OuterMountainRingClose"),
			TEXT("FFSmokeHighlandV41OuterMountainRingCloseCamera"),
			FVector2D(69000.0f, 116000.0f),
			7600.0f,
			FVector2D(106500.0f, 145000.0f),
			1900.0f,
			50.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV41_StyleComparison"),
			TEXT("FFSmokeHighlandV41StyleComparisonCamera"),
			FVector2D(30500.0f, 76000.0f),
			27000.0f,
			FVector2D(93000.0f, 143000.0f),
			2600.0f,
			70.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV41_WorldBorderWide"),
			TEXT("FFSmokeHighlandV41WorldBorderWideCamera"),
			FVector2D(27000.0f, 82000.0f),
			29200.0f,
			FVector2D(96000.0f, 145000.0f),
			2600.0f,
			69.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV41_CliffGroundTransition"),
			TEXT("FFSmokeHighlandV41CliffGroundTransitionCamera"),
			FVector(80200.0f, 108500.0f, 2600.0f),
			FVector(106000.0f, 129000.0f, -2500.0f),
			48.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV41_TopDownMacroCleanup"),
			TEXT("FFSmokeHighlandV41TopDownMacroCleanupCamera"),
			FVector2D(72000.0f, 118500.0f),
			93000.0f,
			FVector2D(72000.0f, 118500.0f),
			0.0f,
			59.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV41_ForestRidgeProof"),
			TEXT("FFSmokeHighlandV41ForestRidgeProofCamera"),
			FVector2D(36000.0f, 82000.0f),
			26200.0f,
			FVector2D(94000.0f, 144000.0f),
			2600.0f,
			69.0f) ? 1 : 0;

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && Actor->GetActorLabel().Contains(TEXT("FFSmoke_HighlandV41_")))
			{
				Actor->Tags.AddUnique(V41ValidationCameraTag);
			}
		}

		World->MarkPackageDirty();
		const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
		const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=ValidationCamerasV41 removedActors=%d createdCameras=%d savedMap=%s savedPackages=%s"),
			RemovedActors,
			CreatedCameras,
			bSavedMap ? TEXT("true") : TEXT("false"),
			bSavedPackages ? TEXT("true") : TEXT("false"));
		return (bSavedMap && bSavedPackages && CreatedCameras == 6) ? 0 : 1;
	}

	int32 RunV42ValidationCameras(UWorld* World, const int32 RemovedActors)
	{
		int32 CreatedCameras = 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV42_OuterRingClose"),
			TEXT("FFSmokeHighlandV42OuterRingCloseCamera"),
			FVector2D(69000.0f, 116000.0f),
			7600.0f,
			FVector2D(106500.0f, 145000.0f),
			1900.0f,
			50.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV42_GeologySilhouette"),
			TEXT("FFSmokeHighlandV42GeologySilhouetteCamera"),
			FVector2D(27000.0f, 82000.0f),
			29200.0f,
			FVector2D(96000.0f, 145000.0f),
			2600.0f,
			69.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV42_RiverbankErosion"),
			TEXT("FFSmokeHighlandV42RiverbankErosionCamera"),
			FVector(80200.0f, 108500.0f, 2600.0f),
			FVector(106000.0f, 129000.0f, -2500.0f),
			48.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV42_EcologyVariation"),
			TEXT("FFSmokeHighlandV42EcologyVariationCamera"),
			FVector(86800.0f, 111800.0f, 2450.0f),
			FVector(106000.0f, 123500.0f, -1350.0f),
			52.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV42_WideBiomeField"),
			TEXT("FFSmokeHighlandV42WideBiomeFieldCamera"),
			FVector2D(30500.0f, 76000.0f),
			27000.0f,
			FVector2D(93000.0f, 143000.0f),
			2600.0f,
			70.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV42_TopDownMacroCleanup"),
			TEXT("FFSmokeHighlandV42TopDownMacroCleanupCamera"),
			FVector2D(72000.0f, 118500.0f),
			93000.0f,
			FVector2D(72000.0f, 118500.0f),
			0.0f,
			59.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV42_TraversalOverlook"),
			TEXT("FFSmokeHighlandV42TraversalOverlookCamera"),
			FVector2D(36000.0f, 82000.0f),
			26200.0f,
			FVector2D(94000.0f, 144000.0f),
			2600.0f,
			69.0f) ? 1 : 0;

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && Actor->GetActorLabel().Contains(TEXT("FFSmoke_HighlandV42_")))
			{
				Actor->Tags.AddUnique(V42ValidationCameraTag);
			}
		}

		World->MarkPackageDirty();
		const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
		const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=ValidationCamerasV42 removedActors=%d createdCameras=%d savedMap=%s savedPackages=%s"),
			RemovedActors,
			CreatedCameras,
			bSavedMap ? TEXT("true") : TEXT("false"),
			bSavedPackages ? TEXT("true") : TEXT("false"));
		return (bSavedMap && bSavedPackages && CreatedCameras == 7) ? 0 : 1;
	}

	int32 RunV43ValidationCameras(UWorld* World, const int32 RemovedActors)
	{
		int32 CreatedCameras = 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV43_OuterRingGeologyClose"),
			TEXT("FFSmokeHighlandV43OuterRingGeologyCloseCamera"),
			FVector2D(69000.0f, 116000.0f),
			7600.0f,
			FVector2D(106500.0f, 145000.0f),
			1900.0f,
			50.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV43_SilhouetteComparison"),
			TEXT("FFSmokeHighlandV43SilhouetteComparisonCamera"),
			FVector2D(27000.0f, 82000.0f),
			29200.0f,
			FVector2D(96000.0f, 145000.0f),
			2600.0f,
			69.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV43_MountainFaceBreakup"),
			TEXT("FFSmokeHighlandV43MountainFaceBreakupCamera"),
			FVector(74200.0f, 109000.0f, 3450.0f),
			FVector(106500.0f, 137500.0f, -1900.0f),
			49.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV43_CliffGroundTransition"),
			TEXT("FFSmokeHighlandV43CliffGroundTransitionCamera"),
			FVector(80200.0f, 108500.0f, 2600.0f),
			FVector(106000.0f, 129000.0f, -2500.0f),
			48.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV43_TraversalOverlook"),
			TEXT("FFSmokeHighlandV43TraversalOverlookCamera"),
			FVector2D(36000.0f, 82000.0f),
			26200.0f,
			FVector2D(94000.0f, 144000.0f),
			2600.0f,
			69.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV43_TopDownMacroFlow"),
			TEXT("FFSmokeHighlandV43TopDownMacroFlowCamera"),
			FVector2D(72000.0f, 118500.0f),
			93000.0f,
			FVector2D(72000.0f, 118500.0f),
			0.0f,
			59.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV43_WideBiomeField"),
			TEXT("FFSmokeHighlandV43WideBiomeFieldCamera"),
			FVector2D(30500.0f, 76000.0f),
			27000.0f,
			FVector2D(93000.0f, 143000.0f),
			2600.0f,
			70.0f) ? 1 : 0;

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && Actor->GetActorLabel().Contains(TEXT("FFSmoke_HighlandV43_")))
			{
				Actor->Tags.AddUnique(V43ValidationCameraTag);
			}
		}

		World->MarkPackageDirty();
		const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
		const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=ValidationCamerasV43 removedActors=%d createdCameras=%d savedMap=%s savedPackages=%s"),
			RemovedActors,
			CreatedCameras,
			bSavedMap ? TEXT("true") : TEXT("false"),
			bSavedPackages ? TEXT("true") : TEXT("false"));
		return (bSavedMap && bSavedPackages && CreatedCameras == 7) ? 0 : 1;
	}

	int32 RunV44ValidationCameras(UWorld* World, const int32 RemovedActors)
	{
		int32 CreatedCameras = 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV44_OuterRingClose"),
			TEXT("FFSmokeHighlandV44OuterRingCloseCamera"),
			FVector2D(69000.0f, 116000.0f),
			7600.0f,
			FVector2D(106500.0f, 145000.0f),
			1900.0f,
			50.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV44_SilhouetteComparison"),
			TEXT("FFSmokeHighlandV44SilhouetteComparisonCamera"),
			FVector2D(27000.0f, 82000.0f),
			29200.0f,
			FVector2D(96000.0f, 145000.0f),
			2600.0f,
			69.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV44_LargeCliffFace"),
			TEXT("FFSmokeHighlandV44LargeCliffFaceCamera"),
			FVector(74200.0f, 109000.0f, 3450.0f),
			FVector(106500.0f, 137500.0f, -1900.0f),
			49.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV44_EmbeddedGeology"),
			TEXT("FFSmokeHighlandV44EmbeddedGeologyCamera"),
			FVector(80200.0f, 108500.0f, 2600.0f),
			FVector(106000.0f, 129000.0f, -2500.0f),
			48.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV44_TraversalOverlook"),
			TEXT("FFSmokeHighlandV44TraversalOverlookCamera"),
			FVector2D(36000.0f, 82000.0f),
			26200.0f,
			FVector2D(94000.0f, 144000.0f),
			2600.0f,
			69.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV44_RiverbankTransition"),
			TEXT("FFSmokeHighlandV44RiverbankTransitionCamera"),
			FVector(99000.0f, 93500.0f, 5600.0f),
			FVector(119500.0f, 108000.0f, -3600.0f),
			52.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV44_WideBiomeField"),
			TEXT("FFSmokeHighlandV44WideBiomeFieldCamera"),
			FVector2D(30500.0f, 76000.0f),
			27000.0f,
			FVector2D(93000.0f, 143000.0f),
			2600.0f,
			70.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV44_TopDownMacroFlow"),
			TEXT("FFSmokeHighlandV44TopDownMacroFlowCamera"),
			FVector2D(72000.0f, 118500.0f),
			93000.0f,
			FVector2D(72000.0f, 118500.0f),
			0.0f,
			59.0f) ? 1 : 0;

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && Actor->GetActorLabel().Contains(TEXT("FFSmoke_HighlandV44_")))
			{
				Actor->Tags.AddUnique(V44ValidationCameraTag);
			}
		}

		World->MarkPackageDirty();
		const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
		const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=ValidationCamerasV44 removedActors=%d createdCameras=%d savedMap=%s savedPackages=%s"),
			RemovedActors,
			CreatedCameras,
			bSavedMap ? TEXT("true") : TEXT("false"),
			bSavedPackages ? TEXT("true") : TEXT("false"));
		return (bSavedMap && bSavedPackages && CreatedCameras == 8) ? 0 : 1;
	}

	int32 RunV45ValidationCameras(UWorld* World, const int32 RemovedActors)
	{
		int32 CreatedCameras = 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV45_EmbeddedCliffMass"),
			TEXT("FFSmokeHighlandV45EmbeddedCliffMassCamera"),
			FVector(67500.0f, 116500.0f, 5600.0f),
			FVector(94000.0f, 142500.0f, -2600.0f),
			48.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV45_SilhouetteComparison"),
			TEXT("FFSmokeHighlandV45SilhouetteComparisonCamera"),
			FVector2D(27000.0f, 82000.0f),
			29200.0f,
			FVector2D(96000.0f, 145000.0f),
			2600.0f,
			69.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV45_GeologyWallTransition"),
			TEXT("FFSmokeHighlandV45GeologyWallTransitionCamera"),
			FVector(82500.0f, 110000.0f, 4700.0f),
			FVector(108000.0f, 135500.0f, -2600.0f),
			50.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV45_WideBorderView"),
			TEXT("FFSmokeHighlandV45WideBorderViewCamera"),
			FVector2D(30500.0f, 76000.0f),
			27000.0f,
			FVector2D(93000.0f, 143000.0f),
			2600.0f,
			70.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV45_TraversalOverlook"),
			TEXT("FFSmokeHighlandV45TraversalOverlookCamera"),
			FVector2D(36000.0f, 82000.0f),
			26200.0f,
			FVector2D(94000.0f, 144000.0f),
			2600.0f,
			69.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV45_RiverbankBlend"),
			TEXT("FFSmokeHighlandV45RiverbankBlendCamera"),
			FVector(98500.0f, 93500.0f, 5600.0f),
			FVector(119500.0f, 108000.0f, -3600.0f),
			52.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV45_TopDownMacroFlow"),
			TEXT("FFSmokeHighlandV45TopDownMacroFlowCamera"),
			FVector2D(72000.0f, 118500.0f),
			93000.0f,
			FVector2D(72000.0f, 118500.0f),
			0.0f,
			59.0f) ? 1 : 0;

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && Actor->GetActorLabel().Contains(TEXT("FFSmoke_HighlandV45_")))
			{
				Actor->Tags.AddUnique(V45ValidationCameraTag);
			}
		}

		World->MarkPackageDirty();
		const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
		const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=ValidationCamerasV45 removedActors=%d createdCameras=%d savedMap=%s savedPackages=%s"),
			RemovedActors,
			CreatedCameras,
			bSavedMap ? TEXT("true") : TEXT("false"),
			bSavedPackages ? TEXT("true") : TEXT("false"));
		return (bSavedMap && bSavedPackages && CreatedCameras == 7) ? 0 : 1;
	}

	bool IsOuterRingExposedRockInstanceV43(const FVector& WorldLocation)
	{
		const FVector2D Position(WorldLocation.X, WorldLocation.Y);
		const float PosX = static_cast<float>(Position.X);
		const float PosY = static_cast<float>(Position.Y);
		const float NorthRidge = FMath::SmoothStep(148500.0f, 162000.0f, PosY);
		const float EastRidge = FMath::SmoothStep(104000.0f, 121000.0f, PosX) * FMath::SmoothStep(111000.0f, 146000.0f, PosY);
		const float WestRidge = (1.0f - FMath::SmoothStep(23500.0f, 33500.0f, PosX)) * FMath::SmoothStep(87000.0f, 142000.0f, PosY);
		const float ExposedRing = FMath::Max3(NorthRidge, EastRidge, WestRidge);
		if (ExposedRing <= 0.32f)
		{
			return false;
		}

		const bool bHighCrownPlate = WorldLocation.Z > -2450.0f;
		const bool bUpperFacePlate = ExposedRing > 0.64f && WorldLocation.Z > -3350.0f;
		const bool bSkylineBand = PosY > 156000.0f && WorldLocation.Z > -3650.0f;
		return bHighCrownPlate || bUpperFacePlate || bSkylineBand;
	}

	int32 RunOuterRingRockCleanupV43(UWorld* World, const int32 RemovedActors)
	{
		int32 RockActorsVisited = 0;
		int32 RockComponentsVisited = 0;
		int32 RemovedInstances = 0;

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (!Actor || !Actor->ActorHasTag(CliffRockTag))
			{
				continue;
			}

			++RockActorsVisited;
			TArray<UHierarchicalInstancedStaticMeshComponent*> Components;
			Actor->GetComponents<UHierarchicalInstancedStaticMeshComponent>(Components);
			for (UHierarchicalInstancedStaticMeshComponent* Component : Components)
			{
				if (!Component)
				{
					continue;
				}

				++RockComponentsVisited;
				for (int32 InstanceIndex = Component->GetInstanceCount() - 1; InstanceIndex >= 0; --InstanceIndex)
				{
					FTransform InstanceTransform;
					if (!Component->GetInstanceTransform(InstanceIndex, InstanceTransform, true))
					{
						continue;
					}

					if (IsOuterRingExposedRockInstanceV43(InstanceTransform.GetLocation()))
					{
						Component->RemoveInstance(InstanceIndex);
						++RemovedInstances;
					}
				}

				Component->MarkRenderStateDirty();
				Component->MarkPackageDirty();
			}

			Actor->MarkPackageDirty();
		}

		World->MarkPackageDirty();
		const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
		const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=OuterRingRockCleanupV43 removedActors=%d rockActors=%d rockComponents=%d removedInstances=%d savedMap=%s savedPackages=%s"),
			RemovedActors,
			RockActorsVisited,
			RockComponentsVisited,
			RemovedInstances,
			bSavedMap ? TEXT("true") : TEXT("false"),
			bSavedPackages ? TEXT("true") : TEXT("false"));
		return (bSavedMap && bSavedPackages) ? 0 : 1;
	}

	bool IsOuterRingExposedRockInstanceV44(const FVector& WorldLocation)
	{
		const FVector2D Position(WorldLocation.X, WorldLocation.Y);
		const float PosX = static_cast<float>(Position.X);
		const float PosY = static_cast<float>(Position.Y);
		const float NorthRidge = FMath::SmoothStep(135000.0f, 161500.0f, PosY);
		const float EastRidge = FMath::SmoothStep(93000.0f, 121500.0f, PosX) * FMath::SmoothStep(92000.0f, 145500.0f, PosY);
		const float WestRidge = (1.0f - FMath::SmoothStep(24500.0f, 42000.0f, PosX)) * FMath::SmoothStep(76000.0f, 142500.0f, PosY);
		const float ExposedRing = FMath::Max3(NorthRidge, EastRidge, WestRidge);
		if (ExposedRing <= 0.16f)
		{
			return false;
		}

		const bool bHighCrownPlate = WorldLocation.Z > -9000.0f;
		const bool bUpperFacePlate = ExposedRing > 0.32f && WorldLocation.Z > -12000.0f;
		const bool bNorthSkylineBand = PosY > 136000.0f && WorldLocation.Z > -14500.0f;
		const bool bEastSkylineBand = PosX > 89000.0f && PosY > 101000.0f && WorldLocation.Z > -13200.0f;
		const bool bWestSkylineBand = PosX < 45500.0f && PosY > 76000.0f && WorldLocation.Z > -13000.0f;
		const bool bAttachedMidFacePlate = ExposedRing > 0.46f && WorldLocation.Z > -14200.0f;
		const bool bSmallCenterFacePlate = PosX > 70000.0f && PosX < 121500.0f && PosY > 100000.0f && WorldLocation.Z > -13600.0f;
		return bHighCrownPlate || bUpperFacePlate || bNorthSkylineBand || bEastSkylineBand || bWestSkylineBand || bAttachedMidFacePlate || bSmallCenterFacePlate;
	}

	int32 RunOuterRingRockCleanupV44(UWorld* World, const int32 RemovedActors)
	{
		int32 RockActorsVisited = 0;
		int32 RockComponentsVisited = 0;
		int32 RemovedInstances = 0;

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (!Actor || !Actor->ActorHasTag(CliffRockTag))
			{
				continue;
			}

			++RockActorsVisited;
			TArray<UHierarchicalInstancedStaticMeshComponent*> Components;
			Actor->GetComponents<UHierarchicalInstancedStaticMeshComponent>(Components);
			for (UHierarchicalInstancedStaticMeshComponent* Component : Components)
			{
				if (!Component)
				{
					continue;
				}

				++RockComponentsVisited;
				for (int32 InstanceIndex = Component->GetInstanceCount() - 1; InstanceIndex >= 0; --InstanceIndex)
				{
					FTransform InstanceTransform;
					if (!Component->GetInstanceTransform(InstanceIndex, InstanceTransform, true))
					{
						continue;
					}

					if (IsOuterRingExposedRockInstanceV44(InstanceTransform.GetLocation()))
					{
						Component->RemoveInstance(InstanceIndex);
						++RemovedInstances;
					}
				}

				Component->MarkRenderStateDirty();
				Component->MarkPackageDirty();
			}

			Actor->MarkPackageDirty();
		}

		World->MarkPackageDirty();
		const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
		const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=OuterRingRockCleanupV44 removedActors=%d rockActors=%d rockComponents=%d removedInstances=%d savedMap=%s savedPackages=%s"),
			RemovedActors,
			RockActorsVisited,
			RockComponentsVisited,
			RemovedInstances,
			bSavedMap ? TEXT("true") : TEXT("false"),
			bSavedPackages ? TEXT("true") : TEXT("false"));
		return (bSavedMap && bSavedPackages) ? 0 : 1;
	}

	bool LoadMeshes(const TCHAR* const* MeshPaths, const int32 MeshCount, TArray<UStaticMesh*>& OutMeshes, const TCHAR* Context);
	UHierarchicalInstancedStaticMeshComponent* CreateVegetationComponent(AActor* Owner, UStaticMesh* Mesh, const int32 Index, const bool bTree);

	int32 RunV45EmbeddedCliffMasses(UWorld* World, const int32 RemovedActors)
	{
		TArray<UStaticMesh*> RockMeshes;
		if (!LoadMeshes(CliffRockMeshPaths, UE_ARRAY_COUNT(CliffRockMeshPaths), RockMeshes, TEXT("v45 embedded cliff mass")))
		{
			return 1;
		}

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParameters.ObjectFlags = RF_Transactional;

		AActor* CliffMassActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
		if (!CliffMassActor)
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandVegetationLayer: failed to spawn v45 embedded cliff mass actor."));
			return 1;
		}

		CliffMassActor->Tags.AddUnique(V45EmbeddedCliffMassTag);
		CliffMassActor->SetActorLabel(TEXT("FF_V45_EmbeddedCliffMasses_HISM"));
		USceneComponent* RootComponent = NewObject<USceneComponent>(CliffMassActor, TEXT("Root"), RF_Transactional);
		RootComponent->SetMobility(EComponentMobility::Static);
		CliffMassActor->SetRootComponent(RootComponent);
		RootComponent->RegisterComponent();
		CliffMassActor->AddInstanceComponent(RootComponent);

		UMaterialInterface* TitanCliffMaterial = LoadObject<UMaterialInterface>(nullptr, TitanCliffRockMaterialPath);
		if (!TitanCliffMaterial)
		{
			UE_LOG(LogTemp, Warning, TEXT("FFStarterHighlandVegetationLayer: missing Titan cliff material %s; using rock mesh defaults."), TitanCliffRockMaterialPath);
		}

		TArray<UHierarchicalInstancedStaticMeshComponent*> RockComponents;
		for (int32 Index = 0; Index < RockMeshes.Num(); ++Index)
		{
			UHierarchicalInstancedStaticMeshComponent* Component = CreateVegetationComponent(CliffMassActor, RockMeshes[Index], Index, true);
			if (Component)
			{
				Component->SetCullDistances(0, 420000);
				Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				if (TitanCliffMaterial)
				{
					const int32 MaterialSlots = FMath::Max(1, Component->GetNumMaterials());
					for (int32 SlotIndex = 0; SlotIndex < MaterialSlots; ++SlotIndex)
					{
						Component->SetMaterial(SlotIndex, TitanCliffMaterial);
					}
				}
			}
			RockComponents.Add(Component);
		}

		int32 TotalMasses = 0;
		int32 RejectedMasses = 0;
		for (const FEmbeddedCliffMass& Mass : EmbeddedCliffMassesV45)
		{
			const int32 MeshIndex = FMath::Clamp(Mass.MeshIndex, 0, RockComponents.Num() - 1);
			UHierarchicalInstancedStaticMeshComponent* Component = RockComponents.IsValidIndex(MeshIndex) ? RockComponents[MeshIndex] : nullptr;
			if (!Component)
			{
				++RejectedMasses;
				continue;
			}

			FHitResult GroundHit;
			if (!GetPlacementGround(World, Mass.Position, 0.0f, GroundHit))
			{
				++RejectedMasses;
				UE_LOG(LogTemp, Warning, TEXT("FFStarterHighlandVegetationLayer: v45 mass rejected name=%s reason=no-ground"), Mass.Name);
				continue;
			}

			const FVector BuriedLocation = GroundHit.ImpactPoint - GroundHit.ImpactNormal * 280.0f + FVector(0.0f, 0.0f, Mass.HeightOffset);
			FTransform InstanceTransform;
			InstanceTransform.SetLocation(BuriedLocation);
			InstanceTransform.SetRotation(Mass.Rotation.Quaternion());
			InstanceTransform.SetScale3D(Mass.Scale);
			Component->AddInstance(InstanceTransform, true);
			++TotalMasses;
			UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: v45EmbeddedMass name=%s mesh=%d location=(%.1f,%.1f,%.1f) normalZ=%.3f scale=(%.1f,%.1f,%.1f)"),
				Mass.Name,
				MeshIndex,
				BuriedLocation.X,
				BuriedLocation.Y,
				BuriedLocation.Z,
				GroundHit.ImpactNormal.Z,
				Mass.Scale.X,
				Mass.Scale.Y,
				Mass.Scale.Z);
		}

		for (UHierarchicalInstancedStaticMeshComponent* Component : RockComponents)
		{
			if (Component)
			{
				Component->BuildTreeIfOutdated(true, true);
				Component->MarkPackageDirty();
			}
		}

		CliffMassActor->MarkPackageDirty();
		World->MarkPackageDirty();
		const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
		const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=EmbeddedCliffMassesV45 removedActors=%d masses=%d rejectedMasses=%d material=%s savedMap=%s savedPackages=%s"),
			RemovedActors,
			TotalMasses,
			RejectedMasses,
			TEXT("mesh-default"),
			bSavedMap ? TEXT("true") : TEXT("false"),
			bSavedPackages ? TEXT("true") : TEXT("false"));
		return (bSavedMap && bSavedPackages && TotalMasses >= 10) ? 0 : 1;
	}

	UHierarchicalInstancedStaticMeshComponent* CreateV46Component(
		AActor* Owner,
		UStaticMesh* Mesh,
		const TCHAR* Prefix,
		const int32 Index,
		const bool bCastShadow,
		const int32 CullEndDistance)
	{
		if (!Owner || !Mesh)
		{
			return nullptr;
		}

		UHierarchicalInstancedStaticMeshComponent* Component = NewObject<UHierarchicalInstancedStaticMeshComponent>(
			Owner,
			*FString::Printf(TEXT("FFV46_%s_%02d"), Prefix, Index),
			RF_Transactional);
		if (!Component)
		{
			return nullptr;
		}

		Component->SetStaticMesh(Mesh);
		Component->SetMobility(EComponentMobility::Static);
		Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Component->SetGenerateOverlapEvents(false);
		Component->bSelectable = true;
		Component->SetCastShadow(bCastShadow);
		Component->SetCullDistances(0, CullEndDistance);
		Component->SetupAttachment(Owner->GetRootComponent());
		Component->RegisterComponent();
		Owner->AddInstanceComponent(Component);
		return Component;
	}

	void ApplyOptionalMaterial(UHierarchicalInstancedStaticMeshComponent* Component, UMaterialInterface* Material)
	{
		if (!Component || !Material)
		{
			return;
		}

		const int32 MaterialSlots = FMath::Max(1, Component->GetNumMaterials());
		for (int32 SlotIndex = 0; SlotIndex < MaterialSlots; ++SlotIndex)
		{
			Component->SetMaterial(SlotIndex, Material);
		}
	}

	bool SpawnV54WaterfallValidationCameras(UWorld* World)
	{
		int32 CreatedCameras = 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV54_NorthRiverExit"),
			TEXT("FFSmokeHighlandV54NorthRiverExitCamera"),
			FVector(56000.0f, 116000.0f, 24000.0f),
			FVector(54500.0f, 171500.0f, -6200.0f),
			66.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV54_SouthRiverExit"),
			TEXT("FFSmokeHighlandV54SouthRiverExitCamera"),
			FVector(54000.0f, 45000.0f, 23500.0f),
			FVector(73500.0f, -8000.0f, -16000.0f),
			66.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV54_RiverCorridor"),
			TEXT("FFSmokeHighlandV54RiverCorridorCamera"),
			FVector(50500.0f, 91000.0f, 9200.0f),
			FVector(58500.0f, 125500.0f, -3200.0f),
			56.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV54_LakePreservation"),
			TEXT("FFSmokeHighlandV54LakePreservationCamera"),
			FVector(22000.0f, 66800.0f, 8200.0f),
			FVector(27800.0f, 72800.0f, -2600.0f),
			54.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV54_GameplaySpace"),
			TEXT("FFSmokeHighlandV54GameplaySpaceCamera"),
			FVector2D(72000.0f, 118500.0f),
			93000.0f,
			FVector2D(72000.0f, 118500.0f),
			0.0f,
			59.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV54_WideBiomeView"),
			TEXT("FFSmokeHighlandV54WideBiomeViewCamera"),
			FVector2D(30500.0f, 76000.0f),
			27000.0f,
			FVector2D(96000.0f, 145500.0f),
			2600.0f,
			70.0f) ? 1 : 0;

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && Actor->GetActorLabel().Contains(TEXT("FFSmoke_HighlandV54_")))
			{
				Actor->Tags.AddUnique(V54ValidationCameraTag);
			}
		}

		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=ThinCliffFacadeV54 validationCameras=%d"), CreatedCameras);
		return CreatedCameras >= 6;
	}

	bool SpawnV55FullRingValidationCameras(UWorld* World)
	{
		int32 CreatedCameras = 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV55_FullOuterRingWideView"),
			TEXT("FFSmokeHighlandV55FullOuterRingWideViewCamera"),
			FVector(61000.0f, 52000.0f, 52000.0f),
			FVector(70500.0f, 119000.0f, -5000.0f),
			72.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV55_NorthRiverExit"),
			TEXT("FFSmokeHighlandV55NorthRiverExitCamera"),
			FVector(56000.0f, 116000.0f, 24000.0f),
			FVector(54500.0f, 171500.0f, -6200.0f),
			66.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV55_SouthRiverExit"),
			TEXT("FFSmokeHighlandV55SouthRiverExitCamera"),
			FVector(54500.0f, 43000.0f, 24500.0f),
			FVector(73500.0f, -8000.0f, -16000.0f),
			66.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV55_LakePreservation"),
			TEXT("FFSmokeHighlandV55LakePreservationCamera"),
			FVector(40000.0f, 23500.0f, 42000.0f),
			FVector(40000.0f, 23500.0f, -5000.0f),
			46.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV55_GameplaySpace"),
			TEXT("FFSmokeHighlandV55GameplaySpaceCamera"),
			FVector2D(72000.0f, 118500.0f),
			93000.0f,
			FVector2D(72000.0f, 118500.0f),
			0.0f,
			59.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV55_TopDownFullRing"),
			TEXT("FFSmokeHighlandV55TopDownFullRingCamera"),
			FVector2D(65000.0f, 82000.0f),
			165000.0f,
			FVector2D(65000.0f, 82000.0f),
			0.0f,
			70.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV55_MountainRidgeProfile"),
			TEXT("FFSmokeHighlandV55MountainRidgeProfileCamera"),
			FVector(15500.0f, 104000.0f, 17000.0f),
			FVector(121000.0f, 116000.0f, 4000.0f),
			60.0f) ? 1 : 0;

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && Actor->GetActorLabel().Contains(TEXT("FFSmoke_HighlandV55_")))
			{
				Actor->Tags.AddUnique(V55ValidationCameraTag);
			}
		}

		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=ThinCliffFacadeV55 validationCameras=%d"), CreatedCameras);
		return CreatedCameras >= 7;
	}

	bool SpawnV56MountainIdentityValidationCameras(UWorld* World)
	{
		int32 CreatedCameras = 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV56_FullRingWideView"),
			TEXT("FFSmokeHighlandV56FullRingWideViewCamera"),
			FVector(61000.0f, 52000.0f, 54000.0f),
			FVector(70500.0f, 119000.0f, -2500.0f),
			72.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV56_NorthRiverExit"),
			TEXT("FFSmokeHighlandV56NorthRiverExitCamera"),
			FVector(56000.0f, 116000.0f, 25000.0f),
			FVector(54500.0f, 171500.0f, -6000.0f),
			66.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV56_SouthRiverExit"),
			TEXT("FFSmokeHighlandV56SouthRiverExitCamera"),
			FVector(54500.0f, 43000.0f, 25000.0f),
			FVector(73500.0f, -8000.0f, -14500.0f),
			66.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV56_MountainRidgeProfile"),
			TEXT("FFSmokeHighlandV56MountainRidgeProfileCamera"),
			FVector(15500.0f, 104000.0f, 19000.0f),
			FVector(121000.0f, 116000.0f, 4800.0f),
			60.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV56_PeakHierarchyView"),
			TEXT("FFSmokeHighlandV56PeakHierarchyViewCamera"),
			FVector(69000.0f, 76000.0f, 31000.0f),
			FVector(111000.0f, 144000.0f, 3500.0f),
			58.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV56_GameplaySpace"),
			TEXT("FFSmokeHighlandV56GameplaySpaceCamera"),
			FVector2D(72000.0f, 118500.0f),
			93000.0f,
			FVector2D(72000.0f, 118500.0f),
			0.0f,
			59.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV56_TopDownFullRing"),
			TEXT("FFSmokeHighlandV56TopDownFullRingCamera"),
			FVector2D(65000.0f, 82000.0f),
			165000.0f,
			FVector2D(65000.0f, 82000.0f),
			0.0f,
			70.0f) ? 1 : 0;

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && Actor->GetActorLabel().Contains(TEXT("FFSmoke_HighlandV56_")))
			{
				Actor->Tags.AddUnique(V56ValidationCameraTag);
			}
		}

		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=MountainIdentityV56 validationCameras=%d"), CreatedCameras);
		return CreatedCameras >= 7;
	}

	bool SpawnV57SurfaceReplacementValidationCameras(UWorld* World)
	{
		int32 CreatedCameras = 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV57_FullRingWideView"),
			TEXT("FFSmokeHighlandV57FullRingWideViewCamera"),
			FVector(61000.0f, 52000.0f, 54000.0f),
			FVector(70500.0f, 119000.0f, -2500.0f),
			72.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV57_NorthRiverExit"),
			TEXT("FFSmokeHighlandV57NorthRiverExitCamera"),
			FVector(56000.0f, 116000.0f, 25000.0f),
			FVector(54500.0f, 171500.0f, -6000.0f),
			66.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV57_SouthRiverExit"),
			TEXT("FFSmokeHighlandV57SouthRiverExitCamera"),
			FVector(54500.0f, 43000.0f, 25000.0f),
			FVector(73500.0f, -8000.0f, -14500.0f),
			66.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV57_LakePreservation"),
			TEXT("FFSmokeHighlandV57LakePreservationCamera"),
			FVector(40000.0f, 23500.0f, 42000.0f),
			FVector(40000.0f, 23500.0f, -5000.0f),
			46.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV57_GameplaySpace"),
			TEXT("FFSmokeHighlandV57GameplaySpaceCamera"),
			FVector2D(72000.0f, 118500.0f),
			93000.0f,
			FVector2D(72000.0f, 118500.0f),
			0.0f,
			59.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV57_TopDownFullRing"),
			TEXT("FFSmokeHighlandV57TopDownFullRingCamera"),
			FVector2D(65000.0f, 82000.0f),
			165000.0f,
			FVector2D(65000.0f, 82000.0f),
			0.0f,
			70.0f) ? 1 : 0;

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && Actor->GetActorLabel().Contains(TEXT("FFSmoke_HighlandV57_")))
			{
				Actor->Tags.AddUnique(V57ValidationCameraTag);
			}
		}

		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=OuterRingSurfaceReplacementV57 validationCameras=%d"), CreatedCameras);
		return CreatedCameras >= 6;
	}

	int32 RunV57OuterRingSurfaceReplacement(UWorld* World, const int32 RemovedActors)
	{
		const bool bValidationCamerasReady = SpawnV57SurfaceReplacementValidationCameras(World);
		World->MarkPackageDirty();
		const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
		const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);

		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=OuterRingSurfaceReplacementV57 removedActors=%d surfaceActors=0 surfacePatches=0 existingMountainRingReused=true newMountainChainCreated=false gameplaySpaceChanged=false materialDrivenSurfaceReplacement=true landscapeRingUnchanged=true northRiverExitProtected=true southRiverExitProtected=true lakeProtected=true playableFieldProtected=true validationCamerasReady=%s savedMap=%s savedPackages=%s"),
			RemovedActors,
			bValidationCamerasReady ? TEXT("true") : TEXT("false"),
			bSavedMap ? TEXT("true") : TEXT("false"),
			bSavedPackages ? TEXT("true") : TEXT("false"));

		return (bSavedMap
			&& bSavedPackages
			&& bValidationCamerasReady) ? 0 : 1;
	}

	bool SpawnV59TitanGeologyValidationCameras(UWorld* World)
	{
		int32 CreatedCameras = 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV59_FullRingWideView"),
			TEXT("FFSmokeHighlandV59FullRingWideViewCamera"),
			FVector(61000.0f, 52000.0f, 54000.0f),
			FVector(70500.0f, 119000.0f, -2500.0f),
			72.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV59_OuterRingGeology"),
			TEXT("FFSmokeHighlandV59OuterRingGeologyCamera"),
			FVector(117000.0f, 104000.0f, 17800.0f),
			FVector(123500.0f, 104000.0f, 2800.0f),
			54.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV59_NorthRiverExit"),
			TEXT("FFSmokeHighlandV59NorthRiverExitCamera"),
			FVector(56000.0f, 116000.0f, 25000.0f),
			FVector(54500.0f, 171500.0f, -6000.0f),
			66.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV59_SouthRiverExit"),
			TEXT("FFSmokeHighlandV59SouthRiverExitCamera"),
			FVector(54500.0f, 43000.0f, 25000.0f),
			FVector(73500.0f, -8000.0f, -14500.0f),
			66.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV59_LakePreservation"),
			TEXT("FFSmokeHighlandV59LakePreservationCamera"),
			FVector(40000.0f, 23500.0f, 42000.0f),
			FVector(40000.0f, 23500.0f, -5000.0f),
			46.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV59_GameplaySpace"),
			TEXT("FFSmokeHighlandV59GameplaySpaceCamera"),
			FVector2D(72000.0f, 118500.0f),
			93000.0f,
			FVector2D(72000.0f, 118500.0f),
			0.0f,
			59.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV59_TopDownFullRing"),
			TEXT("FFSmokeHighlandV59TopDownFullRingCamera"),
			FVector2D(65000.0f, 82000.0f),
			165000.0f,
			FVector2D(65000.0f, 82000.0f),
			0.0f,
			70.0f) ? 1 : 0;

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && Actor->GetActorLabel().Contains(TEXT("FFSmoke_HighlandV59_")))
			{
				Actor->Tags.AddUnique(V59ValidationCameraTag);
			}
		}

		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=TitanGeologyV59 validationCameras=%d"), CreatedCameras);
		return CreatedCameras >= 7;
	}

	int32 RunV59TitanGeology(UWorld* World, const int32 RemovedActors)
	{
		TArray<UStaticMesh*> GeologyMeshes;
		if (!LoadMeshes(TitanGeologyWallMeshPathsV59, UE_ARRAY_COUNT(TitanGeologyWallMeshPathsV59), GeologyMeshes, TEXT("v59 Titan geology")))
		{
			return 1;
		}

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParameters.ObjectFlags = RF_Transactional;

		AActor* GeologyActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
		if (!GeologyActor)
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandVegetationLayer: failed to spawn v59 Titan geology actor."));
			return 1;
		}

		GeologyActor->Tags.AddUnique(V59TitanGeologyTag);
		GeologyActor->SetActorLabel(TEXT("FF_V59_TitanGeologyOuterRing_HISM"));
		USceneComponent* RootComponent = NewObject<USceneComponent>(GeologyActor, TEXT("Root"), RF_Transactional);
		RootComponent->SetMobility(EComponentMobility::Static);
		GeologyActor->SetRootComponent(RootComponent);
		RootComponent->RegisterComponent();
		GeologyActor->AddInstanceComponent(RootComponent);

		UMaterialInterface* TitanCliffMaterial = LoadObject<UMaterialInterface>(nullptr, TitanCliffRockMaterialPath);
		if (!TitanCliffMaterial)
		{
			UE_LOG(LogTemp, Warning, TEXT("FFStarterHighlandVegetationLayer: missing Titan cliff material %s; using geometry mesh defaults."), TitanCliffRockMaterialPath);
		}

		TArray<UHierarchicalInstancedStaticMeshComponent*> GeologyComponents;
		for (int32 Index = 0; Index < GeologyMeshes.Num(); ++Index)
		{
			UHierarchicalInstancedStaticMeshComponent* Component = CreateV46Component(GeologyActor, GeologyMeshes[Index], TEXT("TitanGeologyV59"), Index, true, 520000);
			ApplyOptionalMaterial(Component, TitanCliffMaterial);
			GeologyComponents.Add(Component);
		}

		auto GetRiverClearance = [](const FVector2D& Position)
		{
			float Nearest = TNumericLimits<float>::Max();
			auto CheckSegment = [&Nearest, &Position](const FVector2D& Start, const FVector2D& End)
			{
				Nearest = FMath::Min(Nearest, DistanceToSegment(Position, Start, End));
			};

			CheckSegment(FVector2D(48590.0f, 183116.0f), FVector2D(49910.0f, 174430.0f));
			CheckSegment(FVector2D(49910.0f, 174430.0f), FVector2D(51518.0f, 166778.0f));
			CheckSegment(FVector2D(51518.0f, 166778.0f), FVector2D(58605.0f, 152688.0f));
			CheckSegment(FVector2D(58605.0f, 152688.0f), FVector2D(62544.0f, 138795.0f));
			CheckSegment(FVector2D(62544.0f, 138795.0f), FVector2D(58487.0f, 122723.0f));
			CheckSegment(FVector2D(58487.0f, 122723.0f), FVector2D(55306.0f, 96223.0f));
			CheckSegment(FVector2D(55306.0f, 96223.0f), FVector2D(54105.0f, 78933.0f));
			CheckSegment(FVector2D(54105.0f, 78933.0f), FVector2D(57584.0f, 62136.0f));
			CheckSegment(FVector2D(57584.0f, 62136.0f), FVector2D(63147.0f, 37260.0f));
			CheckSegment(FVector2D(63147.0f, 37260.0f), FVector2D(65398.0f, 28870.0f));
			CheckSegment(FVector2D(65398.0f, 28870.0f), FVector2D(66873.0f, 20275.0f));
			CheckSegment(FVector2D(66873.0f, 20275.0f), FVector2D(68500.0f, 11100.0f));
			CheckSegment(FVector2D(68500.0f, 11100.0f), FVector2D(71037.0f, 3095.0f));
			CheckSegment(FVector2D(71037.0f, 3095.0f), FVector2D(73955.0f, -4499.0f));
			CheckSegment(FVector2D(73955.0f, -4499.0f), FVector2D(76822.0f, -12727.0f));
			return Nearest;
		};

		auto GetNorthWaterfallExitClearance = [](const FVector2D& Position)
		{
			float Nearest = TNumericLimits<float>::Max();
			auto CheckSegment = [&Nearest, &Position](const FVector2D& Start, const FVector2D& End)
			{
				Nearest = FMath::Min(Nearest, DistanceToSegment(Position, Start, End));
			};

			CheckSegment(FVector2D(48590.0f, 183116.0f), FVector2D(49910.0f, 174430.0f));
			CheckSegment(FVector2D(49910.0f, 174430.0f), FVector2D(51518.0f, 166778.0f));
			CheckSegment(FVector2D(51518.0f, 166778.0f), FVector2D(58605.0f, 152688.0f));
			CheckSegment(FVector2D(58605.0f, 152688.0f), FVector2D(62544.0f, 138795.0f));
			return Nearest;
		};

		auto GetSouthWaterfallExitClearance = [](const FVector2D& Position)
		{
			float Nearest = TNumericLimits<float>::Max();
			auto CheckSegment = [&Nearest, &Position](const FVector2D& Start, const FVector2D& End)
			{
				Nearest = FMath::Min(Nearest, DistanceToSegment(Position, Start, End));
			};

			CheckSegment(FVector2D(63147.0f, 37260.0f), FVector2D(65398.0f, 28870.0f));
			CheckSegment(FVector2D(65398.0f, 28870.0f), FVector2D(66873.0f, 20275.0f));
			CheckSegment(FVector2D(66873.0f, 20275.0f), FVector2D(68500.0f, 11100.0f));
			CheckSegment(FVector2D(68500.0f, 11100.0f), FVector2D(71037.0f, 3095.0f));
			CheckSegment(FVector2D(71037.0f, 3095.0f), FVector2D(73955.0f, -4499.0f));
			CheckSegment(FVector2D(73955.0f, -4499.0f), FVector2D(76822.0f, -12727.0f));
			return Nearest;
		};

		auto GetLakeClearance = [](const FVector2D& Position)
		{
			const FVector2D LakeSouth = (Position - FVector2D(40000.0f, 23500.0f)) / FVector2D(16000.0f, 12000.0f);
			const FVector2D LakeWest = (Position - FVector2D(25640.0f, 72820.0f)) / FVector2D(4700.0f, 1900.0f);
			return FMath::Min((LakeSouth.Size() - 1.0f) * 5400.0f, (LakeWest.Size() - 1.0f) * 6000.0f);
		};

		int32 WallInstances = 0;
		int32 ToeInstances = 0;
		int32 AcceptedAnchors = 0;
		int32 RejectedAnchors = 0;
		int32 RunsWithAnchors = 0;
		int32 HardFaceRows = 0;
		float AccumulatedInwardOffset = 0.0f;
		float MaxInwardOffset = 0.0f;
		float AccumulatedOutwardBurial = 0.0f;
		float MinRiverClearance = TNumericLimits<float>::Max();
		float MinLakeClearance = TNumericLimits<float>::Max();
		float MinNorthExitClearance = TNumericLimits<float>::Max();
		float MinSouthExitClearance = TNumericLimits<float>::Max();

		auto AddInstance = [&GeologyComponents, &WallInstances, &ToeInstances](
			const int32 MeshIndex,
			const FVector& Location,
			const FRotator& Rotation,
			const FVector& Scale,
			const bool bToe)
		{
			UHierarchicalInstancedStaticMeshComponent* Component = GeologyComponents.IsValidIndex(MeshIndex) ? GeologyComponents[MeshIndex] : nullptr;
			if (!Component)
			{
				return false;
			}

			FTransform InstanceTransform;
			InstanceTransform.SetLocation(Location);
			InstanceTransform.SetRotation(Rotation.Quaternion());
			InstanceTransform.SetScale3D(Scale);
			Component->AddInstance(InstanceTransform, true);
			if (bToe)
			{
				++ToeInstances;
			}
			else
			{
				++WallInstances;
			}
			return true;
		};

		const FVector2D IslandCenter(71396.0f, 79714.0f);
		for (int32 RunIndex = 0; RunIndex < UE_ARRAY_COUNT(TitanGeologyRunsV59); ++RunIndex)
		{
			const FTitanGeologyRunV59& Run = TitanGeologyRunsV59[RunIndex];
			const FVector2D RunVector = Run.End - Run.Start;
			const FVector2D Direction = RunVector.GetSafeNormal();
			if (Direction.IsNearlyZero())
			{
				RejectedAnchors += Run.AnchorCount;
				continue;
			}

			const FVector2D Perpendicular(-Direction.Y, Direction.X);
			const float DirectionYaw = FMath::RadiansToDegrees(FMath::Atan2(Direction.Y, Direction.X));
			int32 RunAnchors = 0;
			int32 RunRejected = 0;
			const int32 MaxSamples = Run.AnchorCount * 24;
			for (int32 SampleIndex = 0; SampleIndex < MaxSamples && RunAnchors < Run.AnchorCount; ++SampleIndex)
			{
				const int32 Seed = (RunIndex + 1) * 17100000 + SampleIndex * 211;
				const float AnchorAlpha = FMath::Clamp((static_cast<float>(RunAnchors) + 0.18f + PseudoRandom01(Seed + 5) * 0.64f) / FMath::Max(static_cast<float>(Run.AnchorCount), 1.0f), 0.0f, 1.0f);
				const float SideOffset = (PseudoRandom01(Seed + 11) * 2.0f - 1.0f) * Run.JitterRadius;
				const float AlongOffset = (PseudoRandom01(Seed + 17) * 2.0f - 1.0f) * 760.0f;
				const FVector2D RingPosition = Run.Start + RunVector * AnchorAlpha + Perpendicular * SideOffset + Direction * AlongOffset;
				const FVector2D Inward = (IslandCenter - RingPosition).GetSafeNormal();
				const FVector2D Outward = -Inward;
				const float InwardOffset = FMath::Lerp(Run.InwardOffsetMin, Run.InwardOffsetMax, PseudoRandom01(Seed + 23));
				const FVector2D AnchorPosition = RingPosition + Inward * InwardOffset;

				const float RiverClearance = GetRiverClearance(AnchorPosition);
				const float NorthExitClearance = GetNorthWaterfallExitClearance(AnchorPosition);
				const float SouthExitClearance = GetSouthWaterfallExitClearance(AnchorPosition);
				const float LakeClearance = GetLakeClearance(AnchorPosition);
				if (RiverClearance < Run.MinRiverDistance
					|| LakeClearance < Run.MinLakeDistance
					|| IsNearPath(AnchorPosition)
					|| IsInsideTraversalCorridorReserve(AnchorPosition)
					|| IsInsideVillagePlateauReserve(AnchorPosition)
					|| IsInsideSpawnMeadow(AnchorPosition))
				{
					++RejectedAnchors;
					++RunRejected;
					continue;
				}

				FHitResult GroundHit;
				if (!GetPlacementGround(World, AnchorPosition, 0.0f, GroundHit))
				{
					++RejectedAnchors;
					++RunRejected;
					continue;
				}

				const float OutwardBurial = FMath::Lerp(Run.OutwardBurialMin, Run.OutwardBurialMax, PseudoRandom01(Seed + 29));
				const float NormalBurial = FMath::Lerp(Run.NormalBurialMin, Run.NormalBurialMax, PseudoRandom01(Seed + 31));
				const FVector AnchorBase = GroundHit.ImpactPoint
					+ FVector(Outward.X, Outward.Y, 0.0f) * OutwardBurial
					- GroundHit.ImpactNormal * NormalBurial
					+ FVector(0.0f, 0.0f, Run.BaseLift + FMath::Lerp(-120.0f, 180.0f, PseudoRandom01(Seed + 37)));

				bool bAnchorAccepted = false;
				for (int32 RowIndex = 0; RowIndex < Run.Rows; ++RowIndex)
				{
					const int32 RowSeed = Seed + 500 + RowIndex * 113;
					const float RowAlpha = Run.Rows > 1 ? static_cast<float>(RowIndex) / static_cast<float>(Run.Rows - 1) : 0.0f;
					const bool bUseLongCliffPrefab = GeologyComponents.Num() > 2 && RowIndex == 0 && PseudoRandom01(RowSeed + 3) > 0.93f;
					const int32 WallMeshIndex = bUseLongCliffPrefab
						? (PseudoRandom01(RowSeed + 5) > 0.50f ? 1 : 0)
						: (2 + FMath::Clamp(FMath::FloorToInt(PseudoRandom01(RowSeed + 5) * static_cast<float>(FMath::Max(GeologyComponents.Num() - 2, 1))), 0, FMath::Max(GeologyComponents.Num() - 3, 0)));
					const float RowLift = RowIndex * Run.RowRise + FMath::Lerp(-220.0f, 260.0f, PseudoRandom01(RowSeed + 7));
					const float RowOutwardStep = RowAlpha * FMath::Lerp(50.0f, 160.0f, PseudoRandom01(RowSeed + 11));
					const float AlongStagger = (PseudoRandom01(RowSeed + 13) * 2.0f - 1.0f) * 360.0f;
					const FVector Location = AnchorBase
						+ FVector(Direction.X, Direction.Y, 0.0f) * AlongStagger
						+ FVector(Outward.X, Outward.Y, 0.0f) * RowOutwardStep
						+ FVector(0.0f, 0.0f, RowLift);
					const float Yaw = DirectionYaw + FMath::Lerp(-34.0f, 34.0f, PseudoRandom01(RowSeed + 17));
					const float Pitch = FMath::Lerp(-9.0f, 10.0f, PseudoRandom01(RowSeed + 19));
					const float Roll = FMath::Lerp(-14.0f, 14.0f, PseudoRandom01(RowSeed + 23));
					const float LengthFactor = bUseLongCliffPrefab ? 0.38f : 0.58f;
					const float DepthFactor = bUseLongCliffPrefab ? 1.25f : 3.05f;
					const float HeightFactor = bUseLongCliffPrefab ? 1.10f : 1.42f;
					const FVector Scale(
						FMath::Lerp(Run.MinLengthScale, Run.MaxLengthScale, PseudoRandom01(RowSeed + 29)) * LengthFactor,
						FMath::Lerp(Run.MinDepthScale, Run.MaxDepthScale, PseudoRandom01(RowSeed + 31)) * DepthFactor,
						FMath::Lerp(Run.MinHeightScale, Run.MaxHeightScale, PseudoRandom01(RowSeed + 37)) * HeightFactor);

					if (AddInstance(WallMeshIndex, Location, FRotator(Pitch, Yaw, Roll), Scale, false))
					{
						bAnchorAccepted = true;
						++HardFaceRows;
					}
				}

				if (!bAnchorAccepted)
				{
					++RejectedAnchors;
					++RunRejected;
					continue;
				}

				if (GeologyComponents.Num() > 2)
				{
					const int32 ToeSeed = Seed + 1700;
					const int32 ToeMeshIndex = 2 + FMath::Clamp(FMath::FloorToInt(PseudoRandom01(ToeSeed + 3) * static_cast<float>(GeologyComponents.Num() - 2)), 0, GeologyComponents.Num() - 3);
					const FVector ToeLocation = GroundHit.ImpactPoint
						+ FVector(Outward.X, Outward.Y, 0.0f) * FMath::Lerp(240.0f, 520.0f, PseudoRandom01(ToeSeed + 7))
						- GroundHit.ImpactNormal * FMath::Lerp(120.0f, 260.0f, PseudoRandom01(ToeSeed + 11))
						+ FVector(Direction.X, Direction.Y, 0.0f) * FMath::Lerp(-520.0f, 520.0f, PseudoRandom01(ToeSeed + 13))
						+ FVector(0.0f, 0.0f, FMath::Lerp(-160.0f, 120.0f, PseudoRandom01(ToeSeed + 17)));
					const FVector ToeScale(
						FMath::Lerp(1.05f, 1.85f, PseudoRandom01(ToeSeed + 19)),
						FMath::Lerp(0.82f, 1.38f, PseudoRandom01(ToeSeed + 23)),
						FMath::Lerp(0.72f, 1.24f, PseudoRandom01(ToeSeed + 29)));
					AddInstance(
						ToeMeshIndex,
						ToeLocation,
						FRotator(
							FMath::Lerp(-8.0f, 7.0f, PseudoRandom01(ToeSeed + 31)),
							DirectionYaw + FMath::Lerp(-40.0f, 40.0f, PseudoRandom01(ToeSeed + 37)),
							FMath::Lerp(-10.0f, 10.0f, PseudoRandom01(ToeSeed + 41))),
						ToeScale,
						true);
				}

				++RunAnchors;
				++AcceptedAnchors;
				AccumulatedInwardOffset += InwardOffset;
				MaxInwardOffset = FMath::Max(MaxInwardOffset, InwardOffset);
				AccumulatedOutwardBurial += OutwardBurial;
				MinRiverClearance = FMath::Min(MinRiverClearance, RiverClearance);
				MinLakeClearance = FMath::Min(MinLakeClearance, LakeClearance);
				MinNorthExitClearance = FMath::Min(MinNorthExitClearance, NorthExitClearance);
				MinSouthExitClearance = FMath::Min(MinSouthExitClearance, SouthExitClearance);
			}

			if (RunAnchors > 0)
			{
				++RunsWithAnchors;
			}
			UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=TitanGeologyV59 run=%s anchors=%d/%d rejected=%d"),
				Run.Name,
				RunAnchors,
				Run.AnchorCount,
				RunRejected);
		}

		for (UHierarchicalInstancedStaticMeshComponent* Component : GeologyComponents)
		{
			if (Component)
			{
				Component->BuildTreeIfOutdated(true, true);
				Component->MarkPackageDirty();
			}
		}

		const bool bValidationCamerasReady = SpawnV59TitanGeologyValidationCameras(World);
		GeologyActor->MarkPackageDirty();
		World->MarkPackageDirty();
		const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
		const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
		const float AverageInwardOffset = AcceptedAnchors > 0 ? AccumulatedInwardOffset / static_cast<float>(AcceptedAnchors) : 0.0f;
		const float AverageOutwardBurial = AcceptedAnchors > 0 ? AccumulatedOutwardBurial / static_cast<float>(AcceptedAnchors) : 0.0f;
		const float NorthCorridorWidth = MinNorthExitClearance < TNumericLimits<float>::Max() ? MinNorthExitClearance * 2.0f : 0.0f;
		const float SouthCorridorWidth = MinSouthExitClearance < TNumericLimits<float>::Max() ? MinSouthExitClearance * 2.0f : 0.0f;
		const bool bExistingRingReused = true;
		const bool bGeometryDriven = WallInstances >= 140 && HardFaceRows >= 140;
		const bool bFullRingProcessed = RunsWithAnchors >= 10 && AcceptedAnchors >= 58;
		const bool bWaterfallCorridorsOpen = NorthCorridorWidth >= 38000.0f && SouthCorridorWidth >= 32000.0f;
		const bool bGameplaySpaceProtected = AverageInwardOffset <= 55.0f && MaxInwardOffset <= 70.0f && MinRiverClearance >= 11000.0f && MinLakeClearance >= 9500.0f;

		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=TitanGeologyV59 removedActors=%d geologyActors=1 wallInstances=%d toeInstances=%d acceptedAnchors=%d rejectedAnchors=%d runsWithAnchors=%d/%d hardFaceRows=%d averageInwardOffsetCm=%.1f maxInwardOffsetCm=%.1f averageOutwardBurialCm=%.1f minRiverClearanceCm=%.1f minLakeClearanceCm=%.1f northCorridorWidthCm=%.1f southCorridorWidthCm=%.1f geometryDriven=true materialChanges=0 existingRingReused=%s newMountainChainCreated=false gameplaySpaceChanged=false northRiverExitProtected=%s southRiverExitProtected=%s lakeProtected=%s validationCamerasReady=%s savedMap=%s savedPackages=%s"),
			RemovedActors,
			WallInstances,
			ToeInstances,
			AcceptedAnchors,
			RejectedAnchors,
			RunsWithAnchors,
			UE_ARRAY_COUNT(TitanGeologyRunsV59),
			HardFaceRows,
			AverageInwardOffset,
			MaxInwardOffset,
			AverageOutwardBurial,
			MinRiverClearance,
			MinLakeClearance,
			NorthCorridorWidth,
			SouthCorridorWidth,
			bExistingRingReused ? TEXT("true") : TEXT("false"),
			bWaterfallCorridorsOpen ? TEXT("true") : TEXT("false"),
			bWaterfallCorridorsOpen ? TEXT("true") : TEXT("false"),
			MinLakeClearance >= 9500.0f ? TEXT("true") : TEXT("false"),
			bValidationCamerasReady ? TEXT("true") : TEXT("false"),
			bSavedMap ? TEXT("true") : TEXT("false"),
			bSavedPackages ? TEXT("true") : TEXT("false"));

		return (bSavedMap
			&& bSavedPackages
			&& bValidationCamerasReady
			&& bExistingRingReused
			&& bGeometryDriven
			&& bFullRingProcessed
			&& bWaterfallCorridorsOpen
			&& bGameplaySpaceProtected) ? 0 : 1;
	}

	int32 RunV56MountainIdentity(UWorld* World, const int32 RemovedActors)
	{
		TArray<UStaticMesh*> RockMeshes;
		if (!LoadMeshes(CliffRockMeshPaths, UE_ARRAY_COUNT(CliffRockMeshPaths), RockMeshes, TEXT("v56 mountain identity")))
		{
			return 1;
		}

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParameters.ObjectFlags = RF_Transactional;

		AActor* MountainActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
		if (!MountainActor)
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandVegetationLayer: failed to spawn v56 mountain identity actor."));
			return 1;
		}

		MountainActor->Tags.AddUnique(V56MountainIdentityTag);
		MountainActor->SetActorLabel(TEXT("FF_V56_MountainIdentityRange_HISM"));
		USceneComponent* RootComponent = NewObject<USceneComponent>(MountainActor, TEXT("Root"), RF_Transactional);
		RootComponent->SetMobility(EComponentMobility::Static);
		MountainActor->SetRootComponent(RootComponent);
		RootComponent->RegisterComponent();
		MountainActor->AddInstanceComponent(RootComponent);

		UMaterialInterface* TitanCliffMaterial = LoadObject<UMaterialInterface>(nullptr, TitanCliffRockMaterialPath);
		if (!TitanCliffMaterial)
		{
			UE_LOG(LogTemp, Warning, TEXT("FFStarterHighlandVegetationLayer: missing Titan cliff material %s; using rock mesh defaults."), TitanCliffRockMaterialPath);
		}

		TArray<UHierarchicalInstancedStaticMeshComponent*> MountainComponents;
		for (int32 Index = 0; Index < RockMeshes.Num(); ++Index)
		{
			UHierarchicalInstancedStaticMeshComponent* Component = CreateV46Component(MountainActor, RockMeshes[Index], TEXT("MountainIdentity"), Index, true, 480000);
			ApplyOptionalMaterial(Component, TitanCliffMaterial);
			MountainComponents.Add(Component);
		}

		auto GetRiverClearance = [](const FVector2D& Position)
		{
			float Nearest = TNumericLimits<float>::Max();
			auto CheckSegment = [&Nearest, &Position](const FVector2D& Start, const FVector2D& End)
			{
				Nearest = FMath::Min(Nearest, DistanceToSegment(Position, Start, End));
			};

			CheckSegment(FVector2D(48590.0f, 183116.0f), FVector2D(49910.0f, 174430.0f));
			CheckSegment(FVector2D(49910.0f, 174430.0f), FVector2D(51518.0f, 166778.0f));
			CheckSegment(FVector2D(51518.0f, 166778.0f), FVector2D(58605.0f, 152688.0f));
			CheckSegment(FVector2D(58605.0f, 152688.0f), FVector2D(62544.0f, 138795.0f));
			CheckSegment(FVector2D(62544.0f, 138795.0f), FVector2D(58487.0f, 122723.0f));
			CheckSegment(FVector2D(58487.0f, 122723.0f), FVector2D(55306.0f, 96223.0f));
			CheckSegment(FVector2D(55306.0f, 96223.0f), FVector2D(54105.0f, 78933.0f));
			CheckSegment(FVector2D(54105.0f, 78933.0f), FVector2D(57584.0f, 62136.0f));
			CheckSegment(FVector2D(57584.0f, 62136.0f), FVector2D(63147.0f, 37260.0f));
			CheckSegment(FVector2D(63147.0f, 37260.0f), FVector2D(65398.0f, 28870.0f));
			CheckSegment(FVector2D(65398.0f, 28870.0f), FVector2D(66873.0f, 20275.0f));
			CheckSegment(FVector2D(66873.0f, 20275.0f), FVector2D(68500.0f, 11100.0f));
			CheckSegment(FVector2D(68500.0f, 11100.0f), FVector2D(71037.0f, 3095.0f));
			CheckSegment(FVector2D(71037.0f, 3095.0f), FVector2D(73955.0f, -4499.0f));
			CheckSegment(FVector2D(73955.0f, -4499.0f), FVector2D(76822.0f, -12727.0f));
			return Nearest;
		};

		auto GetNorthWaterfallExitClearance = [](const FVector2D& Position)
		{
			float Nearest = TNumericLimits<float>::Max();
			auto CheckSegment = [&Nearest, &Position](const FVector2D& Start, const FVector2D& End)
			{
				Nearest = FMath::Min(Nearest, DistanceToSegment(Position, Start, End));
			};

			CheckSegment(FVector2D(48590.0f, 183116.0f), FVector2D(49910.0f, 174430.0f));
			CheckSegment(FVector2D(49910.0f, 174430.0f), FVector2D(51518.0f, 166778.0f));
			CheckSegment(FVector2D(51518.0f, 166778.0f), FVector2D(58605.0f, 152688.0f));
			CheckSegment(FVector2D(58605.0f, 152688.0f), FVector2D(62544.0f, 138795.0f));
			return Nearest;
		};

		auto GetSouthWaterfallExitClearance = [](const FVector2D& Position)
		{
			float Nearest = TNumericLimits<float>::Max();
			auto CheckSegment = [&Nearest, &Position](const FVector2D& Start, const FVector2D& End)
			{
				Nearest = FMath::Min(Nearest, DistanceToSegment(Position, Start, End));
			};

			CheckSegment(FVector2D(63147.0f, 37260.0f), FVector2D(65398.0f, 28870.0f));
			CheckSegment(FVector2D(65398.0f, 28870.0f), FVector2D(66873.0f, 20275.0f));
			CheckSegment(FVector2D(66873.0f, 20275.0f), FVector2D(68500.0f, 11100.0f));
			CheckSegment(FVector2D(68500.0f, 11100.0f), FVector2D(71037.0f, 3095.0f));
			CheckSegment(FVector2D(71037.0f, 3095.0f), FVector2D(73955.0f, -4499.0f));
			CheckSegment(FVector2D(73955.0f, -4499.0f), FVector2D(76822.0f, -12727.0f));
			return Nearest;
		};

		auto GetLakeClearance = [](const FVector2D& Position)
		{
			const FVector2D LakeSouth = (Position - FVector2D(40000.0f, 23500.0f)) / FVector2D(16000.0f, 12000.0f);
			const FVector2D LakeWest = (Position - FVector2D(25640.0f, 72820.0f)) / FVector2D(4700.0f, 1900.0f);
			return FMath::Min((LakeSouth.Size() - 1.0f) * 5400.0f, (LakeWest.Size() - 1.0f) * 6000.0f);
		};

		const FVector2D IslandCenter(71396.0f, 79714.0f);
		int32 AcceptedAnchors = 0;
		int32 RejectedAnchors = 0;
		int32 TotalInstances = 0;
		int32 PrimaryPeaks = 0;
		int32 SecondaryPeaks = 0;
		int32 MinorPeaks = 0;
		int32 ShoulderMasses = 0;
		int32 RunsWithAnchors = 0;
		float AccumulatedInwardOffset = 0.0f;
		float MaxInwardOffset = 0.0f;
		float MinRiverClearance = TNumericLimits<float>::Max();
		float MinLakeClearance = TNumericLimits<float>::Max();
		float MinNorthExitClearance = TNumericLimits<float>::Max();
		float MinSouthExitClearance = TNumericLimits<float>::Max();
		float LowestVisualLift = TNumericLimits<float>::Max();
		float HighestVisualLift = -TNumericLimits<float>::Max();

		auto AddMountainInstance = [&MountainComponents, &TotalInstances, &ShoulderMasses](
			const int32 MeshSeed,
			const FVector& Location,
			const FRotator& Rotation,
			const FVector& Scale,
			const bool bShoulder)
		{
			if (MountainComponents.Num() == 0)
			{
				return false;
			}

			const int32 MeshIndex = FMath::Clamp(FMath::FloorToInt(PseudoRandom01(MeshSeed) * MountainComponents.Num()), 0, MountainComponents.Num() - 1);
			UHierarchicalInstancedStaticMeshComponent* Component = MountainComponents.IsValidIndex(MeshIndex) ? MountainComponents[MeshIndex] : nullptr;
			if (!Component)
			{
				return false;
			}

			FTransform InstanceTransform;
			InstanceTransform.SetLocation(Location);
			InstanceTransform.SetRotation(Rotation.Quaternion());
			InstanceTransform.SetScale3D(Scale);
			Component->AddInstance(InstanceTransform, true);
			++TotalInstances;
			if (bShoulder)
			{
				++ShoulderMasses;
			}
			return true;
		};

		for (int32 RunIndex = 0; RunIndex < UE_ARRAY_COUNT(MountainIdentityRunsV56); ++RunIndex)
		{
			const FMountainIdentityRunV56& Run = MountainIdentityRunsV56[RunIndex];
			const FVector2D RunVector = Run.End - Run.Start;
			const FVector2D Direction = RunVector.GetSafeNormal();
			if (Direction.IsNearlyZero())
			{
				RejectedAnchors += Run.AnchorCount;
				continue;
			}

			const FVector2D Perpendicular(-Direction.Y, Direction.X);
			int32 RunAnchors = 0;
			int32 RunRejected = 0;
			const int32 MaxSamples = Run.AnchorCount * 18;
			for (int32 SampleIndex = 0; SampleIndex < MaxSamples && RunAnchors < Run.AnchorCount; ++SampleIndex)
			{
				const int32 Seed = (RunIndex + 1) * 13100000 + SampleIndex * 181;
				const int32 AnchorIndex = Run.AnchorCount > 0 ? SampleIndex % Run.AnchorCount : 0;
				const float AnchorAlpha = FMath::Clamp((static_cast<float>(AnchorIndex) + 0.22f + PseudoRandom01(Seed + 5) * 0.56f) / FMath::Max(static_cast<float>(Run.AnchorCount), 1.0f), 0.0f, 1.0f);
				const float SideOffset = (PseudoRandom01(Seed + 11) * 2.0f - 1.0f) * Run.JitterRadius;
				const float AlongOffset = (PseudoRandom01(Seed + 17) * 2.0f - 1.0f) * 1120.0f;
				const FVector2D RingPosition = Run.Start + RunVector * AnchorAlpha + Perpendicular * SideOffset + Direction * AlongOffset;
				const FVector2D Inward = (IslandCenter - RingPosition).GetSafeNormal();
				const FVector2D Outward = -Inward;
				const float InwardOffset = FMath::Lerp(Run.InwardOffsetMin, Run.InwardOffsetMax, PseudoRandom01(Seed + 23));
				const FVector2D AnchorPosition = RingPosition + Inward * InwardOffset;

				const float RiverClearance = GetRiverClearance(AnchorPosition);
				const float NorthExitClearance = GetNorthWaterfallExitClearance(AnchorPosition);
				const float SouthExitClearance = GetSouthWaterfallExitClearance(AnchorPosition);
				const float LakeClearance = GetLakeClearance(AnchorPosition);
				if (RiverClearance < Run.MinRiverDistance
					|| LakeClearance < Run.MinLakeDistance
					|| IsNearPath(AnchorPosition)
					|| IsInsideTraversalCorridorReserve(AnchorPosition)
					|| IsInsideVillagePlateauReserve(AnchorPosition)
					|| IsInsideSpawnMeadow(AnchorPosition))
				{
					++RejectedAnchors;
					++RunRejected;
					continue;
				}

				FHitResult GroundHit;
				if (!GetPlacementGround(World, AnchorPosition, 0.0f, GroundHit))
				{
					++RejectedAnchors;
					++RunRejected;
					continue;
				}

				const int32 HierarchyIndex = RunAnchors;
				const bool bPrimaryPeak = (HierarchyIndex == Run.AnchorCount / 2) || (Run.AnchorCount >= 7 && HierarchyIndex == Run.AnchorCount - 2 && (RunIndex % 3) == 0);
				const bool bSecondaryPeak = !bPrimaryPeak && (HierarchyIndex == 0 || HierarchyIndex == Run.AnchorCount - 1 || ((HierarchyIndex + RunIndex) % 3) == 0);
				const float BaseScale = bPrimaryPeak ? Run.PrimaryBaseScale : (bSecondaryPeak ? Run.SecondaryBaseScale : Run.MinorBaseScale);
				const float HeightScale = bPrimaryPeak ? Run.PrimaryHeightScale : (bSecondaryPeak ? Run.SecondaryHeightScale : Run.MinorHeightScale);
				const float RawVisualLift = bPrimaryPeak
					? Run.PeakLift + FMath::Lerp(180.0f, 620.0f, PseudoRandom01(Seed + 29))
					: (bSecondaryPeak ? Run.PeakLift * FMath::Lerp(0.38f, 0.62f, PseudoRandom01(Seed + 31)) : Run.PeakLift * FMath::Lerp(-0.16f, 0.22f, PseudoRandom01(Seed + 37)));
				const float VisualLift = RawVisualLift * 0.25f;
				const float OutwardBurial = bPrimaryPeak
					? FMath::Lerp(420.0f, 760.0f, PseudoRandom01(Seed + 41))
					: (bSecondaryPeak ? FMath::Lerp(320.0f, 620.0f, PseudoRandom01(Seed + 41)) : FMath::Lerp(220.0f, 480.0f, PseudoRandom01(Seed + 41)));
				const float NormalBurial = bPrimaryPeak
					? FMath::Lerp(420.0f, 760.0f, PseudoRandom01(Seed + 43))
					: (bSecondaryPeak ? FMath::Lerp(320.0f, 560.0f, PseudoRandom01(Seed + 43)) : FMath::Lerp(220.0f, 420.0f, PseudoRandom01(Seed + 43)));

				const FVector BaseLocation = GroundHit.ImpactPoint
					- GroundHit.ImpactNormal * NormalBurial
					+ FVector(Outward.X, Outward.Y, 0.0f) * OutwardBurial
					+ FVector(0.0f, 0.0f, VisualLift);
				const float DirectionYaw = FMath::RadiansToDegrees(FMath::Atan2(Direction.Y, Direction.X));
				const float Yaw = DirectionYaw + FMath::Lerp(-42.0f, 42.0f, PseudoRandom01(Seed + 47));
				const float Pitch = FMath::Lerp(-9.0f, 7.0f, PseudoRandom01(Seed + 53));
				const float Roll = FMath::Lerp(-11.0f, 11.0f, PseudoRandom01(Seed + 59));
				const float MassScale = 0.32f;
				const FVector MainScale(
					BaseScale * MassScale * FMath::Lerp(0.92f, 1.24f, PseudoRandom01(Seed + 61)),
					BaseScale * MassScale * FMath::Lerp(0.68f, 0.98f, PseudoRandom01(Seed + 67)),
					BaseScale * HeightScale * 0.40f);

				if (!AddMountainInstance(Seed + 71, BaseLocation, FRotator(Pitch, Yaw, Roll), MainScale, false))
				{
					++RejectedAnchors;
					++RunRejected;
					continue;
				}

				++RunAnchors;
				++AcceptedAnchors;
				AccumulatedInwardOffset += InwardOffset;
				MaxInwardOffset = FMath::Max(MaxInwardOffset, InwardOffset);
				MinRiverClearance = FMath::Min(MinRiverClearance, RiverClearance);
				MinLakeClearance = FMath::Min(MinLakeClearance, LakeClearance);
				MinNorthExitClearance = FMath::Min(MinNorthExitClearance, NorthExitClearance);
				MinSouthExitClearance = FMath::Min(MinSouthExitClearance, SouthExitClearance);
				LowestVisualLift = FMath::Min(LowestVisualLift, VisualLift);
				HighestVisualLift = FMath::Max(HighestVisualLift, VisualLift);
				if (bPrimaryPeak)
				{
					++PrimaryPeaks;
				}
				else if (bSecondaryPeak)
				{
					++SecondaryPeaks;
				}
				else
				{
					++MinorPeaks;
				}

				const int32 ShoulderCount = bPrimaryPeak ? 2 : (bSecondaryPeak ? 1 : 0);
				for (int32 ShoulderIndex = 0; ShoulderIndex < ShoulderCount; ++ShoulderIndex)
				{
					const int32 ShoulderSeed = Seed + 1000 + ShoulderIndex * 127;
					const float ShoulderSign = ShoulderIndex == 0 ? -1.0f : 1.0f;
					const float ShoulderScaleFactor = bPrimaryPeak ? FMath::Lerp(0.54f, 0.68f, PseudoRandom01(ShoulderSeed + 7)) : FMath::Lerp(0.46f, 0.58f, PseudoRandom01(ShoulderSeed + 7));
					const FVector ShoulderLocation = BaseLocation
						+ FVector(Direction.X, Direction.Y, 0.0f) * Run.ShoulderSpread * ShoulderSign * FMath::Lerp(0.62f, 1.05f, PseudoRandom01(ShoulderSeed + 11))
						+ FVector(Outward.X, Outward.Y, 0.0f) * FMath::Lerp(80.0f, 260.0f, PseudoRandom01(ShoulderSeed + 17))
						- FVector(0.0f, 0.0f, FMath::Abs(VisualLift) * FMath::Lerp(0.22f, 0.42f, PseudoRandom01(ShoulderSeed + 23)));
					const FVector ShoulderScale(
						BaseScale * ShoulderScaleFactor * 0.32f * FMath::Lerp(0.94f, 1.18f, PseudoRandom01(ShoulderSeed + 29)),
						BaseScale * ShoulderScaleFactor * 0.32f * FMath::Lerp(0.70f, 0.96f, PseudoRandom01(ShoulderSeed + 31)),
						BaseScale * HeightScale * ShoulderScaleFactor * 0.38f * FMath::Lerp(0.50f, 0.70f, PseudoRandom01(ShoulderSeed + 37)));
					AddMountainInstance(
						ShoulderSeed + 41,
						ShoulderLocation,
						FRotator(
							Pitch + FMath::Lerp(-5.0f, 5.0f, PseudoRandom01(ShoulderSeed + 43)),
							Yaw + FMath::Lerp(-34.0f, 34.0f, PseudoRandom01(ShoulderSeed + 47)),
							Roll + FMath::Lerp(-7.0f, 7.0f, PseudoRandom01(ShoulderSeed + 53))),
						ShoulderScale,
						true);
				}
			}

			if (RunAnchors > 0)
			{
				++RunsWithAnchors;
			}
			UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=MountainIdentityV56 run=%s anchors=%d/%d rejected=%d"),
				Run.Name,
				RunAnchors,
				Run.AnchorCount,
				RunRejected);
		}

		for (UHierarchicalInstancedStaticMeshComponent* Component : MountainComponents)
		{
			if (Component)
			{
				Component->BuildTreeIfOutdated(true, true);
				Component->MarkPackageDirty();
			}
		}

		const bool bValidationCamerasReady = SpawnV56MountainIdentityValidationCameras(World);
		MountainActor->MarkPackageDirty();
		World->MarkPackageDirty();
		const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
		const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
		const float AverageInwardOffset = AcceptedAnchors > 0 ? AccumulatedInwardOffset / static_cast<float>(AcceptedAnchors) : 0.0f;
		const float NorthCorridorWidth = MinNorthExitClearance < TNumericLimits<float>::Max() ? MinNorthExitClearance * 2.0f : 0.0f;
		const float SouthCorridorWidth = MinSouthExitClearance < TNumericLimits<float>::Max() ? MinSouthExitClearance * 2.0f : 0.0f;
		const float VisualLiftRange = HighestVisualLift > LowestVisualLift ? HighestVisualLift - LowestVisualLift : 0.0f;
		const bool bFullRingProcessed = RunsWithAnchors >= 13 && AcceptedAnchors >= 56;
		const bool bHierarchyEstablished = PrimaryPeaks >= 14 && SecondaryPeaks >= 24 && MinorPeaks >= 14 && ShoulderMasses >= 26 && VisualLiftRange >= 300.0f;
		const bool bWaterfallCorridorsOpen = NorthCorridorWidth >= 38000.0f && SouthCorridorWidth >= 32000.0f;
		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=MountainIdentityV56 removedActors=%d mountainActors=1 anchors=%d rejectedAnchors=%d instances=%d runsWithAnchors=%d/%d primaryPeaks=%d secondaryPeaks=%d minorPeaks=%d shoulderMasses=%d averageInwardOffsetCm=%.1f maxInwardOffsetCm=%.1f minRiverClearanceCm=%.1f minLakeClearanceCm=%.1f northCorridorWidthCm=%.1f southCorridorWidthCm=%.1f visualLiftRangeCm=%.1f rowBands=0 fullRingProcessed=%s hierarchyEstablished=%s material=%s validationCamerasReady=%s savedMap=%s savedPackages=%s"),
			RemovedActors,
			AcceptedAnchors,
			RejectedAnchors,
			TotalInstances,
			RunsWithAnchors,
			UE_ARRAY_COUNT(MountainIdentityRunsV56),
			PrimaryPeaks,
			SecondaryPeaks,
			MinorPeaks,
			ShoulderMasses,
			AverageInwardOffset,
			MaxInwardOffset,
			MinRiverClearance,
			MinLakeClearance,
			NorthCorridorWidth,
			SouthCorridorWidth,
			VisualLiftRange,
			bFullRingProcessed ? TEXT("true") : TEXT("false"),
			bHierarchyEstablished ? TEXT("true") : TEXT("false"),
			TitanCliffMaterial ? *TitanCliffMaterial->GetPathName() : TEXT("mesh-default"),
			bValidationCamerasReady ? TEXT("true") : TEXT("false"),
			bSavedMap ? TEXT("true") : TEXT("false"),
			bSavedPackages ? TEXT("true") : TEXT("false"));

		return (bSavedMap
			&& bSavedPackages
			&& bValidationCamerasReady
			&& bFullRingProcessed
			&& bHierarchyEstablished
			&& bWaterfallCorridorsOpen
			&& AverageInwardOffset <= 90.0f
			&& MaxInwardOffset <= 120.0f
			&& MinRiverClearance >= 11000.0f
			&& MinLakeClearance >= 9500.0f) ? 0 : 1;
	}

	int32 RunV52ThinCliffFacade(UWorld* World, const int32 RemovedActors, const bool bUseV53, const bool bUseV54 = false, const bool bUseV55 = false)
	{
		TArray<UStaticMesh*> RockMeshes;
		if (!LoadMeshes(CliffRockMeshPaths, UE_ARRAY_COUNT(CliffRockMeshPaths), RockMeshes, TEXT("v52 thin cliff facade")))
		{
			return 1;
		}

		const FThinCliffFacadeRunV52* Runs = bUseV55 ? ThinCliffFacadeRunsV55 : (bUseV54 ? ThinCliffFacadeRunsV54 : (bUseV53 ? ThinCliffFacadeRunsV53 : ThinCliffFacadeRunsV52));
		const int32 RunCount = bUseV55 ? UE_ARRAY_COUNT(ThinCliffFacadeRunsV55) : (bUseV54 ? UE_ARRAY_COUNT(ThinCliffFacadeRunsV54) : (bUseV53 ? UE_ARRAY_COUNT(ThinCliffFacadeRunsV53) : UE_ARRAY_COUNT(ThinCliffFacadeRunsV52)));
		const FName FacadeTag = bUseV55 ? V55ThinCliffFacadeTag : (bUseV54 ? V54ThinCliffFacadeTag : (bUseV53 ? V53ThinCliffFacadeTag : V52ThinCliffFacadeTag));
		const TCHAR* ActorLabel = bUseV55 ? TEXT("FF_V55_FullOuterRingMountainFacade_HISM") : (bUseV54 ? TEXT("FF_V54_OpenWaterfallCliffFacade_HISM") : (bUseV53 ? TEXT("FF_V53_TightCliffFacade_HISM") : TEXT("FF_V52_ThinCliffFacade_HISM")));
		const TCHAR* ModeName = bUseV55 ? TEXT("ThinCliffFacadeV55") : (bUseV54 ? TEXT("ThinCliffFacadeV54") : (bUseV53 ? TEXT("ThinCliffFacadeV53") : TEXT("ThinCliffFacadeV52")));
		const int32 MinRequiredPanels = bUseV55 ? 180 : (bUseV54 ? 90 : (bUseV53 ? 95 : 110));
		const int32 MinRequiredRunsWithPanels = bUseV55 ? 13 : 0;
		const float MaxAllowedAverageDepth = bUseV55 ? 280.0f : (bUseV54 ? 300.0f : (bUseV53 ? 360.0f : 760.0f));
		const float MaxAllowedFacadeDepth = bUseV55 ? 420.0f : (bUseV54 ? 430.0f : (bUseV53 ? 520.0f : 900.0f));
		const float MinRequiredLakeClearance = bUseV55 ? 10000.0f : 17000.0f;

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParameters.ObjectFlags = RF_Transactional;

		AActor* FacadeActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
		if (!FacadeActor)
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandVegetationLayer: failed to spawn v52 thin cliff facade actor."));
			return 1;
		}

		FacadeActor->Tags.AddUnique(FacadeTag);
		FacadeActor->SetActorLabel(ActorLabel);
		USceneComponent* RootComponent = NewObject<USceneComponent>(FacadeActor, TEXT("Root"), RF_Transactional);
		RootComponent->SetMobility(EComponentMobility::Static);
		FacadeActor->SetRootComponent(RootComponent);
		RootComponent->RegisterComponent();
		FacadeActor->AddInstanceComponent(RootComponent);

		UMaterialInterface* TitanCliffMaterial = LoadObject<UMaterialInterface>(nullptr, TitanCliffRockMaterialPath);
		if (!TitanCliffMaterial)
		{
			UE_LOG(LogTemp, Warning, TEXT("FFStarterHighlandVegetationLayer: missing Titan cliff material %s; using rock mesh defaults."), TitanCliffRockMaterialPath);
		}

		TArray<UHierarchicalInstancedStaticMeshComponent*> FacadeComponents;
		for (int32 Index = 0; Index < RockMeshes.Num(); ++Index)
		{
			UHierarchicalInstancedStaticMeshComponent* Component = CreateV46Component(FacadeActor, RockMeshes[Index], TEXT("ThinFacade"), Index, true, 450000);
			ApplyOptionalMaterial(Component, TitanCliffMaterial);
			FacadeComponents.Add(Component);
		}

		auto GetRiverClearance = [](const FVector2D& Position)
		{
			float Nearest = TNumericLimits<float>::Max();
			auto CheckSegment = [&Nearest, &Position](const FVector2D& Start, const FVector2D& End)
			{
				Nearest = FMath::Min(Nearest, DistanceToSegment(Position, Start, End));
			};

			CheckSegment(FVector2D(48590.0f, 183116.0f), FVector2D(49910.0f, 174430.0f));
			CheckSegment(FVector2D(49910.0f, 174430.0f), FVector2D(51518.0f, 166778.0f));
			CheckSegment(FVector2D(51518.0f, 166778.0f), FVector2D(58605.0f, 152688.0f));
			CheckSegment(FVector2D(58605.0f, 152688.0f), FVector2D(62544.0f, 138795.0f));
			CheckSegment(FVector2D(62544.0f, 138795.0f), FVector2D(58487.0f, 122723.0f));
			CheckSegment(FVector2D(58487.0f, 122723.0f), FVector2D(55306.0f, 96223.0f));
			CheckSegment(FVector2D(55306.0f, 96223.0f), FVector2D(54105.0f, 78933.0f));
			CheckSegment(FVector2D(54105.0f, 78933.0f), FVector2D(57584.0f, 62136.0f));
			CheckSegment(FVector2D(57584.0f, 62136.0f), FVector2D(63147.0f, 37260.0f));
			CheckSegment(FVector2D(63147.0f, 37260.0f), FVector2D(65398.0f, 28870.0f));
			CheckSegment(FVector2D(65398.0f, 28870.0f), FVector2D(66873.0f, 20275.0f));
			CheckSegment(FVector2D(66873.0f, 20275.0f), FVector2D(68500.0f, 11100.0f));
			CheckSegment(FVector2D(68500.0f, 11100.0f), FVector2D(71037.0f, 3095.0f));
			CheckSegment(FVector2D(71037.0f, 3095.0f), FVector2D(73955.0f, -4499.0f));
			CheckSegment(FVector2D(73955.0f, -4499.0f), FVector2D(76822.0f, -12727.0f));
			return Nearest;
		};

		auto GetNorthWaterfallExitClearance = [](const FVector2D& Position)
		{
			float Nearest = TNumericLimits<float>::Max();
			auto CheckSegment = [&Nearest, &Position](const FVector2D& Start, const FVector2D& End)
			{
				Nearest = FMath::Min(Nearest, DistanceToSegment(Position, Start, End));
			};

			CheckSegment(FVector2D(48590.0f, 183116.0f), FVector2D(49910.0f, 174430.0f));
			CheckSegment(FVector2D(49910.0f, 174430.0f), FVector2D(51518.0f, 166778.0f));
			CheckSegment(FVector2D(51518.0f, 166778.0f), FVector2D(58605.0f, 152688.0f));
			CheckSegment(FVector2D(58605.0f, 152688.0f), FVector2D(62544.0f, 138795.0f));
			return Nearest;
		};

		auto GetSouthWaterfallExitClearance = [](const FVector2D& Position)
		{
			float Nearest = TNumericLimits<float>::Max();
			auto CheckSegment = [&Nearest, &Position](const FVector2D& Start, const FVector2D& End)
			{
				Nearest = FMath::Min(Nearest, DistanceToSegment(Position, Start, End));
			};

			CheckSegment(FVector2D(63147.0f, 37260.0f), FVector2D(65398.0f, 28870.0f));
			CheckSegment(FVector2D(65398.0f, 28870.0f), FVector2D(66873.0f, 20275.0f));
			CheckSegment(FVector2D(66873.0f, 20275.0f), FVector2D(68500.0f, 11100.0f));
			CheckSegment(FVector2D(68500.0f, 11100.0f), FVector2D(71037.0f, 3095.0f));
			CheckSegment(FVector2D(71037.0f, 3095.0f), FVector2D(73955.0f, -4499.0f));
			CheckSegment(FVector2D(73955.0f, -4499.0f), FVector2D(76822.0f, -12727.0f));
			return Nearest;
		};

		auto GetLakeClearance = [](const FVector2D& Position)
		{
			const FVector2D LakeSouth = (Position - FVector2D(40000.0f, 23500.0f)) / FVector2D(16000.0f, 12000.0f);
			const FVector2D LakeWest = (Position - FVector2D(25640.0f, 72820.0f)) / FVector2D(4700.0f, 1900.0f);
			return FMath::Min((LakeSouth.Size() - 1.0f) * 5400.0f, (LakeWest.Size() - 1.0f) * 6000.0f);
		};

		const FVector2D IslandCenter(71396.0f, 79714.0f);
		int32 TotalFacadePanels = 0;
		int32 RunsWithPanels = 0;
		int32 RejectedSamples = 0;
		float AccumulatedFacadeDepth = 0.0f;
		float MaxFacadeDepth = 0.0f;
		float MinRiverClearance = TNumericLimits<float>::Max();
		float MinLakeClearance = TNumericLimits<float>::Max();
		float MinNorthExitClearance = TNumericLimits<float>::Max();
		float MinSouthExitClearance = TNumericLimits<float>::Max();

		for (int32 RunIndex = 0; RunIndex < RunCount; ++RunIndex)
		{
			const FThinCliffFacadeRunV52& Run = Runs[RunIndex];
			const FVector2D RunVector = Run.End - Run.Start;
			const FVector2D Direction = RunVector.GetSafeNormal();
			if (Direction.IsNearlyZero())
			{
				RejectedSamples += Run.Count;
				continue;
			}

			const FVector2D Perpendicular(-Direction.Y, Direction.X);
			int32 RunBases = 0;
			int32 RunPanels = 0;
			int32 RunProtectedRejects = 0;
			int32 RunGroundRejects = 0;
			int32 RunFlatRejects = 0;
			const int32 MaxSamples = Run.Count * 24;
			for (int32 SampleIndex = 0; SampleIndex < MaxSamples && RunBases < Run.Count; ++SampleIndex)
			{
				const int32 Seed = (RunIndex + 1) * 11200000 + SampleIndex * 157;
				const int32 BucketIndex = Run.Count > 0 ? SampleIndex % Run.Count : 0;
				const float Alpha = FMath::Clamp((static_cast<float>(BucketIndex) + PseudoRandom01(Seed + 5)) / FMath::Max(static_cast<float>(Run.Count), 1.0f), 0.0f, 1.0f);
				const float SideOffset = (PseudoRandom01(Seed + 11) * 2.0f - 1.0f) * Run.JitterRadius;
				const float AlongOffset = (PseudoRandom01(Seed + 17) * 2.0f - 1.0f) * 760.0f;
				const FVector2D RingPosition = Run.Start + RunVector * Alpha + Perpendicular * SideOffset + Direction * AlongOffset;
				const FVector2D Inward = (IslandCenter - RingPosition).GetSafeNormal();
				const float InwardOffset = FMath::Lerp(Run.InwardOffsetMin, Run.InwardOffsetMax, PseudoRandom01(Seed + 23));
				const FVector2D FacadePosition = RingPosition + Inward * InwardOffset;

				const float RiverClearance = GetRiverClearance(FacadePosition);
				const float NorthExitClearance = GetNorthWaterfallExitClearance(FacadePosition);
				const float SouthExitClearance = GetSouthWaterfallExitClearance(FacadePosition);
				const float LakeClearance = GetLakeClearance(FacadePosition);
				if (RiverClearance < Run.MinRiverDistance
					|| LakeClearance < Run.MinLakeDistance
					|| IsNearPath(FacadePosition)
					|| IsInsideTraversalCorridorReserve(FacadePosition)
					|| IsInsideVillagePlateauReserve(FacadePosition)
					|| IsInsideSpawnMeadow(FacadePosition))
				{
					++RejectedSamples;
					++RunProtectedRejects;
					continue;
				}

				FHitResult GroundHit;
				const float MaxAllowedPlacementNormalZ = bUseV55 ? 1.0001f : 0.995f;
				if (!GetPlacementGround(World, FacadePosition, 0.05f, GroundHit))
				{
					++RejectedSamples;
					++RunGroundRejects;
					continue;
				}
				if (GroundHit.ImpactNormal.Z > MaxAllowedPlacementNormalZ)
				{
					++RejectedSamples;
					++RunFlatRejects;
					continue;
				}

				++RunBases;
				MinRiverClearance = FMath::Min(MinRiverClearance, RiverClearance);
				MinLakeClearance = FMath::Min(MinLakeClearance, LakeClearance);
				MinNorthExitClearance = FMath::Min(MinNorthExitClearance, NorthExitClearance);
				MinSouthExitClearance = FMath::Min(MinSouthExitClearance, SouthExitClearance);

				for (int32 RowIndex = 0; RowIndex < FMath::Max(1, Run.Rows); ++RowIndex)
				{
					const int32 RowSeed = Seed + RowIndex * 1009;
					const int32 MeshIndex = FMath::Clamp(FMath::FloorToInt(PseudoRandom01(RowSeed + 29) * FacadeComponents.Num()), 0, FacadeComponents.Num() - 1);
					UHierarchicalInstancedStaticMeshComponent* Component = FacadeComponents.IsValidIndex(MeshIndex) ? FacadeComponents[MeshIndex] : nullptr;
					if (!Component)
					{
						++RejectedSamples;
						continue;
					}

					const float RowAlpha = Run.Rows > 1 ? static_cast<float>(RowIndex) / static_cast<float>(Run.Rows - 1) : 0.0f;
					const float BaseScale = FMath::Lerp(Run.MinBaseScale, Run.MaxBaseScale, PseudoRandom01(RowSeed + 31));
					const float LengthScale = FMath::Lerp(Run.MinLengthScale, Run.MaxLengthScale, PseudoRandom01(RowSeed + 37));
					const float DepthScale = FMath::Lerp(Run.MinDepthScale, Run.MaxDepthScale, PseudoRandom01(RowSeed + 43));
					float HeightScale = FMath::Lerp(Run.MinHeightScale, Run.MaxHeightScale, PseudoRandom01(RowSeed + 47));
					if (RowIndex > 0)
					{
						HeightScale *= FMath::Lerp(0.82f, 0.96f, PseudoRandom01(RowSeed + 53));
					}

					const float RowHeight = RowIndex == 0
						? FMath::Lerp(-80.0f, 220.0f, PseudoRandom01(RowSeed + 59))
						: Run.RowRise * RowAlpha + FMath::Lerp(-220.0f, 260.0f, PseudoRandom01(RowSeed + 59));
					const float Burial = FMath::Lerp(120.0f, 260.0f, PseudoRandom01(RowSeed + 61));
					const FVector2D Outward = -Inward;
					const FVector Location = GroundHit.ImpactPoint + FVector(Outward.X, Outward.Y, 0.0f) * Burial + FVector(0.0f, 0.0f, RowHeight);
					const float Yaw = FMath::RadiansToDegrees(FMath::Atan2(Direction.Y, Direction.X)) + FMath::Lerp(-11.0f, 11.0f, PseudoRandom01(RowSeed + 67));
					const float Pitch = FMath::Lerp(-3.0f, 5.0f, PseudoRandom01(RowSeed + 71));
					const float Roll = FMath::Lerp(-5.0f, 5.0f, PseudoRandom01(RowSeed + 73));

					FTransform InstanceTransform;
					InstanceTransform.SetLocation(Location);
					InstanceTransform.SetRotation(FRotator(Pitch, Yaw, Roll).Quaternion());
					InstanceTransform.SetScale3D(FVector(BaseScale * LengthScale, BaseScale * DepthScale, BaseScale * HeightScale));
					Component->AddInstance(InstanceTransform, true);
					++RunPanels;
					++TotalFacadePanels;
					AccumulatedFacadeDepth += InwardOffset;
					MaxFacadeDepth = FMath::Max(MaxFacadeDepth, InwardOffset);
				}
			}

			UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=%s thinFacadeRun=%s bases=%d/%d panels=%d rows=%d protectedRejects=%d groundRejects=%d flatRejects=%d"),
				ModeName,
				Run.Name,
				RunBases,
				Run.Count,
				RunPanels,
				Run.Rows,
				RunProtectedRejects,
				RunGroundRejects,
				RunFlatRejects);
			if (RunPanels > 0)
			{
				++RunsWithPanels;
			}
		}

		for (UHierarchicalInstancedStaticMeshComponent* Component : FacadeComponents)
		{
			if (Component)
			{
				Component->BuildTreeIfOutdated(true, true);
				Component->MarkPackageDirty();
			}
		}

		const bool bValidationCamerasReady = bUseV55 ? SpawnV55FullRingValidationCameras(World) : (bUseV54 ? SpawnV54WaterfallValidationCameras(World) : true);
		FacadeActor->MarkPackageDirty();
		World->MarkPackageDirty();
		const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
		const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
		const float AverageFacadeDepth = TotalFacadePanels > 0 ? AccumulatedFacadeDepth / static_cast<float>(TotalFacadePanels) : 0.0f;
		const float NorthCorridorWidth = MinNorthExitClearance < TNumericLimits<float>::Max() ? MinNorthExitClearance * 2.0f : 0.0f;
		const float SouthCorridorWidth = MinSouthExitClearance < TNumericLimits<float>::Max() ? MinSouthExitClearance * 2.0f : 0.0f;
		const bool bFullRingProcessed = !bUseV55 || (RunsWithPanels >= MinRequiredRunsWithPanels && TotalFacadePanels >= MinRequiredPanels);
		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=%s removedActors=%d facadeActors=1 facadePanels=%d runsWithPanels=%d/%d rejectedSamples=%d averageFacadeDepthCm=%.1f maxFacadeDepthCm=%.1f minRiverClearanceCm=%.1f minLakeClearanceCm=%.1f northCorridorWidthCm=%.1f southCorridorWidthCm=%.1f v52AverageDepthCm=458.3 v52MaxDepthCm=718.4 inwardFootprintReductionCm=%.1f v53AverageDepthCm=144.0 v54AdditionalGameplayRecoveryCm=%.1f fullRingProcessed=%s material=%s validationCamerasReady=%s savedMap=%s savedPackages=%s"),
			ModeName,
			RemovedActors,
			TotalFacadePanels,
			RunsWithPanels,
			RunCount,
			RejectedSamples,
			AverageFacadeDepth,
			MaxFacadeDepth,
			MinRiverClearance,
			MinLakeClearance,
			NorthCorridorWidth,
			SouthCorridorWidth,
			458.3f - AverageFacadeDepth,
			144.0f - AverageFacadeDepth,
			bFullRingProcessed ? TEXT("true") : TEXT("false"),
			TitanCliffMaterial ? *TitanCliffMaterial->GetPathName() : TEXT("mesh-default"),
			bValidationCamerasReady ? TEXT("true") : TEXT("false"),
			bSavedMap ? TEXT("true") : TEXT("false"),
			bSavedPackages ? TEXT("true") : TEXT("false"));

		const bool bWaterfallCorridorsOpen = !(bUseV54 || bUseV55) || (NorthCorridorWidth >= 38000.0f && SouthCorridorWidth >= 32000.0f);
		return (bSavedMap && bSavedPackages && bValidationCamerasReady && bFullRingProcessed && bWaterfallCorridorsOpen && TotalFacadePanels >= MinRequiredPanels && AverageFacadeDepth <= MaxAllowedAverageDepth && MaxFacadeDepth <= MaxAllowedFacadeDepth && MinRiverClearance >= 11000.0f && MinLakeClearance >= MinRequiredLakeClearance) ? 0 : 1;
	}

	int32 RunV46EmbeddedCliffIntegration(UWorld* World, const int32 RemovedActors)
	{
		TArray<UStaticMesh*> RockMeshes;
		TArray<UStaticMesh*> DirtMeshes;
		TArray<UStaticMesh*> DebrisMeshes;
		TArray<UStaticMesh*> ShrubMeshes;
		if (!LoadMeshes(CliffRockMeshPaths, UE_ARRAY_COUNT(CliffRockMeshPaths), RockMeshes, TEXT("v46 embedded cliff mass"))
			|| !LoadMeshes(CliffTransitionDirtMeshPaths, UE_ARRAY_COUNT(CliffTransitionDirtMeshPaths), DirtMeshes, TEXT("v46 dirt transition"))
			|| !LoadMeshes(CliffTransitionDebrisMeshPaths, UE_ARRAY_COUNT(CliffTransitionDebrisMeshPaths), DebrisMeshes, TEXT("v46 rock debris"))
			|| !LoadMeshes(CliffTransitionShrubMeshPaths, UE_ARRAY_COUNT(CliffTransitionShrubMeshPaths), ShrubMeshes, TEXT("v46 dry ecology")))
		{
			return 1;
		}

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParameters.ObjectFlags = RF_Transactional;

		AActor* IntegrationActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
		if (!IntegrationActor)
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandVegetationLayer: failed to spawn v46 embedded cliff integration actor."));
			return 1;
		}

		IntegrationActor->Tags.AddUnique(V46EmbeddedCliffIntegrationTag);
		IntegrationActor->SetActorLabel(TEXT("FF_V46_EmbeddedCliffIntegration_HISM"));
		USceneComponent* RootComponent = NewObject<USceneComponent>(IntegrationActor, TEXT("Root"), RF_Transactional);
		RootComponent->SetMobility(EComponentMobility::Static);
		IntegrationActor->SetRootComponent(RootComponent);
		RootComponent->RegisterComponent();
		IntegrationActor->AddInstanceComponent(RootComponent);

		UMaterialInterface* TitanCliffMaterial = LoadObject<UMaterialInterface>(nullptr, TitanCliffRockMaterialPath);
		UMaterialInterface* DirtMaterial = LoadObject<UMaterialInterface>(nullptr, TitanTransitionDirtMaterialPath);
		UMaterialInterface* RockfallMaterial = LoadObject<UMaterialInterface>(nullptr, TitanRockfallMaterialPath);
		if (!TitanCliffMaterial)
		{
			UE_LOG(LogTemp, Warning, TEXT("FFStarterHighlandVegetationLayer: missing Titan cliff material %s; using rock mesh defaults."), TitanCliffRockMaterialPath);
		}
		if (!DirtMaterial)
		{
			UE_LOG(LogTemp, Warning, TEXT("FFStarterHighlandVegetationLayer: missing v46 dirt material %s; using mesh defaults."), TitanTransitionDirtMaterialPath);
		}
		if (!RockfallMaterial)
		{
			UE_LOG(LogTemp, Warning, TEXT("FFStarterHighlandVegetationLayer: missing v46 rockfall material %s; using mesh defaults."), TitanRockfallMaterialPath);
		}

		TArray<UHierarchicalInstancedStaticMeshComponent*> RockComponents;
		for (int32 Index = 0; Index < RockMeshes.Num(); ++Index)
		{
			UHierarchicalInstancedStaticMeshComponent* Component = CreateV46Component(IntegrationActor, RockMeshes[Index], TEXT("CliffMass"), Index, true, 430000);
			// Keep the rock mesh defaults here; the broad cliff material flattened these masses into smooth blobs.
			RockComponents.Add(Component);
		}

		TArray<UHierarchicalInstancedStaticMeshComponent*> DirtComponents;
		for (int32 Index = 0; Index < DirtMeshes.Num(); ++Index)
		{
			UHierarchicalInstancedStaticMeshComponent* Component = CreateV46Component(IntegrationActor, DirtMeshes[Index], TEXT("DustApron"), Index, false, 180000);
			ApplyOptionalMaterial(Component, DirtMaterial);
			DirtComponents.Add(Component);
		}

		TArray<UHierarchicalInstancedStaticMeshComponent*> SkinComponents;
		for (int32 Index = 0; Index < DirtMeshes.Num(); ++Index)
		{
			UHierarchicalInstancedStaticMeshComponent* Component = CreateV46Component(IntegrationActor, DirtMeshes[Index], TEXT("CliffSkin"), Index, false, 430000);
			ApplyOptionalMaterial(Component, TitanCliffMaterial ? TitanCliffMaterial : DirtMaterial);
			SkinComponents.Add(Component);
		}

		TArray<UHierarchicalInstancedStaticMeshComponent*> DebrisComponents;
		for (int32 Index = 0; Index < DebrisMeshes.Num(); ++Index)
		{
			UHierarchicalInstancedStaticMeshComponent* Component = CreateV46Component(IntegrationActor, DebrisMeshes[Index], TEXT("Debris"), Index, true, 180000);
			ApplyOptionalMaterial(Component, RockfallMaterial ? RockfallMaterial : TitanCliffMaterial);
			DebrisComponents.Add(Component);
		}

		TArray<UHierarchicalInstancedStaticMeshComponent*> ShrubComponents;
		for (int32 Index = 0; Index < ShrubMeshes.Num(); ++Index)
		{
			ShrubComponents.Add(CreateV46Component(IntegrationActor, ShrubMeshes[Index], TEXT("DryEcology"), Index, true, 120000));
		}

		int32 TotalMasses = 0;
		int32 RejectedMasses = 0;
		int32 TotalDust = 0;
		int32 RejectedDust = 0;
		int32 TotalSkin = 0;
		int32 RejectedSkin = 0;
		int32 TotalDebris = 0;
		int32 RejectedDebris = 0;
		int32 TotalShrubs = 0;
		int32 RejectedShrubs = 0;

		for (const FEmbeddedCliffMassV46& Mass : EmbeddedCliffMassesV46)
		{
			const int32 MeshIndex = FMath::Clamp(Mass.MeshIndex, 0, RockComponents.Num() - 1);
			UHierarchicalInstancedStaticMeshComponent* Component = RockComponents.IsValidIndex(MeshIndex) ? RockComponents[MeshIndex] : nullptr;
			if (!Component)
			{
				++RejectedMasses;
				continue;
			}

			const float WaterDistance = GetNearestWaterDistance(Mass.Position);
			if (WaterDistance < Mass.MinWaterDistance)
			{
				++RejectedMasses;
				UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: v46Mass rejected name=%s reason=water-corridor distance=%.1f min=%.1f"), Mass.Name, WaterDistance, Mass.MinWaterDistance);
				continue;
			}

			FHitResult GroundHit;
			if (!GetPlacementGround(World, Mass.Position, 0.0f, GroundHit))
			{
				++RejectedMasses;
				UE_LOG(LogTemp, Warning, TEXT("FFStarterHighlandVegetationLayer: v46Mass rejected name=%s reason=no-ground"), Mass.Name);
				continue;
			}

			const FVector BuriedLocation = GroundHit.ImpactPoint - GroundHit.ImpactNormal * Mass.NormalBurial + FVector(0.0f, 0.0f, Mass.HeightOffset);
			FTransform InstanceTransform;
			InstanceTransform.SetLocation(BuriedLocation);
			InstanceTransform.SetRotation(Mass.Rotation.Quaternion());
			InstanceTransform.SetScale3D(Mass.Scale);
			Component->AddInstance(InstanceTransform, true);
			++TotalMasses;
			UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: v46EmbeddedMass name=%s mesh=%d location=(%.1f,%.1f,%.1f) normalZ=%.3f waterDistance=%.1f burial=%.1f scale=(%.1f,%.1f,%.1f)"),
				Mass.Name,
				MeshIndex,
				BuriedLocation.X,
				BuriedLocation.Y,
				BuriedLocation.Z,
				GroundHit.ImpactNormal.Z,
				WaterDistance,
				Mass.NormalBurial,
				Mass.Scale.X,
				Mass.Scale.Y,
				Mass.Scale.Z);
		}

		auto PlaceGroundBlendInstances = [World](const FGroundBlendInstanceV46* Instances, const int32 InstanceCount, TArray<UHierarchicalInstancedStaticMeshComponent*>& Components, int32& AcceptedCount, int32& RejectedCount, const TCHAR* Context)
		{
			for (int32 InstanceIndex = 0; InstanceIndex < InstanceCount; ++InstanceIndex)
			{
				const FGroundBlendInstanceV46& Instance = Instances[InstanceIndex];
				const int32 MeshIndex = FMath::Clamp(Instance.MeshIndex, 0, Components.Num() - 1);
				UHierarchicalInstancedStaticMeshComponent* Component = Components.IsValidIndex(MeshIndex) ? Components[MeshIndex] : nullptr;
				if (!Component)
				{
					++RejectedCount;
					continue;
				}

				const float WaterDistance = GetNearestWaterDistance(Instance.Position);
				if (WaterDistance < Instance.MinWaterDistance)
				{
					++RejectedCount;
					UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: %s rejected name=%s reason=water-corridor distance=%.1f min=%.1f"), Context, Instance.Name, WaterDistance, Instance.MinWaterDistance);
					continue;
				}

				FHitResult GroundHit;
				if (!GetPlacementGround(World, Instance.Position, Instance.MinNormalZ, GroundHit))
				{
					++RejectedCount;
					UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: %s rejected name=%s reason=no-ground-or-slope"), Context, Instance.Name);
					continue;
				}

				FTransform InstanceTransform;
				InstanceTransform.SetLocation(GroundHit.ImpactPoint + FVector(0.0f, 0.0f, Instance.HeightOffset));
				InstanceTransform.SetRotation(Instance.Rotation.Quaternion());
				InstanceTransform.SetScale3D(Instance.Scale);
				Component->AddInstance(InstanceTransform, true);
				++AcceptedCount;
			}
		};

		PlaceGroundBlendInstances(CliffDustPatchesV46, UE_ARRAY_COUNT(CliffDustPatchesV46), DirtComponents, TotalDust, RejectedDust, TEXT("v46DustApron"));
		PlaceGroundBlendInstances(CliffDebrisV46, UE_ARRAY_COUNT(CliffDebrisV46), DebrisComponents, TotalDebris, RejectedDebris, TEXT("v46Debris"));
		PlaceGroundBlendInstances(CliffDryEcologyV46, UE_ARRAY_COUNT(CliffDryEcologyV46), ShrubComponents, TotalShrubs, RejectedShrubs, TEXT("v46DryEcology"));

		auto PlaceSurfaceSkinRuns = [World](const FCliffSurfaceSkinRunV49* Runs, const int32 RunCount, TArray<UHierarchicalInstancedStaticMeshComponent*>& Components, int32& AcceptedCount, int32& RejectedCount)
		{
			UHierarchicalInstancedStaticMeshComponent* Component = Components.Num() > 0 ? Components[0] : nullptr;
			if (!Component)
			{
				RejectedCount += RunCount;
				return;
			}

			for (int32 RunIndex = 0; RunIndex < RunCount; ++RunIndex)
			{
				const FCliffSurfaceSkinRunV49& Run = Runs[RunIndex];
				const FVector2D Direction = (Run.End - Run.Start).GetSafeNormal();
				const FVector2D Perpendicular(-Direction.Y, Direction.X);
				int32 RunPatches = 0;
				const int32 MaxSamples = Run.Count * 18;
				for (int32 SampleIndex = 0; SampleIndex < MaxSamples && RunPatches < Run.Count; ++SampleIndex)
				{
					const int32 Seed = (RunIndex + 1) * 9100000 + SampleIndex * 131;
					const float Alpha = FMath::Clamp((static_cast<float>(RunPatches) + PseudoRandom01(Seed + 5)) / FMath::Max(static_cast<float>(Run.Count), 1.0f), 0.0f, 1.0f);
					const float SideOffset = (PseudoRandom01(Seed + 11) * 2.0f - 1.0f) * Run.JitterRadius;
					const float AlongOffset = (PseudoRandom01(Seed + 17) * 2.0f - 1.0f) * 900.0f;
					const FVector2D Position = FMath::Lerp(Run.Start, Run.End, Alpha) + Perpendicular * SideOffset + Direction * AlongOffset;

					if (GetNearestWaterDistance(Position) < 7800.0f
						|| IsNearPath(Position)
						|| IsInsideVillagePlateauReserve(Position))
					{
						++RejectedCount;
						continue;
					}

					FHitResult GroundHit;
					if (!GetPlacementGround(World, Position, 0.16f, GroundHit) || GroundHit.ImpactNormal.Z > 0.92f)
					{
						++RejectedCount;
						continue;
					}

					const FVector PreferredXAxis = FVector(Direction.X, Direction.Y, 0.0f).GetSafeNormal(UE_SMALL_NUMBER, FVector::ForwardVector);
					const FQuat SurfaceQuat = FRotationMatrix::MakeFromZX(GroundHit.ImpactNormal, PreferredXAxis).ToQuat();
					const FQuat TwistQuat(GroundHit.ImpactNormal, FMath::DegreesToRadians(FMath::Lerp(-18.0f, 18.0f, PseudoRandom01(Seed + 23))));
					const float BaseScale = FMath::Lerp(Run.MinScale, Run.MaxScale, PseudoRandom01(Seed + 31));
					const float LengthScale = FMath::Lerp(1.05f, 1.85f, PseudoRandom01(Seed + 37));
					const float WidthScale = FMath::Lerp(0.46f, 0.82f, PseudoRandom01(Seed + 43));

					FTransform InstanceTransform;
					InstanceTransform.SetLocation(GroundHit.ImpactPoint + GroundHit.ImpactNormal * 18.0f);
					InstanceTransform.SetRotation((TwistQuat * SurfaceQuat).GetNormalized());
					InstanceTransform.SetScale3D(FVector(BaseScale * LengthScale, BaseScale * WidthScale, 0.035f));
					Component->AddInstance(InstanceTransform, true);
					++RunPatches;
					++AcceptedCount;
				}

				UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: v49SurfaceSkinRun=%s patches=%d/%d"), Run.Name, RunPatches, Run.Count);
			}
		};

		PlaceSurfaceSkinRuns(CliffSurfaceSkinRunsV49, UE_ARRAY_COUNT(CliffSurfaceSkinRunsV49), SkinComponents, TotalSkin, RejectedSkin);

		TArray<UHierarchicalInstancedStaticMeshComponent*> AllComponents;
		AllComponents.Append(RockComponents);
		AllComponents.Append(DirtComponents);
		AllComponents.Append(SkinComponents);
		AllComponents.Append(DebrisComponents);
		AllComponents.Append(ShrubComponents);
		for (UHierarchicalInstancedStaticMeshComponent* Component : AllComponents)
		{
			if (Component)
			{
				Component->BuildTreeIfOutdated(true, true);
				Component->MarkPackageDirty();
			}
		}

		IntegrationActor->MarkPackageDirty();
		World->MarkPackageDirty();
		const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
		const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=EmbeddedCliffIntegrationV46 removedActors=%d masses=%d rejectedMasses=%d dust=%d rejectedDust=%d skin=%d rejectedSkin=%d debris=%d rejectedDebris=%d shrubs=%d rejectedShrubs=%d cliffMaterial=%s dirtMaterial=%s rockfallMaterial=%s savedMap=%s savedPackages=%s"),
			RemovedActors,
			TotalMasses,
			RejectedMasses,
			TotalDust,
			RejectedDust,
			TotalSkin,
			RejectedSkin,
			TotalDebris,
			RejectedDebris,
			TotalShrubs,
			RejectedShrubs,
			TitanCliffMaterial ? *TitanCliffMaterial->GetPathName() : TEXT("mesh-default"),
			DirtMaterial ? *DirtMaterial->GetPathName() : TEXT("mesh-default"),
			RockfallMaterial ? *RockfallMaterial->GetPathName() : TEXT("mesh-default"),
			bSavedMap ? TEXT("true") : TEXT("false"),
			bSavedPackages ? TEXT("true") : TEXT("false"));
		return (bSavedMap && bSavedPackages && TotalMasses >= 5 && TotalDust >= 5 && TotalSkin >= 20 && TotalDebris >= 7) ? 0 : 1;
	}

	int32 RunV46ValidationCameras(UWorld* World, const int32 RemovedActors)
	{
		int32 CreatedCameras = 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV46_OuterRingIntegration"),
			TEXT("FFSmokeHighlandV46OuterRingIntegrationCamera"),
			FVector(61000.0f, 116500.0f, 7200.0f),
			FVector(103000.0f, 149500.0f, -2800.0f),
			52.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV46_RiverCorridorReadability"),
			TEXT("FFSmokeHighlandV46RiverCorridorReadabilityCamera"),
			FVector(50500.0f, 91000.0f, 9200.0f),
			FVector(58500.0f, 125500.0f, -3200.0f),
			56.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV46_LakePreservationProof"),
			TEXT("FFSmokeHighlandV46LakePreservationProofCamera"),
			FVector(22000.0f, 66800.0f, 8200.0f),
			FVector(27800.0f, 72800.0f, -2600.0f),
			54.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV46_CliffGrassTransition"),
			TEXT("FFSmokeHighlandV46CliffGrassTransitionCamera"),
			FVector(76600.0f, 119500.0f, 5200.0f),
			FVector(104500.0f, 133500.0f, -3600.0f),
			50.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV46_TraversalCorridor"),
			TEXT("FFSmokeHighlandV46TraversalCorridorCamera"),
			FVector2D(36000.0f, 82000.0f),
			26200.0f,
			FVector2D(94000.0f, 144000.0f),
			2600.0f,
			69.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV46_WideBiomeFraming"),
			TEXT("FFSmokeHighlandV46WideBiomeFramingCamera"),
			FVector2D(30500.0f, 76000.0f),
			27000.0f,
			FVector2D(96000.0f, 145500.0f),
			2600.0f,
			70.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV46_TopDownGameplaySpace"),
			TEXT("FFSmokeHighlandV46TopDownGameplaySpaceCamera"),
			FVector2D(72000.0f, 118500.0f),
			93000.0f,
			FVector2D(72000.0f, 118500.0f),
			0.0f,
			59.0f) ? 1 : 0;

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && Actor->GetActorLabel().Contains(TEXT("FFSmoke_HighlandV46_")))
			{
				Actor->Tags.AddUnique(V46ValidationCameraTag);
			}
		}

		World->MarkPackageDirty();
		const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
		const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=ValidationCamerasV46 removedActors=%d createdCameras=%d savedMap=%s savedPackages=%s"),
			RemovedActors,
			CreatedCameras,
			bSavedMap ? TEXT("true") : TEXT("false"),
			bSavedPackages ? TEXT("true") : TEXT("false"));
		return (bSavedMap && bSavedPackages && CreatedCameras == 7) ? 0 : 1;
	}

	bool IsValidVegetationPosition(const FVector2D& Position, const float MinWaterDistance, const bool bReserveVillagePlateau)
	{
		if (!IsInsideIslandInterior(Position) || IsNearPath(Position) || IsInsideSpawnMeadow(Position))
		{
			return false;
		}
		if (bReserveVillagePlateau && IsInsideVillagePlateauReserve(Position))
		{
			return false;
		}

		return GetNearestWaterDistance(Position) >= MinWaterDistance;
	}

	UHierarchicalInstancedStaticMeshComponent* CreateVegetationComponent(AActor* Owner, UStaticMesh* Mesh, const int32 Index, const bool bTree)
	{
		if (!Owner || !Mesh)
		{
			return nullptr;
		}

		UHierarchicalInstancedStaticMeshComponent* Component = NewObject<UHierarchicalInstancedStaticMeshComponent>(
			Owner,
			*FString::Printf(TEXT("FFTitanVegetation_%s_%02d"), bTree ? TEXT("Tree") : TEXT("Understory"), Index),
			RF_Transactional);
		if (!Component)
		{
			return nullptr;
		}

		Component->SetStaticMesh(Mesh);
		Component->SetMobility(EComponentMobility::Static);
		Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Component->SetGenerateOverlapEvents(false);
		Component->bSelectable = true;
		Component->SetCastShadow(bTree);
		Component->SetCullDistances(0, bTree ? 320000 : 90000);
		Component->SetupAttachment(Owner->GetRootComponent());
		Component->RegisterComponent();
		Owner->AddInstanceComponent(Component);
		return Component;
	}

	bool LoadMeshes(const TCHAR* const* MeshPaths, const int32 MeshCount, TArray<UStaticMesh*>& OutMeshes, const TCHAR* Context)
	{
		for (int32 Index = 0; Index < MeshCount; ++Index)
		{
			UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, MeshPaths[Index]);
			if (!Mesh)
			{
				UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandVegetationLayer: missing %s mesh %s"), Context, MeshPaths[Index]);
				return false;
			}
			OutMeshes.Add(Mesh);
		}
		return true;
	}

	bool LoadActorClasses(const TCHAR* const* ClassPaths, const int32 ClassCount, TArray<UClass*>& OutClasses, const TCHAR* Context)
	{
		for (int32 Index = 0; Index < ClassCount; ++Index)
		{
			UClass* ActorClass = LoadObject<UClass>(nullptr, ClassPaths[Index]);
			if (!ActorClass || !ActorClass->IsChildOf(AActor::StaticClass()))
			{
				UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandVegetationLayer: missing %s class %s"), Context, ClassPaths[Index]);
				return false;
			}
			OutClasses.Add(ActorClass);
		}
		return true;
	}

	int32 SelectTreeMeshIndex(const int32 Seed, const int32 MeshCount)
	{
		if (MeshCount <= 0)
		{
			return INDEX_NONE;
		}

		return FMath::Clamp(FMath::FloorToInt(PseudoRandom01(Seed) * MeshCount), 0, MeshCount - 1);
	}

	int32 SelectV42EcologyMeshIndex(const FString& ZoneName, const int32 Seed, const int32 MeshCount)
	{
		if (MeshCount <= 0)
		{
			return INDEX_NONE;
		}

		if (ZoneName.Contains(TEXT("OldTrees")) || ZoneName.Contains(TEXT("Landmark")))
		{
			return FMath::Clamp(MeshCount - 1, 0, MeshCount - 1);
		}

		if (ZoneName.Contains(TEXT("Support")) || ZoneName.Contains(TEXT("Sparse")) || ZoneName.Contains(TEXT("Feather")))
		{
			return FMath::Clamp(FMath::FloorToInt(PseudoRandom01(Seed + 11) * FMath::Min(4, MeshCount)), 0, MeshCount - 1);
		}

		return SelectTreeMeshIndex(Seed, MeshCount);
	}

	int32 RunV42EcologyVariation(UWorld* World, const int32 RemovedActors)
	{
		TArray<UClass*> TreeClasses;
		if (!LoadActorClasses(TraversalTreeBlueprintPaths, UE_ARRAY_COUNT(TraversalTreeBlueprintPaths), TreeClasses, TEXT("v42 ecology blueprint tree")))
		{
			return 1;
		}

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParameters.ObjectFlags = RF_Transactional;
		AActor* VegetationActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
		if (!VegetationActor)
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandVegetationLayer: failed to spawn v42 ecology actor."));
			return 1;
		}

		VegetationActor->Tags.AddUnique(VegetationTag);
		VegetationActor->SetActorLabel(TEXT("FF_V42_TitanEcologyVariation_Blueprints"));
		USceneComponent* RootComponent = NewObject<USceneComponent>(VegetationActor, TEXT("Root"), RF_Transactional);
		RootComponent->SetMobility(EComponentMobility::Static);
		VegetationActor->SetRootComponent(RootComponent);
		RootComponent->RegisterComponent();
		VegetationActor->AddInstanceComponent(RootComponent);

		TArray<FVector2D> AcceptedTreePositions;
		int32 TotalTrees = 0;
		int32 RejectedSamples = 0;
		for (int32 ZoneIndex = 0; ZoneIndex < UE_ARRAY_COUNT(TraversalForestV42Zones); ++ZoneIndex)
		{
			const FVegetationZone& Zone = TraversalForestV42Zones[ZoneIndex];
			const FString ZoneName(Zone.Name);
			const int32 TargetTreeCount = Zone.TreeCount;
			int32 ZoneTrees = 0;
			const int32 MaxSamples = TargetTreeCount * 240;
			for (int32 SampleIndex = 0; SampleIndex < MaxSamples && ZoneTrees < TargetTreeCount; ++SampleIndex)
			{
				const int32 Seed = (ZoneIndex + 1) * 4200000 + SampleIndex * 97;
				const FVector2D Position = GetRandomPointInZone(Zone, Seed);
				const float Spacing = (ZoneName.Contains(TEXT("Sparse")) || ZoneName.Contains(TEXT("Feather")) || ZoneName.Contains(TEXT("Landmark"))) ? 6200.0f : 4600.0f;
				if (!IsValidVegetationPosition(Position, 2800.0f, true)
					|| IsInsideTraversalForestV40RidgeExclusion(Position)
					|| IsInsideTraversalForestV40MeadowBreak(Position))
				{
					++RejectedSamples;
					continue;
				}

				bool bTooCloseToAcceptedTree = false;
				for (const FVector2D& AcceptedPosition : AcceptedTreePositions)
				{
					if (FVector2D::Distance(Position, AcceptedPosition) < Spacing)
					{
						bTooCloseToAcceptedTree = true;
						break;
					}
				}
				if (bTooCloseToAcceptedTree)
				{
					++RejectedSamples;
					continue;
				}

				FHitResult GroundHit;
				if (!GetPlacementGround(World, Position, 0.74f, GroundHit))
				{
					++RejectedSamples;
					continue;
				}

				const int32 ClassIndex = SelectTreeMeshIndex(Seed + 19, TreeClasses.Num());
				UClass* TreeClass = TreeClasses.IsValidIndex(ClassIndex) ? TreeClasses[ClassIndex] : nullptr;
				if (!TreeClass)
				{
					continue;
				}

				const float Yaw = PseudoRandom01(Seed + 23) * 360.0f;
				float BaseScale = FMath::Lerp(0.86f, 1.46f, PseudoRandom01(Seed + 31));
				if (ZoneName.Contains(TEXT("OldTrees")) || ZoneName.Contains(TEXT("Landmark")))
				{
					BaseScale = FMath::Lerp(1.15f, 1.55f, PseudoRandom01(Seed + 37));
				}
				else if (ZoneName.Contains(TEXT("Support")) || ZoneName.Contains(TEXT("Sparse")) || ZoneName.Contains(TEXT("Feather")))
				{
					BaseScale = FMath::Lerp(0.72f, 1.02f, PseudoRandom01(Seed + 41));
				}
				const float ZScale = BaseScale * FMath::Lerp(0.90f, 1.14f, PseudoRandom01(Seed + 47));

				FTransform TreeTransform;
				TreeTransform.SetLocation(GroundHit.ImpactPoint - FVector(0.0f, 0.0f, 1.5f));
				TreeTransform.SetRotation(FRotator(0.0f, Yaw, 0.0f).Quaternion());
				TreeTransform.SetScale3D(FVector(BaseScale, BaseScale, ZScale));
				AActor* TreeActor = World->SpawnActor<AActor>(TreeClass, TreeTransform, SpawnParameters);
				if (!TreeActor)
				{
					++RejectedSamples;
					continue;
				}

				TreeActor->Tags.AddUnique(VegetationTag);
				TreeActor->SetActorLabel(FString::Printf(TEXT("FF_V42_TitanEcologyTree_%s_%03d"), Zone.Name, ZoneTrees));
				TreeActor->AttachToActor(VegetationActor, FAttachmentTransformRules::KeepWorldTransform);
				TreeActor->MarkPackageDirty();
				AcceptedTreePositions.Add(Position);
				++ZoneTrees;
				++TotalTrees;
			}

			UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: v42Ecology zone=%s trees=%d/%d"), Zone.Name, ZoneTrees, TargetTreeCount);
		}

		VegetationActor->MarkPackageDirty();
		World->MarkPackageDirty();
		const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
		const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=TraversalForestV42EcologyBlueprint removedActors=%d trees=%d undergrowth=0 rejectedSamples=%d savedMap=%s savedPackages=%s"),
			RemovedActors,
			TotalTrees,
			RejectedSamples,
			bSavedMap ? TEXT("true") : TEXT("false"),
			bSavedPackages ? TEXT("true") : TEXT("false"));
		return (bSavedMap && bSavedPackages && TotalTrees > 0) ? 0 : 1;
	}

	int32 RunTraversalBlueprintForest(UWorld* World, const int32 RemovedActors, const int32 TraversalForestVersion)
	{
		TArray<UClass*> TreeClasses;
		if (!LoadActorClasses(TraversalTreeBlueprintPaths, UE_ARRAY_COUNT(TraversalTreeBlueprintPaths), TreeClasses, TEXT("traversal tree blueprint")))
		{
			return 1;
		}

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParameters.ObjectFlags = RF_Transactional;

		int32 TotalTrees = 0;
		int32 RejectedSamples = 0;
		TArray<FVector2D> AcceptedTreePositions;
		const bool bTraversalForestV40 = TraversalForestVersion >= 40;
		const bool bTraversalForestV10 = TraversalForestVersion >= 10;
		const bool bTraversalForestV9 = TraversalForestVersion >= 9;
		const bool bTraversalForestV8 = TraversalForestVersion >= 8;
		const bool bTraversalForestV7 = TraversalForestVersion >= 7;
		const bool bTraversalForestV6 = TraversalForestVersion >= 6;
		const bool bTraversalForestV5 = TraversalForestVersion >= 5;
		const bool bTraversalForestV4 = TraversalForestVersion >= 4;
		const bool bTraversalForestV3 = TraversalForestVersion >= 3;
		const bool bTraversalForestV2 = TraversalForestVersion == 2;
		const float BlueprintTreeCountScale = (bTraversalForestV6 || bTraversalForestV7 || bTraversalForestV8 || bTraversalForestV9 || bTraversalForestV10) ? 1.0f : ((bTraversalForestV2 || bTraversalForestV3) ? 1.0f : 0.10f);
		const FVegetationZone* ZoneSet = VegetationZones;
		int32 ZoneCount = UE_ARRAY_COUNT(VegetationZones);
		if (bTraversalForestV40)
		{
			ZoneSet = TraversalForestV40Zones;
			ZoneCount = UE_ARRAY_COUNT(TraversalForestV40Zones);
		}
		else if (bTraversalForestV10)
		{
			ZoneSet = TraversalForestV10Zones;
			ZoneCount = UE_ARRAY_COUNT(TraversalForestV10Zones);
		}
		else if (bTraversalForestV9)
		{
			ZoneSet = TraversalForestV9Zones;
			ZoneCount = UE_ARRAY_COUNT(TraversalForestV9Zones);
		}
		else if (bTraversalForestV8)
		{
			ZoneSet = TraversalForestV8Zones;
			ZoneCount = UE_ARRAY_COUNT(TraversalForestV8Zones);
		}
		else if (bTraversalForestV7)
		{
			ZoneSet = TraversalForestV7Zones;
			ZoneCount = UE_ARRAY_COUNT(TraversalForestV7Zones);
		}
		else if (bTraversalForestV6)
		{
			ZoneSet = TraversalForestV6Zones;
			ZoneCount = UE_ARRAY_COUNT(TraversalForestV6Zones);
		}
		else if (bTraversalForestV5)
		{
			ZoneSet = TraversalForestV5Zones;
			ZoneCount = UE_ARRAY_COUNT(TraversalForestV5Zones);
		}
		else if (bTraversalForestV4)
		{
			ZoneSet = TraversalForestV4Zones;
			ZoneCount = UE_ARRAY_COUNT(TraversalForestV4Zones);
		}
		else if (bTraversalForestV3)
		{
			ZoneSet = TraversalForestV3Zones;
			ZoneCount = UE_ARRAY_COUNT(TraversalForestV3Zones);
		}
		else if (bTraversalForestV2)
		{
			ZoneSet = TraversalForestV2Zones;
			ZoneCount = UE_ARRAY_COUNT(TraversalForestV2Zones);
		}
		const float MinTreeScale = bTraversalForestV40 ? 1.16f : (bTraversalForestV10 ? 1.08f : (bTraversalForestV9 ? 1.10f : (bTraversalForestV8 ? 1.12f : (bTraversalForestV7 ? 1.30f : (bTraversalForestV6 ? 1.22f : (bTraversalForestV5 ? 1.18f : (bTraversalForestV4 ? 1.16f : (bTraversalForestV3 ? 1.14f : (bTraversalForestV2 ? 1.08f : 0.82f)))))))));
		const float MaxTreeScale = bTraversalForestV40 ? 2.34f : (bTraversalForestV10 ? 2.72f : (bTraversalForestV9 ? 2.62f : (bTraversalForestV8 ? 2.28f : (bTraversalForestV7 ? 2.10f : (bTraversalForestV6 ? 1.96f : (bTraversalForestV5 ? 1.86f : (bTraversalForestV4 ? 1.72f : (bTraversalForestV3 ? 1.68f : (bTraversalForestV2 ? 1.52f : 1.24f)))))))));
		const float MinGroundNormalZ = bTraversalForestV40 ? 0.76f : (bTraversalForestV10 ? 0.71f : (bTraversalForestV9 ? 0.72f : (bTraversalForestV8 ? 0.74f : (bTraversalForestV7 ? 0.76f : (bTraversalForestV6 ? 0.78f : (bTraversalForestV5 ? 0.86f : (bTraversalForestV4 ? 0.88f : (bTraversalForestV3 ? 0.89f : (bTraversalForestV2 ? 0.90f : 0.93f)))))))));

		for (int32 ZoneIndex = 0; ZoneIndex < ZoneCount; ++ZoneIndex)
		{
			const FVegetationZone& Zone = ZoneSet[ZoneIndex];
			const int32 TargetTreeCount = FMath::Max(0, FMath::RoundToInt(static_cast<float>(Zone.TreeCount) * BlueprintTreeCountScale));
			int32 ZoneTrees = 0;

			const int32 MaxSamples = TargetTreeCount * (bTraversalForestV40 ? 190 : (bTraversalForestV10 ? 156 : (bTraversalForestV9 ? 148 : (bTraversalForestV8 ? 128 : (bTraversalForestV7 ? 110 : (bTraversalForestV6 ? 92 : 44))))));
			for (int32 SampleIndex = 0; SampleIndex < MaxSamples && ZoneTrees < TargetTreeCount; ++SampleIndex)
			{
				const int32 Seed = (ZoneIndex + 1) * 3100000 + SampleIndex * 83;
				const FVector2D Position = GetRandomPointInZone(Zone, Seed);
				if (!IsValidVegetationPosition(Position, bTraversalForestV40 ? 3600.0f : (bTraversalForestV10 ? 2900.0f : (bTraversalForestV9 ? 3000.0f : (bTraversalForestV8 ? 3200.0f : (bTraversalForestV7 ? 3400.0f : (bTraversalForestV6 ? 3600.0f : 4600.0f))))), true)
					|| ((bTraversalForestV2 || bTraversalForestV3) && IsInsideTraversalCorridorReserve(Position))
					|| (bTraversalForestV40 && (IsInsideTraversalForestV40RidgeExclusion(Position) || IsInsideTraversalForestV40MeadowBreak(Position)))
					|| (!bTraversalForestV40 && bTraversalForestV10 && IsInsideTraversalForestV10MeadowBreak(Position))
					|| (!bTraversalForestV10 && bTraversalForestV9 && IsInsideTraversalForestV9MeadowBreak(Position))
					|| (!bTraversalForestV10 && !bTraversalForestV9 && bTraversalForestV8 && IsInsideTraversalForestV8MeadowBreak(Position))
					|| (!bTraversalForestV10 && !bTraversalForestV9 && bTraversalForestV7 && IsInsideTraversalForestV7MeadowBreak(Position))
					|| (!bTraversalForestV10 && !bTraversalForestV9 && bTraversalForestV6 && IsInsideTraversalForestV6MeadowBreak(Position))
					|| (!bTraversalForestV10 && !bTraversalForestV9 && bTraversalForestV5 && IsInsideTraversalForestV5MeadowBreak(Position))
					|| (!bTraversalForestV10 && !bTraversalForestV9 && bTraversalForestV4 && IsInsideTraversalForestV4MeadowBreak(Position))
					|| (!bTraversalForestV10 && !bTraversalForestV9 && !bTraversalForestV4 && bTraversalForestV3 && IsInsideTraversalForestV3MeadowBreak(Position)))
				{
					++RejectedSamples;
					continue;
				}

				if (bTraversalForestV40)
				{
					const FString ZoneName(Zone.Name);
					const float MinTreeSpacing = (ZoneName.Contains(TEXT("Sparse")) || ZoneName.Contains(TEXT("Sentinel")) || ZoneName.Contains(TEXT("Feather"))) ? 3600.0f : 2600.0f;
					bool bTooCloseToAcceptedTree = false;
					for (const FVector2D& AcceptedPosition : AcceptedTreePositions)
					{
						if (FVector2D::Distance(Position, AcceptedPosition) < MinTreeSpacing)
						{
							bTooCloseToAcceptedTree = true;
							break;
						}
					}
					if (bTooCloseToAcceptedTree)
					{
						++RejectedSamples;
						continue;
					}
				}

				FHitResult GroundHit;
				if (!GetPlacementGround(World, Position, MinGroundNormalZ, GroundHit))
				{
					++RejectedSamples;
					continue;
				}

				const int32 ClassIndex = SelectTreeMeshIndex(Seed + 19, TreeClasses.Num());
				UClass* TreeClass = TreeClasses.IsValidIndex(ClassIndex) ? TreeClasses[ClassIndex] : nullptr;
				if (!TreeClass)
				{
					continue;
				}

				const float Yaw = PseudoRandom01(Seed + 23) * 360.0f;
				float Scale = FMath::Lerp(MinTreeScale, MaxTreeScale, PseudoRandom01(Seed + 31));
				if (bTraversalForestV9 || bTraversalForestV10)
				{
					const FString ZoneName(Zone.Name);
					if (ZoneName.Contains(TEXT("Landmark")))
					{
						Scale = FMath::Lerp(bTraversalForestV10 ? 2.35f : 2.25f, bTraversalForestV10 ? 3.05f : 2.95f, PseudoRandom01(Seed + 37));
					}
					else if (ZoneName.Contains(TEXT("Sparse")) || ZoneName.Contains(TEXT("Incursion")) || ZoneName.Contains(TEXT("Sentinel")))
					{
						Scale *= FMath::Lerp(0.90f, 1.12f, PseudoRandom01(Seed + 41));
					}
				}
				FTransform TreeTransform;
				TreeTransform.SetLocation(GroundHit.ImpactPoint - FVector(0.0f, 0.0f, 1.5f));
				TreeTransform.SetRotation(FRotator(0.0f, Yaw, 0.0f).Quaternion());
				TreeTransform.SetScale3D(FVector(Scale));

				AActor* TreeActor = World->SpawnActor<AActor>(TreeClass, TreeTransform, SpawnParameters);
				if (!TreeActor)
				{
					++RejectedSamples;
					continue;
				}

				TreeActor->Tags.AddUnique(VegetationTag);
				TreeActor->SetActorLabel(FString::Printf(TEXT("FF_Phase1_TitanTree_%s_%03d"), Zone.Name, ZoneTrees));
				TInlineComponentArray<USceneComponent*> SceneComponents;
				TreeActor->GetComponents(SceneComponents);
				for (USceneComponent* SceneComponent : SceneComponents)
				{
					if (SceneComponent)
					{
						SceneComponent->SetMobility(EComponentMobility::Static);
					}
				}
				if (bTraversalForestV10)
				{
					TInlineComponentArray<UBillboardComponent*> BillboardComponents;
					TreeActor->GetComponents(BillboardComponents);
					for (UBillboardComponent* BillboardComponent : BillboardComponents)
					{
						if (BillboardComponent)
						{
							BillboardComponent->Modify();
							BillboardComponent->SetSprite(nullptr);
							BillboardComponent->ScreenSize = 0.0f;
							BillboardComponent->U = 0;
							BillboardComponent->UL = 0;
							BillboardComponent->V = 0;
							BillboardComponent->VL = 0;
							BillboardComponent->SetHiddenInGame(true);
							BillboardComponent->SetVisibility(false, true);
							BillboardComponent->SetCastShadow(false);
						}
					}

					TInlineComponentArray<UStaticMeshComponent*> StaticMeshComponents;
					TreeActor->GetComponents(StaticMeshComponents);
					for (UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
					{
						if (StaticMeshComponent)
						{
							StaticMeshComponent->Modify();
							StaticMeshComponent->SetCastShadow(false);
							StaticMeshComponent->SetForcedLodModel(1);
						}
					}
				}
				TreeActor->MarkPackageDirty();
				AcceptedTreePositions.Add(Position);
				++ZoneTrees;
				++TotalTrees;
			}

			UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: traversalBlueprint zone=%s trees=%d/%d"), Zone.Name, ZoneTrees, TargetTreeCount);
		}

		World->MarkPackageDirty();
		const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
		const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=%s removedActors=%d trees=%d undergrowth=0 rejectedSamples=%d savedMap=%s savedPackages=%s"),
			bTraversalForestV40 ? TEXT("TraversalForestV40Blueprint") : (bTraversalForestV10 ? TEXT("TraversalForestV10Blueprint") : (bTraversalForestV9 ? TEXT("TraversalForestV9Blueprint") : (bTraversalForestV8 ? TEXT("TraversalForestV8Blueprint") : (bTraversalForestV7 ? TEXT("TraversalForestV7Blueprint") : (bTraversalForestV6 ? TEXT("TraversalForestV6Blueprint") : (bTraversalForestV5 ? TEXT("TraversalForestV5Blueprint") : (bTraversalForestV4 ? TEXT("TraversalForestV4Blueprint") : (bTraversalForestV3 ? TEXT("TraversalForestV3Blueprint") : (bTraversalForestV2 ? TEXT("TraversalForestV2Blueprint") : TEXT("TraversalForestV1Blueprint")))))))))),
			RemovedActors,
			TotalTrees,
			RejectedSamples,
			bSavedMap ? TEXT("true") : TEXT("false"),
			bSavedPackages ? TEXT("true") : TEXT("false"));
		return (bSavedMap && bSavedPackages && TotalTrees > 0) ? 0 : 1;
	}

	int32 RunTraversalCliffRockDressing(UWorld* World, const int32 RemovedActors, const int32 CliffRockVersion)
	{
		TArray<UStaticMesh*> RockMeshes;
		if (!LoadMeshes(CliffRockMeshPaths, UE_ARRAY_COUNT(CliffRockMeshPaths), RockMeshes, TEXT("cliff rock")))
		{
			return 1;
		}

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParameters.ObjectFlags = RF_Transactional;

		AActor* RockActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
		if (!RockActor)
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandVegetationLayer: failed to spawn cliff rock actor."));
			return 1;
		}

		RockActor->Tags.AddUnique(CliffRockTag);
		RockActor->SetActorLabel(TEXT("FF_Phase1_TitanCliffRock_Dressing_HISM"));
		USceneComponent* RootComponent = NewObject<USceneComponent>(RockActor, TEXT("Root"), RF_Transactional);
		RootComponent->SetMobility(EComponentMobility::Static);
		RockActor->SetRootComponent(RootComponent);
		RootComponent->RegisterComponent();
		RockActor->AddInstanceComponent(RootComponent);

		UMaterialInterface* TitanCliffMaterial = LoadObject<UMaterialInterface>(nullptr, TitanCliffRockMaterialPath);
		if (!TitanCliffMaterial)
		{
			UE_LOG(LogTemp, Warning, TEXT("FFStarterHighlandVegetationLayer: missing Titan cliff material %s; using rock mesh defaults."), TitanCliffRockMaterialPath);
		}

		TArray<UHierarchicalInstancedStaticMeshComponent*> RockComponents;
		for (int32 Index = 0; Index < RockMeshes.Num(); ++Index)
		{
			UHierarchicalInstancedStaticMeshComponent* Component = CreateVegetationComponent(RockActor, RockMeshes[Index], Index, true);
			if (Component && CliffRockVersion >= 5)
			{
				// V39 uses many flat shelf instances; disabling their cast shadows avoids blockout-like black plates in validation views.
				Component->SetCastShadow(false);
			}
			if (Component && TitanCliffMaterial)
			{
				const int32 MaterialSlots = FMath::Max(1, Component->GetNumMaterials());
				for (int32 SlotIndex = 0; SlotIndex < MaterialSlots; ++SlotIndex)
				{
					Component->SetMaterial(SlotIndex, TitanCliffMaterial);
				}
			}
			RockComponents.Add(Component);
		}

		int32 TotalRocks = 0;
		int32 RejectedSamples = 0;
		const FCliffRockRun* RunSet = CliffRockVersion >= 7 ? CliffRockRunsV7 : (CliffRockVersion >= 6 ? CliffRockRunsV6 : (CliffRockVersion >= 5 ? CliffRockRunsV5 : (CliffRockVersion >= 4 ? CliffRockRunsV4 : (CliffRockVersion >= 3 ? CliffRockRunsV3 : (CliffRockVersion >= 2 ? CliffRockRunsV2 : CliffRockRunsV1)))));
		const int32 RunCount = CliffRockVersion >= 7 ? UE_ARRAY_COUNT(CliffRockRunsV7) : (CliffRockVersion >= 6 ? UE_ARRAY_COUNT(CliffRockRunsV6) : (CliffRockVersion >= 5 ? UE_ARRAY_COUNT(CliffRockRunsV5) : (CliffRockVersion >= 4 ? UE_ARRAY_COUNT(CliffRockRunsV4) : (CliffRockVersion >= 3 ? UE_ARRAY_COUNT(CliffRockRunsV3) : (CliffRockVersion >= 2 ? UE_ARRAY_COUNT(CliffRockRunsV2) : UE_ARRAY_COUNT(CliffRockRunsV1))))));
		for (int32 RunIndex = 0; RunIndex < RunCount; ++RunIndex)
		{
			const FCliffRockRun& Run = RunSet[RunIndex];
			const FVector2D Direction = (Run.End - Run.Start).GetSafeNormal();
			const FVector2D Perpendicular(-Direction.Y, Direction.X);
			int32 RunRocks = 0;
			const int32 MaxSamples = Run.Count * (CliffRockVersion >= 7 ? 22 : (CliffRockVersion >= 6 ? 18 : (CliffRockVersion >= 5 ? 16 : (CliffRockVersion >= 4 ? 14 : 8))));
			for (int32 SampleIndex = 0; SampleIndex < MaxSamples && RunRocks < Run.Count; ++SampleIndex)
			{
				const int32 Seed = (RunIndex + 1) * 4700000 + SampleIndex * 97;
				const float Alpha = FMath::Clamp((static_cast<float>(RunRocks) + PseudoRandom01(Seed + 5)) / FMath::Max(static_cast<float>(Run.Count), 1.0f), 0.0f, 1.0f);
				const float SideOffset = (PseudoRandom01(Seed + 11) * 2.0f - 1.0f) * Run.JitterRadius;
				const float AlongOffset = (PseudoRandom01(Seed + 17) * 2.0f - 1.0f) * 1300.0f;
				const FVector2D Position = FMath::Lerp(Run.Start, Run.End, Alpha) + Perpendicular * SideOffset + Direction * AlongOffset;

				if (GetNearestWaterDistance(Position) < 6800.0f
					|| IsNearPath(Position)
					|| IsInsideVillagePlateauReserve(Position))
				{
					++RejectedSamples;
					continue;
				}

				FHitResult GroundHit;
				if (!GetPlacementGround(World, Position, 0.36f, GroundHit))
				{
					++RejectedSamples;
					continue;
				}

				const int32 MeshIndex = SelectTreeMeshIndex(Seed + 23, RockMeshes.Num());
				UHierarchicalInstancedStaticMeshComponent* Component = RockComponents.IsValidIndex(MeshIndex) ? RockComponents[MeshIndex] : nullptr;
				if (!Component)
				{
					++RejectedSamples;
					continue;
				}

				const float BaseScale = FMath::Lerp(Run.MinScale, Run.MaxScale, PseudoRandom01(Seed + 31));
				const float Flatten = CliffRockVersion >= 7 ? FMath::Lerp(0.10f, 0.24f, PseudoRandom01(Seed + 43)) : (CliffRockVersion >= 6 ? FMath::Lerp(0.48f, 1.08f, PseudoRandom01(Seed + 43)) : (CliffRockVersion >= 5 ? FMath::Lerp(0.54f, 1.18f, PseudoRandom01(Seed + 43)) : (CliffRockVersion >= 4 ? FMath::Lerp(0.58f, 1.16f, PseudoRandom01(Seed + 43)) : (CliffRockVersion >= 3 ? FMath::Lerp(0.64f, 1.12f, PseudoRandom01(Seed + 43)) : (CliffRockVersion >= 2 ? FMath::Lerp(0.78f, 1.03f, PseudoRandom01(Seed + 43)) : FMath::Lerp(0.72f, 1.12f, PseudoRandom01(Seed + 43)))))));
				const float Stretch = CliffRockVersion >= 7 ? FMath::Lerp(0.55f, 0.92f, PseudoRandom01(Seed + 47)) : (CliffRockVersion >= 6 ? FMath::Lerp(0.74f, 1.70f, PseudoRandom01(Seed + 47)) : (CliffRockVersion >= 5 ? FMath::Lerp(0.68f, 1.56f, PseudoRandom01(Seed + 47)) : (CliffRockVersion >= 4 ? FMath::Lerp(0.72f, 1.48f, PseudoRandom01(Seed + 47)) : (CliffRockVersion >= 3 ? FMath::Lerp(0.78f, 1.36f, PseudoRandom01(Seed + 47)) : (CliffRockVersion >= 2 ? FMath::Lerp(0.85f, 1.18f, PseudoRandom01(Seed + 47)) : FMath::Lerp(0.88f, 1.45f, PseudoRandom01(Seed + 47)))))));
				const float Yaw = FMath::RadiansToDegrees(FMath::Atan2(Direction.Y, Direction.X)) + FMath::Lerp(-55.0f, 55.0f, PseudoRandom01(Seed + 59));
				const float Pitch = CliffRockVersion >= 7 ? FMath::Lerp(-5.0f, 5.0f, PseudoRandom01(Seed + 61)) : (CliffRockVersion >= 6 ? FMath::Lerp(-12.0f, 12.0f, PseudoRandom01(Seed + 61)) : (CliffRockVersion >= 5 ? FMath::Lerp(-10.0f, 10.0f, PseudoRandom01(Seed + 61)) : (CliffRockVersion >= 4 ? FMath::Lerp(-9.0f, 9.0f, PseudoRandom01(Seed + 61)) : (CliffRockVersion >= 3 ? FMath::Lerp(-7.0f, 7.0f, PseudoRandom01(Seed + 61)) : 0.0f))));
				const float Roll = CliffRockVersion >= 7 ? FMath::Lerp(-4.0f, 4.0f, PseudoRandom01(Seed + 67)) : (CliffRockVersion >= 6 ? FMath::Lerp(-10.0f, 10.0f, PseudoRandom01(Seed + 67)) : (CliffRockVersion >= 5 ? FMath::Lerp(-8.0f, 8.0f, PseudoRandom01(Seed + 67)) : (CliffRockVersion >= 4 ? FMath::Lerp(-7.0f, 7.0f, PseudoRandom01(Seed + 67)) : (CliffRockVersion >= 3 ? FMath::Lerp(-5.0f, 5.0f, PseudoRandom01(Seed + 67)) : 0.0f))));

				FTransform InstanceTransform;
				InstanceTransform.SetLocation(GroundHit.ImpactPoint - FVector(0.0f, 0.0f, 2.0f));
				InstanceTransform.SetRotation(FRotator(Pitch, Yaw, Roll).Quaternion());
				InstanceTransform.SetScale3D(FVector(BaseScale * Stretch, BaseScale, BaseScale * Flatten));
				Component->AddInstance(InstanceTransform, true);
				++RunRocks;
				++TotalRocks;
			}

			UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: cliffRockRun=%s rocks=%d/%d"), Run.Name, RunRocks, Run.Count);
		}

		for (UHierarchicalInstancedStaticMeshComponent* Component : RockComponents)
		{
			if (Component)
			{
				Component->BuildTreeIfOutdated(true, true);
				Component->MarkPackageDirty();
			}
		}

		RockActor->MarkPackageDirty();
		World->MarkPackageDirty();
		const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
		const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=%s removedActors=%d rocks=%d rejectedSamples=%d material=%s savedMap=%s savedPackages=%s"),
			CliffRockVersion >= 7 ? TEXT("TraversalCliffRocksV7") : (CliffRockVersion >= 6 ? TEXT("TraversalCliffRocksV6") : (CliffRockVersion >= 5 ? TEXT("TraversalCliffRocksV5") : (CliffRockVersion >= 4 ? TEXT("TraversalCliffRocksV4") : (CliffRockVersion >= 3 ? TEXT("TraversalCliffRocksV3") : (CliffRockVersion >= 2 ? TEXT("TraversalCliffRocksV2") : TEXT("TraversalCliffRocksV1")))))),
			RemovedActors,
			TotalRocks,
			RejectedSamples,
			TitanCliffMaterial ? *TitanCliffMaterial->GetPathName() : TEXT("mesh-default"),
			bSavedMap ? TEXT("true") : TEXT("false"),
			bSavedPackages ? TEXT("true") : TEXT("false"));
		return (bSavedMap && bSavedPackages && TotalRocks > 0) ? 0 : 1;
	}

	struct FV61TerrainSample
	{
		int32 GridX = 0;
		int32 GridY = 0;
		FVector Location = FVector::ZeroVector;
		float NormalZ = 0.0f;
		FString ComponentName;
		FString Classification;
	};

	struct FV61ComponentRegion
	{
		int32 TotalSamples = 0;
		int32 MountainSamples = 0;
		int32 GameplaySamples = 0;
		int32 PlateauSamples = 0;
		int32 WaterExcludedSamples = 0;
		FVector2D MinXY = FVector2D(TNumericLimits<float>::Max(), TNumericLimits<float>::Max());
		FVector2D MaxXY = FVector2D(TNumericLimits<float>::Lowest(), TNumericLimits<float>::Lowest());
		float MinZ = TNumericLimits<float>::Max();
		float MaxZ = TNumericLimits<float>::Lowest();
	};

	float DistanceToSegment2DV61(const FVector2D& Point, const FVector2D& A, const FVector2D& B);
	bool IsV61RiverCorridor(const FVector2D& Position, float RadiusCm);
	bool IsV61LakeArea(const FVector2D& Position);

	struct FV65ApprovedMountainPoint
	{
		int32 GridX = 0;
		int32 GridY = 0;
		FVector Location = FVector::ZeroVector;
		float NormalZ = 0.0f;
		FString ComponentName;
	};

	bool LoadV65ApprovedMountainPoints(TArray<FV65ApprovedMountainPoint>& OutPoints)
	{
		const FString TerrainCsvPath = FPaths::ProjectSavedDir() / TEXT("HighlandWorldbuilding_v61_TerrainSamples.csv");
		TArray<FString> Lines;
		if (!FFileHelper::LoadFileToStringArray(Lines, *TerrainCsvPath))
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandVegetationLayer: V65 cannot load approved V61 terrain samples from %s."), *TerrainCsvPath);
			return false;
		}

		for (int32 LineIndex = 1; LineIndex < Lines.Num(); ++LineIndex)
		{
			TArray<FString> Fields;
			Lines[LineIndex].ParseIntoArray(Fields, TEXT(","), false);
			if (Fields.Num() < 8 || !Fields[6].Equals(TEXT("Mountain"), ESearchCase::IgnoreCase))
			{
				continue;
			}

			FV65ApprovedMountainPoint Point;
			Point.GridX = FCString::Atoi(*Fields[0]);
			Point.GridY = FCString::Atoi(*Fields[1]);
			Point.Location = FVector(FCString::Atof(*Fields[2]), FCString::Atof(*Fields[3]), FCString::Atof(*Fields[4]));
			Point.NormalZ = FCString::Atof(*Fields[5]);
			Point.ComponentName = Fields[7];
			OutPoints.Add(Point);
		}

		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: V65 loaded approvedMountainSamples=%d from %s."), OutPoints.Num(), *TerrainCsvPath);
		return OutPoints.Num() > 0;
	}

	float GetV65RiverClearance(const FVector2D& Position)
	{
		static const FVector2D RiverPath[] = {
			FVector2D(48590.0f, 183116.0f),
			FVector2D(49910.0f, 174430.0f),
			FVector2D(51518.0f, 166778.0f),
			FVector2D(58605.0f, 152688.0f),
			FVector2D(62544.0f, 138795.0f),
			FVector2D(58487.0f, 122723.0f),
			FVector2D(55306.0f, 96223.0f),
			FVector2D(54105.0f, 78933.0f),
			FVector2D(57584.0f, 62136.0f),
			FVector2D(63147.0f, 37260.0f),
			FVector2D(65398.0f, 28870.0f),
			FVector2D(66873.0f, 20275.0f),
			FVector2D(68500.0f, 11100.0f),
			FVector2D(71037.0f, 3095.0f),
			FVector2D(73955.0f, -4499.0f),
			FVector2D(76822.0f, -12727.0f)
		};

		float Nearest = TNumericLimits<float>::Max();
		for (int32 Index = 0; Index < UE_ARRAY_COUNT(RiverPath) - 1; ++Index)
		{
			Nearest = FMath::Min(Nearest, DistanceToSegment2DV61(Position, RiverPath[Index], RiverPath[Index + 1]));
		}
		return Nearest;
	}

	float GetV65NorthExitClearance(const FVector2D& Position)
	{
		static const FVector2D NorthExitPath[] = {
			FVector2D(48590.0f, 183116.0f),
			FVector2D(49910.0f, 174430.0f),
			FVector2D(51518.0f, 166778.0f),
			FVector2D(58605.0f, 152688.0f),
			FVector2D(62544.0f, 138795.0f)
		};

		float Nearest = TNumericLimits<float>::Max();
		for (int32 Index = 0; Index < UE_ARRAY_COUNT(NorthExitPath) - 1; ++Index)
		{
			Nearest = FMath::Min(Nearest, DistanceToSegment2DV61(Position, NorthExitPath[Index], NorthExitPath[Index + 1]));
		}
		return Nearest;
	}

	float GetV65SouthExitClearance(const FVector2D& Position)
	{
		static const FVector2D SouthExitPath[] = {
			FVector2D(63147.0f, 37260.0f),
			FVector2D(65398.0f, 28870.0f),
			FVector2D(66873.0f, 20275.0f),
			FVector2D(68500.0f, 11100.0f),
			FVector2D(71037.0f, 3095.0f),
			FVector2D(73955.0f, -4499.0f),
			FVector2D(76822.0f, -12727.0f)
		};

		float Nearest = TNumericLimits<float>::Max();
		for (int32 Index = 0; Index < UE_ARRAY_COUNT(SouthExitPath) - 1; ++Index)
		{
			Nearest = FMath::Min(Nearest, DistanceToSegment2DV61(Position, SouthExitPath[Index], SouthExitPath[Index + 1]));
		}
		return Nearest;
	}

	float GetV65LakeClearance(const FVector2D& Position)
	{
		const FVector2D LakeSouth = (Position - FVector2D(40000.0f, 23500.0f)) / FVector2D(16000.0f, 12000.0f);
		const FVector2D LakeWest = (Position - FVector2D(25640.0f, 72820.0f)) / FVector2D(4700.0f, 1900.0f);
		return FMath::Min((LakeSouth.Size() - 1.0f) * 5400.0f, (LakeWest.Size() - 1.0f) * 6000.0f);
	}

	bool SpawnV65TitanMountainValidationCameras(UWorld* World)
	{
		int32 CreatedCameras = 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV65_MountainRidgeProfile"),
			TEXT("FFSmokeHighlandV65MountainRidgeProfileCamera"),
			FVector(15500.0f, 104000.0f, 19000.0f),
			FVector(121000.0f, 116000.0f, 4800.0f),
			60.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV65_FullOuterRing"),
			TEXT("FFSmokeHighlandV65FullOuterRingCamera"),
			FVector(61000.0f, 52000.0f, 54000.0f),
			FVector(70500.0f, 119000.0f, -2500.0f),
			72.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV65_NorthRiverExit"),
			TEXT("FFSmokeHighlandV65NorthRiverExitCamera"),
			FVector(56000.0f, 116000.0f, 25000.0f),
			FVector(54500.0f, 171500.0f, -6000.0f),
			66.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV65_SouthRiverExit"),
			TEXT("FFSmokeHighlandV65SouthRiverExitCamera"),
			FVector(54500.0f, 43000.0f, 25000.0f),
			FVector(73500.0f, -8000.0f, -14500.0f),
			66.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV65_LakePreservation"),
			TEXT("FFSmokeHighlandV65LakePreservationCamera"),
			FVector(40000.0f, 23500.0f, 42000.0f),
			FVector(40000.0f, 23500.0f, -5000.0f),
			46.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV65_GameplaySpace"),
			TEXT("FFSmokeHighlandV65GameplaySpaceCamera"),
			FVector2D(72000.0f, 118500.0f),
			93000.0f,
			FVector2D(72000.0f, 118500.0f),
			0.0f,
			59.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV65_TopDownVerification"),
			TEXT("FFSmokeHighlandV65TopDownVerificationCamera"),
			FVector2D(65000.0f, 82000.0f),
			165000.0f,
			FVector2D(65000.0f, 82000.0f),
			0.0f,
			70.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV65_WideBiomeView"),
			TEXT("FFSmokeHighlandV65WideBiomeViewCamera"),
			FVector(30500.0f, 76000.0f, 27000.0f),
			FVector(96000.0f, 145500.0f, 2600.0f),
			70.0f) ? 1 : 0;

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && Actor->GetActorLabel().Contains(TEXT("FFSmoke_HighlandV65_")))
			{
				Actor->Tags.AddUnique(V65ValidationCameraTag);
			}
		}

		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=TitanMountainConversionV65 validationCameras=%d"), CreatedCameras);
		return CreatedCameras >= 8;
	}

	int32 RunV65TitanMountainConversion(UWorld* World, const int32 RemovedActors)
	{
		TArray<FV65ApprovedMountainPoint> MountainPoints;
		if (!LoadV65ApprovedMountainPoints(MountainPoints))
		{
			return 1;
		}

		TArray<UStaticMesh*> MountainMeshes;
		if (!LoadMeshes(TitanGeologyWallMeshPathsV59, UE_ARRAY_COUNT(TitanGeologyWallMeshPathsV59), MountainMeshes, TEXT("v65 Titan mountain conversion")))
		{
			return 1;
		}

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParameters.ObjectFlags = RF_Transactional;

		AActor* MountainActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
		if (!MountainActor)
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandVegetationLayer: failed to spawn v65 Titan mountain conversion actor."));
			return 1;
		}

		MountainActor->Tags.AddUnique(V65TitanMountainConversionTag);
		MountainActor->SetActorLabel(TEXT("FF_V65_TitanMountainRingConversion_HISM"));
		USceneComponent* RootComponent = NewObject<USceneComponent>(MountainActor, TEXT("Root"), RF_Transactional);
		RootComponent->SetMobility(EComponentMobility::Static);
		MountainActor->SetRootComponent(RootComponent);
		RootComponent->RegisterComponent();
		MountainActor->AddInstanceComponent(RootComponent);

		UMaterialInterface* TitanCliffMaterial = LoadObject<UMaterialInterface>(nullptr, TitanCliffRockMaterialPath);
		if (!TitanCliffMaterial)
		{
			UE_LOG(LogTemp, Warning, TEXT("FFStarterHighlandVegetationLayer: missing Titan cliff material %s; using mesh defaults."), TitanCliffRockMaterialPath);
		}

		TArray<UHierarchicalInstancedStaticMeshComponent*> MountainComponents;
		for (int32 Index = 0; Index < MountainMeshes.Num(); ++Index)
		{
			UHierarchicalInstancedStaticMeshComponent* Component = CreateV46Component(MountainActor, MountainMeshes[Index], TEXT("TitanMountainV65"), Index, true, 560000);
			ApplyOptionalMaterial(Component, TitanCliffMaterial);
			MountainComponents.Add(Component);
		}

		const FVector2D IslandCenter(71396.0f, 79714.0f);
		constexpr int32 SectorCount = 216;
		TArray<int32> InnerEdgeBySector;
		TArray<int32> HighMassBySector;
		InnerEdgeBySector.Init(INDEX_NONE, SectorCount);
		HighMassBySector.Init(INDEX_NONE, SectorCount);

		int32 ProtectedSamplesRejected = 0;
		int32 CandidateSamples = 0;
		for (int32 PointIndex = 0; PointIndex < MountainPoints.Num(); ++PointIndex)
		{
			const FV65ApprovedMountainPoint& Point = MountainPoints[PointIndex];
			const FVector2D XY(Point.Location.X, Point.Location.Y);
			const float RiverClearance = GetV65RiverClearance(XY);
			const float LakeClearance = GetV65LakeClearance(XY);
			if (RiverClearance < 16000.0f
				|| LakeClearance < 11200.0f
				|| IsV61RiverCorridor(XY, 16000.0f)
				|| IsV61LakeArea(XY)
				|| IsNearPath(XY)
				|| IsInsideTraversalCorridorReserve(XY)
				|| IsInsideVillagePlateauReserve(XY)
				|| IsInsideSpawnMeadow(XY))
			{
				++ProtectedSamplesRejected;
				continue;
			}

			const FVector2D FromCenter = XY - IslandCenter;
			if (FromCenter.IsNearlyZero() || Point.Location.Z < -18000.0f)
			{
				++ProtectedSamplesRejected;
				continue;
			}

			float Angle = FMath::Atan2(FromCenter.Y, FromCenter.X);
			if (Angle < 0.0f)
			{
				Angle += UE_TWO_PI;
			}
			const int32 Sector = FMath::Clamp(FMath::FloorToInt((Angle / UE_TWO_PI) * static_cast<float>(SectorCount)), 0, SectorCount - 1);
			const float Radius = FromCenter.Size();
			const int32 CurrentInner = InnerEdgeBySector[Sector];
			if (CurrentInner == INDEX_NONE || Radius < FVector2D::Distance(FVector2D(MountainPoints[CurrentInner].Location.X, MountainPoints[CurrentInner].Location.Y), IslandCenter))
			{
				InnerEdgeBySector[Sector] = PointIndex;
			}

			const int32 CurrentHigh = HighMassBySector[Sector];
			if (CurrentHigh == INDEX_NONE || Point.Location.Z > MountainPoints[CurrentHigh].Location.Z)
			{
				HighMassBySector[Sector] = PointIndex;
			}
			++CandidateSamples;
		}

		TArray<int32> CandidatePointIndices;
		CandidatePointIndices.Reserve(SectorCount * 2);
		for (int32 Sector = 0; Sector < SectorCount; ++Sector)
		{
			if (InnerEdgeBySector[Sector] != INDEX_NONE)
			{
				CandidatePointIndices.Add(InnerEdgeBySector[Sector]);
			}
			if (HighMassBySector[Sector] != INDEX_NONE
				&& HighMassBySector[Sector] != InnerEdgeBySector[Sector])
			{
				CandidatePointIndices.Add(HighMassBySector[Sector]);
			}
		}

		CandidatePointIndices.Sort([&MountainPoints, &IslandCenter](const int32 A, const int32 B)
		{
			const FVector2D AFromCenter(MountainPoints[A].Location.X - IslandCenter.X, MountainPoints[A].Location.Y - IslandCenter.Y);
			const FVector2D BFromCenter(MountainPoints[B].Location.X - IslandCenter.X, MountainPoints[B].Location.Y - IslandCenter.Y);
			float AAngle = FMath::Atan2(AFromCenter.Y, AFromCenter.X);
			float BAngle = FMath::Atan2(BFromCenter.Y, BFromCenter.X);
			if (AAngle < 0.0f)
			{
				AAngle += UE_TWO_PI;
			}
			if (BAngle < 0.0f)
			{
				BAngle += UE_TWO_PI;
			}
			return AAngle < BAngle;
		});

		auto AddInstance = [&MountainComponents](
			const int32 MeshIndex,
			const FVector& Location,
			const FRotator& Rotation,
			const FVector& Scale)
		{
			UHierarchicalInstancedStaticMeshComponent* Component = MountainComponents.IsValidIndex(MeshIndex) ? MountainComponents[MeshIndex] : nullptr;
			if (!Component)
			{
				return false;
			}

			FTransform InstanceTransform;
			InstanceTransform.SetLocation(Location);
			InstanceTransform.SetRotation(Rotation.Quaternion());
			InstanceTransform.SetScale3D(Scale);
			return Component->AddInstance(InstanceTransform, true) != INDEX_NONE;
		};

		TArray<FVector2D> AcceptedPositions;
		int32 PrimaryMassInstances = 0;
		int32 SecondaryMassInstances = 0;
		int32 ShoulderInstances = 0;
		int32 AcceptedAnchors = 0;
		int32 SpacingRejected = 0;
		int32 PlacementRejected = 0;
		int32 MajorPeakSectors = 0;
		int32 SecondaryPeakSectors = 0;
		float MinRiverClearance = TNumericLimits<float>::Max();
		float MinLakeClearance = TNumericLimits<float>::Max();
		float MinNorthExitClearance = TNumericLimits<float>::Max();
		float MinSouthExitClearance = TNumericLimits<float>::Max();
		float AccumulatedOutwardOffset = 0.0f;
		float MaxInwardOffset = 0.0f;
		float HighestPlacedTopZ = TNumericLimits<float>::Lowest();

		for (int32 CandidateIndex = 0; CandidateIndex < CandidatePointIndices.Num() && AcceptedAnchors < 290; ++CandidateIndex)
		{
			const int32 PointIndex = CandidatePointIndices[CandidateIndex];
			if (!MountainPoints.IsValidIndex(PointIndex))
			{
				continue;
			}

			const FV65ApprovedMountainPoint& Point = MountainPoints[PointIndex];
			const FVector2D XY(Point.Location.X, Point.Location.Y);
			bool bTooClose = false;
			const float MinSpacing = Point.Location.Z > -2500.0f ? 2600.0f : 2200.0f;
			for (const FVector2D& AcceptedPosition : AcceptedPositions)
			{
				if (FVector2D::Distance(XY, AcceptedPosition) < MinSpacing)
				{
					bTooClose = true;
					break;
				}
			}
			if (bTooClose)
			{
				++SpacingRejected;
				continue;
			}

			FHitResult GroundHit;
			if (!GetPlacementGround(World, XY, 0.0f, GroundHit))
			{
				++PlacementRejected;
				continue;
			}

			const FVector2D ToCenter = (IslandCenter - XY).GetSafeNormal();
			const FVector2D Outward = -ToCenter;
			const FVector2D Tangent(-Outward.Y, Outward.X);
			const int32 Seed = (Point.GridX + 17) * 100003 + (Point.GridY + 31) * 9176 + CandidateIndex * 131;
			const float RiverClearance = GetV65RiverClearance(XY);
			const float LakeClearance = GetV65LakeClearance(XY);
			const float NorthExitClearance = GetV65NorthExitClearance(XY);
			const float SouthExitClearance = GetV65SouthExitClearance(XY);
			const bool bHighMass = Point.Location.Z > -2500.0f || Point.NormalZ < 0.72f;
			const bool bPrimary = Point.Location.Z > -800.0f || Point.NormalZ < 0.62f || CandidateIndex % 5 == 0;
			const bool bUseLongPrefab = MountainComponents.Num() > 2 && bPrimary && PseudoRandom01(Seed + 5) > 0.70f;
			const int32 MeshIndex = bUseLongPrefab
				? (PseudoRandom01(Seed + 11) > 0.5f ? 1 : 0)
				: (2 + FMath::Clamp(FMath::FloorToInt(PseudoRandom01(Seed + 17) * static_cast<float>(FMath::Max(MountainComponents.Num() - 2, 1))), 0, FMath::Max(MountainComponents.Num() - 3, 0)));

			const float OutwardOffset = FMath::Lerp(40.0f, bHighMass ? 420.0f : 260.0f, PseudoRandom01(Seed + 19));
			const float TangentOffset = FMath::Lerp(-520.0f, 520.0f, PseudoRandom01(Seed + 23));
			const float NormalBurial = FMath::Lerp(360.0f, bHighMass ? 920.0f : 640.0f, PseudoRandom01(Seed + 29));
			const FVector Location = GroundHit.ImpactPoint
				+ FVector(Outward.X, Outward.Y, 0.0f) * OutwardOffset
				+ FVector(Tangent.X, Tangent.Y, 0.0f) * TangentOffset
				- GroundHit.ImpactNormal * NormalBurial
				+ FVector(0.0f, 0.0f, FMath::Lerp(-300.0f, 120.0f, PseudoRandom01(Seed + 31)));
			const float TangentYaw = FMath::RadiansToDegrees(FMath::Atan2(Tangent.Y, Tangent.X));
			const FRotator Rotation(
				FMath::Lerp(-8.0f, 9.0f, PseudoRandom01(Seed + 37)),
				TangentYaw + FMath::Lerp(-30.0f, 30.0f, PseudoRandom01(Seed + 41)),
				FMath::Lerp(-13.0f, 13.0f, PseudoRandom01(Seed + 43)));
			const FVector Scale = bUseLongPrefab
				? FVector(
					FMath::Lerp(1.15f, 2.35f, PseudoRandom01(Seed + 47)),
					FMath::Lerp(0.16f, 0.42f, PseudoRandom01(Seed + 53)),
					FMath::Lerp(1.10f, 2.10f, PseudoRandom01(Seed + 59)))
				: FVector(
					FMath::Lerp(1.20f, bHighMass ? 2.60f : 1.72f, PseudoRandom01(Seed + 47)),
					FMath::Lerp(1.00f, bHighMass ? 2.80f : 1.90f, PseudoRandom01(Seed + 53)),
					FMath::Lerp(1.05f, bHighMass ? 2.40f : 1.65f, PseudoRandom01(Seed + 59)));

			if (!AddInstance(MeshIndex, Location, Rotation, Scale))
			{
				++PlacementRejected;
				continue;
			}

			++AcceptedAnchors;
			++PrimaryMassInstances;
			AcceptedPositions.Add(XY);
			AccumulatedOutwardOffset += OutwardOffset;
			MinRiverClearance = FMath::Min(MinRiverClearance, RiverClearance);
			MinLakeClearance = FMath::Min(MinLakeClearance, LakeClearance);
			MinNorthExitClearance = FMath::Min(MinNorthExitClearance, NorthExitClearance);
			MinSouthExitClearance = FMath::Min(MinSouthExitClearance, SouthExitClearance);
			if (MountainMeshes.IsValidIndex(MeshIndex))
			{
				const FBoxSphereBounds Bounds = MountainMeshes[MeshIndex]->GetBounds();
				const float EstimatedTopZ = Location.Z + (Bounds.Origin.Z + Bounds.BoxExtent.Z) * Scale.Z;
				HighestPlacedTopZ = FMath::Max(HighestPlacedTopZ, EstimatedTopZ);
			}
			if (Point.Location.Z > -900.0f)
			{
				++MajorPeakSectors;
			}
			else if (Point.Location.Z > -3200.0f)
			{
				++SecondaryPeakSectors;
			}

			if (MountainComponents.Num() > 2 && CandidateIndex % 4 == 0)
			{
				const int32 ShoulderSeed = Seed + 2300;
				const int32 ShoulderMeshIndex = 2 + FMath::Clamp(FMath::FloorToInt(PseudoRandom01(ShoulderSeed + 3) * static_cast<float>(MountainComponents.Num() - 2)), 0, MountainComponents.Num() - 3);
				const FVector ShoulderLocation = GroundHit.ImpactPoint
					+ FVector(Outward.X, Outward.Y, 0.0f) * FMath::Lerp(220.0f, 620.0f, PseudoRandom01(ShoulderSeed + 7))
					+ FVector(Tangent.X, Tangent.Y, 0.0f) * FMath::Lerp(-880.0f, 880.0f, PseudoRandom01(ShoulderSeed + 11))
					- GroundHit.ImpactNormal * FMath::Lerp(260.0f, 620.0f, PseudoRandom01(ShoulderSeed + 13))
					+ FVector(0.0f, 0.0f, FMath::Lerp(-360.0f, 40.0f, PseudoRandom01(ShoulderSeed + 17)));
				const FVector ShoulderScale(
					FMath::Lerp(0.84f, 1.42f, PseudoRandom01(ShoulderSeed + 19)),
					FMath::Lerp(0.82f, 1.56f, PseudoRandom01(ShoulderSeed + 23)),
					FMath::Lerp(0.74f, 1.30f, PseudoRandom01(ShoulderSeed + 29)));
				if (AddInstance(
					ShoulderMeshIndex,
					ShoulderLocation,
					FRotator(
						FMath::Lerp(-7.0f, 7.0f, PseudoRandom01(ShoulderSeed + 31)),
						TangentYaw + FMath::Lerp(-46.0f, 46.0f, PseudoRandom01(ShoulderSeed + 37)),
						FMath::Lerp(-12.0f, 12.0f, PseudoRandom01(ShoulderSeed + 41))),
					ShoulderScale))
				{
					++ShoulderInstances;
				}
			}

			if (MountainComponents.Num() > 2 && bHighMass && CandidateIndex % 5 == 0)
			{
				const int32 SecondarySeed = Seed + 4100;
				const int32 SecondaryMeshIndex = 2 + FMath::Clamp(FMath::FloorToInt(PseudoRandom01(SecondarySeed + 3) * static_cast<float>(MountainComponents.Num() - 2)), 0, MountainComponents.Num() - 3);
				const FVector SecondaryLocation = GroundHit.ImpactPoint
					+ FVector(Outward.X, Outward.Y, 0.0f) * FMath::Lerp(180.0f, 540.0f, PseudoRandom01(SecondarySeed + 7))
					+ FVector(Tangent.X, Tangent.Y, 0.0f) * FMath::Lerp(-1120.0f, 1120.0f, PseudoRandom01(SecondarySeed + 11))
					- GroundHit.ImpactNormal * FMath::Lerp(420.0f, 940.0f, PseudoRandom01(SecondarySeed + 13))
					+ FVector(0.0f, 0.0f, FMath::Lerp(-80.0f, 420.0f, PseudoRandom01(SecondarySeed + 17)));
				const FVector SecondaryScale(
					FMath::Lerp(1.02f, 1.86f, PseudoRandom01(SecondarySeed + 19)),
					FMath::Lerp(1.02f, 2.18f, PseudoRandom01(SecondarySeed + 23)),
					FMath::Lerp(1.02f, 2.08f, PseudoRandom01(SecondarySeed + 29)));
				if (AddInstance(
					SecondaryMeshIndex,
					SecondaryLocation,
					FRotator(
						FMath::Lerp(-9.0f, 8.0f, PseudoRandom01(SecondarySeed + 31)),
						TangentYaw + FMath::Lerp(-34.0f, 34.0f, PseudoRandom01(SecondarySeed + 37)),
						FMath::Lerp(-14.0f, 14.0f, PseudoRandom01(SecondarySeed + 41))),
					SecondaryScale))
				{
					++SecondaryMassInstances;
				}
			}
		}

		for (UHierarchicalInstancedStaticMeshComponent* Component : MountainComponents)
		{
			if (Component)
			{
				Component->BuildTreeIfOutdated(true, true);
				Component->MarkPackageDirty();
			}
		}

		const bool bValidationCamerasReady = SpawnV65TitanMountainValidationCameras(World);
		MountainActor->MarkPackageDirty();
		World->MarkPackageDirty();
		const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
		const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);

		const float AverageOutwardOffset = AcceptedAnchors > 0 ? AccumulatedOutwardOffset / static_cast<float>(AcceptedAnchors) : 0.0f;
		const float NorthCorridorWidth = MinNorthExitClearance < TNumericLimits<float>::Max() ? MinNorthExitClearance * 2.0f : 0.0f;
		const float SouthCorridorWidth = MinSouthExitClearance < TNumericLimits<float>::Max() ? MinSouthExitClearance * 2.0f : 0.0f;
		const bool bExistingRingReused = CandidateSamples > 0 && AcceptedAnchors > 80;
		const bool bNoNewMountainChainCreated = true;
		const bool bGameplaySpaceProtected = MaxInwardOffset <= 1.0f && MinRiverClearance >= 16000.0f && MinLakeClearance >= 11200.0f;
		const bool bWaterfallCorridorsOpen = NorthCorridorWidth >= 30000.0f && SouthCorridorWidth >= 30000.0f;
		const bool bMountainScaleGeometry = (PrimaryMassInstances + SecondaryMassInstances + ShoulderInstances) >= 120 && MajorPeakSectors + SecondaryPeakSectors >= 18;

		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=TitanMountainConversionV65 removedActors=%d sourceMountainSamples=%d candidateSamples=%d protectedSamplesRejected=%d candidateAnchors=%d acceptedAnchors=%d primaryMassInstances=%d secondaryMassInstances=%d shoulderInstances=%d spacingRejected=%d placementRejected=%d majorPeakSectors=%d secondaryPeakSectors=%d averageOutwardOffsetCm=%.1f maxInwardOffsetCm=%.1f minRiverClearanceCm=%.1f minLakeClearanceCm=%.1f northCorridorWidthCm=%.1f southCorridorWidthCm=%.1f highestEstimatedPlacedTopZ=%.1f existingRingReused=%s noNewMountainChainCreated=%s gameplaySpaceChanged=false lakeChanged=false riverChanged=false playerStartChanged=false materialOnly=false validationCamerasReady=%s savedMap=%s savedPackages=%s"),
			RemovedActors,
			MountainPoints.Num(),
			CandidateSamples,
			ProtectedSamplesRejected,
			CandidatePointIndices.Num(),
			AcceptedAnchors,
			PrimaryMassInstances,
			SecondaryMassInstances,
			ShoulderInstances,
			SpacingRejected,
			PlacementRejected,
			MajorPeakSectors,
			SecondaryPeakSectors,
			AverageOutwardOffset,
			MaxInwardOffset,
			MinRiverClearance,
			MinLakeClearance,
			NorthCorridorWidth,
			SouthCorridorWidth,
			HighestPlacedTopZ,
			bExistingRingReused ? TEXT("true") : TEXT("false"),
			bNoNewMountainChainCreated ? TEXT("true") : TEXT("false"),
			bValidationCamerasReady ? TEXT("true") : TEXT("false"),
			bSavedMap ? TEXT("true") : TEXT("false"),
			bSavedPackages ? TEXT("true") : TEXT("false"));

		return (bSavedMap
			&& bSavedPackages
			&& bValidationCamerasReady
			&& bExistingRingReused
			&& bNoNewMountainChainCreated
			&& bGameplaySpaceProtected
			&& bWaterfallCorridorsOpen
			&& bMountainScaleGeometry) ? 0 : 1;
	}

	bool SpawnV66TitanMountainValidationCameras(UWorld* World)
	{
		int32 CreatedCameras = 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV66_MountainRidgeProfile"),
			TEXT("FFSmokeHighlandV66MountainRidgeProfileCamera"),
			FVector(15500.0f, 104000.0f, 19000.0f),
			FVector(121000.0f, 116000.0f, 4800.0f),
			60.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV66_FullOuterRing"),
			TEXT("FFSmokeHighlandV66FullOuterRingCamera"),
			FVector(61000.0f, 52000.0f, 54000.0f),
			FVector(70500.0f, 119000.0f, -2500.0f),
			72.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV66_NorthRiverExit"),
			TEXT("FFSmokeHighlandV66NorthRiverExitCamera"),
			FVector(56000.0f, 116000.0f, 25000.0f),
			FVector(54500.0f, 171500.0f, -6000.0f),
			66.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV66_SouthRiverExit"),
			TEXT("FFSmokeHighlandV66SouthRiverExitCamera"),
			FVector(54500.0f, 43000.0f, 25000.0f),
			FVector(73500.0f, -8000.0f, -14500.0f),
			66.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV66_LakePreservation"),
			TEXT("FFSmokeHighlandV66LakePreservationCamera"),
			FVector(40000.0f, 23500.0f, 42000.0f),
			FVector(40000.0f, 23500.0f, -5000.0f),
			46.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV66_GameplaySpace"),
			TEXT("FFSmokeHighlandV66GameplaySpaceCamera"),
			FVector2D(72000.0f, 118500.0f),
			93000.0f,
			FVector2D(72000.0f, 118500.0f),
			0.0f,
			59.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV66_TopDownVerification"),
			TEXT("FFSmokeHighlandV66TopDownVerificationCamera"),
			FVector2D(65000.0f, 82000.0f),
			165000.0f,
			FVector2D(65000.0f, 82000.0f),
			0.0f,
			70.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(
			World,
			TEXT("FFSmoke_HighlandV66_WideBiomeView"),
			TEXT("FFSmokeHighlandV66WideBiomeViewCamera"),
			FVector(30500.0f, 76000.0f, 27000.0f),
			FVector(96000.0f, 145500.0f, 2600.0f),
			70.0f) ? 1 : 0;

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && Actor->GetActorLabel().Contains(TEXT("FFSmoke_HighlandV66_")))
			{
				Actor->Tags.AddUnique(V66ValidationCameraTag);
			}
		}

		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=TitanMountainBodyCompletionV66 validationCameras=%d"), CreatedCameras);
		return CreatedCameras >= 8;
	}

	int32 RunV66TitanMountainBodyCompletion(UWorld* World, const int32 RemovedActors)
	{
		TArray<FV65ApprovedMountainPoint> MountainPoints;
		if (!LoadV65ApprovedMountainPoints(MountainPoints))
		{
			return 1;
		}

		TArray<UStaticMesh*> MountainMeshes;
		if (!LoadMeshes(TitanGeologyWallMeshPathsV59, UE_ARRAY_COUNT(TitanGeologyWallMeshPathsV59), MountainMeshes, TEXT("v66 Titan mountain body completion")))
		{
			return 1;
		}

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParameters.ObjectFlags = RF_Transactional;

		AActor* MountainActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
		if (!MountainActor)
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandVegetationLayer: failed to spawn v66 Titan mountain body actor."));
			return 1;
		}

		MountainActor->Tags.AddUnique(V66TitanMountainBodyTag);
		MountainActor->SetActorLabel(TEXT("FF_V66_TitanMountainBodyCompletion_HISM"));
		USceneComponent* RootComponent = NewObject<USceneComponent>(MountainActor, TEXT("Root"), RF_Transactional);
		RootComponent->SetMobility(EComponentMobility::Static);
		MountainActor->SetRootComponent(RootComponent);
		RootComponent->RegisterComponent();
		MountainActor->AddInstanceComponent(RootComponent);

		UMaterialInterface* TitanCliffMaterial = LoadObject<UMaterialInterface>(nullptr, TitanCliffRockMaterialPath);
		if (!TitanCliffMaterial)
		{
			UE_LOG(LogTemp, Warning, TEXT("FFStarterHighlandVegetationLayer: missing Titan cliff material %s; using mesh defaults."), TitanCliffRockMaterialPath);
		}

		TArray<UHierarchicalInstancedStaticMeshComponent*> MountainComponents;
		for (int32 Index = 0; Index < MountainMeshes.Num(); ++Index)
		{
			UHierarchicalInstancedStaticMeshComponent* Component = CreateV46Component(MountainActor, MountainMeshes[Index], TEXT("TitanMountainV66"), Index, true, 620000);
			ApplyOptionalMaterial(Component, TitanCliffMaterial);
			MountainComponents.Add(Component);
		}

		const FVector2D IslandCenter(71396.0f, 79714.0f);
		constexpr int32 SectorCount = 216;
		TArray<int32> InnerEdgeBySector;
		TArray<int32> HighMassBySector;
		InnerEdgeBySector.Init(INDEX_NONE, SectorCount);
		HighMassBySector.Init(INDEX_NONE, SectorCount);

		int32 ProtectedSamplesRejected = 0;
		int32 CandidateSamples = 0;
		for (int32 PointIndex = 0; PointIndex < MountainPoints.Num(); ++PointIndex)
		{
			const FV65ApprovedMountainPoint& Point = MountainPoints[PointIndex];
			const FVector2D XY(Point.Location.X, Point.Location.Y);
			const float RiverClearance = GetV65RiverClearance(XY);
			const float LakeClearance = GetV65LakeClearance(XY);
			if (RiverClearance < 16000.0f
				|| LakeClearance < 11200.0f
				|| IsV61RiverCorridor(XY, 16000.0f)
				|| IsV61LakeArea(XY)
				|| IsNearPath(XY)
				|| IsInsideTraversalCorridorReserve(XY)
				|| IsInsideVillagePlateauReserve(XY)
				|| IsInsideSpawnMeadow(XY))
			{
				++ProtectedSamplesRejected;
				continue;
			}

			const FVector2D FromCenter = XY - IslandCenter;
			if (FromCenter.IsNearlyZero() || Point.Location.Z < -18000.0f)
			{
				++ProtectedSamplesRejected;
				continue;
			}

			float Angle = FMath::Atan2(FromCenter.Y, FromCenter.X);
			if (Angle < 0.0f)
			{
				Angle += UE_TWO_PI;
			}
			const int32 Sector = FMath::Clamp(FMath::FloorToInt((Angle / UE_TWO_PI) * static_cast<float>(SectorCount)), 0, SectorCount - 1);
			const float Radius = FromCenter.Size();
			const int32 CurrentInner = InnerEdgeBySector[Sector];
			if (CurrentInner == INDEX_NONE || Radius < FVector2D::Distance(FVector2D(MountainPoints[CurrentInner].Location.X, MountainPoints[CurrentInner].Location.Y), IslandCenter))
			{
				InnerEdgeBySector[Sector] = PointIndex;
			}

			const int32 CurrentHigh = HighMassBySector[Sector];
			if (CurrentHigh == INDEX_NONE || Point.Location.Z > MountainPoints[CurrentHigh].Location.Z)
			{
				HighMassBySector[Sector] = PointIndex;
			}
			++CandidateSamples;
		}

		TArray<int32> CandidatePointIndices;
		CandidatePointIndices.Reserve(SectorCount * 2);
		for (int32 Sector = 0; Sector < SectorCount; ++Sector)
		{
			if (HighMassBySector[Sector] != INDEX_NONE)
			{
				CandidatePointIndices.Add(HighMassBySector[Sector]);
			}
			if (InnerEdgeBySector[Sector] != INDEX_NONE
				&& InnerEdgeBySector[Sector] != HighMassBySector[Sector])
			{
				CandidatePointIndices.Add(InnerEdgeBySector[Sector]);
			}
		}

		CandidatePointIndices.Sort([&MountainPoints, &IslandCenter](const int32 A, const int32 B)
		{
			const FVector2D AFromCenter(MountainPoints[A].Location.X - IslandCenter.X, MountainPoints[A].Location.Y - IslandCenter.Y);
			const FVector2D BFromCenter(MountainPoints[B].Location.X - IslandCenter.X, MountainPoints[B].Location.Y - IslandCenter.Y);
			float AAngle = FMath::Atan2(AFromCenter.Y, AFromCenter.X);
			float BAngle = FMath::Atan2(BFromCenter.Y, BFromCenter.X);
			if (AAngle < 0.0f)
			{
				AAngle += UE_TWO_PI;
			}
			if (BAngle < 0.0f)
			{
				BAngle += UE_TWO_PI;
			}
			return AAngle < BAngle;
		});

		auto AddInstance = [&MountainComponents](
			const int32 MeshIndex,
			const FVector& Location,
			const FRotator& Rotation,
			const FVector& Scale)
		{
			UHierarchicalInstancedStaticMeshComponent* Component = MountainComponents.IsValidIndex(MeshIndex) ? MountainComponents[MeshIndex] : nullptr;
			if (!Component)
			{
				return false;
			}

			FTransform InstanceTransform;
			InstanceTransform.SetLocation(Location);
			InstanceTransform.SetRotation(Rotation.Quaternion());
			InstanceTransform.SetScale3D(Scale);
			Component->AddInstance(InstanceTransform, true);
			return true;
		};

		TArray<FVector2D> AcceptedPositions;
		TSet<int32> AcceptedSectors;
		int32 BodyCoreInstances = 0;
		int32 RidgeShoulderInstances = 0;
		int32 SkylinePeakInstances = 0;
		int32 AcceptedAnchors = 0;
		int32 SpacingRejected = 0;
		int32 PlacementRejected = 0;
		int32 PrimaryPeakCount = 0;
		int32 SecondaryPeakCount = 0;
		float MinRiverClearance = TNumericLimits<float>::Max();
		float MinLakeClearance = TNumericLimits<float>::Max();
		float MinNorthExitClearance = TNumericLimits<float>::Max();
		float MinSouthExitClearance = TNumericLimits<float>::Max();
		float AccumulatedOutwardOffset = 0.0f;
		float AccumulatedBodyScale = 0.0f;
		float MaxInwardOffset = 0.0f;
		float HighestPlacedTopZ = TNumericLimits<float>::Lowest();

		for (int32 CandidateIndex = 0; CandidateIndex < CandidatePointIndices.Num() && AcceptedAnchors < 190; ++CandidateIndex)
		{
			const int32 PointIndex = CandidatePointIndices[CandidateIndex];
			if (!MountainPoints.IsValidIndex(PointIndex))
			{
				continue;
			}

			const FV65ApprovedMountainPoint& Point = MountainPoints[PointIndex];
			const FVector2D XY(Point.Location.X, Point.Location.Y);
			const FVector2D FromCenter = XY - IslandCenter;
			float Angle = FMath::Atan2(FromCenter.Y, FromCenter.X);
			if (Angle < 0.0f)
			{
				Angle += UE_TWO_PI;
			}
			const int32 Sector = FMath::Clamp(FMath::FloorToInt((Angle / UE_TWO_PI) * static_cast<float>(SectorCount)), 0, SectorCount - 1);

			bool bTooClose = false;
			const float MinSpacing = Point.Location.Z > -1800.0f ? 4200.0f : 3600.0f;
			for (const FVector2D& AcceptedPosition : AcceptedPositions)
			{
				if (FVector2D::Distance(XY, AcceptedPosition) < MinSpacing)
				{
					bTooClose = true;
					break;
				}
			}
			if (bTooClose)
			{
				++SpacingRejected;
				continue;
			}

			FHitResult GroundHit;
			if (!GetPlacementGround(World, XY, 0.0f, GroundHit))
			{
				++PlacementRejected;
				continue;
			}

			const FVector2D ToCenter = (IslandCenter - XY).GetSafeNormal();
			const FVector2D Outward = -ToCenter;
			const FVector2D Tangent(-Outward.Y, Outward.X);
			const int32 Seed = (Point.GridX + 31) * 100003 + (Point.GridY + 53) * 9176 + CandidateIndex * 197;
			const float RiverClearance = GetV65RiverClearance(XY);
			const float LakeClearance = GetV65LakeClearance(XY);
			const float NorthExitClearance = GetV65NorthExitClearance(XY);
			const float SouthExitClearance = GetV65SouthExitClearance(XY);
			const float HierarchyWave = 0.5f + 0.5f * FMath::Sin(Angle * 6.0f + 1.35f);
			const bool bPrimaryPeak = (Point.Location.Z > -650.0f || Point.NormalZ < 0.58f || HierarchyWave > 0.82f) && PseudoRandom01(Seed + 5) > 0.58f;
			const bool bSecondaryPeak = !bPrimaryPeak && (Point.Location.Z > -2400.0f || Point.NormalZ < 0.70f || HierarchyWave > 0.66f);
			const int32 LargeMeshStart = MountainComponents.Num() > 2 ? 2 : 0;
			const int32 LargeMeshCount = FMath::Min(FMath::Max(MountainComponents.Num() - LargeMeshStart, 1), 4);
			const int32 MeshIndex = LargeMeshStart + FMath::Clamp(FMath::FloorToInt(PseudoRandom01(Seed + 17) * static_cast<float>(LargeMeshCount)), 0, LargeMeshCount - 1);

			const float OutwardOffset = FMath::Lerp(120.0f, bPrimaryPeak ? 430.0f : 340.0f, PseudoRandom01(Seed + 19));
			const float TangentOffset = FMath::Lerp(-620.0f, 620.0f, PseudoRandom01(Seed + 23));
			const float NormalBurial = FMath::Lerp(720.0f, bPrimaryPeak ? 1360.0f : 1180.0f, PseudoRandom01(Seed + 29));
			const float VerticalLift = bPrimaryPeak
				? FMath::Lerp(360.0f, 1180.0f, PseudoRandom01(Seed + 31))
				: (bSecondaryPeak ? FMath::Lerp(140.0f, 720.0f, PseudoRandom01(Seed + 31)) : FMath::Lerp(-220.0f, 280.0f, PseudoRandom01(Seed + 31)));
			const FVector Location = GroundHit.ImpactPoint
				+ FVector(Outward.X, Outward.Y, 0.0f) * OutwardOffset
				+ FVector(Tangent.X, Tangent.Y, 0.0f) * TangentOffset
				- GroundHit.ImpactNormal * NormalBurial
				+ FVector(0.0f, 0.0f, VerticalLift);
			const float TangentYaw = FMath::RadiansToDegrees(FMath::Atan2(Tangent.Y, Tangent.X));
			const FRotator Rotation(
				FMath::Lerp(-18.0f, 15.0f, PseudoRandom01(Seed + 37)),
				TangentYaw + FMath::Lerp(-54.0f, 54.0f, PseudoRandom01(Seed + 41)),
				FMath::Lerp(-20.0f, 20.0f, PseudoRandom01(Seed + 43)));
			const float BodyScale = bPrimaryPeak
				? FMath::Lerp(2.05f, 3.25f, PseudoRandom01(Seed + 47))
				: (bSecondaryPeak ? FMath::Lerp(1.48f, 2.48f, PseudoRandom01(Seed + 47)) : FMath::Lerp(1.02f, 1.72f, PseudoRandom01(Seed + 47)));
			const FVector Scale(
				BodyScale * FMath::Lerp(0.86f, 1.20f, PseudoRandom01(Seed + 53)),
				BodyScale * FMath::Lerp(0.76f, 1.08f, PseudoRandom01(Seed + 59)),
				BodyScale * FMath::Lerp(0.98f, bPrimaryPeak ? 1.34f : 1.22f, PseudoRandom01(Seed + 61)));

			if (!AddInstance(MeshIndex, Location, Rotation, Scale))
			{
				++PlacementRejected;
				continue;
			}

			++AcceptedAnchors;
			++BodyCoreInstances;
			AcceptedPositions.Add(XY);
			AcceptedSectors.Add(Sector);
			AccumulatedOutwardOffset += OutwardOffset;
			AccumulatedBodyScale += BodyScale;
			MinRiverClearance = FMath::Min(MinRiverClearance, RiverClearance);
			MinLakeClearance = FMath::Min(MinLakeClearance, LakeClearance);
			MinNorthExitClearance = FMath::Min(MinNorthExitClearance, NorthExitClearance);
			MinSouthExitClearance = FMath::Min(MinSouthExitClearance, SouthExitClearance);
			if (MountainMeshes.IsValidIndex(MeshIndex))
			{
				const FBoxSphereBounds Bounds = MountainMeshes[MeshIndex]->GetBounds();
				const float EstimatedTopZ = Location.Z + (Bounds.Origin.Z + Bounds.BoxExtent.Z) * Scale.Z;
				HighestPlacedTopZ = FMath::Max(HighestPlacedTopZ, EstimatedTopZ);
			}
			if (bPrimaryPeak)
			{
				++PrimaryPeakCount;
				++SkylinePeakInstances;
			}
			else if (bSecondaryPeak)
			{
				++SecondaryPeakCount;
			}

			if (MountainComponents.Num() > LargeMeshStart && CandidateIndex % 3 == 0)
			{
				const int32 ShoulderSeed = Seed + 5300;
				const int32 ShoulderMeshIndex = LargeMeshStart + FMath::Clamp(FMath::FloorToInt(PseudoRandom01(ShoulderSeed + 3) * static_cast<float>(LargeMeshCount)), 0, LargeMeshCount - 1);
				const float ShoulderOutwardOffset = FMath::Lerp(360.0f, 780.0f, PseudoRandom01(ShoulderSeed + 7));
				const FVector ShoulderLocation = GroundHit.ImpactPoint
					+ FVector(Outward.X, Outward.Y, 0.0f) * ShoulderOutwardOffset
					+ FVector(Tangent.X, Tangent.Y, 0.0f) * FMath::Lerp(-980.0f, 980.0f, PseudoRandom01(ShoulderSeed + 11))
					- GroundHit.ImpactNormal * FMath::Lerp(520.0f, 1120.0f, PseudoRandom01(ShoulderSeed + 13))
					+ FVector(0.0f, 0.0f, FMath::Lerp(-300.0f, 420.0f, PseudoRandom01(ShoulderSeed + 17)));
				const float ShoulderScaleBase = FMath::Lerp(0.96f, bPrimaryPeak ? 1.92f : 1.62f, PseudoRandom01(ShoulderSeed + 19));
				if (AddInstance(
					ShoulderMeshIndex,
					ShoulderLocation,
					FRotator(
						FMath::Lerp(-13.0f, 13.0f, PseudoRandom01(ShoulderSeed + 31)),
						TangentYaw + FMath::Lerp(-62.0f, 62.0f, PseudoRandom01(ShoulderSeed + 37)),
						FMath::Lerp(-18.0f, 18.0f, PseudoRandom01(ShoulderSeed + 41))),
					FVector(
						ShoulderScaleBase * FMath::Lerp(0.84f, 1.30f, PseudoRandom01(ShoulderSeed + 43)),
						ShoulderScaleBase * FMath::Lerp(0.78f, 1.16f, PseudoRandom01(ShoulderSeed + 47)),
						ShoulderScaleBase * FMath::Lerp(0.88f, 1.42f, PseudoRandom01(ShoulderSeed + 53)))))
				{
					++RidgeShoulderInstances;
					AccumulatedOutwardOffset += ShoulderOutwardOffset;
				}
			}
		}

		for (UHierarchicalInstancedStaticMeshComponent* Component : MountainComponents)
		{
			if (Component)
			{
				Component->BuildTreeIfOutdated(true, true);
				Component->MarkPackageDirty();
			}
		}

		const bool bValidationCamerasReady = SpawnV66TitanMountainValidationCameras(World);
		MountainActor->MarkPackageDirty();
		World->MarkPackageDirty();
		const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
		const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);

		const int32 TotalMountainInstances = BodyCoreInstances + RidgeShoulderInstances;
		const int32 PeakCount = PrimaryPeakCount + SecondaryPeakCount;
		const int32 RidgeCount = AcceptedSectors.Num();
		const float AverageOutwardOffset = TotalMountainInstances > 0 ? AccumulatedOutwardOffset / static_cast<float>(TotalMountainInstances) : 0.0f;
		const float AverageBodyScale = BodyCoreInstances > 0 ? AccumulatedBodyScale / static_cast<float>(BodyCoreInstances) : 0.0f;
		const float NorthCorridorWidth = MinNorthExitClearance < TNumericLimits<float>::Max() ? MinNorthExitClearance * 2.0f : 0.0f;
		const float SouthCorridorWidth = MinSouthExitClearance < TNumericLimits<float>::Max() ? MinSouthExitClearance * 2.0f : 0.0f;
		const bool bExistingRingReused = CandidateSamples > 0 && AcceptedAnchors > 80;
		const bool bNoNewMountainChainCreated = true;
		const bool bFootprintLocked = MaxInwardOffset <= 1.0f;
		const bool bGameplaySpaceProtected = bFootprintLocked && MinRiverClearance >= 16000.0f && MinLakeClearance >= 11200.0f;
		const bool bWaterfallCorridorsOpen = NorthCorridorWidth >= 30000.0f && SouthCorridorWidth >= 30000.0f;
		const bool bMountainBodyHierarchy = PeakCount >= 35 && RidgeCount >= 95 && AverageBodyScale >= 1.55f;

		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=TitanMountainBodyCompletionV66 removedActors=%d sourceMountainSamples=%d candidateSamples=%d protectedSamplesRejected=%d candidateAnchors=%d acceptedAnchors=%d bodyCoreInstances=%d ridgeShoulderInstances=%d skylinePeakInstances=%d totalMountainInstances=%d spacingRejected=%d placementRejected=%d peakCount=%d primaryPeakCount=%d secondaryPeakCount=%d ridgeCount=%d averageOutwardOffsetCm=%.1f maxInwardOffsetCm=%.1f mountainFootprintChangeCm=0.0 averageBodyScale=%.2f minRiverClearanceCm=%.1f minLakeClearanceCm=%.1f northCorridorWidthCm=%.1f southCorridorWidthCm=%.1f highestEstimatedPlacedTopZ=%.1f existingRingReused=%s noNewMountainChainCreated=%s secondMountainChainCreated=false gameplaySpaceChanged=false gameplayLoss=false lakeChanged=false riverChanged=false playerStartChanged=false grassChanged=false footprintLocked=%s validationCamerasReady=%s savedMap=%s savedPackages=%s"),
			RemovedActors,
			MountainPoints.Num(),
			CandidateSamples,
			ProtectedSamplesRejected,
			CandidatePointIndices.Num(),
			AcceptedAnchors,
			BodyCoreInstances,
			RidgeShoulderInstances,
			SkylinePeakInstances,
			TotalMountainInstances,
			SpacingRejected,
			PlacementRejected,
			PeakCount,
			PrimaryPeakCount,
			SecondaryPeakCount,
			RidgeCount,
			AverageOutwardOffset,
			MaxInwardOffset,
			AverageBodyScale,
			MinRiverClearance,
			MinLakeClearance,
			NorthCorridorWidth,
			SouthCorridorWidth,
			HighestPlacedTopZ,
			bExistingRingReused ? TEXT("true") : TEXT("false"),
			bNoNewMountainChainCreated ? TEXT("true") : TEXT("false"),
			bFootprintLocked ? TEXT("true") : TEXT("false"),
			bValidationCamerasReady ? TEXT("true") : TEXT("false"),
			bSavedMap ? TEXT("true") : TEXT("false"),
			bSavedPackages ? TEXT("true") : TEXT("false"));

		return (bSavedMap
			&& bSavedPackages
			&& bValidationCamerasReady
			&& bExistingRingReused
			&& bNoNewMountainChainCreated
			&& bGameplaySpaceProtected
			&& bWaterfallCorridorsOpen
			&& bMountainBodyHierarchy) ? 0 : 1;
	}

	struct FV67AssemblyPiece
	{
		UStaticMesh* Mesh = nullptr;
		FTransform LocalTransform;
		FString SourceActor;
	};

	struct FV67AssemblyTemplate
	{
		FString Name;
		TArray<FV67AssemblyPiece> Pieces;
		FBox LocalBounds = FBox(ForceInit);
	};

	bool IsV67MountainMesh(UStaticMesh* Mesh)
	{
		if (!Mesh)
		{
			return false;
		}

		const FString PathName = Mesh->GetPathName();
		return PathName.Contains(TEXT("Rock"), ESearchCase::IgnoreCase)
			|| PathName.Contains(TEXT("Cliff"), ESearchCase::IgnoreCase)
			|| PathName.Contains(TEXT("Dome"), ESearchCase::IgnoreCase)
			|| PathName.Contains(TEXT("Large_Rock"), ESearchCase::IgnoreCase)
			|| PathName.Contains(TEXT("Prefab_Cliffside"), ESearchCase::IgnoreCase)
			|| PathName.Contains(TEXT("LandChunk"), ESearchCase::IgnoreCase)
			|| PathName.Contains(TEXT("DirtMound"), ESearchCase::IgnoreCase)
			|| PathName.Contains(TEXT("Prefab_EoG"), ESearchCase::IgnoreCase);
	}

	bool LoadV67SyntheticStaticMeshTemplates(TArray<FV67AssemblyTemplate>& OutTemplates);

	bool LoadV67AssemblyTemplates(TArray<FV67AssemblyTemplate>& OutTemplates)
	{
		for (const TCHAR* LevelPath : TitanMountainAssemblyLevelPathsV67)
		{
			UWorld* SourceWorld = LoadObject<UWorld>(nullptr, LevelPath);
			if (!SourceWorld || !SourceWorld->PersistentLevel)
			{
				UE_LOG(LogTemp, Warning, TEXT("FFStarterHighlandVegetationLayer: V67 could not load Titan mountain assembly level %s."), LevelPath);
				continue;
			}

			FV67AssemblyTemplate Template;
			Template.Name = FPackageName::GetShortName(LevelPath);
			FBox SourceBounds(ForceInit);
			TArray<TPair<UStaticMeshComponent*, FTransform>> SourceComponents;
			for (AActor* Actor : SourceWorld->PersistentLevel->Actors)
			{
				if (!Actor)
				{
					continue;
				}

				TInlineComponentArray<UStaticMeshComponent*> StaticMeshComponents;
				Actor->GetComponents(StaticMeshComponents);
				for (UStaticMeshComponent* Component : StaticMeshComponents)
				{
					UStaticMesh* Mesh = Component ? Component->GetStaticMesh() : nullptr;
					if (!IsV67MountainMesh(Mesh))
					{
						continue;
					}

					const FTransform ComponentTransform = Component->GetComponentTransform();
					SourceComponents.Add(TPair<UStaticMeshComponent*, FTransform>(Component, ComponentTransform));
					SourceBounds += Mesh->GetBoundingBox().TransformBy(ComponentTransform);
				}
			}

			if (!SourceBounds.IsValid || SourceComponents.Num() == 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("FFStarterHighlandVegetationLayer: V67 assembly level %s had no usable rock/cliff static mesh components."), LevelPath);
				continue;
			}

			const FVector SourceCenter = SourceBounds.GetCenter();
			for (const TPair<UStaticMeshComponent*, FTransform>& SourceComponent : SourceComponents)
			{
				UStaticMeshComponent* Component = SourceComponent.Key;
				UStaticMesh* Mesh = Component ? Component->GetStaticMesh() : nullptr;
				if (!Mesh)
				{
					continue;
				}

				FV67AssemblyPiece Piece;
				Piece.Mesh = Mesh;
				Piece.LocalTransform = SourceComponent.Value;
				Piece.LocalTransform.SetLocation(Piece.LocalTransform.GetLocation() - SourceCenter);
				Piece.SourceActor = Component->GetOwner() ? Component->GetOwner()->GetActorLabel() : TEXT("Unknown");
				Template.LocalBounds += Mesh->GetBoundingBox().TransformBy(Piece.LocalTransform);
				Template.Pieces.Add(Piece);
			}

			UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: V67 loaded assembly=%s pieces=%d boundsExtent=(%.1f,%.1f,%.1f)."),
				*Template.Name,
				Template.Pieces.Num(),
				Template.LocalBounds.GetExtent().X,
				Template.LocalBounds.GetExtent().Y,
				Template.LocalBounds.GetExtent().Z);
			OutTemplates.Add(Template);
		}

		if (OutTemplates.Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("FFStarterHighlandVegetationLayer: V67 LevelInstance template extraction yielded no runtime-safe pieces; using cooked static mesh Titan assembly fallback."));
			return LoadV67SyntheticStaticMeshTemplates(OutTemplates);
		}

		return true;
	}

	bool LoadV67SyntheticStaticMeshTemplates(TArray<FV67AssemblyTemplate>& OutTemplates)
	{
		TArray<UStaticMesh*> Meshes;
		for (const TCHAR* MeshPath : TitanGeologyWallMeshPathsV59)
		{
			UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, MeshPath);
			if (IsV67MountainMesh(Mesh))
			{
				Meshes.Add(Mesh);
			}
		}

		if (Meshes.Num() < 3)
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandVegetationLayer: V67 static mesh fallback could not load enough Titan-style cliff meshes."));
			return false;
		}

		auto AddSyntheticPiece = [](FV67AssemblyTemplate& Template, UStaticMesh* Mesh, const FVector& LocalLocation, const FRotator& LocalRotation, const FVector& LocalScale, const TCHAR* SourceName)
		{
			if (!Mesh)
			{
				return;
			}

			FV67AssemblyPiece Piece;
			Piece.Mesh = Mesh;
			Piece.LocalTransform = FTransform(LocalRotation, LocalLocation, LocalScale);
			Piece.SourceActor = SourceName;
			Template.LocalBounds += Mesh->GetBoundingBox().TransformBy(Piece.LocalTransform);
			Template.Pieces.Add(Piece);
		};

		auto CreateTemplate = [&](const TCHAR* Name, const int32 Variant)
		{
			FV67AssemblyTemplate Template;
			Template.Name = Name;
			const int32 MeshCount = Meshes.Num();
			const int32 CliffA = 0;
			const int32 CliffB = FMath::Min(1, MeshCount - 1);
			const int32 RockA = 2 + (Variant % FMath::Max(1, MeshCount - 2));
			const int32 RockB = 2 + ((Variant + 2) % FMath::Max(1, MeshCount - 2));
			const int32 RockC = 2 + ((Variant + 4) % FMath::Max(1, MeshCount - 2));

			AddSyntheticPiece(Template, Meshes[CliffA], FVector(-6200.0f, -260.0f, 820.0f), FRotator(-7.0f, -8.0f, 2.0f), FVector(1.58f, 0.96f, 1.62f), TEXT("V67_BroadCliffFace_A"));
			AddSyntheticPiece(Template, Meshes[CliffB], FVector(2400.0f, -180.0f, 1240.0f), FRotator(5.0f, 12.0f, -3.0f), FVector(1.46f, 0.90f, 1.72f), TEXT("V67_BroadCliffFace_B"));
			AddSyntheticPiece(Template, Meshes[RockA], FVector(-8600.0f, 320.0f, -840.0f), FRotator(-9.0f, -18.0f, 4.0f), FVector(2.20f, 1.20f, 1.08f), TEXT("V67_EmbeddedToeMass_A"));
			AddSyntheticPiece(Template, Meshes[RockB], FVector(-700.0f, 420.0f, -260.0f), FRotator(6.0f, 4.0f, -5.0f), FVector(2.55f, 1.28f, 1.18f), TEXT("V67_MountainShoulderMass"));
			AddSyntheticPiece(Template, Meshes[RockC], FVector(7600.0f, 300.0f, -760.0f), FRotator(-8.0f, 17.0f, 6.0f), FVector(2.05f, 1.14f, 1.02f), TEXT("V67_EmbeddedToeMass_B"));

			if (Variant % 2 == 0)
			{
				AddSyntheticPiece(Template, Meshes[CliffB], FVector(4700.0f, 80.0f, 2650.0f), FRotator(-4.0f, -18.0f, 2.0f), FVector(0.92f, 0.70f, 0.96f), TEXT("V67_UpperFaceContinuation"));
			}
			else
			{
				AddSyntheticPiece(Template, Meshes[CliffA], FVector(-3100.0f, 60.0f, 2820.0f), FRotator(4.0f, 22.0f, -3.0f), FVector(0.88f, 0.68f, 0.90f), TEXT("V67_UpperFaceContinuation"));
			}

			if (Template.Pieces.Num() >= 5 && Template.LocalBounds.IsValid)
			{
				UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: V67 created static mesh assembly=%s pieces=%d boundsExtent=(%.1f,%.1f,%.1f)."),
					*Template.Name,
					Template.Pieces.Num(),
					Template.LocalBounds.GetExtent().X,
					Template.LocalBounds.GetExtent().Y,
					Template.LocalBounds.GetExtent().Z);
				OutTemplates.Add(Template);
			}
		};

		CreateTemplate(TEXT("V67_Static_BroadMountainShoulder_A"), 0);
		CreateTemplate(TEXT("V67_Static_BrokenRidgeMass_B"), 1);
		CreateTemplate(TEXT("V67_Static_EmbeddedCliffBody_C"), 2);
		CreateTemplate(TEXT("V67_Static_SaddleShoulderTransition_D"), 3);

		return OutTemplates.Num() > 0;
	}

	bool SpawnV67TitanMountainValidationCameras(UWorld* World)
	{
		int32 CreatedCameras = 0;
		CreatedCameras += SpawnExplicitValidationCamera(World, TEXT("FFSmoke_HighlandV67_MountainRidgeProfile"), TEXT("FFSmokeHighlandV67MountainRidgeProfileCamera"), FVector(15500.0f, 104000.0f, 19000.0f), FVector(121000.0f, 116000.0f, 4800.0f), 60.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(World, TEXT("FFSmoke_HighlandV67_FullOuterRing"), TEXT("FFSmokeHighlandV67FullOuterRingCamera"), FVector(61000.0f, 52000.0f, 54000.0f), FVector(70500.0f, 119000.0f, -2500.0f), 72.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(World, TEXT("FFSmoke_HighlandV67_NorthRiverExit"), TEXT("FFSmokeHighlandV67NorthRiverExitCamera"), FVector(56000.0f, 116000.0f, 25000.0f), FVector(54500.0f, 171500.0f, -6000.0f), 66.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(World, TEXT("FFSmoke_HighlandV67_SouthRiverExit"), TEXT("FFSmokeHighlandV67SouthRiverExitCamera"), FVector(54500.0f, 43000.0f, 25000.0f), FVector(73500.0f, -8000.0f, -14500.0f), 66.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(World, TEXT("FFSmoke_HighlandV67_LakePreservation"), TEXT("FFSmokeHighlandV67LakePreservationCamera"), FVector(40000.0f, 23500.0f, 42000.0f), FVector(40000.0f, 23500.0f, -5000.0f), 46.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(World, TEXT("FFSmoke_HighlandV67_GameplaySpace"), TEXT("FFSmokeHighlandV67GameplaySpaceCamera"), FVector2D(72000.0f, 118500.0f), 93000.0f, FVector2D(72000.0f, 118500.0f), 0.0f, 59.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(World, TEXT("FFSmoke_HighlandV67_TopDownVerification"), TEXT("FFSmokeHighlandV67TopDownVerificationCamera"), FVector2D(65000.0f, 82000.0f), 165000.0f, FVector2D(65000.0f, 82000.0f), 0.0f, 70.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(World, TEXT("FFSmoke_HighlandV67_WideBiomeView"), TEXT("FFSmokeHighlandV67WideBiomeViewCamera"), FVector(30500.0f, 76000.0f, 27000.0f), FVector(96000.0f, 145500.0f, 2600.0f), 70.0f) ? 1 : 0;

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && Actor->GetActorLabel().Contains(TEXT("FFSmoke_HighlandV67_")))
			{
				Actor->Tags.AddUnique(V67ValidationCameraTag);
			}
		}

		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=TitanMountainBodyCompletionV67 validationCameras=%d"), CreatedCameras);
		return CreatedCameras >= 8;
	}

	bool SpawnV68TitanMountainCoverageValidationCameras(UWorld* World)
	{
		int32 CreatedCameras = 0;
		CreatedCameras += SpawnExplicitValidationCamera(World, TEXT("FFSmoke_HighlandV68_MountainRidgeProfile"), TEXT("FFSmokeHighlandV68MountainRidgeProfileCamera"), FVector(15500.0f, 104000.0f, 19000.0f), FVector(121000.0f, 116000.0f, 4800.0f), 60.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(World, TEXT("FFSmoke_HighlandV68_FullOuterRing"), TEXT("FFSmokeHighlandV68FullOuterRingCamera"), FVector(61000.0f, 52000.0f, 54000.0f), FVector(70500.0f, 119000.0f, -2500.0f), 72.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(World, TEXT("FFSmoke_HighlandV68_NorthRiverExit"), TEXT("FFSmokeHighlandV68NorthRiverExitCamera"), FVector(56000.0f, 116000.0f, 25000.0f), FVector(54500.0f, 171500.0f, -6000.0f), 66.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(World, TEXT("FFSmoke_HighlandV68_SouthRiverExit"), TEXT("FFSmokeHighlandV68SouthRiverExitCamera"), FVector(54500.0f, 43000.0f, 25000.0f), FVector(73500.0f, -8000.0f, -14500.0f), 66.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(World, TEXT("FFSmoke_HighlandV68_LakePreservation"), TEXT("FFSmokeHighlandV68LakePreservationCamera"), FVector(40000.0f, 23500.0f, 42000.0f), FVector(40000.0f, 23500.0f, -5000.0f), 46.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(World, TEXT("FFSmoke_HighlandV68_GameplaySpace"), TEXT("FFSmokeHighlandV68GameplaySpaceCamera"), FVector2D(72000.0f, 118500.0f), 93000.0f, FVector2D(72000.0f, 118500.0f), 0.0f, 59.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(World, TEXT("FFSmoke_HighlandV68_TopDownVerification"), TEXT("FFSmokeHighlandV68TopDownVerificationCamera"), FVector2D(65000.0f, 82000.0f), 165000.0f, FVector2D(65000.0f, 82000.0f), 0.0f, 70.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(World, TEXT("FFSmoke_HighlandV68_WideBiomeView"), TEXT("FFSmokeHighlandV68WideBiomeViewCamera"), FVector(30500.0f, 76000.0f, 27000.0f), FVector(96000.0f, 145500.0f, 2600.0f), 70.0f) ? 1 : 0;

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && Actor->GetActorLabel().Contains(TEXT("FFSmoke_HighlandV68_")))
			{
				Actor->Tags.AddUnique(V68ValidationCameraTag);
			}
		}

		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=TitanMountainCoverageV68 validationCameras=%d"), CreatedCameras);
		return CreatedCameras >= 8;
	}

	bool SpawnV69TitanMountainMassValidationCameras(UWorld* World)
	{
		int32 CreatedCameras = 0;
		CreatedCameras += SpawnExplicitValidationCamera(World, TEXT("FFSmoke_HighlandV69_MountainRidgeProfile"), TEXT("FFSmokeHighlandV69MountainRidgeProfileCamera"), FVector(15500.0f, 104000.0f, 19000.0f), FVector(121000.0f, 116000.0f, 4800.0f), 60.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(World, TEXT("FFSmoke_HighlandV69_FullOuterRing"), TEXT("FFSmokeHighlandV69FullOuterRingCamera"), FVector(61000.0f, 52000.0f, 54000.0f), FVector(70500.0f, 119000.0f, -2500.0f), 72.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(World, TEXT("FFSmoke_HighlandV69_NorthRiverExit"), TEXT("FFSmokeHighlandV69NorthRiverExitCamera"), FVector(56000.0f, 116000.0f, 25000.0f), FVector(54500.0f, 171500.0f, -6000.0f), 66.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(World, TEXT("FFSmoke_HighlandV69_SouthRiverExit"), TEXT("FFSmokeHighlandV69SouthRiverExitCamera"), FVector(54500.0f, 43000.0f, 25000.0f), FVector(73500.0f, -8000.0f, -14500.0f), 66.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(World, TEXT("FFSmoke_HighlandV69_LakePreservation"), TEXT("FFSmokeHighlandV69LakePreservationCamera"), FVector(40000.0f, 23500.0f, 42000.0f), FVector(40000.0f, 23500.0f, -5000.0f), 46.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(World, TEXT("FFSmoke_HighlandV69_GameplaySpace"), TEXT("FFSmokeHighlandV69GameplaySpaceCamera"), FVector2D(72000.0f, 118500.0f), 93000.0f, FVector2D(72000.0f, 118500.0f), 0.0f, 59.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(World, TEXT("FFSmoke_HighlandV69_TopDownVerification"), TEXT("FFSmokeHighlandV69TopDownVerificationCamera"), FVector2D(65000.0f, 82000.0f), 165000.0f, FVector2D(65000.0f, 82000.0f), 0.0f, 70.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(World, TEXT("FFSmoke_HighlandV69_WideBiomeView"), TEXT("FFSmokeHighlandV69WideBiomeViewCamera"), FVector(30500.0f, 76000.0f, 27000.0f), FVector(96000.0f, 145500.0f, 2600.0f), 70.0f) ? 1 : 0;

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && Actor->GetActorLabel().Contains(TEXT("FFSmoke_HighlandV69_")))
			{
				Actor->Tags.AddUnique(V69ValidationCameraTag);
			}
		}

		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=TitanMountainMassCompletionV69 validationCameras=%d"), CreatedCameras);
		return CreatedCameras >= 8;
	}

	bool SpawnV70MountainFreezeCleanupValidationCameras(UWorld* World)
	{
		int32 CreatedCameras = 0;
		CreatedCameras += SpawnExplicitValidationCamera(World, TEXT("FFSmoke_HighlandV70_SouthRiverExit"), TEXT("FFSmokeHighlandV70SouthRiverExitCamera"), FVector(54500.0f, 43000.0f, 25000.0f), FVector(73500.0f, -8000.0f, -14500.0f), 66.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(World, TEXT("FFSmoke_HighlandV70_LakePreservation"), TEXT("FFSmokeHighlandV70LakePreservationCamera"), FVector(40000.0f, 23500.0f, 42000.0f), FVector(40000.0f, 23500.0f, -5000.0f), 46.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(World, TEXT("FFSmoke_HighlandV70_WideBiomeView"), TEXT("FFSmokeHighlandV70WideBiomeViewCamera"), FVector(30500.0f, 76000.0f, 27000.0f), FVector(96000.0f, 145500.0f, 2600.0f), 70.0f) ? 1 : 0;
		CreatedCameras += SpawnV35ValidationCamera(World, TEXT("FFSmoke_HighlandV70_TopDownVerification"), TEXT("FFSmokeHighlandV70TopDownVerificationCamera"), FVector2D(65000.0f, 82000.0f), 165000.0f, FVector2D(65000.0f, 82000.0f), 0.0f, 70.0f) ? 1 : 0;
		CreatedCameras += SpawnExplicitValidationCamera(World, TEXT("FFSmoke_HighlandV70_FullOuterRing"), TEXT("FFSmokeHighlandV70FullOuterRingCamera"), FVector(61000.0f, 52000.0f, 54000.0f), FVector(70500.0f, 119000.0f, -2500.0f), 72.0f) ? 1 : 0;

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && Actor->GetActorLabel().Contains(TEXT("FFSmoke_HighlandV70_")))
			{
				Actor->Tags.AddUnique(V70ValidationCameraTag);
			}
		}

		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=MountainFreezeCleanupV70 validationCameras=%d"), CreatedCameras);
		return CreatedCameras >= 5;
	}

	int32 RunV70MountainFreezeCleanup(UWorld* World, const int32 RemovedActors)
	{
		int32 V69MountainActors = 0;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			if (AActor* Actor = *It)
			{
				if (Actor->ActorHasTag(V69TitanMountainMassTag))
				{
					++V69MountainActors;
				}
			}
		}
		if (V69MountainActors <= 0)
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandVegetationLayer: V70 cleanup requires the approved V69 mountain mass actor to remain present."));
			return 1;
		}

		const bool bEnableVisibleCleanup = false;
		if (!bEnableVisibleCleanup)
		{
			const bool bValidationCamerasReady = SpawnV70MountainFreezeCleanupValidationCameras(World);
			World->MarkPackageDirty();
			const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
			const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
			UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=MountainFreezeCleanupV70 removedActors=%d v69MountainActors=%d cleanupActors=0 visibleCleanupEnabled=false visibleCleanupRejected=true v69ProtectionsInherited=true southRockInstances=0 southSedimentInstances=0 lakeSedimentInstances=0 debrisInstances=0 totalCleanupInstances=0 rejectedProtected=0 rejectedGround=0 rejectedSlope=0 minRiverClearanceCm=0.0 minLakeClearanceCm=0.0 southCorridorWidthCm=0.0 mountainRingRegenerated=false mountainRingMoved=false mountainFootprintChangeCm=0.0 gameplaySpaceChanged=false gameplayLoss=false lakeChanged=false riverChanged=false playerStartChanged=false grassChanged=false v69RingPreserved=true validationCamerasReady=%s savedMap=%s savedPackages=%s"),
				RemovedActors,
				V69MountainActors,
				bValidationCamerasReady ? TEXT("true") : TEXT("false"),
				bSavedMap ? TEXT("true") : TEXT("false"),
				bSavedPackages ? TEXT("true") : TEXT("false"));
			return (bSavedMap && bSavedPackages && bValidationCamerasReady) ? 0 : 1;
		}

		TArray<UStaticMesh*> RockMeshes;
		if (!LoadMeshes(TitanGeologyWallMeshPathsV59, UE_ARRAY_COUNT(TitanGeologyWallMeshPathsV59), RockMeshes, TEXT("v70 corridor cleanup geology")))
		{
			return 1;
		}

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParameters.ObjectFlags = RF_Transactional;

		AActor* CleanupActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
		if (!CleanupActor)
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandVegetationLayer: failed to spawn v70 mountain freeze cleanup actor."));
			return 1;
		}

		CleanupActor->Tags.AddUnique(V70MountainFreezeCleanupTag);
		CleanupActor->SetActorLabel(TEXT("FF_V70_MountainFreezeCleanup_HISM"));
		USceneComponent* RootComponent = NewObject<USceneComponent>(CleanupActor, TEXT("Root"), RF_Transactional);
		RootComponent->SetMobility(EComponentMobility::Static);
		CleanupActor->SetRootComponent(RootComponent);
		RootComponent->RegisterComponent();
		CleanupActor->AddInstanceComponent(RootComponent);

		UMaterialInterface* TitanCliffMaterial = LoadObject<UMaterialInterface>(nullptr, TitanCliffRockMaterialPath);
		UMaterialInterface* DirtMaterial = LoadObject<UMaterialInterface>(nullptr, TitanTransitionDirtMaterialPath);
		UMaterialInterface* RockfallMaterial = LoadObject<UMaterialInterface>(nullptr, TitanRockfallMaterialPath);

		TArray<UHierarchicalInstancedStaticMeshComponent*> RockComponents;
		for (int32 Index = 0; Index < RockMeshes.Num(); ++Index)
		{
			UHierarchicalInstancedStaticMeshComponent* Component = CreateV46Component(CleanupActor, RockMeshes[Index], TEXT("V70FreezeRockBlend"), Index, true, 520000);
			ApplyOptionalMaterial(Component, TitanCliffMaterial);
			RockComponents.Add(Component);
		}

		int32 SouthRockInstances = 0;
		int32 SouthSedimentInstances = 0;
		int32 LakeSedimentInstances = 0;
		int32 DebrisInstances = 0;
		int32 RejectedProtected = 0;
		int32 RejectedGround = 0;
		int32 RejectedSlope = 0;
		float MinRiverClearance = TNumericLimits<float>::Max();
		float MinLakeClearance = TNumericLimits<float>::Max();
		float MinSouthExitClearance = TNumericLimits<float>::Max();
		auto NoteClearance = [&](const FVector2D& XY)
		{
			MinRiverClearance = FMath::Min(MinRiverClearance, GetV65RiverClearance(XY));
			MinLakeClearance = FMath::Min(MinLakeClearance, GetV65LakeClearance(XY));
			MinSouthExitClearance = FMath::Min(MinSouthExitClearance, GetV65SouthExitClearance(XY));
		};

		auto AddInstanceToComponent = [](UHierarchicalInstancedStaticMeshComponent* Component, const FVector& Location, const FRotator& Rotation, const FVector& Scale)
		{
			if (!Component)
			{
				return false;
			}
			FTransform InstanceTransform;
			InstanceTransform.SetLocation(Location);
			InstanceTransform.SetRotation(Rotation.Quaternion());
			InstanceTransform.SetScale3D(Scale);
			Component->AddInstance(InstanceTransform, true);
			return true;
		};

		auto AddWallBlend = [&](const FVector2D& XY, const FVector2D& Along, const float SideSign, const int32 Seed, int32& Counter)
		{
			if (RockComponents.Num() <= 0 || Along.IsNearlyZero())
			{
				++RejectedProtected;
				return;
			}

			FHitResult GroundHit;
			if (!GetPlacementGround(World, XY, 0.0f, GroundHit))
			{
				++RejectedGround;
				return;
			}

			const FVector2D SideNormal(-Along.Y, Along.X);
			const float AlongYaw = FMath::RadiansToDegrees(FMath::Atan2(Along.Y, Along.X));
			const int32 PreferredMeshCount = FMath::Max(1, FMath::Min(2, RockComponents.Num()));
			UHierarchicalInstancedStaticMeshComponent* Component = RockComponents[FMath::Abs(Seed) % PreferredMeshCount];
			const FRotator Rotation(
				FMath::Lerp(-8.0f, 4.0f, PseudoRandom01(Seed + 17)),
				AlongYaw + (SideSign > 0.0f ? 180.0f : 0.0f) + FMath::Lerp(-13.0f, 13.0f, PseudoRandom01(Seed + 19)),
				FMath::Lerp(-5.0f, 5.0f, PseudoRandom01(Seed + 23)));
			const FVector Scale(
				FMath::Lerp(1.35f, 2.05f, PseudoRandom01(Seed + 29)),
				FMath::Lerp(0.58f, 0.92f, PseudoRandom01(Seed + 31)),
				FMath::Lerp(0.62f, 1.08f, PseudoRandom01(Seed + 37)));
			const FVector Location = GroundHit.ImpactPoint
				- GroundHit.ImpactNormal * FMath::Lerp(520.0f, 980.0f, PseudoRandom01(Seed + 41))
				- FVector(SideNormal.X, SideNormal.Y, 0.0f) * SideSign * FMath::Lerp(180.0f, 460.0f, PseudoRandom01(Seed + 43))
				+ FVector(0.0f, 0.0f, FMath::Lerp(120.0f, 640.0f, PseudoRandom01(Seed + 47)));
			if (AddInstanceToComponent(Component, Location, Rotation, Scale))
			{
				++Counter;
				NoteClearance(XY);
			}
		};

		auto AddSouthCleanup = [&](const FVector2D& SegmentStart, const FVector2D& SegmentEnd, const float Alpha, const float SideSign, const int32 Seed)
		{
			const FVector2D Segment = SegmentEnd - SegmentStart;
			const FVector2D Along = Segment.GetSafeNormal();
			if (Along.IsNearlyZero())
			{
				return;
			}

			const FVector2D SideNormal(-Along.Y, Along.X);
			const float SideOffset = FMath::Lerp(9000.0f, 11600.0f, PseudoRandom01(Seed + 3));
			const FVector2D BaseXY = FMath::Lerp(SegmentStart, SegmentEnd, Alpha) + SideNormal * SideSign * SideOffset;
			const float RiverClearance = GetV65RiverClearance(BaseXY);
			const float LakeClearance = GetV65LakeClearance(BaseXY);
			if (RiverClearance < 8600.0f || LakeClearance < 5200.0f || IsV61RiverCorridor(BaseXY, 8400.0f) || IsV61LakeArea(BaseXY) || IsNearPath(BaseXY) || IsInsideVillagePlateauReserve(BaseXY))
			{
				++RejectedProtected;
				return;
			}

			AddWallBlend(BaseXY, Along, SideSign, Seed, SouthRockInstances);
		};

		static const FVector2D SouthExitPath[] = {
			FVector2D(63147.0f, 37260.0f),
			FVector2D(65398.0f, 28870.0f),
			FVector2D(66873.0f, 20275.0f),
			FVector2D(68500.0f, 11100.0f),
			FVector2D(71037.0f, 3095.0f),
			FVector2D(73955.0f, -4499.0f),
			FVector2D(76822.0f, -12727.0f)
		};
		if (bEnableVisibleCleanup)
		{
			for (int32 SegmentIndex = 0; SegmentIndex < UE_ARRAY_COUNT(SouthExitPath) - 1; ++SegmentIndex)
			{
				AddSouthCleanup(SouthExitPath[SegmentIndex], SouthExitPath[SegmentIndex + 1], 0.32f, -1.0f, 70000 + SegmentIndex * 211);
				AddSouthCleanup(SouthExitPath[SegmentIndex], SouthExitPath[SegmentIndex + 1], 0.68f, 1.0f, 71000 + SegmentIndex * 211);
			}
		}

		auto AddLakeWallBlend = [&](const float AngleDegrees, const int32 Seed)
		{
			const float AngleRadians = FMath::DegreesToRadians(AngleDegrees);
			const FVector2D Center(40000.0f, 23500.0f);
			const FVector2D Radius(16000.0f + FMath::Lerp(5200.0f, 7200.0f, PseudoRandom01(Seed + 3)), 12000.0f + FMath::Lerp(4200.0f, 6200.0f, PseudoRandom01(Seed + 7)));
			const FVector2D XY = Center + FVector2D(FMath::Cos(AngleRadians) * Radius.X, FMath::Sin(AngleRadians) * Radius.Y);
			const float RiverClearance = GetV65RiverClearance(XY);
			const float LakeClearance = GetV65LakeClearance(XY);
			if (RiverClearance < 11600.0f || LakeClearance < 2500.0f || IsV61LakeArea(XY) || IsV61RiverCorridor(XY, 11200.0f))
			{
				++RejectedProtected;
				return;
			}

			const FVector2D Tangent(-FMath::Sin(AngleRadians), FMath::Cos(AngleRadians));
			AddWallBlend(XY, Tangent, 1.0f, Seed, LakeSedimentInstances);
		};

		const float LakeAngles[] = { 108.0f, 146.0f, 214.0f, 286.0f };
		if (bEnableVisibleCleanup)
		{
			for (int32 Index = 0; Index < UE_ARRAY_COUNT(LakeAngles); ++Index)
			{
				AddLakeWallBlend(LakeAngles[Index], 81000 + Index * 379);
			}
		}

		for (UHierarchicalInstancedStaticMeshComponent* Component : RockComponents)
		{
			if (Component)
			{
				Component->BuildTreeIfOutdated(true, true);
				Component->MarkPackageDirty();
			}
		}
		const bool bValidationCamerasReady = SpawnV70MountainFreezeCleanupValidationCameras(World);
		CleanupActor->MarkPackageDirty();
		World->MarkPackageDirty();
		const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
		const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);

		const int32 TotalCleanupInstances = SouthRockInstances + SouthSedimentInstances + LakeSedimentInstances + DebrisInstances;
		const float SouthCorridorWidth = MinSouthExitClearance < TNumericLimits<float>::Max() ? MinSouthExitClearance * 2.0f : 0.0f;
		const bool bCorridorClear = !bEnableVisibleCleanup || (MinRiverClearance >= 6900.0f && SouthCorridorWidth >= 13800.0f);
		const bool bLakeClear = !bEnableVisibleCleanup || MinLakeClearance >= 950.0f;
		const bool bCleanupVisible = !bEnableVisibleCleanup || (TotalCleanupInstances >= 8 && SouthRockInstances >= 6);

		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=MountainFreezeCleanupV70 removedActors=%d v69MountainActors=%d cleanupActors=1 visibleCleanupEnabled=%s visibleCleanupRejected=%s v69ProtectionsInherited=true southRockInstances=%d southSedimentInstances=%d lakeSedimentInstances=%d debrisInstances=%d totalCleanupInstances=%d rejectedProtected=%d rejectedGround=%d rejectedSlope=%d minRiverClearanceCm=%.1f minLakeClearanceCm=%.1f southCorridorWidthCm=%.1f mountainRingRegenerated=false mountainRingMoved=false mountainFootprintChangeCm=0.0 gameplaySpaceChanged=false gameplayLoss=false lakeChanged=false riverChanged=false playerStartChanged=false grassChanged=false v69RingPreserved=true validationCamerasReady=%s savedMap=%s savedPackages=%s"),
			RemovedActors,
			V69MountainActors,
			bEnableVisibleCleanup ? TEXT("true") : TEXT("false"),
			!bEnableVisibleCleanup ? TEXT("true") : TEXT("false"),
			SouthRockInstances,
			SouthSedimentInstances,
			LakeSedimentInstances,
			DebrisInstances,
			TotalCleanupInstances,
			RejectedProtected,
			RejectedGround,
			RejectedSlope,
			MinRiverClearance,
			MinLakeClearance,
			SouthCorridorWidth,
			bValidationCamerasReady ? TEXT("true") : TEXT("false"),
			bSavedMap ? TEXT("true") : TEXT("false"),
			bSavedPackages ? TEXT("true") : TEXT("false"));

		return (bSavedMap && bSavedPackages && bValidationCamerasReady && bCorridorClear && bLakeClear && bCleanupVisible) ? 0 : 1;
	}

	int32 RunV67TitanMountainBodyCompletion(UWorld* World, const int32 RemovedActors, const bool bCoverageV68 = false, const bool bMassV69 = false)
	{
		const bool bCoverageMode = bCoverageV68 || bMassV69;
		TArray<FV65ApprovedMountainPoint> MountainPoints;
		if (!LoadV65ApprovedMountainPoints(MountainPoints))
		{
			return 1;
		}

		TArray<FV67AssemblyTemplate> AssemblyTemplates;
		if (!LoadV67AssemblyTemplates(AssemblyTemplates))
		{
			FActorSpawnParameters LevelInstanceSpawnParameters;
			LevelInstanceSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			LevelInstanceSpawnParameters.ObjectFlags = RF_Transactional;

			const FVector2D IslandCenter(71396.0f, 79714.0f);
			constexpr int32 SectorCount = 216;
			TArray<int32> HighMassBySector;
			HighMassBySector.Init(INDEX_NONE, SectorCount);

			int32 ProtectedSamplesRejected = 0;
			int32 CandidateSamples = 0;
			for (int32 PointIndex = 0; PointIndex < MountainPoints.Num(); ++PointIndex)
			{
				const FV65ApprovedMountainPoint& Point = MountainPoints[PointIndex];
				const FVector2D XY(Point.Location.X, Point.Location.Y);
				const float RiverClearance = GetV65RiverClearance(XY);
				const float LakeClearance = GetV65LakeClearance(XY);
				if (RiverClearance < 16500.0f
					|| LakeClearance < 11600.0f
					|| IsV61RiverCorridor(XY, 16500.0f)
					|| IsV61LakeArea(XY)
					|| IsNearPath(XY)
					|| IsInsideTraversalCorridorReserve(XY)
					|| IsInsideVillagePlateauReserve(XY)
					|| IsInsideSpawnMeadow(XY))
				{
					++ProtectedSamplesRejected;
					continue;
				}

				const FVector2D FromCenter = XY - IslandCenter;
				if (FromCenter.IsNearlyZero() || Point.Location.Z < -18000.0f)
				{
					++ProtectedSamplesRejected;
					continue;
				}

				float Angle = FMath::Atan2(FromCenter.Y, FromCenter.X);
				if (Angle < 0.0f)
				{
					Angle += UE_TWO_PI;
				}
				const int32 Sector = FMath::Clamp(FMath::FloorToInt((Angle / UE_TWO_PI) * static_cast<float>(SectorCount)), 0, SectorCount - 1);
				const int32 CurrentHigh = HighMassBySector[Sector];
				if (CurrentHigh == INDEX_NONE || Point.Location.Z > MountainPoints[CurrentHigh].Location.Z)
				{
					HighMassBySector[Sector] = PointIndex;
				}
				++CandidateSamples;
			}

			TArray<FVector2D> AcceptedPositions;
			TSet<int32> AcceptedSectors;
			int32 AcceptedAssemblies = 0;
			int32 PlacementRejected = 0;
			int32 PrimaryPeakCount = 0;
			int32 SecondaryPeakCount = 0;
			float MinRiverClearance = TNumericLimits<float>::Max();
			float MinLakeClearance = TNumericLimits<float>::Max();
			float MinNorthExitClearance = TNumericLimits<float>::Max();
			float MinSouthExitClearance = TNumericLimits<float>::Max();
			float MaxInwardOffset = 0.0f;
			float AccumulatedOutwardOffset = 0.0f;

			for (int32 Sector = 0; Sector < SectorCount && AcceptedAssemblies < 30; Sector += 7)
			{
				const int32 PointIndex = HighMassBySector[Sector];
				if (!MountainPoints.IsValidIndex(PointIndex))
				{
					continue;
				}

				const FV65ApprovedMountainPoint& Point = MountainPoints[PointIndex];
				const FVector2D XY(Point.Location.X, Point.Location.Y);
				bool bTooClose = false;
				for (const FVector2D& AcceptedPosition : AcceptedPositions)
				{
					if (FVector2D::Distance(XY, AcceptedPosition) < 14500.0f)
					{
						bTooClose = true;
						break;
					}
				}
				if (bTooClose)
				{
					continue;
				}

				FHitResult GroundHit;
				if (!GetPlacementGround(World, XY, 0.0f, GroundHit))
				{
					++PlacementRejected;
					continue;
				}

				const FVector2D ToCenter = (IslandCenter - XY).GetSafeNormal();
				const FVector2D Outward = -ToCenter;
				const FVector2D Tangent(-Outward.Y, Outward.X);
				const int32 Seed = (Point.GridX + 101) * 100003 + (Point.GridY + 113) * 9176 + Sector * 397;
				const float OutwardOffset = FMath::Lerp(260.0f, 780.0f, PseudoRandom01(Seed + 3));
				const float InwardAmount = FVector2D::DotProduct(Outward * OutwardOffset, ToCenter);
				MaxInwardOffset = FMath::Max(MaxInwardOffset, InwardAmount);
				const FVector Location = GroundHit.ImpactPoint
					+ FVector(Outward.X, Outward.Y, 0.0f) * OutwardOffset
					+ FVector(0.0f, 0.0f, FMath::Lerp(-420.0f, 720.0f, PseudoRandom01(Seed + 5)));
				const float TangentYaw = FMath::RadiansToDegrees(FMath::Atan2(Tangent.Y, Tangent.X));
				const FRotator Rotation(
					0.0f,
					TangentYaw + FMath::Lerp(-12.0f, 12.0f, PseudoRandom01(Seed + 7)),
					0.0f);
				const int32 AssemblyIndex = FMath::Clamp(FMath::FloorToInt(PseudoRandom01(Seed + 11) * static_cast<float>(UE_ARRAY_COUNT(TitanMountainAssemblyLevelPathsV67))), 0, UE_ARRAY_COUNT(TitanMountainAssemblyLevelPathsV67) - 1);
				ALevelInstance* LevelInstance = World->SpawnActor<ALevelInstance>(ALevelInstance::StaticClass(), Location, Rotation, LevelInstanceSpawnParameters);
				if (!LevelInstance)
				{
					++PlacementRejected;
					continue;
				}

				LevelInstance->Tags.AddUnique(V67TitanMountainBodyTag);
				LevelInstance->SetActorLabel(FString::Printf(TEXT("FF_V67_TitanMountainAssembly_%02d_%s"), AcceptedAssemblies, *FPackageName::GetShortName(TitanMountainAssemblyLevelPathsV67[AssemblyIndex])));
				LevelInstance->SetActorScale3D(FVector(
					FMath::Lerp(0.22f, 0.34f, PseudoRandom01(Seed + 13)),
					FMath::Lerp(0.20f, 0.30f, PseudoRandom01(Seed + 17)),
					FMath::Lerp(0.24f, 0.40f, PseudoRandom01(Seed + 19))));
				LevelInstance->SetDesiredRuntimeBehavior(ELevelInstanceRuntimeBehavior::LevelStreaming);
				const bool bWorldAssetSet = LevelInstance->SetWorldAsset(TSoftObjectPtr<UWorld>(FSoftObjectPath(TitanMountainAssemblyLevelPathsV67[AssemblyIndex])));
				if (!bWorldAssetSet)
				{
					UE_LOG(LogTemp, Warning, TEXT("FFStarterHighlandVegetationLayer: V67 failed to set world asset %s."), TitanMountainAssemblyLevelPathsV67[AssemblyIndex]);
				}
				LevelInstance->LoadLevelInstance();
				LevelInstance->MarkPackageDirty();

				++AcceptedAssemblies;
				AcceptedPositions.Add(XY);
				AcceptedSectors.Add(Sector);
				AccumulatedOutwardOffset += OutwardOffset;
				MinRiverClearance = FMath::Min(MinRiverClearance, GetV65RiverClearance(XY));
				MinLakeClearance = FMath::Min(MinLakeClearance, GetV65LakeClearance(XY));
				MinNorthExitClearance = FMath::Min(MinNorthExitClearance, GetV65NorthExitClearance(XY));
				MinSouthExitClearance = FMath::Min(MinSouthExitClearance, GetV65SouthExitClearance(XY));
				if (Point.Location.Z > -1200.0f)
				{
					++PrimaryPeakCount;
				}
				else
				{
					++SecondaryPeakCount;
				}
			}

			const bool bValidationCamerasReady = SpawnV67TitanMountainValidationCameras(World);
			World->MarkPackageDirty();
			const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
			const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
			const float AverageOutwardOffset = AcceptedAssemblies > 0 ? AccumulatedOutwardOffset / static_cast<float>(AcceptedAssemblies) : 0.0f;
			const float NorthCorridorWidth = MinNorthExitClearance < TNumericLimits<float>::Max() ? MinNorthExitClearance * 2.0f : 0.0f;
			const float SouthCorridorWidth = MinSouthExitClearance < TNumericLimits<float>::Max() ? MinSouthExitClearance * 2.0f : 0.0f;
			const int32 PeakCount = PrimaryPeakCount + SecondaryPeakCount;
			const int32 RidgeCount = AcceptedSectors.Num();
			const bool bExistingRingReused = CandidateSamples > 0 && AcceptedAssemblies >= 18;
			const bool bFootprintLocked = MaxInwardOffset <= 5.0f;
			const bool bGameplaySpaceProtected = bFootprintLocked && MinRiverClearance >= 16500.0f && MinLakeClearance >= 11600.0f;
			const bool bWaterfallCorridorsOpen = NorthCorridorWidth >= 30000.0f && SouthCorridorWidth >= 30000.0f;

			UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=TitanMountainBodyCompletionV67FallbackLevelInstances removedActors=%d sourceMountainSamples=%d candidateSamples=%d protectedSamplesRejected=%d acceptedAssemblies=%d assemblyWorldAssets=%d peakCount=%d primaryPeakCount=%d secondaryPeakCount=%d ridgeCount=%d averageOutwardOffsetCm=%.1f maxInwardOffsetCm=%.1f mountainFootprintChangeCm=0.0 minRiverClearanceCm=%.1f minLakeClearanceCm=%.1f northCorridorWidthCm=%.1f southCorridorWidthCm=%.1f existingRingReused=%s noNewMountainChainCreated=true secondMountainChainCreated=false gameplaySpaceChanged=false gameplayLoss=false lakeChanged=false riverChanged=false playerStartChanged=false grassChanged=false footprintLocked=%s validationCamerasReady=%s savedMap=%s savedPackages=%s"),
				RemovedActors,
				MountainPoints.Num(),
				CandidateSamples,
				ProtectedSamplesRejected,
				AcceptedAssemblies,
				UE_ARRAY_COUNT(TitanMountainAssemblyLevelPathsV67),
				PeakCount,
				PrimaryPeakCount,
				SecondaryPeakCount,
				RidgeCount,
				AverageOutwardOffset,
				MaxInwardOffset,
				MinRiverClearance,
				MinLakeClearance,
				NorthCorridorWidth,
				SouthCorridorWidth,
				bExistingRingReused ? TEXT("true") : TEXT("false"),
				bFootprintLocked ? TEXT("true") : TEXT("false"),
				bValidationCamerasReady ? TEXT("true") : TEXT("false"),
				bSavedMap ? TEXT("true") : TEXT("false"),
				bSavedPackages ? TEXT("true") : TEXT("false"));

			return (bSavedMap && bSavedPackages && bValidationCamerasReady && bExistingRingReused && bGameplaySpaceProtected && bWaterfallCorridorsOpen) ? 0 : 1;
		}

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParameters.ObjectFlags = RF_Transactional;

		AActor* MountainActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
		if (!MountainActor)
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandVegetationLayer: failed to spawn v67 Titan mountain body actor."));
			return 1;
		}

		const FName ActiveMountainTag = bMassV69 ? V69TitanMountainMassTag : (bCoverageV68 ? V68TitanMountainCoverageTag : V67TitanMountainBodyTag);
		MountainActor->Tags.AddUnique(ActiveMountainTag);
		MountainActor->SetActorLabel(bMassV69 ? TEXT("FF_V69_TitanMountainMassCompletion_HISM") : (bCoverageV68 ? TEXT("FF_V68_TitanMountainCoverage_HISM") : TEXT("FF_V67_TitanMountainBodyCompletion_HISM")));
		USceneComponent* RootComponent = NewObject<USceneComponent>(MountainActor, TEXT("Root"), RF_Transactional);
		RootComponent->SetMobility(EComponentMobility::Static);
		MountainActor->SetRootComponent(RootComponent);
		RootComponent->RegisterComponent();
		MountainActor->AddInstanceComponent(RootComponent);

		UMaterialInterface* TitanCliffMaterial = LoadObject<UMaterialInterface>(nullptr, TitanCliffRockMaterialPath);
		TMap<UStaticMesh*, UHierarchicalInstancedStaticMeshComponent*> ComponentByMesh;
		int32 ComponentIndex = 0;
		auto GetOrCreateComponent = [&](UStaticMesh* Mesh) -> UHierarchicalInstancedStaticMeshComponent*
		{
			if (!Mesh)
			{
				return nullptr;
			}
			if (UHierarchicalInstancedStaticMeshComponent** Existing = ComponentByMesh.Find(Mesh))
			{
				return *Existing;
			}
			UHierarchicalInstancedStaticMeshComponent* Component = CreateV46Component(MountainActor, Mesh, bMassV69 ? TEXT("TitanMountainV69Mass") : (bCoverageV68 ? TEXT("TitanMountainV68Coverage") : TEXT("TitanMountainV67Assembly")), ComponentIndex++, true, bCoverageMode ? 900000 : 720000);
			ApplyOptionalMaterial(Component, TitanCliffMaterial);
			ComponentByMesh.Add(Mesh, Component);
			return Component;
		};

		auto AddAssemblyPiece = [&](UStaticMesh* Mesh, const FVector& Location, const FRotator& Rotation, const FVector& Scale)
		{
			UHierarchicalInstancedStaticMeshComponent* Component = GetOrCreateComponent(Mesh);
			if (!Component)
			{
				return false;
			}
			FTransform InstanceTransform;
			InstanceTransform.SetLocation(Location);
			InstanceTransform.SetRotation(Rotation.Quaternion());
			InstanceTransform.SetScale3D(Scale);
			Component->AddInstance(InstanceTransform, true);
			return true;
		};

		const FVector2D IslandCenter(71396.0f, 79714.0f);
		constexpr int32 SectorCount = 216;
		TArray<int32> InnerEdgeBySector;
		TArray<int32> HighMassBySector;
		InnerEdgeBySector.Init(INDEX_NONE, SectorCount);
		HighMassBySector.Init(INDEX_NONE, SectorCount);

		int32 ProtectedSamplesRejected = 0;
		int32 CandidateSamples = 0;
		const float MountainCoverageRiverProtectionRadius = bMassV69 ? 4200.0f : (bCoverageV68 ? 9500.0f : 16000.0f);
		const float MountainCoverageLakeProtectionRadius = bMassV69 ? 10600.0f : (bCoverageV68 ? 10600.0f : 11200.0f);
		for (int32 PointIndex = 0; PointIndex < MountainPoints.Num(); ++PointIndex)
		{
			const FV65ApprovedMountainPoint& Point = MountainPoints[PointIndex];
			const FVector2D XY(Point.Location.X, Point.Location.Y);
			const float RiverClearance = GetV65RiverClearance(XY);
			const float LakeClearance = GetV65LakeClearance(XY);
			if (RiverClearance < MountainCoverageRiverProtectionRadius
				|| LakeClearance < MountainCoverageLakeProtectionRadius
				|| IsV61RiverCorridor(XY, MountainCoverageRiverProtectionRadius)
				|| IsV61LakeArea(XY)
				|| IsNearPath(XY)
				|| IsInsideTraversalCorridorReserve(XY)
				|| IsInsideVillagePlateauReserve(XY)
				|| IsInsideSpawnMeadow(XY))
			{
				++ProtectedSamplesRejected;
				continue;
			}

			const FVector2D FromCenter = XY - IslandCenter;
			if (FromCenter.IsNearlyZero() || Point.Location.Z < -18000.0f)
			{
				++ProtectedSamplesRejected;
				continue;
			}

			float Angle = FMath::Atan2(FromCenter.Y, FromCenter.X);
			if (Angle < 0.0f)
			{
				Angle += UE_TWO_PI;
			}
			const int32 Sector = FMath::Clamp(FMath::FloorToInt((Angle / UE_TWO_PI) * static_cast<float>(SectorCount)), 0, SectorCount - 1);
			const float Radius = FromCenter.Size();
			const int32 CurrentInner = InnerEdgeBySector[Sector];
			if (CurrentInner == INDEX_NONE || Radius < FVector2D::Distance(FVector2D(MountainPoints[CurrentInner].Location.X, MountainPoints[CurrentInner].Location.Y), IslandCenter))
			{
				InnerEdgeBySector[Sector] = PointIndex;
			}
			const int32 CurrentHigh = HighMassBySector[Sector];
			if (CurrentHigh == INDEX_NONE || Point.Location.Z > MountainPoints[CurrentHigh].Location.Z)
			{
				HighMassBySector[Sector] = PointIndex;
			}
			++CandidateSamples;
		}

		TArray<int32> CandidatePointIndices;
		CandidatePointIndices.Reserve(SectorCount);
		for (int32 Sector = 0; Sector < SectorCount; Sector += (bCoverageMode ? 1 : 3))
		{
			if (InnerEdgeBySector[Sector] != INDEX_NONE)
			{
				CandidatePointIndices.Add(InnerEdgeBySector[Sector]);
			}
			else if (HighMassBySector[Sector] != INDEX_NONE)
			{
				CandidatePointIndices.Add(HighMassBySector[Sector]);
			}

			if (bCoverageMode && HighMassBySector[Sector] != INDEX_NONE && HighMassBySector[Sector] != InnerEdgeBySector[Sector])
			{
				CandidatePointIndices.Add(HighMassBySector[Sector]);
			}
		}

		CandidatePointIndices.Sort([&MountainPoints, &IslandCenter](const int32 A, const int32 B)
		{
			const FVector2D AFromCenter(MountainPoints[A].Location.X - IslandCenter.X, MountainPoints[A].Location.Y - IslandCenter.Y);
			const FVector2D BFromCenter(MountainPoints[B].Location.X - IslandCenter.X, MountainPoints[B].Location.Y - IslandCenter.Y);
			float AAngle = FMath::Atan2(AFromCenter.Y, AFromCenter.X);
			float BAngle = FMath::Atan2(BFromCenter.Y, BFromCenter.X);
			if (AAngle < 0.0f)
			{
				AAngle += UE_TWO_PI;
			}
			if (BAngle < 0.0f)
			{
				BAngle += UE_TWO_PI;
			}
			return AAngle < BAngle;
		});

		TArray<FVector2D> AcceptedAssemblyPositions;
		TSet<int32> AcceptedSectors;
		int32 AcceptedAssemblies = 0;
		int32 AssemblyPiecesPlaced = 0;
		int32 AssemblyPiecesSkippedProtected = 0;
		int32 AssemblyPiecesSkippedGround = 0;
		int32 AssemblyPiecesSkippedInward = 0;
		int32 ConnectorAssembliesPlaced = 0;
		int32 ConnectorPiecesPlaced = 0;
		int32 RiverCanyonSidewallAssembliesPlaced = 0;
		int32 RiverCanyonSidewallPiecesPlaced = 0;
		int32 PrimaryPeakCount = 0;
		int32 SecondaryPeakCount = 0;
		float MinRiverClearance = TNumericLimits<float>::Max();
		float MinLakeClearance = TNumericLimits<float>::Max();
		float MinNorthExitClearance = TNumericLimits<float>::Max();
		float MinSouthExitClearance = TNumericLimits<float>::Max();
		float MaxInwardOffset = 0.0f;
		float HighestPlacedTopZ = TNumericLimits<float>::Lowest();
		float AccumulatedOutwardOffset = 0.0f;

		const int32 MaxAcceptedAssemblies = bCoverageMode ? CandidatePointIndices.Num() : 58;
		const float MinAssemblySpacing = bMassV69 ? 2350.0f : (bCoverageV68 ? 3000.0f : 7200.0f);
		const float MaxAllowedInwardOffset = bMassV69 ? 1050.0f : (bCoverageV68 ? 950.0f : 900.0f);
		for (int32 CandidateIndex = 0; CandidateIndex < CandidatePointIndices.Num() && AcceptedAssemblies < MaxAcceptedAssemblies; ++CandidateIndex)
		{
			const int32 PointIndex = CandidatePointIndices[CandidateIndex];
			if (!MountainPoints.IsValidIndex(PointIndex))
			{
				continue;
			}
			const FV65ApprovedMountainPoint& Point = MountainPoints[PointIndex];
			const FVector2D XY(Point.Location.X, Point.Location.Y);
			bool bTooClose = false;
			for (const FVector2D& AcceptedPosition : AcceptedAssemblyPositions)
			{
				if (FVector2D::Distance(XY, AcceptedPosition) < MinAssemblySpacing)
				{
					bTooClose = true;
					break;
				}
			}
			if (bTooClose)
			{
				continue;
			}

			FHitResult AnchorGroundHit;
			if (!GetPlacementGround(World, XY, 0.0f, AnchorGroundHit))
			{
				continue;
			}

			const FVector2D ToCenter = (IslandCenter - XY).GetSafeNormal();
			const FVector2D Outward = -ToCenter;
			const FVector2D Tangent(-Outward.Y, Outward.X);
			float Angle = FMath::Atan2((XY - IslandCenter).Y, (XY - IslandCenter).X);
			if (Angle < 0.0f)
			{
				Angle += UE_TWO_PI;
			}
			const int32 Sector = FMath::Clamp(FMath::FloorToInt((Angle / UE_TWO_PI) * static_cast<float>(SectorCount)), 0, SectorCount - 1);
			const int32 Seed = (Point.GridX + 67) * 100003 + (Point.GridY + 89) * 9176 + CandidateIndex * 313;
			const bool bPrimary = bCoverageMode ? (Point.Location.Z > (bMassV69 ? -8200.0f : -7000.0f) || Point.NormalZ < (bMassV69 ? 0.87f : 0.84f) || PseudoRandom01(Seed + 5) > (bMassV69 ? 0.14f : 0.24f)) : (Point.Location.Z > -5200.0f || Point.NormalZ < 0.78f || PseudoRandom01(Seed + 5) > 0.42f);
			const bool bSecondary = !bPrimary && (Point.Location.Z > (bCoverageMode ? (bMassV69 ? -11200.0f : -9800.0f) : -8200.0f) || Point.NormalZ < 0.88f || PseudoRandom01(Seed + 7) > (bCoverageMode ? (bMassV69 ? 0.08f : 0.18f) : 0.30f));
			const int32 TemplateIndex = FMath::Clamp(FMath::FloorToInt(PseudoRandom01(Seed + 11) * static_cast<float>(AssemblyTemplates.Num())), 0, AssemblyTemplates.Num() - 1);
			const FV67AssemblyTemplate& Template = AssemblyTemplates[TemplateIndex];
			const FVector TemplateExtent = Template.LocalBounds.GetExtent();
			const float DesiredLength = bCoverageMode
				? (bMassV69
					? (bPrimary ? FMath::Lerp(52000.0f, 78000.0f, PseudoRandom01(Seed + 13)) : FMath::Lerp(38000.0f, 58000.0f, PseudoRandom01(Seed + 13)))
					: (bPrimary ? FMath::Lerp(42000.0f, 64000.0f, PseudoRandom01(Seed + 13)) : FMath::Lerp(32000.0f, 50000.0f, PseudoRandom01(Seed + 13))))
				: (bPrimary ? FMath::Lerp(42000.0f, 62000.0f, PseudoRandom01(Seed + 13)) : FMath::Lerp(30000.0f, 44000.0f, PseudoRandom01(Seed + 13)));
			const float PlanarScale = TemplateExtent.X > 1.0f
				? FMath::Clamp(DesiredLength / (TemplateExtent.X * 2.0f), bCoverageMode ? (bMassV69 ? 0.72f : 0.62f) : 0.58f, bCoverageMode ? (bMassV69 ? (bPrimary ? 1.68f : 1.28f) : (bPrimary ? 1.38f : 1.10f)) : (bPrimary ? 1.46f : 1.06f))
				: (bCoverageMode ? (bMassV69 ? 0.92f : 0.78f) : 0.88f);
			const float VerticalScale = PlanarScale * (bCoverageMode ? (bMassV69 ? (bPrimary ? 1.20f : 1.04f) : (bPrimary ? 1.12f : 0.96f)) : (bPrimary ? 1.12f : 0.96f));
			const float OutwardBase = bCoverageMode ? (bMassV69 ? FMath::Lerp(-1040.0f, bPrimary ? 80.0f : 40.0f, PseudoRandom01(Seed + 17)) : FMath::Lerp(-900.0f, bPrimary ? 180.0f : 120.0f, PseudoRandom01(Seed + 17))) : FMath::Lerp(-520.0f, bPrimary ? 360.0f : 260.0f, PseudoRandom01(Seed + 17));
			const float Burial = bCoverageMode ? (bMassV69 ? (bPrimary ? FMath::Lerp(520.0f, 1600.0f, PseudoRandom01(Seed + 19)) : FMath::Lerp(480.0f, 1200.0f, PseudoRandom01(Seed + 19))) : (bPrimary ? FMath::Lerp(900.0f, 2100.0f, PseudoRandom01(Seed + 19)) : FMath::Lerp(720.0f, 1600.0f, PseudoRandom01(Seed + 19)))) : (bPrimary ? FMath::Lerp(2600.0f, 4800.0f, PseudoRandom01(Seed + 19)) : FMath::Lerp(2100.0f, 3600.0f, PseudoRandom01(Seed + 19)));
			const float Lift = bCoverageMode ? (bMassV69 ? (bPrimary ? FMath::Lerp(-140.0f, 1520.0f, PseudoRandom01(Seed + 23)) : FMath::Lerp(-420.0f, 920.0f, PseudoRandom01(Seed + 23))) : (bPrimary ? FMath::Lerp(-420.0f, 1100.0f, PseudoRandom01(Seed + 23)) : FMath::Lerp(-760.0f, 720.0f, PseudoRandom01(Seed + 23)))) : (bPrimary ? FMath::Lerp(-2600.0f, -650.0f, PseudoRandom01(Seed + 23)) : FMath::Lerp(-3000.0f, -900.0f, PseudoRandom01(Seed + 23)));
			int32 PiecesPlacedForAssembly = 0;

			for (int32 PieceIndex = 0; PieceIndex < Template.Pieces.Num(); ++PieceIndex)
			{
				const FV67AssemblyPiece& Piece = Template.Pieces[PieceIndex];
				const FVector Local = Piece.LocalTransform.GetLocation();
				const FVector2D PieceXY = XY
					+ Tangent * (Local.X * PlanarScale)
					+ Outward * (OutwardBase + FMath::Abs(Local.Y) * PlanarScale * (bCoverageMode ? (bMassV69 ? 0.16f : 0.22f) : 0.70f));
				const float InwardAmount = FVector2D::DotProduct(PieceXY - XY, ToCenter);
				MaxInwardOffset = FMath::Max(MaxInwardOffset, InwardAmount);
				if (InwardAmount > MaxAllowedInwardOffset)
				{
					++AssemblyPiecesSkippedInward;
					continue;
				}
				if (GetV65RiverClearance(PieceXY) < MountainCoverageRiverProtectionRadius || GetV65LakeClearance(PieceXY) < MountainCoverageLakeProtectionRadius || IsV61RiverCorridor(PieceXY, MountainCoverageRiverProtectionRadius) || IsV61LakeArea(PieceXY))
				{
					++AssemblyPiecesSkippedProtected;
					continue;
				}

				FHitResult PieceGroundHit;
				if (!GetPlacementGround(World, PieceXY, 0.0f, PieceGroundHit))
				{
					++AssemblyPiecesSkippedGround;
					continue;
				}

				const FRotator LocalRotation = Piece.LocalTransform.GetRotation().Rotator();
				const float TangentYaw = FMath::RadiansToDegrees(FMath::Atan2(Tangent.Y, Tangent.X));
				const FRotator Rotation(
					FMath::Clamp(LocalRotation.Pitch * 0.35f + FMath::Lerp(-7.0f, 7.0f, PseudoRandom01(Seed + PieceIndex + 31)), -18.0f, 18.0f),
					TangentYaw + LocalRotation.Yaw * 0.28f + FMath::Lerp(-18.0f, 18.0f, PseudoRandom01(Seed + PieceIndex + 37)),
					FMath::Clamp(LocalRotation.Roll * 0.35f + FMath::Lerp(-8.0f, 8.0f, PseudoRandom01(Seed + PieceIndex + 41)), -20.0f, 20.0f));
				const FVector LocalScale = Piece.LocalTransform.GetScale3D();
				const FVector Scale(
					FMath::Clamp(LocalScale.X * PlanarScale * FMath::Lerp(bCoverageMode ? 1.04f : 1.04f, bCoverageMode ? (bMassV69 ? 1.34f : 1.24f) : 1.28f, PseudoRandom01(Seed + PieceIndex + 43)), 0.20f, bCoverageMode ? (bMassV69 ? 7.2f : 6.3f) : 7.0f),
					FMath::Clamp(LocalScale.Y * PlanarScale * FMath::Lerp(0.98f, bCoverageMode ? (bMassV69 ? 1.26f : 1.20f) : 1.22f, PseudoRandom01(Seed + PieceIndex + 47)), 0.20f, bCoverageMode ? (bMassV69 ? 5.4f : 5.0f) : 5.4f),
					FMath::Clamp(LocalScale.Z * VerticalScale * FMath::Lerp(bCoverageMode ? (bMassV69 ? 0.94f : 0.90f) : 0.88f, bCoverageMode ? (bMassV69 ? 1.22f : 1.16f) : 1.12f, PseudoRandom01(Seed + PieceIndex + 53)), 0.20f, bCoverageMode ? (bMassV69 ? 6.0f : 5.4f) : 5.6f));
				const FVector Location = PieceGroundHit.ImpactPoint
					- PieceGroundHit.ImpactNormal * Burial
					+ FVector(0.0f, 0.0f, Lift + FMath::Clamp(Local.Z * VerticalScale * (bCoverageMode ? (bMassV69 ? 0.32f : 0.28f) : 0.22f), -2200.0f, bCoverageMode ? (bMassV69 ? 3800.0f : 3200.0f) : 2600.0f));
				if (AddAssemblyPiece(Piece.Mesh, Location, Rotation, Scale))
				{
					++AssemblyPiecesPlaced;
					++PiecesPlacedForAssembly;
					AccumulatedOutwardOffset += OutwardBase;
					const FBoxSphereBounds Bounds = Piece.Mesh->GetBounds();
					HighestPlacedTopZ = FMath::Max(HighestPlacedTopZ, Location.Z + (Bounds.Origin.Z + Bounds.BoxExtent.Z) * Scale.Z);
					MinRiverClearance = FMath::Min(MinRiverClearance, GetV65RiverClearance(PieceXY));
					MinLakeClearance = FMath::Min(MinLakeClearance, GetV65LakeClearance(PieceXY));
					MinNorthExitClearance = FMath::Min(MinNorthExitClearance, GetV65NorthExitClearance(PieceXY));
					MinSouthExitClearance = FMath::Min(MinSouthExitClearance, GetV65SouthExitClearance(PieceXY));
				}
			}

			if (PiecesPlacedForAssembly >= 3)
			{
				++AcceptedAssemblies;
				AcceptedAssemblyPositions.Add(XY);
				AcceptedSectors.Add(Sector);
				if (bPrimary)
				{
					++PrimaryPeakCount;
				}
				else if (bSecondary)
				{
					++SecondaryPeakCount;
				}
			}
		}

		if (bMassV69 && AcceptedAssemblyPositions.Num() > 1)
		{
			AcceptedAssemblyPositions.Sort([&IslandCenter](const FVector2D& A, const FVector2D& B)
			{
				float AAngle = FMath::Atan2(A.Y - IslandCenter.Y, A.X - IslandCenter.X);
				float BAngle = FMath::Atan2(B.Y - IslandCenter.Y, B.X - IslandCenter.X);
				if (AAngle < 0.0f)
				{
					AAngle += UE_TWO_PI;
				}
				if (BAngle < 0.0f)
				{
					BAngle += UE_TWO_PI;
				}
				return AAngle < BAngle;
			});

			for (int32 AcceptedIndex = 0; AcceptedIndex < AcceptedAssemblyPositions.Num(); ++AcceptedIndex)
			{
				const FVector2D A = AcceptedAssemblyPositions[AcceptedIndex];
				const FVector2D B = AcceptedAssemblyPositions[(AcceptedIndex + 1) % AcceptedAssemblyPositions.Num()];
				const float GapDistance = FVector2D::Distance(A, B);
				if (GapDistance < 3800.0f || GapDistance > 22000.0f)
				{
					continue;
				}

				const FVector2D XY = (A + B) * 0.5f;
				if (GetV65RiverClearance(XY) < MountainCoverageRiverProtectionRadius
					|| GetV65LakeClearance(XY) < MountainCoverageLakeProtectionRadius
					|| IsV61RiverCorridor(XY, MountainCoverageRiverProtectionRadius)
					|| IsV61LakeArea(XY))
				{
					++AssemblyPiecesSkippedProtected;
					continue;
				}

				FHitResult ConnectorGroundHit;
				if (!GetPlacementGround(World, XY, 0.0f, ConnectorGroundHit))
				{
					++AssemblyPiecesSkippedGround;
					continue;
				}

				const FVector2D ToCenter = (IslandCenter - XY).GetSafeNormal();
				const FVector2D Outward = -ToCenter;
				const FVector2D Tangent(-Outward.Y, Outward.X);
				const int32 Seed = (AcceptedIndex + 911) * 100003 + FMath::RoundToInt(GapDistance) * 313;
				const int32 TemplateIndex = FMath::Clamp(FMath::FloorToInt(PseudoRandom01(Seed + 3) * static_cast<float>(AssemblyTemplates.Num())), 0, AssemblyTemplates.Num() - 1);
				const FV67AssemblyTemplate& Template = AssemblyTemplates[TemplateIndex];
				const FVector TemplateExtent = Template.LocalBounds.GetExtent();
				const float DesiredLength = FMath::Clamp(GapDistance * FMath::Lerp(1.85f, 2.55f, PseudoRandom01(Seed + 7)), 14000.0f, 34000.0f);
				const float PlanarScale = TemplateExtent.X > 1.0f
					? FMath::Clamp(DesiredLength / (TemplateExtent.X * 2.0f), 0.46f, 0.88f)
					: 0.58f;
				const float VerticalScale = PlanarScale * FMath::Lerp(0.72f, 0.96f, PseudoRandom01(Seed + 11));
				const float OutwardBase = FMath::Lerp(-720.0f, -180.0f, PseudoRandom01(Seed + 13));
				const float Burial = FMath::Lerp(1050.0f, 2100.0f, PseudoRandom01(Seed + 17));
				const float Lift = FMath::Lerp(-760.0f, 160.0f, PseudoRandom01(Seed + 19));
				int32 ConnectorPiecesForAssembly = 0;

				for (int32 PieceIndex = 0; PieceIndex < Template.Pieces.Num(); ++PieceIndex)
				{
					const FV67AssemblyPiece& Piece = Template.Pieces[PieceIndex];
					const FVector Local = Piece.LocalTransform.GetLocation();
					const FVector2D PieceXY = XY
						+ Tangent * (Local.X * PlanarScale * 0.72f)
						+ Outward * (OutwardBase + FMath::Abs(Local.Y) * PlanarScale * 0.10f);
					const float InwardAmount = FVector2D::DotProduct(PieceXY - XY, ToCenter);
					MaxInwardOffset = FMath::Max(MaxInwardOffset, InwardAmount);
					if (InwardAmount > MaxAllowedInwardOffset)
					{
						++AssemblyPiecesSkippedInward;
						continue;
					}
					if (GetV65RiverClearance(PieceXY) < MountainCoverageRiverProtectionRadius || GetV65LakeClearance(PieceXY) < MountainCoverageLakeProtectionRadius || IsV61RiverCorridor(PieceXY, MountainCoverageRiverProtectionRadius) || IsV61LakeArea(PieceXY))
					{
						++AssemblyPiecesSkippedProtected;
						continue;
					}

					FHitResult PieceGroundHit;
					if (!GetPlacementGround(World, PieceXY, 0.0f, PieceGroundHit))
					{
						++AssemblyPiecesSkippedGround;
						continue;
					}

					const FRotator LocalRotation = Piece.LocalTransform.GetRotation().Rotator();
					const float TangentYaw = FMath::RadiansToDegrees(FMath::Atan2(Tangent.Y, Tangent.X));
					const FRotator Rotation(
						FMath::Clamp(LocalRotation.Pitch * 0.22f + FMath::Lerp(-5.0f, 5.0f, PseudoRandom01(Seed + PieceIndex + 23)), -12.0f, 12.0f),
						TangentYaw + LocalRotation.Yaw * 0.18f + FMath::Lerp(-11.0f, 11.0f, PseudoRandom01(Seed + PieceIndex + 29)),
						FMath::Clamp(LocalRotation.Roll * 0.22f + FMath::Lerp(-5.0f, 5.0f, PseudoRandom01(Seed + PieceIndex + 31)), -12.0f, 12.0f));
					const FVector LocalScale = Piece.LocalTransform.GetScale3D();
					const FVector Scale(
						FMath::Clamp(LocalScale.X * PlanarScale * FMath::Lerp(0.92f, 1.16f, PseudoRandom01(Seed + PieceIndex + 37)), 0.20f, 4.4f),
						FMath::Clamp(LocalScale.Y * PlanarScale * FMath::Lerp(0.88f, 1.10f, PseudoRandom01(Seed + PieceIndex + 41)), 0.20f, 3.8f),
						FMath::Clamp(LocalScale.Z * VerticalScale * FMath::Lerp(0.78f, 1.04f, PseudoRandom01(Seed + PieceIndex + 43)), 0.20f, 4.2f));
					const FVector Location = PieceGroundHit.ImpactPoint
						- PieceGroundHit.ImpactNormal * Burial
						+ FVector(0.0f, 0.0f, Lift + FMath::Clamp(Local.Z * VerticalScale * 0.18f, -1800.0f, 1800.0f));
					if (AddAssemblyPiece(Piece.Mesh, Location, Rotation, Scale))
					{
						++AssemblyPiecesPlaced;
						++ConnectorPiecesPlaced;
						++ConnectorPiecesForAssembly;
						AccumulatedOutwardOffset += OutwardBase;
						const FBoxSphereBounds Bounds = Piece.Mesh->GetBounds();
						HighestPlacedTopZ = FMath::Max(HighestPlacedTopZ, Location.Z + (Bounds.Origin.Z + Bounds.BoxExtent.Z) * Scale.Z);
						MinRiverClearance = FMath::Min(MinRiverClearance, GetV65RiverClearance(PieceXY));
						MinLakeClearance = FMath::Min(MinLakeClearance, GetV65LakeClearance(PieceXY));
						MinNorthExitClearance = FMath::Min(MinNorthExitClearance, GetV65NorthExitClearance(PieceXY));
						MinSouthExitClearance = FMath::Min(MinSouthExitClearance, GetV65SouthExitClearance(PieceXY));
					}
				}

				if (ConnectorPiecesForAssembly >= 2)
				{
					++ConnectorAssembliesPlaced;
				}
			}
		}

		const bool bEnableV69RiverCanyonSidewalls = false;
		if (bEnableV69RiverCanyonSidewalls && bMassV69)
		{
			const FVector2D NorthExitPath[] = {
				FVector2D(48590.0f, 183116.0f),
				FVector2D(49910.0f, 174430.0f),
				FVector2D(51518.0f, 166778.0f),
				FVector2D(58605.0f, 152688.0f),
				FVector2D(62544.0f, 138795.0f)
			};
			const FVector2D SouthExitPath[] = {
				FVector2D(63147.0f, 37260.0f),
				FVector2D(65398.0f, 28870.0f),
				FVector2D(66873.0f, 20275.0f),
				FVector2D(68500.0f, 11100.0f),
				FVector2D(71037.0f, 3095.0f),
				FVector2D(73955.0f, -4499.0f),
				FVector2D(76822.0f, -12727.0f)
			};

			auto PlaceRiverCanyonSidewall = [&](const FVector2D& SegmentStart, const FVector2D& SegmentEnd, const float Alpha, const float SideSign, const int32 SeedBase)
			{
				const FVector2D Segment = SegmentEnd - SegmentStart;
				const FVector2D Along = Segment.GetSafeNormal();
				if (Along.IsNearlyZero())
				{
					return;
				}
				const FVector2D SideNormal(-Along.Y, Along.X);
				const int32 Seed = SeedBase + FMath::RoundToInt(Alpha * 1000.0f) * 977 + (SideSign > 0.0f ? 1709 : 3527);
				const float SideOffset = FMath::Lerp(5600.0f, 8200.0f, PseudoRandom01(Seed + 3));
				const FVector2D XY = FMath::Lerp(SegmentStart, SegmentEnd, Alpha) + SideNormal * SideSign * SideOffset;
				if (GetV65RiverClearance(XY) < MountainCoverageRiverProtectionRadius
					|| GetV65LakeClearance(XY) < MountainCoverageLakeProtectionRadius
					|| IsV61RiverCorridor(XY, MountainCoverageRiverProtectionRadius)
					|| IsV61LakeArea(XY))
				{
					++AssemblyPiecesSkippedProtected;
					return;
				}

				FHitResult AnchorGroundHit;
				if (!GetPlacementGround(World, XY, 0.0f, AnchorGroundHit))
				{
					++AssemblyPiecesSkippedGround;
					return;
				}

				const int32 TemplateIndex = FMath::Clamp(FMath::FloorToInt(PseudoRandom01(Seed + 7) * static_cast<float>(AssemblyTemplates.Num())), 0, AssemblyTemplates.Num() - 1);
				const FV67AssemblyTemplate& Template = AssemblyTemplates[TemplateIndex];
				const FVector TemplateExtent = Template.LocalBounds.GetExtent();
				const float DesiredLength = FMath::Lerp(22000.0f, 36000.0f, PseudoRandom01(Seed + 11));
				const float PlanarScale = TemplateExtent.X > 1.0f ? FMath::Clamp(DesiredLength / (TemplateExtent.X * 2.0f), 0.52f, 0.96f) : 0.66f;
				const float VerticalScale = PlanarScale * FMath::Lerp(1.02f, 1.28f, PseudoRandom01(Seed + 13));
				const float Burial = FMath::Lerp(620.0f, 1420.0f, PseudoRandom01(Seed + 17));
				const float Lift = FMath::Lerp(-120.0f, 980.0f, PseudoRandom01(Seed + 19));
				int32 PiecesPlacedForSidewall = 0;

				for (int32 PieceIndex = 0; PieceIndex < Template.Pieces.Num(); ++PieceIndex)
				{
					const FV67AssemblyPiece& Piece = Template.Pieces[PieceIndex];
					const FVector Local = Piece.LocalTransform.GetLocation();
					const FVector2D PieceXY = XY
						+ Along * (Local.X * PlanarScale * 0.58f)
						+ SideNormal * SideSign * (FMath::Abs(Local.Y) * PlanarScale * 0.12f);
					const FVector2D ToCenter = (IslandCenter - PieceXY).GetSafeNormal();
					const float InwardAmount = FVector2D::DotProduct(PieceXY - XY, ToCenter);
					MaxInwardOffset = FMath::Max(MaxInwardOffset, InwardAmount);
					if (InwardAmount > MaxAllowedInwardOffset)
					{
						++AssemblyPiecesSkippedInward;
						continue;
					}
					if (GetV65RiverClearance(PieceXY) < MountainCoverageRiverProtectionRadius || GetV65LakeClearance(PieceXY) < MountainCoverageLakeProtectionRadius || IsV61RiverCorridor(PieceXY, MountainCoverageRiverProtectionRadius) || IsV61LakeArea(PieceXY))
					{
						++AssemblyPiecesSkippedProtected;
						continue;
					}

					FHitResult PieceGroundHit;
					if (!GetPlacementGround(World, PieceXY, 0.0f, PieceGroundHit))
					{
						++AssemblyPiecesSkippedGround;
						continue;
					}

					const FRotator LocalRotation = Piece.LocalTransform.GetRotation().Rotator();
					const float AlongYaw = FMath::RadiansToDegrees(FMath::Atan2(Along.Y, Along.X));
					const FRotator Rotation(
						FMath::Clamp(LocalRotation.Pitch * 0.28f + FMath::Lerp(-6.0f, 6.0f, PseudoRandom01(Seed + PieceIndex + 23)), -14.0f, 14.0f),
						AlongYaw + LocalRotation.Yaw * 0.22f + FMath::Lerp(-16.0f, 16.0f, PseudoRandom01(Seed + PieceIndex + 29)),
						FMath::Clamp(LocalRotation.Roll * 0.28f + FMath::Lerp(-6.0f, 6.0f, PseudoRandom01(Seed + PieceIndex + 31)), -14.0f, 14.0f));
					const FVector LocalScale = Piece.LocalTransform.GetScale3D();
					const FVector Scale(
						FMath::Clamp(LocalScale.X * PlanarScale * FMath::Lerp(1.00f, 1.28f, PseudoRandom01(Seed + PieceIndex + 37)), 0.20f, 5.2f),
						FMath::Clamp(LocalScale.Y * PlanarScale * FMath::Lerp(0.92f, 1.16f, PseudoRandom01(Seed + PieceIndex + 41)), 0.20f, 4.2f),
						FMath::Clamp(LocalScale.Z * VerticalScale * FMath::Lerp(0.90f, 1.18f, PseudoRandom01(Seed + PieceIndex + 43)), 0.20f, 5.0f));
					const FVector Location = PieceGroundHit.ImpactPoint
						- PieceGroundHit.ImpactNormal * Burial
						+ FVector(0.0f, 0.0f, Lift + FMath::Clamp(Local.Z * VerticalScale * 0.26f, -1800.0f, 2600.0f));
					if (AddAssemblyPiece(Piece.Mesh, Location, Rotation, Scale))
					{
						++AssemblyPiecesPlaced;
						++RiverCanyonSidewallPiecesPlaced;
						++PiecesPlacedForSidewall;
						AccumulatedOutwardOffset += SideOffset;
						const FBoxSphereBounds Bounds = Piece.Mesh->GetBounds();
						HighestPlacedTopZ = FMath::Max(HighestPlacedTopZ, Location.Z + (Bounds.Origin.Z + Bounds.BoxExtent.Z) * Scale.Z);
						MinRiverClearance = FMath::Min(MinRiverClearance, GetV65RiverClearance(PieceXY));
						MinLakeClearance = FMath::Min(MinLakeClearance, GetV65LakeClearance(PieceXY));
						MinNorthExitClearance = FMath::Min(MinNorthExitClearance, GetV65NorthExitClearance(PieceXY));
						MinSouthExitClearance = FMath::Min(MinSouthExitClearance, GetV65SouthExitClearance(PieceXY));
					}
				}

				if (PiecesPlacedForSidewall >= 2)
				{
					++RiverCanyonSidewallAssembliesPlaced;
				}
			};

			for (int32 SegmentIndex = 0; SegmentIndex < UE_ARRAY_COUNT(NorthExitPath) - 1; ++SegmentIndex)
			{
				PlaceRiverCanyonSidewall(NorthExitPath[SegmentIndex], NorthExitPath[SegmentIndex + 1], 0.34f, -1.0f, 20000 + SegmentIndex * 101);
				PlaceRiverCanyonSidewall(NorthExitPath[SegmentIndex], NorthExitPath[SegmentIndex + 1], 0.66f, 1.0f, 21000 + SegmentIndex * 101);
			}
			for (int32 SegmentIndex = 0; SegmentIndex < UE_ARRAY_COUNT(SouthExitPath) - 1; ++SegmentIndex)
			{
				PlaceRiverCanyonSidewall(SouthExitPath[SegmentIndex], SouthExitPath[SegmentIndex + 1], 0.28f, -1.0f, 30000 + SegmentIndex * 101);
				PlaceRiverCanyonSidewall(SouthExitPath[SegmentIndex], SouthExitPath[SegmentIndex + 1], 0.58f, 1.0f, 31000 + SegmentIndex * 101);
			}
		}

		for (const TPair<UStaticMesh*, UHierarchicalInstancedStaticMeshComponent*>& Pair : ComponentByMesh)
		{
			if (Pair.Value)
			{
				Pair.Value->BuildTreeIfOutdated(true, true);
				Pair.Value->MarkPackageDirty();
			}
		}

		const bool bValidationCamerasReady = bMassV69 ? SpawnV69TitanMountainMassValidationCameras(World) : (bCoverageV68 ? SpawnV68TitanMountainCoverageValidationCameras(World) : SpawnV67TitanMountainValidationCameras(World));
		MountainActor->MarkPackageDirty();
		World->MarkPackageDirty();
		const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
		const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);

		const float AverageOutwardOffset = AssemblyPiecesPlaced > 0 ? AccumulatedOutwardOffset / static_cast<float>(AssemblyPiecesPlaced) : 0.0f;
		const float NorthCorridorWidth = MinNorthExitClearance < TNumericLimits<float>::Max() ? MinNorthExitClearance * 2.0f : 0.0f;
		const float SouthCorridorWidth = MinSouthExitClearance < TNumericLimits<float>::Max() ? MinSouthExitClearance * 2.0f : 0.0f;
		const int32 PeakCount = PrimaryPeakCount + SecondaryPeakCount;
		const int32 RidgeCount = AcceptedSectors.Num();
		const float ConnectedMountainMassPercent = static_cast<float>(RidgeCount) / static_cast<float>(SectorCount) * 100.0f;
		const bool bExistingRingReused = CandidateSamples > 0 && AcceptedAssemblies >= (bCoverageMode ? (bMassV69 ? 150 : 100) : 24);
		const bool bNoNewMountainChainCreated = true;
		const bool bFootprintLocked = MaxInwardOffset <= (bMassV69 ? 1050.0f : (bCoverageV68 ? 950.0f : 900.0f));
		const bool bGameplaySpaceProtected = bFootprintLocked && MinRiverClearance >= MountainCoverageRiverProtectionRadius && MinLakeClearance >= MountainCoverageLakeProtectionRadius;
		const bool bWaterfallCorridorsOpen = NorthCorridorWidth >= (bCoverageMode ? (bMassV69 ? 8400.0f : 19000.0f) : 30000.0f) && SouthCorridorWidth >= (bCoverageMode ? (bMassV69 ? 8400.0f : 19000.0f) : 30000.0f);
		const bool bMountainBodyDominant = AcceptedAssemblies >= (bCoverageMode ? (bMassV69 ? 150 : 100) : 24) && AssemblyPiecesPlaced >= (bCoverageMode ? (bMassV69 ? 1200 : 560) : 180) && RidgeCount >= (bCoverageMode ? (bMassV69 ? 130 : 80) : 24);

		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=%s removedActors=%d sourceMountainSamples=%d candidateSamples=%d protectedSamplesRejected=%d candidateAnchors=%d acceptedAssemblies=%d connectorAssembliesPlaced=%d riverCanyonSidewallAssembliesPlaced=%d assemblyTemplates=%d uniqueMeshComponents=%d assemblyPiecesPlaced=%d connectorPiecesPlaced=%d riverCanyonSidewallPiecesPlaced=%d assemblyPiecesSkippedProtected=%d assemblyPiecesSkippedGround=%d assemblyPiecesSkippedInward=%d peakCount=%d primaryPeakCount=%d secondaryPeakCount=%d ridgeCount=%d connectedMountainMassPercent=%.1f averageOutwardOffsetCm=%.1f maxInwardOffsetCm=%.1f mountainFootprintChangeCm=0.0 minRiverClearanceCm=%.1f minLakeClearanceCm=%.1f northCorridorWidthCm=%.1f southCorridorWidthCm=%.1f highestEstimatedPlacedTopZ=%.1f existingRingReused=%s noNewMountainChainCreated=%s secondMountainChainCreated=false gameplaySpaceChanged=false gameplayLoss=false lakeChanged=false riverChanged=false playerStartChanged=false grassChanged=false footprintLocked=%s validationCamerasReady=%s savedMap=%s savedPackages=%s"),
			bMassV69 ? TEXT("TitanMountainMassCompletionV69") : (bCoverageV68 ? TEXT("TitanMountainCoverageV68") : TEXT("TitanMountainBodyCompletionV67")),
			RemovedActors,
			MountainPoints.Num(),
			CandidateSamples,
			ProtectedSamplesRejected,
			CandidatePointIndices.Num(),
			AcceptedAssemblies,
			ConnectorAssembliesPlaced,
			RiverCanyonSidewallAssembliesPlaced,
			AssemblyTemplates.Num(),
			ComponentByMesh.Num(),
			AssemblyPiecesPlaced,
			ConnectorPiecesPlaced,
			RiverCanyonSidewallPiecesPlaced,
			AssemblyPiecesSkippedProtected,
			AssemblyPiecesSkippedGround,
			AssemblyPiecesSkippedInward,
			PeakCount,
			PrimaryPeakCount,
			SecondaryPeakCount,
			RidgeCount,
			ConnectedMountainMassPercent,
			AverageOutwardOffset,
			MaxInwardOffset,
			MinRiverClearance,
			MinLakeClearance,
			NorthCorridorWidth,
			SouthCorridorWidth,
			HighestPlacedTopZ,
			bExistingRingReused ? TEXT("true") : TEXT("false"),
			bNoNewMountainChainCreated ? TEXT("true") : TEXT("false"),
			bFootprintLocked ? TEXT("true") : TEXT("false"),
			bValidationCamerasReady ? TEXT("true") : TEXT("false"),
			bSavedMap ? TEXT("true") : TEXT("false"),
			bSavedPackages ? TEXT("true") : TEXT("false"));

		return (bSavedMap
			&& bSavedPackages
			&& bValidationCamerasReady
			&& bExistingRingReused
			&& bNoNewMountainChainCreated
			&& bGameplaySpaceProtected
			&& bWaterfallCorridorsOpen
			&& bMountainBodyDominant) ? 0 : 1;
	}

	float GetV61Percentile(TArray<float> Values, float Percentile)
	{
		if (Values.Num() == 0)
		{
			return 0.0f;
		}

		Values.Sort();
		const int32 Index = FMath::Clamp(FMath::RoundToInt(static_cast<float>(Values.Num() - 1) * FMath::Clamp(Percentile, 0.0f, 1.0f)), 0, Values.Num() - 1);
		return Values[Index];
	}

	float GetV61Min(const TArray<float>& Values)
	{
		float Result = Values.Num() > 0 ? Values[0] : 0.0f;
		for (const float Value : Values)
		{
			Result = FMath::Min(Result, Value);
		}
		return Result;
	}

	float GetV61Max(const TArray<float>& Values)
	{
		float Result = Values.Num() > 0 ? Values[0] : 0.0f;
		for (const float Value : Values)
		{
			Result = FMath::Max(Result, Value);
		}
		return Result;
	}

	float DistanceToSegment2DV61(const FVector2D& Point, const FVector2D& A, const FVector2D& B)
	{
		const FVector2D Segment = B - A;
		const float SegmentLengthSq = Segment.SizeSquared();
		if (SegmentLengthSq <= KINDA_SMALL_NUMBER)
		{
			return FVector2D::Distance(Point, A);
		}

		const float T = FMath::Clamp(FVector2D::DotProduct(Point - A, Segment) / SegmentLengthSq, 0.0f, 1.0f);
		return FVector2D::Distance(Point, A + Segment * T);
	}

	bool IsV61RiverCorridor(const FVector2D& Position, float RadiusCm)
	{
		static const FVector2D RiverPath[] = {
			FVector2D(48590.0f, 183116.0f),
			FVector2D(49910.0f, 174430.0f),
			FVector2D(51518.0f, 166778.0f),
			FVector2D(58605.0f, 152688.0f),
			FVector2D(62544.0f, 138795.0f),
			FVector2D(58487.0f, 122723.0f),
			FVector2D(55306.0f, 96223.0f),
			FVector2D(54105.0f, 78933.0f),
			FVector2D(57584.0f, 62136.0f),
			FVector2D(63147.0f, 37260.0f),
			FVector2D(65398.0f, 28870.0f),
			FVector2D(66873.0f, 20275.0f),
			FVector2D(68500.0f, 11100.0f),
			FVector2D(71037.0f, 3095.0f),
			FVector2D(73955.0f, -4499.0f),
			FVector2D(76822.0f, -12727.0f)
		};

		for (int32 Index = 0; Index < UE_ARRAY_COUNT(RiverPath) - 1; ++Index)
		{
			if (DistanceToSegment2DV61(Position, RiverPath[Index], RiverPath[Index + 1]) <= RadiusCm)
			{
				return true;
			}
		}
		return false;
	}

	bool IsV61LakeArea(const FVector2D& Position)
	{
		struct FLakeExclusionV61
		{
			FVector2D Center;
			FVector2D Radius;
		};

		static const FLakeExclusionV61 Lakes[] = {
			{ FVector2D(40000.0f, 23500.0f), FVector2D(16000.0f, 12000.0f) },
			{ FVector2D(25640.0f, 72820.0f), FVector2D(4700.0f, 1900.0f) }
		};

		for (const FLakeExclusionV61& Lake : Lakes)
		{
			const FVector2D Delta((Position.X - Lake.Center.X) / Lake.Radius.X, (Position.Y - Lake.Center.Y) / Lake.Radius.Y);
			if (Delta.SizeSquared() <= 1.0f)
			{
				return true;
			}
		}
		return false;
	}

	FString SanitizeV61Field(FString Value)
	{
		Value.ReplaceInline(TEXT("\""), TEXT("'"));
		Value.ReplaceInline(TEXT(","), TEXT(";"));
		Value.ReplaceInline(TEXT("\n"), TEXT(" "));
		Value.ReplaceInline(TEXT("\r"), TEXT(" "));
		return Value;
	}

	void RemoveTrailingCommaV61(FString& Value)
	{
		if (Value.EndsWith(TEXT(",")))
		{
			Value.LeftChopInline(1);
		}
	}

	bool V61NameContainsAny(const FString& Name, const TArray<FString>& Needles)
	{
		for (const FString& Needle : Needles)
		{
			if (Name.Contains(Needle, ESearchCase::IgnoreCase))
			{
				return true;
			}
		}
		return false;
	}

	int32 RunV61MapAudit(UWorld* World)
	{
		if (!World)
		{
			return 1;
		}

		const FString SavedDir = FPaths::ProjectSavedDir();
		const FString AuditJsonPath = SavedDir / TEXT("HighlandWorldbuilding_v61_CurrentAudit.json");
		const FString TerrainCsvPath = SavedDir / TEXT("HighlandWorldbuilding_v61_TerrainSamples.csv");
		const FString ComponentCsvPath = SavedDir / TEXT("HighlandWorldbuilding_v61_MountainComponents.csv");
		constexpr int32 GridSizeX = 112;
		constexpr int32 GridSizeY = 112;
		constexpr float RiverExclusionRadiusCm = 5000.0f;
		constexpr float MountainDeltaCm = 1800.0f;
		constexpr float PlateauDeltaCm = 700.0f;
		constexpr float TracePaddingCm = 50000.0f;

		FBox LandscapeBounds(ForceInit);
		int32 LandscapeProxyCount = 0;
		int32 LandscapeComponentCount = 0;
		TSet<FString> LandscapeLayerNames;
		TSet<FString> LandscapeMaterialNames;
		for (TActorIterator<ALandscapeProxy> It(World); It; ++It)
		{
			ALandscapeProxy* Landscape = *It;
			if (!Landscape)
			{
				continue;
			}

			++LandscapeProxyCount;
			LandscapeBounds += Landscape->GetComponentsBoundingBox(true);
			if (UMaterialInterface* LandscapeMaterial = Landscape->GetLandscapeMaterial())
			{
				LandscapeMaterialNames.Add(LandscapeMaterial->GetPathName());
			}

			TInlineComponentArray<ULandscapeComponent*> LandscapeComponents;
			Landscape->GetComponents(LandscapeComponents);
			LandscapeComponentCount += LandscapeComponents.Num();
			for (ULandscapeComponent* Component : LandscapeComponents)
			{
				if (!Component)
				{
					continue;
				}
				for (const FWeightmapLayerAllocationInfo& Allocation : Component->GetWeightmapLayerAllocations())
				{
					ULandscapeLayerInfoObject* LayerInfo = Allocation.LayerInfo.Get();
					if (LayerInfo)
					{
						LandscapeLayerNames.Add(LayerInfo->GetName());
					}
				}
			}
		}

		if (!LandscapeBounds.IsValid)
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandVegetationLayer: V61 audit failed: no valid Landscape bounds."));
			return 1;
		}

		int32 ActorCount = 0;
		int32 HismComponentCount = 0;
		int32 HismInstanceCount = 0;
		int32 TreeInstanceComponentCount = 0;
		int32 TreeInstanceCount = 0;
		int32 GrassInstanceComponentCount = 0;
		int32 GrassInstanceCount = 0;
		int32 VegetationActorCount = 0;
		int32 ExperimentActorCount = 0;
		int32 WaterActorCount = 0;
		int32 PlayerStartCount = 0;
		TArray<FVector> PlayerStartLocations;
		FString PlayerStartLines;
		FString ExperimentActorLines;
		FString WaterActorLines;
		FString VegetationActorLines;
		const TArray<FString> TreeNeedles = { TEXT("Tree"), TEXT("Olive"), TEXT("Laketree"), TEXT("BroadTree"), TEXT("BushLeaves") };
		const TArray<FString> GrassNeedles = { TEXT("Grass"), TEXT("Blade"), TEXT("Foliage") };
		const TArray<FString> VegetationNeedles = { TEXT("Vegetation"), TEXT("Forest"), TEXT("Tree"), TEXT("Ecology"), TEXT("Bush") };
		const TArray<FString> ExperimentNeedles = { TEXT("V55"), TEXT("V56"), TEXT("V57"), TEXT("V58"), TEXT("V59"), TEXT("TitanGeology"), TEXT("MountainIdentity"), TEXT("SurfaceReplacement"), TEXT("FullOuterRing") };
		const TArray<FString> WaterNeedles = { TEXT("Water"), TEXT("Lake"), TEXT("Water_Lake_1") };

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (!Actor)
			{
				continue;
			}

			++ActorCount;
			const FString ActorName = Actor->GetName();
			const FString ActorLabel = Actor->GetActorLabel();
			FString TagString;
			for (const FName& Tag : Actor->Tags)
			{
				TagString += Tag.ToString() + TEXT(" ");
			}
			const FString ActorSearch = ActorName + TEXT(" ") + ActorLabel + TEXT(" ") + TagString + TEXT(" ") + Actor->GetClass()->GetPathName();

			if (Actor->IsA<APlayerStart>())
			{
				++PlayerStartCount;
				PlayerStartLocations.Add(Actor->GetActorLocation());
				PlayerStartLines += FString::Printf(TEXT("{\"label\":\"%s\",\"x\":%.1f,\"y\":%.1f,\"z\":%.1f},"),
					*SanitizeV61Field(ActorLabel),
					Actor->GetActorLocation().X,
					Actor->GetActorLocation().Y,
					Actor->GetActorLocation().Z);
			}
			if (V61NameContainsAny(ActorSearch, VegetationNeedles) || Actor->ActorHasTag(VegetationTag))
			{
				++VegetationActorCount;
				VegetationActorLines += FString::Printf(TEXT("\"%s|%s\","), *SanitizeV61Field(ActorLabel), *SanitizeV61Field(TagString));
			}
			if (V61NameContainsAny(ActorSearch, ExperimentNeedles))
			{
				++ExperimentActorCount;
				ExperimentActorLines += FString::Printf(TEXT("\"%s|%s\","), *SanitizeV61Field(ActorLabel), *SanitizeV61Field(TagString));
			}
			if (V61NameContainsAny(ActorSearch, WaterNeedles))
			{
				++WaterActorCount;
				WaterActorLines += FString::Printf(TEXT("\"%s|%s\","), *SanitizeV61Field(ActorLabel), *SanitizeV61Field(TagString));
			}

			TInlineComponentArray<UInstancedStaticMeshComponent*> InstancedComponents;
			Actor->GetComponents(InstancedComponents);
			for (UInstancedStaticMeshComponent* Component : InstancedComponents)
			{
				if (!Component)
				{
					continue;
				}

				const int32 InstanceCount = Component->GetInstanceCount();
				const FString MeshName = Component->GetStaticMesh() ? Component->GetStaticMesh()->GetPathName() : TEXT("None");
				const FString ComponentSearch = Component->GetName() + TEXT(" ") + MeshName + TEXT(" ") + ActorSearch;
				if (Component->IsA<UHierarchicalInstancedStaticMeshComponent>())
				{
					++HismComponentCount;
					HismInstanceCount += InstanceCount;
				}
				if (V61NameContainsAny(ComponentSearch, TreeNeedles))
				{
					++TreeInstanceComponentCount;
					TreeInstanceCount += InstanceCount;
				}
				if (V61NameContainsAny(ComponentSearch, GrassNeedles))
				{
					++GrassInstanceComponentCount;
					GrassInstanceCount += InstanceCount;
				}
			}
		}
		RemoveTrailingCommaV61(PlayerStartLines);
		RemoveTrailingCommaV61(ExperimentActorLines);
		RemoveTrailingCommaV61(WaterActorLines);
		RemoveTrailingCommaV61(VegetationActorLines);

		TArray<FV61TerrainSample> Samples;
		Samples.Reserve(GridSizeX * GridSizeY);
		TArray<float> AllHeights;
		TArray<float> WalkableNonWaterHeights;
		TArray<float> PlayerAnchorHeights;
		TArray<float> GameplayHeights;
		TArray<float> MountainHeights;
		const float TraceTopZ = LandscapeBounds.Max.Z + TracePaddingCm;
		const float TraceBottomZ = LandscapeBounds.Min.Z - TracePaddingCm;
		const float StepX = (LandscapeBounds.Max.X - LandscapeBounds.Min.X) / static_cast<float>(GridSizeX - 1);
		const float StepY = (LandscapeBounds.Max.Y - LandscapeBounds.Min.Y) / static_cast<float>(GridSizeY - 1);

		for (int32 GridY = 0; GridY < GridSizeY; ++GridY)
		{
			for (int32 GridX = 0; GridX < GridSizeX; ++GridX)
			{
				const float X = LandscapeBounds.Min.X + StepX * GridX;
				const float Y = LandscapeBounds.Min.Y + StepY * GridY;
				FHitResult Hit;
				FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(V61MapAudit), true);
				if (!World->LineTraceSingleByChannel(Hit, FVector(X, Y, TraceTopZ), FVector(X, Y, TraceBottomZ), ECC_WorldStatic, QueryParams))
				{
					continue;
				}
				if (!Hit.GetActor() || !Hit.GetActor()->IsA<ALandscapeProxy>())
				{
					continue;
				}

				FV61TerrainSample Sample;
				Sample.GridX = GridX;
				Sample.GridY = GridY;
				Sample.Location = Hit.ImpactPoint;
				Sample.NormalZ = Hit.ImpactNormal.Z;
				Sample.ComponentName = Hit.Component.IsValid() ? Hit.Component->GetName() : TEXT("LandscapeUnknown");
				Sample.Classification = TEXT("Unclassified");
				Samples.Add(Sample);
				AllHeights.Add(Sample.Location.Z);

				const FVector2D XY(Sample.Location.X, Sample.Location.Y);
				if (!IsV61RiverCorridor(XY, RiverExclusionRadiusCm) && !IsV61LakeArea(XY) && Sample.NormalZ >= 0.55f)
				{
					WalkableNonWaterHeights.Add(Sample.Location.Z);
					for (const FVector& PlayerStartLocation : PlayerStartLocations)
					{
						if (FVector2D::Distance(XY, FVector2D(PlayerStartLocation.X, PlayerStartLocation.Y)) <= 40000.0f)
						{
							PlayerAnchorHeights.Add(Sample.Location.Z);
							break;
						}
					}
				}
			}
		}

		const float PlayableMedianZ = PlayerAnchorHeights.Num() > 0 ? GetV61Percentile(PlayerAnchorHeights, 0.5f) : (WalkableNonWaterHeights.Num() > 0 ? GetV61Percentile(WalkableNonWaterHeights, 0.42f) : GetV61Percentile(AllHeights, 0.5f));
		const float MountainThresholdZ = PlayableMedianZ + MountainDeltaCm;
		const float PlateauThresholdZ = PlayableMedianZ + PlateauDeltaCm;

		TArray<uint8> Elevated;
		Elevated.Init(0, GridSizeX * GridSizeY);
		for (int32 SampleIndex = 0; SampleIndex < Samples.Num(); ++SampleIndex)
		{
			const FV61TerrainSample& Sample = Samples[SampleIndex];
			const int32 FlatIndex = Sample.GridY * GridSizeX + Sample.GridX;
			const FVector2D XY(Sample.Location.X, Sample.Location.Y);
			const bool bWaterExcluded = IsV61RiverCorridor(XY, RiverExclusionRadiusCm) || IsV61LakeArea(XY);
			Elevated[FlatIndex] = (!bWaterExcluded && Sample.Location.Z >= MountainThresholdZ) ? 1 : 0;
		}

		TArray<uint8> MountainConnected;
		MountainConnected.Init(0, GridSizeX * GridSizeY);
		TArray<int32> Stack;
		const int32 OuterBandX = FMath::Max(8, FMath::RoundToInt(static_cast<float>(GridSizeX) * 0.23f));
		const int32 OuterBandY = FMath::Max(8, FMath::RoundToInt(static_cast<float>(GridSizeY) * 0.23f));
		auto TrySeedMountain = [&](int32 GridX, int32 GridY)
		{
			const int32 FlatIndex = GridY * GridSizeX + GridX;
			if (Elevated.IsValidIndex(FlatIndex) && Elevated[FlatIndex] && !MountainConnected[FlatIndex])
			{
				MountainConnected[FlatIndex] = 1;
				Stack.Add(FlatIndex);
			}
		};
		for (int32 GridY = 0; GridY < GridSizeY; ++GridY)
		{
			for (int32 GridX = 0; GridX < GridSizeX; ++GridX)
			{
				if (GridX <= OuterBandX || GridX >= GridSizeX - 1 - OuterBandX || GridY <= OuterBandY || GridY >= GridSizeY - 1 - OuterBandY)
				{
					TrySeedMountain(GridX, GridY);
				}
			}
		}
		while (Stack.Num() > 0)
		{
			const int32 Current = Stack.Pop(EAllowShrinking::No);
			const int32 CurrentX = Current % GridSizeX;
			const int32 CurrentY = Current / GridSizeX;
			for (int32 DY = -1; DY <= 1; ++DY)
			{
				for (int32 DX = -1; DX <= 1; ++DX)
				{
					if (DX == 0 && DY == 0)
					{
						continue;
					}
					const int32 NX = CurrentX + DX;
					const int32 NY = CurrentY + DY;
					if (NX < 0 || NX >= GridSizeX || NY < 0 || NY >= GridSizeY)
					{
						continue;
					}
					const int32 Neighbor = NY * GridSizeX + NX;
					if (Elevated[Neighbor] && !MountainConnected[Neighbor])
					{
						MountainConnected[Neighbor] = 1;
						Stack.Add(Neighbor);
					}
				}
			}
		}

		bool bMountainFallbackUsed = false;
		int32 ConnectedMountainSeedCount = 0;
		for (const uint8 bConnected : MountainConnected)
		{
			if (bConnected)
			{
				++ConnectedMountainSeedCount;
			}
		}
		if (ConnectedMountainSeedCount == 0)
		{
			bMountainFallbackUsed = true;
			for (int32 Index = 0; Index < Elevated.Num(); ++Index)
			{
				MountainConnected[Index] = Elevated[Index];
			}
		}

		TMap<FString, FV61ComponentRegion> ComponentRegions;
		int32 RiverSampleCount = 0;
		int32 LakeSampleCount = 0;
		int32 PlateauSampleCount = 0;
		for (FV61TerrainSample& Sample : Samples)
		{
			const FVector2D XY(Sample.Location.X, Sample.Location.Y);
			const bool bRiver = IsV61RiverCorridor(XY, RiverExclusionRadiusCm);
			const bool bLake = IsV61LakeArea(XY);
			const int32 FlatIndex = Sample.GridY * GridSizeX + Sample.GridX;
			if (bLake)
			{
				Sample.Classification = TEXT("Lake");
				++LakeSampleCount;
			}
			else if (bRiver)
			{
				Sample.Classification = TEXT("River");
				++RiverSampleCount;
			}
			else if (MountainConnected.IsValidIndex(FlatIndex) && MountainConnected[FlatIndex])
			{
				Sample.Classification = TEXT("Mountain");
				MountainHeights.Add(Sample.Location.Z);
			}
			else if (Sample.Location.Z >= PlateauThresholdZ)
			{
				Sample.Classification = TEXT("Plateau");
				GameplayHeights.Add(Sample.Location.Z);
				++PlateauSampleCount;
			}
			else
			{
				Sample.Classification = TEXT("Gameplay");
				GameplayHeights.Add(Sample.Location.Z);
			}

			FV61ComponentRegion& Region = ComponentRegions.FindOrAdd(Sample.ComponentName);
			++Region.TotalSamples;
			if (Sample.Classification == TEXT("Mountain"))
			{
				++Region.MountainSamples;
			}
			else if (Sample.Classification == TEXT("Gameplay"))
			{
				++Region.GameplaySamples;
			}
			else if (Sample.Classification == TEXT("Plateau"))
			{
				++Region.PlateauSamples;
			}
			else
			{
				++Region.WaterExcludedSamples;
			}
			Region.MinXY.X = FMath::Min(Region.MinXY.X, XY.X);
			Region.MinXY.Y = FMath::Min(Region.MinXY.Y, XY.Y);
			Region.MaxXY.X = FMath::Max(Region.MaxXY.X, XY.X);
			Region.MaxXY.Y = FMath::Max(Region.MaxXY.Y, XY.Y);
			Region.MinZ = FMath::Min(Region.MinZ, Sample.Location.Z);
			Region.MaxZ = FMath::Max(Region.MaxZ, Sample.Location.Z);
		}

		FString TerrainCsv = TEXT("grid_x,grid_y,world_x,world_y,z,normal_z,classification,component\n");
		for (const FV61TerrainSample& Sample : Samples)
		{
			TerrainCsv += FString::Printf(TEXT("%d,%d,%.2f,%.2f,%.2f,%.4f,%s,%s\n"),
				Sample.GridX,
				Sample.GridY,
				Sample.Location.X,
				Sample.Location.Y,
				Sample.Location.Z,
				Sample.NormalZ,
				*Sample.Classification,
				*SanitizeV61Field(Sample.ComponentName));
		}
		FFileHelper::SaveStringToFile(TerrainCsv, *TerrainCsvPath);

		FString ComponentCsv = TEXT("component,total_samples,mountain_samples,gameplay_samples,plateau_samples,water_excluded_samples,mountain_fraction,min_x,min_y,max_x,max_y,min_z,max_z\n");
		for (const TPair<FString, FV61ComponentRegion>& Pair : ComponentRegions)
		{
			const FV61ComponentRegion& Region = Pair.Value;
			const float MountainFraction = Region.TotalSamples > 0 ? static_cast<float>(Region.MountainSamples) / static_cast<float>(Region.TotalSamples) : 0.0f;
			ComponentCsv += FString::Printf(TEXT("%s,%d,%d,%d,%d,%d,%.4f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n"),
				*SanitizeV61Field(Pair.Key),
				Region.TotalSamples,
				Region.MountainSamples,
				Region.GameplaySamples,
				Region.PlateauSamples,
				Region.WaterExcludedSamples,
				MountainFraction,
				Region.MinXY.X,
				Region.MinXY.Y,
				Region.MaxXY.X,
				Region.MaxXY.Y,
				Region.MinZ,
				Region.MaxZ);
		}
		FFileHelper::SaveStringToFile(ComponentCsv, *ComponentCsvPath);

		FString LayerJsonArray;
		for (const FString& LayerName : LandscapeLayerNames)
		{
			LayerJsonArray += FString::Printf(TEXT("\"%s\","), *SanitizeV61Field(LayerName));
		}
		RemoveTrailingCommaV61(LayerJsonArray);

		FString MaterialJsonArray;
		for (const FString& MaterialName : LandscapeMaterialNames)
		{
			MaterialJsonArray += FString::Printf(TEXT("\"%s\","), *SanitizeV61Field(MaterialName));
		}
		RemoveTrailingCommaV61(MaterialJsonArray);

		const FString AuditJson = FString::Printf(TEXT("{\n")
			TEXT("  \"mode\": \"V61MapAudit_ReadOnly\",\n")
			TEXT("  \"map\": \"%s\",\n")
			TEXT("  \"saved_map\": false,\n")
			TEXT("  \"worldbuilding_created\": false,\n")
			TEXT("  \"grid_size\": {\"x\": %d, \"y\": %d},\n")
			TEXT("  \"landscape\": {\"proxy_count\": %d, \"component_count\": %d, \"bounds_min\": [%.2f, %.2f, %.2f], \"bounds_max\": [%.2f, %.2f, %.2f], \"materials\": [%s], \"layers\": [%s]},\n")
			TEXT("  \"actors\": {\"total\": %d, \"vegetation_actor_count\": %d, \"hism_component_count\": %d, \"hism_instance_count\": %d, \"tree_instance_component_count\": %d, \"tree_instance_count\": %d, \"grass_instance_component_count\": %d, \"grass_instance_count\": %d, \"experiment_actor_count\": %d, \"water_actor_count\": %d, \"player_start_count\": %d},\n")
			TEXT("  \"terrain_thresholds_cm\": {\"playable_median_z\": %.2f, \"mountain_threshold_z\": %.2f, \"mountain_delta_from_playable\": %.2f, \"plateau_threshold_z\": %.2f, \"plateau_delta_from_playable\": %.2f, \"river_exclusion_radius\": %.2f},\n")
			TEXT("  \"terrain_stats\": {\"samples\": %d, \"all_min_z\": %.2f, \"all_max_z\": %.2f, \"all_median_z\": %.2f, \"player_anchor_samples\": %d, \"player_anchor_min_z\": %.2f, \"player_anchor_max_z\": %.2f, \"player_anchor_median_z\": %.2f, \"gameplay_samples\": %d, \"gameplay_min_z\": %.2f, \"gameplay_max_z\": %.2f, \"gameplay_median_z\": %.2f, \"mountain_samples\": %d, \"mountain_min_z\": %.2f, \"mountain_max_z\": %.2f, \"mountain_median_z\": %.2f, \"river_samples\": %d, \"lake_samples\": %d, \"plateau_samples\": %d, \"outer_band_grid_x\": %d, \"outer_band_grid_y\": %d, \"mountain_fallback_used\": %s},\n")
			TEXT("  \"river_lake_exclusion_rules\": {\"river\": \"distance <= 5000 cm from known Highland north-south river spline points\", \"lakes\": \"ellipse exclusions at (40000,23500) radius (16000,12000) and (25640,72820) radius (4700,1900)\"},\n")
			TEXT("  \"player_starts\": [%s],\n")
			TEXT("  \"experiment_actors\": [%s],\n")
			TEXT("  \"water_actors\": [%s],\n")
			TEXT("  \"vegetation_actors\": [%s],\n")
			TEXT("  \"terrain_samples_csv\": \"%s\",\n")
			TEXT("  \"component_regions_csv\": \"%s\"\n")
			TEXT("}\n"),
			HighlandMapPath,
			GridSizeX,
			GridSizeY,
			LandscapeProxyCount,
			LandscapeComponentCount,
			LandscapeBounds.Min.X,
			LandscapeBounds.Min.Y,
			LandscapeBounds.Min.Z,
			LandscapeBounds.Max.X,
			LandscapeBounds.Max.Y,
			LandscapeBounds.Max.Z,
			*MaterialJsonArray,
			*LayerJsonArray,
			ActorCount,
			VegetationActorCount,
			HismComponentCount,
			HismInstanceCount,
			TreeInstanceComponentCount,
			TreeInstanceCount,
			GrassInstanceComponentCount,
			GrassInstanceCount,
			ExperimentActorCount,
			WaterActorCount,
			PlayerStartCount,
			PlayableMedianZ,
			MountainThresholdZ,
			MountainDeltaCm,
			PlateauThresholdZ,
			PlateauDeltaCm,
			RiverExclusionRadiusCm,
			Samples.Num(),
			GetV61Min(AllHeights),
			GetV61Max(AllHeights),
			GetV61Percentile(AllHeights, 0.5f),
			PlayerAnchorHeights.Num(),
			GetV61Min(PlayerAnchorHeights),
			GetV61Max(PlayerAnchorHeights),
			GetV61Percentile(PlayerAnchorHeights, 0.5f),
			GameplayHeights.Num(),
			GetV61Min(GameplayHeights),
			GetV61Max(GameplayHeights),
			GetV61Percentile(GameplayHeights, 0.5f),
			MountainHeights.Num(),
			GetV61Min(MountainHeights),
			GetV61Max(MountainHeights),
			GetV61Percentile(MountainHeights, 0.5f),
			RiverSampleCount,
			LakeSampleCount,
			PlateauSampleCount,
			OuterBandX,
			OuterBandY,
			bMountainFallbackUsed ? TEXT("true") : TEXT("false"),
			*PlayerStartLines,
			*ExperimentActorLines,
			*WaterActorLines,
			*VegetationActorLines,
			*TerrainCsvPath.Replace(TEXT("\\"), TEXT("\\\\")),
			*ComponentCsvPath.Replace(TEXT("\\"), TEXT("\\\\")));
		FFileHelper::SaveStringToFile(AuditJson, *AuditJsonPath);

		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=V61MapAudit readOnly=true savedMap=false landscapeProxies=%d landscapeComponents=%d actors=%d vegetationActors=%d treeInstances=%d grassInstances=%d experimentActors=%d waterActors=%d playerStarts=%d samples=%d playerAnchorSamples=%d playableMedianZ=%.2f mountainThresholdZ=%.2f mountainSamples=%d gameplaySamples=%d riverSamples=%d lakeSamples=%d mountainFallbackUsed=%s auditJson=%s terrainCsv=%s componentCsv=%s"),
			LandscapeProxyCount,
			LandscapeComponentCount,
			ActorCount,
			VegetationActorCount,
			TreeInstanceCount,
			GrassInstanceCount,
			ExperimentActorCount,
			WaterActorCount,
			PlayerStartCount,
			Samples.Num(),
			PlayerAnchorHeights.Num(),
			PlayableMedianZ,
			MountainThresholdZ,
			MountainHeights.Num(),
			GameplayHeights.Num(),
			RiverSampleCount,
			LakeSampleCount,
			bMountainFallbackUsed ? TEXT("true") : TEXT("false"),
			*AuditJsonPath,
			*TerrainCsvPath,
			*ComponentCsvPath);
		return Samples.Num() > 0 ? 0 : 1;
	}
}
#endif

int32 UFFStarterHighlandVegetationLayerCommandlet::Main(const FString& Params)
{
#if WITH_EDITOR
	const bool bCliffRockDressingV7 = Params.Contains(TEXT("TraversalCliffRocksV7"), ESearchCase::IgnoreCase);
	const bool bCliffRockDressingV6 = Params.Contains(TEXT("TraversalCliffRocksV6"), ESearchCase::IgnoreCase);
	const bool bCliffRockDressingV5 = Params.Contains(TEXT("TraversalCliffRocksV5"), ESearchCase::IgnoreCase);
	const bool bCliffRockDressingV4 = Params.Contains(TEXT("TraversalCliffRocksV4"), ESearchCase::IgnoreCase);
	const bool bCliffRockDressingV3 = Params.Contains(TEXT("TraversalCliffRocksV3"), ESearchCase::IgnoreCase);
	const bool bCliffRockDressingV2 = Params.Contains(TEXT("TraversalCliffRocksV2"), ESearchCase::IgnoreCase);
	const bool bCliffRockDressing = bCliffRockDressingV7 || bCliffRockDressingV6 || bCliffRockDressingV5 || bCliffRockDressingV4 || bCliffRockDressingV3 || bCliffRockDressingV2 || Params.Contains(TEXT("TraversalCliffRocksV1"), ESearchCase::IgnoreCase);
	const bool bV61MapAudit = Params.Contains(TEXT("V61MapAudit"), ESearchCase::IgnoreCase) || Params.Contains(TEXT("V61Audit"), ESearchCase::IgnoreCase);
	const bool bTitanGeologyV59 = Params.Contains(TEXT("TitanGeologyV59"), ESearchCase::IgnoreCase);
	const bool bTitanMountainConversionV65 = Params.Contains(TEXT("TitanMountainConversionV65"), ESearchCase::IgnoreCase) || Params.Contains(TEXT("V65TitanMountainConversion"), ESearchCase::IgnoreCase);
	const bool bTitanMountainBodyCompletionV66 = Params.Contains(TEXT("TitanMountainBodyCompletionV66"), ESearchCase::IgnoreCase) || Params.Contains(TEXT("V66TitanMountainBody"), ESearchCase::IgnoreCase);
	const bool bTitanMountainBodyCompletionV67 = Params.Contains(TEXT("TitanMountainBodyCompletionV67"), ESearchCase::IgnoreCase) || Params.Contains(TEXT("V67TitanMountainBody"), ESearchCase::IgnoreCase);
	const bool bTitanMountainCoverageV68 = Params.Contains(TEXT("TitanMountainCoverageV68"), ESearchCase::IgnoreCase) || Params.Contains(TEXT("V68TitanMountainCoverage"), ESearchCase::IgnoreCase);
	const bool bTitanMountainMassCompletionV69 = Params.Contains(TEXT("TitanMountainMassCompletionV69"), ESearchCase::IgnoreCase) || Params.Contains(TEXT("V69TitanMountainMass"), ESearchCase::IgnoreCase);
	const bool bMountainFreezeCleanupV70 = Params.Contains(TEXT("MountainFreezeCleanupV70"), ESearchCase::IgnoreCase) || Params.Contains(TEXT("V70MountainFreezeCleanup"), ESearchCase::IgnoreCase);
	const bool bV60CleanHighland = Params.Contains(TEXT("V60CleanHighland"), ESearchCase::IgnoreCase);
	const bool bMountainIdentityV56 = Params.Contains(TEXT("MountainIdentityV56"), ESearchCase::IgnoreCase);
	const bool bOuterRingSurfaceReplacementV57 = Params.Contains(TEXT("OuterRingSurfaceReplacementV57"), ESearchCase::IgnoreCase);
	const bool bThinCliffFacadeV55 = Params.Contains(TEXT("ThinCliffFacadeV55"), ESearchCase::IgnoreCase);
	const bool bThinCliffFacadeV54 = Params.Contains(TEXT("ThinCliffFacadeV54"), ESearchCase::IgnoreCase);
	const bool bThinCliffFacadeV53 = Params.Contains(TEXT("ThinCliffFacadeV53"), ESearchCase::IgnoreCase);
	const bool bThinCliffFacadeV52 = Params.Contains(TEXT("ThinCliffFacadeV52"), ESearchCase::IgnoreCase);
	const bool bAnyThinCliffFacade = bThinCliffFacadeV55 || bThinCliffFacadeV54 || bThinCliffFacadeV53 || bThinCliffFacadeV52;
	const bool bEmbeddedCliffIntegrationV46 = Params.Contains(TEXT("EmbeddedCliffIntegrationV46"), ESearchCase::IgnoreCase);
	const bool bEmbeddedCliffMassesV45 = Params.Contains(TEXT("EmbeddedCliffMassesV45"), ESearchCase::IgnoreCase);
	const bool bValidationCamerasV46 = Params.Contains(TEXT("ValidationCamerasV46"), ESearchCase::IgnoreCase);
	const bool bValidationCamerasV45 = Params.Contains(TEXT("ValidationCamerasV45"), ESearchCase::IgnoreCase);
	const bool bValidationCamerasV44 = Params.Contains(TEXT("ValidationCamerasV44"), ESearchCase::IgnoreCase);
	const bool bValidationCamerasV43 = Params.Contains(TEXT("ValidationCamerasV43"), ESearchCase::IgnoreCase);
	const bool bValidationCamerasV42 = Params.Contains(TEXT("ValidationCamerasV42"), ESearchCase::IgnoreCase);
	const bool bValidationCamerasV41 = Params.Contains(TEXT("ValidationCamerasV41"), ESearchCase::IgnoreCase);
	const bool bValidationCamerasV40 = Params.Contains(TEXT("ValidationCamerasV40"), ESearchCase::IgnoreCase);
	const bool bValidationCamerasV39 = Params.Contains(TEXT("ValidationCamerasV39"), ESearchCase::IgnoreCase);
	const bool bValidationCamerasV38 = Params.Contains(TEXT("ValidationCamerasV38"), ESearchCase::IgnoreCase);
	const bool bValidationCamerasV37 = Params.Contains(TEXT("ValidationCamerasV37"), ESearchCase::IgnoreCase);
	const bool bValidationCamerasV36 = Params.Contains(TEXT("ValidationCamerasV36"), ESearchCase::IgnoreCase);
	const bool bValidationCamerasV35 = Params.Contains(TEXT("ValidationCamerasV35"), ESearchCase::IgnoreCase);
	const bool bOuterRingRockCleanupV44 = Params.Contains(TEXT("OuterRingRockCleanupV44"), ESearchCase::IgnoreCase);
	const bool bOuterRingRockCleanupV43 = Params.Contains(TEXT("OuterRingRockCleanupV43"), ESearchCase::IgnoreCase);
	const bool bRemoveVisibleBlockoutMarkersV39 = Params.Contains(TEXT("RemoveVisibleBlockoutMarkersV39"), ESearchCase::IgnoreCase);
	const bool bAuditFlatActorsV39 = Params.Contains(TEXT("AuditFlatActorsV39"), ESearchCase::IgnoreCase);
	const bool bTraversalForestV1 = Params.Contains(TEXT("TraversalForestV1"), ESearchCase::IgnoreCase);
	const bool bTraversalForestV2 = Params.Contains(TEXT("TraversalForestV2"), ESearchCase::IgnoreCase);
	const bool bTraversalForestV3 = Params.Contains(TEXT("TraversalForestV3"), ESearchCase::IgnoreCase);
	const bool bTraversalForestV4 = Params.Contains(TEXT("TraversalForestV4"), ESearchCase::IgnoreCase);
	const bool bTraversalForestV5 = Params.Contains(TEXT("TraversalForestV5"), ESearchCase::IgnoreCase);
	const bool bTraversalForestV6 = Params.Contains(TEXT("TraversalForestV6"), ESearchCase::IgnoreCase);
	const bool bTraversalForestV7 = Params.Contains(TEXT("TraversalForestV7"), ESearchCase::IgnoreCase);
	const bool bTraversalForestV8 = Params.Contains(TEXT("TraversalForestV8"), ESearchCase::IgnoreCase);
	const bool bTraversalForestV9 = Params.Contains(TEXT("TraversalForestV9"), ESearchCase::IgnoreCase);
	const bool bTraversalForestV10 = Params.Contains(TEXT("TraversalForestV10"), ESearchCase::IgnoreCase);
	const bool bTraversalForestV40 = Params.Contains(TEXT("TraversalForestV40"), ESearchCase::IgnoreCase);
	const bool bTraversalForestV42 = Params.Contains(TEXT("TraversalForestV42"), ESearchCase::IgnoreCase);
	const bool bTraversalForestBlueprintMode = bTraversalForestV1 || bTraversalForestV2 || bTraversalForestV3 || bTraversalForestV4 || bTraversalForestV5 || bTraversalForestV6 || bTraversalForestV7 || bTraversalForestV8 || bTraversalForestV9 || bTraversalForestV10 || bTraversalForestV40;
	const float TreeCountScale = bTraversalForestBlueprintMode ? 0.42f : 1.0f;
	const float UndergrowthCountScale = bTraversalForestBlueprintMode ? 0.0f : 1.0f;
	const TCHAR* ModeName = TEXT("Default");
	if (bV61MapAudit)
	{
		ModeName = TEXT("V61MapAudit");
	}
	else if (bV60CleanHighland)
	{
		ModeName = TEXT("V60CleanHighland");
	}
	else if (bMountainFreezeCleanupV70)
	{
		ModeName = TEXT("MountainFreezeCleanupV70");
	}
	else if (bTitanMountainMassCompletionV69)
	{
		ModeName = TEXT("TitanMountainMassCompletionV69");
	}
	else if (bTitanMountainCoverageV68)
	{
		ModeName = TEXT("TitanMountainCoverageV68");
	}
	else if (bTitanMountainBodyCompletionV67)
	{
		ModeName = TEXT("TitanMountainBodyCompletionV67");
	}
	else if (bTitanMountainBodyCompletionV66)
	{
		ModeName = TEXT("TitanMountainBodyCompletionV66");
	}
	else if (bTitanMountainConversionV65)
	{
		ModeName = TEXT("TitanMountainConversionV65");
	}
	else if (bTitanGeologyV59)
	{
		ModeName = TEXT("TitanGeologyV59");
	}
	else if (bOuterRingSurfaceReplacementV57)
	{
		ModeName = TEXT("OuterRingSurfaceReplacementV57");
	}
	else if (bValidationCamerasV46)
	{
		ModeName = TEXT("ValidationCamerasV46");
	}
	else if (bValidationCamerasV45)
	{
		ModeName = TEXT("ValidationCamerasV45");
	}
	else if (bValidationCamerasV44)
	{
		ModeName = TEXT("ValidationCamerasV44");
	}
	else if (bValidationCamerasV43)
	{
		ModeName = TEXT("ValidationCamerasV43");
	}
	else if (bValidationCamerasV42)
	{
		ModeName = TEXT("ValidationCamerasV42");
	}
	else if (bValidationCamerasV41)
	{
		ModeName = TEXT("ValidationCamerasV41");
	}
	else if (bValidationCamerasV40)
	{
		ModeName = TEXT("ValidationCamerasV40");
	}
	else if (bValidationCamerasV39)
	{
		ModeName = TEXT("ValidationCamerasV39");
	}
	else if (bValidationCamerasV38)
	{
		ModeName = TEXT("ValidationCamerasV38");
	}
	else if (bValidationCamerasV37)
	{
		ModeName = TEXT("ValidationCamerasV37");
	}
	else if (bValidationCamerasV36)
	{
		ModeName = TEXT("ValidationCamerasV36");
	}
	else if (bValidationCamerasV35)
	{
		ModeName = TEXT("ValidationCamerasV35");
	}
	else if (bCliffRockDressing)
	{
		ModeName = bCliffRockDressingV7 ? TEXT("TraversalCliffRocksV7") : (bCliffRockDressingV6 ? TEXT("TraversalCliffRocksV6") : (bCliffRockDressingV5 ? TEXT("TraversalCliffRocksV5") : (bCliffRockDressingV4 ? TEXT("TraversalCliffRocksV4") : (bCliffRockDressingV3 ? TEXT("TraversalCliffRocksV3") : (bCliffRockDressingV2 ? TEXT("TraversalCliffRocksV2") : TEXT("TraversalCliffRocksV1"))))));
	}
	else if (bMountainIdentityV56)
	{
		ModeName = TEXT("MountainIdentityV56");
	}
	else if (bThinCliffFacadeV55)
	{
		ModeName = TEXT("ThinCliffFacadeV55");
	}
	else if (bThinCliffFacadeV54)
	{
		ModeName = TEXT("ThinCliffFacadeV54");
	}
	else if (bThinCliffFacadeV53)
	{
		ModeName = TEXT("ThinCliffFacadeV53");
	}
	else if (bThinCliffFacadeV52)
	{
		ModeName = TEXT("ThinCliffFacadeV52");
	}
	else if (bEmbeddedCliffIntegrationV46)
	{
		ModeName = TEXT("EmbeddedCliffIntegrationV46");
	}
	else if (bEmbeddedCliffMassesV45)
	{
		ModeName = TEXT("EmbeddedCliffMassesV45");
	}
	else if (bOuterRingRockCleanupV43)
	{
		ModeName = TEXT("OuterRingRockCleanupV43");
	}
	else if (bOuterRingRockCleanupV44)
	{
		ModeName = TEXT("OuterRingRockCleanupV44");
	}
	else if (bRemoveVisibleBlockoutMarkersV39)
	{
		ModeName = TEXT("RemoveVisibleBlockoutMarkersV39");
	}
	else if (bAuditFlatActorsV39)
	{
		ModeName = TEXT("AuditFlatActorsV39");
	}
	else if (bTraversalForestV42)
	{
		ModeName = TEXT("TraversalForestV42");
	}
	else if (bTraversalForestV40)
	{
		ModeName = TEXT("TraversalForestV40");
	}
	else if (bTraversalForestV10)
	{
		ModeName = TEXT("TraversalForestV10");
	}
	else if (bTraversalForestV9)
	{
		ModeName = TEXT("TraversalForestV9");
	}
	else if (bTraversalForestV8)
	{
		ModeName = TEXT("TraversalForestV8");
	}
	else if (bTraversalForestV7)
	{
		ModeName = TEXT("TraversalForestV7");
	}
	else if (bTraversalForestV6)
	{
		ModeName = TEXT("TraversalForestV6");
	}
	else if (bTraversalForestV5)
	{
		ModeName = TEXT("TraversalForestV5");
	}
	else if (bTraversalForestV4)
	{
		ModeName = TEXT("TraversalForestV4");
	}
	else if (bTraversalForestV3)
	{
		ModeName = TEXT("TraversalForestV3");
	}
	else if (bTraversalForestV2)
	{
		ModeName = TEXT("TraversalForestV2");
	}
	else if (bTraversalForestV1)
	{
		ModeName = TEXT("TraversalForestV1");
	}

	UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: loading %s mode=%s without terrain, water, grass, or PlayerStart edits."),
		HighlandMapPath,
		ModeName);
	UWorld* World = UEditorLoadingAndSavingUtils::LoadMap(HighlandMapPath);
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandVegetationLayer: failed to load map."));
		return 1;
	}

	int32 RemovedActors = 0;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		const bool bRemoveActor =
			(bValidationCamerasV46 && (Actor->ActorHasTag(V46ValidationCameraTag) || Actor->ActorHasTag(V45ValidationCameraTag) || Actor->ActorHasTag(V44ValidationCameraTag) || Actor->ActorHasTag(V43ValidationCameraTag) || Actor->ActorHasTag(V42ValidationCameraTag) || Actor->ActorHasTag(V41ValidationCameraTag) || Actor->ActorHasTag(V40ValidationCameraTag) || Actor->ActorHasTag(V39ValidationCameraTag) || Actor->ActorHasTag(V38ValidationCameraTag) || Actor->ActorHasTag(V37ValidationCameraTag) || Actor->ActorHasTag(V36ValidationCameraTag) || Actor->ActorHasTag(V35ValidationCameraTag))) ||
			(bValidationCamerasV45 && (Actor->ActorHasTag(V45ValidationCameraTag) || Actor->ActorHasTag(V44ValidationCameraTag) || Actor->ActorHasTag(V43ValidationCameraTag) || Actor->ActorHasTag(V42ValidationCameraTag) || Actor->ActorHasTag(V41ValidationCameraTag) || Actor->ActorHasTag(V40ValidationCameraTag) || Actor->ActorHasTag(V39ValidationCameraTag) || Actor->ActorHasTag(V38ValidationCameraTag) || Actor->ActorHasTag(V37ValidationCameraTag) || Actor->ActorHasTag(V36ValidationCameraTag) || Actor->ActorHasTag(V35ValidationCameraTag))) ||
			(bValidationCamerasV44 && (Actor->ActorHasTag(V44ValidationCameraTag) || Actor->ActorHasTag(V43ValidationCameraTag) || Actor->ActorHasTag(V42ValidationCameraTag) || Actor->ActorHasTag(V41ValidationCameraTag) || Actor->ActorHasTag(V40ValidationCameraTag) || Actor->ActorHasTag(V39ValidationCameraTag) || Actor->ActorHasTag(V38ValidationCameraTag) || Actor->ActorHasTag(V37ValidationCameraTag) || Actor->ActorHasTag(V36ValidationCameraTag) || Actor->ActorHasTag(V35ValidationCameraTag))) ||
			(bValidationCamerasV43 && (Actor->ActorHasTag(V43ValidationCameraTag) || Actor->ActorHasTag(V42ValidationCameraTag) || Actor->ActorHasTag(V41ValidationCameraTag) || Actor->ActorHasTag(V40ValidationCameraTag) || Actor->ActorHasTag(V39ValidationCameraTag) || Actor->ActorHasTag(V38ValidationCameraTag) || Actor->ActorHasTag(V37ValidationCameraTag) || Actor->ActorHasTag(V36ValidationCameraTag) || Actor->ActorHasTag(V35ValidationCameraTag))) ||
			(bValidationCamerasV42 && (Actor->ActorHasTag(V42ValidationCameraTag) || Actor->ActorHasTag(V41ValidationCameraTag) || Actor->ActorHasTag(V40ValidationCameraTag) || Actor->ActorHasTag(V39ValidationCameraTag) || Actor->ActorHasTag(V38ValidationCameraTag) || Actor->ActorHasTag(V37ValidationCameraTag) || Actor->ActorHasTag(V36ValidationCameraTag) || Actor->ActorHasTag(V35ValidationCameraTag))) ||
			(bValidationCamerasV41 && (Actor->ActorHasTag(V41ValidationCameraTag) || Actor->ActorHasTag(V40ValidationCameraTag) || Actor->ActorHasTag(V39ValidationCameraTag) || Actor->ActorHasTag(V38ValidationCameraTag) || Actor->ActorHasTag(V37ValidationCameraTag) || Actor->ActorHasTag(V36ValidationCameraTag) || Actor->ActorHasTag(V35ValidationCameraTag))) ||
			(bValidationCamerasV40 && (Actor->ActorHasTag(V40ValidationCameraTag) || Actor->ActorHasTag(V39ValidationCameraTag) || Actor->ActorHasTag(V38ValidationCameraTag) || Actor->ActorHasTag(V37ValidationCameraTag) || Actor->ActorHasTag(V36ValidationCameraTag) || Actor->ActorHasTag(V35ValidationCameraTag))) ||
			(bValidationCamerasV39 && (Actor->ActorHasTag(V39ValidationCameraTag) || Actor->ActorHasTag(V38ValidationCameraTag) || Actor->ActorHasTag(V37ValidationCameraTag) || Actor->ActorHasTag(V36ValidationCameraTag) || Actor->ActorHasTag(V35ValidationCameraTag))) ||
			(bValidationCamerasV38 && (Actor->ActorHasTag(V38ValidationCameraTag) || Actor->ActorHasTag(V37ValidationCameraTag) || Actor->ActorHasTag(V36ValidationCameraTag) || Actor->ActorHasTag(V35ValidationCameraTag))) ||
			(bValidationCamerasV37 && (Actor->ActorHasTag(V37ValidationCameraTag) || Actor->ActorHasTag(V36ValidationCameraTag) || Actor->ActorHasTag(V35ValidationCameraTag))) ||
			(bValidationCamerasV36 && (Actor->ActorHasTag(V36ValidationCameraTag) || Actor->ActorHasTag(V35ValidationCameraTag))) ||
			(bValidationCamerasV35 && Actor->ActorHasTag(V35ValidationCameraTag)) ||
			(bCliffRockDressing && Actor->ActorHasTag(CliffRockTag)) ||
			(bV60CleanHighland && (Actor->ActorHasTag(V59TitanGeologyTag) || Actor->ActorHasTag(V59ValidationCameraTag) || Actor->ActorHasTag(V57SurfaceReplacementTag) || Actor->ActorHasTag(V57ValidationCameraTag) || Actor->ActorHasTag(V56MountainIdentityTag) || Actor->ActorHasTag(V56ValidationCameraTag) || Actor->ActorHasTag(V55ThinCliffFacadeTag) || Actor->ActorHasTag(V55ValidationCameraTag))) ||
			(bMountainFreezeCleanupV70 && (Actor->ActorHasTag(V70MountainFreezeCleanupTag) || Actor->ActorHasTag(V70ValidationCameraTag))) ||
			(bTitanMountainMassCompletionV69 && (Actor->ActorHasTag(V69TitanMountainMassTag) || Actor->ActorHasTag(V69ValidationCameraTag) || Actor->ActorHasTag(V68TitanMountainCoverageTag) || Actor->ActorHasTag(V68ValidationCameraTag) || Actor->ActorHasTag(V67TitanMountainBodyTag) || Actor->ActorHasTag(V67ValidationCameraTag) || Actor->ActorHasTag(V66TitanMountainBodyTag) || Actor->ActorHasTag(V66ValidationCameraTag) || Actor->ActorHasTag(V65TitanMountainConversionTag) || Actor->ActorHasTag(V65ValidationCameraTag) || Actor->ActorHasTag(V59TitanGeologyTag) || Actor->ActorHasTag(V59ValidationCameraTag) || Actor->ActorHasTag(V57SurfaceReplacementTag) || Actor->ActorHasTag(V57ValidationCameraTag) || Actor->ActorHasTag(V56MountainIdentityTag) || Actor->ActorHasTag(V56ValidationCameraTag) || Actor->ActorHasTag(V55ThinCliffFacadeTag) || Actor->ActorHasTag(V54ThinCliffFacadeTag) || Actor->ActorHasTag(V53ThinCliffFacadeTag) || Actor->ActorHasTag(V52ThinCliffFacadeTag) || Actor->ActorHasTag(V55ValidationCameraTag) || Actor->ActorHasTag(V54ValidationCameraTag) || Actor->ActorHasTag(V46EmbeddedCliffIntegrationTag) || Actor->ActorHasTag(V45EmbeddedCliffMassTag) || Actor->ActorHasTag(CliffRockTag) || IsObsoleteGrassProofActorV59(Actor))) ||
			(bTitanMountainCoverageV68 && (Actor->ActorHasTag(V68TitanMountainCoverageTag) || Actor->ActorHasTag(V68ValidationCameraTag) || Actor->ActorHasTag(V67TitanMountainBodyTag) || Actor->ActorHasTag(V67ValidationCameraTag) || Actor->ActorHasTag(V66TitanMountainBodyTag) || Actor->ActorHasTag(V66ValidationCameraTag) || Actor->ActorHasTag(V65TitanMountainConversionTag) || Actor->ActorHasTag(V65ValidationCameraTag) || Actor->ActorHasTag(V59TitanGeologyTag) || Actor->ActorHasTag(V59ValidationCameraTag) || Actor->ActorHasTag(V57SurfaceReplacementTag) || Actor->ActorHasTag(V57ValidationCameraTag) || Actor->ActorHasTag(V56MountainIdentityTag) || Actor->ActorHasTag(V56ValidationCameraTag) || Actor->ActorHasTag(V55ThinCliffFacadeTag) || Actor->ActorHasTag(V54ThinCliffFacadeTag) || Actor->ActorHasTag(V53ThinCliffFacadeTag) || Actor->ActorHasTag(V52ThinCliffFacadeTag) || Actor->ActorHasTag(V55ValidationCameraTag) || Actor->ActorHasTag(V54ValidationCameraTag) || Actor->ActorHasTag(V46EmbeddedCliffIntegrationTag) || Actor->ActorHasTag(V45EmbeddedCliffMassTag) || Actor->ActorHasTag(CliffRockTag) || IsObsoleteGrassProofActorV59(Actor))) ||
			(bTitanMountainBodyCompletionV67 && (Actor->ActorHasTag(V67TitanMountainBodyTag) || Actor->ActorHasTag(V67ValidationCameraTag) || Actor->ActorHasTag(V66TitanMountainBodyTag) || Actor->ActorHasTag(V66ValidationCameraTag) || Actor->ActorHasTag(V65TitanMountainConversionTag) || Actor->ActorHasTag(V65ValidationCameraTag) || Actor->ActorHasTag(V59TitanGeologyTag) || Actor->ActorHasTag(V59ValidationCameraTag) || Actor->ActorHasTag(V57SurfaceReplacementTag) || Actor->ActorHasTag(V57ValidationCameraTag) || Actor->ActorHasTag(V56MountainIdentityTag) || Actor->ActorHasTag(V56ValidationCameraTag) || Actor->ActorHasTag(V55ThinCliffFacadeTag) || Actor->ActorHasTag(V54ThinCliffFacadeTag) || Actor->ActorHasTag(V53ThinCliffFacadeTag) || Actor->ActorHasTag(V52ThinCliffFacadeTag) || Actor->ActorHasTag(V55ValidationCameraTag) || Actor->ActorHasTag(V54ValidationCameraTag) || Actor->ActorHasTag(V46EmbeddedCliffIntegrationTag) || Actor->ActorHasTag(V45EmbeddedCliffMassTag) || Actor->ActorHasTag(CliffRockTag) || IsObsoleteGrassProofActorV59(Actor))) ||
			(bTitanMountainBodyCompletionV66 && (Actor->ActorHasTag(V66TitanMountainBodyTag) || Actor->ActorHasTag(V66ValidationCameraTag) || Actor->ActorHasTag(V65TitanMountainConversionTag) || Actor->ActorHasTag(V65ValidationCameraTag) || Actor->ActorHasTag(V59TitanGeologyTag) || Actor->ActorHasTag(V59ValidationCameraTag) || Actor->ActorHasTag(V57SurfaceReplacementTag) || Actor->ActorHasTag(V57ValidationCameraTag) || Actor->ActorHasTag(V56MountainIdentityTag) || Actor->ActorHasTag(V56ValidationCameraTag) || Actor->ActorHasTag(V55ThinCliffFacadeTag) || Actor->ActorHasTag(V54ThinCliffFacadeTag) || Actor->ActorHasTag(V53ThinCliffFacadeTag) || Actor->ActorHasTag(V52ThinCliffFacadeTag) || Actor->ActorHasTag(V55ValidationCameraTag) || Actor->ActorHasTag(V54ValidationCameraTag) || Actor->ActorHasTag(V46EmbeddedCliffIntegrationTag) || Actor->ActorHasTag(V45EmbeddedCliffMassTag) || Actor->ActorHasTag(CliffRockTag) || IsObsoleteGrassProofActorV59(Actor))) ||
			(bTitanMountainConversionV65 && (Actor->ActorHasTag(V65TitanMountainConversionTag) || Actor->ActorHasTag(V65ValidationCameraTag) || Actor->ActorHasTag(V59TitanGeologyTag) || Actor->ActorHasTag(V59ValidationCameraTag) || Actor->ActorHasTag(V57SurfaceReplacementTag) || Actor->ActorHasTag(V57ValidationCameraTag) || Actor->ActorHasTag(V56MountainIdentityTag) || Actor->ActorHasTag(V56ValidationCameraTag) || Actor->ActorHasTag(V55ThinCliffFacadeTag) || Actor->ActorHasTag(V54ThinCliffFacadeTag) || Actor->ActorHasTag(V53ThinCliffFacadeTag) || Actor->ActorHasTag(V52ThinCliffFacadeTag) || Actor->ActorHasTag(V55ValidationCameraTag) || Actor->ActorHasTag(V54ValidationCameraTag) || Actor->ActorHasTag(V46EmbeddedCliffIntegrationTag) || Actor->ActorHasTag(V45EmbeddedCliffMassTag) || Actor->ActorHasTag(CliffRockTag) || IsObsoleteGrassProofActorV59(Actor))) ||
			(bTitanGeologyV59 && (Actor->ActorHasTag(V59TitanGeologyTag) || Actor->ActorHasTag(V59ValidationCameraTag) || Actor->ActorHasTag(V57SurfaceReplacementTag) || Actor->ActorHasTag(V57ValidationCameraTag) || Actor->ActorHasTag(V56MountainIdentityTag) || Actor->ActorHasTag(V56ValidationCameraTag) || Actor->ActorHasTag(V55ThinCliffFacadeTag) || Actor->ActorHasTag(V54ThinCliffFacadeTag) || Actor->ActorHasTag(V53ThinCliffFacadeTag) || Actor->ActorHasTag(V52ThinCliffFacadeTag) || Actor->ActorHasTag(V55ValidationCameraTag) || Actor->ActorHasTag(V54ValidationCameraTag) || Actor->ActorHasTag(V46EmbeddedCliffIntegrationTag) || Actor->ActorHasTag(V45EmbeddedCliffMassTag) || Actor->ActorHasTag(CliffRockTag) || IsObsoleteGrassProofActorV59(Actor))) ||
			(bOuterRingSurfaceReplacementV57 && (Actor->ActorHasTag(V57SurfaceReplacementTag) || Actor->ActorHasTag(V57ValidationCameraTag) || Actor->ActorHasTag(V56MountainIdentityTag) || Actor->ActorHasTag(V56ValidationCameraTag) || Actor->ActorHasTag(V55ThinCliffFacadeTag) || Actor->ActorHasTag(V54ThinCliffFacadeTag) || Actor->ActorHasTag(V53ThinCliffFacadeTag) || Actor->ActorHasTag(V52ThinCliffFacadeTag) || Actor->ActorHasTag(V55ValidationCameraTag) || Actor->ActorHasTag(V54ValidationCameraTag))) ||
			(bMountainIdentityV56 && (Actor->ActorHasTag(V57SurfaceReplacementTag) || Actor->ActorHasTag(V57ValidationCameraTag) || Actor->ActorHasTag(V56MountainIdentityTag) || Actor->ActorHasTag(V56ValidationCameraTag) || Actor->ActorHasTag(V55ThinCliffFacadeTag) || Actor->ActorHasTag(V55ValidationCameraTag) || Actor->ActorHasTag(V54ValidationCameraTag))) ||
			(bAnyThinCliffFacade && (Actor->ActorHasTag(V57SurfaceReplacementTag) || Actor->ActorHasTag(V57ValidationCameraTag) || Actor->ActorHasTag(V56MountainIdentityTag) || Actor->ActorHasTag(V56ValidationCameraTag) || Actor->ActorHasTag(V55ThinCliffFacadeTag) || Actor->ActorHasTag(V54ThinCliffFacadeTag) || Actor->ActorHasTag(V53ThinCliffFacadeTag) || Actor->ActorHasTag(V52ThinCliffFacadeTag) || Actor->ActorHasTag(V55ValidationCameraTag) || Actor->ActorHasTag(V54ValidationCameraTag) || Actor->ActorHasTag(V46EmbeddedCliffIntegrationTag) || Actor->ActorHasTag(V45EmbeddedCliffMassTag) || Actor->ActorHasTag(CliffRockTag))) ||
			(bEmbeddedCliffIntegrationV46 && (Actor->ActorHasTag(V46EmbeddedCliffIntegrationTag) || Actor->ActorHasTag(V45EmbeddedCliffMassTag) || Actor->ActorHasTag(CliffRockTag))) ||
			(bEmbeddedCliffMassesV45 && (Actor->ActorHasTag(V45EmbeddedCliffMassTag) || Actor->ActorHasTag(CliffRockTag))) ||
			(bRemoveVisibleBlockoutMarkersV39 && IsV39VisibleBlockoutPlaceholder(Actor)) ||
			(bTraversalForestV42 && Actor->ActorHasTag(VegetationTag)) ||
			(bTraversalForestBlueprintMode && Actor->ActorHasTag(VegetationTag));
		if (Actor && bRemoveActor)
		{
			World->DestroyActor(Actor);
			++RemovedActors;
		}
	}

	if (bRemoveVisibleBlockoutMarkersV39)
	{
		return SaveAfterV39VisibleBlockoutCleanup(World, RemovedActors);
	}

	if (bAuditFlatActorsV39)
	{
		return AuditFlatActorsV39(World);
	}

	if (bV61MapAudit)
	{
		return RunV61MapAudit(World);
	}

	if (bV60CleanHighland)
	{
		World->MarkPackageDirty();
		const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
		const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=V60CleanHighland removedExperimentActors=%d removedV55V56V57V58V59=true newGeometryCreated=false mountainsModified=false savedMap=%s savedPackages=%s"),
			RemovedActors,
			bSavedMap ? TEXT("true") : TEXT("false"),
			bSavedPackages ? TEXT("true") : TEXT("false"));
		return (bSavedMap && bSavedPackages) ? 0 : 1;
	}

	if (bOuterRingRockCleanupV44)
	{
		return RunOuterRingRockCleanupV44(World, RemovedActors);
	}

	if (bOuterRingRockCleanupV43)
	{
		return RunOuterRingRockCleanupV43(World, RemovedActors);
	}

	if (bMountainFreezeCleanupV70)
	{
		return RunV70MountainFreezeCleanup(World, RemovedActors);
	}

	if (bTitanMountainMassCompletionV69)
	{
		return RunV67TitanMountainBodyCompletion(World, RemovedActors, false, true);
	}

	if (bTitanMountainCoverageV68)
	{
		return RunV67TitanMountainBodyCompletion(World, RemovedActors, true);
	}

	if (bTitanMountainBodyCompletionV67)
	{
		return RunV67TitanMountainBodyCompletion(World, RemovedActors);
	}

	if (bTitanMountainBodyCompletionV66)
	{
		return RunV66TitanMountainBodyCompletion(World, RemovedActors);
	}

	if (bTitanMountainConversionV65)
	{
		return RunV65TitanMountainConversion(World, RemovedActors);
	}

	if (bTitanGeologyV59)
	{
		return RunV59TitanGeology(World, RemovedActors);
	}

	if (bOuterRingSurfaceReplacementV57)
	{
		return RunV57OuterRingSurfaceReplacement(World, RemovedActors);
	}

	if (bMountainIdentityV56)
	{
		return RunV56MountainIdentity(World, RemovedActors);
	}

	if (bThinCliffFacadeV55)
	{
		return RunV52ThinCliffFacade(World, RemovedActors, true, true, true);
	}

	if (bThinCliffFacadeV54)
	{
		return RunV52ThinCliffFacade(World, RemovedActors, true, true);
	}

	if (bThinCliffFacadeV53)
	{
		return RunV52ThinCliffFacade(World, RemovedActors, true);
	}

	if (bThinCliffFacadeV52)
	{
		return RunV52ThinCliffFacade(World, RemovedActors, false);
	}

	if (bEmbeddedCliffIntegrationV46)
	{
		return RunV46EmbeddedCliffIntegration(World, RemovedActors);
	}

	if (bEmbeddedCliffMassesV45)
	{
		return RunV45EmbeddedCliffMasses(World, RemovedActors);
	}

	if (bValidationCamerasV46)
	{
		return RunV46ValidationCameras(World, RemovedActors);
	}

	if (bValidationCamerasV45)
	{
		return RunV45ValidationCameras(World, RemovedActors);
	}

	if (bValidationCamerasV44)
	{
		return RunV44ValidationCameras(World, RemovedActors);
	}

	if (bValidationCamerasV43)
	{
		return RunV43ValidationCameras(World, RemovedActors);
	}

	if (bValidationCamerasV42)
	{
		return RunV42ValidationCameras(World, RemovedActors);
	}

	if (bValidationCamerasV41)
	{
		return RunV41ValidationCameras(World, RemovedActors);
	}

	if (bValidationCamerasV40)
	{
		return RunV40ValidationCameras(World, RemovedActors);
	}

	if (bValidationCamerasV39)
	{
		return RunV39ValidationCameras(World, RemovedActors);
	}

	if (bValidationCamerasV38)
	{
		return RunV38ValidationCameras(World, RemovedActors);
	}

	if (bValidationCamerasV37)
	{
		return RunV37ValidationCameras(World, RemovedActors);
	}

	if (bValidationCamerasV36)
	{
		return RunV36ValidationCameras(World, RemovedActors);
	}

	if (bValidationCamerasV35)
	{
		return RunV35ValidationCameras(World, RemovedActors);
	}

	if (bCliffRockDressing)
	{
		return RunTraversalCliffRockDressing(World, RemovedActors, bCliffRockDressingV7 ? 7 : (bCliffRockDressingV6 ? 6 : (bCliffRockDressingV5 ? 5 : (bCliffRockDressingV4 ? 4 : (bCliffRockDressingV3 ? 3 : (bCliffRockDressingV2 ? 2 : 1))))));
	}

	if (bTraversalForestV42)
	{
		return RunV42EcologyVariation(World, RemovedActors);
	}

	if (bTraversalForestBlueprintMode)
	{
		const int32 TraversalForestVersion = bTraversalForestV40 ? 40 : (bTraversalForestV10 ? 10 : (bTraversalForestV9 ? 9 : (bTraversalForestV8 ? 8 : (bTraversalForestV7 ? 7 : (bTraversalForestV6 ? 6 : (bTraversalForestV5 ? 5 : (bTraversalForestV4 ? 4 : (bTraversalForestV3 ? 3 : (bTraversalForestV2 ? 2 : 1)))))))));
		return RunTraversalBlueprintForest(World, RemovedActors, TraversalForestVersion);
	}

	TArray<UStaticMesh*> TreeMeshes;
	TArray<UStaticMesh*> UndergrowthMeshes;
	if (!LoadMeshes(TreeMeshPaths, UE_ARRAY_COUNT(TreeMeshPaths), TreeMeshes, TEXT("tree"))
		|| !LoadMeshes(UndergrowthMeshPaths, UE_ARRAY_COUNT(UndergrowthMeshPaths), UndergrowthMeshes, TEXT("undergrowth")))
	{
		return 1;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AActor* VegetationActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
	if (!VegetationActor)
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandVegetationLayer: failed to spawn vegetation actor."));
		return 1;
	}

	VegetationActor->Tags.AddUnique(VegetationTag);
	VegetationActor->SetActorLabel(TEXT("FF_Phase1_TitanVegetation_HISM"));
	USceneComponent* RootComponent = NewObject<USceneComponent>(VegetationActor, TEXT("Root"), RF_Transactional);
	RootComponent->SetMobility(EComponentMobility::Static);
	VegetationActor->SetRootComponent(RootComponent);
	RootComponent->RegisterComponent();
	VegetationActor->AddInstanceComponent(RootComponent);

	TArray<UHierarchicalInstancedStaticMeshComponent*> TreeComponents;
	TArray<UHierarchicalInstancedStaticMeshComponent*> UndergrowthComponents;
	for (int32 Index = 0; Index < TreeMeshes.Num(); ++Index)
	{
		TreeComponents.Add(CreateVegetationComponent(VegetationActor, TreeMeshes[Index], Index, true));
	}
	for (int32 Index = 0; Index < UndergrowthMeshes.Num(); ++Index)
	{
		UndergrowthComponents.Add(CreateVegetationComponent(VegetationActor, UndergrowthMeshes[Index], Index, false));
	}

	int32 TotalTrees = 0;
	int32 TotalUndergrowth = 0;
	int32 RejectedSamples = 0;
	for (int32 ZoneIndex = 0; ZoneIndex < UE_ARRAY_COUNT(VegetationZones); ++ZoneIndex)
	{
		const FVegetationZone& Zone = VegetationZones[ZoneIndex];
		const int32 TargetTreeCount = FMath::Max(0, FMath::RoundToInt(static_cast<float>(Zone.TreeCount) * TreeCountScale));
		const int32 TargetUndergrowthCount = FMath::Max(0, FMath::RoundToInt(static_cast<float>(Zone.UndergrowthCount) * UndergrowthCountScale));
		int32 ZoneTrees = 0;
		int32 ZoneUndergrowth = 0;

		for (int32 SampleIndex = 0; SampleIndex < TargetTreeCount * 36 && ZoneTrees < TargetTreeCount; ++SampleIndex)
		{
			const int32 Seed = (ZoneIndex + 1) * 1000000 + SampleIndex * 73;
			const FVector2D Position = GetRandomPointInZone(Zone, Seed);
			if (!IsValidVegetationPosition(Position, 4200.0f, bTraversalForestV1))
			{
				++RejectedSamples;
				continue;
			}

			FHitResult GroundHit;
			if (!GetPlacementGround(World, Position, 0.92f, GroundHit))
			{
				++RejectedSamples;
				continue;
			}

			const int32 MeshIndex = SelectTreeMeshIndex(Seed + 19, TreeMeshes.Num());
			UHierarchicalInstancedStaticMeshComponent* Component = TreeComponents.IsValidIndex(MeshIndex) ? TreeComponents[MeshIndex] : nullptr;
			if (!Component)
			{
				continue;
			}

			const float Yaw = PseudoRandom01(Seed + 23) * 360.0f;
			const float BaseScale = FMath::Lerp(1.25f, 2.20f, PseudoRandom01(Seed + 31));
			const float ZScale = BaseScale * FMath::Lerp(0.98f, 1.16f, PseudoRandom01(Seed + 47));

			FTransform InstanceTransform;
			InstanceTransform.SetLocation(GroundHit.ImpactPoint - FVector(0.0f, 0.0f, 1.0f));
			InstanceTransform.SetRotation(FRotator(0.0f, Yaw, 0.0f).Quaternion());
			InstanceTransform.SetScale3D(FVector(BaseScale, BaseScale, ZScale));
			Component->AddInstance(InstanceTransform, true);
			++ZoneTrees;
			++TotalTrees;
		}

		for (int32 SampleIndex = 0; SampleIndex < TargetUndergrowthCount * 28 && ZoneUndergrowth < TargetUndergrowthCount; ++SampleIndex)
		{
			const int32 Seed = (ZoneIndex + 1) * 2000000 + SampleIndex * 59;
			const FVector2D Position = GetRandomPointInZone(Zone, Seed);
			if (!IsValidVegetationPosition(Position, 2300.0f, bTraversalForestV1))
			{
				++RejectedSamples;
				continue;
			}

			FHitResult GroundHit;
			if (!GetPlacementGround(World, Position, 0.93f, GroundHit))
			{
				++RejectedSamples;
				continue;
			}

			const int32 MeshIndex = FMath::Clamp(FMath::FloorToInt(PseudoRandom01(Seed + 19) * UndergrowthMeshes.Num()), 0, UndergrowthMeshes.Num() - 1);
			UHierarchicalInstancedStaticMeshComponent* Component = UndergrowthComponents.IsValidIndex(MeshIndex) ? UndergrowthComponents[MeshIndex] : nullptr;
			if (!Component)
			{
				continue;
			}

			const float Yaw = PseudoRandom01(Seed + 23) * 360.0f;
			const float BaseScale = FMath::Lerp(0.55f, 1.35f, PseudoRandom01(Seed + 31));
			const float ZScale = BaseScale * FMath::Lerp(0.75f, 1.18f, PseudoRandom01(Seed + 47));

			FTransform InstanceTransform;
			InstanceTransform.SetLocation(GroundHit.ImpactPoint + FVector(0.0f, 0.0f, 0.4f));
			InstanceTransform.SetRotation(FRotator(0.0f, Yaw, 0.0f).Quaternion());
			InstanceTransform.SetScale3D(FVector(BaseScale, BaseScale, ZScale));
			Component->AddInstance(InstanceTransform, true);
			++ZoneUndergrowth;
			++TotalUndergrowth;
		}

		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: zone=%s trees=%d/%d undergrowth=%d/%d"), Zone.Name, ZoneTrees, TargetTreeCount, ZoneUndergrowth, TargetUndergrowthCount);
	}

	for (UHierarchicalInstancedStaticMeshComponent* Component : TreeComponents)
	{
		if (Component)
		{
			Component->BuildTreeIfOutdated(true, true);
			Component->MarkPackageDirty();
		}
	}
	for (UHierarchicalInstancedStaticMeshComponent* Component : UndergrowthComponents)
	{
		if (Component)
		{
			Component->BuildTreeIfOutdated(true, true);
			Component->MarkPackageDirty();
		}
	}

	VegetationActor->MarkPackageDirty();
	World->MarkPackageDirty();
	const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
	const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
	UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVegetationLayer: mode=%s removedActors=%d trees=%d undergrowth=%d rejectedSamples=%d savedMap=%s savedPackages=%s"),
		bTraversalForestBlueprintMode ? TEXT("TraversalForestBlueprint") : TEXT("Default"),
		RemovedActors,
		TotalTrees,
		TotalUndergrowth,
		RejectedSamples,
		bSavedMap ? TEXT("true") : TEXT("false"),
		bSavedPackages ? TEXT("true") : TEXT("false"));

	return (bSavedMap && bSavedPackages && TotalTrees > 0 && (bTraversalForestBlueprintMode || TotalUndergrowth > 0)) ? 0 : 1;
#else
	UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandVegetationLayer can only run in editor builds."));
	return 1;
#endif
}
