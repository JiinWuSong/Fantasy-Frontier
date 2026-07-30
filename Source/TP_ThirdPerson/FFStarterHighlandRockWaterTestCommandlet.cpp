#include "FFStarterHighlandRockWaterTestCommandlet.h"

#if WITH_EDITOR
#include "Components/PrimitiveComponent.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "FileHelpers.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionCustom.h"
#include "Materials/MaterialExpressionLandscapeLayerWeight.h"
#include "Materials/MaterialExpressionLinearInterpolate.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionVertexNormalWS.h"
#include "Materials/MaterialExpressionWorldPosition.h"
#include "Materials/MaterialInstance.h"
#include "MaterialEditingLibrary.h"
#endif

UFFStarterHighlandRockWaterTestCommandlet::UFFStarterHighlandRockWaterTestCommandlet()
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
	const TCHAR* LandscapeMaterialPath = TEXT("/Game/FantasyFrontier/Blockout/Materials/M_FF_Blockout_Grass_Green.M_FF_Blockout_Grass_Green");
	const TCHAR* TitanCliffMaterialPath = TEXT("/Game/Environment/Grassland/Materials/MI_Cliffside_Dirt_B.MI_Cliffside_Dirt_B");
	const TCHAR* TitanRockTexturePath = TEXT("/Game/Environment/Grassland/Materials/Rock/RockCliff/T_Grassland_Rock_Cliff_D.T_Grassland_Rock_Cliff_D");
	const TCHAR* TitanWaterRiverPath = TEXT("/Game/Environment/_Global/Core/Materials/Water/Water_Material_River.Water_Material_River");
	const TCHAR* TitanWaterCustomMeshPath = TEXT("/Game/Environment/_Global/Core/Materials/Water/Water_Material_CustomMesh.Water_Material_CustomMesh");
	const TCHAR* TitanRockCandidateMaterialPaths[] = {
		TEXT("/Game/Environment/Grassland/Materials/MI_Cliffside_Dirt_B.MI_Cliffside_Dirt_B"),
		TEXT("/Game/Environment/Grassland/Materials/MI_Cliffside_Dirt.MI_Cliffside_Dirt"),
		TEXT("/Game/Environment/Grassland/Materials/MI_Cliffside_Blended1.MI_Cliffside_Blended1"),
		TEXT("/Game/Environment/Grassland/Materials/MI_Cliffside_Blended_02.MI_Cliffside_Blended_02"),
		TEXT("/Game/Environment/Grassland/Materials/MI_Cliffside_Blended_LessHarsh.MI_Cliffside_Blended_LessHarsh"),
		TEXT("/Game/Environment/Grassland/Materials/Rock/AltarRockKit/MI_Grasslands_AltarRocks.MI_Grasslands_AltarRocks")
	};

	struct FHighlandLayerSpec
	{
		FName LayerName;
		FLinearColor Color;
	};

	const FHighlandLayerSpec LayerSpecs[] = {
		{ TEXT("Grass_Light"), FLinearColor(0.31f, 0.52f, 0.12f) },
		{ TEXT("Grass_Dark"), FLinearColor(0.048f, 0.19f, 0.035f) },
		{ TEXT("Rock_Cliff"), FLinearColor(0.34f, 0.32f, 0.27f) },
		{ TEXT("Path_Dirt"), FLinearColor(0.58f, 0.48f, 0.31f) },
		{ TEXT("Shore_WetDirt"), FLinearColor(0.12f, 0.25f, 0.07f) },
		{ TEXT("Waterbed_MudStone"), FLinearColor(0.13f, 0.13f, 0.11f) },
	};

	bool TextContainsAny(const FString& Text, std::initializer_list<const TCHAR*> Needles)
	{
		for (const TCHAR* Needle : Needles)
		{
			if (Text.Contains(Needle, ESearchCase::IgnoreCase))
			{
				return true;
			}
		}
		return false;
	}

	UMaterialExpressionWorldPosition* AddWorldPositionNode(UMaterial* Material, const int32 X, const int32 Y)
	{
		UMaterialExpressionWorldPosition* Node = Cast<UMaterialExpressionWorldPosition>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionWorldPosition::StaticClass(), X, Y));
		if (Node)
		{
			Node->WorldPositionShaderOffset = WPT_ExcludeAllShaderOffsets;
		}
		return Node;
	}

	void ConnectCustomInput(UMaterialExpressionCustom* CustomNode, UMaterialExpression* Expression, const FString& InputName)
	{
		if (!CustomNode || !Expression)
		{
			return;
		}

		FCustomInput Input;
		Input.InputName = FName(*InputName);
		Input.Input.Expression = Expression;
		CustomNode->Inputs.Add(Input);
	}

	UMaterialExpressionConstant3Vector* AddLayerColorNode(UMaterial* Material, const FHighlandLayerSpec& Spec, const int32 X, const int32 Y)
	{
		UMaterialExpressionConstant3Vector* ColorNode = Cast<UMaterialExpressionConstant3Vector>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant3Vector::StaticClass(), X, Y));
		if (ColorNode)
		{
			ColorNode->Constant = Spec.Color;
			ColorNode->Desc = Spec.LayerName.ToString();
		}
		return ColorNode;
	}

	UMaterialExpression* AddLandscapeLayerWeight(UMaterial* Material, UMaterialExpression* BaseExpression, UMaterialExpression* LayerExpression, const FName LayerName, const int32 X, const int32 Y)
	{
		UMaterialExpressionLandscapeLayerWeight* LayerWeight = Cast<UMaterialExpressionLandscapeLayerWeight>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionLandscapeLayerWeight::StaticClass(), X, Y));
		if (!LayerWeight || !BaseExpression || !LayerExpression)
		{
			return BaseExpression;
		}

		LayerWeight->ParameterName = LayerName;
		LayerWeight->PreviewWeight = 0.0f;
		LayerWeight->Base.Expression = BaseExpression;
		LayerWeight->Layer.Expression = LayerExpression;
		return LayerWeight;
	}

	UMaterialExpressionCustom* AddProceduralFallbackColor(UMaterial* Material, UMaterialExpressionWorldPosition* WorldPosition, UMaterialExpressionVertexNormalWS* VertexNormal)
	{
		UMaterialExpressionCustom* ColorNode = Cast<UMaterialExpressionCustom>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionCustom::StaticClass(), -1020, -180));
		if (!ColorNode || !WorldPosition || !VertexNormal)
		{
			return nullptr;
		}

		ColorNode->Description = TEXT("Procedural Highland fallback masks; rock tint is intentionally muted before Titan cliff texture blend");
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
float slopeRock = smoothstep(0.15, 0.33, slopeAmount);
float edgeRock = smoothstep(0.86, 1.05, organic) * islandMask * smoothstep(0.06, 0.16, slopeAmount);
float rimShadow = smoothstep(0.78, 0.98, organic);
float rock = saturate(max(slopeRock, edgeRock));

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

float shore = saturate((1.0 - smoothstep(3100.0, 5400.0, DISTSEG(p, float2(-30000.0, 64000.0), float2(-42000.0, 50000.0)))) * (1.0 - water));
shore = max(shore, saturate((1.0 - smoothstep(3100.0, 5400.0, DISTSEG(p, float2(-42000.0, 50000.0), float2(-52000.0, 28000.0)))) * (1.0 - water)));
shore = max(shore, saturate((1.0 - smoothstep(3100.0, 5400.0, DISTSEG(p, float2(-52000.0, 28000.0), float2(-42000.0, 8000.0)))) * (1.0 - water)));
shore = max(shore, saturate((1.0 - smoothstep(2900.0, 5200.0, DISTSEG(p, float2(-42000.0, 8000.0), float2(-15000.0, 2000.0)))) * (1.0 - water)));
shore = max(shore, saturate((1.0 - smoothstep(2900.0, 5200.0, DISTSEG(p, float2(-15000.0, 2000.0), float2(13000.0, 8000.0)))) * (1.0 - water)));
shore = max(shore, saturate((1.0 - smoothstep(1.10, 1.42, length(lake0))) * (1.0 - water)));

float northDark = smoothstep(30000.0, 76000.0, p.y);
float darkPatch = northDark;
darkPatch = max(darkPatch, 1.0 - smoothstep(9000.0, 35000.0, length(p - float2(46000.0, 24000.0))));
darkPatch = max(darkPatch, 1.0 - smoothstep(9000.0, 31000.0, length(p - float2(-52000.0, -26000.0))));
float lightPatch = max(1.0 - smoothstep(6000.0, 26000.0, length(p - float2(-12000.0, -22000.0))),
                       1.0 - smoothstep(6000.0, 30000.0, length(p - float2(36000.0, -26000.0))));
float cellNoise = frac(sin(dot(floor(p / 3600.0), float2(12.9898, 78.233))) * 43758.5453);

float3 lightGrass = float3(0.31, 0.52, 0.12);
float3 openGrass = float3(0.17, 0.38, 0.075);
float3 darkGrass = float3(0.048, 0.19, 0.035);
float3 pathColor = float3(0.58, 0.48, 0.31);
float3 shoreColor = float3(0.12, 0.25, 0.07);
float3 waterbedColor = float3(0.13, 0.13, 0.11);
float3 rockLow = float3(0.18, 0.16, 0.12);
float3 rockMid = float3(0.34, 0.29, 0.21);
float3 rockHigh = float3(0.49, 0.42, 0.30);
float3 waterColor = float3(0.02, 0.31, 0.50);

float3 grass = lerp(openGrass, darkGrass, saturate(darkPatch));
grass = lerp(grass, lightGrass, saturate(lightPatch * 0.62));
grass *= lerp(0.93, 1.08, cellNoise);
float rockStrata = frac(sin(dot(floor((p + float2(1700.0, -900.0)) / 5200.0), float2(39.346, 11.135))) * 24634.6345);
float3 rockColor = lerp(rockLow, rockMid, saturate(0.48 + rockStrata * 0.52));
rockColor = lerp(rockColor, rockHigh, saturate((1.0 - NormalWS.z) * 0.65 + rimShadow * 0.20));
float3 color = grass;
color = lerp(color, rockColor, saturate(rock * (1.0 - water * 0.80) * (1.0 - path * 0.65)));
color = lerp(color, shoreColor, saturate(shore * 0.62));
color = lerp(color, waterbedColor, saturate(water * 0.35));
color = lerp(color, waterColor, saturate(water * 0.88));
color = lerp(color, pathColor, saturate(path * (1.0 - water * 0.55)));
color = lerp(color, lightGrass, saturate(spawnGrassMask * (1.0 - water)));
return color;
#undef DISTSEG
)");
		return ColorNode;
	}

	UMaterialExpressionCustom* AddRockMaskNode(UMaterial* Material, UMaterialExpressionWorldPosition* WorldPosition, UMaterialExpressionVertexNormalWS* VertexNormal)
	{
		UMaterialExpressionCustom* MaskNode = Cast<UMaterialExpressionCustom>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionCustom::StaticClass(), -1020, 180));
		if (!MaskNode || !WorldPosition || !VertexNormal)
		{
			return nullptr;
		}

		MaskNode->Description = TEXT("Rock_Cliff procedural visibility mask for unpainted fallback");
		MaskNode->OutputType = CMOT_Float1;
		ConnectCustomInput(MaskNode, WorldPosition, TEXT("WorldPos"));
		ConnectCustomInput(MaskNode, VertexNormal, TEXT("NormalWS"));
		MaskNode->Code = TEXT(R"(
#define DISTSEG(P,A,B) length((P) - ((A) + saturate(dot((P) - (A), (B) - (A)) / max(dot((B) - (A), (B) - (A)), 1.0)) * ((B) - (A))))
float2 p = WorldPos.xy;
float2 centered = (p - float2(0.0, 15000.0)) / float2(100000.0, 72000.0);
float organic = length(centered) + 0.055 * sin(p.x / 11500.0) - 0.045 * cos(p.y / 9000.0) + 0.035 * sin((p.x + p.y) / 17000.0);
float islandMask = 1.0 - smoothstep(0.92, 1.16, organic);
float slopeAmount = 1.0 - saturate(NormalWS.z);
float slopeRock = smoothstep(0.15, 0.33, slopeAmount);
float edgeRock = smoothstep(0.86, 1.05, organic) * islandMask * smoothstep(0.06, 0.16, slopeAmount);
float rock = saturate(max(slopeRock, edgeRock));
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
float spawnDryMask = smoothstep(6500.0, 10500.0, length(p - float2(-18120.0, 4000.0)));
water *= spawnDryMask;
return saturate(rock * (1.0 - path * 0.70) * (1.0 - water * 0.90));
#undef DISTSEG
)");
		return MaskNode;
	}

	UMaterialExpressionCustom* AddRockUvNode(UMaterial* Material, UMaterialExpressionWorldPosition* WorldPosition)
	{
		UMaterialExpressionCustom* UvNode = Cast<UMaterialExpressionCustom>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionCustom::StaticClass(), -1020, 360));
		if (!UvNode || !WorldPosition)
		{
			return nullptr;
		}

		UvNode->Description = TEXT("World-projected Titan cliff texture UVs");
		UvNode->OutputType = CMOT_Float2;
		ConnectCustomInput(UvNode, WorldPosition, TEXT("WorldPos"));
		UvNode->Code = TEXT("return (WorldPos.xy + float2(WorldPos.z * 0.35, -WorldPos.z * 0.20)) / 5200.0;");
		return UvNode;
	}

	UMaterialExpressionCustom* AddSpawnGrassSafetyMaskNode(UMaterial* Material, UMaterialExpressionWorldPosition* WorldPosition)
	{
		UMaterialExpressionCustom* MaskNode = Cast<UMaterialExpressionCustom>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionCustom::StaticClass(), 760, -260));
		if (!MaskNode || !WorldPosition)
		{
			return nullptr;
		}

		MaskNode->Description = TEXT("Preserve accepted spawn-area light grass after editable layer stack");
		MaskNode->OutputType = CMOT_Float1;
		ConnectCustomInput(MaskNode, WorldPosition, TEXT("WorldPos"));
		MaskNode->Code = TEXT("float2 p = WorldPos.xy; return 1.0 - smoothstep(1200.0, 2000.0, length(p - float2(-18120.0, 4000.0)));");
		return MaskNode;
	}

	UMaterialExpression* AddTitanRockTextureNode(UMaterial* Material, UMaterialExpressionCustom* RockUvNode, UTexture2D* RockTexture)
	{
		if (!RockTexture || RockTexture->VirtualTextureStreaming)
		{
			UE_LOG(LogTemp, Warning, TEXT("FFStarterHighlandRockWaterTest: Titan rock texture unavailable or VT-backed; falling back to constant Rock_Cliff color."));
			for (const FHighlandLayerSpec& Spec : LayerSpecs)
			{
				if (Spec.LayerName == TEXT("Rock_Cliff"))
				{
					return AddLayerColorNode(Material, Spec, -460, 20);
				}
			}
			return nullptr;
		}

		UMaterialExpressionTextureSample* TextureSample = Cast<UMaterialExpressionTextureSample>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionTextureSample::StaticClass(), -620, 220));
		if (!TextureSample)
		{
			return nullptr;
		}

		TextureSample->Texture = RockTexture;
		TextureSample->SamplerType = SAMPLERTYPE_Color;
		TextureSample->Desc = TEXT("Titan Rock_Cliff: T_Grassland_Rock_Cliff_D from MI_Cliffside_Dirt_B family");
		if (RockUvNode)
		{
			TextureSample->Coordinates.Expression = RockUvNode;
		}
		return TextureSample;
	}

	UTexture2D* SelectCookSafeTitanRockTexture()
	{
		UTexture2D* ExplicitTexture = LoadObject<UTexture2D>(nullptr, TitanRockTexturePath);
		if (ExplicitTexture)
		{
			UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandRockWaterTest: explicitRockTexture=%s virtualTexture=%s"),
				*ExplicitTexture->GetPathName(),
				ExplicitTexture->VirtualTextureStreaming ? TEXT("true") : TEXT("false"));
			if (!ExplicitTexture->VirtualTextureStreaming)
			{
				return ExplicitTexture;
			}
		}

		for (const TCHAR* CandidatePath : TitanRockCandidateMaterialPaths)
		{
			UMaterialInterface* Candidate = LoadObject<UMaterialInterface>(nullptr, CandidatePath);
			if (!Candidate)
			{
				UE_LOG(LogTemp, Warning, TEXT("FFStarterHighlandRockWaterTest: missing rock candidate %s"), CandidatePath);
				continue;
			}

			TArray<UTexture*> UsedTextures;
			Candidate->GetUsedTextures(UsedTextures);
			UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandRockWaterTest: rockCandidate=%s parent=%s usedTextures=%d"),
				*Candidate->GetPathName(),
				Candidate->GetMaterial() ? *Candidate->GetMaterial()->GetPathName() : TEXT("None"),
				UsedTextures.Num());

			for (UTexture* Texture : UsedTextures)
			{
				UTexture2D* Texture2D = Cast<UTexture2D>(Texture);
				if (!Texture2D)
				{
					continue;
				}

				const bool bLikelyCliffTexture = TextContainsAny(Texture2D->GetPathName(), { TEXT("Cliff"), TEXT("Rock"), TEXT("Stone"), TEXT("Dirt") });
				UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandRockWaterTest:   texture=%s virtualTexture=%s likelyCliff=%s"),
					*Texture2D->GetPathName(),
					Texture2D->VirtualTextureStreaming ? TEXT("true") : TEXT("false"),
					bLikelyCliffTexture ? TEXT("true") : TEXT("false"));

				if (bLikelyCliffTexture && !Texture2D->VirtualTextureStreaming)
				{
					UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandRockWaterTest: selected safe Titan rock texture %s from %s"),
						*Texture2D->GetPathName(),
						*Candidate->GetPathName());
					return Texture2D;
				}
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("FFStarterHighlandRockWaterTest: no non-VT Titan Grassland rock texture found among strict candidates."));
		return nullptr;
	}

	bool RebuildLandscapeRockLayerMaterial()
	{
		UMaterial* Material = LoadObject<UMaterial>(nullptr, LandscapeMaterialPath);
		UMaterialInterface* CliffCandidate = LoadObject<UMaterialInterface>(nullptr, TitanCliffMaterialPath);
		UTexture2D* RockTexture = SelectCookSafeTitanRockTexture();
		if (!Material || !CliffCandidate)
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandRockWaterTest: missing material=%s or cliff candidate=%s"), LandscapeMaterialPath, TitanCliffMaterialPath);
			return false;
		}

		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandRockWaterTest: cliffCandidate=%s parent=%s"),
			*CliffCandidate->GetPathName(),
			CliffCandidate->GetMaterial() ? *CliffCandidate->GetMaterial()->GetPathName() : TEXT("None"));
		Material->Modify();
		UMaterialEditingLibrary::DeleteAllMaterialExpressions(Material);

		UMaterialExpressionWorldPosition* WorldPosition = AddWorldPositionNode(Material, -1280, -120);
		UMaterialExpressionVertexNormalWS* VertexNormal = Cast<UMaterialExpressionVertexNormalWS>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionVertexNormalWS::StaticClass(), -1280, 40));
		UMaterialExpressionCustom* FallbackColor = AddProceduralFallbackColor(Material, WorldPosition, VertexNormal);
		UMaterialExpressionCustom* RockMask = AddRockMaskNode(Material, WorldPosition, VertexNormal);
		UMaterialExpressionCustom* RockUv = AddRockUvNode(Material, WorldPosition);
		UMaterialExpression* RockTextureExpression = AddTitanRockTextureNode(Material, RockUv, RockTexture);
		UMaterialExpressionLinearInterpolate* RockBlend = Cast<UMaterialExpressionLinearInterpolate>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionLinearInterpolate::StaticClass(), -160, 120));
		UMaterialExpressionCustom* SpawnSafetyMask = AddSpawnGrassSafetyMaskNode(Material, WorldPosition);
		UMaterialExpressionConstant3Vector* SpawnSafetyColor = Cast<UMaterialExpressionConstant3Vector>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant3Vector::StaticClass(), 760, -120));
		UMaterialExpressionLinearInterpolate* SpawnSafetyBlend = Cast<UMaterialExpressionLinearInterpolate>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionLinearInterpolate::StaticClass(), 1020, -120));
		UMaterialExpressionConstant* Roughness = Cast<UMaterialExpressionConstant>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant::StaticClass(), 780, 260));

		if (!WorldPosition || !VertexNormal || !FallbackColor || !RockMask || !RockTextureExpression || !RockBlend || !SpawnSafetyMask || !SpawnSafetyColor || !SpawnSafetyBlend || !Roughness)
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandRockWaterTest: failed to build landscape material expressions."));
			return false;
		}

		RockBlend->A.Expression = FallbackColor;
		RockBlend->B.Expression = RockTextureExpression;
		RockBlend->Alpha.Expression = RockMask;
		Roughness->R = 0.96f;

		UMaterialExpression* CurrentColor = RockBlend;
		int32 LayerIndex = 0;
		for (const FHighlandLayerSpec& Spec : LayerSpecs)
		{
			UMaterialExpression* LayerExpression = nullptr;
			if (Spec.LayerName == TEXT("Rock_Cliff"))
			{
				LayerExpression = RockTextureExpression;
			}
			else
			{
				LayerExpression = AddLayerColorNode(Material, Spec, -420, -520 + LayerIndex * 150);
			}
			CurrentColor = AddLandscapeLayerWeight(Material, CurrentColor, LayerExpression, Spec.LayerName, 120 + LayerIndex * 150, -520 + LayerIndex * 150);
			++LayerIndex;
		}

		SpawnSafetyColor->Constant = FLinearColor(0.31f, 0.52f, 0.12f);
		SpawnSafetyColor->Desc = TEXT("Spawn light grass preservation");
		SpawnSafetyBlend->A.Expression = CurrentColor;
		SpawnSafetyBlend->B.Expression = SpawnSafetyColor;
		SpawnSafetyBlend->Alpha.Expression = SpawnSafetyMask;
		CurrentColor = SpawnSafetyBlend;

		Material->BlendMode = BLEND_Opaque;
		Material->TwoSided = false;
		Material->bUsedWithNanite = true;
		UMaterialEditingLibrary::ConnectMaterialProperty(CurrentColor, TEXT(""), MP_BaseColor);
		UMaterialEditingLibrary::ConnectMaterialProperty(Roughness, TEXT(""), MP_Roughness);
		UMaterialEditingLibrary::LayoutMaterialExpressions(Material);
		UMaterialEditingLibrary::RecompileMaterial(Material);
		Material->PostEditChange();
		Material->MarkPackageDirty();
		return true;
	}

	bool IsNativeWaterBodyActor(const AActor* Actor)
	{
		if (!Actor)
		{
			return false;
		}

		const FString ActorName = Actor->GetName();
		const FString ActorLabel = Actor->GetActorLabel();
		const FString ClassName = Actor->GetClass() ? Actor->GetClass()->GetName() : FString();
		return TextContainsAny(ActorName, { TEXT("WaterBodyCustom"), TEXT("WaterBodyRiver"), TEXT("WaterBodyLake") })
			|| TextContainsAny(ActorLabel, { TEXT("WaterBodyCustom"), TEXT("WaterBodyRiver"), TEXT("WaterBodyLake") })
			|| TextContainsAny(ClassName, { TEXT("WaterBodyCustom"), TEXT("WaterBodyRiver"), TEXT("WaterBodyLake") });
	}

	bool IsWaterBodyCustom2(const AActor* Actor)
	{
		if (!Actor)
		{
			return false;
		}

		const FString ActorName = Actor->GetName();
		const FString ActorLabel = Actor->GetActorLabel();
		return TextContainsAny(ActorName, { TEXT("WaterBodyCustom2"), TEXT("WaterBodyCustom_2") })
			|| TextContainsAny(ActorLabel, { TEXT("WaterBodyCustom2"), TEXT("WaterBodyCustom_2") });
	}

	FBox GetPrimitiveBounds(const TArray<UPrimitiveComponent*>& PrimitiveComponents)
	{
		FBox Bounds(ForceInit);
		for (const UPrimitiveComponent* Component : PrimitiveComponents)
		{
			if (Component)
			{
				Bounds += Component->Bounds.GetBox();
			}
		}
		return Bounds;
	}

	int32 SetComponentMaterials(UPrimitiveComponent* Component, UMaterialInterface* Material)
	{
		if (!Component || !Material)
		{
			return 0;
		}

		Component->Modify();
		const int32 SlotCount = FMath::Max(1, Component->GetNumMaterials());
		for (int32 SlotIndex = 0; SlotIndex < SlotCount; ++SlotIndex)
		{
			Component->SetMaterial(SlotIndex, Material);
		}
		Component->MarkPackageDirty();
		return SlotCount;
	}

	void DiagnoseAndApplyWater(UWorld* World)
	{
		UMaterialInterface* WaterRiverMaterial = LoadObject<UMaterialInterface>(nullptr, TitanWaterRiverPath);
		UMaterialInterface* WaterCustomMeshMaterial = LoadObject<UMaterialInterface>(nullptr, TitanWaterCustomMeshPath);
		UMaterialInterface* ChosenWaterMaterial = WaterRiverMaterial ? WaterRiverMaterial : WaterCustomMeshMaterial;
		if (ChosenWaterMaterial)
		{
			UMaterial* BaseMaterial = ChosenWaterMaterial->GetMaterial();
			UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandRockWaterTest: waterCandidate=%s base=%s domain=%d blend=%d"),
				*ChosenWaterMaterial->GetPathName(),
				BaseMaterial ? *BaseMaterial->GetPathName() : TEXT("None"),
				BaseMaterial ? static_cast<int32>(BaseMaterial->MaterialDomain) : -1,
				BaseMaterial ? static_cast<int32>(BaseMaterial->BlendMode) : -1);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("FFStarterHighlandRockWaterTest: no Titan water material candidate loaded."));
		}

		int32 WaterActors = 0;
		int32 TargetActors = 0;
		int32 MaterialSlots = 0;
		int32 KeptHiddenFloodRisk = 0;

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (!IsNativeWaterBodyActor(Actor))
			{
				continue;
			}

			++WaterActors;
			TArray<UPrimitiveComponent*> PrimitiveComponents;
			Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
			const FBox Bounds = GetPrimitiveBounds(PrimitiveComponents);
			const FVector Extent = Bounds.IsValid ? Bounds.GetExtent() : FVector::ZeroVector;
			const bool bFloodRisk = Extent.X > 90000.0f && Extent.Y > 65000.0f;
			const bool bTarget = IsWaterBodyCustom2(Actor);
			const FString ClassName = Actor->GetClass() ? Actor->GetClass()->GetName() : FString();
			UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandRockWaterTest: waterActor name=%s label=%s class=%s targetCustom2=%s hidden=%s primitiveCount=%d extent=(%.1f, %.1f, %.1f) floodRisk=%s"),
				*Actor->GetName(),
				*Actor->GetActorLabel(),
				*ClassName,
				bTarget ? TEXT("true") : TEXT("false"),
				Actor->IsHidden() ? TEXT("true") : TEXT("false"),
				PrimitiveComponents.Num(),
				Extent.X,
				Extent.Y,
				Extent.Z,
				bFloodRisk ? TEXT("true") : TEXT("false"));

			if (!bTarget)
			{
				continue;
			}

			++TargetActors;
			if (bFloodRisk)
			{
				UE_LOG(LogTemp, Warning, TEXT("FFStarterHighlandRockWaterTest: WaterBodyCustom2 left hidden because its bounds are flood-risk scale; no fake mesh fallback created."));
				++KeptHiddenFloodRisk;
				Actor->Modify();
				Actor->SetActorHiddenInGame(true);
				Actor->SetIsTemporarilyHiddenInEditor(true);
				for (UPrimitiveComponent* Component : PrimitiveComponents)
				{
					if (!Component)
					{
						continue;
					}
					Component->Modify();
					Component->SetVisibility(false, true);
					Component->SetHiddenInGame(true);
					if (ChosenWaterMaterial)
					{
						MaterialSlots += SetComponentMaterials(Component, ChosenWaterMaterial);
					}
				}
				Actor->MarkPackageDirty();
				continue;
			}

			Actor->Modify();
			Actor->SetActorHiddenInGame(false);
			Actor->SetIsTemporarilyHiddenInEditor(false);
			for (UPrimitiveComponent* Component : PrimitiveComponents)
			{
				if (!Component)
				{
					continue;
				}
				Component->Modify();
				Component->SetVisibility(true, true);
				Component->SetHiddenInGame(false);
				if (ChosenWaterMaterial)
				{
					MaterialSlots += SetComponentMaterials(Component, ChosenWaterMaterial);
				}
			}
			Actor->MarkPackageDirty();
		}

		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandRockWaterTest: waterSummary waterActors=%d targetWaterBodyCustom2=%d materialSlots=%d keptHiddenFloodRisk=%d"),
			WaterActors,
			TargetActors,
			MaterialSlots,
			KeptHiddenFloodRisk);
	}
}
#endif

int32 UFFStarterHighlandRockWaterTestCommandlet::Main(const FString& Params)
{
#if WITH_EDITOR
	UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandRockWaterTest: loading %s without terrain/layout/grass edits."), HighlandMapPath);
	UWorld* World = UEditorLoadingAndSavingUtils::LoadMap(HighlandMapPath);
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandRockWaterTest: failed to load %s"), HighlandMapPath);
		return 1;
	}

	if (!RebuildLandscapeRockLayerMaterial())
	{
		return 1;
	}

	DiagnoseAndApplyWater(World);

	World->MarkPackageDirty();
	const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
	const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
	UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandRockWaterTest: savedMap=%s savedPackages=%s"),
		bSavedMap ? TEXT("true") : TEXT("false"),
		bSavedPackages ? TEXT("true") : TEXT("false"));

	return bSavedMap && bSavedPackages ? 0 : 1;
#else
	return 0;
#endif
}
