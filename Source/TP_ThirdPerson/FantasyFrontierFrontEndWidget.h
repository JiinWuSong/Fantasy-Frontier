#pragma once

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
	TSharedRef<SWidget> BuildMainMenu();
	TSharedRef<SWidget> BuildOptionsMenu();

	FSimpleDelegate OnStartGame;
	FSimpleDelegate OnQuit;
	FSimpleDelegate OnCycleWindowMode;
	FSimpleDelegate OnCycleQuality;
	TAttribute<FText> WindowModeLabel;
	TAttribute<FText> QualityLabel;
	TSharedPtr<FSlateDynamicImageBrush> BackgroundBrush;
	FButtonStyle MenuButtonStyle;
	FSlateColorBrush ScreenTintBrush = FSlateColorBrush(FLinearColor(0.01f, 0.02f, 0.04f, 0.42f));
	FSlateColorBrush PanelOuterBrush = FSlateColorBrush(FLinearColor(0.70f, 0.54f, 0.23f, 0.20f));
	FSlateColorBrush PanelInnerBrush = FSlateColorBrush(FLinearColor(0.02f, 0.05f, 0.09f, 0.86f));
	FSlateColorBrush AccentLineBrush = FSlateColorBrush(FLinearColor(0.79f, 0.62f, 0.25f, 0.95f));
	bool bShowingOptions = false;
};
