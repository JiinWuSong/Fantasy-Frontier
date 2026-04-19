// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FantasyFrontierCharacterCreatorTypes.h"
#include "GameFramework/PlayerController.h"
#include "TP_ThirdPersonPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;
class UAudioComponent;
class USoundBase;
class UTextureRenderTarget2D;
class UFantasyFrontierAppearancePresetSaveGame;
class SFantasyFrontierFrontEndWidget;
class SFantasyFrontierCharacterCreatorWidget;
class AFantasyFrontierCharacterPreviewActor;
class AFantasyFrontierPlayableCharacter;
class AFantasyFrontierTutorialDirector;
class AFantasyFrontierFunctionalNpc;
class AFantasyFrontierEnemyBase;

/**
 *  Basic PlayerController class for a third person game
 *  Manages input mappings and the startup front-end flow.
 */
UCLASS(abstract)
class ATP_ThirdPersonPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Cleanup */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;

private:
	enum class EFantasyFrontierMusicState : uint8
	{
		None,
		Title,
		CharacterCreator,
		Overworld,
		Combat,
		City,
		Shop
	};

	void ShowFrontEnd();
	void HideFrontEnd(bool bStopMusic = true);
	void ShowCharacterCreator();
	void HideCharacterCreator(bool bDestroyPreviewActor = true);
	void ApplyMenuInputState(bool bUIEnabled, const TSharedPtr<SWidget>& FocusWidget);
	void StartFrontEndMusic(EFantasyFrontierMusicState DesiredState);
	void StopFrontEndMusic(float FadeOutDuration = 0.6f);
	void SetFrontEndMusicState(EFantasyFrontierMusicState DesiredState, float FadeOutDuration = 0.25f);
	USoundBase* ResolveMusicCue(EFantasyFrontierMusicState DesiredState) const;
	void StartGameFromFrontEnd();
	void HandleCharacterCreatorCancelled();
	void HandleCharacterDraftChanged(FFantasyFrontierCharacterDraft InDraft);
	void HandleCharacterPreviewRotated(float InDeltaYaw);
	void HandleCharacterPreviewZoomed(float InDeltaZoom);
	void SaveAppearancePreset();
	void LoadAppearancePreset();
	void CommitCharacterCreator(FFantasyFrontierCharacterDraft InDraft);
	void EnsureCharacterPreview();
	void ApplyDraftToPlayerPawn(const FFantasyFrontierCharacterDraft& InDraft);
	void QuitFromFrontEnd();
	void CycleWindowMode();
	void CycleQualityLevel();
	void CycleShadowQuality();
	void CycleAntiAliasingQuality();
	void CyclePostProcessQuality();
	void CycleViewDistanceQuality();
	FText GetWindowModeText() const;
	FText GetQualityLevelText() const;
	FText GetShadowQualityText() const;
	FText GetAntiAliasingQualityText() const;
	FText GetPostProcessQualityText() const;
	FText GetViewDistanceQualityText() const;
	void ApplyAndSaveUserSettings() const;
	void AdvanceSmokeTest();
	void RunSmokeMovementPulse();
	void FinishSmokeTest(bool bSuccess, const FString& FailureReason = FString());
	void RequestSmokeTestExit();
	void CaptureSmokeScreenshot(const FString& Label);
	void LogSmokeTestStep(const FString& StepLabel, bool bPassed, const FString& Details = FString()) const;
	AFantasyFrontierPlayableCharacter* GetSmokePlayerCharacter() const;
	AFantasyFrontierPlayableCharacter* EnsurePlayableCharacterPawn();
	AFantasyFrontierTutorialDirector* GetSmokeTutorialDirector() const;
	AFantasyFrontierTutorialDirector* EnsureTutorialDirector() const;
	AFantasyFrontierFunctionalNpc* FindNpcByRole(uint8 RoleValue) const;
	AFantasyFrontierEnemyBase* FindFirstEnemy() const;

	TSharedPtr<SFantasyFrontierFrontEndWidget> FrontEndWidget;
	TSharedPtr<SFantasyFrontierCharacterCreatorWidget> CharacterCreatorWidget;
	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> FrontEndMusicComponent;
	EFantasyFrontierMusicState CurrentMusicState = EFantasyFrontierMusicState::None;
	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> CharacterPreviewRenderTarget;
	UPROPERTY(Transient)
	TObjectPtr<AFantasyFrontierCharacterPreviewActor> CharacterPreviewActor;
	FFantasyFrontierCharacterDraft LastLoadedDraft;
	bool bFrontEndVisible = false;
	bool bCharacterCreatorVisible = false;
	bool bRunSmokeTest = false;
	bool bSmokeTestComplete = false;
	bool bCaptureSmokeScreenshots = false;
	int32 SmokeStepIndex = 0;
	int32 SmokeScreenshotIndex = 0;
	int32 SmokeMoveTicksRemaining = 0;
	float SmokeActionStaminaBefore = 0.0f;
	FVector SmokeMovementStart = FVector::ZeroVector;
	FString SmokeScreenshotPrefix;
	FTimerHandle SmokeStepTimer;
	FTimerHandle SmokeMoveTimer;
	FTimerHandle SmokeExitTimer;
	TWeakObjectPtr<AFantasyFrontierEnemyBase> SmokeTrackedEnemy;

};
