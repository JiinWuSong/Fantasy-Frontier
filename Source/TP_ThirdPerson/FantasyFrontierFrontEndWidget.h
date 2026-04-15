#pragma once

#include "Animation/CurveSequence.h"
#include "Brushes/SlateColorBrush.h"
#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"
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
	{
	}
		SLATE_EVENT(FSimpleDelegate, OnStartGame)
		SLATE_EVENT(FSimpleDelegate, OnQuit)
		SLATE_EVENT(FSimpleDelegate, OnCycleWindowMode)
		SLATE_EVENT(FSimpleDelegate, OnCycleQuality)
		SLATE_ATTRIBUTE(FText, WindowModeLabel)
		SLATE_ATTRIBUTE(FText, QualityLabel)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SFantasyFrontierFrontEndWidget() override = default;
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	FReply HandleStartGame();
	FReply HandleQuit();
	FReply HandleShowOptions();
	FReply HandleCloseOptions();
	FReply HandleCycleWindowMode();
	FReply HandleCycleQuality();
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
	const FSlateBrush* ResolveBrush(const TSharedPtr<FSlateDynamicImageBrush>& Brush, const FSlateBrush* Fallback) const;
	TSharedRef<SWidget> BuildMenuFrame();
	TSharedRef<SWidget> BuildMainMenu();
	TSharedRef<SWidget> BuildOptionsMenu();

	FSimpleDelegate OnStartGame;
	FSimpleDelegate OnQuit;
	FSimpleDelegate OnCycleWindowMode;
	FSimpleDelegate OnCycleQuality;
	TAttribute<FText> WindowModeLabel;
	TAttribute<FText> QualityLabel;
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
	FSlateColorBrush ScreenTintBrush = FSlateColorBrush(FLinearColor(0.01f, 0.03f, 0.05f, 0.18f));
	FSlateColorBrush PanelFallbackBrush = FSlateColorBrush(FLinearColor(0.07f, 0.05f, 0.04f, 0.92f));
	FSlateColorBrush DividerFallbackBrush = FSlateColorBrush(FLinearColor(0.80f, 0.66f, 0.34f, 0.95f));
	FCurveSequence IntroSequence;
	FCurveHandle TitleRevealCurve;
	FCurveHandle MenuRevealCurve;
	double AmbientTime = 0.0;
	bool bShowingOptions = false;
};
