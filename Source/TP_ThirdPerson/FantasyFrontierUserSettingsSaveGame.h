// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "FantasyFrontierUserSettingsSaveGame.generated.h"

UCLASS()
class UFantasyFrontierUserSettingsSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MasterVolume = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GraphicsQuality = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ShadowQuality = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AntiAliasingQuality = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PostProcessQuality = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ViewDistanceQuality = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GrassDensityQuality = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FoliageDistanceQuality = 2;
};
