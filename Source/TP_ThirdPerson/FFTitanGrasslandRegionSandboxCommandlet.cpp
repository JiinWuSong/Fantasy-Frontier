#include "FFTitanGrasslandRegionSandboxCommandlet.h"

#if WITH_EDITOR
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/LightComponent.h"
#include "Components/RuntimeVirtualTextureComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/LevelStreamingAlwaysLoaded.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "FileHelpers.h"
#include "GameFramework/PlayerStart.h"
#include "InstancedFoliageActor.h"
#include "Landscape.h"
#include "LandscapeInfo.h"
#include "LandscapeLayerInfoObject.h"
#include "LandscapeProxy.h"
#include "Misc/FileHelper.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/PackageName.h"
#include "VT/RuntimeVirtualTextureVolume.h"
#include "WorldPartition/LoaderAdapter/LoaderAdapterShape.h"
#include "WorldPartition/WorldPartition.h"
#include "WorldPartition/WorldPartitionActorLoaderInterface.h"
#include "WorldPartition/WorldPartitionEditorLoaderAdapter.h"
#endif

UFFTitanGrasslandRegionSandboxCommandlet::UFFTitanGrasslandRegionSandboxCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

#if WITH_EDITOR
namespace
{
	const TCHAR* SandboxMapPath = TEXT("/Game/Maps/FF_Starter_Highland_Blockout_TitanRegionSandbox");
	const TCHAR* DefaultSourceRegionMapPath = TEXT("/Game/Environment/Grassland/Blockout/LI_Lake_Cliffs");
	const FName SandboxTag(TEXT("FFTitanGrasslandRegionSandbox"));
	const TCHAR* ReportPath = TEXT("TitanGrasslandRegionSandbox_Report.txt");
	const TCHAR* CandidateReportPath = TEXT("TitanGrasslandRegionCandidateAudit.txt");

	struct FRegionAudit
	{
		FString SourceMap;
		FBox LandscapeBounds = FBox(ForceInit);
		FVector SpawnLocation = FVector::ZeroVector;
		int32 LandscapeCount = 0;
		int32 RVTVolumeCount = 0;
		int32 FoliageActorCount = 0;
		int32 HISMComponentCount = 0;
		int32 HISMInstanceCount = 0;
		int32 DirectionalLightCount = 0;
		int32 SkyLightCount = 0;
		int32 FogCount = 0;
		int32 SkyAtmosphereCount = 0;
		TSet<FString> LandscapeMaterials;
		TSet<FString> LayerInfos;
		TSet<FString> GrassMeshes;
		TSet<FString> GrassMaterials;
		TArray<FString> Notes;
	};

	FString ObjectPath(const UObject* Object)
	{
		return Object ? Object->GetPathName() : FString(TEXT("None"));
	}

	UWorldPartitionEditorLoaderAdapter* LoadWorldPartitionAuditCells(UWorld* World, FRegionAudit& Audit)
	{
		if (!World)
		{
			return nullptr;
		}

		UWorldPartition* WorldPartition = World->GetWorldPartition();
		if (!WorldPartition)
		{
			Audit.Notes.Add(TEXT("Source region is not a World Partition map; auditing currently loaded actors only."));
			return nullptr;
		}

		// The candidate level instances are small authored regions, but their actors live in external
		// World Partition cells. Load a bounded editor region so the audit sees the real landscape stack.
		constexpr double RegionRadius = 500000.0;
		constexpr double RegionHeight = 200000.0;
		const FBox LoadCellsBox(
			FVector(-RegionRadius, -RegionRadius, -RegionHeight),
			FVector(RegionRadius, RegionRadius, RegionHeight));

		UWorldPartitionEditorLoaderAdapter* EditorLoaderAdapter =
			WorldPartition->CreateEditorLoaderAdapter<FLoaderAdapterShape>(World, LoadCellsBox, TEXT("Titan Region Sandbox Audit"));
		if (!EditorLoaderAdapter || !EditorLoaderAdapter->GetLoaderAdapter())
		{
			Audit.Notes.Add(TEXT("World Partition exists, but the editor loader adapter could not be created."));
			return nullptr;
		}

		EditorLoaderAdapter->GetLoaderAdapter()->Load();
		Audit.Notes.Add(FString::Printf(TEXT("Loaded World Partition audit cells in bounds min=%s max=%s."),
			*LoadCellsBox.Min.ToString(),
			*LoadCellsBox.Max.ToString()));
		return EditorLoaderAdapter;
	}

	void ReleaseWorldPartitionAuditCells(UWorld* World, UWorldPartitionEditorLoaderAdapter* EditorLoaderAdapter)
	{
		if (!World || !EditorLoaderAdapter)
		{
			return;
		}

		if (UWorldPartition* WorldPartition = World->GetWorldPartition())
		{
			WorldPartition->ReleaseEditorLoaderAdapter(EditorLoaderAdapter);
		}
	}

	bool TraceLandscape(UWorld* World, const FVector2D& XY, FHitResult& OutHit)
	{
		if (!World)
		{
			return false;
		}

		FCollisionQueryParams Params(SCENE_QUERY_STAT(FFTitanRegionSandboxTrace), true);
		const FVector Start(XY.X, XY.Y, 200000.0f);
		const FVector End(XY.X, XY.Y, -200000.0f);
		if (!World->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, Params))
		{
			return false;
		}

		return OutHit.GetActor() && OutHit.GetActor()->IsA<ALandscapeProxy>();
	}

	FVector FindSpawnOnLandscape(UWorld* World, const FBox& Bounds)
	{
		if (!World || !Bounds.IsValid)
		{
			return FVector(0.0f, 0.0f, 2000.0f);
		}

		const FVector Center = Bounds.GetCenter();
		const FVector Extent = Bounds.GetExtent();
		const float StepX = FMath::Max(Extent.X * 0.20f, 1200.0f);
		const float StepY = FMath::Max(Extent.Y * 0.20f, 1200.0f);

		TArray<FVector2D> Candidates;
		Candidates.Add(FVector2D(Center.X, Center.Y));
		for (int32 Radius = 1; Radius <= 3; ++Radius)
		{
			for (int32 Y = -Radius; Y <= Radius; ++Y)
			{
				for (int32 X = -Radius; X <= Radius; ++X)
				{
					if (FMath::Abs(X) != Radius && FMath::Abs(Y) != Radius)
					{
						continue;
					}
					Candidates.Add(FVector2D(Center.X + X * StepX, Center.Y + Y * StepY));
				}
			}
		}

		for (const FVector2D& XY : Candidates)
		{
			if (!Bounds.IsInsideOrOn(FVector(XY.X, XY.Y, Center.Z)))
			{
				continue;
			}

			FHitResult Hit;
			if (TraceLandscape(World, XY, Hit) && Hit.ImpactNormal.Z > 0.82f)
			{
				return Hit.ImpactPoint + FVector(0.0f, 0.0f, 120.0f);
			}
		}

		FHitResult Hit;
		if (TraceLandscape(World, FVector2D(Center.X, Center.Y), Hit))
		{
			return Hit.ImpactPoint + FVector(0.0f, 0.0f, 120.0f);
		}

		return Center + FVector(0.0f, 0.0f, 2000.0f);
	}

	void AuditWorld(UWorld* World, FRegionAudit& Audit)
	{
		if (!World)
		{
			return;
		}

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (!Actor)
			{
				continue;
			}

			if (ALandscapeProxy* Landscape = Cast<ALandscapeProxy>(Actor))
			{
				++Audit.LandscapeCount;
				Audit.LandscapeBounds += Landscape->GetComponentsBoundingBox(true);
				Audit.LandscapeMaterials.Add(ObjectPath(Landscape->LandscapeMaterial));

				if (ULandscapeInfo* Info = Landscape->GetLandscapeInfo())
				{
					for (const FLandscapeInfoLayerSettings& LayerSettings : Info->Layers)
					{
						if (LayerSettings.LayerInfoObj)
						{
							Audit.LayerInfos.Add(ObjectPath(LayerSettings.LayerInfoObj));
						}
					}
				}
			}
			else if (Actor->IsA<ARuntimeVirtualTextureVolume>())
			{
				++Audit.RVTVolumeCount;
			}
			else if (Actor->IsA<AInstancedFoliageActor>())
			{
				++Audit.FoliageActorCount;
			}
			else if (Actor->IsA<ADirectionalLight>())
			{
				++Audit.DirectionalLightCount;
			}
			else if (Actor->IsA<ASkyLight>())
			{
				++Audit.SkyLightCount;
			}
			else if (Actor->IsA<AExponentialHeightFog>())
			{
				++Audit.FogCount;
			}
			else if (Actor->GetClass()->GetName().Contains(TEXT("SkyAtmosphere")))
			{
				++Audit.SkyAtmosphereCount;
			}

			TArray<UHierarchicalInstancedStaticMeshComponent*> HISMComponents;
			Actor->GetComponents(HISMComponents);
			for (UHierarchicalInstancedStaticMeshComponent* HISM : HISMComponents)
			{
				if (!HISM)
				{
					continue;
				}

				++Audit.HISMComponentCount;
				Audit.HISMInstanceCount += HISM->GetInstanceCount();
				if (HISM->GetStaticMesh())
				{
					Audit.GrassMeshes.Add(ObjectPath(HISM->GetStaticMesh()));
				}
				for (int32 Index = 0; Index < HISM->GetNumMaterials(); ++Index)
				{
					Audit.GrassMaterials.Add(ObjectPath(HISM->GetMaterial(Index)));
				}
			}
		}

		Audit.SpawnLocation = FindSpawnOnLandscape(World, Audit.LandscapeBounds);
		if (Audit.DirectionalLightCount == 0 || Audit.SkyLightCount == 0 || Audit.FogCount == 0)
		{
			Audit.Notes.Add(TEXT("Source region does not carry a complete standalone lighting/fog stack; sandbox adds a minimal stable host light for package visibility."));
		}
	}

	void AppendSet(FString& Report, const TCHAR* Title, const TSet<FString>& Values)
	{
		Report += FString::Printf(TEXT("\n%s:\n"), Title);
		if (Values.Num() == 0)
		{
			Report += TEXT("- None found\n");
			return;
		}

		TArray<FString> Sorted = Values.Array();
		Sorted.Sort();
		for (const FString& Value : Sorted)
		{
			Report += FString::Printf(TEXT("- %s\n"), *Value);
		}
	}

	void WriteAuditReport(const FRegionAudit& Audit)
	{
		FString Report;
		Report += TEXT("# Titan Grassland Region Sandbox Report\n\n");
		Report += FString::Printf(TEXT("Source region: %s\n"), *Audit.SourceMap);
		Report += FString::Printf(TEXT("Sandbox map: %s\n"), SandboxMapPath);
		Report += FString::Printf(TEXT("Landscape count: %d\n"), Audit.LandscapeCount);
		Report += FString::Printf(TEXT("Landscape bounds: min=%s max=%s\n"), *Audit.LandscapeBounds.Min.ToString(), *Audit.LandscapeBounds.Max.ToString());
		Report += FString::Printf(TEXT("Spawn location: %s\n"), *Audit.SpawnLocation.ToString());
		Report += FString::Printf(TEXT("RVT volumes: %d\n"), Audit.RVTVolumeCount);
		Report += FString::Printf(TEXT("Foliage actors: %d\n"), Audit.FoliageActorCount);
		Report += FString::Printf(TEXT("HISM components: %d\n"), Audit.HISMComponentCount);
		Report += FString::Printf(TEXT("HISM instances: %d\n"), Audit.HISMInstanceCount);
		Report += FString::Printf(TEXT("Directional lights: %d\n"), Audit.DirectionalLightCount);
		Report += FString::Printf(TEXT("Sky lights: %d\n"), Audit.SkyLightCount);
		Report += FString::Printf(TEXT("Fog actors: %d\n"), Audit.FogCount);
		Report += FString::Printf(TEXT("SkyAtmosphere actors: %d\n"), Audit.SkyAtmosphereCount);

		AppendSet(Report, TEXT("Landscape materials"), Audit.LandscapeMaterials);
		AppendSet(Report, TEXT("Layer infos"), Audit.LayerInfos);
		AppendSet(Report, TEXT("Foliage/HISM meshes"), Audit.GrassMeshes);
		AppendSet(Report, TEXT("Foliage/HISM materials"), Audit.GrassMaterials);

		Report += TEXT("\nNotes:\n");
		if (Audit.Notes.Num() == 0)
		{
			Report += TEXT("- None\n");
		}
		else
		{
			for (const FString& Note : Audit.Notes)
			{
				Report += FString::Printf(TEXT("- %s\n"), *Note);
			}
		}

		const FString FullPath = FPaths::Combine(FPaths::ProjectSavedDir(), ReportPath);
		FFileHelper::SaveStringToFile(Report, *FullPath);
		UE_LOG(LogTemp, Display, TEXT("TitanRegionSandbox: wrote report %s"), *FullPath);
	}

	void WriteCandidateAuditReport(const TArray<FRegionAudit>& Audits)
	{
		FString Report;
		Report += TEXT("# Titan Grassland Region Candidate Audit\n\n");
		Report += TEXT("Purpose: identify a real small Titan Grassland source region with Landscape/RVT/Foliage context, without modifying active Highland.\n\n");

		for (const FRegionAudit& Audit : Audits)
		{
			Report += FString::Printf(TEXT("## %s\n"), *Audit.SourceMap);
			Report += FString::Printf(TEXT("- Landscape count: %d\n"), Audit.LandscapeCount);
			Report += FString::Printf(TEXT("- Landscape bounds valid: %s\n"), Audit.LandscapeBounds.IsValid ? TEXT("YES") : TEXT("NO"));
			Report += FString::Printf(TEXT("- RVT volumes: %d\n"), Audit.RVTVolumeCount);
			Report += FString::Printf(TEXT("- Foliage actors: %d\n"), Audit.FoliageActorCount);
			Report += FString::Printf(TEXT("- HISM components: %d\n"), Audit.HISMComponentCount);
			Report += FString::Printf(TEXT("- HISM instances: %d\n"), Audit.HISMInstanceCount);
			Report += FString::Printf(TEXT("- Lighting/fog: directional=%d skylight=%d fog=%d skyAtmosphere=%d\n"),
				Audit.DirectionalLightCount,
				Audit.SkyLightCount,
				Audit.FogCount,
				Audit.SkyAtmosphereCount);
			Report += FString::Printf(TEXT("- Recommended for region reproduction: %s\n"),
				(Audit.LandscapeCount > 0 && Audit.LandscapeBounds.IsValid) ? TEXT("POSSIBLE") : TEXT("NO - no Landscape actor loaded"));
			if (Audit.LandscapeMaterials.Num() > 0)
			{
				TArray<FString> LandscapeMaterials = Audit.LandscapeMaterials.Array();
				LandscapeMaterials.Sort();
				Report += TEXT("- Landscape materials:\n");
				for (const FString& Material : LandscapeMaterials)
				{
					Report += FString::Printf(TEXT("  - %s\n"), *Material);
				}
			}
			if (Audit.LayerInfos.Num() > 0)
			{
				TArray<FString> LayerInfos = Audit.LayerInfos.Array();
				LayerInfos.Sort();
				Report += TEXT("- Layer infos:\n");
				for (const FString& LayerInfo : LayerInfos)
				{
					Report += FString::Printf(TEXT("  - %s\n"), *LayerInfo);
				}
			}
			if (Audit.Notes.Num() > 0)
			{
				Report += TEXT("- Notes:\n");
				for (const FString& Note : Audit.Notes)
				{
					Report += FString::Printf(TEXT("  - %s\n"), *Note);
				}
			}
			Report += TEXT("\n");
		}

		const FString FullPath = FPaths::Combine(FPaths::ProjectSavedDir(), CandidateReportPath);
		FFileHelper::SaveStringToFile(Report, *FullPath);
		UE_LOG(LogTemp, Display, TEXT("TitanRegionSandbox: wrote candidate audit %s"), *FullPath);
	}

	bool RunCandidateAudit(const FString& CandidateList)
	{
		TArray<FString> CandidatePaths;
		CandidateList.ParseIntoArray(CandidatePaths, TEXT(";"), true);
		if (CandidatePaths.Num() == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("TitanRegionSandbox: AuditCandidates was provided without candidates."));
			return false;
		}

		TArray<FRegionAudit> Audits;
		for (FString CandidatePath : CandidatePaths)
		{
			CandidatePath.TrimStartAndEndInline();
			if (CandidatePath.IsEmpty())
			{
				continue;
			}

			UE_LOG(LogTemp, Display, TEXT("TitanRegionSandbox: candidate audit loading %s"), *CandidatePath);
			UWorld* CandidateWorld = UEditorLoadingAndSavingUtils::LoadMap(CandidatePath);
			FRegionAudit Audit;
			Audit.SourceMap = CandidatePath;
			if (!CandidateWorld)
			{
				Audit.Notes.Add(TEXT("Failed to load map."));
				Audits.Add(MoveTemp(Audit));
				continue;
			}

			UWorldPartitionEditorLoaderAdapter* EditorLoaderAdapter = LoadWorldPartitionAuditCells(CandidateWorld, Audit);
			AuditWorld(CandidateWorld, Audit);
			ReleaseWorldPartitionAuditCells(CandidateWorld, EditorLoaderAdapter);
			Audits.Add(MoveTemp(Audit));
		}

		WriteCandidateAuditReport(Audits);
		return true;
	}

	void ClearSandboxWorld(UWorld* World)
	{
		if (!World)
		{
			return;
		}

		TArray<AActor*> ActorsToDestroy;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && !Actor->IsA<AWorldSettings>())
			{
				ActorsToDestroy.Add(Actor);
			}
		}

		for (AActor* Actor : ActorsToDestroy)
		{
			World->DestroyActor(Actor, true);
		}

		World->SetStreamingLevels(TArray<ULevelStreaming*>());
	}

	UWorld* CreateOrLoadSandboxMap()
	{
		if (FPackageName::DoesPackageExist(SandboxMapPath))
		{
			if (UWorld* ExistingWorld = UEditorLoadingAndSavingUtils::LoadMap(SandboxMapPath))
			{
				ClearSandboxWorld(ExistingWorld);
				return ExistingWorld;
			}
		}

		UWorld* NewWorld = UEditorLoadingAndSavingUtils::NewBlankMap(false);
		if (NewWorld)
		{
			UEditorLoadingAndSavingUtils::SaveMap(NewWorld, SandboxMapPath);
			ClearSandboxWorld(NewWorld);
		}
		return NewWorld;
	}

	void AddSourceRegionStreamingLevel(UWorld* SandboxWorld, const FString& SourceRegionMapPath)
	{
		ULevelStreamingAlwaysLoaded* StreamingLevel = NewObject<ULevelStreamingAlwaysLoaded>(SandboxWorld, ULevelStreamingAlwaysLoaded::StaticClass(), NAME_None, RF_Transactional);
		StreamingLevel->SetWorldAssetByPackageName(FName(SourceRegionMapPath));
		StreamingLevel->SetShouldBeLoaded(true);
		StreamingLevel->SetShouldBeVisible(true);
		StreamingLevel->bShouldBlockOnLoad = true;
		SandboxWorld->AddStreamingLevel(StreamingLevel);
	}

	template<typename TActorClass>
	TActorClass* SpawnTaggedActor(UWorld* World, const FString& Label, const FVector& Location, const FRotator& Rotation)
	{
		TActorClass* Actor = World ? World->SpawnActor<TActorClass>(Location, Rotation) : nullptr;
		if (Actor)
		{
			Actor->SetActorLabel(Label);
			Actor->Tags.AddUnique(SandboxTag);
		}
		return Actor;
	}

	void SpawnMinimalHostLighting(UWorld* World, const FRegionAudit& Audit)
	{
		if (!World || (Audit.DirectionalLightCount > 0 && Audit.SkyLightCount > 0 && Audit.FogCount > 0))
		{
			return;
		}

		ADirectionalLight* DirectionalLight = SpawnTaggedActor<ADirectionalLight>(
			World,
			TEXT("FF_TitanRegionSandbox_Minimal_DirectionalLight"),
			FVector(0.0f, 0.0f, 8000.0f),
			FRotator(-42.0f, 32.0f, 0.0f));
		if (DirectionalLight && DirectionalLight->GetLightComponent())
		{
			DirectionalLight->GetLightComponent()->SetIntensity(4.0f);
			DirectionalLight->GetLightComponent()->SetCastShadows(true);
		}

		ASkyLight* SkyLight = SpawnTaggedActor<ASkyLight>(
			World,
			TEXT("FF_TitanRegionSandbox_Minimal_SkyLight"),
			FVector(0.0f, 0.0f, 6000.0f),
			FRotator::ZeroRotator);
		if (SkyLight && SkyLight->GetLightComponent())
		{
			SkyLight->GetLightComponent()->SetIntensity(0.75f);
		}

		AExponentialHeightFog* Fog = SpawnTaggedActor<AExponentialHeightFog>(
			World,
			TEXT("FF_TitanRegionSandbox_Minimal_HeightFog"),
			FVector(0.0f, 0.0f, 0.0f),
			FRotator::ZeroRotator);
		if (Fog && Fog->GetComponent())
		{
			Fog->GetComponent()->SetFogDensity(0.0009f);
			Fog->GetComponent()->SetStartDistance(4500.0f);
			Fog->GetComponent()->SetFogMaxOpacity(0.18f);
		}
	}

	void SpawnPlayerStart(UWorld* World, const FRegionAudit& Audit)
	{
		APlayerStart* PlayerStart = SpawnTaggedActor<APlayerStart>(
			World,
			TEXT("FF_TitanRegionSandbox_PlayerStart"),
			Audit.SpawnLocation,
			FRotator(0.0f, 40.0f, 0.0f));
		if (PlayerStart)
		{
			PlayerStart->PlayerStartTag = FName(TEXT("TitanRegionSandbox"));
		}
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

	void SpawnSmokeCameras(UWorld* World, const FRegionAudit& Audit)
	{
		if (!World || !Audit.LandscapeBounds.IsValid)
		{
			return;
		}

		const FVector Center = Audit.LandscapeBounds.GetCenter();
		const FVector Extent = Audit.LandscapeBounds.GetExtent();
		const float MaxExtent = FMath::Max3(static_cast<float>(Extent.X), static_cast<float>(Extent.Y), 25000.0f);
		const FVector Spawn = Audit.SpawnLocation;

		SpawnCamera(
			World,
			TEXT("FF_TitanRegionSandbox_ReferenceCamera"),
			FName(TEXT("FFSmokeTitanRegionReferenceCamera")),
			Spawn + FVector(-2400.0f, -2600.0f, 1300.0f),
			FRotator(-14.0f, 42.0f, 0.0f),
			45.0f);

		SpawnCamera(
			World,
			TEXT("FF_TitanRegionSandbox_SunCamera"),
			FName(TEXT("FFSmokeTitanRegionSunCamera")),
			Spawn + FVector(-1200.0f, -1600.0f, 520.0f),
			FRotator(-8.0f, 36.0f, 0.0f),
			38.0f);

		SpawnCamera(
			World,
			TEXT("FF_TitanRegionSandbox_ShadowCamera"),
			FName(TEXT("FFSmokeTitanRegionShadowCamera")),
			Spawn + FVector(1600.0f, 2200.0f, 620.0f),
			FRotator(-9.0f, -142.0f, 0.0f),
			38.0f);

		SpawnCamera(
			World,
			TEXT("FF_TitanRegionSandbox_GroundCloseCamera"),
			FName(TEXT("FFSmokeTitanRegionGroundCloseCamera")),
			Spawn + FVector(-420.0f, -640.0f, 180.0f),
			FRotator(-18.0f, 39.0f, 0.0f),
			34.0f);

		SpawnCamera(
			World,
			TEXT("FF_TitanRegionSandbox_GrassCloseCamera"),
			FName(TEXT("FFSmokeTitanRegionGrassCloseCamera")),
			Spawn + FVector(-280.0f, -360.0f, 120.0f),
			FRotator(-12.0f, 44.0f, 0.0f),
			30.0f);

		SpawnCamera(
			World,
			TEXT("FF_TitanRegionSandbox_TopDownCamera"),
			FName(TEXT("FFSmokeTopDownCamera")),
			Center + FVector(0.0f, 0.0f, MaxExtent * 2.2f),
			FRotator(-90.0f, 0.0f, 0.0f),
			62.0f);
	}
}
#endif

int32 UFFTitanGrasslandRegionSandboxCommandlet::Main(const FString& Params)
{
#if WITH_EDITOR
	FString CandidateList;
	if (FParse::Value(FCommandLine::Get(), TEXT("AuditCandidates="), CandidateList))
	{
		return RunCandidateAudit(CandidateList) ? 0 : 1;
	}

	FString SourceRegionMapPath = DefaultSourceRegionMapPath;
	FParse::Value(FCommandLine::Get(), TEXT("SourceRegion="), SourceRegionMapPath);
	UE_LOG(LogTemp, Display, TEXT("TitanRegionSandbox: loading source region %s"), *SourceRegionMapPath);
	UWorld* SourceWorld = UEditorLoadingAndSavingUtils::LoadMap(SourceRegionMapPath);
	if (!SourceWorld)
	{
		UE_LOG(LogTemp, Error, TEXT("TitanRegionSandbox: failed to load source region."));
		return 1;
	}

	FRegionAudit Audit;
	Audit.SourceMap = SourceRegionMapPath;
	UWorldPartitionEditorLoaderAdapter* EditorLoaderAdapter = LoadWorldPartitionAuditCells(SourceWorld, Audit);
	AuditWorld(SourceWorld, Audit);
	WriteAuditReport(Audit);
	ReleaseWorldPartitionAuditCells(SourceWorld, EditorLoaderAdapter);

	if (Audit.LandscapeCount <= 0 || !Audit.LandscapeBounds.IsValid)
	{
		UE_LOG(LogTemp, Error, TEXT("TitanRegionSandbox: source region has no valid landscape."));
		return 1;
	}

	UE_LOG(LogTemp, Display, TEXT("TitanRegionSandbox: creating sandbox map %s"), SandboxMapPath);
	UWorld* SandboxWorld = CreateOrLoadSandboxMap();
	if (!SandboxWorld)
	{
		UE_LOG(LogTemp, Error, TEXT("TitanRegionSandbox: failed to create sandbox map."));
		return 1;
	}

	AddSourceRegionStreamingLevel(SandboxWorld, SourceRegionMapPath);
	SpawnMinimalHostLighting(SandboxWorld, Audit);
	SpawnPlayerStart(SandboxWorld, Audit);
	SpawnSmokeCameras(SandboxWorld, Audit);

	const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(SandboxWorld, SandboxMapPath);
	UE_LOG(LogTemp, Display, TEXT("TitanRegionSandbox: savedMap=%s source=%s spawn=%s landscapes=%d rvt=%d foliageActors=%d hismComponents=%d hismInstances=%d"),
		bSavedMap ? TEXT("true") : TEXT("false"),
		*SourceRegionMapPath,
		*Audit.SpawnLocation.ToString(),
		Audit.LandscapeCount,
		Audit.RVTVolumeCount,
		Audit.FoliageActorCount,
		Audit.HISMComponentCount,
		Audit.HISMInstanceCount);

	return bSavedMap ? 0 : 1;
#else
	UE_LOG(LogTemp, Error, TEXT("FFTitanGrasslandRegionSandboxCommandlet can only run in editor builds."));
	return 1;
#endif
}
