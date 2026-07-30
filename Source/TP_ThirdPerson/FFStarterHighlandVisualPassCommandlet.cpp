#include "FFStarterHighlandVisualPassCommandlet.h"

#if WITH_EDITOR
#include "AssetToolsModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Factories/MaterialFactoryNew.h"
#include "FileHelpers.h"
#include "GameFramework/PlayerStart.h"
#include "Landscape.h"
#include "LandscapeComponent.h"
#include "LandscapeGrassType.h"
#include "LandscapeProxy.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionClamp.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionCustom.h"
#include "Materials/MaterialExpressionLandscapeGrassOutput.h"
#include "Materials/MaterialExpressionLinearInterpolate.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionOneMinus.h"
#include "Materials/MaterialExpressionPixelNormalWS.h"
#include "Materials/MaterialExpressionVertexNormalWS.h"
#include "Materials/MaterialExpressionWorldPosition.h"
#include "MaterialEditingLibrary.h"
#include "Misc/PackageName.h"
#endif

UFFStarterHighlandVisualPassCommandlet::UFFStarterHighlandVisualPassCommandlet()
{
	LogToConsole = true;
	IsClient = false;
	IsEditor = true;
	IsServer = false;
}

#if WITH_EDITOR
namespace
{
	const FString HighlandMapPath = TEXT("/Game/Maps/FF_Starter_Highland_Blockout");
	const FString MaterialFolderPath = TEXT("/Game/FantasyFrontier/Blockout/Materials");
	const FString GrassMaterialName = TEXT("M_FF_Blockout_Grass_Green");
	const FString RockMaterialName = TEXT("M_FF_Blockout_Rock_Cliff");
	const FString WaterMaterialName = TEXT("M_FF_Blockout_Water");
	const FString GrassBladeMaterialName = TEXT("M_FF_Blockout_GrassBlade");
	const FString GrassTypeName = TEXT("GT_FF_Blockout_LandscapeGrass");
	const FString SkyFallbackMaterialName = TEXT("M_FF_Blockout_SkyFallback");
	const FString PathMaterialName = TEXT("M_FF_Blockout_Path_Tan");
	const FString MarkerMaterialName = TEXT("M_FF_Blockout_Marker_Gold");
	const FString ForestMassMaterialName = TEXT("M_FF_Blockout_ForestMass_Green");
	const FString TreeCanopyMaterialName = TEXT("M_FF_Blockout_TreeCanopy_Green");
	const FString TreeTrunkMaterialName = TEXT("M_FF_Blockout_TreeTrunk_Brown");

	const TCHAR* TitanCliffMaterialPath = TEXT("/Game/Environment/Grassland/Materials/MI_Cliffside_Dirt_B.MI_Cliffside_Dirt_B");
	const TCHAR* TitanGrassBaseMaterialPath = TEXT("/Game/Environment/_Global/Materials/FlatCol/MI_FlatCol_Green_Dark.MI_FlatCol_Green_Dark");
	const TCHAR* TitanGrassBladeMaterialPath = TEXT("/Game/Environment/Foliage/Materials/MI_GrassBlade.MI_GrassBlade");
	const TCHAR* TitanWaterCustomMeshMaterialPath = TEXT("/Game/Environment/_Global/Core/Materials/Water/Water_Material_CustomMesh.Water_Material_CustomMesh");
	const TCHAR* TitanGrassBladeMeshPath = TEXT("/Game/Environment/Foliage/Meshes/SM_GrassBlade.SM_GrassBlade");
	const TCHAR* TitanRyegrassAMeshPath = TEXT("/Game/Environment/Foliage/Grass/Ryegrass_Grass_A.Ryegrass_Grass_A");
	const TCHAR* TitanRyegrassCMeshPath = TEXT("/Game/Environment/Foliage/Grass/Ryegrass_Grass_C.Ryegrass_Grass_C");

	FString GetObjectPath(const FString& FolderPath, const FString& AssetName)
	{
		return FString::Printf(TEXT("%s/%s.%s"), *FolderPath, *AssetName, *AssetName);
	}

	UMaterial* CreateOrLoadMaterial(const FString& AssetName)
	{
		if (UMaterial* Existing = LoadObject<UMaterial>(nullptr, *GetObjectPath(MaterialFolderPath, AssetName)))
		{
			Existing->Modify();
			UMaterialEditingLibrary::DeleteAllMaterialExpressions(Existing);
			return Existing;
		}

		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
		UMaterialFactoryNew* Factory = NewObject<UMaterialFactoryNew>();
		return Cast<UMaterial>(AssetToolsModule.Get().CreateAsset(AssetName, MaterialFolderPath, UMaterial::StaticClass(), Factory));
	}

	void FinalizeMaterial(UMaterial* Material)
	{
		if (!Material)
		{
			return;
		}

		Material->Modify();
		Material->PreEditChange(nullptr);
		UMaterialEditingLibrary::LayoutMaterialExpressions(Material);
		UMaterialEditingLibrary::RecompileMaterial(Material);
		Material->PostEditChange();
		Material->MarkPackageDirty();
	}

	UMaterial* BuildSolidMaterial(const FString& AssetName, const FLinearColor& BaseColor, float Roughness)
	{
		UMaterial* Material = CreateOrLoadMaterial(AssetName);
		if (!Material)
		{
			return nullptr;
		}

		UMaterialExpressionConstant3Vector* BaseColorNode = Cast<UMaterialExpressionConstant3Vector>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant3Vector::StaticClass(), -320, -40));
		UMaterialExpressionConstant* RoughnessNode = Cast<UMaterialExpressionConstant>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant::StaticClass(), -320, 120));

		BaseColorNode->Constant = BaseColor;
		RoughnessNode->R = Roughness;

		Material->BlendMode = BLEND_Opaque;
		Material->TwoSided = false;
		UMaterialEditingLibrary::ConnectMaterialProperty(BaseColorNode, TEXT(""), MP_BaseColor);
		UMaterialEditingLibrary::ConnectMaterialProperty(RoughnessNode, TEXT(""), MP_Roughness);
		FinalizeMaterial(Material);
		return Material;
	}

	void ConnectCustomInput(UMaterialExpressionCustom* CustomNode, UMaterialExpression* InputExpression, const FName InputName)
	{
		if (!CustomNode || !InputExpression)
		{
			return;
		}

		FCustomInput CustomInput;
		CustomInput.InputName = InputName;
		CustomInput.Input.Expression = InputExpression;
		CustomNode->Inputs.Add(CustomInput);
	}

	UMaterialExpressionWorldPosition* AddWorldPositionNode(UMaterial* Material, int32 NodePosX, int32 NodePosY)
	{
		UMaterialExpressionWorldPosition* WorldPosition = Cast<UMaterialExpressionWorldPosition>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionWorldPosition::StaticClass(), NodePosX, NodePosY));
		if (WorldPosition)
		{
			WorldPosition->WorldPositionShaderOffset = WPT_ExcludeAllShaderOffsets;
		}
		return WorldPosition;
	}

	FString GetRiverLakeMaskCode()
	{
		return TEXT(R"(
#define DISTSEG(P,A,B) length((P) - ((A) + saturate(dot((P) - (A), (B) - (A)) / max(dot((B) - (A), (B) - (A)), 1.0)) * ((B) - (A))))
float2 p = WorldPos.xy;
float water = 0.0;
water = max(water, 1.0 - smoothstep(1450.0, 3050.0, DISTSEG(p, float2(-30000.0, 64000.0), float2(-42000.0, 50000.0))));
water = max(water, 1.0 - smoothstep(1450.0, 3050.0, DISTSEG(p, float2(-42000.0, 50000.0), float2(-52000.0, 28000.0))));
water = max(water, 1.0 - smoothstep(1450.0, 3050.0, DISTSEG(p, float2(-52000.0, 28000.0), float2(-42000.0, 8000.0))));
water = max(water, 1.0 - smoothstep(1450.0, 3050.0, DISTSEG(p, float2(-42000.0, 8000.0), float2(-52000.0, -14000.0))));
water = max(water, 1.0 - smoothstep(1450.0, 3050.0, DISTSEG(p, float2(-52000.0, -14000.0), float2(-64000.0, -38000.0))));
water = max(water, 1.0 - smoothstep(1450.0, 3050.0, DISTSEG(p, float2(-64000.0, -38000.0), float2(-70000.0, -62000.0))));
water = max(water, 1.0 - smoothstep(1200.0, 2600.0, DISTSEG(p, float2(-42000.0, 8000.0), float2(-15000.0, 2000.0))));
water = max(water, 1.0 - smoothstep(1200.0, 2600.0, DISTSEG(p, float2(-15000.0, 2000.0), float2(13000.0, 8000.0))));
float2 lake0 = (p - float2(-30000.0, 62000.0)) / float2(6200.0, 4500.0);
water = max(water, 1.0 - smoothstep(0.78, 1.08, length(lake0)));
float2 lake1 = (p - float2(-48000.0, -36500.0)) / float2(6800.0, 5200.0);
water = max(water, 1.0 - smoothstep(0.72, 1.08, length(lake1)));
return saturate(water);
#undef DISTSEG
)");
	}

	FString GetPathMaskCode()
	{
		return TEXT(R"(
#define DISTSEG(P,A,B) length((P) - ((A) + saturate(dot((P) - (A), (B) - (A)) / max(dot((B) - (A), (B) - (A)), 1.0)) * ((B) - (A))))
float2 p = WorldPos.xy;
float path = 0.0;
path = max(path, 1.0 - smoothstep(850.0, 1850.0, DISTSEG(p, float2(-18000.0, 4000.0), float2(9000.0, 9000.0))));
path = max(path, 1.0 - smoothstep(850.0, 1850.0, DISTSEG(p, float2(9000.0, 9000.0), float2(36000.0, 21000.0))));
path = max(path, 1.0 - smoothstep(850.0, 1850.0, DISTSEG(p, float2(36000.0, 21000.0), float2(76000.0, 32000.0))));
path = max(path, 1.0 - smoothstep(850.0, 1750.0, DISTSEG(p, float2(10000.0, 36000.0), float2(12000.0, 60000.0))));
path = max(path, 1.0 - smoothstep(850.0, 1750.0, DISTSEG(p, float2(12000.0, 60000.0), float2(16000.0, 87000.0))));
path = max(path, 1.0 - smoothstep(900.0, 2000.0, length(p - float2(-18000.0, 4000.0)) - 10500.0));
return saturate(path);
#undef DISTSEG
)");
	}

	UMaterial* BuildBiomeLandscapeMaterial(ULandscapeGrassType* GrassType)
	{
		UMaterial* Material = CreateOrLoadMaterial(GrassMaterialName);
		if (!Material)
		{
			return nullptr;
		}

		UMaterialExpressionWorldPosition* WorldPosition = AddWorldPositionNode(Material, -1160, -120);
		UMaterialExpressionVertexNormalWS* VertexNormal = Cast<UMaterialExpressionVertexNormalWS>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionVertexNormalWS::StaticClass(), -1160, 40));
		UMaterialExpressionCustom* ColorNode = Cast<UMaterialExpressionCustom>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionCustom::StaticClass(), -760, -120));
		UMaterialExpressionCustom* GrassDensityNode = Cast<UMaterialExpressionCustom>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionCustom::StaticClass(), -760, 180));
		UMaterialExpressionConstant* RoughnessNode = Cast<UMaterialExpressionConstant>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant::StaticClass(), -320, 360));
		UMaterialExpressionLandscapeGrassOutput* GrassOutput = Cast<UMaterialExpressionLandscapeGrassOutput>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionLandscapeGrassOutput::StaticClass(), -320, 520));

		if (!WorldPosition || !VertexNormal || !ColorNode || !GrassDensityNode || !RoughnessNode || !GrassOutput)
		{
			return nullptr;
		}

		ColorNode->Description = TEXT("Cook-safe Highland v50 slope material: flat grass, mid sediment, steep ring rock");
		ColorNode->OutputType = CMOT_Float3;
		ConnectCustomInput(ColorNode, WorldPosition, TEXT("WorldPos"));
		ConnectCustomInput(ColorNode, VertexNormal, TEXT("NormalWS"));
		ColorNode->Code = TEXT(R"(
#define DISTSEG(P,A,B) length((P) - ((A) + saturate(dot((P) - (A), (B) - (A)) / max(dot((B) - (A), (B) - (A)), 1.0)) * ((B) - (A))))
float2 p = WorldPos.xy;
float2 centered = (p - float2(0.0, 15000.0)) / float2(100000.0, 72000.0);
float organic = length(centered) + 0.055 * sin(p.x / 11500.0) - 0.045 * cos(p.y / 9000.0) + 0.035 * sin((p.x + p.y) / 17000.0);
float islandMask = 1.0 - smoothstep(0.92, 1.16, organic);
float slopeAmount = 1.0 - saturate(NormalWS.z);
float rimMask = smoothstep(0.58, 1.02, organic);
float outerRingMask = smoothstep(0.72, 1.06, organic);
float slopeRock = smoothstep(0.040, 0.170, slopeAmount);
float sheerRock = smoothstep(0.150, 0.340, slopeAmount);
float ringWallRock = outerRingMask * smoothstep(0.006, 0.060, slopeAmount);
float ringMaterialCloak = outerRingMask * smoothstep(0.000, 0.035, slopeAmount);
float shoulderRock = rimMask * smoothstep(0.055, 0.150, slopeAmount);
float elevatedRock = smoothstep(-4200.0, -900.0, WorldPos.z) * rimMask * smoothstep(0.030, 0.120, slopeAmount);
float rimShadow = smoothstep(0.70, 0.98, organic);
float rock = saturate(max(max(max(max(max(slopeRock, sheerRock), ringWallRock), ringMaterialCloak), shoulderRock), elevatedRock));
float sediment = saturate((smoothstep(0.010, 0.050, slopeAmount) - smoothstep(0.135, 0.240, slopeAmount)) * (0.34 + rimMask * 0.66));

float path = 0.0;
path = max(path, 1.0 - smoothstep(520.0, 1150.0, DISTSEG(p, float2(-10000.0, 6500.0), float2(9000.0, 9000.0))));
path = max(path, 1.0 - smoothstep(520.0, 1150.0, DISTSEG(p, float2(9000.0, 9000.0), float2(36000.0, 21000.0))));
path = max(path, 1.0 - smoothstep(520.0, 1150.0, DISTSEG(p, float2(36000.0, 21000.0), float2(76000.0, 32000.0))));
path = max(path, 1.0 - smoothstep(520.0, 1100.0, DISTSEG(p, float2(10000.0, 36000.0), float2(12000.0, 60000.0))));
path = max(path, 1.0 - smoothstep(520.0, 1100.0, DISTSEG(p, float2(12000.0, 60000.0), float2(16000.0, 87000.0))));

float water = 0.0;
water = max(water, 1.0 - smoothstep(1600.0, 3100.0, DISTSEG(p, float2(-30000.0, 64000.0), float2(-42000.0, 50000.0))));
water = max(water, 1.0 - smoothstep(1600.0, 3100.0, DISTSEG(p, float2(-42000.0, 50000.0), float2(-52000.0, 28000.0))));
water = max(water, 1.0 - smoothstep(1600.0, 3100.0, DISTSEG(p, float2(-52000.0, 28000.0), float2(-42000.0, 8000.0))));
water = max(water, 1.0 - smoothstep(1500.0, 2900.0, DISTSEG(p, float2(-42000.0, 8000.0), float2(-15000.0, 2000.0))));
water = max(water, 1.0 - smoothstep(1500.0, 2900.0, DISTSEG(p, float2(-15000.0, 2000.0), float2(13000.0, 8000.0))));
float2 lake0 = (p - float2(-30000.0, 62000.0)) / float2(6400.0, 4600.0);
water = max(water, 1.0 - smoothstep(0.78, 1.10, length(lake0)));
float spawnDryMask = smoothstep(6500.0, 10500.0, length(p - float2(-18120.0, 4000.0)));
water *= spawnDryMask;
float spawnGrassMask = 1.0 - smoothstep(1200.0, 2000.0, length(p - float2(-18120.0, 4000.0)));

float northDark = smoothstep(30000.0, 76000.0, p.y);
float darkPatch = northDark;
darkPatch = max(darkPatch, 1.0 - smoothstep(9000.0, 35000.0, length(p - float2(46000.0, 24000.0))));
darkPatch = max(darkPatch, 1.0 - smoothstep(9000.0, 31000.0, length(p - float2(-52000.0, -26000.0))));
float lightPatch = max(1.0 - smoothstep(6000.0, 26000.0, length(p - float2(-12000.0, -22000.0))),
                       1.0 - smoothstep(6000.0, 30000.0, length(p - float2(36000.0, -26000.0))));
float cellNoise = frac(sin(dot(floor(p / 3600.0), float2(12.9898, 78.233))) * 43758.5453);

float3 lightGrass = float3(0.34, 0.45, 0.18);
float3 openGrass = float3(0.17, 0.30, 0.085);
float3 darkGrass = float3(0.055, 0.155, 0.040);
float3 pathColor = float3(0.42, 0.34, 0.22);
float3 sedimentLow = float3(0.185, 0.140, 0.088);
float3 sedimentHigh = float3(0.300, 0.235, 0.150);
float3 rockLow = float3(0.035, 0.034, 0.030);
float3 rockMid = float3(0.095, 0.080, 0.062);
float3 rockHigh = float3(0.215, 0.180, 0.138);
float3 waterbank = float3(0.15, 0.18, 0.095);
float3 waterColor = float3(0.02, 0.31, 0.50);

float3 grass = lerp(openGrass, darkGrass, saturate(darkPatch));
grass = lerp(grass, lightGrass, saturate(lightPatch * 0.62));
grass *= lerp(0.93, 1.08, cellNoise);
float rockStrata = frac(sin(dot(floor((p + float2(1700.0, -900.0)) / 4600.0), float2(39.346, 11.135))) * 24634.6345);
float cliffBanding = 0.5 + 0.5 * sin(WorldPos.z * 0.018 + p.x * 0.00055 + p.y * 0.00035);
float diagonalErosion = 0.5 + 0.5 * sin((p.x - p.y) * 0.00042 + WorldPos.z * 0.011);
float3 sedimentColor = lerp(sedimentLow, sedimentHigh, saturate(0.30 + cellNoise * 0.36 + diagonalErosion * 0.22));
float3 rockColor = lerp(rockLow, rockMid, saturate(0.42 + rockStrata * 0.48 + cliffBanding * 0.12));
rockColor = lerp(rockColor, rockHigh, saturate((1.0 - NormalWS.z) * 0.34 + rimShadow * 0.16 + diagonalErosion * 0.10));
rockColor *= lerp(1.0, 0.70, saturate(outerRingMask * smoothstep(0.025, 0.150, slopeAmount)));
rockColor = lerp(rockColor, sedimentColor, saturate(sediment * 0.18));
float artifactRepair = 0.0;
artifactRepair = max(artifactRepair, 1.0 - smoothstep(3600.0, 8600.0, DISTSEG(p, float2(13000.0, 8000.0), float2(42000.0, 22000.0))));
artifactRepair = max(artifactRepair, 1.0 - smoothstep(3600.0, 9000.0, DISTSEG(p, float2(42000.0, 22000.0), float2(76000.0, 32000.0))));
float2 artifactCap = (p - float2(76000.0, 32000.0)) / float2(7600.0, 5200.0);
artifactRepair = max(artifactRepair, 1.0 - smoothstep(0.80, 1.30, length(artifactCap)));
artifactRepair *= saturate(1.0 - rock * 0.90 - sediment * 0.35);
float3 grassRepair = lerp(openGrass, lightGrass, saturate(0.28 + lightPatch * 0.35));
grassRepair *= lerp(0.93, 1.08, cellNoise);
float3 color = grass;
color = lerp(color, sedimentColor, saturate(sediment * (1.0 - water * 0.65) * (1.0 - path * 0.45)));
color = lerp(color, rockColor, saturate(rock * (1.0 - water * 0.80) * (1.0 - path * 0.65)));
color = lerp(color, waterbank, saturate(water * 0.35));
color = lerp(color, pathColor, saturate(path * (1.0 - water * 0.55)));
color = lerp(color, lightGrass, saturate(spawnGrassMask * (1.0 - water)));
color = lerp(color, grassRepair, saturate(artifactRepair));
return color;
#undef DISTSEG
)");

		GrassDensityNode->Description = TEXT("Grass density mask: flat green terrain only");
		GrassDensityNode->OutputType = CMOT_Float1;
		ConnectCustomInput(GrassDensityNode, WorldPosition, TEXT("WorldPos"));
		ConnectCustomInput(GrassDensityNode, VertexNormal, TEXT("NormalWS"));
		GrassDensityNode->Code = TEXT(R"(
#define DISTSEG(P,A,B) length((P) - ((A) + saturate(dot((P) - (A), (B) - (A)) / max(dot((B) - (A), (B) - (A)), 1.0)) * ((B) - (A))))
float2 p = WorldPos.xy;
float2 centered = (p - float2(0.0, 15000.0)) / float2(100000.0, 72000.0);
float organic = length(centered) + 0.055 * sin(p.x / 11500.0) - 0.045 * cos(p.y / 9000.0) + 0.035 * sin((p.x + p.y) / 17000.0);
float islandMask = 1.0 - smoothstep(0.90, 1.14, organic);
float slopeAmount = 1.0 - saturate(NormalWS.z);
float rimMask = smoothstep(0.58, 1.02, organic);
float edgeRock = smoothstep(0.72, 1.06, organic) * islandMask * smoothstep(0.000, 0.035, slopeAmount);
float slopeRock = smoothstep(0.040, 0.170, slopeAmount);
float sediment = saturate((smoothstep(0.010, 0.050, slopeAmount) - smoothstep(0.135, 0.240, slopeAmount)) * (0.34 + rimMask * 0.66));
float path = 0.0;
path = max(path, 1.0 - smoothstep(720.0, 1700.0, DISTSEG(p, float2(-10000.0, 6500.0), float2(9000.0, 9000.0))));
path = max(path, 1.0 - smoothstep(720.0, 1700.0, DISTSEG(p, float2(9000.0, 9000.0), float2(36000.0, 21000.0))));
path = max(path, 1.0 - smoothstep(720.0, 1700.0, DISTSEG(p, float2(36000.0, 21000.0), float2(76000.0, 32000.0))));
path = max(path, 1.0 - smoothstep(720.0, 1600.0, DISTSEG(p, float2(10000.0, 36000.0), float2(12000.0, 60000.0))));
path = max(path, 1.0 - smoothstep(720.0, 1600.0, DISTSEG(p, float2(12000.0, 60000.0), float2(16000.0, 87000.0))));
float water = 0.0;
water = max(water, 1.0 - smoothstep(1900.0, 3700.0, DISTSEG(p, float2(-30000.0, 64000.0), float2(-42000.0, 50000.0))));
water = max(water, 1.0 - smoothstep(1900.0, 3700.0, DISTSEG(p, float2(-42000.0, 50000.0), float2(-52000.0, 28000.0))));
water = max(water, 1.0 - smoothstep(1900.0, 3700.0, DISTSEG(p, float2(-52000.0, 28000.0), float2(-42000.0, 8000.0))));
float2 lake0 = (p - float2(-30000.0, 62000.0)) / float2(7400.0, 5600.0);
water = max(water, 1.0 - smoothstep(0.75, 1.12, length(lake0)));
float spawnDryMask = smoothstep(6500.0, 10500.0, length(p - float2(-18120.0, 4000.0)));
water *= spawnDryMask;
float spawnGrassMask = 1.0 - smoothstep(1200.0, 2000.0, length(p - float2(-18120.0, 4000.0)));
float darkDensity = smoothstep(30000.0, 76000.0, p.y);
darkDensity = max(darkDensity, 1.0 - smoothstep(9000.0, 33000.0, length(p - float2(46000.0, 24000.0))));
float density = lerp(0.48, 1.0, saturate(darkDensity));
float rock = saturate(max(edgeRock, slopeRock));
float baseDensity = saturate((1.0 - rock) * (1.0 - sediment * 0.72) * (1.0 - path) * (1.0 - water) * density);
float spawnDensity = saturate(spawnGrassMask * (1.0 - water) * 0.82);
return saturate(max(baseDensity, spawnDensity));
#undef DISTSEG
)");

		RoughnessNode->R = 0.96f;
		GrassOutput->GrassTypes.Empty();
		if (GrassType)
		{
			GrassOutput->GrassTypes.Add(FGrassInput(TEXT("FF_Grass")));
			GrassOutput->GrassTypes[0].GrassType = GrassType;
			GrassOutput->GrassTypes[0].Input.Expression = GrassDensityNode;
		}

		Material->BlendMode = BLEND_Opaque;
		Material->TwoSided = false;
		UMaterialEditingLibrary::ConnectMaterialProperty(ColorNode, TEXT(""), MP_BaseColor);
		UMaterialEditingLibrary::ConnectMaterialProperty(RoughnessNode, TEXT(""), MP_Roughness);
		FinalizeMaterial(Material);
		return Material;
	}

	UMaterial* BuildWaterMaterial()
	{
		UMaterial* Material = CreateOrLoadMaterial(WaterMaterialName);
		if (!Material)
		{
			return nullptr;
		}

		UMaterialExpressionWorldPosition* WorldPosition = AddWorldPositionNode(Material, -760, -140);
		UMaterialExpressionConstant3Vector* BaseColorNode = Cast<UMaterialExpressionConstant3Vector>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant3Vector::StaticClass(), -420, -40));
		UMaterialExpressionConstant* RoughnessNode = Cast<UMaterialExpressionConstant>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant::StaticClass(), -420, 120));
		UMaterialExpressionConstant* SpecularNode = Cast<UMaterialExpressionConstant>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant::StaticClass(), -420, 220));
		UMaterialExpressionCustom* OpacityNode = Cast<UMaterialExpressionCustom>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionCustom::StaticClass(), -420, 340));
		if (!WorldPosition || !BaseColorNode || !RoughnessNode || !SpecularNode || !OpacityNode)
		{
			return nullptr;
		}

		BaseColorNode->Constant = FLinearColor(0.015f, 0.36f, 0.62f);
		RoughnessNode->R = 0.11f;
		SpecularNode->R = 0.62f;
		OpacityNode->Description = TEXT("Disabled legacy blockout water placeholder opacity");
		OpacityNode->OutputType = CMOT_Float1;
		ConnectCustomInput(OpacityNode, WorldPosition, TEXT("WorldPos"));
		OpacityNode->Code = TEXT(R"(
return 0.0;
)");

		Material->BlendMode = BLEND_Translucent;
		Material->TwoSided = true;
		UMaterialEditingLibrary::ConnectMaterialProperty(BaseColorNode, TEXT(""), MP_BaseColor);
		UMaterialEditingLibrary::ConnectMaterialProperty(RoughnessNode, TEXT(""), MP_Roughness);
		UMaterialEditingLibrary::ConnectMaterialProperty(SpecularNode, TEXT(""), MP_Specular);
		UMaterialEditingLibrary::ConnectMaterialProperty(OpacityNode, TEXT(""), MP_Opacity);
		FinalizeMaterial(Material);
		return Material;
	}

	ULandscapeGrassType* CreateOrUpdateGrassType(UMaterialInterface* GrassBladeMaterial, const TArray<UStaticMesh*>& GrassMeshes)
	{
		if (!GrassBladeMaterial || GrassMeshes.Num() == 0)
		{
			return nullptr;
		}

		const FString PackagePath = FString::Printf(TEXT("%s/%s"), *MaterialFolderPath, *GrassTypeName);
		const FString ObjectPath = FString::Printf(TEXT("%s.%s"), *PackagePath, *GrassTypeName);
		ULandscapeGrassType* GrassType = LoadObject<ULandscapeGrassType>(nullptr, *ObjectPath);
		if (!GrassType)
		{
			UPackage* Package = CreatePackage(*PackagePath);
			if (!Package)
			{
				return nullptr;
			}
			GrassType = NewObject<ULandscapeGrassType>(Package, *GrassTypeName, RF_Public | RF_Standalone | RF_Transactional);
			FAssetRegistryModule::AssetCreated(GrassType);
		}

		GrassType->Modify();
		GrassType->GrassVarieties.Reset();
		for (int32 MeshIndex = 0; MeshIndex < GrassMeshes.Num(); ++MeshIndex)
		{
			UStaticMesh* GrassMesh = GrassMeshes[MeshIndex];
			if (!GrassMesh)
			{
				continue;
			}
			FGrassVariety GrassVariety;
			GrassVariety.GrassMesh = GrassMesh;
			GrassVariety.OverrideMaterials.Add(GrassBladeMaterial);
			const float Density = MeshIndex == 0 ? 120.0f : (MeshIndex == 1 ? 60.0f : 50.0f);
			GrassVariety.GrassDensity = FPerPlatformFloat(Density);
			GrassVariety.GrassDensityQuality = FPerQualityLevelFloat(Density);
			GrassVariety.bUseGrid = true;
			GrassVariety.PlacementJitter = 0.35f;
			GrassVariety.StartCullDistance = FPerPlatformInt(0);
			GrassVariety.StartCullDistanceQuality = FPerQualityLevelInt(0);
			GrassVariety.EndCullDistance = FPerPlatformInt(60000);
			GrassVariety.EndCullDistanceQuality = FPerQualityLevelInt(60000);
			GrassVariety.MinLOD = 0;
			GrassVariety.AllowedDensityRange = FFloatInterval(0.0f, 1.0f);
			GrassVariety.Scaling = EGrassScaling::Free;
			GrassVariety.ScaleX = MeshIndex == 0 ? FFloatInterval(1.80f, 2.80f) : FFloatInterval(1.60f, 2.50f);
			GrassVariety.ScaleY = MeshIndex == 0 ? FFloatInterval(1.80f, 2.80f) : FFloatInterval(1.60f, 2.50f);
			GrassVariety.ScaleZ = MeshIndex == 0 ? FFloatInterval(2.20f, 3.50f) : FFloatInterval(2.00f, 3.00f);
			GrassVariety.RandomRotation = true;
			GrassVariety.AlignToSurface = true;
			GrassVariety.bAlignToTriangleNormals = false;
			GrassVariety.bUseLandscapeLightmap = false;
			GrassVariety.bReceivesDecals = false;
			GrassVariety.bAffectDistanceFieldLighting = false;
			GrassVariety.bCastDynamicShadow = false;
			GrassVariety.bCastContactShadow = false;
			GrassVariety.bKeepInstanceBufferCPUCopy = false;
			GrassVariety.InstanceWorldPositionOffsetDisableDistance = 0;
			GrassType->GrassVarieties.Add(GrassVariety);
		}
		if (GrassType->GrassVarieties.Num() == 0)
		{
			return nullptr;
		}
		GrassType->bEnableDensityScaling = false;
		GrassType->PostEditChange();
		GrassType->MarkPackageDirty();
		return GrassType;
	}

	UMaterial* BuildGrassRockLandscapeMaterial(UMaterial* GrassMaterial)
	{
		UMaterial* Material = GrassMaterial ? GrassMaterial : CreateOrLoadMaterial(GrassMaterialName);
		if (!Material)
		{
			return nullptr;
		}

		UMaterialExpressionConstant3Vector* GrassColor = Cast<UMaterialExpressionConstant3Vector>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant3Vector::StaticClass(), -820, -100));
		UMaterialExpressionConstant3Vector* RockColor = Cast<UMaterialExpressionConstant3Vector>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant3Vector::StaticClass(), -820, 80));
		UMaterialExpressionPixelNormalWS* PixelNormal = Cast<UMaterialExpressionPixelNormalWS>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionPixelNormalWS::StaticClass(), -820, 260));
		UMaterialExpressionComponentMask* NormalZMask = Cast<UMaterialExpressionComponentMask>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionComponentMask::StaticClass(), -600, 260));
		UMaterialExpressionOneMinus* OneMinus = Cast<UMaterialExpressionOneMinus>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionOneMinus::StaticClass(), -380, 260));
		UMaterialExpressionConstant* SlopeBoost = Cast<UMaterialExpressionConstant>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant::StaticClass(), -380, 420));
		UMaterialExpressionMultiply* SlopeAlpha = Cast<UMaterialExpressionMultiply>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionMultiply::StaticClass(), -160, 260));
		UMaterialExpressionClamp* SlopeClamp = Cast<UMaterialExpressionClamp>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionClamp::StaticClass(), 60, 260));
		UMaterialExpressionLinearInterpolate* ColorBlend = Cast<UMaterialExpressionLinearInterpolate>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionLinearInterpolate::StaticClass(), 300, 20));
		UMaterialExpressionConstant* RoughnessNode = Cast<UMaterialExpressionConstant>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant::StaticClass(), 300, 260));

		GrassColor->Constant = FLinearColor(0.22f, 0.46f, 0.10f);
		RockColor->Constant = FLinearColor(0.24f, 0.22f, 0.18f);
		NormalZMask->B = true;
		SlopeBoost->R = 3.4f;
		SlopeClamp->MinDefault = 0.0f;
		SlopeClamp->MaxDefault = 1.0f;
		RoughnessNode->R = 0.94f;

		Material->BlendMode = BLEND_Opaque;
		Material->TwoSided = false;
		UMaterialEditingLibrary::ConnectMaterialExpressions(PixelNormal, TEXT(""), NormalZMask, TEXT("Input"));
		UMaterialEditingLibrary::ConnectMaterialExpressions(NormalZMask, TEXT(""), OneMinus, TEXT("Input"));
		UMaterialEditingLibrary::ConnectMaterialExpressions(OneMinus, TEXT(""), SlopeAlpha, TEXT("A"));
		UMaterialEditingLibrary::ConnectMaterialExpressions(SlopeBoost, TEXT(""), SlopeAlpha, TEXT("B"));
		UMaterialEditingLibrary::ConnectMaterialExpressions(SlopeAlpha, TEXT(""), SlopeClamp, TEXT("Input"));
		UMaterialEditingLibrary::ConnectMaterialExpressions(GrassColor, TEXT(""), ColorBlend, TEXT("A"));
		UMaterialEditingLibrary::ConnectMaterialExpressions(RockColor, TEXT(""), ColorBlend, TEXT("B"));
		UMaterialEditingLibrary::ConnectMaterialExpressions(SlopeClamp, TEXT(""), ColorBlend, TEXT("Alpha"));
		UMaterialEditingLibrary::ConnectMaterialProperty(ColorBlend, TEXT(""), MP_BaseColor);
		UMaterialEditingLibrary::ConnectMaterialProperty(RoughnessNode, TEXT(""), MP_Roughness);
		FinalizeMaterial(Material);
		return Material;
	}

	bool ActorOrMaterialContains(const AStaticMeshActor* Actor, const UStaticMeshComponent* MeshComponent, const FString& Needle)
	{
		if (!Actor || !MeshComponent)
		{
			return false;
		}

		if (Actor->GetName().Contains(Needle))
		{
			return true;
		}
		for (const FName& Tag : Actor->Tags)
		{
			if (Tag.ToString().Contains(Needle))
			{
				return true;
			}
		}
		for (int32 Index = 0; Index < MeshComponent->GetNumMaterials(); ++Index)
		{
			if (UMaterialInterface* Material = MeshComponent->GetMaterial(Index))
			{
				if (Material->GetName().Contains(Needle))
				{
					return true;
				}
			}
		}
		return false;
	}

	int32 SetAllSlots(UStaticMeshComponent* MeshComponent, UMaterialInterface* Material)
	{
		if (!MeshComponent || !Material)
		{
			return 0;
		}

		const int32 SlotCount = FMath::Max(1, MeshComponent->GetNumMaterials());
		for (int32 Slot = 0; Slot < SlotCount; ++Slot)
		{
			MeshComponent->SetMaterial(Slot, Material);
		}
		MeshComponent->MarkPackageDirty();
		return 1;
	}

	void HideActorRendering(AActor* Actor)
	{
		if (!Actor)
		{
			return;
		}

		Actor->Modify();
		Actor->SetActorHiddenInGame(true);
		Actor->SetIsTemporarilyHiddenInEditor(true);

		TArray<UPrimitiveComponent*> PrimitiveComponents;
		Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
		for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
		{
			if (!PrimitiveComponent)
			{
				continue;
			}
			PrimitiveComponent->Modify();
			PrimitiveComponent->SetVisibility(false, true);
			PrimitiveComponent->SetHiddenInGame(true, true);
			PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			PrimitiveComponent->MarkPackageDirty();
		}
		Actor->MarkPackageDirty();
	}

	void ShowActorRendering(AActor* Actor)
	{
		if (!Actor)
		{
			return;
		}

		Actor->Modify();
		Actor->SetActorHiddenInGame(false);
		Actor->SetIsTemporarilyHiddenInEditor(false);

		TArray<UPrimitiveComponent*> PrimitiveComponents;
		Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
		for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
		{
			if (!PrimitiveComponent)
			{
				continue;
			}
			PrimitiveComponent->Modify();
			PrimitiveComponent->SetVisibility(true, true);
			PrimitiveComponent->SetHiddenInGame(false, true);
			PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			PrimitiveComponent->MarkPackageDirty();
		}
		Actor->MarkPackageDirty();
	}

	bool IsNativeWaterBodyActor(const AActor* Actor)
	{
		if (!Actor || Actor->Tags.Contains(TEXT("FFVisualPassWaterPlaceholder")))
		{
			return false;
		}

		const FString ActorName = Actor->GetName();
		const FString ClassName = Actor->GetClass() ? Actor->GetClass()->GetName() : FString();
		return ActorName.Contains(TEXT("WaterBodyCustom"))
			|| ClassName.Contains(TEXT("WaterBodyCustom"))
			|| ActorName.Contains(TEXT("WaterBodyRiver"))
			|| ClassName.Contains(TEXT("WaterBodyRiver"))
			|| ActorName.Contains(TEXT("WaterBodyLake"))
			|| ClassName.Contains(TEXT("WaterBodyLake"));
	}

	bool IsBroadOceanActor(const AActor* Actor)
	{
		if (!Actor)
		{
			return false;
		}
		if (Actor->Tags.Contains(TEXT("FFVisualPassWaterPlaceholder")))
		{
			return false;
		}
		if (IsNativeWaterBodyActor(Actor))
		{
			const FString ActorName = Actor->GetName();
			const FString ClassName = Actor->GetClass() ? Actor->GetClass()->GetName() : FString();
			return ActorName.Contains(TEXT("WaterBodyCustom")) || ClassName.Contains(TEXT("WaterBodyCustom"));
		}

		const FString ActorName = Actor->GetName();
		const FString ClassName = Actor->GetClass() ? Actor->GetClass()->GetName() : FString();
		return ActorName.Contains(TEXT("Ocean"))
			|| ClassName.Contains(TEXT("Ocean"))
			|| ClassName.Contains(TEXT("WaterMesh"))
			|| ActorName.Contains(TEXT("WaterBodyCustom"))
			|| ClassName.Contains(TEXT("WaterBodyCustom"));
	}

	int32 SetAllPrimitiveSlots(AActor* Actor, UMaterialInterface* Material)
	{
		if (!Actor || !Material)
		{
			return 0;
		}

		int32 UpdatedComponents = 0;
		TArray<UPrimitiveComponent*> PrimitiveComponents;
		Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
		for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
		{
			if (!PrimitiveComponent)
			{
				continue;
			}

			PrimitiveComponent->Modify();
			const int32 SlotCount = FMath::Max(1, PrimitiveComponent->GetNumMaterials());
			for (int32 Slot = 0; Slot < SlotCount; ++Slot)
			{
				PrimitiveComponent->SetMaterial(Slot, Material);
			}
			PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			PrimitiveComponent->MarkPackageDirty();
			++UpdatedComponents;
		}
		Actor->MarkPackageDirty();
		return UpdatedComponents;
	}

	int32 HideTaggedVisualWaterPlaceholders(UWorld* World)
	{
		if (!World)
		{
			return 0;
		}

		int32 Hidden = 0;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			if (!It->Tags.Contains(TEXT("FFVisualPassWaterPlaceholder")))
			{
				continue;
			}
			HideActorRendering(*It);
			++Hidden;
		}
		return Hidden;
	}

	bool HasTaggedVisualWaterPlaceholder(UWorld* World)
	{
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			if (It->Tags.Contains(TEXT("FFVisualPassWaterPlaceholder")))
			{
				return true;
			}
		}
		return false;
	}

	void SpawnFlatWaterSegment(UWorld* World, UStaticMesh* CubeMesh, UMaterialInterface* WaterMaterial, const FVector2D& Start, const FVector2D& End, float Width, float WorldZ)
	{
		if (!World || !CubeMesh || !WaterMaterial)
		{
			return;
		}

		const FVector2D Mid = (Start + End) * 0.5f;
		const FVector2D Direction = (End - Start).GetSafeNormal();
		const float SegmentLength = FVector2D::Distance(Start, End);
		const FRotator Rotation(0.0f, FMath::RadiansToDegrees(FMath::Atan2(Direction.Y, Direction.X)), 0.0f);
		if (AStaticMeshActor* Segment = World->SpawnActor<AStaticMeshActor>(FVector(Mid.X, Mid.Y, WorldZ), Rotation))
		{
			Segment->Tags.AddUnique(TEXT("FFVisualPassWaterPlaceholder"));
			if (UStaticMeshComponent* MeshComponent = Segment->GetStaticMeshComponent())
			{
				MeshComponent->SetStaticMesh(CubeMesh);
				MeshComponent->SetMaterial(0, WaterMaterial);
				MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				MeshComponent->SetCastShadow(false);
				MeshComponent->SetMobility(EComponentMobility::Static);
			}
			Segment->SetActorScale3D(FVector(SegmentLength / 100.0f, Width / 100.0f, 0.04f));
			Segment->MarkPackageDirty();
		}
	}

	int32 EnsureWaterPlaceholders(UWorld* World, UMaterialInterface* WaterMaterial)
	{
		if (!World || !WaterMaterial || HasTaggedVisualWaterPlaceholder(World))
		{
			return 0;
		}

		UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
		if (!CubeMesh)
		{
			return 0;
		}

		int32 Spawned = 0;
		const TArray<TPair<FVector2D, FVector2D>> WaterSegments = {
			TPair<FVector2D, FVector2D>(FVector2D(9000.0f, 65000.0f), FVector2D(7000.0f, 48000.0f)),
			TPair<FVector2D, FVector2D>(FVector2D(7000.0f, 48000.0f), FVector2D(11000.0f, 30000.0f)),
			TPair<FVector2D, FVector2D>(FVector2D(11000.0f, 30000.0f), FVector2D(2000.0f, 10000.0f)),
			TPair<FVector2D, FVector2D>(FVector2D(2000.0f, 10000.0f), FVector2D(-3000.0f, -12000.0f)),
			TPair<FVector2D, FVector2D>(FVector2D(-3000.0f, -12000.0f), FVector2D(-1000.0f, -36000.0f))
		};
		for (const TPair<FVector2D, FVector2D>& Segment : WaterSegments)
		{
			SpawnFlatWaterSegment(World, CubeMesh, WaterMaterial, Segment.Key, Segment.Value, 1800.0f, 38.0f);
			++Spawned;
		}
		return Spawned;
	}

	int32 EnsureVoidBackdrop(UWorld* World, UMaterialInterface* SkyFallbackMaterial)
	{
		if (!World || !SkyFallbackMaterial)
		{
			return 0;
		}

		for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
		{
			if (!It->Tags.Contains(TEXT("FFVisualPassVoidBackdrop")))
			{
				continue;
			}
			HideActorRendering(*It);
			return 0;
		}

		return 0;
	}

	void ConfigureLighting(UWorld* World, int32& OutDirectionalLights, int32& OutSkyLights, int32& OutAtmospheres, int32& OutFogs)
	{
		OutDirectionalLights = 0;
		OutSkyLights = 0;
		OutAtmospheres = 0;
		OutFogs = 0;

		for (TActorIterator<ADirectionalLight> It(World); It; ++It)
		{
			++OutDirectionalLights;
			It->SetActorRotation(FRotator(-42.0f, -36.0f, 0.0f));
			if (UDirectionalLightComponent* LightComponent = It->GetComponent())
			{
				LightComponent->SetIntensity(8.0f);
				LightComponent->SetLightColor(FLinearColor(1.0f, 0.94f, 0.82f));
				LightComponent->bUseTemperature = false;
			}
			It->MarkPackageDirty();
		}

		for (TActorIterator<ASkyLight> It(World); It; ++It)
		{
			++OutSkyLights;
			if (USkyLightComponent* SkyComponent = It->GetLightComponent())
			{
				SkyComponent->SourceType = ESkyLightSourceType::SLS_CapturedScene;
				SkyComponent->SetIntensity(1.35f);
				SkyComponent->bRealTimeCapture = false;
			}
			It->MarkPackageDirty();
		}

		for (TActorIterator<ASkyAtmosphere> It(World); It; ++It)
		{
			++OutAtmospheres;
		}

		for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
		{
			++OutFogs;
			if (UExponentialHeightFogComponent* FogComponent = It->GetComponent())
			{
				FogComponent->FogDensity = 0.0007f;
				FogComponent->FogHeightFalloff = 0.22f;
				FogComponent->DirectionalInscatteringExponent = 5.0f;
			}
			It->MarkPackageDirty();
		}

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		if (OutDirectionalLights == 0)
		{
			if (ADirectionalLight* Sun = World->SpawnActor<ADirectionalLight>(FVector(-70000.0f, -30000.0f, 50000.0f), FRotator(-42.0f, -36.0f, 0.0f), SpawnParameters))
			{
				++OutDirectionalLights;
				if (UDirectionalLightComponent* LightComponent = Sun->GetComponent())
				{
					LightComponent->SetIntensity(8.0f);
					LightComponent->SetLightColor(FLinearColor(1.0f, 0.94f, 0.82f));
				}
			}
		}
		if (OutSkyLights == 0)
		{
			if (ASkyLight* Sky = World->SpawnActor<ASkyLight>(FVector(0.0f, 0.0f, 12000.0f), FRotator::ZeroRotator, SpawnParameters))
			{
				++OutSkyLights;
				if (USkyLightComponent* SkyComponent = Sky->GetLightComponent())
				{
					SkyComponent->SourceType = ESkyLightSourceType::SLS_CapturedScene;
					SkyComponent->SetIntensity(1.35f);
					SkyComponent->bRealTimeCapture = false;
				}
			}
		}
		if (OutAtmospheres == 0 && World->SpawnActor<ASkyAtmosphere>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters))
		{
			++OutAtmospheres;
		}
		if (OutFogs == 0)
		{
			if (AExponentialHeightFog* Fog = World->SpawnActor<AExponentialHeightFog>(FVector(0.0f, 0.0f, -12000.0f), FRotator::ZeroRotator, SpawnParameters))
			{
				++OutFogs;
				if (UExponentialHeightFogComponent* FogComponent = Fog->GetComponent())
				{
					FogComponent->FogDensity = 0.0007f;
					FogComponent->FogHeightFalloff = 0.22f;
				}
			}
		}
	}
}
#endif

int32 UFFStarterHighlandVisualPassCommandlet::Main(const FString& Params)
{
#if WITH_EDITOR
	UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVisualPass: loading %s without terrain regeneration."), *HighlandMapPath);

	UWorld* World = UEditorLoadingAndSavingUtils::LoadMap(HighlandMapPath);
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandVisualPass: failed to load %s"), *HighlandMapPath);
		return 1;
	}

	if (Params.Contains(TEXT("LogActors")))
	{
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			const AActor* Actor = *It;
			UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVisualPassActor name=%s class=%s loc=(%.1f, %.1f, %.1f) tags=%s"),
				*Actor->GetName(),
				Actor->GetClass() ? *Actor->GetClass()->GetName() : TEXT("None"),
				Actor->GetActorLocation().X,
				Actor->GetActorLocation().Y,
				Actor->GetActorLocation().Z,
				*FString::JoinBy(Actor->Tags, TEXT(","), [](const FName& Tag) { return Tag.ToString(); }));
		}
	}

	if (Params.Contains(TEXT("OuterRingMaterialV50")))
	{
		const FString GrassTypeObjectPath = GetObjectPath(MaterialFolderPath, GrassTypeName);
		ULandscapeGrassType* GrassType = LoadObject<ULandscapeGrassType>(nullptr, *GrassTypeObjectPath);
		if (!GrassType)
		{
			UMaterialInterface* TitanGrassBladeMaterial = LoadObject<UMaterialInterface>(nullptr, TitanGrassBladeMaterialPath);
			TArray<UStaticMesh*> TitanGrassMeshes;
			TitanGrassMeshes.Add(LoadObject<UStaticMesh>(nullptr, TitanGrassBladeMeshPath));
			TitanGrassMeshes.Add(LoadObject<UStaticMesh>(nullptr, TitanRyegrassAMeshPath));
			TitanGrassMeshes.Add(LoadObject<UStaticMesh>(nullptr, TitanRyegrassCMeshPath));
			GrassType = CreateOrUpdateGrassType(TitanGrassBladeMaterial, TitanGrassMeshes);
		}
		if (!GrassType)
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandVisualPass v50: missing grass type; refusing to drop the proven native grass output."));
			return 1;
		}

		UMaterial* V50LandscapeMaterial = BuildBiomeLandscapeMaterial(GrassType);
		if (!V50LandscapeMaterial)
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandVisualPass v50: failed to build slope-converted landscape material."));
			return 1;
		}

		int32 LandscapeCount = 0;
		int32 LandscapeComponentCount = 0;
		for (TActorIterator<ALandscapeProxy> It(World); It; ++It)
		{
			++LandscapeCount;
			It->Modify();
			It->LandscapeMaterial = V50LandscapeMaterial;
			for (ULandscapeComponent* LandscapeComponent : It->LandscapeComponents)
			{
				if (!LandscapeComponent)
				{
					continue;
				}
				++LandscapeComponentCount;
				LandscapeComponent->Modify();
				LandscapeComponent->OverrideMaterial = V50LandscapeMaterial;
				LandscapeComponent->MaterialInstances.Empty();
				LandscapeComponent->UpdateMaterialInstances();
				LandscapeComponent->UpdateGrassTypes(true);
				LandscapeComponent->MarkPackageDirty();
			}
			It->InvalidateGeneratedComponentData(false);
			It->FlushGrassComponents(nullptr, true);
			It->PostEditChange();
			It->MarkPackageDirty();
		}

		World->MarkPackageDirty();
		const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
		const bool bSavedAssets = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVisualPass v50: material-only outer-ring conversion landscapes=%d components=%d material=%s grassType=%s newStaticMeshActors=0 waterActorsTouched=0 playerStartsTouched=0"),
			LandscapeCount,
			LandscapeComponentCount,
			*V50LandscapeMaterial->GetPathName(),
			*GrassType->GetPathName());
		return (bSavedMap && bSavedAssets && LandscapeCount > 0) ? 0 : 1;
	}

	UMaterial* RockMaterial = BuildSolidMaterial(RockMaterialName, FLinearColor(0.34f, 0.34f, 0.30f), 0.97f);
	UMaterial* WaterMaterial = BuildWaterMaterial();
	UMaterial* SkyFallbackMaterial = BuildSolidMaterial(SkyFallbackMaterialName, FLinearColor(0.11f, 0.16f, 0.20f), 0.98f);
	UMaterial* PathMaterial = BuildSolidMaterial(PathMaterialName, FLinearColor(0.58f, 0.47f, 0.29f), 0.90f);
	UMaterial* MarkerMaterial = BuildSolidMaterial(MarkerMaterialName, FLinearColor(0.95f, 0.78f, 0.34f), 0.58f);
	UMaterial* ForestMassMaterial = BuildSolidMaterial(ForestMassMaterialName, FLinearColor(0.05f, 0.25f, 0.06f), 0.92f);
	UMaterial* TreeCanopyMaterial = BuildSolidMaterial(TreeCanopyMaterialName, FLinearColor(0.03f, 0.18f, 0.04f), 0.90f);
	UMaterial* TreeTrunkMaterial = BuildSolidMaterial(TreeTrunkMaterialName, FLinearColor(0.27f, 0.17f, 0.08f), 0.86f);
	UMaterialInterface* TitanCliffMaterial = LoadObject<UMaterialInterface>(nullptr, TitanCliffMaterialPath);
	UMaterialInterface* TitanGrassBaseMaterial = LoadObject<UMaterialInterface>(nullptr, TitanGrassBaseMaterialPath);
	UMaterialInterface* TitanGrassBladeMaterial = LoadObject<UMaterialInterface>(nullptr, TitanGrassBladeMaterialPath);
	UMaterialInterface* TitanWaterMaterial = LoadObject<UMaterialInterface>(nullptr, TitanWaterCustomMeshMaterialPath);
	TArray<UStaticMesh*> TitanGrassMeshes;
	TitanGrassMeshes.Add(LoadObject<UStaticMesh>(nullptr, TitanGrassBladeMeshPath));
	TitanGrassMeshes.Add(LoadObject<UStaticMesh>(nullptr, TitanRyegrassAMeshPath));
	TitanGrassMeshes.Add(LoadObject<UStaticMesh>(nullptr, TitanRyegrassCMeshPath));
	ULandscapeGrassType* GrassType = CreateOrUpdateGrassType(TitanGrassBladeMaterial, TitanGrassMeshes);
	UMaterial* GrassMaterial = BuildBiomeLandscapeMaterial(GrassType);
	if (!GrassMaterial || !GrassType || !TitanCliffMaterial || !TitanGrassBaseMaterial || !TitanGrassBladeMaterial || !TitanWaterMaterial || !RockMaterial || !WaterMaterial || !SkyFallbackMaterial || !PathMaterial || !MarkerMaterial || !ForestMassMaterial || !TreeCanopyMaterial || !TreeTrunkMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandVisualPass: failed to load one or more strict-list Titan materials or create fallback support materials."));
		return 1;
	}
	UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVisualPass: strictTitanAssets cliff=%s grassBase=%s grassBladeMat=%s water=%s grassMeshes=%d"),
		*TitanCliffMaterial->GetPathName(),
		*TitanGrassBaseMaterial->GetPathName(),
		*TitanGrassBladeMaterial->GetPathName(),
		*TitanWaterMaterial->GetPathName(),
		TitanGrassMeshes.Num());

	int32 LandscapeCount = 0;
	for (TActorIterator<ALandscapeProxy> It(World); It; ++It)
	{
		++LandscapeCount;
		It->Modify();
		It->LandscapeMaterial = GrassMaterial;
		for (ULandscapeComponent* LandscapeComponent : It->LandscapeComponents)
		{
			if (!LandscapeComponent)
			{
				continue;
			}
			LandscapeComponent->Modify();
			LandscapeComponent->OverrideMaterial = GrassMaterial;
			LandscapeComponent->MaterialInstances.Empty();
			LandscapeComponent->UpdateMaterialInstances();
			LandscapeComponent->UpdateGrassTypes(true);
			LandscapeComponent->MarkPackageDirty();
		}
		It->InvalidateGeneratedComponentData(false);
		It->FlushGrassComponents(nullptr, true);
		It->PostEditChange();
		It->MarkPackageDirty();
	}

	int32 WaterActors = 0;
	int32 RockActors = 0;
	int32 PathActors = 0;
	int32 MarkerActors = 0;
	int32 ForestActors = 0;
	int32 OceanActorsHidden = 0;
	int32 NativeWaterActors = 0;
	int32 HiddenWaterPlaceholders = HideTaggedVisualWaterPlaceholders(World);
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (IsNativeWaterBodyActor(Actor))
		{
			// Keep the existing WaterBodyCustom2 visibility/state unchanged; only swap material if the actor is already visible.
			if (!Actor->IsHidden())
			{
				NativeWaterActors += SetAllPrimitiveSlots(Actor, TitanWaterMaterial);
			}
			continue;
		}
		if (IsBroadOceanActor(Actor))
		{
			HideActorRendering(Actor);
			++OceanActorsHidden;
			continue;
		}
	}
	for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
	{
		AStaticMeshActor* Actor = *It;
		UStaticMeshComponent* MeshComponent = Actor ? Actor->GetStaticMeshComponent() : nullptr;
		if (!Actor || !MeshComponent)
		{
			continue;
		}
		if (Actor->Tags.Contains(TEXT("FFVisualPassWaterPlaceholder")))
		{
			HideActorRendering(Actor);
			continue;
		}

		if (!IsNativeWaterBodyActor(Actor) && (IsBroadOceanActor(Actor) || ActorOrMaterialContains(Actor, MeshComponent, TEXT("Ocean")) || ActorOrMaterialContains(Actor, MeshComponent, TEXT("Sea"))))
		{
			HideActorRendering(Actor);
			Actor->MarkPackageDirty();
			continue;
		}
		if (ActorOrMaterialContains(Actor, MeshComponent, TEXT("River")) || ActorOrMaterialContains(Actor, MeshComponent, TEXT("Water")))
		{
			WaterActors += SetAllSlots(MeshComponent, TitanWaterMaterial);
			continue;
		}
		if (ActorOrMaterialContains(Actor, MeshComponent, TEXT("Canyon")) || ActorOrMaterialContains(Actor, MeshComponent, TEXT("Rock")) || ActorOrMaterialContains(Actor, MeshComponent, TEXT("Cliff")))
		{
			RockActors += SetAllSlots(MeshComponent, TitanCliffMaterial);
			continue;
		}
		if (ActorOrMaterialContains(Actor, MeshComponent, TEXT("Path")))
		{
			PathActors += SetAllSlots(MeshComponent, PathMaterial);
			continue;
		}
		if (ActorOrMaterialContains(Actor, MeshComponent, TEXT("Village")) || ActorOrMaterialContains(Actor, MeshComponent, TEXT("Bridge")) || ActorOrMaterialContains(Actor, MeshComponent, TEXT("BossGate")))
		{
			MarkerActors += SetAllSlots(MeshComponent, MarkerMaterial);
			continue;
		}
		if (ActorOrMaterialContains(Actor, MeshComponent, TEXT("ForestMass")))
		{
			ForestActors += SetAllSlots(MeshComponent, ForestMassMaterial);
			continue;
		}
		if (ActorOrMaterialContains(Actor, MeshComponent, TEXT("TreeCanopy")))
		{
			ForestActors += SetAllSlots(MeshComponent, TreeCanopyMaterial);
			continue;
		}
		if (ActorOrMaterialContains(Actor, MeshComponent, TEXT("TreeTrunk")))
		{
			ForestActors += SetAllSlots(MeshComponent, TreeTrunkMaterial);
			continue;
		}
	}
	const int32 WaterPlaceholdersAdded = 0;
	const int32 VoidBackdropAdded = EnsureVoidBackdrop(World, SkyFallbackMaterial);

	int32 DirectionalLights = 0;
	int32 SkyLights = 0;
	int32 Atmospheres = 0;
	int32 Fogs = 0;
	ConfigureLighting(World, DirectionalLights, SkyLights, Atmospheres, Fogs);

	FVector FirstPlayerStart = FVector::ZeroVector;
	int32 PlayerStartCount = 0;
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		++PlayerStartCount;
		if (PlayerStartCount == 1)
		{
			FirstPlayerStart = It->GetActorLocation();
		}
	}

	World->MarkPackageDirty();
	const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
	const bool bSavedAssets = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);

	UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVisualPass: landscapes=%d waterActors=%d nativeWaterComponents=%d rockActors=%d pathActors=%d markerActors=%d forestActors=%d hiddenOceanActors=%d hiddenWaterPlaceholders=%d waterPlaceholdersAdded=%d voidBackdropAdded=%d"),
		LandscapeCount, WaterActors, NativeWaterActors, RockActors, PathActors, MarkerActors, ForestActors, OceanActorsHidden, HiddenWaterPlaceholders, WaterPlaceholdersAdded, VoidBackdropAdded);
	UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandVisualPass: lights directional=%d skylight=%d atmosphere=%d fog=%d playerStarts=%d firstPlayerStart=(%.1f, %.1f, %.1f)"),
		DirectionalLights, SkyLights, Atmospheres, Fogs, PlayerStartCount, FirstPlayerStart.X, FirstPlayerStart.Y, FirstPlayerStart.Z);

	return (bSavedMap && bSavedAssets) ? 0 : 1;
#else
	UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandVisualPass can only run in editor builds."));
	return 1;
#endif
}
