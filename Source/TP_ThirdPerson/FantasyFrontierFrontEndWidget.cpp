#include "FantasyFrontierFrontEndWidget.h"

#include "Brushes/SlateColorBrush.h"
#include "Brushes/SlateDynamicImageBrush.h"
#include "HAL/PlatformFileManager.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/Paths.h"
#include "Styling/SlateTypes.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FantasyFrontierFrontEnd"

namespace
{
	TSharedPtr<FSlateDynamicImageBrush> LoadBrush(const TCHAR* RelativePath, const FVector2D& Size)
	{
		const FString FullPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / RelativePath);
		if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*FullPath))
		{
			return nullptr;
		}

		return MakeShared<FSlateDynamicImageBrush>(FName(*FullPath), Size);
	}

	FButtonStyle BuildMenuButtonStyle(const FSlateBrush* NormalBrush, const FSlateBrush* HoveredBrush, const FSlateBrush* PressedBrush)
	{
		if (NormalBrush && HoveredBrush && PressedBrush)
		{
			return FButtonStyle()
				.SetNormal(*NormalBrush)
				.SetHovered(*HoveredBrush)
				.SetPressed(*PressedBrush)
				.SetNormalForeground(FLinearColor(0.98f, 0.94f, 0.84f))
				.SetHoveredForeground(FLinearColor(1.0f, 0.98f, 0.90f))
				.SetPressedForeground(FLinearColor(0.95f, 0.90f, 0.82f))
				.SetPressedPadding(FMargin(0.0f, 3.0f, 0.0f, -3.0f));
		}

		const FLinearColor Normal = FLinearColor(0.16f, 0.10f, 0.07f, 0.95f);
		const FLinearColor Hovered = FLinearColor(0.24f, 0.15f, 0.09f, 0.97f);
		const FLinearColor Pressed = FLinearColor(0.11f, 0.07f, 0.05f, 0.99f);

		return FButtonStyle()
			.SetNormal(FSlateColorBrush(Normal))
			.SetHovered(FSlateColorBrush(Hovered))
			.SetPressed(FSlateColorBrush(Pressed))
			.SetNormalForeground(FLinearColor(0.98f, 0.94f, 0.84f))
			.SetHoveredForeground(FLinearColor(1.0f, 0.98f, 0.90f))
			.SetPressedForeground(FLinearColor(0.95f, 0.90f, 0.82f));
	}

	FButtonStyle BuildTextMenuButtonStyle()
	{
		return FButtonStyle()
			.SetNormal(FSlateNoResource())
			.SetHovered(FSlateNoResource())
			.SetPressed(FSlateNoResource())
			.SetNormalForeground(FLinearColor(0.96f, 0.84f, 0.58f))
			.SetHoveredForeground(FLinearColor(1.0f, 0.95f, 0.78f))
			.SetPressedForeground(FLinearColor(0.89f, 0.76f, 0.48f))
			.SetPressedPadding(FMargin(0.0f, 2.0f, 0.0f, -2.0f));
	}
}

void SFantasyFrontierFrontEndWidget::Construct(const FArguments& InArgs)
{
	OnStartGame = InArgs._OnStartGame;
	OnQuit = InArgs._OnQuit;
	OnCycleWindowMode = InArgs._OnCycleWindowMode;
	OnCycleQuality = InArgs._OnCycleQuality;
	WindowModeLabel = InArgs._WindowModeLabel;
	QualityLabel = InArgs._QualityLabel;

	SkyBrush = LoadBrush(TEXT("Slate/MenuSky.png"), FVector2D(1920.0f, 1080.0f));
	MidgroundBrush = LoadBrush(TEXT("Slate/MenuMidground.png"), FVector2D(1920.0f, 1080.0f));
	MistBrush = LoadBrush(TEXT("Slate/MenuMist.png"), FVector2D(1920.0f, 1080.0f));
	ForegroundBrush = LoadBrush(TEXT("Slate/MenuForeground.png"), FVector2D(1920.0f, 1080.0f));
	TitleLogoBrush = LoadBrush(TEXT("Slate/TitleLogo.png"), FVector2D(1660.0f, 320.0f));
	DividerBrush = LoadBrush(TEXT("Slate/TitleDivider.png"), FVector2D(1600.0f, 40.0f));
	PanelBrush = LoadBrush(TEXT("Slate/MenuPanel.png"), FVector2D(920.0f, 560.0f));
	ButtonNormalBrush = LoadBrush(TEXT("Slate/MenuButton.png"), FVector2D(640.0f, 156.0f));
	ButtonHoveredBrush = LoadBrush(TEXT("Slate/MenuButtonHover.png"), FVector2D(640.0f, 156.0f));
	ButtonPressedBrush = LoadBrush(TEXT("Slate/MenuButtonPressed.png"), FVector2D(640.0f, 156.0f));
	MenuButtonStyle = BuildMenuButtonStyle(ButtonNormalBrush.Get(), ButtonHoveredBrush.Get(), ButtonPressedBrush.Get());
	TextMenuButtonStyle = BuildTextMenuButtonStyle();

	IntroSequence = FCurveSequence();
	TitleRevealCurve = IntroSequence.AddCurve(0.05f, 1.25f, ECurveEaseFunction::CubicOut);
	MenuRevealCurve = IntroSequence.AddCurve(0.75f, 0.95f, ECurveEaseFunction::QuadOut);
	LastInteractionTime = AmbientTime;
	ShowcaseBlend = 0.0f;

	TSharedRef<SWidget> TitleWidget = SNew(SSpacer);
	if (TitleLogoBrush.IsValid())
	{
		TitleWidget =
			SNew(SImage)
			.Image(TitleLogoBrush.Get())
			.ColorAndOpacity_Lambda([this]()
			{
				return FLinearColor(1.0f, 1.0f, 1.0f, GetTitleOpacity());
			})
			.RenderTransform(this, &SFantasyFrontierFrontEndWidget::GetTitleTransform)
			.RenderTransformPivot(FVector2D(0.5f, 0.5f));
	}
	else
	{
		TitleWidget =
			SNew(STextBlock)
			.Text(LOCTEXT("FrontEndTitleFallback", "FANTASY FRONTIER"))
			.Font(MakeTitleFont())
			.ColorAndOpacity_Lambda([this]()
			{
				return FLinearColor(0.97f, 0.92f, 0.84f, GetTitleOpacity());
			});
	}

	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SImage)
			.Image(ResolveBrush(SkyBrush, nullptr))
			.RenderTransform(this, &SFantasyFrontierFrontEndWidget::GetSkyTransform)
			.RenderTransformPivot(FVector2D(0.5f, 0.5f))
		]
		+ SOverlay::Slot()
		[
			SNew(SImage)
			.Image(ResolveBrush(MidgroundBrush, nullptr))
			.RenderTransform(this, &SFantasyFrontierFrontEndWidget::GetMidgroundTransform)
			.RenderTransformPivot(FVector2D(0.5f, 0.5f))
		]
		+ SOverlay::Slot()
		[
			SNew(SImage)
			.Image(ResolveBrush(MistBrush, nullptr))
			.ColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 0.72f))
			.RenderTransform(this, &SFantasyFrontierFrontEndWidget::GetMistTransform)
			.RenderTransformPivot(FVector2D(0.5f, 0.5f))
		]
		+ SOverlay::Slot()
		[
			SNew(SImage)
			.Image(ResolveBrush(ForegroundBrush, nullptr))
			.RenderTransform(this, &SFantasyFrontierFrontEndWidget::GetForegroundTransform)
			.RenderTransformPivot(FVector2D(0.5f, 0.5f))
		]
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(&ScreenTintBrush)
		]
		+ SOverlay::Slot()
		.Padding(FMargin(48.0f, 52.0f, 48.0f, 54.0f))
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Top)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(1180.0f)
					.HeightOverride(212.0f)
					[
						SNew(SScaleBox)
						.Stretch(EStretch::ScaleToFit)
						[
							TitleWidget
						]
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(FMargin(0.0f, 8.0f, 0.0f, 0.0f))
				[
					SNew(SBox)
					.WidthOverride(920.0f)
					.HeightOverride(18.0f)
					[
						SNew(SImage)
						.Image(ResolveBrush(DividerBrush, &DividerFallbackBrush))
						.ColorAndOpacity_Lambda([this]()
						{
							return FLinearColor(1.0f, 1.0f, 1.0f, GetTitleOpacity());
						})
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(FMargin(0.0f, 14.0f, 0.0f, 0.0f))
				[
					SNew(STextBlock)
					.Text(LOCTEXT("FrontEndTagline", "Sunlit forests. Ancient ruins. Open horizons."))
					.Font(MakeBodyFont(20))
					.ColorAndOpacity_Lambda([this]()
					{
						return FLinearColor(0.46f, 0.35f, 0.21f, GetTitleOpacity() * 0.92f);
					})
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Bottom)
			.Padding(FMargin(0.0f, 0.0f, 0.0f, 74.0f))
			[
				BuildMenuFrame()
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Bottom)
			.Padding(FMargin(72.0f, 0.0f, 0.0f, 54.0f))
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ShowcaseCaption", "A brighter frontier stirs beyond the old ruins."))
				.Font(MakeBodyFont(18))
				.ColorAndOpacity_Lambda([this]()
				{
					return FLinearColor(0.95f, 0.92f, 0.82f, GetShowcaseCaptionOpacity());
				})
				.ShadowOffset(FVector2D(0.0f, 2.0f))
				.ShadowColorAndOpacity(FLinearColor(0.12f, 0.09f, 0.05f, 0.48f))
			]
		]
	];

	IntroSequence.Play(AsShared());
}

void SFantasyFrontierFrontEndWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	AmbientTime += InDeltaTime;

	const bool bShouldShowcase = !bShowingOptions && (AmbientTime - LastInteractionTime) >= IdleShowcaseDelay;
	const float TargetBlend = bShouldShowcase ? 1.0f : 0.0f;
	const float InterpSpeed = bShouldShowcase ? 0.28f : 2.4f;
	ShowcaseBlend = FMath::FInterpTo(ShowcaseBlend, TargetBlend, InDeltaTime, InterpSpeed);
}

FReply SFantasyFrontierFrontEndWidget::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	ResetIdleTimer();
	return SCompoundWidget::OnMouseMove(MyGeometry, MouseEvent);
}

FReply SFantasyFrontierFrontEndWidget::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	ResetIdleTimer();
	return SCompoundWidget::OnMouseButtonDown(MyGeometry, MouseEvent);
}

FReply SFantasyFrontierFrontEndWidget::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	ResetIdleTimer();
	return SCompoundWidget::OnMouseWheel(MyGeometry, MouseEvent);
}

FReply SFantasyFrontierFrontEndWidget::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	ResetIdleTimer();
	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

void SFantasyFrontierFrontEndWidget::ResetIdleTimer()
{
	LastInteractionTime = AmbientTime;
}

FReply SFantasyFrontierFrontEndWidget::HandleStartGame()
{
	ResetIdleTimer();
	OnStartGame.ExecuteIfBound();
	return FReply::Handled();
}

FReply SFantasyFrontierFrontEndWidget::HandleQuit()
{
	ResetIdleTimer();
	OnQuit.ExecuteIfBound();
	return FReply::Handled();
}

FReply SFantasyFrontierFrontEndWidget::HandleShowOptions()
{
	ResetIdleTimer();
	bShowingOptions = true;
	return FReply::Handled();
}

FReply SFantasyFrontierFrontEndWidget::HandleCloseOptions()
{
	ResetIdleTimer();
	bShowingOptions = false;
	return FReply::Handled();
}

FReply SFantasyFrontierFrontEndWidget::HandleCycleWindowMode()
{
	ResetIdleTimer();
	OnCycleWindowMode.ExecuteIfBound();
	return FReply::Handled();
}

FReply SFantasyFrontierFrontEndWidget::HandleCycleQuality()
{
	ResetIdleTimer();
	OnCycleQuality.ExecuteIfBound();
	return FReply::Handled();
}

EVisibility SFantasyFrontierFrontEndWidget::GetMainMenuVisibility() const
{
	return (bShowingOptions || ShowcaseBlend > 0.02f) ? EVisibility::Collapsed : EVisibility::Visible;
}

EVisibility SFantasyFrontierFrontEndWidget::GetOptionsVisibility() const
{
	return bShowingOptions ? EVisibility::Visible : EVisibility::Collapsed;
}

FSlateFontInfo SFantasyFrontierFrontEndWidget::MakeTitleFont() const
{
	FSlateFontInfo Font(FPaths::ProjectContentDir() / TEXT("Slate/Fonts/Georgia-Bold.ttf"), 112);
	Font.LetterSpacing = 80;
	return Font;
}

FSlateFontInfo SFantasyFrontierFrontEndWidget::MakeBodyFont(int32 Size) const
{
	return FSlateFontInfo(FPaths::ProjectContentDir() / TEXT("Slate/Fonts/Georgia-Bold.ttf"), Size);
}

TOptional<FSlateRenderTransform> SFantasyFrontierFrontEndWidget::GetSkyTransform() const
{
	return FSlateRenderTransform(
		FVector2D(
			FMath::Sin(AmbientTime * 0.05) * 12.0f - ShowcaseBlend * 96.0f,
			FMath::Cos(AmbientTime * 0.04) * 8.0f - ShowcaseBlend * 14.0f));
}

TOptional<FSlateRenderTransform> SFantasyFrontierFrontEndWidget::GetMidgroundTransform() const
{
	return FSlateRenderTransform(
		FVector2D(
			FMath::Sin(AmbientTime * 0.09) * 18.0f - ShowcaseBlend * 168.0f,
			FMath::Cos(AmbientTime * 0.07) * 10.0f - ShowcaseBlend * 8.0f));
}

TOptional<FSlateRenderTransform> SFantasyFrontierFrontEndWidget::GetMistTransform() const
{
	return FSlateRenderTransform(
		FVector2D(
			FMath::Sin(AmbientTime * 0.14) * -22.0f - ShowcaseBlend * 244.0f,
			FMath::Cos(AmbientTime * 0.06) * 4.0f));
}

TOptional<FSlateRenderTransform> SFantasyFrontierFrontEndWidget::GetForegroundTransform() const
{
	return FSlateRenderTransform(
		FVector2D(
			FMath::Sin(AmbientTime * 0.12) * 28.0f - ShowcaseBlend * 312.0f,
			FMath::Cos(AmbientTime * 0.09) * 12.0f + ShowcaseBlend * 16.0f));
}

TOptional<FSlateRenderTransform> SFantasyFrontierFrontEndWidget::GetTitleTransform() const
{
	const float Reveal = GetTitleOpacity();
	return FSlateRenderTransform(FVector2D(0.0f, FMath::Lerp(46.0f, -36.0f, Reveal) - ShowcaseBlend * 24.0f));
}

TOptional<FSlateRenderTransform> SFantasyFrontierFrontEndWidget::GetPanelTransform() const
{
	const float Reveal = GetMenuOpacity();
	return FSlateRenderTransform(FVector2D(0.0f, FMath::Lerp(28.0f, 0.0f, Reveal) + ShowcaseBlend * 28.0f));
}

float SFantasyFrontierFrontEndWidget::GetTitleOpacity() const
{
	return TitleRevealCurve.GetLerp() * FMath::Lerp(1.0f, 0.56f, ShowcaseBlend);
}

float SFantasyFrontierFrontEndWidget::GetMenuOpacity() const
{
	return MenuRevealCurve.GetLerp();
}

float SFantasyFrontierFrontEndWidget::GetMainMenuOpacity() const
{
	return GetMenuOpacity() * FMath::Lerp(1.0f, 0.0f, ShowcaseBlend);
}

float SFantasyFrontierFrontEndWidget::GetOptionsOpacity() const
{
	return GetMenuOpacity();
}

float SFantasyFrontierFrontEndWidget::GetShowcaseCaptionOpacity() const
{
	return GetMenuOpacity() * FMath::Clamp(ShowcaseBlend * 1.15f, 0.0f, 0.92f);
}

const FSlateBrush* SFantasyFrontierFrontEndWidget::ResolveBrush(const TSharedPtr<FSlateDynamicImageBrush>& Brush, const FSlateBrush* Fallback) const
{
	return Brush.IsValid() ? Brush.Get() : Fallback;
}

TSharedRef<SWidget> SFantasyFrontierFrontEndWidget::BuildMenuFrame()
{
	return SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SBox)
			.WidthOverride(430.0f)
			[
				BuildMainMenu()
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Bottom)
		[
			BuildOptionsMenu()
		];
}

TSharedRef<SWidget> SFantasyFrontierFrontEndWidget::BuildMainMenu()
{
	const FSlateFontInfo HeadingFont = MakeBodyFont(17);
	const FSlateFontInfo ButtonFont = MakeBodyFont(31);
	const FSlateColor MenuTextColor = FSlateColor::UseForeground();

	return SNew(SVerticalBox)
		.Visibility(this, &SFantasyFrontierFrontEndWidget::GetMainMenuVisibility)
		.RenderTransform(this, &SFantasyFrontierFrontEndWidget::GetPanelTransform)
		.RenderTransformPivot(FVector2D(0.5f, 1.0f))
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(FMargin(0.0f, 0.0f, 0.0f, 8.0f))
		[
			SNew(STextBlock)
			.Text(LOCTEXT("FrontEndHeading", "MAIN MENU"))
			.Font(HeadingFont)
			.ColorAndOpacity(FLinearColor(0.94f, 0.88f, 0.76f, 0.92f))
			.ShadowOffset(FVector2D(0.0f, 2.0f))
			.ShadowColorAndOpacity(FLinearColor(0.10f, 0.06f, 0.03f, 0.28f))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(FMargin(0.0f, 0.0f, 0.0f, 14.0f))
		[
			SNew(SBox)
			.WidthOverride(300.0f)
			.HeightOverride(14.0f)
			[
				SNew(SImage)
				.Image(ResolveBrush(DividerBrush, &DividerFallbackBrush))
				.ColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 0.84f))
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(FMargin(0.0f, 0.0f, 0.0f, 6.0f))
		[
			SNew(SBox)
			.WidthOverride(330.0f)
			[
				SNew(SButton)
				.ButtonStyle(&TextMenuButtonStyle)
				.ContentPadding(FMargin(16.0f, 10.0f))
				.OnClicked(this, &SFantasyFrontierFrontEndWidget::HandleStartGame)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("StartGame", "START"))
					.Font(ButtonFont)
					.ColorAndOpacity(MenuTextColor)
					.Justification(ETextJustify::Center)
					.ShadowOffset(FVector2D(0.0f, 2.0f))
					.ShadowColorAndOpacity(FLinearColor(0.10f, 0.05f, 0.02f, 0.64f))
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(FMargin(0.0f, 0.0f, 0.0f, 6.0f))
		[
			SNew(SBox)
			.WidthOverride(330.0f)
			[
				SNew(SButton)
				.ButtonStyle(&TextMenuButtonStyle)
				.ContentPadding(FMargin(16.0f, 10.0f))
				.OnClicked(this, &SFantasyFrontierFrontEndWidget::HandleShowOptions)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Options", "OPTIONS"))
					.Font(ButtonFont)
					.ColorAndOpacity(MenuTextColor)
					.Justification(ETextJustify::Center)
					.ShadowOffset(FVector2D(0.0f, 2.0f))
					.ShadowColorAndOpacity(FLinearColor(0.10f, 0.05f, 0.02f, 0.64f))
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(FMargin(0.0f, 0.0f, 0.0f, 12.0f))
		[
			SNew(SBox)
			.WidthOverride(330.0f)
			[
				SNew(SButton)
				.ButtonStyle(&TextMenuButtonStyle)
				.ContentPadding(FMargin(16.0f, 10.0f))
				.OnClicked(this, &SFantasyFrontierFrontEndWidget::HandleQuit)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("QuitGame", "EXIT"))
					.Font(ButtonFont)
					.ColorAndOpacity(MenuTextColor)
					.Justification(ETextJustify::Center)
					.ShadowOffset(FVector2D(0.0f, 2.0f))
					.ShadowColorAndOpacity(FLinearColor(0.10f, 0.05f, 0.02f, 0.64f))
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(300.0f)
			.HeightOverride(14.0f)
			[
				SNew(SImage)
				.Image(ResolveBrush(DividerBrush, &DividerFallbackBrush))
				.ColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 0.78f))
			]
		];
}

TSharedRef<SWidget> SFantasyFrontierFrontEndWidget::BuildOptionsMenu()
{
	const FSlateFontInfo HeadingFont = MakeBodyFont(28);
	const FSlateFontInfo BodyFont = MakeBodyFont(21);

	return SNew(SBox)
		.WidthOverride(660.0f)
		[
			SNew(SBorder)
			.Visibility(this, &SFantasyFrontierFrontEndWidget::GetOptionsVisibility)
			.BorderImage(ResolveBrush(PanelBrush, &PanelFallbackBrush))
			.BorderBackgroundColor_Lambda([this]()
			{
				return FLinearColor(1.0f, 1.0f, 1.0f, GetOptionsOpacity());
			})
			.RenderTransform(this, &SFantasyFrontierFrontEndWidget::GetPanelTransform)
			.RenderTransformPivot(FVector2D(0.5f, 1.0f))
			.Padding(FMargin(48.0f, 42.0f, 48.0f, 40.0f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(FMargin(0.0f, 0.0f, 0.0f, 14.0f))
				[
					SNew(STextBlock)
					.Text(LOCTEXT("OptionsTitle", "OPTIONS"))
					.Font(HeadingFont)
					.ColorAndOpacity(FLinearColor(0.97f, 0.92f, 0.82f))
					.ShadowOffset(FVector2D(0.0f, 2.0f))
					.ShadowColorAndOpacity(FLinearColor(0.08f, 0.04f, 0.02f, 0.65f))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.0f, 10.0f, 0.0f, 0.0f))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("WindowMode", "Display Mode"))
						.Font(BodyFont)
						.ColorAndOpacity(FLinearColor(0.89f, 0.82f, 0.70f))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SBox)
						.WidthOverride(248.0f)
						[
							SNew(SButton)
							.ButtonStyle(&MenuButtonStyle)
							.ContentPadding(FMargin(18.0f, 14.0f))
							.OnClicked(this, &SFantasyFrontierFrontEndWidget::HandleCycleWindowMode)
							[
								SNew(STextBlock)
								.Text(WindowModeLabel)
								.Font(BodyFont)
								.Justification(ETextJustify::Center)
								.ColorAndOpacity(FLinearColor(0.98f, 0.94f, 0.86f))
							]
						]
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.0f, 16.0f, 0.0f, 0.0f))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("Quality", "Graphics Preset"))
						.Font(BodyFont)
						.ColorAndOpacity(FLinearColor(0.89f, 0.82f, 0.70f))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SBox)
						.WidthOverride(248.0f)
						[
							SNew(SButton)
							.ButtonStyle(&MenuButtonStyle)
							.ContentPadding(FMargin(18.0f, 14.0f))
							.OnClicked(this, &SFantasyFrontierFrontEndWidget::HandleCycleQuality)
							[
								SNew(STextBlock)
								.Text(QualityLabel)
								.Font(BodyFont)
								.Justification(ETextJustify::Center)
								.ColorAndOpacity(FLinearColor(0.98f, 0.94f, 0.86f))
							]
						]
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(FMargin(0.0f, 24.0f, 0.0f, 0.0f))
				[
					SNew(SBox)
					.WidthOverride(280.0f)
					[
						SNew(SButton)
						.ButtonStyle(&MenuButtonStyle)
						.ContentPadding(FMargin(18.0f, 14.0f))
						.OnClicked(this, &SFantasyFrontierFrontEndWidget::HandleCloseOptions)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("Back", "Back"))
							.Font(BodyFont)
							.Justification(ETextJustify::Center)
							.ColorAndOpacity(FLinearColor(0.98f, 0.94f, 0.86f))
						]
					]
				]
			]
		];
}

#undef LOCTEXT_NAMESPACE
