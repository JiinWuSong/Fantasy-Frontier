#include "FantasyFrontierFrontEndWidget.h"

#include "Brushes/SlateColorBrush.h"
#include "Brushes/SlateDynamicImageBrush.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/Paths.h"
#include "Styling/SlateTypes.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SOverlay.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FantasyFrontierFrontEnd"

namespace
{
	FButtonStyle BuildMenuButtonStyle()
	{
		const FLinearColor Normal = FLinearColor(0.06f, 0.11f, 0.18f, 0.84f);
		const FLinearColor Hovered = FLinearColor(0.13f, 0.18f, 0.26f, 0.96f);
		const FLinearColor Pressed = FLinearColor(0.18f, 0.13f, 0.08f, 0.98f);

		return FButtonStyle()
			.SetNormal(FSlateColorBrush(Normal))
			.SetHovered(FSlateColorBrush(Hovered))
			.SetPressed(FSlateColorBrush(Pressed))
			.SetNormalForeground(FLinearColor(0.97f, 0.93f, 0.84f))
			.SetHoveredForeground(FLinearColor(1.0f, 0.97f, 0.88f))
			.SetPressedForeground(FLinearColor(1.0f, 0.97f, 0.90f));
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
	MenuButtonStyle = BuildMenuButtonStyle();

	const FString BackgroundPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / TEXT("Slate/MenuBackground.png"));
	if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*BackgroundPath))
	{
		BackgroundBrush = MakeShared<FSlateDynamicImageBrush>(FName(*BackgroundPath), FVector2D(1920.0f, 1080.0f));
	}

	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SImage)
			.Image(BackgroundBrush.IsValid() ? BackgroundBrush.Get() : nullptr)
		]
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(&ScreenTintBrush)
		]
		+ SOverlay::Slot()
		.Padding(FMargin(72.0f, 50.0f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.FillHeight(0.14f)
			[
				SNew(SSpacer)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("FrontEndTitle", "FANTASY FRONTIER"))
				.Font(MakeTitleFont())
				.ColorAndOpacity(FLinearColor(0.97f, 0.91f, 0.80f))
				.ShadowOffset(FVector2D(0.0f, 4.0f))
				.ShadowColorAndOpacity(FLinearColor(0.32f, 0.19f, 0.05f, 0.55f))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(FMargin(0.0f, 14.0f, 0.0f, 0.0f))
			[
				SNew(SBox)
				.WidthOverride(1540.0f)
				.HeightOverride(3.0f)
				[
					SNew(SBorder)
					.BorderImage(&AccentLineBrush)
				]
			]
			+ SVerticalBox::Slot()
			.FillHeight(0.22f)
			[
				SNew(SSpacer)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(SBorder)
				.BorderImage(&PanelOuterBrush)
				.Padding(1.0f)
				[
					SNew(SBorder)
					.BorderImage(&PanelInnerBrush)
					.Padding(FMargin(34.0f, 30.0f))
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
				]
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SSpacer)
			]
		]
	];
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
	Font.LetterSpacing = 120;
	return Font;
}

FSlateFontInfo SFantasyFrontierFrontEndWidget::MakeBodyFont(int32 Size) const
{
	return FSlateFontInfo(FPaths::ProjectContentDir() / TEXT("Slate/Fonts/Georgia-Bold.ttf"), Size);
}

TSharedRef<SWidget> SFantasyFrontierFrontEndWidget::BuildMainMenu()
{
	const FSlateFontInfo ButtonFont = MakeBodyFont(26);

	return SNew(SVerticalBox)
		.Visibility(this, &SFantasyFrontierFrontEndWidget::GetMainMenuVisibility)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(FMargin(0.0f, 0.0f, 0.0f, 6.0f))
		[
			SNew(STextBlock)
			.Text(LOCTEXT("FrontEndSubline", "Fantasy MMO Prototype"))
			.Font(MakeBodyFont(20))
			.ColorAndOpacity(FLinearColor(0.83f, 0.76f, 0.60f, 0.88f))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(FMargin(0.0f, 8.0f, 0.0f, 0.0f))
		[
			SNew(SBox)
			.WidthOverride(420.0f)
			[
				SNew(SButton)
				.ButtonStyle(&MenuButtonStyle)
				.ContentPadding(FMargin(22.0f, 14.0f))
				.OnClicked(this, &SFantasyFrontierFrontEndWidget::HandleStartGame)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("StartGame", "Game Start"))
					.Font(ButtonFont)
					.Justification(ETextJustify::Center)
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(FMargin(0.0f, 14.0f, 0.0f, 0.0f))
		[
			SNew(SBox)
			.WidthOverride(420.0f)
			[
				SNew(SButton)
				.ButtonStyle(&MenuButtonStyle)
				.ContentPadding(FMargin(22.0f, 14.0f))
				.OnClicked(this, &SFantasyFrontierFrontEndWidget::HandleShowOptions)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Options", "Optionen"))
					.Font(ButtonFont)
					.Justification(ETextJustify::Center)
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(FMargin(0.0f, 14.0f, 0.0f, 0.0f))
		[
			SNew(SBox)
			.WidthOverride(420.0f)
			[
				SNew(SButton)
				.ButtonStyle(&MenuButtonStyle)
				.ContentPadding(FMargin(22.0f, 14.0f))
				.OnClicked(this, &SFantasyFrontierFrontEndWidget::HandleQuit)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("QuitGame", "Spiel verlassen"))
					.Font(ButtonFont)
					.Justification(ETextJustify::Center)
				]
			]
		];
}

TSharedRef<SWidget> SFantasyFrontierFrontEndWidget::BuildOptionsMenu()
{
	const FSlateFontInfo HeadingFont = MakeBodyFont(28);
	const FSlateFontInfo BodyFont = MakeBodyFont(22);

	return SNew(SVerticalBox)
		.Visibility(this, &SFantasyFrontierFrontEndWidget::GetOptionsVisibility)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(FMargin(0.0f, 0.0f, 0.0f, 10.0f))
		[
			SNew(STextBlock)
			.Text(LOCTEXT("OptionsTitle", "Optionen"))
			.Font(HeadingFont)
			.ColorAndOpacity(FLinearColor(0.97f, 0.92f, 0.82f))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(0.0f, 8.0f, 0.0f, 0.0f))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("WindowMode", "Fenstermodus"))
				.Font(BodyFont)
				.ColorAndOpacity(FLinearColor(0.85f, 0.80f, 0.70f))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(260.0f)
				[
					SNew(SButton)
					.ButtonStyle(&MenuButtonStyle)
					.ContentPadding(FMargin(20.0f, 12.0f))
					.OnClicked(this, &SFantasyFrontierFrontEndWidget::HandleCycleWindowMode)
					[
						SNew(STextBlock)
						.Text(WindowModeLabel)
						.Font(BodyFont)
						.Justification(ETextJustify::Center)
					]
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(0.0f, 14.0f, 0.0f, 0.0f))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Quality", "Grafikqualitaet"))
				.Font(BodyFont)
				.ColorAndOpacity(FLinearColor(0.85f, 0.80f, 0.70f))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(260.0f)
				[
					SNew(SButton)
					.ButtonStyle(&MenuButtonStyle)
					.ContentPadding(FMargin(20.0f, 12.0f))
					.OnClicked(this, &SFantasyFrontierFrontEndWidget::HandleCycleQuality)
					[
						SNew(STextBlock)
						.Text(QualityLabel)
						.Font(BodyFont)
						.Justification(ETextJustify::Center)
					]
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(FMargin(0.0f, 22.0f, 0.0f, 0.0f))
		[
			SNew(SBox)
			.WidthOverride(260.0f)
			[
				SNew(SButton)
				.ButtonStyle(&MenuButtonStyle)
				.ContentPadding(FMargin(20.0f, 12.0f))
				.OnClicked(this, &SFantasyFrontierFrontEndWidget::HandleCloseOptions)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Back", "Zurueck"))
					.Font(BodyFont)
					.Justification(ETextJustify::Center)
				]
			]
		];
}

#undef LOCTEXT_NAMESPACE
