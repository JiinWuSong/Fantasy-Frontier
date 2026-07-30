#include "FFStarterHighlandTerrainNaturalizeCommandlet.h"

#if WITH_EDITOR
#include "Engine/World.h"
#include "EngineUtils.h"
#include "FileHelpers.h"
#include "GameFramework/Actor.h"
#include "Landscape.h"
#include "LandscapeComponent.h"
#include "LandscapeEdit.h"
#include "LandscapeHeightfieldCollisionComponent.h"
#include "LandscapeInfo.h"
#include "LandscapeProxy.h"
#include "Misc/Parse.h"
#include "Engine/CollisionProfile.h"
#endif

UFFStarterHighlandTerrainNaturalizeCommandlet::UFFStarterHighlandTerrainNaturalizeCommandlet()
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
	const FVector2D SpawnKeepFlatCenter(38550.0f, 147850.0f);
	const FName TraversalReadabilityV1Tag(TEXT("FFHighlandTraversalReadabilityV1Applied"));
	const FName TraversalReadabilityV2Tag(TEXT("FFHighlandTraversalReadabilityV2Applied"));
	const FName TraversalReadabilityV3Tag(TEXT("FFHighlandTraversalReadabilityV3Applied"));
	const FName TraversalReadabilityV4Tag(TEXT("FFHighlandTraversalReadabilityV4Applied"));
	const FName TraversalReadabilityV5Tag(TEXT("FFHighlandTraversalReadabilityV5Applied"));
	const FName TraversalReadabilityV6Tag(TEXT("FFHighlandTraversalReadabilityV6Applied"));
	const FName OuterRingGeologyV42Tag(TEXT("FFHighlandOuterRingGeologyV42Applied"));
	const FName OuterRingGeologyV42RefineTag(TEXT("FFHighlandOuterRingGeologyV42RefineApplied"));
	const FName OuterRingGeologyV43Tag(TEXT("FFHighlandOuterRingGeologyV43Applied"));
	const FName OuterRingGeologyV43RefineTag(TEXT("FFHighlandOuterRingGeologyV43RefineApplied"));
	const FName OuterRingGeologyV44Tag(TEXT("FFHighlandOuterRingGeologyV44Applied"));
	const FName OuterRingGeologyV44RefineTag(TEXT("FFHighlandOuterRingGeologyV44RefineApplied"));
	const FName OuterRingGeologyV44HardTag(TEXT("FFHighlandOuterRingGeologyV44HardApplied"));
	const FName OuterRingGeologyV44CorrectionTag(TEXT("FFHighlandOuterRingGeologyV44CorrectionApplied"));
	const FName OuterRingSilhouetteV51Tag(TEXT("FFHighlandOuterRingSilhouetteV51Applied"));
	const FName OuterRingSilhouetteV51RefineTag(TEXT("FFHighlandOuterRingSilhouetteV51RefineApplied"));
	const FName OuterRingSilhouetteV51FaceBreakTag(TEXT("FFHighlandOuterRingSilhouetteV51FaceBreakApplied"));
	const FName GameplayFieldV632Tag(TEXT("FFHighlandGameplayFieldV632Applied"));
	const FName TerrainFoundationV79Tag(TEXT("FFHighlandTerrainFoundationV79Applied"));
	const FName TerrainFoundationV79RefineTag(TEXT("FFHighlandTerrainFoundationV79RefineApplied"));
	const FName TerrainFoundationV79ValidationRefineTag(TEXT("FFHighlandTerrainFoundationV79ValidationRefineApplied"));
	const FName TerrainFoundationV80Tag(TEXT("FFHighlandTerrainFoundationV80Applied"));
	const FName TerrainFoundationV80RefineTag(TEXT("FFHighlandTerrainFoundationV80RefineApplied"));
	const FName TerrainFoundationV81Tag(TEXT("FFHighlandTerrainFoundationV81Applied"));
	const FName TerrainFoundationV81RefineTag(TEXT("FFHighlandTerrainFoundationV81RefineApplied"));
	const FName TerrainFoundationV81FinalTag(TEXT("FFHighlandTerrainFoundationV81FinalApplied"));
	const FName AAATraversalTopographyV83Tag(TEXT("FFHighlandAAATraversalTopographyV83Applied"));
	const FName AAATraversalTopographyV83RefineTag(TEXT("FFHighlandAAATraversalTopographyV83RefineApplied"));

	float DistanceToSegment(const FVector2D& Point, const FVector2D& Start, const FVector2D& End)
	{
		const FVector2D Segment = End - Start;
		const float Denominator = FMath::Max(FVector2D::DotProduct(Segment, Segment), 1.0f);
		const float Alpha = FMath::Clamp(FVector2D::DotProduct(Point - Start, Segment) / Denominator, 0.0f, 1.0f);
		return FVector2D::Distance(Point, Start + Segment * Alpha);
	}

	float GetNearestWaterDistance(const FVector2D& Position)
	{
		float Nearest = TNumericLimits<float>::Max();
		auto CheckSegment = [&Nearest, &Position](const FVector2D& Start, const FVector2D& End)
		{
			Nearest = FMath::Min(Nearest, DistanceToSegment(Position, Start, End));
		};

		CheckSegment(FVector2D(48590.0f, 183116.0f), FVector2D(49910.0f, 174430.0f));
		CheckSegment(FVector2D(49910.0f, 174430.0f), FVector2D(51518.0f, 166778.0f));
		CheckSegment(FVector2D(51518.0f, 166778.0f), FVector2D(58605.0f, 152688.0f));
		CheckSegment(FVector2D(58605.0f, 152688.0f), FVector2D(62544.0f, 138795.0f));
		CheckSegment(FVector2D(62544.0f, 138795.0f), FVector2D(58487.0f, 122723.0f));
		CheckSegment(FVector2D(58487.0f, 122723.0f), FVector2D(55306.0f, 96223.0f));
		CheckSegment(FVector2D(55306.0f, 96223.0f), FVector2D(54105.0f, 78933.0f));
		CheckSegment(FVector2D(54105.0f, 78933.0f), FVector2D(57584.0f, 62136.0f));
		CheckSegment(FVector2D(57584.0f, 62136.0f), FVector2D(63147.0f, 37260.0f));

		const FVector2D Lake0 = (Position - FVector2D(40000.0f, 23500.0f)) / FVector2D(16000.0f, 12000.0f);
		const FVector2D Lake1 = (Position - FVector2D(25640.0f, 72820.0f)) / FVector2D(4700.0f, 1900.0f);
		const FVector2D PondA = (Position - FVector2D(45457.0f, 136962.0f)) / FVector2D(3700.0f, 1700.0f);
		const FVector2D PondB = (Position - FVector2D(19220.0f, 124250.0f)) / FVector2D(3100.0f, 1600.0f);
		return FMath::Min(
			FMath::Min(Nearest, (Lake0.Size() - 1.0f) * 5400.0f),
			FMath::Min((Lake1.Size() - 1.0f) * 6000.0f, FMath::Min((PondA.Size() - 1.0f) * 3200.0f, (PondB.Size() - 1.0f) * 2800.0f)));
	}

	float GetNearestPathDistance(const FVector2D& Position)
	{
		float Nearest = TNumericLimits<float>::Max();
		auto CheckSegment = [&Nearest, &Position](const FVector2D& Start, const FVector2D& End)
		{
			Nearest = FMath::Min(Nearest, DistanceToSegment(Position, Start, End));
		};

		CheckSegment(FVector2D(18000.0f, 151500.0f), FVector2D(82000.0f, 151500.0f));
		CheckSegment(FVector2D(-10000.0f, 6500.0f), FVector2D(9000.0f, 9000.0f));
		CheckSegment(FVector2D(9000.0f, 9000.0f), FVector2D(36000.0f, 21000.0f));
		CheckSegment(FVector2D(36000.0f, 21000.0f), FVector2D(76000.0f, 32000.0f));
		CheckSegment(FVector2D(10000.0f, 36000.0f), FVector2D(12000.0f, 60000.0f));
		CheckSegment(FVector2D(12000.0f, 60000.0f), FVector2D(16000.0f, 87000.0f));
		const float SpawnLoopDistance = FMath::Abs(FVector2D::Distance(Position, FVector2D(-18000.0f, 4000.0f)) - 10500.0f);
		return FMath::Min(Nearest, SpawnLoopDistance);
	}

	float GetIslandOrganicValue(const FVector2D& Position)
	{
		const FVector2D Centered = (Position - FVector2D(71396.0f, 79714.0f)) / FVector2D(112000.0f, 124000.0f);
		return Centered.Size()
			+ 0.055f * FMath::Sin(Position.X / 11500.0f)
			- 0.045f * FMath::Cos(Position.Y / 9000.0f)
			+ 0.035f * FMath::Sin((Position.X + Position.Y) / 17000.0f);
	}

	float GetIslandInteriorMask(const FVector2D& Position)
	{
		return 1.0f - FMath::SmoothStep(0.66f, 0.78f, GetIslandOrganicValue(Position));
	}

	float GetIslandRimMask(const FVector2D& Position)
	{
		return FMath::SmoothStep(0.50f, 0.76f, GetIslandOrganicValue(Position));
	}

	float SoftHill(const FVector2D& Position, const FVector2D& Center, const FVector2D& Radius, const float Height)
	{
		const FVector2D Normalized = (Position - Center) / Radius;
		const float Alpha = FMath::Clamp(1.0f - Normalized.Size(), 0.0f, 1.0f);
		return Height * Alpha * Alpha * (3.0f - 2.0f * Alpha);
	}

	float SoftPlateau(const FVector2D& Position, const FVector2D& Center, const FVector2D& Radius, const float InnerRadius, const float Height)
	{
		const FVector2D Normalized = (Position - Center) / Radius;
		const float Distance = Normalized.Size();
		const float Alpha = 1.0f - FMath::SmoothStep(InnerRadius, 1.0f, Distance);
		return Height * FMath::Clamp(Alpha, 0.0f, 1.0f);
	}

	float SoftSegmentBand(const FVector2D& Position, const FVector2D& Start, const FVector2D& End, const float Width, const float Height)
	{
		const float Distance = DistanceToSegment(Position, Start, End);
		const float Alpha = 1.0f - FMath::SmoothStep(Width * 0.42f, Width, Distance);
		const float SmoothAlpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
		return Height * SmoothAlpha * SmoothAlpha * (3.0f - 2.0f * SmoothAlpha);
	}

	float HardCliffPlaneShift(
		const FVector2D& Position,
		const FVector2D& Start,
		const FVector2D& End,
		const float HalfWidth,
		const float EdgeFeather,
		const float WidthFeather,
		const float Height,
		const float SideSign)
	{
		const FVector2D Segment = End - Start;
		const float SegmentLength = FMath::Max(Segment.Size(), 1.0f);
		const FVector2D Direction = Segment / SegmentLength;
		const FVector2D Normal(-Direction.Y, Direction.X);
		const FVector2D Local = Position - Start;
		const float Along = FVector2D::DotProduct(Local, Direction);
		const float SignedSide = FVector2D::DotProduct(Local, Normal) * SideSign;
		const float AlongMask = FMath::SmoothStep(-HalfWidth * 0.35f, 0.0f, Along)
			* (1.0f - FMath::SmoothStep(SegmentLength, SegmentLength + HalfWidth * 0.35f, Along));
		const float WidthMask = 1.0f - FMath::SmoothStep(HalfWidth, HalfWidth + WidthFeather, FMath::Abs(SignedSide));
		const float FaceStep = FMath::SmoothStep(-EdgeFeather, EdgeFeather, SignedSide);
		return Height * ((FaceStep * 2.0f) - 1.0f) * AlongMask * FMath::Clamp(WidthMask, 0.0f, 1.0f);
	}

	float ComputeNaturalHeightDelta(const FVector2D& Position)
	{
		const float InteriorMask = GetIslandInteriorMask(Position);
		if (InteriorMask <= 0.0f)
		{
			return 0.0f;
		}

		const float WaterMask = FMath::SmoothStep(6500.0f, 12500.0f, GetNearestWaterDistance(Position));
		const float PathMask = FMath::SmoothStep(2600.0f, 7600.0f, GetNearestPathDistance(Position));
		const float SpawnDistance = static_cast<float>(FVector2D::Distance(Position, SpawnKeepFlatCenter));
		const float SpawnMask = FMath::SmoothStep(3600.0f, 9200.0f, SpawnDistance);
		const float ProtectedMask = InteriorMask * WaterMask * PathMask * SpawnMask;
		const float RimMask = GetIslandRimMask(Position);
		const float RimProtectedMask = RimMask * WaterMask * PathMask * SpawnMask;
		if (ProtectedMask <= 0.01f && RimProtectedMask <= 0.01f)
		{
			return 0.0f;
		}

		float Delta = 0.0f;
		Delta += FMath::PerlinNoise2D((Position + FVector2D(12000.0f, -9000.0f)) / 64000.0f) * 18.0f;
		Delta += FMath::PerlinNoise2D((Position + FVector2D(-4300.0f, 7100.0f)) / 28000.0f) * 10.0f;
		Delta += SoftHill(Position, FVector2D(42000.0f, 132000.0f), FVector2D(42000.0f, 30000.0f), 120.0f);
		Delta += SoftHill(Position, FVector2D(78000.0f, 112000.0f), FVector2D(46000.0f, 34000.0f), 95.0f);
		Delta += SoftHill(Position, FVector2D(32000.0f, 76000.0f), FVector2D(38000.0f, 28000.0f), 80.0f);
		Delta += SoftHill(Position, FVector2D(89000.0f, 65000.0f), FVector2D(38000.0f, 30000.0f), 75.0f);
		Delta += SoftHill(Position, FVector2D(52000.0f, 160500.0f), FVector2D(72000.0f, 33000.0f), -460.0f);
		Delta += SoftHill(Position, FVector2D(82000.0f, 151000.0f), FVector2D(42000.0f, 26000.0f), -210.0f);
		Delta += SoftHill(Position, FVector2D(26000.0f, 151500.0f), FVector2D(36000.0f, 24000.0f), -160.0f);

		float RimDelta = 0.0f;
		RimDelta += FMath::PerlinNoise2D((Position + FVector2D(8200.0f, 17100.0f)) / 10500.0f) * 95.0f;
		RimDelta += FMath::PerlinNoise2D((Position + FVector2D(-3100.0f, -6400.0f)) / 5200.0f) * 38.0f;
		return (Delta * ProtectedMask) + (RimDelta * RimProtectedMask);
	}

	float ComputePhase2PositiveHeightDelta(const FVector2D& Position)
	{
		const float InteriorMask = GetIslandInteriorMask(Position);
		if (InteriorMask <= 0.0f)
		{
			return 0.0f;
		}

		const float WaterMask = FMath::SmoothStep(7000.0f, 13500.0f, GetNearestWaterDistance(Position));
		const float PathMask = FMath::SmoothStep(2600.0f, 7800.0f, GetNearestPathDistance(Position));
		const float SpawnDistance = static_cast<float>(FVector2D::Distance(Position, SpawnKeepFlatCenter));
		const float SpawnMask = FMath::SmoothStep(7200.0f, 14500.0f, SpawnDistance);
		const float ProtectedMask = InteriorMask * WaterMask * PathMask * SpawnMask;
		if (ProtectedMask <= 0.01f)
		{
			return 0.0f;
		}

		float Delta = 0.0f;
		// A gentle future village overlook east of the central water, reachable by broad slopes.
		Delta += SoftHill(Position, FVector2D(76500.0f, 124000.0f), FVector2D(33000.0f, 26000.0f), 185.0f);
		Delta += SoftHill(Position, FVector2D(68000.0f, 116000.0f), FVector2D(42000.0f, 34000.0f), 95.0f);
		// Open meadow rises so the inner island feels raised and flowing rather than only carved down.
		Delta += SoftHill(Position, FVector2D(36000.0f, 106000.0f), FVector2D(36000.0f, 30000.0f), 92.0f);
		Delta += SoftHill(Position, FVector2D(92500.0f, 82000.0f), FVector2D(32000.0f, 28000.0f), 78.0f);
		Delta += SoftHill(Position, FVector2D(31000.0f, 56000.0f), FVector2D(30000.0f, 25000.0f), 66.0f);
		Delta += (0.5f + 0.5f * FMath::PerlinNoise2D((Position + FVector2D(9300.0f, -14100.0f)) / 54000.0f)) * 34.0f;
		Delta += FMath::PerlinNoise2D((Position + FVector2D(-7100.0f, 5100.0f)) / 26000.0f) * 14.0f;

		return Delta * ProtectedMask;
	}

	float ComputeTraversalReadabilityHeightDelta(const FVector2D& Position)
	{
		const float InteriorMask = GetIslandInteriorMask(Position);
		if (InteriorMask <= 0.0f)
		{
			return 0.0f;
		}

		// Keep authored water, visible paths, and the fixed spawn meadow stable while adding broad traversal-readable rolls.
		const float WaterMask = FMath::SmoothStep(10500.0f, 19000.0f, GetNearestWaterDistance(Position));
		const float PathMask = FMath::SmoothStep(3400.0f, 9200.0f, GetNearestPathDistance(Position));
		const float SpawnDistance = static_cast<float>(FVector2D::Distance(Position, SpawnKeepFlatCenter));
		const float SpawnMask = FMath::SmoothStep(13000.0f, 23500.0f, SpawnDistance);
		const float ProtectedMask = InteriorMask * WaterMask * PathMask * SpawnMask;
		if (ProtectedMask <= 0.01f)
		{
			return 0.0f;
		}

		float Delta = 0.0f;
		// Future village overlook: a readable, slightly raised grassland shelf with a broad approach.
		Delta += SoftHill(Position, FVector2D(76500.0f, 124000.0f), FVector2D(38000.0f, 28000.0f), 82.0f);
		Delta += SoftHill(Position, FVector2D(65000.0f, 116000.0f), FVector2D(46000.0f, 36000.0f), 38.0f);

		// Traversal/readability lines: long, soft rolls that guide movement without creating corridors.
		Delta += SoftHill(Position, FVector2D(48000.0f, 132000.0f), FVector2D(52000.0f, 24500.0f), 52.0f);
		Delta += SoftHill(Position, FVector2D(91000.0f, 88000.0f), FVector2D(36000.0f, 27500.0f), 44.0f);
		Delta += SoftHill(Position, FVector2D(33500.0f, 82000.0f), FVector2D(34500.0f, 27000.0f), 40.0f);
		Delta += SoftHill(Position, FVector2D(61000.0f, 56000.0f), FVector2D(30000.0f, 23500.0f), 32.0f);

		// Titan-like large-scale ground flow, deliberately low amplitude to avoid procedural spikes.
		Delta += FMath::PerlinNoise2D((Position + FVector2D(19100.0f, -8800.0f)) / 76000.0f) * 18.0f;
		Delta += FMath::PerlinNoise2D((Position + FVector2D(-6100.0f, 14200.0f)) / 39000.0f) * 8.0f;

		return FMath::Clamp(Delta, -18.0f, 96.0f) * ProtectedMask;
	}

	float ComputeGameplayFieldV632HeightDelta(const FVector2D& Position)
	{
		const float OrganicValue = GetIslandOrganicValue(Position);
		const float FieldInteriorMask = 1.0f - FMath::SmoothStep(0.47f, 0.61f, OrganicValue);
		if (FieldInteriorMask <= 0.0f)
		{
			return 0.0f;
		}

		const float WaterMask = FMath::SmoothStep(14500.0f, 25500.0f, GetNearestWaterDistance(Position));
		const float PathMask = FMath::SmoothStep(4300.0f, 11200.0f, GetNearestPathDistance(Position));
		const float SpawnDistance = static_cast<float>(FVector2D::Distance(Position, SpawnKeepFlatCenter));
		const float SpawnMask = FMath::SmoothStep(9800.0f, 18200.0f, SpawnDistance);
		const float ProtectedMask = FieldInteriorMask * WaterMask * PathMask * SpawnMask;
		if (ProtectedMask <= 0.01f)
		{
			return 0.0f;
		}

		float Delta = 0.0f;
		Delta += SoftHill(Position, FVector2D(54000.0f, 126000.0f), FVector2D(52000.0f, 31500.0f), 34.0f);
		Delta += SoftHill(Position, FVector2D(90000.0f, 108000.0f), FVector2D(39000.0f, 28000.0f), 28.0f);
		Delta += SoftHill(Position, FVector2D(33000.0f, 91000.0f), FVector2D(43000.0f, 33000.0f), 22.0f);
		Delta += SoftHill(Position, FVector2D(70500.0f, 76000.0f), FVector2D(36000.0f, 29500.0f), -24.0f);
		Delta += SoftHill(Position, FVector2D(44500.0f, 111500.0f), FVector2D(28500.0f, 23500.0f), -16.0f);
		Delta += FMath::PerlinNoise2D((Position + FVector2D(17100.0f, -9600.0f)) / 84000.0f) * 12.0f;
		Delta += FMath::PerlinNoise2D((Position + FVector2D(-6200.0f, 21800.0f)) / 46000.0f) * 5.0f;

		return FMath::Clamp(Delta, -32.0f, 46.0f) * ProtectedMask;
	}

	float ComputeTerrainFoundationV79HeightDelta(const FVector2D& Position)
	{
		const float OrganicValue = GetIslandOrganicValue(Position);
		const float InteriorMask = 1.0f - FMath::SmoothStep(0.58f, 0.76f, OrganicValue);
		if (InteriorMask <= 0.0f)
		{
			return 0.0f;
		}

		const float WaterDistance = GetNearestWaterDistance(Position);
		const float DirectWaterMask = FMath::SmoothStep(3200.0f, 7600.0f, WaterDistance);
		const float SpawnDistance = static_cast<float>(FVector2D::Distance(Position, SpawnKeepFlatCenter));
		const float SpawnMask = FMath::SmoothStep(7600.0f, 16400.0f, SpawnDistance);
		const float RouteSoftMask = 0.62f + 0.38f * FMath::SmoothStep(1800.0f, 7200.0f, GetNearestPathDistance(Position));
		const float ProtectedMask = InteriorMask * DirectWaterMask * SpawnMask * RouteSoftMask;
		if (ProtectedMask <= 0.01f)
		{
			return 0.0f;
		}

		float Delta = 0.0f;

		// V78 Variant A: safe mid terraces for village/training before the meadow drops into basin space.
		Delta += SoftHill(Position, FVector2D(76500.0f, 124000.0f), FVector2D(43000.0f, 30500.0f), 250.0f);
		Delta += SoftHill(Position, FVector2D(57000.0f, 143000.0f), FVector2D(35500.0f, 18500.0f), 155.0f);
		Delta += SoftHill(Position, FVector2D(93500.0f, 136000.0f), FVector2D(34000.0f, 22500.0f), 215.0f);

		// Keep the central open plains readable: broad, low-frequency rolls instead of clutter.
		Delta += SoftHill(Position, FVector2D(68000.0f, 109000.0f), FVector2D(56000.0f, 39500.0f), 72.0f);
		Delta += SoftHill(Position, FVector2D(90500.0f, 99000.0f), FVector2D(38500.0f, 31000.0f), -58.0f);
		Delta += SoftHill(Position, FVector2D(45500.0f, 107000.0f), FVector2D(36000.0f, 26500.0f), 48.0f);

		// River basin: lower the terrain around the existing water spine without editing water actors or the core water bed.
		const float RiverBasinBand = FMath::SmoothStep(5200.0f, 9200.0f, WaterDistance)
			* (1.0f - FMath::SmoothStep(19000.0f, 34500.0f, WaterDistance));
		Delta += -165.0f * RiverBasinBand * InteriorMask * SpawnMask;
		Delta += SoftSegmentBand(Position, FVector2D(51518.0f, 166778.0f), FVector2D(62544.0f, 138795.0f), 27500.0f, -70.0f);
		Delta += SoftSegmentBand(Position, FVector2D(58487.0f, 122723.0f), FVector2D(55306.0f, 96223.0f), 31500.0f, -108.0f);
		Delta += SoftSegmentBand(Position, FVector2D(55306.0f, 96223.0f), FVector2D(63147.0f, 37260.0f), 29000.0f, -128.0f);

		// Future forest floors: terrain reason only, no vegetation or forest assets.
		Delta += SoftHill(Position, FVector2D(60000.0f, 78000.0f), FVector2D(50000.0f, 36000.0f), -260.0f);
		Delta += SoftHill(Position, FVector2D(33500.0f, 84500.0f), FVector2D(32500.0f, 27500.0f), -104.0f);
		Delta += SoftHill(Position, FVector2D(87000.0f, 142500.0f), FVector2D(28000.0f, 18500.0f), -72.0f);

		// South exit / waterfall preparation: a broad descent and low basin, kept inside the approved gameplay footprint.
		Delta += SoftSegmentBand(Position, FVector2D(75500.0f, 43000.0f), FVector2D(63147.0f, 37260.0f), 22500.0f, -245.0f);
		Delta += SoftHill(Position, FVector2D(67500.0f, 40500.0f), FVector2D(27500.0f, 20500.0f), -210.0f);
		Delta += SoftHill(Position, FVector2D(89500.0f, 56000.0f), FVector2D(30000.0f, 24000.0f), 92.0f);

		// Event reserve pockets are terrain-only depressions/shelves for later content passes.
		Delta += SoftHill(Position, FVector2D(77500.0f, 110000.0f), FVector2D(22500.0f, 17000.0f), -42.0f);
		Delta += SoftHill(Position, FVector2D(68500.0f, 72000.0f), FVector2D(26500.0f, 19500.0f), -56.0f);
		Delta += SoftHill(Position, FVector2D(103000.0f, 126000.0f), FVector2D(21000.0f, 17500.0f), 58.0f);
		Delta += SoftHill(Position, FVector2D(74500.0f, 50500.0f), FVector2D(23000.0f, 16000.0f), -64.0f);

		Delta += FMath::PerlinNoise2D((Position + FVector2D(13800.0f, -19700.0f)) / 92000.0f) * 44.0f;
		Delta += FMath::PerlinNoise2D((Position + FVector2D(-23100.0f, 11800.0f)) / 52000.0f) * 18.0f;

		return FMath::Clamp(Delta, -520.0f, 380.0f) * ProtectedMask;
	}

	float ComputeTerrainFoundationV79RefineHeightDelta(const FVector2D& Position)
	{
		const float OrganicValue = GetIslandOrganicValue(Position);
		const float InteriorMask = 1.0f - FMath::SmoothStep(0.58f, 0.76f, OrganicValue);
		if (InteriorMask <= 0.0f)
		{
			return 0.0f;
		}

		const float WaterDistance = GetNearestWaterDistance(Position);
		const float DirectWaterMask = FMath::SmoothStep(4200.0f, 8800.0f, WaterDistance);
		const float SpawnDistance = static_cast<float>(FVector2D::Distance(Position, SpawnKeepFlatCenter));
		const float SpawnMask = FMath::SmoothStep(7200.0f, 15800.0f, SpawnDistance);
		const float RouteSoftMask = 0.72f + 0.28f * FMath::SmoothStep(1500.0f, 6400.0f, GetNearestPathDistance(Position));
		const float ProtectedMask = InteriorMask * DirectWaterMask * SpawnMask * RouteSoftMask;
		if (ProtectedMask <= 0.01f)
		{
			return 0.0f;
		}

		float Delta = 0.0f;
		Delta += SoftHill(Position, FVector2D(73000.0f, 121500.0f), FVector2D(33000.0f, 24500.0f), 178.0f);
		Delta += SoftHill(Position, FVector2D(65000.0f, 127000.0f), FVector2D(25500.0f, 18500.0f), 94.0f);
		Delta += SoftHill(Position, FVector2D(53000.0f, 145000.0f), FVector2D(30500.0f, 16000.0f), 210.0f);
		Delta += SoftHill(Position, FVector2D(46500.0f, 139000.0f), FVector2D(25000.0f, 15000.0f), 78.0f);
		Delta += SoftHill(Position, FVector2D(64000.0f, 132000.0f), FVector2D(38000.0f, 24500.0f), 82.0f);
		Delta += SoftHill(Position, FVector2D(73000.0f, 111500.0f), FVector2D(42000.0f, 27000.0f), 64.0f);
		Delta += SoftHill(Position, FVector2D(54500.0f, 114500.0f), FVector2D(31500.0f, 22000.0f), 46.0f);

		return FMath::Clamp(Delta, 0.0f, 260.0f) * ProtectedMask;
	}

	float ComputeTerrainFoundationV79ValidationRefineHeightDelta(const FVector2D& Position)
	{
		const float OrganicValue = GetIslandOrganicValue(Position);
		const float InteriorMask = 1.0f - FMath::SmoothStep(0.58f, 0.76f, OrganicValue);
		if (InteriorMask <= 0.0f)
		{
			return 0.0f;
		}

		const float WaterDistance = GetNearestWaterDistance(Position);
		const float DirectWaterMask = FMath::SmoothStep(3000.0f, 6800.0f, WaterDistance);
		const float SpawnDistance = static_cast<float>(FVector2D::Distance(Position, SpawnKeepFlatCenter));
		const float SpawnMask = FMath::SmoothStep(5600.0f, 12800.0f, SpawnDistance);
		const float RouteSoftMask = 0.78f + 0.22f * FMath::SmoothStep(1300.0f, 5600.0f, GetNearestPathDistance(Position));
		const float ProtectedMask = InteriorMask * DirectWaterMask * SpawnMask * RouteSoftMask;
		if (ProtectedMask <= 0.01f)
		{
			return 0.0f;
		}

		float Delta = 0.0f;

		// V79 validation correction: strengthen the early training shelf and approach line
		// after real screenshots showed the area still reading too flat.
		Delta += SoftHill(Position, FVector2D(52000.0f, 143500.0f), FVector2D(34500.0f, 17500.0f), 178.0f);
		Delta += SoftHill(Position, FVector2D(45500.0f, 136500.0f), FVector2D(23500.0f, 13500.0f), 72.0f);
		Delta += SoftHill(Position, FVector2D(61000.0f, 135000.0f), FVector2D(28000.0f, 16500.0f), 58.0f);

		// Open-plains readability: low, broad counter-shapes so the playable field is not a single flat sheet.
		Delta += SoftHill(Position, FVector2D(71000.0f, 106500.0f), FVector2D(47000.0f, 29000.0f), 82.0f);
		Delta += SoftHill(Position, FVector2D(88500.0f, 90000.0f), FVector2D(32000.0f, 22500.0f), -64.0f);
		Delta += SoftHill(Position, FVector2D(42000.0f, 95500.0f), FVector2D(30000.0f, 22500.0f), 46.0f);

		// River support only: shape banks and adjacent basin, not water actors or the protected water core.
		const float RiverEdgeMask = FMath::SmoothStep(6400.0f, 10500.0f, WaterDistance)
			* (1.0f - FMath::SmoothStep(18500.0f, 30500.0f, WaterDistance));
		Delta += -72.0f * RiverEdgeMask * InteriorMask * SpawnMask;
		Delta += SoftSegmentBand(Position, FVector2D(58487.0f, 122723.0f), FVector2D(55306.0f, 96223.0f), 24500.0f, -46.0f);
		Delta += SoftSegmentBand(Position, FVector2D(55306.0f, 96223.0f), FVector2D(57584.0f, 62136.0f), 23500.0f, -52.0f);

		// South gorge and forest-basin edge clarity: keep it broad, not crater-like.
		Delta += SoftHill(Position, FVector2D(64500.0f, 61000.0f), FVector2D(35000.0f, 23500.0f), -58.0f);
		Delta += SoftHill(Position, FVector2D(83500.0f, 57500.0f), FVector2D(28500.0f, 20500.0f), 46.0f);
		Delta += SoftHill(Position, FVector2D(68000.0f, 39500.0f), FVector2D(24000.0f, 17500.0f), -68.0f);

		return FMath::Clamp(Delta, -150.0f, 220.0f) * ProtectedMask;
	}

	float ComputeTerrainFoundationV80HeightDelta(const FVector2D& Position)
	{
		const float OrganicValue = GetIslandOrganicValue(Position);
		const float InteriorMask = 1.0f - FMath::SmoothStep(0.57f, 0.74f, OrganicValue);
		if (InteriorMask <= 0.0f)
		{
			return 0.0f;
		}

		const float WaterDistance = GetNearestWaterDistance(Position);
		const float DirectWaterMask = FMath::SmoothStep(3000.0f, 7200.0f, WaterDistance);
		const float SpawnDistance = static_cast<float>(FVector2D::Distance(Position, SpawnKeepFlatCenter));
		const float SpawnMask = FMath::SmoothStep(6000.0f, 13200.0f, SpawnDistance);
		const float RouteSoftMask = 0.86f + 0.14f * FMath::SmoothStep(1200.0f, 5200.0f, GetNearestPathDistance(Position));
		const float ProtectedMask = InteriorMask * DirectWaterMask * SpawnMask * RouteSoftMask;
		if (ProtectedMask <= 0.01f)
		{
			return 0.0f;
		}

		float Delta = 0.0f;

		// Shangri-La read: the starting settlement sits on a genuine elevated terrace, not a subtle mound.
		Delta += SoftPlateau(Position, FVector2D(76500.0f, 124000.0f), FVector2D(36000.0f, 25500.0f), 0.42f, 520.0f);
		Delta += SoftHill(Position, FVector2D(70500.0f, 117000.0f), FVector2D(47000.0f, 34000.0f), 170.0f);
		Delta += SoftSegmentBand(Position, FVector2D(56500.0f, 122000.0f), FVector2D(92500.0f, 116000.0f), 18500.0f, 118.0f);

		// Training plateau: a distinct early-game shelf separated from open plains.
		Delta += SoftPlateau(Position, FVector2D(52000.0f, 143500.0f), FVector2D(28500.0f, 15000.0f), 0.36f, 430.0f);
		Delta += SoftHill(Position, FVector2D(46000.0f, 136500.0f), FVector2D(25500.0f, 15000.0f), 135.0f);
		Delta += SoftHill(Position, FVector2D(62000.0f, 135000.0f), FVector2D(30000.0f, 17200.0f), 95.0f);

		// Open plains remain broad, but roll like a playable meadow rather than a flat arena.
		Delta += SoftHill(Position, FVector2D(72000.0f, 104500.0f), FVector2D(56500.0f, 34000.0f), 135.0f);
		Delta += SoftHill(Position, FVector2D(87500.0f, 92000.0f), FVector2D(35000.0f, 24000.0f), -100.0f);
		Delta += SoftHill(Position, FVector2D(43000.0f, 98000.0f), FVector2D(33000.0f, 23500.0f), 86.0f);
		Delta += SoftHill(Position, FVector2D(83500.0f, 111000.0f), FVector2D(26000.0f, 17000.0f), -58.0f);

		// River basin: lower the banks and surrounding approach so the water reads as older than the terrain.
		const float RiverEdgeBand = FMath::SmoothStep(5600.0f, 9600.0f, WaterDistance)
			* (1.0f - FMath::SmoothStep(18500.0f, 34000.0f, WaterDistance));
		Delta += -150.0f * RiverEdgeBand * InteriorMask * SpawnMask;
		Delta += SoftSegmentBand(Position, FVector2D(62544.0f, 138795.0f), FVector2D(58487.0f, 122723.0f), 27000.0f, -125.0f);
		Delta += SoftSegmentBand(Position, FVector2D(58487.0f, 122723.0f), FVector2D(55306.0f, 96223.0f), 29000.0f, -155.0f);
		Delta += SoftSegmentBand(Position, FVector2D(55306.0f, 96223.0f), FVector2D(57584.0f, 62136.0f), 27000.0f, -138.0f);

		// Main forest basin: a real low valley reserved for the later forest pass.
		Delta += SoftPlateau(Position, FVector2D(61000.0f, 76000.0f), FVector2D(47000.0f, 33000.0f), 0.25f, -360.0f);
		Delta += SoftHill(Position, FVector2D(41500.0f, 82000.0f), FVector2D(30500.0f, 24500.0f), -120.0f);
		Delta += SoftHill(Position, FVector2D(72500.0f, 72000.0f), FVector2D(31000.0f, 22000.0f), -112.0f);

		// Secondary forest shoulder: a small overlook/high shoulder, still open for later forest work.
		Delta += SoftPlateau(Position, FVector2D(93000.0f, 137000.0f), FVector2D(30500.0f, 19500.0f), 0.34f, 335.0f);
		Delta += SoftHill(Position, FVector2D(102000.0f, 124500.0f), FVector2D(24500.0f, 17000.0f), 145.0f);
		Delta += SoftHill(Position, FVector2D(85000.0f, 145000.0f), FVector2D(26000.0f, 16000.0f), -82.0f);

		// South gorge: make the future boss/waterfall route read as a descent.
		Delta += SoftSegmentBand(Position, FVector2D(57584.0f, 62136.0f), FVector2D(63147.0f, 37260.0f), 25500.0f, -300.0f);
		Delta += SoftHill(Position, FVector2D(67500.0f, 40500.0f), FVector2D(27000.0f, 19000.0f), -235.0f);
		Delta += SoftHill(Position, FVector2D(86500.0f, 56000.0f), FVector2D(30500.0f, 20500.0f), 155.0f);
		Delta += SoftHill(Position, FVector2D(47500.0f, 56000.0f), FVector2D(23000.0f, 17000.0f), 92.0f);

		// Low-frequency hand-built terrain noise, deliberately broad so it supports forms instead of making chatter.
		Delta += FMath::PerlinNoise2D((Position + FVector2D(13800.0f, -19700.0f)) / 108000.0f) * 62.0f;
		Delta += FMath::PerlinNoise2D((Position + FVector2D(-23100.0f, 11800.0f)) / 65000.0f) * 24.0f;

		return FMath::Clamp(Delta, -560.0f, 720.0f) * ProtectedMask;
	}

	float ComputeTerrainFoundationV80RefineHeightDelta(const FVector2D& Position)
	{
		const float OrganicValue = GetIslandOrganicValue(Position);
		const float InteriorMask = 1.0f - FMath::SmoothStep(0.57f, 0.74f, OrganicValue);
		if (InteriorMask <= 0.0f)
		{
			return 0.0f;
		}

		const float WaterDistance = GetNearestWaterDistance(Position);
		const float DirectWaterMask = FMath::SmoothStep(3000.0f, 7000.0f, WaterDistance);
		const float SpawnDistance = static_cast<float>(FVector2D::Distance(Position, SpawnKeepFlatCenter));
		const float SpawnMask = FMath::SmoothStep(6000.0f, 12800.0f, SpawnDistance);
		const float ProtectedMask = InteriorMask * DirectWaterMask * SpawnMask;
		if (ProtectedMask <= 0.01f)
		{
			return 0.0f;
		}

		float Delta = 0.0f;
		Delta += SoftPlateau(Position, FVector2D(76000.0f, 124500.0f), FVector2D(33000.0f, 22500.0f), 0.46f, 220.0f);
		Delta += SoftPlateau(Position, FVector2D(52000.0f, 143500.0f), FVector2D(25500.0f, 14000.0f), 0.42f, 210.0f);
		Delta += SoftHill(Position, FVector2D(72000.0f, 102000.0f), FVector2D(47000.0f, 28000.0f), 70.0f);
		Delta += SoftSegmentBand(Position, FVector2D(58487.0f, 122723.0f), FVector2D(55306.0f, 96223.0f), 25000.0f, -85.0f);
		Delta += SoftPlateau(Position, FVector2D(61000.0f, 76000.0f), FVector2D(42000.0f, 29500.0f), 0.28f, -155.0f);
		Delta += SoftSegmentBand(Position, FVector2D(57584.0f, 62136.0f), FVector2D(63147.0f, 37260.0f), 23500.0f, -140.0f);
		Delta += SoftPlateau(Position, FVector2D(93000.0f, 137000.0f), FVector2D(28500.0f, 17500.0f), 0.42f, 125.0f);

		return FMath::Clamp(Delta, -230.0f, 260.0f) * ProtectedMask;
	}

	float ComputeTerrainFoundationV81HeightDelta(const FVector2D& Position)
	{
		const float OrganicValue = GetIslandOrganicValue(Position);
		const float InteriorMask = 1.0f - FMath::SmoothStep(0.56f, 0.735f, OrganicValue);
		if (InteriorMask <= 0.0f)
		{
			return 0.0f;
		}

		const float WaterDistance = GetNearestWaterDistance(Position);
		const float DirectWaterMask = FMath::SmoothStep(2600.0f, 6500.0f, WaterDistance);
		const float SpawnDistance = static_cast<float>(FVector2D::Distance(Position, SpawnKeepFlatCenter));
		const float SpawnMask = FMath::SmoothStep(5200.0f, 11600.0f, SpawnDistance);
		const float RouteSoftMask = 0.9f + 0.1f * FMath::SmoothStep(900.0f, 4300.0f, GetNearestPathDistance(Position));
		const float ProtectedMask = InteriorMask * DirectWaterMask * SpawnMask * RouteSoftMask;
		if (ProtectedMask <= 0.01f)
		{
			return 0.0f;
		}

		float Delta = 0.0f;

		// V81: terrain must read in High/Mid/Low from the top-down without labels.
		// Village terrace: a real safe shelf that can later hold a starter settlement.
		Delta += SoftPlateau(Position, FVector2D(77000.0f, 124500.0f), FVector2D(40000.0f, 28000.0f), 0.50f, 900.0f);
		Delta += SoftSegmentBand(Position, FVector2D(56500.0f, 132500.0f), FVector2D(95500.0f, 121500.0f), 23500.0f, 255.0f);
		Delta += SoftHill(Position, FVector2D(70500.0f, 116000.0f), FVector2D(50000.0f, 34500.0f), 185.0f);
		Delta += SoftSegmentBand(Position, FVector2D(62544.0f, 138795.0f), FVector2D(58487.0f, 122723.0f), 23000.0f, -245.0f);

		// Training plateau: distinct early gameplay feature connected to the village shelf.
		Delta += SoftPlateau(Position, FVector2D(52000.0f, 143500.0f), FVector2D(31000.0f, 16500.0f), 0.47f, 760.0f);
		Delta += SoftSegmentBand(Position, FVector2D(50000.0f, 142000.0f), FVector2D(72500.0f, 128000.0f), 15500.0f, 210.0f);
		Delta += SoftHill(Position, FVector2D(43500.0f, 136000.0f), FVector2D(24500.0f, 15000.0f), 130.0f);

		// Open plains: keep the readable meadow size while adding broad route-scale waves.
		Delta += SoftHill(Position, FVector2D(72000.0f, 104000.0f), FVector2D(58500.0f, 36000.0f), 230.0f);
		Delta += SoftHill(Position, FVector2D(90000.0f, 92000.0f), FVector2D(36000.0f, 24500.0f), -185.0f);
		Delta += SoftHill(Position, FVector2D(43000.0f, 100000.0f), FVector2D(33500.0f, 24000.0f), 130.0f);
		Delta += SoftHill(Position, FVector2D(79500.0f, 111000.0f), FVector2D(28500.0f, 17500.0f), -90.0f);

		// River basin: sculpt low banks and adjacent basin; do not touch water actors or water-system logic.
		const float RiverEdgeBand = FMath::SmoothStep(5200.0f, 9000.0f, WaterDistance)
			* (1.0f - FMath::SmoothStep(17000.0f, 33000.0f, WaterDistance));
		Delta += -230.0f * RiverEdgeBand * InteriorMask * SpawnMask;
		Delta += SoftSegmentBand(Position, FVector2D(62544.0f, 138795.0f), FVector2D(58487.0f, 122723.0f), 26500.0f, -195.0f);
		Delta += SoftSegmentBand(Position, FVector2D(58487.0f, 122723.0f), FVector2D(55306.0f, 96223.0f), 30000.0f, -245.0f);
		Delta += SoftSegmentBand(Position, FVector2D(55306.0f, 96223.0f), FVector2D(57584.0f, 62136.0f), 28500.0f, -220.0f);

		// Forest basin: a real lowland bowl reserved for future forest ecology.
		Delta += SoftPlateau(Position, FVector2D(61000.0f, 76000.0f), FVector2D(49000.0f, 34500.0f), 0.36f, -620.0f);
		Delta += SoftHill(Position, FVector2D(45500.0f, 87000.0f), FVector2D(31000.0f, 25000.0f), -220.0f);
		Delta += SoftHill(Position, FVector2D(74500.0f, 70000.0f), FVector2D(32500.0f, 22500.0f), -185.0f);
		Delta += SoftHill(Position, FVector2D(34500.0f, 73500.0f), FVector2D(23000.0f, 17500.0f), 105.0f);

		// Secondary forest shoulder: high exploration edge overlooking the lower forest basin.
		Delta += SoftPlateau(Position, FVector2D(93000.0f, 137000.0f), FVector2D(33000.0f, 20500.0f), 0.50f, 560.0f);
		Delta += SoftHill(Position, FVector2D(103000.0f, 124000.0f), FVector2D(26000.0f, 17200.0f), 245.0f);
		Delta += SoftHill(Position, FVector2D(85500.0f, 145500.0f), FVector2D(26000.0f, 16000.0f), -135.0f);

		// South gorge: obvious descent/funnel for future boss and waterfall route.
		Delta += SoftSegmentBand(Position, FVector2D(57584.0f, 62136.0f), FVector2D(63147.0f, 37260.0f), 27500.0f, -560.0f);
		Delta += SoftPlateau(Position, FVector2D(67500.0f, 40500.0f), FVector2D(28000.0f, 19500.0f), 0.28f, -420.0f);
		Delta += SoftHill(Position, FVector2D(87000.0f, 56500.0f), FVector2D(31500.0f, 21500.0f), 260.0f);
		Delta += SoftHill(Position, FVector2D(46500.0f, 56500.0f), FVector2D(24500.0f, 17500.0f), 150.0f);

		Delta += FMath::PerlinNoise2D((Position + FVector2D(13800.0f, -19700.0f)) / 112000.0f) * 75.0f;
		Delta += FMath::PerlinNoise2D((Position + FVector2D(-23100.0f, 11800.0f)) / 66000.0f) * 28.0f;

		return FMath::Clamp(Delta, -920.0f, 1120.0f) * ProtectedMask;
	}

	float ComputeTerrainFoundationV81RefineHeightDelta(const FVector2D& Position)
	{
		const float OrganicValue = GetIslandOrganicValue(Position);
		const float InteriorMask = 1.0f - FMath::SmoothStep(0.56f, 0.735f, OrganicValue);
		if (InteriorMask <= 0.0f)
		{
			return 0.0f;
		}

		const float WaterDistance = GetNearestWaterDistance(Position);
		const float DirectWaterMask = FMath::SmoothStep(2600.0f, 6400.0f, WaterDistance);
		const float SpawnDistance = static_cast<float>(FVector2D::Distance(Position, SpawnKeepFlatCenter));
		const float SpawnMask = FMath::SmoothStep(5200.0f, 11400.0f, SpawnDistance);
		const float ProtectedMask = InteriorMask * DirectWaterMask * SpawnMask;
		if (ProtectedMask <= 0.01f)
		{
			return 0.0f;
		}

		float Delta = 0.0f;
		Delta += SoftPlateau(Position, FVector2D(77000.0f, 124500.0f), FVector2D(35500.0f, 24500.0f), 0.55f, 360.0f);
		Delta += SoftSegmentBand(Position, FVector2D(62544.0f, 138795.0f), FVector2D(58487.0f, 122723.0f), 22000.0f, -160.0f);
		Delta += SoftPlateau(Position, FVector2D(52000.0f, 143500.0f), FVector2D(27000.0f, 14500.0f), 0.52f, 330.0f);
		Delta += SoftHill(Position, FVector2D(70500.0f, 103500.0f), FVector2D(47000.0f, 28500.0f), 90.0f);
		Delta += SoftSegmentBand(Position, FVector2D(58487.0f, 122723.0f), FVector2D(55306.0f, 96223.0f), 25500.0f, -145.0f);
		Delta += SoftPlateau(Position, FVector2D(61000.0f, 76000.0f), FVector2D(42000.0f, 30000.0f), 0.40f, -260.0f);
		Delta += SoftPlateau(Position, FVector2D(93000.0f, 137000.0f), FVector2D(29000.0f, 18000.0f), 0.52f, 185.0f);
		Delta += SoftSegmentBand(Position, FVector2D(57584.0f, 62136.0f), FVector2D(63147.0f, 37260.0f), 24500.0f, -230.0f);
		Delta += SoftHill(Position, FVector2D(87000.0f, 56500.0f), FVector2D(28000.0f, 19000.0f), 95.0f);

		return FMath::Clamp(Delta, -360.0f, 420.0f) * ProtectedMask;
	}

	float ComputeTerrainFoundationV81FinalHeightDelta(const FVector2D& Position)
	{
		const float OrganicValue = GetIslandOrganicValue(Position);
		const float InteriorMask = 1.0f - FMath::SmoothStep(0.55f, 0.735f, OrganicValue);
		if (InteriorMask <= 0.0f)
		{
			return 0.0f;
		}

		const float WaterDistance = GetNearestWaterDistance(Position);
		const float DirectWaterMask = FMath::SmoothStep(3000.0f, 7600.0f, WaterDistance);
		const float SpawnDistance = static_cast<float>(FVector2D::Distance(Position, SpawnKeepFlatCenter));
		const float SpawnMask = FMath::SmoothStep(6200.0f, 14200.0f, SpawnDistance);
		const float RouteSoftMask = 0.88f + 0.12f * FMath::SmoothStep(1200.0f, 5200.0f, GetNearestPathDistance(Position));
		const float ProtectedMask = InteriorMask * DirectWaterMask * SpawnMask * RouteSoftMask;
		if (ProtectedMask <= 0.01f)
		{
			return 0.0f;
		}

		float Delta = 0.0f;

		// Final V81 correction: terrain forms must read without labels at gameplay/top-down scale.
		Delta += SoftPlateau(Position, FVector2D(77200.0f, 124500.0f), FVector2D(42000.0f, 29500.0f), 0.58f, 1450.0f);
		Delta += SoftSegmentBand(Position, FVector2D(56500.0f, 133000.0f), FVector2D(96500.0f, 121500.0f), 25000.0f, 390.0f);
		Delta += SoftSegmentBand(Position, FVector2D(63000.0f, 137500.0f), FVector2D(58500.0f, 122500.0f), 22000.0f, -420.0f);

		Delta += SoftPlateau(Position, FVector2D(52500.0f, 143500.0f), FVector2D(32500.0f, 17000.0f), 0.55f, 1120.0f);
		Delta += SoftSegmentBand(Position, FVector2D(50000.0f, 142000.0f), FVector2D(73000.0f, 128000.0f), 17000.0f, 310.0f);
		Delta += SoftHill(Position, FVector2D(43500.0f, 136000.0f), FVector2D(25500.0f, 15500.0f), 175.0f);

		Delta += SoftHill(Position, FVector2D(72000.0f, 104000.0f), FVector2D(62000.0f, 38500.0f), 410.0f);
		Delta += SoftHill(Position, FVector2D(90500.0f, 92000.0f), FVector2D(38000.0f, 25500.0f), -360.0f);
		Delta += SoftHill(Position, FVector2D(43000.0f, 100000.0f), FVector2D(36000.0f, 25500.0f), 240.0f);
		Delta += SoftHill(Position, FVector2D(79000.0f, 112000.0f), FVector2D(30000.0f, 18500.0f), -180.0f);

		const float RiverEdgeBand = FMath::SmoothStep(6200.0f, 11000.0f, WaterDistance)
			* (1.0f - FMath::SmoothStep(18500.0f, 36000.0f, WaterDistance));
		Delta += -520.0f * RiverEdgeBand * InteriorMask * SpawnMask;
		Delta += SoftSegmentBand(Position, FVector2D(62544.0f, 138795.0f), FVector2D(58487.0f, 122723.0f), 28500.0f, -420.0f);
		Delta += SoftSegmentBand(Position, FVector2D(58487.0f, 122723.0f), FVector2D(55306.0f, 96223.0f), 31500.0f, -560.0f);
		Delta += SoftSegmentBand(Position, FVector2D(55306.0f, 96223.0f), FVector2D(57584.0f, 62136.0f), 30000.0f, -500.0f);

		Delta += SoftPlateau(Position, FVector2D(61000.0f, 76000.0f), FVector2D(51000.0f, 36000.0f), 0.44f, -1100.0f);
		Delta += SoftHill(Position, FVector2D(45500.0f, 87000.0f), FVector2D(32500.0f, 26000.0f), -390.0f);
		Delta += SoftHill(Position, FVector2D(74500.0f, 70000.0f), FVector2D(34000.0f, 23500.0f), -330.0f);
		Delta += SoftHill(Position, FVector2D(34500.0f, 73500.0f), FVector2D(24000.0f, 18000.0f), 210.0f);

		Delta += SoftPlateau(Position, FVector2D(93000.0f, 137000.0f), FVector2D(34500.0f, 21500.0f), 0.56f, 780.0f);
		Delta += SoftHill(Position, FVector2D(103000.0f, 124000.0f), FVector2D(27000.0f, 18200.0f), 320.0f);
		Delta += SoftHill(Position, FVector2D(85500.0f, 145500.0f), FVector2D(27000.0f, 16500.0f), -230.0f);

		Delta += SoftSegmentBand(Position, FVector2D(57584.0f, 62136.0f), FVector2D(63147.0f, 37260.0f), 29000.0f, -1250.0f);
		Delta += SoftPlateau(Position, FVector2D(67500.0f, 40500.0f), FVector2D(29500.0f, 20500.0f), 0.34f, -820.0f);
		Delta += SoftHill(Position, FVector2D(87000.0f, 56500.0f), FVector2D(33000.0f, 22500.0f), 430.0f);
		Delta += SoftHill(Position, FVector2D(46500.0f, 56500.0f), FVector2D(25500.0f, 18500.0f), 260.0f);

		Delta += FMath::PerlinNoise2D((Position + FVector2D(13800.0f, -19700.0f)) / 120000.0f) * 95.0f;
		Delta += FMath::PerlinNoise2D((Position + FVector2D(-23100.0f, 11800.0f)) / 72000.0f) * 36.0f;

		return FMath::Clamp(Delta, -1580.0f, 1720.0f) * ProtectedMask;
	}

	float ComputeAAATraversalTopographyV83HeightDelta(const FVector2D& Position)
	{
		const float OrganicValue = GetIslandOrganicValue(Position);
		const float InteriorMask = 1.0f - FMath::SmoothStep(0.545f, 0.735f, OrganicValue);
		if (InteriorMask <= 0.0f)
		{
			return 0.0f;
		}

		const float WaterDistance = GetNearestWaterDistance(Position);
		const float WaterCoreMask = FMath::SmoothStep(3500.0f, 7800.0f, WaterDistance);
		const float SpawnDistance = static_cast<float>(FVector2D::Distance(Position, SpawnKeepFlatCenter));
		const float SpawnMask = FMath::SmoothStep(6200.0f, 13200.0f, SpawnDistance);
		const float OuterRingGuard = 1.0f - FMath::SmoothStep(0.62f, 0.735f, OrganicValue);
		const float ProtectedMask = InteriorMask * WaterCoreMask * SpawnMask * OuterRingGuard;
		if (ProtectedMask <= 0.01f)
		{
			return 0.0f;
		}

		float Delta = 0.0f;

		// Macro route grammar: terrace -> plateau -> plains -> basin -> gorge.
		Delta += SoftPlateau(Position, FVector2D(77500.0f, 124000.0f), FVector2D(38500.0f, 26500.0f), 0.54f, 980.0f);
		Delta += SoftPlateau(Position, FVector2D(81200.0f, 130500.0f), FVector2D(24500.0f, 15000.0f), 0.42f, 520.0f);
		Delta += SoftSegmentBand(Position, FVector2D(56000.0f, 133500.0f), FVector2D(90000.0f, 119500.0f), 20500.0f, 390.0f);

		Delta += SoftPlateau(Position, FVector2D(52300.0f, 143400.0f), FVector2D(30000.0f, 15800.0f), 0.52f, 780.0f);
		Delta += SoftHill(Position, FVector2D(46200.0f, 136700.0f), FVector2D(22000.0f, 13000.0f), 350.0f);

		Delta += SoftHill(Position, FVector2D(70000.0f, 105000.0f), FVector2D(57000.0f, 35000.0f), 270.0f);
		Delta += SoftHill(Position, FVector2D(41000.0f, 108000.0f), FVector2D(32000.0f, 22000.0f), 230.0f);
		Delta += SoftHill(Position, FVector2D(89000.0f, 92500.0f), FVector2D(32000.0f, 22500.0f), -360.0f);

		// River valley and forest basin: lower terrain around the authored water spine without touching the water core.
		const float RiverBankMask = FMath::SmoothStep(7000.0f, 13000.0f, WaterDistance)
			* (1.0f - FMath::SmoothStep(18000.0f, 36000.0f, WaterDistance));
		Delta += -560.0f * RiverBankMask * InteriorMask * SpawnMask;
		Delta += SoftSegmentBand(Position, FVector2D(62544.0f, 138795.0f), FVector2D(58487.0f, 122723.0f), 26000.0f, -360.0f);
		Delta += SoftSegmentBand(Position, FVector2D(58487.0f, 122723.0f), FVector2D(55306.0f, 96223.0f), 29500.0f, -620.0f);
		Delta += SoftSegmentBand(Position, FVector2D(55306.0f, 96223.0f), FVector2D(57584.0f, 62136.0f), 28000.0f, -680.0f);

		Delta += SoftPlateau(Position, FVector2D(61000.0f, 76000.0f), FVector2D(47500.0f, 33500.0f), 0.40f, -1180.0f);
		Delta += SoftHill(Position, FVector2D(45500.0f, 87000.0f), FVector2D(30000.0f, 23000.0f), -520.0f);
		Delta += SoftHill(Position, FVector2D(76000.0f, 71000.0f), FVector2D(30000.0f, 21000.0f), -410.0f);

		// Compression corridor: raised flanks around the open-plains to forest-basin approach.
		Delta += SoftSegmentBand(Position, FVector2D(48500.0f, 112000.0f), FVector2D(50000.0f, 80500.0f), 13200.0f, 520.0f);
		Delta += SoftSegmentBand(Position, FVector2D(77000.0f, 111500.0f), FVector2D(78500.0f, 80000.0f), 14200.0f, 610.0f);
		Delta += SoftSegmentBand(Position, FVector2D(57500.0f, 106000.0f), FVector2D(67500.0f, 81500.0f), 11800.0f, -520.0f);

		// Secondary shoulder / overlook and future event pocket C.
		Delta += SoftPlateau(Position, FVector2D(93000.0f, 137000.0f), FVector2D(31500.0f, 19500.0f), 0.52f, 760.0f);
		Delta += SoftHill(Position, FVector2D(102500.0f, 125500.0f), FVector2D(25000.0f, 17000.0f), 540.0f);
		Delta += SoftHill(Position, FVector2D(85000.0f, 145000.0f), FVector2D(23500.0f, 14500.0f), -260.0f);

		// South gorge: stronger descent while preserving the exit corridor.
		Delta += SoftSegmentBand(Position, FVector2D(57584.0f, 62136.0f), FVector2D(63147.0f, 37260.0f), 27000.0f, -1450.0f);
		Delta += SoftPlateau(Position, FVector2D(67500.0f, 40500.0f), FVector2D(28000.0f, 19000.0f), 0.34f, -980.0f);
		Delta += SoftHill(Position, FVector2D(87500.0f, 56000.0f), FVector2D(30000.0f, 20500.0f), 620.0f);
		Delta += SoftHill(Position, FVector2D(47000.0f, 56500.0f), FVector2D(24000.0f, 17000.0f), 430.0f);

		// Future public-event pockets: terrain-only bowls with readable rims.
		Delta += SoftPlateau(Position, FVector2D(58500.0f, 131000.0f), FVector2D(15000.0f, 10500.0f), 0.45f, -280.0f);
		Delta += SoftPlateau(Position, FVector2D(71000.0f, 80500.0f), FVector2D(17500.0f, 12500.0f), 0.42f, -330.0f);
		Delta += SoftPlateau(Position, FVector2D(99500.0f, 132500.0f), FVector2D(14500.0f, 9500.0f), 0.42f, -220.0f);
		Delta += SoftPlateau(Position, FVector2D(73500.0f, 47000.0f), FVector2D(15500.0f, 10500.0f), 0.40f, -360.0f);

		Delta += FMath::PerlinNoise2D((Position + FVector2D(15100.0f, -20500.0f)) / 98000.0f) * 80.0f;
		Delta += FMath::PerlinNoise2D((Position + FVector2D(-24000.0f, 9700.0f)) / 42000.0f) * 45.0f;

		return FMath::Clamp(Delta, -1850.0f, 1560.0f) * ProtectedMask;
	}

	float ComputeAAATraversalTopographyV83RefineHeightDelta(const FVector2D& Position)
	{
		const float OrganicValue = GetIslandOrganicValue(Position);
		const float InteriorMask = 1.0f - FMath::SmoothStep(0.545f, 0.735f, OrganicValue);
		if (InteriorMask <= 0.0f)
		{
			return 0.0f;
		}

		const float WaterDistance = GetNearestWaterDistance(Position);
		const float WaterCoreMask = FMath::SmoothStep(3600.0f, 8200.0f, WaterDistance);
		const float SpawnDistance = static_cast<float>(FVector2D::Distance(Position, SpawnKeepFlatCenter));
		const float SpawnMask = FMath::SmoothStep(5800.0f, 12600.0f, SpawnDistance);
		const float OuterRingGuard = 1.0f - FMath::SmoothStep(0.62f, 0.735f, OrganicValue);
		const float ProtectedMask = InteriorMask * WaterCoreMask * SpawnMask * OuterRingGuard;
		if (ProtectedMask <= 0.01f)
		{
			return 0.0f;
		}

		float Delta = 0.0f;

		// Strengthen the authored traversal reads after the macro pass without creating a new terrain system.
		Delta += SoftPlateau(Position, FVector2D(78400.0f, 124500.0f), FVector2D(30000.0f, 20500.0f), 0.58f, 430.0f);
		Delta += SoftSegmentBand(Position, FVector2D(61000.0f, 132500.0f), FVector2D(92500.0f, 121500.0f), 15000.0f, 250.0f);
		Delta += SoftPlateau(Position, FVector2D(52500.0f, 143500.0f), FVector2D(25000.0f, 13200.0f), 0.56f, 360.0f);

		// Corridor side walls and a lower reveal line into the forest basin.
		Delta += SoftSegmentBand(Position, FVector2D(50000.0f, 111500.0f), FVector2D(50500.0f, 82000.0f), 10000.0f, 330.0f);
		Delta += SoftSegmentBand(Position, FVector2D(77000.0f, 110500.0f), FVector2D(77000.0f, 80500.0f), 11200.0f, 370.0f);
		Delta += SoftSegmentBand(Position, FVector2D(58500.0f, 104000.0f), FVector2D(66500.0f, 80000.0f), 9300.0f, -360.0f);
		Delta += SoftPlateau(Position, FVector2D(61000.0f, 76000.0f), FVector2D(39500.0f, 28000.0f), 0.46f, -460.0f);

		// River banks and future crossing shelves: readable valley, protected water core.
		const float RiverShelfMask = FMath::SmoothStep(8200.0f, 12800.0f, WaterDistance)
			* (1.0f - FMath::SmoothStep(16000.0f, 27000.0f, WaterDistance));
		Delta += -240.0f * RiverShelfMask * InteriorMask * SpawnMask;
		Delta += SoftHill(Position, FVector2D(66500.0f, 119000.0f), FVector2D(17000.0f, 11200.0f), 230.0f);
		Delta += SoftHill(Position, FVector2D(52000.0f, 116500.0f), FVector2D(16000.0f, 11200.0f), 170.0f);

		// Overlook and south descent readability.
		Delta += SoftPlateau(Position, FVector2D(93000.0f, 137000.0f), FVector2D(26000.0f, 16000.0f), 0.58f, 330.0f);
		Delta += SoftSegmentBand(Position, FVector2D(57584.0f, 62136.0f), FVector2D(63147.0f, 37260.0f), 22500.0f, -620.0f);
		Delta += SoftHill(Position, FVector2D(87500.0f, 56000.0f), FVector2D(24500.0f, 17000.0f), 300.0f);
		Delta += SoftHill(Position, FVector2D(48200.0f, 55500.0f), FVector2D(20500.0f, 14200.0f), 250.0f);

		// Meso undulation only where it supports foot-level handmade terrain.
		Delta += FMath::PerlinNoise2D((Position + FVector2D(7500.0f, -13600.0f)) / 36000.0f) * 55.0f;
		Delta += FMath::PerlinNoise2D((Position + FVector2D(-17800.0f, 12300.0f)) / 21000.0f) * 24.0f;

		return FMath::Clamp(Delta, -760.0f, 620.0f) * ProtectedMask;
	}

	float ComputeTraversalReadabilityV2HeightDelta(const FVector2D& Position)
	{
		const float InteriorMask = GetIslandInteriorMask(Position);
		if (InteriorMask <= 0.0f)
		{
			return 0.0f;
		}

		// Titan-style traversal terrain reads in broad silhouettes: keep water, paths,
		// spawn, and authored clearings stable while adding soft elevated reads.
		const float WaterMask = FMath::SmoothStep(12000.0f, 21500.0f, GetNearestWaterDistance(Position));
		const float PathMask = FMath::SmoothStep(4300.0f, 11200.0f, GetNearestPathDistance(Position));
		const float SpawnDistance = static_cast<float>(FVector2D::Distance(Position, SpawnKeepFlatCenter));
		const float SpawnMask = FMath::SmoothStep(15500.0f, 28000.0f, SpawnDistance);
		const float ProtectedMask = InteriorMask * WaterMask * PathMask * SpawnMask;
		if (ProtectedMask <= 0.01f)
		{
			return 0.0f;
		}

		float Delta = 0.0f;
		// Future village shelf: a slightly clearer overlook without flattening the hand-made layout.
		Delta += SoftHill(Position, FVector2D(76500.0f, 124000.0f), FVector2D(42000.0f, 31000.0f), 58.0f);
		Delta += SoftHill(Position, FVector2D(68500.0f, 116000.0f), FVector2D(50000.0f, 38000.0f), 26.0f);

		// Movement-readable rolls: broad, low-amplitude ridges that imply glide/slide lines later.
		Delta += SoftHill(Position, FVector2D(57000.0f, 143000.0f), FVector2D(42000.0f, 21000.0f), 36.0f);
		Delta += SoftHill(Position, FVector2D(93500.0f, 118000.0f), FVector2D(33000.0f, 25500.0f), 42.0f);
		Delta += SoftHill(Position, FVector2D(54500.0f, 92000.0f), FVector2D(41000.0f, 31000.0f), 28.0f);
		Delta += SoftHill(Position, FVector2D(39500.0f, 108000.0f), FVector2D(31500.0f, 23500.0f), 22.0f);

		// Low frequency only: enough macro flow to kill blockout flatness, not enough to create spikes.
		Delta += FMath::PerlinNoise2D((Position + FVector2D(11200.0f, -17100.0f)) / 94000.0f) * 10.0f;
		Delta += FMath::PerlinNoise2D((Position + FVector2D(-21000.0f, 6200.0f)) / 62000.0f) * 5.0f;

		return FMath::Clamp(Delta, -10.0f, 74.0f) * ProtectedMask;
	}

	float ComputeTraversalReadabilityV3HeightDelta(const FVector2D& Position)
	{
		const float InteriorMask = GetIslandInteriorMask(Position);
		if (InteriorMask <= 0.0f)
		{
			return 0.0f;
		}

		// V3 keeps the authored island intact and only adds broad readable terrain language:
		// forest-framing shoulders, a clearer future village shelf, and open meadow traversal lines.
		const float WaterMask = FMath::SmoothStep(13500.0f, 23500.0f, GetNearestWaterDistance(Position));
		const float PathMask = FMath::SmoothStep(5200.0f, 12500.0f, GetNearestPathDistance(Position));
		const float SpawnDistance = static_cast<float>(FVector2D::Distance(Position, SpawnKeepFlatCenter));
		const float SpawnMask = FMath::SmoothStep(18000.0f, 31500.0f, SpawnDistance);
		const float ProtectedMask = InteriorMask * WaterMask * PathMask * SpawnMask;
		if (ProtectedMask <= 0.01f)
		{
			return 0.0f;
		}

		float Delta = 0.0f;
		// Village plateau preparation: more readable as an overlook, still approached by soft grades.
		Delta += SoftHill(Position, FVector2D(76800.0f, 124500.0f), FVector2D(34500.0f, 25500.0f), 42.0f);
		Delta += SoftHill(Position, FVector2D(64200.0f, 117000.0f), FVector2D(46500.0f, 34500.0f), 18.0f);

		// Forest edge terrain: tiny shoulders/gaps so tree masses frame routes instead of forming a flat wall.
		Delta += SoftHill(Position, FVector2D(91500.0f, 139500.0f), FVector2D(27500.0f, 14500.0f), 28.0f);
		Delta += SoftHill(Position, FVector2D(82200.0f, 136500.0f), FVector2D(14500.0f, 9500.0f), -16.0f);
		Delta += SoftHill(Position, FVector2D(61000.0f, 157500.0f), FVector2D(24000.0f, 8500.0f), -10.0f);

		// Traversal readability: long, low-amplitude lines visible from player height.
		Delta += SoftHill(Position, FVector2D(53500.0f, 135000.0f), FVector2D(45500.0f, 19000.0f), 24.0f);
		Delta += SoftHill(Position, FVector2D(96000.0f, 102000.0f), FVector2D(32000.0f, 23000.0f), 22.0f);
		Delta += FMath::PerlinNoise2D((Position + FVector2D(31000.0f, -7300.0f)) / 118000.0f) * 7.0f;

		return FMath::Clamp(Delta, -18.0f, 48.0f) * ProtectedMask;
	}

	float ComputeTraversalReadabilityV4HeightDelta(const FVector2D& Position)
	{
		const float InteriorMask = GetIslandInteriorMask(Position);
		if (InteriorMask <= 0.0f)
		{
			return 0.0f;
		}

		// V4 is a readable-flow refinement over the existing authored terrain:
		// low-amplitude shoulders and meadow sags, not a rebuild.
		const float WaterMask = FMath::SmoothStep(15000.0f, 26000.0f, GetNearestWaterDistance(Position));
		const float PathMask = FMath::SmoothStep(6400.0f, 14500.0f, GetNearestPathDistance(Position));
		const float SpawnDistance = static_cast<float>(FVector2D::Distance(Position, SpawnKeepFlatCenter));
		const float SpawnMask = FMath::SmoothStep(20500.0f, 34000.0f, SpawnDistance);
		const float ProtectedMask = InteriorMask * WaterMask * PathMask * SpawnMask;
		if (ProtectedMask <= 0.01f)
		{
			return 0.0f;
		}

		float Delta = 0.0f;
		Delta += SoftHill(Position, FVector2D(77000.0f, 124500.0f), FVector2D(31000.0f, 22000.0f), 24.0f);
		Delta += SoftHill(Position, FVector2D(61000.0f, 112000.0f), FVector2D(39000.0f, 30000.0f), 12.0f);
		Delta += SoftHill(Position, FVector2D(92500.0f, 139500.0f), FVector2D(25000.0f, 12500.0f), 16.0f);
		Delta += SoftHill(Position, FVector2D(87500.0f, 132500.0f), FVector2D(11800.0f, 8200.0f), -12.0f);
		Delta += SoftHill(Position, FVector2D(72500.0f, 156500.0f), FVector2D(18500.0f, 7200.0f), -8.0f);
		Delta += SoftHill(Position, FVector2D(52500.0f, 137500.0f), FVector2D(50000.0f, 16500.0f), 14.0f);
		Delta += SoftHill(Position, FVector2D(94000.0f, 101000.0f), FVector2D(29500.0f, 21500.0f), 13.0f);
		Delta += FMath::PerlinNoise2D((Position + FVector2D(42000.0f, -18000.0f)) / 135000.0f) * 4.0f;

		return FMath::Clamp(Delta, -12.0f, 30.0f) * ProtectedMask;
	}

	float ComputeTraversalReadabilityV5HeightDelta(const FVector2D& Position)
	{
		const float InteriorMask = GetIslandInteriorMask(Position);
		if (InteriorMask <= 0.0f)
		{
			return 0.0f;
		}

		// V5 follows Titan's readable traversal rhythm: a few layered shelves and
		// shoulder lines that can be read from player height, while keeping water,
		// paths, spawn and the hand-built map structure stable.
		const float WaterMask = FMath::SmoothStep(16500.0f, 28500.0f, GetNearestWaterDistance(Position));
		const float PathMask = FMath::SmoothStep(7200.0f, 15800.0f, GetNearestPathDistance(Position));
		const float SpawnDistance = static_cast<float>(FVector2D::Distance(Position, SpawnKeepFlatCenter));
		const float SpawnMask = FMath::SmoothStep(23500.0f, 36500.0f, SpawnDistance);
		const float ProtectedMask = InteriorMask * WaterMask * PathMask * SpawnMask;
		if (ProtectedMask <= 0.01f)
		{
			return 0.0f;
		}

		float Delta = 0.0f;

		// Future village overlook: readable but not flat, with a gentler approach shelf below it.
		Delta += SoftHill(Position, FVector2D(77200.0f, 124800.0f), FVector2D(25000.0f, 17000.0f), 34.0f);
		Delta += SoftHill(Position, FVector2D(68500.0f, 115500.0f), FVector2D(36000.0f, 25500.0f), 18.0f);
		Delta += SoftHill(Position, FVector2D(84200.0f, 112500.0f), FVector2D(18500.0f, 10500.0f), -12.0f);

		// Traversal step-ups: long soft shoulder lines that imply routes without forming corridors.
		Delta += SoftHill(Position, FVector2D(50500.0f, 132000.0f), FVector2D(42000.0f, 11500.0f), 20.0f);
		Delta += SoftHill(Position, FVector2D(91500.0f, 96000.0f), FVector2D(30500.0f, 17500.0f), 24.0f);
		Delta += SoftHill(Position, FVector2D(39000.0f, 90000.0f), FVector2D(27000.0f, 16500.0f), 18.0f);

		// Forest edge footing: subtle rises and pockets so canopy masses sit on terrain instead of flat planes.
		Delta += SoftHill(Position, FVector2D(93000.0f, 141500.0f), FVector2D(20500.0f, 9400.0f), 22.0f);
		Delta += SoftHill(Position, FVector2D(80500.0f, 135200.0f), FVector2D(12000.0f, 7600.0f), -11.0f);
		Delta += SoftHill(Position, FVector2D(64200.0f, 158200.0f), FVector2D(26500.0f, 6200.0f), 12.0f);

		// Cliff-foot breakup: only near the organic rim, creating readable toe undulation without mountain spam.
		const float RimShoulder = FMath::SmoothStep(0.48f, 0.70f, GetIslandOrganicValue(Position));
		const float RimNoiseA = FMath::PerlinNoise2D((Position + FVector2D(-5400.0f, 28400.0f)) / 26000.0f);
		const float RimNoiseB = FMath::PerlinNoise2D((Position + FVector2D(19000.0f, -7600.0f)) / 13500.0f);
		Delta += (RimNoiseA * 16.0f + RimNoiseB * 7.0f) * RimShoulder * WaterMask * PathMask;

		// Broad low-frequency meadow rhythm, deliberately small so it avoids procedural spikes.
		Delta += FMath::PerlinNoise2D((Position + FVector2D(6100.0f, -21400.0f)) / 92000.0f) * 10.0f;
		Delta += FMath::PerlinNoise2D((Position + FVector2D(-23800.0f, 17300.0f)) / 47000.0f) * 5.0f;

		return FMath::Clamp(Delta, -20.0f, 58.0f) * ProtectedMask;
	}

	float ComputeTraversalReadabilityV6HeightDelta(const FVector2D& Position)
	{
		// V6 is a small additive pass over v5. It targets Titan-style cliff language:
		// broken rim shoulders, ledge-like shelves, and readable toe transitions
		// without moving water, spawn, paths, or flattening the handmade layout.
		const float OrganicValue = GetIslandOrganicValue(Position);
		const float RimMask = FMath::SmoothStep(0.43f, 0.72f, OrganicValue);
		if (RimMask <= 0.01f)
		{
			return 0.0f;
		}

		const float WaterMask = FMath::SmoothStep(16000.0f, 30000.0f, GetNearestWaterDistance(Position));
		const float PathMask = FMath::SmoothStep(7600.0f, 16500.0f, GetNearestPathDistance(Position));
		const float SpawnDistance = static_cast<float>(FVector2D::Distance(Position, SpawnKeepFlatCenter));
		const float SpawnMask = FMath::SmoothStep(26000.0f, 38500.0f, SpawnDistance);
		const float ProtectionMask = WaterMask * PathMask * SpawnMask;
		if (ProtectionMask <= 0.01f)
		{
			return 0.0f;
		}

		const float InnerShoulder = FMath::SmoothStep(0.36f, 0.58f, OrganicValue) * (1.0f - FMath::SmoothStep(0.66f, 0.80f, OrganicValue));
		const float OuterToe = FMath::SmoothStep(0.56f, 0.77f, OrganicValue);
		float Delta = 0.0f;

		// Long non-axis-aligned ledges read better than rectangular component bands.
		Delta += SoftHill(Position, FVector2D(104500.0f, 132500.0f), FVector2D(25000.0f, 6800.0f), 16.0f);
		Delta += SoftHill(Position, FVector2D(99000.0f, 105000.0f), FVector2D(21000.0f, 6400.0f), -13.0f);
		Delta += SoftHill(Position, FVector2D(33000.0f, 154500.0f), FVector2D(20500.0f, 5200.0f), 13.0f);
		Delta += SoftHill(Position, FVector2D(52000.0f, 166500.0f), FVector2D(24500.0f, 5100.0f), -9.0f);
		Delta += SoftHill(Position, FVector2D(87000.0f, 47000.0f), FVector2D(22000.0f, 6200.0f), 12.0f);
		Delta += SoftHill(Position, FVector2D(19500.0f, 85000.0f), FVector2D(18200.0f, 5900.0f), -10.0f);

		const float StrataA = FMath::PerlinNoise2D((Position + FVector2D(17400.0f, -9200.0f)) / 22000.0f);
		const float StrataB = FMath::PerlinNoise2D((Position + FVector2D(-8100.0f, 26100.0f)) / 9800.0f);
		const float DiagonalBreak = FMath::Sin(Position.X * 0.000105f + Position.Y * 0.000071f + StrataA * 1.7f);
		Delta += (StrataA * 14.0f + StrataB * 6.0f + DiagonalBreak * 5.0f) * InnerShoulder;
		Delta += (StrataA * -8.0f + StrataB * 9.0f) * OuterToe;

		return FMath::Clamp(Delta, -22.0f, 24.0f) * RimMask * ProtectionMask;
	}

	float ComputeOuterRingGeologyV42HeightDelta(const FVector2D& Position)
	{
		// V42 targets the enclosing mountain mass itself. It avoids the playable
		// water/path/spawn spaces and only adds broad geological asymmetry to the
		// visible world-border ring.
		const float OrganicValue = GetIslandOrganicValue(Position);
		const float PosX = static_cast<float>(Position.X);
		const float PosY = static_cast<float>(Position.Y);
		const float NorthRing = FMath::SmoothStep(108000.0f, 139000.0f, PosY);
		const float EastRing = FMath::SmoothStep(88000.0f, 113000.0f, PosX) * FMath::SmoothStep(83000.0f, 127000.0f, PosY);
		const float WestRing = (1.0f - FMath::SmoothStep(32000.0f, 50000.0f, PosX)) * FMath::SmoothStep(72000.0f, 130000.0f, PosY);
		const float RingCoordMask = FMath::Max3(NorthRing, EastRing, WestRing);
		const float RimMask = RingCoordMask * FMath::SmoothStep(0.45f, 0.76f, OrganicValue);
		if (RimMask <= 0.01f)
		{
			return 0.0f;
		}

		const float WaterMask = FMath::SmoothStep(18000.0f, 32000.0f, GetNearestWaterDistance(Position));
		const float PathMask = FMath::SmoothStep(9000.0f, 21000.0f, GetNearestPathDistance(Position));
		const float SpawnDistance = static_cast<float>(FVector2D::Distance(Position, SpawnKeepFlatCenter));
		const float SpawnMask = FMath::SmoothStep(32000.0f, 50000.0f, SpawnDistance);
		const float ProtectionMask = WaterMask * PathMask * SpawnMask;
		if (ProtectionMask <= 0.01f)
		{
			return 0.0f;
		}

		float Delta = 0.0f;

		// Large silhouette notches and shoulders: broad enough to affect the ring
		// mass, small enough to avoid cutting away the border terrain.
		Delta += SoftHill(Position, FVector2D(40500.0f, 160000.0f), FVector2D(17500.0f, 9200.0f), -120.0f);
		Delta += SoftHill(Position, FVector2D(61000.0f, 162500.0f), FVector2D(23500.0f, 10400.0f), 130.0f);
		Delta += SoftHill(Position, FVector2D(79000.0f, 158000.0f), FVector2D(20500.0f, 8700.0f), -105.0f);
		Delta += SoftHill(Position, FVector2D(102000.0f, 139000.0f), FVector2D(18500.0f, 14500.0f), 115.0f);
		Delta += SoftHill(Position, FVector2D(116500.0f, 120000.0f), FVector2D(13800.0f, 18000.0f), -145.0f);
		Delta += SoftHill(Position, FVector2D(26200.0f, 118000.0f), FVector2D(10800.0f, 30000.0f), -95.0f);
		Delta += SoftHill(Position, FVector2D(32500.0f, 144000.0f), FVector2D(14500.0f, 19000.0f), 92.0f);

		// Diagonal erosion cuts that break the rounded dome read without creating
		// protruding slab silhouettes.
		Delta += SoftSegmentBand(Position, FVector2D(36500.0f, 165000.0f), FVector2D(52000.0f, 132000.0f), 5200.0f, -165.0f);
		Delta += SoftSegmentBand(Position, FVector2D(66000.0f, 165500.0f), FVector2D(76000.0f, 137000.0f), 4700.0f, -118.0f);
		Delta += SoftSegmentBand(Position, FVector2D(91000.0f, 151000.0f), FVector2D(112000.0f, 120500.0f), 5600.0f, -158.0f);
		Delta += SoftSegmentBand(Position, FVector2D(121000.0f, 136000.0f), FVector2D(108000.0f, 104000.0f), 5100.0f, -132.0f);
		Delta += SoftSegmentBand(Position, FVector2D(24500.0f, 139000.0f), FVector2D(33000.0f, 92500.0f), 4500.0f, -108.0f);

		// Shelf rhythm along the lower faces and river-side cliff toe. These are
		// mass edits, not extra rock actors.
		Delta += SoftSegmentBand(Position, FVector2D(28000.0f, 151500.0f), FVector2D(86000.0f, 149000.0f), 3600.0f, 64.0f);
		Delta += SoftSegmentBand(Position, FVector2D(86000.0f, 135500.0f), FVector2D(119000.0f, 113000.0f), 3400.0f, 74.0f);
		Delta += SoftSegmentBand(Position, FVector2D(57500.0f, 70000.0f), FVector2D(94000.0f, 56000.0f), 3100.0f, -42.0f);

		const float BroadNoise = FMath::PerlinNoise2D((Position + FVector2D(17700.0f, -24600.0f)) / 42000.0f);
		const float MediumNoise = FMath::PerlinNoise2D((Position + FVector2D(-8300.0f, 19200.0f)) / 18500.0f);
		const float DiagonalFlow = FMath::Sin(Position.X * 0.000085f - Position.Y * 0.000066f + BroadNoise * 2.1f);
		Delta += (BroadNoise * 58.0f + MediumNoise * 24.0f + DiagonalFlow * 28.0f) * RimMask;

		return FMath::Clamp(Delta, -240.0f, 220.0f) * RimMask * ProtectionMask;
	}

	float ComputeOuterRingGeologyV42RefineHeightDelta(const FVector2D& Position)
	{
		// Additive v42 correction: broad asymmetric cuts and shoulders over the
		// existing ring, aimed at reducing the rounded tool-dome read without
		// placing additional slab actors or cutting away the playable border.
		const float OrganicValue = GetIslandOrganicValue(Position);
		const float PosX = static_cast<float>(Position.X);
		const float PosY = static_cast<float>(Position.Y);
		const float NorthRing = FMath::SmoothStep(106000.0f, 138000.0f, PosY);
		const float EastRing = FMath::SmoothStep(90000.0f, 116000.0f, PosX) * FMath::SmoothStep(77000.0f, 126000.0f, PosY);
		const float WestRing = (1.0f - FMath::SmoothStep(30000.0f, 51000.0f, PosX)) * FMath::SmoothStep(72000.0f, 131000.0f, PosY);
		const float RingCoordMask = FMath::Max3(NorthRing, EastRing, WestRing);
		const float RimMask = RingCoordMask * FMath::SmoothStep(0.43f, 0.75f, OrganicValue);
		if (RimMask <= 0.01f)
		{
			return 0.0f;
		}

		const float WaterMask = FMath::SmoothStep(21000.0f, 36000.0f, GetNearestWaterDistance(Position));
		const float PathMask = FMath::SmoothStep(10000.0f, 23000.0f, GetNearestPathDistance(Position));
		const float SpawnDistance = static_cast<float>(FVector2D::Distance(Position, SpawnKeepFlatCenter));
		const float SpawnMask = FMath::SmoothStep(34000.0f, 52000.0f, SpawnDistance);
		const float ProtectionMask = WaterMask * PathMask * SpawnMask;
		if (ProtectionMask <= 0.01f)
		{
			return 0.0f;
		}

		float Delta = 0.0f;

		Delta += SoftSegmentBand(Position, FVector2D(33000.0f, 165000.0f), FVector2D(52000.0f, 123000.0f), 8200.0f, -178.0f);
		Delta += SoftSegmentBand(Position, FVector2D(65000.0f, 166000.0f), FVector2D(82000.0f, 128500.0f), 7600.0f, -142.0f);
		Delta += SoftSegmentBand(Position, FVector2D(93000.0f, 151500.0f), FVector2D(116000.0f, 111000.0f), 8500.0f, -184.0f);
		Delta += SoftSegmentBand(Position, FVector2D(120500.0f, 134500.0f), FVector2D(100500.0f, 92500.0f), 7400.0f, -124.0f);
		Delta += SoftSegmentBand(Position, FVector2D(22500.0f, 135500.0f), FVector2D(36000.0f, 90500.0f), 7000.0f, -118.0f);

		Delta += SoftSegmentBand(Position, FVector2D(28500.0f, 151000.0f), FVector2D(90000.0f, 145500.0f), 6100.0f, 72.0f);
		Delta += SoftSegmentBand(Position, FVector2D(84000.0f, 134000.0f), FVector2D(119500.0f, 107500.0f), 5700.0f, 86.0f);
		Delta += SoftSegmentBand(Position, FVector2D(24000.0f, 113000.0f), FVector2D(52000.0f, 119000.0f), 5200.0f, 58.0f);

		Delta += SoftHill(Position, FVector2D(45000.0f, 157500.0f), FVector2D(23500.0f, 11200.0f), -86.0f);
		Delta += SoftHill(Position, FVector2D(68500.0f, 161000.0f), FVector2D(24500.0f, 10600.0f), 92.0f);
		Delta += SoftHill(Position, FVector2D(101500.0f, 137000.0f), FVector2D(21500.0f, 17600.0f), 104.0f);
		Delta += SoftHill(Position, FVector2D(116000.0f, 120500.0f), FVector2D(17500.0f, 20500.0f), -98.0f);
		Delta += SoftHill(Position, FVector2D(26500.0f, 124000.0f), FVector2D(13200.0f, 31800.0f), -76.0f);

		const float BroadNoise = FMath::PerlinNoise2D((Position + FVector2D(-9400.0f, 32100.0f)) / 52000.0f);
		const float MediumNoise = FMath::PerlinNoise2D((Position + FVector2D(22600.0f, -17600.0f)) / 24500.0f);
		const float DiagonalFlow = FMath::Sin(Position.X * 0.000061f - Position.Y * 0.000081f + BroadNoise * 2.6f);
		Delta += (BroadNoise * 44.0f + MediumNoise * 22.0f + DiagonalFlow * 18.0f) * RimMask;

		return FMath::Clamp(Delta, -210.0f, 190.0f) * RimMask * ProtectionMask;
	}

	float ComputeOuterRingGeologyV43HeightDelta(const FVector2D& Position)
	{
		// V43 is a broad mountain-body pass. It uses wide face cuts, shoulders,
		// and asymmetric mass shifts so the border ring reads as eroded geology
		// rather than heightmap domes with separate rocks attached.
		const float OrganicValue = GetIslandOrganicValue(Position);
		const float PosX = static_cast<float>(Position.X);
		const float PosY = static_cast<float>(Position.Y);
		const float NorthRing = FMath::SmoothStep(103000.0f, 140000.0f, PosY);
		const float EastRing = FMath::SmoothStep(88500.0f, 117000.0f, PosX) * FMath::SmoothStep(76000.0f, 128500.0f, PosY);
		const float WestRing = (1.0f - FMath::SmoothStep(29000.0f, 51500.0f, PosX)) * FMath::SmoothStep(70000.0f, 132000.0f, PosY);
		const float RingCoordMask = FMath::Max3(NorthRing, EastRing, WestRing);
		const float RimMask = RingCoordMask * FMath::SmoothStep(0.41f, 0.74f, OrganicValue);
		if (RimMask <= 0.01f)
		{
			return 0.0f;
		}

		const float WaterMask = FMath::SmoothStep(23000.0f, 39000.0f, GetNearestWaterDistance(Position));
		const float PathMask = FMath::SmoothStep(11500.0f, 25500.0f, GetNearestPathDistance(Position));
		const float SpawnDistance = static_cast<float>(FVector2D::Distance(Position, SpawnKeepFlatCenter));
		const float SpawnMask = FMath::SmoothStep(36000.0f, 55500.0f, SpawnDistance);
		const float ProtectionMask = WaterMask * PathMask * SpawnMask;
		if (ProtectionMask <= 0.01f)
		{
			return 0.0f;
		}

		float Delta = 0.0f;

		// Large face splits: wider than the previous detail cuts, with uneven
		// angles so the ring no longer reads as evenly ribbed landscape terrain.
		Delta += SoftSegmentBand(Position, FVector2D(31500.0f, 168000.0f), FVector2D(56000.0f, 119500.0f), 11200.0f, -215.0f);
		Delta += SoftSegmentBand(Position, FVector2D(61000.0f, 166800.0f), FVector2D(89000.0f, 124000.0f), 10400.0f, -172.0f);
		Delta += SoftSegmentBand(Position, FVector2D(92500.0f, 154000.0f), FVector2D(121000.0f, 104500.0f), 11600.0f, -228.0f);
		Delta += SoftSegmentBand(Position, FVector2D(126000.0f, 136000.0f), FVector2D(99000.0f, 86500.0f), 9800.0f, -146.0f);
		Delta += SoftSegmentBand(Position, FVector2D(20500.0f, 140500.0f), FVector2D(39000.0f, 85000.0f), 9200.0f, -142.0f);

		// Mountain shoulders and embedded shelves. These are broad mass edits,
		// not isolated protrusions, and are biased toward toe/blend continuity.
		Delta += SoftSegmentBand(Position, FVector2D(29000.0f, 153500.0f), FVector2D(93000.0f, 146500.0f), 7600.0f, 92.0f);
		Delta += SoftSegmentBand(Position, FVector2D(85000.0f, 136500.0f), FVector2D(124500.0f, 108000.0f), 7200.0f, 116.0f);
		Delta += SoftSegmentBand(Position, FVector2D(23500.0f, 113000.0f), FVector2D(60000.0f, 121000.0f), 6800.0f, 76.0f);
		Delta += SoftSegmentBand(Position, FVector2D(104000.0f, 98000.0f), FVector2D(121500.0f, 79000.0f), 7200.0f, 68.0f);

		// Broad asymmetric notches and lifted shoulders break dome peaks without
		// making spike silhouettes.
		Delta += SoftHill(Position, FVector2D(43000.0f, 157000.0f), FVector2D(26500.0f, 13800.0f), -96.0f);
		Delta += SoftHill(Position, FVector2D(70500.0f, 160000.0f), FVector2D(29200.0f, 12600.0f), 118.0f);
		Delta += SoftHill(Position, FVector2D(100500.0f, 137000.0f), FVector2D(25200.0f, 19800.0f), 126.0f);
		Delta += SoftHill(Position, FVector2D(118000.0f, 121000.0f), FVector2D(21200.0f, 23600.0f), -118.0f);
		Delta += SoftHill(Position, FVector2D(26800.0f, 125000.0f), FVector2D(15400.0f, 35000.0f), -82.0f);
		Delta += SoftHill(Position, FVector2D(33000.0f, 146000.0f), FVector2D(16000.0f, 22600.0f), 78.0f);

		const float BroadNoise = FMath::PerlinNoise2D((Position + FVector2D(18400.0f, -37200.0f)) / 61000.0f);
		const float MediumNoise = FMath::PerlinNoise2D((Position + FVector2D(-29400.0f, 21600.0f)) / 28000.0f);
		const float DiagonalFlowA = FMath::Sin(Position.X * 0.000052f - Position.Y * 0.000073f + BroadNoise * 2.8f);
		const float DiagonalFlowB = FMath::Sin(Position.X * -0.000041f + Position.Y * 0.000097f + MediumNoise * 2.2f);
		Delta += (BroadNoise * 56.0f + MediumNoise * 28.0f + DiagonalFlowA * 34.0f + DiagonalFlowB * 18.0f) * RimMask;

		return FMath::Clamp(Delta, -275.0f, 240.0f) * RimMask * ProtectionMask;
	}

	float ComputeOuterRingGeologyV43RefineHeightDelta(const FVector2D& Position)
	{
		// Second v43 correction: stronger, still broad, mountain-body edits.
		// The first v43 pass was technically stable but visually too subtle
		// against the existing ring height. These deltas deliberately target
		// large planes and ravines, not small spikes or detached slabs.
		const float OrganicValue = GetIslandOrganicValue(Position);
		const float PosX = static_cast<float>(Position.X);
		const float PosY = static_cast<float>(Position.Y);
		const float NorthRing = FMath::SmoothStep(101000.0f, 141000.0f, PosY);
		const float EastRing = FMath::SmoothStep(87000.0f, 118500.0f, PosX) * FMath::SmoothStep(74000.0f, 130000.0f, PosY);
		const float WestRing = (1.0f - FMath::SmoothStep(28500.0f, 52500.0f, PosX)) * FMath::SmoothStep(69000.0f, 133000.0f, PosY);
		const float RingCoordMask = FMath::Max3(NorthRing, EastRing, WestRing);
		const float RimMask = RingCoordMask * FMath::SmoothStep(0.40f, 0.74f, OrganicValue);
		if (RimMask <= 0.01f)
		{
			return 0.0f;
		}

		const float WaterMask = FMath::SmoothStep(26000.0f, 43000.0f, GetNearestWaterDistance(Position));
		const float PathMask = FMath::SmoothStep(13500.0f, 29000.0f, GetNearestPathDistance(Position));
		const float SpawnDistance = static_cast<float>(FVector2D::Distance(Position, SpawnKeepFlatCenter));
		const float SpawnMask = FMath::SmoothStep(39000.0f, 61000.0f, SpawnDistance);
		const float ProtectionMask = WaterMask * PathMask * SpawnMask;
		if (ProtectionMask <= 0.01f)
		{
			return 0.0f;
		}

		float Delta = 0.0f;

		// Broad ravines and face planes. Widths are intentionally large so the
		// result reads as carved mountain mass instead of decorative scratches.
		Delta += SoftSegmentBand(Position, FVector2D(27000.0f, 169000.0f), FVector2D(55500.0f, 114000.0f), 16800.0f, -820.0f);
		Delta += SoftSegmentBand(Position, FVector2D(61000.0f, 168500.0f), FVector2D(92000.0f, 121000.0f), 15600.0f, -680.0f);
		Delta += SoftSegmentBand(Position, FVector2D(95500.0f, 156500.0f), FVector2D(124500.0f, 101000.0f), 17200.0f, -890.0f);
		Delta += SoftSegmentBand(Position, FVector2D(126500.0f, 141000.0f), FVector2D(99000.0f, 80500.0f), 13800.0f, -520.0f);
		Delta += SoftSegmentBand(Position, FVector2D(17800.0f, 143000.0f), FVector2D(39800.0f, 80500.0f), 13400.0f, -530.0f);

		// Supporting shoulders give each carved face a mass to read against.
		Delta += SoftSegmentBand(Position, FVector2D(29000.0f, 155000.0f), FVector2D(101000.0f, 147000.0f), 9800.0f, 320.0f);
		Delta += SoftSegmentBand(Position, FVector2D(82500.0f, 139000.0f), FVector2D(128500.0f, 107000.0f), 9300.0f, 410.0f);
		Delta += SoftSegmentBand(Position, FVector2D(23000.0f, 113000.0f), FVector2D(65000.0f, 122000.0f), 8600.0f, 260.0f);
		Delta += SoftSegmentBand(Position, FVector2D(103000.0f, 95500.0f), FVector2D(123500.0f, 76500.0f), 8800.0f, 240.0f);

		// Peak/body asymmetry: big rounded domes lose their even crown shape.
		Delta += SoftHill(Position, FVector2D(41500.0f, 158500.0f), FVector2D(32000.0f, 17000.0f), -360.0f);
		Delta += SoftHill(Position, FVector2D(72000.0f, 160500.0f), FVector2D(35000.0f, 15000.0f), 430.0f);
		Delta += SoftHill(Position, FVector2D(101500.0f, 138000.0f), FVector2D(30000.0f, 22800.0f), 470.0f);
		Delta += SoftHill(Position, FVector2D(118500.0f, 121000.0f), FVector2D(25800.0f, 28600.0f), -420.0f);
		Delta += SoftHill(Position, FVector2D(25700.0f, 126000.0f), FVector2D(18500.0f, 39000.0f), -300.0f);
		Delta += SoftHill(Position, FVector2D(34500.0f, 146500.0f), FVector2D(18800.0f, 26400.0f), 260.0f);

		const float BroadNoise = FMath::PerlinNoise2D((Position + FVector2D(33600.0f, -48800.0f)) / 74000.0f);
		const float MediumNoise = FMath::PerlinNoise2D((Position + FVector2D(-41000.0f, 29500.0f)) / 36500.0f);
		const float DiagonalFlowA = FMath::Sin(Position.X * 0.000041f - Position.Y * 0.000064f + BroadNoise * 3.0f);
		const float DiagonalFlowB = FMath::Sin(Position.X * -0.000037f + Position.Y * 0.000082f + MediumNoise * 2.5f);
		Delta += (BroadNoise * 150.0f + MediumNoise * 86.0f + DiagonalFlowA * 96.0f + DiagonalFlowB * 52.0f) * RimMask;

		return FMath::Clamp(Delta, -1120.0f, 860.0f) * RimMask * ProtectionMask;
	}

	float ComputeOuterRingGeologyV44HeightDelta(const FVector2D& Position)
	{
		// V44 is deliberately hard-corrective: fewer, stronger cliff planes and
		// shelves restore readable mountain mass after v43 over-softened the ring.
		const float OrganicValue = GetIslandOrganicValue(Position);
		const float PosX = static_cast<float>(Position.X);
		const float PosY = static_cast<float>(Position.Y);
		const float NorthRing = FMath::SmoothStep(99500.0f, 141500.0f, PosY);
		const float EastRing = FMath::SmoothStep(86000.0f, 119000.0f, PosX) * FMath::SmoothStep(72000.0f, 130500.0f, PosY);
		const float WestRing = (1.0f - FMath::SmoothStep(28000.0f, 53000.0f, PosX)) * FMath::SmoothStep(67500.0f, 133500.0f, PosY);
		const float RingCoordMask = FMath::Max3(NorthRing, EastRing, WestRing);
		const float RimMask = RingCoordMask * FMath::SmoothStep(0.39f, 0.74f, OrganicValue);
		if (RimMask <= 0.01f)
		{
			return 0.0f;
		}

		const float WaterMask = FMath::SmoothStep(28000.0f, 46500.0f, GetNearestWaterDistance(Position));
		const float PathMask = FMath::SmoothStep(14500.0f, 31500.0f, GetNearestPathDistance(Position));
		const float SpawnDistance = static_cast<float>(FVector2D::Distance(Position, SpawnKeepFlatCenter));
		const float SpawnMask = FMath::SmoothStep(40500.0f, 63500.0f, SpawnDistance);
		const float ProtectionMask = WaterMask * PathMask * SpawnMask;
		if (ProtectionMask <= 0.01f)
		{
			return 0.0f;
		}

		float Delta = 0.0f;

		// Large hard planes: each one shifts a broad mountain face across a
		// controlled edge, producing readable cliff mass instead of blob noise.
		Delta += HardCliffPlaneShift(Position, FVector2D(26000.0f, 154500.0f), FVector2D(112000.0f, 148000.0f), 18500.0f, 1900.0f, 5200.0f, 560.0f, 1.0f);
		Delta += HardCliffPlaneShift(Position, FVector2D(36500.0f, 165500.0f), FVector2D(98000.0f, 159000.0f), 14500.0f, 1700.0f, 4300.0f, -380.0f, -1.0f);
		Delta += HardCliffPlaneShift(Position, FVector2D(85000.0f, 137500.0f), FVector2D(127500.0f, 105000.0f), 17000.0f, 1800.0f, 5000.0f, 620.0f, -1.0f);
		Delta += HardCliffPlaneShift(Position, FVector2D(103000.0f, 124000.0f), FVector2D(123500.0f, 79000.0f), 14200.0f, 1650.0f, 4400.0f, -420.0f, 1.0f);
		Delta += HardCliffPlaneShift(Position, FVector2D(22000.0f, 134000.0f), FVector2D(38500.0f, 83000.0f), 15000.0f, 1700.0f, 4700.0f, 470.0f, 1.0f);

		// Embedded strata/shelf systems. These are wide landscape shelves that
		// connect into the wall, not detached dressing actors.
		Delta += SoftSegmentBand(Position, FVector2D(29500.0f, 151000.0f), FVector2D(101500.0f, 145500.0f), 5200.0f, 285.0f);
		Delta += SoftSegmentBand(Position, FVector2D(34200.0f, 158300.0f), FVector2D(92000.0f, 153800.0f), 3900.0f, -230.0f);
		Delta += SoftSegmentBand(Position, FVector2D(84000.0f, 133500.0f), FVector2D(126000.0f, 106000.0f), 4800.0f, 330.0f);
		Delta += SoftSegmentBand(Position, FVector2D(96500.0f, 124500.0f), FVector2D(122500.0f, 93500.0f), 3600.0f, -260.0f);
		Delta += SoftSegmentBand(Position, FVector2D(23200.0f, 118000.0f), FVector2D(58500.0f, 122500.0f), 4300.0f, 235.0f);

		// Broad shoulders keep the cliffs heavy and grounded from gameplay
		// distance, while opposing notches prevent smooth dome crowns.
		Delta += SoftHill(Position, FVector2D(69000.0f, 160500.0f), FVector2D(33000.0f, 13800.0f), 360.0f);
		Delta += SoftHill(Position, FVector2D(47000.0f, 158000.0f), FVector2D(25000.0f, 15400.0f), -260.0f);
		Delta += SoftHill(Position, FVector2D(100500.0f, 137500.0f), FVector2D(28500.0f, 22600.0f), 410.0f);
		Delta += SoftHill(Position, FVector2D(119500.0f, 119500.0f), FVector2D(21800.0f, 27000.0f), -310.0f);
		Delta += SoftHill(Position, FVector2D(30500.0f, 127000.0f), FVector2D(17000.0f, 36000.0f), -210.0f);

		// Very low-frequency variation only, to avoid procedural noise terrain.
		const float BroadMass = FMath::PerlinNoise2D((Position + FVector2D(52000.0f, -61000.0f)) / 98000.0f);
		const float ShoulderFlow = FMath::Sin(Position.X * 0.000027f - Position.Y * 0.000041f + BroadMass * 2.2f);
		Delta += (BroadMass * 90.0f + ShoulderFlow * 54.0f) * RimMask;

		return FMath::Clamp(Delta, -920.0f, 860.0f) * RimMask * ProtectionMask;
	}

	float ComputeOuterRingGeologyV44RefineHeightDelta(const FVector2D& Position)
	{
		// Additive v44 refinement after screenshot review: sharper, broader
		// cliff mass edges to fight the remaining melted-dome read.
		const float OrganicValue = GetIslandOrganicValue(Position);
		const float PosX = static_cast<float>(Position.X);
		const float PosY = static_cast<float>(Position.Y);
		const float NorthRing = FMath::SmoothStep(98500.0f, 142000.0f, PosY);
		const float EastRing = FMath::SmoothStep(85000.0f, 119500.0f, PosX) * FMath::SmoothStep(71000.0f, 131000.0f, PosY);
		const float WestRing = (1.0f - FMath::SmoothStep(27000.0f, 53500.0f, PosX)) * FMath::SmoothStep(66500.0f, 134000.0f, PosY);
		const float RingCoordMask = FMath::Max3(NorthRing, EastRing, WestRing);
		const float RimMask = RingCoordMask * FMath::SmoothStep(0.38f, 0.74f, OrganicValue);
		if (RimMask <= 0.01f)
		{
			return 0.0f;
		}

		const float WaterMask = FMath::SmoothStep(30000.0f, 50000.0f, GetNearestWaterDistance(Position));
		const float PathMask = FMath::SmoothStep(15500.0f, 34000.0f, GetNearestPathDistance(Position));
		const float SpawnDistance = static_cast<float>(FVector2D::Distance(Position, SpawnKeepFlatCenter));
		const float SpawnMask = FMath::SmoothStep(42500.0f, 66000.0f, SpawnDistance);
		const float ProtectionMask = WaterMask * PathMask * SpawnMask;
		if (ProtectionMask <= 0.01f)
		{
			return 0.0f;
		}

		float Delta = 0.0f;

		// Fewer, harder planes: broad enough for silhouette, with tighter edge
		// feathering than the first v44 pass so the ring stops reading melted.
		Delta += HardCliffPlaneShift(Position, FVector2D(27500.0f, 153500.0f), FVector2D(110000.0f, 146500.0f), 15000.0f, 720.0f, 3600.0f, 980.0f, 1.0f);
		Delta += HardCliffPlaneShift(Position, FVector2D(40500.0f, 165000.0f), FVector2D(97000.0f, 158000.0f), 11800.0f, 680.0f, 3100.0f, -680.0f, -1.0f);
		Delta += HardCliffPlaneShift(Position, FVector2D(88500.0f, 137500.0f), FVector2D(127000.0f, 105500.0f), 14200.0f, 760.0f, 3500.0f, 1060.0f, -1.0f);
		Delta += HardCliffPlaneShift(Position, FVector2D(101000.0f, 124500.0f), FVector2D(123500.0f, 81500.0f), 11200.0f, 690.0f, 3300.0f, -720.0f, 1.0f);
		Delta += HardCliffPlaneShift(Position, FVector2D(23000.0f, 132000.0f), FVector2D(39500.0f, 85000.0f), 12800.0f, 720.0f, 3500.0f, 760.0f, 1.0f);

		// Strong connected shelf ledges. These create broad terrain-owned strata
		// while avoiding repeated small chips.
		Delta += SoftSegmentBand(Position, FVector2D(30500.0f, 149800.0f), FVector2D(103000.0f, 143800.0f), 3100.0f, 420.0f);
		Delta += SoftSegmentBand(Position, FVector2D(35200.0f, 157300.0f), FVector2D(94500.0f, 152500.0f), 2600.0f, -360.0f);
		Delta += SoftSegmentBand(Position, FVector2D(85500.0f, 132800.0f), FVector2D(126500.0f, 105500.0f), 3000.0f, 470.0f);
		Delta += SoftSegmentBand(Position, FVector2D(97500.0f, 121500.0f), FVector2D(123000.0f, 94000.0f), 2500.0f, -390.0f);

		const float BroadMass = FMath::PerlinNoise2D((Position + FVector2D(-61000.0f, 70000.0f)) / 118000.0f);
		Delta += BroadMass * 72.0f * RimMask;

		return FMath::Clamp(Delta, -1180.0f, 1120.0f) * RimMask * ProtectionMask;
	}

	float ComputeOuterRingGeologyV44HardHeightDelta(const FVector2D& Position)
	{
		// Hard-corrective v44 layer: restore readable Titan-style cliff mass
		// language after the first refine pass still read too smooth in shots.
		const float OrganicValue = GetIslandOrganicValue(Position);
		const float PosX = static_cast<float>(Position.X);
		const float PosY = static_cast<float>(Position.Y);
		const float NorthRing = FMath::SmoothStep(97000.0f, 142500.0f, PosY);
		const float EastRing = FMath::SmoothStep(83500.0f, 120500.0f, PosX) * FMath::SmoothStep(69500.0f, 132000.0f, PosY);
		const float WestRing = (1.0f - FMath::SmoothStep(26000.0f, 54500.0f, PosX)) * FMath::SmoothStep(65000.0f, 135000.0f, PosY);
		const float RingCoordMask = FMath::Max3(NorthRing, EastRing, WestRing);
		const float RimMask = RingCoordMask * FMath::SmoothStep(0.36f, 0.74f, OrganicValue);
		if (RimMask <= 0.01f)
		{
			return 0.0f;
		}

		const float WaterMask = FMath::SmoothStep(31000.0f, 53000.0f, GetNearestWaterDistance(Position));
		const float PathMask = FMath::SmoothStep(16500.0f, 36500.0f, GetNearestPathDistance(Position));
		const float SpawnDistance = static_cast<float>(FVector2D::Distance(Position, SpawnKeepFlatCenter));
		const float SpawnMask = FMath::SmoothStep(44500.0f, 69000.0f, SpawnDistance);
		const float ProtectionMask = WaterMask * PathMask * SpawnMask;
		if (ProtectionMask <= 0.01f)
		{
			return 0.0f;
		}

		float Delta = 0.0f;

		// Fewer but heavier planes. Tight edge feathering gives broad cliff
		// face breaks without returning to noisy brush-ribbing.
		Delta += HardCliffPlaneShift(Position, FVector2D(25000.0f, 151000.0f), FVector2D(115000.0f, 143000.0f), 16400.0f, 260.0f, 2100.0f, 2100.0f, 1.0f);
		Delta += HardCliffPlaneShift(Position, FVector2D(33000.0f, 160500.0f), FVector2D(101000.0f, 154500.0f), 12300.0f, 240.0f, 1900.0f, -1550.0f, -1.0f);
		Delta += HardCliffPlaneShift(Position, FVector2D(79000.0f, 136000.0f), FVector2D(130500.0f, 101500.0f), 15600.0f, 280.0f, 2300.0f, 2300.0f, -1.0f);
		Delta += HardCliffPlaneShift(Position, FVector2D(99000.0f, 122500.0f), FVector2D(125500.0f, 76000.0f), 11800.0f, 260.0f, 2100.0f, -1700.0f, 1.0f);
		Delta += HardCliffPlaneShift(Position, FVector2D(20500.0f, 130500.0f), FVector2D(41000.0f, 78000.0f), 13600.0f, 250.0f, 2100.0f, 1650.0f, 1.0f);

		// Broad strata shelves, intentionally sparse so the ring gains weight
		// without looking like repeated contour lines.
		Delta += SoftSegmentBand(Position, FVector2D(28500.0f, 147900.0f), FVector2D(108000.0f, 141900.0f), 1800.0f, 920.0f);
		Delta += SoftSegmentBand(Position, FVector2D(36500.0f, 156400.0f), FVector2D(96500.0f, 151100.0f), 1550.0f, -760.0f);
		Delta += SoftSegmentBand(Position, FVector2D(80500.0f, 130800.0f), FVector2D(130000.0f, 101200.0f), 1900.0f, 1080.0f);
		Delta += SoftSegmentBand(Position, FVector2D(101000.0f, 118500.0f), FVector2D(125000.0f, 90000.0f), 1500.0f, -820.0f);
		Delta += SoftSegmentBand(Position, FVector2D(21500.0f, 122000.0f), FVector2D(39000.0f, 84000.0f), 1700.0f, 740.0f);

		const float Shoulder = FMath::SmoothStep(0.48f, 0.64f, OrganicValue) * (1.0f - FMath::SmoothStep(0.70f, 0.82f, OrganicValue));
		const float BroadBreak = FMath::PerlinNoise2D((Position + FVector2D(-88000.0f, 54000.0f)) / 92000.0f);
		Delta += (Shoulder * -520.0f + BroadBreak * 110.0f) * RimMask;

		return FMath::Clamp(Delta, -2650.0f, 2550.0f) * RimMask * ProtectionMask;
	}

	float ComputeOuterRingGeologyV44CorrectionHeightDelta(const FVector2D& Position)
	{
		// Focused post-screenshot correction: restore broad cliff mass and
		// shoulder breaks without adding random dressing or brush noise.
		const float OrganicValue = GetIslandOrganicValue(Position);
		const float PosX = static_cast<float>(Position.X);
		const float PosY = static_cast<float>(Position.Y);
		const float NorthRing = FMath::SmoothStep(99500.0f, 142000.0f, PosY);
		const float EastRing = FMath::SmoothStep(89000.0f, 121000.0f, PosX) * FMath::SmoothStep(78500.0f, 133500.0f, PosY);
		const float WestRing = (1.0f - FMath::SmoothStep(25000.0f, 49000.0f, PosX)) * FMath::SmoothStep(69000.0f, 135500.0f, PosY);
		const float RingCoordMask = FMath::Max3(NorthRing, EastRing, WestRing);
		const float RimMask = RingCoordMask * FMath::SmoothStep(0.38f, 0.74f, OrganicValue);
		if (RimMask <= 0.01f)
		{
			return 0.0f;
		}

		const float WaterMask = FMath::SmoothStep(33000.0f, 55000.0f, GetNearestWaterDistance(Position));
		const float PathMask = FMath::SmoothStep(17500.0f, 38500.0f, GetNearestPathDistance(Position));
		const float SpawnDistance = static_cast<float>(FVector2D::Distance(Position, SpawnKeepFlatCenter));
		const float SpawnMask = FMath::SmoothStep(45500.0f, 70500.0f, SpawnDistance);
		const float ProtectionMask = WaterMask * PathMask * SpawnMask;
		if (ProtectionMask <= 0.01f)
		{
			return 0.0f;
		}

		float Delta = 0.0f;

		// Large mass planes visible in the v44 validation angles.
		Delta += HardCliffPlaneShift(Position, FVector2D(44500.0f, 145800.0f), FVector2D(119000.0f, 137800.0f), 12200.0f, 145.0f, 1350.0f, -1750.0f, -1.0f);
		Delta += HardCliffPlaneShift(Position, FVector2D(55000.0f, 153000.0f), FVector2D(112000.0f, 148500.0f), 9800.0f, 130.0f, 1250.0f, 1320.0f, 1.0f);
		Delta += HardCliffPlaneShift(Position, FVector2D(91000.0f, 135000.0f), FVector2D(129500.0f, 104500.0f), 11800.0f, 150.0f, 1400.0f, -1580.0f, 1.0f);
		Delta += HardCliffPlaneShift(Position, FVector2D(103500.0f, 123000.0f), FVector2D(124500.0f, 83000.0f), 9200.0f, 135.0f, 1300.0f, 1280.0f, -1.0f);
		Delta += HardCliffPlaneShift(Position, FVector2D(21500.0f, 128500.0f), FVector2D(40500.0f, 80500.0f), 10000.0f, 145.0f, 1250.0f, -1280.0f, -1.0f);

		// Broad shoulders and toe weight keep the forms readable at gameplay distance.
		Delta += SoftHill(Position, FVector2D(72000.0f, 149500.0f), FVector2D(20500.0f, 13000.0f), -820.0f);
		Delta += SoftHill(Position, FVector2D(99000.0f, 144500.0f), FVector2D(19500.0f, 14500.0f), 780.0f);
		Delta += SoftHill(Position, FVector2D(109500.0f, 122500.0f), FVector2D(16500.0f, 19000.0f), -760.0f);
		Delta += SoftHill(Position, FVector2D(33000.0f, 111500.0f), FVector2D(11000.0f, 23000.0f), 650.0f);

		// Sparse shelf accents, angled with the wall instead of horizontal rings.
		Delta += SoftSegmentBand(Position, FVector2D(52000.0f, 143500.0f), FVector2D(113000.0f, 136800.0f), 980.0f, -520.0f);
		Delta += SoftSegmentBand(Position, FVector2D(61000.0f, 151000.0f), FVector2D(105000.0f, 146800.0f), 860.0f, 460.0f);
		Delta += SoftSegmentBand(Position, FVector2D(94500.0f, 132000.0f), FVector2D(127500.0f, 105500.0f), 920.0f, -560.0f);
		Delta += SoftSegmentBand(Position, FVector2D(23500.0f, 126000.0f), FVector2D(39500.0f, 84500.0f), 860.0f, -440.0f);

		return FMath::Clamp(Delta, -1850.0f, 1650.0f) * RimMask * ProtectionMask;
	}

	float ComputeOuterRingSilhouetteV51HeightDelta(const FVector2D& Position)
	{
		// v51 is silhouette-only. It edits the existing Landscape ring mass into
		// a connected mountain profile: peaks, saddles, broken crest lines, and
		// broad erosion notches. It does not rely on materials or mesh dressing.
		const float OrganicValue = GetIslandOrganicValue(Position);
		const float PosX = static_cast<float>(Position.X);
		const float PosY = static_cast<float>(Position.Y);

		const float NorthRing = FMath::SmoothStep(100000.0f, 144500.0f, PosY);
		const float EastRing = FMath::SmoothStep(90000.0f, 122500.0f, PosX) * FMath::SmoothStep(77000.0f, 137000.0f, PosY);
		const float WestRing = (1.0f - FMath::SmoothStep(25500.0f, 52000.0f, PosX)) * FMath::SmoothStep(70000.0f, 138500.0f, PosY);
		const float RingCoordMask = FMath::Max3(NorthRing, EastRing, WestRing);
		const float RimMassMask = RingCoordMask * FMath::SmoothStep(0.44f, 0.74f, OrganicValue);
		if (RimMassMask <= 0.01f)
		{
			return 0.0f;
		}

		// Keep authored playable space open. These thresholds are deliberately
		// wider than earlier geology passes because v51 must not narrow river,
		// lake, spawn, or traversal corridors while reshaping the border profile.
		const float WaterMask = FMath::SmoothStep(39000.0f, 66500.0f, GetNearestWaterDistance(Position));
		const float PathMask = FMath::SmoothStep(22500.0f, 49500.0f, GetNearestPathDistance(Position));
		const float SpawnDistance = static_cast<float>(FVector2D::Distance(Position, SpawnKeepFlatCenter));
		const float SpawnMask = FMath::SmoothStep(52000.0f, 79000.0f, SpawnDistance);
		const float ProtectionMask = WaterMask * PathMask * SpawnMask;
		if (ProtectionMask <= 0.01f)
		{
			return 0.0f;
		}

		const float CrestMask = RimMassMask * FMath::SmoothStep(0.56f, 0.78f, OrganicValue);
		const float FaceMask = RimMassMask * (1.0f - FMath::SmoothStep(0.82f, 0.95f, OrganicValue));
		float Delta = 0.0f;

		// Connected peak chain. Alternating high points and saddles break the
		// dome line without adding new interior mountain bodies.
		Delta += SoftHill(Position, FVector2D(40500.0f, 158800.0f), FVector2D(25000.0f, 13200.0f), 2400.0f);
		Delta += SoftHill(Position, FVector2D(56500.0f, 158300.0f), FVector2D(21500.0f, 11500.0f), -1800.0f);
		Delta += SoftHill(Position, FVector2D(73500.0f, 162800.0f), FVector2D(30500.0f, 12800.0f), 3150.0f);
		Delta += SoftHill(Position, FVector2D(90500.0f, 151000.0f), FVector2D(24500.0f, 13600.0f), -2300.0f);
		Delta += SoftHill(Position, FVector2D(106500.0f, 139500.0f), FVector2D(25500.0f, 20800.0f), 2850.0f);
		Delta += SoftHill(Position, FVector2D(119500.0f, 122500.0f), FVector2D(19000.0f, 25000.0f), -2550.0f);
		Delta += SoftHill(Position, FVector2D(118000.0f, 103000.0f), FVector2D(18200.0f, 22500.0f), 1850.0f);
		Delta += SoftHill(Position, FVector2D(29500.0f, 132000.0f), FVector2D(15500.0f, 33000.0f), 2300.0f);
		Delta += SoftHill(Position, FVector2D(27800.0f, 111000.0f), FVector2D(14500.0f, 27000.0f), -1950.0f);
		Delta += SoftHill(Position, FVector2D(34000.0f, 91000.0f), FVector2D(16000.0f, 24500.0f), 1650.0f);

		// Ridge saddles and erosion cuts. These are broad negative edits through
		// the existing ring mass, not inward-facing cliffs or extra structures.
		Delta += SoftSegmentBand(Position, FVector2D(51500.0f, 166500.0f), FVector2D(61000.0f, 136500.0f), 9300.0f, -1850.0f);
		Delta += SoftSegmentBand(Position, FVector2D(82500.0f, 166000.0f), FVector2D(93000.0f, 132000.0f), 9800.0f, -2100.0f);
		Delta += SoftSegmentBand(Position, FVector2D(103000.0f, 151000.0f), FVector2D(116500.0f, 113500.0f), 11200.0f, -2250.0f);
		Delta += SoftSegmentBand(Position, FVector2D(124000.0f, 134000.0f), FVector2D(111000.0f, 95000.0f), 9000.0f, -1700.0f);
		Delta += SoftSegmentBand(Position, FVector2D(22500.0f, 136000.0f), FVector2D(36500.0f, 87500.0f), 8200.0f, -1600.0f);

		// Long crest breaks: sparse, angled, and connected to the wall so they
		// read as mountain-ridge profile rather than horizontal contour bands.
		Delta += SoftSegmentBand(Position, FVector2D(33500.0f, 156800.0f), FVector2D(76000.0f, 162200.0f), 5200.0f, 1050.0f);
		Delta += SoftSegmentBand(Position, FVector2D(66000.0f, 154500.0f), FVector2D(111000.0f, 142000.0f), 5600.0f, -820.0f);
		Delta += SoftSegmentBand(Position, FVector2D(93000.0f, 140000.0f), FVector2D(123000.0f, 109000.0f), 5000.0f, 980.0f);
		Delta += SoftSegmentBand(Position, FVector2D(25500.0f, 128500.0f), FVector2D(36500.0f, 100000.0f), 4400.0f, 760.0f);

		// A few wide face-plane pushes sharpen the profile from distance without
		// creating overhangs, slabs, or new geometry.
		Delta += HardCliffPlaneShift(Position, FVector2D(30000.0f, 153500.0f), FVector2D(93500.0f, 148500.0f), 17500.0f, 420.0f, 2800.0f, 1150.0f, 1.0f) * FaceMask;
		Delta += HardCliffPlaneShift(Position, FVector2D(87500.0f, 136500.0f), FVector2D(124000.0f, 106500.0f), 16200.0f, 440.0f, 2900.0f, -1250.0f, 1.0f) * FaceMask;
		Delta += HardCliffPlaneShift(Position, FVector2D(22000.0f, 130500.0f), FVector2D(39000.0f, 85000.0f), 14200.0f, 430.0f, 2700.0f, 1050.0f, -1.0f) * FaceMask;

		// Low-frequency asymmetry only. This breaks identical crest spacing but
		// avoids procedural micro-noise.
		const float CrestNoiseA = FMath::PerlinNoise2D((Position + FVector2D(71000.0f, -43000.0f)) / 88000.0f);
		const float CrestNoiseB = FMath::PerlinNoise2D((Position + FVector2D(-39000.0f, 58000.0f)) / 54000.0f);
		const float RidgeWave = FMath::Sin(Position.X * 0.000048f - Position.Y * 0.000036f + CrestNoiseA * 2.4f);
		Delta += (CrestNoiseA * 330.0f + CrestNoiseB * 190.0f + RidgeWave * 260.0f) * CrestMask;

		return FMath::Clamp(Delta, -3350.0f, 3600.0f) * RimMassMask * ProtectionMask;
	}

	float ComputeOuterRingSilhouetteV51RefineHeightDelta(const FVector2D& Position)
	{
		// v51 refine keeps the same strict rule set as v51: Landscape height only.
		// It strengthens the visible mountain-chain profile after the first pass
		// proved too subtle and left the outer ring reading as smooth domes.
		const float OrganicValue = GetIslandOrganicValue(Position);
		const float PosX = static_cast<float>(Position.X);
		const float PosY = static_cast<float>(Position.Y);

		const float NorthRing = FMath::SmoothStep(101500.0f, 143000.0f, PosY);
		const float EastRing = FMath::SmoothStep(91500.0f, 121000.0f, PosX) * FMath::SmoothStep(78500.0f, 135500.0f, PosY);
		const float WestRing = (1.0f - FMath::SmoothStep(27500.0f, 52000.0f, PosX)) * FMath::SmoothStep(74500.0f, 136500.0f, PosY);
		const float RingCoordMask = FMath::Max3(NorthRing, EastRing, WestRing);
		const float RingCoreMask = RingCoordMask * FMath::SmoothStep(0.38f, 0.66f, OrganicValue);
		if (RingCoreMask <= 0.01f)
		{
			return 0.0f;
		}

		const float WaterMask = FMath::SmoothStep(34000.0f, 58500.0f, GetNearestWaterDistance(Position));
		const float PathMask = FMath::SmoothStep(18500.0f, 43000.0f, GetNearestPathDistance(Position));
		const float SpawnDistance = static_cast<float>(FVector2D::Distance(Position, SpawnKeepFlatCenter));
		const float SpawnMask = FMath::SmoothStep(47000.0f, 73500.0f, SpawnDistance);
		const float ProtectionMask = WaterMask * PathMask * SpawnMask;
		if (ProtectionMask <= 0.01f)
		{
			return 0.0f;
		}

		const float CrestMask = RingCoreMask * FMath::SmoothStep(0.55f, 0.78f, OrganicValue);
		const float FaceMask = RingCoreMask * (1.0f - FMath::SmoothStep(0.84f, 0.96f, OrganicValue));
		float Delta = 0.0f;

		// Stronger mountain-chain rhythm: connected peaks and saddles across the
		// authored ring line, with no inward gameplay-space expansion.
		Delta += SoftHill(Position, FVector2D(35000.0f, 159500.0f), FVector2D(16500.0f, 9400.0f), 3900.0f);
		Delta += SoftHill(Position, FVector2D(51500.0f, 157500.0f), FVector2D(15500.0f, 8800.0f), -3600.0f);
		Delta += SoftHill(Position, FVector2D(68000.0f, 162800.0f), FVector2D(20500.0f, 9200.0f), 4700.0f);
		Delta += SoftHill(Position, FVector2D(86500.0f, 154000.0f), FVector2D(18500.0f, 9800.0f), -4300.0f);
		Delta += SoftHill(Position, FVector2D(101500.0f, 143000.0f), FVector2D(20500.0f, 15200.0f), 4300.0f);
		Delta += SoftHill(Position, FVector2D(117500.0f, 124500.0f), FVector2D(15200.0f, 19800.0f), -4100.0f);
		Delta += SoftHill(Position, FVector2D(121000.0f, 102000.0f), FVector2D(14500.0f, 17800.0f), 3200.0f);
		Delta += SoftHill(Position, FVector2D(25500.0f, 131500.0f), FVector2D(11200.0f, 25200.0f), 3600.0f);
		Delta += SoftHill(Position, FVector2D(24500.0f, 110500.0f), FVector2D(10800.0f, 22500.0f), -3250.0f);
		Delta += SoftHill(Position, FVector2D(33000.0f, 89500.0f), FVector2D(12200.0f, 20500.0f), 2900.0f);

		// Broad erosional notches break the visible rounded dome surfaces.
		Delta += SoftSegmentBand(Position, FVector2D(45500.0f, 168000.0f), FVector2D(58000.0f, 136000.0f), 7600.0f, -3300.0f);
		Delta += SoftSegmentBand(Position, FVector2D(76000.0f, 168500.0f), FVector2D(91000.0f, 134000.0f), 7800.0f, -3650.0f);
		Delta += SoftSegmentBand(Position, FVector2D(100500.0f, 151500.0f), FVector2D(116000.0f, 115500.0f), 8600.0f, -3800.0f);
		Delta += SoftSegmentBand(Position, FVector2D(126000.0f, 134000.0f), FVector2D(111500.0f, 93000.0f), 7200.0f, -2850.0f);
		Delta += SoftSegmentBand(Position, FVector2D(21000.0f, 134000.0f), FVector2D(36500.0f, 90000.0f), 7000.0f, -2850.0f);

		// Wider face shifts reduce soft hill reads while remaining terrain-native.
		Delta += HardCliffPlaneShift(Position, FVector2D(29500.0f, 154500.0f), FVector2D(101500.0f, 147800.0f), 14500.0f, 360.0f, 2300.0f, 2050.0f, 1.0f) * FaceMask;
		Delta += HardCliffPlaneShift(Position, FVector2D(83500.0f, 137500.0f), FVector2D(125500.0f, 103500.0f), 13200.0f, 360.0f, 2400.0f, -2250.0f, 1.0f) * FaceMask;
		Delta += HardCliffPlaneShift(Position, FVector2D(20500.0f, 130500.0f), FVector2D(39200.0f, 83500.0f), 11600.0f, 340.0f, 2200.0f, 1850.0f, -1.0f) * FaceMask;

		const float RidgeNoise = FMath::PerlinNoise2D((Position + FVector2D(64000.0f, -52000.0f)) / 72000.0f);
		const float RidgeWave = FMath::Sin(Position.X * 0.000062f - Position.Y * 0.000044f + RidgeNoise * 2.1f);
		Delta += (RidgeNoise * 480.0f + RidgeWave * 520.0f) * CrestMask;

		return FMath::Clamp(Delta, -5200.0f, 5400.0f) * RingCoreMask * ProtectionMask;
	}

	float ComputeOuterRingSilhouetteV51FaceBreakHeightDelta(const FVector2D& Position)
	{
		// Final v51 correction: break the visually dominant smooth north/east
		// dome faces with terrain-native saddles and erosion cuts. This remains
		// silhouette/form sculpting only: no new meshes, materials, or foliage.
		const float OrganicValue = GetIslandOrganicValue(Position);
		const float PosX = static_cast<float>(Position.X);
		const float PosY = static_cast<float>(Position.Y);

		const float NorthEastWall = FMath::SmoothStep(78500.0f, 103000.0f, PosX)
			* (1.0f - FMath::SmoothStep(125000.0f, 133500.0f, PosX))
			* FMath::SmoothStep(119000.0f, 144500.0f, PosY);
		const float NorthWall = FMath::SmoothStep(56000.0f, 73500.0f, PosX)
			* (1.0f - FMath::SmoothStep(112000.0f, 126000.0f, PosX))
			* FMath::SmoothStep(127500.0f, 148500.0f, PosY);
		const float WestWall = (1.0f - FMath::SmoothStep(26000.0f, 47000.0f, PosX))
			* FMath::SmoothStep(83500.0f, 132500.0f, PosY);
		const float FocusMask = FMath::Max3(NorthEastWall, NorthWall, WestWall)
			* FMath::SmoothStep(0.40f, 0.68f, OrganicValue);
		if (FocusMask <= 0.01f)
		{
			return 0.0f;
		}

		const float WaterMask = FMath::SmoothStep(36500.0f, 61000.0f, GetNearestWaterDistance(Position));
		const float PathMask = FMath::SmoothStep(20500.0f, 45500.0f, GetNearestPathDistance(Position));
		const float SpawnDistance = static_cast<float>(FVector2D::Distance(Position, SpawnKeepFlatCenter));
		const float SpawnMask = FMath::SmoothStep(50000.0f, 76000.0f, SpawnDistance);
		const float ProtectionMask = WaterMask * PathMask * SpawnMask;
		if (ProtectionMask <= 0.01f)
		{
			return 0.0f;
		}

		float Delta = 0.0f;

		// Carve the big north/east dome into a chain of shoulders, saddles, and
		// diagonal erosion lines so the camera reads mountain mass, not a single
		// rounded landscape brush stroke.
		Delta += SoftHill(Position, FVector2D(82000.0f, 154000.0f), FVector2D(20500.0f, 11200.0f), 3600.0f);
		Delta += SoftHill(Position, FVector2D(99000.0f, 145500.0f), FVector2D(27000.0f, 14600.0f), -4700.0f);
		Delta += SoftHill(Position, FVector2D(113000.0f, 134000.0f), FVector2D(20500.0f, 17800.0f), 3800.0f);
		Delta += SoftHill(Position, FVector2D(92000.0f, 129000.0f), FVector2D(18500.0f, 18200.0f), -3400.0f);

		Delta += SoftSegmentBand(Position, FVector2D(78500.0f, 161500.0f), FVector2D(103500.0f, 123500.0f), 7600.0f, -5200.0f);
		Delta += SoftSegmentBand(Position, FVector2D(90500.0f, 161000.0f), FVector2D(118000.0f, 127000.0f), 6200.0f, 3600.0f);
		Delta += SoftSegmentBand(Position, FVector2D(107500.0f, 151500.0f), FVector2D(121500.0f, 113000.0f), 6800.0f, -4550.0f);
		Delta += SoftSegmentBand(Position, FVector2D(65500.0f, 158500.0f), FVector2D(88000.0f, 132500.0f), 5900.0f, 3000.0f);

		// West ring receives the same silhouette logic at lower intensity so it
		// does not remain a separate smooth wall from top-down.
		Delta += SoftHill(Position, FVector2D(25500.0f, 127000.0f), FVector2D(10200.0f, 23800.0f), 2500.0f);
		Delta += SoftHill(Position, FVector2D(28500.0f, 107000.0f), FVector2D(10000.0f, 21000.0f), -2300.0f);
		Delta += SoftSegmentBand(Position, FVector2D(19500.0f, 133000.0f), FVector2D(36000.0f, 91000.0f), 5400.0f, -2600.0f);

		const float FaceNoise = FMath::PerlinNoise2D((Position + FVector2D(-28000.0f, 36000.0f)) / 46000.0f);
		const float BroadWave = FMath::Sin(Position.X * 0.000074f + Position.Y * 0.000031f + FaceNoise);
		Delta += (FaceNoise * 360.0f + BroadWave * 420.0f) * FocusMask;

		return FMath::Clamp(Delta, -6200.0f, 5600.0f) * FocusMask * ProtectionMask;
	}

	int32 SmoothGroundingProbeHill(
		TArray<uint16>& HeightData,
		int32 Width,
		int32 Height,
		int32 MinX,
		int32 MinY,
		const FTransform& LandscapeTransform,
		float LandscapeZScale,
		float& OutMaxAbsDeltaWorld)
	{
		const FVector2D ProbeCenter(42000.0f, 132000.0f);
		constexpr float InnerRadius = 8200.0f;
		constexpr float OuterRadius = 24000.0f;
		constexpr int32 KernelRadius = 7;
		constexpr int32 PassCount = 8;
		int32 ModifiedSamples = 0;
		OutMaxAbsDeltaWorld = 0.0f;
		TArray<uint16> OriginalHeightData = HeightData;
		TArray<uint16> CurrentHeightData = HeightData;

		auto GetIndex = [Width](int32 X, int32 Y)
		{
			return Y * Width + X;
		};

		for (int32 Pass = 0; Pass < PassCount; ++Pass)
		{
			TArray<uint16> NextHeightData = CurrentHeightData;
			for (int32 Y = 0; Y < Height; ++Y)
			{
				for (int32 X = 0; X < Width; ++X)
				{
					const int32 LandscapeX = MinX + X;
					const int32 LandscapeY = MinY + Y;
					const FVector WorldPosition = LandscapeTransform.TransformPosition(FVector(static_cast<float>(LandscapeX), static_cast<float>(LandscapeY), 0.0f));
					const FVector2D Position(WorldPosition.X, WorldPosition.Y);
					const float Distance = FVector2D::Distance(Position, ProbeCenter);
					const bool bLandingCore = Distance <= InnerRadius;
					if (Distance >= OuterRadius ||
						(!bLandingCore && GetNearestWaterDistance(Position) < 5200.0f) ||
						(!bLandingCore && GetNearestPathDistance(Position) < 2200.0f))
					{
						continue;
					}

					const float Blend = 1.0f - FMath::SmoothStep(InnerRadius, OuterRadius, Distance);
					if (Blend <= 0.01f)
					{
						continue;
					}

					float WeightedSum = 0.0f;
					float WeightTotal = 0.0f;
					for (int32 OffsetY = -KernelRadius; OffsetY <= KernelRadius; ++OffsetY)
					{
						for (int32 OffsetX = -KernelRadius; OffsetX <= KernelRadius; ++OffsetX)
						{
							const int32 SampleX = FMath::Clamp(X + OffsetX, 0, Width - 1);
							const int32 SampleY = FMath::Clamp(Y + OffsetY, 0, Height - 1);
							const float KernelDistance = FMath::Sqrt(static_cast<float>(OffsetX * OffsetX + OffsetY * OffsetY));
							const float Weight = FMath::Max(0.0f, static_cast<float>(KernelRadius + 1) - KernelDistance);
							WeightedSum += static_cast<float>(CurrentHeightData[GetIndex(SampleX, SampleY)]) * Weight;
							WeightTotal += Weight;
						}
					}

					if (WeightTotal <= 0.0f)
					{
						continue;
					}

					const float AverageHeight = WeightedSum / WeightTotal;
					const int32 Index = GetIndex(X, Y);
					const float PassStrength = 0.34f * Blend;
					NextHeightData[Index] = static_cast<uint16>(FMath::Clamp(FMath::RoundToInt(FMath::Lerp(
						static_cast<float>(CurrentHeightData[Index]),
						AverageHeight,
						PassStrength)), 0, 65535));
				}
			}

			CurrentHeightData = MoveTemp(NextHeightData);
		}

		float LandingHeightSum = 0.0f;
		float LandingWeightSum = 0.0f;
		constexpr float LandingSampleRadius = 2600.0f;
		for (int32 Y = 0; Y < Height; ++Y)
		{
			for (int32 X = 0; X < Width; ++X)
			{
				const int32 LandscapeX = MinX + X;
				const int32 LandscapeY = MinY + Y;
				const FVector WorldPosition = LandscapeTransform.TransformPosition(FVector(static_cast<float>(LandscapeX), static_cast<float>(LandscapeY), 0.0f));
				const FVector2D Position(WorldPosition.X, WorldPosition.Y);
				const float Distance = FVector2D::Distance(Position, ProbeCenter);
				if (Distance > LandingSampleRadius)
				{
					continue;
				}

				const float Weight = 1.0f - FMath::SmoothStep(0.0f, LandingSampleRadius, Distance);
				LandingHeightSum += static_cast<float>(CurrentHeightData[GetIndex(X, Y)]) * Weight;
				LandingWeightSum += Weight;
			}
		}

		if (LandingWeightSum > 0.0f)
		{
			const float LandingHeight = LandingHeightSum / LandingWeightSum;
			constexpr float LandingInnerRadius = 6200.0f;
			constexpr float LandingOuterRadius = 11800.0f;
			for (int32 Y = 0; Y < Height; ++Y)
			{
				for (int32 X = 0; X < Width; ++X)
				{
					const int32 LandscapeX = MinX + X;
					const int32 LandscapeY = MinY + Y;
					const FVector WorldPosition = LandscapeTransform.TransformPosition(FVector(static_cast<float>(LandscapeX), static_cast<float>(LandscapeY), 0.0f));
					const FVector2D Position(WorldPosition.X, WorldPosition.Y);
					const float Distance = FVector2D::Distance(Position, ProbeCenter);
					const bool bLandingCore = Distance <= LandingInnerRadius;
					if (Distance >= LandingOuterRadius ||
						(!bLandingCore && GetNearestWaterDistance(Position) < 5200.0f))
					{
						continue;
					}

					const float LandingBlend = 1.0f - FMath::SmoothStep(LandingInnerRadius, LandingOuterRadius, Distance);
					if (LandingBlend <= 0.01f)
					{
						continue;
					}

					const int32 Index = GetIndex(X, Y);
					CurrentHeightData[Index] = static_cast<uint16>(FMath::Clamp(FMath::RoundToInt(FMath::Lerp(
						static_cast<float>(CurrentHeightData[Index]),
						LandingHeight,
						LandingBlend)), 0, 65535));
				}
			}
		}

		for (int32 Index = 0; Index < HeightData.Num(); ++Index)
		{
			if (CurrentHeightData[Index] == HeightData[Index])
			{
				continue;
			}

			const float DeltaWorld = (static_cast<float>(CurrentHeightData[Index]) - static_cast<float>(OriginalHeightData[Index])) / 128.0f * LandscapeZScale;
			OutMaxAbsDeltaWorld = FMath::Max(OutMaxAbsDeltaWorld, FMath::Abs(DeltaWorld));
			HeightData[Index] = CurrentHeightData[Index];
			++ModifiedSamples;
		}

		return ModifiedSamples;
	}

	struct FLandscapeGroundingAudit
	{
		int32 ProbeCount = 0;
		int32 MissCount = 0;
		int32 NonLandscapeHitCount = 0;
		int32 WalkableCount = 0;
		int32 TooSteepCount = 0;
		float MinGroundZ = TNumericLimits<float>::Max();
		float MaxGroundZ = -TNumericLimits<float>::Max();
	};

	bool TraceLandscapeGround(UWorld* World, const FVector2D& Position, FHitResult& OutHit)
	{
		if (!World)
		{
			return false;
		}

		FCollisionQueryParams QueryParams(FName(TEXT("FFHighlandGroundingAudit")), true);
		QueryParams.bTraceComplex = false;
		QueryParams.bReturnPhysicalMaterial = false;
		return World->LineTraceSingleByChannel(
			OutHit,
			FVector(Position.X, Position.Y, 42000.0f),
			FVector(Position.X, Position.Y, -42000.0f),
			ECC_WorldStatic,
			QueryParams);
	}

	FLandscapeGroundingAudit AuditPlayableLandscapeCollision(UWorld* World)
	{
		FLandscapeGroundingAudit Audit;
		const FVector2D Center(71396.0f, 79714.0f);
		const FVector2D Extents(96500.0f, 111000.0f);
		constexpr float Step = 5500.0f;
		for (float Y = Center.Y - Extents.Y; Y <= Center.Y + Extents.Y; Y += Step)
		{
			for (float X = Center.X - Extents.X; X <= Center.X + Extents.X; X += Step)
			{
				const FVector2D Position(X, Y);
				if (GetIslandOrganicValue(Position) >= 0.68f || GetNearestWaterDistance(Position) < 3200.0f || GetNearestPathDistance(Position) < 1600.0f)
				{
					continue;
				}

				++Audit.ProbeCount;
				FHitResult Hit;
				if (!TraceLandscapeGround(World, Position, Hit) || !Hit.bBlockingHit)
				{
					++Audit.MissCount;
					continue;
				}

				if (!Hit.GetActor() || !Hit.GetActor()->IsA<ALandscapeProxy>())
				{
					++Audit.NonLandscapeHitCount;
					continue;
				}

				Audit.MinGroundZ = FMath::Min(Audit.MinGroundZ, static_cast<float>(Hit.ImpactPoint.Z));
				Audit.MaxGroundZ = FMath::Max(Audit.MaxGroundZ, static_cast<float>(Hit.ImpactPoint.Z));
				if (Hit.ImpactNormal.Z >= 0.72f)
				{
					++Audit.WalkableCount;
				}
				else
				{
					++Audit.TooSteepCount;
				}
			}
		}

		if (Audit.ProbeCount == 0)
		{
			Audit.MinGroundZ = 0.0f;
			Audit.MaxGroundZ = 0.0f;
		}
		return Audit;
	}

	void LogNamedGroundingProbe(UWorld* World, const TCHAR* Label, const FVector2D& Position)
	{
		FHitResult Hit;
		const bool bHit = TraceLandscapeGround(World, Position, Hit) && Hit.bBlockingHit;
		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandTerrainNaturalize: namedProbe label=%s hit=%s actor=%s groundZ=%.1f normalZ=%.3f"),
			Label,
			bHit ? TEXT("true") : TEXT("false"),
			bHit && Hit.GetActor() ? *Hit.GetActor()->GetName() : TEXT("None"),
			bHit ? Hit.ImpactPoint.Z : 0.0f,
			bHit ? Hit.ImpactNormal.Z : 0.0f);
	}

	void ForceFullResolutionLandscapeCollision(ALandscapeProxy* LandscapeProxy)
	{
		if (!LandscapeProxy)
		{
			return;
		}

		LandscapeProxy->Modify();
		for (ULandscapeComponent* Component : LandscapeProxy->LandscapeComponents)
		{
			if (!Component)
			{
				continue;
			}

			Component->Modify();
			Component->CollisionMipLevel = 0;
			Component->SimpleCollisionMipLevel = 0;
			Component->UpdateCollisionData(true);
			Component->MarkPackageDirty();
		}

		LandscapeProxy->RecreateCollisionComponents();
		for (ULandscapeHeightfieldCollisionComponent* CollisionComponent : LandscapeProxy->CollisionComponents)
		{
			if (!CollisionComponent)
			{
				continue;
			}

			CollisionComponent->Modify();
			CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			CollisionComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
			CollisionComponent->RecreateCollision();
			CollisionComponent->MarkPackageDirty();
		}

		LandscapeProxy->FlushGrassComponents(nullptr, true);
		LandscapeProxy->InvalidateGeneratedComponentData(true);
		LandscapeProxy->PostEditChange();
		LandscapeProxy->MarkPackageDirty();
	}
}
#endif

int32 UFFStarterHighlandTerrainNaturalizeCommandlet::Main(const FString& Params)
{
#if WITH_EDITOR
	const bool bCollisionOnly = FParse::Param(*Params, TEXT("CollisionOnly"));
	const bool bSmoothGroundingHills = FParse::Param(*Params, TEXT("SmoothGroundingHills"));
	const bool bPhase2TerrainPolish = FParse::Param(*Params, TEXT("Phase2TerrainPolish"));
	const bool bGameplayFieldV632 = FParse::Param(*Params, TEXT("V632GameplayField"))
		|| Params.Contains(TEXT("V632GameplayField"), ESearchCase::IgnoreCase);
	const bool bTerrainFoundationV81Final = FParse::Param(*Params, TEXT("TerrainFoundationV81Final"))
		|| Params.Contains(TEXT("TerrainFoundationV81Final"), ESearchCase::IgnoreCase);
	const bool bTerrainFoundationV81Refine = !bTerrainFoundationV81Final
		&& (FParse::Param(*Params, TEXT("TerrainFoundationV81Refine"))
			|| Params.Contains(TEXT("TerrainFoundationV81Refine"), ESearchCase::IgnoreCase));
	const bool bTerrainFoundationV81 = !bTerrainFoundationV81Final && !bTerrainFoundationV81Refine
		&& (FParse::Param(*Params, TEXT("TerrainFoundationV81"))
			|| Params.Contains(TEXT("TerrainFoundationV81"), ESearchCase::IgnoreCase));
	const bool bTerrainFoundationV80Refine = !bTerrainFoundationV81 && !bTerrainFoundationV81Refine && !bTerrainFoundationV81Final
		&& (FParse::Param(*Params, TEXT("TerrainFoundationV80Refine"))
			|| Params.Contains(TEXT("TerrainFoundationV80Refine"), ESearchCase::IgnoreCase));
	const bool bTerrainFoundationV80 = !bTerrainFoundationV80Refine && !bTerrainFoundationV81 && !bTerrainFoundationV81Refine && !bTerrainFoundationV81Final
		&& (FParse::Param(*Params, TEXT("TerrainFoundationV80"))
			|| Params.Contains(TEXT("TerrainFoundationV80"), ESearchCase::IgnoreCase));
	const bool bTerrainFoundationV79ValidationRefine = !bTerrainFoundationV80 && !bTerrainFoundationV80Refine
		&& (FParse::Param(*Params, TEXT("TerrainFoundationV79ValidationRefine"))
			|| Params.Contains(TEXT("TerrainFoundationV79ValidationRefine"), ESearchCase::IgnoreCase));
	const bool bTerrainFoundationV79Refine = !bTerrainFoundationV79ValidationRefine && !bTerrainFoundationV80 && !bTerrainFoundationV80Refine
		&& (FParse::Param(*Params, TEXT("TerrainFoundationV79Refine"))
			|| Params.Contains(TEXT("TerrainFoundationV79Refine"), ESearchCase::IgnoreCase));
	const bool bTerrainFoundationV79 = !bTerrainFoundationV79Refine && !bTerrainFoundationV79ValidationRefine && !bTerrainFoundationV80 && !bTerrainFoundationV80Refine
		&& (FParse::Param(*Params, TEXT("TerrainFoundationV79"))
			|| Params.Contains(TEXT("TerrainFoundationV79"), ESearchCase::IgnoreCase));
	const bool bTraversalReadabilityV1 = FParse::Param(*Params, TEXT("TraversalReadabilityV1"))
		|| Params.Contains(TEXT("TraversalReadabilityV1"), ESearchCase::IgnoreCase);
	const bool bTraversalReadabilityV2 = FParse::Param(*Params, TEXT("TraversalReadabilityV2"))
		|| Params.Contains(TEXT("TraversalReadabilityV2"), ESearchCase::IgnoreCase);
	const bool bTraversalReadabilityV3 = FParse::Param(*Params, TEXT("TraversalReadabilityV3"))
		|| Params.Contains(TEXT("TraversalReadabilityV3"), ESearchCase::IgnoreCase);
	const bool bTraversalReadabilityV4 = FParse::Param(*Params, TEXT("TraversalReadabilityV4"))
		|| Params.Contains(TEXT("TraversalReadabilityV4"), ESearchCase::IgnoreCase);
	const bool bTraversalReadabilityV5 = FParse::Param(*Params, TEXT("TraversalReadabilityV5"))
		|| Params.Contains(TEXT("TraversalReadabilityV5"), ESearchCase::IgnoreCase);
	const bool bTraversalReadabilityV6 = FParse::Param(*Params, TEXT("TraversalReadabilityV6"))
		|| Params.Contains(TEXT("TraversalReadabilityV6"), ESearchCase::IgnoreCase);
	const bool bOuterRingGeologyV42 = FParse::Param(*Params, TEXT("OuterRingGeologyV42"))
		|| Params.Contains(TEXT("OuterRingGeologyV42"), ESearchCase::IgnoreCase);
	const bool bOuterRingGeologyV42Refine = FParse::Param(*Params, TEXT("OuterRingGeologyV42Refine"))
		|| Params.Contains(TEXT("OuterRingGeologyV42Refine"), ESearchCase::IgnoreCase);
	const bool bOuterRingGeologyV43 = FParse::Param(*Params, TEXT("OuterRingGeologyV43"))
		|| Params.Contains(TEXT("OuterRingGeologyV43"), ESearchCase::IgnoreCase);
	const bool bOuterRingGeologyV43Refine = FParse::Param(*Params, TEXT("OuterRingGeologyV43Refine"))
		|| Params.Contains(TEXT("OuterRingGeologyV43Refine"), ESearchCase::IgnoreCase);
	const bool bOuterRingGeologyV44 = FParse::Param(*Params, TEXT("OuterRingGeologyV44"))
		|| Params.Contains(TEXT("OuterRingGeologyV44"), ESearchCase::IgnoreCase);
	const bool bOuterRingGeologyV44Refine = FParse::Param(*Params, TEXT("OuterRingGeologyV44Refine"))
		|| Params.Contains(TEXT("OuterRingGeologyV44Refine"), ESearchCase::IgnoreCase);
	const bool bOuterRingGeologyV44Hard = FParse::Param(*Params, TEXT("OuterRingGeologyV44Hard"))
		|| Params.Contains(TEXT("OuterRingGeologyV44Hard"), ESearchCase::IgnoreCase);
	const bool bOuterRingGeologyV44Correction = FParse::Param(*Params, TEXT("OuterRingGeologyV44Correction"))
		|| Params.Contains(TEXT("OuterRingGeologyV44Correction"), ESearchCase::IgnoreCase);
	const bool bOuterRingSilhouetteV51 = FParse::Param(*Params, TEXT("OuterRingSilhouetteV51"))
		|| Params.Contains(TEXT("OuterRingSilhouetteV51"), ESearchCase::IgnoreCase);
	const bool bOuterRingSilhouetteV51Refine = FParse::Param(*Params, TEXT("OuterRingSilhouetteV51Refine"))
		|| Params.Contains(TEXT("OuterRingSilhouetteV51Refine"), ESearchCase::IgnoreCase);
	const bool bOuterRingSilhouetteV51FaceBreak = FParse::Param(*Params, TEXT("OuterRingSilhouetteV51FaceBreak"))
		|| Params.Contains(TEXT("OuterRingSilhouetteV51FaceBreak"), ESearchCase::IgnoreCase);
	const bool bAAATraversalTopographyV83 = FParse::Param(*Params, TEXT("V83AAATraversalTopography"))
		|| Params.Contains(TEXT("V83AAATraversalTopography"), ESearchCase::IgnoreCase);
	const bool bAAATraversalTopographyV83Refine = FParse::Param(*Params, TEXT("V83AAATraversalTopographyRefine"))
		|| Params.Contains(TEXT("V83AAATraversalTopographyRefine"), ESearchCase::IgnoreCase);
	const bool bTraversalMode = bTraversalReadabilityV1 || bTraversalReadabilityV2 || bTraversalReadabilityV3 || bTraversalReadabilityV4 || bTraversalReadabilityV5 || bTraversalReadabilityV6 || bOuterRingGeologyV42 || bOuterRingGeologyV42Refine || bOuterRingGeologyV43 || bOuterRingGeologyV43Refine || bOuterRingGeologyV44 || bOuterRingGeologyV44Refine || bOuterRingGeologyV44Hard || bOuterRingGeologyV44Correction || bOuterRingSilhouetteV51 || bOuterRingSilhouetteV51Refine || bOuterRingSilhouetteV51FaceBreak || bAAATraversalTopographyV83 || bAAATraversalTopographyV83Refine;
	FName ActiveTraversalTag = TraversalReadabilityV1Tag;
	const TCHAR* ActiveTraversalLabel = TEXT("TraversalReadabilityV1");
	if (bAAATraversalTopographyV83Refine)
	{
		ActiveTraversalTag = AAATraversalTopographyV83RefineTag;
		ActiveTraversalLabel = TEXT("V83AAATraversalTopographyRefine");
	}
	else if (bAAATraversalTopographyV83)
	{
		ActiveTraversalTag = AAATraversalTopographyV83Tag;
		ActiveTraversalLabel = TEXT("V83AAATraversalTopography");
	}
	else if (bOuterRingSilhouetteV51FaceBreak)
	{
		ActiveTraversalTag = OuterRingSilhouetteV51FaceBreakTag;
		ActiveTraversalLabel = TEXT("OuterRingSilhouetteV51FaceBreak");
	}
	else if (bOuterRingSilhouetteV51Refine)
	{
		ActiveTraversalTag = OuterRingSilhouetteV51RefineTag;
		ActiveTraversalLabel = TEXT("OuterRingSilhouetteV51Refine");
	}
	else if (bOuterRingSilhouetteV51)
	{
		ActiveTraversalTag = OuterRingSilhouetteV51Tag;
		ActiveTraversalLabel = TEXT("OuterRingSilhouetteV51");
	}
	else if (bOuterRingGeologyV44Correction)
	{
		ActiveTraversalTag = OuterRingGeologyV44CorrectionTag;
		ActiveTraversalLabel = TEXT("OuterRingGeologyV44Correction");
	}
	else if (bOuterRingGeologyV44Hard)
	{
		ActiveTraversalTag = OuterRingGeologyV44HardTag;
		ActiveTraversalLabel = TEXT("OuterRingGeologyV44Hard");
	}
	else if (bOuterRingGeologyV44Refine)
	{
		ActiveTraversalTag = OuterRingGeologyV44RefineTag;
		ActiveTraversalLabel = TEXT("OuterRingGeologyV44Refine");
	}
	else if (bOuterRingGeologyV44)
	{
		ActiveTraversalTag = OuterRingGeologyV44Tag;
		ActiveTraversalLabel = TEXT("OuterRingGeologyV44");
	}
	else if (bOuterRingGeologyV43Refine)
	{
		ActiveTraversalTag = OuterRingGeologyV43RefineTag;
		ActiveTraversalLabel = TEXT("OuterRingGeologyV43Refine");
	}
	else if (bOuterRingGeologyV43)
	{
		ActiveTraversalTag = OuterRingGeologyV43Tag;
		ActiveTraversalLabel = TEXT("OuterRingGeologyV43");
	}
	else if (bOuterRingGeologyV42Refine)
	{
		ActiveTraversalTag = OuterRingGeologyV42RefineTag;
		ActiveTraversalLabel = TEXT("OuterRingGeologyV42Refine");
	}
	else if (bOuterRingGeologyV42)
	{
		ActiveTraversalTag = OuterRingGeologyV42Tag;
		ActiveTraversalLabel = TEXT("OuterRingGeologyV42");
	}
	else if (bTraversalReadabilityV6)
	{
		ActiveTraversalTag = TraversalReadabilityV6Tag;
		ActiveTraversalLabel = TEXT("TraversalReadabilityV6");
	}
	else if (bTraversalReadabilityV5)
	{
		ActiveTraversalTag = TraversalReadabilityV5Tag;
		ActiveTraversalLabel = TEXT("TraversalReadabilityV5");
	}
	else if (bTraversalReadabilityV4)
	{
		ActiveTraversalTag = TraversalReadabilityV4Tag;
		ActiveTraversalLabel = TEXT("TraversalReadabilityV4");
	}
	else if (bTraversalReadabilityV3)
	{
		ActiveTraversalTag = TraversalReadabilityV3Tag;
		ActiveTraversalLabel = TEXT("TraversalReadabilityV3");
	}
	else if (bTraversalReadabilityV2)
	{
		ActiveTraversalTag = TraversalReadabilityV2Tag;
		ActiveTraversalLabel = TEXT("TraversalReadabilityV2");
	}
	const TCHAR* ModeName = bCollisionOnly ? TEXT("CollisionOnly") : (bTerrainFoundationV81Final ? TEXT("TerrainFoundationV81Final") : (bTerrainFoundationV81Refine ? TEXT("TerrainFoundationV81Refine") : (bTerrainFoundationV81 ? TEXT("TerrainFoundationV81") : (bTerrainFoundationV80Refine ? TEXT("TerrainFoundationV80Refine") : (bTerrainFoundationV80 ? TEXT("TerrainFoundationV80") : (bTerrainFoundationV79ValidationRefine ? TEXT("TerrainFoundationV79ValidationRefine") : (bTerrainFoundationV79Refine ? TEXT("TerrainFoundationV79Refine") : (bTerrainFoundationV79 ? TEXT("TerrainFoundationV79") : (bGameplayFieldV632 ? TEXT("V632GameplayField") : (bSmoothGroundingHills ? TEXT("SmoothGroundingHills") : (bPhase2TerrainPolish ? TEXT("Phase2TerrainPolish") : (bTraversalMode ? ActiveTraversalLabel : TEXT("HeightAndCollision")))))))))))));
	UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandTerrainNaturalize: loading %s mode=%s."), HighlandMapPath, ModeName);
	UWorld* World = UEditorLoadingAndSavingUtils::LoadMap(HighlandMapPath);
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandTerrainNaturalize: failed to load map."));
		return 1;
	}

	ALandscapeProxy* LandscapeProxy = nullptr;
	for (TActorIterator<ALandscapeProxy> It(World); It; ++It)
	{
		LandscapeProxy = *It;
		break;
	}
	if (!LandscapeProxy)
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandTerrainNaturalize: no landscape proxy found."));
		return 1;
	}

	bool bTraversalAlreadyApplied = false;
	bool bGameplayFieldAlreadyApplied = false;
	bool bTerrainFoundationAlreadyApplied = false;
	bool bTerrainFoundationRefineAlreadyApplied = false;
	bool bTerrainFoundationValidationRefineAlreadyApplied = false;
	bool bTerrainFoundationV80AlreadyApplied = false;
	bool bTerrainFoundationV80RefineAlreadyApplied = false;
	bool bTerrainFoundationV81AlreadyApplied = false;
	bool bTerrainFoundationV81RefineAlreadyApplied = false;
	bool bTerrainFoundationV81FinalAlreadyApplied = false;
	if (bTraversalMode || bGameplayFieldV632 || bTerrainFoundationV79 || bTerrainFoundationV79Refine || bTerrainFoundationV79ValidationRefine || bTerrainFoundationV80 || bTerrainFoundationV80Refine || bTerrainFoundationV81 || bTerrainFoundationV81Refine || bTerrainFoundationV81Final)
	{
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			if (bTraversalMode && It->ActorHasTag(ActiveTraversalTag))
			{
				bTraversalAlreadyApplied = true;
			}
			if (bGameplayFieldV632 && It->ActorHasTag(GameplayFieldV632Tag))
			{
				bGameplayFieldAlreadyApplied = true;
			}
			if (bTerrainFoundationV79 && It->ActorHasTag(TerrainFoundationV79Tag))
			{
				bTerrainFoundationAlreadyApplied = true;
			}
			if (bTerrainFoundationV79Refine && It->ActorHasTag(TerrainFoundationV79RefineTag))
			{
				bTerrainFoundationRefineAlreadyApplied = true;
			}
			if (bTerrainFoundationV79ValidationRefine && It->ActorHasTag(TerrainFoundationV79ValidationRefineTag))
			{
				bTerrainFoundationValidationRefineAlreadyApplied = true;
			}
			if (bTerrainFoundationV80 && It->ActorHasTag(TerrainFoundationV80Tag))
			{
				bTerrainFoundationV80AlreadyApplied = true;
			}
			if (bTerrainFoundationV80Refine && It->ActorHasTag(TerrainFoundationV80RefineTag))
			{
				bTerrainFoundationV80RefineAlreadyApplied = true;
			}
			if (bTerrainFoundationV81 && It->ActorHasTag(TerrainFoundationV81Tag))
			{
				bTerrainFoundationV81AlreadyApplied = true;
			}
			if (bTerrainFoundationV81Refine && It->ActorHasTag(TerrainFoundationV81RefineTag))
			{
				bTerrainFoundationV81RefineAlreadyApplied = true;
			}
			if (bTerrainFoundationV81Final && It->ActorHasTag(TerrainFoundationV81FinalTag))
			{
				bTerrainFoundationV81FinalAlreadyApplied = true;
			}
			if ((!bTraversalMode || bTraversalAlreadyApplied) && (!bGameplayFieldV632 || bGameplayFieldAlreadyApplied) && (!bTerrainFoundationV79 || bTerrainFoundationAlreadyApplied) && (!bTerrainFoundationV79Refine || bTerrainFoundationRefineAlreadyApplied) && (!bTerrainFoundationV79ValidationRefine || bTerrainFoundationValidationRefineAlreadyApplied) && (!bTerrainFoundationV80 || bTerrainFoundationV80AlreadyApplied) && (!bTerrainFoundationV80Refine || bTerrainFoundationV80RefineAlreadyApplied) && (!bTerrainFoundationV81 || bTerrainFoundationV81AlreadyApplied) && (!bTerrainFoundationV81Refine || bTerrainFoundationV81RefineAlreadyApplied) && (!bTerrainFoundationV81Final || bTerrainFoundationV81FinalAlreadyApplied))
			{
				break;
			}
		}
		if (bTraversalAlreadyApplied)
		{
			UE_LOG(LogTemp, Warning, TEXT("FFStarterHighlandTerrainNaturalize: %s marker already exists; skipping additive height edits and refreshing collision only."), ActiveTraversalLabel);
		}
		if (bGameplayFieldAlreadyApplied)
		{
			UE_LOG(LogTemp, Warning, TEXT("FFStarterHighlandTerrainNaturalize: V632GameplayField marker already exists; skipping additive height edits and refreshing collision only."));
		}
		if (bTerrainFoundationAlreadyApplied)
		{
			UE_LOG(LogTemp, Warning, TEXT("FFStarterHighlandTerrainNaturalize: TerrainFoundationV79 marker already exists; skipping additive height edits and refreshing collision only."));
		}
		if (bTerrainFoundationRefineAlreadyApplied)
		{
			UE_LOG(LogTemp, Warning, TEXT("FFStarterHighlandTerrainNaturalize: TerrainFoundationV79Refine marker already exists; skipping additive height edits and refreshing collision only."));
		}
		if (bTerrainFoundationValidationRefineAlreadyApplied)
		{
			UE_LOG(LogTemp, Warning, TEXT("FFStarterHighlandTerrainNaturalize: TerrainFoundationV79ValidationRefine marker already exists; skipping additive height edits and refreshing collision only."));
		}
		if (bTerrainFoundationV80AlreadyApplied)
		{
			UE_LOG(LogTemp, Warning, TEXT("FFStarterHighlandTerrainNaturalize: TerrainFoundationV80 marker already exists; skipping additive height edits and refreshing collision only."));
		}
		if (bTerrainFoundationV80RefineAlreadyApplied)
		{
			UE_LOG(LogTemp, Warning, TEXT("FFStarterHighlandTerrainNaturalize: TerrainFoundationV80Refine marker already exists; skipping additive height edits and refreshing collision only."));
		}
		if (bTerrainFoundationV81AlreadyApplied)
		{
			UE_LOG(LogTemp, Warning, TEXT("FFStarterHighlandTerrainNaturalize: TerrainFoundationV81 marker already exists; skipping additive height edits and refreshing collision only."));
		}
		if (bTerrainFoundationV81RefineAlreadyApplied)
		{
			UE_LOG(LogTemp, Warning, TEXT("FFStarterHighlandTerrainNaturalize: TerrainFoundationV81Refine marker already exists; skipping additive height edits and refreshing collision only."));
		}
		if (bTerrainFoundationV81FinalAlreadyApplied)
		{
			UE_LOG(LogTemp, Warning, TEXT("FFStarterHighlandTerrainNaturalize: TerrainFoundationV81Final marker already exists; skipping additive height edits and refreshing collision only."));
		}
	}

	ULandscapeInfo* LandscapeInfo = LandscapeProxy->GetLandscapeInfo();
	if (!LandscapeInfo)
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandTerrainNaturalize: landscape info missing."));
		return 1;
	}

	int32 MinX = 0;
	int32 MinY = 0;
	int32 MaxX = 0;
	int32 MaxY = 0;
	if (!LandscapeInfo->GetLandscapeExtent(MinX, MinY, MaxX, MaxY))
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandTerrainNaturalize: failed to read landscape extent."));
		return 1;
	}

	const int32 Width = MaxX - MinX + 1;
	const int32 Height = MaxY - MinY + 1;
	int32 ModifiedSamples = 0;
	float MaxAbsDeltaWorld = 0.0f;
	const FTransform LandscapeTransform = LandscapeProxy->LandscapeActorToWorld();
	const float LandscapeZScale = FMath::Max(FMath::Abs(LandscapeTransform.GetScale3D().Z), 1.0f);
	if (!bCollisionOnly && !bTraversalAlreadyApplied && !bGameplayFieldAlreadyApplied && !bTerrainFoundationAlreadyApplied && !bTerrainFoundationRefineAlreadyApplied && !bTerrainFoundationValidationRefineAlreadyApplied && !bTerrainFoundationV80AlreadyApplied && !bTerrainFoundationV80RefineAlreadyApplied && !bTerrainFoundationV81AlreadyApplied && !bTerrainFoundationV81RefineAlreadyApplied && !bTerrainFoundationV81FinalAlreadyApplied)
	{
		TArray<uint16> HeightData;
		HeightData.SetNumZeroed(Width * Height);
		int32 ReadMinX = MinX;
		int32 ReadMinY = MinY;
		int32 ReadMaxX = MaxX;
		int32 ReadMaxY = MaxY;
		FLandscapeEditDataInterface EditInterface(LandscapeInfo);
		EditInterface.GetHeightData(ReadMinX, ReadMinY, ReadMaxX, ReadMaxY, HeightData.GetData(), 0);

		if (bSmoothGroundingHills)
		{
			ModifiedSamples = SmoothGroundingProbeHill(HeightData, Width, Height, MinX, MinY, LandscapeTransform, LandscapeZScale, MaxAbsDeltaWorld);
		}
		else
		{
			for (int32 Y = 0; Y < Height; ++Y)
			{
				for (int32 X = 0; X < Width; ++X)
				{
					const int32 LandscapeX = MinX + X;
					const int32 LandscapeY = MinY + Y;
					const FVector WorldPosition = LandscapeTransform.TransformPosition(FVector(static_cast<float>(LandscapeX), static_cast<float>(LandscapeY), 0.0f));
					const FVector2D WorldXY(WorldPosition.X, WorldPosition.Y);
					float DeltaWorld = 0.0f;
					if (bTerrainFoundationV81Final)
					{
						DeltaWorld = ComputeTerrainFoundationV81FinalHeightDelta(WorldXY);
					}
					else if (bTerrainFoundationV81Refine)
					{
						DeltaWorld = ComputeTerrainFoundationV81RefineHeightDelta(WorldXY);
					}
					else if (bTerrainFoundationV81)
					{
						DeltaWorld = ComputeTerrainFoundationV81HeightDelta(WorldXY);
					}
					else if (bTerrainFoundationV80Refine)
					{
						DeltaWorld = ComputeTerrainFoundationV80RefineHeightDelta(WorldXY);
					}
					else if (bTerrainFoundationV80)
					{
						DeltaWorld = ComputeTerrainFoundationV80HeightDelta(WorldXY);
					}
					else if (bTerrainFoundationV79ValidationRefine)
					{
						DeltaWorld = ComputeTerrainFoundationV79ValidationRefineHeightDelta(WorldXY);
					}
					else if (bTerrainFoundationV79Refine)
					{
						DeltaWorld = ComputeTerrainFoundationV79RefineHeightDelta(WorldXY);
					}
					else if (bTerrainFoundationV79)
					{
						DeltaWorld = ComputeTerrainFoundationV79HeightDelta(WorldXY);
					}
					else if (bGameplayFieldV632)
					{
						DeltaWorld = ComputeGameplayFieldV632HeightDelta(WorldXY);
					}
					else if (bAAATraversalTopographyV83Refine)
					{
						DeltaWorld = ComputeAAATraversalTopographyV83RefineHeightDelta(WorldXY);
					}
					else if (bAAATraversalTopographyV83)
					{
						DeltaWorld = ComputeAAATraversalTopographyV83HeightDelta(WorldXY);
					}
					else if (bOuterRingSilhouetteV51FaceBreak)
					{
						DeltaWorld = ComputeOuterRingSilhouetteV51FaceBreakHeightDelta(WorldXY);
					}
					else if (bOuterRingSilhouetteV51Refine)
					{
						DeltaWorld = ComputeOuterRingSilhouetteV51RefineHeightDelta(WorldXY);
					}
					else if (bOuterRingSilhouetteV51)
					{
						DeltaWorld = ComputeOuterRingSilhouetteV51HeightDelta(WorldXY);
					}
					else if (bOuterRingGeologyV44Correction)
					{
						DeltaWorld = ComputeOuterRingGeologyV44CorrectionHeightDelta(WorldXY);
					}
					else if (bOuterRingGeologyV44Refine)
					{
						DeltaWorld = ComputeOuterRingGeologyV44RefineHeightDelta(WorldXY);
					}
					else if (bOuterRingGeologyV44Hard)
					{
						DeltaWorld = ComputeOuterRingGeologyV44HardHeightDelta(WorldXY);
					}
					else if (bOuterRingGeologyV44)
					{
						DeltaWorld = ComputeOuterRingGeologyV44HeightDelta(WorldXY);
					}
					else if (bOuterRingGeologyV43Refine)
					{
						DeltaWorld = ComputeOuterRingGeologyV43RefineHeightDelta(WorldXY);
					}
					else if (bOuterRingGeologyV43)
					{
						DeltaWorld = ComputeOuterRingGeologyV43HeightDelta(WorldXY);
					}
					else if (bOuterRingGeologyV42Refine)
					{
						DeltaWorld = ComputeOuterRingGeologyV42RefineHeightDelta(WorldXY);
					}
					else if (bOuterRingGeologyV42)
					{
						DeltaWorld = ComputeOuterRingGeologyV42HeightDelta(WorldXY);
					}
					else if (bTraversalReadabilityV6)
					{
						DeltaWorld = ComputeTraversalReadabilityV6HeightDelta(WorldXY);
					}
					else if (bTraversalReadabilityV5)
					{
						DeltaWorld = ComputeTraversalReadabilityV5HeightDelta(WorldXY);
					}
					else if (bTraversalReadabilityV4)
					{
						DeltaWorld = ComputeTraversalReadabilityV4HeightDelta(WorldXY);
					}
					else if (bTraversalReadabilityV3)
					{
						DeltaWorld = ComputeTraversalReadabilityV3HeightDelta(WorldXY);
					}
					else if (bTraversalReadabilityV2)
					{
						DeltaWorld = ComputeTraversalReadabilityV2HeightDelta(WorldXY);
					}
					else if (bTraversalReadabilityV1)
					{
						DeltaWorld = ComputeTraversalReadabilityHeightDelta(WorldXY);
					}
					else if (bPhase2TerrainPolish)
					{
						DeltaWorld = ComputePhase2PositiveHeightDelta(WorldXY);
					}
					else
					{
						DeltaWorld = ComputeNaturalHeightDelta(WorldXY);
					}
					if (FMath::Abs(DeltaWorld) <= 1.0f)
					{
						continue;
					}

					const int32 Index = Y * Width + X;
					const int32 EncodedDelta = FMath::RoundToInt((DeltaWorld / LandscapeZScale) * 128.0f);
					const int32 NewHeight = FMath::Clamp(static_cast<int32>(HeightData[Index]) + EncodedDelta, 0, 65535);
					if (NewHeight != HeightData[Index])
					{
						HeightData[Index] = static_cast<uint16>(NewHeight);
						MaxAbsDeltaWorld = FMath::Max(MaxAbsDeltaWorld, FMath::Abs(DeltaWorld));
						++ModifiedSamples;
					}
				}
			}
		}

		EditInterface.SetHeightData(MinX, MinY, MaxX, MaxY, HeightData.GetData(), 0, true, nullptr, nullptr, nullptr, false, nullptr, nullptr, true, true, true);
	}

	if (bTraversalMode && !bTraversalAlreadyApplied && ModifiedSamples > 0)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AActor* MarkerActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector(76500.0f, 124000.0f, 2500.0f), FRotator::ZeroRotator, SpawnParameters);
		if (MarkerActor)
		{
			MarkerActor->Tags.AddUnique(ActiveTraversalTag);
			MarkerActor->SetActorLabel(FString::Printf(TEXT("FF_%s_AppliedMarker"), ActiveTraversalLabel));
			MarkerActor->SetIsTemporarilyHiddenInEditor(true);
			MarkerActor->SetActorHiddenInGame(true);
			MarkerActor->MarkPackageDirty();
		}
	}
	if (bGameplayFieldV632 && !bGameplayFieldAlreadyApplied && ModifiedSamples > 0)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AActor* MarkerActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector(54500.0f, 126000.0f, 2500.0f), FRotator::ZeroRotator, SpawnParameters);
		if (MarkerActor)
		{
			MarkerActor->Tags.AddUnique(GameplayFieldV632Tag);
			MarkerActor->SetActorLabel(TEXT("FF_V632GameplayField_AppliedMarker"));
			MarkerActor->SetIsTemporarilyHiddenInEditor(true);
			MarkerActor->SetActorHiddenInGame(true);
			MarkerActor->MarkPackageDirty();
		}
	}
	if (bTerrainFoundationV79 && !bTerrainFoundationAlreadyApplied && ModifiedSamples > 0)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AActor* MarkerActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector(76500.0f, 124000.0f, 2500.0f), FRotator::ZeroRotator, SpawnParameters);
		if (MarkerActor)
		{
			MarkerActor->Tags.AddUnique(TerrainFoundationV79Tag);
			MarkerActor->SetActorLabel(TEXT("FF_TerrainFoundationV79_AppliedMarker"));
			MarkerActor->SetIsTemporarilyHiddenInEditor(true);
			MarkerActor->SetActorHiddenInGame(true);
			MarkerActor->MarkPackageDirty();
		}
	}
	if (bTerrainFoundationV79Refine && !bTerrainFoundationRefineAlreadyApplied && ModifiedSamples > 0)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AActor* MarkerActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector(73000.0f, 121500.0f, 2500.0f), FRotator::ZeroRotator, SpawnParameters);
		if (MarkerActor)
		{
			MarkerActor->Tags.AddUnique(TerrainFoundationV79RefineTag);
			MarkerActor->SetActorLabel(TEXT("FF_TerrainFoundationV79Refine_AppliedMarker"));
			MarkerActor->SetIsTemporarilyHiddenInEditor(true);
			MarkerActor->SetActorHiddenInGame(true);
			MarkerActor->MarkPackageDirty();
		}
	}
	if (bTerrainFoundationV79ValidationRefine && !bTerrainFoundationValidationRefineAlreadyApplied && ModifiedSamples > 0)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AActor* MarkerActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector(52000.0f, 143500.0f, 2500.0f), FRotator::ZeroRotator, SpawnParameters);
		if (MarkerActor)
		{
			MarkerActor->Tags.AddUnique(TerrainFoundationV79ValidationRefineTag);
			MarkerActor->SetActorLabel(TEXT("FF_TerrainFoundationV79ValidationRefine_AppliedMarker"));
			MarkerActor->SetIsTemporarilyHiddenInEditor(true);
			MarkerActor->SetActorHiddenInGame(true);
			MarkerActor->MarkPackageDirty();
		}
	}
	if (bTerrainFoundationV80 && !bTerrainFoundationV80AlreadyApplied && ModifiedSamples > 0)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AActor* MarkerActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector(76500.0f, 124000.0f, 2500.0f), FRotator::ZeroRotator, SpawnParameters);
		if (MarkerActor)
		{
			MarkerActor->Tags.AddUnique(TerrainFoundationV80Tag);
			MarkerActor->SetActorLabel(TEXT("FF_TerrainFoundationV80_AppliedMarker"));
			MarkerActor->SetIsTemporarilyHiddenInEditor(true);
			MarkerActor->SetActorHiddenInGame(true);
			MarkerActor->MarkPackageDirty();
		}
	}
	if (bTerrainFoundationV80Refine && !bTerrainFoundationV80RefineAlreadyApplied && ModifiedSamples > 0)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AActor* MarkerActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector(61000.0f, 76000.0f, 2500.0f), FRotator::ZeroRotator, SpawnParameters);
		if (MarkerActor)
		{
			MarkerActor->Tags.AddUnique(TerrainFoundationV80RefineTag);
			MarkerActor->SetActorLabel(TEXT("FF_TerrainFoundationV80Refine_AppliedMarker"));
			MarkerActor->SetIsTemporarilyHiddenInEditor(true);
			MarkerActor->SetActorHiddenInGame(true);
			MarkerActor->MarkPackageDirty();
		}
	}
	if (bTerrainFoundationV81 && !bTerrainFoundationV81AlreadyApplied && ModifiedSamples > 0)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AActor* MarkerActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector(77000.0f, 124500.0f, 2500.0f), FRotator::ZeroRotator, SpawnParameters);
		if (MarkerActor)
		{
			MarkerActor->Tags.AddUnique(TerrainFoundationV81Tag);
			MarkerActor->SetActorLabel(TEXT("FF_TerrainFoundationV81_AppliedMarker"));
			MarkerActor->SetIsTemporarilyHiddenInEditor(true);
			MarkerActor->SetActorHiddenInGame(true);
			MarkerActor->MarkPackageDirty();
		}
	}
	if (bTerrainFoundationV81Refine && !bTerrainFoundationV81RefineAlreadyApplied && ModifiedSamples > 0)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AActor* MarkerActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector(67500.0f, 40500.0f, 2500.0f), FRotator::ZeroRotator, SpawnParameters);
		if (MarkerActor)
		{
			MarkerActor->Tags.AddUnique(TerrainFoundationV81RefineTag);
			MarkerActor->SetActorLabel(TEXT("FF_TerrainFoundationV81Refine_AppliedMarker"));
			MarkerActor->SetIsTemporarilyHiddenInEditor(true);
			MarkerActor->SetActorHiddenInGame(true);
			MarkerActor->MarkPackageDirty();
		}
	}
	if (bTerrainFoundationV81Final && !bTerrainFoundationV81FinalAlreadyApplied && ModifiedSamples > 0)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AActor* MarkerActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector(61000.0f, 76000.0f, 2500.0f), FRotator::ZeroRotator, SpawnParameters);
		if (MarkerActor)
		{
			MarkerActor->Tags.AddUnique(TerrainFoundationV81FinalTag);
			MarkerActor->SetActorLabel(TEXT("FF_TerrainFoundationV81Final_AppliedMarker"));
			MarkerActor->SetIsTemporarilyHiddenInEditor(true);
			MarkerActor->SetActorHiddenInGame(true);
			MarkerActor->MarkPackageDirty();
		}
	}
	// Let FLandscapeEditDataInterface release heightmap texture locks before the save path recompresses them.
	ForceFullResolutionLandscapeCollision(LandscapeProxy);
	LandscapeProxy->MarkPackageDirty();
	World->MarkPackageDirty();
	LogNamedGroundingProbe(World, TEXT("NorthWestSoftHill"), FVector2D(42000.0f, 132000.0f));
	const FLandscapeGroundingAudit Audit = AuditPlayableLandscapeCollision(World);

	const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
	const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
	UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandTerrainNaturalize: extent=(%d,%d)-(%d,%d) samples=%d modified=%d maxDeltaWorld=%.1f auditProbes=%d walkable=%d tooSteep=%d misses=%d nonLandscape=%d groundZ=(%.1f..%.1f) savedMap=%s savedPackages=%s"),
		MinX,
		MinY,
		MaxX,
		MaxY,
		Width * Height,
		ModifiedSamples,
		MaxAbsDeltaWorld,
		Audit.ProbeCount,
		Audit.WalkableCount,
		Audit.TooSteepCount,
		Audit.MissCount,
		Audit.NonLandscapeHitCount,
		Audit.MinGroundZ,
		Audit.MaxGroundZ,
		bSavedMap ? TEXT("true") : TEXT("false"),
		bSavedPackages ? TEXT("true") : TEXT("false"));

	const bool bAuditPassed = Audit.ProbeCount > 0 && Audit.MissCount == 0 && Audit.NonLandscapeHitCount == 0 && Audit.WalkableCount > 0;
	return (bSavedMap && bSavedPackages && bAuditPassed && (bCollisionOnly || bTraversalAlreadyApplied || bGameplayFieldAlreadyApplied || bTerrainFoundationAlreadyApplied || bTerrainFoundationRefineAlreadyApplied || bTerrainFoundationValidationRefineAlreadyApplied || bTerrainFoundationV80AlreadyApplied || bTerrainFoundationV80RefineAlreadyApplied || bTerrainFoundationV81AlreadyApplied || bTerrainFoundationV81RefineAlreadyApplied || bTerrainFoundationV81FinalAlreadyApplied || ModifiedSamples > 0)) ? 0 : 1;
#else
	UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandTerrainNaturalize can only run in editor builds."));
	return 1;
#endif
}
