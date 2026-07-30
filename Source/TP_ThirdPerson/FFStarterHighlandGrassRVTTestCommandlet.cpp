#include "FFStarterHighlandGrassRVTTestCommandlet.h"

#if WITH_EDITOR
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/RuntimeVirtualTextureComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "FileHelpers.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerStart.h"
#include "Landscape.h"
#include "LandscapeComponent.h"
#include "LandscapeEdit.h"
#include "LandscapeEditLayer.h"
#include "LandscapeGrassType.h"
#include "LandscapeInfo.h"
#include "LandscapeLayerInfoObject.h"
#include "LandscapeProxy.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionCustom.h"
#include "Materials/MaterialExpressionLandscapeGrassOutput.h"
#include "Materials/MaterialExpressionLandscapeLayerSample.h"
#include "Materials/MaterialExpressionLandscapeLayerWeight.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionRuntimeVirtualTextureOutput.h"
#include "Materials/MaterialExpressionVertexNormalWS.h"
#include "Materials/MaterialExpressionWorldPosition.h"
#include "Materials/MaterialInterface.h"
#include "MaterialEditingLibrary.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "VT/RuntimeVirtualTexture.h"
#include "VT/RuntimeVirtualTextureVolume.h"
#endif

UFFStarterHighlandGrassRVTTestCommandlet::UFFStarterHighlandGrassRVTTestCommandlet()
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
	const TCHAR* HighlandLandscapeMaterialPath = TEXT("/Game/FantasyFrontier/Blockout/Materials/M_FF_Blockout_Grass_Green.M_FF_Blockout_Grass_Green");
	const TCHAR* RVTTitanDPath = TEXT("/Game/Landscape/RVT/RVT_Titan_D.RVT_Titan_D");
	const TCHAR* RVTTitanHPath = TEXT("/Game/Landscape/RVT/RVT_Titan_H.RVT_Titan_H");
	const TCHAR* RVTTitanDHPath = TEXT("/Game/Landscape/RVT/RVT_Titan_DH.RVT_Titan_DH");
	const TCHAR* TitanGrassTypePath = TEXT("/Game/Landscape/LGT/LGT_Grass.LGT_Grass");
	const TCHAR* GrasslandGrassLayerInfoPath = TEXT("/Game/Landscape/WeightLayers/LI_Grassland_Grass.LI_Grassland_Grass");
	const TCHAR* NativeGrassMaterialPath = TEXT("/Game/Environment/Foliage/Materials/MI_GrassBlade.MI_GrassBlade");
	const TCHAR* FallbackGrassMaterialPath = TEXT("/Game/FantasyFrontier/Blockout/Materials/M_FF_Blockout_GrassBlade.M_FF_Blockout_GrassBlade");
	const TCHAR* GrassMeshPath = TEXT("/Game/Environment/Foliage/Meshes/SM_GrassBlade.SM_GrassBlade");

	const FName RVTSupportTag(TEXT("FFHighlandTitanGrassRVTSupport"));
	const FName TestPatchTag(TEXT("FFHighlandTitanGrassNativeTestPatch"));
	const FName MiniProofTag(TEXT("FFHighlandTitanNativeMiniProof"));
	const FName MiniRVTTag(TEXT("FFHighlandTitanNativeMiniRVT"));
	const FName LegacyMiniGrassInputName(TEXT("FF_Highland_NativeTitan_Mini"));
	const FName MiniGrassInputName(TEXT("Grassland_Grass"));
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

	struct FNativeMiniPaintResult
	{
		int32 PaintedSamples = 0;
		int32 ComponentMapHits = 0;
		int32 ComponentsWithBaseGrassAllocation = 0;
		int32 ComponentsWithEditGrassAllocation = 0;
		int32 ComponentsWithTargetGrassAllocation = 0;
		int32 NonZeroVerifySamples = 0;
		FGuid PaintEditLayerGuid;
		FName PaintEditLayerName = NAME_None;
		FString DebugText;
	};

	float PseudoRandom01(const int32 Seed)
	{
		return FMath::Frac(FMath::Sin(static_cast<float>(Seed) * 12.9898f + 78.233f) * 43758.5453f);
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

	FBox BuildHighlandNativeClusterRVTBounds(ALandscapeProxy* Landscape, const FVector& FallbackCenter, float FallbackRadius, int32& OutComponentCount)
	{
		OutComponentCount = 0;
		FBox Bounds(ForceInit);
		if (Landscape)
		{
			for (ULandscapeComponent* Component : Landscape->LandscapeComponents)
			{
				if (!Component || !IsHighlandNativeClusterComponentName(Component->GetName()))
				{
					continue;
				}

				Bounds += Component->Bounds.GetBox();
				++OutComponentCount;
			}
		}

		if (!Bounds.IsValid)
		{
			const FVector BoundsMin(FallbackCenter.X - FallbackRadius - 1600.0f, FallbackCenter.Y - FallbackRadius - 1600.0f, FallbackCenter.Z - 1600.0f);
			const FVector BoundsMax(FallbackCenter.X + FallbackRadius + 1600.0f, FallbackCenter.Y + FallbackRadius + 1600.0f, FallbackCenter.Z + 4200.0f);
			return FBox(BoundsMin, BoundsMax);
		}

		return Bounds.ExpandBy(FVector(1600.0f, 1600.0f, 4200.0f));
	}

	ALandscapeProxy* FindPrimaryLandscape(UWorld* World)
	{
		for (TActorIterator<ALandscapeProxy> It(World); It; ++It)
		{
			return *It;
		}
		return nullptr;
	}

	void RemoveTaggedActors(UWorld* World)
	{
		TArray<AActor*> ActorsToRemove;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && (Actor->ActorHasTag(RVTSupportTag) || Actor->ActorHasTag(TestPatchTag)))
			{
				ActorsToRemove.Add(Actor);
			}
		}

		for (AActor* Actor : ActorsToRemove)
		{
			World->DestroyActor(Actor);
		}

		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandGrassRVTTest: removed tagged test actors=%d"), ActorsToRemove.Num());
	}

	void RemoveMiniProofActors(UWorld* World)
	{
		TArray<AActor*> ActorsToRemove;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && (Actor->ActorHasTag(MiniProofTag) || Actor->ActorHasTag(MiniRVTTag)))
			{
				ActorsToRemove.Add(Actor);
			}
		}

		for (AActor* Actor : ActorsToRemove)
		{
			World->DestroyActor(Actor);
		}

		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandGrassRVTTest: removed mini-proof actors=%d"), ActorsToRemove.Num());
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

	bool AddOrRefreshRVTOutput(UMaterial* Material)
	{
		if (!Material)
		{
			return false;
		}

		Material->Modify();
		TArray<UMaterialExpression*> ExistingExpressions;
		for (UMaterialExpression* Expression : Material->GetExpressions())
		{
			ExistingExpressions.Add(Expression);
		}
		for (UMaterialExpression* Expression : ExistingExpressions)
		{
			if (Expression && Expression->IsA<UMaterialExpressionRuntimeVirtualTextureOutput>())
			{
				UMaterialEditingLibrary::DeleteMaterialExpression(Material, Expression);
			}
		}

		FExpressionInput* BaseColorInput = Material->GetExpressionInputForProperty(MP_BaseColor);
		if (!BaseColorInput || !BaseColorInput->Expression)
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandGrassRVTTest: landscape material has no BaseColor expression to write into RVT."));
			return false;
		}

		UMaterialExpressionRuntimeVirtualTextureOutput* RVTOutput = Cast<UMaterialExpressionRuntimeVirtualTextureOutput>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionRuntimeVirtualTextureOutput::StaticClass(), 1240, -160));
		UMaterialExpressionConstant* Specular = Cast<UMaterialExpressionConstant>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant::StaticClass(), 920, 20));
		UMaterialExpressionConstant* Roughness = Cast<UMaterialExpressionConstant>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant::StaticClass(), 920, 120));
		UMaterialExpressionConstant* Opacity = Cast<UMaterialExpressionConstant>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant::StaticClass(), 920, 220));
		UMaterialExpressionWorldPosition* WorldPosition = Cast<UMaterialExpressionWorldPosition>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionWorldPosition::StaticClass(), 920, 330));
		UMaterialExpressionComponentMask* HeightMask = Cast<UMaterialExpressionComponentMask>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionComponentMask::StaticClass(), 1080, 330));
		UMaterialExpressionVertexNormalWS* Normal = Cast<UMaterialExpressionVertexNormalWS>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionVertexNormalWS::StaticClass(), 920, 460));

		if (!RVTOutput || !Specular || !Roughness || !Opacity || !WorldPosition || !HeightMask || !Normal)
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandGrassRVTTest: failed creating RVT output nodes."));
			return false;
		}

		Specular->R = 0.2f;
		Roughness->R = 0.9f;
		Opacity->R = 1.0f;
		WorldPosition->WorldPositionShaderOffset = WPT_ExcludeAllShaderOffsets;
		HeightMask->Input.Expression = WorldPosition;
		HeightMask->B = true;

		RVTOutput->BaseColor = *BaseColorInput;
		RVTOutput->Specular.Expression = Specular;
		RVTOutput->Roughness.Expression = Roughness;
		RVTOutput->Opacity.Expression = Opacity;
		RVTOutput->WorldHeight.Expression = HeightMask;
		RVTOutput->Displacement.Expression = HeightMask;
		RVTOutput->Normal.Expression = Normal;

		Material->bUsedWithInstancedStaticMeshes = true;
		Material->bUsedWithNanite = true;
		UMaterialEditingLibrary::LayoutMaterialExpressions(Material);
		UMaterialEditingLibrary::RecompileMaterial(Material);
		Material->PostEditChange();
		Material->MarkPackageDirty();
		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandGrassRVTTest: added RVT output to %s"), *Material->GetPathName());
		return true;
	}

	bool RemoveRVTOutput(UMaterial* Material)
	{
		if (!Material)
		{
			return false;
		}

		Material->Modify();
		int32 RemovedOutputs = 0;
		TArray<UMaterialExpression*> ExistingExpressions;
		for (UMaterialExpression* Expression : Material->GetExpressions())
		{
			ExistingExpressions.Add(Expression);
		}
		for (UMaterialExpression* Expression : ExistingExpressions)
		{
			if (Expression && Expression->IsA<UMaterialExpressionRuntimeVirtualTextureOutput>())
			{
				UMaterialEditingLibrary::DeleteMaterialExpression(Material, Expression);
				++RemovedOutputs;
			}
		}

		if (RemovedOutputs > 0)
		{
			UMaterialEditingLibrary::LayoutMaterialExpressions(Material);
			UMaterialEditingLibrary::RecompileMaterial(Material);
			Material->PostEditChange();
			Material->MarkPackageDirty();
		}

		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandGrassRVTTest: removed RVT output nodes=%d from %s"), RemovedOutputs, *Material->GetPathName());
		return true;
	}

	bool AddOrRefreshNativeMiniGrassOutput(UMaterial* Material, ULandscapeGrassType* GrassType, FName GrassLayerName, const FVector2D& Center, float Radius)
	{
		if (!Material || !GrassType || GrassLayerName.IsNone())
		{
			return false;
		}

		Material->Modify();

		TArray<UMaterialExpressionLandscapeGrassOutput*> GrassOutputs;
		for (UMaterialExpression* Expression : Material->GetExpressions())
		{
			UMaterialExpressionLandscapeGrassOutput* GrassOutput = Cast<UMaterialExpressionLandscapeGrassOutput>(Expression);
			if (GrassOutput)
			{
				GrassOutputs.Add(GrassOutput);
			}
		}

		UMaterialExpressionLandscapeGrassOutput* TargetGrassOutput = GrassOutputs.IsEmpty() ? nullptr : GrassOutputs[0];
		auto HasGrassInputName = [](const UMaterialExpressionLandscapeGrassOutput* GrassOutput, FName Name)
		{
			if (!GrassOutput || Name.IsNone())
			{
				return false;
			}
			for (const FGrassInput& Input : GrassOutput->GrassTypes)
			{
				if (Input.Name == Name)
				{
					return true;
				}
			}
			return false;
		};

		if (TargetGrassOutput)
		{
			for (int32 OutputIndex = 0; OutputIndex < GrassOutputs.Num(); ++OutputIndex)
			{
				UMaterialExpressionLandscapeGrassOutput* GrassOutput = GrassOutputs[OutputIndex];
				if (!GrassOutput)
				{
					continue;
				}

				// The material can compile with only one LandscapeGrassOutput node.
				// Merge any legacy/proof outputs into the first node, then delete
				// the extras so Highland keeps its existing grass plus this mini proof.
				if (GrassOutput != TargetGrassOutput)
				{
					for (const FGrassInput& Input : GrassOutput->GrassTypes)
					{
						if (Input.Name != MiniGrassInputName && Input.Name != LegacyMiniGrassInputName && !HasGrassInputName(TargetGrassOutput, Input.Name))
						{
							TargetGrassOutput->GrassTypes.Add(Input);
						}
					}
					UMaterialEditingLibrary::DeleteMaterialExpression(Material, GrassOutput);
					continue;
				}

				TSet<FName> SeenInputNames;
				for (int32 InputIndex = GrassOutput->GrassTypes.Num() - 1; InputIndex >= 0; --InputIndex)
				{
					const FName InputName = GrassOutput->GrassTypes[InputIndex].Name;
					if (!GrassOutput->GrassTypes[InputIndex].GrassType
						|| InputName == MiniGrassInputName
						|| InputName == LegacyMiniGrassInputName
						|| SeenInputNames.Contains(InputName))
					{
						GrassOutput->GrassTypes.RemoveAt(InputIndex);
						continue;
					}
					SeenInputNames.Add(InputName);
				}
			}
		}
		else
		{
			TargetGrassOutput = Cast<UMaterialExpressionLandscapeGrassOutput>(
				UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionLandscapeGrassOutput::StaticClass(), 1970, -270));
		}

		UMaterialExpressionLandscapeLayerSample* GrassLayerSample = Cast<UMaterialExpressionLandscapeLayerSample>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionLandscapeLayerSample::StaticClass(), 1380, 20));
		UMaterialExpressionLandscapeGrassOutput* GrassOutput = TargetGrassOutput;
		UMaterialExpressionConstant* ZeroSpecular = Cast<UMaterialExpressionConstant>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant::StaticClass(), 1660, -30));
		UMaterialExpressionMultiply* SpecularLayerKeepAlive = Cast<UMaterialExpressionMultiply>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionMultiply::StaticClass(), 1820, -30));

		if (!GrassLayerSample || !GrassOutput || !ZeroSpecular || !SpecularLayerKeepAlive)
		{
			return false;
		}

		GrassLayerSample->ParameterName = GrassLayerName;
		GrassLayerSample->PreviewWeight = 1.0f;

		FGrassInput MiniInput(MiniGrassInputName);
		MiniInput.GrassType = GrassType;
		// Keep the proof constrained by painted layer data. The full-editor
		// GrassWeight pass was stripping the previous custom WorldPosition/normal
		// mask as all-zero, while the layer sample itself is already limited to
		// the mini patch written by this commandlet.
		MiniInput.Input.Expression = GrassLayerSample;
		GrassOutput->Desc = TEXT("FF Highland Native Mini Proof LandscapeGrassOutput");
		GrassOutput->GrassTypes.Add(MiniInput);

		// Titan grass samples RVT_Titan_D output 1 as a downward WPO term. The
		// real Titan landscape writes a low specular-like D channel, so keep the
		// Highland material's standard RVT pass from feeding a default 0.5. A
		// tiny non-zero contribution also keeps the layer referenced by the main
		// material path; a literal zero is optimized out and the editor layer
		// resolve strips the proof weightmap before GrassMap generation can read it.
		ZeroSpecular->R = 0.001f;
		SpecularLayerKeepAlive->A.Expression = GrassLayerSample;
		SpecularLayerKeepAlive->B.Expression = ZeroSpecular;
		UMaterialEditingLibrary::ConnectMaterialProperty(SpecularLayerKeepAlive, TEXT(""), MP_Specular);

		Material->bUsedWithInstancedStaticMeshes = true;
		UMaterialEditingLibrary::LayoutMaterialExpressions(Material);
		UMaterialEditingLibrary::RecompileMaterial(Material);
		Material->PostEditChange();
		Material->MarkPackageDirty();
		return true;
	}

	ARuntimeVirtualTextureVolume* SpawnRVTVolume(UWorld* World, ALandscapeProxy* Landscape, URuntimeVirtualTexture* RVT, const FString& Label, const FVector& BoundsMin, const FVector& BoundsSize)
	{
		if (!World || !RVT)
		{
			return nullptr;
		}

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ARuntimeVirtualTextureVolume* Volume = World->SpawnActor<ARuntimeVirtualTextureVolume>(ARuntimeVirtualTextureVolume::StaticClass(), BoundsMin, FRotator::ZeroRotator, SpawnParameters);
		if (!Volume)
		{
			return nullptr;
		}

		Volume->Modify();
		Volume->Tags.AddUnique(RVTSupportTag);
		Volume->SetActorLabel(Label);
		Volume->SetActorScale3D(BoundsSize);
		if (Volume->VirtualTextureComponent)
		{
			Volume->VirtualTextureComponent->Modify();
			Volume->VirtualTextureComponent->SetVirtualTexture(RVT);
			Volume->VirtualTextureComponent->SetBoundsAlignActor(Landscape);
			Volume->VirtualTextureComponent->MarkPackageDirty();
		}
		Volume->MarkPackageDirty();
		return Volume;
	}

	ARuntimeVirtualTextureVolume* SpawnMiniRVTVolume(UWorld* World, URuntimeVirtualTexture* RVT, const FString& Label, const FBox& Bounds)
	{
		if (!World || !RVT || !Bounds.IsValid)
		{
			return nullptr;
		}

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ARuntimeVirtualTextureVolume* Volume = World->SpawnActor<ARuntimeVirtualTextureVolume>(
			ARuntimeVirtualTextureVolume::StaticClass(),
			Bounds.Min,
			FRotator::ZeroRotator,
			SpawnParameters);
		if (!Volume)
		{
			return nullptr;
		}

		Volume->Modify();
		Volume->Tags.AddUnique(MiniRVTTag);
		Volume->SetActorLabel(Label);
		Volume->SetActorScale3D(Bounds.GetSize());
		if (Volume->VirtualTextureComponent)
		{
			Volume->VirtualTextureComponent->Modify();
			Volume->VirtualTextureComponent->SetVirtualTexture(RVT);
			Volume->VirtualTextureComponent->MarkPackageDirty();
		}
		Volume->MarkPackageDirty();
		return Volume;
	}

	bool TraceLandscapeGround(UWorld* World, const FVector2D& XY, float ReferenceZ, FHitResult& OutHit)
	{
		if (!World)
		{
			return false;
		}

		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(FFHighlandNativeMiniProofTrace), true);
		const FVector TraceStart(XY.X, XY.Y, ReferenceZ + 20000.0f);
		const FVector TraceEnd(XY.X, XY.Y, ReferenceZ - 30000.0f);
		if (!World->LineTraceSingleByChannel(OutHit, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
		{
			return false;
		}
		return OutHit.GetActor() && OutHit.GetActor()->IsA<ALandscapeProxy>();
	}

	FNativeMiniPaintResult PaintNativeMiniGrassLayer(ALandscapeProxy* Landscape, ULandscapeLayerInfoObject* LayerInfo, const FVector& Center, float Radius)
	{
		FNativeMiniPaintResult Result;
		if (!Landscape || !LayerInfo)
		{
			Result.DebugText = TEXT("- paint: missing Landscape or LayerInfo.\n");
			return Result;
		}

		ULandscapeInfo* LandscapeInfo = Landscape->GetLandscapeInfo();
		if (!LandscapeInfo)
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandGrassRVTTest: cannot paint mini grass layer; LandscapeInfo missing."));
			Result.DebugText = TEXT("- paint: LandscapeInfo missing.\n");
			return Result;
		}
		LandscapeInfo->UpdateLayerInfoMap(Landscape, true);

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
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandGrassRVTTest: cannot paint mini grass layer; no landscape component extents."));
			Result.DebugText = TEXT("- paint: no landscape component extents.\n");
			return Result;
		}

		const FTransform& LandscapeTransform = Landscape->GetActorTransform();
		const FVector LocalCenter = LandscapeTransform.InverseTransformPosition(Center);
		const FVector Scale = LandscapeTransform.GetScale3D().GetAbs();
		const float RadiusX = Radius / FMath::Max(Scale.X, 1.0f);
		const float RadiusY = Radius / FMath::Max(Scale.Y, 1.0f);
		const int32 X1 = FMath::Clamp(FMath::FloorToInt(LocalCenter.X - RadiusX), MinX, MaxX);
		const int32 Y1 = FMath::Clamp(FMath::FloorToInt(LocalCenter.Y - RadiusY), MinY, MaxY);
		const int32 X2 = FMath::Clamp(FMath::CeilToInt(LocalCenter.X + RadiusX), MinX, MaxX);
		const int32 Y2 = FMath::Clamp(FMath::CeilToInt(LocalCenter.Y + RadiusY), MinY, MaxY);
		if (X2 <= X1 || Y2 <= Y1)
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandGrassRVTTest: mini grass layer paint bounds invalid X=%d..%d Y=%d..%d."), X1, X2, Y1, Y2);
			Result.DebugText = FString::Printf(TEXT("- paint: invalid bounds X=%d..%d Y=%d..%d.\n"), X1, X2, Y1, Y2);
			return Result;
		}

		const int32 ComponentSizeQuads = LandscapeInfo->ComponentSizeQuads;
		const int32 ComponentIndexX1 = (X1 - 1 >= 0) ? (X1 - 1) / ComponentSizeQuads : (X1) / ComponentSizeQuads - 1;
		const int32 ComponentIndexY1 = (Y1 - 1 >= 0) ? (Y1 - 1) / ComponentSizeQuads : (Y1) / ComponentSizeQuads - 1;
		const int32 ComponentIndexX2 = (X2 >= 0) ? X2 / ComponentSizeQuads : (X2 + 1) / ComponentSizeQuads - 1;
		const int32 ComponentIndexY2 = (Y2 >= 0) ? Y2 / ComponentSizeQuads : (Y2 + 1) / ComponentSizeQuads - 1;
		for (int32 ComponentIndexY = ComponentIndexY1; ComponentIndexY <= ComponentIndexY2; ++ComponentIndexY)
		{
			for (int32 ComponentIndexX = ComponentIndexX1; ComponentIndexX <= ComponentIndexX2; ++ComponentIndexX)
			{
				if (LandscapeInfo->XYtoComponentMap.FindRef(FIntPoint(ComponentIndexX, ComponentIndexY)) != nullptr)
				{
					++Result.ComponentMapHits;
				}
			}
		}

		const int32 Width = X2 - X1 + 1;
		const int32 Height = Y2 - Y1 + 1;
		TArray<uint8> LayerData;
		LayerData.Init(0, Width * Height);

		FGuid PaintEditLayerGuid;
		FName PaintEditLayerName = TEXT("BaseFinalWeightmap");
		if (ALandscape* MainLandscape = Cast<ALandscape>(Landscape))
		{
			for (const FLandscapeLayer& EditLayer : MainLandscape->GetLayersConst())
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
				MainLandscape->SetEditingLayer(PaintEditLayerGuid);
			}
		}
		Result.PaintEditLayerGuid = PaintEditLayerGuid;
		Result.PaintEditLayerName = PaintEditLayerName;

		// Highland uses landscape edit layers. Writing directly to the resolved
		// final/base weightmap can be erased by the full-editor layer resolve.
		// Write into the active edit-layer data when present, then let the full
		// editor workflow resolve that into component allocations/GrassMaps.
		FLandscapeEditDataInterface EditData(LandscapeInfo, PaintEditLayerGuid);
		EditData.GetWeightDataFast(LayerInfo, X1, Y1, X2, Y2, LayerData.GetData(), 0);

		int32 PaintedSamples = 0;
		for (int32 Y = Y1; Y <= Y2; ++Y)
		{
			for (int32 X = X1; X <= X2; ++X)
			{
				const float NormX = RadiusX > 0.0f ? (static_cast<float>(X) - LocalCenter.X) / RadiusX : 0.0f;
				const float NormY = RadiusY > 0.0f ? (static_cast<float>(Y) - LocalCenter.Y) / RadiusY : 0.0f;
				const float Dist = FMath::Sqrt((NormX * NormX) + (NormY * NormY));
				const float SoftWeight = 1.0f - FMath::SmoothStep(0.72f, 1.0f, Dist);
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

		if (PaintedSamples <= 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("FFStarterHighlandGrassRVTTest: mini grass layer produced no painted samples for %s."), *LayerInfo->GetPathName());
			Result.DebugText = FString::Printf(TEXT("- paint: generated zero non-zero samples. componentMapHits=%d bounds X=%d..%d Y=%d..%d editLayer=`%s` guid=`%s`.\n"),
				Result.ComponentMapHits,
				X1,
				X2,
				Y1,
				Y2,
				*PaintEditLayerName.ToString(),
				*PaintEditLayerGuid.ToString());
			return Result;
		}
		Result.PaintedSamples = PaintedSamples;

		Landscape->Modify();
		LandscapeInfo->Modify();
		EditData.SetAlphaData(
			LayerInfo,
			X1,
			Y1,
			X2,
			Y2,
			LayerData.GetData(),
			0,
			ELandscapeLayerPaintingRestriction::None,
			/*bWeightAdjust=*/true,
			/*bTotalWeightAdjust=*/true);
		EditData.Flush();

		TArray<uint8> VerifyLayerData;
		VerifyLayerData.Init(0, Width * Height);
		EditData.GetWeightDataFast(LayerInfo, X1, Y1, X2, Y2, VerifyLayerData.GetData(), 0);
		for (uint8 Value : VerifyLayerData)
		{
			if (Value > 0)
			{
				++Result.NonZeroVerifySamples;
			}
		}

		LandscapeInfo->UpdateLayerInfoMap(Landscape, true);

		auto ComponentIntersectsPaintBounds = [X1, Y1, X2, Y2](const ULandscapeComponent* Component)
		{
			if (!Component)
			{
				return false;
			}
			const FIntRect Extent = Component->GetComponentExtent();
			return Extent.Min.X <= X2 && Extent.Max.X >= X1 && Extent.Min.Y <= Y2 && Extent.Max.Y >= Y1;
		};

		for (ULandscapeComponent* Component : Landscape->LandscapeComponents)
		{
			if (!Component || !ComponentIntersectsPaintBounds(Component))
			{
				continue;
			}

			const TArray<FWeightmapLayerAllocationInfo>& BaseAllocations = Component->GetWeightmapLayerAllocations(false);
			if (BaseAllocations.ContainsByPredicate([LayerInfo](const FWeightmapLayerAllocationInfo& Allocation) { return Allocation.LayerInfo == LayerInfo; }))
			{
				++Result.ComponentsWithBaseGrassAllocation;
			}

			const TArray<FWeightmapLayerAllocationInfo>& EditAllocations = Component->GetWeightmapLayerAllocations(true);
			if (EditAllocations.ContainsByPredicate([LayerInfo](const FWeightmapLayerAllocationInfo& Allocation) { return Allocation.LayerInfo == LayerInfo; }))
			{
				++Result.ComponentsWithEditGrassAllocation;
			}

			const TArray<FWeightmapLayerAllocationInfo>& TargetAllocations = Component->GetWeightmapLayerAllocations(PaintEditLayerGuid);
			if (TargetAllocations.ContainsByPredicate([LayerInfo](const FWeightmapLayerAllocationInfo& Allocation) { return Allocation.LayerInfo == LayerInfo; }))
			{
				++Result.ComponentsWithTargetGrassAllocation;
			}
		}

		if (ALandscape* MainLandscape = Cast<ALandscape>(Landscape))
		{
			MainLandscape->RequestLayersContentUpdate(ELandscapeLayerUpdateMode::Update_Weightmap_All);
		}
		Landscape->UpdateAllComponentMaterialInstances(true);
		for (ULandscapeComponent* Component : Landscape->LandscapeComponents)
		{
			if (Component)
			{
				Component->Modify();
				Component->UpdateMaterialInstances();
				Component->UpdateGrassTypes(true);
				Component->MarkPackageDirty();
			}
		}
		Landscape->MarkPackageDirty();

		Result.DebugText = FString::Printf(TEXT("- paint bounds: X=%d..%d Y=%d..%d componentIndex=%d..%d/%d..%d componentMapHits=%d componentSizeQuads=%d\n"),
			X1,
			X2,
			Y1,
			Y2,
			ComponentIndexX1,
			ComponentIndexX2,
			ComponentIndexY1,
			ComponentIndexY2,
			Result.ComponentMapHits,
			ComponentSizeQuads);
		Result.DebugText += FString::Printf(TEXT("- paint edit layer: `%s` guid=`%s`\n"), *PaintEditLayerName.ToString(), *PaintEditLayerGuid.ToString());
		Result.DebugText += FString::Printf(TEXT("- paint allocation check after SetAlphaData: baseGrassComponents=%d editGrassComponents=%d targetGrassComponents=%d nonZeroVerifySamples=%d\n"),
			Result.ComponentsWithBaseGrassAllocation,
			Result.ComponentsWithEditGrassAllocation,
			Result.ComponentsWithTargetGrassAllocation,
			Result.NonZeroVerifySamples);

		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandGrassRVTTest: painted mini grass layer %s layerName=%s samples=%d bounds X=%d..%d Y=%d..%d componentHits=%d targetAllocs=%d verifySamples=%d editLayer=%s"),
			*LayerInfo->GetPathName(),
			*LayerInfo->GetLayerName().ToString(),
			PaintedSamples,
			X1,
			X2,
			Y1,
			Y2,
			Result.ComponentMapHits,
			Result.ComponentsWithTargetGrassAllocation,
			Result.NonZeroVerifySamples,
			*PaintEditLayerName.ToString());
		return Result;
	}

	FVector ResolveMiniProofCenter(UWorld* World)
	{
		FVector BaseLocation(38550.0f, 147850.0f, -3200.0f);
		for (TActorIterator<APlayerStart> It(World); It; ++It)
		{
			BaseLocation = It->GetActorLocation();
			break;
		}

		const FVector2D Offsets[] = {
			FVector2D(3200.0f, 2200.0f),
			FVector2D(4600.0f, 0.0f),
			FVector2D(-3200.0f, 2400.0f),
			FVector2D(0.0f, 4600.0f),
			FVector2D(2200.0f, -2800.0f)
		};
		for (const FVector2D& Offset : Offsets)
		{
			FHitResult Hit;
			const FVector2D XY(BaseLocation.X + Offset.X, BaseLocation.Y + Offset.Y);
			if (TraceLandscapeGround(World, XY, BaseLocation.Z, Hit) && Hit.ImpactNormal.Z >= 0.88f)
			{
				return Hit.ImpactPoint;
			}
		}

		FHitResult BaseHit;
		if (TraceLandscapeGround(World, FVector2D(BaseLocation.X, BaseLocation.Y), BaseLocation.Z, BaseHit))
		{
			return BaseHit.ImpactPoint;
		}
		return BaseLocation;
	}

	void SpawnMiniProofCamera(UWorld* World, const FString& Label, const FName& Tag, const FVector& Location, const FVector& Target, float FOV)
	{
		if (!World)
		{
			return;
		}

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ACameraActor* Camera = World->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), Location, (Target - Location).Rotation(), SpawnParameters);
		if (!Camera)
		{
			return;
		}

		Camera->Modify();
		Camera->Tags.AddUnique(MiniProofTag);
		Camera->Tags.AddUnique(Tag);
		Camera->SetActorLabel(Label);
		if (Camera->GetCameraComponent())
		{
			Camera->GetCameraComponent()->FieldOfView = FOV;
		}
		Camera->MarkPackageDirty();
	}

	int32 RunNativeMiniRollout()
	{
		FString Report = TEXT("# Highland Native Titan Mini Grass Proof\n\n");
		Report += TEXT("Mode: isolated Highland mini-rollout proof. No Water_Lake_1, water actors, PlayerStart, or terrain height edits.\n\n");

		UWorld* World = UEditorLoadingAndSavingUtils::LoadMap(HighlandMapPath);
		if (!World)
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandGrassRVTTest: failed to load Highland map."));
			return 1;
		}

		ALandscapeProxy* Landscape = FindPrimaryLandscape(World);
		if (!Landscape)
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandGrassRVTTest: no landscape found."));
			return 1;
		}

		RemoveMiniProofActors(World);

		UMaterial* LandscapeMaterial = LoadObject<UMaterial>(nullptr, HighlandLandscapeMaterialPath);
		ULandscapeGrassType* TitanGrassType = LoadObject<ULandscapeGrassType>(nullptr, TitanGrassTypePath);
		ULandscapeLayerInfoObject* GrassLayerInfo = LoadObject<ULandscapeLayerInfoObject>(nullptr, GrasslandGrassLayerInfoPath);
		URuntimeVirtualTexture* RVTD = LoadObject<URuntimeVirtualTexture>(nullptr, RVTTitanDPath);
		URuntimeVirtualTexture* RVTH = LoadObject<URuntimeVirtualTexture>(nullptr, RVTTitanHPath);
		URuntimeVirtualTexture* RVTDH = LoadObject<URuntimeVirtualTexture>(nullptr, RVTTitanDHPath);
		if (!LandscapeMaterial || !TitanGrassType || !GrassLayerInfo || !RVTD || !RVTH || !RVTDH)
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandGrassRVTTest: missing mini proof dependency LandscapeMat=%s LGT=%s LayerInfo=%s D=%s H=%s DH=%s"),
				LandscapeMaterial ? TEXT("ok") : HighlandLandscapeMaterialPath,
				TitanGrassType ? TEXT("ok") : TitanGrassTypePath,
				GrassLayerInfo ? TEXT("ok") : GrasslandGrassLayerInfoPath,
				RVTD ? TEXT("ok") : RVTTitanDPath,
				RVTH ? TEXT("ok") : RVTTitanHPath,
				RVTDH ? TEXT("ok") : RVTTitanDHPath);
			return 1;
		}

		const FVector ProofCenter = ResolveMiniProofCenter(World);
		constexpr float ProofRadius = 2600.0f;
		int32 ClusterComponentCount = 0;
		const FBox ProofBounds = BuildHighlandNativeClusterRVTBounds(Landscape, ProofCenter, ProofRadius, ClusterComponentCount);
		const FVector ClusterCenter = ProofBounds.IsValid ? ProofBounds.GetCenter() : ProofCenter;

		const bool bGrassOutputUpdated = AddOrRefreshNativeMiniGrassOutput(LandscapeMaterial, TitanGrassType, GrassLayerInfo->GetLayerName(), FVector2D(ProofCenter.X, ProofCenter.Y), ProofRadius);
		const FNativeMiniPaintResult PaintResult = PaintNativeMiniGrassLayer(Landscape, GrassLayerInfo, ProofCenter, ProofRadius);
		Landscape->Modify();
		Landscape->RuntimeVirtualTextures.AddUnique(RVTD);
		Landscape->RuntimeVirtualTextures.AddUnique(RVTH);
		Landscape->RuntimeVirtualTextures.AddUnique(RVTDH);
		Landscape->VirtualTextureRenderPassType = ERuntimeVirtualTextureMainPassType::Always;
		Landscape->SetDisableRuntimeGrassMapGenerationProxyOnly(false);
		Landscape->MarkPackageDirty();
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

		ARuntimeVirtualTextureVolume* VolumeD = SpawnMiniRVTVolume(World, RVTD, TEXT("FF_HighlandNativeMini_RVT_Titan_D"), ProofBounds);
		ARuntimeVirtualTextureVolume* VolumeH = SpawnMiniRVTVolume(World, RVTH, TEXT("FF_HighlandNativeMini_RVT_Titan_H"), ProofBounds);
		ARuntimeVirtualTextureVolume* VolumeDH = SpawnMiniRVTVolume(World, RVTDH, TEXT("FF_HighlandNativeMini_RVT_Titan_DH"), ProofBounds);

		FVector PlayerStartLocation = ProofCenter - FVector(3200.0f, 2200.0f, -160.0f);
		for (TActorIterator<APlayerStart> It(World); It; ++It)
		{
			PlayerStartLocation = It->GetActorLocation();
			break;
		}
		SpawnMiniProofCamera(World, TEXT("FFSmoke_HighlandNativeMini_Proxy"), TEXT("FFSmokeHighlandNativeMiniProxyCamera"),
			PlayerStartLocation + FVector(-360.0f, -520.0f, 220.0f),
			PlayerStartLocation + FVector(280.0f, 220.0f, 42.0f),
			38.0f);
		const FVector ClusterCameraCenter(ClusterCenter.X, ClusterCenter.Y, ProofCenter.Z);
		SpawnMiniProofCamera(World, TEXT("FFSmoke_HighlandNativeMini_Sun"), TEXT("FFSmokeHighlandNativeMiniSunCamera"),
			ProofCenter + FVector(-620.0f, -780.0f, 260.0f),
			ProofCenter + FVector(120.0f, 80.0f, 50.0f),
			34.0f);
		SpawnMiniProofCamera(World, TEXT("FFSmoke_HighlandNativeMini_Shadow"), TEXT("FFSmokeHighlandNativeMiniShadowCamera"),
			ProofCenter + FVector(780.0f, 620.0f, 245.0f),
			ProofCenter + FVector(-120.0f, -80.0f, 48.0f),
			34.0f);
		SpawnMiniProofCamera(World, TEXT("FFSmoke_HighlandNativeMini_Transition"), TEXT("FFSmokeHighlandNativeMiniTransitionCamera"),
			ProofCenter + FVector(2100.0f, -1550.0f, 520.0f),
			ProofCenter + FVector(850.0f, 80.0f, 60.0f),
			50.0f);
		SpawnMiniProofCamera(World, TEXT("FFSmoke_HighlandNativeMini_ObliqueOverview"), TEXT("FFSmokeHighlandNativeMiniObliqueOverviewCamera"),
			ProofCenter + FVector(-2600.0f, -2850.0f, 980.0f),
			ProofCenter + FVector(900.0f, 180.0f, 60.0f),
			55.0f);
		SpawnMiniProofCamera(World, TEXT("FFSmoke_HighlandNativeMini_TraversalPath"), TEXT("FFSmokeHighlandNativeMiniTraversalPathCamera"),
			ProofCenter + FVector(-1200.0f, -1400.0f, 190.0f),
			ProofCenter + FVector(1250.0f, 260.0f, 75.0f),
			42.0f);
		SpawnMiniProofCamera(World, TEXT("FFSmoke_HighlandNativeMini_TopDown"), TEXT("FFSmokeHighlandNativeMiniTopDownCamera"),
			ProofCenter + FVector(0.0f, -350.0f, 2100.0f),
			ProofCenter + FVector(0.0f, 120.0f, 0.0f),
			60.0f);

		World->MarkPackageDirty();
		const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
		const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);

		Report += FString::Printf(TEXT("- Proof center: `%s`\n"), *ProofCenter.ToString());
		Report += FString::Printf(TEXT("- Proof radius: %.1f\n"), ProofRadius);
		Report += FString::Printf(TEXT("- Cluster components: %s found=%d\n"), *HighlandNativeClusterComponentSummary(), ClusterComponentCount);
		Report += FString::Printf(TEXT("- Landscape material: `%s`\n"), *LandscapeMaterial->GetPathName());
		Report += FString::Printf(TEXT("- GrassType: `%s`\n"), *TitanGrassType->GetPathName());
		Report += FString::Printf(TEXT("- Grass LayerInfo: `%s` layerName=`%s` paintedLayerSamples=%d\n"),
			*GrassLayerInfo->GetPathName(),
			*GrassLayerInfo->GetLayerName().ToString(),
			PaintResult.PaintedSamples);
		Report += FString::Printf(TEXT("- Grass paint edit layer: `%s` guid=`%s`\n"),
			*PaintResult.PaintEditLayerName.ToString(),
			*PaintResult.PaintEditLayerGuid.ToString());
		Report += PaintResult.DebugText;
		Report += FString::Printf(TEXT("- RVT volumes: D=%s H=%s DH=%s boundsMin=`%s` boundsMax=`%s`\n"),
			VolumeD ? TEXT("ok") : TEXT("failed"),
			VolumeH ? TEXT("ok") : TEXT("failed"),
			VolumeDH ? TEXT("ok") : TEXT("failed"),
			*ProofBounds.Min.ToString(),
			*ProofBounds.Max.ToString());
		Report += FString::Printf(TEXT("- grassOutputUpdated=%d savedMap=%d savedPackages=%d\n\n"),
			bGrassOutputUpdated ? 1 : 0,
			bSavedMap ? 1 : 0,
			bSavedPackages ? 1 : 0);
		Report += (PaintResult.PaintedSamples > 0 && PaintResult.ComponentMapHits > 0 && PaintResult.ComponentsWithTargetGrassAllocation > 0 && PaintResult.NonZeroVerifySamples > 0 && bGrassOutputUpdated && VolumeD && VolumeH && VolumeDH && bSavedMap && bSavedPackages)
			? TEXT("Result: PASS - Highland mini native Titan grass proof authored; requires build/package/smoke validation.\n")
			: TEXT("Result: FAIL - Highland mini native Titan grass proof could not be authored safely.\n");
		FFileHelper::SaveStringToFile(Report, *FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("HighlandNativeTitanMiniProof_Report.txt")));

		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandGrassRVTTest: NativeMiniRollout center=%s radius=%.1f clusterComponents=%d grassOutput=%s volumes=%s/%s/%s savedMap=%s savedPackages=%s"),
			*ProofCenter.ToString(),
			ProofRadius,
			ClusterComponentCount,
			bGrassOutputUpdated ? TEXT("true") : TEXT("false"),
			VolumeD ? TEXT("D") : TEXT("-"),
			VolumeH ? TEXT("H") : TEXT("-"),
			VolumeDH ? TEXT("DH") : TEXT("-"),
			bSavedMap ? TEXT("true") : TEXT("false"),
			bSavedPackages ? TEXT("true") : TEXT("false"));

		return (PaintResult.PaintedSamples > 0 && PaintResult.ComponentMapHits > 0 && PaintResult.ComponentsWithTargetGrassAllocation > 0 && PaintResult.NonZeroVerifySamples > 0 && bGrassOutputUpdated && VolumeD && VolumeH && VolumeDH && bSavedMap && bSavedPackages) ? 0 : 1;
	}

	int32 CreateNativeGrassPatch(UWorld* World, UStaticMesh* GrassMesh, UMaterialInterface* GrassMaterial, const FVector& PatchCenter)
	{
		if (!World || !GrassMesh || !GrassMaterial)
		{
			return 0;
		}

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AActor* GrassActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
		if (!GrassActor)
		{
			return 0;
		}

		GrassActor->Modify();
		GrassActor->Tags.AddUnique(TestPatchTag);
		GrassActor->SetActorLabel(TEXT("FF_TitanGrass_RVT_Native_TestPatch"));

		USceneComponent* RootComponent = NewObject<USceneComponent>(GrassActor, TEXT("Root"), RF_Transactional);
		RootComponent->SetMobility(EComponentMobility::Static);
		GrassActor->SetRootComponent(RootComponent);
		RootComponent->RegisterComponent();
		GrassActor->AddInstanceComponent(RootComponent);

		UHierarchicalInstancedStaticMeshComponent* Component = NewObject<UHierarchicalInstancedStaticMeshComponent>(GrassActor, TEXT("NativeTitanGrassBlade_HISM"), RF_Transactional);
		Component->SetStaticMesh(GrassMesh);
		for (int32 SlotIndex = 0; SlotIndex < FMath::Max(1, GrassMesh->GetStaticMaterials().Num()); ++SlotIndex)
		{
			Component->SetMaterial(SlotIndex, GrassMaterial);
		}
		Component->SetMobility(EComponentMobility::Static);
		Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Component->SetGenerateOverlapEvents(false);
		Component->SetCastShadow(false);
		Component->SetCullDistances(0, 32000);
		Component->SetupAttachment(RootComponent);
		Component->RegisterComponent();
		GrassActor->AddInstanceComponent(Component);

		int32 AddedInstances = 0;
		const FVector2D Center2D(PatchCenter.X + 900.0f, PatchCenter.Y + 1400.0f);
		for (int32 SampleIndex = 0; SampleIndex < 360 * 8 && AddedInstances < 360; ++SampleIndex)
		{
			const float Angle = PseudoRandom01(SampleIndex + 11) * 2.0f * PI;
			const float Radius = FMath::Sqrt(PseudoRandom01(SampleIndex + 31)) * 1450.0f;
			const FVector2D Position = Center2D + FVector2D(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius);

			FHitResult Hit;
			FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(FFHighlandTitanGrassRVTTest), true);
			const FVector TraceStart(Position.X, Position.Y, PatchCenter.Z + 12000.0f);
			const FVector TraceEnd(Position.X, Position.Y, PatchCenter.Z - 12000.0f);
			if (!World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
			{
				continue;
			}
			if (!Hit.GetActor() || !Hit.GetActor()->IsA<ALandscapeProxy>() || Hit.ImpactNormal.Z < 0.92f)
			{
				continue;
			}

			const float XYScale = FMath::Lerp(0.15f, 0.28f, PseudoRandom01(SampleIndex + 47));
			const float ZScale = FMath::Lerp(0.95f, 1.55f, PseudoRandom01(SampleIndex + 59));
			FTransform Transform;
			Transform.SetLocation(Hit.ImpactPoint + FVector(0.0f, 0.0f, 2.0f));
			Transform.SetRotation(FRotator(0.0f, PseudoRandom01(SampleIndex + 71) * 360.0f, 0.0f).Quaternion());
			Transform.SetScale3D(FVector(XYScale, XYScale, ZScale));
			Component->AddInstance(Transform, true);
			++AddedInstances;
		}

		Component->BuildTreeIfOutdated(true, true);
		Component->MarkPackageDirty();
		GrassActor->MarkPackageDirty();
		return AddedInstances;
	}
}
#endif

int32 UFFStarterHighlandGrassRVTTestCommandlet::Main(const FString& Params)
{
#if WITH_EDITOR
	if (Params.Contains(TEXT("NativeMiniRollout"), ESearchCase::IgnoreCase))
	{
		return RunNativeMiniRollout();
	}

	const bool bUseFallbackPatchMaterial = Params.Contains(TEXT("FallbackPatch"), ESearchCase::IgnoreCase);
	const bool bRollbackNativeTest = Params.Contains(TEXT("RollbackNativeTest"), ESearchCase::IgnoreCase);
	UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandGrassRVTTest: loading %s fallbackPatch=%s"),
		HighlandMapPath,
		bUseFallbackPatchMaterial ? TEXT("true") : TEXT("false"));

	UWorld* World = UEditorLoadingAndSavingUtils::LoadMap(HighlandMapPath);
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandGrassRVTTest: failed to load Highland map."));
		return 1;
	}

	ALandscapeProxy* Landscape = FindPrimaryLandscape(World);
	if (!Landscape)
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandGrassRVTTest: no landscape found."));
		return 1;
	}

	RemoveTaggedActors(World);

	URuntimeVirtualTexture* RVTTitanD = LoadObject<URuntimeVirtualTexture>(nullptr, RVTTitanDPath);
	URuntimeVirtualTexture* RVTTitanH = LoadObject<URuntimeVirtualTexture>(nullptr, RVTTitanHPath);
	UMaterial* LandscapeMaterial = LoadObject<UMaterial>(nullptr, HighlandLandscapeMaterialPath);
	UStaticMesh* GrassMesh = LoadObject<UStaticMesh>(nullptr, GrassMeshPath);
	UMaterialInterface* NativeGrassMaterial = LoadObject<UMaterialInterface>(nullptr, NativeGrassMaterialPath);
	UMaterialInterface* FallbackGrassMaterial = LoadObject<UMaterialInterface>(nullptr, FallbackGrassMaterialPath);
	if (!RVTTitanD || !RVTTitanH || !LandscapeMaterial || !GrassMesh || !NativeGrassMaterial || !FallbackGrassMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandGrassRVTTest: missing dependency D=%s H=%s LandscapeMat=%s Mesh=%s NativeMat=%s FallbackMat=%s"),
			RVTTitanD ? TEXT("ok") : RVTTitanDPath,
			RVTTitanH ? TEXT("ok") : RVTTitanHPath,
			LandscapeMaterial ? TEXT("ok") : HighlandLandscapeMaterialPath,
			GrassMesh ? TEXT("ok") : GrassMeshPath,
			NativeGrassMaterial ? TEXT("ok") : NativeGrassMaterialPath,
			FallbackGrassMaterial ? TEXT("ok") : FallbackGrassMaterialPath);
		return 1;
	}

	if (bRollbackNativeTest)
	{
		RemoveRVTOutput(LandscapeMaterial);
		Landscape->Modify();
		Landscape->RuntimeVirtualTextures.Reset();
		Landscape->VirtualTextureRenderPassType = ERuntimeVirtualTextureMainPassType::Always;
		Landscape->MarkPackageDirty();
		World->MarkPackageDirty();
		const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
		const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandGrassRVTTest: rollback savedMap=%s savedPackages=%s"),
			bSavedMap ? TEXT("true") : TEXT("false"),
			bSavedPackages ? TEXT("true") : TEXT("false"));
		return (bSavedMap && bSavedPackages) ? 0 : 1;
	}

	// Let Unreal's standard material-to-RVT pass write the Highland base color
	// and world height. The explicit RVT output node tripped UE 5.7's LWC
	// WorldHeight shader path in packaged builds.
	RemoveRVTOutput(LandscapeMaterial);

	Landscape->Modify();
	Landscape->RuntimeVirtualTextures.Reset();
	Landscape->RuntimeVirtualTextures.Add(RVTTitanD);
	Landscape->RuntimeVirtualTextures.Add(RVTTitanH);
	Landscape->VirtualTextureRenderPassType = ERuntimeVirtualTextureMainPassType::Always;
	Landscape->MarkPackageDirty();

	const FBox LandscapeBounds = Landscape->GetComponentsBoundingBox(true);
	const FVector BoundsMin = LandscapeBounds.Min - FVector(0.0f, 0.0f, 5000.0f);
	const FVector BoundsSize = LandscapeBounds.GetSize() + FVector(0.0f, 0.0f, 10000.0f);
	ARuntimeVirtualTextureVolume* VolumeD = SpawnRVTVolume(World, Landscape, RVTTitanD, TEXT("FF_Highland_RVT_Titan_D"), BoundsMin, BoundsSize);
	ARuntimeVirtualTextureVolume* VolumeH = SpawnRVTVolume(World, Landscape, RVTTitanH, TEXT("FF_Highland_RVT_Titan_H"), BoundsMin, BoundsSize);
	if (!VolumeD || !VolumeH)
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandGrassRVTTest: failed to spawn RVT support volumes."));
		return 1;
	}

	FVector PatchCenter(-18120.0f, 4000.0f, 0.0f);
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		PatchCenter = It->GetActorLocation();
		break;
	}

	UMaterialInterface* PatchMaterial = bUseFallbackPatchMaterial ? FallbackGrassMaterial : NativeGrassMaterial;
	if (UMaterial* BaseMaterial = PatchMaterial->GetMaterial())
	{
		BaseMaterial->Modify();
		BaseMaterial->TwoSided = true;
		BaseMaterial->bUsedWithInstancedStaticMeshes = true;
		BaseMaterial->bUsedWithNanite = true;
		BaseMaterial->MarkPackageDirty();
	}

	const int32 TestInstances = CreateNativeGrassPatch(World, GrassMesh, PatchMaterial, PatchCenter);
	World->MarkPackageDirty();
	const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
	const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);

	UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandGrassRVTTest: rvtD=%s rvtH=%s patchMaterial=%s instances=%d savedMap=%s savedPackages=%s"),
		VolumeD ? *VolumeD->GetName() : TEXT("None"),
		VolumeH ? *VolumeH->GetName() : TEXT("None"),
		*PatchMaterial->GetPathName(),
		TestInstances,
		bSavedMap ? TEXT("true") : TEXT("false"),
		bSavedPackages ? TEXT("true") : TEXT("false"));

	return (bSavedMap && bSavedPackages && TestInstances > 0) ? 0 : 1;
#else
	UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandGrassRVTTest can only run in editor builds."));
	return 1;
#endif
}
