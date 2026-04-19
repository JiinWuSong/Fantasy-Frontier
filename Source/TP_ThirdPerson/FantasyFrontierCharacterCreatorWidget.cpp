// Copyright Epic Games, Inc. All Rights Reserved.

#include "FantasyFrontierCharacterCreatorWidget.h"

#include "Brushes/SlateDynamicImageBrush.h"
#include "Engine/TextureRenderTarget2D.h"
#include "HAL/PlatformFileManager.h"
#include "InputCoreTypes.h"
#include "Misc/Paths.h"
#include "Styling/SlateTypes.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FantasyFrontierCharacterCreator"

namespace
{
	FButtonStyle MakeFilledButtonStyle(const FLinearColor& Normal, const FLinearColor& Hovered, const FLinearColor& Pressed)
	{
		return FButtonStyle()
			.SetNormal(FSlateRoundedBoxBrush(Normal, 24.0f, FLinearColor(0.93f, 0.82f, 0.56f, 0.14f), 1.0f))
			.SetHovered(FSlateRoundedBoxBrush(Hovered, 24.0f, FLinearColor(0.98f, 0.90f, 0.66f, 0.24f), 1.2f))
			.SetPressed(FSlateRoundedBoxBrush(Pressed, 24.0f, FLinearColor(0.76f, 0.60f, 0.28f, 0.18f), 1.0f))
			.SetNormalForeground(FLinearColor(0.98f, 0.96f, 0.91f))
			.SetHoveredForeground(FLinearColor::White)
			.SetPressedForeground(FLinearColor(0.96f, 0.94f, 0.88f));
	}

	bool IsSupportedNameCharacter(const TCHAR Character)
	{
		return FChar::IsAlpha(Character) || Character == TEXT('-') || Character == TEXT('\'') || Character == TEXT(' ');
	}

	TSharedPtr<FSlateDynamicImageBrush> LoadBrush(const TCHAR* RelativePath, const FVector2D& Size)
	{
		const FString FullPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / RelativePath);
		if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*FullPath))
		{
			return nullptr;
		}

		return MakeShared<FSlateDynamicImageBrush>(FName(*FullPath), Size);
	}

	class SCharacterPreviewSurface : public SCompoundWidget
	{
	public:
		SLATE_BEGIN_ARGS(SCharacterPreviewSurface)
			: _Brush(nullptr)
		{
		}
			SLATE_ARGUMENT(const FSlateBrush*, Brush)
			SLATE_EVENT(FFantasyFrontierPreviewRotate, OnRotate)
			SLATE_EVENT(FFantasyFrontierPreviewZoom, OnZoom)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			Brush = InArgs._Brush;
			OnRotate = InArgs._OnRotate;
			OnZoom = InArgs._OnZoom;

			ChildSlot
			[
				SNew(SImage)
				.Image(Brush)
			];
		}

		virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
		{
			if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
			{
				bIsDragging = true;
				LastScreenSpacePosition = MouseEvent.GetScreenSpacePosition();
				return FReply::Handled().CaptureMouse(AsShared());
			}

			return FReply::Unhandled();
		}

		virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
		{
			if (bIsDragging && MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
			{
				bIsDragging = false;
				return FReply::Handled().ReleaseMouseCapture();
			}

			return FReply::Unhandled();
		}

		virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
		{
			if (!bIsDragging || !HasMouseCapture())
			{
				return FReply::Unhandled();
			}

			const FVector2D CurrentPosition = MouseEvent.GetScreenSpacePosition();
			const float DeltaX = CurrentPosition.X - LastScreenSpacePosition.X;
			LastScreenSpacePosition = CurrentPosition;
			OnRotate.ExecuteIfBound(DeltaX * 0.42f);
			return FReply::Handled();
		}

		virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
		{
			OnZoom.ExecuteIfBound(MouseEvent.GetWheelDelta() * 0.08f);
			return FReply::Handled();
		}

		virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override
		{
			SCompoundWidget::OnMouseLeave(MouseEvent);
			if (!HasMouseCapture())
			{
				bIsDragging = false;
			}
		}

	private:
		const FSlateBrush* Brush = nullptr;
		FFantasyFrontierPreviewRotate OnRotate;
		FFantasyFrontierPreviewZoom OnZoom;
		bool bIsDragging = false;
		FVector2D LastScreenSpacePosition = FVector2D::ZeroVector;
	};
}

void SFantasyFrontierCharacterCreatorWidget::Construct(const FArguments& InArgs)
{
	OnDraftChanged = InArgs._OnDraftChanged;
	OnConfirm = InArgs._OnConfirm;
	OnCancel = InArgs._OnCancel;
	OnSavePreset = InArgs._OnSavePreset;
	OnLoadPreset = InArgs._OnLoadPreset;
	OnPreviewRotate = InArgs._OnPreviewRotate;
	OnPreviewZoom = InArgs._OnPreviewZoom;

	PreviewBrush = FSlateBrush();
	PreviewBrush.SetResourceObject(InArgs._PreviewTexture);
	PreviewBrush.ImageSize = FVector2D(1220.0f, 1760.0f);
	PreviewBrush.DrawAs = ESlateBrushDrawType::Image;

	BackgroundBrush = LoadBrush(TEXT("Slate/MenuBackground.png"), FVector2D(1920.0f, 1080.0f));
	SkyBrush = LoadBrush(TEXT("Slate/MenuSky.png"), FVector2D(1920.0f, 1080.0f));
	MidgroundBrush = LoadBrush(TEXT("Slate/MenuMidground.png"), FVector2D(1920.0f, 1080.0f));
	MistBrush = LoadBrush(TEXT("Slate/MenuMist.png"), FVector2D(1920.0f, 1080.0f));
	ForegroundBrush = LoadBrush(TEXT("Slate/MenuForeground.png"), FVector2D(1920.0f, 1080.0f));
	PanelTextureBrush = LoadBrush(TEXT("Slate/MenuPanel.png"), FVector2D(920.0f, 560.0f));
	DividerTextureBrush = LoadBrush(TEXT("Slate/TitleDivider.png"), FVector2D(1600.0f, 40.0f));

	PrimaryButtonStyle = MakeFilledButtonStyle(
		FLinearColor(0.66f, 0.50f, 0.23f, 0.98f),
		FLinearColor(0.82f, 0.63f, 0.29f, 0.99f),
		FLinearColor(0.52f, 0.37f, 0.17f, 1.0f));
	SecondaryButtonStyle = MakeFilledButtonStyle(
		FLinearColor(0.11f, 0.20f, 0.26f, 0.84f),
		FLinearColor(0.18f, 0.30f, 0.37f, 0.92f),
		FLinearColor(0.09f, 0.14f, 0.18f, 0.94f));
	CardButtonStyle = MakeFilledButtonStyle(
		FLinearColor(0.10f, 0.18f, 0.24f, 0.78f),
		FLinearColor(0.18f, 0.29f, 0.36f, 0.88f),
		FLinearColor(0.09f, 0.14f, 0.18f, 0.94f));

	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SImage)
			.Image(SkyBrush.IsValid() ? static_cast<const FSlateBrush*>(SkyBrush.Get()) : (BackgroundBrush.IsValid() ? static_cast<const FSlateBrush*>(BackgroundBrush.Get()) : &BackgroundFallbackBrush))
			.RenderTransform(this, &SFantasyFrontierCharacterCreatorWidget::GetSkyTransform)
			.RenderTransformPivot(FVector2D(0.5f, 0.5f))
		]
		+ SOverlay::Slot()
		[
			SNew(SImage)
			.Image(MidgroundBrush.IsValid() ? static_cast<const FSlateBrush*>(MidgroundBrush.Get()) : &BackgroundFallbackBrush)
			.RenderTransform(this, &SFantasyFrontierCharacterCreatorWidget::GetMidgroundTransform)
			.RenderTransformPivot(FVector2D(0.5f, 0.5f))
		]
		+ SOverlay::Slot()
		[
			SNew(SImage)
			.Image(MistBrush.IsValid() ? static_cast<const FSlateBrush*>(MistBrush.Get()) : &BackgroundFallbackBrush)
			.ColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 0.70f))
			.RenderTransform(this, &SFantasyFrontierCharacterCreatorWidget::GetMistTransform)
			.RenderTransformPivot(FVector2D(0.5f, 0.5f))
		]
		+ SOverlay::Slot()
		[
			SNew(SImage)
			.Image(ForegroundBrush.IsValid() ? static_cast<const FSlateBrush*>(ForegroundBrush.Get()) : &BackgroundFallbackBrush)
			.RenderTransform(this, &SFantasyFrontierCharacterCreatorWidget::GetForegroundTransform)
			.RenderTransformPivot(FVector2D(0.5f, 0.5f))
		]
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(&BackdropTintBrush)
		]
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(&HeroWashBrush)
		]
		+ SOverlay::Slot()
		.Padding(FMargin(20.0f))
		[
			BuildPreviewPane()
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.Padding(FMargin(28.0f, 62.0f, 0.0f, 34.0f))
		[
			SNew(SBox)
			.WidthOverride(432.0f)
			[
				BuildStepPane()
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		.Padding(FMargin(0.0f, 58.0f, 28.0f, 0.0f))
		[
			SNew(SBox)
			.WidthOverride(248.0f)
			.Visibility_Lambda([this]() { return ShouldShowRaceRail() ? EVisibility::Visible : EVisibility::Collapsed; })
			[
				BuildRaceRail()
			]
		]
	];

	BroadcastDraftChanged();
}

void SFantasyFrontierCharacterCreatorWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	AmbientTime += InDeltaTime;
}

void SFantasyFrontierCharacterCreatorWidget::SetDraft(const FFantasyFrontierCharacterDraft& InDraft)
{
	Draft = InDraft;
	BroadcastDraftChanged();
}

void SFantasyFrontierCharacterCreatorWidget::BroadcastDraftChanged()
{
	OnDraftChanged.ExecuteIfBound(Draft);
}

FReply SFantasyFrontierCharacterCreatorWidget::HandleNextStep()
{
	if (!CanAdvanceFromCurrentStep())
	{
		return FReply::Handled();
	}

	switch (CurrentStep)
	{
	case EFantasyFrontierCreatorStep::Race:
		CurrentStep = EFantasyFrontierCreatorStep::Gender;
		break;
	case EFantasyFrontierCreatorStep::Gender:
		CurrentStep = EFantasyFrontierCreatorStep::Appearance;
		break;
	case EFantasyFrontierCreatorStep::Appearance:
		CurrentStep = EFantasyFrontierCreatorStep::Name;
		break;
	case EFantasyFrontierCreatorStep::Name:
		CurrentStep = EFantasyFrontierCreatorStep::Class;
		break;
	case EFantasyFrontierCreatorStep::Class:
	default:
		break;
	}

	return FReply::Handled();
}

FReply SFantasyFrontierCharacterCreatorWidget::HandlePreviousStep()
{
	switch (CurrentStep)
	{
	case EFantasyFrontierCreatorStep::Race:
		OnCancel.ExecuteIfBound();
		break;
	case EFantasyFrontierCreatorStep::Gender:
		CurrentStep = EFantasyFrontierCreatorStep::Race;
		break;
	case EFantasyFrontierCreatorStep::Appearance:
		CurrentStep = EFantasyFrontierCreatorStep::Gender;
		break;
	case EFantasyFrontierCreatorStep::Name:
		CurrentStep = EFantasyFrontierCreatorStep::Appearance;
		break;
	case EFantasyFrontierCreatorStep::Class:
		CurrentStep = EFantasyFrontierCreatorStep::Name;
		break;
	}

	return FReply::Handled();
}

FReply SFantasyFrontierCharacterCreatorWidget::HandleConfirm()
{
	if (!CanFinish())
	{
		return FReply::Handled();
	}

	OnConfirm.ExecuteIfBound(Draft);
	return FReply::Handled();
}

FReply SFantasyFrontierCharacterCreatorWidget::HandleSelectRace(EFantasyFrontierRace Race)
{
	if (!IsRaceEnabled(Race))
	{
		return FReply::Handled();
	}

	Draft.Race = Race;
	BroadcastDraftChanged();
	return FReply::Handled();
}

FReply SFantasyFrontierCharacterCreatorWidget::HandleSelectGender(EFantasyFrontierGender Gender)
{
	Draft.Gender = Gender;
	BroadcastDraftChanged();
	return FReply::Handled();
}

FReply SFantasyFrontierCharacterCreatorWidget::HandleSelectClass(EFantasyFrontierClass CharacterClass)
{
	Draft.CharacterClass = CharacterClass;
	return FReply::Handled();
}

FReply SFantasyFrontierCharacterCreatorWidget::HandleSelectCustomizationTab(EFantasyFrontierCustomizationTab Tab)
{
	CurrentTab = Tab;
	return FReply::Handled();
}

FReply SFantasyFrontierCharacterCreatorWidget::HandleSelectStylePreset(int32 StylePreset)
{
	Draft.StylePreset = StylePreset;
	BroadcastDraftChanged();
	return FReply::Handled();
}

void SFantasyFrontierCharacterCreatorWidget::HandleHeightChanged(float NewValue)
{
	Draft.HeightScale = FMath::Lerp(0.92f, 1.08f, NewValue);
	BroadcastDraftChanged();
}

void SFantasyFrontierCharacterCreatorWidget::HandleBuildChanged(float NewValue)
{
	Draft.Build = NewValue;
	BroadcastDraftChanged();
}

void SFantasyFrontierCharacterCreatorWidget::HandleMusculatureChanged(float NewValue)
{
	Draft.Musculature = NewValue;
	BroadcastDraftChanged();
}

void SFantasyFrontierCharacterCreatorWidget::HandleSkinToneChanged(float NewValue)
{
	Draft.SkinTone = NewValue;
	BroadcastDraftChanged();
}

void SFantasyFrontierCharacterCreatorWidget::HandleScarChanged(float NewValue)
{
	Draft.ScarIntensity = NewValue;
	BroadcastDraftChanged();
}

void SFantasyFrontierCharacterCreatorWidget::HandleTattooChanged(float NewValue)
{
	Draft.TattooIntensity = NewValue;
	BroadcastDraftChanged();
}

void SFantasyFrontierCharacterCreatorWidget::HandleNameChanged(const FText& NewValue)
{
	Draft.CharacterName = NewValue.ToString();
}

void SFantasyFrontierCharacterCreatorWidget::HandlePreviewRotated(float YawDelta)
{
	OnPreviewRotate.ExecuteIfBound(YawDelta);
}

void SFantasyFrontierCharacterCreatorWidget::HandlePreviewZoomed(float ZoomDelta)
{
	OnPreviewZoom.ExecuteIfBound(ZoomDelta);
}

FReply SFantasyFrontierCharacterCreatorWidget::HandleSetPreviewMode(EFantasyFrontierPreviewMode PreviewMode)
{
	Draft.PreviewMode = PreviewMode;
	BroadcastDraftChanged();
	return FReply::Handled();
}

FReply SFantasyFrontierCharacterCreatorWidget::HandleSetOriginStyle(EFantasyFrontierOriginStyle OriginStyle)
{
	Draft.OriginStyle = OriginStyle;
	BroadcastDraftChanged();
	return FReply::Handled();
}

FReply SFantasyFrontierCharacterCreatorWidget::HandleSetHairStyle(int32 HairStyle)
{
	Draft.HairStyle = HairStyle;
	BroadcastDraftChanged();
	return FReply::Handled();
}

FReply SFantasyFrontierCharacterCreatorWidget::HandleSetFaceVariant(int32 FaceVariant)
{
	Draft.FaceVariant = FaceVariant;
	BroadcastDraftChanged();
	return FReply::Handled();
}

FReply SFantasyFrontierCharacterCreatorWidget::HandleCycleHairColor()
{
	static const TArray<FLinearColor> Palette = {
		FLinearColor(0.16f, 0.13f, 0.10f, 1.0f),
		FLinearColor(0.49f, 0.24f, 0.18f, 1.0f),
		FLinearColor(0.79f, 0.67f, 0.31f, 1.0f),
		FLinearColor(0.14f, 0.31f, 0.52f, 1.0f),
		FLinearColor(0.72f, 0.74f, 0.81f, 1.0f)
	};

	const int32 CurrentIndex = Palette.IndexOfByPredicate([this](const FLinearColor& Candidate)
	{
		return Candidate.Equals(Draft.HairColor, 0.01f);
	});

	Draft.HairColor = Palette[(CurrentIndex + 1 + Palette.Num()) % Palette.Num()];
	BroadcastDraftChanged();
	return FReply::Handled();
}

FReply SFantasyFrontierCharacterCreatorWidget::HandleCycleEyeColor()
{
	static const TArray<FLinearColor> Palette = {
		FLinearColor(0.25f, 0.73f, 0.96f, 1.0f),
		FLinearColor(0.40f, 0.82f, 0.54f, 1.0f),
		FLinearColor(0.86f, 0.51f, 0.28f, 1.0f),
		FLinearColor(0.78f, 0.42f, 0.86f, 1.0f)
	};

	const int32 CurrentIndex = Palette.IndexOfByPredicate([this](const FLinearColor& Candidate)
	{
		return Candidate.Equals(Draft.EyeColor, 0.01f);
	});

	Draft.EyeColor = Palette[(CurrentIndex + 1 + Palette.Num()) % Palette.Num()];
	BroadcastDraftChanged();
	return FReply::Handled();
}

FReply SFantasyFrontierCharacterCreatorWidget::HandleSavePreset()
{
	OnSavePreset.ExecuteIfBound();
	return FReply::Handled();
}

FReply SFantasyFrontierCharacterCreatorWidget::HandleLoadPreset()
{
	OnLoadPreset.ExecuteIfBound();
	BroadcastDraftChanged();
	return FReply::Handled();
}

bool SFantasyFrontierCharacterCreatorWidget::IsRaceEnabled(EFantasyFrontierRace Race) const
{
	return Race == EFantasyFrontierRace::Human;
}

bool SFantasyFrontierCharacterCreatorWidget::IsNameValid() const
{
	const FString TrimmedName = Draft.CharacterName.TrimStartAndEnd();
	if (TrimmedName.Len() < 3 || TrimmedName.Len() > 16)
	{
		return false;
	}

	for (const TCHAR Character : TrimmedName)
	{
		if (!IsSupportedNameCharacter(Character))
		{
			return false;
		}
	}

	return true;
}

bool SFantasyFrontierCharacterCreatorWidget::CanAdvanceFromCurrentStep() const
{
	switch (CurrentStep)
	{
	case EFantasyFrontierCreatorStep::Race:
		return Draft.Race == EFantasyFrontierRace::Human;
	case EFantasyFrontierCreatorStep::Gender:
	case EFantasyFrontierCreatorStep::Appearance:
		return true;
	case EFantasyFrontierCreatorStep::Name:
		return IsNameValid();
	case EFantasyFrontierCreatorStep::Class:
	default:
		return false;
	}
}

bool SFantasyFrontierCharacterCreatorWidget::CanFinish() const
{
	return IsNameValid();
}

int32 SFantasyFrontierCharacterCreatorWidget::GetStepIndex() const
{
	switch (CurrentStep)
	{
	case EFantasyFrontierCreatorStep::Race:
		return 0;
	case EFantasyFrontierCreatorStep::Gender:
		return 1;
	case EFantasyFrontierCreatorStep::Appearance:
		return 2;
	case EFantasyFrontierCreatorStep::Name:
		return 3;
	case EFantasyFrontierCreatorStep::Class:
	default:
		return 4;
	}
}

int32 SFantasyFrontierCharacterCreatorWidget::GetCustomizationTabIndex() const
{
	switch (CurrentTab)
	{
	case EFantasyFrontierCustomizationTab::Body:
		return 0;
	case EFantasyFrontierCustomizationTab::HairStyle:
		return 1;
	case EFantasyFrontierCustomizationTab::Face:
		return 2;
	case EFantasyFrontierCustomizationTab::Markings:
	default:
		return 3;
	}
}

bool SFantasyFrontierCharacterCreatorWidget::ShouldShowRaceRail() const
{
	return CurrentStep == EFantasyFrontierCreatorStep::Race || CurrentStep == EFantasyFrontierCreatorStep::Gender;
}

FText SFantasyFrontierCharacterCreatorWidget::GetStepTitle() const
{
	switch (CurrentStep)
	{
	case EFantasyFrontierCreatorStep::Race:
		return LOCTEXT("RaceStepTitle", "Choose a Race");
	case EFantasyFrontierCreatorStep::Gender:
		return LOCTEXT("GenderStepTitle", "Choose a Body Type");
	case EFantasyFrontierCreatorStep::Appearance:
		return LOCTEXT("AppearanceStepTitle", "Refine Your Hero");
	case EFantasyFrontierCreatorStep::Name:
		return LOCTEXT("NameStepTitle", "Choose a Name");
	case EFantasyFrontierCreatorStep::Class:
	default:
		return LOCTEXT("ClassStepTitle", "Choose a Starting Class");
	}
}

FText SFantasyFrontierCharacterCreatorWidget::GetStepBody() const
{
	switch (CurrentStep)
	{
	case EFantasyFrontierCreatorStep::Race:
		return LOCTEXT("RaceStepBody", "Humans are live for this sprint. The remaining races stay visible as roadmap-ready slots so the creator already reads like a full MMORPG flow.");
	case EFantasyFrontierCreatorStep::Gender:
		return LOCTEXT("GenderStepBody", "Pick the adult hero frame you want to shape. The same body continues into appearance, presets and gameplay.");
	case EFantasyFrontierCreatorStep::Appearance:
		return LOCTEXT("AppearanceStepBody", "Adjust the editable base body first, then preview how starter gear and origin styling layer over the same character.");
	case EFantasyFrontierCreatorStep::Name:
		return LOCTEXT("NameStepBody", "Choose the adventurer name that will carry into the tutorial slice. Validation is local for now, but follows the final intended flow.");
	case EFantasyFrontierCreatorStep::Class:
	default:
		return LOCTEXT("ClassStepBody", "Classes set your opening direction, not a permanent lock. This final choice should feel decisive before you enter the frontier.");
	}
}

FText SFantasyFrontierCharacterCreatorWidget::GetStepProgressText() const
{
	return FText::Format(LOCTEXT("StepProgress", "STEP {0} / 5"), FText::AsNumber(GetStepIndex() + 1));
}

FText SFantasyFrontierCharacterCreatorWidget::GetNextButtonText() const
{
	switch (CurrentStep)
	{
	case EFantasyFrontierCreatorStep::Race:
		return LOCTEXT("NextRace", "Continue to Body Frame");
	case EFantasyFrontierCreatorStep::Gender:
		return LOCTEXT("NextGender", "Continue to Appearance");
	case EFantasyFrontierCreatorStep::Appearance:
		return LOCTEXT("NextAppearance", "Continue to Naming");
	case EFantasyFrontierCreatorStep::Name:
		return LOCTEXT("NextName", "Continue to Class");
	case EFantasyFrontierCreatorStep::Class:
	default:
		return LOCTEXT("NextClassLocked", "Ready to Enter");
	}
}

FText SFantasyFrontierCharacterCreatorWidget::GetConfirmButtonText() const
{
	return LOCTEXT("ConfirmCreator", "Enter the Frontier");
}

FText SFantasyFrontierCharacterCreatorWidget::GetBackButtonText() const
{
	return CurrentStep == EFantasyFrontierCreatorStep::Race ? LOCTEXT("BackToTitle", "Back to Title") : LOCTEXT("BackStep", "Back");
}

FText SFantasyFrontierCharacterCreatorWidget::GetRaceLabel(EFantasyFrontierRace Race) const
{
	switch (Race)
	{
	case EFantasyFrontierRace::Human:
		return LOCTEXT("RaceHuman", "Humans");
	case EFantasyFrontierRace::Wolfkin:
		return LOCTEXT("RaceWolfkin", "Wolfkin");
	case EFantasyFrontierRace::Orc:
		return LOCTEXT("RaceOrc", "Orcs");
	case EFantasyFrontierRace::Dwarf:
		return LOCTEXT("RaceDwarf", "Dwarves");
	case EFantasyFrontierRace::Elf:
		return LOCTEXT("RaceElf", "Elves");
	case EFantasyFrontierRace::Stagborn:
		return LOCTEXT("RaceStagborn", "Stagborn");
	case EFantasyFrontierRace::Drakyn:
	default:
		return LOCTEXT("RaceDrakyn", "Drakyn");
	}
}

FText SFantasyFrontierCharacterCreatorWidget::GetGenderLabel(EFantasyFrontierGender Gender) const
{
	return Gender == EFantasyFrontierGender::Female ? LOCTEXT("GenderFemale", "Female") : LOCTEXT("GenderMale", "Male");
}

FText SFantasyFrontierCharacterCreatorWidget::GetClassLabel(EFantasyFrontierClass CharacterClass) const
{
	switch (CharacterClass)
	{
	case EFantasyFrontierClass::Vanguard:
		return LOCTEXT("ClassVanguard", "Vanguard");
	case EFantasyFrontierClass::Spellweaver:
		return LOCTEXT("ClassSpellweaver", "Spellweaver");
	case EFantasyFrontierClass::Lancer:
	default:
		return LOCTEXT("ClassLancer", "Lancer");
	}
}

FText SFantasyFrontierCharacterCreatorWidget::GetDraftHeadline() const
{
	return FText::Format(LOCTEXT("DraftHeadline", "{0}  |  {1}"), GetRaceLabel(Draft.Race), GetGenderLabel(Draft.Gender));
}

FText SFantasyFrontierCharacterCreatorWidget::GetDraftSubtitle() const
{
	return LOCTEXT("DraftSubtitle", "Rotate with the left mouse button. Use the wheel to zoom. The same body carries through every preview mode.");
}

FText SFantasyFrontierCharacterCreatorWidget::GetPreviewFrameLabel() const
{
	return LOCTEXT("PreviewFrameLabel", "HERO PREVIEW");
}

FText SFantasyFrontierCharacterCreatorWidget::GetNameValidationText() const
{
	const FString TrimmedName = Draft.CharacterName.TrimStartAndEnd();
	if (TrimmedName.IsEmpty())
	{
		return LOCTEXT("NameValidationEmpty", "Enter a name between 3 and 16 letters. Spaces, apostrophes and hyphens are allowed.");
	}

	if (IsNameValid())
	{
		return LOCTEXT("NameValidationOk", "Name accepted for local testing. Online validation can be layered in later without changing this flow.");
	}

	return LOCTEXT("NameValidationBad", "Name invalid. Use 3 to 16 letters with optional spaces, apostrophes or hyphens.");
}

FText SFantasyFrontierCharacterCreatorWidget::GetHeightValueText() const
{
	return FText::AsPercent((Draft.HeightScale - 0.92f) / (1.08f - 0.92f));
}

FText SFantasyFrontierCharacterCreatorWidget::GetBuildValueText() const
{
	return FText::AsPercent(Draft.Build);
}

FText SFantasyFrontierCharacterCreatorWidget::GetMusculatureValueText() const
{
	return FText::AsPercent(Draft.Musculature);
}

FText SFantasyFrontierCharacterCreatorWidget::GetSkinToneValueText() const
{
	return FText::AsPercent(Draft.SkinTone);
}

FText SFantasyFrontierCharacterCreatorWidget::GetScarValueText() const
{
	return FText::AsPercent(Draft.ScarIntensity);
}

FText SFantasyFrontierCharacterCreatorWidget::GetTattooValueText() const
{
	return FText::AsPercent(Draft.TattooIntensity);
}

FSlateColor SFantasyFrontierCharacterCreatorWidget::GetRaceButtonTint(EFantasyFrontierRace Race) const
{
	if (Draft.Race == Race)
	{
		return FSlateColor(FLinearColor(0.98f, 0.90f, 0.70f));
	}

	return FSlateColor(IsRaceEnabled(Race) ? FLinearColor(0.88f, 0.92f, 0.96f) : FLinearColor(0.56f, 0.61f, 0.68f));
}

FSlateColor SFantasyFrontierCharacterCreatorWidget::GetGenderButtonTint(EFantasyFrontierGender Gender) const
{
	return FSlateColor(Draft.Gender == Gender ? FLinearColor(0.97f, 0.90f, 0.74f) : FLinearColor(0.86f, 0.91f, 0.96f));
}

FSlateColor SFantasyFrontierCharacterCreatorWidget::GetClassButtonTint(EFantasyFrontierClass CharacterClass) const
{
	return FSlateColor(Draft.CharacterClass == CharacterClass ? FLinearColor(0.98f, 0.90f, 0.74f) : FLinearColor(0.84f, 0.90f, 0.95f));
}

FSlateColor SFantasyFrontierCharacterCreatorWidget::GetStyleButtonTint(int32 StylePreset) const
{
	return FSlateColor(Draft.StylePreset == StylePreset ? FLinearColor(0.98f, 0.90f, 0.74f) : FLinearColor(0.84f, 0.90f, 0.95f));
}

FSlateColor SFantasyFrontierCharacterCreatorWidget::GetTabTint(EFantasyFrontierCustomizationTab Tab) const
{
	return FSlateColor(CurrentTab == Tab ? FLinearColor(0.97f, 0.89f, 0.72f) : FLinearColor(0.76f, 0.84f, 0.92f));
}

FSlateColor SFantasyFrontierCharacterCreatorWidget::GetPreviewModeTint(EFantasyFrontierPreviewMode PreviewMode) const
{
	return FSlateColor(Draft.PreviewMode == PreviewMode ? FLinearColor(0.98f, 0.91f, 0.76f) : FLinearColor(0.80f, 0.87f, 0.93f));
}

FSlateColor SFantasyFrontierCharacterCreatorWidget::GetOriginStyleTint(EFantasyFrontierOriginStyle OriginStyle) const
{
	return FSlateColor(Draft.OriginStyle == OriginStyle ? FLinearColor(0.98f, 0.91f, 0.76f) : FLinearColor(0.80f, 0.87f, 0.93f));
}

FSlateColor SFantasyFrontierCharacterCreatorWidget::GetHairStyleTint(int32 HairStyle) const
{
	return FSlateColor(Draft.HairStyle == HairStyle ? FLinearColor(0.98f, 0.91f, 0.76f) : FLinearColor(0.80f, 0.87f, 0.93f));
}

FSlateColor SFantasyFrontierCharacterCreatorWidget::GetFaceVariantTint(int32 FaceVariant) const
{
	return FSlateColor(Draft.FaceVariant == FaceVariant ? FLinearColor(0.98f, 0.91f, 0.76f) : FLinearColor(0.80f, 0.87f, 0.93f));
}

FSlateFontInfo SFantasyFrontierCharacterCreatorWidget::MakeDisplayFont(int32 Size, int32 LetterSpacing) const
{
	FSlateFontInfo Font(FPaths::ProjectContentDir() / TEXT("Slate/Fonts/Georgia-Bold.ttf"), Size);
	Font.LetterSpacing = LetterSpacing;
	return Font;
}

FSlateFontInfo SFantasyFrontierCharacterCreatorWidget::MakeBodyFont(int32 Size) const
{
	return FSlateFontInfo(FPaths::ProjectContentDir() / TEXT("Slate/Fonts/Georgia-Bold.ttf"), Size);
}

TOptional<FSlateRenderTransform> SFantasyFrontierCharacterCreatorWidget::GetSkyTransform() const
{
	return FSlateRenderTransform(
		FVector2D(
			FMath::Sin(AmbientTime * 0.04) * 10.0f,
			FMath::Cos(AmbientTime * 0.03) * 8.0f));
}

TOptional<FSlateRenderTransform> SFantasyFrontierCharacterCreatorWidget::GetMidgroundTransform() const
{
	return FSlateRenderTransform(
		FVector2D(
			FMath::Sin(AmbientTime * 0.08) * 18.0f - 32.0f,
			FMath::Cos(AmbientTime * 0.05) * 6.0f));
}

TOptional<FSlateRenderTransform> SFantasyFrontierCharacterCreatorWidget::GetMistTransform() const
{
	return FSlateRenderTransform(
		FVector2D(
			FMath::Sin(AmbientTime * 0.12) * -24.0f,
			FMath::Cos(AmbientTime * 0.06) * 3.0f));
}

TOptional<FSlateRenderTransform> SFantasyFrontierCharacterCreatorWidget::GetForegroundTransform() const
{
	return FSlateRenderTransform(
		FVector2D(
			FMath::Sin(AmbientTime * 0.10) * 28.0f,
			FMath::Cos(AmbientTime * 0.07) * 9.0f));
}

TOptional<FSlateRenderTransform> SFantasyFrontierCharacterCreatorWidget::GetHeroTransform() const
{
	return FSlateRenderTransform(
		FVector2D(
			0.0f,
			FMath::Sin(AmbientTime * 0.65) * 5.0f));
}

TSharedRef<SWidget> SFantasyFrontierCharacterCreatorWidget::BuildRaceStep()
{
	return SNew(SBorder)
		.BorderImage(&PanelBrush)
		.Padding(FMargin(22.0f, 22.0f, 22.0f, 24.0f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("RaceCurrentTitle", "Humans lead this first playable frontier slice."))
				.Font(MakeBodyFont(22))
				.ColorAndOpacity(FLinearColor(0.97f, 0.90f, 0.72f))
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 12.0f, 0.0f, 0.0f))
			[
				SNew(STextBlock)
				.Text(LOCTEXT("RaceCurrentBody", "Wolfkin, Orcs, Dwarves, Elves, Stagborn and Drakyn remain visible as planned expansions, but the human path is the stable player-facing route for this sprint."))
				.Font(MakeBodyFont(15))
				.ColorAndOpacity(FLinearColor(0.80f, 0.87f, 0.93f))
				.WrapTextAt(300.0f)
			]
		];
}

TSharedRef<SWidget> SFantasyFrontierCharacterCreatorWidget::BuildRaceRail()
{
	auto MakeRaceCard = [this](EFantasyFrontierRace Race)
	{
		const bool bEnabled = IsRaceEnabled(Race);
		return SNew(SBorder)
			.BorderImage(&PanelBrush)
			.Padding(FMargin(10.0f))
			[
				SNew(SButton)
				.ButtonStyle(&CardButtonStyle)
				.IsEnabled(bEnabled)
				.ContentPadding(FMargin(14.0f, 14.0f))
				.OnClicked(this, &SFantasyFrontierCharacterCreatorWidget::HandleSelectRace, Race)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock)
						.Text(GetRaceLabel(Race))
						.Font(MakeDisplayFont(20))
						.ColorAndOpacity(this, &SFantasyFrontierCharacterCreatorWidget::GetRaceButtonTint, Race)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 4.0f, 0.0f, 0.0f))
					[
						SNew(STextBlock)
						.Text(bEnabled ? LOCTEXT("RaceStatusLive", "Available now") : LOCTEXT("RaceStatusLater", "Planned next"))
						.Font(MakeBodyFont(14))
						.ColorAndOpacity(bEnabled ? FLinearColor(0.78f, 0.88f, 0.95f) : FLinearColor(0.55f, 0.62f, 0.69f))
					]
				]
			];
	};

	return SNew(SBox)
		.WidthOverride(248.0f)
		[
			SNew(SBorder)
			.BorderImage(&PanelBrush)
			.Padding(FMargin(14.0f, 18.0f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
						.Text(LOCTEXT("RaceRailTitle", "Races"))
					.Font(MakeDisplayFont(30))
					.ColorAndOpacity(FLinearColor(0.98f, 0.94f, 0.84f))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 10.0f, 0.0f, 20.0f))
				[
					SNew(STextBlock)
						.Text(LOCTEXT("RaceRailBody", "Only the human route is active right now. The remaining races stay staged here so the final structure is already locked in."))
					.Font(MakeBodyFont(14))
					.ColorAndOpacity(FLinearColor(0.78f, 0.85f, 0.91f))
					.WrapTextAt(200.0f)
				]
				+ SVerticalBox::Slot().FillHeight(1.0f)
				[
					SNew(SScrollBox)
					+ SScrollBox::Slot()[MakeRaceCard(EFantasyFrontierRace::Human)]
					+ SScrollBox::Slot()[MakeRaceCard(EFantasyFrontierRace::Wolfkin)]
					+ SScrollBox::Slot()[MakeRaceCard(EFantasyFrontierRace::Orc)]
					+ SScrollBox::Slot()[MakeRaceCard(EFantasyFrontierRace::Dwarf)]
					+ SScrollBox::Slot()[MakeRaceCard(EFantasyFrontierRace::Elf)]
					+ SScrollBox::Slot()[MakeRaceCard(EFantasyFrontierRace::Stagborn)]
					+ SScrollBox::Slot()[MakeRaceCard(EFantasyFrontierRace::Drakyn)]
				]
			]
		];
}

TSharedRef<SWidget> SFantasyFrontierCharacterCreatorWidget::BuildPreviewPane()
{
	auto MakePreviewToggle = [this](const FText& Label, EFantasyFrontierPreviewMode PreviewMode)
	{
		return SNew(SButton)
			.ButtonStyle(&SecondaryButtonStyle)
			.ContentPadding(FMargin(16.0f, 10.0f))
			.OnClicked(this, &SFantasyFrontierCharacterCreatorWidget::HandleSetPreviewMode, PreviewMode)
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(MakeBodyFont(17))
				.ColorAndOpacity(this, &SFantasyFrontierCharacterCreatorWidget::GetPreviewModeTint, PreviewMode)
			];
	};

	return SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Fill)
		.Padding(FMargin(330.0f, 36.0f, 250.0f, 78.0f))
		[
			SNew(SBox)
			.WidthOverride(1040.0f)
			.HeightOverride(1420.0f)
			.RenderTransform(this, &SFantasyFrontierCharacterCreatorWidget::GetHeroTransform)
			.RenderTransformPivot(FVector2D(0.5f, 0.5f))
			[
				SNew(SCharacterPreviewSurface)
				.Brush(&PreviewBrush)
				.OnRotate(OnPreviewRotate)
				.OnZoom(OnPreviewZoom)
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		.Padding(FMargin(360.0f, 46.0f, 250.0f, 0.0f))
		[
			SNew(SBorder)
			.BorderImage(&GlassCardBrush)
			.Padding(FMargin(24.0f, 20.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.3f).Padding(FMargin(0.0f, 0.0f, 16.0f, 0.0f))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock)
						.Text(this, &SFantasyFrontierCharacterCreatorWidget::GetPreviewFrameLabel)
						.Font(MakeBodyFont(16))
						.ColorAndOpacity(FLinearColor(0.83f, 0.89f, 0.94f))
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 6.0f, 0.0f, 0.0f))
					[
						SNew(STextBlock)
						.Text(this, &SFantasyFrontierCharacterCreatorWidget::GetDraftHeadline)
						.Font(MakeDisplayFont(34))
						.ColorAndOpacity(FLinearColor(0.99f, 0.95f, 0.86f))
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 6.0f, 0.0f, 0.0f))
					[
						SNew(STextBlock)
						.Text(this, &SFantasyFrontierCharacterCreatorWidget::GetDraftSubtitle)
						.Font(MakeBodyFont(14))
						.ColorAndOpacity(FLinearColor(0.78f, 0.85f, 0.92f))
						.WrapTextAt(520.0f)
					]
				]
				+ SHorizontalBox::Slot().FillWidth(0.7f)
				[
					SNew(SBorder)
					.BorderImage(&PreviewFrameBrush)
					.Padding(FMargin(18.0f, 14.0f))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("CreatorSceneTitle", "Scenic Creator View"))
							.Font(MakeBodyFont(18))
							.ColorAndOpacity(FLinearColor(0.96f, 0.92f, 0.82f))
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 6.0f, 0.0f, 0.0f))
						[
							SNew(STextBlock)
							.Text(LOCTEXT("CreatorSceneBody", "Rotate the hero against a calm wilderness backdrop. Keep the body centered and readable instead of boxed into a dev-style preview frame."))
							.Font(MakeBodyFont(13))
							.ColorAndOpacity(FLinearColor(0.76f, 0.84f, 0.90f))
							.WrapTextAt(290.0f)
						]
					]
				]
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Bottom)
		.Padding(FMargin(410.0f, 0.0f, 290.0f, 164.0f))
		[
			SNew(SBorder)
			.BorderImage(&GlassCardBrush)
			.Padding(FMargin(16.0f, 14.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(FMargin(0.0f, 0.0f, 10.0f, 0.0f))[MakePreviewToggle(LOCTEXT("PreviewBase", "Base Body"), EFantasyFrontierPreviewMode::BaseBody)]
				+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(FMargin(0.0f, 0.0f, 10.0f, 0.0f))[MakePreviewToggle(LOCTEXT("PreviewStarter", "Starter Gear"), EFantasyFrontierPreviewMode::StarterGear)]
				+ SHorizontalBox::Slot().FillWidth(1.0f)[MakePreviewToggle(LOCTEXT("PreviewOrigin", "Origin Style"), EFantasyFrontierPreviewMode::OriginStyle)]
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Bottom)
		.Padding(FMargin(460.0f, 0.0f, 340.0f, 54.0f))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(FMargin(0.0f, 0.0f, 10.0f, 0.0f))[BuildStatLine(LOCTEXT("PresetLabel", "Preset"), TAttribute<FText>::CreateLambda([this]() { return FText::Format(LOCTEXT("PresetValue", "Style {0}"), FText::AsNumber(Draft.StylePreset + 1)); }))]
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(FMargin(0.0f, 0.0f, 10.0f, 0.0f))[BuildStatLine(LOCTEXT("PreviewBodyTypeLabel", "Body"), TAttribute<FText>::CreateLambda([this]() { return Draft.Gender == EFantasyFrontierGender::Female ? LOCTEXT("PreviewBodyFemale", "Female Frame") : LOCTEXT("PreviewBodyMale", "Male Frame"); }))]
			+ SHorizontalBox::Slot().FillWidth(1.0f)[BuildStatLine(LOCTEXT("PreviewModeLabel", "Preview"), TAttribute<FText>::CreateLambda([this]() { return Draft.PreviewMode == EFantasyFrontierPreviewMode::BaseBody ? LOCTEXT("ModeBase", "Base Body") : Draft.PreviewMode == EFantasyFrontierPreviewMode::StarterGear ? LOCTEXT("ModeStarter", "Starter Gear") : LOCTEXT("ModeOrigin", "Origin Style"); }))]
		];
}

TSharedRef<SWidget> SFantasyFrontierCharacterCreatorWidget::BuildStepPane()
{
	return SNew(SBorder)
		.BorderImage(&PanelBrush)
		.Padding(FMargin(28.0f, 28.0f, 28.0f, 24.0f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(this, &SFantasyFrontierCharacterCreatorWidget::GetStepProgressText)
				.Font(MakeBodyFont(18))
				.ColorAndOpacity(FLinearColor(0.80f, 0.88f, 0.94f))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 10.0f, 0.0f, 0.0f))
			[
				SNew(STextBlock)
				.Text(this, &SFantasyFrontierCharacterCreatorWidget::GetStepTitle)
				.Font(MakeDisplayFont(44))
				.ColorAndOpacity(FLinearColor(0.99f, 0.95f, 0.86f))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 10.0f, 0.0f, 18.0f))
			[
				SNew(STextBlock)
				.Text(this, &SFantasyFrontierCharacterCreatorWidget::GetStepBody)
				.Font(MakeBodyFont(16))
				.ColorAndOpacity(FLinearColor(0.80f, 0.87f, 0.93f))
				.WrapTextAt(320.0f)
			]
			+ SVerticalBox::Slot().FillHeight(1.0f)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SNew(SWidgetSwitcher)
					.WidgetIndex(this, &SFantasyFrontierCharacterCreatorWidget::GetStepIndex)
					+ SWidgetSwitcher::Slot()[BuildRaceStep()]
					+ SWidgetSwitcher::Slot()[BuildGenderStep()]
					+ SWidgetSwitcher::Slot()[BuildAppearanceStep()]
					+ SWidgetSwitcher::Slot()[BuildNameStep()]
					+ SWidgetSwitcher::Slot()[BuildClassStep()]
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 20.0f, 0.0f, 0.0f))
			[
				BuildFooter()
			]
		];
}

TSharedRef<SWidget> SFantasyFrontierCharacterCreatorWidget::BuildGenderStep()
{
	auto MakeGenderCard = [this](EFantasyFrontierGender Gender, const FText& Title, const FText& Body)
	{
		return SNew(SBorder)
			.BorderImage(&PanelBrush)
			.Padding(FMargin(14.0f))
			[
				SNew(SButton)
				.ButtonStyle(&CardButtonStyle)
				.ContentPadding(FMargin(22.0f, 20.0f))
				.OnClicked(this, &SFantasyFrontierCharacterCreatorWidget::HandleSelectGender, Gender)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock)
						.Text(Title)
						.Font(MakeDisplayFont(34))
						.ColorAndOpacity(this, &SFantasyFrontierCharacterCreatorWidget::GetGenderButtonTint, Gender)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 10.0f, 0.0f, 0.0f))
					[
						SNew(STextBlock)
						.Text(Body)
						.Font(MakeBodyFont(17))
						.ColorAndOpacity(FLinearColor(0.80f, 0.87f, 0.93f))
						.WrapTextAt(230.0f)
					]
				]
			];
	};

	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(FMargin(0.0f, 0.0f, 10.0f, 0.0f))
		[
			MakeGenderCard(
				EFantasyFrontierGender::Female,
				LOCTEXT("FemaleCardTitle", "Female"),
				LOCTEXT("FemaleCardBody", "Elegant fantasy proportions with a softer silhouette, tuned for a premium MMORPG creator instead of a dev-only body frame."))
		]
		+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(FMargin(10.0f, 0.0f, 0.0f, 0.0f))
		[
			MakeGenderCard(
				EFantasyFrontierGender::Male,
				LOCTEXT("MaleCardTitle", "Male"),
				LOCTEXT("MaleCardBody", "A sturdier adult hero frame that reads clearly in combat while staying clean, attractive and anime-leaning in proportion."))
		];
}

TSharedRef<SWidget> SFantasyFrontierCharacterCreatorWidget::BuildAppearanceStep()
{
	auto MakeTabButton = [this](const FText& Label, EFantasyFrontierCustomizationTab Tab)
	{
		return SNew(SButton)
			.ButtonStyle(&SecondaryButtonStyle)
			.ContentPadding(FMargin(16.0f, 10.0f))
			.OnClicked(this, &SFantasyFrontierCharacterCreatorWidget::HandleSelectCustomizationTab, Tab)
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(MakeBodyFont(17))
				.ColorAndOpacity(this, &SFantasyFrontierCharacterCreatorWidget::GetTabTint, Tab)
			];
	};

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(FMargin(0.0f, 0.0f, 8.0f, 0.0f))[MakeTabButton(LOCTEXT("BodyTab", "Body"), EFantasyFrontierCustomizationTab::Body)]
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(FMargin(0.0f, 0.0f, 8.0f, 0.0f))[MakeTabButton(LOCTEXT("HairTab", "Hair"), EFantasyFrontierCustomizationTab::HairStyle)]
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(FMargin(0.0f, 0.0f, 8.0f, 0.0f))[MakeTabButton(LOCTEXT("FaceTab", "Face"), EFantasyFrontierCustomizationTab::Face)]
			+ SHorizontalBox::Slot().FillWidth(1.0f)[MakeTabButton(LOCTEXT("MarkingsTab", "Markings"), EFantasyFrontierCustomizationTab::Markings)]
		]
		+ SVerticalBox::Slot().FillHeight(1.0f).Padding(FMargin(0.0f, 18.0f, 0.0f, 0.0f))
		[
			SNew(SWidgetSwitcher)
			.WidgetIndex(this, &SFantasyFrontierCharacterCreatorWidget::GetCustomizationTabIndex)
			+ SWidgetSwitcher::Slot()[BuildBodyTab()]
			+ SWidgetSwitcher::Slot()[BuildHairStyleTab()]
			+ SWidgetSwitcher::Slot()[BuildFaceTab()]
			+ SWidgetSwitcher::Slot()[BuildMarkingsTab()]
		];
}

TSharedRef<SWidget> SFantasyFrontierCharacterCreatorWidget::BuildNameStep()
{
	return SNew(SBorder)
		.BorderImage(&PanelBrush)
		.Padding(FMargin(22.0f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("NameStepLead", "Character Name"))
				.Font(MakeBodyFont(22))
				.ColorAndOpacity(FLinearColor(0.97f, 0.91f, 0.76f))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 12.0f, 0.0f, 0.0f))
			[
				SNew(SEditableTextBox)
				.Text_Lambda([this]() { return FText::FromString(Draft.CharacterName); })
				.OnTextChanged(this, &SFantasyFrontierCharacterCreatorWidget::HandleNameChanged)
				.Font(MakeBodyFont(20))
				.Padding(FMargin(16.0f, 12.0f))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 12.0f, 0.0f, 0.0f))
			[
				SNew(STextBlock)
				.Text(this, &SFantasyFrontierCharacterCreatorWidget::GetNameValidationText)
				.Font(MakeBodyFont(16))
				.ColorAndOpacity_Lambda([this]()
				{
					return IsNameValid() || Draft.CharacterName.TrimStartAndEnd().IsEmpty()
						? FLinearColor(0.78f, 0.87f, 0.93f)
						: FLinearColor(0.95f, 0.67f, 0.61f);
				})
				.WrapTextAt(420.0f)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 20.0f, 0.0f, 0.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(FMargin(0.0f, 0.0f, 8.0f, 0.0f))[BuildStatLine(LOCTEXT("NameRaceLabel", "Race"), GetRaceLabel(Draft.Race))]
				+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(FMargin(0.0f, 0.0f, 8.0f, 0.0f))[BuildStatLine(LOCTEXT("NameGenderLabel", "Frame"), GetGenderLabel(Draft.Gender))]
				+ SHorizontalBox::Slot().FillWidth(1.0f)[BuildStatLine(LOCTEXT("NamePreviewLabel", "Preview"), Draft.PreviewMode == EFantasyFrontierPreviewMode::BaseBody ? LOCTEXT("NamePreviewBody", "Base Body") : Draft.PreviewMode == EFantasyFrontierPreviewMode::StarterGear ? LOCTEXT("NamePreviewGear", "Starter Gear") : LOCTEXT("NamePreviewOrigin", "Origin Style"))]
			]
		];
}

TSharedRef<SWidget> SFantasyFrontierCharacterCreatorWidget::BuildClassStep()
{
	auto MakeClassCard = [this](EFantasyFrontierClass CharacterClass, const FText& Title, const FText& Body)
	{
		return SNew(SBorder)
			.BorderImage(&PanelBrush)
			.Padding(FMargin(14.0f))
			[
				SNew(SButton)
				.ButtonStyle(&CardButtonStyle)
				.ContentPadding(FMargin(18.0f, 18.0f))
				.OnClicked(this, &SFantasyFrontierCharacterCreatorWidget::HandleSelectClass, CharacterClass)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock)
						.Text(Title)
						.Font(MakeDisplayFont(30))
						.ColorAndOpacity(this, &SFantasyFrontierCharacterCreatorWidget::GetClassButtonTint, CharacterClass)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 10.0f, 0.0f, 0.0f))
					[
						SNew(STextBlock)
						.Text(Body)
						.Font(MakeBodyFont(16))
						.ColorAndOpacity(FLinearColor(0.80f, 0.87f, 0.93f))
						.WrapTextAt(220.0f)
					]
				]
			];
	};

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(FMargin(0.0f, 0.0f, 10.0f, 0.0f))
			[
				MakeClassCard(
					EFantasyFrontierClass::Vanguard,
					LOCTEXT("VanguardClassTitle", "Vanguard"),
					LOCTEXT("VanguardClassBody", "A resilient frontliner who anchors fights and survives dangerous pulls while the rest of the party sets up damage windows."))
			]
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(FMargin(10.0f, 0.0f, 10.0f, 0.0f))
			[
				MakeClassCard(
					EFantasyFrontierClass::Lancer,
					LOCTEXT("LancerClassTitle", "Lancer"),
					LOCTEXT("LancerClassBody", "A disciplined reach fighter built around spacing, burst entries and clear combat silhouettes for the first tutorial slice."))
			]
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(FMargin(10.0f, 0.0f, 0.0f, 0.0f))
			[
				MakeClassCard(
					EFantasyFrontierClass::Spellweaver,
					LOCTEXT("SpellweaverClassTitle", "Spellweaver"),
					LOCTEXT("SpellweaverClassBody", "A ranged magical starter path with lighter defenses, expressive effects and room for later combo-fusion expansion."))
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 18.0f, 0.0f, 0.0f))
		[
			SNew(SBorder)
			.BorderImage(&PanelBrush)
			.Padding(FMargin(18.0f))
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ClassStepNote", "Class selection is the final setup step. It should feel decisive, but not permanently lock the future system into rigid MMO class rails."))
				.Font(MakeBodyFont(16))
				.ColorAndOpacity(FLinearColor(0.78f, 0.86f, 0.93f))
				.WrapTextAt(700.0f)
			]
		];
}

TSharedRef<SWidget> SFantasyFrontierCharacterCreatorWidget::BuildBodyTab()
{
	return SNew(SBorder)
		.BorderImage(&PanelBrush)
		.Padding(FMargin(18.0f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()[BuildStatLine(LOCTEXT("HeightLabel", "Height"), TAttribute<FText>::CreateSP(this, &SFantasyFrontierCharacterCreatorWidget::GetHeightValueText))]
			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 6.0f, 0.0f, 14.0f))
			[
				SNew(SSlider)
				.Value((Draft.HeightScale - 0.92f) / (1.08f - 0.92f))
				.OnValueChanged(this, &SFantasyFrontierCharacterCreatorWidget::HandleHeightChanged)
			]
			+ SVerticalBox::Slot().AutoHeight()[BuildStatLine(LOCTEXT("BuildLabel", "Build"), TAttribute<FText>::CreateSP(this, &SFantasyFrontierCharacterCreatorWidget::GetBuildValueText))]
			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 6.0f, 0.0f, 14.0f))
			[
				SNew(SSlider)
				.Value(Draft.Build)
				.OnValueChanged(this, &SFantasyFrontierCharacterCreatorWidget::HandleBuildChanged)
			]
			+ SVerticalBox::Slot().AutoHeight()[BuildStatLine(LOCTEXT("MusculatureLabel", "Definition"), TAttribute<FText>::CreateSP(this, &SFantasyFrontierCharacterCreatorWidget::GetMusculatureValueText))]
			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 6.0f, 0.0f, 14.0f))
			[
				SNew(SSlider)
				.Value(Draft.Musculature)
				.OnValueChanged(this, &SFantasyFrontierCharacterCreatorWidget::HandleMusculatureChanged)
			]
			+ SVerticalBox::Slot().AutoHeight()[BuildStatLine(LOCTEXT("SkinToneLabel", "Skin Tone"), TAttribute<FText>::CreateSP(this, &SFantasyFrontierCharacterCreatorWidget::GetSkinToneValueText))]
			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 6.0f, 0.0f, 0.0f))
			[
				SNew(SSlider)
				.Value(Draft.SkinTone)
				.OnValueChanged(this, &SFantasyFrontierCharacterCreatorWidget::HandleSkinToneChanged)
			]
		];
}

TSharedRef<SWidget> SFantasyFrontierCharacterCreatorWidget::BuildHairStyleTab()
{
	auto MakeHairCard = [this](int32 HairStyle, const FText& Title, const FText& Body)
	{
		return SNew(SButton)
			.ButtonStyle(&CardButtonStyle)
			.ContentPadding(FMargin(18.0f, 16.0f))
			.OnClicked(this, &SFantasyFrontierCharacterCreatorWidget::HandleSetHairStyle, HairStyle)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(Title)
					.Font(MakeBodyFont(18))
					.ColorAndOpacity(this, &SFantasyFrontierCharacterCreatorWidget::GetHairStyleTint, HairStyle)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 6.0f, 0.0f, 0.0f))
				[
					SNew(STextBlock)
					.Text(Body)
					.Font(MakeBodyFont(14))
					.ColorAndOpacity(FLinearColor(0.77f, 0.85f, 0.92f))
					.WrapTextAt(170.0f)
				]
			];
	};

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(FMargin(0.0f, 0.0f, 8.0f, 0.0f))[MakeHairCard(0, LOCTEXT("HairStyle0", "Wanderer"), LOCTEXT("HairStyle0Body", "Clean layered length with a more adventurous profile."))]
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(FMargin(0.0f, 0.0f, 8.0f, 0.0f))[MakeHairCard(1, LOCTEXT("HairStyle1", "Court"), LOCTEXT("HairStyle1Body", "A fuller premium style better suited to hub or city presentation."))]
			+ SHorizontalBox::Slot().FillWidth(1.0f)[MakeHairCard(2, LOCTEXT("HairStyle2", "Field"), LOCTEXT("HairStyle2Body", "A tighter practical style that reads clearly during movement and combat."))]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 16.0f, 0.0f, 0.0f))
		[
			SNew(SButton)
			.ButtonStyle(&SecondaryButtonStyle)
			.ContentPadding(FMargin(18.0f, 14.0f))
			.OnClicked(this, &SFantasyFrontierCharacterCreatorWidget::HandleCycleHairColor)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("CycleHairColor", "Cycle Hair Color"))
				.Font(MakeBodyFont(17))
				.ColorAndOpacity(FLinearColor(Draft.HairColor.R, Draft.HairColor.G, Draft.HairColor.B, 1.0f))
			]
		];
}

TSharedRef<SWidget> SFantasyFrontierCharacterCreatorWidget::BuildFaceTab()
{
	auto MakeFaceCard = [this](int32 FaceVariant, const FText& Title, const FText& Body)
	{
		return SNew(SButton)
			.ButtonStyle(&CardButtonStyle)
			.ContentPadding(FMargin(18.0f, 16.0f))
			.OnClicked(this, &SFantasyFrontierCharacterCreatorWidget::HandleSetFaceVariant, FaceVariant)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(Title)
					.Font(MakeBodyFont(18))
					.ColorAndOpacity(this, &SFantasyFrontierCharacterCreatorWidget::GetFaceVariantTint, FaceVariant)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 6.0f, 0.0f, 0.0f))
				[
					SNew(STextBlock)
					.Text(Body)
					.Font(MakeBodyFont(14))
					.ColorAndOpacity(FLinearColor(0.77f, 0.85f, 0.92f))
					.WrapTextAt(170.0f)
				]
			];
	};

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(FMargin(0.0f, 0.0f, 8.0f, 0.0f))[MakeFaceCard(0, LOCTEXT("Face0", "Classic"), LOCTEXT("Face0Body", "Balanced heroic proportions for a broad fantasy look."))]
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(FMargin(0.0f, 0.0f, 8.0f, 0.0f))[MakeFaceCard(1, LOCTEXT("Face1", "Refined"), LOCTEXT("Face1Body", "Sharper lines and a cleaner, more elegant read."))]
			+ SHorizontalBox::Slot().FillWidth(1.0f)[MakeFaceCard(2, LOCTEXT("Face2", "Bold"), LOCTEXT("Face2Body", "Stronger facial emphasis for a more expressive anime-leaning silhouette."))]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 16.0f, 0.0f, 0.0f))
		[
			SNew(SButton)
			.ButtonStyle(&SecondaryButtonStyle)
			.ContentPadding(FMargin(18.0f, 14.0f))
			.OnClicked(this, &SFantasyFrontierCharacterCreatorWidget::HandleCycleEyeColor)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("CycleEyeColor", "Cycle Eye Color"))
				.Font(MakeBodyFont(17))
				.ColorAndOpacity(FLinearColor(Draft.EyeColor.R, Draft.EyeColor.G, Draft.EyeColor.B, 1.0f))
			]
		];
}

TSharedRef<SWidget> SFantasyFrontierCharacterCreatorWidget::BuildMarkingsTab()
{
	auto MakeOriginButton = [this](const FText& Label, EFantasyFrontierOriginStyle OriginStyle)
	{
		return SNew(SButton)
			.ButtonStyle(&SecondaryButtonStyle)
			.ContentPadding(FMargin(16.0f, 10.0f))
			.OnClicked(this, &SFantasyFrontierCharacterCreatorWidget::HandleSetOriginStyle, OriginStyle)
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(MakeBodyFont(17))
				.ColorAndOpacity(this, &SFantasyFrontierCharacterCreatorWidget::GetOriginStyleTint, OriginStyle)
			];
	};

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()[BuildStatLine(LOCTEXT("ScarLabel", "Scar Intensity"), TAttribute<FText>::CreateSP(this, &SFantasyFrontierCharacterCreatorWidget::GetScarValueText))]
		+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 6.0f, 0.0f, 14.0f))
		[
			SNew(SSlider)
			.Value(Draft.ScarIntensity)
			.OnValueChanged(this, &SFantasyFrontierCharacterCreatorWidget::HandleScarChanged)
		]
		+ SVerticalBox::Slot().AutoHeight()[BuildStatLine(LOCTEXT("TattooLabel", "Tattoo Intensity"), TAttribute<FText>::CreateSP(this, &SFantasyFrontierCharacterCreatorWidget::GetTattooValueText))]
		+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 6.0f, 0.0f, 16.0f))
		[
			SNew(SSlider)
			.Value(Draft.TattooIntensity)
			.OnValueChanged(this, &SFantasyFrontierCharacterCreatorWidget::HandleTattooChanged)
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("OriginStyleLabel", "Origin Style Preview"))
			.Font(MakeBodyFont(18))
			.ColorAndOpacity(FLinearColor(0.96f, 0.90f, 0.76f))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 12.0f, 0.0f, 0.0f))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(FMargin(0.0f, 0.0f, 8.0f, 0.0f))[MakeOriginButton(LOCTEXT("OriginWanderer", "Frontier Wanderer"), EFantasyFrontierOriginStyle::FrontierWanderer)]
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(FMargin(0.0f, 0.0f, 8.0f, 0.0f))[MakeOriginButton(LOCTEXT("OriginRuneseeker", "Runeseeker"), EFantasyFrontierOriginStyle::Runeseeker)]
			+ SHorizontalBox::Slot().FillWidth(1.0f)[MakeOriginButton(LOCTEXT("OriginVerdant", "Verdant Vanguard"), EFantasyFrontierOriginStyle::VerdantVanguard)]
		];
}

TSharedRef<SWidget> SFantasyFrontierCharacterCreatorWidget::BuildFooter()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBox)
			.Visibility_Lambda([this]()
			{
				return CurrentStep == EFantasyFrontierCreatorStep::Appearance
					|| CurrentStep == EFantasyFrontierCreatorStep::Name
					|| CurrentStep == EFantasyFrontierCreatorStep::Class
					? EVisibility::Visible
					: EVisibility::Collapsed;
			})
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f)
				[
					SNew(SSpacer)
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(FMargin(0.0f, 0.0f, 10.0f, 0.0f))
				[
					SNew(SButton)
					.ButtonStyle(&SecondaryButtonStyle)
					.ContentPadding(FMargin(16.0f, 10.0f))
					.OnClicked(this, &SFantasyFrontierCharacterCreatorWidget::HandleSavePreset)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("SavePreset", "Save Preset"))
						.Font(MakeBodyFont(16))
					]
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SButton)
					.ButtonStyle(&SecondaryButtonStyle)
					.ContentPadding(FMargin(16.0f, 10.0f))
					.OnClicked(this, &SFantasyFrontierCharacterCreatorWidget::HandleLoadPreset)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("LoadPreset", "Load Preset"))
						.Font(MakeBodyFont(16))
					]
				]
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 12.0f, 0.0f, 0.0f))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(FMargin(0.0f, 0.0f, 10.0f, 0.0f))
			[
				SNew(SButton)
				.ButtonStyle(&SecondaryButtonStyle)
				.ContentPadding(FMargin(18.0f, 12.0f))
				.OnClicked(this, &SFantasyFrontierCharacterCreatorWidget::HandlePreviousStep)
				[
					SNew(STextBlock)
					.Text(this, &SFantasyFrontierCharacterCreatorWidget::GetBackButtonText)
					.Font(MakeBodyFont(18))
				]
			]
			+ SHorizontalBox::Slot().FillWidth(1.0f)
			[
				SNew(SSpacer)
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SButton)
					.Visibility_Lambda([this]() { return CurrentStep == EFantasyFrontierCreatorStep::Class ? EVisibility::Collapsed : EVisibility::Visible; })
					.ButtonStyle(&PrimaryButtonStyle)
					.ContentPadding(FMargin(22.0f, 12.0f))
					.IsEnabled(this, &SFantasyFrontierCharacterCreatorWidget::CanAdvanceFromCurrentStep)
					.OnClicked(this, &SFantasyFrontierCharacterCreatorWidget::HandleNextStep)
					[
						SNew(STextBlock)
						.Text(this, &SFantasyFrontierCharacterCreatorWidget::GetNextButtonText)
						.Font(MakeBodyFont(17))
					]
				]
				+ SOverlay::Slot()
				[
					SNew(SButton)
					.Visibility_Lambda([this]() { return CurrentStep == EFantasyFrontierCreatorStep::Class ? EVisibility::Visible : EVisibility::Collapsed; })
					.ButtonStyle(&PrimaryButtonStyle)
					.ContentPadding(FMargin(22.0f, 12.0f))
					.IsEnabled(this, &SFantasyFrontierCharacterCreatorWidget::CanFinish)
					.OnClicked(this, &SFantasyFrontierCharacterCreatorWidget::HandleConfirm)
					[
						SNew(STextBlock)
						.Text(this, &SFantasyFrontierCharacterCreatorWidget::GetConfirmButtonText)
						.Font(MakeBodyFont(17))
					]
				]
			]
		];
}

TSharedRef<SWidget> SFantasyFrontierCharacterCreatorWidget::BuildStatLine(const FText& Label, TAttribute<FText> Value) const
{
	return SNew(SBorder)
		.BorderImage(&PreviewFrameBrush)
		.Padding(FMargin(12.0f, 10.0f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(MakeBodyFont(14))
				.ColorAndOpacity(FLinearColor(0.73f, 0.82f, 0.90f))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 4.0f, 0.0f, 0.0f))
			[
				SNew(STextBlock)
				.Text(Value)
				.Font(MakeBodyFont(18))
				.ColorAndOpacity(FLinearColor(0.98f, 0.94f, 0.84f))
			]
		];
}

#undef LOCTEXT_NAMESPACE
