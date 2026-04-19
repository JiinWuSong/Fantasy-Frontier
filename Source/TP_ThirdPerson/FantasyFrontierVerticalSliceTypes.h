// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FantasyFrontierCharacterCreatorTypes.h"
#include "FantasyFrontierVerticalSliceTypes.generated.h"

UENUM(BlueprintType)
enum class EFantasyFrontierGearSlot : uint8
{
	Head,
	Hair,
	Torso,
	ArmsHands,
	Legs,
	Feet,
	Accessory
};

UENUM(BlueprintType)
enum class EFantasyFrontierNpcRole : uint8
{
	Guide,
	Smith,
	Trainer
};

UENUM(BlueprintType)
enum class EFantasyFrontierScenarioRewardModifier : uint8
{
	None,
	SwiftstepFragility
};

UENUM(BlueprintType)
enum class EFantasyFrontierScenarioState : uint8
{
	Locked,
	Discovered,
	Completed
};

USTRUCT(BlueprintType)
struct FFantasyFrontierGearPieceDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName GearId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFantasyFrontierGearSlot Slot = EFantasyFrontierGearSlot::Torso;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor AccentColor = FLinearColor(0.65f, 0.52f, 0.27f, 1.0f);
};

USTRUCT(BlueprintType)
struct FFantasyFrontierStarterGearSetDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SetId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FFantasyFrontierGearPieceDefinition> Pieces;
};

USTRUCT(BlueprintType)
struct FFantasyFrontierAppearancePresetRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PresetName = TEXT("Latest");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FFantasyFrontierCharacterDraft Draft;
};

USTRUCT(BlueprintType)
struct FFantasyFrontierScenarioDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ScenarioId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Summary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFantasyFrontierScenarioRewardModifier RewardModifier = EFantasyFrontierScenarioRewardModifier::None;
};
