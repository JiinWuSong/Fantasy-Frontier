#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FantasyFrontierScenarioDefinition.generated.h"

UENUM(BlueprintType)
enum class EFantasyFrontierScenarioRewardType : uint8
{
	SystemUnlock UMETA(DisplayName = "System Unlock"),
	GearHook UMETA(DisplayName = "Gear Hook"),
	Modifier UMETA(DisplayName = "Modifier"),
	Lore UMETA(DisplayName = "Lore")
};

USTRUCT(BlueprintType)
struct FFantasyFrontierScenarioCondition
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scenario")
	FName ConditionId = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scenario")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scenario", meta = (MultiLine = true))
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scenario")
	bool bIsRequired = true;
};

USTRUCT(BlueprintType)
struct FFantasyFrontierScenarioReward
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scenario")
	EFantasyFrontierScenarioRewardType RewardType = EFantasyFrontierScenarioRewardType::SystemUnlock;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scenario")
	FName RewardId = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scenario")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scenario", meta = (MultiLine = true))
	FText Description;
};

UCLASS(BlueprintType)
class UFantasyFrontierScenarioDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scenario")
	FName ScenarioId = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scenario")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scenario", meta = (MultiLine = true))
	FText Summary;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scenario")
	bool bHiddenUntilTriggered = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scenario")
	TArray<FFantasyFrontierScenarioCondition> TriggerConditions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scenario")
	TArray<FFantasyFrontierScenarioReward> Rewards;
};
