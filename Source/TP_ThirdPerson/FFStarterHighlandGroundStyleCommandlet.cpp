#include "FFStarterHighlandGroundStyleCommandlet.h"

#if WITH_EDITOR
#include "AssetToolsModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Factories/MaterialFactoryNew.h"
#include "FileHelpers.h"
#include "LandscapeComponent.h"
#include "LandscapeProxy.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionCustom.h"
#include "Materials/MaterialExpressionLocalPosition.h"
#include "Materials/MaterialExpressionPerInstanceRandom.h"
#include "Materials/MaterialExpressionTime.h"
#include "Materials/MaterialExpressionVertexNormalWS.h"
#include "Materials/MaterialExpressionWorldPosition.h"
#include "MaterialEditingLibrary.h"
#include "Misc/PackageName.h"
#include "Modules/ModuleManager.h"
#endif

UFFStarterHighlandGroundStyleCommandlet::UFFStarterHighlandGroundStyleCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

#if WITH_EDITOR
namespace
{
	const FString HighlandMapPath = TEXT("/Game/Maps/FF_Starter_Highland_Blockout");
	const FString MaterialFolderPath = TEXT("/Game/FantasyFrontier/Blockout/Materials");
	const FString LandscapeMaterialName = TEXT("M_FF_Blockout_Grass_Green");
	const FString SoftGrassBladeMaterialName = TEXT("M_FF_Highland_GrassBlade_Soft");
	const FName PipelineProofTag(TEXT("FFTitanGrasslandPipelineProof"));

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

	UMaterial* BuildSoftGrassBladeMaterial()
	{
		UMaterial* Material = CreateOrLoadMaterial(SoftGrassBladeMaterialName);
		if (!Material)
		{
			return nullptr;
		}

		UMaterialExpressionWorldPosition* WorldPosition = AddWorldPositionNode(Material, -760, -80);
		UMaterialExpressionLocalPosition* LocalPosition = Cast<UMaterialExpressionLocalPosition>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionLocalPosition::StaticClass(), -760, -200));
		UMaterialExpressionPerInstanceRandom* InstanceRandom = Cast<UMaterialExpressionPerInstanceRandom>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionPerInstanceRandom::StaticClass(), -760, 40));
		UMaterialExpressionCustom* GrassColor = Cast<UMaterialExpressionCustom>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionCustom::StaticClass(), -480, -65));
		UMaterialExpressionCustom* GrassEmissive = Cast<UMaterialExpressionCustom>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionCustom::StaticClass(), -210, -55));
		UMaterialExpressionConstant* Roughness = Cast<UMaterialExpressionConstant>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant::StaticClass(), -360, 120));
		UMaterialExpressionConstant* Specular = Cast<UMaterialExpressionConstant>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant::StaticClass(), -360, 220));
		UMaterialExpressionTime* Time = Cast<UMaterialExpressionTime>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionTime::StaticClass(), -700, 140));
		UMaterialExpressionCustom* WindNode = Cast<UMaterialExpressionCustom>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionCustom::StaticClass(), -430, 55));
		if (!WorldPosition || !LocalPosition || !InstanceRandom || !GrassColor || !GrassEmissive || !Roughness || !Specular || !Time || !WindNode)
		{
			return nullptr;
		}

		GrassColor->Description = TEXT("Cook-safe Titan grass blade color variation");
		GrassColor->OutputType = CMOT_Float3;
		ConnectCustomInput(GrassColor, WorldPosition, TEXT("WorldPos"));
		ConnectCustomInput(GrassColor, LocalPosition, TEXT("LocalPos"));
		ConnectCustomInput(GrassColor, InstanceRandom, TEXT("InstanceRandom"));
		GrassColor->Code = TEXT(R"(
float2 p = WorldPos.xy;
float broadA = 0.5 + 0.5 * sin(p.x / 43000.0 + p.y / 69000.0 + 0.9 * sin(p.y / 52000.0));
float broadB = 0.5 + 0.5 * sin(-p.x / 62000.0 + p.y / 37000.0 + 1.7);
float mid = 0.5 + 0.5 * sin(p.x / 12600.0 - p.y / 17100.0 + InstanceRandom * 6.28318);
float groundBlend = saturate(broadA * 0.48 + broadB * 0.34 + mid * 0.18);
float bladeHeight = smoothstep(-4.0, 72.0, LocalPos.z);

float3 groundOlive = float3(0.490, 0.555, 0.215);
float3 groundSun = float3(0.620, 0.635, 0.295);
float3 groundShade = float3(0.420, 0.505, 0.180);
float3 strawTip = float3(0.660, 0.635, 0.335);
float3 groundTint = lerp(groundShade, groundSun, saturate(groundBlend * 0.66 + 0.20));
groundTint = lerp(groundTint, groundOlive, 0.34);

float3 rootColor = groundTint * float3(0.94, 0.98, 0.88);
float3 tipColor = lerp(groundTint, strawTip, saturate(0.10 + mid * 0.12 + InstanceRandom * 0.025));
float3 color = lerp(rootColor, tipColor, saturate(bladeHeight * 0.42));
color *= lerp(0.985, 1.025, broadA * 0.60 + broadB * 0.40);
return color;
)");
		GrassEmissive->Description = TEXT("Soft grass fill light to avoid harsh black striping");
		GrassEmissive->OutputType = CMOT_Float3;
		ConnectCustomInput(GrassEmissive, GrassColor, TEXT("BaseColor"));
		GrassEmissive->Code = TEXT("return BaseColor * 0.255;");
		Roughness->R = 0.96f;
		Specular->R = 0.05f;
		WindNode->Description = TEXT("Lightweight local Titan-style grass sway");
		WindNode->OutputType = CMOT_Float3;
		ConnectCustomInput(WindNode, WorldPosition, TEXT("WorldPos"));
		ConnectCustomInput(WindNode, LocalPosition, TEXT("LocalPos"));
		ConnectCustomInput(WindNode, Time, TEXT("Time"));
		ConnectCustomInput(WindNode, InstanceRandom, TEXT("InstanceRandom"));
		WindNode->Code = TEXT(R"(
float randomPhase = InstanceRandom * 6.28318;
float rootMask = smoothstep(4.0, 82.0, LocalPos.z);
rootMask *= rootMask;
float2 baseDir = normalize(float2(0.72, 0.42));
float dirAngle = (InstanceRandom - 0.5) * 0.95;
float s = sin(dirAngle);
float c = cos(dirAngle);
float2 windDir = normalize(float2(baseDir.x * c - baseDir.y * s, baseDir.x * s + baseDir.y * c));
float broadGust = 0.5 + 0.5 * sin(dot(WorldPos.xy, float2(0.0018, 0.0012)) + Time * 0.58);
float phaseA = dot(WorldPos.xy, float2(0.0095, 0.0140)) + Time * (1.18 + InstanceRandom * 0.30) + randomPhase;
float phaseB = dot(WorldPos.xy, float2(-0.0150, 0.0070)) + Time * (0.62 + InstanceRandom * 0.20) + randomPhase * 1.7;
float phaseC = dot(WorldPos.xy, float2(0.0220, -0.0045)) + Time * 1.85 + randomPhase * 0.33;
float sway = sin(phaseA) * 0.54 + sin(phaseB) * 0.30 + sin(phaseC) * 0.16;
float bend = lerp(2.6, 6.4, InstanceRandom) * lerp(0.58, 1.12, broadGust) * sway * rootMask;
return float3(windDir.x * bend, windDir.y * bend, 0.0);
)");
		Material->BlendMode = BLEND_Opaque;
		Material->TwoSided = true;
		Material->bUsedWithInstancedStaticMeshes = true;
		Material->bUsedWithNanite = true;
		UMaterialEditingLibrary::ConnectMaterialProperty(GrassColor, TEXT(""), MP_BaseColor);
		UMaterialEditingLibrary::ConnectMaterialProperty(GrassEmissive, TEXT(""), MP_EmissiveColor);
		UMaterialEditingLibrary::ConnectMaterialProperty(Roughness, TEXT(""), MP_Roughness);
		UMaterialEditingLibrary::ConnectMaterialProperty(Specular, TEXT(""), MP_Specular);
		UMaterialEditingLibrary::ConnectMaterialProperty(WindNode, TEXT(""), MP_WorldPositionOffset);
		FinalizeMaterial(Material);
		return Material;
	}

	UMaterial* BuildTitanStyleLandscapeMaterial()
	{
		UMaterial* Material = CreateOrLoadMaterial(LandscapeMaterialName);
		if (!Material)
		{
			return nullptr;
		}

		UMaterialExpressionWorldPosition* WorldPosition = AddWorldPositionNode(Material, -860, -120);
		UMaterialExpressionVertexNormalWS* VertexNormal = Cast<UMaterialExpressionVertexNormalWS>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionVertexNormalWS::StaticClass(), -860, 80));
		UMaterialExpressionCustom* ColorNode = Cast<UMaterialExpressionCustom>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionCustom::StaticClass(), -500, -90));
		UMaterialExpressionConstant* Roughness = Cast<UMaterialExpressionConstant>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant::StaticClass(), -500, 210));
		if (!WorldPosition || !VertexNormal || !ColorNode || !Roughness)
		{
			return nullptr;
		}

		ColorNode->Description = TEXT("V82 Titan-style Highland biome readability: soft zone identity without terrain edits");
		ColorNode->OutputType = CMOT_Float3;
		ConnectCustomInput(ColorNode, WorldPosition, TEXT("WorldPos"));
		ConnectCustomInput(ColorNode, VertexNormal, TEXT("NormalWS"));
		ColorNode->Code = TEXT(R"(
#define DISTSEG(P,A,B) length((P) - ((A) + saturate(dot((P) - (A), (B) - (A)) / max(dot((B) - (A), (B) - (A)), 1.0)) * ((B) - (A))))
#define ELLIPSE(P,C,R) length(((P) - (C)) / (R))
float2 p = WorldPos.xy;
float2 rel = p - float2(65000.0, 98000.0);
float slope = 1.0 - saturate(NormalWS.z);
float height = smoothstep(-6500.0, 2500.0, WorldPos.z);

// Continuous low-frequency bands only: no floor/grid quantization.
float broadA = 0.5 + 0.5 * sin(rel.x / 43000.0 + rel.y / 69000.0 + 0.9 * sin(rel.y / 52000.0));
float broadB = 0.5 + 0.5 * sin(-rel.x / 62000.0 + rel.y / 37000.0 + 1.7);
float broadC = 0.5 + 0.5 * sin((rel.x + rel.y) / 88000.0 - 0.65 * cos(rel.x / 47000.0));
float broad = saturate(broadA * 0.42 + broadB * 0.34 + broadC * 0.24);
float fineA = 0.5 + 0.5 * sin(p.x * 0.00043 - p.y * 0.00031 + broadA * 2.7);
float fineB = 0.5 + 0.5 * sin(p.x * -0.00029 + p.y * 0.00037 + broadB * 2.1);
float ecologyFlow = saturate(broad * 0.48 + fineA * 0.28 + fineB * 0.24);
float lowPatch = 1.0 - height;
float slopeLight = smoothstep(0.045, 0.18, slope) * (1.0 - smoothstep(0.28, 0.46, slope));

float path = 0.0;
path = max(path, 1.0 - smoothstep(900.0, 2450.0, DISTSEG(p, float2(38550.0, 147850.0), float2(52500.0, 143500.0))));
path = max(path, 1.0 - smoothstep(900.0, 2500.0, DISTSEG(p, float2(52500.0, 143500.0), float2(77200.0, 124500.0))));
path = max(path, 1.0 - smoothstep(1000.0, 3000.0, DISTSEG(p, float2(77200.0, 124500.0), float2(72000.0, 104000.0))));
path = max(path, 1.0 - smoothstep(1000.0, 3200.0, DISTSEG(p, float2(72000.0, 104000.0), float2(61000.0, 76000.0))));
path = max(path, 1.0 - smoothstep(1000.0, 3150.0, DISTSEG(p, float2(61000.0, 76000.0), float2(57584.0, 62136.0))));
path = max(path, 1.0 - smoothstep(1000.0, 3100.0, DISTSEG(p, float2(57584.0, 62136.0), float2(63147.0, 37260.0))));
path = max(path, 1.0 - smoothstep(900.0, 2550.0, DISTSEG(p, float2(72000.0, 104000.0), float2(93000.0, 137000.0))));
)")
TEXT(R"(
float riverDistance = 999999.0;
riverDistance = min(riverDistance, DISTSEG(p, float2(48590.0, 183116.0), float2(49910.0, 174430.0)));
riverDistance = min(riverDistance, DISTSEG(p, float2(49910.0, 174430.0), float2(51518.0, 166778.0)));
riverDistance = min(riverDistance, DISTSEG(p, float2(51518.0, 166778.0), float2(58605.0, 152688.0)));
riverDistance = min(riverDistance, DISTSEG(p, float2(58605.0, 152688.0), float2(62544.0, 138795.0)));
riverDistance = min(riverDistance, DISTSEG(p, float2(62544.0, 138795.0), float2(58487.0, 122723.0)));
riverDistance = min(riverDistance, DISTSEG(p, float2(58487.0, 122723.0), float2(55306.0, 96223.0)));
riverDistance = min(riverDistance, DISTSEG(p, float2(55306.0, 96223.0), float2(54105.0, 78933.0)));
riverDistance = min(riverDistance, DISTSEG(p, float2(54105.0, 78933.0), float2(57584.0, 62136.0)));
riverDistance = min(riverDistance, DISTSEG(p, float2(57584.0, 62136.0), float2(63147.0, 37260.0)));
float riverWater = 1.0 - smoothstep(1850.0, 3800.0, riverDistance);
float riverBasin = 1.0 - smoothstep(7200.0, 35500.0, riverDistance);
float2 pondA = (p - float2(45457.0, 136962.0)) / float2(4400.0, 2300.0);
float2 pondB = (p - float2(19220.0, 124250.0)) / float2(3600.0, 2100.0);
float2 lake0 = (p - float2(40000.0, 23500.0)) / float2(17000.0, 12500.0);
float2 lake1 = (p - float2(25640.0, 72820.0)) / float2(5200.0, 2400.0);
float pondLakeWater = 0.0;
pondLakeWater = max(pondLakeWater, 1.0 - smoothstep(0.80, 1.12, length(pondA)));
pondLakeWater = max(pondLakeWater, 1.0 - smoothstep(0.80, 1.12, length(pondB)));
pondLakeWater = max(pondLakeWater, 1.0 - smoothstep(0.78, 1.12, length(lake0)));
pondLakeWater = max(pondLakeWater, 1.0 - smoothstep(0.78, 1.12, length(lake1)));
float water = max(riverWater, pondLakeWater);
float lakeShore = max(1.0 - smoothstep(1.04, 1.40, length(lake0)), 1.0 - smoothstep(1.04, 1.38, length(lake1)));
lakeShore = max(lakeShore, max(1.0 - smoothstep(1.03, 1.36, length(pondA)), 1.0 - smoothstep(1.03, 1.36, length(pondB))));

float villageTerrace = 1.0 - smoothstep(0.55, 1.08, ELLIPSE(p, float2(77200.0, 124500.0), float2(42000.0, 29500.0)));
float trainingPlateau = 1.0 - smoothstep(0.50, 1.07, ELLIPSE(p, float2(52500.0, 143500.0), float2(32500.0, 17000.0)));
float openPlains = 1.0 - smoothstep(0.54, 1.16, ELLIPSE(p, float2(72000.0, 104000.0), float2(62000.0, 38500.0)));
float forestBasin = 1.0 - smoothstep(0.50, 1.16, ELLIPSE(p, float2(61000.0, 76000.0), float2(51000.0, 36000.0)));
float secondaryShoulder = 1.0 - smoothstep(0.48, 1.12, ELLIPSE(p, float2(93000.0, 137000.0), float2(34500.0, 21500.0)));
float southGorge = max(1.0 - smoothstep(6800.0, 31000.0, DISTSEG(p, float2(57584.0, 62136.0), float2(63147.0, 37260.0))),
                       1.0 - smoothstep(0.52, 1.18, ELLIPSE(p, float2(67500.0, 40500.0), float2(29500.0, 20500.0))));
float terraceEdge = smoothstep(0.58, 0.86, ELLIPSE(p, float2(77200.0, 124500.0), float2(42000.0, 29500.0)))
                  * (1.0 - smoothstep(0.92, 1.20, ELLIPSE(p, float2(77200.0, 124500.0), float2(42000.0, 29500.0))));
float forestLip = smoothstep(0.62, 0.96, ELLIPSE(p, float2(61000.0, 76000.0), float2(51000.0, 36000.0)))
                * (1.0 - smoothstep(1.0, 1.26, ELLIPSE(p, float2(61000.0, 76000.0), float2(51000.0, 36000.0))));

float shore = saturate(max(riverBasin * 0.48, lakeShore * 0.58) * (1.0 - water));
float rock = smoothstep(0.20, 0.46, slope);
float rimRock = smoothstep(0.62, 0.98, length(rel / float2(116000.0, 126000.0))) * smoothstep(0.10, 0.28, slope);
float shoulderRock = secondaryShoulder * smoothstep(0.05, 0.22, slope) * 0.52;
float gorgeRock = southGorge * smoothstep(0.04, 0.19, slope) * 0.70;
rock = saturate(max(rock, rimRock));
rock = saturate(max(rock, max(shoulderRock, gorgeRock)));
)")
TEXT(R"(
float3 grassBase = float3(0.300, 0.435, 0.130);
float3 grassSun = float3(0.590, 0.660, 0.260);
float3 grassOpen = float3(0.430, 0.610, 0.180);
float3 grassLow = float3(0.205, 0.365, 0.095);
float3 grassShade = float3(0.075, 0.215, 0.052);
float3 grassWarm = float3(0.620, 0.565, 0.220);
float3 grass = lerp(grassBase, grassOpen, saturate(0.22 + broad * 0.22 + ecologyFlow * 0.18));
grass = lerp(grass, grassLow, saturate(lowPatch * 0.10 + (1.0 - broad) * 0.08));
grass = lerp(grass, grassSun, saturate(openPlains * 0.56 + slopeLight * 0.10 + height * 0.05));
grass = lerp(grass, grassWarm, saturate(villageTerrace * 0.62 + terraceEdge * 0.22));
grass = lerp(grass, float3(0.485, 0.510, 0.165), saturate(trainingPlateau * 0.48));
grass = lerp(grass, float3(0.080, 0.245, 0.062), saturate(forestBasin * 0.76 + forestLip * 0.22));
grass = lerp(grass, float3(0.335, 0.365, 0.112), saturate(secondaryShoulder * 0.62));
grass = lerp(grass, float3(0.105, 0.255, 0.085), saturate(riverBasin * 0.58));
grass = lerp(grass, float3(0.405, 0.295, 0.145), saturate(southGorge * 0.72));
grass = lerp(grass, grassShade, saturate((1.0 - broad) * lowPatch * 0.055 + forestBasin * 0.20));
grass *= lerp(0.91, 1.08, saturate(fineA * 0.34 + fineB * 0.25 + broad * 0.41));
grass = lerp(grass, grass * float3(1.09, 1.04, 0.83), saturate((1.0 - forestBasin) * (1.0 - riverBasin) * (1.0 - southGorge) * ecologyFlow * 0.11));

float3 pathColor = float3(0.470, 0.375, 0.205);
float3 shoreColor = lerp(float3(0.175, 0.230, 0.115), float3(0.085, 0.140, 0.075), saturate(riverBasin * 0.82 + lakeShore * 0.34));
float3 gorgeFloor = float3(0.440, 0.305, 0.160);
float3 rockLow = float3(0.235, 0.205, 0.160);
float3 rockMid = float3(0.355, 0.310, 0.225);
float3 rockHigh = float3(0.555, 0.485, 0.355);
float rockVariation = saturate(0.35 + broad * 0.36 + fineA * 0.14 + (0.5 + 0.5 * sin((p.x - p.y) / 58000.0 + WorldPos.z * 0.0012)) * 0.10);
float3 rockColor = lerp(rockLow, rockHigh, saturate(0.28 + slope * 0.58 + rockVariation * 0.18));
rockColor = lerp(rockColor, rockMid, saturate((secondaryShoulder + southGorge) * 0.26));
rockColor = lerp(rockColor, rockColor * float3(0.76, 0.86, 0.82), saturate(forestBasin * slope * 0.22));
rockColor = lerp(rockColor, rockColor * float3(1.13, 0.96, 0.76), saturate(southGorge * 0.24 + secondaryShoulder * 0.12));

float3 color = grass;
color = lerp(color, shoreColor, shore * (1.0 - path));
color = lerp(color, gorgeFloor, saturate(southGorge * 0.34 * (1.0 - water)));
color = lerp(color, pathColor, saturate(path * 0.42 * (1.0 - water * 0.40)));
color = lerp(color, rockColor, saturate(rock * (1.0 - path * 0.85)));
color = lerp(color, float3(0.105, 0.145, 0.090), saturate(water * 0.22));
return color;
#undef DISTSEG
#undef ELLIPSE
)");

		Roughness->R = 0.96f;
		Material->BlendMode = BLEND_Opaque;
		Material->TwoSided = false;
		UMaterialEditingLibrary::ConnectMaterialProperty(ColorNode, TEXT(""), MP_BaseColor);
		UMaterialEditingLibrary::ConnectMaterialProperty(Roughness, TEXT(""), MP_Roughness);
		FinalizeMaterial(Material);
		return Material;
	}

	int32 RemoveInactivePipelineProofActors(UWorld* World)
	{
		if (!World)
		{
			return 0;
		}

		TArray<AActor*> ActorsToRemove;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (!Actor || !Actor->ActorHasTag(PipelineProofTag))
			{
				continue;
			}

			const FString Label = Actor->GetActorLabel();
			if (Label.Contains(TEXT("FF_TitanPipeline_B_NativeGrassland")) ||
				Label.Contains(TEXT("FF_TitanPipeline_C_StrippedGrassland")))
			{
				ActorsToRemove.Add(Actor);
			}
		}

		for (AActor* Actor : ActorsToRemove)
		{
			World->DestroyActor(Actor);
		}

		return ActorsToRemove.Num();
	}
}
#endif

int32 UFFStarterHighlandGroundStyleCommandlet::Main(const FString& Params)
{
#if WITH_EDITOR
	UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandGroundStyle: updating Highland ground/grass materials only."));
	const bool bV82BiomeFoundationOnly = Params.Contains(TEXT("V82BiomeFoundation"), ESearchCase::IgnoreCase);
	UWorld* World = UEditorLoadingAndSavingUtils::LoadMap(HighlandMapPath);
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandGroundStyle: failed to load %s"), *HighlandMapPath);
		return 1;
	}

	UMaterial* LandscapeMaterial = BuildTitanStyleLandscapeMaterial();
	UMaterial* SoftGrassMaterial = bV82BiomeFoundationOnly
		? LoadObject<UMaterial>(nullptr, *GetObjectPath(MaterialFolderPath, SoftGrassBladeMaterialName))
		: BuildSoftGrassBladeMaterial();
	if (!LandscapeMaterial || (!SoftGrassMaterial && !bV82BiomeFoundationOnly))
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandGroundStyle: failed to create material assets."));
		return 1;
	}

	const int32 RemovedInactivePipelineProofActors = bV82BiomeFoundationOnly ? 0 : RemoveInactivePipelineProofActors(World);

	int32 LandscapeCount = 0;
	int32 ComponentCount = 0;
	for (TActorIterator<ALandscapeProxy> It(World); It; ++It)
	{
		++LandscapeCount;
		It->Modify();
		It->LandscapeMaterial = LandscapeMaterial;
		for (ULandscapeComponent* LandscapeComponent : It->LandscapeComponents)
		{
			if (!LandscapeComponent)
			{
				continue;
			}
			++ComponentCount;
			LandscapeComponent->Modify();
			LandscapeComponent->OverrideMaterial = LandscapeMaterial;
			LandscapeComponent->MaterialInstances.Empty();
			LandscapeComponent->UpdateMaterialInstances();
			LandscapeComponent->MarkPackageDirty();
		}
		It->PostEditChange();
		It->MarkPackageDirty();
	}

	World->MarkPackageDirty();
	const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
	const bool bSavedAssets = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
	UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandGroundStyle: landscapeMaterial=%s softGrassMaterial=%s landscapes=%d components=%d removedInactivePipelineProofActors=%d v82BiomeFoundationOnly=%s savedMap=%s savedAssets=%s"),
		*LandscapeMaterial->GetPathName(),
		SoftGrassMaterial ? *SoftGrassMaterial->GetPathName() : TEXT("not touched"),
		LandscapeCount,
		ComponentCount,
		RemovedInactivePipelineProofActors,
		bV82BiomeFoundationOnly ? TEXT("true") : TEXT("false"),
		bSavedMap ? TEXT("true") : TEXT("false"),
		bSavedAssets ? TEXT("true") : TEXT("false"));
	return (bSavedMap && bSavedAssets && LandscapeCount > 0) ? 0 : 1;
#else
	UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandGroundStyle can only run in editor builds."));
	return 1;
#endif
}
