// Copyright Epic Games, Inc. All Rights Reserved.


#include "TP_ThirdPersonPlayerController.h"
#include "FantasyFrontierFrontEndWidget.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/GameUserSettings.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/Pawn.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TP_ThirdPerson.h"
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
}

void ATP_ThirdPersonPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
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
		.WindowModeLabel_Lambda([this]() { return GetWindowModeText(); })
		.QualityLabel_Lambda([this]() { return GetQualityLevelText(); });

	GEngine->GameViewport->AddViewportWidgetContent(FrontEndWidget.ToSharedRef(), 100);
	bFrontEndVisible = true;
	ApplyFrontEndInputState(true);
}

void ATP_ThirdPersonPlayerController::HideFrontEnd()
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
}

void ATP_ThirdPersonPlayerController::ApplyFrontEndInputState(bool bFrontEndEnabled)
{
	bShowMouseCursor = bFrontEndEnabled;
	bEnableClickEvents = bFrontEndEnabled;
	bEnableMouseOverEvents = bFrontEndEnabled;
	SetIgnoreMoveInput(bFrontEndEnabled);
	SetIgnoreLookInput(bFrontEndEnabled);

	if (bFrontEndEnabled)
	{
		FInputModeUIOnly InputMode;
		if (FrontEndWidget.IsValid())
		{
			InputMode.SetWidgetToFocus(FrontEndWidget);
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

void ATP_ThirdPersonPlayerController::StartGameFromFrontEnd()
{
	if (!bFrontEndVisible)
	{
		return;
	}

	HideFrontEnd();
	ApplyFrontEndInputState(false);
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
	UserSettings->ApplySettings(false);
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
	UserSettings->ApplySettings(false);
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
		return FText::FromString(TEXT("Vollbild"));
	case EWindowMode::Windowed:
		return FText::FromString(TEXT("Fenster"));
	case EWindowMode::WindowedFullscreen:
	default:
		return FText::FromString(TEXT("Randlos"));
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
		return FText::FromString(TEXT("Niedrig"));
	case 1:
		return FText::FromString(TEXT("Mittel"));
	case 2:
		return FText::FromString(TEXT("Hoch"));
	case 3:
		return FText::FromString(TEXT("Episch"));
	case 4:
		return FText::FromString(TEXT("Kino"));
	default:
		return FText::FromString(TEXT("Benutzerdefiniert"));
	}
}
