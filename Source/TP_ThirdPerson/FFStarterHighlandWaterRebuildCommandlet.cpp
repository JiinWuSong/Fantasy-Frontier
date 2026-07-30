#include "FFStarterHighlandWaterRebuildCommandlet.h"

#if WITH_EDITOR
#include "CollisionQueryParams.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/HitResult.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "FileHelpers.h"
#include "LandscapeProxy.h"
#include "Materials/MaterialInterface.h"
#include "WaterBodyActor.h"
#include "WaterBodyComponent.h"
#include "WaterBodyCustomActor.h"
#include "WaterBodyLakeActor.h"
#include "WaterBodyRiverActor.h"
#include "WaterBodyRiverComponent.h"
#include "WaterSplineComponent.h"
#include "WaterSplineMetadata.h"
#include "WaterZoneActor.h"
#endif

UFFStarterHighlandWaterRebuildCommandlet::UFFStarterHighlandWaterRebuildCommandlet()
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
	const TCHAR* TitanWaterRiverPath = TEXT("/Game/VFX/Environment/_Global/WaterPainterly/Regional_Material_Instances/Global/MI_WaterPainterly_Grassland_River.MI_WaterPainterly_Grassland_River");
	const TCHAR* TitanWaterRiverLodPath = TEXT("/Game/VFX/Environment/_Global/WaterPainterly/Regional_Material_Instances/Global/MI_WaterPainterly_Grassland_River_LOD.MI_WaterPainterly_Grassland_River_LOD");
	const TCHAR* TitanWaterRiverToLakePath = TEXT("/Game/VFX/Environment/_Global/WaterPainterly/Regional_Material_Instances/Global/MI_WaterPainterly_Grassland_River_To_Lake.MI_WaterPainterly_Grassland_River_To_Lake");
	const TCHAR* TitanWaterRiverToOceanPath = TEXT("/Game/VFX/Environment/_Global/WaterPainterly/Regional_Material_Instances/Global/MI_WaterPainterly_Grassland_River_To_Ocean.MI_WaterPainterly_Grassland_River_To_Ocean");
	const TCHAR* TitanWaterLakePath = TEXT("/Game/VFX/Environment/_Global/WaterPainterly/Regional_Material_Instances/Global/MI_WaterPainterly_Grassland_Lake.MI_WaterPainterly_Grassland_Lake");
	const TCHAR* TitanWaterLakeLodPath = TEXT("/Game/VFX/Environment/_Global/WaterPainterly/Regional_Material_Instances/Global/MI_WaterPainterly_Grassland_Lake_LOD.MI_WaterPainterly_Grassland_Lake_LOD");
	const TCHAR* FallbackWaterRiverPath = TEXT("/Game/Environment/_Global/Core/Materials/Water/Water_Material_River.Water_Material_River");
	const TCHAR* FallbackWaterCustomMeshPath = TEXT("/Game/Environment/_Global/Core/Materials/Water/Water_Material_CustomMesh.Water_Material_CustomMesh");

	const FName WaterRebuildTag(TEXT("FFPhase1WaterRebuild"));

	struct FWaterPoint2D
	{
		float X;
		float Y;
	};

	struct FDerivedLake
	{
		TArray<FVector> WorldLoop;
		FVector Center = FVector::ZeroVector;
		FString Label;
	};

	struct FDerivedWaterLayout
	{
		FBox LandscapeBounds;
		TArray<FVector> RiverWorldPoints;
		TArray<FDerivedLake> Lakes;
		float RiverHalfWidth = 600.0f;
	};

	struct FTerrainSample
	{
		int32 XIndex = 0;
		int32 YIndex = 0;
		FVector Location = FVector::ZeroVector;
		bool bValid = false;
		bool bInterior = false;
		bool bCandidate = false;
		float Z = 0.0f;
		float Relief = 0.0f;
	};

	struct FWaterComponentInfo
	{
		TArray<int32> SampleIndices;
		FBox2D Bounds;
		int32 Count = 0;
		float MinZ = TNumericLimits<float>::Max();
		float MaxZ = -TNumericLimits<float>::Max();
	};

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

	bool IsBrokenCustomWaterActor(const AActor* Actor)
	{
		if (!Actor)
		{
			return false;
		}

		const FString Name = Actor->GetName();
		const FString Label = Actor->GetActorLabel();
		const FString ClassName = Actor->GetClass() ? Actor->GetClass()->GetName() : FString();
		if (!TextContainsAny(ClassName, { TEXT("WaterBodyCustom") }) && !TextContainsAny(Name, { TEXT("WaterBodyCustom") }) && !TextContainsAny(Label, { TEXT("WaterBodyCustom") }))
		{
			return false;
		}

		return TextContainsAny(Name, { TEXT("WaterBodyCustom_0"), TEXT("WaterBodyCustom_1"), TEXT("WaterBodyCustom_2") })
			|| TextContainsAny(Label, { TEXT("WaterBodyCustom"), TEXT("WaterBodyCustom2"), TEXT("WaterBodyCustom_0"), TEXT("WaterBodyCustom_1"), TEXT("WaterBodyCustom_2") });
	}

	bool IsPriorGeneratedWaterActor(const AActor* Actor)
	{
		if (!Actor)
		{
			return false;
		}

		return Actor->ActorHasTag(WaterRebuildTag)
			|| TextContainsAny(Actor->GetName(), { TEXT("FF_Water_River_"), TEXT("FF_Water_Lake_"), TEXT("FF_WaterZone_Highland") })
			|| TextContainsAny(Actor->GetActorLabel(), { TEXT("FF_Water_River_"), TEXT("FF_Water_Lake_"), TEXT("FF_WaterZone_Highland") });
	}

	bool IsAcceptedSmallLakeActor(const AActor* Actor)
	{
		if (!Actor)
		{
			return false;
		}

		const FString Name = Actor->GetName();
		const FString Label = Actor->GetActorLabel();
		return TextContainsAny(Name, { TEXT("FF_Water_Lake_01"), TEXT("FF_Water_Lake_02"), TEXT("FF_Water_Lake_03"), TEXT("FF_Water_Lake_04") })
			|| TextContainsAny(Label, { TEXT("FF_Water_Lake_01"), TEXT("FF_Water_Lake_02"), TEXT("FF_Water_Lake_03"), TEXT("FF_Water_Lake_04") });
	}

	int32 DeleteWaterActors(UWorld* World)
	{
		TArray<AActor*> ActorsToDelete;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			const bool bMainRiverOnly =
				TextContainsAny(Actor->GetName(), { TEXT("FF_Water_River_Main") })
				|| TextContainsAny(Actor->GetActorLabel(), { TEXT("FF_Water_River_Main") });
			if (bMainRiverOnly)
			{
				ActorsToDelete.Add(Actor);
			}
		}

		for (AActor* Actor : ActorsToDelete)
		{
			UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandWaterRebuild: deleting actor name=%s label=%s class=%s"),
				*Actor->GetName(),
				*Actor->GetActorLabel(),
				Actor->GetClass() ? *Actor->GetClass()->GetName() : TEXT("None"));
			World->DestroyActor(Actor, false, false);
		}

		return ActorsToDelete.Num();
	}

	template<typename TActorType>
	TActorType* FindNamedActor(UWorld* World, const TCHAR* ActorToken)
	{
		if (!World)
		{
			return nullptr;
		}

		for (TActorIterator<TActorType> It(World); It; ++It)
		{
			TActorType* Actor = *It;
			if (!Actor)
			{
				continue;
			}

			if (TextContainsAny(Actor->GetName(), { ActorToken }) || TextContainsAny(Actor->GetActorLabel(), { ActorToken }))
			{
				return Actor;
			}
		}

		return nullptr;
	}

	TArray<FVector2D> GetMainRiverTrenchGuide()
	{
		// Fixed manual spline anchors that follow the visible carved trench directly.
		// Start at the lake outlet rather than the lake center so WaterBodyRiver does not cut diagonally across the basin.
		return {
			FVector2D(-36000.0f, 59500.0f),
			FVector2D(-38250.0f, 56500.0f),
			FVector2D(-40500.0f, 53500.0f),
			FVector2D(-42500.0f, 50000.0f),
			FVector2D(-44500.0f, 45500.0f),
			FVector2D(-46750.0f, 40500.0f),
			FVector2D(-48800.0f, 35250.0f),
			FVector2D(-50750.0f, 29500.0f),
			FVector2D(-50500.0f, 23500.0f),
			FVector2D(-49000.0f, 18250.0f),
			FVector2D(-46800.0f, 13250.0f),
			FVector2D(-44600.0f, 8500.0f),
			FVector2D(-43600.0f, 2500.0f),
			FVector2D(-44600.0f, -3750.0f),
			FVector2D(-47000.0f, -10500.0f),
			FVector2D(-50500.0f, -18250.0f),
			FVector2D(-54500.0f, -27500.0f),
			FVector2D(-59000.0f, -38250.0f),
			FVector2D(-63500.0f, -49500.0f),
			FVector2D(-67500.0f, -61250.0f)
		};
	}

	FVector2D GetNorthernLakeBasinSeed()
	{
		// Center of the manually carved northern lake depression in the current map.
		return FVector2D(-30000.0f, 62000.0f);
	}

	TArray<FVector2D> GetNorthernLakeBasinLoop()
	{
		// Fixed basin outline sized to fill the carved northern lake while preserving visible islands.
		return {
			FVector2D(-39250.0f, 61800.0f),
			FVector2D(-38250.0f, 65250.0f),
			FVector2D(-35750.0f, 68600.0f),
			FVector2D(-31750.0f, 70800.0f),
			FVector2D(-27250.0f, 70500.0f),
			FVector2D(-23250.0f, 68400.0f),
			FVector2D(-20500.0f, 65250.0f),
			FVector2D(-19350.0f, 61500.0f),
			FVector2D(-20250.0f, 58000.0f),
			FVector2D(-22800.0f, 55250.0f),
			FVector2D(-26800.0f, 53500.0f),
			FVector2D(-31300.0f, 53250.0f),
			FVector2D(-35250.0f, 54500.0f),
			FVector2D(-38250.0f, 57250.0f)
		};
	}

	bool IsInsideLandscape2D(const FBox& LandscapeBounds, const FVector2D& Point)
	{
		return Point.X >= LandscapeBounds.Min.X && Point.X <= LandscapeBounds.Max.X
			&& Point.Y >= LandscapeBounds.Min.Y && Point.Y <= LandscapeBounds.Max.Y;
	}

	int32 GridIndex(const int32 XIndex, const int32 YIndex, const int32 NumY)
	{
		return XIndex * NumY + YIndex;
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
				UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandWaterRebuild: landscape=%s boundsMin=(%.1f, %.1f, %.1f) boundsMax=(%.1f, %.1f, %.1f)"),
					*Landscape->GetActorLabel(),
					Bounds.Min.X,
					Bounds.Min.Y,
					Bounds.Min.Z,
					Bounds.Max.X,
					Bounds.Max.Y,
					Bounds.Max.Z);
			}
		}
		return OutBounds.IsValid != 0;
	}

	bool SampleLandscapeZ(UWorld* World, const FBox& LandscapeBounds, const float X, const float Y, float& OutZ)
	{
		TArray<FHitResult> Hits;
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(FFStarterWaterTerrainTrace), true);
		QueryParams.bTraceComplex = true;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && (Actor->IsA<AWaterBody>() || Actor->IsA<AWaterZone>()))
			{
				QueryParams.AddIgnoredActor(Actor);
			}
		}
		const FVector Start(X, Y, LandscapeBounds.Max.Z + 50000.0f);
		const FVector End(X, Y, LandscapeBounds.Min.Z - 50000.0f);
		if (!World || !World->LineTraceMultiByChannel(Hits, Start, End, ECC_WorldStatic, QueryParams))
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

	float Percentile(TArray<float> Values, const float Alpha)
	{
		if (Values.IsEmpty())
		{
			return 0.0f;
		}

		Values.Sort();
		const int32 Index = FMath::Clamp(FMath::RoundToInt(static_cast<float>(Values.Num() - 1) * Alpha), 0, Values.Num() - 1);
		return Values[Index];
	}

	float EstimateWaterSurfaceZ(UWorld* World, const FBox& LandscapeBounds, const FVector2D Point, const float FloorZ, const float RingRadius)
	{
		TArray<float> BankHeights;
		constexpr int32 RingSamples = 12;
		for (int32 Index = 0; Index < RingSamples; ++Index)
		{
			const float Angle = (static_cast<float>(Index) / static_cast<float>(RingSamples)) * (2.0f * UE_PI);
			float SampleZ = 0.0f;
			if (SampleLandscapeZ(World, LandscapeBounds, Point.X + FMath::Cos(Angle) * RingRadius, Point.Y + FMath::Sin(Angle) * RingRadius, SampleZ))
			{
				BankHeights.Add(SampleZ);
			}
		}

		if (BankHeights.Num() < 4)
		{
			return FloorZ + 35.0f;
		}

		BankHeights.Sort();
		const float BankMedian = BankHeights[BankHeights.Num() / 2];
		const float Relief = BankMedian - FloorZ;
		if (Relief <= 80.0f)
		{
			return FloorZ + 90.0f;
		}

		const float MinSurface = FloorZ + 90.0f;
		const float MaxSurface = FMath::Max(MinSurface, BankMedian - 45.0f);
		return FMath::Clamp(FloorZ + Relief * 0.58f, MinSurface, MaxSurface);
	}

	bool FindLowestLandscapePointNear(UWorld* World, const FBox& LandscapeBounds, const FVector2D Seed, const float SearchRadius, const float Step, FVector& OutFloorPoint)
	{
		if (!World)
		{
			return false;
		}

		constexpr float BoundsInset = 900.0f;
		const FVector2D ClampedSeed(
			FMath::Clamp(Seed.X, LandscapeBounds.Min.X + BoundsInset, LandscapeBounds.Max.X - BoundsInset),
			FMath::Clamp(Seed.Y, LandscapeBounds.Min.Y + BoundsInset, LandscapeBounds.Max.Y - BoundsInset));
		if (!IsInsideLandscape2D(LandscapeBounds, ClampedSeed))
		{
			return false;
		}

		bool bFound = false;
		float BestZ = TNumericLimits<float>::Max();
		FVector2D BestPoint = ClampedSeed;
		const int32 Steps = FMath::Max(1, FMath::CeilToInt(SearchRadius / Step));
		for (int32 XStep = -Steps; XStep <= Steps; ++XStep)
		{
			for (int32 YStep = -Steps; YStep <= Steps; ++YStep)
			{
				const FVector2D Candidate = ClampedSeed + FVector2D(static_cast<float>(XStep) * Step, static_cast<float>(YStep) * Step);
				if (FVector2D::Distance(Candidate, ClampedSeed) > SearchRadius || !IsInsideLandscape2D(LandscapeBounds, Candidate))
				{
					continue;
				}

				float CandidateZ = 0.0f;
				if (SampleLandscapeZ(World, LandscapeBounds, Candidate.X, Candidate.Y, CandidateZ) && CandidateZ < BestZ)
				{
					BestZ = CandidateZ;
					BestPoint = Candidate;
					bFound = true;
				}
			}
		}

		if (!bFound)
		{
			return false;
		}

		OutFloorPoint = FVector(BestPoint.X, BestPoint.Y, BestZ);
		return true;
	}

	bool SampleClampedLandscapePoint(UWorld* World, const FBox& LandscapeBounds, const FVector2D Seed, FVector& OutFloorPoint)
	{
		if (!World)
		{
			return false;
		}

		constexpr float BoundsInset = 900.0f;
		const FVector2D ClampedSeed(
			FMath::Clamp(Seed.X, LandscapeBounds.Min.X + BoundsInset, LandscapeBounds.Max.X - BoundsInset),
			FMath::Clamp(Seed.Y, LandscapeBounds.Min.Y + BoundsInset, LandscapeBounds.Max.Y - BoundsInset));

		float FloorZ = 0.0f;
		if (!SampleLandscapeZ(World, LandscapeBounds, ClampedSeed.X, ClampedSeed.Y, FloorZ))
		{
			return false;
		}

		OutFloorPoint = FVector(ClampedSeed.X, ClampedSeed.Y, FloorZ);
		return true;
	}

	TArray<FVector2D> DensifyPolyline(const TArray<FVector2D>& Points, const float MaxSegmentLength)
	{
		TArray<FVector2D> DensePoints;
		if (Points.IsEmpty())
		{
			return DensePoints;
		}

		DensePoints.Add(Points[0]);
		for (int32 Index = 0; Index < Points.Num() - 1; ++Index)
		{
			const FVector2D Start = Points[Index];
			const FVector2D End = Points[Index + 1];
			const float Length = FVector2D::Distance(Start, End);
			const int32 SegmentCount = FMath::Max(1, FMath::CeilToInt(Length / MaxSegmentLength));
			for (int32 SegmentIndex = 1; SegmentIndex <= SegmentCount; ++SegmentIndex)
			{
				const float Alpha = static_cast<float>(SegmentIndex) / static_cast<float>(SegmentCount);
				DensePoints.Add(FMath::Lerp(Start, End, Alpha));
			}
		}
		return DensePoints;
	}

	bool BuildRiverFromExistingTrench(UWorld* World, const FBox& LandscapeBounds, TArray<FVector>& OutRiverPoints)
	{
		OutRiverPoints.Reset();
		const TArray<FVector2D> GuidePoints = GetMainRiverTrenchGuide();
		for (int32 Index = 0; Index < GuidePoints.Num(); ++Index)
		{
			FVector FloorPoint = FVector::ZeroVector;
			if (!SampleClampedLandscapePoint(World, LandscapeBounds, GuidePoints[Index], FloorPoint))
			{
				UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandWaterRebuild: failed to sample manual river trench point[%d]=(%.1f, %.1f); refusing to guess."),
					Index,
					GuidePoints[Index].X,
					GuidePoints[Index].Y);
				return false;
			}

			const float DesignedSurfaceZ = FMath::GetMappedRangeValueClamped(FVector2D(66000.0f, -64000.0f), FVector2D(40.0f, -280.0f), FloorPoint.Y);
			const float SurfaceZ = FMath::Clamp(DesignedSurfaceZ, FloorPoint.Z + 85.0f, FloorPoint.Z + 175.0f);
			OutRiverPoints.Add(FVector(FloorPoint.X, FloorPoint.Y, SurfaceZ));
			UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandWaterRebuild: corrected river point[%d] trench=(%.1f, %.1f) floor=(%.1f, %.1f, %.1f) surfaceZ=%.1f"),
				Index,
				GuidePoints[Index].X,
				GuidePoints[Index].Y,
				FloorPoint.X,
				FloorPoint.Y,
				FloorPoint.Z,
				SurfaceZ);
		}

		return OutRiverPoints.Num() >= 3;
	}

	bool BuildNorthernLakeFromExistingBasin(UWorld* World, const FBox& LandscapeBounds, FDerivedLake& OutLake)
	{
		const FVector2D BasinSeed = GetNorthernLakeBasinSeed();
		if (!IsInsideLandscape2D(LandscapeBounds, BasinSeed))
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandWaterRebuild: northern lake basin seed is outside landscape bounds; refusing to guess."));
			return false;
		}

		FVector FloorPoint = FVector::ZeroVector;
		if (!SampleClampedLandscapePoint(World, LandscapeBounds, BasinSeed, FloorPoint))
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandWaterRebuild: failed to sample the manual northern basin center; refusing to guess."));
			return false;
		}

		const float WaterZ = FMath::Clamp(FloorPoint.Z + 120.0f, FloorPoint.Z + 90.0f, FloorPoint.Z + 170.0f);

		OutLake = FDerivedLake();
		OutLake.Center = FVector(BasinSeed.X, BasinSeed.Y, WaterZ);
		OutLake.Label = TEXT("FF_Water_Lake_North_Basin");
		for (const FVector2D& LoopPoint : GetNorthernLakeBasinLoop())
		{
			if (!IsInsideLandscape2D(LandscapeBounds, LoopPoint))
			{
				UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandWaterRebuild: manual northern basin loop point (%.1f, %.1f) is outside terrain bounds; refusing to guess."),
					LoopPoint.X,
					LoopPoint.Y);
				return false;
			}
			OutLake.WorldLoop.Add(FVector(LoopPoint.X, LoopPoint.Y, WaterZ));
		}

		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandWaterRebuild: corrected northern lake seed=(%.1f, %.1f) floor=(%.1f, %.1f, %.1f) waterZ=%.1f points=%d"),
			BasinSeed.X,
			BasinSeed.Y,
			FloorPoint.X,
			FloorPoint.Y,
			FloorPoint.Z,
			WaterZ,
			OutLake.WorldLoop.Num());
		return OutLake.WorldLoop.Num() >= 8;
	}

	void AddWaterComponent(TArray<FWaterComponentInfo>& Components, const TArray<FTerrainSample>& Samples, const TArray<int32>& ComponentSamples)
	{
		FWaterComponentInfo Component;
		Component.SampleIndices = ComponentSamples;
		Component.Count = ComponentSamples.Num();
		Component.Bounds.Init();
		for (const int32 SampleIndex : ComponentSamples)
		{
			const FTerrainSample& Sample = Samples[SampleIndex];
			Component.Bounds += FVector2D(Sample.Location.X, Sample.Location.Y);
			Component.MinZ = FMath::Min(Component.MinZ, Sample.Z);
			Component.MaxZ = FMath::Max(Component.MaxZ, Sample.Z);
		}
		Components.Add(Component);
	}

	bool BuildDerivedWaterLayout(UWorld* World, FDerivedWaterLayout& OutLayout)
	{
		FBox LandscapeBounds;
		if (!FindLandscapeBounds(World, LandscapeBounds))
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandWaterRebuild: no landscape bounds found; refusing to guess water placement."));
			return false;
		}

		const FVector Extent = LandscapeBounds.GetExtent();
		const int32 NumX = FMath::Clamp(FMath::RoundToInt((Extent.X * 2.0f) / 2500.0f), 48, 96);
		const int32 NumY = FMath::Clamp(FMath::RoundToInt((Extent.Y * 2.0f) / 2500.0f), 40, 80);
		const float StepX = (LandscapeBounds.Max.X - LandscapeBounds.Min.X) / static_cast<float>(NumX - 1);
		const float StepY = (LandscapeBounds.Max.Y - LandscapeBounds.Min.Y) / static_cast<float>(NumY - 1);
		const float SampleStep = FMath::Max(StepX, StepY);
		const float MarginX = (LandscapeBounds.Max.X - LandscapeBounds.Min.X) * 0.08f;
		const float MarginY = (LandscapeBounds.Max.Y - LandscapeBounds.Min.Y) * 0.08f;

		TArray<FTerrainSample> Samples;
		Samples.SetNum(NumX * NumY);
		TArray<float> InteriorHeights;
		TArray<float> InteriorReliefs;

		for (int32 XIndex = 0; XIndex < NumX; ++XIndex)
		{
			for (int32 YIndex = 0; YIndex < NumY; ++YIndex)
			{
				FTerrainSample& Sample = Samples[GridIndex(XIndex, YIndex, NumY)];
				Sample.XIndex = XIndex;
				Sample.YIndex = YIndex;
				Sample.Location.X = LandscapeBounds.Min.X + StepX * XIndex;
				Sample.Location.Y = LandscapeBounds.Min.Y + StepY * YIndex;
				Sample.bInterior = Sample.Location.X > LandscapeBounds.Min.X + MarginX
					&& Sample.Location.X < LandscapeBounds.Max.X - MarginX
					&& Sample.Location.Y > LandscapeBounds.Min.Y + MarginY
					&& Sample.Location.Y < LandscapeBounds.Max.Y - MarginY;
				Sample.bValid = SampleLandscapeZ(World, LandscapeBounds, Sample.Location.X, Sample.Location.Y, Sample.Z);
				Sample.Location.Z = Sample.Z;
				if (Sample.bValid && Sample.bInterior)
				{
					InteriorHeights.Add(Sample.Z);
				}
			}
		}

		if (InteriorHeights.Num() < 100)
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandWaterRebuild: insufficient landscape samples (%d); refusing to guess water placement."), InteriorHeights.Num());
			return false;
		}

		for (int32 XIndex = 0; XIndex < NumX; ++XIndex)
		{
			for (int32 YIndex = 0; YIndex < NumY; ++YIndex)
			{
				FTerrainSample& Sample = Samples[GridIndex(XIndex, YIndex, NumY)];
				if (!Sample.bValid || !Sample.bInterior)
				{
					continue;
				}

				float NeighborZ = 0.0f;
				int32 NeighborCount = 0;
				for (int32 DX = -2; DX <= 2; ++DX)
				{
					for (int32 DY = -2; DY <= 2; ++DY)
					{
						if (DX == 0 && DY == 0)
						{
							continue;
						}

						const int32 NX = XIndex + DX;
						const int32 NY = YIndex + DY;
						if (NX < 0 || NX >= NumX || NY < 0 || NY >= NumY)
						{
							continue;
						}

						const FTerrainSample& Neighbor = Samples[GridIndex(NX, NY, NumY)];
						if (Neighbor.bValid)
						{
							NeighborZ += Neighbor.Z;
							++NeighborCount;
						}
					}
				}

				if (NeighborCount > 0)
				{
					Sample.Relief = (NeighborZ / static_cast<float>(NeighborCount)) - Sample.Z;
					InteriorReliefs.Add(Sample.Relief);
				}
			}
		}

		const float LowHeightCutoff = Percentile(InteriorHeights, 0.22f);
		const float ReliefCutoff = FMath::Max(Percentile(InteriorReliefs, 0.70f), 90.0f);
		for (FTerrainSample& Sample : Samples)
		{
			Sample.bCandidate = Sample.bValid
				&& Sample.bInterior
				&& (Sample.Z <= LowHeightCutoff || Sample.Relief >= ReliefCutoff);
		}

		TArray<int32> ComponentIds;
		ComponentIds.Init(-1, Samples.Num());
		TArray<FWaterComponentInfo> Components;
		for (int32 Index = 0; Index < Samples.Num(); ++Index)
		{
			if (!Samples[Index].bCandidate || ComponentIds[Index] != -1)
			{
				continue;
			}

			TArray<int32> Stack;
			TArray<int32> ComponentSamples;
			Stack.Add(Index);
			ComponentIds[Index] = Components.Num();
			while (!Stack.IsEmpty())
			{
				const int32 CurrentIndex = Stack.Pop(EAllowShrinking::No);
				ComponentSamples.Add(CurrentIndex);
				const FTerrainSample& Current = Samples[CurrentIndex];
				for (int32 DX = -1; DX <= 1; ++DX)
				{
					for (int32 DY = -1; DY <= 1; ++DY)
					{
						if (DX == 0 && DY == 0)
						{
							continue;
						}

						const int32 NX = Current.XIndex + DX;
						const int32 NY = Current.YIndex + DY;
						if (NX < 0 || NX >= NumX || NY < 0 || NY >= NumY)
						{
							continue;
						}

						const int32 NeighborIndex = GridIndex(NX, NY, NumY);
						if (Samples[NeighborIndex].bCandidate && ComponentIds[NeighborIndex] == -1)
						{
							ComponentIds[NeighborIndex] = Components.Num();
							Stack.Add(NeighborIndex);
						}
					}
				}
			}

			if (ComponentSamples.Num() >= 3)
			{
				AddWaterComponent(Components, Samples, ComponentSamples);
			}
		}

		if (Components.IsEmpty())
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandWaterRebuild: no terrain depression components found; refusing to guess water placement."));
			return false;
		}

		int32 RiverComponentIndex = INDEX_NONE;
		float BestRiverScore = -1.0f;
		for (int32 Index = 0; Index < Components.Num(); ++Index)
		{
			const FWaterComponentInfo& Component = Components[Index];
			const FVector2D Size = Component.Bounds.GetSize();
			const float MaxExtent = FMath::Max(Size.X, Size.Y);
			const float Score = MaxExtent * FMath::Sqrt(static_cast<float>(Component.Count));
			if (Score > BestRiverScore)
			{
				BestRiverScore = Score;
				RiverComponentIndex = Index;
			}
		}

		if (RiverComponentIndex == INDEX_NONE)
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandWaterRebuild: no river-like depression component found; refusing to guess water placement."));
			return false;
		}

		const FWaterComponentInfo& RiverComponent = Components[RiverComponentIndex];
		const FVector2D RiverSize = RiverComponent.Bounds.GetSize();
		const bool bRiverUsesXAsPrimaryAxis = RiverSize.X >= RiverSize.Y;
		const float PrimaryMin = bRiverUsesXAsPrimaryAxis ? RiverComponent.Bounds.Min.X : RiverComponent.Bounds.Min.Y;
		const float PrimaryMax = bRiverUsesXAsPrimaryAxis ? RiverComponent.Bounds.Max.X : RiverComponent.Bounds.Max.Y;
		const float PrimaryLength = PrimaryMax - PrimaryMin;
		const int32 BinCount = FMath::Clamp(FMath::RoundToInt(PrimaryLength / (SampleStep * 3.0f)), 5, 14);
		TArray<TArray<const FTerrainSample*>> Bins;
		Bins.SetNum(BinCount);
		for (const int32 SampleIndex : RiverComponent.SampleIndices)
		{
			const FTerrainSample& Sample = Samples[SampleIndex];
			const float Primary = bRiverUsesXAsPrimaryAxis ? Sample.Location.X : Sample.Location.Y;
			const int32 BinIndex = FMath::Clamp(FMath::FloorToInt(((Primary - PrimaryMin) / FMath::Max(PrimaryLength, 1.0f)) * static_cast<float>(BinCount)), 0, BinCount - 1);
			Bins[BinIndex].Add(&Sample);
		}

		TArray<FVector> RiverPoints;
		for (const TArray<const FTerrainSample*>& Bin : Bins)
		{
			if (Bin.IsEmpty())
			{
				continue;
			}

			float WeightSum = 0.0f;
			FVector WeightedPoint = FVector::ZeroVector;
			float WeightedFloorZ = 0.0f;
			for (const FTerrainSample* Sample : Bin)
			{
				const float Weight = FMath::Max(100.0f, Sample->Relief + (LowHeightCutoff - Sample->Z));
				WeightedPoint.X += Sample->Location.X * Weight;
				WeightedPoint.Y += Sample->Location.Y * Weight;
				WeightedFloorZ += Sample->Z * Weight;
				WeightSum += Weight;
			}

			if (WeightSum <= 0.0f)
			{
				continue;
			}

			WeightedPoint.X /= WeightSum;
			WeightedPoint.Y /= WeightSum;
			WeightedFloorZ /= WeightSum;
			WeightedPoint.Z = EstimateWaterSurfaceZ(World, LandscapeBounds, FVector2D(WeightedPoint.X, WeightedPoint.Y), WeightedFloorZ, SampleStep * 1.4f);
			RiverPoints.Add(WeightedPoint);
		}

		if (RiverPoints.Num() < 3)
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandWaterRebuild: detected river depression but could only derive %d spline points; refusing to guess placement."), RiverPoints.Num());
			return false;
		}

		const float RiverMinorExtent = FMath::Max(1.0f, FMath::Min(RiverSize.X, RiverSize.Y));
		OutLayout.RiverHalfWidth = FMath::Clamp(RiverMinorExtent * 0.10f, 350.0f, 1100.0f);
		OutLayout.RiverWorldPoints = RiverPoints;

		for (int32 Index = 0; Index < Components.Num(); ++Index)
		{
			if (Index == RiverComponentIndex)
			{
				continue;
			}

			const FWaterComponentInfo& Component = Components[Index];
			const FVector2D Size = Component.Bounds.GetSize();
			const float MaxSize = FMath::Max(Size.X, Size.Y);
			const float MinSize = FMath::Min(Size.X, Size.Y);
			const float Compactness = MinSize / FMath::Max(MaxSize, 1.0f);
			if (Component.Count < 4 || Compactness < 0.32f || MaxSize < SampleStep * 1.35f || MaxSize > SampleStep * 10.0f)
			{
				continue;
			}

			FVector Center = FVector::ZeroVector;
			float MinFloorZ = TNumericLimits<float>::Max();
			for (const int32 SampleIndex : Component.SampleIndices)
			{
				const FTerrainSample& Sample = Samples[SampleIndex];
				Center.X += Sample.Location.X;
				Center.Y += Sample.Location.Y;
				MinFloorZ = FMath::Min(MinFloorZ, Sample.Z);
			}
			Center.X /= static_cast<float>(Component.Count);
			Center.Y /= static_cast<float>(Component.Count);
			Center.Z = EstimateWaterSurfaceZ(World, LandscapeBounds, FVector2D(Center.X, Center.Y), MinFloorZ, SampleStep * 1.4f);

			FDerivedLake Lake;
			Lake.Center = Center;
			const FVector2D Radius(FMath::Max(Size.X * 0.62f, SampleStep * 0.75f), FMath::Max(Size.Y * 0.62f, SampleStep * 0.75f));
			constexpr int32 LakePointCount = 16;
			for (int32 PointIndex = 0; PointIndex < LakePointCount; ++PointIndex)
			{
				const float Angle = (static_cast<float>(PointIndex) / static_cast<float>(LakePointCount)) * (2.0f * UE_PI);
				Lake.WorldLoop.Add(FVector(Center.X + FMath::Cos(Angle) * Radius.X, Center.Y + FMath::Sin(Angle) * Radius.Y, Center.Z));
			}
			OutLayout.Lakes.Add(Lake);
			if (OutLayout.Lakes.Num() >= 4)
			{
				break;
			}
		}

		OutLayout.LandscapeBounds = LandscapeBounds;
		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandWaterRebuild: derived layout samples=%d validInterior=%d components=%d riverComponent=%d riverPoints=%d riverHalfWidth=%.1f lakes=%d lowHeightCutoff=%.1f reliefCutoff=%.1f boundsMin=(%.1f, %.1f) boundsMax=(%.1f, %.1f)"),
			Samples.Num(),
			InteriorHeights.Num(),
			Components.Num(),
			RiverComponentIndex,
			OutLayout.RiverWorldPoints.Num(),
			OutLayout.RiverHalfWidth,
			OutLayout.Lakes.Num(),
			LowHeightCutoff,
			ReliefCutoff,
			LandscapeBounds.Min.X,
			LandscapeBounds.Min.Y,
			LandscapeBounds.Max.X,
			LandscapeBounds.Max.Y);
		for (int32 Index = 0; Index < OutLayout.RiverWorldPoints.Num(); ++Index)
		{
			const FVector& Point = OutLayout.RiverWorldPoints[Index];
			UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandWaterRebuild: derived river point[%d]=(%.1f, %.1f, %.1f)"), Index, Point.X, Point.Y, Point.Z);
		}
		for (int32 Index = 0; Index < OutLayout.Lakes.Num(); ++Index)
		{
			const FVector& Center = OutLayout.Lakes[Index].Center;
			UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandWaterRebuild: derived lake[%d] center=(%.1f, %.1f, %.1f) points=%d"), Index, Center.X, Center.Y, Center.Z, OutLayout.Lakes[Index].WorldLoop.Num());
		}

		return true;
	}

	bool BuildCorrectedWaterLayout(UWorld* World, FDerivedWaterLayout& OutLayout)
	{
		FBox LandscapeBounds;
		if (!FindLandscapeBounds(World, LandscapeBounds))
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandWaterRebuild: no landscape bounds found; refusing to guess water placement."));
			return false;
		}

		OutLayout = FDerivedWaterLayout();
		OutLayout.LandscapeBounds = LandscapeBounds;
		OutLayout.RiverHalfWidth = 1250.0f;
		if (!BuildRiverFromExistingTrench(World, LandscapeBounds, OutLayout.RiverWorldPoints))
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandWaterRebuild: corrected river placement failed; map was not saved."));
			return false;
		}

		FDerivedLake NorthernLake;
		if (!BuildNorthernLakeFromExistingBasin(World, LandscapeBounds, NorthernLake))
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandWaterRebuild: corrected northern lake placement failed; map was not saved."));
			return false;
		}
		OutLayout.Lakes.Add(NorthernLake);

		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandWaterRebuild: corrected layout riverPoints=%d riverHalfWidth=%.1f correctedLakes=%d preservedSmallLakes=true boundsMin=(%.1f, %.1f) boundsMax=(%.1f, %.1f)"),
			OutLayout.RiverWorldPoints.Num(),
			OutLayout.RiverHalfWidth,
			OutLayout.Lakes.Num(),
			LandscapeBounds.Min.X,
			LandscapeBounds.Min.Y,
			LandscapeBounds.Max.X,
			LandscapeBounds.Max.Y);
		return true;
	}

	void ApplyWaterMaterial(AWaterBody* WaterBody, UMaterialInterface* WaterMaterial, UMaterialInterface* WaterLodMaterial)
	{
		if (!WaterBody || !WaterMaterial)
		{
			return;
		}

		WaterBody->Modify();
		WaterBody->Tags.AddUnique(WaterRebuildTag);
		WaterBody->SetActorHiddenInGame(false);
		WaterBody->SetIsTemporarilyHiddenInEditor(false);

		UWaterBodyComponent* WaterComponent = WaterBody->GetWaterBodyComponent();
		if (!WaterComponent)
		{
			return;
		}

		WaterComponent->Modify();
		WaterComponent->bAffectsLandscape = false;
		WaterComponent->SetVisibility(true, true);
		WaterComponent->SetHiddenInGame(false);
		WaterComponent->SetWaterMaterial(WaterMaterial);
		WaterComponent->SetWaterStaticMeshMaterial(WaterLodMaterial ? WaterLodMaterial : WaterMaterial);

		FOnWaterBodyChangedParams ChangedParams;
		ChangedParams.bShapeOrPositionChanged = true;
		ChangedParams.bWeightmapSettingsChanged = false;
		ChangedParams.bUserTriggered = false;
		WaterComponent->UpdateAll(ChangedParams);
		WaterComponent->MarkPackageDirty();
	}

	void ApplyRiverTransitionMaterials(AWaterBodyRiver* River, UMaterialInterface* RiverToLakeMaterial, UMaterialInterface* RiverToOceanMaterial)
	{
		if (!River)
		{
			return;
		}

		UWaterBodyRiverComponent* RiverComponent = Cast<UWaterBodyRiverComponent>(River->GetWaterBodyComponent());
		if (!RiverComponent)
		{
			return;
		}

		RiverComponent->Modify();
		if (RiverToLakeMaterial)
		{
			RiverComponent->SetLakeTransitionMaterial(RiverToLakeMaterial);
		}
		if (RiverToOceanMaterial)
		{
			RiverComponent->SetOceanTransitionMaterial(RiverToOceanMaterial);
		}
		RiverComponent->MarkPackageDirty();
	}

	int32 ReattachAcceptedSmallLakes(UWorld* World, AWaterZone* WaterZone, UMaterialInterface* LakeMaterial, UMaterialInterface* LakeLodMaterial)
	{
		int32 PreservedLakeCount = 0;
		for (TActorIterator<AWaterBodyLake> It(World); It; ++It)
		{
			AWaterBodyLake* Lake = *It;
			if (!IsAcceptedSmallLakeActor(Lake))
			{
				continue;
			}

			ApplyWaterMaterial(Lake, LakeMaterial, LakeLodMaterial);
			if (WaterZone && Lake->GetWaterBodyComponent())
			{
				Lake->GetWaterBodyComponent()->SetWaterZoneOverride(TSoftObjectPtr<AWaterZone>(WaterZone));
				WaterZone->AddWaterBodyComponent(Lake->GetWaterBodyComponent());
			}
			++PreservedLakeCount;
			const FVector Location = Lake->GetActorLocation();
			UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandWaterRebuild: preserved small lake label=%s location=(%.1f, %.1f, %.1f)"),
				*Lake->GetActorLabel(),
				Location.X,
				Location.Y,
				Location.Z);
		}
		return PreservedLakeCount;
	}

	void ConfigureSpline(AWaterBody* WaterBody, const TArray<FVector>& WorldPoints, const bool bClosedLoop, const float Width, const float Depth)
	{
		if (!WaterBody || WorldPoints.IsEmpty())
		{
			return;
		}

		UWaterSplineComponent* Spline = WaterBody->GetWaterSpline();
		if (!Spline)
		{
			return;
		}

		FVector SplineOrigin = FVector::ZeroVector;
		if (bClosedLoop)
		{
			for (const FVector& Point : WorldPoints)
			{
				SplineOrigin += Point;
			}
			SplineOrigin /= static_cast<float>(WorldPoints.Num());
		}
		else
		{
			SplineOrigin = WorldPoints[0];
		}
		WaterBody->Modify();
		WaterBody->SetActorLocation(SplineOrigin);

		TArray<FVector> LocalPoints;
		LocalPoints.Reserve(WorldPoints.Num());
		for (const FVector& Point : WorldPoints)
		{
			LocalPoints.Add(Point - SplineOrigin);
		}

		Spline->Modify();
		Spline->WaterSplineDefaults.DefaultWidth = Width;
		Spline->WaterSplineDefaults.DefaultDepth = Depth;
		Spline->WaterSplineDefaults.DefaultVelocity = bClosedLoop ? 0.0f : 96.0f;
		Spline->ResetSpline(LocalPoints);
		for (int32 PointIndex = 0; PointIndex < LocalPoints.Num(); ++PointIndex)
		{
			Spline->SetSplinePointType(PointIndex, bClosedLoop ? ESplinePointType::Curve : ESplinePointType::Linear, false);
		}
		Spline->SetClosedLoop(bClosedLoop, true);
		Spline->K2_SynchronizeAndBroadcastDataChange();
		Spline->MarkPackageDirty();

		if (UWaterBodyRiverComponent* RiverComponent = Cast<UWaterBodyRiverComponent>(WaterBody->GetWaterBodyComponent()))
		{
			for (int32 PointIndex = 0; PointIndex < LocalPoints.Num(); ++PointIndex)
			{
				const float InputKey = static_cast<float>(PointIndex);
				RiverComponent->SetRiverWidthAtSplineInputKey(InputKey, Width);
				RiverComponent->SetRiverDepthAtSplineInputKey(InputKey, Depth);
			}
			RiverComponent->MarkPackageDirty();
		}
	}

	template<typename TWaterBodyActor>
	TWaterBodyActor* SpawnWaterBody(UWorld* World, const FName ActorName, const FString& ActorLabel)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Name = MakeUniqueObjectName(World ? World->PersistentLevel : nullptr, TWaterBodyActor::StaticClass(), ActorName);
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		TWaterBodyActor* WaterBody = World->SpawnActor<TWaterBodyActor>(TWaterBodyActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
		if (WaterBody)
		{
			WaterBody->SetActorLabel(ActorLabel);
			WaterBody->Tags.AddUnique(WaterRebuildTag);
		}
		return WaterBody;
	}

	template<typename TWaterBodyActor>
	void DisableLandscapeAffectingDefault()
	{
		TWaterBodyActor* DefaultWaterBody = GetMutableDefault<TWaterBodyActor>();
		if (DefaultWaterBody)
		{
			if (UWaterBodyComponent* DefaultWaterComponent = DefaultWaterBody->GetWaterBodyComponent())
			{
				// Avoid WaterEditor spawning a landscape water brush from commandlet mode.
				DefaultWaterComponent->bAffectsLandscape = false;
			}
		}
	}
}
#endif

int32 UFFStarterHighlandWaterRebuildCommandlet::Main(const FString& Params)
{
#if WITH_EDITOR
	UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandWaterRebuild: loading %s without terrain geometry edits."), HighlandMapPath);
	UWorld* World = UEditorLoadingAndSavingUtils::LoadMap(HighlandMapPath);
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandWaterRebuild: failed to load %s"), HighlandMapPath);
		return 1;
	}

	UMaterialInterface* RiverMaterial = LoadObject<UMaterialInterface>(nullptr, TitanWaterRiverPath);
	UMaterialInterface* RiverLodMaterial = LoadObject<UMaterialInterface>(nullptr, TitanWaterRiverLodPath);
	UMaterialInterface* RiverToLakeMaterial = LoadObject<UMaterialInterface>(nullptr, TitanWaterRiverToLakePath);
	UMaterialInterface* RiverToOceanMaterial = LoadObject<UMaterialInterface>(nullptr, TitanWaterRiverToOceanPath);
	UMaterialInterface* LakeMaterial = LoadObject<UMaterialInterface>(nullptr, TitanWaterLakePath);
	UMaterialInterface* LakeLodMaterial = LoadObject<UMaterialInterface>(nullptr, TitanWaterLakeLodPath);
	bool bUsingTitanPainterlyMaterials = RiverMaterial && LakeMaterial;
	if (!RiverMaterial)
	{
		RiverMaterial = LoadObject<UMaterialInterface>(nullptr, FallbackWaterRiverPath);
		RiverLodMaterial = RiverMaterial;
	}
	if (!LakeMaterial)
	{
		LakeMaterial = LoadObject<UMaterialInterface>(nullptr, FallbackWaterCustomMeshPath);
		LakeLodMaterial = LakeMaterial;
	}
	if (!RiverMaterial || !LakeMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandWaterRebuild: missing both Titan WaterPainterly and fallback water material candidates."));
		return 1;
	}
	UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandWaterRebuild: riverMaterial=%s riverLOD=%s lakeMaterial=%s lakeLOD=%s usingTitanPainterly=%s"),
		*RiverMaterial->GetPathName(),
		RiverLodMaterial ? *RiverLodMaterial->GetPathName() : TEXT("None"),
		*LakeMaterial->GetPathName(),
		LakeLodMaterial ? *LakeLodMaterial->GetPathName() : TEXT("None"),
		bUsingTitanPainterlyMaterials ? TEXT("true") : TEXT("false"));

	FDerivedWaterLayout WaterLayout;
	if (!BuildCorrectedWaterLayout(World, WaterLayout))
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandWaterRebuild: terrain-derived placement failed; map was not saved."));
		return 1;
	}

	DisableLandscapeAffectingDefault<AWaterBodyRiver>();
	DisableLandscapeAffectingDefault<AWaterBodyLake>();
	const int32 DeletedWaterActors = DeleteWaterActors(World);

	AWaterZone* WaterZone = FindNamedActor<AWaterZone>(World, TEXT("FF_WaterZone_Highland"));
	if (!WaterZone)
	{
		WaterZone = SpawnWaterBody<AWaterZone>(World, TEXT("FF_WaterZone_Highland"), TEXT("FF_WaterZone_Highland"));
	}
	if (WaterZone)
	{
		WaterZone->Modify();
		WaterZone->SetActorLocation(WaterLayout.LandscapeBounds.GetCenter());
		WaterZone->SetZoneExtent(2.0f * FVector2D(WaterLayout.LandscapeBounds.GetExtent()));
		WaterZone->Tags.AddUnique(WaterRebuildTag);
		WaterZone->MarkForRebuild(EWaterZoneRebuildFlags::All, WaterZone);
		WaterZone->Update();
		WaterZone->MarkPackageDirty();
	}

	const int32 PreservedSmallLakes = ReattachAcceptedSmallLakes(World, WaterZone, LakeMaterial, LakeLodMaterial);

	AWaterBodyRiver* MainRiver = SpawnWaterBody<AWaterBodyRiver>(World, TEXT("FF_Water_River_Main"), TEXT("FF_Water_River_Main"));
	if (MainRiver)
	{
		ConfigureSpline(MainRiver, WaterLayout.RiverWorldPoints, false, WaterLayout.RiverHalfWidth, 180.0f);
		ApplyWaterMaterial(MainRiver, RiverMaterial, RiverLodMaterial);
		ApplyRiverTransitionMaterials(MainRiver, RiverToLakeMaterial, RiverToOceanMaterial);
		if (WaterZone && MainRiver->GetWaterBodyComponent())
		{
			MainRiver->GetWaterBodyComponent()->SetWaterZoneOverride(TSoftObjectPtr<AWaterZone>(WaterZone));
			WaterZone->AddWaterBodyComponent(MainRiver->GetWaterBodyComponent());
		}
	}

	int32 CreatedLakes = 0;
	for (int32 LakeIndex = 0; LakeIndex < WaterLayout.Lakes.Num(); ++LakeIndex)
	{
		const FString ActorLabel = WaterLayout.Lakes[LakeIndex].Label.IsEmpty()
			? FString::Printf(TEXT("FF_Water_Lake_Corrected_%02d"), LakeIndex + 1)
			: WaterLayout.Lakes[LakeIndex].Label;
		AWaterBodyLake* Lake = FindNamedActor<AWaterBodyLake>(World, *ActorLabel);
		if (!Lake)
		{
			const FName ActorName(*ActorLabel);
			Lake = SpawnWaterBody<AWaterBodyLake>(World, ActorName, ActorLabel);
		}
		if (!Lake)
		{
			continue;
		}

		ConfigureSpline(Lake, WaterLayout.Lakes[LakeIndex].WorldLoop, true, 1000.0f, 180.0f);
		ApplyWaterMaterial(Lake, LakeMaterial, LakeLodMaterial);
		if (WaterZone && Lake->GetWaterBodyComponent())
		{
			Lake->GetWaterBodyComponent()->SetWaterZoneOverride(TSoftObjectPtr<AWaterZone>(WaterZone));
			WaterZone->AddWaterBodyComponent(Lake->GetWaterBodyComponent());
		}
		++CreatedLakes;
	}

	if (WaterZone)
	{
		WaterZone->MarkForRebuild(EWaterZoneRebuildFlags::All, WaterZone);
		WaterZone->Update();
		WaterZone->MarkPackageDirty();
	}

	World->MarkPackageDirty();
	const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
	const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
	UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandWaterRebuild: deletedWaterActors=%d createdRivers=%d createdLakes=%d waterZone=%s titanPainterly=%s savedMap=%s savedPackages=%s"),
		DeletedWaterActors,
		MainRiver ? 1 : 0,
		CreatedLakes,
		WaterZone ? TEXT("true") : TEXT("false"),
		bUsingTitanPainterlyMaterials ? TEXT("true") : TEXT("false"),
		bSavedMap ? TEXT("true") : TEXT("false"),
		bSavedPackages ? TEXT("true") : TEXT("false"));
	UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandWaterRebuild: preservedSmallLakes=%d movedOrResizedActors=FF_Water_River_Main,FF_Water_Lake_North_Basin"), PreservedSmallLakes);

	return bSavedMap && bSavedPackages ? 0 : 1;
#else
	return 0;
#endif
}
