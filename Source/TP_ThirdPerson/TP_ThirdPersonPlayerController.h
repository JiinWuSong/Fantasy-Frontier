// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TP_ThirdPersonPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;
class SFantasyFrontierFrontEndWidget;

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
	void ShowFrontEnd();
	void HideFrontEnd();
	void ApplyFrontEndInputState(bool bFrontEndEnabled);
	void StartGameFromFrontEnd();
	void QuitFromFrontEnd();
	void CycleWindowMode();
	void CycleQualityLevel();
	FText GetWindowModeText() const;
	FText GetQualityLevelText() const;

	TSharedPtr<SFantasyFrontierFrontEndWidget> FrontEndWidget;
	bool bFrontEndVisible = false;

};
