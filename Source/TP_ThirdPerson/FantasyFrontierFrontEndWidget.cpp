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
	TitleLogoBrush = LoadBrush(TEXT("Slate/TitleLogo.png"), FVector2D(1760.0f, 360.0f));
	DividerBrush = LoadBrush(TEXT("Slate/TitleDivider.png"), FVector2D(1600.0f, 40.0f));
	PanelBrush = LoadBrush(TEXT("Slate/MenuPanel.png"), FVector2D(920.0f, 560.0f));
	ButtonNormalBrush = LoadBrush(TEXT("Slate/MenuButton.png"), FVector2D(640.0f, 156.0f));
	ButtonHoveredBrush = LoadBrush(TEXT("Slate/MenuButtonHover.png"), FVector2D(640.0f, 156.0f));
	ButtonPressedBrush = LoadBrush(TEXT("Slate/MenuButtonPressed.png"), FVector2D(640.0f, 156.0f));
	MenuButtonStyle = BuildMenuButtonStyle(ButtonNormalBrush.Get(), ButtonHoveredBrush.Get(), ButtonPressedBrush.Get());

	IntroSequence = FCurveSequence();
	TitleRevealCurve = IntroSequence.AddCurve(0.05f, 1.25f, ECurveEaseFunction::CubicOut);
	MenuRevealCurve = IntroSequence.AddCurve(0.75f, 0.95f, ECurveEaseFunction::QuadOut);

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
					.WidthOverride(1420.0f)
					.HeightOverride(274.0f)
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
					.WidthOverride(1220.0f)
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
				.Padding(FMargin(0.0f, 16.0f, 0.0f, 0.0f))
				[
					SNew(STextBlock)
					.Text(LOCTEXT("FrontEndTagline", "Moonlit kingdoms. Wild forests. Ancient enemies awake."))
					.Font(MakeBodyFont(22))
					.ColorAndOpacity_Lambda([this]()
					{
						return FLinearColor(0.87f, 0.80f, 0.67f, GetTitleOpacity() * 0.88f);
					})
					.ShadowOffset(FVector2D(0.0f, 2.0f))
					.ShadowColorAndOpacity(FLinearColor(0.03f, 0.02f, 0.01f, 0.6f))
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Bottom)
			.Padding(FMargin(0.0f, 0.0f, 0.0f, 72.0f))
			[
				BuildMenuFrame()
			]
		]
	];

	IntroSequence.Play(AsShared());
}

void SFantasyFrontierFrontEndWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	AmbientTime += InDeltaTime;
}

FReply SFantasyFrontierFrontEndWidget::HandleStartGame()
{
	OnStartGame.ExecuteIfBound();
	return FReply::Handled();
}

FReply SFantasyFrontierFrontEndWidget::HandleQuit()
{
	OnQuit.ExecuteIfBound();
	return FReply::Handled();
}

FReply SFantasyFrontierFrontEndWidget::HandleShowOptions()
{
	bShowingOptions = true;
	return FReply::Handled();
}

FReply SFantasyFrontierFrontEndWidget::HandleCloseOptions()
{
	bShowingOptions = false;
	return FReply::Handled();
}

FReply SFantasyFrontierFrontEndWidget::HandleCycleWindowMode()
{
	OnCycleWindowMode.ExecuteIfBound();
	return FReply::Handled();
}

FReply SFantasyFrontierFrontEndWidget::HandleCycleQuality()
{
	OnCycleQuality.ExecuteIfBound();
	return FReply::Handled();
}

EVisibility SFantasyFrontierFrontEndWidget::GetMainMenuVisibility() const
{
	return bShowingOptions ? EVisibility::Collapsed : EVisibility::Visible;
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
	return FSlateRenderTransform(FVector2D(FMath::Sin(AmbientTime * 0.05) * 12.0f, FMath::Cos(AmbientTime * 0.04) * 8.0f));
}

TOptional<FSlateRenderTransform> SFantasyFrontierFrontEndWidget::GetMidgroundTransform() const
{
	return FSlateRenderTransform(FVector2D(FMath::Sin(AmbientTime * 0.09) * 18.0f, FMath::Cos(AmbientTime * 0.07) * 10.0f));
}

TOptional<FSlateRenderTransform> SFantasyFrontierFrontEndWidget::GetMistTransform() const
{
	return FSlateRenderTransform(FVector2D(FMath::Sin(AmbientTime * 0.14) * -22.0f, FMath::Cos(AmbientTime * 0.06) * 4.0f));
}

TOptional<FSlateRenderTransform> SFantasyFrontierFrontEndWidget::GetForegroundTransform() const
{
	return FSlateRenderTransform(FVector2D(FMath::Sin(AmbientTime * 0.12) * 28.0f, FMath::Cos(AmbientTime * 0.09) * 12.0f));
}

TOptional<FSlateRenderTransform> SFantasyFrontierFrontEndWidget::GetTitleTransform() const
{
	const float Reveal = GetTitleOpacity();
	return FSlateRenderTransform(FVector2D(0.0f, FMath::Lerp(46.0f, 0.0f, Reveal)));
}

TOptional<FSlateRenderTransform> SFantasyFrontierFrontEndWidget::GetPanelTransform() const
{
	const float Reveal = GetMenuOpacity();
	return FSlateRenderTransform(FVector2D(0.0f, FMath::Lerp(28.0f, 0.0f, Reveal)));
}

float SFantasyFrontierFrontEndWidget::GetTitleOpacity() const
{
	return TitleRevealCurve.GetLerp();
}

float SFantasyFrontierFrontEndWidget::GetMenuOpacity() const
{
	return MenuRevealCurve.GetLerp();
}

const FSlateBrush* SFantasyFrontierFrontEndWidget::ResolveBrush(const TSharedPtr<FSlateDynamicImageBrush>& Brush, const FSlateBrush* Fallback) const
{
	return Brush.IsValid() ? Brush.Get() : Fallback;
}

TSharedRef<SWidget> SFantasyFrontierFrontEndWidget::BuildMenuFrame()
{
	return SNew(SBox)
		.WidthOverride(760.0f)
		.HeightOverride(470.0f)
		[
			SNew(SBorder)
			.BorderImage(ResolveBrush(PanelBrush, &PanelFallbackBrush))
			.BorderBackgroundColor_Lambda([this]()
			{
				return FLinearColor(1.0f, 1.0f, 1.0f, GetMenuOpacity());
			})
			.RenderTransform(this, &SFantasyFrontierFrontEndWidget::GetPanelTransform)
			.RenderTransformPivot(FVector2D(0.5f, 0.5f))
			.Padding(FMargin(78.0f, 70.0f, 78.0f, 64.0f))
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					BuildMainMenu()
				]
				+ SOverlay::Slot()
				[
					BuildOptionsMenu()
				]
			]
		];
}

TSharedRef<SWidget> SFantasyFrontierFrontEndWidget::BuildMainMenu()
{
	const FSlateFontInfo HeadingFont = MakeBodyFont(18);
	const FSlateFontInfo ButtonFont = MakeBodyFont(28);

	return SNew(SVerticalBox)
		.Visibility(this, &SFantasyFrontierFrontEndWidget::GetMainMenuVisibility)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(FMargin(0.0f, 0.0f, 0.0f, 10.0f))
		[
			SNew(STextBlock)
			.Text(LOCTEXT("FrontEndHeading", "ADVENTURE AWAITS"))
			.Font(HeadingFont)
			.ColorAndOpacity(FLinearColor(0.84f, 0.75f, 0.58f, 0.9f))
			.ShadowOffset(FVector2D(0.0f, 2.0f))
			.ShadowColorAndOpacity(FLinearColor(0.04f, 0.02f, 0.01f, 0.55f))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(FMargin(0.0f, 10.0f, 0.0f, 0.0f))
		[
			SNew(SBox)
			.WidthOverride(510.0f)
			[
				SNew(SButton)
				.ButtonStyle(&MenuButtonStyle)
				.ContentPadding(FMargin(24.0f, 18.0f))
				.OnClicked(this, &SFantasyFrontierFrontEndWidget::HandleStartGame)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("StartGame", "Game Start"))
					.Font(ButtonFont)
					.Justification(ETextJustify::Center)
					.ColorAndOpacity(FLinearColor(0.98f, 0.94f, 0.86f))
					.ShadowOffset(FVector2D(0.0f, 2.0f))
					.ShadowColorAndOpacity(FLinearColor(0.10f, 0.05f, 0.02f, 0.72f))
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(FMargin(0.0f, 16.0f, 0.0f, 0.0f))
		[
			SNew(SBox)
			.WidthOverride(510.0f)
			[
				SNew(SButton)
				.ButtonStyle(&MenuButtonStyle)
				.ContentPadding(FMargin(24.0f, 18.0f))
				.OnClicked(this, &SFantasyFrontierFrontEndWidget::HandleShowOptions)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Options", "Options"))
					.Font(ButtonFont)
					.Justification(ETextJustify::Center)
					.ColorAndOpacity(FLinearColor(0.98f, 0.94f, 0.86f))
					.ShadowOffset(FVector2D(0.0f, 2.0f))
					.ShadowColorAndOpacity(FLinearColor(0.10f, 0.05f, 0.02f, 0.72f))
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(FMargin(0.0f, 16.0f, 0.0f, 0.0f))
		[
			SNew(SBox)
			.WidthOverride(510.0f)
			[
				SNew(SButton)
				.ButtonStyle(&MenuButtonStyle)
				.ContentPadding(FMargin(24.0f, 18.0f))
				.OnClicked(this, &SFantasyFrontierFrontEndWidget::HandleQuit)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("QuitGame", "Exit"))
					.Font(ButtonFont)
					.Justification(ETextJustify::Center)
					.ColorAndOpacity(FLinearColor(0.98f, 0.94f, 0.86f))
					.ShadowOffset(FVector2D(0.0f, 2.0f))
					.ShadowColorAndOpacity(FLinearColor(0.10f, 0.05f, 0.02f, 0.72f))
				]
			]
		];
}

TSharedRef<SWidget> SFantasyFrontierFrontEndWidget::BuildOptionsMenu()
{
	const FSlateFontInfo HeadingFont = MakeBodyFont(30);
	const FSlateFontInfo BodyFont = MakeBodyFont(22);

	return SNew(SVerticalBox)
		.Visibility(this, &SFantasyFrontierFrontEndWidget::GetOptionsVisibility)
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
				.WidthOverride(270.0f)
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
				.WidthOverride(270.0f)
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
		.Padding(FMargin(0.0f, 26.0f, 0.0f, 0.0f))
		[
			SNew(SBox)
			.WidthOverride(300.0f)
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
		];
}

#undef LOCTEXT_NAMESPACE
