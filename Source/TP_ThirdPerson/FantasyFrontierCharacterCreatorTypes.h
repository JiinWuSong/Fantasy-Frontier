// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FantasyFrontierCharacterCreatorTypes.generated.h"

UENUM(BlueprintType)
enum class EFantasyFrontierRace : uint8
{
	Human,
	Wolfkin,
	Orc,
	Dwarf,
	Elf,
	Stagborn,
	Drakyn
};

UENUM(BlueprintType)
enum class EFantasyFrontierGender : uint8
{
	Male,
	Female
};

UENUM(BlueprintType)
enum class EFantasyFrontierClass : uint8
{
	Vanguard,
	Lancer,
	Spellweaver
};

UENUM(BlueprintType)
enum class EFantasyFrontierPreviewMode : uint8
{
	BaseBody,
	StarterGear,
	OriginStyle
};

UENUM(BlueprintType)
enum class EFantasyFrontierOriginStyle : uint8
{
	FrontierWanderer,
	Runeseeker,
	VerdantVanguard
};

UENUM()
enum class EFantasyFrontierCreatorStep : uint8
{
	Race,
	Gender,
	Appearance,
	Name,
	Class
};

UENUM()
enum class EFantasyFrontierCustomizationTab : uint8
{
	Body,
	HairStyle,
	Face,
	Markings
};

USTRUCT(BlueprintType)
struct FFantasyFrontierCharacterDraft
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFantasyFrontierRace Race = EFantasyFrontierRace::Human;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFantasyFrontierGender Gender = EFantasyFrontierGender::Female;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HeightScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Build = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Musculature = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StylePreset = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SkinTone = 0.56f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ScarIntensity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TattooIntensity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HairStyle = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor HairColor = FLinearColor(0.16f, 0.13f, 0.10f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FaceVariant = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor EyeColor = FLinearColor(0.25f, 0.73f, 0.96f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFantasyFrontierPreviewMode PreviewMode = EFantasyFrontierPreviewMode::BaseBody;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFantasyFrontierOriginStyle OriginStyle = EFantasyFrontierOriginStyle::FrontierWanderer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CharacterName = TEXT("Aerin");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFantasyFrontierClass CharacterClass = EFantasyFrontierClass::Lancer;
};
