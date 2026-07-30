#include "FFTitanGrasslandPipelineProofCommandlet.h"

#if WITH_EDITOR
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "AssetToolsModule.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/RuntimeVirtualTextureComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Editor.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Factories/MaterialFactoryNew.h"
#include "FileHelpers.h"
#include "GameFramework/Actor.h"
#include "Landscape.h"
#include "LandscapeGrassType.h"
#include "LandscapeInfo.h"
#include "LandscapeLayerInfoObject.h"
#include "LandscapeProxy.h"
#include "MaterialEditingLibrary.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionCustom.h"
#include "Materials/MaterialExpressionLandscapeLayerWeight.h"
#include "Materials/MaterialExpressionLinearInterpolate.h"
#include "Materials/MaterialExpressionRuntimeVirtualTextureOutput.h"
#include "Materials/MaterialExpressionVertexNormalWS.h"
#include "Materials/MaterialExpressionWorldPosition.h"
#include "Materials/MaterialInterface.h"
#include "Modules/ModuleManager.h"
#include "UObject/Package.h"
#include "VT/RuntimeVirtualTexture.h"
#include "VT/RuntimeVirtualTextureVolume.h"
#endif

#if WITH_EDITOR
namespace
{
	const TCHAR* HighlandMapPath = TEXT("/Game/Maps/FF_Starter_Highland_Blockout");
	const FName ProofTag(TEXT("FFTitanGrasslandPipelineProof"));

	const TCHAR* ProxyGrassMaterialPath = TEXT("/Game/FantasyFrontier/Blockout/Materials/M_FF_Highland_GrassBlade_Soft.M_FF_Highland_GrassBlade_Soft");
	const TCHAR* NativeGrassMaterialPath = TEXT("/Game/Environment/Foliage/Materials/MI_GrassBlade.MI_GrassBlade");
	const TCHAR* NativeGrassDarkerMaterialPath = TEXT("/Game/Environment/Foliage/Materials/MI_GrassBladeDarker.MI_GrassBladeDarker");
	const TCHAR* TitanGrassTypePath = TEXT("/Game/Landscape/LGT/LGT_Grass.LGT_Grass");
	const TCHAR* TitanGroundMaterialPath = TEXT("/Game/Landscape/Materials/MI_LandscapeMain.MI_LandscapeMain");
	const FString StrippedMaterialFolderPath = TEXT("/Game/FantasyFrontier/Blockout/Materials");
	const FString StrippedGroundMaterialName = TEXT("M_FF_TitanGrassland_StrippedProof");
	const TCHAR* TitanRVTDPath = TEXT("/Game/Landscape/RVT/RVT_Titan_D.RVT_Titan_D");
	const TCHAR* TitanRVTHPath = TEXT("/Game/Landscape/RVT/RVT_Titan_H.RVT_Titan_H");
	const TCHAR* TitanRVTDHPath = TEXT("/Game/Landscape/RVT/RVT_Titan_DH.RVT_Titan_DH");
	const TCHAR* FoliageMPCPath = TEXT("/Game/Blueprint/FoliageInteraction/MPC_Player.MPC_Player");
	const TCHAR* FoliageRTPath = TEXT("/Game/Blueprint/FoliageInteraction/RT_Player.RT_Player");
	const TCHAR* FoliageUpdateMaterialPath = TEXT("/Game/Blueprint/FoliageInteraction/M_UpdateFoliageDraw.M_UpdateFoliageDraw");
	const TCHAR* EnginePlaneMeshPath = TEXT("/Engine/BasicShapes/Plane.Plane");

	const TCHAR* GrasslandLayerInfoPaths[] = {
		TEXT("/Game/Landscape/WeightLayers/LI_Grassland_Grass.LI_Grassland_Grass"),
		TEXT("/Game/Landscape/WeightLayers/LI_Grassland_Dirt.LI_Grassland_Dirt"),
		TEXT("/Game/Landscape/WeightLayers/LI_Grassland_Rock.LI_Grassland_Rock"),
		TEXT("/Game/Landscape/WeightLayers/LI_Grassland_Sand.LI_Grassland_Sand")
	};

	enum class EPipelinePatchKind : uint8
	{
		Proxy,
		NativeTitan,
		NativeTitanParity
	};

	struct FPipelinePatchSpec
	{
		const TCHAR* Label;
		const TCHAR* CameraTag;
		FVector2D Center;
		EPipelinePatchKind Kind;
		bool bShadow;
	};

	const FPipelinePatchSpec PatchSpecs[] = {
		{ TEXT("FF_TitanPipeline_A_Proxy_Sun"), TEXT("FFSmokeTitanPipelineProxySunCamera"), FVector2D(35450.0f, 145350.0f), EPipelinePatchKind::Proxy, false },
		{ TEXT("FF_TitanPipeline_B_NativeGrassland_Sun"), TEXT("FFSmokeTitanPipelineNativeSunCamera"), FVector2D(42450.0f, 145350.0f), EPipelinePatchKind::NativeTitan, false },
		{ TEXT("FF_TitanPipeline_BPlus_LayerRVTInteraction_Sun"), TEXT("FFSmokeTitanPipelineParitySunCamera"), FVector2D(49450.0f, 145350.0f), EPipelinePatchKind::NativeTitanParity, false },
		{ TEXT("FF_TitanPipeline_A_Proxy_Shadow"), TEXT("FFSmokeTitanPipelineProxyShadowCamera"), FVector2D(81200.0f, 111500.0f), EPipelinePatchKind::Proxy, true },
		{ TEXT("FF_TitanPipeline_B_NativeGrassland_Shadow"), TEXT("FFSmokeTitanPipelineNativeShadowCamera"), FVector2D(87200.0f, 111500.0f), EPipelinePatchKind::NativeTitan, true },
		{ TEXT("FF_TitanPipeline_BPlus_LayerRVTInteraction_Shadow"), TEXT("FFSmokeTitanPipelineParityShadowCamera"), FVector2D(93200.0f, 111500.0f), EPipelinePatchKind::NativeTitanParity, true },
		{ TEXT("FF_TitanPipeline_TopDown_Reference"), TEXT("FFSmokeTitanPipelineTopDownCamera"), FVector2D(65000.0f, 128400.0f), EPipelinePatchKind::Proxy, false }
	};

	float PseudoRandom01(int32 Seed)
	{
		uint32 Hash = static_cast<uint32>(Seed);
		Hash ^= Hash >> 16;
		Hash *= 0x7feb352du;
		Hash ^= Hash >> 15;
		Hash *= 0x846ca68bu;
		Hash ^= Hash >> 16;
		return static_cast<float>(Hash & 0x00ffffffu) / 16777215.0f;
	}

	bool GetLandscapeGround(UWorld* World, const FVector2D& XY, FHitResult& OutHit)
	{
		if (!World)
		{
			return false;
		}

		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(FFTitanGrasslandPipelineProofTrace), true);
		const FVector Start(XY.X, XY.Y, 6000.0f);
		const FVector End(XY.X, XY.Y, -16000.0f);
		if (!World->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, QueryParams))
		{
			return false;
		}
		return OutHit.GetActor() && OutHit.GetActor()->IsA<ALandscapeProxy>();
	}

	void RemovePreviousProofActors(UWorld* World, int32& RemovedActors)
	{
		RemovedActors = 0;
		if (!World)
		{
			return;
		}

		TArray<AActor*> ToRemove;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && Actor->ActorHasTag(ProofTag))
			{
				ToRemove.Add(Actor);
			}
		}

		for (AActor* Actor : ToRemove)
		{
			World->DestroyActor(Actor);
			++RemovedActors;
		}
	}

	AActor* SpawnProofActor(UWorld* World, const FString& Label, const FVector& Location)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AActor* Actor = World->SpawnActor<AActor>(AActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParameters);
		if (!Actor)
		{
			return nullptr;
		}

		Actor->Modify();
		Actor->Tags.AddUnique(ProofTag);
		Actor->SetActorLabel(Label);
		USceneComponent* RootComponent = NewObject<USceneComponent>(Actor, TEXT("Root"), RF_Transactional);
		RootComponent->SetMobility(EComponentMobility::Static);
		Actor->SetRootComponent(RootComponent);
		RootComponent->RegisterComponent();
		Actor->AddInstanceComponent(RootComponent);
		Actor->MarkPackageDirty();
		return Actor;
	}

	UHierarchicalInstancedStaticMeshComponent* CreateHISM(AActor* Owner, UStaticMesh* Mesh, UMaterialInterface* Material, const FString& Name, int32 StartCullDistance, int32 EndCullDistance)
	{
		if (!Owner || !Mesh)
		{
			return nullptr;
		}

		UHierarchicalInstancedStaticMeshComponent* Component = NewObject<UHierarchicalInstancedStaticMeshComponent>(Owner, *Name, RF_Transactional);
		if (!Component)
		{
			return nullptr;
		}

		Component->SetStaticMesh(Mesh);
		if (Material)
		{
			const int32 MaterialSlots = FMath::Max(1, Mesh->GetStaticMaterials().Num());
			for (int32 SlotIndex = 0; SlotIndex < MaterialSlots; ++SlotIndex)
			{
				Component->SetMaterial(SlotIndex, Material);
			}
		}
		Component->SetMobility(EComponentMobility::Static);
		Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Component->SetGenerateOverlapEvents(false);
		Component->SetCastShadow(false);
		Component->SetCullDistances(StartCullDistance, EndCullDistance);
		Component->SetupAttachment(Owner->GetRootComponent());
		Component->RegisterComponent();
		Owner->AddInstanceComponent(Component);
		return Component;
	}

	int32 AddProxyGrassInstances(UWorld* World, UHierarchicalInstancedStaticMeshComponent* Component, const FVector2D& Center, int32 SeedOffset)
	{
		if (!World || !Component)
		{
			return 0;
		}

		int32 Added = 0;
		constexpr float Radius = 1220.0f;
		constexpr float Spacing = 128.0f;
		const int32 GridRadius = FMath::CeilToInt(Radius / Spacing);
		for (int32 GridY = -GridRadius; GridY <= GridRadius; ++GridY)
		{
			for (int32 GridX = -GridRadius; GridX <= GridRadius; ++GridX)
			{
				const int32 Seed = SeedOffset + GridX * 73856093 + GridY * 19349663;
				const FVector2D Position = Center + FVector2D(
					GridX * Spacing + FMath::Lerp(-Spacing * 0.45f, Spacing * 0.45f, PseudoRandom01(Seed + 11)),
					GridY * Spacing + FMath::Lerp(-Spacing * 0.45f, Spacing * 0.45f, PseudoRandom01(Seed + 23)));
				if (FVector2D::Distance(Position, Center) > Radius)
				{
					continue;
				}

				FHitResult GroundHit;
				if (!GetLandscapeGround(World, Position, GroundHit) || GroundHit.ImpactNormal.Z < 0.88f)
				{
					continue;
				}

				const float Yaw = PseudoRandom01(Seed + 37) * 360.0f;
				const float XYScale = FMath::Lerp(1.05f, 1.42f, PseudoRandom01(Seed + 41));
				const float ZScale = FMath::Lerp(0.36f, 0.78f, PseudoRandom01(Seed + 53));
				const FQuat SurfaceRotation = FRotationMatrix::MakeFromZ(GroundHit.ImpactNormal).ToQuat();
				const FQuat YawAroundSurface(GroundHit.ImpactNormal, FMath::DegreesToRadians(Yaw));

				FTransform InstanceTransform;
				InstanceTransform.SetLocation(GroundHit.ImpactPoint + FVector(0.0f, 0.0f, 0.8f));
				InstanceTransform.SetRotation(YawAroundSurface * SurfaceRotation);
				InstanceTransform.SetScale3D(FVector(XYScale, XYScale, ZScale));
				Component->AddInstance(InstanceTransform, true);
				++Added;
			}
		}

		Component->BuildTreeIfOutdated(true, true);
		Component->MarkPackageDirty();
		return Added;
	}

	FString GetObjectPath(const FString& FolderPath, const FString& AssetName)
	{
		return FString::Printf(TEXT("%s/%s.%s"), *FolderPath, *AssetName, *AssetName);
	}

	UMaterial* CreateOrLoadMaterial(const FString& FolderPath, const FString& AssetName)
	{
		if (UMaterial* Existing = LoadObject<UMaterial>(nullptr, *GetObjectPath(FolderPath, AssetName)))
		{
			Existing->Modify();
			UMaterialEditingLibrary::DeleteAllMaterialExpressions(Existing);
			return Existing;
		}

		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
		UMaterialFactoryNew* Factory = NewObject<UMaterialFactoryNew>();
		return Cast<UMaterial>(AssetToolsModule.Get().CreateAsset(AssetName, FolderPath, UMaterial::StaticClass(), Factory));
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

	UMaterialExpressionWorldPosition* AddWorldPositionNode(UMaterial* Material, const int32 X, const int32 Y)
	{
		UMaterialExpressionWorldPosition* WorldPosition = Cast<UMaterialExpressionWorldPosition>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionWorldPosition::StaticClass(), X, Y));
		if (WorldPosition)
		{
			WorldPosition->WorldPositionShaderOffset = WPT_ExcludeAllShaderOffsets;
		}
		return WorldPosition;
	}

	UMaterialExpressionConstant3Vector* AddConstantColor(UMaterial* Material, const FLinearColor& Color, const int32 X, const int32 Y, const FString& Desc)
	{
		UMaterialExpressionConstant3Vector* ColorNode = Cast<UMaterialExpressionConstant3Vector>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant3Vector::StaticClass(), X, Y));
		if (ColorNode)
		{
			ColorNode->Constant = Color;
			ColorNode->Desc = Desc;
		}
		return ColorNode;
	}

	UMaterialExpression* AddLayerWeight(UMaterial* Material, UMaterialExpression* BaseExpression, UMaterialExpression* LayerExpression, const FName LayerName, const int32 X, const int32 Y)
	{
		UMaterialExpressionLandscapeLayerWeight* LayerWeight = Cast<UMaterialExpressionLandscapeLayerWeight>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionLandscapeLayerWeight::StaticClass(), X, Y));
		if (!LayerWeight || !BaseExpression || !LayerExpression)
		{
			return BaseExpression;
		}

		LayerWeight->ParameterName = LayerName;
		LayerWeight->PreviewWeight = LayerName == FName(TEXT("Grassland_Grass")) ? 1.0f : 0.0f;
		LayerWeight->Base.Expression = BaseExpression;
		LayerWeight->Layer.Expression = LayerExpression;
		return LayerWeight;
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

	UMaterial* BuildStrippedGrasslandMaterial()
	{
		UMaterial* Material = CreateOrLoadMaterial(StrippedMaterialFolderPath, StrippedGroundMaterialName);
		if (!Material)
		{
			return nullptr;
		}

		UMaterialExpressionWorldPosition* WorldPosition = AddWorldPositionNode(Material, -1180, -180);
		UMaterialExpressionVertexNormalWS* VertexNormal = Cast<UMaterialExpressionVertexNormalWS>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionVertexNormalWS::StaticClass(), -1180, 0));
		UMaterialExpressionCustom* GrasslandColor = Cast<UMaterialExpressionCustom>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionCustom::StaticClass(), -820, -150));
		UMaterialExpressionCustom* HeightValue = Cast<UMaterialExpressionCustom>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionCustom::StaticClass(), -820, 190));
		UMaterialExpressionConstant* Roughness = Cast<UMaterialExpressionConstant>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant::StaticClass(), -210, 260));
		UMaterialExpressionConstant* Specular = Cast<UMaterialExpressionConstant>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant::StaticClass(), -210, 360));
		UMaterialExpressionConstant* Opacity = Cast<UMaterialExpressionConstant>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant::StaticClass(), -210, 460));

		if (!WorldPosition || !VertexNormal || !GrasslandColor || !HeightValue || !Roughness || !Specular || !Opacity)
		{
			return nullptr;
		}

		GrasslandColor->Description = TEXT("Stripped cook-safe Titan Grassland color without M_LandscapeMain functions");
		GrasslandColor->OutputType = CMOT_Float3;
		ConnectCustomInput(GrasslandColor, WorldPosition, TEXT("WorldPos"));
		ConnectCustomInput(GrasslandColor, VertexNormal, TEXT("NormalWS"));
		GrasslandColor->Code = TEXT(R"(
float2 p = WorldPos.xy;
float slope = 1.0 - saturate(NormalWS.z);
float broadA = 0.5 + 0.5 * sin(p.x / 39000.0 + p.y / 61000.0 + 0.75 * sin(p.y / 52000.0));
float broadB = 0.5 + 0.5 * sin(-p.x / 54000.0 + p.y / 33000.0 + 1.4);
float broadC = 0.5 + 0.5 * sin((p.x + p.y) / 76000.0 - cos(p.x / 44000.0));
float broad = saturate(broadA * 0.44 + broadB * 0.34 + broadC * 0.22);
float mid = 0.5 + 0.5 * sin(p.x / 9400.0 - p.y / 12800.0 + broad * 3.14159);
float micro = 0.5 + 0.5 * sin(p.x * 0.0042 + p.y * 0.0031 + sin(p.y * 0.0017));

float3 olive = float3(0.410, 0.500, 0.160);
float3 sunGrass = float3(0.630, 0.640, 0.275);
float3 softMeadow = float3(0.520, 0.585, 0.215);
float3 lowGreen = float3(0.285, 0.420, 0.115);
float3 warmThatch = float3(0.470, 0.420, 0.150);

float3 grass = lerp(lowGreen, olive, saturate(0.45 + broad * 0.35));
grass = lerp(grass, softMeadow, saturate(broad * 0.28 + mid * 0.12));
grass = lerp(grass, sunGrass, saturate(broadA * 0.16 + slope * 0.08));
grass = lerp(grass, warmThatch, saturate((1.0 - micro) * 0.075 + broadB * 0.055));
grass *= lerp(0.92, 1.08, saturate(broad * 0.65 + mid * 0.35));

float3 rockLow = float3(0.245, 0.220, 0.175);
float3 rockHigh = float3(0.540, 0.490, 0.385);
float3 rock = lerp(rockLow, rockHigh, saturate(slope * 1.7 + broad * 0.18));
return lerp(grass, rock, smoothstep(0.28, 0.58, slope));
)");

		HeightValue->Description = TEXT("Minimal RVT world height writer");
		HeightValue->OutputType = CMOT_Float1;
		ConnectCustomInput(HeightValue, WorldPosition, TEXT("WorldPos"));
		HeightValue->Code = TEXT("return WorldPos.z;");

		UMaterialExpressionConstant3Vector* DirtColor = AddConstantColor(Material, FLinearColor(0.445f, 0.360f, 0.210f), -520, 40, TEXT("Grassland_Dirt safe color"));
		UMaterialExpressionConstant3Vector* RockColor = AddConstantColor(Material, FLinearColor(0.360f, 0.315f, 0.245f), -520, 190, TEXT("Grassland_Rock safe color"));
		UMaterialExpressionConstant3Vector* SandColor = AddConstantColor(Material, FLinearColor(0.610f, 0.535f, 0.315f), -520, 340, TEXT("Grassland_Sand safe color"));
		if (!DirtColor || !RockColor || !SandColor)
		{
			return nullptr;
		}

		UMaterialExpression* CurrentColor = GrasslandColor;
		CurrentColor = AddLayerWeight(Material, CurrentColor, GrasslandColor, FName(TEXT("Grassland_Grass")), -120, -170);
		CurrentColor = AddLayerWeight(Material, CurrentColor, DirtColor, FName(TEXT("Grassland_Dirt")), 80, 20);
		CurrentColor = AddLayerWeight(Material, CurrentColor, RockColor, FName(TEXT("Grassland_Rock")), 280, 180);
		CurrentColor = AddLayerWeight(Material, CurrentColor, SandColor, FName(TEXT("Grassland_Sand")), 480, 340);

		Roughness->R = 0.96f;
		Specular->R = 0.05f;
		Opacity->R = 1.0f;

		UMaterialExpressionRuntimeVirtualTextureOutput* RVTOutput = Cast<UMaterialExpressionRuntimeVirtualTextureOutput>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionRuntimeVirtualTextureOutput::StaticClass(), 820, 40));
		if (!RVTOutput)
		{
			return nullptr;
		}
		RVTOutput->BaseColor.Expression = CurrentColor;
		RVTOutput->Specular.Expression = Specular;
		RVTOutput->Roughness.Expression = Roughness;
		RVTOutput->Opacity.Expression = Opacity;
		RVTOutput->WorldHeight.Expression = HeightValue;
		RVTOutput->Normal.Expression = VertexNormal;

		Material->BlendMode = BLEND_Opaque;
		Material->TwoSided = false;
		UMaterialEditingLibrary::ConnectMaterialProperty(CurrentColor, TEXT(""), MP_BaseColor);
		UMaterialEditingLibrary::ConnectMaterialProperty(Roughness, TEXT(""), MP_Roughness);
		UMaterialEditingLibrary::ConnectMaterialProperty(Specular, TEXT(""), MP_Specular);
		FinalizeMaterial(Material);
		return Material;
	}

	ARuntimeVirtualTextureVolume* SpawnRVTVolume(UWorld* World, const FString& Label, const FVector& Center, URuntimeVirtualTexture* RVT)
	{
		if (!World || !RVT)
		{
			return nullptr;
		}

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ARuntimeVirtualTextureVolume* Volume = World->SpawnActor<ARuntimeVirtualTextureVolume>(
			ARuntimeVirtualTextureVolume::StaticClass(),
			Center,
			FRotator::ZeroRotator,
			SpawnParameters);
		if (!Volume)
		{
			return nullptr;
		}

		Volume->Modify();
		Volume->Tags.AddUnique(ProofTag);
		Volume->SetActorLabel(Label);
		// Default RVT volumes are brush-sized; this scale keeps the volume local to
		// the proof patch without leaking across the Highland map.
		Volume->SetActorScale3D(FVector(26.0f, 26.0f, 32.0f));
		if (Volume->VirtualTextureComponent)
		{
			Volume->VirtualTextureComponent->Modify();
			Volume->VirtualTextureComponent->SetVirtualTexture(RVT);
			Volume->VirtualTextureComponent->MarkPackageDirty();
		}
		Volume->MarkPackageDirty();
		return Volume;
	}

	FName ResolveLayerName(ULandscapeLayerInfoObject* LayerInfo, const TCHAR* Fallback)
	{
		if (LayerInfo && !LayerInfo->GetLayerName().IsNone())
		{
			return LayerInfo->GetLayerName();
		}
		return FName(Fallback);
	}

	ALandscape* CreateNativeLandscapePatch(
		UWorld* World,
		const FPipelinePatchSpec& Patch,
		UMaterialInterface* TitanGroundMaterial,
		const TArray<ULandscapeLayerInfoObject*>& LayerInfos,
		URuntimeVirtualTexture* RVTD,
		URuntimeVirtualTexture* RVTH,
		URuntimeVirtualTexture* RVTDH,
		FVector& OutSurfaceCenter)
	{
		if (!World || !TitanGroundMaterial || LayerInfos.Num() < 4 || !LayerInfos[0] || !RVTD || !RVTH)
		{
			return nullptr;
		}

		FHitResult GroundHit;
		OutSurfaceCenter = FVector(Patch.Center.X, Patch.Center.Y, -3200.0f);
		if (GetLandscapeGround(World, Patch.Center, GroundHit))
		{
			OutSurfaceCenter = GroundHit.ImpactPoint + FVector(0.0f, 0.0f, 18.0f);
		}

		constexpr int32 SectionsPerComponent = 1;
		constexpr int32 QuadsPerSection = 31;
		constexpr int32 ComponentCountX = 1;
		constexpr int32 ComponentCountY = 1;
		constexpr int32 QuadsPerComponent = SectionsPerComponent * QuadsPerSection;
		constexpr int32 SizeX = ComponentCountX * QuadsPerComponent + 1;
		constexpr int32 SizeY = ComponentCountY * QuadsPerComponent + 1;
		const FVector LandscapeScale(100.0f, 100.0f, 100.0f);
		const FVector LandscapeLocation = OutSurfaceCenter + FTransform(FRotator::ZeroRotator, FVector::ZeroVector, LandscapeScale)
			.TransformVector(FVector(-ComponentCountX * QuadsPerComponent / 2.0f, -ComponentCountY * QuadsPerComponent / 2.0f, 0.0f));
		const bool bRichTitanContext = Patch.Kind == EPipelinePatchKind::NativeTitanParity;

		TArray<uint16> HeightData;
		HeightData.Init(32768, SizeX * SizeY);
		if (bRichTitanContext)
		{
			for (int32 Y = 0; Y < SizeY; ++Y)
			{
				for (int32 X = 0; X < SizeX; ++X)
				{
					const float U = static_cast<float>(X) / static_cast<float>(SizeX - 1);
					const float V = static_cast<float>(Y) / static_cast<float>(SizeY - 1);
					const float WaveA = FMath::Sin(U * UE_TWO_PI * 1.35f + V * 1.70f);
					const float WaveB = FMath::Sin((U + V) * UE_TWO_PI * 0.85f + 0.55f);
					const float HeightCm = (WaveA * 0.58f + WaveB * 0.42f) * 34.0f;
					// Landscape height units are scaled by ZScale / 128, so this is
					// a subtle proof-only rolling surface, not a Highland terrain edit.
					HeightData[Y * SizeX + X] = static_cast<uint16>(FMath::Clamp(32768 + FMath::RoundToInt(HeightCm * 128.0f / LandscapeScale.Z), 0, 65535));
				}
			}
		}

		TArray<FLandscapeImportLayerInfo> ImportLayers;
		const TCHAR* FallbackLayerNames[] = {
			TEXT("Grassland_Grass"),
			TEXT("Grassland_Dirt"),
			TEXT("Grassland_Rock"),
			TEXT("Grassland_Sand")
		};
		for (int32 LayerIndex = 0; LayerIndex < 4; ++LayerIndex)
		{
			FLandscapeImportLayerInfo LayerInfo(ResolveLayerName(LayerInfos[LayerIndex], FallbackLayerNames[LayerIndex]));
			LayerInfo.LayerInfo = LayerInfos[LayerIndex];
			LayerInfo.LayerData.Init(LayerIndex == 0 ? 255 : 0, SizeX * SizeY);
			ImportLayers.Add(MoveTemp(LayerInfo));
		}

		if (bRichTitanContext)
		{
			for (int32 Y = 0; Y < SizeY; ++Y)
			{
				for (int32 X = 0; X < SizeX; ++X)
				{
					const float U = static_cast<float>(X) / static_cast<float>(SizeX - 1);
					const float V = static_cast<float>(Y) / static_cast<float>(SizeY - 1);
					const FVector2D Centered(U - 0.5f, V - 0.5f);
					const float Edge = FMath::Clamp((Centered.Size() - 0.34f) / 0.25f, 0.0f, 1.0f);
					const float Meander = 0.5f + 0.17f * FMath::Sin(U * UE_TWO_PI * 1.45f + 0.35f) + 0.06f * FMath::Sin(U * UE_TWO_PI * 3.1f);
					const float DirtPath = FMath::Exp(-FMath::Square((V - Meander) / 0.055f));
					const float PatchNoise = 0.5f + 0.5f * FMath::Sin((U * 5.1f + V * 7.7f) * UE_TWO_PI + 0.9f * FMath::Sin(U * 9.0f));
					const float SandPocket = FMath::Exp(-((FMath::Square(U - 0.72f) + FMath::Square(V - 0.26f)) / 0.020f));
					const int32 RockWeight = FMath::RoundToInt(FMath::Clamp(Edge * 62.0f + FMath::Max(0.0f, PatchNoise - 0.78f) * 42.0f, 0.0f, 88.0f));
					const int32 DirtWeight = FMath::RoundToInt(FMath::Clamp(DirtPath * 74.0f + PatchNoise * 12.0f, 0.0f, 96.0f));
					const int32 SandWeight = FMath::RoundToInt(FMath::Clamp(SandPocket * 54.0f + (1.0f - PatchNoise) * 10.0f, 0.0f, 64.0f));
					const int32 GrassWeight = FMath::Max(0, 255 - RockWeight - DirtWeight - SandWeight);
					const int32 DataIndex = Y * SizeX + X;
					ImportLayers[0].LayerData[DataIndex] = static_cast<uint8>(GrassWeight);
					ImportLayers[1].LayerData[DataIndex] = static_cast<uint8>(DirtWeight);
					ImportLayers[2].LayerData[DataIndex] = static_cast<uint8>(RockWeight);
					ImportLayers[3].LayerData[DataIndex] = static_cast<uint8>(SandWeight);
				}
			}
		}

		TMap<FGuid, TArray<uint16>> HeightDataPerLayers;
		TMap<FGuid, TArray<FLandscapeImportLayerInfo>> MaterialLayerDataPerLayers;
		HeightDataPerLayers.Add(FGuid(), MoveTemp(HeightData));
		MaterialLayerDataPerLayers.Add(FGuid(), MoveTemp(ImportLayers));

		ALandscape* LandscapePatch = World->SpawnActor<ALandscape>(LandscapeLocation, FRotator::ZeroRotator);
		if (!LandscapePatch)
		{
			return nullptr;
		}

		LandscapePatch->Modify();
		LandscapePatch->Tags.AddUnique(ProofTag);
		LandscapePatch->SetActorLabel(FString::Printf(TEXT("%s_TitanLandscapeLayerRVTWriter"), Patch.Label));
		LandscapePatch->LandscapeMaterial = TitanGroundMaterial;
		LandscapePatch->SetActorScale3D(LandscapeScale);
		LandscapePatch->StaticLightingLOD = 0;
		LandscapePatch->RuntimeVirtualTextures.AddUnique(RVTD);
		LandscapePatch->RuntimeVirtualTextures.AddUnique(RVTH);
		if (RVTDH)
		{
			LandscapePatch->RuntimeVirtualTextures.AddUnique(RVTDH);
		}
		LandscapePatch->VirtualTextureRenderPassType = ERuntimeVirtualTextureMainPassType::Always;
		LandscapePatch->Import(FGuid::NewGuid(), 0, 0, SizeX - 1, SizeY - 1, SectionsPerComponent, QuadsPerSection, HeightDataPerLayers, TEXT(""), MaterialLayerDataPerLayers, ELandscapeImportAlphamapType::Additive, TArrayView<const FLandscapeLayer>());
		LandscapePatch->PostEditChange();
		LandscapePatch->MarkPackageDirty();
		return LandscapePatch;
	}

	float GetVarietyDensity(const FGrassVariety& Variety, int32 VarietyIndex)
	{
		const float Density = Variety.GrassDensity.Default;
		if (Density > 0.0f)
		{
			return Density;
		}
		return VarietyIndex == 0 ? 25.0f : 200.0f;
	}

	UMaterialInterface* ResolveNativeGrassMaterial(const FGrassVariety& Variety, int32 VarietyIndex, UMaterialInterface* NativeMaterial, UMaterialInterface* NativeDarkerMaterial)
	{
		for (UMaterialInterface* OverrideMaterial : Variety.OverrideMaterials)
		{
			if (OverrideMaterial)
			{
				return OverrideMaterial;
			}
		}
		return VarietyIndex == 1 && NativeDarkerMaterial ? NativeDarkerMaterial : NativeMaterial;
	}

	bool SpawnFoliageInteractionCookProbe(
		UWorld* World,
		const FPipelinePatchSpec& Patch,
		const FVector& SurfaceCenter,
		UStaticMesh* PlaneMesh,
		UMaterialInterface* UpdateMaterial);

	int32 AddNativeGrassTypeInstances(
		UWorld* World,
		UHierarchicalInstancedStaticMeshComponent* Component,
		const FGrassVariety& Variety,
		const FVector& SurfaceCenter,
		int32 VarietyIndex,
		int32 SeedOffset)
	{
		if (!Component)
		{
			return 0;
		}

		const float Density = FMath::Clamp(GetVarietyDensity(Variety, VarietyIndex), 5.0f, 260.0f);
		const float NativeSpacing = FMath::Sqrt(100000.0f / Density) * 1.45f;
		const float Spacing = FMath::Clamp(NativeSpacing, 42.0f, 285.0f);
		constexpr float Radius = 1260.0f;
		const int32 GridRadius = FMath::CeilToInt(Radius / Spacing);
		int32 Added = 0;

		for (int32 GridY = -GridRadius; GridY <= GridRadius; ++GridY)
		{
			for (int32 GridX = -GridRadius; GridX <= GridRadius; ++GridX)
			{
				const int32 Seed = SeedOffset + GridX * 73471 + GridY * 912367;
				const FVector2D Offset(
					GridX * Spacing + FMath::Lerp(-Spacing * Variety.PlacementJitter * 0.5f, Spacing * Variety.PlacementJitter * 0.5f, PseudoRandom01(Seed + 13)),
					GridY * Spacing + FMath::Lerp(-Spacing * Variety.PlacementJitter * 0.5f, Spacing * Variety.PlacementJitter * 0.5f, PseudoRandom01(Seed + 29)));
				if (Offset.Size() > Radius)
				{
					continue;
				}

				const float Yaw = Variety.RandomRotation ? PseudoRandom01(Seed + 61) * 360.0f : 0.0f;
				FVector InstanceLocation = SurfaceCenter + FVector(Offset.X, Offset.Y, 2.0f);
				FQuat InstanceRotation = FRotator(0.0f, Yaw, 0.0f).Quaternion();
				if (World)
				{
					FHitResult GroundHit;
					if (GetLandscapeGround(World, FVector2D(InstanceLocation.X, InstanceLocation.Y), GroundHit))
					{
						InstanceLocation = GroundHit.ImpactPoint + FVector(0.0f, 0.0f, 2.0f);
						const FQuat SurfaceRotation = FRotationMatrix::MakeFromZ(GroundHit.ImpactNormal).ToQuat();
						const FQuat YawAroundSurface(GroundHit.ImpactNormal, FMath::DegreesToRadians(Yaw));
						InstanceRotation = YawAroundSurface * SurfaceRotation;
					}
				}

				const float XScale = FMath::Lerp(Variety.ScaleX.Min, Variety.ScaleX.Max, PseudoRandom01(Seed + 43));
				const float YScale = FMath::Lerp(Variety.ScaleY.Min > 0.0f ? Variety.ScaleY.Min : 1.0f, Variety.ScaleY.Max > 0.0f ? Variety.ScaleY.Max : 1.0f, PseudoRandom01(Seed + 47));
				const float ZScale = FMath::Lerp(Variety.ScaleZ.Min > 0.0f ? Variety.ScaleZ.Min : 0.5f, Variety.ScaleZ.Max > 0.0f ? Variety.ScaleZ.Max : 1.0f, PseudoRandom01(Seed + 59));

				FTransform InstanceTransform;
				InstanceTransform.SetLocation(InstanceLocation);
				InstanceTransform.SetRotation(InstanceRotation);
				InstanceTransform.SetScale3D(FVector(XScale, YScale, ZScale));
				Component->AddInstance(InstanceTransform, true);
				++Added;
			}
		}

		Component->BuildTreeIfOutdated(true, true);
		Component->MarkPackageDirty();
		return Added;
	}

	int32 CreateNativePipelinePatch(
		UWorld* World,
		const FPipelinePatchSpec& Patch,
		UMaterialInterface* TitanGroundMaterial,
		const TArray<ULandscapeLayerInfoObject*>& LayerInfos,
		URuntimeVirtualTexture* RVTD,
		URuntimeVirtualTexture* RVTH,
		URuntimeVirtualTexture* RVTDH,
		ULandscapeGrassType* GrassType,
		UMaterialInterface* NativeMaterial,
		UMaterialInterface* NativeDarkerMaterial,
		UMaterialInterface* FoliageUpdateMaterial,
		UStaticMesh* ProbePlaneMesh,
		int32& OutLandscapes,
		int32& OutRVTVolumes,
		int32& OutFoliageInteractionProbes)
	{
		FVector SurfaceCenter;
		ALandscape* LandscapePatch = CreateNativeLandscapePatch(World, Patch, TitanGroundMaterial, LayerInfos, RVTD, RVTH, RVTDH, SurfaceCenter);
		if (!LandscapePatch)
		{
			return 0;
		}
		++OutLandscapes;

		if (SpawnRVTVolume(World, FString::Printf(TEXT("%s_RVT_Titan_D"), Patch.Label), SurfaceCenter, RVTD))
		{
			++OutRVTVolumes;
		}
		if (SpawnRVTVolume(World, FString::Printf(TEXT("%s_RVT_Titan_H"), Patch.Label), SurfaceCenter, RVTH))
		{
			++OutRVTVolumes;
		}
		if (RVTDH && SpawnRVTVolume(World, FString::Printf(TEXT("%s_RVT_Titan_DH"), Patch.Label), SurfaceCenter, RVTDH))
		{
			++OutRVTVolumes;
		}
		if (SpawnFoliageInteractionCookProbe(World, Patch, SurfaceCenter, ProbePlaneMesh, FoliageUpdateMaterial))
		{
			++OutFoliageInteractionProbes;
			UE_LOG(LogTemp, Display, TEXT("FFTitanGrasslandPipelineProof: foliageInteractionCookProbe label=%s material=%s"),
				Patch.Label,
				FoliageUpdateMaterial ? *FoliageUpdateMaterial->GetPathName() : TEXT("None"));
		}

		AActor* GrassActor = SpawnProofActor(World, FString::Printf(TEXT("%s_LGT_Grass_Reproduction"), Patch.Label), SurfaceCenter);
		if (!GrassActor || !GrassType)
		{
			return 0;
		}

		int32 TotalInstances = 0;
		for (int32 VarietyIndex = 0; VarietyIndex < GrassType->GrassVarieties.Num(); ++VarietyIndex)
		{
			const FGrassVariety& Variety = GrassType->GrassVarieties[VarietyIndex];
			if (!Variety.GrassMesh)
			{
				continue;
			}

			UMaterialInterface* Material = ResolveNativeGrassMaterial(Variety, VarietyIndex, NativeMaterial, NativeDarkerMaterial);
			const int32 StartCull = FMath::Max(0, Variety.GetStartCullDistance());
			const int32 EndCull = FMath::Max(10000, Variety.GetEndCullDistance());
			UHierarchicalInstancedStaticMeshComponent* Component = CreateHISM(
				GrassActor,
				Variety.GrassMesh,
				Material,
				FString::Printf(TEXT("LGT_Grass_Variety_%d_%s"), VarietyIndex, *Variety.GrassMesh->GetName()),
				StartCull,
				EndCull);
			const int32 Added = AddNativeGrassTypeInstances(World, Component, Variety, SurfaceCenter, VarietyIndex, 9000000 + VarietyIndex * 100000);
			TotalInstances += Added;
			UE_LOG(LogTemp, Display, TEXT("FFTitanGrasslandPipelineProof: nativeVariety=%d mesh=%s material=%s density=%.2f instances=%d cull=%d-%d scaleX=%.2f..%.2f scaleY=%.2f..%.2f scaleZ=%.2f..%.2f"),
				VarietyIndex,
				Variety.GrassMesh ? *Variety.GrassMesh->GetPathName() : TEXT("None"),
				Material ? *Material->GetPathName() : TEXT("mesh default"),
				GetVarietyDensity(Variety, VarietyIndex),
				Added,
				StartCull,
				EndCull,
				Variety.ScaleX.Min,
				Variety.ScaleX.Max,
				Variety.ScaleY.Min,
				Variety.ScaleY.Max,
				Variety.ScaleZ.Min,
				Variety.ScaleZ.Max);
		}

		GrassActor->MarkPackageDirty();
		return TotalInstances;
	}

	bool SpawnFoliageInteractionCookProbe(
		UWorld* World,
		const FPipelinePatchSpec& Patch,
		const FVector& SurfaceCenter,
		UStaticMesh* PlaneMesh,
		UMaterialInterface* UpdateMaterial)
	{
		if (!World || Patch.Kind != EPipelinePatchKind::NativeTitanParity || !PlaneMesh || !UpdateMaterial)
		{
			return false;
		}

		AActor* ProbeActor = SpawnProofActor(World, FString::Printf(TEXT("%s_FoliageInteractionCookProbe"), Patch.Label), SurfaceCenter + FVector(0.0f, 0.0f, 70.0f));
		if (!ProbeActor)
		{
			return false;
		}

		ProbeActor->SetActorHiddenInGame(true);
		UStaticMeshComponent* ProbeMesh = NewObject<UStaticMeshComponent>(ProbeActor, TEXT("M_UpdateFoliageDraw_Reference"), RF_Transactional);
		if (!ProbeMesh)
		{
			return false;
		}

		ProbeMesh->SetStaticMesh(PlaneMesh);
		ProbeMesh->SetMaterial(0, UpdateMaterial);
		ProbeMesh->SetMobility(EComponentMobility::Static);
		ProbeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ProbeMesh->SetVisibility(false, true);
		ProbeMesh->SetupAttachment(ProbeActor->GetRootComponent());
		ProbeMesh->RegisterComponent();
		ProbeActor->AddInstanceComponent(ProbeMesh);
		ProbeActor->MarkPackageDirty();
		return true;
	}

	void CreateComparisonCamera(UWorld* World, const FPipelinePatchSpec& Patch)
	{
		if (!World)
		{
			return;
		}

		FHitResult GroundHit;
		FVector LookAt(Patch.Center.X, Patch.Center.Y, -3200.0f);
		if (GetLandscapeGround(World, Patch.Center, GroundHit))
		{
			LookAt = GroundHit.ImpactPoint + FVector(0.0f, 0.0f, 80.0f);
		}

		const FVector CameraLocation = Patch.CameraTag == FString(TEXT("FFSmokeTitanPipelineTopDownCamera"))
			? FVector(61200.0f, 128400.0f, 7800.0f)
			: LookAt + FVector(-1450.0f, -980.0f, 620.0f);
		const FRotator CameraRotation = Patch.CameraTag == FString(TEXT("FFSmokeTitanPipelineTopDownCamera"))
			? FRotator(-90.0f, 0.0f, 0.0f)
			: (LookAt - CameraLocation).Rotation();

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ACameraActor* CameraActor = World->SpawnActor<ACameraActor>(CameraLocation, CameraRotation, SpawnParameters);
		if (!CameraActor)
		{
			return;
		}

		CameraActor->Modify();
		CameraActor->Tags.AddUnique(ProofTag);
		CameraActor->Tags.AddUnique(FName(Patch.CameraTag));
		CameraActor->SetActorLabel(FString::Printf(TEXT("%s_Camera"), Patch.Label));
		if (UCameraComponent* CameraComponent = CameraActor->GetCameraComponent())
		{
			CameraComponent->ProjectionMode = ECameraProjectionMode::Perspective;
			CameraComponent->FieldOfView = Patch.CameraTag == FString(TEXT("FFSmokeTitanPipelineTopDownCamera")) ? 72.0f : 38.0f;
		}
		CameraActor->MarkPackageDirty();
	}
}
#endif

UFFTitanGrasslandPipelineProofCommandlet::UFFTitanGrasslandPipelineProofCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

int32 UFFTitanGrasslandPipelineProofCommandlet::Main(const FString& Params)
{
#if WITH_EDITOR
	UE_LOG(LogTemp, Display, TEXT("FFTitanGrasslandPipelineProof: loading %s"), HighlandMapPath);
	UWorld* World = UEditorLoadingAndSavingUtils::LoadMap(HighlandMapPath);
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("FFTitanGrasslandPipelineProof: failed to load map."));
		return 1;
	}

	int32 RemovedActors = 0;
	RemovePreviousProofActors(World, RemovedActors);

	UMaterialInterface* ProxyMaterial = LoadObject<UMaterialInterface>(nullptr, ProxyGrassMaterialPath);
	UMaterialInterface* NativeMaterial = LoadObject<UMaterialInterface>(nullptr, NativeGrassMaterialPath);
	UMaterialInterface* NativeDarkerMaterial = LoadObject<UMaterialInterface>(nullptr, NativeGrassDarkerMaterialPath);
	UMaterialInterface* TitanGroundMaterial = LoadObject<UMaterialInterface>(nullptr, TitanGroundMaterialPath);
	ULandscapeGrassType* TitanGrassType = LoadObject<ULandscapeGrassType>(nullptr, TitanGrassTypePath);
	URuntimeVirtualTexture* RVTD = LoadObject<URuntimeVirtualTexture>(nullptr, TitanRVTDPath);
	URuntimeVirtualTexture* RVTH = LoadObject<URuntimeVirtualTexture>(nullptr, TitanRVTHPath);
	URuntimeVirtualTexture* RVTDH = LoadObject<URuntimeVirtualTexture>(nullptr, TitanRVTDHPath);
	UMaterialInterface* FoliageUpdateMaterial = LoadObject<UMaterialInterface>(nullptr, FoliageUpdateMaterialPath);
	UStaticMesh* ProbePlaneMesh = LoadObject<UStaticMesh>(nullptr, EnginePlaneMeshPath);
	UObject* FoliageMPC = LoadObject<UObject>(nullptr, FoliageMPCPath);
	UObject* FoliageRT = LoadObject<UObject>(nullptr, FoliageRTPath);

	TArray<ULandscapeLayerInfoObject*> LayerInfos;
	for (const TCHAR* LayerInfoPath : GrasslandLayerInfoPaths)
	{
		LayerInfos.Add(LoadObject<ULandscapeLayerInfoObject>(nullptr, LayerInfoPath));
	}

	if (!ProxyMaterial || !NativeMaterial || !NativeDarkerMaterial || !TitanGroundMaterial || !TitanGrassType || !RVTD || !RVTH || !FoliageUpdateMaterial || !ProbePlaneMesh || LayerInfos.Contains(nullptr))
	{
		UE_LOG(LogTemp, Error, TEXT("FFTitanGrasslandPipelineProof: missing dependency proxy=%s native=%s nativeDark=%s ground=%s grassType=%s rvtD=%s rvtH=%s foliageUpdate=%s probePlane=%s layerInfos=%d"),
			ProxyMaterial ? TEXT("ok") : ProxyGrassMaterialPath,
			NativeMaterial ? TEXT("ok") : NativeGrassMaterialPath,
			NativeDarkerMaterial ? TEXT("ok") : NativeGrassDarkerMaterialPath,
			TitanGroundMaterial ? TEXT("ok") : TitanGroundMaterialPath,
			TitanGrassType ? TEXT("ok") : TitanGrassTypePath,
			RVTD ? TEXT("ok") : TitanRVTDPath,
			RVTH ? TEXT("ok") : TitanRVTHPath,
			FoliageUpdateMaterial ? TEXT("ok") : FoliageUpdateMaterialPath,
			ProbePlaneMesh ? TEXT("ok") : EnginePlaneMeshPath,
			LayerInfos.Num());
		return 1;
	}

	UStaticMesh* ProxyMesh = TitanGrassType->GrassVarieties.Num() > 0 ? TitanGrassType->GrassVarieties[0].GrassMesh : nullptr;
	if (!ProxyMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("FFTitanGrasslandPipelineProof: LGT_Grass has no usable first grass mesh."));
		return 1;
	}

	UE_LOG(LogTemp, Display, TEXT("FFTitanGrasslandPipelineProof: assets ground=%s grassType=%s varieties=%d rvtD=%s rvtH=%s rvtDH=%s mpc=%s rt=%s foliageUpdate=%s"),
		*TitanGroundMaterial->GetPathName(),
		*TitanGrassType->GetPathName(),
		TitanGrassType->GrassVarieties.Num(),
		*RVTD->GetPathName(),
		*RVTH->GetPathName(),
		RVTDH ? *RVTDH->GetPathName() : TEXT("missing"),
		FoliageMPC ? *FoliageMPC->GetPathName() : TEXT("missing"),
		FoliageRT ? *FoliageRT->GetPathName() : TEXT("missing"),
		*FoliageUpdateMaterial->GetPathName());
	for (ULandscapeLayerInfoObject* LayerInfo : LayerInfos)
	{
		UE_LOG(LogTemp, Display, TEXT("FFTitanGrasslandPipelineProof: layerInfo=%s layerName=%s"), *LayerInfo->GetPathName(), *LayerInfo->GetLayerName().ToString());
	}

	int32 ProxyInstances = 0;
	int32 NativeInstances = 0;
	int32 NativeParityInstances = 0;
	int32 NativeLandscapes = 0;
	int32 RVTVolumes = 0;
	int32 FoliageInteractionProbes = 0;
	int32 Cameras = 0;

	for (const FPipelinePatchSpec& Patch : PatchSpecs)
	{
		if (FCString::Strcmp(Patch.Label, TEXT("FF_TitanPipeline_TopDown_Reference")) == 0)
		{
			CreateComparisonCamera(World, Patch);
			++Cameras;
			continue;
		}

		if (Patch.Kind == EPipelinePatchKind::NativeTitan || Patch.Kind == EPipelinePatchKind::NativeTitanParity)
		{
			const int32 Added = CreateNativePipelinePatch(World, Patch, TitanGroundMaterial, LayerInfos, RVTD, RVTH, RVTDH, TitanGrassType, NativeMaterial, NativeDarkerMaterial, FoliageUpdateMaterial, ProbePlaneMesh, NativeLandscapes, RVTVolumes, FoliageInteractionProbes);
			if (Patch.Kind == EPipelinePatchKind::NativeTitanParity)
			{
				NativeParityInstances += Added;
			}
			else
			{
				NativeInstances += Added;
			}
			UE_LOG(LogTemp, Display, TEXT("FFTitanGrasslandPipelineProof: %sPatch label=%s ground=%s instances=%d center=(%.1f, %.1f)"),
				Patch.Kind == EPipelinePatchKind::NativeTitanParity ? TEXT("nativeParity") : TEXT("native"),
				Patch.Label,
				TitanGroundMaterial ? *TitanGroundMaterial->GetPathName() : TEXT("None"),
				Added,
				Patch.Center.X,
				Patch.Center.Y);
		}
		else
		{
			FHitResult GroundHit;
			FVector Location(Patch.Center.X, Patch.Center.Y, -3200.0f);
			if (GetLandscapeGround(World, Patch.Center, GroundHit))
			{
				Location = GroundHit.ImpactPoint;
			}
			AActor* ProxyActor = SpawnProofActor(World, Patch.Label, Location);
			UHierarchicalInstancedStaticMeshComponent* Component = CreateHISM(ProxyActor, ProxyMesh, ProxyMaterial, TEXT("CurrentHighlandProxyGrass_HISM"), 0, 12000);
			const int32 Added = AddProxyGrassInstances(World, Component, Patch.Center, Patch.bShadow ? 3200000 : 2200000);
			ProxyInstances += Added;
			UE_LOG(LogTemp, Display, TEXT("FFTitanGrasslandPipelineProof: proxyPatch label=%s instances=%d center=(%.1f, %.1f)"),
				Patch.Label,
				Added,
				Patch.Center.X,
				Patch.Center.Y);
		}

		CreateComparisonCamera(World, Patch);
		++Cameras;
	}

	World->MarkPackageDirty();
	const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
	const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
	UE_LOG(LogTemp, Display, TEXT("FFTitanGrasslandPipelineProof: removed=%d proxyInstances=%d nativeInstances=%d nativeParityInstances=%d nativeLandscapes=%d rvtVolumes=%d foliageInteractionProbes=%d cameras=%d savedMap=%s savedPackages=%s"),
		RemovedActors,
		ProxyInstances,
		NativeInstances,
		NativeParityInstances,
		NativeLandscapes,
		RVTVolumes,
		FoliageInteractionProbes,
		Cameras,
		bSavedMap ? TEXT("true") : TEXT("false"),
		bSavedPackages ? TEXT("true") : TEXT("false"));

	return (bSavedMap && bSavedPackages && ProxyInstances > 0 && NativeInstances > 0 && NativeParityInstances > 0 && NativeLandscapes == 4 && RVTVolumes >= 12 && FoliageInteractionProbes == 2) ? 0 : 1;
#else
	UE_LOG(LogTemp, Error, TEXT("FFTitanGrasslandPipelineProof can only run in editor builds."));
	return 1;
#endif
}
