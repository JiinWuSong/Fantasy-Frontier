// Copyright Epic Games, Inc. All Rights Reserved.

#include "TP_ThirdPerson.h"
#include "Modules/ModuleManager.h"

#if WITH_EDITOR
#include "AssetCompilingManager.h"
#include "Containers/Ticker.h"
#include "Editor.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "FileHelpers.h"
#include "HAL/IConsoleManager.h"
#include "Landscape.h"
#include "LandscapeComponent.h"
#include "LandscapeEdit.h"
#include "LandscapeEditLayer.h"
#include "LandscapeGrassType.h"
#include "LandscapeInfo.h"
#include "LandscapeLayerInfoObject.h"
#include "LandscapeProxy.h"
#include "MaterialCachedData.h"
#include "MaterialShared.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialParameters.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#endif

DEFINE_LOG_CATEGORY(LogTP_ThirdPerson)

#if WITH_EDITOR
namespace
{
	const TCHAR* EditorGrassSandboxMapPath = TEXT("/Game/FantasyFrontier/Test/TitanMainGrasslandRealComponentSandbox");
	const TCHAR* EditorGrassReportFileName = TEXT("TitanMainRealComponentSandbox_EditorGrassMapBuild_Report.txt");
	const TCHAR* HighlandEditorGrassReportFileName = TEXT("HighlandNativeMiniProof_EditorGrassMapBuild_Report.txt");
	const TCHAR* HighlandNativeMiniGrassLayerInfoPath = TEXT("/Game/Landscape/WeightLayers/LI_Grassland_Grass.LI_Grassland_Grass");
	const FVector HighlandNativeMiniProofCenter(41753.467f, 150044.634f, -3300.174f);
	const float HighlandNativeMiniProofRadius = 2600.0f;
	const TCHAR* HighlandNativeClusterComponentNames[] = {
		TEXT("LandscapeComponent_81"),
		TEXT("LandscapeComponent_82"),
		TEXT("LandscapeComponent_83"),
		TEXT("LandscapeComponent_84"),
		TEXT("LandscapeComponent_85"),
		TEXT("LandscapeComponent_86"),
		TEXT("LandscapeComponent_87"),
		TEXT("LandscapeComponent_88")
	};
	FString GEditorGrassTargetMapPath = EditorGrassSandboxMapPath;
	FString GEditorGrassTargetReportFileName = EditorGrassReportFileName;
	const TCHAR* EditorGrasslandLayerInfoPaths[] = {
		TEXT("/Game/Landscape/WeightLayers/LI_Grassland_Grass.LI_Grassland_Grass"),
		TEXT("/Game/Landscape/WeightLayers/LI_Grassland_Dirt.LI_Grassland_Dirt"),
		TEXT("/Game/Landscape/WeightLayers/LI_Grassland_Rock.LI_Grassland_Rock"),
		TEXT("/Game/Landscape/WeightLayers/LI_Grassland_Sand.LI_Grassland_Sand")
	};

	FString EditorGrassObjectPath(const UObject* Object)
	{
		return Object ? Object->GetPathName() : FString(TEXT("None"));
	}

	FString EditorGrassSavedReportPath()
	{
		return FPaths::Combine(FPaths::ProjectSavedDir(), GEditorGrassTargetReportFileName);
	}

	bool IsHighlandEditorGrassTarget(const FString& TargetMapPath)
	{
		return TargetMapPath.Contains(TEXT("FF_Starter_Highland_Blockout"));
	}

	bool IsHighlandNativeClusterComponentName(const FString& ComponentName)
	{
		for (const TCHAR* TargetName : HighlandNativeClusterComponentNames)
		{
			if (ComponentName.Equals(TargetName))
			{
				return true;
			}
		}
		return false;
	}

	FString HighlandNativeClusterComponentSummary()
	{
		FString Summary;
		for (const TCHAR* TargetName : HighlandNativeClusterComponentNames)
		{
			if (!Summary.IsEmpty())
			{
				Summary += TEXT(", ");
			}
			Summary += TargetName;
		}
		return Summary;
	}

	void ConfigureEditorGrassWorkflowFromCommandLine()
	{
		GEditorGrassTargetMapPath = EditorGrassSandboxMapPath;
		FParse::Value(FCommandLine::Get(), TEXT("FFEditorGrassMapPath="), GEditorGrassTargetMapPath);

		FString ReportOverride;
		const bool bHasReportOverride = FParse::Value(FCommandLine::Get(), TEXT("FFEditorGrassReport="), ReportOverride)
			&& !ReportOverride.IsEmpty();
		GEditorGrassTargetReportFileName = bHasReportOverride
			? ReportOverride
			: (IsHighlandEditorGrassTarget(GEditorGrassTargetMapPath)
				? HighlandEditorGrassReportFileName
				: EditorGrassReportFileName);
	}

	FString EditorGrassLayerInfoDebugName(const ULandscapeLayerInfoObject* LayerInfo)
	{
		if (!LayerInfo)
		{
			return TEXT("None");
		}
		return FString::Printf(TEXT("asset=`%s` objectName=`%s` layerName=`%s`"),
			*EditorGrassObjectPath(LayerInfo),
			*LayerInfo->GetName(),
			*LayerInfo->GetLayerName().ToString());
	}

	FString EditorGrassAllocationSummary(const TArray<FWeightmapLayerAllocationInfo>& Allocations)
	{
		if (Allocations.IsEmpty())
		{
			return TEXT("None");
		}

		TArray<FString> Notes;
		for (const FWeightmapLayerAllocationInfo& Allocation : Allocations)
		{
			Notes.Add(FString::Printf(
				TEXT("%s@tex%d.%d"),
				*EditorGrassLayerInfoDebugName(Allocation.LayerInfo),
				Allocation.WeightmapTextureIndex,
				static_cast<int32>(Allocation.WeightmapTextureChannel)));
		}
		return FString::Join(Notes, TEXT("; "));
	}

	FString EditorGrassTextureSummary(const TArray<UTexture2D*>& Textures)
	{
		if (Textures.IsEmpty())
		{
			return TEXT("None");
		}

		TArray<FString> Notes;
		const int32 MaxTexturesToList = 6;
		for (int32 TextureIndex = 0; TextureIndex < Textures.Num() && TextureIndex < MaxTexturesToList; ++TextureIndex)
		{
			const UTexture2D* Texture = Textures[TextureIndex];
			Notes.Add(FString::Printf(
				TEXT("%s[%dx%d]"),
				*EditorGrassObjectPath(Texture),
				Texture ? Texture->GetSizeX() : 0,
				Texture ? Texture->GetSizeY() : 0));
		}
		if (Textures.Num() > MaxTexturesToList)
		{
			Notes.Add(FString::Printf(TEXT("...+%d"), Textures.Num() - MaxTexturesToList));
		}
		return FString::Join(Notes, TEXT("; "));
	}

	FString EditorGrassTextureSourceWeightSummary(UTexture2D* Texture)
	{
		if (!Texture)
		{
			return TEXT("None");
		}
		if (!Texture->Source.IsValid())
		{
			return FString::Printf(TEXT("%s sourceInvalid"), *EditorGrassObjectPath(Texture));
		}

		const int64 Width = Texture->Source.GetSizeX();
		const int64 Height = Texture->Source.GetSizeY();
		const ETextureSourceFormat Format = Texture->Source.GetFormat();
		const FString FormatName = StaticEnum<ETextureSourceFormat>()
			? StaticEnum<ETextureSourceFormat>()->GetNameStringByValue(static_cast<int64>(Format))
			: FString(TEXT("Unknown"));
		const uint8* MipData = Texture->Source.LockMipReadOnly(0);
		if (!MipData)
		{
			return FString::Printf(TEXT("%s source=%s %lldx%lld lockFailed"), *EditorGrassObjectPath(Texture), *FormatName, Width, Height);
		}

		const int64 PixelCount = Width * Height;
		TArray<uint32, TInlineAllocator<4>> NonZeroCounts;
		TArray<uint32, TInlineAllocator<4>> MaxValues;
		TArray<uint64, TInlineAllocator<4>> Sums;
		int32 ChannelCount = 0;
		int32 BytesPerPixel = 0;
		if (Format == TSF_BGRA8 || Format == TSF_BGRE8)
		{
			ChannelCount = 4;
			BytesPerPixel = 4;
		}
		else if (Format == TSF_G8)
		{
			ChannelCount = 1;
			BytesPerPixel = 1;
		}

		if (ChannelCount == 0 || BytesPerPixel == 0)
		{
			Texture->Source.UnlockMip(0);
			return FString::Printf(
				TEXT("%s source=%s %lldx%lld unsupportedForChannelProbe"),
				*EditorGrassObjectPath(Texture),
				*FormatName,
				Width,
				Height);
		}

		NonZeroCounts.Init(0, ChannelCount);
		MaxValues.Init(0, ChannelCount);
		Sums.Init(0, ChannelCount);
		for (int64 PixelIndex = 0; PixelIndex < PixelCount; ++PixelIndex)
		{
			const uint8* Pixel = MipData + (PixelIndex * BytesPerPixel);
			for (int32 ChannelIndex = 0; ChannelIndex < ChannelCount; ++ChannelIndex)
			{
				const uint8 Value = Pixel[ChannelIndex];
				if (Value > 0)
				{
					++NonZeroCounts[ChannelIndex];
				}
				MaxValues[ChannelIndex] = FMath::Max(MaxValues[ChannelIndex], static_cast<uint32>(Value));
				Sums[ChannelIndex] += Value;
			}
		}
		Texture->Source.UnlockMip(0);

		static const TCHAR* ChannelNames[] = { TEXT("B/0"), TEXT("G/1"), TEXT("R/2"), TEXT("A/3") };
		TArray<FString, TInlineAllocator<4>> ChannelNotes;
		for (int32 ChannelIndex = 0; ChannelIndex < ChannelCount; ++ChannelIndex)
		{
			const double Average = PixelCount > 0
				? static_cast<double>(Sums[ChannelIndex]) / static_cast<double>(PixelCount)
				: 0.0;
			ChannelNotes.Add(FString::Printf(
				TEXT("%s nz=%u max=%u avg=%.2f"),
				ChannelNames[ChannelIndex],
				NonZeroCounts[ChannelIndex],
				MaxValues[ChannelIndex],
				Average));
		}

		return FString::Printf(
			TEXT("%s source=%s %lldx%lld %s"),
			*EditorGrassObjectPath(Texture),
			*FormatName,
			Width,
			Height,
			*FString::Join(ChannelNotes, TEXT("; ")));
	}

	FString EditorGrassTypeSummary(const TArray<TObjectPtr<ULandscapeGrassType>>& GrassTypes)
	{
		if (GrassTypes.IsEmpty())
		{
			return TEXT("None");
		}

		TArray<FString> Notes;
		for (const TObjectPtr<ULandscapeGrassType>& GrassType : GrassTypes)
		{
			Notes.Add(EditorGrassObjectPath(GrassType.Get()));
		}
		return FString::Join(Notes, TEXT("; "));
	}

	FString EditorGrassTerrainLayerParameterSummary(const UMaterialInstance* MaterialInstance)
	{
		if (!MaterialInstance)
		{
			return TEXT("NoMaterialInstance");
		}

		TArray<FString> Notes;
		for (const FStaticTerrainLayerWeightParameter& Parameter : MaterialInstance->GetEditorOnlyStaticParameters().TerrainLayerWeightParameters)
		{
			Notes.Add(FString::Printf(TEXT("%s@Weightmap%d"), *Parameter.LayerName.ToString(), Parameter.WeightmapIndex));
		}
		return Notes.IsEmpty() ? FString(TEXT("None")) : FString::Join(Notes, TEXT("; "));
	}

	FString EditorGrassMaterialParentChainSummary(const UMaterialInterface* MaterialInterface)
	{
		if (!MaterialInterface)
		{
			return TEXT("NoMaterialInterface");
		}

		TArray<FString> Notes;
		const UMaterialInterface* CurrentInterface = MaterialInterface;
		for (int32 Depth = 0; CurrentInterface && Depth < 8; ++Depth)
		{
			const UMaterialInstance* CurrentInstance = Cast<UMaterialInstance>(CurrentInterface);
			const FString TerrainLayerSummary = CurrentInstance
				? EditorGrassTerrainLayerParameterSummary(CurrentInstance)
				: FString(TEXT("BaseMaterial"));
			Notes.Add(FString::Printf(
				TEXT("[%d] `%s` class=%s terrainLayerStaticParams=%s"),
				Depth,
				*EditorGrassObjectPath(CurrentInterface),
				*CurrentInterface->GetClass()->GetName(),
				*TerrainLayerSummary));

			CurrentInterface = CurrentInstance ? CurrentInstance->Parent.Get() : nullptr;
		}

		return FString::Join(Notes, TEXT(" -> "));
	}

	FString EditorGrassMaterialWeightParameterSummary(const UMaterialInterface* MaterialInterface)
	{
		if (!MaterialInterface)
		{
			return TEXT("NoMaterialInterface");
		}

		UTexture* Weightmap0 = nullptr;
		const bool bHasWeightmap0 = MaterialInterface->GetTextureParameterValue(
			FHashedMaterialParameterInfo(TEXT("Weightmap0")),
			Weightmap0,
			/*bOveriddenOnly=*/false);

		FLinearColor GrassLayerMask = FLinearColor::Black;
		const bool bHasGrassLayerMask = MaterialInterface->GetVectorParameterValue(
			FHashedMaterialParameterInfo(TEXT("LayerMask_Grassland_Grass")),
			GrassLayerMask,
			/*bOveriddenOnly=*/false);

		return FString::Printf(
			TEXT("Weightmap0=%s `%s`; LayerMask_Grassland_Grass=%s (%.3f, %.3f, %.3f, %.3f)"),
			bHasWeightmap0 ? TEXT("set") : TEXT("missing"),
			*EditorGrassObjectPath(Weightmap0),
			bHasGrassLayerMask ? TEXT("set") : TEXT("missing"),
			GrassLayerMask.R,
			GrassLayerMask.G,
			GrassLayerMask.B,
			GrassLayerMask.A);
	}

	int32 EditorGrassForceComponentTerrainLayerStaticParams(ALandscape* Landscape, FString& Report)
	{
		if (!Landscape)
		{
			return 0;
		}

		int32 UpdatedMaterialInstances = 0;
		for (ULandscapeComponent* Component : Landscape->LandscapeComponents)
		{
			if (!Component || !IsHighlandNativeClusterComponentName(Component->GetName()))
			{
				continue;
			}

			const TArray<FWeightmapLayerAllocationInfo>& Allocations = Component->GetWeightmapLayerAllocations();
			if (Allocations.IsEmpty())
			{
				Report += FString::Printf(TEXT("- static terrain layer mirror skipped: %s has no base weightmap allocations.\n"), *Component->GetName());
				continue;
			}

			UMaterialInstanceConstant* ComponentMIC = Component->GetMaterialInstanceCount(false) > 0
				? Cast<UMaterialInstanceConstant>(Component->GetMaterialInstance(0, false))
				: nullptr;
			if (!ComponentMIC)
			{
				Report += FString::Printf(TEXT("- static terrain layer mirror skipped: %s has no editable component MIC.\n"), *Component->GetName());
				continue;
			}

			FStaticParameterSet StaticParameters;
			ComponentMIC->GetStaticParameterValues(StaticParameters);
			StaticParameters.EditorOnly.TerrainLayerWeightParameters.Empty();
			for (const FWeightmapLayerAllocationInfo& Allocation : Allocations)
			{
				const FName LayerName = Allocation.LayerInfo ? Allocation.LayerInfo->GetLayerName() : NAME_None;
				if (LayerName != NAME_None)
				{
					StaticParameters.EditorOnly.TerrainLayerWeightParameters.Add(
						FStaticTerrainLayerWeightParameter(LayerName, Allocation.WeightmapTextureIndex));
				}
			}

			if (StaticParameters.EditorOnly.TerrainLayerWeightParameters.IsEmpty())
			{
				Report += FString::Printf(TEXT("- static terrain layer mirror skipped: %s allocations did not resolve to named layers.\n"), *Component->GetName());
				continue;
			}

			ComponentMIC->Modify();
			ComponentMIC->UpdateStaticPermutation(StaticParameters);
			ComponentMIC->PostEditChange();
			Component->Modify();
			Component->MarkRenderStateDirty();
			++UpdatedMaterialInstances;

			Report += FString::Printf(
				TEXT("- static terrain layer mirror applied to %s `%s`: %s\n"),
				*Component->GetName(),
				*EditorGrassObjectPath(ComponentMIC),
				*EditorGrassTerrainLayerParameterSummary(ComponentMIC));
		}

		return UpdatedMaterialInstances;
	}

	TArray<ULandscapeLayerInfoObject*> LoadEditorGrasslandLayerInfos()
	{
		TArray<ULandscapeLayerInfoObject*> LayerInfos;
		for (const TCHAR* LayerPath : EditorGrasslandLayerInfoPaths)
		{
			LayerInfos.Add(LoadObject<ULandscapeLayerInfoObject>(nullptr, LayerPath));
		}
		return LayerInfos;
	}

	struct FHighlandNativeClusterPaintTarget
	{
		FString ComponentName;
		FVector2D LocalCenter = FVector2D::ZeroVector;
		FIntRect ComponentExtent;
	};

	TArray<FHighlandNativeClusterPaintTarget> GetHighlandNativeClusterPaintTargets(ALandscape* Landscape, const FVector& ProofLocalCenter)
	{
		TArray<FHighlandNativeClusterPaintTarget> Targets;
		if (!Landscape)
		{
			return Targets;
		}

		for (ULandscapeComponent* Component : Landscape->LandscapeComponents)
		{
			if (!Component || !IsHighlandNativeClusterComponentName(Component->GetName()))
			{
				continue;
			}

			const FIntRect Extent = Component->GetComponentExtent();
			FHighlandNativeClusterPaintTarget Target;
			Target.ComponentName = Component->GetName();
			Target.ComponentExtent = Extent;
			Target.LocalCenter = FVector2D(
				0.5f * static_cast<float>(Extent.Min.X + Extent.Max.X),
				FMath::Clamp(
					static_cast<float>(ProofLocalCenter.Y),
					static_cast<float>(Extent.Min.Y + 4),
					static_cast<float>(Extent.Max.Y - 4)));

			// Keep the originally validated proof location exactly on the known-good component.
			if (Target.ComponentName.Equals(TEXT("LandscapeComponent_83")))
			{
				Target.LocalCenter = FVector2D(ProofLocalCenter.X, ProofLocalCenter.Y);
			}

			Targets.Add(Target);
		}

		Targets.Sort([](const FHighlandNativeClusterPaintTarget& A, const FHighlandNativeClusterPaintTarget& B)
		{
			return A.ComponentName < B.ComponentName;
		});
		return Targets;
	}

	FString HighlandNativeClusterPaintTargetSummary(const TArray<FHighlandNativeClusterPaintTarget>& Targets)
	{
		if (Targets.IsEmpty())
		{
			return TEXT("None");
		}

		FString Summary;
		for (const FHighlandNativeClusterPaintTarget& Target : Targets)
		{
			if (!Summary.IsEmpty())
			{
				Summary += TEXT("; ");
			}
			Summary += FString::Printf(
				TEXT("%s localCenter=(%.1f, %.1f) extent=(%d,%d)-(%d,%d)"),
				*Target.ComponentName,
				Target.LocalCenter.X,
				Target.LocalCenter.Y,
				Target.ComponentExtent.Min.X,
				Target.ComponentExtent.Min.Y,
				Target.ComponentExtent.Max.X,
				Target.ComponentExtent.Max.Y);
		}
		return Summary;
	}

	bool RepaintHighlandNativeMiniLayerInEditor(ALandscape* Landscape, FString& Report, bool bRequestLayerResolveAfterPaint = true)
	{
		if (!Landscape)
		{
			Report += TEXT("## Highland Native Mini Repaint\n- skipped: missing Landscape.\n\n");
			return false;
		}

		ULandscapeLayerInfoObject* GrassLayerInfo = LoadObject<ULandscapeLayerInfoObject>(nullptr, HighlandNativeMiniGrassLayerInfoPath);
		ULandscapeInfo* LandscapeInfo = Landscape->GetLandscapeInfo();
		if (!GrassLayerInfo || !LandscapeInfo)
		{
			Report += FString::Printf(TEXT("## Highland Native Mini Repaint\n- failed: layerInfo=%s landscapeInfo=%s\n\n"),
				GrassLayerInfo ? TEXT("ok") : HighlandNativeMiniGrassLayerInfoPath,
				LandscapeInfo ? TEXT("ok") : TEXT("missing"));
			return false;
		}

		FGuid PaintEditLayerGuid;
		FName PaintEditLayerName = TEXT("BaseFinalWeightmap");
		for (const FLandscapeLayer& EditLayer : Landscape->GetLayersConst())
		{
			const ULandscapeEditLayerBase* EditLayerObject = EditLayer.EditLayer;
			if (!EditLayerObject || !EditLayerObject->IsVisible() || EditLayerObject->IsLocked())
			{
				continue;
			}

			PaintEditLayerGuid = EditLayerObject->GetGuid();
			PaintEditLayerName = EditLayerObject->GetName();
			break;
		}
		if (PaintEditLayerGuid.IsValid())
		{
			Landscape->SetEditingLayer(PaintEditLayerGuid);
		}

		bool bHasExtent = false;
		int32 MinX = 0;
		int32 MinY = 0;
		int32 MaxX = 0;
		int32 MaxY = 0;
		for (ULandscapeComponent* Component : Landscape->LandscapeComponents)
		{
			if (!Component)
			{
				continue;
			}

			const FIntRect ComponentExtent = Component->GetComponentExtent();
			if (!bHasExtent)
			{
				MinX = ComponentExtent.Min.X;
				MinY = ComponentExtent.Min.Y;
				MaxX = ComponentExtent.Max.X;
				MaxY = ComponentExtent.Max.Y;
				bHasExtent = true;
			}
			else
			{
				MinX = FMath::Min(MinX, ComponentExtent.Min.X);
				MinY = FMath::Min(MinY, ComponentExtent.Min.Y);
				MaxX = FMath::Max(MaxX, ComponentExtent.Max.X);
				MaxY = FMath::Max(MaxY, ComponentExtent.Max.Y);
			}
		}
		if (!bHasExtent)
		{
			Report += TEXT("## Highland Native Mini Repaint\n- failed: no Landscape component extents.\n\n");
			return false;
		}

		const FTransform& LandscapeTransform = Landscape->GetActorTransform();
		const FVector LocalCenter = LandscapeTransform.InverseTransformPosition(HighlandNativeMiniProofCenter);
		const FVector Scale = LandscapeTransform.GetScale3D().GetAbs();
		const float RadiusX = HighlandNativeMiniProofRadius / FMath::Max(Scale.X, 1.0f);
		const float RadiusY = HighlandNativeMiniProofRadius / FMath::Max(Scale.Y, 1.0f);
		const TArray<FHighlandNativeClusterPaintTarget> ClusterTargets = GetHighlandNativeClusterPaintTargets(Landscape, LocalCenter);
		if (ClusterTargets.IsEmpty())
		{
			Report += FString::Printf(TEXT("## Highland Native Mini Repaint\n- failed: no target cluster components found. requested=%s\n\n"),
				*HighlandNativeClusterComponentSummary());
			return false;
		}

		float ClusterMinX = TNumericLimits<float>::Max();
		float ClusterMinY = TNumericLimits<float>::Max();
		float ClusterMaxX = TNumericLimits<float>::Lowest();
		float ClusterMaxY = TNumericLimits<float>::Lowest();
		for (const FHighlandNativeClusterPaintTarget& Target : ClusterTargets)
		{
			ClusterMinX = FMath::Min(ClusterMinX, Target.LocalCenter.X - RadiusX);
			ClusterMinY = FMath::Min(ClusterMinY, Target.LocalCenter.Y - RadiusY);
			ClusterMaxX = FMath::Max(ClusterMaxX, Target.LocalCenter.X + RadiusX);
			ClusterMaxY = FMath::Max(ClusterMaxY, Target.LocalCenter.Y + RadiusY);
		}

		const int32 X1 = FMath::Clamp(FMath::FloorToInt(ClusterMinX), MinX, MaxX);
		const int32 Y1 = FMath::Clamp(FMath::FloorToInt(ClusterMinY), MinY, MaxY);
		const int32 X2 = FMath::Clamp(FMath::CeilToInt(ClusterMaxX), MinX, MaxX);
		const int32 Y2 = FMath::Clamp(FMath::CeilToInt(ClusterMaxY), MinY, MaxY);
		if (X2 <= X1 || Y2 <= Y1 || LandscapeInfo->ComponentSizeQuads <= 0)
		{
			Report += FString::Printf(TEXT("## Highland Native Mini Repaint\n- failed: invalid bounds X=%d..%d Y=%d..%d componentSizeQuads=%d\n\n"),
				X1,
				X2,
				Y1,
				Y2,
				LandscapeInfo->ComponentSizeQuads);
			return false;
		}

		const int32 ComponentSizeQuads = LandscapeInfo->ComponentSizeQuads;
		const int32 ComponentIndexX1 = (X1 - 1 >= 0) ? (X1 - 1) / ComponentSizeQuads : (X1) / ComponentSizeQuads - 1;
		const int32 ComponentIndexY1 = (Y1 - 1 >= 0) ? (Y1 - 1) / ComponentSizeQuads : (Y1) / ComponentSizeQuads - 1;
		const int32 ComponentIndexX2 = (X2 >= 0) ? X2 / ComponentSizeQuads : (X2 + 1) / ComponentSizeQuads - 1;
		const int32 ComponentIndexY2 = (Y2 >= 0) ? Y2 / ComponentSizeQuads : (Y2 + 1) / ComponentSizeQuads - 1;
		int32 ComponentMapHits = 0;
		for (ULandscapeComponent* Component : Landscape->LandscapeComponents)
		{
			if (!Component || !IsHighlandNativeClusterComponentName(Component->GetName()))
			{
				continue;
			}

			const FIntRect Extent = Component->GetComponentExtent();
			if (Extent.Min.X <= X2 && Extent.Max.X >= X1 && Extent.Min.Y <= Y2 && Extent.Max.Y >= Y1)
			{
				++ComponentMapHits;
			}
		}

		const int32 Width = X2 - X1 + 1;
		const int32 Height = Y2 - Y1 + 1;
		TArray<uint8> LayerData;
		LayerData.Init(0, Width * Height);

		FLandscapeEditDataInterface EditData(LandscapeInfo, PaintEditLayerGuid);
		EditData.GetWeightDataFast(GrassLayerInfo, X1, Y1, X2, Y2, LayerData.GetData(), 0);

		int32 PaintedSamples = 0;
		for (int32 Y = Y1; Y <= Y2; ++Y)
		{
			for (int32 X = X1; X <= X2; ++X)
			{
				float SoftWeight = 0.0f;
				for (const FHighlandNativeClusterPaintTarget& Target : ClusterTargets)
				{
					const float NormX = RadiusX > 0.0f ? (static_cast<float>(X) - Target.LocalCenter.X) / RadiusX : 0.0f;
					const float NormY = RadiusY > 0.0f ? (static_cast<float>(Y) - Target.LocalCenter.Y) / RadiusY : 0.0f;
					const float Dist = FMath::Sqrt((NormX * NormX) + (NormY * NormY));
					SoftWeight = FMath::Max(SoftWeight, 1.0f - FMath::SmoothStep(0.72f, 1.0f, Dist));
				}
				const uint8 PaintValue = static_cast<uint8>(FMath::Clamp(FMath::RoundToInt(SoftWeight * 255.0f), 0, 255));
				if (PaintValue == 0)
				{
					continue;
				}

				const int32 DataIndex = (Y - Y1) * Width + (X - X1);
				LayerData[DataIndex] = FMath::Max(LayerData[DataIndex], PaintValue);
				++PaintedSamples;
			}
		}
		if (PaintedSamples <= 0 || ComponentMapHits <= 0)
		{
			Report += FString::Printf(TEXT("## Highland Native Mini Repaint\n- failed: paintedSamples=%d componentMapHits=%d bounds X=%d..%d Y=%d..%d\n\n"),
				PaintedSamples,
				ComponentMapHits,
				X1,
				X2,
				Y1,
				Y2);
			return false;
		}

		Landscape->Modify();
PRAGMA_DISABLE_DEPRECATION_WARNINGS
		EditData.SetAlphaData(
			GrassLayerInfo,
			X1,
			Y1,
			X2,
			Y2,
			LayerData.GetData(),
			0,
			ELandscapeLayerPaintingRestriction::None,
			/*bWeightAdjust=*/true,
			/*bTotalWeightAdjust=*/true);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
		EditData.Flush();

		TArray<uint8> VerifyLayerData;
		VerifyLayerData.Init(0, Width * Height);
		EditData.GetWeightDataFast(GrassLayerInfo, X1, Y1, X2, Y2, VerifyLayerData.GetData(), 0);
		int32 NonZeroVerifySamples = 0;
		for (uint8 Value : VerifyLayerData)
		{
			if (Value > 0)
			{
				++NonZeroVerifySamples;
			}
		}

		int32 TargetGrassComponents = 0;
		for (ULandscapeComponent* Component : Landscape->LandscapeComponents)
		{
			if (!Component || !IsHighlandNativeClusterComponentName(Component->GetName()))
			{
				continue;
			}
			const FIntRect Extent = Component->GetComponentExtent();
			if (!(Extent.Min.X <= X2 && Extent.Max.X >= X1 && Extent.Min.Y <= Y2 && Extent.Max.Y >= Y1))
			{
				continue;
			}

			const TArray<FWeightmapLayerAllocationInfo>& TargetAllocations = Component->GetWeightmapLayerAllocations(PaintEditLayerGuid);
			if (TargetAllocations.ContainsByPredicate([GrassLayerInfo](const FWeightmapLayerAllocationInfo& Allocation) { return Allocation.LayerInfo == GrassLayerInfo; }))
			{
				++TargetGrassComponents;
			}
		}

		// The Highland map currently drops this edit-layer allocation during the final
		// layer resolve pass. Mirror the tiny proof mask into the final/base weightmap
		// so the native GrassMap build has a real, cookable allocation to read.
		FGuid BaseLayerGuid;
		FLandscapeEditDataInterface BaseEditData(LandscapeInfo, BaseLayerGuid);
		TArray<uint8> BaseLayerData;
		BaseLayerData.Init(0, Width * Height);
		BaseEditData.GetWeightDataFast(GrassLayerInfo, X1, Y1, X2, Y2, BaseLayerData.GetData(), 0);
		for (int32 Index = 0; Index < BaseLayerData.Num(); ++Index)
		{
			BaseLayerData[Index] = FMath::Max(BaseLayerData[Index], LayerData[Index]);
		}
PRAGMA_DISABLE_DEPRECATION_WARNINGS
		BaseEditData.SetAlphaData(
			GrassLayerInfo,
			X1,
			Y1,
			X2,
			Y2,
			BaseLayerData.GetData(),
			0,
			ELandscapeLayerPaintingRestriction::None,
			/*bWeightAdjust=*/true,
			/*bTotalWeightAdjust=*/true);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
		BaseEditData.Flush();

		TArray<uint8> BaseVerifyLayerData;
		BaseVerifyLayerData.Init(0, Width * Height);
		BaseEditData.GetWeightDataFast(GrassLayerInfo, X1, Y1, X2, Y2, BaseVerifyLayerData.GetData(), 0);
		int32 BaseNonZeroVerifySamples = 0;
		for (uint8 Value : BaseVerifyLayerData)
		{
			if (Value > 0)
			{
				++BaseNonZeroVerifySamples;
			}
		}

		int32 BaseGrassComponents = 0;
		for (ULandscapeComponent* Component : Landscape->LandscapeComponents)
		{
			if (!Component || !IsHighlandNativeClusterComponentName(Component->GetName()))
			{
				continue;
			}
			const FIntRect Extent = Component->GetComponentExtent();
			if (!(Extent.Min.X <= X2 && Extent.Max.X >= X1 && Extent.Min.Y <= Y2 && Extent.Max.Y >= Y1))
			{
				continue;
			}

			const TArray<FWeightmapLayerAllocationInfo>& BaseAllocations = Component->GetWeightmapLayerAllocations(BaseLayerGuid);
			if (BaseAllocations.ContainsByPredicate([GrassLayerInfo](const FWeightmapLayerAllocationInfo& Allocation) { return Allocation.LayerInfo == GrassLayerInfo; }))
			{
				++BaseGrassComponents;
			}
		}

		LandscapeInfo->UpdateLayerInfoMap(Landscape, true);
		LandscapeInfo->UpdateAllComponentMaterialInstances(true);
		if (bRequestLayerResolveAfterPaint)
		{
			Landscape->RequestLayersContentUpdate(ELandscapeLayerUpdateMode::Update_Weightmap_All);
		}
		Landscape->MarkPackageDirty();

		Report += TEXT("## Highland Native Component Cluster Repaint\n");
		Report += FString::Printf(TEXT("- seedProofCenter=`%s` perComponentRadius=%.1f\n"), *HighlandNativeMiniProofCenter.ToString(), HighlandNativeMiniProofRadius);
		Report += FString::Printf(TEXT("- requestedComponents=%s\n"), *HighlandNativeClusterComponentSummary());
		Report += FString::Printf(TEXT("- paintTargets=%s\n"), *HighlandNativeClusterPaintTargetSummary(ClusterTargets));
		Report += FString::Printf(TEXT("- layerInfo=`%s` layerName=`%s`\n"), *GrassLayerInfo->GetPathName(), *GrassLayerInfo->GetLayerName().ToString());
		Report += FString::Printf(TEXT("- paintEditLayer=`%s` guid=`%s`\n"), *PaintEditLayerName.ToString(), *PaintEditLayerGuid.ToString());
		Report += FString::Printf(TEXT("- requestLayerResolveAfterPaint=%d\n"), bRequestLayerResolveAfterPaint ? 1 : 0);
		Report += FString::Printf(TEXT("- bounds X=%d..%d Y=%d..%d componentIndex=%d..%d/%d..%d componentMapHits=%d\n"),
			X1,
			X2,
			Y1,
			Y2,
			ComponentIndexX1,
			ComponentIndexX2,
			ComponentIndexY1,
			ComponentIndexY2,
			ComponentMapHits);
		Report += FString::Printf(TEXT("- paintedSamples=%d targetGrassComponents=%d nonZeroVerifySamples=%d\n\n"),
			PaintedSamples,
			TargetGrassComponents,
			NonZeroVerifySamples);
		Report += FString::Printf(TEXT("- baseFallbackGrassComponents=%d baseNonZeroVerifySamples=%d\n\n"),
			BaseGrassComponents,
			BaseNonZeroVerifySamples);

		return (TargetGrassComponents > 0 || BaseGrassComponents > 0)
			&& (NonZeroVerifySamples > 0 || BaseNonZeroVerifySamples > 0);
	}

	float EditorGrassAverageWeight(const TArray<uint8>& Data)
	{
		if (Data.IsEmpty())
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

	uint8 EditorGrassMaxWeight(const TArray<uint8>& Data)
	{
		uint8 MaxValue = 0;
		for (uint8 Value : Data)
		{
			MaxValue = FMath::Max(MaxValue, Value);
		}
		return MaxValue;
	}

	int32 EditorGrassCountNonZeroWeights(const TArray<uint8>& Data)
	{
		int32 Count = 0;
		for (uint8 Value : Data)
		{
			if (Value > 0)
			{
				++Count;
			}
		}
		return Count;
	}

	ALandscape* FindEditorGrassTargetLandscape(UWorld* World)
	{
		if (!World)
		{
			return nullptr;
		}

		for (TActorIterator<ALandscape> It(World); It; ++It)
		{
			ALandscape* Landscape = *It;
			if (Landscape && Landscape->GetActorLabel().Contains(TEXT("TitanMainRealComponentSandbox")))
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

	struct FEditorGrassSnapshot
	{
		int32 Components = 0;
		int32 Registered = 0;
		int32 RenderStateCreated = 0;
		int32 SceneProxy = 0;
		int32 MaterialInstances = 0;
		int32 MaterialGrass = 0;
		int32 CanRenderGrassMap = 0;
		int32 ValidGrassData = 0;
		int64 GrassElements = 0;
		int32 GrassWeightTypes = 0;
		int64 GrassWeightBytes = 0;
		int64 NonZeroGrassWeightSamples = 0;
		int32 WeightmapLayerAllocations = 0;
		int64 NonZeroLandscapeLayerSamples = 0;
		int32 OutdatedGrassMaps = 0;
		TArray<FString> ComponentNotes;
	};

	FEditorGrassSnapshot CaptureEditorGrassSnapshot(ALandscape* Landscape, const FString& Label, bool bRefreshComponentState = true)
	{
		FEditorGrassSnapshot Snapshot;
		if (!Landscape)
		{
			Snapshot.ComponentNotes.Add(FString::Printf(TEXT("%s: no landscape."), *Label));
			return Snapshot;
		}

		UWorld* World = Landscape->GetWorld();
		Snapshot.Components = Landscape->LandscapeComponents.Num();
		Snapshot.OutdatedGrassMaps = Landscape->GetOutdatedGrassMapCount();

		for (ULandscapeComponent* Component : Landscape->LandscapeComponents)
		{
			if (!Component)
			{
				continue;
			}

			if (bRefreshComponentState)
			{
				Component->UpdateMaterialInstances();
				Component->UpdateGrassTypes(true);
			}

			if (Component->IsRegistered())
			{
				++Snapshot.Registered;
			}
			if (Component->IsRenderStateCreated())
			{
				++Snapshot.RenderStateCreated;
			}
			if (Component->SceneProxy)
			{
				++Snapshot.SceneProxy;
			}
			const int32 MaterialInstanceCount = Component->GetMaterialInstanceCount(false);
			UMaterialInstance* MaterialInstance = MaterialInstanceCount > 0 ? Component->GetMaterialInstance(0, false) : nullptr;
			if (MaterialInstance)
			{
				++Snapshot.MaterialInstances;
			}
			if (Component->MaterialHasGrass())
			{
				++Snapshot.MaterialGrass;
			}
			if (Component->CanRenderGrassMap())
			{
				++Snapshot.CanRenderGrassMap;
			}

			const TArray<FWeightmapLayerAllocationInfo>& Allocations = Component->GetWeightmapLayerAllocations();
			Snapshot.WeightmapLayerAllocations += Allocations.Num();
			if (Component->GrassData->NumElements > 0)
			{
				++Snapshot.ValidGrassData;
				Snapshot.GrassElements += Component->GrassData->NumElements;
			}
			Snapshot.GrassWeightTypes += Component->GrassData->WeightOffsets.Num();
			Snapshot.GrassWeightBytes += Component->GrassData->HeightWeightData.Num()
				- (Component->GrassData->NumElements * static_cast<int64>(sizeof(uint16)));

			TArray<FString> GrassTypeNotes;
			for (const TPair<TObjectPtr<ULandscapeGrassType>, int32>& Pair : Component->GrassData->WeightOffsets)
			{
				const ULandscapeGrassType* GrassType = Pair.Key.Get();
				int32 NonZeroSamples = 0;
				uint8 MaxWeight = 0;
				uint64 WeightSum = 0;
				const int32 WeightSampleCount = Component->GrassData->NumElements;
				const bool bValidWeightRange = WeightSampleCount > 0
					&& Pair.Value >= 0
					&& (Pair.Value + WeightSampleCount) <= Component->GrassData->HeightWeightData.Num();
				const uint8* WeightData = bValidWeightRange
					? Component->GrassData->HeightWeightData.GetData() + Pair.Value
					: nullptr;
				for (int32 WeightIndex = 0; WeightIndex < WeightSampleCount && WeightData; ++WeightIndex)
				{
					const uint8 Weight = WeightData[WeightIndex];
					if (Weight > 0)
					{
						++NonZeroSamples;
					}
					MaxWeight = FMath::Max(MaxWeight, Weight);
					WeightSum += Weight;
				}
				Snapshot.NonZeroGrassWeightSamples += NonZeroSamples;
				const double AverageWeight = WeightSampleCount > 0
					? static_cast<double>(WeightSum) / static_cast<double>(WeightSampleCount)
					: 0.0;
				GrassTypeNotes.Add(FString::Printf(
					TEXT("grassType=`%s` offset=%d weightRangeValid=%d weightSamples=%d nonZero=%d max=%d avg=%.2f"),
					*EditorGrassObjectPath(GrassType),
					Pair.Value,
					bValidWeightRange ? 1 : 0,
					WeightSampleCount,
					NonZeroSamples,
					static_cast<int32>(MaxWeight),
					AverageWeight));
			}
			if (GrassTypeNotes.IsEmpty())
			{
				GrassTypeNotes.Add(TEXT("grassTypeWeights=None"));
			}

			TArray<FString> AllocationNotes;
			if (ULandscapeInfo* LandscapeInfo = Component->GetLandscapeInfo())
			{
				FLandscapeEditDataInterface EditData(LandscapeInfo);
				const FIntRect Extent = Component->GetComponentExtent();
				const int32 SampleSize = (Extent.Width() + 1) * (Extent.Height() + 1);
				for (const FWeightmapLayerAllocationInfo& Allocation : Allocations)
				{
					TArray<uint8> LayerData;
					LayerData.Init(0, SampleSize);
					if (Allocation.LayerInfo)
					{
						EditData.GetWeightDataFast(Allocation.LayerInfo, Extent.Min.X, Extent.Min.Y, Extent.Max.X, Extent.Max.Y, LayerData.GetData(), 0);
					}
					const int32 NonZero = EditorGrassCountNonZeroWeights(LayerData);
					Snapshot.NonZeroLandscapeLayerSamples += NonZero;
					AllocationNotes.Add(FString::Printf(
						TEXT("allocation %s textureIndex=%d channel=%d avg=%.2f max=%d nonZero=%d"),
						*EditorGrassLayerInfoDebugName(Allocation.LayerInfo),
						Allocation.WeightmapTextureIndex,
						static_cast<int32>(Allocation.WeightmapTextureChannel),
						EditorGrassAverageWeight(LayerData),
						static_cast<int32>(EditorGrassMaxWeight(LayerData)),
						NonZero));
				}
			}
			if (AllocationNotes.IsEmpty())
			{
				AllocationNotes.Add(TEXT("weightmapLayerAllocations=None"));
			}

			const bool bEditorWorldGate = GIsEditor
				&& !GUsingNullRHI
				&& World
				&& !World->IsGameWorld()
				&& World->GetFeatureLevel() >= ERHIFeatureLevel::SM5;
			const AActor* Owner = Component->GetOwner();
			const FMaterialResource* MaterialResource = nullptr;
			if (MaterialInstance && World)
			{
				MaterialResource = MaterialInstance->GetMaterialResource(
					GetFeatureLevelShaderPlatform_Checked(World->GetFeatureLevel()));
			}

			const bool bHighlandTarget = IsHighlandEditorGrassTarget(GEditorGrassTargetMapPath);
			const bool bIncludeBindingDetails = !bHighlandTarget
				|| IsHighlandNativeClusterComponentName(Component->GetName())
				|| !Allocations.IsEmpty()
				|| Component->GrassData->WeightOffsets.Num() > 0;

			Snapshot.ComponentNotes.Add(FString::Printf(
				TEXT("%s component `%s`: editorWorldGate=%d registered=%d renderState=%d sceneProxy=%d shouldRender=%d shouldAddToScene=%d visible=%d ownerHiddenEd=%d ownerHiddenGame=%d materialInstanceCount=%d materialInterface=`%s` materialResource=%d materialGrass=%d canRenderGrassMap=%d grassElements=%d heightWeightBytes=%d weightTypes=%d hasWeightData=%d nonZeroWeightSamples=%lld."),
				*Label,
				*Component->GetName(),
				bEditorWorldGate ? 1 : 0,
				Component->IsRegistered() ? 1 : 0,
				Component->IsRenderStateCreated() ? 1 : 0,
				Component->SceneProxy ? 1 : 0,
				Component->ShouldRender() ? 1 : 0,
				Component->ShouldComponentAddToScene() ? 1 : 0,
				Component->GetVisibleFlag() ? 1 : 0,
				(Owner && Owner->IsHiddenEd()) ? 1 : 0,
				(Owner && Owner->IsHidden()) ? 1 : 0,
				MaterialInstanceCount,
				*EditorGrassObjectPath(MaterialInstance),
				MaterialResource ? 1 : 0,
				Component->MaterialHasGrass() ? 1 : 0,
				Component->CanRenderGrassMap() ? 1 : 0,
				Component->GrassData->NumElements,
				Component->GrassData->HeightWeightData.Num(),
				Component->GrassData->WeightOffsets.Num(),
				Component->GrassData->WeightOffsets.Num() > 0 ? 1 : 0,
				Snapshot.NonZeroGrassWeightSamples));
			if (bIncludeBindingDetails)
			{
				const TArray<UTexture2D*>& BaseWeightmaps = Component->GetWeightmapTextures(false);
				const TArray<UTexture2D*>& EditWeightmaps = Component->GetWeightmapTextures(true);
				const TArray<UTexture2D*>* RenderedWeightmaps = World
					? &Component->GetRenderedWeightmapTexturesForFeatureLevel(World->GetFeatureLevel())
					: nullptr;
				const FString RenderedWeightmapSummary = World
					? EditorGrassTextureSummary(*RenderedWeightmaps)
					: FString(TEXT("WorldMissing"));
				const FString CachedGrassTypes = MaterialInstance
					? EditorGrassTypeSummary(MaterialInstance->GetCachedExpressionData().GrassTypes)
					: FString(TEXT("NoMaterialInstance"));
				Snapshot.ComponentNotes.Add(FString::Printf(
					TEXT("%s component `%s` bindingDetails: componentGrassTypes=%s cachedMaterialGrassTypes=%s terrainLayerStaticParams=%s runtimeAllocations=%s baseWeightmaps=%s editWeightmaps=%s renderedWeightmaps=%s."),
					*Label,
					*Component->GetName(),
					*EditorGrassTypeSummary(Component->GetGrassTypes()),
					*CachedGrassTypes,
					*EditorGrassTerrainLayerParameterSummary(MaterialInstance),
					*EditorGrassAllocationSummary(Component->GetCurrentRuntimeWeightmapLayerAllocations()),
					*EditorGrassTextureSummary(BaseWeightmaps),
					*EditorGrassTextureSummary(EditWeightmaps),
					*RenderedWeightmapSummary));
				Snapshot.ComponentNotes.Add(FString::Printf(
					TEXT("%s component `%s` materialParentChain: %s."),
					*Label,
					*Component->GetName(),
					*EditorGrassMaterialParentChainSummary(MaterialInstance)));
				Snapshot.ComponentNotes.Add(FString::Printf(
					TEXT("%s component `%s` materialWeightParameters: %s."),
					*Label,
					*Component->GetName(),
					*EditorGrassMaterialWeightParameterSummary(MaterialInstance)));

				const int32 ProbeTextureIndex = !Allocations.IsEmpty()
					? Allocations[0].WeightmapTextureIndex
					: 0;
				TArray<FString> TextureSourceNotes;
				if (BaseWeightmaps.IsValidIndex(ProbeTextureIndex))
				{
					TextureSourceNotes.Add(FString::Printf(TEXT("base[%d]=%s"), ProbeTextureIndex, *EditorGrassTextureSourceWeightSummary(BaseWeightmaps[ProbeTextureIndex])));
				}
				if (EditWeightmaps.IsValidIndex(ProbeTextureIndex))
				{
					TextureSourceNotes.Add(FString::Printf(TEXT("edit[%d]=%s"), ProbeTextureIndex, *EditorGrassTextureSourceWeightSummary(EditWeightmaps[ProbeTextureIndex])));
				}
				if (RenderedWeightmaps && RenderedWeightmaps->IsValidIndex(ProbeTextureIndex))
				{
					TextureSourceNotes.Add(FString::Printf(TEXT("rendered[%d]=%s"), ProbeTextureIndex, *EditorGrassTextureSourceWeightSummary((*RenderedWeightmaps)[ProbeTextureIndex])));
				}
				Snapshot.ComponentNotes.Add(FString::Printf(
					TEXT("%s component `%s` weightmapSourceProbe: textureIndex=%d %s."),
					*Label,
					*Component->GetName(),
					ProbeTextureIndex,
					TextureSourceNotes.IsEmpty() ? TEXT("NoTextureAtProbeIndex") : *FString::Join(TextureSourceNotes, TEXT(" | "))));
			}
			for (const FString& AllocationNote : AllocationNotes)
			{
				Snapshot.ComponentNotes.Add(FString::Printf(TEXT("%s component `%s` %s."), *Label, *Component->GetName(), *AllocationNote));
			}
			for (const FString& GrassTypeNote : GrassTypeNotes)
			{
				Snapshot.ComponentNotes.Add(FString::Printf(TEXT("%s component `%s` %s."), *Label, *Component->GetName(), *GrassTypeNote));
			}
		}

		return Snapshot;
	}

	void AppendEditorGrassSnapshot(FString& Report, const FString& Label, const FEditorGrassSnapshot& Snapshot)
	{
		Report += FString::Printf(TEXT("## %s\n"), *Label);
		Report += FString::Printf(
			TEXT("- components=%d registered=%d renderState=%d sceneProxy=%d materialInstance=%d materialGrass=%d canRenderGrassMap=%d validGrassData=%d grassElements=%lld grassWeightTypes=%d grassWeightBytes=%lld nonZeroWeightSamples=%lld weightmapLayerAllocations=%d nonZeroLandscapeLayerSamples=%lld outdatedGrassMaps=%d\n"),
			Snapshot.Components,
			Snapshot.Registered,
			Snapshot.RenderStateCreated,
			Snapshot.SceneProxy,
			Snapshot.MaterialInstances,
			Snapshot.MaterialGrass,
			Snapshot.CanRenderGrassMap,
			Snapshot.ValidGrassData,
			Snapshot.GrassElements,
			Snapshot.GrassWeightTypes,
			Snapshot.GrassWeightBytes,
			Snapshot.NonZeroGrassWeightSamples,
			Snapshot.WeightmapLayerAllocations,
			Snapshot.NonZeroLandscapeLayerSamples,
			Snapshot.OutdatedGrassMaps);
		for (const FString& Note : Snapshot.ComponentNotes)
		{
			Report += FString::Printf(TEXT("- %s\n"), *Note);
		}
		Report += TEXT("\n");
	}

	bool IsEditorGrassSnapshotReadyForBuild(const FEditorGrassSnapshot& Snapshot)
	{
		return Snapshot.WeightmapLayerAllocations > 0
			&& Snapshot.NonZeroLandscapeLayerSamples > 0
			&& Snapshot.Components > 0
			&& Snapshot.CanRenderGrassMap == Snapshot.Components;
	}

	void FinishEditorGrassMaterialCompilation(ALandscape* Landscape, FString& Report)
	{
		if (!Landscape)
		{
			return;
		}

		UWorld* World = Landscape->GetWorld();
		if (World)
		{
			UMaterialInterface::SubmitRemainingJobsForWorld(World);
		}
		FAssetCompilingManager::Get().FinishAllCompilation();

		int32 MaterialResources = 0;
		int32 FinishedResources = 0;
		for (ULandscapeComponent* Component : Landscape->LandscapeComponents)
		{
			if (!Component)
			{
				continue;
			}

			const int32 MaterialInstanceCount = Component->GetMaterialInstanceCount(false);
			UMaterialInstance* MaterialInstance = MaterialInstanceCount > 0 ? Component->GetMaterialInstance(0, false) : nullptr;
			if (!MaterialInstance || !World)
			{
				continue;
			}

			if (FMaterialResource* MaterialResource = MaterialInstance->GetMaterialResource(GetFeatureLevelShaderPlatform_Checked(World->GetFeatureLevel())))
			{
				++MaterialResources;
				MaterialResource->FinishCompilation();
				++FinishedResources;
			}
		}

		if (World)
		{
			World->SendAllEndOfFrameUpdates();
		}

		Report += FString::Printf(
			TEXT("## Material Compilation Finish\n- Submitted remaining material jobs and finished %d/%d landscape material resources without refreshing component material instances.\n\n"),
			FinishedResources,
			MaterialResources);
	}
}
#endif

class FTP_ThirdPersonModule : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override
	{
		FDefaultGameModuleImpl::StartupModule();

#if WITH_EDITOR
		const bool bRunEditorGrassMapBuild = !IsRunningCommandlet()
			&& (FParse::Param(FCommandLine::Get(), TEXT("FFEditorGrassMapBuild"))
				|| FParse::Param(FCommandLine::Get(), TEXT("FFEditorLayerResolveGrassMapBuild")));
		if (bRunEditorGrassMapBuild)
		{
			ConfigureEditorGrassWorkflowFromCommandLine();
			bEditorGrassResolveLayersBeforeBuild = FParse::Param(FCommandLine::Get(), TEXT("FFEditorLayerResolveGrassMapBuild"));
			UE_LOG(LogTP_ThirdPerson, Display, TEXT("Starting full-editor GrassMap build workflow for %s."), *GEditorGrassTargetMapPath);
			EditorGrassTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
				FTickerDelegate::CreateRaw(this, &FTP_ThirdPersonModule::TickEditorGrassMapBuild),
				0.25f);
		}
#endif
	}

	virtual void ShutdownModule() override
	{
#if WITH_EDITOR
		if (EditorGrassTickerHandle.IsValid())
		{
			FTSTicker::GetCoreTicker().RemoveTicker(EditorGrassTickerHandle);
			EditorGrassTickerHandle.Reset();
		}
#endif

		FDefaultGameModuleImpl::ShutdownModule();
	}

#if WITH_EDITOR
private:
	enum class EEditorGrassStage : uint8
	{
		LoadMap,
		WaitForSceneProxy,
		ResolveLayers,
		WaitForLayerResolve,
		BuildGrassMaps,
		WaitForGrassData,
		SaveAndExit,
		BlockedExit
	};

	bool TickEditorGrassMapBuild(float DeltaTime)
	{
		++EditorGrassTickCount;

		switch (EditorGrassStage)
		{
		case EEditorGrassStage::LoadMap:
			return LoadEditorGrassTargetMap();
		case EEditorGrassStage::WaitForSceneProxy:
			return WaitForEditorGrassSceneProxy();
		case EEditorGrassStage::ResolveLayers:
			return ResolveEditorGrassLayers();
		case EEditorGrassStage::WaitForLayerResolve:
			return WaitForEditorGrassLayerResolve();
		case EEditorGrassStage::BuildGrassMaps:
			return BuildEditorGrassMaps();
		case EEditorGrassStage::WaitForGrassData:
			return WaitForEditorGrassData();
		case EEditorGrassStage::SaveAndExit:
			return SaveEditorGrassResultAndExit();
		case EEditorGrassStage::BlockedExit:
			return WriteEditorGrassBlockedAndExit();
		default:
			return false;
		}
	}

	bool LoadEditorGrassTargetMap()
	{
		if (IConsoleVariable* FixedStreamingPool = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Streaming.UseFixedPoolSize")))
		{
			FixedStreamingPool->Set(1, ECVF_SetByCode);
		}
		if (IConsoleVariable* StreamingPoolSize = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Streaming.PoolSize")))
		{
			StreamingPoolSize->Set(128, ECVF_SetByCode);
		}

		Report = TEXT("# Full Editor Landscape GrassMap Build\n\n");
		Report += bEditorGrassResolveLayersBeforeBuild
			? TEXT("Mode: explicit full-editor workflow launched by `-FFEditorLayerResolveGrassMapBuild`; not a commandlet. Resolves target landscape layer content before GrassMap build.\n\n")
			: TEXT("Mode: explicit full-editor workflow launched by `-FFEditorGrassMapBuild`; not a commandlet.\n\n");
		Report += FString::Printf(TEXT("- Target map: `%s`\n"), *GEditorGrassTargetMapPath);
		Report += FString::Printf(TEXT("- GIsEditor=%d GUsingNullRHI=%d IsRunningCommandlet=%d\n\n"),
			GIsEditor ? 1 : 0,
			GUsingNullRHI ? 1 : 0,
			IsRunningCommandlet() ? 1 : 0);
		Report += TEXT("- Validation-only texture streaming pool: fixed at 128 MB for the low-memory GrassMap bake.\n\n");

		UWorld* ActiveEditorWorld = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
		const bool bCanReuseActiveWorld = ActiveEditorWorld
			&& ActiveEditorWorld->GetOutermost()
			&& ActiveEditorWorld->GetOutermost()->GetName().Equals(GEditorGrassTargetMapPath, ESearchCase::IgnoreCase);
		EditorGrassWorld = bCanReuseActiveWorld
			? ActiveEditorWorld
			: UEditorLoadingAndSavingUtils::LoadMap(GEditorGrassTargetMapPath);
		if (!EditorGrassWorld)
		{
			Report += TEXT("- Loaded world: missing\n");
			Report += TEXT("- Landscape: missing\n\n");
			BlockedReason = FString::Printf(TEXT("Could not load target map `%s` in full editor context."), *GEditorGrassTargetMapPath);
			EditorGrassStage = EEditorGrassStage::BlockedExit;
			return true;
		}
		Report += bCanReuseActiveWorld
			? TEXT("- World acquisition: reused the already loaded target editor world to avoid a duplicate full-map allocation.\n")
			: TEXT("- World acquisition: loaded the requested target map because a matching editor world was not already active.\n");

		EditorGrassLandscape = FindEditorGrassTargetLandscape(EditorGrassWorld);
		if (!EditorGrassLandscape)
		{
			Report += FString::Printf(TEXT("- Loaded world: `%s`\n"), *EditorGrassWorld->GetPathName());
			Report += TEXT("- Landscape: missing\n\n");
			BlockedReason = FString::Printf(TEXT("Target map `%s` has no ALandscape actor."), *GEditorGrassTargetMapPath);
			EditorGrassStage = EEditorGrassStage::BlockedExit;
			return true;
		}

		Report += FString::Printf(TEXT("- Loaded world: `%s`\n"), *EditorGrassWorld->GetPathName());
		Report += FString::Printf(TEXT("- Landscape: `%s`\n"), *EditorGrassLandscape->GetActorLabel());
		Report += FString::Printf(TEXT("- Landscape material: `%s`\n\n"), *EditorGrassObjectPath(EditorGrassLandscape->LandscapeMaterial));
		Report += TEXT("## Expected Grassland LayerInfos\n");
		for (ULandscapeLayerInfoObject* LayerInfo : LoadEditorGrasslandLayerInfos())
		{
			Report += FString::Printf(TEXT("- %s\n"), *EditorGrassLayerInfoDebugName(LayerInfo));
		}
		Report += TEXT("\n");

		EditorGrassLandscape->RegisterAllComponents();
		ULandscapeInfo* LandscapeInfo = EditorGrassLandscape->CreateLandscapeInfo(false, true);
		EditorGrassLandscape->SetDisableRuntimeGrassMapGeneration(false);
		if (LandscapeInfo)
		{
			LandscapeInfo->UpdateAllComponentMaterialInstances(true);
		}
		EditorGrassLandscape->UpdateAllComponentMaterialInstances(true);
		EditorGrassLandscape->ReregisterAllComponents();
		EditorGrassWorld->UpdateWorldComponents(true, false);
		EditorGrassWorld->SendAllEndOfFrameUpdates();
		EditorGrassStageStartTick = EditorGrassTickCount;
		EditorGrassStage = EEditorGrassStage::WaitForSceneProxy;
		return true;
	}

	bool WaitForEditorGrassSceneProxy()
	{
		if (!EditorGrassLandscape || !EditorGrassWorld)
		{
			BlockedReason = TEXT("Lost target landscape/world while waiting for SceneProxy.");
			EditorGrassStage = EEditorGrassStage::BlockedExit;
			return true;
		}

		EditorGrassWorld->UpdateWorldComponents(true, false);
		EditorGrassWorld->SendAllEndOfFrameUpdates();

		const FEditorGrassSnapshot Snapshot = CaptureEditorGrassSnapshot(EditorGrassLandscape, TEXT("SceneProxy wait"));
		if (Snapshot.SceneProxy > 0 && Snapshot.CanRenderGrassMap > 0)
		{
			AppendEditorGrassSnapshot(Report, bEditorGrassResolveLayersBeforeBuild ? TEXT("Before Layer Resolve") : TEXT("Before GrassMap Build"), Snapshot);
			EditorGrassStage = bEditorGrassResolveLayersBeforeBuild ? EEditorGrassStage::ResolveLayers : EEditorGrassStage::BuildGrassMaps;
			return true;
		}

		if ((EditorGrassTickCount - EditorGrassStageStartTick) >= 80)
		{
			AppendEditorGrassSnapshot(Report, TEXT("SceneProxy wait timeout"), Snapshot);
			BlockedReason = TEXT("Full editor opened the target map but did not create a renderable Landscape SceneProxy/GrassMap gate within 80 ticks.");
			EditorGrassStage = EEditorGrassStage::BlockedExit;
			return true;
		}

		return true;
	}

	bool ResolveEditorGrassLayers()
	{
		if (!EditorGrassLandscape || !EditorGrassWorld)
		{
			BlockedReason = TEXT("Lost target landscape/world before resolving layer content.");
			EditorGrassStage = EEditorGrassStage::BlockedExit;
			return true;
		}

		Report += TEXT("## Layer Resolve Request\n");
		Report += TEXT("- Calling `RequestLayersContentUpdateForceAll(Update_All, true)` and `ForceUpdateLayersContent()` in full editor render context before GrassMap build.\n\n");

		EditorGrassLandscape->Modify();
		if (IsHighlandEditorGrassTarget(GEditorGrassTargetMapPath)
			&& !RepaintHighlandNativeMiniLayerInEditor(EditorGrassLandscape, Report))
		{
			BlockedReason = TEXT("Highland native mini proof layer repaint failed in full editor context before layer resolve.");
			EditorGrassStage = EEditorGrassStage::BlockedExit;
			return true;
		}
		EditorGrassLandscape->RequestLayersContentUpdateForceAll(ELandscapeLayerUpdateMode::Update_All, true);
		EditorGrassLandscape->ForceUpdateLayersContent();
		if (IsHighlandEditorGrassTarget(GEditorGrassTargetMapPath)
			&& !RepaintHighlandNativeMiniLayerInEditor(EditorGrassLandscape, Report, /*bRequestLayerResolveAfterPaint=*/false))
		{
			BlockedReason = TEXT("Highland native mini proof base weightmap repaint failed after full editor layer resolve.");
			EditorGrassStage = EEditorGrassStage::BlockedExit;
			return true;
		}
		const bool bHighlandMiniTarget = IsHighlandEditorGrassTarget(GEditorGrassTargetMapPath);
		if (!bHighlandMiniTarget)
		{
			EditorGrassLandscape->UpdateAllComponentMaterialInstances(true);
			for (ULandscapeComponent* Component : EditorGrassLandscape->LandscapeComponents)
			{
				if (Component)
				{
					Component->Modify();
					Component->UpdateMaterialInstances();
					Component->UpdateGrassTypes(true);
				}
			}
			if (ULandscapeInfo* LandscapeInfo = EditorGrassLandscape->GetLandscapeInfo())
			{
				LandscapeInfo->UpdateLayerInfoMap(EditorGrassLandscape);
				LandscapeInfo->UpdateAllComponentMaterialInstances(true);
			}
		}
		else
		{
			const FEditorGrassSnapshot ImmediateSnapshot = CaptureEditorGrassSnapshot(EditorGrassLandscape, TEXT("Highland post-resolve paint"));
			if (ImmediateSnapshot.WeightmapLayerAllocations > 0 && ImmediateSnapshot.NonZeroLandscapeLayerSamples > 0)
			{
				AppendEditorGrassSnapshot(Report, TEXT("After Layer Resolve"), ImmediateSnapshot);
				if (IsEditorGrassSnapshotReadyForBuild(ImmediateSnapshot))
				{
					EditorGrassStage = EEditorGrassStage::BuildGrassMaps;
					return BuildEditorGrassMaps();
				}
				FinishEditorGrassMaterialCompilation(EditorGrassLandscape, Report);
				const FEditorGrassSnapshot PostCompileSnapshot = CaptureEditorGrassSnapshot(EditorGrassLandscape, TEXT("Highland post-compile no-refresh"), /*bRefreshComponentState=*/false);
				AppendEditorGrassSnapshot(Report, TEXT("After Material Compile Finish"), PostCompileSnapshot);
				if (IsEditorGrassSnapshotReadyForBuild(PostCompileSnapshot))
				{
					EditorGrassStage = EEditorGrassStage::BuildGrassMaps;
					return BuildEditorGrassMaps();
				}
				Report += TEXT("## Layer Resolve Wait\n");
				Report += TEXT("- Non-zero Highland proof layer is present, but at least one affected component cannot render GrassMap yet. Waiting for material/resource readiness before BuildGrassMaps.\n\n");
				EditorGrassWorld->UpdateWorldComponents(true, false);
				EditorGrassWorld->SendAllEndOfFrameUpdates();
				GEditor->RedrawAllViewports(false);
				EditorGrassStageStartTick = EditorGrassTickCount;
				EditorGrassStage = EEditorGrassStage::WaitForLayerResolve;
				return true;
			}
		}
		EditorGrassLandscape->MarkPackageDirty();
		EditorGrassWorld->UpdateWorldComponents(true, false);
		EditorGrassWorld->SendAllEndOfFrameUpdates();
		GEditor->RedrawAllViewports(false);
		EditorGrassStageStartTick = EditorGrassTickCount;
		EditorGrassStage = EEditorGrassStage::WaitForLayerResolve;
		return true;
	}

	bool WaitForEditorGrassLayerResolve()
	{
		if (!EditorGrassLandscape || !EditorGrassWorld)
		{
			BlockedReason = TEXT("Lost target landscape/world while waiting for layer resolve.");
			EditorGrassStage = EEditorGrassStage::BlockedExit;
			return true;
		}

		EditorGrassWorld->UpdateWorldComponents(true, false);
		EditorGrassWorld->SendAllEndOfFrameUpdates();
		GEditor->RedrawAllViewports(false);

		const FEditorGrassSnapshot Snapshot = CaptureEditorGrassSnapshot(
			EditorGrassLandscape,
			TEXT("LayerResolve wait"),
			!IsHighlandEditorGrassTarget(GEditorGrassTargetMapPath));
		if (IsEditorGrassSnapshotReadyForBuild(Snapshot))
		{
			AppendEditorGrassSnapshot(Report, TEXT("After Layer Resolve"), Snapshot);
			EditorGrassStage = EEditorGrassStage::BuildGrassMaps;
			return true;
		}

		if ((EditorGrassTickCount - EditorGrassStageStartTick) >= 80)
		{
			AppendEditorGrassSnapshot(Report, TEXT("LayerResolve wait timeout"), Snapshot);
			BlockedReason = TEXT("Full editor layer resolve completed without producing non-zero component weightmap layer allocations/samples.");
			EditorGrassStage = EEditorGrassStage::BlockedExit;
			return true;
		}

		return true;
	}

	bool BuildEditorGrassMaps()
	{
		if (!EditorGrassLandscape || !EditorGrassWorld)
		{
			BlockedReason = TEXT("Lost target landscape/world before BuildGrassMaps.");
			EditorGrassStage = EEditorGrassStage::BlockedExit;
			return true;
		}

		Report += TEXT("## Build Request\n");
		Report += TEXT("- Calling `ALandscapeProxy::BuildGrassMaps()` after SceneProxy/CanRenderGrassMap succeeded.\n\n");
		if (IsHighlandEditorGrassTarget(GEditorGrassTargetMapPath))
		{
			Report += TEXT("## Highland Render State Sync\n");
			Report += TEXT("- Rebuilding Landscape material instances and re-registering components immediately before GrassMap render so the SceneProxy uses the mini-proof Grassland_Grass layer combination.\n\n");
			EditorGrassLandscape->UpdateAllComponentMaterialInstances(true);
			const int32 MirroredComponentMICs = EditorGrassForceComponentTerrainLayerStaticParams(EditorGrassLandscape, Report);
			Report += FString::Printf(TEXT("- component MIC static terrain layer mirrors=%d\n\n"), MirroredComponentMICs);
			EditorGrassLandscape->ReregisterAllComponents();
			EditorGrassWorld->UpdateWorldComponents(true, false);
			EditorGrassWorld->SendAllEndOfFrameUpdates();
			GEditor->RedrawAllViewports(false);
			AppendEditorGrassSnapshot(Report, TEXT("Before GrassMap Build render-state sync"), CaptureEditorGrassSnapshot(EditorGrassLandscape, TEXT("Pre-build render-state sync"), /*bRefreshComponentState=*/false));
			AppendEditorGrassSnapshot(Report, TEXT("Before GrassMap Build (no refresh)"), CaptureEditorGrassSnapshot(EditorGrassLandscape, TEXT("Pre-build no-refresh"), /*bRefreshComponentState=*/false));
		}
		EditorGrassLandscape->BuildGrassMaps();
		EditorGrassWorld->SendAllEndOfFrameUpdates();
		EditorGrassStageStartTick = EditorGrassTickCount;
		EditorGrassStage = EEditorGrassStage::WaitForGrassData;
		return true;
	}

	bool WaitForEditorGrassData()
	{
		if (!EditorGrassLandscape || !EditorGrassWorld)
		{
			BlockedReason = TEXT("Lost target landscape/world while waiting for GrassData.");
			EditorGrassStage = EEditorGrassStage::BlockedExit;
			return true;
		}

		EditorGrassWorld->SendAllEndOfFrameUpdates();
		const FEditorGrassSnapshot Snapshot = CaptureEditorGrassSnapshot(
			EditorGrassLandscape,
			TEXT("GrassData wait"),
			!IsHighlandEditorGrassTarget(GEditorGrassTargetMapPath));
		const int32 RequiredGrassWeightTypes = IsHighlandEditorGrassTarget(GEditorGrassTargetMapPath)
			? UE_ARRAY_COUNT(HighlandNativeClusterComponentNames)
			: 1;
		if (Snapshot.GrassElements > 0
			&& Snapshot.GrassWeightTypes >= RequiredGrassWeightTypes
			&& Snapshot.NonZeroGrassWeightSamples > 0
			&& Snapshot.OutdatedGrassMaps == 0)
		{
			AppendEditorGrassSnapshot(Report, TEXT("After GrassMap Build"), Snapshot);
			EditorGrassStage = EEditorGrassStage::SaveAndExit;
			return true;
		}

		if ((EditorGrassTickCount - EditorGrassStageStartTick) >= 80)
		{
			AppendEditorGrassSnapshot(Report, TEXT("GrassData wait timeout"), Snapshot);
			BlockedReason = FString::Printf(
				TEXT("BuildGrassMaps ran in full editor context, but serialized GrassData did not reach the required target count before timeout. grassWeightTypes=%d required=%d nonZeroWeightSamples=%lld outdatedGrassMaps=%d."),
				Snapshot.GrassWeightTypes,
				RequiredGrassWeightTypes,
				Snapshot.NonZeroGrassWeightSamples,
				Snapshot.OutdatedGrassMaps);
			EditorGrassStage = EEditorGrassStage::BlockedExit;
			return true;
		}

		return true;
	}

	bool SaveEditorGrassResultAndExit()
	{
		if (!EditorGrassWorld)
		{
			BlockedReason = TEXT("Lost target world before save.");
			EditorGrassStage = EEditorGrassStage::BlockedExit;
			return true;
		}

		const bool bSaved = UEditorLoadingAndSavingUtils::SaveMap(EditorGrassWorld, GEditorGrassTargetMapPath);
		Report += TEXT("## Save\n");
		Report += FString::Printf(TEXT("- SaveMap result=%d\n\n"), bSaved ? 1 : 0);
		Report += bSaved
			? TEXT("Result: PASS - Full-editor SceneProxy gate opened and native GrassMap data generated/saved for the target map.\n")
			: TEXT("Result: BLOCKED - GrassMap data generated, but SaveMap failed.\n");
		FFileHelper::SaveStringToFile(Report, *EditorGrassSavedReportPath());
		UE_LOG(LogTP_ThirdPerson, Display, TEXT("Editor GrassMap workflow report: %s"), *EditorGrassSavedReportPath());
		FPlatformMisc::RequestExit(false);
		return false;
	}

	bool WriteEditorGrassBlockedAndExit()
	{
		Report += TEXT("## Blocker\n");
		Report += FString::Printf(TEXT("- %s\n\n"), *BlockedReason);
		Report += TEXT("Result: BLOCKED - Full-editor automation could not prove native GrassMap generation.\n");
		FFileHelper::SaveStringToFile(Report, *EditorGrassSavedReportPath());
		UE_LOG(LogTP_ThirdPerson, Warning, TEXT("Editor GrassMap workflow blocked: %s"), *BlockedReason);
		UE_LOG(LogTP_ThirdPerson, Warning, TEXT("Editor GrassMap workflow report: %s"), *EditorGrassSavedReportPath());
		FPlatformMisc::RequestExit(false);
		return false;
	}

	FTSTicker::FDelegateHandle EditorGrassTickerHandle;
	EEditorGrassStage EditorGrassStage = EEditorGrassStage::LoadMap;
	int32 EditorGrassTickCount = 0;
	int32 EditorGrassStageStartTick = 0;
	bool bEditorGrassResolveLayersBeforeBuild = false;
	UWorld* EditorGrassWorld = nullptr;
	ALandscape* EditorGrassLandscape = nullptr;
	FString Report;
	FString BlockedReason;
#endif
};

IMPLEMENT_PRIMARY_GAME_MODULE( FTP_ThirdPersonModule, TP_ThirdPerson, "TP_ThirdPerson" );
