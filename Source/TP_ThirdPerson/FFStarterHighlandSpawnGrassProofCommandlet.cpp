#include "FFStarterHighlandSpawnGrassProofCommandlet.h"

#if WITH_EDITOR
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "FileHelpers.h"
#include "GameFramework/Actor.h"
#include "Landscape.h"
#include "LandscapeGrassType.h"
#include "LandscapeInfo.h"
#include "LandscapeProxy.h"
#include "Materials/Material.h"
#include "Materials/MaterialInterface.h"
#include "UObject/Package.h"
#include "VT/RuntimeVirtualTexture.h"
#include "VT/RuntimeVirtualTextureVolume.h"
#include "Components/RuntimeVirtualTextureComponent.h"
#endif

#if WITH_EDITOR
namespace
{
	const TCHAR* HighlandMapPath = TEXT("/Game/Maps/FF_Starter_Highland_Blockout");
	const FName SpawnGrassProofTag(TEXT("FFSpawnGrassProof"));
	const FName Phase1GrassTag(TEXT("FFPhase1GrassPlacement"));
	const FName NativeTestPatchTag(TEXT("FFHighlandTitanGrassNativeTestPatch"));
	const FName ABTestPatchTag(TEXT("FFHighlandGrassABTestPatch"));
	const TCHAR* BlockedLandscapeGrassTypePath = TEXT("/Game/FantasyFrontier/Blockout/Materials/GT_FF_Blockout_LandscapeGrass.GT_FF_Blockout_LandscapeGrass");
	const TCHAR* ProxyGrassMaterialPath = TEXT("/Game/FantasyFrontier/Blockout/Materials/M_FF_Highland_GrassBlade_Soft.M_FF_Highland_GrassBlade_Soft");
	const TCHAR* NativeGrassMaterialPath = TEXT("/Game/Environment/Foliage/Materials/MI_GrassBlade.MI_GrassBlade");
	const TCHAR* NativeGrassDarkerMaterialPath = TEXT("/Game/Environment/Foliage/Materials/MI_GrassBladeDarker.MI_GrassBladeDarker");
	const TCHAR* TitanGrassTypePath = TEXT("/Game/Landscape/LGT/LGT_Grass.LGT_Grass");
	const TCHAR* TitanGroundMaterialPath = TEXT("/Game/Landscape/Materials/MI_LandscapeMain.MI_LandscapeMain");
	const TCHAR* TitanRVTDPath = TEXT("/Game/Landscape/RVT/RVT_Titan_D.RVT_Titan_D");
	const TCHAR* TitanRVTHPath = TEXT("/Game/Landscape/RVT/RVT_Titan_H.RVT_Titan_H");
	const TCHAR* TitanRVTDHPath = TEXT("/Game/Landscape/RVT/RVT_Titan_DH.RVT_Titan_DH");
	const TCHAR* EnginePlaneMeshPath = TEXT("/Engine/BasicShapes/Plane.Plane");

	const TCHAR* GrassMeshPaths[] = {
		TEXT("/Game/Environment/Foliage/Meshes/SM_GrassBlade.SM_GrassBlade"),
		TEXT("/Game/Environment/Foliage/Meshes/SM_GrassBlade.SM_GrassBlade"),
		TEXT("/Game/Environment/Foliage/Grass/Ryegrass_Grass_C.Ryegrass_Grass_C")
	};

	const TCHAR* GrassMaterialPaths[] = {
		TEXT("/Game/FantasyFrontier/Blockout/Materials/M_FF_Highland_GrassBlade_Soft.M_FF_Highland_GrassBlade_Soft"),
		TEXT("/Game/FantasyFrontier/Blockout/Materials/M_FF_Highland_GrassBlade_Soft.M_FF_Highland_GrassBlade_Soft"),
		TEXT("/Game/FantasyFrontier/Blockout/Materials/M_FF_Highland_GrassBlade_Soft.M_FF_Highland_GrassBlade_Soft")
	};

	struct FGrassPlacementZone
	{
		const TCHAR* Name;
		FVector2D Center;
		FVector2D Extents;
		float RotationDegrees;
		float GroundZ;
		float ZVariation;
		int32 InstanceCounts[3];
		bool bRequireShoreBand;
		bool bAllowSpawnDryOverride;
	};

	struct FGrassExclusionCircle
	{
		FVector2D Center;
		float Radius;
		FString Source;
	};

	const FGrassPlacementZone GrassZones[] = {
		{ TEXT("TitanCarpetFullInteriorA"), FVector2D(65000.0f, 98000.0f), FVector2D(76000.0f, 96000.0f), -4.0f, -5100.0f, 70.0f, { 80000, 54000, 39000 }, false, false },
		{ TEXT("TitanCarpetFullInteriorB"), FVector2D(59000.0f, 112000.0f), FVector2D(65000.0f, 80000.0f), 11.0f, -4700.0f, 58.0f, { 42000, 28000, 21000 }, false, false },
		{ TEXT("TitanCarpetNorthBand"), FVector2D(58500.0f, 154000.0f), FVector2D(54000.0f, 36000.0f), 2.0f, -3950.0f, 46.0f, { 25000, 16000, 12000 }, false, false },
		{ TEXT("TitanCarpetSouthBand"), FVector2D(61500.0f, 47000.0f), FVector2D(59000.0f, 39000.0f), -8.0f, -6900.0f, 58.0f, { 25000, 16000, 12000 }, false, false },
		{ TEXT("TitanCarpetWestLobe"), FVector2D(25500.0f, 88000.0f), FVector2D(35000.0f, 59000.0f), 6.0f, -5350.0f, 46.0f, { 20000, 12500, 9000 }, false, false },
		{ TEXT("TitanCarpetEastLobe"), FVector2D(92500.0f, 91000.0f), FVector2D(39000.0f, 56000.0f), -13.0f, -5200.0f, 46.0f, { 22000, 14000, 10000 }, false, false },
		{ TEXT("TitanSpawnMeadowBlend"), FVector2D(38550.0f, 147850.0f), FVector2D(13200.0f, 10800.0f), 0.0f, -3200.0f, 16.0f, { 16000, 10000, 7500 }, false, true }
	};

	struct FGrassCarpetLayer
	{
		const TCHAR* Name;
		float Spacing;
		float KeepChance;
		float MinXYScale;
		float MaxXYScale;
		float MinZScale;
		float MaxZScale;
		float JitterRatio;
		float MinNormalZ;
		int32 EndCullDistance;
	};

	const FGrassCarpetLayer GrassCarpetLayers[] = {
		{ TEXT("ShortBaseFill"), 198.0f, 0.998f, 1.08f, 1.55f, 0.42f, 0.62f, 0.78f, 0.870f, 22000 },
		{ TEXT("MediumPatternBreakFill"), 760.0f, 0.82f, 0.92f, 1.34f, 0.40f, 0.64f, 0.92f, 0.880f, 20000 },
		{ TEXT("TallSoftAccentsDisabled"), 1280.0f, 0.0f, 0.64f, 0.92f, 0.86f, 1.16f, 0.70f, 0.930f, 42000 }
	};

	const FVector2D CarpetCenter(71396.0f, 79714.0f);
	const FVector2D CarpetExtents(96500.0f, 111000.0f);
	constexpr float MaxGrassHeightZ = 4200.0f;

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

	bool ContainsAnyToken(const FString& Source, const TArray<FString>& Tokens)
	{
		for (const FString& Token : Tokens)
		{
			if (Source.Contains(Token))
			{
				return true;
			}
		}

		return false;
	}

	bool IsGrassBlockerSource(const FString& SourceName)
	{
		const FString LowerName = SourceName.ToLower();
		const TArray<FString> IgnoredTokens = {
			TEXT("water"), TEXT("river"), TEXT("lake"), TEXT("pond"), TEXT("ocean"),
			TEXT("landscape"), TEXT("playerstart"), TEXT("grasscarpet"), TEXT("spawngrassproof")
		};
		if (ContainsAnyToken(LowerName, IgnoredTokens))
		{
			return false;
		}

		const TArray<FString> BlockerTokens = {
			TEXT("tree"), TEXT("olive"), TEXT("bush"), TEXT("shrub"), TEXT("understory"),
			TEXT("undergrowth"), TEXT("vegetation"), TEXT("foliage"), TEXT("rock"),
			TEXT("boulder"), TEXT("trunk"), TEXT("stump")
		};
		return ContainsAnyToken(LowerName, BlockerTokens);
	}

	float GetExclusionRadiusForSource(const FString& SourceName, const FVector& BoundsExtent, const FVector& Scale)
	{
		const FString LowerName = SourceName.ToLower();
		const float MaxExtent = FMath::Max(BoundsExtent.X, BoundsExtent.Y);
		const float MaxScale = FMath::Max(Scale.X, Scale.Y);
		if (LowerName.Contains(TEXT("tree")) || LowerName.Contains(TEXT("olive")) || LowerName.Contains(TEXT("trunk")))
		{
			return FMath::Clamp(MaxExtent * MaxScale * 0.18f + 240.0f, 320.0f, 860.0f);
		}
		if (LowerName.Contains(TEXT("bush")) || LowerName.Contains(TEXT("shrub")) || LowerName.Contains(TEXT("understory")) || LowerName.Contains(TEXT("undergrowth")))
		{
			return FMath::Clamp(MaxExtent * MaxScale * 0.48f + 180.0f, 260.0f, 640.0f);
		}
		if (LowerName.Contains(TEXT("rock")) || LowerName.Contains(TEXT("boulder")))
		{
			return FMath::Clamp(MaxExtent * MaxScale * 0.80f + 190.0f, 320.0f, 1800.0f);
		}

		return FMath::Clamp(MaxExtent * MaxScale * 0.45f + 180.0f, 280.0f, 900.0f);
	}

	void BuildGrassExclusionCircles(UWorld* World, TArray<FGrassExclusionCircle>& OutExclusions)
	{
		if (!World)
		{
			return;
		}

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (!Actor || Actor->IsA<ALandscapeProxy>() || Actor->ActorHasTag(SpawnGrassProofTag) || Actor->ActorHasTag(Phase1GrassTag) || Actor->ActorHasTag(NativeTestPatchTag))
			{
				continue;
			}

			const FString ActorSourceName = Actor->GetActorLabel() + TEXT("_") + Actor->GetName();
			TArray<UInstancedStaticMeshComponent*> InstancedComponents;
			Actor->GetComponents(InstancedComponents);
			for (UInstancedStaticMeshComponent* Component : InstancedComponents)
			{
				if (!Component || !Component->GetStaticMesh())
				{
					continue;
				}

				const FString SourceName = ActorSourceName + TEXT("_") + Component->GetName() + TEXT("_") + Component->GetStaticMesh()->GetName();
				if (!IsGrassBlockerSource(SourceName))
				{
					continue;
				}

				const FVector MeshExtent = Component->GetStaticMesh()->GetBounds().BoxExtent;
				for (int32 InstanceIndex = 0; InstanceIndex < Component->GetInstanceCount(); ++InstanceIndex)
				{
					FTransform InstanceTransform;
					if (!Component->GetInstanceTransform(InstanceIndex, InstanceTransform, true))
					{
						continue;
					}

					OutExclusions.Add({
						FVector2D(InstanceTransform.GetLocation()),
						GetExclusionRadiusForSource(SourceName, MeshExtent, InstanceTransform.GetScale3D()),
						SourceName
					});
				}
			}

			TArray<UStaticMeshComponent*> StaticMeshComponents;
			Actor->GetComponents(StaticMeshComponents);
			for (UStaticMeshComponent* Component : StaticMeshComponents)
			{
				if (!Component || Component->IsA<UInstancedStaticMeshComponent>() || !Component->GetStaticMesh())
				{
					continue;
				}

				const FString SourceName = ActorSourceName + TEXT("_") + Component->GetName() + TEXT("_") + Component->GetStaticMesh()->GetName();
				if (!IsGrassBlockerSource(SourceName))
				{
					continue;
				}

				OutExclusions.Add({
					FVector2D(Component->Bounds.Origin),
					GetExclusionRadiusForSource(SourceName, Component->Bounds.BoxExtent, Component->GetComponentScale()),
					SourceName
				});
			}
		}
	}

	bool IsInsideGrassExclusion(const TArray<FGrassExclusionCircle>& Exclusions, const FVector2D& Position)
	{
		for (const FGrassExclusionCircle& Exclusion : Exclusions)
		{
			if (FVector2D::DistSquared(Position, Exclusion.Center) <= FMath::Square(Exclusion.Radius))
			{
				return true;
			}
		}

		return false;
	}

	bool ShouldHideVegetationActor(AActor* Actor)
	{
		if (!Actor || Actor->IsA<ALandscapeProxy>() || Actor->ActorHasTag(SpawnGrassProofTag) || Actor->ActorHasTag(Phase1GrassTag) || Actor->ActorHasTag(NativeTestPatchTag))
		{
			return false;
		}

		const FString ActorSourceName = Actor->GetActorLabel() + TEXT("_") + Actor->GetName();
		const FString LowerActorName = ActorSourceName.ToLower();
		if (LowerActorName.Contains(TEXT("water")) || LowerActorName.Contains(TEXT("river")) || LowerActorName.Contains(TEXT("lake")) || LowerActorName.Contains(TEXT("pond")) || LowerActorName.Contains(TEXT("playerstart")))
		{
			return false;
		}

		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Actor->GetComponents(StaticMeshComponents);
		for (UStaticMeshComponent* Component : StaticMeshComponents)
		{
			if (!Component || !Component->GetStaticMesh())
			{
				continue;
			}

			const FString SourceName = ActorSourceName + TEXT("_") + Component->GetName() + TEXT("_") + Component->GetStaticMesh()->GetName();
			const FString LowerSourceName = SourceName.ToLower();
			if (LowerSourceName.Contains(TEXT("tree")) || LowerSourceName.Contains(TEXT("olive")) || LowerSourceName.Contains(TEXT("bush")) || LowerSourceName.Contains(TEXT("shrub")) || LowerSourceName.Contains(TEXT("understory")) || LowerSourceName.Contains(TEXT("undergrowth")) || LowerSourceName.Contains(TEXT("vegetation")))
			{
				return true;
			}
		}

		return false;
	}

	void HideVegetationForGrassValidation(UWorld* World, int32& OutHiddenActors, int32& OutHiddenComponents)
	{
		OutHiddenActors = 0;
		OutHiddenComponents = 0;
		if (!World)
		{
			return;
		}

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (!ShouldHideVegetationActor(Actor))
			{
				continue;
			}

			Actor->Modify();
			Actor->SetActorHiddenInGame(true);
			Actor->SetActorEnableCollision(false);
			++OutHiddenActors;

			TArray<UActorComponent*> Components;
			Actor->GetComponents(Components);
			for (UActorComponent* Component : Components)
			{
				UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(Component);
				if (!PrimitiveComponent)
				{
					continue;
				}

				PrimitiveComponent->Modify();
				PrimitiveComponent->SetVisibility(false, true);
				PrimitiveComponent->SetHiddenInGame(true, true);
				PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				PrimitiveComponent->MarkRenderStateDirty();
				++OutHiddenComponents;
			}

			Actor->MarkPackageDirty();
		}
	}

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
		const float Lake0Distance = (Lake0.Size() - 1.0f) * 5400.0f;
		const FVector2D Lake1 = (Position - FVector2D(25640.0f, 72820.0f)) / FVector2D(4700.0f, 1900.0f);
		const float Lake1Distance = (Lake1.Size() - 1.0f) * 6000.0f;
		const FVector2D PondA = (Position - FVector2D(45457.0f, 136962.0f)) / FVector2D(3700.0f, 1700.0f);
		const FVector2D PondB = (Position - FVector2D(19220.0f, 124250.0f)) / FVector2D(3100.0f, 1600.0f);
		const FVector2D PondC = (Position - FVector2D(87614.0f, 97369.0f)) / FVector2D(2300.0f, 1650.0f);
		return FMath::Min(
			FMath::Min(Nearest, Lake0Distance),
			FMath::Min(
				Lake1Distance,
				FMath::Min(
					(PondA.Size() - 1.0f) * 3200.0f,
					FMath::Min((PondB.Size() - 1.0f) * 2800.0f, (PondC.Size() - 1.0f) * 2400.0f))));
	}

	bool IsNearPath(const FVector2D& Position)
	{
		constexpr float PathPadding = 280.0f;
		if (DistanceToSegment(Position, FVector2D(18000.0f, 151500.0f), FVector2D(82000.0f, 151500.0f)) < PathPadding)
		{
			return true;
		}
		if (DistanceToSegment(Position, FVector2D(-10000.0f, 6500.0f), FVector2D(9000.0f, 9000.0f)) < PathPadding)
		{
			return true;
		}
		if (DistanceToSegment(Position, FVector2D(9000.0f, 9000.0f), FVector2D(36000.0f, 21000.0f)) < PathPadding)
		{
			return true;
		}
		if (DistanceToSegment(Position, FVector2D(36000.0f, 21000.0f), FVector2D(76000.0f, 32000.0f)) < PathPadding)
		{
			return true;
		}
		if (DistanceToSegment(Position, FVector2D(10000.0f, 36000.0f), FVector2D(12000.0f, 60000.0f)) < PathPadding)
		{
			return true;
		}
		if (DistanceToSegment(Position, FVector2D(12000.0f, 60000.0f), FVector2D(16000.0f, 87000.0f)) < PathPadding)
		{
			return true;
		}
		const float SpawnLoopDistance = FMath::Abs(FVector2D::Distance(Position, FVector2D(-18000.0f, 4000.0f)) - 10500.0f);
		return SpawnLoopDistance < 950.0f;
	}

	bool IsInsideIslandInterior(const FVector2D& Position)
	{
		const FVector2D Centered = (Position - FVector2D(71396.0f, 79714.0f)) / FVector2D(112000.0f, 124000.0f);
		const float Organic = Centered.Size()
			+ 0.055f * FMath::Sin(Position.X / 11500.0f)
			- 0.045f * FMath::Cos(Position.Y / 9000.0f)
			+ 0.035f * FMath::Sin((Position.X + Position.Y) / 17000.0f);
		return Organic < 0.765f;
	}

	bool IsValidGrassPosition(const FGrassPlacementZone& Zone, const FVector2D& Position)
	{
		if (!IsInsideIslandInterior(Position) || IsNearPath(Position))
		{
			return false;
		}

		const float WaterDistance = GetNearestWaterDistance(Position);
		if (Zone.bAllowSpawnDryOverride)
		{
			return true;
		}

		if (Zone.bRequireShoreBand)
		{
			return WaterDistance >= 3000.0f && WaterDistance <= 8500.0f;
		}

		if (WaterDistance < 1350.0f)
		{
			return false;
		}

		return true;
	}

	FVector2D GetRandomPointInZone(const FGrassPlacementZone& Zone, int32 Seed)
	{
		const float Angle = PseudoRandom01(Seed) * 2.0f * PI;
		const float RadiusAlpha = FMath::Sqrt(PseudoRandom01(Seed + 11));
		const float LocalJitter = FMath::Lerp(-420.0f, 420.0f, PseudoRandom01(Seed + 19));
		const FVector2D Local(
			FMath::Cos(Angle) * Zone.Extents.X * RadiusAlpha + FMath::Sin(Angle * 1.7f) * LocalJitter,
			FMath::Sin(Angle) * Zone.Extents.Y * RadiusAlpha + FMath::Cos(Angle * 1.3f) * LocalJitter);
		const float RotationRadians = FMath::DegreesToRadians(Zone.RotationDegrees);
		const float CosAngle = FMath::Cos(RotationRadians);
		const float SinAngle = FMath::Sin(RotationRadians);
		return Zone.Center + FVector2D(Local.X * CosAngle - Local.Y * SinAngle, Local.X * SinAngle + Local.Y * CosAngle);
	}

	float GetDensityKeepChance(const FVector2D& Position, int32 MeshIndex, int32 Seed)
	{
		const float Broad = 0.5f + 0.5f * FMath::Sin(Position.X / 18500.0f + Position.Y / 31000.0f + PseudoRandom01(Seed + 7) * 2.0f);
		const float Patch = 0.5f + 0.5f * FMath::Sin(Position.X / 7200.0f - Position.Y / 9400.0f + PseudoRandom01(Seed + 17) * 6.28318f);
		const float MeshBaseChance = MeshIndex == 0 ? 0.98f : (MeshIndex == 1 ? 0.90f : 0.86f);
		return FMath::Clamp(MeshBaseChance * FMath::Lerp(0.82f, 1.04f, Broad * 0.72f + Patch * 0.28f), 0.58f, 0.992f);
	}

	float GetCarpetKeepChance(const FVector2D& Position, const FGrassCarpetLayer& Layer, int32 Seed)
	{
		if (FCString::Strcmp(Layer.Name, TEXT("ShortBaseFill")) == 0)
		{
			// Base fill should read as a continuous carpet; keep breakup subtle so it does not create crop-circle gaps.
			const float FineNoise = 0.5f + 0.5f * FMath::Sin(Position.X / 6200.0f + Position.Y / 7900.0f + PseudoRandom01(Seed + 79) * 6.28318f);
			return FMath::Clamp(0.992f + FineNoise * 0.006f, 0.992f, 0.998f);
		}

		const float LargeNoise = 0.5f + 0.5f * FMath::Sin(Position.X / 29500.0f + Position.Y / 41000.0f + PseudoRandom01(Seed + 31) * 1.4f);
		const float MidNoise = 0.5f + 0.5f * FMath::Sin(Position.X / 12600.0f - Position.Y / 17300.0f + PseudoRandom01(Seed + 53) * 4.2f);
		const float FineNoise = 0.5f + 0.5f * FMath::Sin(Position.X / 3700.0f + Position.Y / 5200.0f + PseudoRandom01(Seed + 79) * 6.28318f);
		const float NaturalBreakup = LargeNoise * 0.55f + MidNoise * 0.32f + FineNoise * 0.13f;
		return FMath::Clamp(Layer.KeepChance * FMath::Lerp(0.72f, 1.08f, NaturalBreakup), 0.0f, 0.985f);
	}

	FVector2D GetCarpetPoint(const FGrassCarpetLayer& Layer, int32 GridX, int32 GridY, int32 Seed)
	{
		const FVector2D MinCorner = CarpetCenter - CarpetExtents;
		const float Jitter = Layer.Spacing * Layer.JitterRatio;
		const float RowWarp = FMath::Sin(static_cast<float>(GridY) * 0.731f + PseudoRandom01(Seed + 191) * 2.0f) * Layer.Spacing * 0.34f;
		const float ColumnWarp = FMath::Cos(static_cast<float>(GridX) * 0.619f + PseudoRandom01(Seed + 223) * 2.0f) * Layer.Spacing * 0.26f;
		const FVector2D Base(
			MinCorner.X + (static_cast<float>(GridX) + 0.5f) * Layer.Spacing + RowWarp,
			MinCorner.Y + (static_cast<float>(GridY) + 0.5f) * Layer.Spacing + ColumnWarp);
		const FVector2D JitterOffset(
			FMath::Lerp(-Jitter, Jitter, PseudoRandom01(Seed + 101)),
			FMath::Lerp(-Jitter, Jitter, PseudoRandom01(Seed + 137)));
		return Base + JitterOffset;
	}

	float EstimateZoneGroundZ(const FGrassPlacementZone& Zone, const FVector2D& Position)
	{
		// Editor commandlet landscape traces are not reliable for this hand-built map,
		// so zones use conservative manually bounded flat-area height estimates.
		const FVector2D Offset = Position - Zone.Center;
		const float SoftRoll = FMath::Sin(Offset.X * 0.0013f) * Zone.ZVariation + FMath::Cos(Offset.Y * 0.0011f) * Zone.ZVariation * 0.65f;
		return Zone.GroundZ + SoftRoll;
	}

	bool GetPlacementGround(UWorld* World, const FVector2D& Position, FHitResult& OutHit)
	{
		if (!World)
		{
			return false;
		}

		const FVector Start(Position.X, Position.Y, 36000.0f);
		const FVector End(Position.X, Position.Y, -36000.0f);
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(FFStarterGrassPlacement), true);
		QueryParams.bReturnPhysicalMaterial = false;
		if (!World->LineTraceSingleByChannel(OutHit, Start, End, ECC_WorldStatic, QueryParams))
		{
			return false;
		}

		if (!OutHit.GetActor() || !OutHit.GetActor()->IsA<ALandscapeProxy>())
		{
			return false;
		}

		return OutHit.ImpactNormal.Z >= 0.875f && OutHit.ImpactPoint.Z <= MaxGrassHeightZ;
	}

	UHierarchicalInstancedStaticMeshComponent* CreateGrassComponent(AActor* Owner, UStaticMesh* Mesh, UMaterialInterface* GrassMaterial, int32 MeshIndex)
	{
		if (!Owner || !Mesh)
		{
			return nullptr;
		}

		UHierarchicalInstancedStaticMeshComponent* Component = NewObject<UHierarchicalInstancedStaticMeshComponent>(
			Owner,
			*FString::Printf(TEXT("FFSpawnGrassProof_%d"), MeshIndex),
			RF_Transactional);
		if (!Component)
		{
			return nullptr;
		}

		Component->SetStaticMesh(Mesh);
		if (GrassMaterial)
		{
			const int32 MaterialSlots = FMath::Max(1, Mesh->GetStaticMaterials().Num());
			for (int32 SlotIndex = 0; SlotIndex < MaterialSlots; ++SlotIndex)
			{
				Component->SetMaterial(SlotIndex, GrassMaterial);
			}
		}
		Component->SetMobility(EComponentMobility::Static);
		Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Component->SetGenerateOverlapEvents(false);
		Component->bSelectable = true;
		Component->SetCastShadow(false);
		const int32 EndCullDistance = MeshIndex >= 0 && MeshIndex < UE_ARRAY_COUNT(GrassCarpetLayers)
			? GrassCarpetLayers[MeshIndex].EndCullDistance
			: 52000;
		Component->SetCullDistances(0, EndCullDistance);
		Component->SetupAttachment(Owner->GetRootComponent());
		Component->RegisterComponent();
		Owner->AddInstanceComponent(Component);
		return Component;
	}

	struct FGrassABPatchSpec
	{
		const TCHAR* Label;
		const TCHAR* CameraTag;
		FVector2D Center;
		bool bNative;
	};

	const FGrassABPatchSpec ABPatchSpecs[] = {
		{ TEXT("FF_GrassAB_A_Proxy_Sun_HISM"), TEXT("FFSmokeGrassABProxySunCamera"), FVector2D(35450.0f, 145350.0f), false },
		{ TEXT("FF_GrassAB_B_FullTitanStack_Sun_HISM"), TEXT("FFSmokeGrassABNativeSunCamera"), FVector2D(42450.0f, 145350.0f), true },
		{ TEXT("FF_GrassAB_A_Proxy_Shadow_HISM"), TEXT("FFSmokeGrassABProxyShadowCamera"), FVector2D(81200.0f, 111500.0f), false },
		{ TEXT("FF_GrassAB_B_FullTitanStack_Shadow_HISM"), TEXT("FFSmokeGrassABNativeShadowCamera"), FVector2D(87200.0f, 111500.0f), true }
	};

	ALandscapeProxy* FindPrimaryLandscape(UWorld* World)
	{
		if (!World)
		{
			return nullptr;
		}

		for (TActorIterator<ALandscapeProxy> It(World); It; ++It)
		{
			return *It;
		}
		return nullptr;
	}

	void AddRVTToLandscapeForNativeGrass(ALandscapeProxy* Landscape, URuntimeVirtualTexture* RVTD, URuntimeVirtualTexture* RVTH, URuntimeVirtualTexture* RVTDH)
	{
		if (!Landscape || !RVTD || !RVTH)
		{
			return;
		}

		Landscape->Modify();
		Landscape->RuntimeVirtualTextures.AddUnique(RVTD);
		Landscape->RuntimeVirtualTextures.AddUnique(RVTH);
		if (RVTDH)
		{
			Landscape->RuntimeVirtualTextures.AddUnique(RVTDH);
		}
		Landscape->VirtualTextureRenderPassType = ERuntimeVirtualTextureMainPassType::Always;
		Landscape->MarkPackageDirty();
	}

	void SpawnRVTVolumesForBounds(UWorld* World, ALandscapeProxy* Landscape, const FString& LabelPrefix, const FVector& BoundsMin, const FVector& BoundsSize, URuntimeVirtualTexture* RVTD, URuntimeVirtualTexture* RVTH, URuntimeVirtualTexture* RVTDH)
	{
		if (!World || !RVTD || !RVTH)
		{
			return;
		}

		const FVector VolumeLocation = BoundsMin;
		const auto SpawnVolume = [&](URuntimeVirtualTexture* RVT, const FString& Label)
		{
			if (!RVT)
			{
				return;
			}

			FActorSpawnParameters SpawnParameters;
			SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			ARuntimeVirtualTextureVolume* Volume = World->SpawnActor<ARuntimeVirtualTextureVolume>(
				ARuntimeVirtualTextureVolume::StaticClass(),
				VolumeLocation,
				FRotator::ZeroRotator,
				SpawnParameters);
			if (!Volume)
			{
				return;
			}

			Volume->Modify();
			Volume->Tags.AddUnique(NativeTestPatchTag);
			Volume->Tags.AddUnique(ABTestPatchTag);
			Volume->SetActorLabel(Label);
			Volume->SetActorScale3D(BoundsSize);
			if (Volume->VirtualTextureComponent)
			{
				Volume->VirtualTextureComponent->Modify();
				Volume->VirtualTextureComponent->SetVirtualTexture(RVT);
				if (Landscape && !LabelPrefix.Contains(TEXT("TitanLandscapeGroundContext")))
				{
					Volume->VirtualTextureComponent->SetBoundsAlignActor(Landscape);
				}
				Volume->VirtualTextureComponent->MarkPackageDirty();
			}
			Volume->MarkPackageDirty();
		};

		SpawnVolume(RVTD, FString::Printf(TEXT("%s_RVT_Titan_D"), *LabelPrefix));
		SpawnVolume(RVTH, FString::Printf(TEXT("%s_RVT_Titan_H"), *LabelPrefix));
		SpawnVolume(RVTDH, FString::Printf(TEXT("%s_RVT_Titan_DH"), *LabelPrefix));
	}

	void SpawnPatchRVTVolumes(UWorld* World, ALandscapeProxy* Landscape, const FGrassABPatchSpec& Patch, URuntimeVirtualTexture* RVTD, URuntimeVirtualTexture* RVTH, URuntimeVirtualTexture* RVTDH)
	{
		FHitResult GroundHit;
		FVector Center(Patch.Center.X, Patch.Center.Y, -3200.0f);
		if (GetPlacementGround(World, Patch.Center, GroundHit))
		{
			Center = GroundHit.ImpactPoint;
		}

		const FVector BoundsSize(4200.0f, 4200.0f, 5200.0f);
		const FVector BoundsMin = Center - FVector(BoundsSize.X * 0.5f, BoundsSize.Y * 0.5f, BoundsSize.Z * 0.5f);
		SpawnRVTVolumesForBounds(World, Landscape, Patch.Label, BoundsMin, BoundsSize, RVTD, RVTH, RVTDH);
	}

	uint16 EncodeFlatLandscapeHeight()
	{
		return 32768;
	}

	ALandscape* CreateTitanLandscapeGroundContextPatch(
		UWorld* World,
		const FGrassABPatchSpec& Patch,
		UMaterialInterface* TitanGroundMaterial,
		URuntimeVirtualTexture* RVTD,
		URuntimeVirtualTexture* RVTH,
		URuntimeVirtualTexture* RVTDH)
	{
		if (!World || !TitanGroundMaterial || !RVTD || !RVTH)
		{
			return nullptr;
		}

		FHitResult GroundHit;
		FVector Center(Patch.Center.X, Patch.Center.Y, -3200.0f);
		if (GetPlacementGround(World, Patch.Center, GroundHit))
		{
			Center = GroundHit.ImpactPoint + FVector(0.0f, 0.0f, 14.0f);
		}

		constexpr int32 SectionsPerComponent = 1;
		constexpr int32 QuadsPerSection = 31;
		constexpr int32 ComponentCountX = 1;
		constexpr int32 ComponentCountY = 1;
		constexpr int32 QuadsPerComponent = SectionsPerComponent * QuadsPerSection;
		constexpr int32 SizeX = ComponentCountX * QuadsPerComponent + 1;
		constexpr int32 SizeY = ComponentCountY * QuadsPerComponent + 1;
		const FVector LandscapeScale(120.0f, 120.0f, 100.0f);
		const FVector LandscapeOffset = Center + FTransform(FRotator::ZeroRotator, FVector::ZeroVector, LandscapeScale)
			.TransformVector(FVector(-ComponentCountX * QuadsPerComponent / 2.0f, -ComponentCountY * QuadsPerComponent / 2.0f, 0.0f));

		TArray<uint16> HeightData;
		HeightData.Init(EncodeFlatLandscapeHeight(), SizeX * SizeY);

		TMap<FGuid, TArray<uint16>> HeightDataPerLayers;
		TMap<FGuid, TArray<FLandscapeImportLayerInfo>> MaterialLayerDataPerLayers;
		HeightDataPerLayers.Add(FGuid(), MoveTemp(HeightData));
		MaterialLayerDataPerLayers.Add(FGuid(), TArray<FLandscapeImportLayerInfo>());

		ALandscape* TitanLandscapePatch = World->SpawnActor<ALandscape>(LandscapeOffset, FRotator::ZeroRotator);
		if (!TitanLandscapePatch)
		{
			return nullptr;
		}

		TitanLandscapePatch->Modify();
		TitanLandscapePatch->Tags.AddUnique(NativeTestPatchTag);
		TitanLandscapePatch->Tags.AddUnique(ABTestPatchTag);
		TitanLandscapePatch->SetActorLabel(FString::Printf(TEXT("%s_TitanLandscapeGroundContext"), Patch.Label));
		TitanLandscapePatch->LandscapeMaterial = TitanGroundMaterial;
		TitanLandscapePatch->SetActorScale3D(LandscapeScale);
		TitanLandscapePatch->StaticLightingLOD = 0;
		TitanLandscapePatch->RuntimeVirtualTextures.Add(RVTD);
		TitanLandscapePatch->RuntimeVirtualTextures.Add(RVTH);
		if (RVTDH)
		{
			TitanLandscapePatch->RuntimeVirtualTextures.Add(RVTDH);
		}
		TitanLandscapePatch->VirtualTextureRenderPassType = ERuntimeVirtualTextureMainPassType::Always;
		TitanLandscapePatch->Import(FGuid::NewGuid(), 0, 0, SizeX - 1, SizeY - 1, SectionsPerComponent, QuadsPerSection, HeightDataPerLayers, TEXT(""), MaterialLayerDataPerLayers, ELandscapeImportAlphamapType::Additive, TArrayView<const FLandscapeLayer>());

		// Do not force UpdateLayerInfoMap here. In the existing full-map
		// creation commandlet it is safe after a fresh blank map, but creating a
		// second tiny Landscape inside an already loaded world can trip a
		// LandscapeInfo refresh crash in commandlet mode. The imported actor and
		// its components are sufficient for the isolated render/RVT test.

		// Keep this actor self-contained for the first ground-context test. Extra
		// RVT volume alignment against a second landscape inside the loaded
		// Highland world has proven crash-prone in commandlet mode; the landscape
		// still advertises the native RVTs and the existing Highland RVT support
		// remains in the map.

		return TitanLandscapePatch;
	}

	UHierarchicalInstancedStaticMeshComponent* CreateABGrassComponent(
		AActor* Owner,
		UStaticMesh* Mesh,
		UMaterialInterface* Material,
		const FString& ComponentName,
		int32 EndCullDistance)
	{
		if (!Owner || !Mesh || !Material)
		{
			return nullptr;
		}

		UHierarchicalInstancedStaticMeshComponent* Component = NewObject<UHierarchicalInstancedStaticMeshComponent>(
			Owner,
			*ComponentName,
			RF_Transactional);
		if (!Component)
		{
			return nullptr;
		}

		Component->SetStaticMesh(Mesh);
		const int32 MaterialSlots = FMath::Max(1, Mesh->GetStaticMaterials().Num());
		for (int32 SlotIndex = 0; SlotIndex < MaterialSlots; ++SlotIndex)
		{
			Component->SetMaterial(SlotIndex, Material);
		}
		Component->SetMobility(EComponentMobility::Static);
		Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Component->SetGenerateOverlapEvents(false);
		Component->SetCastShadow(false);
		Component->SetCullDistances(0, EndCullDistance);
		Component->SetupAttachment(Owner->GetRootComponent());
		Component->RegisterComponent();
		Owner->AddInstanceComponent(Component);
		return Component;
	}

	int32 AddABPatchInstances(
		UWorld* World,
		UHierarchicalInstancedStaticMeshComponent* Component,
		const FVector2D& Center,
		int32 SeedOffset,
		float Radius,
		float Spacing,
		float MinZScale,
		float MaxZScale)
	{
		if (!World || !Component)
		{
			return 0;
		}

		int32 Added = 0;
		const int32 GridRadius = FMath::CeilToInt(Radius / Spacing);
		for (int32 GridY = -GridRadius; GridY <= GridRadius; ++GridY)
		{
			for (int32 GridX = -GridRadius; GridX <= GridRadius; ++GridX)
			{
				const int32 Seed = SeedOffset + GridX * 92837111 + GridY * 689287499;
				const FVector2D Position = Center + FVector2D(
					static_cast<float>(GridX) * Spacing + FMath::Lerp(-Spacing * 0.44f, Spacing * 0.44f, PseudoRandom01(Seed + 17)),
					static_cast<float>(GridY) * Spacing + FMath::Lerp(-Spacing * 0.44f, Spacing * 0.44f, PseudoRandom01(Seed + 29)));
				if (FVector2D::Distance(Position, Center) > Radius)
				{
					continue;
				}
				if (GetNearestWaterDistance(Position) < 1500.0f || IsNearPath(Position))
				{
					continue;
				}

				FHitResult GroundHit;
				if (!GetPlacementGround(World, Position, GroundHit) || GroundHit.ImpactNormal.Z < 0.88f)
				{
					continue;
				}

				FTransform InstanceTransform;
				InstanceTransform.SetLocation(GroundHit.ImpactPoint + FVector(0.0f, 0.0f, 0.8f));
				const float Yaw = PseudoRandom01(Seed + 43) * 360.0f;
				const float XYScale = FMath::Lerp(1.05f, 1.48f, PseudoRandom01(Seed + 59));
				const float ZScale = FMath::Lerp(MinZScale, MaxZScale, PseudoRandom01(Seed + 71));
				const FQuat SurfaceRotation = FRotationMatrix::MakeFromZ(GroundHit.ImpactNormal).ToQuat();
				const FQuat YawAroundSurface(GroundHit.ImpactNormal, FMath::DegreesToRadians(Yaw));
				InstanceTransform.SetRotation(YawAroundSurface * SurfaceRotation);
				InstanceTransform.SetScale3D(FVector(XYScale, XYScale, ZScale));
				Component->AddInstance(InstanceTransform, true);
				++Added;
			}
		}
		return Added;
	}

	int32 AddTitanGrassTypeVarietyInstances(
		UWorld* World,
		UHierarchicalInstancedStaticMeshComponent* Component,
		const FVector2D& Center,
		int32 SeedOffset,
		float Radius,
		float Spacing,
		float MinXScale,
		float MaxXScale,
		float MinYScale,
		float MaxYScale,
		float MinZScale,
		float MaxZScale)
	{
		if (!World || !Component)
		{
			return 0;
		}

		int32 Added = 0;
		const int32 GridRadius = FMath::CeilToInt(Radius / Spacing);
		for (int32 GridY = -GridRadius; GridY <= GridRadius; ++GridY)
		{
			for (int32 GridX = -GridRadius; GridX <= GridRadius; ++GridX)
			{
				const int32 Seed = SeedOffset + GridX * 73471 + GridY * 912367;
				const FVector2D Position = Center + FVector2D(
					static_cast<float>(GridX) * Spacing + FMath::Lerp(-Spacing * 0.50f, Spacing * 0.50f, PseudoRandom01(Seed + 101)),
					static_cast<float>(GridY) * Spacing + FMath::Lerp(-Spacing * 0.50f, Spacing * 0.50f, PseudoRandom01(Seed + 203)));
				if (FVector2D::Distance(Position, Center) > Radius)
				{
					continue;
				}
				if (GetNearestWaterDistance(Position) < 1500.0f || IsNearPath(Position))
				{
					continue;
				}

				FHitResult GroundHit;
				if (!GetPlacementGround(World, Position, GroundHit) || GroundHit.ImpactNormal.Z < 0.88f)
				{
					continue;
				}

				const float Yaw = PseudoRandom01(Seed + 307) * 360.0f;
				const float XScale = FMath::Lerp(MinXScale, MaxXScale, PseudoRandom01(Seed + 409));
				const float YScale = FMath::Lerp(MinYScale, MaxYScale, PseudoRandom01(Seed + 503));
				const float ZScale = FMath::Lerp(MinZScale, MaxZScale, PseudoRandom01(Seed + 601));

				FTransform InstanceTransform;
				InstanceTransform.SetLocation(GroundHit.ImpactPoint + FVector(0.0f, 0.0f, 0.8f));
				const FQuat SurfaceRotation = FRotationMatrix::MakeFromZ(GroundHit.ImpactNormal).ToQuat();
				const FQuat YawAroundSurface(GroundHit.ImpactNormal, FMath::DegreesToRadians(Yaw));
				InstanceTransform.SetRotation(YawAroundSurface * SurfaceRotation);
				InstanceTransform.SetScale3D(FVector(XScale, YScale, ZScale));
				Component->AddInstance(InstanceTransform, true);
				++Added;
			}
		}
		return Added;
	}

	AActor* CreateTitanGroundSampleActor(
		UWorld* World,
		const FGrassABPatchSpec& Patch,
		UStaticMesh* PlaneMesh,
		UMaterialInterface* GroundMaterial)
	{
		if (!World || !PlaneMesh || !GroundMaterial)
		{
			return nullptr;
		}

		FHitResult GroundHit;
		FVector Location(Patch.Center.X, Patch.Center.Y, -3200.0f);
		if (GetPlacementGround(World, Patch.Center, GroundHit))
		{
			Location = GroundHit.ImpactPoint + FVector(0.0f, 0.0f, 0.35f);
		}

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AActor* GroundActor = World->SpawnActor<AActor>(AActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParameters);
		if (!GroundActor)
		{
			return nullptr;
		}

		GroundActor->Modify();
		GroundActor->Tags.AddUnique(NativeTestPatchTag);
		GroundActor->Tags.AddUnique(ABTestPatchTag);
		GroundActor->SetActorLabel(FString::Printf(TEXT("%s_TitanGrasslandGroundSample"), Patch.Label));

		USceneComponent* RootComponent = NewObject<USceneComponent>(GroundActor, TEXT("Root"), RF_Transactional);
		RootComponent->SetMobility(EComponentMobility::Static);
		GroundActor->SetRootComponent(RootComponent);
		RootComponent->RegisterComponent();
		GroundActor->AddInstanceComponent(RootComponent);

		UStaticMeshComponent* PlaneComponent = NewObject<UStaticMeshComponent>(GroundActor, TEXT("TitanGrasslandGroundPlane"), RF_Transactional);
		PlaneComponent->SetStaticMesh(PlaneMesh);
		PlaneComponent->SetMaterial(0, GroundMaterial);
		PlaneComponent->SetMobility(EComponentMobility::Static);
		PlaneComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		PlaneComponent->SetGenerateOverlapEvents(false);
		PlaneComponent->SetCastShadow(false);
		PlaneComponent->SetWorldScale3D(FVector(31.0f, 31.0f, 1.0f));
		PlaneComponent->SetupAttachment(RootComponent);
		PlaneComponent->RegisterComponent();
		GroundActor->AddInstanceComponent(PlaneComponent);
		GroundActor->MarkPackageDirty();
		return GroundActor;
	}

	AActor* CreateABPatchActor(
		UWorld* World,
		const FGrassABPatchSpec& Patch,
		UStaticMesh* Mesh,
		UMaterialInterface* ProxyMaterial,
		UMaterialInterface* NativeMaterial,
		UMaterialInterface* NativeDarkerMaterial,
		UMaterialInterface* TitanGroundMaterial,
		UStaticMesh* GroundPlaneMesh,
		int32& OutInstances)
	{
		OutInstances = 0;
		if (!World || !Mesh || !ProxyMaterial || !NativeMaterial || !NativeDarkerMaterial)
		{
			return nullptr;
		}

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AActor* PatchActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector(Patch.Center.X, Patch.Center.Y, 0.0f), FRotator::ZeroRotator, SpawnParameters);
		if (!PatchActor)
		{
			return nullptr;
		}

		PatchActor->Tags.AddUnique(NativeTestPatchTag);
		PatchActor->Tags.AddUnique(ABTestPatchTag);
		PatchActor->SetActorLabel(Patch.Label);
		USceneComponent* RootComponent = NewObject<USceneComponent>(PatchActor, TEXT("Root"), RF_Transactional);
		RootComponent->SetMobility(EComponentMobility::Static);
		PatchActor->SetRootComponent(RootComponent);
		RootComponent->RegisterComponent();
		PatchActor->AddInstanceComponent(RootComponent);

		if (Patch.bNative)
		{
			// MI_LandscapeMain needs real landscape layer context; using it on an
			// isolated static mesh sample renders black in packaged top-down views.
			// Keep the full native grass/RVT stack isolated and report the ground
			// material context as the remaining blocker instead of baking a bad
			// visual artifact into the test map.
			UHierarchicalInstancedStaticMeshComponent* TitanVarietyHighDensity = CreateABGrassComponent(PatchActor, Mesh, NativeDarkerMaterial, TEXT("TitanLGT_Grass_Variety1_Density200_Darker"), 10000);
			UHierarchicalInstancedStaticMeshComponent* TitanVarietyLowDensity = CreateABGrassComponent(PatchActor, Mesh, NativeMaterial, TEXT("TitanLGT_Grass_Variety0_Density25"), 10000);
			const int32 HighDensityCount = AddTitanGrassTypeVarietyInstances(
				World,
				TitanVarietyHighDensity,
				Patch.Center,
				5100000,
				1380.0f,
				105.0f,
				1.0f,
				1.5f,
				1.0f,
				1.0f,
				0.3f,
				0.7f);
			const int32 LowDensityCount = AddTitanGrassTypeVarietyInstances(
				World,
				TitanVarietyLowDensity,
				Patch.Center + FVector2D(23.0f, -19.0f),
				6100000,
				1380.0f,
				295.0f,
				1.0f,
				2.0f,
				1.0f,
				1.0f,
				0.5f,
				1.2f);
			if (TitanVarietyHighDensity)
			{
				TitanVarietyHighDensity->BuildTreeIfOutdated(true, true);
				TitanVarietyHighDensity->MarkPackageDirty();
			}
			if (TitanVarietyLowDensity)
			{
				TitanVarietyLowDensity->BuildTreeIfOutdated(true, true);
				TitanVarietyLowDensity->MarkPackageDirty();
			}
			OutInstances = HighDensityCount + LowDensityCount;
		}
		else
		{
			UHierarchicalInstancedStaticMeshComponent* ProxyBase = CreateABGrassComponent(PatchActor, Mesh, ProxyMaterial, TEXT("Proxy_GrassBlade_Base"), 12000);
			UHierarchicalInstancedStaticMeshComponent* ProxyHighlight = CreateABGrassComponent(PatchActor, Mesh, ProxyMaterial, TEXT("Proxy_GrassBlade_Highlight"), 12000);
			const int32 BaseCount = AddABPatchInstances(World, ProxyBase, Patch.Center, 7000000, 1180.0f, 118.0f, 0.34f, 0.70f);
			const int32 HighlightCount = AddABPatchInstances(World, ProxyHighlight, Patch.Center + FVector2D(18.0f, -22.0f), 8000000, 1180.0f, 188.0f, 0.46f, 1.02f);
			if (ProxyBase)
			{
				ProxyBase->BuildTreeIfOutdated(true, true);
				ProxyBase->MarkPackageDirty();
			}
			if (ProxyHighlight)
			{
				ProxyHighlight->BuildTreeIfOutdated(true, true);
				ProxyHighlight->MarkPackageDirty();
			}
			OutInstances = BaseCount + HighlightCount;
		}

		PatchActor->MarkPackageDirty();
		return PatchActor;
	}

	void CreateABComparisonCamera(UWorld* World, const FGrassABPatchSpec& Patch)
	{
		if (!World)
		{
			return;
		}

		FHitResult GroundHit;
		const FVector2D LookAt2D = Patch.Center;
		FVector LookAt(LookAt2D.X, LookAt2D.Y, -3200.0f);
		if (GetPlacementGround(World, LookAt2D, GroundHit))
		{
			LookAt = GroundHit.ImpactPoint;
		}

		const FVector CameraLocation = LookAt + FVector(-1500.0f, -980.0f, 640.0f);
		const FRotator CameraRotation = (LookAt + FVector(0.0f, 0.0f, 115.0f) - CameraLocation).Rotation();
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ACameraActor* CameraActor = World->SpawnActor<ACameraActor>(CameraLocation, CameraRotation, SpawnParameters);
		if (!CameraActor)
		{
			return;
		}

		CameraActor->Tags.AddUnique(NativeTestPatchTag);
		CameraActor->Tags.AddUnique(ABTestPatchTag);
		CameraActor->Tags.AddUnique(FName(Patch.CameraTag));
		CameraActor->SetActorLabel(FString::Printf(TEXT("%s_%s"), Patch.Label, TEXT("Camera")));
		if (UCameraComponent* CameraComponent = CameraActor->GetCameraComponent())
		{
			CameraComponent->ProjectionMode = ECameraProjectionMode::Perspective;
			CameraComponent->FieldOfView = 38.0f;
		}
		CameraActor->MarkPackageDirty();
	}
}
#endif

UFFStarterHighlandSpawnGrassProofCommandlet::UFFStarterHighlandSpawnGrassProofCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

int32 UFFStarterHighlandSpawnGrassProofCommandlet::Main(const FString& Params)
{
#if WITH_EDITOR
	const bool bFullTitanStackABOnly = Params.Contains(TEXT("FullTitanStackABOnly"), ESearchCase::IgnoreCase);
	const bool bTitanGroundContextABOnly = Params.Contains(TEXT("TitanGroundContextABOnly"), ESearchCase::IgnoreCase);
	UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandSpawnGrassProof: loading %s fullTitanStackABOnly=%s titanGroundContextABOnly=%s"),
		HighlandMapPath,
		bFullTitanStackABOnly ? TEXT("true") : TEXT("false"),
		bTitanGroundContextABOnly ? TEXT("true") : TEXT("false"));
	UWorld* World = UEditorLoadingAndSavingUtils::LoadMap(HighlandMapPath);
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandSpawnGrassProof: failed to load map."));
		return 1;
	}

	int32 RemovedActors = 0;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		const bool bRemovePhase1Grass = !bFullTitanStackABOnly && !bTitanGroundContextABOnly && Actor && (Actor->ActorHasTag(SpawnGrassProofTag) || Actor->ActorHasTag(Phase1GrassTag));
		const bool bRemoveABGrass = Actor && (Actor->ActorHasTag(NativeTestPatchTag) || Actor->ActorHasTag(ABTestPatchTag));
		if (Actor && (bRemovePhase1Grass || bRemoveABGrass))
		{
			World->DestroyActor(Actor);
			++RemovedActors;
		}
	}

	TArray<UStaticMesh*> GrassMeshes;
	for (const TCHAR* MeshPath : GrassMeshPaths)
	{
		UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, MeshPath);
		if (!Mesh)
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandSpawnGrassProof: missing grass mesh %s"), MeshPath);
			return 1;
		}
		GrassMeshes.Add(Mesh);
	}
	TArray<UMaterialInterface*> GrassMaterials;
	for (const TCHAR* MaterialPath : GrassMaterialPaths)
	{
		if (FCString::Strlen(MaterialPath) == 0)
		{
			GrassMaterials.Add(nullptr);
			UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandSpawnGrassProof: grassMaterial=mesh default"));
			continue;
		}

		UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, MaterialPath);
		if (!Material)
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandSpawnGrassProof: missing Titan grass material %s"), MaterialPath);
			return 1;
		}
		GrassMaterials.Add(Material);
		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandSpawnGrassProof: grassMaterial=%s"), *Material->GetPathName());
	}
	UMaterialInterface* ProxyMaterial = LoadObject<UMaterialInterface>(nullptr, ProxyGrassMaterialPath);
	UMaterialInterface* NativeMaterial = LoadObject<UMaterialInterface>(nullptr, NativeGrassMaterialPath);
	UMaterialInterface* NativeDarkerMaterial = LoadObject<UMaterialInterface>(nullptr, NativeGrassDarkerMaterialPath);
	UMaterialInterface* TitanGroundMaterial = LoadObject<UMaterialInterface>(nullptr, TitanGroundMaterialPath);
	UStaticMesh* GroundPlaneMesh = LoadObject<UStaticMesh>(nullptr, EnginePlaneMeshPath);
	URuntimeVirtualTexture* TitanRVTD = LoadObject<URuntimeVirtualTexture>(nullptr, TitanRVTDPath);
	URuntimeVirtualTexture* TitanRVTH = LoadObject<URuntimeVirtualTexture>(nullptr, TitanRVTHPath);
	URuntimeVirtualTexture* TitanRVTDH = LoadObject<URuntimeVirtualTexture>(nullptr, TitanRVTDHPath);
	ULandscapeGrassType* TitanGrassType = LoadObject<ULandscapeGrassType>(nullptr, TitanGrassTypePath);
	if (!ProxyMaterial || !NativeMaterial || !NativeDarkerMaterial || !TitanGroundMaterial || !GroundPlaneMesh || !TitanRVTD || !TitanRVTH || !TitanGrassType)
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandSpawnGrassProof: missing A/B full-stack deps proxy=%s native=%s nativeDarker=%s ground=%s plane=%s rvtD=%s rvtH=%s grassType=%s"),
			ProxyMaterial ? TEXT("ok") : ProxyGrassMaterialPath,
			NativeMaterial ? TEXT("ok") : NativeGrassMaterialPath,
			NativeDarkerMaterial ? TEXT("ok") : NativeGrassDarkerMaterialPath,
			TitanGroundMaterial ? TEXT("ok") : TitanGroundMaterialPath,
			GroundPlaneMesh ? TEXT("ok") : EnginePlaneMeshPath,
			TitanRVTD ? TEXT("ok") : TitanRVTDPath,
			TitanRVTH ? TEXT("ok") : TitanRVTHPath,
			TitanGrassType ? TEXT("ok") : TitanGrassTypePath);
		return 1;
	}
	UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandSpawnGrassProof: fullStackAssets grassType=%s varieties=%d ground=%s rvtD=%s rvtH=%s rvtDH=%s"),
		*TitanGrassType->GetPathName(),
		TitanGrassType->GrassVarieties.Num(),
		*TitanGroundMaterial->GetPathName(),
		*TitanRVTD->GetPathName(),
		*TitanRVTH->GetPathName(),
		TitanRVTDH ? *TitanRVTDH->GetPathName() : TEXT("missing"));
	UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandSpawnGrassProof: reproduced Titan LGT_Grass settings v0 density=25 grid=true jitter=1 cull=2000-10000 scaleX=1..2 scaleY=1 scaleZ=0.5..1.2; v1 density=200 grid=true jitter=1 cull=2000-10000 scaleX=1..1.5 scaleY=1 scaleZ=0.3..0.7"));

	ALandscapeProxy* Landscape = FindPrimaryLandscape(World);
	AddRVTToLandscapeForNativeGrass(Landscape, TitanRVTD, TitanRVTH, TitanRVTDH);

	if (bFullTitanStackABOnly || bTitanGroundContextABOnly)
	{
		int32 ABPatchCount = 0;
		int32 ABPatchInstances = 0;
		int32 TitanGroundContextLandscapes = 0;
		for (const FGrassABPatchSpec& PatchSpec : ABPatchSpecs)
		{
			if (PatchSpec.bNative)
			{
				if (bTitanGroundContextABOnly)
				{
					if (CreateTitanLandscapeGroundContextPatch(World, PatchSpec, TitanGroundMaterial, TitanRVTD, TitanRVTH, TitanRVTDH))
					{
						++TitanGroundContextLandscapes;
					}
				}
				else
				{
					SpawnPatchRVTVolumes(World, Landscape, PatchSpec, TitanRVTD, TitanRVTH, TitanRVTDH);
				}
			}

			int32 PatchInstances = 0;
			AActor* PatchActor = CreateABPatchActor(
				World,
				PatchSpec,
				GrassMeshes[0],
				ProxyMaterial,
				NativeMaterial,
				NativeDarkerMaterial,
				TitanGroundMaterial,
				GroundPlaneMesh,
				PatchInstances);
			if (PatchActor && PatchInstances > 0)
			{
				++ABPatchCount;
				ABPatchInstances += PatchInstances;
				CreateABComparisonCamera(World, PatchSpec);
				UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandSpawnGrassProof: ABOnly label=%s fullTitan=%s instances=%d center=(%.1f, %.1f) cameraTag=%s titanGroundContext=%s"),
					PatchSpec.Label,
					PatchSpec.bNative ? TEXT("true") : TEXT("false"),
					PatchInstances,
					PatchSpec.Center.X,
					PatchSpec.Center.Y,
					PatchSpec.CameraTag,
					(bTitanGroundContextABOnly && PatchSpec.bNative) ? TEXT("true") : TEXT("false"));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("FFStarterHighlandSpawnGrassProof: ABOnly patch failed or empty label=%s"), PatchSpec.Label);
			}
		}

		World->MarkPackageDirty();
		const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
		const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandSpawnGrassProof: ABOnly removedActors=%d abPatches=%d abPatchInstances=%d titanGroundContextLandscapes=%d savedMap=%s savedPackages=%s"),
			RemovedActors,
			ABPatchCount,
			ABPatchInstances,
			TitanGroundContextLandscapes,
			bSavedMap ? TEXT("true") : TEXT("false"),
			bSavedPackages ? TEXT("true") : TEXT("false"));
		return (bSavedMap && bSavedPackages && ABPatchCount == UE_ARRAY_COUNT(ABPatchSpecs)) ? 0 : 1;
	}

	if (ULandscapeGrassType* BlockedLandscapeGrassType = LoadObject<ULandscapeGrassType>(nullptr, BlockedLandscapeGrassTypePath))
	{
		BlockedLandscapeGrassType->Modify();
		for (FGrassVariety& Variety : BlockedLandscapeGrassType->GrassVarieties)
		{
			Variety.GrassDensity = FPerPlatformFloat(0.0f);
			Variety.GrassDensityQuality = FPerQualityLevelFloat(0.0f);
		}
		BlockedLandscapeGrassType->MarkPackageDirty();
		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandSpawnGrassProof: disabled blocked LandscapeGrassOutput source %s varieties=%d"),
			BlockedLandscapeGrassTypePath,
			BlockedLandscapeGrassType->GrassVarieties.Num());
	}

	int32 HiddenVegetationActors = 0;
	UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandSpawnGrassProof: vegetation visibility left unchanged for scoped grass/ground polish."));

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AActor* GrassActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
	if (!GrassActor)
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandSpawnGrassProof: failed to spawn proof actor."));
		return 1;
	}

	GrassActor->Tags.AddUnique(SpawnGrassProofTag);
	GrassActor->Tags.AddUnique(Phase1GrassTag);
	GrassActor->SetActorLabel(TEXT("FF_Phase1_TitanGrassCarpet_HISM"));
	USceneComponent* RootComponent = NewObject<USceneComponent>(GrassActor, TEXT("Root"), RF_Transactional);
	RootComponent->SetMobility(EComponentMobility::Static);
	GrassActor->SetRootComponent(RootComponent);
	RootComponent->RegisterComponent();
	GrassActor->AddInstanceComponent(RootComponent);

	int32 TotalInstances = 0;
	int32 RejectedSamples = 0;
	int32 RejectedByWater = 0;
	int32 RejectedByPath = 0;
	int32 RejectedBySlope = 0;
	int32 RejectedByDensityNoise = 0;
	TArray<int32> LayerTotals;
	LayerTotals.Init(0, UE_ARRAY_COUNT(GrassCarpetLayers));
	for (int32 LayerIndex = 0; LayerIndex < UE_ARRAY_COUNT(GrassCarpetLayers) && LayerIndex < GrassMeshes.Num(); ++LayerIndex)
	{
		const FGrassCarpetLayer& Layer = GrassCarpetLayers[LayerIndex];
		UHierarchicalInstancedStaticMeshComponent* Component = CreateGrassComponent(GrassActor, GrassMeshes[LayerIndex], GrassMaterials.IsValidIndex(LayerIndex) ? GrassMaterials[LayerIndex] : nullptr, LayerIndex);
		if (!Component)
		{
			continue;
		}

		int32 AddedForLayer = 0;
		const int32 GridCountX = FMath::CeilToInt((CarpetExtents.X * 2.0f) / Layer.Spacing);
		const int32 GridCountY = FMath::CeilToInt((CarpetExtents.Y * 2.0f) / Layer.Spacing);
		for (int32 GridY = 0; GridY < GridCountY; ++GridY)
		{
			for (int32 GridX = 0; GridX < GridCountX; ++GridX)
			{
				const int32 Seed = (LayerIndex + 1) * 100000000 + GridX * 73856093 + GridY * 19349663;
				const FVector2D Position = GetCarpetPoint(Layer, GridX, GridY, Seed);
				if (!IsInsideIslandInterior(Position))
				{
					++RejectedSamples;
					continue;
				}

				if (GetNearestWaterDistance(Position) < 1500.0f)
				{
					++RejectedSamples;
					++RejectedByWater;
					continue;
				}

				if (IsNearPath(Position))
				{
					++RejectedSamples;
					++RejectedByPath;
					continue;
				}

				if (PseudoRandom01(Seed + 83) > GetCarpetKeepChance(Position, Layer, Seed))
				{
					++RejectedSamples;
					++RejectedByDensityNoise;
					continue;
				}

				FHitResult GroundHit;
				if (!GetPlacementGround(World, Position, GroundHit) || GroundHit.ImpactNormal.Z < Layer.MinNormalZ)
				{
					++RejectedSamples;
					++RejectedBySlope;
					continue;
				}

				const float Yaw = PseudoRandom01(Seed + 23) * 360.0f;
				const float XYScale = FMath::Lerp(Layer.MinXYScale, Layer.MaxXYScale, PseudoRandom01(Seed + 31));
				const float ZScale = FMath::Lerp(Layer.MinZScale, Layer.MaxZScale, PseudoRandom01(Seed + 47));

				FTransform InstanceTransform;
				InstanceTransform.SetLocation(GroundHit.ImpactPoint + FVector(0.0f, 0.0f, 0.8f));
				const FQuat SurfaceRotation = FRotationMatrix::MakeFromZ(GroundHit.ImpactNormal).ToQuat();
				const FQuat YawAroundSurface(GroundHit.ImpactNormal, FMath::DegreesToRadians(Yaw));
				InstanceTransform.SetRotation(YawAroundSurface * SurfaceRotation);
				InstanceTransform.SetScale3D(FVector(XYScale, XYScale, ZScale));
				Component->AddInstance(InstanceTransform, true);
				++AddedForLayer;
				++LayerTotals[LayerIndex];
				++TotalInstances;
			}
		}

		Component->BuildTreeIfOutdated(true, true);
		Component->MarkPackageDirty();
		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandSpawnGrassProof: layer=%s mesh=%s material=%s instances=%d spacing=%.1f cullEnd=%d"),
			Layer.Name,
			*GrassMeshes[LayerIndex]->GetPathName(),
			GrassMaterials.IsValidIndex(LayerIndex) ? *GrassMaterials[LayerIndex]->GetPathName() : TEXT("None"),
			AddedForLayer,
			Layer.Spacing,
			Layer.EndCullDistance);
	}

	for (int32 LayerIndex = 0; LayerIndex < LayerTotals.Num(); ++LayerIndex)
	{
		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandSpawnGrassProof: layerTotal=%s instances=%d"),
			GrassCarpetLayers[LayerIndex].Name,
			LayerTotals[LayerIndex]);
	}

	int32 ABPatchCount = 0;
	int32 ABPatchInstances = 0;
	for (const FGrassABPatchSpec& PatchSpec : ABPatchSpecs)
	{
		int32 PatchInstances = 0;
		if (PatchSpec.bNative)
		{
			SpawnPatchRVTVolumes(World, Landscape, PatchSpec, TitanRVTD, TitanRVTH, TitanRVTDH);
		}
		AActor* PatchActor = CreateABPatchActor(
			World,
			PatchSpec,
			GrassMeshes[0],
			ProxyMaterial,
			NativeMaterial,
			NativeDarkerMaterial,
			TitanGroundMaterial,
			GroundPlaneMesh,
			PatchInstances);
		if (PatchActor && PatchInstances > 0)
		{
			++ABPatchCount;
			ABPatchInstances += PatchInstances;
			CreateABComparisonCamera(World, PatchSpec);
			UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandSpawnGrassProof: ABPatch label=%s native=%s instances=%d center=(%.1f, %.1f) cameraTag=%s"),
				PatchSpec.Label,
				PatchSpec.bNative ? TEXT("true") : TEXT("false"),
				PatchInstances,
				PatchSpec.Center.X,
				PatchSpec.Center.Y,
				PatchSpec.CameraTag);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("FFStarterHighlandSpawnGrassProof: ABPatch failed or empty label=%s"), PatchSpec.Label);
		}
	}

	GrassActor->MarkPackageDirty();
	World->MarkPackageDirty();
	const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
	const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
	UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandSpawnGrassProof: removedActors=%d hiddenVegetationActors=%d totalInstances=%d abPatches=%d abPatchInstances=%d rejectedSamples=%d rejectedByWater=%d rejectedByPath=%d rejectedBySlope=%d rejectedByDensityNoise=%d savedMap=%s savedPackages=%s"),
		RemovedActors,
		HiddenVegetationActors,
		TotalInstances,
		ABPatchCount,
		ABPatchInstances,
		RejectedSamples,
		RejectedByWater,
		RejectedByPath,
		RejectedBySlope,
		RejectedByDensityNoise,
		bSavedMap ? TEXT("true") : TEXT("false"),
		bSavedPackages ? TEXT("true") : TEXT("false"));

	return (bSavedMap && bSavedPackages && TotalInstances > 0 && ABPatchCount == UE_ARRAY_COUNT(ABPatchSpecs)) ? 0 : 1;
#else
	UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandSpawnGrassProof can only run in editor builds."));
	return 1;
#endif
}
