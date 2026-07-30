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
class UFantasyFrontierUserSettingsSaveGame;
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
UCLASS(abstract, Config=Game)
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

public:
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

private:
	void ShowFrontEnd();
	void HideFrontEnd(bool bStopMusic = true);
	void ShowCharacterCreator();
	void HideCharacterCreator(bool bDestroyPreviewActor = true);
	void ApplyMenuInputState(bool bUIEnabled, const TSharedPtr<SWidget>& FocusWidget);
	void StartFrontEndMusic(EFantasyFrontierMusicState DesiredState);
	void StopFrontEndMusic(float FadeOutDuration = 0.6f);
	void SetFrontEndMusicState(EFantasyFrontierMusicState DesiredState, float FadeOutDuration = 0.25f);
	USoundBase* ResolveMusicCue(EFantasyFrontierMusicState DesiredState) const;
	bool IsExpectedMusicPlaying(EFantasyFrontierMusicState ExpectedState, FString* OutReason = nullptr) const;
	void SetLivePawnMenuHold(bool bHeld);
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
	void CycleGrassDensityQuality();
	void CycleFoliageDistanceQuality();
	void LoadPersistentSettings();
	void SavePersistentSettings() const;
	void SetMasterVolumeValue(float NewValue, bool bPersist);
	void HandleMasterVolumeChanged(float NewValue);
	FText GetWindowModeText() const;
	FText GetQualityLevelText() const;
	FText GetShadowQualityText() const;
	FText GetAntiAliasingQualityText() const;
	FText GetPostProcessQualityText() const;
	FText GetViewDistanceQualityText() const;
	FText GetGrassDensityQualityText() const;
	FText GetFoliageDistanceQualityText() const;
	FText GetMasterVolumeText() const;
	float GetMasterVolumeNormalized() const;
	void ApplyHighlandPerformanceSettings() const;
	void ApplyMasterVolume();
	void ApplyAndSaveUserSettings() const;
	void AdvanceSmokeTest();
	void RunSmokeMovementPulse();
	void FinishSmokeTest(bool bSuccess, const FString& FailureReason = FString());
	void RequestSmokeTestExit();
	void CaptureSmokeScreenshot(const FString& Label);
	bool FocusSmokeCameraOnTaggedActor(const FName& CameraTag);
	void QueueSmokeScreenshotCapture(const FString& Label, float DelaySeconds = 0.18f);
	void ExecuteQueuedSmokeScreenshot();
	void BeginSmokeCameraSequence(const FString& CameraTagList);
	void CaptureNextSmokeCamera();
	void FrameSmokeWorldView(const FVector& CharacterLocation, const FRotator& ViewRotation);
	void LogSmokeTestStep(const FString& StepLabel, bool bPassed, const FString& Details = FString()) const;
	AFantasyFrontierPlayableCharacter* GetSmokePlayerCharacter() const;
	AFantasyFrontierPlayableCharacter* EnsurePlayableCharacterPawn();
	AFantasyFrontierTutorialDirector* GetSmokeTutorialDirector() const;
	AFantasyFrontierTutorialDirector* EnsureTutorialDirector() const;
	bool ValidateHighlandHillGrounding(AFantasyFrontierPlayableCharacter* PlayerCharacter);
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
	FTimerHandle SmokeCaptureTimer;
	TWeakObjectPtr<AFantasyFrontierEnemyBase> SmokeTrackedEnemy;
	FString PendingSmokeCaptureLabel;
	TArray<FString> PendingSmokeCameraTags;
	int32 PendingSmokeCameraIndex = 0;
	bool bSmokeV842WindProof = false;
	float SmokeV842WindProofStartSeconds = -1.0f;
	int32 SmokeV842WindProofFrameIndex = 0;

	UPROPERTY(Config)
	float MasterVolume = 0.30f;

	UPROPERTY(Config)
	int32 GrassDensityQuality = 2;

	UPROPERTY(Config)
	int32 FoliageDistanceQuality = 2;

	static inline const TCHAR* UserSettingsSlotName = TEXT("FantasyFrontierUserSettings");
	static constexpr int32 UserSettingsSlotIndex = 0;

};
