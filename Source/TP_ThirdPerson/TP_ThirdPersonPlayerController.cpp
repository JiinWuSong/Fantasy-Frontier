// Copyright Epic Games, Inc. All Rights Reserved.


#include "TP_ThirdPersonPlayerController.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "FantasyFrontierAppearancePresetSaveGame.h"
#include "FantasyFrontierCharacterCreatorTypes.h"
#include "FantasyFrontierCharacterCreatorWidget.h"
#include "FantasyFrontierCharacterPreviewActor.h"
#include "FantasyFrontierEnemyBase.h"
#include "FantasyFrontierFunctionalNpc.h"
#include "FantasyFrontierFrontEndWidget.h"
#include "FantasyFrontierPlayableCharacter.h"
#include "FantasyFrontierTutorialDirector.h"
#include "FantasyFrontierUserSettingsSaveGame.h"
#include "FantasyFrontierVerticalSliceTypes.h"
#include "AudioDevice.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Engine/TextureRenderTarget2D.h"
#include "EngineUtils.h"
#include "GameFramework/GameUserSettings.h"
#include "Engine/GameViewportClient.h"
#include "Engine/HitResult.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputMappingContext.h"
#include "LandscapeComponent.h"
#include "LandscapeDataAccess.h"
#include "LandscapeGrassType.h"
#include "LandscapeProxy.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/DateTime.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "HAL/IConsoleManager.h"
#include "Sound/SoundBase.h"
#include "TP_ThirdPerson.h"
#include "UnrealClient.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Widgets/Input/SVirtualJoystick.h"

namespace
{
	bool IsStarterForestWorld(const UWorld* World)
	{
		if (!World)
		{
			return false;
		}

		const FString MapName = World->GetMapName();
		if (MapName.Contains(TEXT("TitanMainGrasslandHostSandbox")) ||
			MapName.Contains(TEXT("TitanMainGrasslandRealComponentSandbox")))
		{
			return false;
		}

		return MapName.Contains(TEXT("LI_CozyLake")) ||
			MapName.Contains(TEXT("LI_StarterIsland_LowerIsland")) ||
			MapName.Contains(TEXT("LI_CelticVillage")) ||
			MapName.Contains(TEXT("LI_Grassland_Celtic_Village_Landscaping")) ||
			MapName.Contains(TEXT("LI_Grassland_Farm")) ||
			MapName.Contains(TEXT("LI_Likana_Valley")) ||
			MapName.Contains(TEXT("LI_ShoreLake_village")) ||
			MapName.Contains(TEXT("LI_Littlegarden")) ||
			MapName.Contains(TEXT("LI_Grassland_Village")) ||
			MapName.Contains(TEXT("LI_Grassland_FLerihnVillage_Lerigothi")) ||
			MapName.Contains(TEXT("LI_Kashkeh_")) ||
			MapName.Contains(TEXT("FF_Starter_GrasslandRegion")) ||
			MapName.Contains(TEXT("Lvl_StarterForest")) ||
			MapName.Contains(TEXT("TitanMain"));
	}

	bool IsLowerIslandStarterForestWorld(const UWorld* World)
	{
		return World && World->GetMapName().Contains(TEXT("LI_StarterIsland_LowerIsland"));
	}

	bool IsTitanMainStarterForestWorld(const UWorld* World)
	{
		return World && World->GetMapName().Contains(TEXT("TitanMain")) &&
			!World->GetMapName().Contains(TEXT("TitanMainGrasslandHostSandbox")) &&
			!World->GetMapName().Contains(TEXT("TitanMainGrasslandRealComponentSandbox"));
	}

	bool IsFFStarterGrasslandRegionWorld(const UWorld* World)
	{
		return World && World->GetMapName().Contains(TEXT("FF_Starter_GrasslandRegion"));
	}

	bool IsFFStarterHighlandBlockoutWorld(const UWorld* World)
	{
		return World && World->GetMapName().Contains(TEXT("FF_Starter_Highland_Blockout"));
	}

	bool IsHighlandNativeMiniProofSmokeWorld(const UWorld* World)
	{
		return IsFFStarterHighlandBlockoutWorld(World) && FParse::Param(FCommandLine::Get(), TEXT("FFHighlandNativeMiniProofSmoke"));
	}

	bool IsTitanMainGrasslandHostSandboxWorld(const UWorld* World)
	{
		return World &&
			(World->GetMapName().Contains(TEXT("TitanMainGrasslandHostSandbox")) ||
				World->GetMapName().Contains(TEXT("TitanMainGrasslandRealComponentSandbox")));
	}

	int64 CountNonZeroGrassWeights(const ULandscapeComponent* Component)
	{
		if (!Component || Component->GrassData->NumElements <= 0)
		{
			return 0;
		}

		int64 NonZeroSamples = 0;
		const int32 NumElements = Component->GrassData->NumElements;
		for (const TPair<TObjectPtr<ULandscapeGrassType>, int32>& Pair : Component->GrassData->WeightOffsets)
		{
			const int32 Offset = Pair.Value;
			const bool bValidRange = NumElements > 0
				&& Offset >= 0
				&& (Offset + NumElements) <= Component->GrassData->HeightWeightData.Num();
			if (!bValidRange)
			{
				continue;
			}

			const uint8* WeightData = Component->GrassData->HeightWeightData.GetData() + Offset;
			for (int32 Index = 0; Index < NumElements; ++Index)
			{
				if (WeightData[Index] > 0)
				{
					++NonZeroSamples;
				}
			}
		}
		return NonZeroSamples;
	}

	struct FSandboxGrassWeightStats
	{
		int32 Samples = 0;
		int32 NonZero = 0;
		int32 InRange = 0;
		uint8 MaxWeight = 0;
		double AverageWeight = 0.0;
	};

	struct FSandboxGrassKeepStats
	{
		int32 SqrtMaxInstances = 0;
		int32 SqrtSubsections = 0;
		int32 CandidateSamples = 0;
		int32 WeightPositive = 0;
		int32 WeightPassesRandom = 0;
		int32 ExclusionBoxes = 0;
		float DensityScale = 1.0f;
		float EffectiveDensity = 0.0f;
	};

	FSandboxGrassKeepStats EstimateSandboxGrassKeepStats(
		const ALandscapeProxy* Landscape,
		const ULandscapeComponent* Component,
		const ULandscapeGrassType* GrassType,
		const FGrassVariety& Variety,
		int32 VarietyIndex);

	FSandboxGrassWeightStats GetGrassWeightStatsForRange(
		const ULandscapeComponent* Component,
		const ULandscapeGrassType* GrassType,
		const FFloatInterval& AllowedRange)
	{
		FSandboxGrassWeightStats Stats;
		if (!Component || !GrassType || Component->GrassData->NumElements <= 0)
		{
			return Stats;
		}

		const int32* OffsetPtr = Component->GrassData->WeightOffsets.Find(GrassType);
		if (!OffsetPtr)
		{
			return Stats;
		}

		const int32 Offset = *OffsetPtr;
		const int32 NumElements = Component->GrassData->NumElements;
		const bool bValidRange = Offset >= 0
			&& (Offset + NumElements) <= Component->GrassData->HeightWeightData.Num();
		if (!bValidRange)
		{
			return Stats;
		}

		const uint8* WeightData = Component->GrassData->HeightWeightData.GetData() + Offset;
		uint64 WeightSum = 0;
		Stats.Samples = NumElements;
		for (int32 Index = 0; Index < NumElements; ++Index)
		{
			const uint8 RawWeight = WeightData[Index];
			const float Weight = static_cast<float>(RawWeight) / 255.0f;
			if (RawWeight > 0)
			{
				++Stats.NonZero;
			}
			if (Weight > AllowedRange.Min && Weight <= AllowedRange.Max)
			{
				++Stats.InRange;
			}
			Stats.MaxWeight = FMath::Max(Stats.MaxWeight, RawWeight);
			WeightSum += RawWeight;
		}

		Stats.AverageWeight = NumElements > 0
			? static_cast<double>(WeightSum) / static_cast<double>(NumElements)
			: 0.0;
		return Stats;
	}

	float GetConsoleVariableFloat(const TCHAR* Name, float DefaultValue)
	{
		if (IConsoleVariable* Variable = IConsoleManager::Get().FindConsoleVariable(Name))
		{
			return Variable->GetFloat();
		}

		return DefaultValue;
	}

	int32 GetConsoleVariableInt(const TCHAR* Name, int32 DefaultValue)
	{
		if (IConsoleVariable* Variable = IConsoleManager::Get().FindConsoleVariable(Name))
		{
			return Variable->GetInt();
		}

		return DefaultValue;
	}

	uint8 SampleSandboxGrassWeight(const ULandscapeComponent* Component, const ULandscapeGrassType* GrassType, float LocalX, float LocalY)
	{
		if (!Component || !GrassType || Component->GrassData->WeightOffsets.Num() == 0)
		{
			return 0;
		}

		const int32* OffsetPtr = Component->GrassData->WeightOffsets.Find(GrassType);
		if (!OffsetPtr)
		{
			return 0;
		}

		const int32 Stride = Component->ComponentSizeQuads + 1;
		const int32 X = FMath::Clamp(FMath::RoundToInt(LocalX), 0, Stride - 1);
		const int32 Y = FMath::Clamp(FMath::RoundToInt(LocalY), 0, Stride - 1);
		const int32 Index = *OffsetPtr + X + Stride * Y;
		if (!Component->GrassData->HeightWeightData.IsValidIndex(Index))
		{
			return 0;
		}

		return Component->GrassData->HeightWeightData[Index];
	}

	float SampleSandboxGrassHeight(const ULandscapeComponent* Component, float LocalX, float LocalY)
	{
		if (!Component || Component->GrassData->NumElements <= 0)
		{
			return 0.0f;
		}

		const int32 Stride = Component->ComponentSizeQuads + 1;
		const int32 X = FMath::Clamp(FMath::RoundToInt(LocalX), 0, Stride - 1);
		const int32 Y = FMath::Clamp(FMath::RoundToInt(LocalY), 0, Stride - 1);
		const int32 Index = X + Stride * Y;
		const int32 ByteIndex = Index * static_cast<int32>(sizeof(uint16));
		if (!Component->GrassData->HeightWeightData.IsValidIndex(ByteIndex + 1))
		{
			return 0.0f;
		}

		const uint16* HeightData = reinterpret_cast<const uint16*>(Component->GrassData->HeightWeightData.GetData());
		return LandscapeDataAccess::GetLocalHeight(HeightData[Index]);
	}

	bool FindSandboxFirstPredictedGrassWorldLocation(UWorld* World, FVector& OutWorldLocation)
	{
		if (!World)
		{
			return false;
		}

		for (TActorIterator<ALandscapeProxy> It(World); It; ++It)
		{
			ALandscapeProxy* Landscape = *It;
			if (!Landscape || !Landscape->GetRootComponent())
			{
				continue;
			}

			for (ULandscapeComponent* Component : Landscape->LandscapeComponents)
			{
				if (!Component || Component->GrassData->WeightOffsets.Num() == 0)
				{
					continue;
				}

				for (ULandscapeGrassType* GrassType : Component->GetGrassTypesBP())
				{
					if (!GrassType || !GrassType->GetName().Contains(TEXT("Grass")))
					{
						continue;
					}

					for (int32 VarietyIndex = 0; VarietyIndex < GrassType->GrassVarieties.Num(); ++VarietyIndex)
					{
						const FGrassVariety& Variety = GrassType->GrassVarieties[VarietyIndex];
						if (!Variety.GrassMesh || Variety.GetDensity() <= 0.0f || Variety.GetEndCullDistance() <= 0 || !Variety.bUseGrid)
						{
							continue;
						}

						const FSandboxGrassKeepStats KeepStats = EstimateSandboxGrassKeepStats(Landscape, Component, GrassType, Variety, VarietyIndex);
						if (KeepStats.SqrtMaxInstances <= 0 || KeepStats.WeightPassesRandom <= 0)
						{
							continue;
						}

						const int32 ComponentSizeQuads = Component->ComponentSizeQuads;
						const int32 SubsectionSqrtMaxInstances = FMath::Max(1, KeepStats.SqrtMaxInstances / FMath::Max(1, KeepStats.SqrtSubsections));
						const float MaxJitter1D = FMath::Clamp<float>(Variety.PlacementJitter, 0.0f, 0.99f) * 0.5f / static_cast<float>(SubsectionSqrtMaxInstances);
						for (int32 SubX = 0; SubX < FMath::Max(1, KeepStats.SqrtSubsections); ++SubX)
						{
							for (int32 SubY = 0; SubY < FMath::Max(1, KeepStats.SqrtSubsections); ++SubY)
							{
								const FString SeedString = FString::Printf(TEXT("%s%s%d %d %d"), *GrassType->GetName().ToLower(), *Component->GetName().ToLower(), SubX, SubY, VarietyIndex);
								int32 Seed = FCrc::StrCrc32(StringCast<ANSICHAR>(*SeedString).Get());
								if (Seed == 0)
								{
									++Seed;
								}

								FRandomStream RandomStream(Seed);
								for (int32 X = 0; X < SubsectionSqrtMaxInstances; ++X)
								{
									for (int32 Y = 0; Y < SubsectionSqrtMaxInstances; ++Y)
									{
										const float SubsectionSize = static_cast<float>(ComponentSizeQuads) / static_cast<float>(FMath::Max(1, KeepStats.SqrtSubsections));
										float LocalX = (static_cast<float>(SubX) + (static_cast<float>(X) + 0.5f) / static_cast<float>(SubsectionSqrtMaxInstances)) * SubsectionSize;
										float LocalY = (static_cast<float>(SubY) + (static_cast<float>(Y) + 0.5f) / static_cast<float>(SubsectionSqrtMaxInstances)) * SubsectionSize;
										LocalX += (RandomStream.GetFraction() * 2.0f - 1.0f) * MaxJitter1D * static_cast<float>(ComponentSizeQuads);
										LocalY += (RandomStream.GetFraction() * 2.0f - 1.0f) * MaxJitter1D * static_cast<float>(ComponentSizeQuads);

										const float Weight = static_cast<float>(SampleSandboxGrassWeight(Component, GrassType, LocalX, LocalY)) / 255.0f;
										if (Weight > Variety.AllowedDensityRange.Min && Weight <= Variety.AllowedDensityRange.Max && Weight >= RandomStream.GetFraction())
										{
											const FVector DrawScale = Landscape->GetRootComponent()->GetRelativeScale3D();
											const FIntPoint SectionBase = Component->GetSectionBase();
											const FIntPoint LandscapeSectionOffset = Landscape->GetSectionBase();
											const FVector LocalScaled(
												DrawScale.X * (static_cast<float>(SectionBase.X - LandscapeSectionOffset.X) + LocalX),
												DrawScale.Y * (static_cast<float>(SectionBase.Y - LandscapeSectionOffset.Y) + LocalY),
												DrawScale.Z * SampleSandboxGrassHeight(Component, LocalX, LocalY));
											OutWorldLocation = Landscape->GetRootComponent()->GetComponentTransform().TransformPositionNoScale(LocalScaled);
											return true;
										}
									}
								}
							}
						}
					}
				}
			}
		}

		return false;
	}

	bool TraceSandboxLandscapeSurfaceAtXY(UWorld* World, const FVector& WorldLocation, float& OutSurfaceZ, FString& OutHitDescription)
	{
		if (!World)
		{
			return false;
		}

		const FVector TraceStart(WorldLocation.X, WorldLocation.Y, WorldLocation.Z + 50000.0);
		const FVector TraceEnd(WorldLocation.X, WorldLocation.Y, WorldLocation.Z - 50000.0);
		FCollisionQueryParams QueryParams(TEXT("FFSandboxGrassSurfaceTrace"), true);
		QueryParams.bTraceComplex = false;

		FHitResult Hit;
		bool bHit = World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
		if (!bHit)
		{
			bHit = World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams);
		}

		if (!bHit)
		{
			OutHitDescription = TEXT("No landscape/world hit");
			return false;
		}

		OutSurfaceZ = Hit.ImpactPoint.Z;
		OutHitDescription = FString::Printf(
			TEXT("actor=%s component=%s location=%s"),
			*GetNameSafe(Hit.GetActor()),
			*GetNameSafe(Hit.GetComponent()),
			*Hit.ImpactPoint.ToString());
		return true;
	}

	void LogSandboxGrassSurfaceProbe(UWorld* World, const TCHAR* Label)
	{
		if (!World)
		{
			return;
		}

		FVector GrassLocation = FVector::ZeroVector;
		if (!FindSandboxFirstPredictedGrassWorldLocation(World, GrassLocation))
		{
			UE_LOG(LogTP_ThirdPerson, Warning, TEXT("FFSandboxGrass %s surfaceProbe no predicted grass location"), Label);
			return;
		}

		float SurfaceZ = 0.0f;
		FString HitDescription;
		const bool bHasSurface = TraceSandboxLandscapeSurfaceAtXY(World, GrassLocation, SurfaceZ, HitDescription);
		const float DeltaToSurface = bHasSurface ? (GrassLocation.Z - SurfaceZ) : 0.0f;
		UE_LOG(LogTP_ThirdPerson, Display,
			TEXT("FFSandboxGrass %s surfaceProbe predictedGrass=%s landscapeSurfaceZ=%.3f grassMinusSurfaceZ=%.3f trace=%s"),
			Label,
			*GrassLocation.ToString(),
			SurfaceZ,
			DeltaToSurface,
			*HitDescription);
	}

	FSandboxGrassKeepStats EstimateSandboxGrassKeepStats(
		const ALandscapeProxy* Landscape,
		const ULandscapeComponent* Component,
		const ULandscapeGrassType* GrassType,
		const FGrassVariety& Variety,
		int32 VarietyIndex)
	{
		FSandboxGrassKeepStats Stats;
		if (!Landscape || !Component || !GrassType || !Landscape->GetRootComponent())
		{
			return Stats;
		}

		const FVector DrawScale = Landscape->GetRootComponent()->GetRelativeScale3D();
		const int32 ComponentSizeQuads = Component->ComponentSizeQuads;
		Stats.DensityScale = GrassType->bEnableDensityScaling ? GetConsoleVariableFloat(TEXT("grass.DensityScale"), 1.0f) : 1.0f;
		Stats.EffectiveDensity = Variety.GetDensity() * Stats.DensityScale;
		const float ExtentX = DrawScale.X * static_cast<float>(ComponentSizeQuads);
		const float ExtentY = DrawScale.Y * static_cast<float>(ComponentSizeQuads);
		Stats.SqrtMaxInstances = FMath::CeilToInt32(FMath::Sqrt(FMath::Abs(ExtentX * ExtentY * Stats.EffectiveDensity / 1000.0f / 1000.0f)));
		if (Stats.SqrtMaxInstances <= 0)
		{
			return Stats;
		}

		const int32 MaxInstancesPerComponent = FMath::Max(1024, GetConsoleVariableInt(TEXT("grass.MaxInstancesPerComponent"), 65536));
		Stats.SqrtSubsections = FMath::Clamp(FMath::CeilToInt(static_cast<float>(Stats.SqrtMaxInstances) / FMath::Sqrt(static_cast<float>(MaxInstancesPerComponent))), 1, 16);
		const int32 SubsectionSqrtMaxInstances = FMath::Max(1, Stats.SqrtMaxInstances / Stats.SqrtSubsections);
		Stats.CandidateSamples = SubsectionSqrtMaxInstances * SubsectionSqrtMaxInstances * Stats.SqrtSubsections * Stats.SqrtSubsections;
		Stats.ExclusionBoxes = Component->ActiveExcludedBoxes.Num();

		const bool bUseHalton = !Variety.bUseGrid;
		const float MaxJitter1D = FMath::Clamp<float>(Variety.PlacementJitter, 0.0f, 0.99f) * 0.5f / static_cast<float>(SubsectionSqrtMaxInstances);
		int32 SampleBudget = 0;
		for (int32 SubX = 0; SubX < Stats.SqrtSubsections; ++SubX)
		{
			for (int32 SubY = 0; SubY < Stats.SqrtSubsections; ++SubY)
			{
				const FString SeedString = FString::Printf(TEXT("%s%s%d %d %d"), *GrassType->GetName().ToLower(), *Component->GetName().ToLower(), SubX, SubY, VarietyIndex);
				int32 Seed = FCrc::StrCrc32(StringCast<ANSICHAR>(*SeedString).Get());
				if (Seed == 0)
				{
					++Seed;
				}

				FRandomStream RandomStream(Seed);
				for (int32 X = 0; X < SubsectionSqrtMaxInstances; ++X)
				{
					for (int32 Y = 0; Y < SubsectionSqrtMaxInstances; ++Y)
					{
						if (++SampleBudget > 20000)
						{
							return Stats;
						}

						float LocalX = 0.0f;
						float LocalY = 0.0f;
						if (bUseHalton)
						{
							LocalX = (static_cast<float>(SubX) + 0.5f) / static_cast<float>(Stats.SqrtSubsections) * static_cast<float>(ComponentSizeQuads);
							LocalY = (static_cast<float>(SubY) + 0.5f) / static_cast<float>(Stats.SqrtSubsections) * static_cast<float>(ComponentSizeQuads);
						}
						else
						{
							const float SubsectionSize = static_cast<float>(ComponentSizeQuads) / static_cast<float>(Stats.SqrtSubsections);
							LocalX = (static_cast<float>(SubX) + (static_cast<float>(X) + 0.5f) / static_cast<float>(SubsectionSqrtMaxInstances)) * SubsectionSize;
							LocalY = (static_cast<float>(SubY) + (static_cast<float>(Y) + 0.5f) / static_cast<float>(SubsectionSqrtMaxInstances)) * SubsectionSize;
							LocalX += (RandomStream.GetFraction() * 2.0f - 1.0f) * MaxJitter1D * static_cast<float>(ComponentSizeQuads);
							LocalY += (RandomStream.GetFraction() * 2.0f - 1.0f) * MaxJitter1D * static_cast<float>(ComponentSizeQuads);
						}

						const float Weight = static_cast<float>(SampleSandboxGrassWeight(Component, GrassType, LocalX, LocalY)) / 255.0f;
						if (Weight > Variety.AllowedDensityRange.Min && Weight <= Variety.AllowedDensityRange.Max)
						{
							++Stats.WeightPositive;
							if (Weight >= RandomStream.GetFraction())
							{
								++Stats.WeightPassesRandom;
							}
						}
					}
				}
			}
		}

		return Stats;
	}

	FVector GetSandboxGrassProbeLocation(const ALandscapeProxy* Landscape)
	{
		if (!Landscape)
		{
			return FVector::ZeroVector;
		}

		const FBox Bounds = Landscape->GetComponentsBoundingBox(true);
		return FVector(Bounds.GetCenter().X, Bounds.GetCenter().Y, Bounds.Max.Z + 150.0f);
	}

	void LogSandboxNativeGrassState(UWorld* World, const TCHAR* Label)
	{
		if (!World)
		{
			return;
		}

		int32 LandscapeCount = 0;
		int32 LandscapeComponents = 0;
		int32 ComponentsWithGrassTypes = 0;
		int32 ComponentsWithGrassData = 0;
		int32 GrassWeightTypes = 0;
		int64 NonZeroWeightSamples = 0;
		int32 FoliageComponentCount = 0;
		int32 RegisteredFoliageComponents = 0;
		int32 VisibleFoliageComponents = 0;
		int32 TotalSourceInstances = 0;
		int32 TotalRenderInstances = 0;
		TArray<FString> FoliageNotes;
		TArray<FString> GrassTypeNotes;

		for (TActorIterator<ALandscapeProxy> It(World); It; ++It)
		{
			ALandscapeProxy* Landscape = *It;
			if (!Landscape)
			{
				continue;
			}

			++LandscapeCount;
			LandscapeComponents += Landscape->LandscapeComponents.Num();
			for (ULandscapeComponent* Component : Landscape->LandscapeComponents)
			{
				if (!Component)
				{
					continue;
				}

				if (!Component->GetGrassTypes().IsEmpty())
				{
					++ComponentsWithGrassTypes;
				}
				if (Component->GrassData->NumElements > 0 && Component->GrassData->WeightOffsets.Num() > 0)
				{
					++ComponentsWithGrassData;
				}
				GrassWeightTypes += Component->GrassData->WeightOffsets.Num();
				NonZeroWeightSamples += CountNonZeroGrassWeights(Component);
				for (ULandscapeGrassType* GrassType : Component->GetGrassTypesBP())
				{
					if (!GrassType)
					{
						continue;
					}
					GrassTypeNotes.Add(FString::Printf(TEXT("%s varieties=%d enableDensityScaling=%d"),
						*GrassType->GetPathName(),
						GrassType->GrassVarieties.Num(),
						GrassType->bEnableDensityScaling ? 1 : 0));
					for (int32 VarietyIndex = 0; VarietyIndex < GrassType->GrassVarieties.Num(); ++VarietyIndex)
					{
						const FGrassVariety& Variety = GrassType->GrassVarieties[VarietyIndex];
						const FSandboxGrassWeightStats WeightStats = GetGrassWeightStatsForRange(Component, GrassType, Variety.AllowedDensityRange);
						const FSandboxGrassKeepStats KeepStats = EstimateSandboxGrassKeepStats(Landscape, Component, GrassType, Variety, VarietyIndex);
						const FBox MeshBounds = Variety.GrassMesh ? Variety.GrassMesh->GetBounds().GetBox() : FBox(EForceInit::ForceInit);
						FString MeshMaterialPath = TEXT("None");
						if (Variety.GrassMesh && Variety.GrassMesh->GetStaticMaterials().Num() > 0)
						{
							MeshMaterialPath = GetPathNameSafe(Variety.GrassMesh->GetStaticMaterials()[0].MaterialInterface);
						}
						GrassTypeNotes.Add(FString::Printf(
							TEXT("%s variety=%d mesh=%s meshMaterial=%s meshBoundsSize=%s density=%.2f effectiveDensity=%.2f densityScale=%.3f grid=%d jitter=%.2f cull=%d-%d allowed=%.3f..%.3f scaleX=%.3f..%.3f scaleY=%.3f..%.3f scaleZ=%.3f..%.3f scaling=%d randomRotation=%d alignSurface=%d alignNormals=%d wpoDisable=%d samples=%d nonZero=%d inAllowedRange=%d maxRaw=%d avgRaw=%.2f sqrtMax=%d sqrtSub=%d candidates=%d predictedWeight=%d predictedKeep=%d excludeBoxes=%d"),
							*GrassType->GetName(),
							VarietyIndex,
							*GetNameSafe(Variety.GrassMesh),
							*MeshMaterialPath,
							*MeshBounds.GetSize().ToString(),
							Variety.GetDensity(),
							KeepStats.EffectiveDensity,
							KeepStats.DensityScale,
							Variety.bUseGrid ? 1 : 0,
							Variety.PlacementJitter,
							Variety.GetStartCullDistance(),
							Variety.GetEndCullDistance(),
							Variety.AllowedDensityRange.Min,
							Variety.AllowedDensityRange.Max,
							Variety.ScaleX.Min,
							Variety.ScaleX.Max,
							Variety.ScaleY.Min,
							Variety.ScaleY.Max,
							Variety.ScaleZ.Min,
							Variety.ScaleZ.Max,
							static_cast<int32>(Variety.Scaling),
							Variety.RandomRotation ? 1 : 0,
							Variety.AlignToSurface ? 1 : 0,
							Variety.bAlignToTriangleNormals ? 1 : 0,
							Variety.InstanceWorldPositionOffsetDisableDistance,
							WeightStats.Samples,
							WeightStats.NonZero,
							WeightStats.InRange,
							static_cast<int32>(WeightStats.MaxWeight),
							WeightStats.AverageWeight,
							KeepStats.SqrtMaxInstances,
							KeepStats.SqrtSubsections,
							KeepStats.CandidateSamples,
							KeepStats.WeightPositive,
							KeepStats.WeightPassesRandom,
							KeepStats.ExclusionBoxes));
					}
				}
			}

			for (UHierarchicalInstancedStaticMeshComponent* GrassComponent : Landscape->FoliageComponents)
			{
				if (!GrassComponent)
				{
					continue;
				}

				++FoliageComponentCount;
				RegisteredFoliageComponents += GrassComponent->IsRegistered() ? 1 : 0;
				VisibleFoliageComponents += GrassComponent->IsVisible() ? 1 : 0;
				TotalSourceInstances += GrassComponent->GetInstanceCount();
				TotalRenderInstances += GrassComponent->GetNumRenderInstances();
				if (FoliageNotes.Num() < 8)
				{
					FoliageNotes.Add(FString::Printf(TEXT("mesh=%s material0=%s sourceInstances=%d renderInstances=%d registered=%d visible=%d hiddenGame=%d cull=%d-%d bounds=%s"),
						*GetNameSafe(GrassComponent->GetStaticMesh()),
						*GetPathNameSafe(GrassComponent->GetMaterial(0)),
						GrassComponent->GetInstanceCount(),
						GrassComponent->GetNumRenderInstances(),
						GrassComponent->IsRegistered() ? 1 : 0,
						GrassComponent->IsVisible() ? 1 : 0,
						GrassComponent->bHiddenInGame ? 1 : 0,
						GrassComponent->InstanceStartCullDistance,
						GrassComponent->InstanceEndCullDistance,
						*GrassComponent->Bounds.GetBox().ToString()));
				}
			}
		}

		UE_LOG(LogTP_ThirdPerson, Display,
			TEXT("FFSandboxGrass %s landscapes=%d components=%d componentsWithGrassTypes=%d componentsWithGrassData=%d grassWeightTypes=%d nonZeroWeightSamples=%lld foliageComponents=%d registeredFoliage=%d visibleFoliage=%d sourceInstances=%d renderInstances=%d grassEnable=%d grassDensityScale=%.3f grassCullScale=%.3f maxInstancesPerComponent=%d grassDiscardData=%d grassDisableGPUCull=%d foliageCullScale=%.3f foliageDiscardData=%d"),
			Label,
			LandscapeCount,
			LandscapeComponents,
			ComponentsWithGrassTypes,
			ComponentsWithGrassData,
			GrassWeightTypes,
			NonZeroWeightSamples,
			FoliageComponentCount,
			RegisteredFoliageComponents,
			VisibleFoliageComponents,
			TotalSourceInstances,
			TotalRenderInstances,
			GetConsoleVariableInt(TEXT("grass.Enable"), -1),
			GetConsoleVariableFloat(TEXT("grass.DensityScale"), -1.0f),
			GetConsoleVariableFloat(TEXT("grass.CullDistanceScale"), -1.0f),
			GetConsoleVariableInt(TEXT("grass.MaxInstancesPerComponent"), -1),
			GetConsoleVariableInt(TEXT("grass.DiscardDataOnLoad"), -1),
			GetConsoleVariableInt(TEXT("grass.DisableGPUCull"), -1),
			GetConsoleVariableFloat(TEXT("foliage.CullDistanceScale"), -1.0f),
			GetConsoleVariableInt(TEXT("foliage.DiscardDataOnLoad"), -1));

		for (const FString& Note : GrassTypeNotes)
		{
			UE_LOG(LogTP_ThirdPerson, Display, TEXT("FFSandboxGrass %s grassType %s"), Label, *Note);
		}
		for (const FString& Note : FoliageNotes)
		{
			UE_LOG(LogTP_ThirdPerson, Display, TEXT("FFSandboxGrass %s foliage %s"), Label, *Note);
		}
	}

	int32 ForceSandboxNativeLandscapeGrass(UWorld* World, const FVector& CameraLocation)
	{
		if (!World)
		{
			return 0;
		}

		TArray<FVector> Cameras;
		if (!CameraLocation.IsNearlyZero())
		{
			Cameras.Add(CameraLocation);
		}

		int32 TotalComponentsCreated = 0;
		for (TActorIterator<ALandscapeProxy> It(World); It; ++It)
		{
			ALandscapeProxy* Landscape = *It;
			if (!Landscape)
			{
				continue;
			}

			Landscape->SetDisableRuntimeGrassMapGenerationProxyOnly(true);
			Landscape->FlushGrassComponents(nullptr, false);

			TArray<FVector> LandscapeCameras = Cameras;
			LandscapeCameras.Add(GetSandboxGrassProbeLocation(Landscape));

			int32 ComponentsCreated = 0;
			Landscape->UpdateGrass(LandscapeCameras, ComponentsCreated, true);
			TotalComponentsCreated += ComponentsCreated;
			UE_LOG(LogTP_ThirdPerson, Display,
				TEXT("FFSandboxGrass ForceUpdate landscape=%s componentsCreated=%d cameraCount=%d probe=%s"),
				*GetNameSafe(Landscape),
				ComponentsCreated,
				LandscapeCameras.Num(),
				*GetSandboxGrassProbeLocation(Landscape).ToString());
		}

		return TotalComponentsCreated;
	}

	void ApplySandboxGrassMaterialToggle(UWorld* World)
	{
		if (!World || !IsTitanMainGrasslandHostSandboxWorld(World))
		{
			return;
		}

		FString Toggle = TEXT("A");
		FParse::Value(FCommandLine::Get(), TEXT("FFSandboxGrassMaterialToggle="), Toggle);
		Toggle = Toggle.ToUpper();

		const bool bNativeUntouched = Toggle == TEXT("A") || Toggle == TEXT("NATIVE") || Toggle == TEXT("OFF");
		UMaterialInterface* SimpleDiagnosticMaterial = nullptr;
		if (Toggle == TEXT("E") || Toggle == TEXT("SIMPLE"))
		{
			SimpleDiagnosticMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/FantasyFrontier/Blockout/Materials/M_FF_Highland_GrassBlade_Soft.M_FF_Highland_GrassBlade_Soft"));
		}
		else if (Toggle == TEXT("F") || Toggle == TEXT("NOPDO"))
		{
			SimpleDiagnosticMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/FantasyFrontier/Test/Materials/MI_TitanGrassBlade_NoPDO.MI_TitanGrassBlade_NoPDO"));
		}
		else if (Toggle == TEXT("G") || Toggle == TEXT("NOWPOCLONE"))
		{
			SimpleDiagnosticMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/FantasyFrontier/Test/Materials/MI_TitanGrassBlade_NoWPO.MI_TitanGrassBlade_NoWPO"));
		}
		else if (Toggle == TEXT("H") || Toggle == TEXT("NOMASK"))
		{
			SimpleDiagnosticMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/FantasyFrontier/Test/Materials/MI_TitanGrassBlade_NoMask.MI_TitanGrassBlade_NoMask"));
		}
		else if (Toggle == TEXT("I") || Toggle == TEXT("NOWPO_NOMASK"))
		{
			SimpleDiagnosticMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/FantasyFrontier/Test/Materials/MI_TitanGrassBlade_NoWPO_NoMask.MI_TitanGrassBlade_NoWPO_NoMask"));
		}
		else if (Toggle == TEXT("J") || Toggle == TEXT("BRIGHT_NATIVE"))
		{
			SimpleDiagnosticMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/FantasyFrontier/Test/Materials/MI_TitanGrassBlade_NoWPO_NoMask_Bright.MI_TitanGrassBlade_NoWPO_NoMask_Bright"));
		}
		else if (Toggle == TEXT("ATTR_B") || Toggle == TEXT("ATTR_NOATTR"))
		{
			SimpleDiagnosticMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/FantasyFrontier/Test/Materials/M_FF_TitanGrassAttr_B_NoAttributes.M_FF_TitanGrassAttr_B_NoAttributes"));
		}
		else if (Toggle == TEXT("ATTR_C") || Toggle == TEXT("ATTR_BASECOLOR"))
		{
			SimpleDiagnosticMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/FantasyFrontier/Test/Materials/M_FF_TitanGrassAttr_C_BaseColor.M_FF_TitanGrassAttr_C_BaseColor"));
		}
		else if (Toggle == TEXT("ATTR_D") || Toggle == TEXT("ATTR_MASK"))
		{
			SimpleDiagnosticMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/FantasyFrontier/Test/Materials/M_FF_TitanGrassAttr_D_OpacityMask.M_FF_TitanGrassAttr_D_OpacityMask"));
		}
		else if (Toggle == TEXT("ATTR_E") || Toggle == TEXT("ATTR_NORMALS"))
		{
			SimpleDiagnosticMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/FantasyFrontier/Test/Materials/M_FF_TitanGrassAttr_E_Normals.M_FF_TitanGrassAttr_E_Normals"));
		}
		else if (Toggle == TEXT("ATTR_F") || Toggle == TEXT("ATTR_WIND"))
		{
			SimpleDiagnosticMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/FantasyFrontier/Test/Materials/M_FF_TitanGrassAttr_F_Wind.M_FF_TitanGrassAttr_F_Wind"));
		}
		else if (Toggle == TEXT("ATTR_G") || Toggle == TEXT("ATTR_FI"))
		{
			SimpleDiagnosticMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/FantasyFrontier/Test/Materials/M_FF_TitanGrassAttr_G_FoliageInteractionBypass.M_FF_TitanGrassAttr_G_FoliageInteractionBypass"));
		}
		else if (Toggle == TEXT("ATTR_H") || Toggle == TEXT("ATTR_RVT"))
		{
			SimpleDiagnosticMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/FantasyFrontier/Test/Materials/M_FF_TitanGrassAttr_H_RVTGroundTint.M_FF_TitanGrassAttr_H_RVTGroundTint"));
		}
		else if (Toggle == TEXT("ATTR_I") || Toggle == TEXT("ATTR_DEPTH"))
		{
			SimpleDiagnosticMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/FantasyFrontier/Test/Materials/M_FF_TitanGrassAttr_I_DepthOffset.M_FF_TitanGrassAttr_I_DepthOffset"));
		}
		else if (Toggle == TEXT("ATTR_J") || Toggle == TEXT("ATTR_COMBINED"))
		{
			SimpleDiagnosticMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/FantasyFrontier/Test/Materials/M_FF_TitanGrassAttr_J_CombinedSafe.M_FF_TitanGrassAttr_J_CombinedSafe"));
		}
		else if (Toggle == TEXT("ATTR_K") || Toggle == TEXT("ATTR_SAFE_MA"))
		{
			SimpleDiagnosticMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/FantasyFrontier/Test/Materials/M_FF_TitanGrassAttr_K_SafeMaterialAttributes.M_FF_TitanGrassAttr_K_SafeMaterialAttributes"));
		}
		else if (Toggle == TEXT("ATTR_L") || Toggle == TEXT("NATIVE_ROOT_NOWPO"))
		{
			SimpleDiagnosticMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/FantasyFrontier/Test/Materials/MI_TitanGrassBlade_RootSetNoWPO.MI_TitanGrassBlade_RootSetNoWPO"));
		}
		else if (Toggle == TEXT("ATTR_M") || Toggle == TEXT("NATIVE_ROOT_NOOPACITY"))
		{
			SimpleDiagnosticMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/FantasyFrontier/Test/Materials/MI_TitanGrassBlade_RootSetNoOpacity.MI_TitanGrassBlade_RootSetNoOpacity"));
		}
		else if (Toggle == TEXT("ATTR_N") || Toggle == TEXT("NATIVE_ROOT_NOWPO_NOOPACITY"))
		{
			SimpleDiagnosticMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/FantasyFrontier/Test/Materials/MI_TitanGrassBlade_RootSetNoWPO_NoOpacity.MI_TitanGrassBlade_RootSetNoWPO_NoOpacity"));
		}
		else if (Toggle == TEXT("ATTR_O") || Toggle == TEXT("NATIVE_ROOT_WPO_INTERACTION_WIND"))
		{
			SimpleDiagnosticMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/FantasyFrontier/Test/Materials/MI_TitanGrassBlade_RootSetWPO_InteractionWind.MI_TitanGrassBlade_RootSetWPO_InteractionWind"));
		}
		else if (Toggle == TEXT("ATTR_P") || Toggle == TEXT("NATIVE_ROOT_WPO_OFFSET_ONLY"))
		{
			SimpleDiagnosticMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/FantasyFrontier/Test/Materials/MI_TitanGrassBlade_RootSetWPO_OffsetOnly.MI_TitanGrassBlade_RootSetWPO_OffsetOnly"));
		}
		else if (Toggle == TEXT("ATTR_Q") || Toggle == TEXT("NATIVE_ROOT_WPO_FI_ONLY"))
		{
			SimpleDiagnosticMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/FantasyFrontier/Test/Materials/MI_TitanGrassBlade_RootSetWPO_FoliageInteractionOnly.MI_TitanGrassBlade_RootSetWPO_FoliageInteractionOnly"));
		}
		else if (Toggle == TEXT("ATTR_R") || Toggle == TEXT("NATIVE_ROOT_WPO_WIND_ONLY"))
		{
			SimpleDiagnosticMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/FantasyFrontier/Test/Materials/MI_TitanGrassBlade_RootSetWPO_WindOnly.MI_TitanGrassBlade_RootSetWPO_WindOnly"));
		}
		else if (Toggle == TEXT("ATTR_S") || Toggle == TEXT("NATIVE_ROOT_WPO_RVT_SCALE10"))
		{
			SimpleDiagnosticMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/FantasyFrontier/Test/Materials/MI_TitanGrassBlade_RootSetWPO_RVTOffsetScale10.MI_TitanGrassBlade_RootSetWPO_RVTOffsetScale10"));
		}
		else if (Toggle == TEXT("ATTR_T") || Toggle == TEXT("NATIVE_ROOT_WPO_RVT_SCALE01"))
		{
			SimpleDiagnosticMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/FantasyFrontier/Test/Materials/MI_TitanGrassBlade_RootSetWPO_RVTOffsetScale01.MI_TitanGrassBlade_RootSetWPO_RVTOffsetScale01"));
		}
		else if (Toggle == TEXT("ATTR_U") || Toggle == TEXT("NATIVE_ROOT_WPO_RVT_CLAMP005"))
		{
			SimpleDiagnosticMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/FantasyFrontier/Test/Materials/MI_TitanGrassBlade_RootSetWPO_RVTOffsetClamp005.MI_TitanGrassBlade_RootSetWPO_RVTOffsetClamp005"));
		}
		else if (Toggle == TEXT("ATTR_V") || Toggle == TEXT("NATIVE_ROOT_WPO_RVT_CLAMP010"))
		{
			SimpleDiagnosticMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/FantasyFrontier/Test/Materials/MI_TitanGrassBlade_RootSetWPO_RVTOffsetClamp010.MI_TitanGrassBlade_RootSetWPO_RVTOffsetClamp010"));
		}

		int32 ComponentCount = 0;
		int32 MaterialSlotCount = 0;
		for (TActorIterator<ALandscapeProxy> It(World); It; ++It)
		{
			ALandscapeProxy* Landscape = *It;
			if (!Landscape)
			{
				continue;
			}

			for (UHierarchicalInstancedStaticMeshComponent* GrassComponent : Landscape->FoliageComponents)
			{
				if (!GrassComponent)
				{
					continue;
				}

				++ComponentCount;
				const int32 NumMaterials = FMath::Max(1, GrassComponent->GetNumMaterials());
				for (int32 MaterialIndex = 0; MaterialIndex < NumMaterials; ++MaterialIndex)
				{
					if (bNativeUntouched)
					{
						continue;
					}

					if (SimpleDiagnosticMaterial)
					{
						GrassComponent->SetMaterial(MaterialIndex, SimpleDiagnosticMaterial);
						++MaterialSlotCount;
						continue;
					}

					UMaterialInterface* SourceMaterial = GrassComponent->GetMaterial(MaterialIndex);
					if (!SourceMaterial)
					{
						continue;
					}

					UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(SourceMaterial, GrassComponent);
					if (!DynamicMaterial)
					{
						continue;
					}

					if (Toggle == TEXT("B") || Toggle == TEXT("NORVT"))
					{
						DynamicMaterial->SetVectorParameterValue(TEXT("RVT Blend"), FLinearColor::Transparent);
					}
					else if (Toggle == TEXT("C") || Toggle == TEXT("NOFI"))
					{
						DynamicMaterial->SetScalarParameterValue(TEXT("FI  Max Rotation"), 0.0f);
						DynamicMaterial->SetScalarParameterValue(TEXT("FI Interaction Size"), 0.0f);
					}
					else if (Toggle == TEXT("D") || Toggle == TEXT("NOWPO"))
					{
						DynamicMaterial->SetVectorParameterValue(TEXT("RVT Blend"), FLinearColor::Transparent);
						DynamicMaterial->SetScalarParameterValue(TEXT("FI  Max Rotation"), 0.0f);
						DynamicMaterial->SetScalarParameterValue(TEXT("FI Interaction Size"), 0.0f);
						DynamicMaterial->SetScalarParameterValue(TEXT("Per Blade Wind Offset"), 0.0f);
						DynamicMaterial->SetScalarParameterValue(TEXT("UV Pivot Area"), 0.0f);
					}
					else
					{
						UE_LOG(LogTP_ThirdPerson, Warning, TEXT("FFSandboxGrass material toggle %s is unknown; leaving native material unchanged."), *Toggle);
						continue;
					}

					GrassComponent->SetMaterial(MaterialIndex, DynamicMaterial);
					++MaterialSlotCount;
				}
			}
		}

		UE_LOG(LogTP_ThirdPerson, Display,
			TEXT("FFSandboxGrass MaterialToggle=%s components=%d materialSlotsChanged=%d simpleMaterial=%s"),
			*Toggle,
			ComponentCount,
			MaterialSlotCount,
			*GetPathNameSafe(SimpleDiagnosticMaterial));
	}

	bool IsGrasslandVillageStarterForestWorld(const UWorld* World)
	{
		return World && World->GetMapName().Contains(TEXT("LI_Grassland_Village"));
	}

	bool IsGrasslandMolehillStarterForestWorld(const UWorld* World)
	{
		return World && World->GetMapName().Contains(TEXT("LI_Grassland_MolehillGrove"));
	}

	bool IsGrasslandFarmStarterForestWorld(const UWorld* World)
	{
		return World && World->GetMapName().Contains(TEXT("LI_Grassland_Farm"));
	}

	bool IsLikanaValleyStarterForestWorld(const UWorld* World)
	{
		return World && World->GetMapName().Contains(TEXT("LI_Likana_Valley"));
	}

	bool IsShoreLakeStarterForestWorld(const UWorld* World)
	{
		return World && World->GetMapName().Contains(TEXT("LI_ShoreLake_village"));
	}

	bool IsCelticVillageStarterForestWorld(const UWorld* World)
	{
		if (!World)
		{
			return false;
		}

		const FString MapName = World->GetMapName();
		return MapName.Contains(TEXT("LI_CelticVillage")) || MapName.Contains(TEXT("LI_Grassland_Celtic_Village_Landscaping"));
	}

	bool IsKashkehStarterForestWorld(const UWorld* World)
	{
		return World && World->GetMapName().Contains(TEXT("LI_Kashkeh_"));
	}

	bool UsesKashkehStarterBaseWorld(const UWorld* World)
	{
		return IsKashkehStarterForestWorld(World) || IsTitanMainStarterForestWorld(World) || IsFFStarterGrasslandRegionWorld(World);
	}

	FVector GetKashkehSmokeViewDirection()
	{
		return FVector(3218.1f, 796.5f, 3628.4f) - FVector(2113.0f, 2817.0f, 3600.0f);
	}

	bool IsLerigothiStarterForestWorld(const UWorld* World)
	{
		return World && World->GetMapName().Contains(TEXT("LI_Grassland_FLerihnVillage_Lerigothi"));
	}

	bool UsesLowerIslandStarterForestLayout(const UWorld* World)
	{
		return IsLowerIslandStarterForestWorld(World);
	}

	bool IsLittleGardenStarterForestWorld(const UWorld* World)
	{
		return World && World->GetMapName().Contains(TEXT("LI_Littlegarden"));
	}

	FRotator GetStarterForestPreferredViewRotation(const UWorld* World)
	{
		if (UsesLowerIslandStarterForestLayout(World))
		{
			return FRotator(-10.0f, 30.0f, 0.0f);
		}

		if (IsFFStarterHighlandBlockoutWorld(World))
		{
			return FRotator(-10.0f, 16.0f, 0.0f);
		}

		if (UsesKashkehStarterBaseWorld(World))
		{
			return FRotator(-16.0f, GetKashkehSmokeViewDirection().Rotation().Yaw, 0.0f);
		}

		if (IsGrasslandVillageStarterForestWorld(World))
		{
			return FRotator(-10.0f, -28.0f, 0.0f);
		}

		if (IsGrasslandMolehillStarterForestWorld(World))
		{
			return FRotator(-9.0f, 8.0f, 0.0f);
		}

		if (IsGrasslandFarmStarterForestWorld(World))
		{
			return FRotator(-10.0f, 18.0f, 0.0f);
		}

		if (IsLikanaValleyStarterForestWorld(World))
		{
			return FRotator(-10.0f, 34.0f, 0.0f);
		}

		if (IsShoreLakeStarterForestWorld(World))
		{
			return FRotator(-10.0f, 24.0f, 0.0f);
		}

		if (IsCelticVillageStarterForestWorld(World))
		{
			return FRotator(-10.0f, -32.0f, 0.0f);
		}

		if (IsLerigothiStarterForestWorld(World))
		{
			return FRotator(-8.0f, -22.0f, 0.0f);
		}

		return IsLittleGardenStarterForestWorld(World)
			? FRotator(-10.0f, 18.0f, 0.0f)
			: FRotator(-10.0f, -18.0f, 0.0f);
	}

	float GetDefaultMasterVolume()
	{
		return 0.20f;
	}

	float GetAppliedMasterVolume(float MasterVolume)
	{
		return FMath::Clamp(MasterVolume, 0.0f, 1.0f);
	}

	void ApplyStarterForestCameraPreset(AFantasyFrontierPlayableCharacter* PlayerCharacter)
	{
		if (!PlayerCharacter)
		{
			return;
		}

		if (USpringArmComponent* CameraBoom = PlayerCharacter->GetCameraBoom())
		{
			const bool bIsHighlandBlockout = IsFFStarterHighlandBlockoutWorld(PlayerCharacter->GetWorld());
			const bool bIsLowerIsland = UsesLowerIslandStarterForestLayout(PlayerCharacter->GetWorld());
			const bool bIsTitanMain = IsTitanMainStarterForestWorld(PlayerCharacter->GetWorld());
			const bool bIsGrasslandVillage = IsGrasslandVillageStarterForestWorld(PlayerCharacter->GetWorld());
			const bool bIsGrasslandMolehill = IsGrasslandMolehillStarterForestWorld(PlayerCharacter->GetWorld());
			const bool bIsGrasslandFarm = IsGrasslandFarmStarterForestWorld(PlayerCharacter->GetWorld());
			const bool bIsLikanaValley = IsLikanaValleyStarterForestWorld(PlayerCharacter->GetWorld());
			const bool bIsShoreLake = IsShoreLakeStarterForestWorld(PlayerCharacter->GetWorld());
			const bool bIsCelticVillage = IsCelticVillageStarterForestWorld(PlayerCharacter->GetWorld());
			const bool bIsKashkeh = IsKashkehStarterForestWorld(PlayerCharacter->GetWorld());
			const bool bUsesKashkehStarterBase = UsesKashkehStarterBaseWorld(PlayerCharacter->GetWorld());
			const bool bIsLerigothi = IsLerigothiStarterForestWorld(PlayerCharacter->GetWorld());
			const bool bIsLittleGarden = IsLittleGardenStarterForestWorld(PlayerCharacter->GetWorld());

			float TargetArmLength = 780.0f;
			FVector SocketOffset(0.0f, 84.0f, 136.0f);
			if (bIsHighlandBlockout)
			{
				TargetArmLength = 760.0f;
				SocketOffset = FVector(0.0f, 84.0f, 156.0f);
			}
			else if (bIsLowerIsland)
			{
				TargetArmLength = 620.0f;
				SocketOffset = FVector(0.0f, 72.0f, 136.0f);
			}
			else if (bUsesKashkehStarterBase)
			{
				TargetArmLength = 700.0f;
				SocketOffset = FVector(0.0f, 84.0f, 144.0f);
			}
			else if (bIsLikanaValley)
			{
				TargetArmLength = 760.0f;
				SocketOffset = FVector(0.0f, 96.0f, 152.0f);
			}
			else if (bIsShoreLake)
			{
				TargetArmLength = 760.0f;
				SocketOffset = FVector(0.0f, 92.0f, 148.0f);
			}
			else if (bIsLerigothi)
			{
				TargetArmLength = 700.0f;
				SocketOffset = FVector(0.0f, 74.0f, 150.0f);
			}
		else if (bIsTitanMain || bIsGrasslandVillage || bIsGrasslandMolehill || bIsGrasslandFarm || bIsCelticVillage)
		{
			TargetArmLength = 760.0f;
			SocketOffset = FVector(0.0f, 96.0f, 152.0f);
		}
			else if (bIsLittleGarden)
			{
				TargetArmLength = 760.0f;
				SocketOffset = FVector(0.0f, 92.0f, 148.0f);
			}

			CameraBoom->TargetArmLength = TargetArmLength;
			CameraBoom->SocketOffset = SocketOffset;
			CameraBoom->bEnableCameraLag = false;
			CameraBoom->bEnableCameraRotationLag = false;
			CameraBoom->ProbeChannel = ECC_Camera;
			CameraBoom->ProbeSize = bIsHighlandBlockout ? 24.0f : 12.0f;
			CameraBoom->bDoCollisionTest = !(bIsLowerIsland || bIsCelticVillage || bIsLerigothi || bIsLikanaValley);
			if (bIsGrasslandFarm)
			{
				CameraBoom->TargetArmLength = 560.0f;
				CameraBoom->SocketOffset = FVector(0.0f, 72.0f, 132.0f);
				CameraBoom->bDoCollisionTest = false;
			}
		}

		if (UCameraComponent* FollowCamera = PlayerCharacter->GetFollowCamera())
		{
			FollowCamera->SetFieldOfView(IsFFStarterHighlandBlockoutWorld(PlayerCharacter->GetWorld())
				? 68.0f
				: (UsesLowerIslandStarterForestLayout(PlayerCharacter->GetWorld())
				? 72.0f
				: (IsLikanaValleyStarterForestWorld(PlayerCharacter->GetWorld())
					? 69.0f
					: (IsShoreLakeStarterForestWorld(PlayerCharacter->GetWorld())
						? 69.0f
					: (IsLerigothiStarterForestWorld(PlayerCharacter->GetWorld())
						? 71.0f
						: ((UsesKashkehStarterBaseWorld(PlayerCharacter->GetWorld()) || IsGrasslandVillageStarterForestWorld(PlayerCharacter->GetWorld()) || IsGrasslandMolehillStarterForestWorld(PlayerCharacter->GetWorld()) || IsGrasslandFarmStarterForestWorld(PlayerCharacter->GetWorld()) || IsCelticVillageStarterForestWorld(PlayerCharacter->GetWorld())) ? 69.0f : (IsLittleGardenStarterForestWorld(PlayerCharacter->GetWorld()) ? 69.0f : 70.0f)))))));
		}
	}

	const TCHAR* LexToStringMusicState(ATP_ThirdPersonPlayerController::EFantasyFrontierMusicState State)
	{
		switch (State)
		{
		case ATP_ThirdPersonPlayerController::EFantasyFrontierMusicState::Title:
			return TEXT("Title");
		case ATP_ThirdPersonPlayerController::EFantasyFrontierMusicState::CharacterCreator:
			return TEXT("CharacterCreator");
		case ATP_ThirdPersonPlayerController::EFantasyFrontierMusicState::Overworld:
			return TEXT("Overworld");
		case ATP_ThirdPersonPlayerController::EFantasyFrontierMusicState::Combat:
			return TEXT("Combat");
		case ATP_ThirdPersonPlayerController::EFantasyFrontierMusicState::City:
			return TEXT("City");
		case ATP_ThirdPersonPlayerController::EFantasyFrontierMusicState::Shop:
			return TEXT("Shop");
		case ATP_ThirdPersonPlayerController::EFantasyFrontierMusicState::None:
		default:
			return TEXT("None");
		}
	}

	float GetTargetMusicVolume(ATP_ThirdPersonPlayerController::EFantasyFrontierMusicState State)
	{
		switch (State)
		{
		case ATP_ThirdPersonPlayerController::EFantasyFrontierMusicState::Title:
			return 0.78f;
		case ATP_ThirdPersonPlayerController::EFantasyFrontierMusicState::CharacterCreator:
			return 0.68f;
		case ATP_ThirdPersonPlayerController::EFantasyFrontierMusicState::Overworld:
			return 0.72f;
		case ATP_ThirdPersonPlayerController::EFantasyFrontierMusicState::Combat:
			return 0.78f;
		case ATP_ThirdPersonPlayerController::EFantasyFrontierMusicState::City:
			return 0.64f;
		case ATP_ThirdPersonPlayerController::EFantasyFrontierMusicState::Shop:
			return 0.62f;
		case ATP_ThirdPersonPlayerController::EFantasyFrontierMusicState::None:
		default:
			return 0.0f;
		}
	}

	float GetMusicPlaybackGain()
	{
		return 5.0f;
	}

	float GetEffectiveMusicVolume(ATP_ThirdPersonPlayerController::EFantasyFrontierMusicState State, float MasterVolume)
	{
		return GetTargetMusicVolume(State) * GetMusicPlaybackGain() * GetAppliedMasterVolume(MasterVolume);
	}

	bool TrySnapCharacterToGroundWithCapsuleSweep(AFantasyFrontierPlayableCharacter* PlayerCharacter, FVector& OutSnappedLocation)
	{
		if (!PlayerCharacter)
		{
			return false;
		}

		UWorld* World = PlayerCharacter->GetWorld();
		UCapsuleComponent* CapsuleComponent = PlayerCharacter->GetCapsuleComponent();
		if (!World || !CapsuleComponent)
		{
			return false;
		}

		const float CapsuleRadius = CapsuleComponent->GetScaledCapsuleRadius();
		const float CapsuleHalfHeight = CapsuleComponent->GetScaledCapsuleHalfHeight();
		const FVector CurrentLocation = PlayerCharacter->GetActorLocation();
		const FVector SweepStart = CurrentLocation + FVector(0.0f, 0.0f, CapsuleHalfHeight + 120.0f);
		const FVector SweepEnd = CurrentLocation - FVector(0.0f, 0.0f, CapsuleHalfHeight + 320.0f);
		const FCollisionShape SweepShape = FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight);

		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(FFSnapCharacterToGroundWithCapsuleSweep), false, PlayerCharacter);
		QueryParams.bReturnPhysicalMaterial = false;
		QueryParams.bTraceComplex = false;

		FHitResult SweepHit;
		if (!World->SweepSingleByChannel(SweepHit, SweepStart, SweepEnd, FQuat::Identity, ECC_WorldStatic, SweepShape, QueryParams) || !SweepHit.bBlockingHit)
		{
			return false;
		}

		OutSnappedLocation = SweepHit.Location + FVector(0.0f, 0.0f, 2.0f);
		return true;
	}
}

void ATP_ThirdPersonPlayerController::BeginPlay()
{
	Super::BeginPlay();
	LoadConfig();
	LoadPersistentSettings();
	ApplyHighlandPerformanceSettings();
	float PersistedCommandLineMasterVolume = 0.0f;
	float CommandLineMasterVolume = 0.0f;
	if (FParse::Value(FCommandLine::Get(), TEXT("FFPersistMasterVolume="), PersistedCommandLineMasterVolume))
	{
		SetMasterVolumeValue(PersistedCommandLineMasterVolume, true);
	}
	else if (FParse::Value(FCommandLine::Get(), TEXT("FFMasterVolume="), CommandLineMasterVolume))
	{
		SetMasterVolumeValue(CommandLineMasterVolume, false);
	}
	else
	{
		ApplyMasterVolume();
	}

	// only spawn touch controls on local player controllers
	if (ShouldUseTouchControls() && IsLocalPlayerController())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogTP_ThirdPerson, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}

	ShowFrontEnd();
	SetLivePawnMenuHold(true);

	bRunSmokeTest = FParse::Param(FCommandLine::Get(), TEXT("FFSmokeTest"));
	if (bRunSmokeTest && GetWorld())
	{
		bCaptureSmokeScreenshots = !FParse::Param(FCommandLine::Get(), TEXT("FFSmokeNoScreens"));
		if (!FParse::Value(FCommandLine::Get(), TEXT("FFSmokeCapturePrefix="), SmokeScreenshotPrefix) || SmokeScreenshotPrefix.IsEmpty())
		{
			SmokeScreenshotPrefix = FDateTime::Now().ToString(TEXT("FFSmoke_yyyyMMdd_HHmmss"));
		}

		ConsoleCommand(TEXT("DisableAllScreenMessages"));
		ConsoleCommand(TEXT("r.MotionBlurQuality 0"));
		if (IsGrasslandFarmStarterForestWorld(GetWorld()) || UsesKashkehStarterBaseWorld(GetWorld()) || IsFFStarterHighlandBlockoutWorld(GetWorld()) || IsTitanMainGrasslandHostSandboxWorld(GetWorld()))
		{
			UE_LOG(LogTP_ThirdPerson, Display, TEXT("FFSmoke Applying starter-region render safeguards."));
			ConsoleCommand(TEXT("r.HZBOcclusion 0"));
			ConsoleCommand(TEXT("r.SceneCulling.Async.Query 0"));
			ConsoleCommand(TEXT("r.SceneCulling.Async.Update 0"));
			ConsoleCommand(TEXT("r.Shadow.Virtual.Enable 0"));
		}
		if (IsTitanMainGrasslandHostSandboxWorld(GetWorld()))
		{
			// The sandbox is a direct visual proof map; bypass the normal title/creator smoke flow.
			ConsoleCommand(TEXT("grass.Enable 1"));
			ConsoleCommand(TEXT("grass.DensityScale 1.0"));
			ConsoleCommand(TEXT("grass.CullDistanceScale 1.0"));
			HideFrontEnd(false);
			SetLivePawnMenuHold(false);
			ApplyMenuInputState(false, nullptr);
		}
		if (IsHighlandNativeMiniProofSmokeWorld(GetWorld()))
		{
			// Direct visual proof for the isolated Highland native Titan grass zone.
			ConsoleCommand(TEXT("grass.Enable 1"));
			ConsoleCommand(TEXT("grass.DensityScale 1.0"));
			ConsoleCommand(TEXT("grass.CullDistanceScale 1.0"));
			HideFrontEnd(false);
			SetLivePawnMenuHold(false);
			ApplyMenuInputState(false, nullptr);
		}
		if (IsFFStarterHighlandBlockoutWorld(GetWorld()) && FParse::Param(FCommandLine::Get(), TEXT("FFSmokeHighlandCaptureOnly")))
		{
			// Lightweight art-review captures skip the title/creator/tutorial smoke sequence.
			HideFrontEnd(false);
			SetLivePawnMenuHold(false);
			ApplyMenuInputState(false, nullptr);
		}
		GetWorldTimerManager().SetTimer(SmokeStepTimer, this, &ThisClass::AdvanceSmokeTest, 0.6f, false);
	}
}

void ATP_ThirdPersonPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(SmokeStepTimer);
	GetWorldTimerManager().ClearTimer(SmokeMoveTimer);
	GetWorldTimerManager().ClearTimer(SmokeExitTimer);
	HideCharacterCreator();
	HideFrontEnd();
	Super::EndPlay(EndPlayReason);
}

void ATP_ThirdPersonPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

bool ATP_ThirdPersonPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}

void ATP_ThirdPersonPlayerController::ShowFrontEnd()
{
	if (!IsLocalPlayerController() || FrontEndWidget.IsValid() || !GEngine || !GEngine->GameViewport)
	{
		return;
	}

	SAssignNew(FrontEndWidget, SFantasyFrontierFrontEndWidget)
		.OnStartGame(FSimpleDelegate::CreateUObject(this, &ThisClass::StartGameFromFrontEnd))
		.OnQuit(FSimpleDelegate::CreateUObject(this, &ThisClass::QuitFromFrontEnd))
		.OnCycleWindowMode(FSimpleDelegate::CreateUObject(this, &ThisClass::CycleWindowMode))
		.OnCycleQuality(FSimpleDelegate::CreateUObject(this, &ThisClass::CycleQualityLevel))
		.OnCycleShadowQuality(FSimpleDelegate::CreateUObject(this, &ThisClass::CycleShadowQuality))
		.OnCycleAntiAliasingQuality(FSimpleDelegate::CreateUObject(this, &ThisClass::CycleAntiAliasingQuality))
		.OnCyclePostProcessQuality(FSimpleDelegate::CreateUObject(this, &ThisClass::CyclePostProcessQuality))
		.OnCycleViewDistanceQuality(FSimpleDelegate::CreateUObject(this, &ThisClass::CycleViewDistanceQuality))
		.OnCycleGrassDensityQuality(FSimpleDelegate::CreateUObject(this, &ThisClass::CycleGrassDensityQuality))
		.OnCycleFoliageDistanceQuality(FSimpleDelegate::CreateUObject(this, &ThisClass::CycleFoliageDistanceQuality))
		.WindowModeLabel_Lambda([this]() { return GetWindowModeText(); })
		.QualityLabel_Lambda([this]() { return GetQualityLevelText(); })
		.ShadowQualityLabel_Lambda([this]() { return GetShadowQualityText(); })
		.AntiAliasingLabel_Lambda([this]() { return GetAntiAliasingQualityText(); })
		.PostProcessLabel_Lambda([this]() { return GetPostProcessQualityText(); })
		.ViewDistanceLabel_Lambda([this]() { return GetViewDistanceQualityText(); })
		.GrassDensityLabel_Lambda([this]() { return GetGrassDensityQualityText(); })
		.FoliageDistanceLabel_Lambda([this]() { return GetFoliageDistanceQualityText(); })
		.OnMasterVolumeChanged(FOnFloatValueChanged::CreateUObject(this, &ThisClass::HandleMasterVolumeChanged))
		.MasterVolumeLabel_Lambda([this]() { return GetMasterVolumeText(); })
		.MasterVolumeValue_Lambda([this]() { return GetMasterVolumeNormalized(); });

	GEngine->GameViewport->AddViewportWidgetContent(FrontEndWidget.ToSharedRef(), 100);
	bFrontEndVisible = true;
	ApplyMenuInputState(true, FrontEndWidget);
	StartFrontEndMusic(EFantasyFrontierMusicState::Title);
	SetLivePawnMenuHold(true);
}

void ATP_ThirdPersonPlayerController::HideFrontEnd(bool bStopMusic)
{
	if (!FrontEndWidget.IsValid())
	{
		return;
	}

	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(FrontEndWidget.ToSharedRef());
	}

	FrontEndWidget.Reset();
	bFrontEndVisible = false;
	if (bStopMusic)
	{
		StopFrontEndMusic();
	}
}

void ATP_ThirdPersonPlayerController::ShowCharacterCreator()
{
	if (!IsLocalPlayerController() || CharacterCreatorWidget.IsValid() || !GEngine || !GEngine->GameViewport)
	{
		return;
	}

	EnsureCharacterPreview();

	SAssignNew(CharacterCreatorWidget, SFantasyFrontierCharacterCreatorWidget)
		.PreviewTexture(CharacterPreviewRenderTarget)
		.OnCancel(FSimpleDelegate::CreateUObject(this, &ThisClass::HandleCharacterCreatorCancelled))
		.OnDraftChanged(FFantasyFrontierCharacterDraftChanged::CreateUObject(this, &ThisClass::HandleCharacterDraftChanged))
		.OnPreviewRotate(FFantasyFrontierPreviewRotate::CreateUObject(this, &ThisClass::HandleCharacterPreviewRotated))
		.OnPreviewZoom(FFantasyFrontierPreviewZoom::CreateUObject(this, &ThisClass::HandleCharacterPreviewZoomed))
		.OnSavePreset(FSimpleDelegate::CreateUObject(this, &ThisClass::SaveAppearancePreset))
		.OnLoadPreset(FSimpleDelegate::CreateUObject(this, &ThisClass::LoadAppearancePreset))
		.OnConfirm(FFantasyFrontierCharacterDraftCommitted::CreateUObject(this, &ThisClass::CommitCharacterCreator));

	if (!LastLoadedDraft.CharacterName.IsEmpty())
	{
		CharacterCreatorWidget->SetDraft(LastLoadedDraft);
	}

	GEngine->GameViewport->AddViewportWidgetContent(CharacterCreatorWidget.ToSharedRef(), 105);
	bCharacterCreatorVisible = true;
	ApplyMenuInputState(true, CharacterCreatorWidget);
	StartFrontEndMusic(EFantasyFrontierMusicState::CharacterCreator);
	SetLivePawnMenuHold(true);
}

void ATP_ThirdPersonPlayerController::HideCharacterCreator(bool bDestroyPreviewActor)
{
	if (CharacterCreatorWidget.IsValid() && GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(CharacterCreatorWidget.ToSharedRef());
	}

	CharacterCreatorWidget.Reset();
	bCharacterCreatorVisible = false;

	if (bDestroyPreviewActor && CharacterPreviewActor)
	{
		CharacterPreviewActor->Destroy();
		CharacterPreviewActor = nullptr;
	}
}

void ATP_ThirdPersonPlayerController::ApplyMenuInputState(bool bUIEnabled, const TSharedPtr<SWidget>& FocusWidget)
{
	bShowMouseCursor = bUIEnabled;
	bEnableClickEvents = bUIEnabled;
	bEnableMouseOverEvents = bUIEnabled;
	ResetIgnoreMoveInput();
	ResetIgnoreLookInput();
	SetIgnoreMoveInput(bUIEnabled);
	SetIgnoreLookInput(bUIEnabled);

	if (bUIEnabled)
	{
		FInputModeUIOnly InputMode;
		if (FocusWidget.IsValid())
		{
			InputMode.SetWidgetToFocus(FocusWidget);
		}
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
	}
	else
	{
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
	}
}

void ATP_ThirdPersonPlayerController::StartFrontEndMusic(EFantasyFrontierMusicState DesiredState)
{
	SetFrontEndMusicState(DesiredState);
}

void ATP_ThirdPersonPlayerController::StopFrontEndMusic(float FadeOutDuration)
{
	if (!FrontEndMusicComponent)
	{
		CurrentMusicState = EFantasyFrontierMusicState::None;
		return;
	}

	if (FadeOutDuration > 0.0f)
	{
		FrontEndMusicComponent->FadeOut(FadeOutDuration, 0.0f);
	}
	else
	{
		FrontEndMusicComponent->Stop();
	}

	FrontEndMusicComponent = nullptr;
	CurrentMusicState = EFantasyFrontierMusicState::None;
	UE_LOG(LogTP_ThirdPerson, Display, TEXT("FFMusic STOP fade=%.2f"), FadeOutDuration);
}

void ATP_ThirdPersonPlayerController::SetFrontEndMusicState(EFantasyFrontierMusicState DesiredState, float FadeOutDuration)
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	if (DesiredState == EFantasyFrontierMusicState::None)
	{
		StopFrontEndMusic(FadeOutDuration);
		return;
	}

	USoundBase* DesiredCue = ResolveMusicCue(DesiredState);
	if (!DesiredCue)
	{
		UE_LOG(LogTP_ThirdPerson, Warning, TEXT("FFMusic FAIL state=%s reason=MissingCue"), LexToStringMusicState(DesiredState));
		StopFrontEndMusic(FadeOutDuration);
		return;
	}

	const float EffectiveVolume = GetEffectiveMusicVolume(DesiredState, GetMasterVolumeNormalized());
	if (FrontEndMusicComponent && FrontEndMusicComponent->Sound == DesiredCue)
	{
		if (!FrontEndMusicComponent->IsPlaying())
		{
			FrontEndMusicComponent->FadeIn(0.25f, EffectiveVolume);
		}
		FrontEndMusicComponent->SetVolumeMultiplier(EffectiveVolume);
		CurrentMusicState = DesiredState;
		UE_LOG(LogTP_ThirdPerson, Display, TEXT("FFMusic REUSE state=%s cue=%s volume=%.2f master=%.2f playing=%s"),
			LexToStringMusicState(DesiredState),
			*GetNameSafe(DesiredCue),
			EffectiveVolume,
			GetMasterVolumeNormalized(),
			FrontEndMusicComponent->IsPlaying() ? TEXT("true") : TEXT("false"));
		return;
	}

	StopFrontEndMusic(0.0f);

	FrontEndMusicComponent = UGameplayStatics::SpawnSound2D(this, DesiredCue, 0.0f, 1.0f, 0.0f, nullptr, false, false);
	if (!FrontEndMusicComponent)
	{
		CurrentMusicState = EFantasyFrontierMusicState::None;
		UE_LOG(LogTP_ThirdPerson, Warning, TEXT("FFMusic FAIL state=%s reason=SpawnSound2D"), LexToStringMusicState(DesiredState));
		return;
	}

	FrontEndMusicComponent->bIsUISound = true;
	FrontEndMusicComponent->SetVolumeMultiplier(EffectiveVolume);
	FrontEndMusicComponent->FadeIn(0.35f, EffectiveVolume);
	CurrentMusicState = DesiredState;
	UE_LOG(LogTP_ThirdPerson, Display, TEXT("FFMusic PLAY state=%s cue=%s volume=%.2f master=%.2f playing=%s"),
		LexToStringMusicState(DesiredState),
		*GetNameSafe(DesiredCue),
		EffectiveVolume,
		GetMasterVolumeNormalized(),
		FrontEndMusicComponent->IsPlaying() ? TEXT("true") : TEXT("false"));
}

bool ATP_ThirdPersonPlayerController::IsExpectedMusicPlaying(EFantasyFrontierMusicState ExpectedState, FString* OutReason) const
{
	if (CurrentMusicState != ExpectedState)
	{
		if (OutReason)
		{
			*OutReason = FString::Printf(TEXT("state=%s"), LexToStringMusicState(CurrentMusicState));
		}
		return false;
	}

	USoundBase* ExpectedCue = ResolveMusicCue(ExpectedState);
	if (!ExpectedCue)
	{
		if (OutReason)
		{
			*OutReason = TEXT("missing expected cue");
		}
		return false;
	}

	if (!FrontEndMusicComponent)
	{
		if (OutReason)
		{
			*OutReason = TEXT("audio component missing");
		}
		return false;
	}

	if (FrontEndMusicComponent->Sound != ExpectedCue)
	{
		if (OutReason)
		{
			*OutReason = FString::Printf(TEXT("wrong cue=%s"), *GetNameSafe(FrontEndMusicComponent->Sound));
		}
		return false;
	}

	if (!FrontEndMusicComponent->IsPlaying())
	{
		if (OutReason)
		{
			*OutReason = TEXT("component not playing");
		}
		return false;
	}

	if (FrontEndMusicComponent->VolumeMultiplier <= KINDA_SMALL_NUMBER)
	{
		if (OutReason)
		{
			*OutReason = TEXT("volume multiplier near zero");
		}
		return false;
	}

	if (OutReason)
	{
		*OutReason = FString::Printf(TEXT("cue=%s volume=%.2f"), *GetNameSafe(ExpectedCue), FrontEndMusicComponent->VolumeMultiplier);
	}
	return true;
}

void ATP_ThirdPersonPlayerController::SetLivePawnMenuHold(bool bHeld)
{
	AFantasyFrontierPlayableCharacter* FrontierCharacter = Cast<AFantasyFrontierPlayableCharacter>(GetPawn());
	if (!FrontierCharacter)
	{
		return;
	}

	if (!bHeld)
	{
		if (AFantasyFrontierTutorialDirector* TutorialDirector = GetSmokeTutorialDirector())
		{
			const bool bNeedsTutorialGroundSnap = IsStarterForestWorld(GetWorld()) || IsFFStarterHighlandBlockoutWorld(GetWorld());
			if (bNeedsTutorialGroundSnap)
			{
				const float HeightOffset = FrontierCharacter->GetCapsuleComponent()
					? FrontierCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 2.0f
					: 96.0f;
				const FVector CurrentLocation = FrontierCharacter->GetActorLocation();
				const FVector GroundReferenceLocation = CurrentLocation - FVector(0.0f, 0.0f, HeightOffset);
				FVector GroundSnappedLocation = TutorialDirector->ResolveTutorialGroundLocation(GroundReferenceLocation, HeightOffset);
				FVector CapsuleSweepSnappedLocation = FVector::ZeroVector;
				bool bUsedCapsuleSweep = false;
				if (TrySnapCharacterToGroundWithCapsuleSweep(FrontierCharacter, CapsuleSweepSnappedLocation))
				{
					const bool bCapsuleSweepLooksSafer =
						CapsuleSweepSnappedLocation.Z <= GroundSnappedLocation.Z + 4.0f &&
						CapsuleSweepSnappedLocation.Z <= CurrentLocation.Z + 4.0f;
					if (bCapsuleSweepLooksSafer)
					{
						GroundSnappedLocation = CapsuleSweepSnappedLocation;
						bUsedCapsuleSweep = true;
					}
				}
				if (FMath::Abs(GroundSnappedLocation.Z - CurrentLocation.Z) > 1.0f)
				{
					FrontierCharacter->SetActorLocation(GroundSnappedLocation, false, nullptr, ETeleportType::ResetPhysics);
				}

				if (IsFFStarterHighlandBlockoutWorld(GetWorld()))
				{
					UE_LOG(
						LogTP_ThirdPerson,
						Display,
						TEXT("FFHighlandGroundSnap current=(%.1f, %.1f, %.1f) snapped=(%.1f, %.1f, %.1f) method=%s"),
						CurrentLocation.X,
						CurrentLocation.Y,
						CurrentLocation.Z,
						GroundSnappedLocation.X,
						GroundSnappedLocation.Y,
						GroundSnappedLocation.Z,
						bUsedCapsuleSweep ? TEXT("CapsuleSweep") : TEXT("LineTrace"));
				}
			}
		}
	}

	if (UCharacterMovementComponent* MovementComponent = FrontierCharacter->GetCharacterMovement())
	{
		if (bHeld)
		{
			MovementComponent->StopMovementImmediately();
			MovementComponent->DisableMovement();
			MovementComponent->GravityScale = 0.0f;
		}
		else
		{
			MovementComponent->StopMovementImmediately();
			MovementComponent->GravityScale = 1.0f;
			MovementComponent->Activate(true);
			MovementComponent->SetComponentTickEnabled(true);
			MovementComponent->SetDefaultMovementMode();
			MovementComponent->SetMovementMode(MOVE_Walking);
			MovementComponent->bForceNextFloorCheck = true;
		}
	}

	FrontierCharacter->SetActorEnableCollision(!bHeld);
	FrontierCharacter->SetActorHiddenInGame(bHeld);
	if (UCapsuleComponent* CapsuleComponent = FrontierCharacter->GetCapsuleComponent())
	{
		CapsuleComponent->SetCollisionEnabled(bHeld ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryAndPhysics);
		CapsuleComponent->SetGenerateOverlapEvents(!bHeld);
	}
	if (USkeletalMeshComponent* MeshComponent = FrontierCharacter->GetMesh())
	{
		MeshComponent->SetVisibility(!bHeld, true);
		MeshComponent->SetHiddenInGame(bHeld, true);
	}
}

USoundBase* ATP_ThirdPersonPlayerController::ResolveMusicCue(EFantasyFrontierMusicState DesiredState) const
{
	switch (DesiredState)
	{
	case EFantasyFrontierMusicState::Title:
	{
		static USoundBase* Cue = LoadObject<USoundBase>(nullptr, TEXT("/Game/PathOfAdventure/Loop/CUE/CUE_Menu_Loop.CUE_Menu_Loop"));
		return Cue;
	}
	case EFantasyFrontierMusicState::CharacterCreator:
	{
		static USoundBase* Cue = LoadObject<USoundBase>(nullptr, TEXT("/Game/PathOfAdventure/Loop/CUE/CUE_Village_Loop.CUE_Village_Loop"));
		return Cue;
	}
	case EFantasyFrontierMusicState::Overworld:
	{
		static USoundBase* Cue = LoadObject<USoundBase>(nullptr, TEXT("/Game/PathOfAdventure/Loop/CUE/CUE_Field_Loop.CUE_Field_Loop"));
		return Cue;
	}
	case EFantasyFrontierMusicState::Combat:
	{
		static USoundBase* Cue = LoadObject<USoundBase>(nullptr, TEXT("/Game/PathOfAdventure/Loop/CUE/CUE_Battle_Loop.CUE_Battle_Loop"));
		return Cue;
	}
	case EFantasyFrontierMusicState::City:
	{
		static USoundBase* Cue = LoadObject<USoundBase>(nullptr, TEXT("/Game/PathOfAdventure/Loop/CUE/CUE_Village_Loop.CUE_Village_Loop"));
		return Cue;
	}
	case EFantasyFrontierMusicState::Shop:
	{
		static USoundBase* Cue = LoadObject<USoundBase>(nullptr, TEXT("/Game/PathOfAdventure/Loop/CUE/CUE_Shop_Loop.CUE_Shop_Loop"));
		return Cue;
	}
	case EFantasyFrontierMusicState::None:
	default:
		return nullptr;
	}
}

void ATP_ThirdPersonPlayerController::StartGameFromFrontEnd()
{
	if (!bFrontEndVisible)
	{
		return;
	}

	HideFrontEnd(false);
	ShowCharacterCreator();
}

void ATP_ThirdPersonPlayerController::HandleCharacterCreatorCancelled()
{
	HideCharacterCreator();
	ShowFrontEnd();
	SetLivePawnMenuHold(true);
}

void ATP_ThirdPersonPlayerController::HandleCharacterDraftChanged(FFantasyFrontierCharacterDraft InDraft)
{
	if (CharacterPreviewActor)
	{
		CharacterPreviewActor->ApplyDraft(InDraft);
	}
}

void ATP_ThirdPersonPlayerController::HandleCharacterPreviewRotated(float InDeltaYaw)
{
	if (CharacterPreviewActor)
	{
		CharacterPreviewActor->AddPreviewYaw(InDeltaYaw);
	}
}

void ATP_ThirdPersonPlayerController::HandleCharacterPreviewZoomed(float InDeltaZoom)
{
	if (CharacterPreviewActor)
	{
		CharacterPreviewActor->AddPreviewZoom(InDeltaZoom);
	}
}

void ATP_ThirdPersonPlayerController::SaveAppearancePreset()
{
	if (!CharacterCreatorWidget.IsValid())
	{
		return;
	}

	UFantasyFrontierAppearancePresetSaveGame* SaveGame = Cast<UFantasyFrontierAppearancePresetSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UFantasyFrontierAppearancePresetSaveGame::StaticClass()));
	if (!SaveGame)
	{
		return;
	}

	FFantasyFrontierAppearancePresetRecord Record;
	Record.PresetName = TEXT("Latest");
	Record.Draft = CharacterCreatorWidget->GetDraft();
	SaveGame->Presets.Add(Record);
	LastLoadedDraft = Record.Draft;
	UGameplayStatics::SaveGameToSlot(SaveGame, TEXT("FF_AppearancePreset"), 0);
}

void ATP_ThirdPersonPlayerController::LoadAppearancePreset()
{
	if (!CharacterCreatorWidget.IsValid())
	{
		return;
	}

	if (!UGameplayStatics::DoesSaveGameExist(TEXT("FF_AppearancePreset"), 0))
	{
		return;
	}

	UFantasyFrontierAppearancePresetSaveGame* SaveGame = Cast<UFantasyFrontierAppearancePresetSaveGame>(
		UGameplayStatics::LoadGameFromSlot(TEXT("FF_AppearancePreset"), 0));
	if (!SaveGame || SaveGame->Presets.IsEmpty())
	{
		return;
	}

	LastLoadedDraft = SaveGame->Presets[0].Draft;
	CharacterCreatorWidget->SetDraft(LastLoadedDraft);
}

void ATP_ThirdPersonPlayerController::CommitCharacterCreator(FFantasyFrontierCharacterDraft InDraft)
{
	LastLoadedDraft = InDraft;
	ApplyDraftToPlayerPawn(InDraft);
	EnsureTutorialDirector();

	if (PlayerState)
	{
		PlayerState->SetPlayerName(InDraft.CharacterName);
	}

	HideCharacterCreator();
	SetFrontEndMusicState(EFantasyFrontierMusicState::Overworld, 0.25f);
	ApplyMenuInputState(false, nullptr);
	SetLivePawnMenuHold(false);
	const FRotator StarterForestViewRotation = GetStarterForestPreferredViewRotation(GetWorld());
	SetControlRotation(StarterForestViewRotation);
	if (APawn* ControlledPawn = GetPawn())
	{
		ControlledPawn->SetActorRotation(FRotator(0.0f, StarterForestViewRotation.Yaw, 0.0f));
		if (AFantasyFrontierPlayableCharacter* FrontierCharacter = Cast<AFantasyFrontierPlayableCharacter>(ControlledPawn))
		{
			if (IsStarterForestWorld(GetWorld()) || IsFFStarterHighlandBlockoutWorld(GetWorld()))
			{
				ApplyStarterForestCameraPreset(FrontierCharacter);
			}
			else if (USpringArmComponent* CameraBoom = FrontierCharacter->GetCameraBoom())
			{
				CameraBoom->TargetArmLength = 520.0f;
				CameraBoom->SocketOffset = FVector(0.0f, 42.0f, 66.0f);
				CameraBoom->bEnableCameraLag = false;
				CameraBoom->bEnableCameraRotationLag = false;
			}
		}
	}

	if (GEngine && !bRunSmokeTest)
	{
		const FString Message = FString::Printf(TEXT("%s enters the frontier as %s."),
			*InDraft.CharacterName,
			InDraft.CharacterClass == EFantasyFrontierClass::Lancer ? TEXT("Lancer") : TEXT("Adventurer"));
		GEngine->AddOnScreenDebugMessage(-1, 6.0f, FColor::Emerald, Message);
	}
}

void ATP_ThirdPersonPlayerController::EnsureCharacterPreview()
{
	if (!CharacterPreviewRenderTarget)
	{
		CharacterPreviewRenderTarget = NewObject<UTextureRenderTarget2D>(this, TEXT("CharacterPreviewRenderTarget"));
		CharacterPreviewRenderTarget->ClearColor = FLinearColor(0.64f, 0.74f, 0.84f, 1.0f);
		CharacterPreviewRenderTarget->RenderTargetFormat = RTF_RGBA16f;
		CharacterPreviewRenderTarget->InitAutoFormat(1200, 1800);
		CharacterPreviewRenderTarget->TargetGamma = 2.2f;
		CharacterPreviewRenderTarget->UpdateResourceImmediate(true);
	}

	if (!CharacterPreviewActor && GetWorld())
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = this;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		CharacterPreviewActor = GetWorld()->SpawnActor<AFantasyFrontierCharacterPreviewActor>(
			AFantasyFrontierCharacterPreviewActor::StaticClass(),
			FVector(0.0f, 0.0f, -5000.0f),
			FRotator::ZeroRotator,
			SpawnParameters);

		if (CharacterPreviewActor)
		{
			CharacterPreviewActor->SetPreviewRenderTarget(CharacterPreviewRenderTarget);
			CharacterPreviewActor->ApplyDraft(FFantasyFrontierCharacterDraft());
		}
	}
}

void ATP_ThirdPersonPlayerController::ApplyDraftToPlayerPawn(const FFantasyFrontierCharacterDraft& InDraft)
{
	ACharacter* PlayerCharacter = EnsurePlayableCharacterPawn();
	if (!PlayerCharacter)
	{
		return;
	}

	AFantasyFrontierCharacterPreviewActor::ApplyDraftToGameplayMesh(PlayerCharacter->GetMesh(), InDraft);
	if (AFantasyFrontierPlayableCharacter* FrontierCharacter = Cast<AFantasyFrontierPlayableCharacter>(PlayerCharacter))
	{
		FrontierCharacter->ApplyPresentationBodyDraft(InDraft);
	}

	if (UCapsuleComponent* Capsule = PlayerCharacter->GetCapsuleComponent())
	{
		const float RadiusScale = FMath::Lerp(0.96f, 1.05f, InDraft.Build);
		Capsule->SetCapsuleSize(42.0f * RadiusScale, 96.0f * InDraft.HeightScale, true);
	}
}

void ATP_ThirdPersonPlayerController::QuitFromFrontEnd()
{
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
}

void ATP_ThirdPersonPlayerController::CycleWindowMode()
{
	if (!GEngine)
	{
		return;
	}

	UGameUserSettings* UserSettings = GEngine->GetGameUserSettings();
	if (!UserSettings)
	{
		return;
	}

	EWindowMode::Type NextMode = EWindowMode::WindowedFullscreen;
	switch (UserSettings->GetFullscreenMode())
	{
	case EWindowMode::Fullscreen:
		NextMode = EWindowMode::Windowed;
		break;
	case EWindowMode::Windowed:
		NextMode = EWindowMode::WindowedFullscreen;
		break;
	case EWindowMode::WindowedFullscreen:
	default:
		NextMode = EWindowMode::Fullscreen;
		break;
	}

	UserSettings->SetFullscreenMode(NextMode);
	ApplyAndSaveUserSettings();
}

void ATP_ThirdPersonPlayerController::CycleQualityLevel()
{
	if (!GEngine)
	{
		return;
	}

	UGameUserSettings* UserSettings = GEngine->GetGameUserSettings();
	if (!UserSettings)
	{
		return;
	}

	int32 CurrentLevel = UserSettings->GetOverallScalabilityLevel();
	if (CurrentLevel < 0)
	{
		CurrentLevel = 3;
	}

	const int32 NextLevel = (CurrentLevel + 1) % 4;
	UserSettings->SetOverallScalabilityLevel(NextLevel);
	GrassDensityQuality = FMath::Clamp(NextLevel, 0, 3);
	FoliageDistanceQuality = FMath::Clamp(NextLevel, 0, 3);
	ApplyAndSaveUserSettings();
}

void ATP_ThirdPersonPlayerController::CycleShadowQuality()
{
	if (!GEngine)
	{
		return;
	}

	if (UGameUserSettings* UserSettings = GEngine->GetGameUserSettings())
	{
		UserSettings->SetShadowQuality((UserSettings->GetShadowQuality() + 1) % 5);
		ApplyAndSaveUserSettings();
	}
}

void ATP_ThirdPersonPlayerController::CycleAntiAliasingQuality()
{
	if (!GEngine)
	{
		return;
	}

	if (UGameUserSettings* UserSettings = GEngine->GetGameUserSettings())
	{
		UserSettings->SetAntiAliasingQuality((UserSettings->GetAntiAliasingQuality() + 1) % 5);
		ApplyAndSaveUserSettings();
	}
}

void ATP_ThirdPersonPlayerController::CyclePostProcessQuality()
{
	if (!GEngine)
	{
		return;
	}

	if (UGameUserSettings* UserSettings = GEngine->GetGameUserSettings())
	{
		UserSettings->SetPostProcessingQuality((UserSettings->GetPostProcessingQuality() + 1) % 5);
		ApplyAndSaveUserSettings();
	}
}

void ATP_ThirdPersonPlayerController::CycleViewDistanceQuality()
{
	if (!GEngine)
	{
		return;
	}

	if (UGameUserSettings* UserSettings = GEngine->GetGameUserSettings())
	{
		UserSettings->SetViewDistanceQuality((UserSettings->GetViewDistanceQuality() + 1) % 5);
		ApplyAndSaveUserSettings();
	}
}

void ATP_ThirdPersonPlayerController::CycleGrassDensityQuality()
{
	GrassDensityQuality = (FMath::Clamp(GrassDensityQuality, 0, 3) + 1) % 4;
	ApplyAndSaveUserSettings();
}

void ATP_ThirdPersonPlayerController::CycleFoliageDistanceQuality()
{
	FoliageDistanceQuality = (FMath::Clamp(FoliageDistanceQuality, 0, 3) + 1) % 4;
	ApplyAndSaveUserSettings();
}

void ATP_ThirdPersonPlayerController::HandleMasterVolumeChanged(float NewValue)
{
	SetMasterVolumeValue(NewValue, true);
}

FText ATP_ThirdPersonPlayerController::GetWindowModeText() const
{
	const UGameUserSettings* UserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!UserSettings)
	{
		return FText::FromString(TEXT("Unbekannt"));
	}

	switch (UserSettings->GetFullscreenMode())
	{
	case EWindowMode::Fullscreen:
		return FText::FromString(TEXT("Fullscreen"));
	case EWindowMode::Windowed:
		return FText::FromString(TEXT("Windowed"));
	case EWindowMode::WindowedFullscreen:
	default:
		return FText::FromString(TEXT("Borderless"));
	}
}

FText ATP_ThirdPersonPlayerController::GetQualityLevelText() const
{
	const UGameUserSettings* UserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!UserSettings)
	{
		return FText::FromString(TEXT("Unbekannt"));
	}

	switch (UserSettings->GetOverallScalabilityLevel())
	{
	case 0:
		return FText::FromString(TEXT("Low"));
	case 1:
		return FText::FromString(TEXT("Medium"));
	case 2:
		return FText::FromString(TEXT("High"));
	case 3:
		return FText::FromString(TEXT("Epic"));
	case 4:
		return FText::FromString(TEXT("Cinematic"));
	default:
		return FText::FromString(TEXT("Custom"));
	}
}

FText ATP_ThirdPersonPlayerController::GetShadowQualityText() const
{
	const UGameUserSettings* UserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	return FText::FromString(UserSettings ? FString::FromInt(UserSettings->GetShadowQuality()) : TEXT("0"));
}

FText ATP_ThirdPersonPlayerController::GetAntiAliasingQualityText() const
{
	const UGameUserSettings* UserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	return FText::FromString(UserSettings ? FString::FromInt(UserSettings->GetAntiAliasingQuality()) : TEXT("0"));
}

FText ATP_ThirdPersonPlayerController::GetPostProcessQualityText() const
{
	const UGameUserSettings* UserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	return FText::FromString(UserSettings ? FString::FromInt(UserSettings->GetPostProcessingQuality()) : TEXT("0"));
}

FText ATP_ThirdPersonPlayerController::GetViewDistanceQualityText() const
{
	const UGameUserSettings* UserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	return FText::FromString(UserSettings ? FString::FromInt(UserSettings->GetViewDistanceQuality()) : TEXT("0"));
}

FText ATP_ThirdPersonPlayerController::GetGrassDensityQualityText() const
{
	static const TCHAR* Labels[] = { TEXT("Low"), TEXT("Medium"), TEXT("High"), TEXT("Epic") };
	const int32 Index = FMath::Clamp(GrassDensityQuality, 0, 3);
	return FText::FromString(Labels[Index]);
}

FText ATP_ThirdPersonPlayerController::GetFoliageDistanceQualityText() const
{
	static const TCHAR* Labels[] = { TEXT("Low"), TEXT("Medium"), TEXT("High"), TEXT("Epic") };
	const int32 Index = FMath::Clamp(FoliageDistanceQuality, 0, 3);
	return FText::FromString(Labels[Index]);
}

FText ATP_ThirdPersonPlayerController::GetMasterVolumeText() const
{
	return FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(GetMasterVolumeNormalized() * 100.0f)));
}

float ATP_ThirdPersonPlayerController::GetMasterVolumeNormalized() const
{
	return FMath::Clamp(MasterVolume, 0.0f, 1.0f);
}

void ATP_ThirdPersonPlayerController::LoadPersistentSettings()
{
	if (UFantasyFrontierUserSettingsSaveGame* SavedSettings = Cast<UFantasyFrontierUserSettingsSaveGame>(
		UGameplayStatics::LoadGameFromSlot(UserSettingsSlotName, UserSettingsSlotIndex)))
	{
		MasterVolume = FMath::Clamp(SavedSettings->MasterVolume, 0.0f, 1.0f);
		GrassDensityQuality = FMath::Clamp(SavedSettings->GrassDensityQuality, 0, 3);
		FoliageDistanceQuality = FMath::Clamp(SavedSettings->FoliageDistanceQuality, 0, 3);
		if (GEngine)
		{
			if (UGameUserSettings* UserSettings = GEngine->GetGameUserSettings())
			{
				UserSettings->SetOverallScalabilityLevel(FMath::Clamp(SavedSettings->GraphicsQuality, 0, 3));
				UserSettings->SetShadowQuality(FMath::Clamp(SavedSettings->ShadowQuality, 0, 3));
				UserSettings->SetAntiAliasingQuality(FMath::Clamp(SavedSettings->AntiAliasingQuality, 0, 3));
				UserSettings->SetPostProcessingQuality(FMath::Clamp(SavedSettings->PostProcessQuality, 0, 3));
				UserSettings->SetViewDistanceQuality(FMath::Clamp(SavedSettings->ViewDistanceQuality, 0, 3));
			}
		}
		UE_LOG(LogTP_ThirdPerson, Display, TEXT("FFSettings LoadedPersistent masterVolume=%.2f grass=%d foliageDistance=%d"),
			MasterVolume,
			GrassDensityQuality,
			FoliageDistanceQuality);
		return;
	}

	MasterVolume = GetDefaultMasterVolume();
	GrassDensityQuality = 2;
	FoliageDistanceQuality = 2;
	UE_LOG(LogTP_ThirdPerson, Display, TEXT("FFSettings LoadedPersistent masterVolume=%.2f grass=%d foliageDistance=%d source=default"),
		MasterVolume,
		GrassDensityQuality,
		FoliageDistanceQuality);
}

void ATP_ThirdPersonPlayerController::SavePersistentSettings() const
{
	UFantasyFrontierUserSettingsSaveGame* SavedSettings = Cast<UFantasyFrontierUserSettingsSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UFantasyFrontierUserSettingsSaveGame::StaticClass()));
	if (!SavedSettings)
	{
		UE_LOG(LogTP_ThirdPerson, Warning, TEXT("FFAudio SavePersistentSettings failed reason=CreateSaveGameObject"));
		return;
	}

	SavedSettings->MasterVolume = GetMasterVolumeNormalized();
	if (GEngine)
	{
		if (UGameUserSettings* UserSettings = GEngine->GetGameUserSettings())
		{
			SavedSettings->GraphicsQuality = FMath::Clamp(UserSettings->GetOverallScalabilityLevel(), 0, 3);
			SavedSettings->ShadowQuality = FMath::Clamp(UserSettings->GetShadowQuality(), 0, 3);
			SavedSettings->AntiAliasingQuality = FMath::Clamp(UserSettings->GetAntiAliasingQuality(), 0, 3);
			SavedSettings->PostProcessQuality = FMath::Clamp(UserSettings->GetPostProcessingQuality(), 0, 3);
			SavedSettings->ViewDistanceQuality = FMath::Clamp(UserSettings->GetViewDistanceQuality(), 0, 3);
		}
	}
	SavedSettings->GrassDensityQuality = FMath::Clamp(GrassDensityQuality, 0, 3);
	SavedSettings->FoliageDistanceQuality = FMath::Clamp(FoliageDistanceQuality, 0, 3);
	const bool bSaved = UGameplayStatics::SaveGameToSlot(SavedSettings, UserSettingsSlotName, UserSettingsSlotIndex);
	UE_LOG(LogTP_ThirdPerson, Display, TEXT("FFSettings SavePersistent volume=%.2f grass=%d foliageDistance=%d result=%s"),
		SavedSettings->MasterVolume,
		SavedSettings->GrassDensityQuality,
		SavedSettings->FoliageDistanceQuality,
		bSaved ? TEXT("true") : TEXT("false"));
}

void ATP_ThirdPersonPlayerController::SetMasterVolumeValue(float NewValue, bool bPersist)
{
	MasterVolume = FMath::Clamp(NewValue, 0.0f, 1.0f);

	if (bPersist)
	{
		SavePersistentSettings();
	}

	ApplyMasterVolume();
	UE_LOG(LogTP_ThirdPerson, Display, TEXT("FFAudio MasterVolume=%.2f persist=%s"), MasterVolume, bPersist ? TEXT("true") : TEXT("false"));
}

void ATP_ThirdPersonPlayerController::ApplyMasterVolume()
{
	MasterVolume = GetMasterVolumeNormalized();
	const float AppliedMasterVolume = GetAppliedMasterVolume(MasterVolume);

	if (GetWorld())
	{
		if (FAudioDeviceHandle AudioDevice = GetWorld()->GetAudioDevice())
		{
			AudioDevice->SetTransientPrimaryVolume(AppliedMasterVolume);
		}
	}

	if (FrontEndMusicComponent && CurrentMusicState != EFantasyFrontierMusicState::None)
	{
		FrontEndMusicComponent->SetVolumeMultiplier(GetEffectiveMusicVolume(CurrentMusicState, MasterVolume));
	}

	UE_LOG(LogTP_ThirdPerson, Display, TEXT("FFAudio AppliedMasterVolume=%.3f stored=%.2f"), AppliedMasterVolume, MasterVolume);
}

void ATP_ThirdPersonPlayerController::ApplyHighlandPerformanceSettings() const
{
	const int32 GrassQuality = FMath::Clamp(GrassDensityQuality, 0, 3);
	const int32 AppliedFoliageDistanceQuality = FMath::Clamp(FoliageDistanceQuality, 0, 3);
	const float GrassDensityScales[] = { 0.28f, 0.48f, 0.68f, 0.86f };
	const float FoliageCullScales[] = { 0.45f, 0.65f, 0.85f, 1.05f };
	const float ShadowDistanceScales[] = { 0.45f, 0.60f, 0.78f, 0.95f };

	if (GEngine)
	{
		if (UGameUserSettings* UserSettings = GEngine->GetGameUserSettings())
		{
			UserSettings->ApplySettings(false);
			UserSettings->SaveSettings();
		}
	}

	ATP_ThirdPersonPlayerController* MutableThis = const_cast<ATP_ThirdPersonPlayerController*>(this);
	MutableThis->ConsoleCommand(*FString::Printf(TEXT("grass.DensityScale %.2f"), GrassDensityScales[GrassQuality]));
	MutableThis->ConsoleCommand(*FString::Printf(TEXT("foliage.DensityScale %.2f"), GrassDensityScales[GrassQuality]));
	MutableThis->ConsoleCommand(*FString::Printf(TEXT("foliage.CullDistanceScale %.2f"), FoliageCullScales[AppliedFoliageDistanceQuality]));
	MutableThis->ConsoleCommand(*FString::Printf(TEXT("r.Shadow.DistanceScale %.2f"), ShadowDistanceScales[AppliedFoliageDistanceQuality]));
	MutableThis->ConsoleCommand(TEXT("r.MotionBlurQuality 0"));
	MutableThis->ConsoleCommand(TEXT("r.ContactShadows 0"));

	if (IsFFStarterHighlandBlockoutWorld(GetWorld()))
	{
		MutableThis->ConsoleCommand(TEXT("r.HZBOcclusion 0"));
		MutableThis->ConsoleCommand(TEXT("r.SceneCulling.Async.Query 0"));
		MutableThis->ConsoleCommand(TEXT("r.SceneCulling.Async.Update 0"));
		MutableThis->ConsoleCommand(TEXT("r.Shadow.Virtual.Enable 0"));
	}

	UE_LOG(LogTP_ThirdPerson, Display, TEXT("FFPerfSettings applied grassQuality=%d grassScale=%.2f foliageDistance=%d foliageCullScale=%.2f"),
		GrassQuality,
		GrassDensityScales[GrassQuality],
		AppliedFoliageDistanceQuality,
		FoliageCullScales[AppliedFoliageDistanceQuality]);
}

void ATP_ThirdPersonPlayerController::AdvanceSmokeTest()
{
	if (!bRunSmokeTest || bSmokeTestComplete)
	{
		return;
	}

	if (IsFFStarterHighlandBlockoutWorld(GetWorld()) && FParse::Param(FCommandLine::Get(), TEXT("FFSmokeHighlandCaptureOnly")))
	{
		if (SmokeStepIndex++ == 0)
		{
			FString HighlandSmokeCameraTags;
			if (!FParse::Value(FCommandLine::Get(), TEXT("FFSmokeCameraTags="), HighlandSmokeCameraTags) || HighlandSmokeCameraTags.IsEmpty())
			{
				FString HighlandSmokeCameraTag;
				if (FParse::Value(FCommandLine::Get(), TEXT("FFSmokeCameraTag="), HighlandSmokeCameraTag) && !HighlandSmokeCameraTag.IsEmpty())
				{
					HighlandSmokeCameraTags = HighlandSmokeCameraTag;
				}
			}

			if (HighlandSmokeCameraTags.IsEmpty())
			{
				FinishSmokeTest(false, TEXT("Highland capture-only smoke requested without FFSmokeCameraTag(s)."));
				return;
			}

			BeginSmokeCameraSequence(HighlandSmokeCameraTags);
			return;
		}

		FinishSmokeTest(true);
		return;
	}

	if (IsTitanMainGrasslandHostSandboxWorld(GetWorld()))
	{
		if (SmokeStepIndex++ == 0)
		{
			FString SandboxCameraTag = TEXT("FFSmokeTitanHostSunCamera");
			FParse::Value(FCommandLine::Get(), TEXT("FFSmokeCameraTag="), SandboxCameraTag);
			bool bCameraReady = FocusSmokeCameraOnTaggedActor(FName(*SandboxCameraTag));
			if (!bCameraReady && SandboxCameraTag == TEXT("FFSmokeTitanRuntimeGrassCloseCamera"))
			{
				FVector GrassLocation = FVector::ZeroVector;
				if (FindSandboxFirstPredictedGrassWorldLocation(GetWorld(), GrassLocation))
				{
					const FVector CameraLocation = GrassLocation + FVector(-260.0f, -220.0f, 145.0f);
					const FVector LookAtLocation = GrassLocation + FVector(0.0f, 0.0f, 35.0f);
					ACameraActor* RuntimeGrassCamera = GetWorld()->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), CameraLocation, (LookAtLocation - CameraLocation).Rotation());
					if (RuntimeGrassCamera)
					{
						RuntimeGrassCamera->GetCameraComponent()->FieldOfView = 32.0f;
						SetViewTarget(RuntimeGrassCamera);
						bCameraReady = true;
						UE_LOG(LogTP_ThirdPerson, Display,
							TEXT("FFSandboxGrass Runtime close camera target=%s camera=%s"),
							*GrassLocation.ToString(),
							*CameraLocation.ToString());
					}
				}
			}
			LogSmokeTestStep(TEXT("TitanMain Host Sandbox Camera"), bCameraReady, SandboxCameraTag);
			if (!bCameraReady)
			{
				FinishSmokeTest(false, FString::Printf(TEXT("Requested Titan host sandbox smoke camera was missing: %s"), *SandboxCameraTag));
				return;
			}

			LogSandboxNativeGrassState(GetWorld(), TEXT("BeforeForceUpdate"));
			LogSandboxGrassSurfaceProbe(GetWorld(), TEXT("BeforeForceUpdate"));
			const FVector CameraLocation = PlayerCameraManager ? PlayerCameraManager->GetCameraLocation() : FVector::ZeroVector;
			const int32 CreatedGrassComponents = ForceSandboxNativeLandscapeGrass(GetWorld(), CameraLocation);
			ApplySandboxGrassMaterialToggle(GetWorld());
			LogSandboxNativeGrassState(GetWorld(), TEXT("AfterForceUpdate"));
			LogSandboxGrassSurfaceProbe(GetWorld(), TEXT("AfterForceUpdate"));
			UE_LOG(LogTP_ThirdPerson, Display,
				TEXT("FFSandboxGrass Runtime force-update completed camera=%s createdComponents=%d"),
				*CameraLocation.ToString(),
				CreatedGrassComponents);

			FString SandboxShotLabel = TEXT("TitanHost_Sandbox");
			FParse::Value(FCommandLine::Get(), TEXT("FFSmokeSandboxShotLabel="), SandboxShotLabel);
			float SandboxShotDelay = 0.75f;
			FParse::Value(FCommandLine::Get(), TEXT("FFSmokeSandboxShotDelay="), SandboxShotDelay);
			SandboxShotDelay = FMath::Clamp(SandboxShotDelay, 0.25f, 5.0f);
			QueueSmokeScreenshotCapture(SandboxShotLabel, SandboxShotDelay);
			GetWorldTimerManager().SetTimer(SmokeStepTimer, this, &ThisClass::AdvanceSmokeTest, SandboxShotDelay + 0.5f, false);
			return;
		}

		LogSmokeTestStep(TEXT("TitanMain Host Sandbox Runtime"), true, TEXT("Direct visual sandbox smoke completed."));
		FinishSmokeTest(true);
		return;
	}

	if (IsHighlandNativeMiniProofSmokeWorld(GetWorld()))
	{
		if (SmokeStepIndex++ == 0)
		{
			FString CameraTag = TEXT("FFSmokeHighlandNativeMiniSunCamera");
			FParse::Value(FCommandLine::Get(), TEXT("FFSmokeCameraTag="), CameraTag);
			const bool bCameraReady = FocusSmokeCameraOnTaggedActor(FName(*CameraTag));
			LogSmokeTestStep(TEXT("Highland Native Mini Proof Camera"), bCameraReady, CameraTag);
			if (!bCameraReady)
			{
				FinishSmokeTest(false, FString::Printf(TEXT("Requested Highland native mini proof smoke camera was missing: %s"), *CameraTag));
				return;
			}

			LogSandboxNativeGrassState(GetWorld(), TEXT("HighlandMiniBeforeForceUpdate"));
			LogSandboxGrassSurfaceProbe(GetWorld(), TEXT("HighlandMiniBeforeForceUpdate"));
			const FVector CameraLocation = PlayerCameraManager ? PlayerCameraManager->GetCameraLocation() : FVector::ZeroVector;
			const int32 CreatedGrassComponents = ForceSandboxNativeLandscapeGrass(GetWorld(), CameraLocation);
			LogSandboxNativeGrassState(GetWorld(), TEXT("HighlandMiniAfterForceUpdate"));
			LogSandboxGrassSurfaceProbe(GetWorld(), TEXT("HighlandMiniAfterForceUpdate"));
			UE_LOG(LogTP_ThirdPerson, Display,
				TEXT("FFHighlandNativeMiniProof Runtime force-update completed camera=%s createdComponents=%d"),
				*CameraLocation.ToString(),
				CreatedGrassComponents);

			FString ShotLabel = TEXT("HighlandNativeMiniProof");
			FParse::Value(FCommandLine::Get(), TEXT("FFSmokeSandboxShotLabel="), ShotLabel);
			float ShotDelay = 0.75f;
			FParse::Value(FCommandLine::Get(), TEXT("FFSmokeSandboxShotDelay="), ShotDelay);
			ShotDelay = FMath::Clamp(ShotDelay, 0.25f, 5.0f);
			QueueSmokeScreenshotCapture(ShotLabel, ShotDelay);
			GetWorldTimerManager().SetTimer(SmokeStepTimer, this, &ThisClass::AdvanceSmokeTest, ShotDelay + 0.5f, false);
			return;
		}

		LogSmokeTestStep(TEXT("Highland Native Mini Proof Runtime"), true, TEXT("Direct isolated Highland native grass proof smoke completed."));
		FinishSmokeTest(true);
		return;
	}

	switch (SmokeStepIndex++)
	{
	case 0:
	{
		const bool bFrontEndReady = bFrontEndVisible && FrontEndWidget.IsValid();
		LogSmokeTestStep(TEXT("Title Screen -> Start"), bFrontEndReady, TEXT("Title screen must appear before the automated flow starts."));
		if (!bFrontEndReady)
		{
			FinishSmokeTest(false, TEXT("Title screen did not initialize."));
			return;
		}

		FString TitleMusicReason;
		const bool bTitleMusicActive = IsExpectedMusicPlaying(EFantasyFrontierMusicState::Title, &TitleMusicReason);
		LogSmokeTestStep(TEXT("Title Music Active"), bTitleMusicActive, TitleMusicReason);
		if (!bTitleMusicActive)
		{
			FinishSmokeTest(false, FString::Printf(TEXT("Title music inactive: %s"), *TitleMusicReason));
			return;
		}

		CaptureSmokeScreenshot(TEXT("TitleScreen"));
		GetWorldTimerManager().SetTimer(SmokeStepTimer, this, &ThisClass::AdvanceSmokeTest, 0.40f, false);
		return;
	}
	case 1:
	{
		StartGameFromFrontEnd();
		GetWorldTimerManager().SetTimer(SmokeStepTimer, this, &ThisClass::AdvanceSmokeTest, 0.45f, false);
		return;
	}
	case 2:
	{
		const bool bCreatorReady = bCharacterCreatorVisible && CharacterCreatorWidget.IsValid() && CharacterPreviewActor != nullptr;
		LogSmokeTestStep(TEXT("Open Character Creator"), bCreatorReady, TEXT("Creator should open with a live preview actor."));
		if (!bCreatorReady)
		{
			FinishSmokeTest(false, TEXT("Character creator failed to open."));
			return;
		}

		FString CreatorMusicReason;
		const bool bCreatorMusicActive = IsExpectedMusicPlaying(EFantasyFrontierMusicState::CharacterCreator, &CreatorMusicReason);
		LogSmokeTestStep(TEXT("Creator Music Active"), bCreatorMusicActive, CreatorMusicReason);
		if (!bCreatorMusicActive)
		{
			FinishSmokeTest(false, FString::Printf(TEXT("Character creator music inactive: %s"), *CreatorMusicReason));
			return;
		}

		FFantasyFrontierCharacterDraft Draft = CharacterCreatorWidget->GetDraft();
		Draft.CharacterName = TEXT("SmokeRunner");
		Draft.Gender = EFantasyFrontierGender::Female;
		CharacterCreatorWidget->SetDraft(Draft);
		CharacterCreatorWidget->SetCurrentStepForSmokeTest(EFantasyFrontierCreatorStep::Gender);
		LogSmokeTestStep(TEXT("Switch Female"), CharacterCreatorWidget->GetDraft().Gender == EFantasyFrontierGender::Female);
		CaptureSmokeScreenshot(TEXT("Creator_RaceGender"));
		GetWorldTimerManager().SetTimer(SmokeStepTimer, this, &ThisClass::AdvanceSmokeTest, 0.25f, false);
		return;
	}
	case 3:
	{
		if (!CharacterCreatorWidget.IsValid())
		{
			FinishSmokeTest(false, TEXT("Character creator was lost before gender swap."));
			return;
		}

		FFantasyFrontierCharacterDraft Draft = CharacterCreatorWidget->GetDraft();
		Draft.Gender = EFantasyFrontierGender::Male;
		CharacterCreatorWidget->SetDraft(Draft);
		LogSmokeTestStep(TEXT("Switch Male"), CharacterCreatorWidget->GetDraft().Gender == EFantasyFrontierGender::Male);
		GetWorldTimerManager().SetTimer(SmokeStepTimer, this, &ThisClass::AdvanceSmokeTest, 0.25f, false);
		return;
	}
	case 4:
	{
		if (!CharacterCreatorWidget.IsValid())
		{
			FinishSmokeTest(false, TEXT("Character creator was lost before preview-mode test."));
			return;
		}

		FFantasyFrontierCharacterDraft Draft = CharacterCreatorWidget->GetDraft();
		Draft.PreviewMode = EFantasyFrontierPreviewMode::BaseBody;
		CharacterCreatorWidget->SetDraft(Draft);
		Draft.PreviewMode = EFantasyFrontierPreviewMode::StarterGear;
		CharacterCreatorWidget->SetDraft(Draft);
		Draft.PreviewMode = EFantasyFrontierPreviewMode::OriginStyle;
		CharacterCreatorWidget->SetDraft(Draft);
		Draft.PreviewMode = EFantasyFrontierPreviewMode::BaseBody;
		CharacterCreatorWidget->SetDraft(Draft);

		const bool bPreviewModesStable = CharacterCreatorWidget->GetDraft().PreviewMode == EFantasyFrontierPreviewMode::BaseBody;
		LogSmokeTestStep(TEXT("Switch Preview Modes"), bPreviewModesStable);
		if (!bPreviewModesStable)
		{
			FinishSmokeTest(false, TEXT("Preview modes did not round-trip correctly."));
			return;
		}

		GetWorldTimerManager().SetTimer(SmokeStepTimer, this, &ThisClass::AdvanceSmokeTest, 0.25f, false);
		return;
	}
	case 5:
	{
		if (!CharacterCreatorWidget.IsValid())
		{
			FinishSmokeTest(false, TEXT("Character creator was lost before preset save/load."));
			return;
		}

		FFantasyFrontierCharacterDraft Draft = CharacterCreatorWidget->GetDraft();
		Draft.CharacterName = TEXT("SmokeRunner");
		Draft.Gender = EFantasyFrontierGender::Female;
		Draft.HairStyle = 1;
		Draft.FaceVariant = 2;
		CharacterCreatorWidget->SetDraft(Draft);
		SaveAppearancePreset();

		Draft.Gender = EFantasyFrontierGender::Male;
		Draft.HairStyle = 2;
		Draft.FaceVariant = 0;
		CharacterCreatorWidget->SetDraft(Draft);
		LoadAppearancePreset();
		CharacterCreatorWidget->SetCurrentStepForSmokeTest(EFantasyFrontierCreatorStep::Appearance);

		const FFantasyFrontierCharacterDraft LoadedDraft = CharacterCreatorWidget->GetDraft();
		const bool bPresetRoundTrip =
			LoadedDraft.CharacterName == TEXT("SmokeRunner") &&
			LoadedDraft.Gender == EFantasyFrontierGender::Female &&
			LoadedDraft.HairStyle == 1 &&
			LoadedDraft.FaceVariant == 2;

		LogSmokeTestStep(TEXT("Save and Load Preset"), bPresetRoundTrip);
		if (!bPresetRoundTrip)
		{
			FinishSmokeTest(false, TEXT("Appearance preset did not reload the saved draft."));
			return;
		}

		CaptureSmokeScreenshot(TEXT("Creator_Appearance"));
		GetWorldTimerManager().SetTimer(SmokeStepTimer, this, &ThisClass::AdvanceSmokeTest, 0.25f, false);
		return;
	}
	case 6:
	{
		if (!CharacterCreatorWidget.IsValid())
		{
			FinishSmokeTest(false, TEXT("Character creator was lost before game start."));
			return;
		}

		FFantasyFrontierCharacterDraft Draft = CharacterCreatorWidget->GetDraft();
		Draft.CharacterName = TEXT("SmokeRunner");
		Draft.CharacterClass = EFantasyFrontierClass::Lancer;
		CommitCharacterCreator(Draft);
		LogSmokeTestStep(TEXT("Start Game"), !bCharacterCreatorVisible && !bFrontEndVisible);
		GetWorldTimerManager().SetTimer(SmokeStepTimer, this, &ThisClass::AdvanceSmokeTest, 1.65f, false);
		return;
	}
	case 7:
	{
		AFantasyFrontierPlayableCharacter* PlayerCharacter = GetSmokePlayerCharacter();
		AFantasyFrontierTutorialDirector* TutorialDirector = GetSmokeTutorialDirector();
		const bool bSpawnedIntoTutorial = PlayerCharacter != nullptr && TutorialDirector != nullptr;
		LogSmokeTestStep(TEXT("Spawn into Tutorial"), bSpawnedIntoTutorial);
		if (!bSpawnedIntoTutorial)
		{
			FinishSmokeTest(false, TEXT("Player or tutorial director did not exist after creator confirm."));
			return;
		}

		FString OverworldMusicReason;
		const bool bOverworldMusicActive = IsExpectedMusicPlaying(EFantasyFrontierMusicState::Overworld, &OverworldMusicReason);
		LogSmokeTestStep(TEXT("World Music Transition"), bOverworldMusicActive, OverworldMusicReason);
		if (!bOverworldMusicActive)
		{
			FinishSmokeTest(false, FString::Printf(TEXT("Overworld music did not start after leaving the creator: %s"), *OverworldMusicReason));
			return;
		}

		FrameSmokeWorldView(PlayerCharacter->GetActorLocation(), GetStarterForestPreferredViewRotation(GetWorld()));
		QueueSmokeScreenshotCapture(TEXT("Starter_Forest_View"));
		GetWorldTimerManager().SetTimer(SmokeStepTimer, this, &ThisClass::AdvanceSmokeTest, 0.85f, false);
		return;
	}
	case 8:
	{
		AFantasyFrontierPlayableCharacter* PlayerCharacter = GetSmokePlayerCharacter();
		if (!PlayerCharacter)
		{
			FinishSmokeTest(false, TEXT("Player disappeared before spawn screenshot."));
			return;
		}

		FrameSmokeWorldView(PlayerCharacter->GetActorLocation(), GetStarterForestPreferredViewRotation(GetWorld()));
		QueueSmokeScreenshotCapture(TEXT("Ingame_Spawn"));
		ApplyMenuInputState(false, nullptr);
		SetLivePawnMenuHold(false);
		SmokeMovementStart = PlayerCharacter->GetActorLocation();
		float SmokeStationarySeconds = 0.0f;
		float SmokeMovementSeconds = 1.2f;
		FParse::Value(FCommandLine::Get(), TEXT("FFSmokeStationarySeconds="), SmokeStationarySeconds);
		FParse::Value(FCommandLine::Get(), TEXT("FFSmokeMovementSeconds="), SmokeMovementSeconds);
		SmokeStationarySeconds = FMath::Clamp(SmokeStationarySeconds, 0.0f, 30.0f);
		SmokeMovementSeconds = FMath::Clamp(SmokeMovementSeconds, 1.2f, 60.0f);
		constexpr float SmokeMovementPulseSeconds = 0.05f;
		SmokeMoveTicksRemaining = FMath::CeilToInt(SmokeMovementSeconds / SmokeMovementPulseSeconds);
		GetWorldTimerManager().SetTimer(
			SmokeMoveTimer,
			this,
			&ThisClass::RunSmokeMovementPulse,
			SmokeMovementPulseSeconds,
			true,
			SmokeStationarySeconds);
		GetWorldTimerManager().SetTimer(
			SmokeStepTimer,
			this,
			&ThisClass::AdvanceSmokeTest,
			SmokeStationarySeconds + SmokeMovementSeconds + 0.60f,
			false);
		UE_LOG(
			LogTP_ThirdPerson,
			Display,
			TEXT("FFSmoke PerformanceWindow stationary=%.1fs movement=%.1fs"),
			SmokeStationarySeconds,
			SmokeMovementSeconds);
		return;
	}
	case 9:
	{
		GetWorldTimerManager().ClearTimer(SmokeMoveTimer);

		AFantasyFrontierPlayableCharacter* PlayerCharacter = GetSmokePlayerCharacter();
		if (!PlayerCharacter)
		{
			FinishSmokeTest(false, TEXT("Player disappeared before movement check."));
			return;
		}

		UE_LOG(LogTP_ThirdPerson, Display, TEXT("FFSmoke MoveResult start=(%.1f, %.1f, %.1f) end=(%.1f, %.1f, %.1f) velocity=(%.1f, %.1f, %.1f)"),
			SmokeMovementStart.X,
			SmokeMovementStart.Y,
			SmokeMovementStart.Z,
			PlayerCharacter->GetActorLocation().X,
			PlayerCharacter->GetActorLocation().Y,
			PlayerCharacter->GetActorLocation().Z,
			PlayerCharacter->GetVelocity().X,
			PlayerCharacter->GetVelocity().Y,
			PlayerCharacter->GetVelocity().Z);

		const float RequiredMoveDistance = UsesKashkehStarterBaseWorld(GetWorld()) ? 40.0f : 55.0f;
		const bool bMoved = FVector::Dist2D(SmokeMovementStart, PlayerCharacter->GetActorLocation()) > RequiredMoveDistance;
		LogSmokeTestStep(TEXT("Move Character"), bMoved);
		if (!bMoved)
		{
			FinishSmokeTest(false, TEXT("Character failed to move during smoke test."));
			return;
		}

		SmokeActionStaminaBefore = PlayerCharacter->GetCurrentStamina();
		PlayerCharacter->TriggerDashForSmokeTest();
		PlayerCharacter->TriggerLightAttackForSmokeTest();
		PlayerCharacter->TriggerHeavyAttackForSmokeTest();
		GetWorldTimerManager().SetTimer(SmokeStepTimer, this, &ThisClass::AdvanceSmokeTest, 0.75f, false);
		return;
	}
	case 10:
	{
		AFantasyFrontierPlayableCharacter* PlayerCharacter = GetSmokePlayerCharacter();
		if (!PlayerCharacter)
		{
			FinishSmokeTest(false, TEXT("Player disappeared before action check."));
			return;
		}

		const bool bActionsConsumedStamina = PlayerCharacter->GetCurrentStamina() < SmokeActionStaminaBefore;
		LogSmokeTestStep(TEXT("Dash + Light/Heavy Attack"), bActionsConsumedStamina);
		if (!bActionsConsumedStamina)
		{
			FinishSmokeTest(false, TEXT("Dash/light/heavy actions did not execute."));
			return;
		}

		if (IsFFStarterHighlandBlockoutWorld(GetWorld()))
		{
			const bool bHillGroundingPassed = ValidateHighlandHillGrounding(PlayerCharacter);
			LogSmokeTestStep(TEXT("Highland Hill Grounding"), bHillGroundingPassed);
			if (!bHillGroundingPassed)
			{
				FinishSmokeTest(false, TEXT("Highland hill grounding validation failed."));
				return;
			}

			FrameSmokeWorldView(PlayerCharacter->GetActorLocation(), GetStarterForestPreferredViewRotation(GetWorld()));
			QueueSmokeScreenshotCapture(TEXT("Hill_Grounding"));
			GetWorldTimerManager().SetTimer(SmokeStepTimer, this, &ThisClass::AdvanceSmokeTest, 0.35f, false);
			return;
		}

		if (IsLikanaValleyStarterForestWorld(GetWorld()) || IsShoreLakeStarterForestWorld(GetWorld()) || IsGrasslandVillageStarterForestWorld(GetWorld()) || IsGrasslandMolehillStarterForestWorld(GetWorld()) || IsGrasslandFarmStarterForestWorld(GetWorld()) || UsesKashkehStarterBaseWorld(GetWorld()))
		{
			LogSmokeTestStep(TEXT("Grassland Base Region Validation"), true, TEXT("Base-region smoke stops after spawn, movement, and action checks."));
			FinishSmokeTest(true);
			return;
		}

		if (AFantasyFrontierFunctionalNpc* GuideNpc = FindNpcByRole(static_cast<uint8>(EFantasyFrontierNpcRole::Guide)))
		{
			PlayerCharacter->SetActorLocation(GuideNpc->GetActorLocation() + FVector(0.0f, 0.0f, 25.0f));
		}
		else
		{
			FinishSmokeTest(false, TEXT("Guide NPC was missing from the tutorial slice."));
			return;
		}

		GetWorldTimerManager().SetTimer(SmokeStepTimer, this, &ThisClass::AdvanceSmokeTest, 0.50f, false);
		return;
	}
	case 11:
	{
	if (IsFFStarterHighlandBlockoutWorld(GetWorld()))
	{
		FString HighlandSmokeCameraTags;
		if (FParse::Value(FCommandLine::Get(), TEXT("FFSmokeCameraTags="), HighlandSmokeCameraTags) && !HighlandSmokeCameraTags.IsEmpty())
		{
			BeginSmokeCameraSequence(HighlandSmokeCameraTags);
			return;
		}

		FString HighlandSmokeCameraTag = TEXT("FFSmokeTopDownCamera");
		FParse::Value(FCommandLine::Get(), TEXT("FFSmokeCameraTag="), HighlandSmokeCameraTag);
		const bool bTopDownReady = FocusSmokeCameraOnTaggedActor(FName(*HighlandSmokeCameraTag));
		LogSmokeTestStep(TEXT("Highland Top-Down Capture"), bTopDownReady);
		if (!bTopDownReady)
		{
			FinishSmokeTest(false, FString::Printf(TEXT("Requested Highland smoke camera was missing from FF_Starter_Highland_Blockout: %s"), *HighlandSmokeCameraTag));
			return;
		}

			QueueSmokeScreenshotCapture(TEXT("Top_Down"));
			GetWorldTimerManager().SetTimer(SmokeStepTimer, this, &ThisClass::AdvanceSmokeTest, 0.35f, false);
			return;
		}

		AFantasyFrontierPlayableCharacter* PlayerCharacter = GetSmokePlayerCharacter();
		AFantasyFrontierTutorialDirector* TutorialDirector = GetSmokeTutorialDirector();
		const bool bNpcInteractionWorked = TutorialDirector != nullptr && TutorialDirector->HasGuideVisited();
		LogSmokeTestStep(TEXT("Interact With NPC"), bNpcInteractionWorked);
		if (!bNpcInteractionWorked || !PlayerCharacter)
		{
			FinishSmokeTest(false, TEXT("NPC overlap did not register with the tutorial director."));
			return;
		}

		SmokeTrackedEnemy = FindFirstEnemy();
		if (!SmokeTrackedEnemy.IsValid())
		{
			FinishSmokeTest(false, TEXT("No tutorial enemy was available for the combat check."));
			return;
		}

		const FVector EnemyLocation = SmokeTrackedEnemy->GetActorLocation();
		const FVector PlayerToEnemy = (EnemyLocation - PlayerCharacter->GetActorLocation()).GetSafeNormal2D();
		const FVector ViewForward = PlayerToEnemy.IsNearlyZero() ? FVector(1.0f, 0.0f, 0.0f) : PlayerToEnemy;
		const FVector ViewRight = FVector::CrossProduct(FVector::UpVector, ViewForward).GetSafeNormal();
		const float EncounterHeightOffset = PlayerCharacter->GetCapsuleComponent()
			? PlayerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 2.0f
			: 96.0f;
		FVector EncounterViewLocation = EnemyLocation - (ViewForward * 340.0f) + (ViewRight * 140.0f);
		EncounterViewLocation = TutorialDirector->ResolveTutorialGroundLocation(EncounterViewLocation, EncounterHeightOffset);
		PlayerCharacter->SetActorLocation(EncounterViewLocation);
		if (UCharacterMovementComponent* MovementComponent = PlayerCharacter->GetCharacterMovement())
		{
			MovementComponent->StopMovementImmediately();
			MovementComponent->SetMovementMode(MOVE_Walking);
		}
		const FRotator EnemyFacing = (EnemyLocation - EncounterViewLocation).Rotation();
		PlayerCharacter->SetActorRotation(FRotator(0.0f, EnemyFacing.Yaw, 0.0f));
		SetControlRotation(FRotator(-8.0f, EnemyFacing.Yaw, 0.0f));
		FrameSmokeWorldView(EncounterViewLocation, FRotator(-10.0f, EnemyFacing.Yaw, 0.0f));
		CaptureSmokeScreenshot(TEXT("Enemy_Encounter"));
		PlayerCharacter->TriggerLightAttackForSmokeTest();
		GetWorldTimerManager().SetTimer(SmokeStepTimer, this, &ThisClass::AdvanceSmokeTest, 1.10f, false);
		return;
	}
	case 12:
	{
		if (IsFFStarterHighlandBlockoutWorld(GetWorld()))
		{
			LogSmokeTestStep(TEXT("Highland Blockout Validation"), true, TEXT("Smoke stops after spawn, movement, action, hill-grounding, and top-down capture."));
			FinishSmokeTest(true);
			return;
		}

		const bool bCombatTriggered = SmokeTrackedEnemy.IsValid() && SmokeTrackedEnemy->HasEngagedTarget();
		LogSmokeTestStep(TEXT("Trigger Combat With Enemy"), bCombatTriggered);
		if (!bCombatTriggered)
		{
			FinishSmokeTest(false, TEXT("Enemy never entered combat state."));
			return;
		}

		FinishSmokeTest(true);
		return;
	}
	default:
		return;
	}
}

void ATP_ThirdPersonPlayerController::RunSmokeMovementPulse()
{
	AFantasyFrontierPlayableCharacter* PlayerCharacter = GetSmokePlayerCharacter();
	if (!PlayerCharacter)
	{
		GetWorldTimerManager().ClearTimer(SmokeMoveTimer);
		return;
	}

	ResetIgnoreMoveInput();
	ResetIgnoreLookInput();
	SetIgnoreMoveInput(false);
	SetIgnoreLookInput(false);
	PlayerCharacter->SetActorEnableCollision(true);
	if (UCapsuleComponent* CapsuleComponent = PlayerCharacter->GetCapsuleComponent())
	{
		CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	if (UCharacterMovementComponent* MovementComponent = PlayerCharacter->GetCharacterMovement())
	{
		MovementComponent->Activate(true);
		MovementComponent->SetComponentTickEnabled(true);
		MovementComponent->SetMovementMode(MOVE_Walking);
	}
	PlayerCharacter->TriggerMovementPulseForSmokeTest();
	--SmokeMoveTicksRemaining;
	if (SmokeMoveTicksRemaining <= 0)
	{
		GetWorldTimerManager().ClearTimer(SmokeMoveTimer);
	}
}

bool ATP_ThirdPersonPlayerController::ValidateHighlandHillGrounding(AFantasyFrontierPlayableCharacter* PlayerCharacter)
{
	if (!PlayerCharacter || !IsFFStarterHighlandBlockoutWorld(GetWorld()))
	{
		return false;
	}

	UCapsuleComponent* CapsuleComponent = PlayerCharacter->GetCapsuleComponent();
	UCharacterMovementComponent* MovementComponent = PlayerCharacter->GetCharacterMovement();
	if (!CapsuleComponent || !MovementComponent)
	{
		return false;
	}

	struct FHighlandGroundProbe
	{
		const TCHAR* Label;
		FVector2D Position;
	};

	const FHighlandGroundProbe Probes[] = {
		{ TEXT("SpawnMeadow"), FVector2D(38553.5f, 147844.6f) },
		{ TEXT("NorthWestSoftHill"), FVector2D(42000.0f, 132000.0f) },
		{ TEXT("CentralEastSoftHill"), FVector2D(78000.0f, 112000.0f) },
		{ TEXT("SouthWestRise"), FVector2D(32000.0f, 76000.0f) },
		{ TEXT("SouthEastMound"), FVector2D(89000.0f, 65000.0f) }
	};

	const FVector2D ProbeOffsets[] = {
		FVector2D::ZeroVector,
		FVector2D(1800.0f, 0.0f),
		FVector2D(-1800.0f, 0.0f),
		FVector2D(0.0f, 1800.0f),
		FVector2D(0.0f, -1800.0f),
		FVector2D(3000.0f, 1800.0f),
		FVector2D(-3000.0f, 1800.0f),
		FVector2D(3000.0f, -1800.0f),
		FVector2D(-3000.0f, -1800.0f),
		FVector2D(5200.0f, 0.0f),
		FVector2D(-5200.0f, 0.0f),
		FVector2D(0.0f, 5200.0f),
		FVector2D(0.0f, -5200.0f)
	};

	const float CapsuleHalfHeight = CapsuleComponent->GetScaledCapsuleHalfHeight();
	bool bAllGrounded = true;
	for (const FHighlandGroundProbe& Probe : Probes)
	{
		FHitResult BestLandscapeHit;
		FVector2D BestPosition = Probe.Position;
		float BestScore = TNumericLimits<float>::Max();
		bool bFoundWalkableLandscape = false;
		bool bAnyLandscapeHit = false;
		FHitResult FirstLandscapeHit;
		for (const FVector2D& ProbeOffset : ProbeOffsets)
		{
			const FVector2D CandidatePosition = Probe.Position + ProbeOffset;
			FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(FFHighlandRuntimeGroundProbe), false, PlayerCharacter);
			QueryParams.bTraceComplex = false;
			QueryParams.bReturnPhysicalMaterial = false;

			FHitResult CandidateHit;
			const bool bLandscapeHit = GetWorld()->LineTraceSingleByChannel(
				CandidateHit,
				FVector(CandidatePosition.X, CandidatePosition.Y, 42000.0f),
				FVector(CandidatePosition.X, CandidatePosition.Y, -42000.0f),
				ECC_WorldStatic,
				QueryParams);

			const bool bHitLandscape = bLandscapeHit && CandidateHit.GetActor() && CandidateHit.GetActor()->IsA<ALandscapeProxy>();
			if (!bHitLandscape)
			{
				continue;
			}

			if (!bAnyLandscapeHit)
			{
				FirstLandscapeHit = CandidateHit;
				bAnyLandscapeHit = true;
			}

			if (CandidateHit.ImpactNormal.Z < 0.72f)
			{
				continue;
			}

			const float DistanceScore = ProbeOffset.SizeSquared();
			if (!bFoundWalkableLandscape || DistanceScore < BestScore)
			{
				BestLandscapeHit = CandidateHit;
				BestPosition = CandidatePosition;
				BestScore = DistanceScore;
				bFoundWalkableLandscape = true;
			}
		}

		if (!bFoundWalkableLandscape)
		{
			UE_LOG(LogTP_ThirdPerson, Warning, TEXT("FFHighlandGroundingProbe FAIL label=%s hit=%s actor=%s normalZ=%.3f"),
				Probe.Label,
				bAnyLandscapeHit ? TEXT("true") : TEXT("false"),
				bAnyLandscapeHit && FirstLandscapeHit.GetActor() ? *FirstLandscapeHit.GetActor()->GetName() : TEXT("None"),
				bAnyLandscapeHit ? FirstLandscapeHit.ImpactNormal.Z : 0.0f);
			bAllGrounded = false;
			continue;
		}

		const FVector TestLocation = BestLandscapeHit.ImpactPoint + FVector(0.0f, 0.0f, CapsuleHalfHeight + 2.0f);
		PlayerCharacter->SetActorLocation(TestLocation, false, nullptr, ETeleportType::ResetPhysics);
		MovementComponent->StopMovementImmediately();
		MovementComponent->SetMovementMode(MOVE_Walking);

		FFindFloorResult FloorResult;
		MovementComponent->FindFloor(PlayerCharacter->GetActorLocation(), FloorResult, false);
		const bool bWalkableFloor = FloorResult.IsWalkableFloor() &&
			FloorResult.HitResult.GetActor() &&
			FloorResult.HitResult.GetActor()->IsA<ALandscapeProxy>() &&
			FloorResult.FloorDist <= 8.0f;

		UE_LOG(LogTP_ThirdPerson, Display, TEXT("FFHighlandGroundingProbe label=%s sample=(%.1f, %.1f) target=(%.1f, %.1f) location=(%.1f, %.1f, %.1f) groundZ=%.1f normalZ=%.3f floorDist=%.2f walkable=%s"),
			Probe.Label,
			BestPosition.X,
			BestPosition.Y,
			Probe.Position.X,
			Probe.Position.Y,
			PlayerCharacter->GetActorLocation().X,
			PlayerCharacter->GetActorLocation().Y,
			PlayerCharacter->GetActorLocation().Z,
			BestLandscapeHit.ImpactPoint.Z,
			BestLandscapeHit.ImpactNormal.Z,
			FloorResult.FloorDist,
			bWalkableFloor ? TEXT("true") : TEXT("false"));

		bAllGrounded = bAllGrounded && bWalkableFloor;
	}

	return bAllGrounded;
}

void ATP_ThirdPersonPlayerController::FinishSmokeTest(bool bSuccess, const FString& FailureReason)
{
	if (bSmokeTestComplete)
	{
		return;
	}

	bSmokeTestComplete = true;
	GetWorldTimerManager().ClearTimer(SmokeStepTimer);
	GetWorldTimerManager().ClearTimer(SmokeMoveTimer);

	if (bSuccess)
	{
		UE_LOG(LogTP_ThirdPerson, Display, TEXT("FFSmoke PASS"));
	}
	else
	{
		UE_LOG(LogTP_ThirdPerson, Error, TEXT("FFSmoke FAIL: %s"), FailureReason.IsEmpty() ? TEXT("Unknown failure.") : *FailureReason);
	}

	GetWorldTimerManager().SetTimer(SmokeExitTimer, this, &ThisClass::RequestSmokeTestExit, 0.75f, false);
}

void ATP_ThirdPersonPlayerController::RequestSmokeTestExit()
{
	ConsoleCommand(TEXT("quit"));
}

void ATP_ThirdPersonPlayerController::CaptureSmokeScreenshot(const FString& Label)
{
	if (!bRunSmokeTest || !bCaptureSmokeScreenshots)
	{
		return;
	}

	const FString CaptureDir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("FFSmokeCaptures"), SmokeScreenshotPrefix);
	IFileManager::Get().MakeDirectory(*CaptureDir, true);
	const FString Filename = FString::Printf(TEXT("%02d_%s.png"), SmokeScreenshotIndex++, *Label);
	const FString AbsolutePath = FPaths::Combine(CaptureDir, Filename);
	const bool bShowUI = !FParse::Param(FCommandLine::Get(), TEXT("FFSmokeNoUI"));
	FScreenshotRequest::RequestScreenshot(AbsolutePath, bShowUI, false, false);
}

void ATP_ThirdPersonPlayerController::FrameSmokeWorldView(const FVector& CharacterLocation, const FRotator& ViewRotation)
{
	AFantasyFrontierPlayableCharacter* PlayerCharacter = GetSmokePlayerCharacter();
	if (!PlayerCharacter)
	{
		return;
	}

	if (USpringArmComponent* CameraBoom = PlayerCharacter->GetCameraBoom())
	{
		if (IsFFStarterHighlandBlockoutWorld(GetWorld()))
		{
			ApplyStarterForestCameraPreset(PlayerCharacter);
			CameraBoom->TargetArmLength = 780.0f;
			CameraBoom->SocketOffset = FVector(0.0f, 88.0f, 162.0f);
			CameraBoom->bEnableCameraLag = false;
			CameraBoom->bEnableCameraRotationLag = false;
			CameraBoom->ProbeChannel = ECC_Camera;
			CameraBoom->ProbeSize = 24.0f;
			CameraBoom->bDoCollisionTest = true;
		}
		else if (IsStarterForestWorld(GetWorld()))
		{
			ApplyStarterForestCameraPreset(PlayerCharacter);
			if (bRunSmokeTest && UsesKashkehStarterBaseWorld(GetWorld()))
			{
				CameraBoom->TargetArmLength = 1500.0f;
				CameraBoom->SocketOffset = FVector(0.0f, 140.0f, 380.0f);
				CameraBoom->bEnableCameraLag = false;
				CameraBoom->bEnableCameraRotationLag = false;
				CameraBoom->bDoCollisionTest = false;
			}
		}
		else
		{
			CameraBoom->TargetArmLength = 520.0f;
			CameraBoom->SocketOffset = FVector(0.0f, 42.0f, 66.0f);
			CameraBoom->bEnableCameraLag = false;
			CameraBoom->bEnableCameraRotationLag = false;
		}
	}

	if (bRunSmokeTest && UsesKashkehStarterBaseWorld(GetWorld()))
	{
		if (UCameraComponent* FollowCamera = PlayerCharacter->GetFollowCamera())
		{
			FollowCamera->SetFieldOfView(58.0f);
		}
	}

	PlayerCharacter->SetActorLocation(CharacterLocation);
	PlayerCharacter->SetActorRotation(FRotator(0.0f, ViewRotation.Yaw, 0.0f));
	SetControlRotation(ViewRotation);
	SetInitialLocationAndRotation(CharacterLocation, ViewRotation);

	if (bRunSmokeTest)
	{
		const FVector PlayerLocation = PlayerCharacter->GetActorLocation();
		const FVector CameraLocation = PlayerCharacter->GetFollowCamera() ? PlayerCharacter->GetFollowCamera()->GetComponentLocation() : FVector::ZeroVector;
		const FRotator CameraRotation = PlayerCharacter->GetFollowCamera() ? PlayerCharacter->GetFollowCamera()->GetComponentRotation() : FRotator::ZeroRotator;
		UE_LOG(LogTP_ThirdPerson, Display,
			TEXT("FFSmoke ViewFrame player=(%.1f, %.1f, %.1f) velocity=(%.1f, %.1f, %.1f) camera=(%.1f, %.1f, %.1f) cameraRot=(P%.1f Y%.1f R%.1f)"),
			PlayerLocation.X, PlayerLocation.Y, PlayerLocation.Z,
			PlayerCharacter->GetVelocity().X, PlayerCharacter->GetVelocity().Y, PlayerCharacter->GetVelocity().Z,
			CameraLocation.X, CameraLocation.Y, CameraLocation.Z,
			CameraRotation.Pitch, CameraRotation.Yaw, CameraRotation.Roll);
	}
}

bool ATP_ThirdPersonPlayerController::FocusSmokeCameraOnTaggedActor(const FName& CameraTag)
{
	if (!GetWorld())
	{
		return false;
	}

	for (TActorIterator<ACameraActor> It(GetWorld()); It; ++It)
	{
		ACameraActor* CameraActor = *It;
		if (!IsValid(CameraActor) || !CameraActor->ActorHasTag(CameraTag))
		{
			continue;
		}

		SetViewTargetWithBlend(CameraActor, 0.0f);
		SetControlRotation(CameraActor->GetActorRotation());
		return true;
	}

	return false;
}

void ATP_ThirdPersonPlayerController::QueueSmokeScreenshotCapture(const FString& Label, float DelaySeconds)
{
	if (!bRunSmokeTest || !bCaptureSmokeScreenshots)
	{
		return;
	}

	PendingSmokeCaptureLabel = Label;
	GetWorldTimerManager().ClearTimer(SmokeCaptureTimer);
	GetWorldTimerManager().SetTimer(SmokeCaptureTimer, this, &ThisClass::ExecuteQueuedSmokeScreenshot, DelaySeconds, false);
}

void ATP_ThirdPersonPlayerController::ExecuteQueuedSmokeScreenshot()
{
	if (PendingSmokeCaptureLabel.IsEmpty())
	{
		return;
	}

	FString Label = PendingSmokeCaptureLabel;
	PendingSmokeCaptureLabel.Reset();
	if (bSmokeV842WindProof && Label.Contains(TEXT("V842_WindProof")))
	{
		if (SmokeV842WindProofFrameIndex++ == 0)
		{
			CaptureSmokeScreenshot(TEXT("V842_Wind_Warmup"));
			return;
		}
		const float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
		if (SmokeV842WindProofStartSeconds < 0.0f)
		{
			SmokeV842WindProofStartSeconds = CurrentTime;
		}
		const float ElapsedSeconds = FMath::Max(0.0f, CurrentTime - SmokeV842WindProofStartSeconds);
		const int32 ElapsedMilliseconds = FMath::RoundToInt(ElapsedSeconds * 1000.0f);
		Label = FString::Printf(TEXT("V842_Wind_T%04dms"), ElapsedMilliseconds);
		UE_LOG(
			LogTP_ThirdPerson,
			Display,
			TEXT("FFSmoke V84.2 wind capture actualTime=%.3fs label=%s"),
			ElapsedSeconds,
			*Label);
	}
	CaptureSmokeScreenshot(Label);
}

void ATP_ThirdPersonPlayerController::BeginSmokeCameraSequence(const FString& CameraTagList)
{
	PendingSmokeCameraTags.Reset();
	PendingSmokeCameraIndex = 0;
	bSmokeV842WindProof = FParse::Param(FCommandLine::Get(), TEXT("FFSmokeV842WindProof"));
	SmokeV842WindProofStartSeconds = -1.0f;
	SmokeV842WindProofFrameIndex = 0;

	FString NormalizedTags = CameraTagList;
	NormalizedTags.ReplaceInline(TEXT(";"), TEXT(","));
	NormalizedTags.ParseIntoArray(PendingSmokeCameraTags, TEXT(","), true);

	for (FString& CameraTag : PendingSmokeCameraTags)
	{
		CameraTag.TrimStartAndEndInline();
	}

	PendingSmokeCameraTags.RemoveAll([](const FString& CameraTag)
	{
		return CameraTag.IsEmpty();
	});
	if (bSmokeV842WindProof && PendingSmokeCameraTags.Num() > 0)
	{
		const FString WarmupCameraTag = PendingSmokeCameraTags[0];
		PendingSmokeCameraTags.Insert(WarmupCameraTag, 0);
	}

	const bool bHasCameraTags = PendingSmokeCameraTags.Num() > 0;
	LogSmokeTestStep(TEXT("Highland Multi-Camera Capture Setup"), bHasCameraTags, FString::Printf(TEXT("camera_count=%d"), PendingSmokeCameraTags.Num()));
	if (!bHasCameraTags)
	{
		FinishSmokeTest(false, TEXT("FFSmokeCameraTags was provided but no camera tags were parsed."));
		return;
	}

	CaptureNextSmokeCamera();
}

void ATP_ThirdPersonPlayerController::CaptureNextSmokeCamera()
{
	if (!GetWorld())
	{
		FinishSmokeTest(false, TEXT("World disappeared during Highland multi-camera capture."));
		return;
	}

	if (PendingSmokeCameraIndex >= PendingSmokeCameraTags.Num())
	{
		LogSmokeTestStep(TEXT("Highland Multi-Camera Capture"), true, FString::Printf(TEXT("captured=%d"), PendingSmokeCameraTags.Num()));
		GetWorldTimerManager().SetTimer(SmokeStepTimer, this, &ThisClass::AdvanceSmokeTest, 0.35f, false);
		return;
	}

	const int32 CameraNumber = PendingSmokeCameraIndex + 1;
	const FString CameraTag = PendingSmokeCameraTags[PendingSmokeCameraIndex];
	const bool bCameraReady = FocusSmokeCameraOnTaggedActor(FName(*CameraTag));
	LogSmokeTestStep(TEXT("Highland Multi-Camera Focus"), bCameraReady, CameraTag);
	if (!bCameraReady)
	{
		FinishSmokeTest(false, FString::Printf(TEXT("Requested Highland multi-camera smoke camera was missing: %s"), *CameraTag));
		return;
	}

	FString Label = CameraTag;
	FString CaptureVersion = TEXT("V832");
	if (Label.Contains(TEXT("FFSmokeHighlandV842")))
	{
		Label.ReplaceInline(TEXT("FFSmokeHighlandV842"), TEXT(""));
		CaptureVersion = TEXT("V842");
	}
	else if (Label.Contains(TEXT("FFSmokeHighlandV841")))
	{
		Label.ReplaceInline(TEXT("FFSmokeHighlandV841"), TEXT(""));
		CaptureVersion = TEXT("V841");
	}
	else
	{
		Label.ReplaceInline(TEXT("FFSmokeHighlandV832"), TEXT(""));
	}
	Label.ReplaceInline(TEXT("Camera"), TEXT(""));
	Label.TrimStartAndEndInline();
	if (Label.IsEmpty())
	{
		Label = CameraTag;
	}
	Label = FString::Printf(TEXT("%s_%02d_%s"), *CaptureVersion, CameraNumber, *Label);
	if (bSmokeV842WindProof)
	{
		Label = TEXT("V842_WindProof");
	}

	float CameraWarmupSeconds = bSmokeV842WindProof ? 0.02f : 0.18f;
	FParse::Value(FCommandLine::Get(), TEXT("FFSmokeCameraWarmup="), CameraWarmupSeconds);
	CameraWarmupSeconds = FMath::Clamp(
		CameraWarmupSeconds,
		bSmokeV842WindProof ? 0.02f : 0.18f,
		5.0f);
	QueueSmokeScreenshotCapture(Label, CameraWarmupSeconds);
	PendingSmokeCameraIndex++;
	float NextCameraDelay = CameraWarmupSeconds + 0.37f;
	if (bSmokeV842WindProof && PendingSmokeCameraIndex < PendingSmokeCameraTags.Num())
	{
		static const float WindFrameGaps[] = { 0.10f, 0.33f, 0.33f, 0.34f, 0.50f, 0.50f };
		const int32 GapIndex = FMath::Clamp(PendingSmokeCameraIndex - 1, 0, UE_ARRAY_COUNT(WindFrameGaps) - 1);
		NextCameraDelay = WindFrameGaps[GapIndex];
	}
	GetWorldTimerManager().SetTimer(
		SmokeStepTimer,
		this,
		&ThisClass::CaptureNextSmokeCamera,
		NextCameraDelay,
		false);
}

void ATP_ThirdPersonPlayerController::LogSmokeTestStep(const FString& StepLabel, bool bPassed, const FString& Details) const
{
	if (Details.IsEmpty())
	{
		UE_LOG(LogTP_ThirdPerson, Display, TEXT("FFSmoke %s: %s"), bPassed ? TEXT("PASS") : TEXT("FAIL"), *StepLabel);
		return;
	}

	UE_LOG(LogTP_ThirdPerson, Display, TEXT("FFSmoke %s: %s -- %s"), bPassed ? TEXT("PASS") : TEXT("FAIL"), *StepLabel, *Details);
}

AFantasyFrontierPlayableCharacter* ATP_ThirdPersonPlayerController::GetSmokePlayerCharacter() const
{
	return Cast<AFantasyFrontierPlayableCharacter>(GetPawn());
}

AFantasyFrontierPlayableCharacter* ATP_ThirdPersonPlayerController::EnsurePlayableCharacterPawn()
{
	if (AFantasyFrontierPlayableCharacter* ExistingPlayableCharacter = GetSmokePlayerCharacter())
	{
		return ExistingPlayableCharacter;
	}

	if (!GetWorld())
	{
		return nullptr;
	}

	APawn* ExistingPawn = GetPawn();
	FVector DesiredSpawnLocation = FVector::ZeroVector;
	FRotator DesiredSpawnRotation = FRotator::ZeroRotator;

	if (ExistingPawn)
	{
		DesiredSpawnLocation = ExistingPawn->GetActorLocation();
		DesiredSpawnRotation = ExistingPawn->GetActorRotation();
	}
	else if (APlayerStart* PlayerStart = Cast<APlayerStart>(UGameplayStatics::GetActorOfClass(this, APlayerStart::StaticClass())))
	{
		DesiredSpawnLocation = PlayerStart->GetActorLocation();
		DesiredSpawnRotation = PlayerStart->GetActorRotation();
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	AFantasyFrontierPlayableCharacter* SpawnedCharacter = GetWorld()->SpawnActor<AFantasyFrontierPlayableCharacter>(
		AFantasyFrontierPlayableCharacter::StaticClass(),
		DesiredSpawnLocation,
		DesiredSpawnRotation,
		SpawnParameters);

	if (!SpawnedCharacter)
	{
		return nullptr;
	}

	Possess(SpawnedCharacter);
	if (ExistingPawn)
	{
		ExistingPawn->Destroy();
	}

	return SpawnedCharacter;
}

AFantasyFrontierTutorialDirector* ATP_ThirdPersonPlayerController::GetSmokeTutorialDirector() const
{
	return GetWorld() ? Cast<AFantasyFrontierTutorialDirector>(UGameplayStatics::GetActorOfClass(GetWorld(), AFantasyFrontierTutorialDirector::StaticClass())) : nullptr;
}

AFantasyFrontierTutorialDirector* ATP_ThirdPersonPlayerController::EnsureTutorialDirector() const
{
	if (AFantasyFrontierTutorialDirector* ExistingDirector = GetSmokeTutorialDirector())
	{
		return ExistingDirector;
	}

	if (!GetWorld())
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = const_cast<ATP_ThirdPersonPlayerController*>(this);
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	return GetWorld()->SpawnActor<AFantasyFrontierTutorialDirector>(
		AFantasyFrontierTutorialDirector::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParameters);
}

AFantasyFrontierFunctionalNpc* ATP_ThirdPersonPlayerController::FindNpcByRole(uint8 RoleValue) const
{
	if (!GetWorld())
	{
		return nullptr;
	}

	for (TActorIterator<AFantasyFrontierFunctionalNpc> It(GetWorld()); It; ++It)
	{
		if (static_cast<uint8>(It->GetNpcRole()) == RoleValue)
		{
			return *It;
		}
	}

	return nullptr;
}

AFantasyFrontierEnemyBase* ATP_ThirdPersonPlayerController::FindFirstEnemy() const
{
	if (!GetWorld())
	{
		return nullptr;
	}

	const AFantasyFrontierPlayableCharacter* PlayerCharacter = GetSmokePlayerCharacter();
	const FVector PlayerLocation = PlayerCharacter ? PlayerCharacter->GetActorLocation() : FVector::ZeroVector;
	AFantasyFrontierEnemyBase* BestEnemy = nullptr;
	float BestDistanceSq = TNumericLimits<float>::Max();

	for (TActorIterator<AFantasyFrontierEnemyBase> It(GetWorld()); It; ++It)
	{
		AFantasyFrontierEnemyBase* CandidateEnemy = *It;
		if (!IsValid(CandidateEnemy))
		{
			continue;
		}

		const FVector CandidateLocation = CandidateEnemy->GetActorLocation();
		const bool bLooksUnstable = CandidateEnemy->GetVelocity().Z < -200.0f || FMath::Abs(CandidateLocation.Z - PlayerLocation.Z) > 900.0f;
		const bool bTooFarForFirstEncounter = PlayerCharacter && FVector::DistSquared2D(CandidateLocation, PlayerLocation) > FMath::Square(4200.0f);
		if (bLooksUnstable || bTooFarForFirstEncounter)
		{
			continue;
		}

		const float DistanceSq = PlayerCharacter ? FVector::DistSquared2D(CandidateLocation, PlayerLocation) : 0.0f;
		if (!BestEnemy || DistanceSq < BestDistanceSq)
		{
			BestEnemy = CandidateEnemy;
			BestDistanceSq = DistanceSq;
		}
	}

	if (BestEnemy)
	{
		return BestEnemy;
	}

	for (TActorIterator<AFantasyFrontierEnemyBase> It(GetWorld()); It; ++It)
	{
		if (IsValid(*It))
		{
			return *It;
		}
	}

	return nullptr;
}

void ATP_ThirdPersonPlayerController::ApplyAndSaveUserSettings() const
{
	if (GEngine)
	{
		if (UGameUserSettings* UserSettings = GEngine->GetGameUserSettings())
		{
			UserSettings->ApplySettings(false);
			UserSettings->SaveSettings();
		}
	}

	ApplyHighlandPerformanceSettings();
	SavePersistentSettings();
}
