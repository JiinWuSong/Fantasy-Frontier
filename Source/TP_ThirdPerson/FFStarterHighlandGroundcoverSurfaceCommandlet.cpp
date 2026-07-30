#include "FFStarterHighlandGroundcoverSurfaceCommandlet.h"

#if WITH_EDITOR
#include "AssetRegistry/AssetRegistryModule.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "FileHelpers.h"
#include "GameFramework/PlayerStart.h"
#include "LandscapeComponent.h"
#include "LandscapeGrassType.h"
#include "LandscapeProxy.h"
#include "MaterialEditingLibrary.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionCustom.h"
#include "Materials/MaterialExpressionLandscapeGrassOutput.h"
#include "Materials/MaterialExpressionLandscapeLayerSample.h"
#include "Materials/MaterialExpressionLocalPosition.h"
#include "Materials/MaterialExpressionPerInstanceRandom.h"
#include "Materials/MaterialExpressionTime.h"
#include "Materials/MaterialExpressionVertexNormalWS.h"
#include "Materials/MaterialExpressionWorldPosition.h"
#include "Misc/Parse.h"
#include "UObject/Package.h"
#endif

UFFStarterHighlandGroundcoverSurfaceCommandlet::UFFStarterHighlandGroundcoverSurfaceCommandlet()
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
	const TCHAR* GroundcoverMaterialPackagePath = TEXT("/Game/FantasyFrontier/Blockout/Materials/M_FF_Highland_GrassBlade_V84");
	const TCHAR* GroundcoverMaterialObjectPath = TEXT("/Game/FantasyFrontier/Blockout/Materials/M_FF_Highland_GrassBlade_V84.M_FF_Highland_GrassBlade_V84");
	const TCHAR* GroundcoverGrassTypePackagePath = TEXT("/Game/FantasyFrontier/Blockout/Materials/GT_FF_Highland_Groundcover_V84");
	const TCHAR* GroundcoverGrassTypeObjectPath = TEXT("/Game/FantasyFrontier/Blockout/Materials/GT_FF_Highland_Groundcover_V84.GT_FF_Highland_Groundcover_V84");
	const TCHAR* TitanGrassMeshPath = TEXT("/Game/Environment/Foliage/Meshes/SM_GrassBlade.SM_GrassBlade");

	const FName AppliedTag(TEXT("FFHighlandGroundcoverSurfaceV84Applied"));
	const FName ValidationActorTag(TEXT("FFHighlandGroundcoverSurfaceV84Validation"));
	const FName GrassInputName(TEXT("FF_V84_TitanGroundcover"));
	const FName HistoricalNativePatchTag(TEXT("FFHighlandTitanGrassNativeTestPatch"));
	const FName HistoricalABPatchTag(TEXT("FFHighlandGrassABTestPatch"));
	const FString V84NodeToken(TEXT("FF V84"));

	struct FProtectedState
	{
		TArray<FTransform> PlayerStarts;
		TArray<FString> WaterActors;
		int32 MountainActors = 0;
		uint32 MountainTransformHash = 0;
	};

	bool IsWaterActor(const AActor* Actor)
	{
		if (!Actor)
		{
			return false;
		}

		const FString Identity = Actor->GetClass()->GetName()
			+ TEXT("|") + Actor->GetName()
			+ TEXT("|") + Actor->GetActorLabel();
		return Identity.Contains(TEXT("WaterBody"), ESearchCase::IgnoreCase)
			|| Identity.Contains(TEXT("Water_Lake_1"), ESearchCase::IgnoreCase)
			|| Identity.Contains(TEXT("Lake"), ESearchCase::IgnoreCase)
			|| Identity.Contains(TEXT("River"), ESearchCase::IgnoreCase);
	}

	bool IsMountainBaselineActor(const AActor* Actor)
	{
		if (!Actor)
		{
			return false;
		}

		for (const FName Tag : Actor->Tags)
		{
			const FString TagString = Tag.ToString();
			if (TagString.Contains(TEXT("V69"), ESearchCase::IgnoreCase)
				|| TagString.Contains(TEXT("V70"), ESearchCase::IgnoreCase)
				|| TagString.Contains(TEXT("Mountain"), ESearchCase::IgnoreCase)
				|| TagString.Contains(TEXT("OuterRing"), ESearchCase::IgnoreCase))
			{
				return true;
			}
		}
		return false;
	}

	FProtectedState CaptureProtectedState(UWorld* World)
	{
		FProtectedState State;
		if (!World)
		{
			return State;
		}

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			const AActor* Actor = *It;
			if (!Actor)
			{
				continue;
			}

			if (Actor->IsA<APlayerStart>())
			{
				State.PlayerStarts.Add(Actor->GetActorTransform());
			}
			if (IsWaterActor(Actor))
			{
				State.WaterActors.Add(FString::Printf(
					TEXT("%s|%s"),
					*Actor->GetPathName(),
					*Actor->GetActorTransform().ToHumanReadableString()));
			}
			if (IsMountainBaselineActor(Actor))
			{
				++State.MountainActors;
				State.MountainTransformHash = HashCombine(
					State.MountainTransformHash,
					GetTypeHash(Actor->GetActorTransform().ToHumanReadableString()));
			}
		}

		State.WaterActors.Sort();
		return State;
	}

	bool ProtectedStateMatches(const FProtectedState& Before, const FProtectedState& After)
	{
		if (Before.PlayerStarts.Num() != After.PlayerStarts.Num()
			|| Before.WaterActors != After.WaterActors
			|| Before.MountainActors != After.MountainActors
			|| Before.MountainTransformHash != After.MountainTransformHash)
		{
			return false;
		}

		for (int32 Index = 0; Index < Before.PlayerStarts.Num(); ++Index)
		{
			if (!Before.PlayerStarts[Index].Equals(After.PlayerStarts[Index], 0.01f))
			{
				return false;
			}
		}
		return true;
	}

	void ConnectCustomInput(
		UMaterialExpressionCustom* CustomNode,
		UMaterialExpression* InputExpression,
		const FName InputName,
		const int32 OutputIndex = 0)
	{
		if (!CustomNode || !InputExpression)
		{
			return;
		}

		FCustomInput CustomInput;
		CustomInput.InputName = InputName;
		CustomInput.Input.Expression = InputExpression;
		CustomInput.Input.OutputIndex = OutputIndex;
		CustomNode->Inputs.Add(CustomInput);
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

	UMaterialExpressionWorldPosition* AddWorldPositionNode(UMaterial* Material, const int32 X, const int32 Y)
	{
		UMaterialExpressionWorldPosition* WorldPosition = Cast<UMaterialExpressionWorldPosition>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionWorldPosition::StaticClass(), X, Y));
		if (WorldPosition)
		{
			WorldPosition->WorldPositionShaderOffset = WPT_ExcludeAllShaderOffsets;
			WorldPosition->Desc = V84NodeToken + TEXT(" world position");
		}
		return WorldPosition;
	}

	UMaterial* CreateOrLoadGroundcoverMaterial()
	{
		UMaterial* Material = LoadObject<UMaterial>(nullptr, GroundcoverMaterialObjectPath);
		if (!Material)
		{
			UPackage* Package = CreatePackage(GroundcoverMaterialPackagePath);
			if (!Package)
			{
				return nullptr;
			}
			Material = NewObject<UMaterial>(
				Package,
				TEXT("M_FF_Highland_GrassBlade_V84"),
				RF_Public | RF_Standalone | RF_Transactional);
			FAssetRegistryModule::AssetCreated(Material);
		}

		Material->Modify();
		Material->PreEditChange(nullptr);
		UMaterialEditingLibrary::DeleteAllMaterialExpressions(Material);
		Material->bUseMaterialAttributes = false;
		Material->BlendMode = BLEND_Opaque;
		Material->TwoSided = true;
		Material->SetShadingModel(MSM_TwoSidedFoliage);
		Material->bUsedWithInstancedStaticMeshes = true;
		Material->bUsedWithNanite = true;

		UMaterialExpressionWorldPosition* WorldPosition = AddWorldPositionNode(Material, -920, -220);
		UMaterialExpressionLocalPosition* LocalPosition = Cast<UMaterialExpressionLocalPosition>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionLocalPosition::StaticClass(), -920, -70));
		UMaterialExpressionPerInstanceRandom* InstanceRandom = Cast<UMaterialExpressionPerInstanceRandom>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionPerInstanceRandom::StaticClass(), -920, 90));
		UMaterialExpressionTime* Time = Cast<UMaterialExpressionTime>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionTime::StaticClass(), -920, 250));
		UMaterialExpressionCustom* Color = Cast<UMaterialExpressionCustom>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionCustom::StaticClass(), -560, -160));
		UMaterialExpressionCustom* Subsurface = Cast<UMaterialExpressionCustom>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionCustom::StaticClass(), -230, -55));
		UMaterialExpressionCustom* Wind = Cast<UMaterialExpressionCustom>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionCustom::StaticClass(), -560, 170));
		UMaterialExpressionConstant* Roughness = Cast<UMaterialExpressionConstant>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant::StaticClass(), -230, 160));
		UMaterialExpressionConstant* Specular = Cast<UMaterialExpressionConstant>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant::StaticClass(), -230, 260));

		if (!WorldPosition || !LocalPosition || !InstanceRandom || !Time || !Color || !Subsurface || !Wind || !Roughness || !Specular)
		{
			return nullptr;
		}

		LocalPosition->Desc = V84NodeToken + TEXT(" local blade position");
		InstanceRandom->Desc = V84NodeToken + TEXT(" per-instance variation");
		Time->Desc = V84NodeToken + TEXT(" wind time");

		Color->Description = V84NodeToken + TEXT(" Titan-inspired blade color with root/tip and macro variation");
		Color->Desc = V84NodeToken + TEXT(" blade color");
		Color->OutputType = CMOT_Float3;
		ConnectCustomInput(Color, WorldPosition, TEXT("WorldPos"));
		ConnectCustomInput(Color, LocalPosition, TEXT("LocalPos"));
		ConnectCustomInput(Color, InstanceRandom, TEXT("InstanceRandom"));
		Color->Code = TEXT(R"(
float2 p = WorldPos.xy;
float rootToTip = smoothstep(-5.0, 86.0, LocalPos.z);
float broadA = 0.5 + 0.5 * sin(p.x / 51000.0 + p.y / 73000.0 + sin(p.y / 61000.0));
float broadB = 0.5 + 0.5 * sin(-p.x / 69000.0 + p.y / 43000.0 + 1.35);
float mid = 0.5 + 0.5 * sin(p.x / 12800.0 - p.y / 16400.0 + InstanceRandom * 6.28318);
float ecology = saturate(broadA * 0.43 + broadB * 0.34 + mid * 0.23);

float3 deepGreen = float3(0.006, 0.022, 0.003);
float3 softGreen = float3(0.014, 0.045, 0.006);
float3 warmGreen = float3(0.028, 0.070, 0.009);
float3 ground = lerp(deepGreen, softGreen, saturate(0.30 + ecology * 0.62));
ground = lerp(ground, warmGreen, saturate((broadB - 0.53) * 0.32));
float3 root = ground * float3(0.64, 0.76, 0.58);
float3 tip = lerp(ground, warmGreen, saturate(0.04 + mid * 0.09 + InstanceRandom * 0.03));
float3 color = lerp(root, tip, saturate(rootToTip * 0.72));
return color * lerp(0.90, 1.08, InstanceRandom);
)");

		Subsurface->Description = V84NodeToken + TEXT(" restrained foliage transmission");
		Subsurface->Desc = V84NodeToken + TEXT(" subsurface");
		Subsurface->OutputType = CMOT_Float3;
		ConnectCustomInput(Subsurface, Color, TEXT("BladeColor"));
		Subsurface->Code = TEXT("return BladeColor * float3(0.18, 0.24, 0.12);");

		Wind->Description = V84NodeToken + TEXT(" lightweight root-fixed two-frequency wind");
		Wind->Desc = V84NodeToken + TEXT(" wind");
		Wind->OutputType = CMOT_Float3;
		ConnectCustomInput(Wind, WorldPosition, TEXT("WorldPos"));
		ConnectCustomInput(Wind, LocalPosition, TEXT("LocalPos"));
		ConnectCustomInput(Wind, Time, TEXT("Time"));
		ConnectCustomInput(Wind, InstanceRandom, TEXT("InstanceRandom"));
		Wind->Code = TEXT(R"(
float rootMask = smoothstep(4.0, 88.0, LocalPos.z);
rootMask *= rootMask;
float phase = InstanceRandom * 6.28318;
float2 baseDir = normalize(float2(0.78, 0.38));
float angle = (InstanceRandom - 0.5) * 0.85;
float s = sin(angle);
float c = cos(angle);
float2 windDir = normalize(float2(baseDir.x * c - baseDir.y * s, baseDir.x * s + baseDir.y * c));
float gust = 0.72 + 0.28 * sin(dot(WorldPos.xy, float2(0.0012, 0.0008)) + Time * 0.48 + phase);
float sway = sin(dot(WorldPos.xy, float2(0.0085, 0.0125)) + Time * 1.08 + phase);
float flutter = sin(dot(WorldPos.xy, float2(-0.014, 0.006)) + Time * 1.75 + phase * 0.42) * 0.18;
float bend = (sway + flutter) * gust * lerp(2.2, 5.2, InstanceRandom) * rootMask;
return float3(windDir * bend, 0.0);
)");

		Roughness->R = 0.92f;
		Roughness->Desc = V84NodeToken + TEXT(" grass roughness");
		Specular->R = 0.035f;
		Specular->Desc = V84NodeToken + TEXT(" grass specular");

		UMaterialEditingLibrary::ConnectMaterialProperty(Color, TEXT(""), MP_BaseColor);
		UMaterialEditingLibrary::ConnectMaterialProperty(Subsurface, TEXT(""), MP_SubsurfaceColor);
		UMaterialEditingLibrary::ConnectMaterialProperty(Wind, TEXT(""), MP_WorldPositionOffset);
		UMaterialEditingLibrary::ConnectMaterialProperty(Roughness, TEXT(""), MP_Roughness);
		UMaterialEditingLibrary::ConnectMaterialProperty(Specular, TEXT(""), MP_Specular);
		FinalizeMaterial(Material);
		return Material;
	}

	ULandscapeGrassType* CreateOrLoadGroundcoverGrassType(UMaterialInterface* GrassMaterial)
	{
		UStaticMesh* GrassMesh = LoadObject<UStaticMesh>(nullptr, TitanGrassMeshPath);
		if (!GrassMaterial || !GrassMesh)
		{
			return nullptr;
		}

		ULandscapeGrassType* GrassType = LoadObject<ULandscapeGrassType>(nullptr, GroundcoverGrassTypeObjectPath);
		if (!GrassType)
		{
			UPackage* Package = CreatePackage(GroundcoverGrassTypePackagePath);
			if (!Package)
			{
				return nullptr;
			}
			GrassType = NewObject<ULandscapeGrassType>(
				Package,
				TEXT("GT_FF_Highland_Groundcover_V84"),
				RF_Public | RF_Standalone | RF_Transactional);
			FAssetRegistryModule::AssetCreated(GrassType);
		}

		auto ConfigureVariety = [GrassMesh, GrassMaterial](
			FGrassVariety& Variety,
			const float Density,
			const FFloatInterval& XScale,
			const FFloatInterval& YScale,
			const FFloatInterval& ZScale,
			const int32 StartCull,
			const int32 EndCull,
			const int32 WpoDisableDistance)
		{
			Variety.GrassMesh = GrassMesh;
			Variety.OverrideMaterials.Reset();
			Variety.OverrideMaterials.Add(GrassMaterial);
			Variety.GrassDensity = FPerPlatformFloat(Density);
			Variety.GrassDensityQuality = FPerQualityLevelFloat(Density);
			Variety.bUseGrid = true;
			Variety.PlacementJitter = 1.0f;
			Variety.StartCullDistance = FPerPlatformInt(StartCull);
			Variety.StartCullDistanceQuality = FPerQualityLevelInt(StartCull);
			Variety.EndCullDistance = FPerPlatformInt(EndCull);
			Variety.EndCullDistanceQuality = FPerQualityLevelInt(EndCull);
			Variety.MinLOD = 0;
			Variety.AllowedDensityRange = FFloatInterval(0.0f, 1.0f);
			Variety.Scaling = EGrassScaling::Free;
			Variety.ScaleX = XScale;
			Variety.ScaleY = YScale;
			Variety.ScaleZ = ZScale;
			Variety.RandomRotation = true;
			Variety.AlignToSurface = true;
			Variety.bAlignToTriangleNormals = false;
			Variety.bUseLandscapeLightmap = false;
			Variety.bReceivesDecals = false;
			Variety.bAffectDistanceFieldLighting = false;
			Variety.bCastDynamicShadow = false;
			Variety.bCastContactShadow = false;
			Variety.bKeepInstanceBufferCPUCopy = false;
			Variety.InstanceWorldPositionOffsetDisableDistance = WpoDisableDistance;
		};

		GrassType->Modify();
		GrassType->GrassVarieties.SetNum(2);
		ConfigureVariety(
			GrassType->GrassVarieties[0],
			200.0f,
			FFloatInterval(1.00f, 1.50f),
			FFloatInterval(1.00f, 1.00f),
			FFloatInterval(0.30f, 0.70f),
			2000,
			10000,
			6500);
		ConfigureVariety(
			GrassType->GrassVarieties[1],
			25.0f,
			FFloatInterval(1.00f, 2.00f),
			FFloatInterval(1.00f, 1.00f),
			FFloatInterval(0.50f, 1.20f),
			2000,
			10000,
			7500);
		GrassType->bEnableDensityScaling = true;
		GrassType->PostEditChange();
		GrassType->MarkPackageDirty();
		return GrassType;
	}

	bool IsV84Expression(const UMaterialExpression* Expression)
	{
		if (!Expression)
		{
			return false;
		}
		if (Expression->Desc.Contains(V84NodeToken, ESearchCase::IgnoreCase))
		{
			return true;
		}
		if (const UMaterialExpressionCustom* Custom = Cast<UMaterialExpressionCustom>(Expression))
		{
			return Custom->Description.Contains(V84NodeToken, ESearchCase::IgnoreCase);
		}
		return false;
	}

	void RemoveExistingV84MaterialGraph(UMaterial* Material)
	{
		if (!Material)
		{
			return;
		}

		FExpressionInput* BaseColorInput = Material->GetExpressionInputForProperty(MP_BaseColor);
		if (BaseColorInput)
		{
			if (UMaterialExpressionCustom* ExistingSurface = Cast<UMaterialExpressionCustom>(BaseColorInput->Expression))
			{
				if (ExistingSurface->Description.Contains(V84NodeToken, ESearchCase::IgnoreCase))
				{
					for (const FCustomInput& Input : ExistingSurface->Inputs)
					{
						if (Input.InputName == TEXT("PreviousColor") && Input.Input.Expression)
						{
							BaseColorInput->Expression = Input.Input.Expression;
							BaseColorInput->OutputIndex = Input.Input.OutputIndex;
							break;
						}
					}
				}
			}
		}

		TArray<UMaterialExpression*> ExistingExpressions;
		for (UMaterialExpression* Expression : Material->GetExpressions())
		{
			ExistingExpressions.Add(Expression);
			if (UMaterialExpressionLandscapeGrassOutput* GrassOutput = Cast<UMaterialExpressionLandscapeGrassOutput>(Expression))
			{
				for (int32 InputIndex = GrassOutput->GrassTypes.Num() - 1; InputIndex >= 0; --InputIndex)
				{
					if (GrassOutput->GrassTypes[InputIndex].Name == GrassInputName)
					{
						GrassOutput->GrassTypes.RemoveAt(InputIndex);
					}
				}
			}
		}

		for (UMaterialExpression* Expression : ExistingExpressions)
		{
			if (IsV84Expression(Expression))
			{
				UMaterialEditingLibrary::DeleteMaterialExpression(Material, Expression);
			}
		}
	}

	UMaterialExpressionLandscapeLayerSample* AddLayerSample(
		UMaterial* Material,
		const FName LayerName,
		const int32 X,
		const int32 Y)
	{
		UMaterialExpressionLandscapeLayerSample* Sample = Cast<UMaterialExpressionLandscapeLayerSample>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionLandscapeLayerSample::StaticClass(), X, Y));
		if (Sample)
		{
			Sample->ParameterName = LayerName;
			Sample->PreviewWeight = 0.0f;
			Sample->Desc = V84NodeToken + TEXT(" layer ") + LayerName.ToString();
		}
		return Sample;
	}

	bool AddV84LandscapeSurfaceAndGrass(UMaterial* Material, ULandscapeGrassType* GrassType)
	{
		if (!Material || !GrassType || Material->bUseMaterialAttributes)
		{
			return false;
		}

		Material->Modify();
		RemoveExistingV84MaterialGraph(Material);

		FExpressionInput* PreviousBaseColorInput = Material->GetExpressionInputForProperty(MP_BaseColor);
		if (!PreviousBaseColorInput || !PreviousBaseColorInput->Expression)
		{
			return false;
		}
		UMaterialExpression* PreviousColor = PreviousBaseColorInput->Expression;
		const int32 PreviousColorOutputIndex = PreviousBaseColorInput->OutputIndex;

		UMaterialExpressionWorldPosition* WorldPosition = AddWorldPositionNode(Material, 1850, -520);
		UMaterialExpressionVertexNormalWS* VertexNormal = Cast<UMaterialExpressionVertexNormalWS>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionVertexNormalWS::StaticClass(), 1850, -350));
		UMaterialExpressionLandscapeLayerSample* GrassLight = AddLayerSample(Material, TEXT("Grass_Light"), 1850, -150);
		UMaterialExpressionLandscapeLayerSample* GrassDark = AddLayerSample(Material, TEXT("Grass_Dark"), 1850, -30);
		UMaterialExpressionLandscapeLayerSample* Rock = AddLayerSample(Material, TEXT("Rock_Cliff"), 1850, 90);
		UMaterialExpressionLandscapeLayerSample* Path = AddLayerSample(Material, TEXT("Path_Dirt"), 1850, 210);
		UMaterialExpressionLandscapeLayerSample* Shore = AddLayerSample(Material, TEXT("Shore_WetDirt"), 1850, 330);
		UMaterialExpressionLandscapeLayerSample* Waterbed = AddLayerSample(Material, TEXT("Waterbed_MudStone"), 1850, 450);
		UMaterialExpressionCustom* SurfaceColor = Cast<UMaterialExpressionCustom>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionCustom::StaticClass(), 2250, -360));
		UMaterialExpressionCustom* GrassDensity = Cast<UMaterialExpressionCustom>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionCustom::StaticClass(), 2250, 260));

		if (!WorldPosition || !VertexNormal || !GrassLight || !GrassDark || !Rock || !Path || !Shore || !Waterbed || !SurfaceColor || !GrassDensity)
		{
			return false;
		}
		VertexNormal->Desc = V84NodeToken + TEXT(" surface normal");

		SurfaceColor->Description = V84NodeToken + TEXT(" surface readability preserving V82/V83 base material");
		SurfaceColor->Desc = V84NodeToken + TEXT(" surface readability");
		SurfaceColor->OutputType = CMOT_Float3;
		ConnectCustomInput(SurfaceColor, PreviousColor, TEXT("PreviousColor"), PreviousColorOutputIndex);
		ConnectCustomInput(SurfaceColor, WorldPosition, TEXT("WorldPos"));
		ConnectCustomInput(SurfaceColor, VertexNormal, TEXT("NormalWS"));
		ConnectCustomInput(SurfaceColor, GrassLight, TEXT("GrassLight"));
		ConnectCustomInput(SurfaceColor, GrassDark, TEXT("GrassDark"));
		ConnectCustomInput(SurfaceColor, Rock, TEXT("Rock"));
		ConnectCustomInput(SurfaceColor, Path, TEXT("Path"));
		ConnectCustomInput(SurfaceColor, Shore, TEXT("Shore"));
		ConnectCustomInput(SurfaceColor, Waterbed, TEXT("Waterbed"));
		SurfaceColor->Code = TEXT(R"(
#define DISTSEG(P,A,B) length((P) - ((A) + saturate(dot((P) - (A), (B) - (A)) / max(dot((B) - (A), (B) - (A)), 1.0)) * ((B) - (A))))
#define ELLIPSE(P,C,R) length(((P) - (C)) / (R))
float2 p = WorldPos.xy;
float grassLayer = saturate(max(GrassLight, GrassDark));
float nonGrass = saturate(max(max(Rock, Path), max(Shore, Waterbed)));
float layerEvidence = step(0.001, grassLayer + nonGrass);

float2 centered = (p - float2(71396.0, 79714.0)) / float2(112000.0, 124000.0);
float organic = length(centered)
	+ 0.055 * sin(p.x / 11500.0)
	- 0.045 * cos(p.y / 9000.0)
	+ 0.035 * sin((p.x + p.y) / 17000.0);
float interior = 1.0 - smoothstep(0.68, 0.79, organic);
float flatGround = 1.0 - smoothstep(0.10, 0.30, 1.0 - saturate(NormalWS.z));

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
float water = 1.0 - smoothstep(2300.0, 4300.0, riverDistance);
float2 lake0 = (p - float2(40000.0, 23500.0)) / float2(17000.0, 13000.0);
float2 lake1 = (p - float2(25640.0, 72820.0)) / float2(5400.0, 2600.0);
float2 pondA = (p - float2(45457.0, 136962.0)) / float2(4300.0, 2300.0);
float2 pondB = (p - float2(19220.0, 124250.0)) / float2(3700.0, 2200.0);
float2 pondC = (p - float2(87614.0, 97369.0)) / float2(3600.0, 2200.0);
water = max(water, 1.0 - smoothstep(0.82, 1.16, length(lake0)));
water = max(water, 1.0 - smoothstep(0.82, 1.16, length(lake1)));
water = max(water, 1.0 - smoothstep(0.82, 1.16, length(pondA)));
water = max(water, 1.0 - smoothstep(0.82, 1.16, length(pondB)));
water = max(water, 1.0 - smoothstep(0.82, 1.16, length(pondC)));

float proceduralGrass = interior * flatGround * (1.0 - water);
float grassSurface = lerp(proceduralGrass, grassLayer * (1.0 - nonGrass), layerEvidence);

float broadA = 0.5 + 0.5 * sin(p.x / 47000.0 + p.y / 71000.0 + 0.8 * sin(p.y / 59000.0));
float broadB = 0.5 + 0.5 * sin(-p.x / 64000.0 + p.y / 41000.0 + 1.15);
float mid = 0.5 + 0.5 * sin(p.x / 13300.0 - p.y / 17700.0 + broadB * 2.4);
float macro = saturate(broadA * 0.46 + broadB * 0.34 + mid * 0.20);

float village = 1.0 - smoothstep(0.48, 1.08, ELLIPSE(p, float2(77500.0, 124000.0), float2(42000.0, 29500.0)));
float training = 1.0 - smoothstep(0.48, 1.08, ELLIPSE(p, float2(52300.0, 143400.0), float2(32500.0, 17000.0)));
float openPlains = 1.0 - smoothstep(0.52, 1.18, ELLIPSE(p, float2(72000.0, 104000.0), float2(62000.0, 38500.0)));
float forestBasin = 1.0 - smoothstep(0.48, 1.18, ELLIPSE(p, float2(61000.0, 76000.0), float2(51000.0, 36000.0)));
float shoulder = 1.0 - smoothstep(0.48, 1.10, ELLIPSE(p, float2(93000.0, 137000.0), float2(30000.0, 21000.0)));
float southGorge = 1.0 - smoothstep(0.48, 1.12, ELLIPSE(p, float2(67500.0, 40500.0), float2(33000.0, 27000.0)));

float3 color = PreviousColor;
float3 coolGreen = color * float3(0.90, 1.055, 0.90);
float3 warmGreen = color * float3(1.075, 1.025, 0.83);
float3 richGreen = color * float3(0.84, 1.08, 0.82);
float3 openGreen = color * float3(1.025, 1.06, 0.91);

float3 ecologyColor = lerp(coolGreen, warmGreen, macro);
ecologyColor = lerp(ecologyColor, openGreen, openPlains * 0.15);
ecologyColor = lerp(ecologyColor, richGreen, forestBasin * 0.22);
ecologyColor = lerp(ecologyColor, warmGreen, saturate(village * 0.10 + training * 0.12 + shoulder * 0.12 + southGorge * 0.09));
ecologyColor *= lerp(0.955, 1.055, macro);
float3 meadowCool = float3(0.020, 0.070, 0.010);
float3 meadowWarm = float3(0.050, 0.135, 0.018);
float3 meadowTarget = lerp(meadowCool, meadowWarm, macro);
ecologyColor = lerp(ecologyColor, meadowTarget, 0.82);
color = lerp(color, ecologyColor, saturate(grassSurface * 0.90));

float route = 0.0;
route = max(route, 1.0 - smoothstep(950.0, 2550.0, DISTSEG(p, float2(38550.0, 147850.0), float2(52300.0, 143400.0))));
route = max(route, 1.0 - smoothstep(950.0, 2700.0, DISTSEG(p, float2(52300.0, 143400.0), float2(77500.0, 124000.0))));
route = max(route, 1.0 - smoothstep(1050.0, 3100.0, DISTSEG(p, float2(77500.0, 124000.0), float2(72000.0, 104000.0))));
route = max(route, 1.0 - smoothstep(1050.0, 3300.0, DISTSEG(p, float2(72000.0, 104000.0), float2(61000.0, 76000.0))));
route = max(route, 1.0 - smoothstep(1050.0, 3200.0, DISTSEG(p, float2(61000.0, 76000.0), float2(57584.0, 62136.0))));
route = max(route, 1.0 - smoothstep(1050.0, 3200.0, DISTSEG(p, float2(57584.0, 62136.0), float2(63147.0, 37260.0))));
float pathMask = max(Path, route);
float3 pathReadable = float3(0.145, 0.105, 0.050);
color = lerp(color, pathReadable, saturate(pathMask * 0.30));
float3 wetReadable = float3(0.040, 0.095, 0.045);
color = lerp(color, wetReadable, saturate(max(max(Shore, Waterbed), water) * 0.24));
return color;
#undef DISTSEG
#undef ELLIPSE
)");

		GrassDensity->Description = V84NodeToken + TEXT(" native grass density from painted Highland layers");
		GrassDensity->Desc = V84NodeToken + TEXT(" grass density");
		GrassDensity->OutputType = CMOT_Float1;
		ConnectCustomInput(GrassDensity, WorldPosition, TEXT("WorldPos"));
		ConnectCustomInput(GrassDensity, VertexNormal, TEXT("NormalWS"));
		ConnectCustomInput(GrassDensity, GrassLight, TEXT("GrassLight"));
		ConnectCustomInput(GrassDensity, GrassDark, TEXT("GrassDark"));
		ConnectCustomInput(GrassDensity, Rock, TEXT("Rock"));
		ConnectCustomInput(GrassDensity, Path, TEXT("Path"));
		ConnectCustomInput(GrassDensity, Shore, TEXT("Shore"));
		ConnectCustomInput(GrassDensity, Waterbed, TEXT("Waterbed"));
		GrassDensity->Code = TEXT(R"(
#define DISTSEG(P,A,B) length((P) - ((A) + saturate(dot((P) - (A), (B) - (A)) / max(dot((B) - (A), (B) - (A)), 1.0)) * ((B) - (A))))
#define ELLIPSE(P,C,R) length(((P) - (C)) / (R))
float2 p = WorldPos.xy;
float grassLayer = saturate(max(GrassLight, GrassDark));
float blocked = saturate(max(max(Rock, Path), max(Shore, Waterbed)));
float layerEvidence = step(0.001, grassLayer + blocked);

float2 centered = (p - float2(71396.0, 79714.0)) / float2(112000.0, 124000.0);
float organic = length(centered)
	+ 0.055 * sin(p.x / 11500.0)
	- 0.045 * cos(p.y / 9000.0)
	+ 0.035 * sin((p.x + p.y) / 17000.0);
float interior = 1.0 - smoothstep(0.68, 0.79, organic);
float flatGround = 1.0 - smoothstep(0.08, 0.27, 1.0 - saturate(NormalWS.z));

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
float water = 1.0 - smoothstep(2800.0, 4800.0, riverDistance);
float2 lake0 = (p - float2(40000.0, 23500.0)) / float2(17500.0, 13500.0);
float2 lake1 = (p - float2(25640.0, 72820.0)) / float2(5700.0, 2850.0);
float2 pondA = (p - float2(45457.0, 136962.0)) / float2(4600.0, 2500.0);
float2 pondB = (p - float2(19220.0, 124250.0)) / float2(4000.0, 2400.0);
float2 pondC = (p - float2(87614.0, 97369.0)) / float2(3900.0, 2400.0);
water = max(water, 1.0 - smoothstep(0.80, 1.18, length(lake0)));
water = max(water, 1.0 - smoothstep(0.80, 1.18, length(lake1)));
water = max(water, 1.0 - smoothstep(0.80, 1.18, length(pondA)));
water = max(water, 1.0 - smoothstep(0.80, 1.18, length(pondB)));
water = max(water, 1.0 - smoothstep(0.80, 1.18, length(pondC)));

float route = 0.0;
route = max(route, 1.0 - smoothstep(950.0, 2650.0, DISTSEG(p, float2(38550.0, 147850.0), float2(52300.0, 143400.0))));
route = max(route, 1.0 - smoothstep(950.0, 2800.0, DISTSEG(p, float2(52300.0, 143400.0), float2(77500.0, 124000.0))));
route = max(route, 1.0 - smoothstep(1050.0, 3200.0, DISTSEG(p, float2(77500.0, 124000.0), float2(72000.0, 104000.0))));
route = max(route, 1.0 - smoothstep(1050.0, 3400.0, DISTSEG(p, float2(72000.0, 104000.0), float2(61000.0, 76000.0))));
route = max(route, 1.0 - smoothstep(1050.0, 3300.0, DISTSEG(p, float2(61000.0, 76000.0), float2(57584.0, 62136.0))));
route = max(route, 1.0 - smoothstep(1050.0, 3300.0, DISTSEG(p, float2(57584.0, 62136.0), float2(63147.0, 37260.0))));

float proceduralDensity = interior * flatGround * (1.0 - water) * (1.0 - route);
float paintedDensity = grassLayer * (1.0 - blocked);
float baseDensity = lerp(proceduralDensity, paintedDensity, layerEvidence);

float broadA = 0.5 + 0.5 * sin(p.x / 51000.0 + p.y / 73000.0 + sin(p.y / 61000.0));
float broadB = 0.5 + 0.5 * sin(-p.x / 69000.0 + p.y / 43000.0 + 1.35);
float mid = 0.5 + 0.5 * sin(p.x / 13900.0 - p.y / 19100.0 + broadB * 2.6);
float ecology = saturate(broadA * 0.43 + broadB * 0.34 + mid * 0.23);
float densityVariation = lerp(0.56, 1.0, smoothstep(0.12, 0.88, ecology));

float village = 1.0 - smoothstep(0.52, 1.06, ELLIPSE(p, float2(77500.0, 124000.0), float2(18500.0, 13500.0)));
float training = 1.0 - smoothstep(0.50, 1.06, ELLIPSE(p, float2(52300.0, 143400.0), float2(16000.0, 9000.0)));
float eventA = 1.0 - smoothstep(0.48, 1.08, ELLIPSE(p, float2(58500.0, 131000.0), float2(7500.0, 6000.0)));
float eventB = 1.0 - smoothstep(0.48, 1.08, ELLIPSE(p, float2(71000.0, 80500.0), float2(8200.0, 6600.0)));
float eventC = 1.0 - smoothstep(0.48, 1.08, ELLIPSE(p, float2(99500.0, 132500.0), float2(7200.0, 5600.0)));
float eventD = 1.0 - smoothstep(0.48, 1.08, ELLIPSE(p, float2(73500.0, 47000.0), float2(8000.0, 6200.0)));
float readability = saturate(max(village * 0.40, training * 0.34));
readability = max(readability, max(max(eventA, eventB), max(eventC, eventD)) * 0.24);
return saturate(baseDensity * densityVariation * (1.0 - readability));
#undef DISTSEG
#undef ELLIPSE
)");

		UMaterialExpressionLandscapeGrassOutput* GrassOutput = nullptr;
		for (UMaterialExpression* Expression : Material->GetExpressions())
		{
			GrassOutput = Cast<UMaterialExpressionLandscapeGrassOutput>(Expression);
			if (GrassOutput)
			{
				break;
			}
		}
		if (!GrassOutput)
		{
			GrassOutput = Cast<UMaterialExpressionLandscapeGrassOutput>(
				UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionLandscapeGrassOutput::StaticClass(), 2650, 290));
		}
		if (!GrassOutput)
		{
			return false;
		}

		GrassOutput->Desc = V84NodeToken + TEXT(" native LandscapeGrassOutput");
		for (int32 InputIndex = GrassOutput->GrassTypes.Num() - 1; InputIndex >= 0; --InputIndex)
		{
			if (GrassOutput->GrassTypes[InputIndex].Name == GrassInputName)
			{
				GrassOutput->GrassTypes.RemoveAt(InputIndex);
			}
		}
		FGrassInput GrassInput(GrassInputName);
		GrassInput.GrassType = GrassType;
		GrassInput.Input.Expression = GrassDensity;
		GrassOutput->GrassTypes.Add(GrassInput);

		UMaterialEditingLibrary::ConnectMaterialProperty(SurfaceColor, TEXT(""), MP_BaseColor);
		Material->bUsedWithInstancedStaticMeshes = true;
		FinalizeMaterial(Material);
		return true;
	}

	int32 RemoveHistoricalGrassProofActors(UWorld* World)
	{
		TArray<AActor*> ActorsToRemove;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor
				&& (Actor->ActorHasTag(HistoricalNativePatchTag)
					|| Actor->ActorHasTag(HistoricalABPatchTag)))
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

	bool TraceLandscapeGround(UWorld* World, const FVector2D& XY, FHitResult& OutHit)
	{
		if (!World)
		{
			return false;
		}

		FCollisionQueryParams Params(SCENE_QUERY_STAT(FFV84GroundTrace), true);
		const FVector Start(XY.X, XY.Y, 50000.0f);
		const FVector End(XY.X, XY.Y, -50000.0f);
		if (!World->LineTraceSingleByChannel(OutHit, Start, End, ECC_WorldStatic, Params))
		{
			return false;
		}
		return OutHit.GetActor() && OutHit.GetActor()->IsA<ALandscapeProxy>();
	}

	int32 GroundUnsafePlayerStarts(UWorld* World)
	{
		int32 AdjustedCount = 0;
		for (TActorIterator<APlayerStart> It(World); It; ++It)
		{
			APlayerStart* PlayerStart = *It;
			if (!PlayerStart)
			{
				continue;
			}

			const FVector BeforeLocation = PlayerStart->GetActorLocation();
			FHitResult GroundHit;
			if (!TraceLandscapeGround(World, FVector2D(BeforeLocation.X, BeforeLocation.Y), GroundHit))
			{
				UE_LOG(
					LogTemp,
					Warning,
					TEXT("FFStarterHighlandGroundcoverSurface: PlayerStart ground trace failed at (%.1f,%.1f); leaving actor unchanged."),
					BeforeLocation.X,
					BeforeLocation.Y);
				continue;
			}

			const float SafeZ = GroundHit.ImpactPoint.Z + 110.0f;
			if (BeforeLocation.Z >= GroundHit.ImpactPoint.Z + 5.0f
				&& BeforeLocation.Z <= GroundHit.ImpactPoint.Z + 500.0f)
			{
				continue;
			}

			PlayerStart->Modify();
			PlayerStart->SetActorLocation(FVector(BeforeLocation.X, BeforeLocation.Y, SafeZ));
			PlayerStart->MarkPackageDirty();
			++AdjustedCount;
			UE_LOG(
				LogTemp,
				Display,
				TEXT("FFStarterHighlandGroundcoverSurface: PlayerStart safety correction old=(%.1f,%.1f,%.1f) groundZ=%.1f newZ=%.1f; XY unchanged."),
				BeforeLocation.X,
				BeforeLocation.Y,
				BeforeLocation.Z,
				GroundHit.ImpactPoint.Z,
				SafeZ);
		}
		return AdjustedCount;
	}

	bool FindFlattestLandscapeGround(
		UWorld* World,
		const FVector2D& Center,
		const float Radius,
		const int32 SamplesPerAxis,
		FHitResult& OutHit)
	{
		bool bFound = false;
		float BestNormalZ = -1.0f;
		float BestDistanceSquared = TNumericLimits<float>::Max();
		const int32 SafeSamples = FMath::Max(SamplesPerAxis, 2);

		for (int32 Y = 0; Y < SafeSamples; ++Y)
		{
			for (int32 X = 0; X < SafeSamples; ++X)
			{
				const float AlphaX = static_cast<float>(X) / static_cast<float>(SafeSamples - 1);
				const float AlphaY = static_cast<float>(Y) / static_cast<float>(SafeSamples - 1);
				const FVector2D Candidate(
					Center.X + FMath::Lerp(-Radius, Radius, AlphaX),
					Center.Y + FMath::Lerp(-Radius, Radius, AlphaY));
				FHitResult CandidateHit;
				if (!TraceLandscapeGround(World, Candidate, CandidateHit))
				{
					continue;
				}

				const float NormalZ = CandidateHit.ImpactNormal.Z;
				const float DistanceSquared = FVector2D::DistSquared(Candidate, Center);
				if (!bFound
					|| NormalZ > BestNormalZ + KINDA_SMALL_NUMBER
					|| (FMath::IsNearlyEqual(NormalZ, BestNormalZ, KINDA_SMALL_NUMBER)
						&& DistanceSquared < BestDistanceSquared))
				{
					bFound = true;
					BestNormalZ = NormalZ;
					BestDistanceSquared = DistanceSquared;
					OutHit = CandidateHit;
				}
			}
		}
		return bFound;
	}

	void RemoveV84ValidationActors(UWorld* World)
	{
		TArray<AActor*> ActorsToRemove;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && Actor->ActorHasTag(ValidationActorTag))
			{
				ActorsToRemove.Add(Actor);
			}
		}
		for (AActor* Actor : ActorsToRemove)
		{
			World->DestroyActor(Actor);
		}
	}

	bool SpawnValidationCamera(
		UWorld* World,
		const FString& Label,
		const FName CameraTag,
		const FVector& Location,
		const FVector& Target,
		const float FieldOfView)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ACameraActor* Camera = World->SpawnActor<ACameraActor>(
			ACameraActor::StaticClass(),
			Location,
			(Target - Location).Rotation(),
			SpawnParameters);
		if (!Camera)
		{
			return false;
		}

		Camera->Modify();
		Camera->Tags.AddUnique(ValidationActorTag);
		Camera->Tags.AddUnique(CameraTag);
		Camera->SetActorLabel(Label);
		if (UCameraComponent* CameraComponent = Camera->GetCameraComponent())
		{
			CameraComponent->ProjectionMode = ECameraProjectionMode::Perspective;
			CameraComponent->FieldOfView = FieldOfView;
		}
		Camera->MarkPackageDirty();
		return true;
	}

	int32 AddV84ValidationCameras(UWorld* World)
	{
		RemoveV84ValidationActors(World);

		FHitResult CloseTargetHit;
		FHitResult CloseCameraHit;
		FHitResult DistanceCameraHit;
		if (!FindFlattestLandscapeGround(
				World,
				FVector2D(87500.0f, 108000.0f),
				9000.0f,
				9,
				CloseTargetHit))
		{
			return 0;
		}

		const FVector2D CloseXY(CloseTargetHit.ImpactPoint.X, CloseTargetHit.ImpactPoint.Y);
		const FVector2D CloseCameraXY = CloseXY + FVector2D(-1450.0f, -1250.0f);
		const FVector2D DistanceCameraXY = CloseXY + FVector2D(-7200.0f, 4300.0f);
		if (!TraceLandscapeGround(World, CloseCameraXY, CloseCameraHit)
			|| !TraceLandscapeGround(World, DistanceCameraXY, DistanceCameraHit))
		{
			return 0;
		}

		const FVector CloseTarget = CloseTargetHit.ImpactPoint + FVector(0.0f, 0.0f, 90.0f);
		const FVector CloseLocation = CloseCameraHit.ImpactPoint + FVector(0.0f, 0.0f, 285.0f);
		const FVector DistanceTarget = CloseTargetHit.ImpactPoint + FVector(850.0f, -350.0f, 150.0f);
		const FVector DistanceLocation = DistanceCameraHit.ImpactPoint + FVector(0.0f, 0.0f, 1450.0f);
		UE_LOG(
			LogTemp,
			Display,
			TEXT("FFStarterHighlandGroundcoverSurface: validation meadow target=(%.1f,%.1f,%.1f) normalZ=%.3f"),
			CloseTargetHit.ImpactPoint.X,
			CloseTargetHit.ImpactPoint.Y,
			CloseTargetHit.ImpactPoint.Z,
			CloseTargetHit.ImpactNormal.Z);

		int32 CameraCount = 0;
		CameraCount += SpawnValidationCamera(
			World,
			TEXT("V84_Grass_Close"),
			TEXT("FFSmokeHighlandV84GrassCloseCamera"),
			CloseLocation,
			CloseTarget,
			42.0f) ? 1 : 0;
		CameraCount += SpawnValidationCamera(
			World,
			TEXT("V84_Grass_Distance"),
			TEXT("FFSmokeHighlandV84GrassDistanceCamera"),
			DistanceLocation,
			DistanceTarget,
			48.0f) ? 1 : 0;
		return CameraCount;
	}

	void RefreshLandscapeGrass(ALandscapeProxy* Landscape)
	{
		if (!Landscape)
		{
			return;
		}

		Landscape->Modify();
		for (ULandscapeComponent* Component : Landscape->LandscapeComponents)
		{
			if (!Component)
			{
				continue;
			}
			Component->Modify();
			Component->UpdateMaterialInstances();
			Component->UpdateGrassTypes(true);
			Component->MarkPackageDirty();
		}
		Landscape->InvalidateGeneratedComponentData(false);
		Landscape->FlushGrassComponents(nullptr, true);
		Landscape->PostEditChange();
		Landscape->MarkPackageDirty();
	}
}
#endif

int32 UFFStarterHighlandGroundcoverSurfaceCommandlet::Main(const FString& Params)
{
#if WITH_EDITOR
	const bool bForce = FParse::Param(*Params, TEXT("Force"));
	UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandGroundcoverSurface: start force=%s"), bForce ? TEXT("true") : TEXT("false"));

	UWorld* World = UEditorLoadingAndSavingUtils::LoadMap(HighlandMapPath);
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandGroundcoverSurface: failed to load Highland map."));
		return 1;
	}

	ALandscapeProxy* PrimaryLandscape = nullptr;
	for (TActorIterator<ALandscapeProxy> It(World); It; ++It)
	{
		if (!PrimaryLandscape || It->LandscapeComponents.Num() > PrimaryLandscape->LandscapeComponents.Num())
		{
			PrimaryLandscape = *It;
		}
	}
	if (!PrimaryLandscape)
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandGroundcoverSurface: no Highland landscape found."));
		return 1;
	}

	if (PrimaryLandscape->ActorHasTag(AppliedTag) && !bForce)
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandGroundcoverSurface: V84 is already applied. Use -Force for an idempotent refinement."));
		return 1;
	}

	UMaterial* LandscapeMaterial = LoadObject<UMaterial>(nullptr, LandscapeMaterialPath);
	UMaterial* GroundcoverMaterial = CreateOrLoadGroundcoverMaterial();
	ULandscapeGrassType* GrassType = CreateOrLoadGroundcoverGrassType(GroundcoverMaterial);
	if (!LandscapeMaterial || !GroundcoverMaterial || !GrassType)
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandGroundcoverSurface: required material, grass type, or Titan grass mesh missing."));
		return 1;
	}

	const int32 GroundedPlayerStarts = GroundUnsafePlayerStarts(World);
	const FProtectedState Before = CaptureProtectedState(World);
	const int32 HistoricalProofActorsRemoved = RemoveHistoricalGrassProofActors(World);
	const bool bMaterialUpdated = AddV84LandscapeSurfaceAndGrass(LandscapeMaterial, GrassType);
	if (!bMaterialUpdated)
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandGroundcoverSurface: landscape material update failed."));
		return 1;
	}

	int32 LandscapeCount = 0;
	int32 ComponentCount = 0;
	for (TActorIterator<ALandscapeProxy> It(World); It; ++It)
	{
		++LandscapeCount;
		ComponentCount += It->LandscapeComponents.Num();
		RefreshLandscapeGrass(*It);
	}

	const int32 ValidationCameras = AddV84ValidationCameras(World);
	PrimaryLandscape->Modify();
	PrimaryLandscape->Tags.AddUnique(AppliedTag);
	PrimaryLandscape->MarkPackageDirty();

	const FProtectedState After = CaptureProtectedState(World);
	const bool bProtectedStateUnchanged = ProtectedStateMatches(Before, After);
	if (!bProtectedStateUnchanged)
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandGroundcoverSurface: protected-state mismatch; refusing to save."));
		return 1;
	}

	World->MarkPackageDirty();
	const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
	const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);

	UE_LOG(LogTemp, Display,
		TEXT("FFStarterHighlandGroundcoverSurface: landscapes=%d components=%d proofActorsRemoved=%d validationCameras=%d"),
		LandscapeCount,
		ComponentCount,
		HistoricalProofActorsRemoved,
		ValidationCameras);
	UE_LOG(LogTemp, Display,
		TEXT("FFStarterHighlandGroundcoverSurface: material=%s grassType=%s mesh=%s varieties=%d densities=(%.1f,%.1f)"),
		*GroundcoverMaterial->GetPathName(),
		*GrassType->GetPathName(),
		TitanGrassMeshPath,
		GrassType->GrassVarieties.Num(),
		GrassType->GrassVarieties.IsValidIndex(0) ? GrassType->GrassVarieties[0].GrassDensity.GetValue() : 0.0f,
		GrassType->GrassVarieties.IsValidIndex(1) ? GrassType->GrassVarieties[1].GrassDensity.GetValue() : 0.0f);
	UE_LOG(LogTemp, Display,
		TEXT("FFStarterHighlandGroundcoverSurface: protected playerStarts=%d waterActors=%d mountainActors=%d unchanged=%s"),
		After.PlayerStarts.Num(),
		After.WaterActors.Num(),
		After.MountainActors,
		bProtectedStateUnchanged ? TEXT("true") : TEXT("false"));
	UE_LOG(
		LogTemp,
		Display,
		TEXT("FFStarterHighlandGroundcoverSurface: PlayerStart safety corrections=%d (vertical-only; XY unchanged)"),
		GroundedPlayerStarts);
	UE_LOG(LogTemp, Display,
		TEXT("FFStarterHighlandGroundcoverSurface: savedMap=%s savedPackages=%s"),
		bSavedMap ? TEXT("true") : TEXT("false"),
		bSavedPackages ? TEXT("true") : TEXT("false"));

	return (bSavedMap && bSavedPackages) ? 0 : 1;
#else
	UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandGroundcoverSurface can only run in editor builds."));
	return 1;
#endif
}
