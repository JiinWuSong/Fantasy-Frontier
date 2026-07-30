#include "FFTitanMainRealComponentSandboxCommandlet.h"

#if WITH_EDITOR
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/LightComponent.h"
#include "Components/RuntimeVirtualTextureComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "FileHelpers.h"
#include "GameFramework/PlayerStart.h"
#include "Landscape.h"
#include "LandscapeComponent.h"
#include "LandscapeEdit.h"
#include "LandscapeGrassType.h"
#include "LandscapeInfo.h"
#include "LandscapeLayerInfoObject.h"
#include "LandscapeProxy.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpression.h"
#include "Materials/MaterialExpressionLandscapeGrassOutput.h"
#include "Materials/MaterialExpressionLandscapeLayerBlend.h"
#include "Materials/MaterialExpressionLandscapeLayerSample.h"
#include "Materials/MaterialExpressionLandscapeLayerWeight.h"
#include "MaterialShared.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
#include "UObject/Package.h"
#include "VT/RuntimeVirtualTexture.h"
#include "VT/RuntimeVirtualTextureVolume.h"
#include "WorldPartition/LoaderAdapter/LoaderAdapterShape.h"
#include "WorldPartition/WorldPartition.h"
#include "WorldPartition/WorldPartitionActorLoaderInterface.h"
#include "WorldPartition/WorldPartitionEditorLoaderAdapter.h"
#endif

UFFTitanMainRealComponentSandboxCommandlet::UFFTitanMainRealComponentSandboxCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

#if WITH_EDITOR
namespace
{
	const TCHAR* TitanMainMapPath = TEXT("/Game/Maps/TitanMain");
	const TCHAR* SandboxMapPath = TEXT("/Game/FantasyFrontier/Test/TitanMainGrasslandRealComponentSandbox");
	const FName SandboxTag(TEXT("FFTitanMainRealComponentSandbox"));
	const TCHAR* ReportPath = TEXT("TitanMainRealComponentSandbox_Report.txt");

	const TCHAR* GrasslandLayerInfoPaths[] = {
		TEXT("/Game/Landscape/WeightLayers/LI_Grassland_Grass.LI_Grassland_Grass"),
		TEXT("/Game/Landscape/WeightLayers/LI_Grassland_Dirt.LI_Grassland_Dirt"),
		TEXT("/Game/Landscape/WeightLayers/LI_Grassland_Rock.LI_Grassland_Rock"),
		TEXT("/Game/Landscape/WeightLayers/LI_Grassland_Sand.LI_Grassland_Sand")
	};

	const TCHAR* FallbackRVTDPath = TEXT("/Game/Landscape/RVT/RVT_Titan_D.RVT_Titan_D");
	const TCHAR* FallbackRVTHPath = TEXT("/Game/Landscape/RVT/RVT_Titan_H.RVT_Titan_H");
	const TCHAR* FallbackRVTDHPath = TEXT("/Game/Landscape/RVT/RVT_Titan_DH.RVT_Titan_DH");

	template <typename T>
	T* LoadObjectCheckedSoft(const TCHAR* Path)
	{
		return LoadObject<T>(nullptr, Path);
	}

	FString ObjectPath(const UObject* Object)
	{
		return Object ? Object->GetPathName() : FString(TEXT("None"));
	}

	FString SavedReportPath()
	{
		return FPaths::Combine(FPaths::ProjectSavedDir(), ReportPath);
	}

	FString LayerInfoDebugName(const ULandscapeLayerInfoObject* LayerInfo)
	{
		if (!LayerInfo)
		{
			return TEXT("None");
		}
		const FString LayerInfoPath = ObjectPath(LayerInfo);
		return FString::Printf(TEXT("asset=`%s` objectName=`%s` layerName=`%s`"),
			*LayerInfoPath,
			*LayerInfo->GetName(),
			*LayerInfo->GetLayerName().ToString());
	}

	FName CanonicalImportLayerName(const ULandscapeLayerInfoObject* LayerInfo)
	{
		if (!LayerInfo)
		{
			return NAME_None;
		}

		if (!LayerInfo->GetLayerName().IsNone())
		{
			return LayerInfo->GetLayerName();
		}

		FString ObjectName = LayerInfo->GetName();
		if (ObjectName.StartsWith(TEXT("LI_")))
		{
			ObjectName.RightChopInline(3);
		}
		return FName(*ObjectName);
	}

	FString ActorLabelOrName(const AActor* Actor)
	{
		return Actor ? Actor->GetActorLabel() : FString(TEXT("None"));
	}

	float AverageWeight(const TArray<uint8>& Data)
	{
		if (Data.Num() == 0)
		{
			return 0.0f;
		}

		uint64 Sum = 0;
		for (uint8 Value : Data)
		{
			Sum += Value;
		}
		return static_cast<float>(Sum) / static_cast<float>(Data.Num());
	}

	uint8 MaxWeight(const TArray<uint8>& Data)
	{
		uint8 MaxValue = 0;
		for (uint8 Value : Data)
		{
			MaxValue = FMath::Max(MaxValue, Value);
		}
		return MaxValue;
	}

	int32 CountNonZeroWeights(const TArray<uint8>& Data)
	{
		int32 Count = 0;
		for (const uint8 Value : Data)
		{
			if (Value > 0)
			{
				++Count;
			}
		}
		return Count;
	}

	UWorldPartitionEditorLoaderAdapter* LoadWorldPartitionCells(UWorld* World, TArray<FString>& Notes)
	{
		if (!World)
		{
			return nullptr;
		}

		UWorldPartition* WorldPartition = World->GetWorldPartition();
		if (!WorldPartition)
		{
			Notes.Add(TEXT("TitanMain loaded as a non-World-Partition map; scanning loaded actors only."));
			return nullptr;
		}

		constexpr double RegionRadius = 500000.0;
		constexpr double RegionHeight = 200000.0;
		const FBox LoadCellsBox(
			FVector(-RegionRadius, -RegionRadius, -RegionHeight),
			FVector(RegionRadius, RegionRadius, RegionHeight));

		UWorldPartitionEditorLoaderAdapter* EditorLoaderAdapter =
			WorldPartition->CreateEditorLoaderAdapter<FLoaderAdapterShape>(World, LoadCellsBox, TEXT("TitanMain Real Component Extract"));
		if (!EditorLoaderAdapter || !EditorLoaderAdapter->GetLoaderAdapter())
		{
			Notes.Add(TEXT("World Partition loader adapter could not be created; extraction cannot see external actors."));
			return nullptr;
		}

		EditorLoaderAdapter->GetLoaderAdapter()->Load();
		Notes.Add(FString::Printf(TEXT("Loaded TitanMain WP cells in bounds min=%s max=%s."),
			*LoadCellsBox.Min.ToString(),
			*LoadCellsBox.Max.ToString()));
		return EditorLoaderAdapter;
	}

	void ReleaseWorldPartitionCells(UWorld* World, UWorldPartitionEditorLoaderAdapter* EditorLoaderAdapter)
	{
		if (World && EditorLoaderAdapter)
		{
			if (UWorldPartition* WorldPartition = World->GetWorldPartition())
			{
				WorldPartition->ReleaseEditorLoaderAdapter(EditorLoaderAdapter);
			}
		}
	}

	struct FLayerExtract
	{
		ULandscapeLayerInfoObject* LayerInfo = nullptr;
		TArray<uint8> Data;
		float Average = 0.0f;
		uint8 Max = 0;
	};

	struct FComponentExtract
	{
		ALandscapeProxy* SourceProxy = nullptr;
		ULandscapeComponent* SourceComponent = nullptr;
		FString SourceProxyLabel;
		FString SourceComponentName;
		FIntRect Extent;
		int32 SizeX = 0;
		int32 SizeY = 0;
		int32 NumSubsections = 1;
		int32 SubsectionSizeQuads = 63;
		int32 ComponentSizeQuads = 63;
		FVector SourceScale = FVector(100.0f, 100.0f, 100.0f);
		FVector SourceWorldCenter = FVector::ZeroVector;
		UMaterialInterface* LandscapeMaterial = nullptr;
		TArray<URuntimeVirtualTexture*> RuntimeVirtualTextures;
		ERuntimeVirtualTextureMainPassType VirtualTextureRenderPassType = ERuntimeVirtualTextureMainPassType::Always;
		TArray<uint16> HeightData;
		TArray<FLayerExtract> Layers;
		float GrassAverage = 0.0f;
		float DirtAverage = 0.0f;
		float RockAverage = 0.0f;
		float SandAverage = 0.0f;
		float Score = -1.0f;
	};

	bool ExtractLayerData(FLandscapeEditDataInterface& EditData, ULandscapeLayerInfoObject* LayerInfo, const FIntRect& Extent, FLayerExtract& OutLayer)
	{
		const int32 SampleSizeX = Extent.Width() + 1;
		const int32 SampleSizeY = Extent.Height() + 1;
		if (!LayerInfo || SampleSizeX <= 1 || SampleSizeY <= 1)
		{
			return false;
		}

		OutLayer.LayerInfo = LayerInfo;
		OutLayer.Data.Init(0, SampleSizeX * SampleSizeY);
		EditData.GetWeightDataFast(LayerInfo, Extent.Min.X, Extent.Min.Y, Extent.Max.X, Extent.Max.Y, OutLayer.Data.GetData(), 0);
		OutLayer.Average = AverageWeight(OutLayer.Data);
		OutLayer.Max = MaxWeight(OutLayer.Data);
		return OutLayer.Max > 0;
	}

	bool ExtractComponentData(ULandscapeComponent* Component, const TArray<ULandscapeLayerInfoObject*>& GrasslandLayerInfos, FComponentExtract& OutExtract)
	{
		if (!Component || !Component->GetOwner() || !Component->GetLandscapeInfo())
		{
			return false;
		}

		ALandscapeProxy* Proxy = Cast<ALandscapeProxy>(Component->GetOwner());
		if (!Proxy || !Proxy->LandscapeMaterial)
		{
			return false;
		}

		FIntRect Extent = Component->GetComponentExtent();
		const int32 SampleSizeX = Extent.Width() + 1;
		const int32 SampleSizeY = Extent.Height() + 1;
		if (SampleSizeX <= 1 || SampleSizeY <= 1)
		{
			return false;
		}

		FLandscapeEditDataInterface EditData(Component->GetLandscapeInfo());
		TArray<uint16> HeightData;
		HeightData.Init(32768, SampleSizeX * SampleSizeY);
		EditData.GetHeightDataFast(Extent.Min.X, Extent.Min.Y, Extent.Max.X, Extent.Max.Y, HeightData.GetData(), 0);

		TMap<ULandscapeLayerInfoObject*, FLayerExtract> LayerMap;
		auto AddLayer = [&](ULandscapeLayerInfoObject* LayerInfo)
		{
			if (!LayerInfo || LayerMap.Contains(LayerInfo))
			{
				return;
			}

			FLayerExtract Layer;
			if (ExtractLayerData(EditData, LayerInfo, Extent, Layer))
			{
				LayerMap.Add(LayerInfo, MoveTemp(Layer));
			}
		};

		for (ULandscapeLayerInfoObject* LayerInfo : GrasslandLayerInfos)
		{
			AddLayer(LayerInfo);
		}

		for (const FWeightmapLayerAllocationInfo& Allocation : Component->GetWeightmapLayerAllocations())
		{
			AddLayer(Allocation.LayerInfo);
		}

		FLayerExtract* GrassLayer = GrasslandLayerInfos.Num() > 0 ? LayerMap.Find(GrasslandLayerInfos[0]) : nullptr;
		FLayerExtract* DirtLayer = GrasslandLayerInfos.Num() > 1 ? LayerMap.Find(GrasslandLayerInfos[1]) : nullptr;
		FLayerExtract* RockLayer = GrasslandLayerInfos.Num() > 2 ? LayerMap.Find(GrasslandLayerInfos[2]) : nullptr;
		FLayerExtract* SandLayer = GrasslandLayerInfos.Num() > 3 ? LayerMap.Find(GrasslandLayerInfos[3]) : nullptr;

		const float GrassAverage = GrassLayer ? GrassLayer->Average : 0.0f;
		const float DirtAverage = DirtLayer ? DirtLayer->Average : 0.0f;
		const float RockAverage = RockLayer ? RockLayer->Average : 0.0f;
		const float SandAverage = SandLayer ? SandLayer->Average : 0.0f;
		const float GrassMax = GrassLayer ? static_cast<float>(GrassLayer->Max) : 0.0f;

		if (GrassMax <= 0.0f || GrassAverage < 15.0f)
		{
			return false;
		}

		OutExtract.SourceProxy = Proxy;
		OutExtract.SourceComponent = Component;
		OutExtract.SourceProxyLabel = ActorLabelOrName(Proxy);
		OutExtract.SourceComponentName = Component->GetName();
		OutExtract.Extent = Extent;
		OutExtract.SizeX = SampleSizeX;
		OutExtract.SizeY = SampleSizeY;
		OutExtract.NumSubsections = Proxy->NumSubsections;
		OutExtract.SubsectionSizeQuads = Proxy->SubsectionSizeQuads;
		OutExtract.ComponentSizeQuads = Proxy->ComponentSizeQuads;
		OutExtract.SourceScale = Proxy->GetActorScale3D();
		OutExtract.SourceWorldCenter = Component->Bounds.Origin;
		OutExtract.LandscapeMaterial = Proxy->LandscapeMaterial;
		OutExtract.RuntimeVirtualTextures = Proxy->RuntimeVirtualTextures;
		OutExtract.VirtualTextureRenderPassType = Proxy->VirtualTextureRenderPassType;
		OutExtract.HeightData = MoveTemp(HeightData);
		OutExtract.GrassAverage = GrassAverage;
		OutExtract.DirtAverage = DirtAverage;
		OutExtract.RockAverage = RockAverage;
		OutExtract.SandAverage = SandAverage;
		OutExtract.Score = GrassAverage * 4.0f + DirtAverage * 0.75f + SandAverage * 0.25f - RockAverage * 0.35f;

		LayerMap.GenerateValueArray(OutExtract.Layers);
		OutExtract.Layers.Sort([](const FLayerExtract& A, const FLayerExtract& B)
		{
			return ObjectPath(A.LayerInfo) < ObjectPath(B.LayerInfo);
		});

		return true;
	}

	bool FindBestTitanMainGrasslandComponent(UWorld* SourceWorld, const TArray<ULandscapeLayerInfoObject*>& GrasslandLayerInfos, FComponentExtract& OutBest, TArray<FComponentExtract>& OutTopCandidates)
	{
		if (!SourceWorld)
		{
			return false;
		}

		TArray<FComponentExtract> Candidates;
		for (TActorIterator<ALandscapeProxy> It(SourceWorld); It; ++It)
		{
			ALandscapeProxy* Proxy = *It;
			if (!Proxy || !Proxy->LandscapeMaterial)
			{
				continue;
			}

			for (ULandscapeComponent* Component : Proxy->LandscapeComponents)
			{
				FComponentExtract Extract;
				if (ExtractComponentData(Component, GrasslandLayerInfos, Extract))
				{
					Candidates.Add(MoveTemp(Extract));
				}
			}
		}

		if (Candidates.Num() == 0)
		{
			return false;
		}

		Candidates.Sort([](const FComponentExtract& A, const FComponentExtract& B)
		{
			return A.Score > B.Score;
		});

		OutBest = Candidates[0];
		const int32 TopCount = FMath::Min(8, Candidates.Num());
		for (int32 Index = 0; Index < TopCount; ++Index)
		{
			OutTopCandidates.Add(Candidates[Index]);
		}
		return true;
	}

	void AppendMaterialLayerDiagnostics(FString& Report, UMaterialInterface* MaterialInterface)
	{
		Report += TEXT("## Material Landscape/Grass Expressions\n");
		const FString MaterialInterfacePath = ObjectPath(MaterialInterface);
		Report += FString::Printf(TEXT("- materialInterface=`%s`\n"), *MaterialInterfacePath);
		UMaterial* Material = MaterialInterface ? MaterialInterface->GetMaterial() : nullptr;
		const FString BaseMaterialPath = ObjectPath(Material);
		Report += FString::Printf(TEXT("- baseMaterial=`%s`\n"), *BaseMaterialPath);
		if (!Material)
		{
			Report += TEXT("- no base material; cannot inspect expressions.\n\n");
			return;
		}

		TArray<UMaterialExpressionLandscapeGrassOutput*> GrassOutputs;
		Material->GetAllExpressionsInMaterialAndFunctionsOfType(GrassOutputs);
		Report += FString::Printf(TEXT("- LandscapeGrassOutput nodes=%d\n"), GrassOutputs.Num());
		for (UMaterialExpressionLandscapeGrassOutput* GrassOutput : GrassOutputs)
		{
			if (!GrassOutput)
			{
				continue;
			}
			for (const FGrassInput& GrassInput : GrassOutput->GrassTypes)
			{
				const FString GrassTypePath = ObjectPath(GrassInput.GrassType);
				Report += FString::Printf(TEXT("  - grassInput name=`%s` grassType=`%s` inputConnected=%d\n"),
					*GrassInput.Name.ToString(),
					*GrassTypePath,
					GrassInput.Input.IsConnected() ? 1 : 0);
			}
		}

		TArray<UMaterialExpressionLandscapeLayerSample*> LayerSamples;
		Material->GetAllExpressionsInMaterialAndFunctionsOfType(LayerSamples);
		LayerSamples.Sort([](const UMaterialExpressionLandscapeLayerSample& A, const UMaterialExpressionLandscapeLayerSample& B)
		{
			return A.ParameterName.LexicalLess(B.ParameterName);
		});
		Report += FString::Printf(TEXT("- LandscapeLayerSample names=%d\n"), LayerSamples.Num());
		for (UMaterialExpressionLandscapeLayerSample* Sample : LayerSamples)
		{
			Report += FString::Printf(TEXT("  - sample `%s` preview=%.2f\n"),
				Sample ? *Sample->ParameterName.ToString() : TEXT("None"),
				Sample ? Sample->PreviewWeight : 0.0f);
		}

		TArray<UMaterialExpressionLandscapeLayerWeight*> LayerWeights;
		Material->GetAllExpressionsInMaterialAndFunctionsOfType(LayerWeights);
		LayerWeights.Sort([](const UMaterialExpressionLandscapeLayerWeight& A, const UMaterialExpressionLandscapeLayerWeight& B)
		{
			return A.ParameterName.LexicalLess(B.ParameterName);
		});
		Report += FString::Printf(TEXT("- LandscapeLayerWeight names=%d\n"), LayerWeights.Num());
		for (UMaterialExpressionLandscapeLayerWeight* Weight : LayerWeights)
		{
			Report += FString::Printf(TEXT("  - weight `%s` preview=%.2f\n"),
				Weight ? *Weight->ParameterName.ToString() : TEXT("None"),
				Weight ? Weight->PreviewWeight : 0.0f);
		}

		TArray<UMaterialExpressionLandscapeLayerBlend*> LayerBlends;
		Material->GetAllExpressionsInMaterialAndFunctionsOfType(LayerBlends);
		Report += FString::Printf(TEXT("- LandscapeLayerBlend nodes=%d\n"), LayerBlends.Num());
		for (UMaterialExpressionLandscapeLayerBlend* Blend : LayerBlends)
		{
			if (!Blend)
			{
				continue;
			}
			for (const FLayerBlendInput& Layer : Blend->Layers)
			{
				Report += FString::Printf(TEXT("  - blendLayer `%s` type=%d preview=%.2f\n"),
					*Layer.LayerName.ToString(),
					static_cast<int32>(Layer.BlendType.GetValue()),
					Layer.PreviewWeight);
			}
		}
		Report += TEXT("\n");
	}

	void AppendComponentLayerDiagnostics(FString& Report, const FString& Label, ULandscapeComponent* Component)
	{
		Report += FString::Printf(TEXT("## %s Component Layer State\n"), *Label);
		if (!Component)
		{
			Report += TEXT("- component=None\n\n");
			return;
		}

		ALandscapeProxy* Proxy = Cast<ALandscapeProxy>(Component->GetOwner());
		ALandscape* LandscapeActor = Proxy ? Proxy->GetLandscapeActor() : nullptr;
		Report += FString::Printf(TEXT("- proxy=`%s` component=`%s` extent=(%d,%d)-(%d,%d)\n"),
			*ActorLabelOrName(Proxy),
			*Component->GetName(),
			Component->GetComponentExtent().Min.X,
			Component->GetComponentExtent().Min.Y,
			Component->GetComponentExtent().Max.X,
			Component->GetComponentExtent().Max.Y);
		const FString LandscapeMaterialPath = ObjectPath(Proxy ? Proxy->LandscapeMaterial : nullptr);
		Report += FString::Printf(TEXT("- landscapeMaterial=`%s`\n"), *LandscapeMaterialPath);
		if (LandscapeActor)
		{
			Report += FString::Printf(TEXT("- editLayers count=%d selectedIndex=%d canHaveLayersContent=%d hasLayersContent=%d\n"),
				LandscapeActor->GetEditLayersConst().Num(),
				LandscapeActor->GetSelectedEditLayerIndex(),
				LandscapeActor->CanHaveLayersContent() ? 1 : 0,
				LandscapeActor->HasLayersContent() ? 1 : 0);
		}

		if (ULandscapeInfo* Info = Component->GetLandscapeInfo())
		{
			FLandscapeEditDataInterface EditData(Info);
			const FIntRect Extent = Component->GetComponentExtent();
			for (const FWeightmapLayerAllocationInfo& Allocation : Component->GetWeightmapLayerAllocations())
			{
				TArray<uint8> LayerData;
				const int32 SampleSize = (Extent.Width() + 1) * (Extent.Height() + 1);
				LayerData.Init(0, SampleSize);
				if (Allocation.LayerInfo)
				{
					EditData.GetWeightDataFast(Allocation.LayerInfo, Extent.Min.X, Extent.Min.Y, Extent.Max.X, Extent.Max.Y, LayerData.GetData(), 0);
				}
				Report += FString::Printf(TEXT("- allocation %s textureIndex=%d channel=%d avg=%.2f max=%d nonZero=%d canonicalImport=`%s`\n"),
					*LayerInfoDebugName(Allocation.LayerInfo),
					Allocation.WeightmapTextureIndex,
					static_cast<int32>(Allocation.WeightmapTextureChannel),
					AverageWeight(LayerData),
					MaxWeight(LayerData),
					CountNonZeroWeights(LayerData),
					*CanonicalImportLayerName(Allocation.LayerInfo).ToString());
			}
		}

		Component->UpdateGrassTypes(true);
		int64 GrassWeightBytes = 0;
		int64 NonZeroGrassWeightSamples = 0;
		if (Component->GrassData->NumElements > 0)
		{
			GrassWeightBytes = Component->GrassData->HeightWeightData.Num()
				- (Component->GrassData->NumElements * static_cast<int64>(sizeof(uint16)));
			for (const TPair<TObjectPtr<ULandscapeGrassType>, int32>& Offset : Component->GrassData->WeightOffsets)
			{
				const int32 WeightSampleCount = Component->GrassData->NumElements;
				const bool bValidWeightRange = WeightSampleCount > 0
					&& Offset.Value >= 0
					&& (Offset.Value + WeightSampleCount) <= Component->GrassData->HeightWeightData.Num();
				const uint8* WeightData = bValidWeightRange
					? Component->GrassData->HeightWeightData.GetData() + Offset.Value
					: nullptr;
				for (int32 WeightIndex = 0; WeightIndex < WeightSampleCount && WeightData; ++WeightIndex)
				{
					if (WeightData[WeightIndex] != 0)
					{
						++NonZeroGrassWeightSamples;
					}
				}
			}
		}
		Report += FString::Printf(TEXT("- cachedGrassTypes=%d materialHasGrass=%d grassDataElements=%d heightWeightBytes=%d grassWeightBytes=%lld weightOffsets=%d nonZeroWeightSamples=%lld\n"),
			Component->GetGrassTypes().Num(),
			Component->MaterialHasGrass() ? 1 : 0,
			Component->GrassData->NumElements,
			Component->GrassData->HeightWeightData.Num(),
			GrassWeightBytes,
			Component->GrassData->WeightOffsets.Num(),
			NonZeroGrassWeightSamples);
		for (const TObjectPtr<ULandscapeGrassType>& GrassType : Component->GetGrassTypes())
		{
			const FString GrassTypePath = ObjectPath(GrassType.Get());
			Report += FString::Printf(TEXT("  - grassType `%s`\n"), *GrassTypePath);
		}
		for (const TPair<TObjectPtr<ULandscapeGrassType>, int32>& Offset : Component->GrassData->WeightOffsets)
		{
			const FString GrassTypePath = ObjectPath(Offset.Key.Get());
			Report += FString::Printf(TEXT("  - grassWeightOffset grassType=`%s` offset=%d\n"), *GrassTypePath, Offset.Value);
		}
		Report += TEXT("\n");
	}

	int32 RunSandboxLayerDiagOnly(TArray<FString>& Notes)
	{
		TArray<ULandscapeLayerInfoObject*> GrasslandLayerInfos;
		for (const TCHAR* LayerPath : GrasslandLayerInfoPaths)
		{
			if (ULandscapeLayerInfoObject* LayerInfo = LoadObjectCheckedSoft<ULandscapeLayerInfoObject>(LayerPath))
			{
				GrasslandLayerInfos.Add(LayerInfo);
				Notes.Add(FString::Printf(TEXT("Loaded expected Grassland layer info %s canonicalImport=`%s`."),
					*LayerInfoDebugName(LayerInfo),
					*CanonicalImportLayerName(LayerInfo).ToString()));
			}
			else
			{
				Notes.Add(FString::Printf(TEXT("Missing expected Grassland layer info `%s`."), LayerPath));
			}
		}

		UWorld* SandboxWorld = UEditorLoadingAndSavingUtils::LoadMap(SandboxMapPath);
		ALandscape* SandboxLandscape = nullptr;
		if (SandboxWorld)
		{
			for (TActorIterator<ALandscape> It(SandboxWorld); It; ++It)
			{
				SandboxLandscape = *It;
				break;
			}
		}
		ULandscapeComponent* SandboxComponent = (SandboxLandscape && SandboxLandscape->LandscapeComponents.Num() > 0)
			? SandboxLandscape->LandscapeComponents[0]
			: nullptr;

		FString Report = TEXT("# TitanMain Real Component Sandbox Layer Diagnostics\n\n");
		Report += TEXT("Mode: sandbox-only read-only diagnostics. TitanMain source cells were not loaded and no map/package was modified.\n\n");
		Report += TEXT("## Notes\n");
		for (const FString& Note : Notes)
		{
			Report += FString::Printf(TEXT("- %s\n"), *Note);
		}
		Report += TEXT("\n");

		AppendComponentLayerDiagnostics(Report, TEXT("Sandbox Copy"), SandboxComponent);
		AppendMaterialLayerDiagnostics(Report, SandboxLandscape ? SandboxLandscape->LandscapeMaterial : nullptr);

		Report += TEXT("## Expected Grass-Producing Layer Bindings\n");
		for (ULandscapeLayerInfoObject* LayerInfo : GrasslandLayerInfos)
		{
			Report += FString::Printf(TEXT("- expected `%s` via %s canonicalImport=`%s`\n"),
				LayerInfo ? *LayerInfo->GetLayerName().ToString() : TEXT("None"),
				*LayerInfoDebugName(LayerInfo),
				*CanonicalImportLayerName(LayerInfo).ToString());
		}
		Report += TEXT("\nResult: PASS - sandbox-only layer diagnostics generated; no modifications applied.\n");

		FFileHelper::SaveStringToFile(Report, *SavedReportPath());
		UE_LOG(LogTemp, Display, TEXT("Sandbox layer diagnostics report: %s"), *SavedReportPath());
		return SandboxComponent ? 0 : 1;
	}

	int32 RunSandboxResolveLayersOnly(TArray<FString>& Notes)
	{
		UWorld* SandboxWorld = UEditorLoadingAndSavingUtils::LoadMap(SandboxMapPath);
		ALandscape* SandboxLandscape = nullptr;
		if (SandboxWorld)
		{
			for (TActorIterator<ALandscape> It(SandboxWorld); It; ++It)
			{
				SandboxLandscape = *It;
				break;
			}
		}
		ULandscapeComponent* SandboxComponent = (SandboxLandscape && SandboxLandscape->LandscapeComponents.Num() > 0)
			? SandboxLandscape->LandscapeComponents[0]
			: nullptr;

		FString Report = TEXT("# TitanMain Real Component Sandbox Layer Resolve\n\n");
		Report += TEXT("Mode: sandbox-only minimal correction. TitanMain source cells were not loaded. Only the isolated sandbox landscape layer content is resolved/saved.\n\n");
		Report += TEXT("## Before Resolve\n");
		AppendComponentLayerDiagnostics(Report, TEXT("Sandbox Copy"), SandboxComponent);

		if (!SandboxLandscape || !SandboxComponent)
		{
			Report += TEXT("Result: FAIL - sandbox landscape/component missing.\n");
			FFileHelper::SaveStringToFile(Report, *SavedReportPath());
			return 1;
		}

		SandboxLandscape->Modify();
		SandboxLandscape->RequestLayersContentUpdateForceAll(ELandscapeLayerUpdateMode::Update_All, true);
		SandboxLandscape->ForceUpdateLayersContent();
		SandboxLandscape->UpdateAllComponentMaterialInstances(true);
		for (ULandscapeComponent* Component : SandboxLandscape->LandscapeComponents)
		{
			if (Component)
			{
				Component->Modify();
				Component->UpdateGrassTypes(true);
				Component->MarkPackageDirty();
			}
		}
		SandboxLandscape->MarkPackageDirty();
		UEditorLoadingAndSavingUtils::SaveMap(SandboxWorld, SandboxMapPath);

		Report += TEXT("## After Resolve\n");
		AppendComponentLayerDiagnostics(Report, TEXT("Sandbox Copy"), SandboxComponent);
		Report += TEXT("Result: PASS - sandbox layer content resolve attempted and sandbox map saved.\n");
		FFileHelper::SaveStringToFile(Report, *SavedReportPath());
		UE_LOG(LogTemp, Display, TEXT("Sandbox layer resolve report: %s"), *SavedReportPath());
		return 0;
	}

	int32 RunGrassWeightCompareOnly(TArray<FString>& Notes)
	{
		TArray<ULandscapeLayerInfoObject*> GrasslandLayerInfos;
		for (const TCHAR* LayerPath : GrasslandLayerInfoPaths)
		{
			if (ULandscapeLayerInfoObject* LayerInfo = LoadObjectCheckedSoft<ULandscapeLayerInfoObject>(LayerPath))
			{
				GrasslandLayerInfos.Add(LayerInfo);
				Notes.Add(FString::Printf(TEXT("Loaded layer info %s canonicalImport=`%s`."),
					*LayerInfoDebugName(LayerInfo),
					*CanonicalImportLayerName(LayerInfo).ToString()));
			}
			else
			{
				Notes.Add(FString::Printf(TEXT("Missing layer info `%s`."), LayerPath));
			}
		}

		UWorld* TitanWorld = UEditorLoadingAndSavingUtils::LoadMap(TitanMainMapPath);
		UWorldPartitionEditorLoaderAdapter* LoaderAdapter = LoadWorldPartitionCells(TitanWorld, Notes);
		FComponentExtract BestExtract;
		TArray<FComponentExtract> TopCandidates;
		const bool bFoundSource = FindBestTitanMainGrasslandComponent(TitanWorld, GrasslandLayerInfos, BestExtract, TopCandidates);

		UWorld* SandboxWorld = UEditorLoadingAndSavingUtils::LoadMap(SandboxMapPath);
		ALandscape* SandboxLandscape = nullptr;
		if (SandboxWorld)
		{
			for (TActorIterator<ALandscape> It(SandboxWorld); It; ++It)
			{
				SandboxLandscape = *It;
				break;
			}
		}
		ULandscapeComponent* SandboxComponent = (SandboxLandscape && SandboxLandscape->LandscapeComponents.Num() > 0)
			? SandboxLandscape->LandscapeComponents[0]
			: nullptr;

		FString Report = TEXT("# TitanMain vs Sandbox GrassWeight Compare\n\n");
		Report += TEXT("Mode: read-only comparison of source TitanMain component against copied sandbox component. No actors/maps are modified.\n\n");
		Report += TEXT("## Notes\n");
		for (const FString& Note : Notes)
		{
			Report += FString::Printf(TEXT("- %s\n"), *Note);
		}
		Report += TEXT("\n");

		if (bFoundSource)
		{
			Report += TEXT("## Selected TitanMain Source\n");
			Report += FString::Printf(TEXT("- sourceProxy=`%s`\n"), *BestExtract.SourceProxyLabel);
			Report += FString::Printf(TEXT("- sourceComponent=`%s`\n"), *BestExtract.SourceComponentName);
			Report += FString::Printf(TEXT("- sourceCenter=`%s`\n"), *BestExtract.SourceWorldCenter.ToString());
			const FString SourceLandscapeMaterialPath = ObjectPath(BestExtract.LandscapeMaterial);
			Report += FString::Printf(TEXT("- sourceMaterial=`%s`\n"), *SourceLandscapeMaterialPath);
			Report += FString::Printf(TEXT("- source layer averages grass/dirt/rock/sand=%.2f/%.2f/%.2f/%.2f score=%.2f\n\n"),
				BestExtract.GrassAverage,
				BestExtract.DirtAverage,
				BestExtract.RockAverage,
				BestExtract.SandAverage,
				BestExtract.Score);
			AppendComponentLayerDiagnostics(Report, TEXT("TitanMain Source"), BestExtract.SourceComponent);
			AppendMaterialLayerDiagnostics(Report, BestExtract.LandscapeMaterial);
		}
		else
		{
			Report += TEXT("## Selected TitanMain Source\n- FAIL: no source component found.\n\n");
		}

		AppendComponentLayerDiagnostics(Report, TEXT("Sandbox Copy"), SandboxComponent);
		AppendMaterialLayerDiagnostics(Report, SandboxLandscape ? SandboxLandscape->LandscapeMaterial : nullptr);

		Report += TEXT("## Top Source Candidates\n");
		for (const FComponentExtract& Candidate : TopCandidates)
		{
			Report += FString::Printf(TEXT("- `%s` / `%s`: center=%s score=%.2f grass=%.2f dirt=%.2f rock=%.2f sand=%.2f\n"),
				*Candidate.SourceProxyLabel,
				*Candidate.SourceComponentName,
				*Candidate.SourceWorldCenter.ToString(),
				Candidate.Score,
				Candidate.GrassAverage,
				Candidate.DirtAverage,
				Candidate.RockAverage,
				Candidate.SandAverage);
		}
		Report += TEXT("\nResult: PASS - compare report generated; no modifications applied.\n");

		FFileHelper::SaveStringToFile(Report, *SavedReportPath());
		ReleaseWorldPartitionCells(TitanWorld, LoaderAdapter);
		UE_LOG(LogTemp, Display, TEXT("GrassWeight compare report: %s"), *SavedReportPath());
		return bFoundSource && SandboxComponent ? 0 : 1;
	}

	UWorld* CreateOrLoadSandboxMap()
	{
		if (FPackageName::DoesPackageExist(SandboxMapPath))
		{
			if (UWorld* ExistingWorld = UEditorLoadingAndSavingUtils::LoadMap(SandboxMapPath))
			{
				return ExistingWorld;
			}
		}

		UWorld* NewWorld = UEditorLoadingAndSavingUtils::NewBlankMap(false);
		if (NewWorld)
		{
			UEditorLoadingAndSavingUtils::SaveMap(NewWorld, SandboxMapPath);
		}
		return NewWorld;
	}

	void ClearSandboxActors(UWorld* World)
	{
		if (!World)
		{
			return;
		}

		TArray<AActor*> ToDestroy;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && !Actor->IsA<AWorldSettings>())
			{
				ToDestroy.Add(Actor);
			}
		}

		for (AActor* Actor : ToDestroy)
		{
			World->DestroyActor(Actor, true);
		}
	}

	template<typename TActorClass>
	TActorClass* SpawnTaggedActor(UWorld* World, const FString& Label, const FVector& Location, const FRotator& Rotation)
	{
		TActorClass* Actor = World ? World->SpawnActor<TActorClass>(Location, Rotation) : nullptr;
		if (Actor)
		{
			Actor->Modify();
			Actor->SetActorLabel(Label);
			Actor->Tags.AddUnique(SandboxTag);
			Actor->MarkPackageDirty();
		}
		return Actor;
	}

	void AddRVTVolume(UWorld* World, const FString& Label, const FBox& Bounds, URuntimeVirtualTexture* RVT)
	{
		if (!World || !RVT || !Bounds.IsValid)
		{
			return;
		}

		constexpr double XYMargin = 2000.0;
		constexpr double ZBelowMargin = 1200.0;
		constexpr double ZAboveMargin = 4200.0;
		const FVector VolumeMin(
			Bounds.Min.X - XYMargin,
			Bounds.Min.Y - XYMargin,
			Bounds.Min.Z - ZBelowMargin);
		const FVector VolumeMax(
			Bounds.Max.X + XYMargin,
			Bounds.Max.Y + XYMargin,
			Bounds.Max.Z + ZAboveMargin);
		const FVector VolumeSize = VolumeMax - VolumeMin;

		ARuntimeVirtualTextureVolume* Volume = SpawnTaggedActor<ARuntimeVirtualTextureVolume>(
			World,
			Label,
			VolumeMin,
			FRotator::ZeroRotator);
		if (!Volume)
		{
			return;
		}

		// Runtime virtual texture volumes use a unit 0..1 local box, so actor
		// scale is the actual world-space coverage size in centimeters.
		Volume->SetActorScale3D(FVector(
			FMath::Max(VolumeSize.X, 100.0),
			FMath::Max(VolumeSize.Y, 100.0),
			FMath::Max(VolumeSize.Z, 100.0)));
		if (Volume->VirtualTextureComponent)
		{
			Volume->VirtualTextureComponent->Modify();
			Volume->VirtualTextureComponent->SetVirtualTexture(RVT);
			Volume->VirtualTextureComponent->MarkPackageDirty();
		}
		Volume->MarkPackageDirty();
	}

	ALandscape* BuildSandboxLandscape(UWorld* World, const FComponentExtract& Extract, TArray<FString>& Notes)
	{
		if (!World || Extract.HeightData.Num() == 0 || Extract.Layers.Num() == 0 || !Extract.LandscapeMaterial)
		{
			Notes.Add(TEXT("Cannot build sandbox landscape: missing world, height data, layer data, or landscape material."));
			return nullptr;
		}

		TArray<FLandscapeImportLayerInfo> ImportLayers;
		for (const FLayerExtract& ExtractLayer : Extract.Layers)
		{
			if (!ExtractLayer.LayerInfo || ExtractLayer.Data.Num() != Extract.SizeX * Extract.SizeY)
			{
				continue;
			}

			const FName LayerName = CanonicalImportLayerName(ExtractLayer.LayerInfo);
			FLandscapeImportLayerInfo ImportLayer(LayerName);
			ImportLayer.LayerInfo = ExtractLayer.LayerInfo;
			ImportLayer.LayerData = ExtractLayer.Data;
			Notes.Add(FString::Printf(TEXT("Sandbox import layer `%s` from %s avg=%.2f max=%d nonZero=%d."),
				*LayerName.ToString(),
				*LayerInfoDebugName(ExtractLayer.LayerInfo),
				ExtractLayer.Average,
				ExtractLayer.Max,
				CountNonZeroWeights(ExtractLayer.Data)));
			ImportLayers.Add(MoveTemp(ImportLayer));
		}

		if (ImportLayers.Num() == 0)
		{
			Notes.Add(TEXT("Cannot build sandbox landscape: no non-empty extracted weight layers survived import setup."));
			return nullptr;
		}

		TArray<uint16> HeightData = Extract.HeightData;
		TMap<FGuid, TArray<uint16>> HeightDataPerLayers;
		TMap<FGuid, TArray<FLandscapeImportLayerInfo>> MaterialLayerDataPerLayers;
		HeightDataPerLayers.Add(FGuid(), MoveTemp(HeightData));
		MaterialLayerDataPerLayers.Add(FGuid(), MoveTemp(ImportLayers));

		const FVector Scale = Extract.SourceScale;
		const FVector LandscapeLocation(
			-0.5f * Extract.ComponentSizeQuads * Scale.X,
			-0.5f * Extract.ComponentSizeQuads * Scale.Y,
			0.0f);

		ALandscape* Landscape = World->SpawnActor<ALandscape>(LandscapeLocation, FRotator::ZeroRotator);
		if (!Landscape)
		{
			Notes.Add(TEXT("Failed to spawn sandbox ALandscape."));
			return nullptr;
		}

		Landscape->Modify();
		Landscape->SetActorLabel(TEXT("FF_TitanMainRealComponentSandbox_Landscape_ExactHeightWeight"));
		Landscape->Tags.AddUnique(SandboxTag);
		Landscape->LandscapeMaterial = Extract.LandscapeMaterial;
		Landscape->SetActorScale3D(Scale);
		Landscape->StaticLightingLOD = 0;
		for (URuntimeVirtualTexture* RVT : Extract.RuntimeVirtualTextures)
		{
			if (RVT)
			{
				Landscape->RuntimeVirtualTextures.AddUnique(RVT);
			}
		}
		if (Landscape->RuntimeVirtualTextures.Num() == 0)
		{
			if (URuntimeVirtualTexture* RVTD = LoadObjectCheckedSoft<URuntimeVirtualTexture>(FallbackRVTDPath))
			{
				Landscape->RuntimeVirtualTextures.AddUnique(RVTD);
			}
			if (URuntimeVirtualTexture* RVTH = LoadObjectCheckedSoft<URuntimeVirtualTexture>(FallbackRVTHPath))
			{
				Landscape->RuntimeVirtualTextures.AddUnique(RVTH);
			}
		}
		Landscape->VirtualTextureRenderPassType = Extract.VirtualTextureRenderPassType;
		Landscape->Import(
			FGuid::NewGuid(),
			0,
			0,
			Extract.SizeX - 1,
			Extract.SizeY - 1,
			Extract.NumSubsections,
			Extract.SubsectionSizeQuads,
			HeightDataPerLayers,
			TEXT(""),
			MaterialLayerDataPerLayers,
			ELandscapeImportAlphamapType::Additive,
			TArrayView<const FLandscapeLayer>());
		Landscape->PostEditChange();
		Landscape->MarkPackageDirty();
		return Landscape;
	}

	void AddBasicLighting(UWorld* World)
	{
		ADirectionalLight* Sun = SpawnTaggedActor<ADirectionalLight>(World, TEXT("FF_TitanMainRealComponentSandbox_Sun"), FVector(-1600.0f, -2600.0f, 3600.0f), FRotator(-42.0f, -38.0f, 0.0f));
		if (Sun && Sun->GetLightComponent())
		{
			Sun->GetLightComponent()->SetIntensity(5.0f);
			Sun->GetLightComponent()->SetCastShadows(true);
			Sun->MarkPackageDirty();
		}

		ASkyLight* Sky = SpawnTaggedActor<ASkyLight>(World, TEXT("FF_TitanMainRealComponentSandbox_SkyLight"), FVector::ZeroVector, FRotator::ZeroRotator);
		if (Sky && Sky->GetLightComponent())
		{
			Sky->GetLightComponent()->SetIntensity(0.75f);
			Sky->GetLightComponent()->SetMobility(EComponentMobility::Movable);
			Sky->MarkPackageDirty();
		}

		AExponentialHeightFog* Fog = SpawnTaggedActor<AExponentialHeightFog>(World, TEXT("FF_TitanMainRealComponentSandbox_Fog"), FVector::ZeroVector, FRotator::ZeroRotator);
		if (Fog && Fog->GetComponent())
		{
			Fog->GetComponent()->FogDensity = 0.012f;
			Fog->GetComponent()->FogHeightFalloff = 0.24f;
			Fog->GetComponent()->FogMaxOpacity = 0.55f;
			Fog->MarkPackageDirty();
		}
	}

	void AddCamera(UWorld* World, const FString& Label, const FName& CameraTag, const FVector& Location, const FRotator& Rotation, float FOV)
	{
		ACameraActor* Camera = SpawnTaggedActor<ACameraActor>(World, Label, Location, Rotation);
		if (Camera)
		{
			Camera->Tags.AddUnique(CameraTag);
			if (Camera->GetCameraComponent())
			{
				Camera->GetCameraComponent()->SetFieldOfView(FOV);
			}
			Camera->MarkPackageDirty();
		}
	}

	void AddValidationActors(UWorld* World, ALandscape* Landscape)
	{
		if (!World || !Landscape)
		{
			return;
		}

		const FBox Bounds = Landscape->GetComponentsBoundingBox(true);
		const FVector Center = Bounds.GetCenter();
		const FVector SpawnLocation(Center.X - 600.0f, Center.Y - 900.0f, Bounds.Max.Z + 180.0f);
		APlayerStart* PlayerStart = SpawnTaggedActor<APlayerStart>(World, TEXT("FF_TitanMainRealComponentSandbox_PlayerStart"), SpawnLocation, FRotator(0.0f, 35.0f, 0.0f));
		if (PlayerStart)
		{
			PlayerStart->MarkPackageDirty();
		}

		AddBasicLighting(World);

		AddCamera(World, TEXT("FF_TitanMainRealComponentSandbox_Camera_Sun"), FName(TEXT("FFSmokeTitanHostSunCamera")),
			FVector(Center.X - 2600.0f, Center.Y - 3200.0f, Bounds.Max.Z + 2100.0f),
			FRotator(-26.0f, 38.0f, 0.0f),
			45.0f);
		AddCamera(World, TEXT("FF_TitanMainRealComponentSandbox_Camera_Shadow"), FName(TEXT("FFSmokeTitanHostShadowCamera")),
			FVector(Center.X + 2200.0f, Center.Y + 1800.0f, Bounds.Max.Z + 1200.0f),
			FRotator(-18.0f, -142.0f, 0.0f),
			42.0f);
		AddCamera(World, TEXT("FF_TitanMainRealComponentSandbox_Camera_TopDown"), FName(TEXT("FFSmokeTitanHostTopDownCamera")),
			FVector(Center.X, Center.Y, Bounds.Max.Z + 8200.0f),
			FRotator(-90.0f, 0.0f, 0.0f),
			35.0f);

		const FBox LandscapeBounds = Landscape->GetComponentsBoundingBox(true);
		for (URuntimeVirtualTexture* RVT : Landscape->RuntimeVirtualTextures)
		{
			AddRVTVolume(World, FString::Printf(TEXT("FF_TitanMainRealComponentSandbox_RVT_%s"), *GetNameSafe(RVT)), LandscapeBounds, RVT);
		}
		if (URuntimeVirtualTexture* RVTDH = LoadObjectCheckedSoft<URuntimeVirtualTexture>(FallbackRVTDHPath))
		{
			AddRVTVolume(World, TEXT("FF_TitanMainRealComponentSandbox_RVT_RVT_Titan_DH"), LandscapeBounds, RVTDH);
		}
	}

	void BuildNativeLandscapeGrassContext(UWorld* World, ALandscape* Landscape, TArray<FString>& Notes)
	{
		if (!World || !Landscape)
		{
			Notes.Add(TEXT("Native grass context skipped: missing sandbox world or landscape."));
			return;
		}

		Landscape->RegisterAllComponents();
		ULandscapeInfo* LandscapeInfo = Landscape->CreateLandscapeInfo(false, true);
		Landscape->SetDisableRuntimeGrassMapGeneration(false);
		if (LandscapeInfo)
		{
			LandscapeInfo->UpdateAllComponentMaterialInstances(true);
		}
		Landscape->UpdateAllComponentMaterialInstances(true);
		Landscape->ReregisterAllComponents();
		if (World)
		{
			World->UpdateWorldComponents(true, false);
			World->SendAllEndOfFrameUpdates();
		}

		TSet<FString> GrassTypePaths;
		int32 ComponentsWithMaterialGrass = 0;
		int32 ComponentsRenderableForGrassMap = 0;
		int32 RegisteredComponents = 0;
		int32 RenderStateCreatedComponents = 0;
		int32 SceneProxyComponents = 0;
		int32 MaterialInstanceComponents = 0;
		int32 ValidGrassDataBefore = 0;
		int32 ValidGrassDataAfter = 0;
		int64 GrassElementsBefore = 0;
		int64 GrassElementsAfter = 0;

		for (ULandscapeComponent* Component : Landscape->LandscapeComponents)
		{
			if (!Component)
			{
				continue;
			}

			Component->UpdateMaterialInstances();
			Component->UpdateGrassTypes(true);
			if (Component->IsRegistered())
			{
				++RegisteredComponents;
			}
			if (Component->IsRenderStateCreated())
			{
				++RenderStateCreatedComponents;
			}
			if (Component->SceneProxy)
			{
				++SceneProxyComponents;
			}
			UMaterialInstance* MaterialInstance = Component->GetMaterialInstanceCount(false) > 0 ? Component->GetMaterialInstance(0, false) : nullptr;
			if (MaterialInstance)
			{
				++MaterialInstanceComponents;
			}
			if (Component->MaterialHasGrass())
			{
				++ComponentsWithMaterialGrass;
			}
			if (Component->CanRenderGrassMap())
			{
				++ComponentsRenderableForGrassMap;
			}

			const bool bEditorWorldGate = GIsEditor
				&& !GUsingNullRHI
				&& Component->GetWorld()
				&& !Component->GetWorld()->IsGameWorld()
				&& Component->GetWorld()->GetFeatureLevel() >= ERHIFeatureLevel::SM5;
			const AActor* ComponentOwner = Component->GetOwner();
			const int32 MaterialInstanceCountForGrass = Component->GetMaterialInstanceCount(false);
			UMaterialInstance* MaterialInstanceForGrass =
				MaterialInstanceCountForGrass > 0 ? Component->GetMaterialInstance(0, false) : nullptr;
			const FString MaterialInstanceForGrassPath = ObjectPath(MaterialInstanceForGrass);
			const FMaterialResource* MaterialResourceForGrass = nullptr;
			if (MaterialInstanceForGrass && Component->GetWorld())
			{
				MaterialResourceForGrass =
					MaterialInstanceForGrass->GetMaterialResource(
						GetFeatureLevelShaderPlatform_Checked(Component->GetWorld()->GetFeatureLevel()));
			}
			Notes.Add(FString::Printf(
				TEXT("GrassMap gate component `%s`: editorWorldGate=%d isRegistered=%d renderStateCreated=%d sceneProxy=%d shouldRender=%d shouldAddToScene=%d visible=%d ownerHiddenEd=%d ownerHiddenGame=%d materialInstanceCount=%d materialInterface=`%s` materialResource=%d canRenderGrassMap=%d."),
				*Component->GetName(),
				bEditorWorldGate ? 1 : 0,
				Component->IsRegistered() ? 1 : 0,
				Component->IsRenderStateCreated() ? 1 : 0,
				Component->SceneProxy ? 1 : 0,
				Component->ShouldRender() ? 1 : 0,
				Component->ShouldComponentAddToScene() ? 1 : 0,
				Component->GetVisibleFlag() ? 1 : 0,
				(ComponentOwner && ComponentOwner->IsHiddenEd()) ? 1 : 0,
				(ComponentOwner && ComponentOwner->IsHidden()) ? 1 : 0,
				MaterialInstanceCountForGrass,
				*MaterialInstanceForGrassPath,
				MaterialResourceForGrass ? 1 : 0,
				Component->CanRenderGrassMap() ? 1 : 0));
			if (bEditorWorldGate
				&& Component->IsRegistered()
				&& Component->IsRenderStateCreated()
				&& Component->SceneProxy
				&& MaterialInstanceCountForGrass > 0
				&& MaterialInstanceForGrass
				&& MaterialResourceForGrass
				&& !Component->CanRenderGrassMap())
			{
				Notes.Add(FString::Printf(
					TEXT("GrassMap exact failing gate for `%s`: all preconditions pass including SceneProxy and material resource; ULandscapeComponent::CanRenderGrassMap() therefore fails at FMaterialResource::HasShaders(FLandscapeGrassWeightVS/PS, FLandscapeFixedGridVertexFactory) for `%s`."),
					*Component->GetName(),
					*MaterialInstanceForGrassPath));
			}
			if (Component->GrassData->NumElements >= 0)
			{
				++ValidGrassDataBefore;
				GrassElementsBefore += Component->GrassData->NumElements;
			}
			for (const TObjectPtr<ULandscapeGrassType>& GrassType : Component->GetGrassTypes())
			{
				GrassTypePaths.Add(ObjectPath(GrassType.Get()));
			}
		}

		const int32 OutdatedBefore = Landscape->GetOutdatedGrassMapCount();
		Landscape->BuildGrassMaps();
		const int32 OutdatedAfter = Landscape->GetOutdatedGrassMapCount();

		for (ULandscapeComponent* Component : Landscape->LandscapeComponents)
		{
			if (!Component)
			{
				continue;
			}

			if (Component->GrassData->NumElements >= 0)
			{
				++ValidGrassDataAfter;
				GrassElementsAfter += Component->GrassData->NumElements;
			}
		}

		TArray<FString> SortedGrassTypes = GrassTypePaths.Array();
		SortedGrassTypes.Sort();
		Notes.Add(FString::Printf(
			TEXT("Native landscape grass context: components=%d registered=%d renderState=%d sceneProxy=%d materialInstance=%d materialGrass=%d renderableForGrassMap=%d validGrassDataBefore=%d elementsBefore=%lld outdatedBefore=%d validGrassDataAfter=%d elementsAfter=%lld outdatedAfter=%d grassTypes=[%s]"),
			Landscape->LandscapeComponents.Num(),
			RegisteredComponents,
			RenderStateCreatedComponents,
			SceneProxyComponents,
			MaterialInstanceComponents,
			ComponentsWithMaterialGrass,
			ComponentsRenderableForGrassMap,
			ValidGrassDataBefore,
			GrassElementsBefore,
			OutdatedBefore,
			ValidGrassDataAfter,
			GrassElementsAfter,
			OutdatedAfter,
			*FString::Join(SortedGrassTypes, TEXT(", "))));

		if (ComponentsWithMaterialGrass == 0 || SortedGrassTypes.Num() == 0)
		{
			Notes.Add(TEXT("Native landscape grass did not bind: the sandbox landscape material exposes no LandscapeGrassOutput grass type on its component."));
		}
		else if (ComponentsRenderableForGrassMap == 0)
		{
			if (SceneProxyComponents == 0)
			{
				Notes.Add(TEXT("Native landscape grass could not build GrassMaps: ULandscapeComponent::CanRenderGrassMap() fails at the SceneProxy gate. The component is visible, registered, render-state-created, and has a material resource, but the commandlet/headless world did not attach a FPrimitiveSceneProxy for the landscape component."));
			}
			else if (RegisteredComponents == Landscape->LandscapeComponents.Num()
				&& RenderStateCreatedComponents == Landscape->LandscapeComponents.Num()
				&& MaterialInstanceComponents == Landscape->LandscapeComponents.Num())
			{
				Notes.Add(TEXT("Native landscape grass could not build GrassMaps: the component has a SceneProxy and material resource, so the remaining failing gate is the material resource missing GrassWeight shaders for FLandscapeFixedGridVertexFactory."));
			}
			else
			{
				Notes.Add(TEXT("Native landscape grass could not build GrassMaps: the component lacks a renderable editor component/material state; do not substitute proxy grass."));
			}
		}
		else if (ValidGrassDataAfter == 0 || GrassElementsAfter == 0)
		{
			Notes.Add(TEXT("Native landscape grass GrassMaps were requested but produced no valid grass data."));
		}
		else
		{
			Notes.Add(TEXT("Native landscape grass GrassMaps built successfully for the sandbox component."));
		}
	}

	ALandscape* FindSandboxLandscape(UWorld* World)
	{
		if (!World)
		{
			return nullptr;
		}

		for (TActorIterator<ALandscape> It(World); It; ++It)
		{
			ALandscape* Landscape = *It;
			if (!Landscape)
			{
				continue;
			}

			if (Landscape->Tags.Contains(SandboxTag) || Landscape->GetActorLabel().Contains(TEXT("TitanMainRealComponentSandbox")))
			{
				return Landscape;
			}
		}

		for (TActorIterator<ALandscape> It(World); It; ++It)
		{
			return *It;
		}

		return nullptr;
	}

	FString EnumValueName(const UEnum* Enum, const int64 Value)
	{
		return Enum ? Enum->GetNameStringByValue(Value) : FString::Printf(TEXT("%lld"), Value);
	}

	FString RVTDescription(const URuntimeVirtualTexture* RVT)
	{
		if (!RVT)
		{
			return TEXT("None");
		}

		const FString RVTPath = ObjectPath(RVT);
		return FString::Printf(
			TEXT("`%s` type=%s tiles=%d tileSize=%d border=%d size=%d clear=%d packed=%d private=%d adaptive=%d continuous=%d removeLowMips=%d"),
			*RVTPath,
			*EnumValueName(StaticEnum<ERuntimeVirtualTextureMaterialType>(), static_cast<int64>(RVT->GetMaterialType())),
			RVT->GetTileCount(),
			RVT->GetTileSize(),
			RVT->GetTileBorderSize(),
			RVT->GetSize(),
			RVT->GetClearTextures() ? 1 : 0,
			RVT->GetSinglePhysicalSpace() ? 1 : 0,
			RVT->GetPrivateSpace() ? 1 : 0,
			RVT->GetAdaptivePageTable() ? 1 : 0,
			RVT->GetContinuousUpdate() ? 1 : 0,
			RVT->GetRemoveLowMips());
	}

	FString DescribeRVTVolume(ARuntimeVirtualTextureVolume* Volume, const FBox& LandscapeBounds)
	{
		if (!Volume)
		{
			return TEXT("None");
		}

		URuntimeVirtualTextureComponent* Component = Volume->VirtualTextureComponent;
		const FBox VolumeBounds = Component ? Component->Bounds.GetBox() : Volume->GetComponentsBoundingBox(true);
		const bool bIntersectsLandscape = VolumeBounds.IsValid && LandscapeBounds.IsValid && VolumeBounds.Intersect(LandscapeBounds);
		const bool bContainsLandscapeMin = VolumeBounds.IsValid && LandscapeBounds.IsValid && VolumeBounds.IsInsideOrOn(LandscapeBounds.Min);
		const bool bContainsLandscapeMax = VolumeBounds.IsValid && LandscapeBounds.IsValid && VolumeBounds.IsInsideOrOn(LandscapeBounds.Max);
		bool bHidePrimitiveEditor = false;
		bool bHidePrimitiveGame = false;
		if (Component)
		{
			Component->GetHidePrimitiveSettings(bHidePrimitiveEditor, bHidePrimitiveGame);
		}

		return FString::Printf(
			TEXT("label=`%s` vt=%s loc=%s scale=%s bounds=%s intersectsLandscape=%d containsLandscapeMinMax=%d/%d enabledInScene=%d scalable=%d scalabilityGroup=%u streamingMips=%d streamingOnly=%d hideEditorGame=%d/%d"),
			*ActorLabelOrName(Volume),
			*RVTDescription(Component ? Component->GetVirtualTexture() : nullptr),
			*Volume->GetActorLocation().ToString(),
			*Volume->GetActorScale3D().ToString(),
			*VolumeBounds.ToString(),
			bIntersectsLandscape ? 1 : 0,
			bContainsLandscapeMin ? 1 : 0,
			bContainsLandscapeMax ? 1 : 0,
			(Component && Component->IsEnabledInScene()) ? 1 : 0,
			(Component && Component->IsScalable()) ? 1 : 0,
			Component ? Component->GetScalabilityGroup() : 0,
			Component ? Component->NumStreamingMips() : 0,
			(Component && Component->IsStreamingLowMipsOnly()) ? 1 : 0,
			bHidePrimitiveEditor ? 1 : 0,
			bHidePrimitiveGame ? 1 : 0);
	}

	int32 RunSandboxRVTContextRepairOnly(TArray<FString>& Notes)
	{
		UWorld* SandboxWorld = UEditorLoadingAndSavingUtils::LoadMap(SandboxMapPath);
		if (!SandboxWorld)
		{
			const FString Report = FString::Printf(
				TEXT("# TitanMain Real Component Sandbox RVT Context Repair\n\nResult: FAIL\n\nCould not load sandbox map `%s`.\n"),
				SandboxMapPath);
			FFileHelper::SaveStringToFile(Report, *SavedReportPath());
			return 1;
		}

		ALandscape* SandboxLandscape = FindSandboxLandscape(SandboxWorld);
		if (!SandboxLandscape)
		{
			const FString Report = FString::Printf(
				TEXT("# TitanMain Real Component Sandbox RVT Context Repair\n\nResult: FAIL\n\nSandbox map `%s` has no ALandscape actor.\n"),
				SandboxMapPath);
			FFileHelper::SaveStringToFile(Report, *SavedReportPath());
			return 1;
		}

		SandboxLandscape->RegisterAllComponents();
		SandboxLandscape->UpdateAllComponentMaterialInstances(true);
		const FBox LandscapeBounds = SandboxLandscape->GetComponentsBoundingBox(true);
		const FString SandboxLandscapeMaterialPath = ObjectPath(SandboxLandscape->LandscapeMaterial);
		Notes.Add(FString::Printf(TEXT("Target landscape `%s` bounds=%s material=`%s` passType=%s."),
			*SandboxLandscape->GetActorLabel(),
			*LandscapeBounds.ToString(),
			*SandboxLandscapeMaterialPath,
			*EnumValueName(StaticEnum<ERuntimeVirtualTextureMainPassType>(), static_cast<int64>(SandboxLandscape->VirtualTextureRenderPassType))));

		if (SandboxLandscape->RuntimeVirtualTextures.Num() == 0)
		{
			if (URuntimeVirtualTexture* RVTD = LoadObjectCheckedSoft<URuntimeVirtualTexture>(FallbackRVTDPath))
			{
				SandboxLandscape->RuntimeVirtualTextures.AddUnique(RVTD);
			}
			if (URuntimeVirtualTexture* RVTH = LoadObjectCheckedSoft<URuntimeVirtualTexture>(FallbackRVTHPath))
			{
				SandboxLandscape->RuntimeVirtualTextures.AddUnique(RVTH);
			}
		}

		Notes.Add(TEXT("Landscape runtime virtual textures after ensure:"));
		for (URuntimeVirtualTexture* RVT : SandboxLandscape->RuntimeVirtualTextures)
		{
			Notes.Add(FString::Printf(TEXT("  %s"), *RVTDescription(RVT)));
		}

		TArray<ARuntimeVirtualTextureVolume*> OldVolumes;
		for (TActorIterator<ARuntimeVirtualTextureVolume> It(SandboxWorld); It; ++It)
		{
			ARuntimeVirtualTextureVolume* Volume = *It;
			if (!Volume)
			{
				continue;
			}
			if (Volume->Tags.Contains(SandboxTag) || Volume->GetActorLabel().Contains(TEXT("TitanMainRealComponentSandbox_RVT")))
			{
				OldVolumes.Add(Volume);
			}
		}

		Notes.Add(FString::Printf(TEXT("Existing sandbox RVT volumes before repair: %d."), OldVolumes.Num()));
		for (ARuntimeVirtualTextureVolume* Volume : OldVolumes)
		{
			Notes.Add(FString::Printf(TEXT("  before %s"), *DescribeRVTVolume(Volume, LandscapeBounds)));
		}
		for (ARuntimeVirtualTextureVolume* Volume : OldVolumes)
		{
			SandboxWorld->DestroyActor(Volume, true);
		}

		TArray<URuntimeVirtualTexture*> RVTsToSpawn;
		for (URuntimeVirtualTexture* RVT : SandboxLandscape->RuntimeVirtualTextures)
		{
			RVTsToSpawn.AddUnique(RVT);
		}
		if (URuntimeVirtualTexture* RVTDH = LoadObjectCheckedSoft<URuntimeVirtualTexture>(FallbackRVTDHPath))
		{
			RVTsToSpawn.AddUnique(RVTDH);
		}

		for (URuntimeVirtualTexture* RVT : RVTsToSpawn)
		{
			AddRVTVolume(SandboxWorld, FString::Printf(TEXT("FF_TitanMainRealComponentSandbox_RVT_%s"), *GetNameSafe(RVT)), LandscapeBounds, RVT);
		}
		SandboxLandscape->PostEditChange();
		SandboxLandscape->MarkPackageDirty();

		TArray<ARuntimeVirtualTextureVolume*> NewVolumes;
		for (TActorIterator<ARuntimeVirtualTextureVolume> It(SandboxWorld); It; ++It)
		{
			ARuntimeVirtualTextureVolume* Volume = *It;
			if (Volume && (Volume->Tags.Contains(SandboxTag) || Volume->GetActorLabel().Contains(TEXT("TitanMainRealComponentSandbox_RVT"))))
			{
				Volume->RegisterAllComponents();
				Volume->MarkComponentsRenderStateDirty();
				NewVolumes.Add(Volume);
			}
		}

		Notes.Add(FString::Printf(TEXT("Sandbox RVT volumes after repair: %d."), NewVolumes.Num()));
		for (ARuntimeVirtualTextureVolume* Volume : NewVolumes)
		{
			Notes.Add(FString::Printf(TEXT("  after %s"), *DescribeRVTVolume(Volume, LandscapeBounds)));
		}

		UEditorLoadingAndSavingUtils::SaveMap(SandboxWorld, SandboxMapPath);

		FString Report = TEXT("# TitanMain Real Component Sandbox RVT Context Repair\n\n");
		Report += TEXT("Mode: sandbox-only RVT volume coverage repair. No TitanMain/Kashkeh/Highland map was modified.\n\n");
		Report += FString::Printf(TEXT("- Sandbox map: `%s`\n"), SandboxMapPath);
		Report += FString::Printf(TEXT("- Target landscape: `%s`\n\n"), *SandboxLandscape->GetActorLabel());
		Report += TEXT("## Notes\n");
		for (const FString& Note : Notes)
		{
			Report += FString::Printf(TEXT("- %s\n"), *Note);
		}
		Report += TEXT("\nResult: PASS - repaired isolated sandbox RVT volume bounds and saved the sandbox map.\n");
		FFileHelper::SaveStringToFile(Report, *SavedReportPath());
		UE_LOG(LogTemp, Display, TEXT("Sandbox RVT context repair report: %s"), *SavedReportPath());
		return 0;
	}

	int32 RunGrassContextOnly(TArray<FString>& Notes)
	{
		UWorld* SandboxWorld = UEditorLoadingAndSavingUtils::LoadMap(SandboxMapPath);
		if (!SandboxWorld)
		{
			const FString Report = FString::Printf(
				TEXT("# TitanMain Real Component Grass Context Only\n\nResult: FAIL\n\nCould not load sandbox map `%s`.\n"),
				SandboxMapPath);
			FFileHelper::SaveStringToFile(Report, *SavedReportPath());
			UE_LOG(LogTemp, Error, TEXT("GrassContextOnly failed to load sandbox map: %s"), SandboxMapPath);
			return 1;
		}

		ALandscape* SandboxLandscape = FindSandboxLandscape(SandboxWorld);
		if (!SandboxLandscape)
		{
			const FString Report = FString::Printf(
				TEXT("# TitanMain Real Component Grass Context Only\n\nResult: FAIL\n\nSandbox map `%s` has no ALandscape actor.\n"),
				SandboxMapPath);
			FFileHelper::SaveStringToFile(Report, *SavedReportPath());
			UE_LOG(LogTemp, Error, TEXT("GrassContextOnly found no landscape in sandbox map: %s"), SandboxMapPath);
			return 1;
		}

		Notes.Add(FString::Printf(TEXT("GrassContextOnly loaded sandbox map `%s` without loading TitanMain source cells."), SandboxMapPath));
		const FString SandboxLandscapeMaterialPath = ObjectPath(SandboxLandscape->LandscapeMaterial.Get());
		Notes.Add(FString::Printf(TEXT("GrassContextOnly target landscape: `%s` material=`%s`."),
			*SandboxLandscape->GetActorLabel(),
			*SandboxLandscapeMaterialPath));

		BuildNativeLandscapeGrassContext(SandboxWorld, SandboxLandscape, Notes);
		UEditorLoadingAndSavingUtils::SaveMap(SandboxWorld, SandboxMapPath);

		FString Report = TEXT("# TitanMain Real Component Grass Context Only\n\n");
		Report += TEXT("Mode: build native LandscapeGrassOutput / LGT grass data on existing real-component sandbox only. TitanMain source map was not loaded.\n\n");
		Report += FString::Printf(TEXT("- Sandbox map: `%s`\n"), SandboxMapPath);
		Report += FString::Printf(TEXT("- Target landscape: `%s`\n"), *SandboxLandscape->GetActorLabel());
		Report += FString::Printf(TEXT("- Landscape material: `%s`\n\n"), *SandboxLandscapeMaterialPath);
		Report += TEXT("## Notes\n");
		for (const FString& Note : Notes)
		{
			Report += FString::Printf(TEXT("- %s\n"), *Note);
		}
		Report += TEXT("\nResult: PASS - GrassContextOnly completed on the isolated sandbox map.\n");
		FFileHelper::SaveStringToFile(Report, *SavedReportPath());
		UE_LOG(LogTemp, Display, TEXT("GrassContextOnly completed. Report: %s"), *SavedReportPath());
		return 0;
	}

	void AppendExtractReport(FString& Report, const FComponentExtract& Extract, const TArray<FComponentExtract>& TopCandidates, const TArray<FString>& Notes)
	{
		Report += TEXT("# TitanMain Real Component Extract Sandbox\n\n");
		Report += TEXT("Mode: real TitanMain Landscape component height/weight extraction. Not synthetic.\n\n");
		Report += TEXT("## Source\n");
		Report += FString::Printf(TEXT("- Source map: `%s`\n"), TitanMainMapPath);
		Report += FString::Printf(TEXT("- Source proxy: `%s`\n"), *Extract.SourceProxyLabel);
		Report += FString::Printf(TEXT("- Source component: `%s`\n"), *Extract.SourceComponentName);
		Report += FString::Printf(TEXT("- Source component extent: min=(%d,%d), max=(%d,%d), size=%dx%d\n"),
			Extract.Extent.Min.X, Extract.Extent.Min.Y, Extract.Extent.Max.X, Extract.Extent.Max.Y, Extract.SizeX, Extract.SizeY);
		Report += FString::Printf(TEXT("- Source world center: `%s`\n"), *Extract.SourceWorldCenter.ToString());
		const FString ExtractLandscapeMaterialPath = ObjectPath(Extract.LandscapeMaterial);
		Report += FString::Printf(TEXT("- Landscape material: `%s`\n"), *ExtractLandscapeMaterialPath);
		Report += FString::Printf(TEXT("- Component layout: NumSubsections=%d, SubsectionSizeQuads=%d, ComponentSizeQuads=%d, Scale=%s\n"),
			Extract.NumSubsections, Extract.SubsectionSizeQuads, Extract.ComponentSizeQuads, *Extract.SourceScale.ToString());
		Report += FString::Printf(TEXT("- Grass/Dirt/Rock/Sand averages: %.2f / %.2f / %.2f / %.2f\n"),
			Extract.GrassAverage, Extract.DirtAverage, Extract.RockAverage, Extract.SandAverage);
		Report += FString::Printf(TEXT("- Candidate score: %.2f\n\n"), Extract.Score);

		Report += TEXT("## Extracted layers\n");
		for (const FLayerExtract& Layer : Extract.Layers)
		{
			const FString LayerInfoPath = ObjectPath(Layer.LayerInfo);
			Report += FString::Printf(TEXT("- `%s`: avg=%.2f max=%d\n"), *LayerInfoPath, Layer.Average, Layer.Max);
		}
		Report += TEXT("\n## Runtime virtual textures copied/assigned\n");
		for (URuntimeVirtualTexture* RVT : Extract.RuntimeVirtualTextures)
		{
			const FString RVTPath = ObjectPath(RVT);
			Report += FString::Printf(TEXT("- `%s`\n"), *RVTPath);
		}
		Report += TEXT("- Fallback volume also attempts `/Game/Landscape/RVT/RVT_Titan_DH` if available.\n\n");

		Report += TEXT("## Top candidate components\n");
		for (const FComponentExtract& Candidate : TopCandidates)
		{
			Report += FString::Printf(TEXT("- `%s` / `%s`: center=%s score=%.2f grass=%.2f dirt=%.2f rock=%.2f sand=%.2f\n"),
				*Candidate.SourceProxyLabel,
				*Candidate.SourceComponentName,
				*Candidate.SourceWorldCenter.ToString(),
				Candidate.Score,
				Candidate.GrassAverage,
				Candidate.DirtAverage,
				Candidate.RockAverage,
				Candidate.SandAverage);
		}
		Report += TEXT("\n## Notes\n");
		for (const FString& Note : Notes)
		{
			Report += FString::Printf(TEXT("- %s\n"), *Note);
		}
	}
}
#endif

int32 UFFTitanMainRealComponentSandboxCommandlet::Main(const FString& Params)
{
#if WITH_EDITOR
	TArray<FString> Notes;

	if (FParse::Param(*Params, TEXT("FFGrassContextOnly")))
	{
		return RunGrassContextOnly(Notes);
	}
	if (FParse::Param(*Params, TEXT("FFGrassWeightCompareOnly")))
	{
		return RunGrassWeightCompareOnly(Notes);
	}
	if (FParse::Param(*Params, TEXT("FFSandboxLayerDiagOnly")))
	{
		return RunSandboxLayerDiagOnly(Notes);
	}
	if (FParse::Param(*Params, TEXT("FFSandboxResolveLayersOnly")))
	{
		return RunSandboxResolveLayersOnly(Notes);
	}
	if (FParse::Param(*Params, TEXT("FFSandboxRVTContextRepairOnly")))
	{
		return RunSandboxRVTContextRepairOnly(Notes);
	}

	TArray<ULandscapeLayerInfoObject*> GrasslandLayerInfos;
	for (const TCHAR* LayerPath : GrasslandLayerInfoPaths)
	{
		ULandscapeLayerInfoObject* LayerInfo = LoadObjectCheckedSoft<ULandscapeLayerInfoObject>(LayerPath);
		if (!LayerInfo)
		{
			UE_LOG(LogTemp, Error, TEXT("Missing required Grassland layer info: %s"), LayerPath);
			return 1;
		}
		GrasslandLayerInfos.Add(LayerInfo);
	}

	UWorld* TitanWorld = UEditorLoadingAndSavingUtils::LoadMap(TitanMainMapPath);
	if (!TitanWorld)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load TitanMain source map: %s"), TitanMainMapPath);
		return 1;
	}

	UWorldPartitionEditorLoaderAdapter* LoaderAdapter = LoadWorldPartitionCells(TitanWorld, Notes);

	FComponentExtract BestExtract;
	TArray<FComponentExtract> TopCandidates;
	const bool bFoundComponent = FindBestTitanMainGrasslandComponent(TitanWorld, GrasslandLayerInfos, BestExtract, TopCandidates);
	if (!bFoundComponent)
	{
		ReleaseWorldPartitionCells(TitanWorld, LoaderAdapter);
		FString Report = TEXT("# TitanMain Real Component Extract Sandbox\n\nResult: FAIL\n\nNo loaded TitanMain Landscape component with non-zero `LI_Grassland_Grass` weight was found.\n\n");
		for (const FString& Note : Notes)
		{
			Report += FString::Printf(TEXT("- %s\n"), *Note);
		}
		const FString ReportFilename = SavedReportPath();
		FFileHelper::SaveStringToFile(Report, *ReportFilename);
		UE_LOG(LogTemp, Error, TEXT("No real Grassland-weighted TitanMain component found. Report: %s"), *ReportFilename);
		return 1;
	}

	UWorld* SandboxWorld = CreateOrLoadSandboxMap();
	if (!SandboxWorld)
	{
		ReleaseWorldPartitionCells(TitanWorld, LoaderAdapter);
		UE_LOG(LogTemp, Error, TEXT("Failed to create/load sandbox map: %s"), SandboxMapPath);
		return 1;
	}

	ClearSandboxActors(SandboxWorld);
	ALandscape* SandboxLandscape = BuildSandboxLandscape(SandboxWorld, BestExtract, Notes);
	if (!SandboxLandscape)
	{
		ReleaseWorldPartitionCells(TitanWorld, LoaderAdapter);
		FString Report;
		AppendExtractReport(Report, BestExtract, TopCandidates, Notes);
		Report += TEXT("\nResult: FAIL - source data extracted, but sandbox landscape could not be created.\n");
		const FString ReportFilename = SavedReportPath();
		FFileHelper::SaveStringToFile(Report, *ReportFilename);
		UE_LOG(LogTemp, Error, TEXT("Extracted source data, but sandbox creation failed. Report: %s"), *ReportFilename);
		return 1;
	}

	AddValidationActors(SandboxWorld, SandboxLandscape);
	BuildNativeLandscapeGrassContext(SandboxWorld, SandboxLandscape, Notes);
	UEditorLoadingAndSavingUtils::SaveMap(SandboxWorld, SandboxMapPath);
	ReleaseWorldPartitionCells(TitanWorld, LoaderAdapter);

	FString Report;
	AppendExtractReport(Report, BestExtract, TopCandidates, Notes);
	Report += TEXT("\nResult: PASS - created isolated sandbox from one real TitanMain component's height/weight/layer data.\n");
	const FString ReportFilename = SavedReportPath();
	FFileHelper::SaveStringToFile(Report, *ReportFilename);
	UE_LOG(LogTemp, Display, TEXT("Created TitanMain real component sandbox: %s"), SandboxMapPath);
	UE_LOG(LogTemp, Display, TEXT("Report: %s"), *ReportFilename);
	return 0;
#else
	return 1;
#endif
}
