// Copyright Epic Games, Inc. All Rights Reserved.


#include "TP_ThirdPersonPlayerController.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "FantasyFrontierAppearancePresetSaveGame.h"
#include "FantasyFrontierCharacterCreatorTypes.h"
#include "FantasyFrontierCharacterCreatorWidget.h"
#include "FantasyFrontierCharacterPreviewActor.h"
#include "FantasyFrontierEnemyBase.h"
#include "FantasyFrontierFunctionalNpc.h"
#include "FantasyFrontierFrontEndWidget.h"
#include "FantasyFrontierPlayableCharacter.h"
#include "FantasyFrontierTutorialDirector.h"
#include "FantasyFrontierVerticalSliceTypes.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Engine/TextureRenderTarget2D.h"
#include "EngineUtils.h"
#include "GameFramework/GameUserSettings.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerState.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/DateTime.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Sound/SoundBase.h"
#include "TP_ThirdPerson.h"
#include "UnrealClient.h"
#include "Widgets/Input/SVirtualJoystick.h"

void ATP_ThirdPersonPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
	if (ShouldUseTouchControls() && IsLocalPlayerController())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogTP_ThirdPerson, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}

	ShowFrontEnd();

	bRunSmokeTest = FParse::Param(FCommandLine::Get(), TEXT("FFSmokeTest"));
	if (bRunSmokeTest && GetWorld())
	{
		bCaptureSmokeScreenshots = !FParse::Param(FCommandLine::Get(), TEXT("FFSmokeNoScreens"));
		if (!FParse::Value(FCommandLine::Get(), TEXT("FFSmokeCapturePrefix="), SmokeScreenshotPrefix) || SmokeScreenshotPrefix.IsEmpty())
		{
			SmokeScreenshotPrefix = FDateTime::Now().ToString(TEXT("FFSmoke_yyyyMMdd_HHmmss"));
		}
		GetWorldTimerManager().SetTimer(SmokeStepTimer, this, &ThisClass::AdvanceSmokeTest, 0.6f, false);
	}
}

void ATP_ThirdPersonPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(SmokeStepTimer);
	GetWorldTimerManager().ClearTimer(SmokeMoveTimer);
	GetWorldTimerManager().ClearTimer(SmokeExitTimer);
	HideCharacterCreator();
	HideFrontEnd();
	Super::EndPlay(EndPlayReason);
}

void ATP_ThirdPersonPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

bool ATP_ThirdPersonPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}

void ATP_ThirdPersonPlayerController::ShowFrontEnd()
{
	if (!IsLocalPlayerController() || FrontEndWidget.IsValid() || !GEngine || !GEngine->GameViewport)
	{
		return;
	}

	SAssignNew(FrontEndWidget, SFantasyFrontierFrontEndWidget)
		.OnStartGame(FSimpleDelegate::CreateUObject(this, &ThisClass::StartGameFromFrontEnd))
		.OnQuit(FSimpleDelegate::CreateUObject(this, &ThisClass::QuitFromFrontEnd))
		.OnCycleWindowMode(FSimpleDelegate::CreateUObject(this, &ThisClass::CycleWindowMode))
		.OnCycleQuality(FSimpleDelegate::CreateUObject(this, &ThisClass::CycleQualityLevel))
		.OnCycleShadowQuality(FSimpleDelegate::CreateUObject(this, &ThisClass::CycleShadowQuality))
		.OnCycleAntiAliasingQuality(FSimpleDelegate::CreateUObject(this, &ThisClass::CycleAntiAliasingQuality))
		.OnCyclePostProcessQuality(FSimpleDelegate::CreateUObject(this, &ThisClass::CyclePostProcessQuality))
		.OnCycleViewDistanceQuality(FSimpleDelegate::CreateUObject(this, &ThisClass::CycleViewDistanceQuality))
		.WindowModeLabel_Lambda([this]() { return GetWindowModeText(); })
		.QualityLabel_Lambda([this]() { return GetQualityLevelText(); })
		.ShadowQualityLabel_Lambda([this]() { return GetShadowQualityText(); })
		.AntiAliasingLabel_Lambda([this]() { return GetAntiAliasingQualityText(); })
		.PostProcessLabel_Lambda([this]() { return GetPostProcessQualityText(); })
		.ViewDistanceLabel_Lambda([this]() { return GetViewDistanceQualityText(); });

	GEngine->GameViewport->AddViewportWidgetContent(FrontEndWidget.ToSharedRef(), 100);
	bFrontEndVisible = true;
	ApplyMenuInputState(true, FrontEndWidget);
	StartFrontEndMusic(EFantasyFrontierMusicState::Title);
}

void ATP_ThirdPersonPlayerController::HideFrontEnd(bool bStopMusic)
{
	if (!FrontEndWidget.IsValid())
	{
		return;
	}

	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(FrontEndWidget.ToSharedRef());
	}

	FrontEndWidget.Reset();
	bFrontEndVisible = false;
	if (bStopMusic)
	{
		StopFrontEndMusic();
	}
}

void ATP_ThirdPersonPlayerController::ShowCharacterCreator()
{
	if (!IsLocalPlayerController() || CharacterCreatorWidget.IsValid() || !GEngine || !GEngine->GameViewport)
	{
		return;
	}

	EnsureCharacterPreview();

	SAssignNew(CharacterCreatorWidget, SFantasyFrontierCharacterCreatorWidget)
		.PreviewTexture(CharacterPreviewRenderTarget)
		.OnCancel(FSimpleDelegate::CreateUObject(this, &ThisClass::HandleCharacterCreatorCancelled))
		.OnDraftChanged(FFantasyFrontierCharacterDraftChanged::CreateUObject(this, &ThisClass::HandleCharacterDraftChanged))
		.OnPreviewRotate(FFantasyFrontierPreviewRotate::CreateUObject(this, &ThisClass::HandleCharacterPreviewRotated))
		.OnPreviewZoom(FFantasyFrontierPreviewZoom::CreateUObject(this, &ThisClass::HandleCharacterPreviewZoomed))
		.OnSavePreset(FSimpleDelegate::CreateUObject(this, &ThisClass::SaveAppearancePreset))
		.OnLoadPreset(FSimpleDelegate::CreateUObject(this, &ThisClass::LoadAppearancePreset))
		.OnConfirm(FFantasyFrontierCharacterDraftCommitted::CreateUObject(this, &ThisClass::CommitCharacterCreator));

	if (!LastLoadedDraft.CharacterName.IsEmpty())
	{
		CharacterCreatorWidget->SetDraft(LastLoadedDraft);
	}

	GEngine->GameViewport->AddViewportWidgetContent(CharacterCreatorWidget.ToSharedRef(), 105);
	bCharacterCreatorVisible = true;
	ApplyMenuInputState(true, CharacterCreatorWidget);
	StartFrontEndMusic(EFantasyFrontierMusicState::CharacterCreator);
}

void ATP_ThirdPersonPlayerController::HideCharacterCreator(bool bDestroyPreviewActor)
{
	if (CharacterCreatorWidget.IsValid() && GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(CharacterCreatorWidget.ToSharedRef());
	}

	CharacterCreatorWidget.Reset();
	bCharacterCreatorVisible = false;

	if (bDestroyPreviewActor && CharacterPreviewActor)
	{
		CharacterPreviewActor->Destroy();
		CharacterPreviewActor = nullptr;
	}
}

void ATP_ThirdPersonPlayerController::ApplyMenuInputState(bool bUIEnabled, const TSharedPtr<SWidget>& FocusWidget)
{
	bShowMouseCursor = bUIEnabled;
	bEnableClickEvents = bUIEnabled;
	bEnableMouseOverEvents = bUIEnabled;
	SetIgnoreMoveInput(bUIEnabled);
	SetIgnoreLookInput(bUIEnabled);

	if (bUIEnabled)
	{
		FInputModeUIOnly InputMode;
		if (FocusWidget.IsValid())
		{
			InputMode.SetWidgetToFocus(FocusWidget);
		}
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
	}
	else
	{
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
	}
}

void ATP_ThirdPersonPlayerController::StartFrontEndMusic(EFantasyFrontierMusicState DesiredState)
{
	SetFrontEndMusicState(DesiredState);
}

void ATP_ThirdPersonPlayerController::StopFrontEndMusic(float FadeOutDuration)
{
	if (!FrontEndMusicComponent)
	{
		CurrentMusicState = EFantasyFrontierMusicState::None;
		return;
	}

	if (FadeOutDuration > 0.0f)
	{
		FrontEndMusicComponent->FadeOut(FadeOutDuration, 0.0f);
	}
	else
	{
		FrontEndMusicComponent->Stop();
	}

	FrontEndMusicComponent = nullptr;
	CurrentMusicState = EFantasyFrontierMusicState::None;
}

void ATP_ThirdPersonPlayerController::SetFrontEndMusicState(EFantasyFrontierMusicState DesiredState, float FadeOutDuration)
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	if (DesiredState == EFantasyFrontierMusicState::None)
	{
		StopFrontEndMusic(FadeOutDuration);
		return;
	}

	USoundBase* DesiredCue = ResolveMusicCue(DesiredState);
	if (!DesiredCue)
	{
		StopFrontEndMusic(FadeOutDuration);
		return;
	}

	if (FrontEndMusicComponent && FrontEndMusicComponent->Sound == DesiredCue)
	{
		if (!FrontEndMusicComponent->IsPlaying())
		{
			FrontEndMusicComponent->FadeIn(0.25f, 0.52f);
		}
		CurrentMusicState = DesiredState;
		return;
	}

	StopFrontEndMusic(0.0f);

	FrontEndMusicComponent = UGameplayStatics::SpawnSound2D(this, DesiredCue, 0.0f, 1.0f, 0.0f, nullptr, false, false);
	if (!FrontEndMusicComponent)
	{
		CurrentMusicState = EFantasyFrontierMusicState::None;
		return;
	}

	FrontEndMusicComponent->bIsUISound = true;
	FrontEndMusicComponent->FadeIn(0.35f, DesiredState == EFantasyFrontierMusicState::Title ? 0.48f : 0.40f);
	CurrentMusicState = DesiredState;
}

USoundBase* ATP_ThirdPersonPlayerController::ResolveMusicCue(EFantasyFrontierMusicState DesiredState) const
{
	switch (DesiredState)
	{
	case EFantasyFrontierMusicState::Title:
	{
		static USoundBase* Cue = LoadObject<USoundBase>(nullptr, TEXT("/Game/PathOfAdventure/Loop/CUE/CUE_Menu_Loop.CUE_Menu_Loop"));
		return Cue;
	}
	case EFantasyFrontierMusicState::CharacterCreator:
	{
		static USoundBase* Cue = LoadObject<USoundBase>(nullptr, TEXT("/Game/PathOfAdventure/Loop/CUE/CUE_Village_Loop.CUE_Village_Loop"));
		return Cue;
	}
	case EFantasyFrontierMusicState::Overworld:
	{
		static USoundBase* Cue = LoadObject<USoundBase>(nullptr, TEXT("/Game/PathOfAdventure/Loop/CUE/CUE_Field_Loop.CUE_Field_Loop"));
		return Cue;
	}
	case EFantasyFrontierMusicState::Combat:
	{
		static USoundBase* Cue = LoadObject<USoundBase>(nullptr, TEXT("/Game/PathOfAdventure/Loop/CUE/CUE_Battle_Loop.CUE_Battle_Loop"));
		return Cue;
	}
	case EFantasyFrontierMusicState::City:
	{
		static USoundBase* Cue = LoadObject<USoundBase>(nullptr, TEXT("/Game/PathOfAdventure/Loop/CUE/CUE_Village_Loop.CUE_Village_Loop"));
		return Cue;
	}
	case EFantasyFrontierMusicState::Shop:
	{
		static USoundBase* Cue = LoadObject<USoundBase>(nullptr, TEXT("/Game/PathOfAdventure/Loop/CUE/CUE_Shop_Loop.CUE_Shop_Loop"));
		return Cue;
	}
	case EFantasyFrontierMusicState::None:
	default:
		return nullptr;
	}
}

void ATP_ThirdPersonPlayerController::StartGameFromFrontEnd()
{
	if (!bFrontEndVisible)
	{
		return;
	}

	HideFrontEnd(false);
	ShowCharacterCreator();
}

void ATP_ThirdPersonPlayerController::HandleCharacterCreatorCancelled()
{
	HideCharacterCreator();
	ShowFrontEnd();
}

void ATP_ThirdPersonPlayerController::HandleCharacterDraftChanged(FFantasyFrontierCharacterDraft InDraft)
{
	if (CharacterPreviewActor)
	{
		CharacterPreviewActor->ApplyDraft(InDraft);
	}
}

void ATP_ThirdPersonPlayerController::HandleCharacterPreviewRotated(float InDeltaYaw)
{
	if (CharacterPreviewActor)
	{
		CharacterPreviewActor->AddPreviewYaw(InDeltaYaw);
	}
}

void ATP_ThirdPersonPlayerController::HandleCharacterPreviewZoomed(float InDeltaZoom)
{
	if (CharacterPreviewActor)
	{
		CharacterPreviewActor->AddPreviewZoom(InDeltaZoom);
	}
}

void ATP_ThirdPersonPlayerController::SaveAppearancePreset()
{
	if (!CharacterCreatorWidget.IsValid())
	{
		return;
	}

	UFantasyFrontierAppearancePresetSaveGame* SaveGame = Cast<UFantasyFrontierAppearancePresetSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UFantasyFrontierAppearancePresetSaveGame::StaticClass()));
	if (!SaveGame)
	{
		return;
	}

	FFantasyFrontierAppearancePresetRecord Record;
	Record.PresetName = TEXT("Latest");
	Record.Draft = CharacterCreatorWidget->GetDraft();
	SaveGame->Presets.Add(Record);
	LastLoadedDraft = Record.Draft;
	UGameplayStatics::SaveGameToSlot(SaveGame, TEXT("FF_AppearancePreset"), 0);
}

void ATP_ThirdPersonPlayerController::LoadAppearancePreset()
{
	if (!CharacterCreatorWidget.IsValid())
	{
		return;
	}

	if (!UGameplayStatics::DoesSaveGameExist(TEXT("FF_AppearancePreset"), 0))
	{
		return;
	}

	UFantasyFrontierAppearancePresetSaveGame* SaveGame = Cast<UFantasyFrontierAppearancePresetSaveGame>(
		UGameplayStatics::LoadGameFromSlot(TEXT("FF_AppearancePreset"), 0));
	if (!SaveGame || SaveGame->Presets.IsEmpty())
	{
		return;
	}

	LastLoadedDraft = SaveGame->Presets[0].Draft;
	CharacterCreatorWidget->SetDraft(LastLoadedDraft);
}

void ATP_ThirdPersonPlayerController::CommitCharacterCreator(FFantasyFrontierCharacterDraft InDraft)
{
	LastLoadedDraft = InDraft;
	ApplyDraftToPlayerPawn(InDraft);
	EnsureTutorialDirector();

	if (PlayerState)
	{
		PlayerState->SetPlayerName(InDraft.CharacterName);
	}

	HideCharacterCreator();
	StopFrontEndMusic();
	ApplyMenuInputState(false, nullptr);

	if (GEngine)
	{
		const FString Message = FString::Printf(TEXT("%s enters the frontier as %s."),
			*InDraft.CharacterName,
			InDraft.CharacterClass == EFantasyFrontierClass::Lancer ? TEXT("Lancer") : TEXT("Adventurer"));
		GEngine->AddOnScreenDebugMessage(-1, 6.0f, FColor::Emerald, Message);
	}
}

void ATP_ThirdPersonPlayerController::EnsureCharacterPreview()
{
	if (!CharacterPreviewRenderTarget)
	{
		CharacterPreviewRenderTarget = NewObject<UTextureRenderTarget2D>(this, TEXT("CharacterPreviewRenderTarget"));
		CharacterPreviewRenderTarget->ClearColor = FLinearColor(0.64f, 0.74f, 0.84f, 1.0f);
		CharacterPreviewRenderTarget->RenderTargetFormat = RTF_RGBA16f;
		CharacterPreviewRenderTarget->InitAutoFormat(1200, 1800);
		CharacterPreviewRenderTarget->TargetGamma = 2.2f;
		CharacterPreviewRenderTarget->UpdateResourceImmediate(true);
	}

	if (!CharacterPreviewActor && GetWorld())
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = this;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		CharacterPreviewActor = GetWorld()->SpawnActor<AFantasyFrontierCharacterPreviewActor>(
			AFantasyFrontierCharacterPreviewActor::StaticClass(),
			FVector(0.0f, 0.0f, -5000.0f),
			FRotator::ZeroRotator,
			SpawnParameters);

		if (CharacterPreviewActor)
		{
			CharacterPreviewActor->SetPreviewRenderTarget(CharacterPreviewRenderTarget);
			CharacterPreviewActor->ApplyDraft(FFantasyFrontierCharacterDraft());
		}
	}
}

void ATP_ThirdPersonPlayerController::ApplyDraftToPlayerPawn(const FFantasyFrontierCharacterDraft& InDraft)
{
	ACharacter* PlayerCharacter = EnsurePlayableCharacterPawn();
	if (!PlayerCharacter)
	{
		return;
	}

	AFantasyFrontierCharacterPreviewActor::ApplyDraftToGameplayMesh(PlayerCharacter->GetMesh(), InDraft);
	if (AFantasyFrontierPlayableCharacter* FrontierCharacter = Cast<AFantasyFrontierPlayableCharacter>(PlayerCharacter))
	{
		FrontierCharacter->ApplyPresentationBodyDraft(InDraft);
	}

	if (UCapsuleComponent* Capsule = PlayerCharacter->GetCapsuleComponent())
	{
		const float RadiusScale = FMath::Lerp(0.96f, 1.05f, InDraft.Build);
		Capsule->SetCapsuleSize(42.0f * RadiusScale, 96.0f * InDraft.HeightScale, true);
	}
}

void ATP_ThirdPersonPlayerController::QuitFromFrontEnd()
{
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
}

void ATP_ThirdPersonPlayerController::CycleWindowMode()
{
	if (!GEngine)
	{
		return;
	}

	UGameUserSettings* UserSettings = GEngine->GetGameUserSettings();
	if (!UserSettings)
	{
		return;
	}

	EWindowMode::Type NextMode = EWindowMode::WindowedFullscreen;
	switch (UserSettings->GetFullscreenMode())
	{
	case EWindowMode::Fullscreen:
		NextMode = EWindowMode::Windowed;
		break;
	case EWindowMode::Windowed:
		NextMode = EWindowMode::WindowedFullscreen;
		break;
	case EWindowMode::WindowedFullscreen:
	default:
		NextMode = EWindowMode::Fullscreen;
		break;
	}

	UserSettings->SetFullscreenMode(NextMode);
	ApplyAndSaveUserSettings();
}

void ATP_ThirdPersonPlayerController::CycleQualityLevel()
{
	if (!GEngine)
	{
		return;
	}

	UGameUserSettings* UserSettings = GEngine->GetGameUserSettings();
	if (!UserSettings)
	{
		return;
	}

	int32 CurrentLevel = UserSettings->GetOverallScalabilityLevel();
	if (CurrentLevel < 0)
	{
		CurrentLevel = 3;
	}

	const int32 NextLevel = (CurrentLevel + 1) % 5;
	UserSettings->SetOverallScalabilityLevel(NextLevel);
	ApplyAndSaveUserSettings();
}

void ATP_ThirdPersonPlayerController::CycleShadowQuality()
{
	if (!GEngine)
	{
		return;
	}

	if (UGameUserSettings* UserSettings = GEngine->GetGameUserSettings())
	{
		UserSettings->SetShadowQuality((UserSettings->GetShadowQuality() + 1) % 5);
		ApplyAndSaveUserSettings();
	}
}

void ATP_ThirdPersonPlayerController::CycleAntiAliasingQuality()
{
	if (!GEngine)
	{
		return;
	}

	if (UGameUserSettings* UserSettings = GEngine->GetGameUserSettings())
	{
		UserSettings->SetAntiAliasingQuality((UserSettings->GetAntiAliasingQuality() + 1) % 5);
		ApplyAndSaveUserSettings();
	}
}

void ATP_ThirdPersonPlayerController::CyclePostProcessQuality()
{
	if (!GEngine)
	{
		return;
	}

	if (UGameUserSettings* UserSettings = GEngine->GetGameUserSettings())
	{
		UserSettings->SetPostProcessingQuality((UserSettings->GetPostProcessingQuality() + 1) % 5);
		ApplyAndSaveUserSettings();
	}
}

void ATP_ThirdPersonPlayerController::CycleViewDistanceQuality()
{
	if (!GEngine)
	{
		return;
	}

	if (UGameUserSettings* UserSettings = GEngine->GetGameUserSettings())
	{
		UserSettings->SetViewDistanceQuality((UserSettings->GetViewDistanceQuality() + 1) % 5);
		ApplyAndSaveUserSettings();
	}
}

FText ATP_ThirdPersonPlayerController::GetWindowModeText() const
{
	const UGameUserSettings* UserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!UserSettings)
	{
		return FText::FromString(TEXT("Unbekannt"));
	}

	switch (UserSettings->GetFullscreenMode())
	{
	case EWindowMode::Fullscreen:
		return FText::FromString(TEXT("Fullscreen"));
	case EWindowMode::Windowed:
		return FText::FromString(TEXT("Windowed"));
	case EWindowMode::WindowedFullscreen:
	default:
		return FText::FromString(TEXT("Borderless"));
	}
}

FText ATP_ThirdPersonPlayerController::GetQualityLevelText() const
{
	const UGameUserSettings* UserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!UserSettings)
	{
		return FText::FromString(TEXT("Unbekannt"));
	}

	switch (UserSettings->GetOverallScalabilityLevel())
	{
	case 0:
		return FText::FromString(TEXT("Low"));
	case 1:
		return FText::FromString(TEXT("Medium"));
	case 2:
		return FText::FromString(TEXT("High"));
	case 3:
		return FText::FromString(TEXT("Epic"));
	case 4:
		return FText::FromString(TEXT("Cinematic"));
	default:
		return FText::FromString(TEXT("Custom"));
	}
}

FText ATP_ThirdPersonPlayerController::GetShadowQualityText() const
{
	const UGameUserSettings* UserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	return FText::FromString(UserSettings ? FString::FromInt(UserSettings->GetShadowQuality()) : TEXT("0"));
}

FText ATP_ThirdPersonPlayerController::GetAntiAliasingQualityText() const
{
	const UGameUserSettings* UserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	return FText::FromString(UserSettings ? FString::FromInt(UserSettings->GetAntiAliasingQuality()) : TEXT("0"));
}

FText ATP_ThirdPersonPlayerController::GetPostProcessQualityText() const
{
	const UGameUserSettings* UserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	return FText::FromString(UserSettings ? FString::FromInt(UserSettings->GetPostProcessingQuality()) : TEXT("0"));
}

FText ATP_ThirdPersonPlayerController::GetViewDistanceQualityText() const
{
	const UGameUserSettings* UserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	return FText::FromString(UserSettings ? FString::FromInt(UserSettings->GetViewDistanceQuality()) : TEXT("0"));
}

void ATP_ThirdPersonPlayerController::AdvanceSmokeTest()
{
	if (!bRunSmokeTest || bSmokeTestComplete)
	{
		return;
	}

	switch (SmokeStepIndex++)
	{
	case 0:
	{
		const bool bFrontEndReady = bFrontEndVisible && FrontEndWidget.IsValid();
		LogSmokeTestStep(TEXT("Title Screen -> Start"), bFrontEndReady, TEXT("Title screen must appear before the automated flow starts."));
		if (!bFrontEndReady)
		{
			FinishSmokeTest(false, TEXT("Title screen did not initialize."));
			return;
		}

		CaptureSmokeScreenshot(TEXT("TitleScreen"));
		GetWorldTimerManager().SetTimer(SmokeStepTimer, this, &ThisClass::AdvanceSmokeTest, 0.40f, false);
		return;
	}
	case 1:
	{
		StartGameFromFrontEnd();
		GetWorldTimerManager().SetTimer(SmokeStepTimer, this, &ThisClass::AdvanceSmokeTest, 0.45f, false);
		return;
	}
	case 2:
	{
		const bool bCreatorReady = bCharacterCreatorVisible && CharacterCreatorWidget.IsValid() && CharacterPreviewActor != nullptr;
		LogSmokeTestStep(TEXT("Open Character Creator"), bCreatorReady, TEXT("Creator should open with a live preview actor."));
		if (!bCreatorReady)
		{
			FinishSmokeTest(false, TEXT("Character creator failed to open."));
			return;
		}

		FFantasyFrontierCharacterDraft Draft = CharacterCreatorWidget->GetDraft();
		Draft.CharacterName = TEXT("SmokeRunner");
		Draft.Gender = EFantasyFrontierGender::Female;
		CharacterCreatorWidget->SetDraft(Draft);
		CharacterCreatorWidget->SetCurrentStepForSmokeTest(EFantasyFrontierCreatorStep::Gender);
		LogSmokeTestStep(TEXT("Switch Female"), CharacterCreatorWidget->GetDraft().Gender == EFantasyFrontierGender::Female);
		CaptureSmokeScreenshot(TEXT("Creator_RaceGender"));
		GetWorldTimerManager().SetTimer(SmokeStepTimer, this, &ThisClass::AdvanceSmokeTest, 0.25f, false);
		return;
	}
	case 3:
	{
		if (!CharacterCreatorWidget.IsValid())
		{
			FinishSmokeTest(false, TEXT("Character creator was lost before gender swap."));
			return;
		}

		FFantasyFrontierCharacterDraft Draft = CharacterCreatorWidget->GetDraft();
		Draft.Gender = EFantasyFrontierGender::Male;
		CharacterCreatorWidget->SetDraft(Draft);
		LogSmokeTestStep(TEXT("Switch Male"), CharacterCreatorWidget->GetDraft().Gender == EFantasyFrontierGender::Male);
		GetWorldTimerManager().SetTimer(SmokeStepTimer, this, &ThisClass::AdvanceSmokeTest, 0.25f, false);
		return;
	}
	case 4:
	{
		if (!CharacterCreatorWidget.IsValid())
		{
			FinishSmokeTest(false, TEXT("Character creator was lost before preview-mode test."));
			return;
		}

		FFantasyFrontierCharacterDraft Draft = CharacterCreatorWidget->GetDraft();
		Draft.PreviewMode = EFantasyFrontierPreviewMode::BaseBody;
		CharacterCreatorWidget->SetDraft(Draft);
		Draft.PreviewMode = EFantasyFrontierPreviewMode::StarterGear;
		CharacterCreatorWidget->SetDraft(Draft);
		Draft.PreviewMode = EFantasyFrontierPreviewMode::OriginStyle;
		CharacterCreatorWidget->SetDraft(Draft);
		Draft.PreviewMode = EFantasyFrontierPreviewMode::BaseBody;
		CharacterCreatorWidget->SetDraft(Draft);

		const bool bPreviewModesStable = CharacterCreatorWidget->GetDraft().PreviewMode == EFantasyFrontierPreviewMode::BaseBody;
		LogSmokeTestStep(TEXT("Switch Preview Modes"), bPreviewModesStable);
		if (!bPreviewModesStable)
		{
			FinishSmokeTest(false, TEXT("Preview modes did not round-trip correctly."));
			return;
		}

		GetWorldTimerManager().SetTimer(SmokeStepTimer, this, &ThisClass::AdvanceSmokeTest, 0.25f, false);
		return;
	}
	case 5:
	{
		if (!CharacterCreatorWidget.IsValid())
		{
			FinishSmokeTest(false, TEXT("Character creator was lost before preset save/load."));
			return;
		}

		FFantasyFrontierCharacterDraft Draft = CharacterCreatorWidget->GetDraft();
		Draft.CharacterName = TEXT("SmokeRunner");
		Draft.Gender = EFantasyFrontierGender::Female;
		Draft.HairStyle = 1;
		Draft.FaceVariant = 2;
		CharacterCreatorWidget->SetDraft(Draft);
		SaveAppearancePreset();

		Draft.Gender = EFantasyFrontierGender::Male;
		Draft.HairStyle = 2;
		Draft.FaceVariant = 0;
		CharacterCreatorWidget->SetDraft(Draft);
		LoadAppearancePreset();
		CharacterCreatorWidget->SetCurrentStepForSmokeTest(EFantasyFrontierCreatorStep::Appearance);

		const FFantasyFrontierCharacterDraft LoadedDraft = CharacterCreatorWidget->GetDraft();
		const bool bPresetRoundTrip =
			LoadedDraft.CharacterName == TEXT("SmokeRunner") &&
			LoadedDraft.Gender == EFantasyFrontierGender::Female &&
			LoadedDraft.HairStyle == 1 &&
			LoadedDraft.FaceVariant == 2;

		LogSmokeTestStep(TEXT("Save and Load Preset"), bPresetRoundTrip);
		if (!bPresetRoundTrip)
		{
			FinishSmokeTest(false, TEXT("Appearance preset did not reload the saved draft."));
			return;
		}

		CaptureSmokeScreenshot(TEXT("Creator_Appearance"));
		GetWorldTimerManager().SetTimer(SmokeStepTimer, this, &ThisClass::AdvanceSmokeTest, 0.25f, false);
		return;
	}
	case 6:
	{
		if (!CharacterCreatorWidget.IsValid())
		{
			FinishSmokeTest(false, TEXT("Character creator was lost before game start."));
			return;
		}

		FFantasyFrontierCharacterDraft Draft = CharacterCreatorWidget->GetDraft();
		Draft.CharacterName = TEXT("SmokeRunner");
		Draft.CharacterClass = EFantasyFrontierClass::Lancer;
		CommitCharacterCreator(Draft);
		LogSmokeTestStep(TEXT("Start Game"), !bCharacterCreatorVisible && !bFrontEndVisible);
		GetWorldTimerManager().SetTimer(SmokeStepTimer, this, &ThisClass::AdvanceSmokeTest, 0.75f, false);
		return;
	}
	case 7:
	{
		AFantasyFrontierPlayableCharacter* PlayerCharacter = GetSmokePlayerCharacter();
		AFantasyFrontierTutorialDirector* TutorialDirector = GetSmokeTutorialDirector();
		const bool bSpawnedIntoTutorial = PlayerCharacter != nullptr && TutorialDirector != nullptr;
		LogSmokeTestStep(TEXT("Spawn into Tutorial"), bSpawnedIntoTutorial);
		if (!bSpawnedIntoTutorial)
		{
			FinishSmokeTest(false, TEXT("Player or tutorial director did not exist after creator confirm."));
			return;
		}

		CaptureSmokeScreenshot(TEXT("Ingame_Spawn"));
		SmokeMovementStart = PlayerCharacter->GetActorLocation();
		SmokeMoveTicksRemaining = 16;
		GetWorldTimerManager().SetTimer(SmokeMoveTimer, this, &ThisClass::RunSmokeMovementPulse, 0.05f, true);
		GetWorldTimerManager().SetTimer(SmokeStepTimer, this, &ThisClass::AdvanceSmokeTest, 1.10f, false);
		return;
	}
	case 8:
	{
		GetWorldTimerManager().ClearTimer(SmokeMoveTimer);

		AFantasyFrontierPlayableCharacter* PlayerCharacter = GetSmokePlayerCharacter();
		if (!PlayerCharacter)
		{
			FinishSmokeTest(false, TEXT("Player disappeared before movement check."));
			return;
		}

		const bool bMoved = FVector::Dist2D(SmokeMovementStart, PlayerCharacter->GetActorLocation()) > 55.0f;
		LogSmokeTestStep(TEXT("Move Character"), bMoved);
		if (!bMoved)
		{
			FinishSmokeTest(false, TEXT("Character failed to move during smoke test."));
			return;
		}

		SmokeActionStaminaBefore = PlayerCharacter->GetCurrentStamina();
		PlayerCharacter->TriggerDashForSmokeTest();
		PlayerCharacter->TriggerLightAttackForSmokeTest();
		PlayerCharacter->TriggerHeavyAttackForSmokeTest();
		GetWorldTimerManager().SetTimer(SmokeStepTimer, this, &ThisClass::AdvanceSmokeTest, 0.75f, false);
		return;
	}
	case 9:
	{
		AFantasyFrontierPlayableCharacter* PlayerCharacter = GetSmokePlayerCharacter();
		if (!PlayerCharacter)
		{
			FinishSmokeTest(false, TEXT("Player disappeared before action check."));
			return;
		}

		const bool bActionsConsumedStamina = PlayerCharacter->GetCurrentStamina() < SmokeActionStaminaBefore;
		LogSmokeTestStep(TEXT("Dash + Light/Heavy Attack"), bActionsConsumedStamina);
		if (!bActionsConsumedStamina)
		{
			FinishSmokeTest(false, TEXT("Dash/light/heavy actions did not execute."));
			return;
		}

		if (AFantasyFrontierFunctionalNpc* GuideNpc = FindNpcByRole(static_cast<uint8>(EFantasyFrontierNpcRole::Guide)))
		{
			PlayerCharacter->SetActorLocation(GuideNpc->GetActorLocation() + FVector(0.0f, 0.0f, 25.0f));
		}
		else
		{
			FinishSmokeTest(false, TEXT("Guide NPC was missing from the tutorial slice."));
			return;
		}

		GetWorldTimerManager().SetTimer(SmokeStepTimer, this, &ThisClass::AdvanceSmokeTest, 0.50f, false);
		return;
	}
	case 10:
	{
		AFantasyFrontierPlayableCharacter* PlayerCharacter = GetSmokePlayerCharacter();
		AFantasyFrontierTutorialDirector* TutorialDirector = GetSmokeTutorialDirector();
		const bool bNpcInteractionWorked = TutorialDirector != nullptr && TutorialDirector->HasGuideVisited();
		LogSmokeTestStep(TEXT("Interact With NPC"), bNpcInteractionWorked);
		if (!bNpcInteractionWorked || !PlayerCharacter)
		{
			FinishSmokeTest(false, TEXT("NPC overlap did not register with the tutorial director."));
			return;
		}

		SmokeTrackedEnemy = FindFirstEnemy();
		if (!SmokeTrackedEnemy.IsValid())
		{
			FinishSmokeTest(false, TEXT("No tutorial enemy was available for the combat check."));
			return;
		}

		const FVector EnemyLocation = SmokeTrackedEnemy->GetActorLocation();
		PlayerCharacter->SetActorLocation(EnemyLocation + FVector(-130.0f, 0.0f, 0.0f));
		PlayerCharacter->SetActorRotation((EnemyLocation - PlayerCharacter->GetActorLocation()).Rotation());
		PlayerCharacter->TriggerLightAttackForSmokeTest();
		GetWorldTimerManager().SetTimer(SmokeStepTimer, this, &ThisClass::AdvanceSmokeTest, 1.10f, false);
		return;
	}
	case 11:
	{
		const bool bCombatTriggered = SmokeTrackedEnemy.IsValid() && SmokeTrackedEnemy->HasEngagedTarget();
		LogSmokeTestStep(TEXT("Trigger Combat With Enemy"), bCombatTriggered);
		if (!bCombatTriggered)
		{
			FinishSmokeTest(false, TEXT("Enemy never entered combat state."));
			return;
		}

		FinishSmokeTest(true);
		return;
	}
	default:
		return;
	}
}

void ATP_ThirdPersonPlayerController::RunSmokeMovementPulse()
{
	AFantasyFrontierPlayableCharacter* PlayerCharacter = GetSmokePlayerCharacter();
	if (!PlayerCharacter)
	{
		GetWorldTimerManager().ClearTimer(SmokeMoveTimer);
		return;
	}

	PlayerCharacter->TriggerMovementPulseForSmokeTest();
	--SmokeMoveTicksRemaining;
	if (SmokeMoveTicksRemaining <= 0)
	{
		GetWorldTimerManager().ClearTimer(SmokeMoveTimer);
	}
}

void ATP_ThirdPersonPlayerController::FinishSmokeTest(bool bSuccess, const FString& FailureReason)
{
	if (bSmokeTestComplete)
	{
		return;
	}

	bSmokeTestComplete = true;
	GetWorldTimerManager().ClearTimer(SmokeStepTimer);
	GetWorldTimerManager().ClearTimer(SmokeMoveTimer);

	if (bSuccess)
	{
		UE_LOG(LogTP_ThirdPerson, Display, TEXT("FFSmoke PASS"));
	}
	else
	{
		UE_LOG(LogTP_ThirdPerson, Error, TEXT("FFSmoke FAIL: %s"), FailureReason.IsEmpty() ? TEXT("Unknown failure.") : *FailureReason);
	}

	GetWorldTimerManager().SetTimer(SmokeExitTimer, this, &ThisClass::RequestSmokeTestExit, 0.75f, false);
}

void ATP_ThirdPersonPlayerController::RequestSmokeTestExit()
{
	ConsoleCommand(TEXT("quit"));
}

void ATP_ThirdPersonPlayerController::CaptureSmokeScreenshot(const FString& Label)
{
	if (!bRunSmokeTest || !bCaptureSmokeScreenshots)
	{
		return;
	}

	const FString CaptureDir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("FFSmokeCaptures"), SmokeScreenshotPrefix);
	IFileManager::Get().MakeDirectory(*CaptureDir, true);
	const FString Filename = FString::Printf(TEXT("%02d_%s.png"), SmokeScreenshotIndex++, *Label);
	const FString AbsolutePath = FPaths::Combine(CaptureDir, Filename);
	FScreenshotRequest::RequestScreenshot(AbsolutePath, true, false, false);
}

void ATP_ThirdPersonPlayerController::LogSmokeTestStep(const FString& StepLabel, bool bPassed, const FString& Details) const
{
	if (Details.IsEmpty())
	{
		UE_LOG(LogTP_ThirdPerson, Display, TEXT("FFSmoke %s: %s"), bPassed ? TEXT("PASS") : TEXT("FAIL"), *StepLabel);
		return;
	}

	UE_LOG(LogTP_ThirdPerson, Display, TEXT("FFSmoke %s: %s -- %s"), bPassed ? TEXT("PASS") : TEXT("FAIL"), *StepLabel, *Details);
}

AFantasyFrontierPlayableCharacter* ATP_ThirdPersonPlayerController::GetSmokePlayerCharacter() const
{
	return Cast<AFantasyFrontierPlayableCharacter>(GetPawn());
}

AFantasyFrontierPlayableCharacter* ATP_ThirdPersonPlayerController::EnsurePlayableCharacterPawn()
{
	if (AFantasyFrontierPlayableCharacter* ExistingPlayableCharacter = GetSmokePlayerCharacter())
	{
		return ExistingPlayableCharacter;
	}

	if (!GetWorld())
	{
		return nullptr;
	}

	APawn* ExistingPawn = GetPawn();
	FVector DesiredSpawnLocation = FVector::ZeroVector;
	FRotator DesiredSpawnRotation = FRotator::ZeroRotator;

	if (ExistingPawn)
	{
		DesiredSpawnLocation = ExistingPawn->GetActorLocation();
		DesiredSpawnRotation = ExistingPawn->GetActorRotation();
	}
	else if (APlayerStart* PlayerStart = Cast<APlayerStart>(UGameplayStatics::GetActorOfClass(this, APlayerStart::StaticClass())))
	{
		DesiredSpawnLocation = PlayerStart->GetActorLocation();
		DesiredSpawnRotation = PlayerStart->GetActorRotation();
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	AFantasyFrontierPlayableCharacter* SpawnedCharacter = GetWorld()->SpawnActor<AFantasyFrontierPlayableCharacter>(
		AFantasyFrontierPlayableCharacter::StaticClass(),
		DesiredSpawnLocation,
		DesiredSpawnRotation,
		SpawnParameters);

	if (!SpawnedCharacter)
	{
		return nullptr;
	}

	Possess(SpawnedCharacter);
	if (ExistingPawn)
	{
		ExistingPawn->Destroy();
	}

	return SpawnedCharacter;
}

AFantasyFrontierTutorialDirector* ATP_ThirdPersonPlayerController::GetSmokeTutorialDirector() const
{
	return GetWorld() ? Cast<AFantasyFrontierTutorialDirector>(UGameplayStatics::GetActorOfClass(GetWorld(), AFantasyFrontierTutorialDirector::StaticClass())) : nullptr;
}

AFantasyFrontierTutorialDirector* ATP_ThirdPersonPlayerController::EnsureTutorialDirector() const
{
	if (AFantasyFrontierTutorialDirector* ExistingDirector = GetSmokeTutorialDirector())
	{
		return ExistingDirector;
	}

	if (!GetWorld())
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = const_cast<ATP_ThirdPersonPlayerController*>(this);
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	return GetWorld()->SpawnActor<AFantasyFrontierTutorialDirector>(
		AFantasyFrontierTutorialDirector::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParameters);
}

AFantasyFrontierFunctionalNpc* ATP_ThirdPersonPlayerController::FindNpcByRole(uint8 RoleValue) const
{
	if (!GetWorld())
	{
		return nullptr;
	}

	for (TActorIterator<AFantasyFrontierFunctionalNpc> It(GetWorld()); It; ++It)
	{
		if (static_cast<uint8>(It->GetNpcRole()) == RoleValue)
		{
			return *It;
		}
	}

	return nullptr;
}

AFantasyFrontierEnemyBase* ATP_ThirdPersonPlayerController::FindFirstEnemy() const
{
	if (!GetWorld())
	{
		return nullptr;
	}

	for (TActorIterator<AFantasyFrontierEnemyBase> It(GetWorld()); It; ++It)
	{
		return *It;
	}

	return nullptr;
}

void ATP_ThirdPersonPlayerController::ApplyAndSaveUserSettings() const
{
	if (GEngine)
	{
		if (UGameUserSettings* UserSettings = GEngine->GetGameUserSettings())
		{
			UserSettings->ApplySettings(false);
			UserSettings->SaveSettings();
		}
	}
}
