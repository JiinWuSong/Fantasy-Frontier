#pragma once

#include "Animation/CurveSequence.h"
#include "Brushes/SlateColorBrush.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"
#include "Input/Events.h"
#include "Input/Reply.h"
#include "Styling/SlateTypes.h"
#include "Widgets/SCompoundWidget.h"

struct FSlateDynamicImageBrush;

class SFantasyFrontierFrontEndWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFantasyFrontierFrontEndWidget)
		: _WindowModeLabel(FText::GetEmpty())
		, _QualityLabel(FText::GetEmpty())
		, _ShadowQualityLabel(FText::GetEmpty())
		, _AntiAliasingLabel(FText::GetEmpty())
		, _PostProcessLabel(FText::GetEmpty())
		, _ViewDistanceLabel(FText::GetEmpty())
	{
	}
		SLATE_EVENT(FSimpleDelegate, OnStartGame)
		SLATE_EVENT(FSimpleDelegate, OnQuit)
		SLATE_EVENT(FSimpleDelegate, OnCycleWindowMode)
		SLATE_EVENT(FSimpleDelegate, OnCycleQuality)
		SLATE_EVENT(FSimpleDelegate, OnCycleShadowQuality)
		SLATE_EVENT(FSimpleDelegate, OnCycleAntiAliasingQuality)
		SLATE_EVENT(FSimpleDelegate, OnCyclePostProcessQuality)
		SLATE_EVENT(FSimpleDelegate, OnCycleViewDistanceQuality)
		SLATE_ATTRIBUTE(FText, WindowModeLabel)
		SLATE_ATTRIBUTE(FText, QualityLabel)
		SLATE_ATTRIBUTE(FText, ShadowQualityLabel)
		SLATE_ATTRIBUTE(FText, AntiAliasingLabel)
		SLATE_ATTRIBUTE(FText, PostProcessLabel)
		SLATE_ATTRIBUTE(FText, ViewDistanceLabel)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SFantasyFrontierFrontEndWidget() override = default;
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	virtual bool SupportsKeyboardFocus() const override { return true; }
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

private:
	void ResetIdleTimer();
	FReply HandleStartGame();
	FReply HandleQuit();
	FReply HandleShowOptions();
	FReply HandleCloseOptions();
	FReply HandleCycleWindowMode();
	FReply HandleCycleQuality();
	FReply HandleCycleShadowQuality();
	FReply HandleCycleAntiAliasingQuality();
	FReply HandleCyclePostProcessQuality();
	FReply HandleCycleViewDistanceQuality();
	EVisibility GetMainMenuVisibility() const;
	EVisibility GetOptionsVisibility() const;
	FSlateFontInfo MakeTitleFont() const;
	FSlateFontInfo MakeBodyFont(int32 Size) const;
	TOptional<FSlateRenderTransform> GetSkyTransform() const;
	TOptional<FSlateRenderTransform> GetMidgroundTransform() const;
	TOptional<FSlateRenderTransform> GetMistTransform() const;
	TOptional<FSlateRenderTransform> GetForegroundTransform() const;
	TOptional<FSlateRenderTransform> GetTitleTransform() const;
	TOptional<FSlateRenderTransform> GetPanelTransform() const;
	float GetTitleOpacity() const;
	float GetMenuOpacity() const;
	float GetMainMenuOpacity() const;
	float GetOptionsOpacity() const;
	float GetShowcaseCaptionOpacity() const;
	const FSlateBrush* ResolveBrush(const TSharedPtr<FSlateDynamicImageBrush>& Brush, const FSlateBrush* Fallback) const;
	TSharedRef<SWidget> BuildMenuFrame();
	TSharedRef<SWidget> BuildMainMenu();
	TSharedRef<SWidget> BuildOptionsMenu();

	FSimpleDelegate OnStartGame;
	FSimpleDelegate OnQuit;
	FSimpleDelegate OnCycleWindowMode;
	FSimpleDelegate OnCycleQuality;
	FSimpleDelegate OnCycleShadowQuality;
	FSimpleDelegate OnCycleAntiAliasingQuality;
	FSimpleDelegate OnCyclePostProcessQuality;
	FSimpleDelegate OnCycleViewDistanceQuality;
	TAttribute<FText> WindowModeLabel;
	TAttribute<FText> QualityLabel;
	TAttribute<FText> ShadowQualityLabel;
	TAttribute<FText> AntiAliasingLabel;
	TAttribute<FText> PostProcessLabel;
	TAttribute<FText> ViewDistanceLabel;
	TSharedPtr<FSlateDynamicImageBrush> SkyBrush;
	TSharedPtr<FSlateDynamicImageBrush> MidgroundBrush;
	TSharedPtr<FSlateDynamicImageBrush> MistBrush;
	TSharedPtr<FSlateDynamicImageBrush> ForegroundBrush;
	TSharedPtr<FSlateDynamicImageBrush> TitleLogoBrush;
	TSharedPtr<FSlateDynamicImageBrush> DividerBrush;
	TSharedPtr<FSlateDynamicImageBrush> PanelBrush;
	TSharedPtr<FSlateDynamicImageBrush> ButtonNormalBrush;
	TSharedPtr<FSlateDynamicImageBrush> ButtonHoveredBrush;
	TSharedPtr<FSlateDynamicImageBrush> ButtonPressedBrush;
	FButtonStyle MenuButtonStyle;
	FButtonStyle TextMenuButtonStyle;
	FSlateColorBrush ScreenTintBrush = FSlateColorBrush(FLinearColor(1.0f, 0.97f, 0.88f, 0.06f));
	FSlateRoundedBoxBrush MenuPanelFallbackBrush = FSlateRoundedBoxBrush(FLinearColor(0.05f, 0.11f, 0.17f, 0.84f), 34.0f, FLinearColor(0.72f, 0.60f, 0.34f, 0.28f), 1.4f);
	FSlateRoundedBoxBrush MenuChipBrush = FSlateRoundedBoxBrush(FLinearColor(0.09f, 0.17f, 0.24f, 0.64f), 22.0f, FLinearColor(0.84f, 0.72f, 0.44f, 0.18f), 1.0f);
	FSlateRoundedBoxBrush MenuShadowBrush = FSlateRoundedBoxBrush(FLinearColor(0.02f, 0.04f, 0.06f, 0.32f), 38.0f);
	FSlateColorBrush DividerFallbackBrush = FSlateColorBrush(FLinearColor(0.80f, 0.66f, 0.34f, 0.95f));
	FCurveSequence IntroSequence;
	FCurveHandle TitleRevealCurve;
	FCurveHandle MenuRevealCurve;
	double AmbientTime = 0.0;
	double LastInteractionTime = 0.0;
	float ShowcaseBlend = 0.0f;
	float IdleShowcaseDelay = 12.5f;
	bool bShowingOptions = false;
};
