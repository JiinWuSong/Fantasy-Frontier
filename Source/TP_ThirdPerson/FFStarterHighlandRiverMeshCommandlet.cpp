#include "FFStarterHighlandRiverMeshCommandlet.h"

#if WITH_EDITOR
#include "CollisionQueryParams.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/HitResult.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "FileHelpers.h"
#include "LandscapeProxy.h"
#include "Materials/MaterialInterface.h"
#include "WaterBodyActor.h"
#include "WaterZoneActor.h"
#endif

UFFStarterHighlandRiverMeshCommandlet::UFFStarterHighlandRiverMeshCommandlet()
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
	const TCHAR* VisibleRiverMaterialPath = TEXT("/Game/FantasyFrontier/Blockout/Materials/M_FF_River_Visible.M_FF_River_Visible");
	const TCHAR* PlaneMeshPath = TEXT("/Engine/BasicShapes/Plane.Plane");

	bool TextContainsAny(const FString& Text, std::initializer_list<const TCHAR*> Needles)
	{
		for (const TCHAR* Needle : Needles)
		{
			if (Text.Contains(Needle, ESearchCase::IgnoreCase))
			{
				return true;
			}
		}
		return false;
	}

	bool FindLandscapeBounds(UWorld* World, FBox& OutBounds)
	{
		OutBounds.Init();
		for (TActorIterator<ALandscapeProxy> It(World); It; ++It)
		{
			ALandscapeProxy* Landscape = *It;
			if (!Landscape || Landscape->IsEditorOnly())
			{
				continue;
			}

			const FBox Bounds = Landscape->GetComponentsBoundingBox(true);
			if (Bounds.IsValid)
			{
				OutBounds += Bounds;
			}
		}

		return OutBounds.IsValid != 0;
	}

	bool IsInsideLandscape2D(const FBox& LandscapeBounds, const FVector2D& Point)
	{
		return Point.X >= LandscapeBounds.Min.X && Point.X <= LandscapeBounds.Max.X
			&& Point.Y >= LandscapeBounds.Min.Y && Point.Y <= LandscapeBounds.Max.Y;
	}

	bool SampleLandscapeZ(UWorld* World, const FBox& LandscapeBounds, const float X, const float Y, float& OutZ)
	{
		if (!World)
		{
			return false;
		}

		TArray<FHitResult> Hits;
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(FFStarterHighlandRiverMeshTrace), true);
		QueryParams.bTraceComplex = true;

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (!Actor)
			{
				continue;
			}

			if (TextContainsAny(Actor->GetActorLabel(), { TEXT("FF_RiverMesh_Main_"), TEXT("FF_Water_Lake_"), TEXT("FF_WaterZone_Highland"), TEXT("FF_InnerWater_") }) ||
				TextContainsAny(Actor->GetName(), { TEXT("FF_RiverMesh_Main_"), TEXT("FF_Water_Lake_"), TEXT("FF_WaterZone_Highland"), TEXT("FF_InnerWater_") }) ||
				Actor->IsA<AWaterBody>() || Actor->IsA<AWaterZone>())
			{
				QueryParams.AddIgnoredActor(Actor);
			}
		}

		const FVector Start(X, Y, LandscapeBounds.Max.Z + 50000.0f);
		const FVector End(X, Y, LandscapeBounds.Min.Z - 50000.0f);
		if (!World->LineTraceMultiByChannel(Hits, Start, End, ECC_WorldStatic, QueryParams))
		{
			return false;
		}

		for (const FHitResult& Hit : Hits)
		{
			if (Cast<ALandscapeProxy>(Hit.GetActor()))
			{
				OutZ = Hit.ImpactPoint.Z;
				return true;
			}
		}

		return false;
	}

	int32 DeleteWaterActors(UWorld* World)
	{
		TArray<AActor*> ActorsToDelete;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (!Actor)
			{
				continue;
			}

			const bool bWaterCleanupActor =
				TextContainsAny(Actor->GetName(), { TEXT("FF_RiverMesh_Main_"), TEXT("FF_Water_Lake_"), TEXT("FF_WaterZone_Highland"), TEXT("FF_InnerWater_") }) ||
				TextContainsAny(Actor->GetActorLabel(), { TEXT("FF_RiverMesh_Main_"), TEXT("FF_Water_Lake_"), TEXT("FF_WaterZone_Highland"), TEXT("FF_InnerWater_") });
			if (bWaterCleanupActor)
			{
				ActorsToDelete.Add(Actor);
			}
		}

		for (AActor* Actor : ActorsToDelete)
		{
			UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandRiverMesh: deleting actor name=%s label=%s class=%s"),
				*Actor->GetName(),
				*Actor->GetActorLabel(),
				Actor->GetClass() ? *Actor->GetClass()->GetName() : TEXT("None"));
			World->DestroyActor(Actor, false, false);
		}

		return ActorsToDelete.Num();
	}

	bool FindMarkerLocation(UWorld* World, const FString& MarkerLabel, FVector& OutLocation)
	{
		if (!World)
		{
			return false;
		}

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (!Actor)
			{
				continue;
			}

			if (Actor->GetActorLabel().Equals(MarkerLabel, ESearchCase::IgnoreCase) ||
				Actor->GetName().Equals(MarkerLabel, ESearchCase::IgnoreCase))
			{
				OutLocation = Actor->GetActorLocation();
				return true;
			}
		}

		return false;
	}

	bool ScanBankRise(
		UWorld* World,
		const FBox& LandscapeBounds,
		const FVector2D& Center,
		const FVector2D& Direction,
		const float CenterFloorZ,
		const float MaxDistance,
		const float StepDistance,
		const float RiseThreshold,
		float& OutBankDistance,
		float& OutBankHeight)
	{
		OutBankDistance = 0.0f;
		OutBankHeight = CenterFloorZ;

		const FVector2D UnitDirection = Direction.GetSafeNormal();
		if (UnitDirection.IsNearlyZero())
		{
			return false;
		}

		float LastValidDistance = 0.0f;
		float LastValidHeight = CenterFloorZ;
		for (float Distance = StepDistance; Distance <= MaxDistance; Distance += StepDistance)
		{
			const FVector2D SamplePoint = Center + (UnitDirection * Distance);
			if (!IsInsideLandscape2D(LandscapeBounds, SamplePoint))
			{
				break;
			}

			float SampleZ = 0.0f;
			if (!SampleLandscapeZ(World, LandscapeBounds, SamplePoint.X, SamplePoint.Y, SampleZ))
			{
				break;
			}

			LastValidDistance = Distance;
			LastValidHeight = SampleZ;
			if ((SampleZ - CenterFloorZ) >= RiseThreshold)
			{
				OutBankDistance = FMath::Max(StepDistance, Distance - (StepDistance * 0.5f));
				OutBankHeight = SampleZ;
				return true;
			}
		}

		if (LastValidDistance > 0.0f && (LastValidHeight - CenterFloorZ) >= (RiseThreshold * 0.75f))
		{
			OutBankDistance = LastValidDistance;
			OutBankHeight = LastValidHeight;
			return true;
		}

		return false;
	}

	struct FResolvedRiverPatch
	{
		FVector Center = FVector::ZeroVector;
		float FullWidth = 0.0f;
	};

	bool ResolveRiverPatchAtAnchor(
		UWorld* World,
		const FBox& LandscapeBounds,
		const FVector2D& GuessPoint,
		const FVector2D& ForwardHint,
		FResolvedRiverPatch& OutPatch)
	{
		OutPatch = FResolvedRiverPatch();

		const FVector2D Forward = ForwardHint.GetSafeNormal();
		if (!World || Forward.IsNearlyZero() || !IsInsideLandscape2D(LandscapeBounds, GuessPoint))
		{
			return false;
		}

		const FVector2D Perpendicular(-Forward.Y, Forward.X);
		const float AlongOffsets[] = { -600.0f, 0.0f, 600.0f };
		const float AcrossOffsets[] = { -1200.0f, -600.0f, 0.0f, 600.0f, 1200.0f };
		const float MaxBankSearchDistance = 3600.0f;
		const float StepDistance = 180.0f;
		const float RiseThreshold = 135.0f;
		const float MinimumChannelWidth = 900.0f;

		bool bFoundPatch = false;
		float BestScore = -TNumericLimits<float>::Max();

		for (float AlongOffset : AlongOffsets)
		{
			for (float AcrossOffset : AcrossOffsets)
			{
				const FVector2D Candidate = GuessPoint + (Forward * AlongOffset) + (Perpendicular * AcrossOffset);
				if (!IsInsideLandscape2D(LandscapeBounds, Candidate))
				{
					continue;
				}

				float CenterFloorZ = 0.0f;
				if (!SampleLandscapeZ(World, LandscapeBounds, Candidate.X, Candidate.Y, CenterFloorZ))
				{
					continue;
				}

				float LeftBankDistance = 0.0f;
				float LeftBankHeight = 0.0f;
				float RightBankDistance = 0.0f;
				float RightBankHeight = 0.0f;
				if (!ScanBankRise(World, LandscapeBounds, Candidate, -Perpendicular, CenterFloorZ, MaxBankSearchDistance, StepDistance, RiseThreshold, LeftBankDistance, LeftBankHeight) ||
					!ScanBankRise(World, LandscapeBounds, Candidate, Perpendicular, CenterFloorZ, MaxBankSearchDistance, StepDistance, RiseThreshold, RightBankDistance, RightBankHeight))
				{
					continue;
				}

				const float FullWidth = LeftBankDistance + RightBankDistance;
				if (FullWidth < MinimumChannelWidth)
				{
					continue;
				}

				const FVector2D CenterAdjustment = Perpendicular * ((RightBankDistance - LeftBankDistance) * 0.5f);
				const FVector2D AdjustedCenter2D = Candidate + CenterAdjustment;
				const float SurfaceZ = FMath::Clamp(
					FMath::Min(LeftBankHeight, RightBankHeight) - 26.0f,
					CenterFloorZ + 60.0f,
					CenterFloorZ + 145.0f);
				const float BalancePenalty = FMath::Abs(RightBankDistance - LeftBankDistance) * 0.45f;
				const float MarkerDistancePenalty = FVector2D::Distance(AdjustedCenter2D, GuessPoint) * 0.30f;
				const float Score = FullWidth - BalancePenalty - MarkerDistancePenalty;
				if (!bFoundPatch || Score > BestScore)
				{
					OutPatch.Center = FVector(AdjustedCenter2D.X, AdjustedCenter2D.Y, SurfaceZ);
					OutPatch.FullWidth = FullWidth * 0.9f;
					BestScore = Score;
					bFoundPatch = true;
				}
			}
		}

		return bFoundPatch;
	}

	bool BuildMarkerDrivenRiverPatches(UWorld* World, const FBox& LandscapeBounds, TArray<FResolvedRiverPatch>& OutPatches)
	{
		OutPatches.Reset();

		const TArray<FString> RiverMarkerLabels = {
			TEXT("WATER_RIVER_01"),
			TEXT("WATER_RIVER_02"),
			TEXT("WATER_RIVER_03"),
			TEXT("WATER_RIVER_04"),
			TEXT("WATER_RIVER_05"),
			TEXT("WATER_RIVER_06")
		};

		TArray<FVector> RiverMarkers;
		for (const FString& MarkerLabel : RiverMarkerLabels)
		{
			FVector MarkerLocation = FVector::ZeroVector;
			if (!FindMarkerLocation(World, MarkerLabel, MarkerLocation))
			{
				UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandRiverMesh: missing river marker %s; refusing to guess."), *MarkerLabel);
				return false;
			}

			if (!IsInsideLandscape2D(LandscapeBounds, FVector2D(MarkerLocation.X, MarkerLocation.Y)))
			{
				UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandRiverMesh: river marker %s is outside terrain bounds; refusing to guess."), *MarkerLabel);
				return false;
			}

			RiverMarkers.Add(MarkerLocation);
		}

		for (int32 MarkerIndex = 0; MarkerIndex + 1 < RiverMarkers.Num(); ++MarkerIndex)
		{
			const FVector2D MarkerStart(RiverMarkers[MarkerIndex].X, RiverMarkers[MarkerIndex].Y);
			const FVector2D MarkerEnd(RiverMarkers[MarkerIndex + 1].X, RiverMarkers[MarkerIndex + 1].Y);
			const FVector2D SegmentDelta = MarkerEnd - MarkerStart;
			const float SegmentLength = SegmentDelta.Size();
			if (SegmentLength <= KINDA_SMALL_NUMBER)
			{
				continue;
			}

			const FVector2D SegmentForward = SegmentDelta / SegmentLength;
			const int32 SegmentSamples = FMath::Max(2, FMath::CeilToInt(SegmentLength / 9000.0f) + 1);
			for (int32 SampleIndex = 0; SampleIndex < SegmentSamples; ++SampleIndex)
			{
				if (MarkerIndex > 0 && SampleIndex == 0)
				{
					continue;
				}

				const float Alpha = SegmentSamples == 1 ? 0.0f : static_cast<float>(SampleIndex) / static_cast<float>(SegmentSamples - 1);
				const FVector2D GuessPoint = FMath::Lerp(MarkerStart, MarkerEnd, Alpha);

				FResolvedRiverPatch ResolvedPatch;
				if (!ResolveRiverPatchAtAnchor(World, LandscapeBounds, GuessPoint, SegmentForward, ResolvedPatch))
				{
					UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandRiverMesh: failed to resolve carved river trench near markers %s -> %s at alpha=%.2f; refusing to guess."),
						*RiverMarkerLabels[MarkerIndex],
						*RiverMarkerLabels[MarkerIndex + 1],
						Alpha);
					return false;
				}

				if (OutPatches.Num() > 0 && FVector::Dist2D(OutPatches.Last().Center, ResolvedPatch.Center) < 900.0f)
				{
					continue;
				}

				UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandRiverMesh: resolved trench sample pair=%s->%s alpha=%.2f center=(%.1f, %.1f, %.1f) width=%.1f"),
					*RiverMarkerLabels[MarkerIndex],
					*RiverMarkerLabels[MarkerIndex + 1],
					Alpha,
					ResolvedPatch.Center.X,
					ResolvedPatch.Center.Y,
					ResolvedPatch.Center.Z,
					ResolvedPatch.FullWidth);
				OutPatches.Add(ResolvedPatch);
			}
		}

		return OutPatches.Num() >= 2;
	}

	AStaticMeshActor* SpawnWaterPlane(UWorld* World, UStaticMesh* PlaneMesh, UMaterialInterface* WaterMaterial, const FString& Label, const FVector& Location, const FRotator& Rotation, const FVector& Scale)
	{
		if (!World || !PlaneMesh || !WaterMaterial)
		{
			return nullptr;
		}

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Name = MakeUniqueObjectName(World->PersistentLevel, AStaticMeshActor::StaticClass(), *Label);
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AStaticMeshActor* Actor = World->SpawnActor<AStaticMeshActor>(Location, Rotation, SpawnParameters);
		if (!Actor)
		{
			return nullptr;
		}

		Actor->SetActorLabel(Label);
		Actor->Modify();
		Actor->SetActorScale3D(Scale);

		if (UStaticMeshComponent* MeshComponent = Actor->GetStaticMeshComponent())
		{
			MeshComponent->Modify();
			MeshComponent->SetStaticMesh(PlaneMesh);
			MeshComponent->SetMaterial(0, WaterMaterial);
			MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			MeshComponent->SetCastShadow(false);
			MeshComponent->SetMobility(EComponentMobility::Static);
			MeshComponent->MarkPackageDirty();
		}

		Actor->MarkPackageDirty();
		return Actor;
	}

	int32 SpawnRiverMeshSegments(UWorld* World, UStaticMesh* PlaneMesh, UMaterialInterface* WaterMaterial, const TArray<FResolvedRiverPatch>& RiverPatches)
	{
		if (!World || !PlaneMesh || !WaterMaterial || RiverPatches.Num() < 2)
		{
			return 0;
		}

		int32 CreatedSegments = 0;
		for (int32 Index = 0; Index + 1 < RiverPatches.Num(); ++Index)
		{
			const FVector Start = RiverPatches[Index].Center;
			const FVector End = RiverPatches[Index + 1].Center;
			const FVector Delta = End - Start;
			const float SegmentLength = FVector2D(Delta.X, Delta.Y).Size();
			if (SegmentLength <= KINDA_SMALL_NUMBER)
			{
				continue;
			}

			const FVector Midpoint = (Start + End) * 0.5f;
			const float Yaw = FMath::RadiansToDegrees(FMath::Atan2(Delta.Y, Delta.X));
			const float SegmentWidth = FMath::Max(800.0f, (RiverPatches[Index].FullWidth + RiverPatches[Index + 1].FullWidth) * 0.5f);
			const FString Label = FString::Printf(TEXT("FF_InnerWater_River_%02d"), Index + 1);
			AStaticMeshActor* Segment = SpawnWaterPlane(
				World,
				PlaneMesh,
				WaterMaterial,
				Label,
				Midpoint,
				FRotator(0.0f, Yaw, 0.0f),
				FVector((SegmentLength * 1.08f) / 100.0f, SegmentWidth / 100.0f, 1.0f));
			if (Segment)
			{
				++CreatedSegments;
			}
		}

		return CreatedSegments;
	}

	struct FResolvedBasinPlane
	{
		FVector Center = FVector::ZeroVector;
		float SizeX = 0.0f;
		float SizeY = 0.0f;
		float Yaw = 0.0f;
	};

	bool ResolveRoundBasinFromMarker(
		UWorld* World,
		const FBox& LandscapeBounds,
		const FVector& MarkerLocation,
		const float SearchRadius,
		const float MaxRadius,
		const float RiseThreshold,
		FResolvedBasinPlane& OutPlane)
	{
		OutPlane = FResolvedBasinPlane();
		if (!World || !IsInsideLandscape2D(LandscapeBounds, FVector2D(MarkerLocation.X, MarkerLocation.Y)))
		{
			return false;
		}

		const FVector2D CardinalDirections[] = {
			FVector2D(1.0f, 0.0f),
			FVector2D(-1.0f, 0.0f),
			FVector2D(0.0f, 1.0f),
			FVector2D(0.0f, -1.0f)
		};
		const float CandidateOffsets[] = { -SearchRadius, 0.0f, SearchRadius };
		const float StepDistance = 180.0f;
		const float MinimumDiameter = 1600.0f;

		bool bFoundPlane = false;
		float BestScore = -TNumericLimits<float>::Max();

		for (float OffsetX : CandidateOffsets)
		{
			for (float OffsetY : CandidateOffsets)
			{
				const FVector2D Candidate(MarkerLocation.X + OffsetX, MarkerLocation.Y + OffsetY);
				if (!IsInsideLandscape2D(LandscapeBounds, Candidate))
				{
					continue;
				}

				float CenterFloorZ = 0.0f;
				if (!SampleLandscapeZ(World, LandscapeBounds, Candidate.X, Candidate.Y, CenterFloorZ))
				{
					continue;
				}

				float EastDistance = 0.0f;
				float EastHeight = 0.0f;
				float WestDistance = 0.0f;
				float WestHeight = 0.0f;
				float NorthDistance = 0.0f;
				float NorthHeight = 0.0f;
				float SouthDistance = 0.0f;
				float SouthHeight = 0.0f;

				if (!ScanBankRise(World, LandscapeBounds, Candidate, CardinalDirections[0], CenterFloorZ, MaxRadius, StepDistance, RiseThreshold, EastDistance, EastHeight) ||
					!ScanBankRise(World, LandscapeBounds, Candidate, CardinalDirections[1], CenterFloorZ, MaxRadius, StepDistance, RiseThreshold, WestDistance, WestHeight) ||
					!ScanBankRise(World, LandscapeBounds, Candidate, CardinalDirections[2], CenterFloorZ, MaxRadius, StepDistance, RiseThreshold, NorthDistance, NorthHeight) ||
					!ScanBankRise(World, LandscapeBounds, Candidate, CardinalDirections[3], CenterFloorZ, MaxRadius, StepDistance, RiseThreshold, SouthDistance, SouthHeight))
				{
					continue;
				}

				const float SizeX = (EastDistance + WestDistance) * 0.9f;
				const float SizeY = (NorthDistance + SouthDistance) * 0.9f;
				if (SizeX < MinimumDiameter || SizeY < MinimumDiameter)
				{
					continue;
				}

				const FVector2D AdjustedCenter2D(
					Candidate.X + ((EastDistance - WestDistance) * 0.5f),
					Candidate.Y + ((NorthDistance - SouthDistance) * 0.5f));
				const float MinimumBankHeight = FMath::Min(FMath::Min(EastHeight, WestHeight), FMath::Min(NorthHeight, SouthHeight));
				const float SurfaceZ = FMath::Clamp(
					MinimumBankHeight - 24.0f,
					CenterFloorZ + 60.0f,
					CenterFloorZ + 145.0f);
				const float BalancePenalty = (FMath::Abs(EastDistance - WestDistance) + FMath::Abs(NorthDistance - SouthDistance)) * 0.35f;
				const float MarkerDistancePenalty = FVector2D::Distance(AdjustedCenter2D, FVector2D(MarkerLocation.X, MarkerLocation.Y)) * 0.22f;
				const float Score = (SizeX + SizeY) - BalancePenalty - MarkerDistancePenalty;
				if (!bFoundPlane || Score > BestScore)
				{
					OutPlane.Center = FVector(AdjustedCenter2D.X, AdjustedCenter2D.Y, SurfaceZ);
					OutPlane.SizeX = SizeX;
					OutPlane.SizeY = SizeY;
					OutPlane.Yaw = 0.0f;
					BestScore = Score;
					bFoundPlane = true;
				}
			}
		}

		return bFoundPlane;
	}

	bool ResolveBigLakeFromMarkers(
		UWorld* World,
		const FBox& LandscapeBounds,
		FResolvedBasinPlane& OutPlane)
	{
		OutPlane = FResolvedBasinPlane();

		FVector CenterMarker = FVector::ZeroVector;
		if (!FindMarkerLocation(World, TEXT("WATER_BIGLAKE_CENTER"), CenterMarker))
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandRiverMesh: missing WATER_BIGLAKE_CENTER marker; refusing to guess."));
			return false;
		}

		const TArray<FString> EdgeLabels = {
			TEXT("WATER_BIGLAKE_EDGE_01"),
			TEXT("WATER_BIGLAKE_EDGE_02"),
			TEXT("WATER_BIGLAKE_EDGE_03"),
			TEXT("WATER_BIGLAKE_EDGE_04")
		};

		TArray<FVector> EdgeMarkers;
		for (const FString& EdgeLabel : EdgeLabels)
		{
			FVector EdgeLocation = FVector::ZeroVector;
			if (!FindMarkerLocation(World, EdgeLabel, EdgeLocation))
			{
				UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandRiverMesh: missing %s marker; refusing to guess."), *EdgeLabel);
				return false;
			}

			EdgeMarkers.Add(EdgeLocation);
		}

		FResolvedBasinPlane BasePlane;
		if (!ResolveRoundBasinFromMarker(World, LandscapeBounds, CenterMarker, 900.0f, 22000.0f, 135.0f, BasePlane))
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandRiverMesh: failed to resolve the big lake basin around WATER_BIGLAKE_CENTER; refusing to guess."));
			return false;
		}

		FVector2D MajorAxis(1.0f, 0.0f);
		float MaxEdgeDistanceSq = 0.0f;
		for (int32 LeftIndex = 0; LeftIndex < EdgeMarkers.Num(); ++LeftIndex)
		{
			for (int32 RightIndex = LeftIndex + 1; RightIndex < EdgeMarkers.Num(); ++RightIndex)
			{
				const FVector2D Delta(
					EdgeMarkers[RightIndex].X - EdgeMarkers[LeftIndex].X,
					EdgeMarkers[RightIndex].Y - EdgeMarkers[LeftIndex].Y);
				const float DistanceSq = Delta.SizeSquared();
				if (DistanceSq > MaxEdgeDistanceSq && !Delta.IsNearlyZero())
				{
					MajorAxis = Delta.GetSafeNormal();
					MaxEdgeDistanceSq = DistanceSq;
				}
			}
		}

		const FVector2D MinorAxis(-MajorAxis.Y, MajorAxis.X);
		float MinMajor = TNumericLimits<float>::Max();
		float MaxMajor = -TNumericLimits<float>::Max();
		float MinMinor = TNumericLimits<float>::Max();
		float MaxMinor = -TNumericLimits<float>::Max();
		float MinEdgeBankZ = TNumericLimits<float>::Max();
		for (const FVector& EdgeMarker : EdgeMarkers)
		{
			const FVector2D DeltaToEdge(EdgeMarker.X - BasePlane.Center.X, EdgeMarker.Y - BasePlane.Center.Y);
			const float MajorProjection = FVector2D::DotProduct(DeltaToEdge, MajorAxis);
			const float MinorProjection = FVector2D::DotProduct(DeltaToEdge, MinorAxis);
			MinMajor = FMath::Min(MinMajor, MajorProjection);
			MaxMajor = FMath::Max(MaxMajor, MajorProjection);
			MinMinor = FMath::Min(MinMinor, MinorProjection);
			MaxMinor = FMath::Max(MaxMinor, MinorProjection);

			float EdgeBankZ = 0.0f;
			if (SampleLandscapeZ(World, LandscapeBounds, EdgeMarker.X, EdgeMarker.Y, EdgeBankZ))
			{
				MinEdgeBankZ = FMath::Min(MinEdgeBankZ, EdgeBankZ);
			}
		}

		OutPlane.Center = BasePlane.Center;
		OutPlane.SizeX = FMath::Max(BasePlane.SizeX, (MaxMajor - MinMajor) * 0.9f);
		OutPlane.SizeY = FMath::Max(BasePlane.SizeY, (MaxMinor - MinMinor) * 0.9f);
		OutPlane.Yaw = FMath::RadiansToDegrees(FMath::Atan2(MajorAxis.Y, MajorAxis.X));
		if (MinEdgeBankZ < TNumericLimits<float>::Max())
		{
			OutPlane.Center.Z = FMath::Min(OutPlane.Center.Z, MinEdgeBankZ - 24.0f);
		}
		return true;
	}

	int32 SpawnLakeAndPondPlanes(UWorld* World, const FBox& LandscapeBounds, UStaticMesh* PlaneMesh, UMaterialInterface* WaterMaterial)
	{
		if (!World || !PlaneMesh || !WaterMaterial)
		{
			return 0;
		}

		auto SpawnBasinFillStrips =
			[World, &LandscapeBounds, PlaneMesh, WaterMaterial](const FString& BaseLabel, const FResolvedBasinPlane& Basin) -> int32
		{
			const float RowThickness = 450.0f;
			const float SampleStep = 300.0f;
			const float WetDepthTolerance = 8.0f;
			const FVector2D MajorAxis(FMath::Cos(FMath::DegreesToRadians(Basin.Yaw)), FMath::Sin(FMath::DegreesToRadians(Basin.Yaw)));
			const FVector2D MinorAxis(-MajorAxis.Y, MajorAxis.X);
			const float HalfMajor = Basin.SizeX * 0.5f;
			const float HalfMinor = Basin.SizeY * 0.5f;
			int32 CreatedStrips = 0;

			for (float RowOffset = -HalfMinor; RowOffset <= HalfMinor; RowOffset += RowThickness)
			{
				bool bInsideWetRun = false;
				float WetRunStart = 0.0f;
				float LastWetOffset = 0.0f;

				for (float MajorOffset = -HalfMajor; MajorOffset <= HalfMajor + SampleStep; MajorOffset += SampleStep)
				{
					const bool bPastEnd = MajorOffset > HalfMajor;
					bool bWetSample = false;

					if (!bPastEnd)
					{
						const FVector2D SamplePoint =
							FVector2D(Basin.Center.X, Basin.Center.Y) +
							(MajorAxis * MajorOffset) +
							(MinorAxis * RowOffset);
						if (IsInsideLandscape2D(LandscapeBounds, SamplePoint))
						{
							float SampleZ = 0.0f;
							if (SampleLandscapeZ(World, LandscapeBounds, SamplePoint.X, SamplePoint.Y, SampleZ))
							{
								bWetSample = SampleZ <= (Basin.Center.Z - WetDepthTolerance);
							}
						}
					}

					if (bWetSample && !bInsideWetRun)
					{
						bInsideWetRun = true;
						WetRunStart = MajorOffset;
						LastWetOffset = MajorOffset;
					}
					else if (bWetSample)
					{
						LastWetOffset = MajorOffset;
					}
					else if (bInsideWetRun)
					{
						const float RunLength = (LastWetOffset - WetRunStart) + SampleStep;
						if (RunLength >= (SampleStep * 1.5f))
						{
							const float RunCenterOffset = (WetRunStart + LastWetOffset) * 0.5f;
							const FVector2D StripCenter2D =
								FVector2D(Basin.Center.X, Basin.Center.Y) +
								(MajorAxis * RunCenterOffset) +
								(MinorAxis * RowOffset);
							const FString StripLabel = FString::Printf(TEXT("%s_%03d"), *BaseLabel, CreatedStrips + 1);
							AStaticMeshActor* StripActor = SpawnWaterPlane(
								World,
								PlaneMesh,
								WaterMaterial,
								StripLabel,
								FVector(StripCenter2D.X, StripCenter2D.Y, Basin.Center.Z),
								FRotator(0.0f, Basin.Yaw, 0.0f),
								FVector((RunLength * 1.06f) / 100.0f, (RowThickness * 1.05f) / 100.0f, 1.0f));
							if (StripActor)
							{
								++CreatedStrips;
							}
						}

						bInsideWetRun = false;
					}
				}
			}

			return CreatedStrips;
		};

		struct FMarkerBasinSpec
		{
			FString Label;
			float SearchRadius;
			float MaxRadius;
			float RiseThreshold;
		};

		const TArray<FMarkerBasinSpec> PondMarkers = {
			{ TEXT("WATER_POND_01"), 600.0f, 9000.0f, 125.0f },
			{ TEXT("WATER_POND_02"), 600.0f, 9000.0f, 125.0f },
			{ TEXT("WATER_POND_03"), 600.0f, 9000.0f, 125.0f },
			{ TEXT("WATER_POND_04"), 600.0f, 9000.0f, 125.0f }
		};

		TArray<TPair<FString, FResolvedBasinPlane>> BasinsToSpawn;

		FResolvedBasinPlane BigLakePlane;
		if (!ResolveBigLakeFromMarkers(World, LandscapeBounds, BigLakePlane))
		{
			return 0;
		}
		BasinsToSpawn.Add(TPair<FString, FResolvedBasinPlane>(TEXT("FF_InnerWater_Lake_Main"), BigLakePlane));

		for (const FMarkerBasinSpec& PondMarker : PondMarkers)
		{
			FVector MarkerLocation = FVector::ZeroVector;
			if (!FindMarkerLocation(World, PondMarker.Label, MarkerLocation))
			{
				UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandRiverMesh: missing pond marker %s; refusing to guess."), *PondMarker.Label);
				return 0;
			}

			FResolvedBasinPlane ResolvedPond;
			if (!ResolveRoundBasinFromMarker(World, LandscapeBounds, MarkerLocation, PondMarker.SearchRadius, PondMarker.MaxRadius, PondMarker.RiseThreshold, ResolvedPond))
			{
				UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandRiverMesh: failed to resolve basin for %s; refusing to guess."), *PondMarker.Label);
				return 0;
			}

			const FString ActorLabel = FString::Printf(TEXT("FF_InnerWater_Pond_%02d"), BasinsToSpawn.Num());
			BasinsToSpawn.Add(TPair<FString, FResolvedBasinPlane>(ActorLabel, ResolvedPond));
		}

		int32 CreatedPlanes = 0;
		for (const TPair<FString, FResolvedBasinPlane>& BasinEntry : BasinsToSpawn)
		{
			const FResolvedBasinPlane& Basin = BasinEntry.Value;
			const int32 CreatedBasinStrips = SpawnBasinFillStrips(BasinEntry.Key, Basin);
			UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandRiverMesh: created %s basin center=(%.1f, %.1f, %.1f) size=(%.1f, %.1f) strips=%d"),
				*BasinEntry.Key,
				Basin.Center.X,
				Basin.Center.Y,
				Basin.Center.Z,
				Basin.SizeX,
				Basin.SizeY,
				CreatedBasinStrips);
			CreatedPlanes += CreatedBasinStrips;
		}

		return CreatedPlanes;
	}
}
#endif

int32 UFFStarterHighlandRiverMeshCommandlet::Main(const FString& Params)
{
#if WITH_EDITOR
	UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandRiverMesh: loading %s for inner-water cleanup and visible river rebuild."), HighlandMapPath);
	UWorld* World = UEditorLoadingAndSavingUtils::LoadMap(HighlandMapPath);
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandRiverMesh: failed to load %s"), HighlandMapPath);
		return 1;
	}

	FBox LandscapeBounds;
	if (!FindLandscapeBounds(World, LandscapeBounds))
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandRiverMesh: no landscape bounds found; refusing to guess river placement."));
		return 1;
	}

	UMaterialInterface* RiverMaterial = LoadObject<UMaterialInterface>(nullptr, VisibleRiverMaterialPath);
	if (!RiverMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandRiverMesh: failed to load visible river material %s"), VisibleRiverMaterialPath);
		return 1;
	}

	UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, PlaneMeshPath);
	if (!PlaneMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandRiverMesh: failed to load %s"), PlaneMeshPath);
		return 1;
	}

	TArray<FResolvedRiverPatch> RiverPatches;
	if (!BuildMarkerDrivenRiverPatches(World, LandscapeBounds, RiverPatches))
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandRiverMesh: marker-driven river trench build failed; map was not saved."));
		return 1;
	}

	const int32 DeletedActors = DeleteWaterActors(World);
	const int32 CreatedSegments = SpawnRiverMeshSegments(World, PlaneMesh, RiverMaterial, RiverPatches);
	const int32 CreatedWaterPlanes = SpawnLakeAndPondPlanes(World, LandscapeBounds, PlaneMesh, RiverMaterial);
	if (CreatedSegments <= 0 || CreatedWaterPlanes < 5)
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandRiverMesh: inner water rebuild was incomplete; map was not saved."));
		return 1;
	}

	World->MarkPackageDirty();
	const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
	const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
	UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandRiverMesh: deletedWaterActors=%d createdRiverSegments=%d createdWaterPlanes=%d material=%s savedMap=%s savedPackages=%s"),
		DeletedActors,
		CreatedSegments,
		CreatedWaterPlanes,
		*RiverMaterial->GetPathName(),
		bSavedMap ? TEXT("true") : TEXT("false"),
		bSavedPackages ? TEXT("true") : TEXT("false"));

	return (bSavedMap && bSavedPackages) ? 0 : 1;
#else
	return 0;
#endif
}
