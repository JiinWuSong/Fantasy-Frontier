#pragma once

#include "CoreMinimal.h"
#include "FantasyFrontierVerticalSliceTypes.h"
#include "GameFramework/Actor.h"
#include "FantasyFrontierTutorialDirector.generated.h"

class AFantasyFrontierAmbientCreature;
class AFantasyFrontierFunctionalNpc;
class AFantasyFrontierHiddenScenarioTrigger;
class AFantasyFrontierPlayableCharacter;
class AFantasyFrontierRuinMysticEnemy;
class AFantasyFrontierStalkerEnemy;
class UInstancedStaticMeshComponent;
class ULevelStreamingDynamic;
class USceneComponent;

UCLASS()
class AFantasyFrontierTutorialDirector : public AActor
{
	GENERATED_BODY()

public:
	AFantasyFrontierTutorialDirector();

	virtual void BeginPlay() override;

	void HandleNpcInteraction(EFantasyFrontierNpcRole NpcRole, AFantasyFrontierPlayableCharacter* PlayerCharacter, bool bWasAlreadyTriggered);
	bool TryUnlockHiddenScenario(AFantasyFrontierPlayableCharacter* PlayerCharacter);
	bool HasGuideVisited() const { return bGuideVisited; }
	bool HasSmithVisited() const { return bSmithVisited; }
	bool HasTrainerVisited() const { return bTrainerVisited; }
	bool HasHiddenScenarioUnlocked() const { return bHiddenScenarioUnlocked; }
	FVector ResolveTutorialGroundLocation(const FVector& DesiredLocation, float HeightOffset) const;

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UInstancedStaticMeshComponent> GroundInstances;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UInstancedStaticMeshComponent> PathInstances;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UInstancedStaticMeshComponent> RuinInstances;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UInstancedStaticMeshComponent> TreeTrunkInstances;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UInstancedStaticMeshComponent> TreeCanopyInstances;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UInstancedStaticMeshComponent> CrystalInstances;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UInstancedStaticMeshComponent> SmithInstances;

	UPROPERTY(EditDefaultsOnly, Category = "FantasyFrontier|Tutorial")
	TSubclassOf<AFantasyFrontierFunctionalNpc> FunctionalNpcClass;

	UPROPERTY(EditDefaultsOnly, Category = "FantasyFrontier|Tutorial")
	TSubclassOf<AFantasyFrontierAmbientCreature> WildlifeClass;

	UPROPERTY(EditDefaultsOnly, Category = "FantasyFrontier|Tutorial")
	TSubclassOf<AFantasyFrontierStalkerEnemy> StalkerEnemyClass;

	UPROPERTY(EditDefaultsOnly, Category = "FantasyFrontier|Tutorial")
	TSubclassOf<AFantasyFrontierRuinMysticEnemy> RuinMysticEnemyClass;

	UPROPERTY(EditDefaultsOnly, Category = "FantasyFrontier|Tutorial")
	TSubclassOf<AFantasyFrontierHiddenScenarioTrigger> HiddenScenarioTriggerClass;

	UPROPERTY(EditDefaultsOnly, Category = "FantasyFrontier|Tutorial")
	TSoftObjectPtr<UWorld> CelticVillageLandscapingLevelAsset = TSoftObjectPtr<UWorld>(
		FSoftObjectPath(TEXT("/Game/Environment/Grassland/CelticVillage/LI_Grassland_Celtic_Village_Landscaping.LI_Grassland_Celtic_Village_Landscaping")));

	UPROPERTY(EditDefaultsOnly, Category = "FantasyFrontier|Tutorial")
	TSoftObjectPtr<UWorld> LowerIslandSurroundingLevelAsset = TSoftObjectPtr<UWorld>(
		FSoftObjectPath(TEXT("/Game/Environment/StarterIsland/LevelInstances/LI_StarterIsland_Stonebridge_Surrounding.LI_StarterIsland_Stonebridge_Surrounding")));

	UPROPERTY(EditDefaultsOnly, Category = "FantasyFrontier|Tutorial")
	TSoftObjectPtr<UWorld> KashkehLandscapeLevelAsset = TSoftObjectPtr<UWorld>(
		FSoftObjectPath(TEXT("/Game/Environment/Grassland/Kashkeh/LI_Kashkeh_Landscape.LI_Kashkeh_Landscape")));

	UPROPERTY(EditDefaultsOnly, Category = "FantasyFrontier|Tutorial")
	TSoftObjectPtr<UWorld> KashkehRoadsLevelAsset = TSoftObjectPtr<UWorld>(
		FSoftObjectPath(TEXT("/Game/Environment/Grassland/Kashkeh/LI_Kashkeh_RoadsandPaths.LI_Kashkeh_RoadsandPaths")));

	UPROPERTY(EditDefaultsOnly, Category = "FantasyFrontier|Tutorial")
	TSoftObjectPtr<UWorld> KashkehFencesLevelAsset = TSoftObjectPtr<UWorld>(
		FSoftObjectPath(TEXT("/Game/Environment/Grassland/Kashkeh/LI_Kashkeh_FencesandBorders.LI_Kashkeh_FencesandBorders")));

	UPROPERTY(EditDefaultsOnly, Category = "FantasyFrontier|Tutorial")
	TSoftObjectPtr<UWorld> KashkehFoliageLevelAsset = TSoftObjectPtr<UWorld>(
		FSoftObjectPath(TEXT("/Game/Environment/Grassland/Kashkeh/LI_Kashkeh_FoliageandRocks.LI_Kashkeh_FoliageandRocks")));

	UPROPERTY(EditDefaultsOnly, Category = "FantasyFrontier|Tutorial")
	TSoftObjectPtr<UWorld> KashkehPostsLevelAsset = TSoftObjectPtr<UWorld>(
		FSoftObjectPath(TEXT("/Game/Environment/Grassland/Kashkeh/LI_Kashkeh_PostsandMiniBridges.LI_Kashkeh_PostsandMiniBridges")));

	UPROPERTY(EditDefaultsOnly, Category = "FantasyFrontier|Tutorial")
	TSoftObjectPtr<UWorld> KashkehHutsLevelAsset = TSoftObjectPtr<UWorld>(
		FSoftObjectPath(TEXT("/Game/Environment/Grassland/Kashkeh/LI_Kashkeh_HutsandProps.LI_Kashkeh_HutsandProps")));

	UPROPERTY(EditDefaultsOnly, Category = "FantasyFrontier|Tutorial")
	TSoftObjectPtr<UWorld> KashkehCropFieldsLevelAsset = TSoftObjectPtr<UWorld>(
		FSoftObjectPath(TEXT("/Game/Environment/Grassland/Kashkeh/LI_Kashkeh_CropFeilds.LI_Kashkeh_CropFeilds")));

	UPROPERTY(EditDefaultsOnly, Category = "FantasyFrontier|Tutorial")
	TSoftObjectPtr<UWorld> KashkehLightingLevelAsset = TSoftObjectPtr<UWorld>(
		FSoftObjectPath(TEXT("/Game/Environment/Grassland/Kashkeh/LI_Kashkeh_Lighting.LI_Kashkeh_Lighting")));

	UPROPERTY(EditDefaultsOnly, Category = "FantasyFrontier|Tutorial")
	TSoftObjectPtr<UWorld> GrasslandFarmShellLevelAsset = TSoftObjectPtr<UWorld>(
		FSoftObjectPath(TEXT("/Game/Environment/Grassland/LevelInstances/Landscape_Asrah.Landscape_Asrah")));

private:
	bool ShouldUseStarterForestSlice() const;
	bool TryResolveTutorialGroundLocation(const FVector& DesiredLocation, float HeightOffset, FVector& OutResolvedLocation) const;
	FVector GetStarterForestPlayerLocation() const;
	void EnsureStarterForestLighting();
	void EnsureLowerIslandSurroundingContext();
	UFUNCTION()
	void HandleLowerIslandSurroundingLevelShown();
	void BuildEnvironment();
	void SpawnRuntimeActors();
	void PositionExistingPlayer();
	void ApplyLerigothiVillageGroundCleanup();
	void ScheduleStarterForestSpawnValidation();
	void ValidateStarterForestSpawn();
	void SuppressTemplateLevelActors();
	void AddGroundTile(const FVector& Location, const FVector& Scale);
	void AddPathTile(const FVector& Location, const FVector& Scale);
	void AddRuinBlock(const FVector& Location, const FVector& Scale, const FRotator& Rotation = FRotator::ZeroRotator);
	void AddTree(const FVector& Location, float Scale);
	void AddCrystal(const FVector& Location, float Scale);
	void ApplyPalette(UInstancedStaticMeshComponent* Component, const FLinearColor& Color) const;

	bool bBuiltEnvironment = false;
	bool bGuideVisited = false;
	bool bSmithVisited = false;
	bool bTrainerVisited = false;
	bool bHiddenScenarioUnlocked = false;
	bool bAppliedLerigothiGroundCleanup = false;
	int32 StarterForestSpawnValidationPassesRemaining = 0;
	FTimerHandle StarterForestSpawnValidationTimer;
	UPROPERTY(Transient)
	TObjectPtr<ULevelStreamingDynamic> CelticVillageLandscapingLevel;
	UPROPERTY(Transient)
	TObjectPtr<ULevelStreamingDynamic> LowerIslandSurroundingLevel;
	UPROPERTY(Transient)
	TObjectPtr<ULevelStreamingDynamic> KashkehLandscapeLevel;
	UPROPERTY(Transient)
	TObjectPtr<ULevelStreamingDynamic> KashkehRoadsLevel;
	UPROPERTY(Transient)
	TObjectPtr<ULevelStreamingDynamic> KashkehFencesLevel;
	UPROPERTY(Transient)
	TObjectPtr<ULevelStreamingDynamic> KashkehFoliageLevel;
	UPROPERTY(Transient)
	TObjectPtr<ULevelStreamingDynamic> KashkehPostsLevel;
	UPROPERTY(Transient)
	TObjectPtr<ULevelStreamingDynamic> KashkehHutsLevel;
	UPROPERTY(Transient)
	TObjectPtr<ULevelStreamingDynamic> KashkehCropFieldsLevel;
	UPROPERTY(Transient)
	TObjectPtr<ULevelStreamingDynamic> KashkehLightingLevel;
	UPROPERTY(Transient)
	TObjectPtr<ULevelStreamingDynamic> GrasslandFarmShellLevel;
};
