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

private:
	void BuildEnvironment();
	void SpawnRuntimeActors();
	void PositionExistingPlayer();
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
};
