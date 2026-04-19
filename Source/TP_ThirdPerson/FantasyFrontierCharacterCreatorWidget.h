// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Brushes/SlateRoundedBoxBrush.h"
#include "Brushes/SlateColorBrush.h"
#include "CoreMinimal.h"
#include "FantasyFrontierCharacterCreatorTypes.h"
#include "Fonts/SlateFontInfo.h"
#include "Input/Reply.h"
#include "Styling/SlateTypes.h"
#include "Widgets/SCompoundWidget.h"

class UTextureRenderTarget2D;
struct FSlateDynamicImageBrush;

DECLARE_DELEGATE_OneParam(FFantasyFrontierCharacterDraftChanged, FFantasyFrontierCharacterDraft);
DECLARE_DELEGATE_OneParam(FFantasyFrontierCharacterDraftCommitted, FFantasyFrontierCharacterDraft);
DECLARE_DELEGATE_OneParam(FFantasyFrontierPreviewRotate, float);
DECLARE_DELEGATE_OneParam(FFantasyFrontierPreviewZoom, float);

class SFantasyFrontierCharacterCreatorWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFantasyFrontierCharacterCreatorWidget)
		: _PreviewTexture(nullptr)
	{
	}
		SLATE_ARGUMENT(UTextureRenderTarget2D*, PreviewTexture)
		SLATE_EVENT(FFantasyFrontierCharacterDraftChanged, OnDraftChanged)
		SLATE_EVENT(FFantasyFrontierCharacterDraftCommitted, OnConfirm)
		SLATE_EVENT(FFantasyFrontierPreviewRotate, OnPreviewRotate)
		SLATE_EVENT(FFantasyFrontierPreviewZoom, OnPreviewZoom)
		SLATE_EVENT(FSimpleDelegate, OnSavePreset)
		SLATE_EVENT(FSimpleDelegate, OnLoadPreset)
		SLATE_EVENT(FSimpleDelegate, OnCancel)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual bool SupportsKeyboardFocus() const override { return true; }
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	void SetDraft(const FFantasyFrontierCharacterDraft& InDraft);
	const FFantasyFrontierCharacterDraft& GetDraft() const { return Draft; }
	void SetCurrentStepForSmokeTest(EFantasyFrontierCreatorStep InStep) { CurrentStep = InStep; }
	EFantasyFrontierCreatorStep GetCurrentStepForSmokeTest() const { return CurrentStep; }

private:
	void BroadcastDraftChanged();
	FReply HandleNextStep();
	FReply HandlePreviousStep();
	FReply HandleConfirm();
	FReply HandleSelectRace(EFantasyFrontierRace Race);
	FReply HandleSelectGender(EFantasyFrontierGender Gender);
	FReply HandleSelectClass(EFantasyFrontierClass CharacterClass);
	FReply HandleSelectCustomizationTab(EFantasyFrontierCustomizationTab Tab);
	FReply HandleSelectStylePreset(int32 StylePreset);
	void HandleHeightChanged(float NewValue);
	void HandleBuildChanged(float NewValue);
	void HandleMusculatureChanged(float NewValue);
	void HandleSkinToneChanged(float NewValue);
	void HandleScarChanged(float NewValue);
	void HandleTattooChanged(float NewValue);
	void HandleNameChanged(const FText& NewValue);
	void HandlePreviewRotated(float YawDelta);
	void HandlePreviewZoomed(float ZoomDelta);
	FReply HandleSetPreviewMode(EFantasyFrontierPreviewMode PreviewMode);
	FReply HandleSetOriginStyle(EFantasyFrontierOriginStyle OriginStyle);
	FReply HandleSetHairStyle(int32 HairStyle);
	FReply HandleSetFaceVariant(int32 FaceVariant);
	FReply HandleCycleHairColor();
	FReply HandleCycleEyeColor();
	FReply HandleSavePreset();
	FReply HandleLoadPreset();
	bool IsRaceEnabled(EFantasyFrontierRace Race) const;
	bool IsNameValid() const;
	bool CanAdvanceFromCurrentStep() const;
	bool CanFinish() const;
	int32 GetStepIndex() const;
	int32 GetCustomizationTabIndex() const;
	bool ShouldShowRaceRail() const;
	FText GetStepTitle() const;
	FText GetStepBody() const;
	FText GetStepProgressText() const;
	FText GetNextButtonText() const;
	FText GetConfirmButtonText() const;
	FText GetBackButtonText() const;
	FText GetRaceLabel(EFantasyFrontierRace Race) const;
	FText GetGenderLabel(EFantasyFrontierGender Gender) const;
	FText GetClassLabel(EFantasyFrontierClass CharacterClass) const;
	FText GetDraftHeadline() const;
	FText GetDraftSubtitle() const;
	FText GetPreviewFrameLabel() const;
	FText GetNameValidationText() const;
	FText GetHeightValueText() const;
	FText GetBuildValueText() const;
	FText GetMusculatureValueText() const;
	FText GetSkinToneValueText() const;
	FText GetScarValueText() const;
	FText GetTattooValueText() const;
	FSlateColor GetRaceButtonTint(EFantasyFrontierRace Race) const;
	FSlateColor GetGenderButtonTint(EFantasyFrontierGender Gender) const;
	FSlateColor GetClassButtonTint(EFantasyFrontierClass CharacterClass) const;
	FSlateColor GetStyleButtonTint(int32 StylePreset) const;
	FSlateColor GetTabTint(EFantasyFrontierCustomizationTab Tab) const;
	FSlateColor GetPreviewModeTint(EFantasyFrontierPreviewMode PreviewMode) const;
	FSlateColor GetOriginStyleTint(EFantasyFrontierOriginStyle OriginStyle) const;
	FSlateColor GetHairStyleTint(int32 HairStyle) const;
	FSlateColor GetFaceVariantTint(int32 FaceVariant) const;
	FSlateFontInfo MakeDisplayFont(int32 Size, int32 LetterSpacing = 0) const;
	FSlateFontInfo MakeBodyFont(int32 Size) const;
	TOptional<FSlateRenderTransform> GetSkyTransform() const;
	TOptional<FSlateRenderTransform> GetMidgroundTransform() const;
	TOptional<FSlateRenderTransform> GetMistTransform() const;
	TOptional<FSlateRenderTransform> GetForegroundTransform() const;
	TOptional<FSlateRenderTransform> GetHeroTransform() const;
	TSharedRef<SWidget> BuildRaceStep();
	TSharedRef<SWidget> BuildRaceRail();
	TSharedRef<SWidget> BuildPreviewPane();
	TSharedRef<SWidget> BuildStepPane();
	TSharedRef<SWidget> BuildGenderStep();
	TSharedRef<SWidget> BuildAppearanceStep();
	TSharedRef<SWidget> BuildNameStep();
	TSharedRef<SWidget> BuildClassStep();
	TSharedRef<SWidget> BuildBodyTab();
	TSharedRef<SWidget> BuildHairStyleTab();
	TSharedRef<SWidget> BuildFaceTab();
	TSharedRef<SWidget> BuildMarkingsTab();
	TSharedRef<SWidget> BuildFooter();
	TSharedRef<SWidget> BuildStatLine(const FText& Label, TAttribute<FText> Value) const;

	FFantasyFrontierCharacterDraftChanged OnDraftChanged;
	FFantasyFrontierCharacterDraftCommitted OnConfirm;
	FFantasyFrontierPreviewRotate OnPreviewRotate;
	FFantasyFrontierPreviewZoom OnPreviewZoom;
	FSimpleDelegate OnSavePreset;
	FSimpleDelegate OnLoadPreset;
	FSimpleDelegate OnCancel;
	FFantasyFrontierCharacterDraft Draft;
	EFantasyFrontierCreatorStep CurrentStep = EFantasyFrontierCreatorStep::Race;
	EFantasyFrontierCustomizationTab CurrentTab = EFantasyFrontierCustomizationTab::Body;
	FSlateBrush PreviewBrush;
	TSharedPtr<FSlateDynamicImageBrush> BackgroundBrush;
	TSharedPtr<FSlateDynamicImageBrush> SkyBrush;
	TSharedPtr<FSlateDynamicImageBrush> MidgroundBrush;
	TSharedPtr<FSlateDynamicImageBrush> MistBrush;
	TSharedPtr<FSlateDynamicImageBrush> ForegroundBrush;
	TSharedPtr<FSlateDynamicImageBrush> PanelTextureBrush;
	TSharedPtr<FSlateDynamicImageBrush> DividerTextureBrush;
	FSlateRoundedBoxBrush PanelBrush = FSlateRoundedBoxBrush(FLinearColor(0.05f, 0.11f, 0.16f, 0.82f), 30.0f, FLinearColor(0.82f, 0.70f, 0.42f, 0.14f), 1.0f);
	FSlateRoundedBoxBrush RailBrush = FSlateRoundedBoxBrush(FLinearColor(0.03f, 0.08f, 0.12f, 0.88f), 26.0f, FLinearColor(0.82f, 0.70f, 0.42f, 0.12f), 1.0f);
	FSlateRoundedBoxBrush PreviewFrameBrush = FSlateRoundedBoxBrush(FLinearColor(0.10f, 0.16f, 0.21f, 0.64f), 24.0f, FLinearColor(0.84f, 0.73f, 0.48f, 0.12f), 1.0f);
	FSlateRoundedBoxBrush GlassCardBrush = FSlateRoundedBoxBrush(FLinearColor(0.05f, 0.10f, 0.15f, 0.58f), 28.0f, FLinearColor(0.84f, 0.73f, 0.48f, 0.16f), 1.0f);
	FSlateColorBrush DividerBrush = FSlateColorBrush(FLinearColor(0.88f, 0.76f, 0.48f, 0.90f));
	FSlateColorBrush HeroWashBrush = FSlateColorBrush(FLinearColor(0.02f, 0.06f, 0.09f, 0.46f));
	FSlateColorBrush BackdropTintBrush = FSlateColorBrush(FLinearColor(0.02f, 0.06f, 0.09f, 0.72f));
	FSlateColorBrush BackgroundFallbackBrush = FSlateColorBrush(FLinearColor(0.10f, 0.18f, 0.22f, 1.0f));
	FButtonStyle PrimaryButtonStyle;
	FButtonStyle SecondaryButtonStyle;
	FButtonStyle CardButtonStyle;
	double AmbientTime = 0.0;
};
