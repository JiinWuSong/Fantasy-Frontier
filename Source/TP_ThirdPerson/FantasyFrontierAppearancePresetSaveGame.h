// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FantasyFrontierVerticalSliceTypes.h"
#include "GameFramework/SaveGame.h"
#include "FantasyFrontierAppearancePresetSaveGame.generated.h"

UCLASS()
class UFantasyFrontierAppearancePresetSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FFantasyFrontierAppearancePresetRecord> Presets;
};
