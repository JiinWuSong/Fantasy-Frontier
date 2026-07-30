#include "FFStarterHighlandGrassNaturalizationCommandlet.h"

#if WITH_EDITOR
#include "AssetRegistry/AssetRegistryModule.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
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
#include "Materials/MaterialExpressionLocalPosition.h"
#include "Materials/MaterialExpressionPerInstanceRandom.h"
#include "Materials/MaterialExpressionQualitySwitch.h"
#include "Materials/MaterialExpressionTime.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionWorldPosition.h"
#include "Materials/MaterialInterface.h"
#include "Misc/Parse.h"
#include "PerQualityLevelProperties.h"
#include "UObject/Package.h"
#endif

UFFStarterHighlandGrassNaturalizationCommandlet::UFFStarterHighlandGrassNaturalizationCommandlet()
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
	const TCHAR* V84BladeMaterialPath = TEXT("/Game/FantasyFrontier/Blockout/Materials/M_FF_Highland_GrassBlade_V84.M_FF_Highland_GrassBlade_V84");

	const TCHAR* V841BladeMaterialPackagePath = TEXT("/Game/FantasyFrontier/Blockout/Materials/M_FF_Highland_GrassBlade_V841");
	const TCHAR* V841BladeMaterialObjectPath = TEXT("/Game/FantasyFrontier/Blockout/Materials/M_FF_Highland_GrassBlade_V841.M_FF_Highland_GrassBlade_V841");
	const TCHAR* V841RyegrassMaterialPackagePath = TEXT("/Game/FantasyFrontier/Blockout/Materials/M_FF_Highland_Ryegrass_V841");
	const TCHAR* V841RyegrassMaterialObjectPath = TEXT("/Game/FantasyFrontier/Blockout/Materials/M_FF_Highland_Ryegrass_V841.M_FF_Highland_Ryegrass_V841");
	const TCHAR* FillerGrassTypePackagePath = TEXT("/Game/FantasyFrontier/Blockout/Materials/GT_FF_Highland_Grass_Filler_V841");
	const TCHAR* FillerGrassTypeObjectPath = TEXT("/Game/FantasyFrontier/Blockout/Materials/GT_FF_Highland_Grass_Filler_V841.GT_FF_Highland_Grass_Filler_V841");
	const TCHAR* MediumGrassTypePackagePath = TEXT("/Game/FantasyFrontier/Blockout/Materials/GT_FF_Highland_Grass_Medium_V841");
	const TCHAR* MediumGrassTypeObjectPath = TEXT("/Game/FantasyFrontier/Blockout/Materials/GT_FF_Highland_Grass_Medium_V841.GT_FF_Highland_Grass_Medium_V841");
	const TCHAR* TallGrassTypePackagePath = TEXT("/Game/FantasyFrontier/Blockout/Materials/GT_FF_Highland_Grass_Tall_V841");
	const TCHAR* TallGrassTypeObjectPath = TEXT("/Game/FantasyFrontier/Blockout/Materials/GT_FF_Highland_Grass_Tall_V841.GT_FF_Highland_Grass_Tall_V841");

	const TCHAR* BladeMeshPath = TEXT("/Game/Environment/Foliage/Meshes/SM_GrassBlade.SM_GrassBlade");
	const TCHAR* RyegrassSoftMeshPath = TEXT("/Game/Environment/Foliage/Grass/Ryegrass_Grass_D.Ryegrass_Grass_D");
	const TCHAR* RyegrassTuftMeshPath = TEXT("/Game/Environment/Foliage/Grass/Ryegrass_Rye_Tuft.Ryegrass_Rye_Tuft");
	const TCHAR* RyegrassStalkMeshPath = TEXT("/Game/Environment/Foliage/Grass/Ryegrass_Stalk_Green.Ryegrass_Stalk_Green");
	const TCHAR* RyegrassMaterialPath = TEXT("/Game/Environment/Clifftop/Materials/Foliage/MI_Clifftop_Ryegrass.MI_Clifftop_Ryegrass");
	const TCHAR* RyegrassAlbedoPath = TEXT("/Game/Environment/Clifftop/Textures/Foliage/T_Ryegrass_D.T_Ryegrass_D");

	const FName LookdevActorTag(TEXT("FFHighlandV841GrassLookdev"));
	const FName FinalCameraTag(TEXT("FFHighlandV841GrassValidation"));
	const FName LightingTag(TEXT("FFHighlandV841GrassLighting"));
	const FName AppliedTag(TEXT("FFHighlandGrassNaturalizationV841Applied"));
	const FName V84GrassInputName(TEXT("FF_V84_TitanGroundcover"));
	const FName FillerInputName(TEXT("FF_V841_Grass_Filler"));
	const FName MediumInputName(TEXT("FF_V841_Grass_Medium"));
	const FName TallInputName(TEXT("FF_V841_Grass_Tall"));
	const FString V841NodeToken(TEXT("FF V84.1"));

	struct FProtectedState
	{
		TArray<FTransform> PlayerStarts;
		TArray<FString> WaterActors;
		int32 MountainActors = 0;
		uint32 MountainTransformHash = 0;
	};

	struct FLookdevStats
	{
		int32 CandidateAInstances = 0;
		int32 CandidateBInstances = 0;
		int32 CandidateCInstances = 0;
		int32 Cameras = 0;
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

		State.PlayerStarts.Sort([](const FTransform& A, const FTransform& B)
		{
			return A.GetLocation().ToString() < B.GetLocation().ToString();
		});
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

	UMaterialExpressionWorldPosition* AddWorldPositionNode(
		UMaterial* Material,
		const int32 X,
		const int32 Y)
	{
		UMaterialExpressionWorldPosition* WorldPosition = Cast<UMaterialExpressionWorldPosition>(
			UMaterialEditingLibrary::CreateMaterialExpression(
				Material,
				UMaterialExpressionWorldPosition::StaticClass(),
				X,
				Y));
		if (WorldPosition)
		{
			WorldPosition->WorldPositionShaderOffset = WPT_ExcludeAllShaderOffsets;
			WorldPosition->Desc = V841NodeToken + TEXT(" world position");
		}
		return WorldPosition;
	}

	UMaterial* CreateOrLoadNaturalizedBladeMaterial()
	{
		UMaterial* Material = LoadObject<UMaterial>(nullptr, V841BladeMaterialObjectPath);
		if (!Material)
		{
			UPackage* Package = CreatePackage(V841BladeMaterialPackagePath);
			if (!Package)
			{
				return nullptr;
			}
			Material = NewObject<UMaterial>(
				Package,
				TEXT("M_FF_Highland_GrassBlade_V841"),
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

		UMaterialExpressionWorldPosition* WorldPosition = AddWorldPositionNode(Material, -960, -250);
		UMaterialExpressionLocalPosition* LocalPosition = Cast<UMaterialExpressionLocalPosition>(
			UMaterialEditingLibrary::CreateMaterialExpression(
				Material,
				UMaterialExpressionLocalPosition::StaticClass(),
				-960,
				-90));
		UMaterialExpressionPerInstanceRandom* InstanceRandom = Cast<UMaterialExpressionPerInstanceRandom>(
			UMaterialEditingLibrary::CreateMaterialExpression(
				Material,
				UMaterialExpressionPerInstanceRandom::StaticClass(),
				-960,
				80));
		UMaterialExpressionTime* Time = Cast<UMaterialExpressionTime>(
			UMaterialEditingLibrary::CreateMaterialExpression(
				Material,
				UMaterialExpressionTime::StaticClass(),
				-960,
				250));
		UMaterialExpressionCustom* Color = Cast<UMaterialExpressionCustom>(
			UMaterialEditingLibrary::CreateMaterialExpression(
				Material,
				UMaterialExpressionCustom::StaticClass(),
				-600,
				-190));
		UMaterialExpressionCustom* Subsurface = Cast<UMaterialExpressionCustom>(
			UMaterialEditingLibrary::CreateMaterialExpression(
				Material,
				UMaterialExpressionCustom::StaticClass(),
				-230,
				-65));
		UMaterialExpressionCustom* WindLow = Cast<UMaterialExpressionCustom>(
			UMaterialEditingLibrary::CreateMaterialExpression(
				Material,
				UMaterialExpressionCustom::StaticClass(),
				-600,
				145));
		UMaterialExpressionCustom* WindHigh = Cast<UMaterialExpressionCustom>(
			UMaterialEditingLibrary::CreateMaterialExpression(
				Material,
				UMaterialExpressionCustom::StaticClass(),
				-600,
				315));
		UMaterialExpressionQualitySwitch* WindQuality = Cast<UMaterialExpressionQualitySwitch>(
			UMaterialEditingLibrary::CreateMaterialExpression(
				Material,
				UMaterialExpressionQualitySwitch::StaticClass(),
				-230,
				265));
		UMaterialExpressionConstant* Roughness = Cast<UMaterialExpressionConstant>(
			UMaterialEditingLibrary::CreateMaterialExpression(
				Material,
				UMaterialExpressionConstant::StaticClass(),
				-230,
				80));
		UMaterialExpressionConstant* Specular = Cast<UMaterialExpressionConstant>(
			UMaterialEditingLibrary::CreateMaterialExpression(
				Material,
				UMaterialExpressionConstant::StaticClass(),
				-230,
				160));
		UMaterialExpressionConstant* AmbientOcclusion = Cast<UMaterialExpressionConstant>(
			UMaterialEditingLibrary::CreateMaterialExpression(
				Material,
				UMaterialExpressionConstant::StaticClass(),
				-230,
				-145));

		if (!WorldPosition || !LocalPosition || !InstanceRandom || !Time || !Color
			|| !Subsurface || !WindLow || !WindHigh || !WindQuality
			|| !Roughness || !Specular || !AmbientOcclusion)
		{
			return nullptr;
		}

		LocalPosition->Desc = V841NodeToken + TEXT(" local blade position");
		InstanceRandom->Desc = V841NodeToken + TEXT(" per-instance phase and color");
		Time->Desc = V841NodeToken + TEXT(" wind time");

		Color->Description = V841NodeToken + TEXT(" restrained meadow blade color");
		Color->Desc = V841NodeToken + TEXT(" naturalized blade color");
		Color->OutputType = CMOT_Float3;
		ConnectCustomInput(Color, WorldPosition, TEXT("WorldPos"));
		ConnectCustomInput(Color, LocalPosition, TEXT("LocalPos"));
		ConnectCustomInput(Color, InstanceRandom, TEXT("InstanceRandom"));
		Color->Code = TEXT(R"(
float2 p = WorldPos.xy;
float rootToTip = smoothstep(-7.0, 82.0, LocalPos.z);
float broadA = 0.5 + 0.5 * sin(p.x / 44000.0 + p.y / 62000.0 + 0.35 * sin(p.y / 17000.0));
float broadB = 0.5 + 0.5 * sin(-p.x / 33000.0 + p.y / 51000.0 + 1.73);
float localBreakup = 0.5 + 0.5 * sin(p.x / 7200.0 - p.y / 9600.0 + InstanceRandom * 6.28318);
float ecology = saturate(broadA * 0.46 + broadB * 0.31 + localBreakup * 0.23);

float3 deepGreen = float3(0.0180, 0.0550, 0.0080);
float3 softGreen = float3(0.0380, 0.1150, 0.0210);
float3 warmGreen = float3(0.0600, 0.1450, 0.0280);
float3 coolGreen = float3(0.0280, 0.0950, 0.0320);
float3 blade = lerp(deepGreen, softGreen, saturate(0.28 + ecology * 0.68));
blade = lerp(blade, warmGreen, saturate((broadB - 0.58) * 0.48));
blade = lerp(blade, coolGreen, saturate((broadA - 0.62) * 0.32));
float3 root = blade * float3(0.72, 0.78, 0.65);
float3 tip = lerp(blade, warmGreen, saturate(0.025 + localBreakup * 0.055));
float3 color = lerp(root, tip, saturate(rootToTip * 0.68));
return color * lerp(0.90, 1.035, InstanceRandom);
)");

		Subsurface->Description = V841NodeToken + TEXT(" low-energy foliage transmission");
		Subsurface->Desc = V841NodeToken + TEXT(" restrained subsurface");
		Subsurface->OutputType = CMOT_Float3;
		ConnectCustomInput(Subsurface, Color, TEXT("BladeColor"));
		Subsurface->Code = TEXT(
			"return saturate(BladeColor * 0.95 + float3(0.012, 0.040, 0.008));");

		auto ConfigureWindInputs = [&](UMaterialExpressionCustom* Wind)
		{
			Wind->OutputType = CMOT_Float3;
			ConnectCustomInput(Wind, WorldPosition, TEXT("WorldPos"));
			ConnectCustomInput(Wind, LocalPosition, TEXT("LocalPos"));
			ConnectCustomInput(Wind, Time, TEXT("Time"));
			ConnectCustomInput(Wind, InstanceRandom, TEXT("InstanceRandom"));
		};

		WindLow->Description = V841NodeToken + TEXT(" LOW quality anchored lean and single sway");
		WindLow->Desc = V841NodeToken + TEXT(" low wind");
		ConfigureWindInputs(WindLow);
		WindLow->Code = TEXT(R"(
float h = saturate((LocalPos.z + 8.0) / 90.0);
float rootMask = smoothstep(0.08, 0.38, h);
float upper = smoothstep(0.26, 1.0, h);
rootMask *= rootMask;
upper *= upper;
float phase = InstanceRandom * 6.28318;
float angle = phase + 0.55;
float2 leanDir = float2(cos(angle), sin(angle));
float2 staticCurve = leanDir * lerp(2.5, 7.0, InstanceRandom) * upper;
float sway = sin(Time * 0.62 + phase + dot(WorldPos.xy, float2(0.0011, 0.0007)));
float2 windDir = normalize(float2(0.82, 0.36));
float2 moving = windDir * sway * lerp(1.1, 2.8, InstanceRandom) * rootMask;
return float3(staticCurve + moving, -0.18 * length(staticCurve) * upper);
)");

		WindHigh->Description = V841NodeToken + TEXT(" HIGH quality anchored curve, gust, sway and tip flutter");
		WindHigh->Desc = V841NodeToken + TEXT(" high wind");
		ConfigureWindInputs(WindHigh);
		WindHigh->Code = TEXT(R"(
float h = saturate((LocalPos.z + 8.0) / 90.0);
float rootMask = smoothstep(0.07, 0.34, h);
float upper = smoothstep(0.24, 1.0, h);
float tip = smoothstep(0.58, 1.0, h);
rootMask *= rootMask;
upper *= upper;
tip *= tip;
float phase = InstanceRandom * 6.28318;
float angle = phase + 0.55;
float2 leanDir = float2(cos(angle), sin(angle));
float2 staticCurve = leanDir * lerp(3.0, 8.5, InstanceRandom) * upper;

float2 windDir = normalize(float2(0.82, 0.36));
float lowFrequency = sin(Time * 0.66 + phase + dot(WorldPos.xy, float2(0.00105, 0.00072)));
float gustField = 0.72 + 0.28 * sin(Time * 0.23 + dot(WorldPos.xy, float2(-0.00022, 0.00031)) + phase * 0.31);
float flutter = sin(Time * 2.15 + phase * 1.73 + dot(WorldPos.xy, float2(0.0052, -0.0036)));
float cross = sin(Time * 0.93 + phase * 0.61 + dot(WorldPos.xy, float2(-0.0016, 0.0011)));
float2 moving = windDir * lowFrequency * gustField * lerp(1.6, 4.2, InstanceRandom) * rootMask;
moving += float2(-windDir.y, windDir.x) * cross * 0.85 * upper;
moving += windDir * flutter * 0.72 * tip;
return float3(staticCurve + moving, -0.20 * length(staticCurve) * upper);
)");

		WindQuality->Desc = V841NodeToken + TEXT(" material quality wind switch");
		WindQuality->Default.Expression = WindHigh;
		WindQuality->Inputs[static_cast<int32>(EMaterialQualityLevel::Low)].Expression = WindLow;
		WindQuality->Inputs[static_cast<int32>(EMaterialQualityLevel::High)].Expression = WindHigh;
		WindQuality->Inputs[static_cast<int32>(EMaterialQualityLevel::Medium)].Expression = WindHigh;
		WindQuality->Inputs[static_cast<int32>(EMaterialQualityLevel::Epic)].Expression = WindHigh;

		Roughness->R = 0.97f;
		Roughness->Desc = V841NodeToken + TEXT(" matte grass roughness");
		Specular->R = 0.0f;
		Specular->Desc = V841NodeToken + TEXT(" zero grass specular");
		AmbientOcclusion->R = 0.96f;
		AmbientOcclusion->Desc = V841NodeToken + TEXT(" blade ambient occlusion");

		UMaterialEditingLibrary::ConnectMaterialProperty(Color, TEXT(""), MP_BaseColor);
		UMaterialEditingLibrary::ConnectMaterialProperty(Subsurface, TEXT(""), MP_SubsurfaceColor);
		UMaterialEditingLibrary::ConnectMaterialProperty(WindQuality, TEXT(""), MP_WorldPositionOffset);
		UMaterialEditingLibrary::ConnectMaterialProperty(Roughness, TEXT(""), MP_Roughness);
		UMaterialEditingLibrary::ConnectMaterialProperty(Specular, TEXT(""), MP_Specular);
		UMaterialEditingLibrary::ConnectMaterialProperty(AmbientOcclusion, TEXT(""), MP_AmbientOcclusion);
		FinalizeMaterial(Material);
		return Material;
	}

	UMaterial* CreateOrLoadNaturalizedRyegrassMaterial()
	{
		UTexture2D* Albedo = LoadObject<UTexture2D>(nullptr, RyegrassAlbedoPath);
		if (!Albedo)
		{
			return nullptr;
		}

		UMaterial* Material = LoadObject<UMaterial>(nullptr, V841RyegrassMaterialObjectPath);
		if (!Material)
		{
			UPackage* Package = CreatePackage(V841RyegrassMaterialPackagePath);
			if (!Package)
			{
				return nullptr;
			}
			Material = NewObject<UMaterial>(
				Package,
				TEXT("M_FF_Highland_Ryegrass_V841"),
				RF_Public | RF_Standalone | RF_Transactional);
			FAssetRegistryModule::AssetCreated(Material);
		}

		Material->Modify();
		Material->PreEditChange(nullptr);
		UMaterialEditingLibrary::DeleteAllMaterialExpressions(Material);
		Material->bUseMaterialAttributes = false;
		Material->BlendMode = BLEND_Masked;
		Material->OpacityMaskClipValue = 0.32f;
		Material->TwoSided = true;
		Material->SetShadingModel(MSM_TwoSidedFoliage);
		Material->bUsedWithInstancedStaticMeshes = true;
		Material->bUsedWithNanite = true;

		UMaterialExpressionWorldPosition* WorldPosition = AddWorldPositionNode(Material, -1080, -300);
		UMaterialExpressionLocalPosition* LocalPosition = Cast<UMaterialExpressionLocalPosition>(
			UMaterialEditingLibrary::CreateMaterialExpression(
				Material,
				UMaterialExpressionLocalPosition::StaticClass(),
				-1080,
				-120));
		UMaterialExpressionPerInstanceRandom* InstanceRandom = Cast<UMaterialExpressionPerInstanceRandom>(
			UMaterialEditingLibrary::CreateMaterialExpression(
				Material,
				UMaterialExpressionPerInstanceRandom::StaticClass(),
				-1080,
				60));
		UMaterialExpressionTime* Time = Cast<UMaterialExpressionTime>(
			UMaterialEditingLibrary::CreateMaterialExpression(
				Material,
				UMaterialExpressionTime::StaticClass(),
				-1080,
				240));
		UMaterialExpressionTextureSample* TextureSample = Cast<UMaterialExpressionTextureSample>(
			UMaterialEditingLibrary::CreateMaterialExpression(
				Material,
				UMaterialExpressionTextureSample::StaticClass(),
				-1080,
				-470));
		UMaterialExpressionCustom* Color = Cast<UMaterialExpressionCustom>(
			UMaterialEditingLibrary::CreateMaterialExpression(
				Material,
				UMaterialExpressionCustom::StaticClass(),
				-700,
				-270));
		UMaterialExpressionCustom* Subsurface = Cast<UMaterialExpressionCustom>(
			UMaterialEditingLibrary::CreateMaterialExpression(
				Material,
				UMaterialExpressionCustom::StaticClass(),
				-290,
				-120));
		UMaterialExpressionCustom* WindLow = Cast<UMaterialExpressionCustom>(
			UMaterialEditingLibrary::CreateMaterialExpression(
				Material,
				UMaterialExpressionCustom::StaticClass(),
				-700,
				150));
		UMaterialExpressionCustom* WindHigh = Cast<UMaterialExpressionCustom>(
			UMaterialEditingLibrary::CreateMaterialExpression(
				Material,
				UMaterialExpressionCustom::StaticClass(),
				-700,
				330));
		UMaterialExpressionQualitySwitch* WindQuality = Cast<UMaterialExpressionQualitySwitch>(
			UMaterialEditingLibrary::CreateMaterialExpression(
				Material,
				UMaterialExpressionQualitySwitch::StaticClass(),
				-290,
				270));
		UMaterialExpressionConstant* Roughness = Cast<UMaterialExpressionConstant>(
			UMaterialEditingLibrary::CreateMaterialExpression(
				Material,
				UMaterialExpressionConstant::StaticClass(),
				-290,
				20));
		UMaterialExpressionConstant* Specular = Cast<UMaterialExpressionConstant>(
			UMaterialEditingLibrary::CreateMaterialExpression(
				Material,
				UMaterialExpressionConstant::StaticClass(),
				-290,
				100));
		UMaterialExpressionConstant* AmbientOcclusion = Cast<UMaterialExpressionConstant>(
			UMaterialEditingLibrary::CreateMaterialExpression(
				Material,
				UMaterialExpressionConstant::StaticClass(),
				-290,
				-210));

		if (!WorldPosition || !LocalPosition || !InstanceRandom || !Time || !TextureSample
			|| !Color || !Subsurface || !WindLow || !WindHigh || !WindQuality
			|| !Roughness || !Specular || !AmbientOcclusion)
		{
			return nullptr;
		}

		LocalPosition->Desc = V841NodeToken + TEXT(" ryegrass local position");
		InstanceRandom->Desc = V841NodeToken + TEXT(" ryegrass per-instance phase");
		Time->Desc = V841NodeToken + TEXT(" ryegrass wind time");
		TextureSample->Texture = Albedo;
		TextureSample->SamplerType = SAMPLERTYPE_VirtualColor;
		TextureSample->Desc = V841NodeToken + TEXT(" existing Titan ryegrass mask texture");

		Color->Description = V841NodeToken + TEXT(" adapted ryegrass color without white clipping");
		Color->Desc = V841NodeToken + TEXT(" adapted ryegrass color");
		Color->OutputType = CMOT_Float3;
		ConnectCustomInput(Color, TextureSample, TEXT("TexColor"));
		ConnectCustomInput(Color, WorldPosition, TEXT("WorldPos"));
		ConnectCustomInput(Color, LocalPosition, TEXT("LocalPos"));
		ConnectCustomInput(Color, InstanceRandom, TEXT("InstanceRandom"));
		Color->Code = TEXT(R"(
float luminance = dot(saturate(TexColor.rgb), float3(0.299, 0.587, 0.114));
float h = saturate((LocalPos.z + 4.0) / 88.0);
float broad = 0.5 + 0.5 * sin(WorldPos.x / 31000.0 - WorldPos.y / 47000.0 + 1.21);
float localBreakup = 0.5 + 0.5 * sin(WorldPos.x / 6100.0 + WorldPos.y / 8400.0 + InstanceRandom * 6.28318);
float3 deepGreen = float3(0.0160, 0.0480, 0.0070);
float3 meadowGreen = float3(0.0350, 0.1050, 0.0190);
float3 warmGreen = float3(0.0600, 0.1450, 0.0260);
float3 coolGreen = float3(0.0250, 0.0900, 0.0300);
float3 blade = lerp(deepGreen, meadowGreen, saturate(0.24 + luminance * 0.56 + localBreakup * 0.14));
blade = lerp(blade, warmGreen, saturate((broad - 0.58) * 0.24 + h * 0.10));
blade = lerp(blade, coolGreen, saturate((0.42 - broad) * 0.16));
blade *= lerp(0.86, 1.04, InstanceRandom);
return lerp(blade * 0.82, blade, smoothstep(0.02, 0.72, h));
)");

		Subsurface->Description = V841NodeToken + TEXT(" controlled ryegrass transmission");
		Subsurface->Desc = V841NodeToken + TEXT(" ryegrass subsurface");
		Subsurface->OutputType = CMOT_Float3;
		ConnectCustomInput(Subsurface, Color, TEXT("BladeColor"));
		Subsurface->Code = TEXT(
			"return saturate(BladeColor + float3(0.014, 0.045, 0.009));");

		auto ConfigureWindInputs = [&](UMaterialExpressionCustom* Wind)
		{
			Wind->OutputType = CMOT_Float3;
			ConnectCustomInput(Wind, WorldPosition, TEXT("WorldPos"));
			ConnectCustomInput(Wind, LocalPosition, TEXT("LocalPos"));
			ConnectCustomInput(Wind, Time, TEXT("Time"));
			ConnectCustomInput(Wind, InstanceRandom, TEXT("InstanceRandom"));
		};

		WindLow->Description = V841NodeToken + TEXT(" ryegrass LOW anchored sway");
		WindLow->Desc = V841NodeToken + TEXT(" ryegrass low wind");
		ConfigureWindInputs(WindLow);
		WindLow->Code = TEXT(R"(
float h = saturate((LocalPos.z + 3.0) / 78.0);
float anchored = smoothstep(0.09, 0.42, h);
float upper = smoothstep(0.32, 1.0, h);
anchored *= anchored;
upper *= upper;
float phase = InstanceRandom * 6.28318;
float2 restDir = float2(cos(phase + 0.45), sin(phase + 0.45));
float2 curve = restDir * lerp(1.8, 5.2, InstanceRandom) * upper;
float sway = sin(Time * 0.54 + phase + dot(WorldPos.xy, float2(0.00085, 0.00058)));
float2 moving = normalize(float2(0.82, 0.36)) * sway * lerp(0.9, 2.4, InstanceRandom) * anchored;
return float3(curve + moving, -0.10 * length(curve) * upper);
)");

		WindHigh->Description = V841NodeToken + TEXT(" ryegrass HIGH anchored curve, gust and tip flutter");
		WindHigh->Desc = V841NodeToken + TEXT(" ryegrass high wind");
		ConfigureWindInputs(WindHigh);
		WindHigh->Code = TEXT(R"(
float h = saturate((LocalPos.z + 3.0) / 78.0);
float anchored = smoothstep(0.07, 0.36, h);
float upper = smoothstep(0.26, 1.0, h);
float tip = smoothstep(0.62, 1.0, h);
anchored *= anchored;
upper *= upper;
tip *= tip;
float phase = InstanceRandom * 6.28318;
float2 restDir = float2(cos(phase + 0.45), sin(phase + 0.45));
float2 curve = restDir * lerp(2.2, 6.8, InstanceRandom) * upper;
float2 windDir = normalize(float2(0.82, 0.36));
float sway = sin(Time * 0.58 + phase + dot(WorldPos.xy, float2(0.00090, 0.00061)));
float gust = 0.74 + 0.26 * sin(Time * 0.21 + dot(WorldPos.xy, float2(-0.00020, 0.00028)) + phase * 0.27);
float flutter = sin(Time * 1.92 + phase * 1.61 + dot(WorldPos.xy, float2(0.0042, -0.0030)));
float cross = sin(Time * 0.81 + phase * 0.57);
float2 moving = windDir * sway * gust * lerp(1.4, 3.8, InstanceRandom) * anchored;
moving += float2(-windDir.y, windDir.x) * cross * 0.72 * upper;
moving += windDir * flutter * 0.58 * tip;
return float3(curve + moving, -0.12 * length(curve) * upper);
)");

		WindQuality->Desc = V841NodeToken + TEXT(" ryegrass material quality wind switch");
		WindQuality->Default.Expression = WindHigh;
		WindQuality->Inputs[static_cast<int32>(EMaterialQualityLevel::Low)].Expression = WindLow;
		WindQuality->Inputs[static_cast<int32>(EMaterialQualityLevel::Medium)].Expression = WindHigh;
		WindQuality->Inputs[static_cast<int32>(EMaterialQualityLevel::High)].Expression = WindHigh;
		WindQuality->Inputs[static_cast<int32>(EMaterialQualityLevel::Epic)].Expression = WindHigh;

		Roughness->R = 0.96f;
		Roughness->Desc = V841NodeToken + TEXT(" matte ryegrass roughness");
		Specular->R = 0.01f;
		Specular->Desc = V841NodeToken + TEXT(" restrained ryegrass specular");
		AmbientOcclusion->R = 0.97f;
		AmbientOcclusion->Desc = V841NodeToken + TEXT(" ryegrass ambient occlusion");

		UMaterialEditingLibrary::ConnectMaterialProperty(Color, TEXT(""), MP_BaseColor);
		UMaterialEditingLibrary::ConnectMaterialProperty(Subsurface, TEXT(""), MP_SubsurfaceColor);
		UMaterialEditingLibrary::ConnectMaterialProperty(TextureSample, TEXT("A"), MP_OpacityMask);
		UMaterialEditingLibrary::ConnectMaterialProperty(WindQuality, TEXT(""), MP_WorldPositionOffset);
		UMaterialEditingLibrary::ConnectMaterialProperty(Roughness, TEXT(""), MP_Roughness);
		UMaterialEditingLibrary::ConnectMaterialProperty(Specular, TEXT(""), MP_Specular);
		UMaterialEditingLibrary::ConnectMaterialProperty(AmbientOcclusion, TEXT(""), MP_AmbientOcclusion);
		FinalizeMaterial(Material);
		return Material;
	}

	void ConfigureQualityFloat(FPerQualityLevelFloat& Value, const float HighValue)
	{
		Value = FPerQualityLevelFloat(HighValue);
		Value.PerQuality.Reset();
		Value.PerQuality.Add(static_cast<int32>(EPerQualityLevels::Low), HighValue * 0.38f);
		Value.PerQuality.Add(static_cast<int32>(EPerQualityLevels::Medium), HighValue * 0.68f);
		Value.PerQuality.Add(static_cast<int32>(EPerQualityLevels::High), HighValue);
		Value.PerQuality.Add(static_cast<int32>(EPerQualityLevels::Epic), HighValue);
		Value.PerQuality.Add(static_cast<int32>(EPerQualityLevels::Cinematic), HighValue);
	}

	void ConfigureQualityInt(FPerQualityLevelInt& Value, const int32 HighValue)
	{
		Value = FPerQualityLevelInt(HighValue);
		Value.PerQuality.Reset();
		Value.PerQuality.Add(static_cast<int32>(EPerQualityLevels::Low), FMath::RoundToInt(HighValue * 0.65f));
		Value.PerQuality.Add(static_cast<int32>(EPerQualityLevels::Medium), FMath::RoundToInt(HighValue * 0.82f));
		Value.PerQuality.Add(static_cast<int32>(EPerQualityLevels::High), HighValue);
		Value.PerQuality.Add(static_cast<int32>(EPerQualityLevels::Epic), HighValue);
		Value.PerQuality.Add(static_cast<int32>(EPerQualityLevels::Cinematic), HighValue);
	}

	ULandscapeGrassType* CreateOrLoadSingleVarietyGrassType(
		const TCHAR* PackagePath,
		const TCHAR* ObjectPath,
		const TCHAR* AssetName,
		const TCHAR* MeshPath,
		UMaterialInterface* Material,
		const float Density,
		const FFloatInterval& ScaleX,
		const FFloatInterval& ScaleY,
		const FFloatInterval& ScaleZ,
		const int32 StartCull,
		const int32 EndCull,
		const int32 WpoDisableDistance)
	{
		UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, MeshPath);
		if (!Mesh || !Material)
		{
			return nullptr;
		}

		ULandscapeGrassType* GrassType = LoadObject<ULandscapeGrassType>(nullptr, ObjectPath);
		if (!GrassType)
		{
			UPackage* Package = CreatePackage(PackagePath);
			if (!Package)
			{
				return nullptr;
			}
			GrassType = NewObject<ULandscapeGrassType>(
				Package,
				AssetName,
				RF_Public | RF_Standalone | RF_Transactional);
			FAssetRegistryModule::AssetCreated(GrassType);
		}

		GrassType->Modify();
		GrassType->GrassVarieties.SetNum(1);
		FGrassVariety& Variety = GrassType->GrassVarieties[0];
		Variety.GrassMesh = Mesh;
		Variety.OverrideMaterials.Reset();
		Variety.OverrideMaterials.Add(Material);
		Variety.GrassDensity = FPerPlatformFloat(Density);
		ConfigureQualityFloat(Variety.GrassDensityQuality, Density);
		Variety.bUseGrid = true;
		Variety.PlacementJitter = 1.0f;
		Variety.StartCullDistance = FPerPlatformInt(StartCull);
		ConfigureQualityInt(Variety.StartCullDistanceQuality, StartCull);
		Variety.EndCullDistance = FPerPlatformInt(EndCull);
		ConfigureQualityInt(Variety.EndCullDistanceQuality, EndCull);
		Variety.MinLOD = 0;
		Variety.AllowedDensityRange = FFloatInterval(0.0f, 1.0f);
		Variety.Scaling = EGrassScaling::Free;
		Variety.ScaleX = ScaleX;
		Variety.ScaleY = ScaleY;
		Variety.ScaleZ = ScaleZ;
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
		GrassType->bEnableDensityScaling = true;
		GrassType->PostEditChange();
		GrassType->MarkPackageDirty();
		return GrassType;
	}

	bool TraceLandscapeGround(UWorld* World, const FVector2D& XY, FHitResult& OutHit)
	{
		if (!World)
		{
			return false;
		}

		FCollisionQueryParams Params(SCENE_QUERY_STAT(FFV841GroundTrace), true);
		const FVector Start(XY.X, XY.Y, 50000.0f);
		const FVector End(XY.X, XY.Y, -50000.0f);
		if (!World->LineTraceSingleByChannel(OutHit, Start, End, ECC_WorldStatic, Params))
		{
			return false;
		}
		return OutHit.GetActor() && OutHit.GetActor()->IsA<ALandscapeProxy>();
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

	int32 RemoveActorsWithTag(UWorld* World, const FName Tag)
	{
		TArray<AActor*> ActorsToRemove;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && Actor->ActorHasTag(Tag))
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

	AActor* SpawnContainerActor(UWorld* World, const FString& Label, const FName SecondaryTag)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AActor* Actor = World->SpawnActor<AActor>(
			AActor::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParameters);
		if (!Actor)
		{
			return nullptr;
		}

		Actor->Modify();
		Actor->Tags.AddUnique(LookdevActorTag);
		Actor->Tags.AddUnique(SecondaryTag);
		Actor->SetActorLabel(Label);

		USceneComponent* Root = NewObject<USceneComponent>(
			Actor,
			TEXT("LookdevRoot"),
			RF_Transactional);
		Root->CreationMethod = EComponentCreationMethod::Instance;
		Root->SetMobility(EComponentMobility::Static);
		Actor->AddInstanceComponent(Root);
		Actor->SetRootComponent(Root);
		Root->RegisterComponent();
		return Actor;
	}

	int32 AddScatterComponent(
		UWorld* World,
		AActor* Owner,
		const FString& ComponentName,
		UStaticMesh* Mesh,
		UMaterialInterface* Material,
		const FVector2D& Center,
		const float Radius,
		const int32 RequestedInstances,
		const FVector2D& XYScale,
		const FVector2D& ZScale,
		const int32 Seed,
		const bool bClustered)
	{
		if (!World || !Owner || !Mesh || !Material || RequestedInstances <= 0)
		{
			return 0;
		}

		UHierarchicalInstancedStaticMeshComponent* Component =
			NewObject<UHierarchicalInstancedStaticMeshComponent>(
				Owner,
				*ComponentName,
				RF_Transactional);
		Component->CreationMethod = EComponentCreationMethod::Instance;
		Component->SetupAttachment(Owner->GetRootComponent());
		Component->SetStaticMesh(Mesh);
		Component->SetMaterial(0, Material);
		Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Component->SetCanEverAffectNavigation(false);
		Component->SetMobility(EComponentMobility::Static);
		Component->SetCullDistances(1800, 10500);
		Component->CastShadow = false;
		Component->bCastDynamicShadow = false;
		Component->bCastContactShadow = false;
		Component->bAffectDistanceFieldLighting = false;
		Owner->AddInstanceComponent(Component);
		Component->RegisterComponent();

		FRandomStream Random(Seed);
		const FVector2D ClusterOffsets[] = {
			FVector2D(-0.44f, -0.18f),
			FVector2D(0.34f, -0.31f),
			FVector2D(-0.05f, 0.37f),
			FVector2D(0.48f, 0.29f)
		};

		int32 AddedInstances = 0;
		for (int32 Index = 0; Index < RequestedInstances; ++Index)
		{
			FVector2D SampleCenter = Center;
			float SampleRadius = Radius;
			if (bClustered)
			{
				const FVector2D ClusterOffset = ClusterOffsets[Index % UE_ARRAY_COUNT(ClusterOffsets)];
				SampleCenter += ClusterOffset * Radius;
				SampleRadius *= Random.FRandRange(0.22f, 0.44f);
			}

			const float Angle = Random.FRandRange(0.0f, 2.0f * PI);
			const float RadialDistance = FMath::Sqrt(Random.FRand()) * SampleRadius;
			const FVector2D XY = SampleCenter + FVector2D(
				FMath::Cos(Angle) * RadialDistance,
				FMath::Sin(Angle) * RadialDistance);

			FHitResult GroundHit;
			if (!TraceLandscapeGround(World, XY, GroundHit) || GroundHit.ImpactNormal.Z < 0.76f)
			{
				continue;
			}

			const float ScaleX = Random.FRandRange(XYScale.X, XYScale.Y);
			const float ScaleY = ScaleX * Random.FRandRange(0.88f, 1.12f);
			const float ScaleZValue = Random.FRandRange(ZScale.X, ZScale.Y);
			const FRotator Rotation(0.0f, Random.FRandRange(-180.0f, 180.0f), 0.0f);
			const FVector Location = GroundHit.ImpactPoint + FVector(0.0f, 0.0f, 2.0f);
			Component->AddInstance(
				FTransform(Rotation, Location, FVector(ScaleX, ScaleY, ScaleZValue)),
				true);
			++AddedInstances;
		}

		Component->BuildTreeIfOutdated(true, true);
		Component->MarkPackageDirty();
		Owner->MarkPackageDirty();
		return AddedInstances;
	}

	bool SpawnValidationCamera(
		UWorld* World,
		const FString& Label,
		const FName CameraTag,
		const FName OwnershipTag,
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
		Camera->Tags.AddUnique(OwnershipTag);
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

	int32 AddCandidateCameras(
		UWorld* World,
		const FString& Candidate,
		const FVector2D& Center)
	{
		struct FCameraSpec
		{
			const TCHAR* Suffix;
			FVector2D Offset;
			float Height;
			float TargetHeight;
			float FieldOfView;
		};

		const FCameraSpec Specs[] = {
			{ TEXT("Close"), FVector2D(-360.0f, -270.0f), 95.0f, 38.0f, 39.0f },
			{ TEXT("Player"), FVector2D(-720.0f, -540.0f), 180.0f, 50.0f, 46.0f },
			{ TEXT("Mid"), FVector2D(-1120.0f, -850.0f), 460.0f, 65.0f, 48.0f },
			{ TEXT("Far"), FVector2D(-2550.0f, -1950.0f), 960.0f, 95.0f, 52.0f }
		};

		FHitResult TargetHit;
		if (!TraceLandscapeGround(World, Center, TargetHit))
		{
			return 0;
		}

		int32 Added = 0;
		for (const FCameraSpec& Spec : Specs)
		{
			FHitResult CameraGround;
			if (!TraceLandscapeGround(World, Center + Spec.Offset, CameraGround))
			{
				continue;
			}

			const FString Label = FString::Printf(
				TEXT("V841_Candidate%s_%s"),
				*Candidate,
				Spec.Suffix);
			const FName Tag(*FString::Printf(
				TEXT("FFSmokeHighlandV841Candidate%s%sCamera"),
				*Candidate,
				Spec.Suffix));
			const FVector Location = CameraGround.ImpactPoint + FVector(0.0f, 0.0f, Spec.Height);
			const FVector Target = TargetHit.ImpactPoint + FVector(0.0f, 0.0f, Spec.TargetHeight);
			Added += SpawnValidationCamera(
				World,
				Label,
				Tag,
				LookdevActorTag,
				Location,
				Target,
				Spec.FieldOfView) ? 1 : 0;
		}
		return Added;
	}

	bool CreateLookdev(
		UWorld* World,
		UMaterial* NaturalizedBladeMaterial,
		UMaterial* NaturalizedRyegrassMaterial,
		FLookdevStats& OutStats)
	{
		UMaterialInterface* V84BladeMaterial = LoadObject<UMaterialInterface>(nullptr, V84BladeMaterialPath);
		UStaticMesh* BladeMesh = LoadObject<UStaticMesh>(nullptr, BladeMeshPath);
		UStaticMesh* SoftGrassMesh = LoadObject<UStaticMesh>(nullptr, RyegrassSoftMeshPath);
		UStaticMesh* TuftMesh = LoadObject<UStaticMesh>(nullptr, RyegrassTuftMeshPath);
		UStaticMesh* StalkMesh = LoadObject<UStaticMesh>(nullptr, RyegrassStalkMeshPath);
		if (!World || !NaturalizedBladeMaterial || !NaturalizedRyegrassMaterial
			|| !V84BladeMaterial
			|| !BladeMesh || !SoftGrassMesh || !TuftMesh || !StalkMesh)
		{
			return false;
		}

		RemoveActorsWithTag(World, LookdevActorTag);

		FHitResult MeadowHit;
		if (!FindFlattestLandscapeGround(
				World,
				FVector2D(87500.0f, 108000.0f),
				6500.0f,
				9,
				MeadowHit))
		{
			return false;
		}

		const FVector2D BaseCenter(MeadowHit.ImpactPoint.X, MeadowHit.ImpactPoint.Y);
		const FVector2D CandidateACenter = BaseCenter + FVector2D(-3300.0f, 0.0f);
		const FVector2D CandidateBCenter = BaseCenter;
		const FVector2D CandidateCCenter = BaseCenter + FVector2D(3300.0f, 0.0f);

		AActor* CandidateA = SpawnContainerActor(
			World,
			TEXT("V841_CandidateA_TitanBaseline"),
			TEXT("FFHighlandV841CandidateA"));
		AActor* CandidateB = SpawnContainerActor(
			World,
			TEXT("V841_CandidateB_SoftCurved"),
			TEXT("FFHighlandV841CandidateB"));
		AActor* CandidateC = SpawnContainerActor(
			World,
			TEXT("V841_CandidateC_MixedWildMeadow"),
			TEXT("FFHighlandV841CandidateC"));
		if (!CandidateA || !CandidateB || !CandidateC)
		{
			return false;
		}

		OutStats.CandidateAInstances += AddScatterComponent(
			World,
			CandidateA,
			TEXT("CandidateA_Blade"),
			BladeMesh,
			V84BladeMaterial,
			CandidateACenter,
			1050.0f,
			1650,
			FVector2D(0.92f, 1.42f),
			FVector2D(0.30f, 0.68f),
			84101,
			false);

		OutStats.CandidateBInstances += AddScatterComponent(
			World,
			CandidateB,
			TEXT("CandidateB_CurvedRyegrass"),
			SoftGrassMesh,
			NaturalizedRyegrassMaterial,
			CandidateBCenter,
			1050.0f,
			750,
			FVector2D(0.28f, 0.48f),
			FVector2D(0.35f, 0.62f),
			84102,
			true);
		OutStats.CandidateBInstances += AddScatterComponent(
			World,
			CandidateB,
			TEXT("CandidateB_ShortFiller"),
			BladeMesh,
			NaturalizedBladeMaterial,
			CandidateBCenter,
			1050.0f,
			650,
			FVector2D(0.62f, 0.96f),
			FVector2D(0.18f, 0.36f),
			84103,
			false);

		OutStats.CandidateCInstances += AddScatterComponent(
			World,
			CandidateC,
			TEXT("CandidateC_Filler"),
			BladeMesh,
			NaturalizedBladeMaterial,
			CandidateCCenter,
			1050.0f,
			1400,
			FVector2D(0.68f, 1.05f),
			FVector2D(0.22f, 0.46f),
			84104,
			false);
		OutStats.CandidateCInstances += AddScatterComponent(
			World,
			CandidateC,
			TEXT("CandidateC_MediumRyeTuft"),
			TuftMesh,
			NaturalizedRyegrassMaterial,
			CandidateCCenter,
			1050.0f,
			260,
			FVector2D(0.42f, 0.72f),
			FVector2D(0.52f, 0.85f),
			84105,
			true);
		OutStats.CandidateCInstances += AddScatterComponent(
			World,
			CandidateC,
			TEXT("CandidateC_Stalk"),
			StalkMesh,
			NaturalizedRyegrassMaterial,
			CandidateCCenter,
			1050.0f,
			14,
			FVector2D(0.22f, 0.36f),
			FVector2D(0.40f, 0.65f),
			84106,
			true);

		OutStats.Cameras += AddCandidateCameras(World, TEXT("A"), CandidateACenter);
		OutStats.Cameras += AddCandidateCameras(World, TEXT("B"), CandidateBCenter);
		OutStats.Cameras += AddCandidateCameras(World, TEXT("C"), CandidateCCenter);

		UE_LOG(
			LogTemp,
			Display,
			TEXT("FFV841Grass: lookdev centers A=(%.1f,%.1f) B=(%.1f,%.1f) C=(%.1f,%.1f)"),
			CandidateACenter.X,
			CandidateACenter.Y,
			CandidateBCenter.X,
			CandidateBCenter.Y,
			CandidateCCenter.X,
			CandidateCCenter.Y);
		return OutStats.CandidateAInstances > 0
			&& OutStats.CandidateBInstances > 0
			&& OutStats.CandidateCInstances > 0
			&& OutStats.Cameras == 12;
	}

	bool IsV841Expression(const UMaterialExpression* Expression)
	{
		if (!Expression)
		{
			return false;
		}
		if (Expression->Desc.Contains(V841NodeToken, ESearchCase::IgnoreCase))
		{
			return true;
		}
		if (const UMaterialExpressionCustom* Custom = Cast<UMaterialExpressionCustom>(Expression))
		{
			return Custom->Description.Contains(V841NodeToken, ESearchCase::IgnoreCase);
		}
		return false;
	}

	bool ApplyFinalLandscapeGrass(
		UMaterial* LandscapeMaterial,
		ULandscapeGrassType* FillerGrassType,
		ULandscapeGrassType* MediumGrassType,
		ULandscapeGrassType* TallGrassType)
	{
		if (!LandscapeMaterial || !FillerGrassType || !MediumGrassType || !TallGrassType
			|| LandscapeMaterial->bUseMaterialAttributes)
		{
			return false;
		}

		UMaterialExpressionCustom* BaseDensity = nullptr;
		UMaterialExpressionLandscapeGrassOutput* GrassOutput = nullptr;
		for (UMaterialExpression* Expression : LandscapeMaterial->GetExpressions())
		{
			if (UMaterialExpressionCustom* Custom = Cast<UMaterialExpressionCustom>(Expression))
			{
				if (Custom->Description.Contains(TEXT("native grass density"), ESearchCase::IgnoreCase)
					&& Custom->Description.Contains(TEXT("FF V84"), ESearchCase::IgnoreCase)
					&& !Custom->Description.Contains(V841NodeToken, ESearchCase::IgnoreCase))
				{
					BaseDensity = Custom;
				}
			}
			if (!GrassOutput)
			{
				GrassOutput = Cast<UMaterialExpressionLandscapeGrassOutput>(Expression);
			}
		}
		if (!BaseDensity)
		{
			return false;
		}

		LandscapeMaterial->Modify();
		if (FExpressionInput* EmissiveInput = LandscapeMaterial->GetExpressionInputForProperty(MP_EmissiveColor))
		{
			// V84 preserved an older ground-style emissive chain, which clipped exposed
			// meadow ground to white between grass instances in runtime.
			EmissiveInput->Expression = nullptr;
			EmissiveInput->OutputIndex = 0;
		}
		if (!GrassOutput)
		{
			GrassOutput = Cast<UMaterialExpressionLandscapeGrassOutput>(
				UMaterialEditingLibrary::CreateMaterialExpression(
					LandscapeMaterial,
					UMaterialExpressionLandscapeGrassOutput::StaticClass(),
					3150,
					420));
		}
		if (!GrassOutput)
		{
			return false;
		}

		for (int32 InputIndex = GrassOutput->GrassTypes.Num() - 1; InputIndex >= 0; --InputIndex)
		{
			const FName InputName = GrassOutput->GrassTypes[InputIndex].Name;
			if (InputName == V84GrassInputName
				|| InputName == FillerInputName
				|| InputName == MediumInputName
				|| InputName == TallInputName)
			{
				GrassOutput->GrassTypes.RemoveAt(InputIndex);
			}
		}

		TArray<UMaterialExpression*> OldV841Expressions;
		for (UMaterialExpression* Expression : LandscapeMaterial->GetExpressions())
		{
			if (Expression != GrassOutput && IsV841Expression(Expression))
			{
				OldV841Expressions.Add(Expression);
			}
		}
		for (UMaterialExpression* Expression : OldV841Expressions)
		{
			UMaterialEditingLibrary::DeleteMaterialExpression(LandscapeMaterial, Expression);
		}

		UMaterialExpressionWorldPosition* WorldPosition = AddWorldPositionNode(
			LandscapeMaterial,
			2650,
			150);
		UMaterialExpressionCustom* FillerMask = Cast<UMaterialExpressionCustom>(
			UMaterialEditingLibrary::CreateMaterialExpression(
				LandscapeMaterial,
				UMaterialExpressionCustom::StaticClass(),
				2850,
				230));
		UMaterialExpressionCustom* MediumMask = Cast<UMaterialExpressionCustom>(
			UMaterialEditingLibrary::CreateMaterialExpression(
				LandscapeMaterial,
				UMaterialExpressionCustom::StaticClass(),
				2850,
				390));
		UMaterialExpressionCustom* TallMask = Cast<UMaterialExpressionCustom>(
			UMaterialEditingLibrary::CreateMaterialExpression(
				LandscapeMaterial,
				UMaterialExpressionCustom::StaticClass(),
				2850,
				550));
		if (!WorldPosition || !FillerMask || !MediumMask || !TallMask)
		{
			return false;
		}

		auto ConfigureMask = [&](UMaterialExpressionCustom* Mask, const FString& Description)
		{
			Mask->Description = V841NodeToken + TEXT(" ") + Description;
			Mask->Desc = V841NodeToken + TEXT(" ") + Description;
			Mask->OutputType = CMOT_Float1;
			ConnectCustomInput(Mask, BaseDensity, TEXT("BaseDensity"));
			ConnectCustomInput(Mask, WorldPosition, TEXT("WorldPos"));
		};

		ConfigureMask(FillerMask, TEXT("short filler patch mask"));
		FillerMask->Code = TEXT(R"(
float2 p = WorldPos.xy;
float2 q0 = p / 52000.0;
float2 i0 = floor(q0);
float2 f0 = frac(q0);
f0 = f0 * f0 * (3.0 - 2.0 * f0);
float a0 = frac(sin(dot(i0, float2(127.1, 311.7))) * 43758.5453);
float b0 = frac(sin(dot(i0 + float2(1.0, 0.0), float2(127.1, 311.7))) * 43758.5453);
float c0 = frac(sin(dot(i0 + float2(0.0, 1.0), float2(127.1, 311.7))) * 43758.5453);
float d0 = frac(sin(dot(i0 + float2(1.0, 1.0), float2(127.1, 311.7))) * 43758.5453);
float broad = lerp(lerp(a0, b0, f0.x), lerp(c0, d0, f0.x), f0.y);

float2 rotated = float2(p.x * 0.73 + p.y * 0.41, -p.x * 0.38 + p.y * 0.91);
float2 q1 = rotated / 17000.0;
float2 i1 = floor(q1);
float2 f1 = frac(q1);
f1 = f1 * f1 * (3.0 - 2.0 * f1);
float a1 = frac(sin(dot(i1, float2(269.5, 183.3))) * 43758.5453);
float b1 = frac(sin(dot(i1 + float2(1.0, 0.0), float2(269.5, 183.3))) * 43758.5453);
float c1 = frac(sin(dot(i1 + float2(0.0, 1.0), float2(269.5, 183.3))) * 43758.5453);
float d1 = frac(sin(dot(i1 + float2(1.0, 1.0), float2(269.5, 183.3))) * 43758.5453);
float middle = lerp(lerp(a1, b1, f1.x), lerp(c1, d1, f1.x), f1.y);

float breakup = saturate(broad * 0.66 + middle * 0.34);
return saturate(BaseDensity * lerp(0.64, 1.0, smoothstep(0.12, 0.88, breakup)));
)");

		ConfigureMask(MediumMask, TEXT("curved medium meadow patch mask"));
		MediumMask->Code = TEXT(R"(
float2 p = WorldPos.xy;
float2 q0 = p / 25500.0;
float2 i0 = floor(q0);
float2 f0 = frac(q0);
f0 = f0 * f0 * (3.0 - 2.0 * f0);
float a0 = frac(sin(dot(i0, float2(127.1, 311.7))) * 43758.5453);
float b0 = frac(sin(dot(i0 + float2(1.0, 0.0), float2(127.1, 311.7))) * 43758.5453);
float c0 = frac(sin(dot(i0 + float2(0.0, 1.0), float2(127.1, 311.7))) * 43758.5453);
float d0 = frac(sin(dot(i0 + float2(1.0, 1.0), float2(127.1, 311.7))) * 43758.5453);
float macro = lerp(lerp(a0, b0, f0.x), lerp(c0, d0, f0.x), f0.y);

float2 rotated = float2(p.x * 0.61 - p.y * 0.56, p.x * 0.49 + p.y * 0.76);
float2 q1 = rotated / 10800.0;
float2 i1 = floor(q1);
float2 f1 = frac(q1);
f1 = f1 * f1 * (3.0 - 2.0 * f1);
float a1 = frac(sin(dot(i1, float2(269.5, 183.3))) * 43758.5453);
float b1 = frac(sin(dot(i1 + float2(1.0, 0.0), float2(269.5, 183.3))) * 43758.5453);
float c1 = frac(sin(dot(i1 + float2(0.0, 1.0), float2(269.5, 183.3))) * 43758.5453);
float d1 = frac(sin(dot(i1 + float2(1.0, 1.0), float2(269.5, 183.3))) * 43758.5453);
float detail = lerp(lerp(a1, b1, f1.x), lerp(c1, d1, f1.x), f1.y);

float patch = smoothstep(0.31, 0.73, macro * 0.66 + detail * 0.34);
return saturate(BaseDensity * lerp(0.12, 0.94, patch));
)");

		ConfigureMask(TallMask, TEXT("sparse wild stalk pocket mask"));
		TallMask->Code = TEXT(R"(
float2 p = WorldPos.xy;
float2 q0 = p / 33000.0;
float2 i0 = floor(q0);
float2 f0 = frac(q0);
f0 = f0 * f0 * (3.0 - 2.0 * f0);
float a0 = frac(sin(dot(i0, float2(127.1, 311.7))) * 43758.5453);
float b0 = frac(sin(dot(i0 + float2(1.0, 0.0), float2(127.1, 311.7))) * 43758.5453);
float c0 = frac(sin(dot(i0 + float2(0.0, 1.0), float2(127.1, 311.7))) * 43758.5453);
float d0 = frac(sin(dot(i0 + float2(1.0, 1.0), float2(127.1, 311.7))) * 43758.5453);
float macro = lerp(lerp(a0, b0, f0.x), lerp(c0, d0, f0.x), f0.y);

float2 rotated = float2(p.x * 0.48 + p.y * 0.79, -p.x * 0.71 + p.y * 0.52);
float2 q1 = rotated / 14500.0;
float2 i1 = floor(q1);
float2 f1 = frac(q1);
f1 = f1 * f1 * (3.0 - 2.0 * f1);
float a1 = frac(sin(dot(i1, float2(269.5, 183.3))) * 43758.5453);
float b1 = frac(sin(dot(i1 + float2(1.0, 0.0), float2(269.5, 183.3))) * 43758.5453);
float c1 = frac(sin(dot(i1 + float2(0.0, 1.0), float2(269.5, 183.3))) * 43758.5453);
float d1 = frac(sin(dot(i1 + float2(1.0, 1.0), float2(269.5, 183.3))) * 43758.5453);
float detail = lerp(lerp(a1, b1, f1.x), lerp(c1, d1, f1.x), f1.y);

float pocket = smoothstep(0.67, 0.84, macro * 0.72 + detail * 0.28);
return saturate(BaseDensity * pocket);
)");

		FGrassInput FillerInput(FillerInputName);
		FillerInput.GrassType = FillerGrassType;
		FillerInput.Input.Expression = FillerMask;
		GrassOutput->GrassTypes.Add(FillerInput);

		FGrassInput MediumInput(MediumInputName);
		MediumInput.GrassType = MediumGrassType;
		MediumInput.Input.Expression = MediumMask;
		GrassOutput->GrassTypes.Add(MediumInput);

		FGrassInput TallInput(TallInputName);
		TallInput.GrassType = TallGrassType;
		TallInput.Input.Expression = TallMask;
		GrassOutput->GrassTypes.Add(TallInput);
		GrassOutput->Desc = V841NodeToken + TEXT(" three-layer native LandscapeGrassOutput");

		FinalizeMaterial(LandscapeMaterial);
		return true;
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

	void ConfigureGrassLookdevLighting(
		UWorld* World,
		const float ExposureBias,
		int32& OutDirectionalLights,
		int32& OutSkyLights,
		int32& OutPostProcessVolumes)
	{
		OutDirectionalLights = 0;
		OutSkyLights = 0;
		OutPostProcessVolumes = 0;
		if (!World)
		{
			return;
		}

		for (TActorIterator<ADirectionalLight> It(World); It; ++It)
		{
			if (UDirectionalLightComponent* LightComponent = It->GetComponent())
			{
				LightComponent->Modify();
				LightComponent->SetIntensity(4.6f);
				++OutDirectionalLights;
			}
		}

		for (TActorIterator<ASkyLight> It(World); It; ++It)
		{
			if (USkyLightComponent* SkyComponent = It->GetLightComponent())
			{
				SkyComponent->Modify();
				SkyComponent->SetIntensity(1.15f);
				++OutSkyLights;
			}
		}

		APostProcessVolume* ExposureVolume = nullptr;
		for (TActorIterator<APostProcessVolume> It(World); It; ++It)
		{
			ExposureVolume = *It;
			break;
		}
		if (!ExposureVolume)
		{
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.SpawnCollisionHandlingOverride =
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			ExposureVolume = World->SpawnActor<APostProcessVolume>(
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				SpawnParameters);
		}
		if (ExposureVolume)
		{
			ExposureVolume->Modify();
			ExposureVolume->SetActorLabel(TEXT("FF_V841_Grass_Lighting"));
			ExposureVolume->Tags.AddUnique(LightingTag);
			ExposureVolume->bUnbound = true;
			ExposureVolume->Priority = FMath::Max(ExposureVolume->Priority, 60.0f);

			FPostProcessSettings& Settings = ExposureVolume->Settings;
			Settings.bOverride_AutoExposureMinBrightness = true;
			Settings.AutoExposureMinBrightness = 1.0f;
			Settings.bOverride_AutoExposureMaxBrightness = true;
			Settings.AutoExposureMaxBrightness = 1.0f;
			Settings.bOverride_AutoExposureBias = true;
			Settings.AutoExposureBias = ExposureBias;
			Settings.bOverride_BloomIntensity = true;
			Settings.BloomIntensity = 0.02f;
			++OutPostProcessVolumes;
		}
	}

	int32 AddFinalValidationCameras(UWorld* World)
	{
		RemoveActorsWithTag(World, FinalCameraTag);

		FHitResult TargetHit;
		if (!FindFlattestLandscapeGround(
				World,
				FVector2D(87500.0f, 108000.0f),
				6500.0f,
				9,
				TargetHit))
		{
			return 0;
		}

		const FVector2D TargetXY(TargetHit.ImpactPoint.X, TargetHit.ImpactPoint.Y);
		struct FFinalCameraSpec
		{
			const TCHAR* Label;
			const TCHAR* Tag;
			FVector2D Offset;
			float Height;
			float TargetHeight;
			float FieldOfView;
		};
		const FFinalCameraSpec Specs[] = {
			{
				TEXT("V841_Grass_Close"),
				TEXT("FFSmokeHighlandV841GrassCloseCamera"),
				FVector2D(-470.0f, -350.0f),
				205.0f,
				48.0f,
				40.0f
			},
			{
				TEXT("V841_Grass_Walking"),
				TEXT("FFSmokeHighlandV841GrassWalkingCamera"),
				FVector2D(-820.0f, -610.0f),
				235.0f,
				72.0f,
				48.0f
			},
			{
				TEXT("V841_Grass_MidDistance"),
				TEXT("FFSmokeHighlandV841GrassMidDistanceCamera"),
				FVector2D(-2400.0f, 1650.0f),
				860.0f,
				120.0f,
				52.0f
			}
		};

		int32 Added = 0;
		for (const FFinalCameraSpec& Spec : Specs)
		{
			FHitResult CameraHit;
			if (!TraceLandscapeGround(World, TargetXY + Spec.Offset, CameraHit))
			{
				continue;
			}

			Added += SpawnValidationCamera(
				World,
				Spec.Label,
				FName(Spec.Tag),
				FinalCameraTag,
				CameraHit.ImpactPoint + FVector(0.0f, 0.0f, Spec.Height),
				TargetHit.ImpactPoint + FVector(0.0f, 0.0f, Spec.TargetHeight),
				Spec.FieldOfView) ? 1 : 0;
		}
		return Added;
	}
}
#endif

int32 UFFStarterHighlandGrassNaturalizationCommandlet::Main(const FString& Params)
{
#if WITH_EDITOR
	FString Mode = TEXT("Lookdev");
	FParse::Value(*Params, TEXT("Mode="), Mode);
	float ExposureBias = 1.25f;
	FParse::Value(*Params, TEXT("ExposureBias="), ExposureBias);
	const bool bForce = FParse::Param(*Params, TEXT("Force"));
	UE_LOG(
		LogTemp,
		Display,
		TEXT("FFV841Grass: start mode=%s force=%s exposureBias=%.2f"),
		*Mode,
		bForce ? TEXT("true") : TEXT("false"),
		ExposureBias);

	if (!bForce)
	{
		UE_LOG(LogTemp, Error, TEXT("FFV841Grass: explicit -Force is required."));
		return 1;
	}

	UWorld* World = UEditorLoadingAndSavingUtils::LoadMap(HighlandMapPath);
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("FFV841Grass: failed to load Highland map."));
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
		UE_LOG(LogTemp, Error, TEXT("FFV841Grass: no Highland landscape found."));
		return 1;
	}

	const FProtectedState Before = CaptureProtectedState(World);
	UMaterial* NaturalizedBladeMaterial = CreateOrLoadNaturalizedBladeMaterial();
	UMaterial* NaturalizedRyegrassMaterial = CreateOrLoadNaturalizedRyegrassMaterial();
	if (!NaturalizedBladeMaterial || !NaturalizedRyegrassMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("FFV841Grass: naturalized grass material creation failed."));
		return 1;
	}

	bool bModeSucceeded = false;
	FLookdevStats LookdevStats;
	int32 RemovedLookdevActors = 0;
	int32 FinalValidationCameras = 0;
	int32 CorrectedDirectionalLights = 0;
	int32 CorrectedSkyLights = 0;
	int32 CorrectedPostProcessVolumes = 0;

	if (Mode.Equals(TEXT("Lookdev"), ESearchCase::IgnoreCase))
	{
		bModeSucceeded = CreateLookdev(
			World,
			NaturalizedBladeMaterial,
			NaturalizedRyegrassMaterial,
			LookdevStats);
	}
	else if (Mode.Equals(TEXT("Cleanup"), ESearchCase::IgnoreCase))
	{
		RemovedLookdevActors = RemoveActorsWithTag(World, LookdevActorTag);
		bModeSucceeded = true;
	}
	else if (Mode.Equals(TEXT("Lighting"), ESearchCase::IgnoreCase))
	{
		ConfigureGrassLookdevLighting(
			World,
			ExposureBias,
			CorrectedDirectionalLights,
			CorrectedSkyLights,
			CorrectedPostProcessVolumes);
		bModeSucceeded =
			CorrectedDirectionalLights > 0 &&
			CorrectedSkyLights > 0 &&
			CorrectedPostProcessVolumes > 0;
	}
	else if (Mode.Equals(TEXT("Final"), ESearchCase::IgnoreCase))
	{
		UMaterial* LandscapeMaterial = LoadObject<UMaterial>(nullptr, LandscapeMaterialPath);
		ULandscapeGrassType* FillerGrassType = CreateOrLoadSingleVarietyGrassType(
			FillerGrassTypePackagePath,
			FillerGrassTypeObjectPath,
			TEXT("GT_FF_Highland_Grass_Filler_V841"),
			BladeMeshPath,
			NaturalizedBladeMaterial,
			220.0f,
			FFloatInterval(0.68f, 1.05f),
			FFloatInterval(0.66f, 1.02f),
			FFloatInterval(0.22f, 0.46f),
			2500,
			9000,
			5500);
		ULandscapeGrassType* MediumGrassType = CreateOrLoadSingleVarietyGrassType(
			MediumGrassTypePackagePath,
			MediumGrassTypeObjectPath,
			TEXT("GT_FF_Highland_Grass_Medium_V841"),
			RyegrassTuftMeshPath,
			NaturalizedRyegrassMaterial,
			54.0f,
			FFloatInterval(0.42f, 0.72f),
			FFloatInterval(0.40f, 0.70f),
			FFloatInterval(0.52f, 0.85f),
			3000,
			11500,
			7200);
		ULandscapeGrassType* TallGrassType = CreateOrLoadSingleVarietyGrassType(
			TallGrassTypePackagePath,
			TallGrassTypeObjectPath,
			TEXT("GT_FF_Highland_Grass_Tall_V841"),
			RyegrassStalkMeshPath,
			NaturalizedRyegrassMaterial,
			5.0f,
			FFloatInterval(0.22f, 0.36f),
			FFloatInterval(0.21f, 0.35f),
			FFloatInterval(0.40f, 0.65f),
			4000,
			14000,
			8000);

		if (!LandscapeMaterial || !FillerGrassType || !MediumGrassType || !TallGrassType)
		{
			UE_LOG(LogTemp, Error, TEXT("FFV841Grass: final grass assets could not be created."));
			return 1;
		}

		RemovedLookdevActors = RemoveActorsWithTag(World, LookdevActorTag);
		bModeSucceeded = ApplyFinalLandscapeGrass(
			LandscapeMaterial,
			FillerGrassType,
			MediumGrassType,
			TallGrassType);
		if (bModeSucceeded)
		{
			for (TActorIterator<ALandscapeProxy> It(World); It; ++It)
			{
				RefreshLandscapeGrass(*It);
			}
			ConfigureGrassLookdevLighting(
				World,
				ExposureBias,
				CorrectedDirectionalLights,
				CorrectedSkyLights,
				CorrectedPostProcessVolumes);
			FinalValidationCameras = AddFinalValidationCameras(World);
			PrimaryLandscape->Modify();
			PrimaryLandscape->Tags.AddUnique(AppliedTag);
			PrimaryLandscape->MarkPackageDirty();
			bModeSucceeded = FinalValidationCameras == 3;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FFV841Grass: unsupported mode '%s'."), *Mode);
		return 1;
	}

	if (!bModeSucceeded)
	{
		UE_LOG(LogTemp, Error, TEXT("FFV841Grass: mode '%s' failed before save."), *Mode);
		return 1;
	}

	const FProtectedState After = CaptureProtectedState(World);
	const bool bProtectedStateUnchanged = ProtectedStateMatches(Before, After);
	if (!bProtectedStateUnchanged)
	{
		UE_LOG(LogTemp, Error, TEXT("FFV841Grass: protected-state mismatch; refusing to save."));
		return 1;
	}

	World->MarkPackageDirty();
	const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
	const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);

	UE_LOG(
		LogTemp,
		Display,
		TEXT("FFV841Grass: lookdev instances A=%d B=%d C=%d cameras=%d removedLookdev=%d finalCameras=%d correctedDirectionalLights=%d correctedSkyLights=%d correctedPostProcessVolumes=%d"),
		LookdevStats.CandidateAInstances,
		LookdevStats.CandidateBInstances,
		LookdevStats.CandidateCInstances,
		LookdevStats.Cameras,
		RemovedLookdevActors,
		FinalValidationCameras,
		CorrectedDirectionalLights,
		CorrectedSkyLights,
		CorrectedPostProcessVolumes);
	UE_LOG(
		LogTemp,
		Display,
		TEXT("FFV841Grass: protected playerStarts=%d waterActors=%d mountainActors=%d unchanged=%s"),
		After.PlayerStarts.Num(),
		After.WaterActors.Num(),
		After.MountainActors,
		bProtectedStateUnchanged ? TEXT("true") : TEXT("false"));
	UE_LOG(
		LogTemp,
		Display,
		TEXT("FFV841Grass: savedMap=%s savedPackages=%s"),
		bSavedMap ? TEXT("true") : TEXT("false"),
		bSavedPackages ? TEXT("true") : TEXT("false"));

	return (bSavedMap && bSavedPackages) ? 0 : 1;
#else
	UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandGrassNaturalization can only run in editor builds."));
	return 1;
#endif
}
