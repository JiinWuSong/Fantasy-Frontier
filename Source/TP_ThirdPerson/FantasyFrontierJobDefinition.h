#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FantasyFrontierJobDefinition.generated.h"

class UFantasyFrontierJobDefinition;

UENUM(BlueprintType)
enum class EFantasyFrontierJobTier : uint8
{
	MainJob UMETA(DisplayName = "Main Job"),
	SubJob UMETA(DisplayName = "Sub Job"),
	LifeSkill UMETA(DisplayName = "Life Skill")
};

USTRUCT(BlueprintType)
struct FFantasyFrontierJobStatBlock
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float Strength = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float Vitality = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float Agility = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float Intellect = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float Spirit = 0.0f;
};

USTRUCT(BlueprintType)
struct FFantasyFrontierRiskRewardModifierDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier")
	FName ModifierId = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier", meta = (MultiLine = true))
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier")
	float StaminaEfficiency = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier")
	float ArmorEfficiencyPenalty = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier")
	float SkillMasteryGain = 0.0f;
};

UCLASS(BlueprintType)
class UFantasyFrontierJobDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Job")
	FName JobId = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Job")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Job", meta = (MultiLine = true))
	FText ShortDescription;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Job")
	EFantasyFrontierJobTier JobTier = EFantasyFrontierJobTier::MainJob;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Job")
	FFantasyFrontierJobStatBlock BaseStats;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Job")
	FFantasyFrontierRiskRewardModifierDefinition RiskRewardModifier;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Job")
	TArray<TSoftObjectPtr<UFantasyFrontierJobDefinition>> RecommendedSubJobs;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Job")
	TArray<FName> StartingSkillTags;
};
