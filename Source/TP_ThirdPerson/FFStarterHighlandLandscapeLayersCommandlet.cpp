#include "FFStarterHighlandLandscapeLayersCommandlet.h"

#if WITH_EDITOR
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/World.h"
#include "Engine/Texture2D.h"
#include "EngineUtils.h"
#include "FileHelpers.h"
#include "LandscapeComponent.h"
#include "LandscapeLayerInfoObject.h"
#include "LandscapeProxy.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionCustom.h"
#include "Materials/MaterialExpressionLandscapeLayerWeight.h"
#include "Materials/MaterialExpressionLinearInterpolate.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionVertexNormalWS.h"
#include "Materials/MaterialExpressionWorldPosition.h"
#include "MaterialEditingLibrary.h"
#include "UObject/Package.h"
#endif

UFFStarterHighlandLandscapeLayersCommandlet::UFFStarterHighlandLandscapeLayersCommandlet()
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
	const TCHAR* LayerInfoFolderPath = TEXT("/Game/FantasyFrontier/Blockout/LandscapeLayers");
	const TCHAR* TitanRockTexturePath = TEXT("/Game/Environment/Grassland/Materials/Rock/RockCliff/T_Grassland_Rock_Cliff_D.T_Grassland_Rock_Cliff_D");

	struct FHighlandLayerSpec
	{
		FName LayerName;
		FLinearColor Color;
	};

	const FHighlandLayerSpec LayerSpecs[] = {
		{ TEXT("Grass_Light"), FLinearColor(0.43f, 0.50f, 0.20f) },
		{ TEXT("Grass_Dark"), FLinearColor(0.085f, 0.195f, 0.055f) },
		{ TEXT("Rock_Cliff"), FLinearColor(0.26f, 0.20f, 0.15f) },
		{ TEXT("Path_Dirt"), FLinearColor(0.42f, 0.34f, 0.22f) },
		{ TEXT("Shore_WetDirt"), FLinearColor(0.15f, 0.18f, 0.095f) },
		{ TEXT("Waterbed_MudStone"), FLinearColor(0.10f, 0.09f, 0.075f) },
	};

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

	UMaterialExpressionCustom* AddGrassLayerBlendColorNode(UMaterial* Material, UMaterialExpressionWorldPosition* WorldPosition, const FName LayerName, const int32 X, const int32 Y)
	{
		UMaterialExpressionCustom* ColorNode = Cast<UMaterialExpressionCustom>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionCustom::StaticClass(), X, Y));
		if (!ColorNode || !WorldPosition)
		{
			return nullptr;
		}

		ColorNode->Description = FString::Printf(TEXT("%s Titan-style mottled grass color"), *LayerName.ToString());
		ColorNode->OutputType = CMOT_Float3;
		ConnectCustomInput(ColorNode, WorldPosition, TEXT("WorldPos"));

		if (LayerName == FName(TEXT("Grass_Dark")))
		{
			ColorNode->Code = TEXT(R"(
float2 p = WorldPos.xy;
float macroWarm = 0.5 + 0.5 * sin(p.x * 0.000055 + p.y * 0.000034 + sin(p.y * 0.000021) * 1.7);
float macroCool = 0.5 + 0.5 * sin(p.x * -0.000041 + p.y * 0.000063 + 2.1);
float moisture = 0.5 + 0.5 * sin((p.x + p.y * 0.73) * 0.000085 + sin(p.x * 0.000019) * 2.2);
float blade = 0.5 + 0.5 * sin(p.x * 0.0022 + p.y * 0.0016 + macroWarm * 2.4);
float weave = 0.5 + 0.5 * sin(p.x * 0.00054 - p.y * 0.00039 + moisture * 3.0);
float3 deep = float3(0.105, 0.195, 0.060);
float3 base = float3(0.165, 0.285, 0.088);
float3 warmTip = float3(0.360, 0.420, 0.155);
float3 thatch = float3(0.335, 0.325, 0.120);
float3 c = lerp(deep, base, saturate(0.32 + macroWarm * 0.30 + moisture * 0.18));
c = lerp(c, warmTip, saturate(macroWarm * 0.16 + weave * 0.08));
c = lerp(c, deep, saturate((1.0 - blade) * 0.18 + macroCool * 0.12));
c = lerp(c, thatch, saturate((1.0 - moisture) * 0.10 + weave * 0.08));
c *= lerp(0.94, 1.04, saturate(blade * 0.38 + weave * 0.34 + macroWarm * 0.28));
return c;
)");
		}
		else
		{
			ColorNode->Code = TEXT(R"(
float2 p = WorldPos.xy;
float macroWarm = 0.5 + 0.5 * sin(p.x * 0.000049 + p.y * 0.000038 + 0.7);
float macroCool = 0.5 + 0.5 * sin(p.x * -0.000037 + p.y * 0.000071 + sin(p.y * 0.000018) * 1.9);
float meadowFlow = 0.5 + 0.5 * sin((p.x * 0.83 + p.y) * 0.000092 + sin(p.x * 0.000026) * 2.5);
float blade = 0.5 + 0.5 * sin(p.x * 0.0020 + p.y * 0.0014 + meadowFlow * 2.2);
float weave = 0.5 + 0.5 * sin(p.x * 0.00048 - p.y * 0.00036 + macroCool * 2.6);
float3 field = float3(0.320, 0.420, 0.150);
float3 sun = float3(0.468, 0.522, 0.205);
float3 under = float3(0.210, 0.325, 0.105);
float3 thatch = float3(0.395, 0.365, 0.135);
float3 c = lerp(field, sun, saturate(0.18 + macroWarm * 0.24 + meadowFlow * 0.16));
c = lerp(c, under, saturate((1.0 - blade) * 0.22 + macroCool * 0.12));
c = lerp(c, thatch, saturate((1.0 - meadowFlow) * 0.08 + weave * 0.07));
c *= lerp(0.94, 1.045, saturate(blade * 0.38 + meadowFlow * 0.34 + macroWarm * 0.28));
return c;
)");
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

	UMaterialExpressionCustom* AddRockUvNode(UMaterial* Material, UMaterialExpressionWorldPosition* WorldPosition)
	{
		UMaterialExpressionCustom* UvNode = Cast<UMaterialExpressionCustom>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionCustom::StaticClass(), -1220, 520));
		if (!UvNode || !WorldPosition)
		{
			return nullptr;
		}

		UvNode->Description = TEXT("V58 world-projected Titan cliff texture UVs");
		UvNode->OutputType = CMOT_Float2;
		ConnectCustomInput(UvNode, WorldPosition, TEXT("WorldPos"));
		UvNode->Code = TEXT("return (WorldPos.xy + float2(WorldPos.z * 0.48, -WorldPos.z * 0.31)) / 3900.0;");
		return UvNode;
	}

	UMaterialExpression* AddV58TitanRockTextureColor(UMaterial* Material, UMaterialExpressionCustom* RockUvNode)
	{
		UTexture2D* RockTexture = LoadObject<UTexture2D>(nullptr, TitanRockTexturePath);
		if (RockTexture)
		{
			UMaterialExpressionTextureSample* TextureSample = Cast<UMaterialExpressionTextureSample>(
				UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionTextureSample::StaticClass(), -900, 560));
			if (TextureSample)
			{
				TextureSample->Texture = RockTexture;
				TextureSample->SamplerType = RockTexture->VirtualTextureStreaming ? SAMPLERTYPE_VirtualColor : SAMPLERTYPE_Color;
				TextureSample->Desc = TEXT("V58 Titan RockCliff diffuse texture");
				if (RockUvNode)
				{
					TextureSample->Coordinates.Expression = RockUvNode;
				}
				UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandLandscapeLayers: V58 using Titan rock texture %s sampler=%s"),
					*RockTexture->GetPathName(),
					RockTexture->VirtualTextureStreaming ? TEXT("VirtualColor") : TEXT("Color"));
				return TextureSample;
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("FFStarterHighlandLandscapeLayers: V58 missing Titan rock texture %s, using procedural fallback."), TitanRockTexturePath);
		}

		UMaterialExpressionConstant3Vector* FallbackRockColor = Cast<UMaterialExpressionConstant3Vector>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant3Vector::StaticClass(), -900, 560));
		if (FallbackRockColor)
		{
			FallbackRockColor->Constant = FLinearColor(0.18f, 0.16f, 0.11f);
			FallbackRockColor->Desc = TEXT("V58 fallback Titan rock texture color");
		}
		return FallbackRockColor;
	}

	UMaterialExpressionCustom* AddRightSideArtifactMask(UMaterial* Material, UMaterialExpressionWorldPosition* WorldPosition)
	{
		UMaterialExpressionCustom* MaskNode = Cast<UMaterialExpressionCustom>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionCustom::StaticClass(), 960, 520));
		if (!MaskNode || !WorldPosition)
		{
			return nullptr;
		}

		MaskNode->Description = TEXT("Right-side legacy water-mask footprint neutralizer");
		MaskNode->OutputType = CMOT_Float1;
		ConnectCustomInput(MaskNode, WorldPosition, TEXT("WorldPos"));
		MaskNode->Code = TEXT(R"(
#define DISTSEG(P,A,B) length((P) - ((A) + saturate(dot((P) - (A), (B) - (A)) / max(dot((B) - (A), (B) - (A)), 1.0)) * ((B) - (A))))
float2 p = WorldPos.xy;
float m = 0.0;
m = max(m, 1.0 - smoothstep(3600.0, 8600.0, DISTSEG(p, float2(13000.0, 8000.0), float2(42000.0, 22000.0))));
m = max(m, 1.0 - smoothstep(3600.0, 9000.0, DISTSEG(p, float2(42000.0, 22000.0), float2(76000.0, 32000.0))));
float2 cap = (p - float2(76000.0, 32000.0)) / float2(7600.0, 5200.0);
m = max(m, 1.0 - smoothstep(0.80, 1.30, length(cap)));
float legacyEastTrace = 0.0;
legacyEastTrace = max(legacyEastTrace, 1.0 - smoothstep(1500.0, 4300.0, DISTSEG(p, float2(10000.0, 36000.0), float2(12000.0, 60000.0))));
legacyEastTrace = max(legacyEastTrace, 1.0 - smoothstep(1500.0, 4700.0, DISTSEG(p, float2(12000.0, 60000.0), float2(16000.0, 87000.0))));
float titanPipelineProofFootprintA = 1.0 - smoothstep(0.72, 1.52, length((p - float2(93200.0, 111500.0)) / float2(9200.0, 7800.0)));
float titanPipelineProofFootprintB = 1.0 - smoothstep(0.68, 1.48, length((p - float2(100000.0, 106000.0)) / float2(10500.0, 8200.0)));
float titanPipelineProofFootprintC = 1.0 - smoothstep(0.45, 1.82, length((p - float2(97000.0, 108500.0)) / float2(18500.0, 13200.0)));
float tileBreak = 0.5 + 0.5 * sin(p.x * 0.00041 + p.y * -0.00033 + sin(p.x * 0.000083) * 2.1);
float tileWeave = 0.5 + 0.5 * sin(p.x * -0.00057 + p.y * 0.00046 + tileBreak * 2.8);
float2 tileA = abs((p - float2(93200.0, 111500.0)) / float2(6400.0, 5600.0));
float2 tileB = abs((p - float2(100000.0, 106000.0)) / float2(7200.0, 6100.0));
float2 tileC = abs((p - float2(97000.0, 108500.0)) / float2(11800.0, 8800.0));
float2 tileD = (p - float2(93200.0, 122500.0)) / float2(9000.0, 7200.0);
float tileDOrganic = length(tileD + float2(sin(p.y * 0.00019), cos(p.x * 0.00017)) * 0.15);
float titanPipelineTileRepairA = 1.0 - smoothstep(0.82 + tileBreak * 0.08, 1.26 + tileBreak * 0.18, max(tileA.x, tileA.y));
float titanPipelineTileRepairB = 1.0 - smoothstep(0.80 + tileBreak * 0.07, 1.24 + tileBreak * 0.17, max(tileB.x, tileB.y));
float titanPipelineTileRepairC = 1.0 - smoothstep(0.74 + tileBreak * 0.09, 1.32 + tileBreak * 0.20, max(tileC.x, tileC.y));
float titanPipelineTileRepairD = (1.0 - smoothstep(0.44 + tileBreak * 0.08, 2.05 + tileBreak * 0.42, tileDOrganic)) * saturate(0.16 + tileBreak * 0.24 + tileWeave * 0.18);
m = max(m, max(max(titanPipelineProofFootprintA, titanPipelineProofFootprintB), titanPipelineProofFootprintC));
m = max(m, max(max(titanPipelineTileRepairA, titanPipelineTileRepairB), titanPipelineTileRepairC));
m = max(m, titanPipelineTileRepairD);
return saturate(max(m * 0.72, legacyEastTrace * 0.92));
#undef DISTSEG
)");
		return MaskNode;
	}

	UMaterialExpressionCustom* AddRightSideGrassRepairColor(UMaterial* Material, UMaterialExpressionWorldPosition* WorldPosition)
	{
		UMaterialExpressionCustom* ColorNode = Cast<UMaterialExpressionCustom>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionCustom::StaticClass(), 960, 680));
		if (!ColorNode || !WorldPosition)
		{
			return nullptr;
		}

		ColorNode->Description = TEXT("Right-side normal grass repair color");
		ColorNode->OutputType = CMOT_Float3;
		ConnectCustomInput(ColorNode, WorldPosition, TEXT("WorldPos"));
		ColorNode->Code = TEXT(R"(
float2 p = WorldPos.xy;
float cellNoise = 0.5 + 0.5 * sin(p.x * 0.00019 + p.y * 0.00011 + sin(p.y * 0.000031) * 1.7);
float meadowNoise = 0.5 + 0.5 * sin(p.x * -0.000083 + p.y * 0.00014 + 1.3);
float bladeShadow = 0.5 + 0.5 * sin(p.x * 0.0020 + p.y * 0.0014 + meadowNoise * 2.2);
float3 openGrass = float3(0.165, 0.380, 0.112);
float3 lightGrass = float3(0.315, 0.515, 0.180);
float3 thatch = float3(0.335, 0.440, 0.135);
float lightPatch = 1.0 - smoothstep(6000.0, 30000.0, length(p - float2(36000.0, -26000.0)));
float3 grass = lerp(openGrass, lightGrass, saturate(0.24 + lightPatch * 0.34 + meadowNoise * 0.10));
grass = lerp(grass, thatch, saturate((1.0 - bladeShadow) * 0.10 + meadowNoise * 0.05));
float titanPipelineProofRepair = max(
    1.0 - smoothstep(0.55, 1.42, length((p - float2(93200.0, 111500.0)) / float2(10200.0, 8600.0))),
    1.0 - smoothstep(0.52, 1.38, length((p - float2(100000.0, 106000.0)) / float2(11600.0, 9000.0))));
titanPipelineProofRepair = max(titanPipelineProofRepair, 1.0 - smoothstep(0.36, 1.72, length((p - float2(97000.0, 108500.0)) / float2(20200.0, 14800.0))));
float tileBreak = 0.5 + 0.5 * sin(p.x * 0.00041 + p.y * -0.00033 + sin(p.x * 0.000083) * 2.1);
float tileWeave = 0.5 + 0.5 * sin(p.x * -0.00057 + p.y * 0.00046 + tileBreak * 2.8);
float2 tileA = abs((p - float2(93200.0, 111500.0)) / float2(6400.0, 5600.0));
float2 tileB = abs((p - float2(100000.0, 106000.0)) / float2(7200.0, 6100.0));
float2 tileC = abs((p - float2(97000.0, 108500.0)) / float2(11800.0, 8800.0));
float2 tileD = (p - float2(93200.0, 122500.0)) / float2(9000.0, 7200.0);
float tileDOrganic = length(tileD + float2(sin(p.y * 0.00019), cos(p.x * 0.00017)) * 0.15);
float titanPipelineTileRepairA = 1.0 - smoothstep(0.82 + tileBreak * 0.08, 1.26 + tileBreak * 0.18, max(tileA.x, tileA.y));
float titanPipelineTileRepairB = 1.0 - smoothstep(0.80 + tileBreak * 0.07, 1.24 + tileBreak * 0.17, max(tileB.x, tileB.y));
float titanPipelineTileRepairC = 1.0 - smoothstep(0.74 + tileBreak * 0.09, 1.32 + tileBreak * 0.20, max(tileC.x, tileC.y));
float titanPipelineTileRepairD = (1.0 - smoothstep(0.44 + tileBreak * 0.08, 2.05 + tileBreak * 0.42, tileDOrganic)) * saturate(0.16 + tileBreak * 0.24 + tileWeave * 0.18);
titanPipelineProofRepair = max(titanPipelineProofRepair, max(max(titanPipelineTileRepairA, titanPipelineTileRepairB), titanPipelineTileRepairC));
titanPipelineProofRepair = max(titanPipelineProofRepair, titanPipelineTileRepairD);
float3 proofGrass = lerp(float3(0.168, 0.300, 0.096), float3(0.274, 0.390, 0.132), saturate(0.18 + meadowNoise * 0.22 + cellNoise * 0.14 + tileBreak * 0.10));
proofGrass = lerp(proofGrass, float3(0.164, 0.290, 0.094), saturate((1.0 - bladeShadow) * 0.20 + (1.0 - tileBreak) * 0.10));
float3 mutedFootprintGrass = lerp(float3(0.205, 0.330, 0.108), float3(0.292, 0.392, 0.128), saturate(cellNoise * 0.36 + meadowNoise * 0.22 + tileBreak * 0.18));
mutedFootprintGrass = lerp(mutedFootprintGrass, grass, saturate(tileWeave * 0.48 + (1.0 - titanPipelineTileRepairD) * 0.32));
proofGrass = lerp(proofGrass, mutedFootprintGrass, saturate(titanPipelineTileRepairD * 0.38));
proofGrass *= lerp(0.92, 1.12, saturate(cellNoise * 0.58 + tileBreak * 0.42));
grass = lerp(grass, proofGrass, saturate(titanPipelineProofRepair * 0.78));
grass *= lerp(0.90, 1.09, cellNoise);
return grass;
)");
		return ColorNode;
	}

	UMaterialExpression* AddRightSideArtifactNeutralizer(UMaterial* Material, UMaterialExpressionWorldPosition* WorldPosition, UMaterialExpression* CurrentColor)
	{
		UMaterialExpressionCustom* MaskNode = AddRightSideArtifactMask(Material, WorldPosition);
		UMaterialExpressionCustom* RepairColor = AddRightSideGrassRepairColor(Material, WorldPosition);
		UMaterialExpressionLinearInterpolate* BlendNode = Cast<UMaterialExpressionLinearInterpolate>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionLinearInterpolate::StaticClass(), 1280, 600));
		if (!MaskNode || !RepairColor || !BlendNode || !CurrentColor)
		{
			return CurrentColor;
		}

		BlendNode->A.Expression = CurrentColor;
		BlendNode->B.Expression = RepairColor;
		BlendNode->Alpha.Expression = MaskNode;
		return BlendNode;
	}

	UMaterialExpressionCustom* AddBorderMountainRockIntegration(UMaterial* Material, UMaterialExpressionWorldPosition* WorldPosition, UMaterialExpressionVertexNormalWS* VertexNormal, UMaterialExpression* CurrentColor, UMaterialExpression* RockTextureColor)
	{
		UMaterialExpressionCustom* ColorNode = Cast<UMaterialExpressionCustom>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionCustom::StaticClass(), 1510, 600));
		if (!ColorNode || !WorldPosition || !VertexNormal || !CurrentColor || !RockTextureColor)
		{
			return nullptr;
		}

		ColorNode->Description = TEXT("V58 outer ring conversion: existing Landscape ring receives Titan rock texture/strata mountain surface");
		ColorNode->OutputType = CMOT_Float3;
		ConnectCustomInput(ColorNode, WorldPosition, TEXT("WorldPos"));
		ConnectCustomInput(ColorNode, VertexNormal, TEXT("NormalWS"));
		ConnectCustomInput(ColorNode, CurrentColor, TEXT("BaseColor"));
		ConnectCustomInput(ColorNode, RockTextureColor, TEXT("RockTex"));
		ColorNode->Code = TEXT(R"(
float2 p = WorldPos.xy;
float2 centered = (p - float2(0.0, 15000.0)) / float2(100000.0, 72000.0);
float organic = length(centered) + 0.055 * sin(p.x / 11500.0) - 0.045 * cos(p.y / 9000.0) + 0.035 * sin((p.x + p.y) / 17000.0);
float slopeAmount = 1.0 - saturate(NormalWS.z);
float northRing = smoothstep(99000.0, 126000.0, p.y);
float eastRing = smoothstep(80000.0, 107000.0, p.x) * smoothstep(45500.0, 123000.0, p.y);
float westRing = (1.0 - smoothstep(36000.0, 52500.0, p.x)) * smoothstep(39000.0, 126000.0, p.y);
float southRing = (1.0 - smoothstep(43000.0, 63500.0, p.y)) * (1.0 - smoothstep(134000.0, 152000.0, abs(p.x - 42000.0)));
float outerRing = max(max(max(northRing, eastRing), westRing), southRing);
float outerShape = smoothstep(0.46, 0.76, organic);
float ringBody = smoothstep(0.50, 0.82, organic);
float heightMask = smoothstep(-5600.0, -1250.0, WorldPos.z);
float faceSteepness = smoothstep(0.020, 0.145, slopeAmount);
float capCoverage = smoothstep(0.60, 0.93, organic) * smoothstep(-5400.0, -850.0, WorldPos.z);
float bodyCoverage = saturate(max(ringBody * 0.92, max(faceSteepness * 0.78, capCoverage * 0.82)));
float borderCoverage = saturate(outerRing * bodyCoverage);
float faceMask = saturate(borderCoverage * (1.26 + faceSteepness * 0.46 + ringBody * 0.32 + heightMask * 0.10));
float geologicFlow = 0.5 + 0.5 * sin(p.x * 0.000052 - p.y * 0.000071 + sin((p.x + p.y) * 0.000019) * 2.7);
float massSplit = 0.5 + 0.5 * sin(p.x * 0.000083 - p.y * 0.000120 + geologicFlow * 2.0);
float broadFace = 0.5 + 0.5 * sin(p.x * 0.000043 - p.y * 0.000061 + massSplit * 2.8);
float strata = 0.5 + 0.5 * sin(p.x * 0.000160 - p.y * 0.000250 + broadFace * 2.4);
float diagonal = 0.5 + 0.5 * sin(p.x * 0.000190 + p.y * 0.000130 + geologicFlow * 2.6);
float fracture = 0.5 + 0.5 * sin(p.x * -0.000340 + p.y * 0.000270 + massSplit * 1.8);
float crossCut = 0.5 + 0.5 * sin(p.x * -0.000170 + p.y * 0.000090 + broadFace * 2.2);
float layerCut = smoothstep(0.70, 0.96, strata) * smoothstep(0.05, 0.24, slopeAmount) * smoothstep(0.38, 0.86, massSplit);
float verticalStain = smoothstep(0.42, 0.86, diagonal) * faceSteepness;
float embeddedBreak = smoothstep(0.62, 0.92, fracture) * faceSteepness;
float hardShelfMass = smoothstep(0.58, 0.92, massSplit) * smoothstep(0.48, 0.88, broadFace);
float cliffPlane = smoothstep(0.42, 0.88, crossCut) * smoothstep(0.04, 0.22, slopeAmount);
float shelfBreak = smoothstep(0.64, 0.94, strata) * smoothstep(0.38, 0.82, diagonal);
float angledStrata = 0.5 + 0.5 * sin(p.x * 0.000105 - p.y * 0.000150 + WorldPos.z * 0.00058 + massSplit * 2.1 + broadFace * 1.3);
float secondaryStrata = 0.5 + 0.5 * sin(p.x * -0.000075 + p.y * 0.000118 + WorldPos.z * 0.00042 + diagonal * 2.6);
float shelfShadow = smoothstep(0.66, 0.94, angledStrata) * smoothstep(0.28, 0.88, broadFace);
float broadStrataShadow = smoothstep(0.58, 0.88, secondaryStrata) * smoothstep(0.30, 0.86, massSplit);
float3 rockLow = float3(0.026, 0.032, 0.031);
float3 rockMid = float3(0.088, 0.084, 0.062);
float3 rockHigh = float3(0.215, 0.185, 0.126);
float3 rockCool = float3(0.040, 0.061, 0.058);
float texLum = dot(saturate(RockTex), float3(0.299, 0.587, 0.114));
float3 texRock = lerp(rockLow, rockHigh, saturate((texLum - 0.12) * 1.28));
texRock = lerp(texRock, saturate(RockTex) * float3(0.40, 0.36, 0.265), saturate(0.26 + faceSteepness * 0.26 + ringBody * 0.12));
float3 titanRock = lerp(rockLow, rockMid, saturate(0.34 + strata * 0.22 + diagonal * 0.16 + broadFace * 0.18 + hardShelfMass * 0.18));
titanRock = lerp(titanRock, rockHigh, saturate(layerCut * 0.32 + heightMask * 0.08 + broadFace * 0.14 + shelfBreak * 0.18 + capCoverage * 0.18));
titanRock = lerp(titanRock, titanRock * float3(0.26, 0.32, 0.30), saturate(verticalStain * 0.62 + embeddedBreak * 0.56 + cliffPlane * 0.42 + shelfShadow * faceSteepness * 0.28));
titanRock = lerp(titanRock, titanRock * float3(1.18, 1.00, 0.76), saturate(strata * diagonal * broadFace * 0.17 + shelfBreak * 0.10));
titanRock = lerp(titanRock, rockCool, saturate((1.0 - diagonal) * faceSteepness * 0.24));
titanRock *= lerp(0.42, 1.18, saturate(crossCut * 0.34 + broadFace * 0.30 + fracture * 0.18 + hardShelfMass * 0.20));
titanRock *= lerp(1.0, 0.62, saturate((shelfShadow * 0.30 + broadStrataShadow * 0.18) * max(faceSteepness, ringBody * 0.55)));
float grainA = 0.5 + 0.5 * sin(p.x * 0.00140 - p.y * 0.00105 + WorldPos.z * 0.00075 + diagonal * 2.1);
float grainB = 0.5 + 0.5 * sin(p.x * -0.00175 + p.y * 0.00122 + broadFace * 2.7);
float fracturedSurface = smoothstep(0.55, 0.92, grainA * 0.56 + grainB * 0.44) * max(faceSteepness, ringBody * 0.38);
titanRock = lerp(titanRock, texRock, saturate(borderCoverage * (0.42 + faceSteepness * 0.30 + fracturedSurface * 0.18)));
titanRock = lerp(titanRock, titanRock * float3(0.48, 0.54, 0.52), saturate(fracturedSurface * 0.38));
titanRock = lerp(titanRock, titanRock * float3(1.16, 1.06, 0.84), saturate((1.0 - grainB) * fracturedSurface * 0.12));
titanRock = lerp(titanRock, float3(0.038, 0.048, 0.044), saturate(faceSteepness * embeddedBreak * 0.22 + cliffPlane * 0.12));
float toeDust = borderCoverage * (1.0 - faceSteepness) * smoothstep(-5600.0, -2700.0, WorldPos.z);
float3 toeColor = float3(0.205, 0.180, 0.125);
float3 blended = lerp(BaseColor, toeColor, saturate(toeDust * 0.34));
float alpha = saturate(faceMask * (1.72 + layerCut * 0.18 + embeddedBreak * 0.20 + hardShelfMass * 0.24 + cliffPlane * 0.20));
return lerp(blended, titanRock, alpha);
)");
		return ColorNode;
	}

	UMaterialExpressionCustom* AddProceduralFallbackColor(UMaterial* Material, UMaterialExpressionWorldPosition* WorldPosition, UMaterialExpressionVertexNormalWS* VertexNormal)
	{
		UMaterialExpressionCustom* ColorNode = Cast<UMaterialExpressionCustom>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionCustom::StaticClass(), -860, -120));
		if (!ColorNode || !WorldPosition || !VertexNormal)
		{
			return nullptr;
		}

		ColorNode->Description = TEXT("Procedural Highland fallback masks: grass, rock, paths, wet shore, waterbed");
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
float edgeRock = smoothstep(0.72, 1.02, organic) * smoothstep(0.04, 0.14, slopeAmount);
float elevatedRock = smoothstep(-4200.0, -900.0, WorldPos.z) * smoothstep(0.38, 0.62, organic);
float rimShadow = smoothstep(0.78, 0.98, organic);
float erosionFracture = smoothstep(0.58, 0.95, 0.5 + 0.5 * sin(p.x * 0.00018 - p.y * 0.00024 + sin((p.x + p.y) * 0.000026) * 2.4));
float rimCut = smoothstep(0.66, 0.94, organic) * smoothstep(0.03, 0.16, slopeAmount) * erosionFracture;
float rock = saturate(max(max(max(slopeRock, edgeRock), elevatedRock), rimCut * 0.82));

float path = 0.0;
path = max(path, 1.0 - smoothstep(520.0, 1150.0, DISTSEG(p, float2(-10000.0, 6500.0), float2(9000.0, 9000.0))));
path = max(path, 1.0 - smoothstep(520.0, 1150.0, DISTSEG(p, float2(9000.0, 9000.0), float2(36000.0, 21000.0))));
path = max(path, 1.0 - smoothstep(520.0, 1150.0, DISTSEG(p, float2(36000.0, 21000.0), float2(76000.0, 32000.0))));
path = max(path, 1.0 - smoothstep(520.0, 1100.0, DISTSEG(p, float2(10000.0, 36000.0), float2(12000.0, 60000.0))));
path = max(path, 1.0 - smoothstep(520.0, 1100.0, DISTSEG(p, float2(12000.0, 60000.0), float2(16000.0, 87000.0))));
)")
TEXT(R"(
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
float warmGradient = 0.5 + 0.5 * sin(p.x * 0.000047 + p.y * 0.000032 + sin(p.y * 0.000017) * 1.8);
float coolGradient = 0.5 + 0.5 * sin(p.x * -0.000035 + p.y * 0.000067 + 2.4);
float moistureGradient = 0.5 + 0.5 * sin((p.x * 0.62 + p.y) * 0.000082 + sin(p.x * 0.000023) * 2.2);
float cellNoise = 0.5 + 0.5 * sin(p.x * 0.00023 - p.y * 0.00017 + warmGradient * 1.6);
float broadNoise = saturate(warmGradient * 0.48 + coolGradient * 0.28 + moistureGradient * 0.24);
darkPatch = saturate(darkPatch * 0.62 + broadNoise * 0.18 + moistureGradient * 0.10);
lightPatch = saturate(lightPatch * 0.48 + (1.0 - broadNoise) * 0.12 + warmGradient * 0.10);
float meadowNoise = 0.5 + 0.5 * sin(p.x * 0.00031 + p.y * -0.00021 + moistureGradient * 2.1);
float microWave = 0.5 + 0.5 * sin(p.x * 0.0021 + p.y * 0.00145 + sin((p.x - p.y) * 0.00036) * 1.8);
float titanFlow = 0.5 + 0.5 * sin(p.x * 0.000026 + p.y * 0.000044 + sin((p.x - p.y) * 0.000013) * 2.8);
float biomeFeather = 0.5 + 0.5 * sin(p.x * 0.000019 - p.y * 0.000029 + sin((p.x + p.y) * 0.000009) * 3.4);
float macroCrossfade = saturate(biomeFeather * 0.42 + broadNoise * 0.34 + moistureGradient * 0.24);
float ecologyFlow = 0.5 + 0.5 * sin(p.x * 0.000041 - p.y * 0.000026 + titanFlow * 2.9);
float meadowThread = 0.5 + 0.5 * sin(p.x * -0.000052 + p.y * 0.000031 + biomeFeather * 2.6);
float canopyCoolness = saturate(moistureGradient * 0.42 + (1.0 - warmGradient) * 0.26 + ecologyFlow * 0.22);
float macroAntiBlob = 0.5 + 0.5 * sin(p.x * 0.000071 + p.y * -0.000049 + sin((p.x + p.y) * 0.000018) * 2.7);
float fieldThread = 0.5 + 0.5 * sin(p.x * -0.000083 + p.y * 0.000039 + macroAntiBlob * 2.3);
float borderTone = smoothstep(0.58, 0.78, organic) * (0.55 + 0.45 * macroAntiBlob);

float3 lightGrass = float3(0.315, 0.515, 0.180);
float3 openGrass = float3(0.165, 0.380, 0.112);
float3 darkGrass = float3(0.065, 0.225, 0.055);
float3 meadowThatch = float3(0.335, 0.440, 0.135);
float3 coolMeadow = float3(0.120, 0.315, 0.110);
float3 dryMeadow = float3(0.365, 0.455, 0.145);
float3 mossWarmth = float3(0.010, 0.018, 0.000);
float3 pathColor = float3(0.42, 0.34, 0.22);
float3 shoreColor = float3(0.15, 0.18, 0.095);
float3 waterbedColor = float3(0.10, 0.09, 0.075);
float3 rockLow = float3(0.175, 0.145, 0.105);
float3 rockMid = float3(0.315, 0.245, 0.170);
float3 rockHigh = float3(0.520, 0.440, 0.320);
float3 waterColor = float3(0.02, 0.31, 0.50);
)")
TEXT(R"(
float grassPlacementShade = 0.0;
grassPlacementShade = max(grassPlacementShade, 1.0 - smoothstep(0.74, 1.12, length((p - float2(-18120.0, 4000.0)) / float2(5400.0, 4300.0))));
grassPlacementShade = max(grassPlacementShade, 1.0 - smoothstep(0.78, 1.18, length((p - float2(-6000.0, -14500.0)) / float2(15000.0, 9000.0))));
grassPlacementShade = max(grassPlacementShade, 1.0 - smoothstep(0.78, 1.20, length((p - float2(26000.0, -17500.0)) / float2(18000.0, 12500.0))));
grassPlacementShade = max(grassPlacementShade, 1.0 - smoothstep(0.76, 1.16, length((p - float2(36000.0, 13500.0)) / float2(14500.0, 9500.0))));
grassPlacementShade = max(grassPlacementShade, 1.0 - smoothstep(0.76, 1.18, length((p - float2(-41000.0, 18000.0)) / float2(11500.0, 15000.0))));
grassPlacementShade *= saturate((1.0 - path) * (1.0 - water) * (1.0 - shore) * (1.0 - rock));

float3 grass = lerp(openGrass, darkGrass, saturate(darkPatch * 0.58 + grassPlacementShade * 0.34 + coolGradient * 0.12));
grass = lerp(grass, lightGrass, saturate(lightPatch * 0.26 + (1.0 - darkPatch) * warmGradient * 0.07));
grass = lerp(grass, meadowThatch, saturate((1.0 - microWave) * 0.12 + (1.0 - moistureGradient) * 0.10));
grass = lerp(grass, coolMeadow, saturate(moistureGradient * 0.13 + (1.0 - titanFlow) * 0.06));
grass = lerp(grass, dryMeadow, saturate((1.0 - moistureGradient) * 0.10 + titanFlow * 0.06));
grass = lerp(grass, grass * float3(0.82, 1.035, 0.88), saturate((1.0 - rock) * macroCrossfade * 0.16));
grass = lerp(grass, grass * float3(1.035, 1.025, 0.90), saturate((1.0 - rock) * (1.0 - macroCrossfade) * 0.10));
grass = lerp(grass, grass * float3(0.86, 0.985, 0.90), saturate(moistureGradient * 0.14));
grass = lerp(grass, grass * float3(0.86, 0.98, 0.82), saturate(canopyCoolness * ecologyFlow * (1.0 - rock) * 0.10));
grass = lerp(grass, grass * float3(1.07, 1.02, 0.86), saturate(meadowThread * (1.0 - canopyCoolness) * (1.0 - rock) * 0.07));
grass = lerp(grass, grass * float3(0.88, 0.98, 0.84), saturate(macroAntiBlob * (1.0 - rock) * 0.08));
grass = lerp(grass, grass * float3(1.035, 1.015, 0.88), saturate(fieldThread * (1.0 - macroAntiBlob) * (1.0 - rock) * 0.045));
grass = lerp(grass, grass * float3(0.86, 0.91, 0.76), saturate(borderTone * (1.0 - rock) * 0.10));
grass *= lerp(0.925, 1.015, saturate(cellNoise * 0.30 + broadNoise * 0.42 + microWave * 0.18 + titanFlow * 0.10));
grass = lerp(grass, grass * float3(0.80, 0.89, 0.76), saturate(grassPlacementShade * (0.12 + meadowNoise * 0.10)));
grass += mossWarmth * saturate((1.0 - rock) * (1.0 - path) * (1.0 - water) * (0.35 + microWave * 0.65));
float rockStrata = 0.5 + 0.5 * sin(p.x * 0.00029 - p.y * 0.00019 + sin((p.x + p.y) * 0.000061) * 2.1);
float cliffBanding = 0.5 + 0.5 * sin(p.x * 0.00021 + p.y * 0.00034 + rockStrata * 1.8 + macroAntiBlob * 0.9);
float diagonalFracture = 0.5 + 0.5 * sin(p.x * 0.00017 + p.y * 0.00011 + cliffBanding * 1.7 + titanFlow * 0.8);
float verticalRhythm = 0.5 + 0.5 * sin(p.x * 0.00009 - p.y * 0.00014 + diagonalFracture * 2.3 + ecologyFlow * 0.7);
float erosionStain = 0.5 + 0.5 * sin(p.x * -0.00007 + p.y * 0.00021 + rockStrata * 2.6 + meadowThread * 0.8);
float softLichen = 0.5 + 0.5 * sin(p.x * -0.00013 + p.y * 0.00018 + fieldThread * 1.3);
float shelfShadow = 0.5 + 0.5 * sin(p.x * 0.00019 - p.y * 0.00016 + broadNoise * 1.9);
float rockTemperature = 0.5 + 0.5 * sin(p.x * 0.000045 + p.y * -0.000058 + ecologyFlow * 2.0);
float3 rockColor = lerp(rockLow, rockMid, saturate(0.36 + rockStrata * 0.28 + cliffBanding * 0.20 + diagonalFracture * 0.12));
rockColor = lerp(rockColor, rockHigh, saturate((1.0 - NormalWS.z) * 0.38 + rimShadow * 0.10 + shelfShadow * 0.14 + verticalRhythm * 0.12));
rockColor = lerp(rockColor, rockColor * float3(0.66, 0.73, 0.66), saturate((1.0 - shelfShadow) * slopeAmount * 0.18 + erosionStain * slopeAmount * 0.13));
rockColor = lerp(rockColor, rockColor * float3(0.90, 0.98, 0.84), saturate(softLichen * slopeAmount * 0.20));
rockColor = lerp(rockColor, rockColor * float3(1.10, 0.96, 0.78), saturate(diagonalFracture * verticalRhythm * rock * 0.12));
rockColor = lerp(rockColor, rockColor * float3(0.86, 0.94, 1.03), saturate((1.0 - rockTemperature) * shelfShadow * slopeAmount * 0.09));
rockColor = lerp(rockColor, rockColor * float3(1.08, 0.96, 0.84), saturate(rockTemperature * erosionStain * slopeAmount * 0.08));
rockColor = lerp(rockColor, rockColor * float3(0.78, 0.84, 0.76), saturate(borderTone * slopeAmount * 0.10));
rockColor = lerp(rockColor, rockColor * float3(1.08, 0.98, 0.86), saturate(macroAntiBlob * shelfShadow * rock * 0.06));
float3 color = grass;
color = lerp(color, rockColor, saturate(rock * (1.0 - water * 0.80) * (1.0 - path * 0.65)));
color = lerp(color, shoreColor, saturate(shore * 0.62));
color = lerp(color, waterbedColor, saturate(water * 0.35));
color = lerp(color, pathColor, saturate(path * (1.0 - water * 0.55)));
color = lerp(color, lightGrass, saturate(spawnGrassMask * (1.0 - water)));
return color;
#undef DISTSEG
)");
		return ColorNode;
	}

	ULandscapeLayerInfoObject* CreateOrUpdateLayerInfo(const FHighlandLayerSpec& Spec)
	{
		const FString AssetName = FString::Printf(TEXT("LI_%s"), *Spec.LayerName.ToString());
		const FString PackagePath = FString::Printf(TEXT("%s/%s"), LayerInfoFolderPath, *AssetName);
		const FString ObjectPath = FString::Printf(TEXT("%s.%s"), *PackagePath, *AssetName);

		ULandscapeLayerInfoObject* LayerInfo = LoadObject<ULandscapeLayerInfoObject>(nullptr, *ObjectPath);
		if (!LayerInfo)
		{
			UPackage* Package = CreatePackage(*PackagePath);
			if (!Package)
			{
				return nullptr;
			}

			LayerInfo = NewObject<ULandscapeLayerInfoObject>(Package, *AssetName, RF_Public | RF_Standalone | RF_Transactional);
			FAssetRegistryModule::AssetCreated(LayerInfo);
		}

		LayerInfo->Modify();
		LayerInfo->SetLayerName(Spec.LayerName, true);
		LayerInfo->SetLayerUsageDebugColor(Spec.Color, true, EPropertyChangeType::ValueSet);
		LayerInfo->MarkPackageDirty();
		return LayerInfo;
	}

	bool RebuildLandscapeLayerMaterial(const bool bIncludeV58MountainRockIntegration)
	{
		UMaterial* Material = LoadObject<UMaterial>(nullptr, LandscapeMaterialPath);
		if (!Material)
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandLandscapeLayers: missing landscape material %s"), LandscapeMaterialPath);
			return false;
		}

		Material->Modify();
		UMaterialEditingLibrary::DeleteAllMaterialExpressions(Material);

		UMaterialExpressionWorldPosition* WorldPosition = AddWorldPositionNode(Material, -1220, -120);
		UMaterialExpressionVertexNormalWS* VertexNormal = Cast<UMaterialExpressionVertexNormalWS>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionVertexNormalWS::StaticClass(), -1220, 40));
		UMaterialExpressionCustom* FallbackColor = AddProceduralFallbackColor(Material, WorldPosition, VertexNormal);
		UMaterialExpressionConstant* Roughness = Cast<UMaterialExpressionConstant>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant::StaticClass(), 760, 260));

		if (!WorldPosition || !VertexNormal || !FallbackColor || !Roughness)
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandLandscapeLayers: failed to build landscape material expressions."));
			return false;
		}

		Roughness->R = 0.96f;

		UMaterialExpression* CurrentColor = FallbackColor;
		int32 LayerIndex = 0;
		for (const FHighlandLayerSpec& Spec : LayerSpecs)
		{
			UMaterialExpression* LayerColor = (Spec.LayerName == FName(TEXT("Grass_Light")) || Spec.LayerName == FName(TEXT("Grass_Dark")))
				? static_cast<UMaterialExpression*>(AddGrassLayerBlendColorNode(Material, WorldPosition, Spec.LayerName, -420, -420 + LayerIndex * 150))
				: static_cast<UMaterialExpression*>(AddLayerColorNode(Material, Spec, -420, -420 + LayerIndex * 150));
			CurrentColor = AddLandscapeLayerWeight(Material, CurrentColor, LayerColor, Spec.LayerName, 80 + LayerIndex * 150, -420 + LayerIndex * 150);
			++LayerIndex;
		}
		CurrentColor = AddRightSideArtifactNeutralizer(Material, WorldPosition, CurrentColor);
		if (bIncludeV58MountainRockIntegration)
		{
			UMaterialExpressionCustom* RockUv = AddRockUvNode(Material, WorldPosition);
			UMaterialExpression* RockTextureColor = AddV58TitanRockTextureColor(Material, RockUv);
			if (UMaterialExpressionCustom* BorderMountainRock = AddBorderMountainRockIntegration(Material, WorldPosition, VertexNormal, CurrentColor, RockTextureColor))
			{
				CurrentColor = BorderMountainRock;
			}
		}

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
}
#endif

int32 UFFStarterHighlandLandscapeLayersCommandlet::Main(const FString& Params)
{
#if WITH_EDITOR
	const bool bRestorePreV58Green = Params.Contains(TEXT("RestorePreV58Green"), ESearchCase::IgnoreCase);
	UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandLandscapeLayers: loading %s without terrain geometry edits."), HighlandMapPath);
	UWorld* World = UEditorLoadingAndSavingUtils::LoadMap(HighlandMapPath);
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandLandscapeLayers: failed to load %s"), HighlandMapPath);
		return 1;
	}

	if (!RebuildLandscapeLayerMaterial(!bRestorePreV58Green))
	{
		return 1;
	}

	TMap<FName, ULandscapeLayerInfoObject*> LayerInfoByName;
	for (const FHighlandLayerSpec& Spec : LayerSpecs)
	{
		ULandscapeLayerInfoObject* LayerInfo = CreateOrUpdateLayerInfo(Spec);
		if (!LayerInfo)
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandLandscapeLayers: failed to create layer info for %s"), *Spec.LayerName.ToString());
			return 1;
		}
		LayerInfoByName.Add(Spec.LayerName, LayerInfo);
		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandLandscapeLayers: layer=%s info=%s"), *Spec.LayerName.ToString(), *LayerInfo->GetPathName());
	}

	int32 LandscapeCount = 0;
	int32 ComponentCount = 0;
	UMaterial* LandscapeMaterial = LoadObject<UMaterial>(nullptr, LandscapeMaterialPath);
	if (!LandscapeMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandLandscapeLayers: failed to reload landscape material %s"), LandscapeMaterialPath);
		return 1;
	}
	for (TActorIterator<ALandscapeProxy> It(World); It; ++It)
	{
		ALandscapeProxy* LandscapeProxy = *It;
		if (!LandscapeProxy)
		{
			continue;
		}

		LandscapeProxy->Modify();
		LandscapeProxy->LandscapeMaterial = LandscapeMaterial;
		for (const TPair<FName, ULandscapeLayerInfoObject*>& Pair : LayerInfoByName)
		{
			const FLandscapeTargetLayerSettings Settings(Pair.Value);
			if (LandscapeProxy->HasTargetLayer(Pair.Key))
			{
				LandscapeProxy->UpdateTargetLayer(Pair.Key, Settings, false);
			}
			else
			{
				LandscapeProxy->AddTargetLayer(Pair.Key, Settings, false);
			}
		}
		for (ULandscapeComponent* LandscapeComponent : LandscapeProxy->LandscapeComponents)
		{
			if (!LandscapeComponent)
			{
				continue;
			}

			LandscapeComponent->Modify();
			LandscapeComponent->OverrideMaterial = LandscapeMaterial;
			LandscapeComponent->MaterialInstances.Empty();
			LandscapeComponent->UpdateMaterialInstances();
			LandscapeComponent->MarkPackageDirty();
			++ComponentCount;
		}
		LandscapeProxy->PostEditChange();
		LandscapeProxy->MarkPackageDirty();
		++LandscapeCount;
	}

	if (LandscapeCount == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandLandscapeLayers: no landscape proxy found in %s"), HighlandMapPath);
		return 1;
	}

	UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
	UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);

	UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandLandscapeLayers: completed layers=%d landscapes=%d components=%d material=%s restorePreV58Green=%s v58MountainRockIntegration=%s"),
		UE_ARRAY_COUNT(LayerSpecs),
		LandscapeCount,
		ComponentCount,
		LandscapeMaterialPath,
		bRestorePreV58Green ? TEXT("true") : TEXT("false"),
		bRestorePreV58Green ? TEXT("false") : TEXT("true"));
	return 0;
#else
	return 0;
#endif
}
