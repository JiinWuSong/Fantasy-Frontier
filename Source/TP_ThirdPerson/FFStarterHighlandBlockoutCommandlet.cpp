#include "FFStarterHighlandBlockoutCommandlet.h"

#if WITH_EDITOR
#include "AssetToolsModule.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/LocalPlayer.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Factories/MaterialFactoryNew.h"
#include "FileHelpers.h"
#include "Landscape.h"
#include "LandscapeInfo.h"
#include "LandscapeProxy.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionClamp.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionDivide.h"
#include "Materials/MaterialExpressionLinearInterpolate.h"
#include "Materials/MaterialExpressionSaturate.h"
#include "Materials/MaterialExpressionSubtract.h"
#include "Materials/MaterialExpressionWorldPosition.h"
#include "MaterialEditingLibrary.h"
#include "Misc/PackageName.h"
#include "UObject/SavePackage.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/WorldSettings.h"
#endif

UFFStarterHighlandBlockoutCommandlet::UFFStarterHighlandBlockoutCommandlet()
{
	LogToConsole = true;
	IsClient = false;
	IsEditor = true;
	IsServer = false;
}

#if WITH_EDITOR
namespace
{
	const FString MapAssetPath = TEXT("/Game/Maps/FF_Starter_Highland_Blockout");
	const FString MaterialFolderPath = TEXT("/Game/FantasyFrontier/Blockout/Materials");
	const FString LandscapeMaterialName = TEXT("M_FF_HighlandLandscape_Blockout");
	const FString WaterMaterialName = TEXT("M_FF_HighlandWater_Blockout");
	const FString MarkerMaterialName = TEXT("M_FF_HighlandMarker_Blockout");
	const FString PathMaterialName = TEXT("M_FF_HighlandPath_Blockout");
	const FString ForestMassMaterialName = TEXT("M_FF_HighlandForestMass_Blockout");
	const FString CanyonFloorMaterialName = TEXT("M_FF_HighlandCanyonFloor_Blockout");
	const FString TreeTrunkMaterialName = TEXT("M_FF_HighlandTreeTrunk_Blockout");
	const FString TreeCanopyMaterialName = TEXT("M_FF_HighlandTreeCanopy_Blockout");
	const TCHAR* TopDownCameraTag = TEXT("FFSmokeTopDownCamera");
	const TCHAR* VillageMarkerTag = TEXT("FFStarterVillagePlaceholder");
	const TCHAR* BridgeMarkerTag = TEXT("FFStarterNorthBridgePlaceholder");
	const TCHAR* BossGateMarkerTag = TEXT("FFStarterBossGatePlaceholder");
	const int32 SectionsPerComponent = 1;
	const int32 QuadsPerSection = 63;
	const int32 ComponentCountX = 16;
	const int32 ComponentCountY = 16;
	const float MapWidth = 220000.0f;
	const float MapDepth = 160000.0f;
	const FVector2D MapCenter = FVector2D(0.0f, 15000.0f);
	const FVector LandscapeScale = FVector(MapWidth / 1008.0f, MapDepth / 1008.0f, 200.0f);
	const float CanyonFloorZ = -32000.0f;
	const float PlateauBaseZ = 0.0f;
	const float LakeWaterZ = -140.0f;
	const FVector SpawnHint = FVector(-50000.0f, -25000.0f, 6000.0f);
	const FVector VillageHint = FVector(-18000.0f, 4000.0f, 6000.0f);
	const FVector BridgeHint = FVector(16000.0f, 87000.0f, 8000.0f);
	const FVector BossGateHint = FVector(76000.0f, 32000.0f, 8000.0f);
	const FVector2D LakeCenter = FVector2D(-30000.0f, 62000.0f);
	const FVector2D LakeRadii = FVector2D(5200.0f, 3800.0f);

	FVector2D GetMapHalfExtents()
	{
		return FVector2D(MapWidth * 0.5f, MapDepth * 0.5f);
	}

	FVector2D GetMapMin()
	{
		return MapCenter - GetMapHalfExtents();
	}

	FVector2D GetMapMax()
	{
		return MapCenter + GetMapHalfExtents();
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

	UMaterial* BuildSolidMaterial(const FString& AssetName, const FLinearColor& BaseColor, float Roughness)
	{
		UMaterial* Material = CreateOrLoadMaterial(MaterialFolderPath, AssetName);
		if (!Material)
		{
			return nullptr;
		}

		UMaterialExpressionConstant3Vector* BaseColorNode = Cast<UMaterialExpressionConstant3Vector>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant3Vector::StaticClass(), -320, -40));
		UMaterialExpressionConstant* RoughnessNode = Cast<UMaterialExpressionConstant>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant::StaticClass(), -320, 120));

		BaseColorNode->Constant = BaseColor;
		RoughnessNode->R = Roughness;

		UMaterialEditingLibrary::ConnectMaterialProperty(BaseColorNode, TEXT(""), MP_BaseColor);
		UMaterialEditingLibrary::ConnectMaterialProperty(RoughnessNode, TEXT(""), MP_Roughness);
		FinalizeMaterial(Material);
		return Material;
	}

	UMaterial* BuildLandscapeMaterial()
	{
		UMaterial* Material = CreateOrLoadMaterial(MaterialFolderPath, LandscapeMaterialName);
		if (!Material)
		{
			return nullptr;
		}

		UMaterialExpressionConstant3Vector* GrassColor = Cast<UMaterialExpressionConstant3Vector>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant3Vector::StaticClass(), -320, -40));
		UMaterialExpressionConstant* RoughnessNode = Cast<UMaterialExpressionConstant>(
			UMaterialEditingLibrary::CreateMaterialExpression(Material, UMaterialExpressionConstant::StaticClass(), -320, 120));

		GrassColor->Constant = FLinearColor(0.18f, 0.43f, 0.14f);
		RoughnessNode->R = 0.96f;

		Material->BlendMode = BLEND_Opaque;
		Material->TwoSided = false;

		UMaterialEditingLibrary::ConnectMaterialProperty(GrassColor, TEXT(""), MP_BaseColor);
		UMaterialEditingLibrary::ConnectMaterialProperty(RoughnessNode, TEXT(""), MP_Roughness);
		FinalizeMaterial(Material);
		return Material;
	}

	bool IsPointInsidePolygon(const TArray<FVector2D>& Polygon, const FVector2D& Point)
	{
		bool bInside = false;
		for (int32 i = 0, j = Polygon.Num() - 1; i < Polygon.Num(); j = i++)
		{
			const FVector2D& A = Polygon[i];
			const FVector2D& B = Polygon[j];
			const float DeltaY = B.Y - A.Y;
			if (FMath::Abs(DeltaY) <= KINDA_SMALL_NUMBER)
			{
				continue;
			}
			const bool bIntersects = ((A.Y > Point.Y) != (B.Y > Point.Y))
				&& (Point.X < (B.X - A.X) * (Point.Y - A.Y) / DeltaY + A.X);
			if (bIntersects)
			{
				bInside = !bInside;
			}
		}
		return bInside;
	}

	float DistancePointToSegment2D(const FVector2D& Point, const FVector2D& SegmentStart, const FVector2D& SegmentEnd)
	{
		const FVector2D Segment = SegmentEnd - SegmentStart;
		const float SegmentLengthSquared = Segment.SizeSquared();
		if (SegmentLengthSquared <= KINDA_SMALL_NUMBER)
		{
			return FVector2D::Distance(Point, SegmentStart);
		}

		const float Alpha = FMath::Clamp(FVector2D::DotProduct(Point - SegmentStart, Segment) / SegmentLengthSquared, 0.0f, 1.0f);
		const FVector2D ClosestPoint = SegmentStart + Alpha * Segment;
		return FVector2D::Distance(Point, ClosestPoint);
	}

	float DistanceToPolygonBoundary(const TArray<FVector2D>& Polygon, const FVector2D& Point)
	{
		float BestDistance = TNumericLimits<float>::Max();
		for (int32 Index = 0; Index < Polygon.Num(); ++Index)
		{
			const FVector2D& SegmentStart = Polygon[Index];
			const FVector2D& SegmentEnd = Polygon[(Index + 1) % Polygon.Num()];
			BestDistance = FMath::Min(BestDistance, DistancePointToSegment2D(Point, SegmentStart, SegmentEnd));
		}
		return BestDistance;
	}

	float DistanceToPolyline(const TArray<FVector2D>& Points, const FVector2D& Point)
	{
		float BestDistance = TNumericLimits<float>::Max();
		for (int32 Index = 0; Index + 1 < Points.Num(); ++Index)
		{
			BestDistance = FMath::Min(BestDistance, DistancePointToSegment2D(Point, Points[Index], Points[Index + 1]));
		}
		return BestDistance;
	}

	float SmoothBlend(float Distance, float InnerRadius, float OuterRadius)
	{
		if (Distance <= InnerRadius)
		{
			return 0.0f;
		}
		if (Distance >= OuterRadius)
		{
			return 1.0f;
		}

		const float Alpha = (Distance - InnerRadius) / FMath::Max(OuterRadius - InnerRadius, KINDA_SMALL_NUMBER);
		return FMath::SmoothStep(0.0f, 1.0f, Alpha);
	}

	uint16 EncodeLandscapeHeight(float WorldZ)
	{
		const float HeightValue = 32768.0f + (WorldZ * 128.0f / LandscapeScale.Z);
		return static_cast<uint16>(FMath::Clamp(FMath::RoundToInt(HeightValue), 0, 65535));
	}

	TArray<FVector2D> GetPlateauPolygonNormalized()
	{
		return {
			FVector2D(-0.58f, -0.90f),
			FVector2D(-0.18f, -0.96f),
			FVector2D(0.31f, -0.88f),
			FVector2D(0.66f, -0.69f),
			FVector2D(0.86f, -0.34f),
			FVector2D(0.70f, -0.04f),
			FVector2D(0.91f, 0.33f),
			FVector2D(0.66f, 0.68f),
			FVector2D(0.23f, 0.91f),
			FVector2D(-0.16f, 0.88f),
			FVector2D(-0.54f, 0.76f),
			FVector2D(-0.84f, 0.43f),
			FVector2D(-0.72f, 0.13f),
			FVector2D(-0.92f, -0.21f),
			FVector2D(-0.77f, -0.58f)
		};
	}

	TArray<FVector2D> GetPlateauPolygonWorld()
	{
		TArray<FVector2D> PolygonWorld;
		const FVector2D HalfExtents = GetMapHalfExtents();
		for (const FVector2D& Point : GetPlateauPolygonNormalized())
		{
			PolygonWorld.Add(MapCenter + FVector2D(Point.X * HalfExtents.X, Point.Y * HalfExtents.Y));
		}
		return PolygonWorld;
	}

	TArray<FVector2D> GetRiverSpline()
	{
		return {
			FVector2D(-30000.0f, 64000.0f),
			FVector2D(-42000.0f, 50000.0f),
			FVector2D(-52000.0f, 28000.0f),
			FVector2D(-42000.0f, 8000.0f),
			FVector2D(-52000.0f, -14000.0f),
			FVector2D(-64000.0f, -38000.0f),
			FVector2D(-70000.0f, -62000.0f)
		};
	}

	TArray<FVector2D> GetRiverBranchSpline()
	{
		return {
			FVector2D(-42000.0f, 8000.0f),
			FVector2D(-15000.0f, 2000.0f),
			FVector2D(13000.0f, 8000.0f),
			FVector2D(42000.0f, 22000.0f),
			FVector2D(76000.0f, 32000.0f)
		};
	}

	TArray<FVector2D> GetBossGatePathSpline()
	{
		return {
			FVector2D(VillageHint.X, VillageHint.Y),
			FVector2D(9000.0f, 9000.0f),
			FVector2D(36000.0f, 21000.0f),
			FVector2D(BossGateHint.X, BossGateHint.Y)
		};
	}

	TArray<FVector2D> GetNorthBridgePathSpline()
	{
		return {
			FVector2D(10000.0f, 36000.0f),
			FVector2D(12000.0f, 60000.0f),
			FVector2D(BridgeHint.X, BridgeHint.Y)
		};
	}

	float ComputeBaseTerrainHeight(const FVector2D& WorldPoint)
	{
		const TArray<FVector2D> PlateauPolygon = GetPlateauPolygonWorld();
		const bool bInsidePlateau = IsPointInsidePolygon(PlateauPolygon, WorldPoint);
		const float DistanceToEdge = DistanceToPolygonBoundary(PlateauPolygon, WorldPoint);

		float Height = PlateauBaseZ;
		Height += FMath::PerlinNoise2D(WorldPoint / 24000.0f) * 120.0f;
		Height += FMath::Sin((WorldPoint.X + 12000.0f) / 26000.0f) * 60.0f;
		Height += FMath::Cos((WorldPoint.Y - 4000.0f) / 22000.0f) * 55.0f;

		const float ForestBandBlend = FMath::Clamp((WorldPoint.Y - 44000.0f) / 30000.0f, 0.0f, 1.0f);
		Height += ForestBandBlend * 1000.0f;

		const TArray<TPair<FVector2D, float>> HillCenters = {
			TPair<FVector2D, float>(FVector2D(-20000.0f, -18000.0f), 5200.0f),
			TPair<FVector2D, float>(FVector2D(35000.0f, -16000.0f), 4600.0f),
			TPair<FVector2D, float>(FVector2D(56000.0f, 30000.0f), 7400.0f),
			TPair<FVector2D, float>(FVector2D(-12000.0f, 36000.0f), 4300.0f)
		};
		for (const TPair<FVector2D, float>& Hill : HillCenters)
		{
			const float Distance = FVector2D::Distance(WorldPoint, Hill.Key);
			const float Alpha = FMath::Clamp(1.0f - Distance / 26000.0f, 0.0f, 1.0f);
			Height += Hill.Value * Alpha * Alpha;
		}

		const TArray<TPair<FVector2D, float>> EdgeMountains = {
			TPair<FVector2D, float>(FVector2D(-98000.0f, 23000.0f), 18500.0f),
			TPair<FVector2D, float>(FVector2D(-90000.0f, -22000.0f), 22000.0f),
			TPair<FVector2D, float>(FVector2D(-16000.0f, -62000.0f), 19000.0f),
			TPair<FVector2D, float>(FVector2D(52000.0f, -56000.0f), 15000.0f)
		};
		for (const TPair<FVector2D, float>& Mountain : EdgeMountains)
		{
			const float Distance = FVector2D::Distance(WorldPoint, Mountain.Key);
			const float Alpha = FMath::Clamp(1.0f - Distance / 24000.0f, 0.0f, 1.0f);
			Height += Mountain.Value * Alpha * Alpha;
		}

		const float SpawnFlattenBlend = SmoothBlend(FVector2D::Distance(WorldPoint, FVector2D(SpawnHint.X, SpawnHint.Y)), 4500.0f, 10000.0f);
		Height = FMath::Lerp(0.0f, Height, SpawnFlattenBlend);

		const float StartAreaFlattenBlend = SmoothBlend(FVector2D::Distance(WorldPoint, FVector2D(VillageHint.X, VillageHint.Y)), 6000.0f, 12000.0f);
		Height = FMath::Lerp(0.0f, Height, StartAreaFlattenBlend);

		const FVector2D LakeDelta = WorldPoint - LakeCenter;
		const float LakeMask = FMath::Clamp(FMath::Sqrt(FMath::Square(LakeDelta.X / LakeRadii.X) + FMath::Square(LakeDelta.Y / LakeRadii.Y)), 0.0f, 4.0f);
		const float LakeBlend = FMath::Clamp((LakeMask - 0.55f) / 0.45f, 0.0f, 1.0f);
		Height = FMath::Lerp(LakeWaterZ - 160.0f, Height, LakeBlend);

		const float RiverDistance = FMath::Min(DistanceToPolyline(GetRiverSpline(), WorldPoint), DistanceToPolyline(GetRiverBranchSpline(), WorldPoint));
		if (RiverDistance < 1500.0f)
		{
			const float RiverBlend = SmoothBlend(RiverDistance, 520.0f, 1500.0f);
			const float RiverSurface = FMath::GetMappedRangeValueClamped(FVector2D(66000.0f, -64000.0f), FVector2D(40.0f, -280.0f), WorldPoint.Y);
			Height = FMath::Lerp(RiverSurface - 190.0f, Height, RiverBlend);
		}

		const float NorthPathDistance = DistanceToPolyline(GetNorthBridgePathSpline(), WorldPoint);
		const float NorthPathBlend = SmoothBlend(NorthPathDistance, 4500.0f, 11000.0f);
		Height = FMath::Lerp(1100.0f, Height, NorthPathBlend);

		const float BossPathDistance = DistanceToPolyline(GetBossGatePathSpline(), WorldPoint);
		const float BossPathBlend = SmoothBlend(BossPathDistance, 3800.0f, 9000.0f);
		Height = FMath::Lerp(500.0f, Height, BossPathBlend);

		if (bInsidePlateau)
		{
			if (DistanceToEdge < 12000.0f)
			{
				const float EdgeBlend = SmoothBlend(DistanceToEdge, 1000.0f, 12000.0f);
				const float EdgeFloor = CanyonFloorZ + FMath::PerlinNoise2D(WorldPoint / 16000.0f) * 1400.0f;
				Height = FMath::Lerp(EdgeFloor, Height, EdgeBlend);
			}
			return Height;
		}

		return CanyonFloorZ + FMath::PerlinNoise2D(WorldPoint / 18000.0f) * 1200.0f;
	}

	void SetStaticMeshMaterial(AStaticMeshActor* Actor, UMaterialInterface* Material, const FVector& Scale, const FRotator& Rotation)
	{
		if (!Actor)
		{
			return;
		}

		Actor->SetActorScale3D(Scale);
		Actor->SetActorRotation(Rotation);
		if (UStaticMeshComponent* MeshComponent = Actor->GetStaticMeshComponent())
		{
			MeshComponent->SetMobility(EComponentMobility::Static);
			MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			if (Material)
			{
				MeshComponent->SetMaterial(0, Material);
			}
		}
	}

	void ClearExistingActors(UWorld* World)
	{
		if (!World)
		{
			return;
		}

		TArray<AActor*> ActorsToDestroy;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (!Actor
				|| Actor->IsA<AWorldSettings>()
				|| Actor->GetClass()->GetName().Contains(TEXT("Brush")))
			{
				continue;
			}
			ActorsToDestroy.Add(Actor);
		}

		for (AActor* Actor : ActorsToDestroy)
		{
			World->DestroyActor(Actor, true);
		}
	}

	UWorld* CreateOrLoadMap()
	{
		if (FPackageName::DoesPackageExist(MapAssetPath))
		{
			if (UWorld* ExistingWorld = UEditorLoadingAndSavingUtils::LoadMap(MapAssetPath))
			{
				ClearExistingActors(ExistingWorld);
				return ExistingWorld;
			}
		}

		UWorld* NewWorld = UEditorLoadingAndSavingUtils::NewBlankMap(false);
		if (NewWorld)
		{
			UEditorLoadingAndSavingUtils::SaveMap(NewWorld, MapAssetPath);
			ClearExistingActors(NewWorld);
		}
		return NewWorld;
	}

	void SpawnLandscape(UWorld* World, UMaterialInterface* LandscapeMaterial)
	{
		const int32 QuadsPerComponent = SectionsPerComponent * QuadsPerSection;
		const int32 SizeX = ComponentCountX * QuadsPerComponent + 1;
		const int32 SizeY = ComponentCountY * QuadsPerComponent + 1;

		TArray<uint16> HeightData;
		HeightData.SetNumZeroed(SizeX * SizeY);

		const FVector2D MapMin = GetMapMin();
		const FVector2D MapMax = GetMapMax();
		for (int32 Y = 0; Y < SizeY; ++Y)
		{
			const float WorldY = MapMin.Y + (static_cast<float>(Y) / (SizeY - 1)) * (MapMax.Y - MapMin.Y);
			for (int32 X = 0; X < SizeX; ++X)
			{
				const float WorldX = MapMin.X + (static_cast<float>(X) / (SizeX - 1)) * (MapMax.X - MapMin.X);
				const float HeightWorld = ComputeBaseTerrainHeight(FVector2D(WorldX, WorldY));
				HeightData[Y * SizeX + X] = EncodeLandscapeHeight(HeightWorld);
			}
		}

		TMap<FGuid, TArray<uint16>> HeightDataPerLayers;
		TMap<FGuid, TArray<FLandscapeImportLayerInfo>> MaterialLayerDataPerLayers;
		HeightDataPerLayers.Add(FGuid(), MoveTemp(HeightData));
		MaterialLayerDataPerLayers.Add(FGuid(), TArray<FLandscapeImportLayerInfo>());

		const FVector LandscapeOffset = MapCenter.X * FVector::ForwardVector + MapCenter.Y * FVector::RightVector
			+ FTransform(FRotator::ZeroRotator, FVector::ZeroVector, LandscapeScale)
				.TransformVector(FVector(-ComponentCountX * QuadsPerComponent / 2.0f, -ComponentCountY * QuadsPerComponent / 2.0f, 0.0f));

		ALandscape* Landscape = World->SpawnActor<ALandscape>(LandscapeOffset, FRotator::ZeroRotator);
		Landscape->LandscapeMaterial = LandscapeMaterial;
		Landscape->SetActorScale3D(LandscapeScale);
		Landscape->StaticLightingLOD = 0;
		Landscape->Import(FGuid::NewGuid(), 0, 0, SizeX - 1, SizeY - 1, SectionsPerComponent, QuadsPerSection, HeightDataPerLayers, TEXT(""), MaterialLayerDataPerLayers, ELandscapeImportAlphamapType::Additive, TArrayView<const FLandscapeLayer>());

		if (ULandscapeInfo* LandscapeInfo = Landscape->GetLandscapeInfo())
		{
			LandscapeInfo->UpdateLayerInfoMap(Landscape);
		}
		Landscape->MarkPackageDirty();
	}

	void SpawnLighting(UWorld* World)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		if (ADirectionalLight* Sun = World->SpawnActor<ADirectionalLight>(FVector(-70000.0f, 20000.0f, 22000.0f), FRotator(-38.0f, 35.0f, 0.0f), SpawnParameters))
		{
			if (UDirectionalLightComponent* LightComponent = Sun->GetComponent())
			{
				LightComponent->SetIntensity(10.0f);
				LightComponent->SetLightColor(FLinearColor(1.0f, 0.96f, 0.88f));
				LightComponent->bUseTemperature = false;
			}
		}

		if (ASkyLight* Sky = World->SpawnActor<ASkyLight>(FVector(0.0f, 15000.0f, 8000.0f), FRotator::ZeroRotator, SpawnParameters))
		{
			if (USkyLightComponent* SkyComponent = Sky->GetLightComponent())
			{
				SkyComponent->SourceType = ESkyLightSourceType::SLS_CapturedScene;
				SkyComponent->SetIntensity(1.2f);
				SkyComponent->bRealTimeCapture = false;
			}
		}

		World->SpawnActor<ASkyAtmosphere>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);

		if (AExponentialHeightFog* Fog = World->SpawnActor<AExponentialHeightFog>(FVector(0.0f, 15000.0f, -12000.0f), FRotator::ZeroRotator, SpawnParameters))
		{
			if (UExponentialHeightFogComponent* FogComponent = Fog->GetComponent())
			{
				FogComponent->FogDensity = 0.0011f;
				FogComponent->FogHeightFalloff = 0.18f;
				FogComponent->DirectionalInscatteringExponent = 6.0f;
			}
		}
	}

	void SpawnMarkerCube(UWorld* World, const FVector& Location, const FVector& Scale, UStaticMesh* CubeMesh, UMaterialInterface* MarkerMaterial, const FName Tag)
	{
		if (AStaticMeshActor* Marker = World->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator))
		{
			Marker->Tags.AddUnique(Tag);
			if (UStaticMeshComponent* MeshComponent = Marker->GetStaticMeshComponent())
			{
				MeshComponent->SetStaticMesh(CubeMesh);
				MeshComponent->SetMaterial(0, MarkerMaterial);
				MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				MeshComponent->SetMobility(EComponentMobility::Static);
			}
			Marker->SetActorScale3D(Scale);
		}
	}

	void SpawnFlatSegment(UWorld* World, UStaticMesh* CubeMesh, UMaterialInterface* Material, const FVector2D& Start, const FVector2D& End, float Width, float HeightOffset, const FName Tag)
	{
		const FVector2D Mid = (Start + End) * 0.5f;
		const FVector2D Direction = (End - Start).GetSafeNormal();
		const float SegmentLength = FVector2D::Distance(Start, End);
		const float GroundZ = ComputeBaseTerrainHeight(Mid);
		const FRotator Rotation(0.0f, FMath::RadiansToDegrees(FMath::Atan2(Direction.Y, Direction.X)), 0.0f);
		if (AStaticMeshActor* Segment = World->SpawnActor<AStaticMeshActor>(FVector(Mid.X, Mid.Y, GroundZ + HeightOffset), Rotation))
		{
			Segment->Tags.AddUnique(Tag);
			SetStaticMeshMaterial(Segment, Material, FVector(SegmentLength / 100.0f, Width / 100.0f, 0.045f), Rotation);
			if (UStaticMeshComponent* MeshComponent = Segment->GetStaticMeshComponent())
			{
				MeshComponent->SetStaticMesh(CubeMesh);
				MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				MeshComponent->SetCastShadow(false);
			}
		}
	}

	void SpawnPolylineSegments(UWorld* World, UStaticMesh* CubeMesh, UMaterialInterface* Material, const TArray<FVector2D>& Points, float Width, float HeightOffset, const FName Tag)
	{
		for (int32 Index = 0; Index + 1 < Points.Num(); ++Index)
		{
			SpawnFlatSegment(World, CubeMesh, Material, Points[Index], Points[Index + 1], Width, HeightOffset, Tag);
		}
	}

	void SpawnWater(UWorld* World, UStaticMesh* CubeMesh, UMaterialInterface* WaterMaterial)
	{
		SpawnPolylineSegments(World, CubeMesh, WaterMaterial, GetRiverSpline(), 1900.0f, 90.0f, TEXT("FFStarterRiver"));
		SpawnPolylineSegments(World, CubeMesh, WaterMaterial, GetRiverBranchSpline(), 1250.0f, 85.0f, TEXT("FFStarterRiverBranch"));

		if (AStaticMeshActor* Lake = World->SpawnActor<AStaticMeshActor>(FVector(LakeCenter.X, LakeCenter.Y, LakeWaterZ), FRotator::ZeroRotator))
		{
			SetStaticMeshMaterial(Lake, WaterMaterial, FVector(LakeRadii.X / 50.0f, LakeRadii.Y / 50.0f, 0.08f), FRotator::ZeroRotator);
			if (UStaticMeshComponent* MeshComponent = Lake->GetStaticMeshComponent())
			{
				MeshComponent->SetStaticMesh(CubeMesh);
				MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				MeshComponent->SetCastShadow(false);
			}
		}

		if (AStaticMeshActor* Waterfall = World->SpawnActor<AStaticMeshActor>(FVector(-70000.0f, -65000.0f, -7000.0f), FRotator(90.0f, -74.0f, 0.0f)))
		{
			SetStaticMeshMaterial(Waterfall, WaterMaterial, FVector(16.0f, 1.4f, 0.08f), FRotator(90.0f, -74.0f, 0.0f));
			if (UStaticMeshComponent* MeshComponent = Waterfall->GetStaticMeshComponent())
			{
				MeshComponent->SetStaticMesh(CubeMesh);
				MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				MeshComponent->SetCastShadow(false);
			}
		}
	}

	void SpawnCanyonFloorMask(UWorld* World, UStaticMesh* CubeMesh, UMaterialInterface* CanyonFloorMaterial)
	{
		const TArray<FVector2D> PlateauPolygon = GetPlateauPolygonWorld();
		const FVector2D MapMin = GetMapMin();
		const FVector2D MapMax = GetMapMax();
		const float TileSize = 6000.0f;

		for (float Y = MapMin.Y + TileSize * 0.5f; Y <= MapMax.Y; Y += TileSize)
		{
			for (float X = MapMin.X + TileSize * 0.5f; X <= MapMax.X; X += TileSize)
			{
				const FVector2D TileCenter(X, Y);
				const float HalfTile = TileSize * 0.5f;
				const bool bTouchesPlateau =
					IsPointInsidePolygon(PlateauPolygon, TileCenter)
					|| IsPointInsidePolygon(PlateauPolygon, TileCenter + FVector2D(-HalfTile, -HalfTile))
					|| IsPointInsidePolygon(PlateauPolygon, TileCenter + FVector2D(HalfTile, -HalfTile))
					|| IsPointInsidePolygon(PlateauPolygon, TileCenter + FVector2D(-HalfTile, HalfTile))
					|| IsPointInsidePolygon(PlateauPolygon, TileCenter + FVector2D(HalfTile, HalfTile));
				if (bTouchesPlateau)
				{
					continue;
				}

				if (AStaticMeshActor* CanyonTile = World->SpawnActor<AStaticMeshActor>(FVector(X, Y, CanyonFloorZ + 420.0f), FRotator::ZeroRotator))
				{
					SetStaticMeshMaterial(CanyonTile, CanyonFloorMaterial, FVector(TileSize / 100.0f, TileSize / 100.0f, 0.035f), FRotator::ZeroRotator);
					if (UStaticMeshComponent* MeshComponent = CanyonTile->GetStaticMeshComponent())
					{
						MeshComponent->SetStaticMesh(CubeMesh);
						MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
						MeshComponent->SetCastShadow(false);
					}
				}
			}
		}
	}

	void SpawnForestMass(UWorld* World, UStaticMesh* CylinderMesh, UMaterialInterface* ForestMassMaterial)
	{
		const TArray<TPair<FVector2D, FVector2D>> Patches = {
			TPair<FVector2D, FVector2D>(FVector2D(-52000.0f, 57500.0f), FVector2D(26000.0f, 11500.0f)),
			TPair<FVector2D, FVector2D>(FVector2D(-18000.0f, 67500.0f), FVector2D(34000.0f, 14500.0f)),
			TPair<FVector2D, FVector2D>(FVector2D(22000.0f, 60500.0f), FVector2D(30000.0f, 12000.0f)),
			TPair<FVector2D, FVector2D>(FVector2D(-70000.0f, 43000.0f), FVector2D(17000.0f, 8600.0f)),
			TPair<FVector2D, FVector2D>(FVector2D(47000.0f, 44000.0f), FVector2D(16000.0f, 7800.0f))
		};

		for (const TPair<FVector2D, FVector2D>& Patch : Patches)
		{
			const FVector2D Center = Patch.Key;
			if (!IsPointInsidePolygon(GetPlateauPolygonWorld(), Center))
			{
				continue;
			}
			if (AStaticMeshActor* ForestPatch = World->SpawnActor<AStaticMeshActor>(FVector(Center.X, Center.Y, ComputeBaseTerrainHeight(Center) + 26.0f), FRotator::ZeroRotator))
			{
				SetStaticMeshMaterial(ForestPatch, ForestMassMaterial, FVector(Patch.Value.X / 50.0f, Patch.Value.Y / 50.0f, 0.045f), FRotator::ZeroRotator);
				if (UStaticMeshComponent* MeshComponent = ForestPatch->GetStaticMeshComponent())
				{
					MeshComponent->SetStaticMesh(CylinderMesh);
					MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
					MeshComponent->SetCastShadow(false);
				}
			}
		}
	}

	void SpawnForest(UWorld* World, UStaticMesh* CylinderMesh, UStaticMesh* ConeMesh, UMaterialInterface* TrunkMaterial, UMaterialInterface* CanopyMaterial)
	{
		FRandomStream Random(44321);
		for (int32 Index = 0; Index < 520; ++Index)
		{
			const float X = Random.FRandRange(-98000.0f, 85000.0f);
			const float Y = Random.FRandRange(36000.0f, 90000.0f);
			const FVector2D Point(X, Y);
			if (!IsPointInsidePolygon(GetPlateauPolygonWorld(), Point))
			{
				continue;
			}
			const FVector2D LakeDelta = Point - LakeCenter;
			const float LakeMask = FMath::Sqrt(FMath::Square(LakeDelta.X / (LakeRadii.X + 2600.0f)) + FMath::Square(LakeDelta.Y / (LakeRadii.Y + 2600.0f)));
			if (LakeMask < 1.0f)
			{
				continue;
			}
			if (DistanceToPolyline(GetRiverSpline(), Point) < 2600.0f)
			{
				continue;
			}

			const float GroundZ = ComputeBaseTerrainHeight(Point);
			const float TreeScale = Random.FRandRange(1.2f, 2.2f);
			const FVector BaseLocation(X, Y, GroundZ + 90.0f);
			if (AStaticMeshActor* Trunk = World->SpawnActor<AStaticMeshActor>(BaseLocation, FRotator::ZeroRotator))
			{
				SetStaticMeshMaterial(Trunk, TrunkMaterial, FVector(0.22f * TreeScale, 0.22f * TreeScale, 2.4f * TreeScale), FRotator::ZeroRotator);
				Trunk->GetStaticMeshComponent()->SetStaticMesh(CylinderMesh);
				Trunk->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}

			if (AStaticMeshActor* Canopy = World->SpawnActor<AStaticMeshActor>(BaseLocation + FVector(0.0f, 0.0f, 440.0f * TreeScale), FRotator::ZeroRotator))
			{
				SetStaticMeshMaterial(Canopy, CanopyMaterial, FVector(1.8f * TreeScale, 1.8f * TreeScale, 3.6f * TreeScale), FRotator::ZeroRotator);
				Canopy->GetStaticMeshComponent()->SetStaticMesh(ConeMesh);
				Canopy->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
		}
	}

	void SpawnPlaceholderLandmarks(UWorld* World, UStaticMesh* CubeMesh, UStaticMesh* CylinderMesh, UMaterialInterface* MarkerMaterial)
	{
		if (AStaticMeshActor* VillageReserve = World->SpawnActor<AStaticMeshActor>(FVector(VillageHint.X, VillageHint.Y, ComputeBaseTerrainHeight(FVector2D(VillageHint.X, VillageHint.Y)) + 38.0f), FRotator::ZeroRotator))
		{
			VillageReserve->Tags.AddUnique(VillageMarkerTag);
			SetStaticMeshMaterial(VillageReserve, MarkerMaterial, FVector(220.0f, 170.0f, 0.03f), FRotator::ZeroRotator);
			if (UStaticMeshComponent* MeshComponent = VillageReserve->GetStaticMeshComponent())
			{
				MeshComponent->SetStaticMesh(CylinderMesh);
				MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				MeshComponent->SetCastShadow(false);
			}
		}

		for (const FVector2D Offset : {
			FVector2D(-9500.0f, -6500.0f),
			FVector2D(9500.0f, -6500.0f),
			FVector2D(11500.0f, 5200.0f),
			FVector2D(-11500.0f, 5200.0f),
			FVector2D(0.0f, 10500.0f),
			FVector2D(0.0f, -10500.0f) })
		{
			const FVector MarkerLocation = FVector(VillageHint.X + Offset.X, VillageHint.Y + Offset.Y, ComputeBaseTerrainHeight(FVector2D(VillageHint.X + Offset.X, VillageHint.Y + Offset.Y)) + 120.0f);
			if (AStaticMeshActor* Marker = World->SpawnActor<AStaticMeshActor>(MarkerLocation, FRotator::ZeroRotator))
			{
				Marker->Tags.AddUnique(VillageMarkerTag);
				SetStaticMeshMaterial(Marker, MarkerMaterial, FVector(0.8f, 0.8f, 1.2f), FRotator::ZeroRotator);
				Marker->GetStaticMeshComponent()->SetStaticMesh(CylinderMesh);
				Marker->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
		}

		SpawnMarkerCube(World, FVector(BridgeHint.X - 1400.0f, BridgeHint.Y, ComputeBaseTerrainHeight(FVector2D(BridgeHint.X - 1400.0f, BridgeHint.Y)) + 240.0f), FVector(0.9f, 0.9f, 2.0f), CubeMesh, MarkerMaterial, BridgeMarkerTag);
		SpawnMarkerCube(World, FVector(BridgeHint.X + 1400.0f, BridgeHint.Y, ComputeBaseTerrainHeight(FVector2D(BridgeHint.X + 1400.0f, BridgeHint.Y)) + 240.0f), FVector(0.9f, 0.9f, 2.0f), CubeMesh, MarkerMaterial, BridgeMarkerTag);
		SpawnMarkerCube(World, FVector(BridgeHint.X, BridgeHint.Y, ComputeBaseTerrainHeight(FVector2D(BridgeHint.X, BridgeHint.Y)) + 60.0f), FVector(8.0f, 1.2f, 0.16f), CubeMesh, MarkerMaterial, BridgeMarkerTag);

		SpawnMarkerCube(World, FVector(BossGateHint.X, BossGateHint.Y, ComputeBaseTerrainHeight(FVector2D(BossGateHint.X, BossGateHint.Y)) + 280.0f), FVector(1.2f, 1.2f, 2.8f), CubeMesh, MarkerMaterial, BossGateMarkerTag);
	}

	void SpawnSupportActors(UWorld* World, UMaterialInterface* WaterMaterial, UMaterialInterface* MarkerMaterial, UMaterialInterface* PathMaterial, UMaterialInterface* ForestMassMaterial, UMaterialInterface* CanyonFloorMaterial, UMaterialInterface* TrunkMaterial, UMaterialInterface* CanopyMaterial)
	{
		UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
		UStaticMesh* CylinderMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
		UStaticMesh* ConeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cone.Cone"));

		if (APlayerStart* PlayerStart = World->SpawnActor<APlayerStart>(SpawnHint, FRotator(0.0f, 22.0f, 0.0f)))
		{
			PlayerStart->SetActorLocation(FVector(SpawnHint.X, SpawnHint.Y, ComputeBaseTerrainHeight(FVector2D(SpawnHint.X, SpawnHint.Y)) + 120.0f));
		}

		if (ACameraActor* TopDownCamera = World->SpawnActor<ACameraActor>(FVector(MapCenter.X, MapCenter.Y, 64000.0f), FRotator(-90.0f, 90.0f, 0.0f)))
		{
			TopDownCamera->Tags.AddUnique(TopDownCameraTag);
			if (UCameraComponent* CameraComponent = TopDownCamera->GetCameraComponent())
			{
				CameraComponent->ProjectionMode = ECameraProjectionMode::Orthographic;
				CameraComponent->OrthoWidth = 300000.0f;
			}
		}

		SpawnCanyonFloorMask(World, CubeMesh, CanyonFloorMaterial);
		SpawnWater(World, CubeMesh, WaterMaterial);
		SpawnPolylineSegments(World, CubeMesh, PathMaterial, GetBossGatePathSpline(), 900.0f, 80.0f, TEXT("FFStarterBossGatePath"));
		SpawnPolylineSegments(World, CubeMesh, PathMaterial, GetNorthBridgePathSpline(), 850.0f, 80.0f, TEXT("FFStarterNorthExitPath"));
		SpawnForestMass(World, CylinderMesh, ForestMassMaterial);
		SpawnForest(World, CylinderMesh, ConeMesh, TrunkMaterial, CanopyMaterial);
		SpawnPlaceholderLandmarks(World, CubeMesh, CylinderMesh, MarkerMaterial);
	}
}
#endif

int32 UFFStarterHighlandBlockoutCommandlet::Main(const FString& Params)
{
#if WITH_EDITOR
	UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandBlockoutCommandlet: generating %s"), *MapAssetPath);

	UMaterial* LandscapeMaterial = BuildLandscapeMaterial();
	UMaterial* WaterMaterial = BuildSolidMaterial(WaterMaterialName, FLinearColor(0.10f, 0.42f, 0.85f), 0.06f);
	UMaterial* MarkerMaterial = BuildSolidMaterial(MarkerMaterialName, FLinearColor(0.92f, 0.72f, 0.22f), 0.55f);
	UMaterial* PathMaterial = BuildSolidMaterial(PathMaterialName, FLinearColor(0.54f, 0.42f, 0.24f), 0.88f);
	UMaterial* ForestMassMaterial = BuildSolidMaterial(ForestMassMaterialName, FLinearColor(0.03f, 0.18f, 0.05f), 0.90f);
	UMaterial* CanyonFloorMaterial = BuildSolidMaterial(CanyonFloorMaterialName, FLinearColor(0.07f, 0.09f, 0.10f), 0.94f);
	UMaterial* TrunkMaterial = BuildSolidMaterial(TreeTrunkMaterialName, FLinearColor(0.30f, 0.18f, 0.08f), 0.82f);
	UMaterial* CanopyMaterial = BuildSolidMaterial(TreeCanopyMaterialName, FLinearColor(0.03f, 0.14f, 0.05f), 0.86f);

	if (!LandscapeMaterial || !WaterMaterial || !MarkerMaterial || !PathMaterial || !ForestMassMaterial || !CanyonFloorMaterial || !TrunkMaterial || !CanopyMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandBlockoutCommandlet: failed to create one or more blockout materials."));
		return 1;
	}

	UWorld* World = CreateOrLoadMap();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandBlockoutCommandlet: failed to create or load the blockout map."));
		return 1;
	}

	SpawnLandscape(World, LandscapeMaterial);
	SpawnLighting(World);
	SpawnSupportActors(World, WaterMaterial, MarkerMaterial, PathMaterial, ForestMassMaterial, CanyonFloorMaterial, TrunkMaterial, CanopyMaterial);

	World->MarkPackageDirty();
	const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, MapAssetPath);
	const bool bSavedAssets = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);

		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandBlockoutCommandlet: spawn=(%.1f, %.1f, %.1f) boundsMin=(%.1f, %.1f) boundsMax=(%.1f, %.1f)"),
			SpawnHint.X, SpawnHint.Y, ComputeBaseTerrainHeight(FVector2D(SpawnHint.X, SpawnHint.Y)) + 120.0f,
		GetMapMin().X, GetMapMin().Y,
		GetMapMax().X, GetMapMax().Y);

	return (bSavedMap && bSavedAssets) ? 0 : 1;
#else
	UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandBlockoutCommandlet can only run in editor builds."));
	return 1;
#endif
}
