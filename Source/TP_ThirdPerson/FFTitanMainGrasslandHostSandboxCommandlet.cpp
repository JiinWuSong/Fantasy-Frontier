#include "FFTitanMainGrasslandHostSandboxCommandlet.h"

#if WITH_EDITOR
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/LightComponent.h"
#include "Components/RuntimeVirtualTextureComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "FileHelpers.h"
#include "GameFramework/PlayerStart.h"
#include "Landscape.h"
#include "LandscapeGrassType.h"
#include "LandscapeLayerInfoObject.h"
#include "LandscapeProxy.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "UObject/Package.h"
#include "VT/RuntimeVirtualTexture.h"
#include "VT/RuntimeVirtualTextureVolume.h"
#endif

UFFTitanMainGrasslandHostSandboxCommandlet::UFFTitanMainGrasslandHostSandboxCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

#if WITH_EDITOR
namespace
{
	const TCHAR* SandboxMapPath = TEXT("/Game/FantasyFrontier/Test/TitanMainGrasslandHostSandbox");
	const FName SandboxTag(TEXT("FFTitanMainGrasslandHostSandbox"));
	const TCHAR* ReportPath = TEXT("TitanMainGrasslandHostSandbox_Report.txt");

	const TCHAR* TitanGroundMaterialPath = TEXT("/Game/Landscape/Materials/MI_LandscapeMain.MI_LandscapeMain");
	const TCHAR* TitanGrassTypePath = TEXT("/Game/Landscape/LGT/LGT_Grass.LGT_Grass");
	const TCHAR* NativeGrassMaterialPath = TEXT("/Game/Environment/Foliage/Materials/MI_GrassBlade.MI_GrassBlade");
	const TCHAR* NativeGrassDarkerMaterialPath = TEXT("/Game/Environment/Foliage/Materials/MI_GrassBladeDarker.MI_GrassBladeDarker");
	const TCHAR* TitanRVTDPath = TEXT("/Game/Landscape/RVT/RVT_Titan_D.RVT_Titan_D");
	const TCHAR* TitanRVTHPath = TEXT("/Game/Landscape/RVT/RVT_Titan_H.RVT_Titan_H");
	const TCHAR* TitanRVTDHPath = TEXT("/Game/Landscape/RVT/RVT_Titan_DH.RVT_Titan_DH");
	const TCHAR* FoliageUpdateMaterialPath = TEXT("/Game/Blueprint/FoliageInteraction/M_UpdateFoliageDraw.M_UpdateFoliageDraw");
	const TCHAR* EnginePlaneMeshPath = TEXT("/Engine/BasicShapes/Plane.Plane");

	const TCHAR* GrasslandLayerInfoPaths[] = {
		TEXT("/Game/Landscape/WeightLayers/LI_Grassland_Grass.LI_Grassland_Grass"),
		TEXT("/Game/Landscape/WeightLayers/LI_Grassland_Dirt.LI_Grassland_Dirt"),
		TEXT("/Game/Landscape/WeightLayers/LI_Grassland_Rock.LI_Grassland_Rock"),
		TEXT("/Game/Landscape/WeightLayers/LI_Grassland_Sand.LI_Grassland_Sand")
	};

	struct FHostSandboxStats
	{
		int32 Landscapes = 0;
		int32 RVTVolumes = 0;
		int32 NativeGrassActors = 0;
		int32 NativeGrassComponents = 0;
		int32 NativeGrassInstances = 0;
		int32 FoliageInteractionProbes = 0;
		FVector PlayerStartLocation = FVector::ZeroVector;
		TArray<FString> GrassVarietyLines;
		TArray<FString> Notes;
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

	FString ObjectPath(const UObject* Object)
	{
		return Object ? Object->GetPathName() : FString(TEXT("None"));
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

	void ClearTaggedActors(UWorld* World)
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

	bool TraceLandscape(UWorld* World, const FVector2D& XY, FHitResult& OutHit)
	{
		if (!World)
		{
			return false;
		}

		FCollisionQueryParams Params(SCENE_QUERY_STAT(FFTitanMainGrasslandHostTrace), true);
		const FVector Start(XY.X, XY.Y, 12000.0f);
		const FVector End(XY.X, XY.Y, -12000.0f);
		if (!World->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, Params))
		{
			return false;
		}

		return OutHit.GetActor() && OutHit.GetActor()->IsA<ALandscapeProxy>();
	}

	FName ResolveLayerName(ULandscapeLayerInfoObject* LayerInfo, const TCHAR* Fallback)
	{
		return LayerInfo && !LayerInfo->GetLayerName().IsNone() ? LayerInfo->GetLayerName() : FName(Fallback);
	}

	ALandscape* CreateHostContextLandscape(
		UWorld* World,
		UMaterialInterface* TitanGroundMaterial,
		const TArray<ULandscapeLayerInfoObject*>& LayerInfos,
		URuntimeVirtualTexture* RVTD,
		URuntimeVirtualTexture* RVTH,
		URuntimeVirtualTexture* RVTDH,
		FHostSandboxStats& Stats)
	{
		if (!World || !TitanGroundMaterial || LayerInfos.Num() < 4 || LayerInfos.Contains(nullptr) || !RVTD || !RVTH)
		{
			return nullptr;
		}

		constexpr int32 SectionsPerComponent = 1;
		constexpr int32 QuadsPerSection = 31;
		constexpr int32 ComponentCountX = 3;
		constexpr int32 ComponentCountY = 3;
		constexpr int32 QuadsPerComponent = SectionsPerComponent * QuadsPerSection;
		constexpr int32 SizeX = ComponentCountX * QuadsPerComponent + 1;
		constexpr int32 SizeY = ComponentCountY * QuadsPerComponent + 1;
		const FVector LandscapeScale(100.0f, 100.0f, 100.0f);
		const FVector LandscapeLocation(
			-0.5f * ComponentCountX * QuadsPerComponent * LandscapeScale.X,
			-0.5f * ComponentCountY * QuadsPerComponent * LandscapeScale.Y,
			0.0f);

		TArray<uint16> HeightData;
		HeightData.Init(32768, SizeX * SizeY);
		for (int32 Y = 0; Y < SizeY; ++Y)
		{
			for (int32 X = 0; X < SizeX; ++X)
			{
				const float U = static_cast<float>(X) / static_cast<float>(SizeX - 1);
				const float V = static_cast<float>(Y) / static_cast<float>(SizeY - 1);
				const float Ridge = FMath::Exp(-FMath::Square((V - (0.48f + 0.08f * FMath::Sin(U * UE_TWO_PI * 1.35f))) / 0.16f));
				const float RollA = FMath::Sin(U * UE_TWO_PI * 1.10f + V * 1.8f);
				const float RollB = FMath::Sin((U + V) * UE_TWO_PI * 0.70f + 0.7f);
				const float EdgeLift = FMath::Clamp((FVector2D(U - 0.5f, V - 0.5f).Size() - 0.36f) / 0.28f, 0.0f, 1.0f);
				const float HeightCm = RollA * 38.0f + RollB * 25.0f + Ridge * 42.0f + EdgeLift * 72.0f;
				HeightData[Y * SizeX + X] = static_cast<uint16>(FMath::Clamp(32768 + FMath::RoundToInt(HeightCm * 128.0f / LandscapeScale.Z), 0, 65535));
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
			LayerInfo.LayerData.Init(0, SizeX * SizeY);
			ImportLayers.Add(MoveTemp(LayerInfo));
		}

		for (int32 Y = 0; Y < SizeY; ++Y)
		{
			for (int32 X = 0; X < SizeX; ++X)
			{
				const float U = static_cast<float>(X) / static_cast<float>(SizeX - 1);
				const float V = static_cast<float>(Y) / static_cast<float>(SizeY - 1);
				const FVector2D Centered(U - 0.5f, V - 0.5f);
				const float Edge = FMath::Clamp((Centered.Size() - 0.36f) / 0.24f, 0.0f, 1.0f);
				const float Meander = 0.48f + 0.10f * FMath::Sin(U * UE_TWO_PI * 1.25f + 0.35f) + 0.035f * FMath::Sin(U * UE_TWO_PI * 3.2f);
				const float DirtPath = FMath::Exp(-FMath::Square((V - Meander) / 0.040f));
				const float SandPocket = FMath::Exp(-((FMath::Square(U - 0.68f) + FMath::Square(V - 0.25f)) / 0.018f));
				const float RockScatter = FMath::Max(0.0f, (0.5f + 0.5f * FMath::Sin((U * 5.1f + V * 7.7f) * UE_TWO_PI)) - 0.80f);
				const int32 RockWeight = FMath::RoundToInt(FMath::Clamp(Edge * 70.0f + RockScatter * 60.0f, 0.0f, 96.0f));
				const int32 DirtWeight = FMath::RoundToInt(FMath::Clamp(DirtPath * 82.0f, 0.0f, 104.0f));
				const int32 SandWeight = FMath::RoundToInt(FMath::Clamp(SandPocket * 70.0f, 0.0f, 78.0f));
				const int32 GrassWeight = FMath::Max(0, 255 - RockWeight - DirtWeight - SandWeight);
				const int32 DataIndex = Y * SizeX + X;
				ImportLayers[0].LayerData[DataIndex] = static_cast<uint8>(GrassWeight);
				ImportLayers[1].LayerData[DataIndex] = static_cast<uint8>(DirtWeight);
				ImportLayers[2].LayerData[DataIndex] = static_cast<uint8>(RockWeight);
				ImportLayers[3].LayerData[DataIndex] = static_cast<uint8>(SandWeight);
			}
		}

		TMap<FGuid, TArray<uint16>> HeightDataPerLayers;
		TMap<FGuid, TArray<FLandscapeImportLayerInfo>> MaterialLayerDataPerLayers;
		HeightDataPerLayers.Add(FGuid(), MoveTemp(HeightData));
		MaterialLayerDataPerLayers.Add(FGuid(), MoveTemp(ImportLayers));

		ALandscape* Landscape = World->SpawnActor<ALandscape>(LandscapeLocation, FRotator::ZeroRotator);
		if (!Landscape)
		{
			return nullptr;
		}

		Landscape->Modify();
		Landscape->SetActorLabel(TEXT("FF_TitanMainHostSandbox_Landscape_MI_LandscapeMain_RVTWriter"));
		Landscape->Tags.AddUnique(SandboxTag);
		Landscape->LandscapeMaterial = TitanGroundMaterial;
		Landscape->SetActorScale3D(LandscapeScale);
		Landscape->StaticLightingLOD = 0;
		Landscape->RuntimeVirtualTextures.AddUnique(RVTD);
		Landscape->RuntimeVirtualTextures.AddUnique(RVTH);
		if (RVTDH)
		{
			Landscape->RuntimeVirtualTextures.AddUnique(RVTDH);
		}
		Landscape->VirtualTextureRenderPassType = ERuntimeVirtualTextureMainPassType::Always;
		Landscape->Import(FGuid::NewGuid(), 0, 0, SizeX - 1, SizeY - 1, SectionsPerComponent, QuadsPerSection, HeightDataPerLayers, TEXT(""), MaterialLayerDataPerLayers, ELandscapeImportAlphamapType::Additive, TArrayView<const FLandscapeLayer>());
		Landscape->PostEditChange();
		Landscape->MarkPackageDirty();
		++Stats.Landscapes;
		return Landscape;
	}

	ARuntimeVirtualTextureVolume* SpawnRVTVolume(UWorld* World, const FString& Label, const FVector& Center, URuntimeVirtualTexture* RVT, FHostSandboxStats& Stats)
	{
		if (!World || !RVT)
		{
			return nullptr;
		}

		ARuntimeVirtualTextureVolume* Volume = SpawnTaggedActor<ARuntimeVirtualTextureVolume>(World, Label, Center + FVector(0.0f, 0.0f, 250.0f), FRotator::ZeroRotator);
		if (!Volume)
		{
			return nullptr;
		}

		Volume->SetActorScale3D(FVector(56.0f, 56.0f, 18.0f));
		if (Volume->VirtualTextureComponent)
		{
			Volume->VirtualTextureComponent->Modify();
			Volume->VirtualTextureComponent->SetVirtualTexture(RVT);
			Volume->VirtualTextureComponent->MarkPackageDirty();
		}
		Volume->MarkPackageDirty();
		++Stats.RVTVolumes;
		return Volume;
	}

	AActor* SpawnContainerActor(UWorld* World, const FString& Label, const FVector& Location)
	{
		AActor* Actor = World ? World->SpawnActor<AActor>(AActor::StaticClass(), Location, FRotator::ZeroRotator) : nullptr;
		if (!Actor)
		{
			return nullptr;
		}

		Actor->Modify();
		Actor->SetActorLabel(Label);
		Actor->Tags.AddUnique(SandboxTag);
		USceneComponent* Root = NewObject<USceneComponent>(Actor, TEXT("Root"), RF_Transactional);
		Root->SetMobility(EComponentMobility::Static);
		Actor->SetRootComponent(Root);
		Root->RegisterComponent();
		Actor->AddInstanceComponent(Root);
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

	float ResolveDensity(const FGrassVariety& Variety, int32 VarietyIndex)
	{
		const float Density = Variety.GrassDensity.Default;
		return Density > 0.0f ? Density : (VarietyIndex == 0 ? 25.0f : 200.0f);
	}

	UMaterialInterface* ResolveGrassMaterial(const FGrassVariety& Variety, int32 VarietyIndex, UMaterialInterface* NativeMaterial, UMaterialInterface* NativeDarkerMaterial)
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

	int32 AddGrassInstances(UWorld* World, UHierarchicalInstancedStaticMeshComponent* Component, const FGrassVariety& Variety, const FVector& Center, int32 VarietyIndex)
	{
		if (!World || !Component)
		{
			return 0;
		}

		const float Density = FMath::Clamp(ResolveDensity(Variety, VarietyIndex), 5.0f, 240.0f);
		const float Spacing = FMath::Clamp(FMath::Sqrt(100000.0f / Density) * 1.35f, 42.0f, 285.0f);
		constexpr float Radius = 2900.0f;
		const int32 GridRadius = FMath::CeilToInt(Radius / Spacing);
		int32 Added = 0;
		for (int32 GridY = -GridRadius; GridY <= GridRadius; ++GridY)
		{
			for (int32 GridX = -GridRadius; GridX <= GridRadius; ++GridX)
			{
				const int32 Seed = 12000000 + VarietyIndex * 100000 + GridX * 73471 + GridY * 912367;
				const FVector2D Offset(
					GridX * Spacing + FMath::Lerp(-Spacing * Variety.PlacementJitter * 0.5f, Spacing * Variety.PlacementJitter * 0.5f, PseudoRandom01(Seed + 13)),
					GridY * Spacing + FMath::Lerp(-Spacing * Variety.PlacementJitter * 0.5f, Spacing * Variety.PlacementJitter * 0.5f, PseudoRandom01(Seed + 29)));
				if (Offset.Size() > Radius)
				{
					continue;
				}

				FHitResult GroundHit;
				const FVector2D XY(Center.X + Offset.X, Center.Y + Offset.Y);
				if (!TraceLandscape(World, XY, GroundHit) || GroundHit.ImpactNormal.Z < 0.78f)
				{
					continue;
				}

				const float Yaw = Variety.RandomRotation ? PseudoRandom01(Seed + 61) * 360.0f : 0.0f;
				const FQuat SurfaceRotation = FRotationMatrix::MakeFromZ(GroundHit.ImpactNormal).ToQuat();
				const FQuat YawAroundSurface(GroundHit.ImpactNormal, FMath::DegreesToRadians(Yaw));
				const float XScale = FMath::Lerp(Variety.ScaleX.Min, Variety.ScaleX.Max, PseudoRandom01(Seed + 43));
				const float YScale = FMath::Lerp(Variety.ScaleY.Min > 0.0f ? Variety.ScaleY.Min : 1.0f, Variety.ScaleY.Max > 0.0f ? Variety.ScaleY.Max : 1.0f, PseudoRandom01(Seed + 47));
				const float ZScale = FMath::Lerp(Variety.ScaleZ.Min > 0.0f ? Variety.ScaleZ.Min : 0.5f, Variety.ScaleZ.Max > 0.0f ? Variety.ScaleZ.Max : 1.0f, PseudoRandom01(Seed + 59));

				FTransform InstanceTransform;
				InstanceTransform.SetLocation(GroundHit.ImpactPoint + FVector(0.0f, 0.0f, 1.5f));
				InstanceTransform.SetRotation(YawAroundSurface * SurfaceRotation);
				InstanceTransform.SetScale3D(FVector(XScale, YScale, ZScale));
				Component->AddInstance(InstanceTransform, true);
				++Added;
			}
		}

		Component->BuildTreeIfOutdated(true, true);
		Component->MarkPackageDirty();
		return Added;
	}

	void SpawnNativeGrass(UWorld* World, ULandscapeGrassType* GrassType, UMaterialInterface* NativeMaterial, UMaterialInterface* NativeDarkerMaterial, FHostSandboxStats& Stats)
	{
		if (!World || !GrassType)
		{
			return;
		}

		AActor* GrassActor = SpawnContainerActor(World, TEXT("FF_TitanMainHostSandbox_LGT_Grass_Reproduction"), FVector::ZeroVector);
		if (!GrassActor)
		{
			return;
		}

		++Stats.NativeGrassActors;
		for (int32 VarietyIndex = 0; VarietyIndex < GrassType->GrassVarieties.Num(); ++VarietyIndex)
		{
			const FGrassVariety& Variety = GrassType->GrassVarieties[VarietyIndex];
			if (!Variety.GrassMesh)
			{
				continue;
			}

			UMaterialInterface* Material = ResolveGrassMaterial(Variety, VarietyIndex, NativeMaterial, NativeDarkerMaterial);
			const int32 StartCull = FMath::Max(0, Variety.GetStartCullDistance());
			const int32 EndCull = FMath::Max(10000, Variety.GetEndCullDistance());
			UHierarchicalInstancedStaticMeshComponent* Component = CreateHISM(
				GrassActor,
				Variety.GrassMesh,
				Material,
				FString::Printf(TEXT("LGT_Grass_Variety_%d_%s"), VarietyIndex, *Variety.GrassMesh->GetName()),
				StartCull,
				EndCull);
			const int32 Added = AddGrassInstances(World, Component, Variety, FVector::ZeroVector, VarietyIndex);
			if (Added > 0)
			{
				++Stats.NativeGrassComponents;
				Stats.NativeGrassInstances += Added;
			}
			Stats.GrassVarietyLines.Add(FString::Printf(TEXT("- variety=%d mesh=%s material=%s density=%.2f instances=%d cull=%d-%d scaleX=%.2f..%.2f scaleY=%.2f..%.2f scaleZ=%.2f..%.2f"),
				VarietyIndex,
				*ObjectPath(Variety.GrassMesh),
				Material ? *ObjectPath(Material) : TEXT("mesh default"),
				ResolveDensity(Variety, VarietyIndex),
				Added,
				StartCull,
				EndCull,
				Variety.ScaleX.Min,
				Variety.ScaleX.Max,
				Variety.ScaleY.Min,
				Variety.ScaleY.Max,
				Variety.ScaleZ.Min,
				Variety.ScaleZ.Max));
		}
		GrassActor->MarkPackageDirty();
	}

	void SpawnFoliageInteractionProbe(UWorld* World, UMaterialInterface* UpdateMaterial, UStaticMesh* PlaneMesh, FHostSandboxStats& Stats)
	{
		if (!World || !UpdateMaterial || !PlaneMesh)
		{
			Stats.Notes.Add(TEXT("FoliageInteraction probe skipped because update material or probe plane was missing."));
			return;
		}

		AActor* ProbeActor = SpawnContainerActor(World, TEXT("FF_TitanMainHostSandbox_FoliageInteractionCookProbe"), FVector(0.0f, 0.0f, 170.0f));
		if (!ProbeActor)
		{
			return;
		}
		ProbeActor->SetActorHiddenInGame(true);
		UStaticMeshComponent* ProbeMesh = NewObject<UStaticMeshComponent>(ProbeActor, TEXT("M_UpdateFoliageDraw_Reference"), RF_Transactional);
		ProbeMesh->SetStaticMesh(PlaneMesh);
		ProbeMesh->SetMaterial(0, UpdateMaterial);
		ProbeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ProbeMesh->SetVisibility(false);
		ProbeMesh->SetupAttachment(ProbeActor->GetRootComponent());
		ProbeMesh->RegisterComponent();
		ProbeActor->AddInstanceComponent(ProbeMesh);
		ProbeActor->MarkPackageDirty();
		++Stats.FoliageInteractionProbes;
	}

	void SpawnLighting(UWorld* World)
	{
		ADirectionalLight* DirectionalLight = SpawnTaggedActor<ADirectionalLight>(World, TEXT("FF_TitanMainHostSandbox_DirectionalLight"), FVector(-2600.0f, -3600.0f, 7200.0f), FRotator(-38.0f, 42.0f, 0.0f));
		if (DirectionalLight && DirectionalLight->GetLightComponent())
		{
			DirectionalLight->GetLightComponent()->SetIntensity(4.2f);
			DirectionalLight->GetLightComponent()->SetCastShadows(true);
		}

		ASkyLight* SkyLight = SpawnTaggedActor<ASkyLight>(World, TEXT("FF_TitanMainHostSandbox_SkyLight"), FVector(0.0f, 0.0f, 6000.0f), FRotator::ZeroRotator);
		if (SkyLight && SkyLight->GetLightComponent())
		{
			SkyLight->GetLightComponent()->SetIntensity(0.8f);
		}

		AExponentialHeightFog* Fog = SpawnTaggedActor<AExponentialHeightFog>(World, TEXT("FF_TitanMainHostSandbox_HeightFog"), FVector::ZeroVector, FRotator::ZeroRotator);
		if (Fog && Fog->GetComponent())
		{
			Fog->GetComponent()->SetFogDensity(0.0012f);
			Fog->GetComponent()->SetStartDistance(1800.0f);
			Fog->GetComponent()->SetFogMaxOpacity(0.22f);
		}
	}

	FVector FindGroundLocation(UWorld* World, const FVector2D& XY, float HeightOffset)
	{
		FHitResult Hit;
		if (TraceLandscape(World, XY, Hit))
		{
			return Hit.ImpactPoint + FVector(0.0f, 0.0f, HeightOffset);
		}
		return FVector(XY.X, XY.Y, 200.0f + HeightOffset);
	}

	void SpawnPlayerStart(UWorld* World, FHostSandboxStats& Stats)
	{
		const FVector Location = FindGroundLocation(World, FVector2D(-1800.0f, -2200.0f), 125.0f);
		APlayerStart* PlayerStart = SpawnTaggedActor<APlayerStart>(World, TEXT("FF_TitanMainHostSandbox_PlayerStart"), Location, FRotator(0.0f, 35.0f, 0.0f));
		if (PlayerStart)
		{
			PlayerStart->PlayerStartTag = FName(TEXT("TitanMainGrasslandHostSandbox"));
		}
		Stats.PlayerStartLocation = Location;
	}

	void SpawnCamera(UWorld* World, const FString& Label, const FName& CameraTag, const FVector& Location, const FRotator& Rotation, float FOV)
	{
		ACameraActor* Camera = SpawnTaggedActor<ACameraActor>(World, Label, Location, Rotation);
		if (!Camera)
		{
			return;
		}
		Camera->Tags.AddUnique(CameraTag);
		if (UCameraComponent* CameraComponent = Camera->GetCameraComponent())
		{
			CameraComponent->FieldOfView = FOV;
		}
	}

	void SpawnSmokeCameras(UWorld* World)
	{
		SpawnCamera(World, TEXT("FF_TitanMainHostSandbox_ReferenceCamera"), FName(TEXT("FFSmokeTitanHostReferenceCamera")), FVector(-4300.0f, -5300.0f, 1300.0f), FRotator(-12.0f, 38.0f, 0.0f), 42.0f);
		SpawnCamera(World, TEXT("FF_TitanMainHostSandbox_SunCamera"), FName(TEXT("FFSmokeTitanHostSunCamera")), FVector(-1900.0f, -2900.0f, 460.0f), FRotator(-8.0f, 36.0f, 0.0f), 38.0f);
		SpawnCamera(World, TEXT("FF_TitanMainHostSandbox_ShadowCamera"), FName(TEXT("FFSmokeTitanHostShadowCamera")), FVector(2200.0f, 1950.0f, 560.0f), FRotator(-9.0f, -138.0f, 0.0f), 38.0f);
		SpawnCamera(World, TEXT("FF_TitanMainHostSandbox_GroundCloseCamera"), FName(TEXT("FFSmokeTitanHostGroundCloseCamera")), FVector(-620.0f, -760.0f, 190.0f), FRotator(-17.0f, 40.0f, 0.0f), 32.0f);
		SpawnCamera(World, TEXT("FF_TitanMainHostSandbox_GrassCloseCamera"), FName(TEXT("FFSmokeTitanHostGrassCloseCamera")), FVector(-380.0f, -470.0f, 125.0f), FRotator(-12.0f, 44.0f, 0.0f), 30.0f);
		SpawnCamera(World, TEXT("FF_TitanMainHostSandbox_TopDownCamera"), FName(TEXT("FFSmokeTopDownCamera")), FVector(0.0f, 0.0f, 16000.0f), FRotator(-90.0f, 0.0f, 0.0f), 62.0f);
	}

	void WriteReport(const FHostSandboxStats& Stats)
	{
		FString Report;
		Report += TEXT("# TitanMain Grassland Host Sandbox Proof\n\n");
		Report += FString::Printf(TEXT("Sandbox map: %s\n"), SandboxMapPath);
		Report += TEXT("Source host reference: /Game/Maps/TitanMain\n");
		Report += TEXT("Important: this map does not stream or cook TitanMain. It recreates a tiny host-context proof with Titan assets.\n\n");
		Report += TEXT("## Titan assets used\n");
		Report += FString::Printf(TEXT("- Landscape material: %s\n"), TitanGroundMaterialPath);
		Report += FString::Printf(TEXT("- GrassType: %s\n"), TitanGrassTypePath);
		Report += FString::Printf(TEXT("- Grass material: %s\n"), NativeGrassMaterialPath);
		Report += FString::Printf(TEXT("- Dark grass material: %s\n"), NativeGrassDarkerMaterialPath);
		Report += FString::Printf(TEXT("- RVT D: %s\n"), TitanRVTDPath);
		Report += FString::Printf(TEXT("- RVT H: %s\n"), TitanRVTHPath);
		Report += FString::Printf(TEXT("- RVT DH: %s\n"), TitanRVTDHPath);
		for (const TCHAR* LayerInfoPath : GrasslandLayerInfoPaths)
		{
			Report += FString::Printf(TEXT("- LayerInfo: %s\n"), LayerInfoPath);
		}
		Report += FString::Printf(TEXT("- FoliageInteraction probe material: %s\n"), FoliageUpdateMaterialPath);
		Report += TEXT("\n## Generated sandbox content\n");
		Report += FString::Printf(TEXT("- Landscapes: %d\n"), Stats.Landscapes);
		Report += FString::Printf(TEXT("- RVT volumes: %d\n"), Stats.RVTVolumes);
		Report += FString::Printf(TEXT("- Native grass actors: %d\n"), Stats.NativeGrassActors);
		Report += FString::Printf(TEXT("- Native grass components: %d\n"), Stats.NativeGrassComponents);
		Report += FString::Printf(TEXT("- Native grass instances: %d\n"), Stats.NativeGrassInstances);
		Report += FString::Printf(TEXT("- FoliageInteraction cook probes: %d\n"), Stats.FoliageInteractionProbes);
		Report += FString::Printf(TEXT("- PlayerStart: %s\n"), *Stats.PlayerStartLocation.ToString());
		Report += TEXT("\n## LGT_Grass varieties\n");
		for (const FString& Line : Stats.GrassVarietyLines)
		{
			Report += Line + TEXT("\n");
		}
		Report += TEXT("\n## Notes\n");
		Report += TEXT("- Weightmap distribution is representative and Titan-layer-compatible, not a raw copied TitanMain weightmap yet.\n");
		Report += TEXT("- This is intentionally a tiny sandbox proof; active Highland, Water_Lake_1, and TitanMain/Kashkeh maps are untouched.\n");
		for (const FString& Note : Stats.Notes)
		{
			Report += FString::Printf(TEXT("- %s\n"), *Note);
		}
		FFileHelper::SaveStringToFile(Report, *FPaths::Combine(FPaths::ProjectSavedDir(), ReportPath));
	}
}
#endif

int32 UFFTitanMainGrasslandHostSandboxCommandlet::Main(const FString& Params)
{
#if WITH_EDITOR
	UWorld* World = CreateOrLoadSandboxMap();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("TitanMainHostSandbox: failed to create/load sandbox map."));
		return 1;
	}

	ClearTaggedActors(World);

	UMaterialInterface* TitanGroundMaterial = LoadObject<UMaterialInterface>(nullptr, TitanGroundMaterialPath);
	ULandscapeGrassType* TitanGrassType = LoadObject<ULandscapeGrassType>(nullptr, TitanGrassTypePath);
	UMaterialInterface* NativeMaterial = LoadObject<UMaterialInterface>(nullptr, NativeGrassMaterialPath);
	UMaterialInterface* NativeDarkerMaterial = LoadObject<UMaterialInterface>(nullptr, NativeGrassDarkerMaterialPath);
	URuntimeVirtualTexture* RVTD = LoadObject<URuntimeVirtualTexture>(nullptr, TitanRVTDPath);
	URuntimeVirtualTexture* RVTH = LoadObject<URuntimeVirtualTexture>(nullptr, TitanRVTHPath);
	URuntimeVirtualTexture* RVTDH = LoadObject<URuntimeVirtualTexture>(nullptr, TitanRVTDHPath);
	UMaterialInterface* FoliageUpdateMaterial = LoadObject<UMaterialInterface>(nullptr, FoliageUpdateMaterialPath);
	UStaticMesh* ProbePlaneMesh = LoadObject<UStaticMesh>(nullptr, EnginePlaneMeshPath);

	TArray<ULandscapeLayerInfoObject*> LayerInfos;
	for (const TCHAR* LayerInfoPath : GrasslandLayerInfoPaths)
	{
		LayerInfos.Add(LoadObject<ULandscapeLayerInfoObject>(nullptr, LayerInfoPath));
	}

	if (!TitanGroundMaterial || !TitanGrassType || !NativeMaterial || !NativeDarkerMaterial || !RVTD || !RVTH || LayerInfos.Contains(nullptr))
	{
		UE_LOG(LogTemp, Error, TEXT("TitanMainHostSandbox: missing required asset. material=%s grassType=%s native=%s darker=%s rvtD=%s rvtH=%s layerCount=%d"),
			TitanGroundMaterial ? TEXT("ok") : TitanGroundMaterialPath,
			TitanGrassType ? TEXT("ok") : TitanGrassTypePath,
			NativeMaterial ? TEXT("ok") : NativeGrassMaterialPath,
			NativeDarkerMaterial ? TEXT("ok") : NativeGrassDarkerMaterialPath,
			RVTD ? TEXT("ok") : TitanRVTDPath,
			RVTH ? TEXT("ok") : TitanRVTHPath,
			LayerInfos.Num());
		return 1;
	}

	FHostSandboxStats Stats;
	ALandscape* Landscape = CreateHostContextLandscape(World, TitanGroundMaterial, LayerInfos, RVTD, RVTH, RVTDH, Stats);
	if (!Landscape)
	{
		UE_LOG(LogTemp, Error, TEXT("TitanMainHostSandbox: failed to create landscape."));
		return 1;
	}

	SpawnRVTVolume(World, TEXT("FF_TitanMainHostSandbox_RVT_Titan_D"), FVector::ZeroVector, RVTD, Stats);
	SpawnRVTVolume(World, TEXT("FF_TitanMainHostSandbox_RVT_Titan_H"), FVector::ZeroVector, RVTH, Stats);
	if (RVTDH)
	{
		SpawnRVTVolume(World, TEXT("FF_TitanMainHostSandbox_RVT_Titan_DH"), FVector::ZeroVector, RVTDH, Stats);
	}

	SpawnLighting(World);
	SpawnNativeGrass(World, TitanGrassType, NativeMaterial, NativeDarkerMaterial, Stats);
	SpawnFoliageInteractionProbe(World, FoliageUpdateMaterial, ProbePlaneMesh, Stats);
	SpawnPlayerStart(World, Stats);
	SpawnSmokeCameras(World);

	const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, SandboxMapPath);
	const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
	WriteReport(Stats);

	UE_LOG(LogTemp, Display, TEXT("TitanMainHostSandbox: savedMap=%s savedPackages=%s landscapes=%d rvt=%d grassComponents=%d grassInstances=%d foliageProbe=%d playerStart=%s"),
		bSavedMap ? TEXT("true") : TEXT("false"),
		bSavedPackages ? TEXT("true") : TEXT("false"),
		Stats.Landscapes,
		Stats.RVTVolumes,
		Stats.NativeGrassComponents,
		Stats.NativeGrassInstances,
		Stats.FoliageInteractionProbes,
		*Stats.PlayerStartLocation.ToString());

	return (bSavedMap && bSavedPackages && Stats.Landscapes == 1 && Stats.RVTVolumes >= 2 && Stats.NativeGrassInstances > 0) ? 0 : 1;
#else
	UE_LOG(LogTemp, Error, TEXT("FFTitanMainGrasslandHostSandboxCommandlet can only run in editor builds."));
	return 1;
#endif
}
