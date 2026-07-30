// Copyright Epic Games, Inc. All Rights Reserved.

#include "FantasyFrontierCharacterPreviewActor.h"

#include "Animation/AnimationAsset.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraTypes.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

namespace
{
	USkeletalMesh* GetPreviewMaleMesh()
	{
		static USkeletalMesh* Mesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
		return Mesh;
	}

	USkeletalMesh* GetPreviewFemaleMesh()
	{
		static USkeletalMesh* Mesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple.SKM_Quinn_Simple"));
		return Mesh;
	}

	UAnimationAsset* GetPreviewMaleIdleAnimation()
	{
		static UAnimationAsset* Animation = LoadObject<UAnimationAsset>(nullptr, TEXT("/Game/Characters/Mannequins/Anims/Unarmed/MM_Idle.MM_Idle"));
		return Animation;
	}

	UAnimationAsset* GetPreviewFemaleIdleAnimation()
	{
		static UAnimationAsset* Animation = LoadObject<UAnimationAsset>(nullptr, TEXT("/Game/Characters/Mannequins/Anims/Unarmed/MM_Idle.MM_Idle"));
		return Animation;
	}

	UAnimationAsset* GetImportedIdleAnimation(EFantasyFrontierGender Gender)
	{
		return Gender == EFantasyFrontierGender::Female ? GetPreviewFemaleIdleAnimation() : GetPreviewMaleIdleAnimation();
	}

	USkeletalMesh* GetGameplayMannyMesh()
	{
		return GetPreviewMaleMesh();
	}

	USkeletalMesh* GetGameplayQuinnMesh()
	{
		return GetPreviewFemaleMesh();
	}

	UMaterialInterface* GetGameplayMannyMaterial(int32 StylePreset)
	{
		const TCHAR* Path = StylePreset == 0
			? TEXT("/Game/Characters/Mannequins/Materials/Manny/MI_Manny_01_New.MI_Manny_01_New")
			: TEXT("/Game/Characters/Mannequins/Materials/Manny/MI_Manny_02_New.MI_Manny_02_New");
		return LoadObject<UMaterialInterface>(nullptr, Path);
	}

	UMaterialInterface* GetGameplayQuinnMaterial(int32 StylePreset)
	{
		const TCHAR* Path = StylePreset == 0
			? TEXT("/Game/Characters/Mannequins/Materials/Quinn/MI_Quinn_01.MI_Quinn_01")
			: TEXT("/Game/Characters/Mannequins/Materials/Quinn/MI_Quinn_02.MI_Quinn_02");
		return LoadObject<UMaterialInterface>(nullptr, Path);
	}

	TSubclassOf<UAnimInstance> GetPreviewAnimClass()
	{
		static TSubclassOf<UAnimInstance> AnimClass = LoadClass<UAnimInstance>(nullptr, TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed.ABP_Unarmed_C"));
		return AnimClass;
	}

	UStaticMesh* GetPreviewStageCubeMesh()
	{
		static UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
		return Mesh;
	}

	UStaticMesh* GetPreviewStageSphereMesh()
	{
		static UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
		return Mesh;
	}

	UStaticMesh* GetPreviewStageCylinderMesh()
	{
		static UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
		return Mesh;
	}

	UStaticMesh* GetPreviewStageConeMesh()
	{
		static UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cone.Cone"));
		return Mesh;
	}

	UMaterialInterface* GetPreviewStageBasicShapeMaterial()
	{
		static UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
		return Material;
	}

	void RestoreImportedMeshMaterials(USkinnedMeshComponent* MeshComponent)
	{
		if (!MeshComponent)
		{
			return;
		}

		USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(MeshComponent->GetSkinnedAsset());
		if (!SkeletalMesh)
		{
			return;
		}

		MeshComponent->EmptyOverrideMaterials();

		const TArray<FSkeletalMaterial>& SkeletalMaterials = SkeletalMesh->GetMaterials();
		for (int32 MaterialIndex = 0; MaterialIndex < SkeletalMaterials.Num(); ++MaterialIndex)
		{
			if (UMaterialInterface* MaterialInterface = SkeletalMaterials[MaterialIndex].MaterialInterface)
			{
				MeshComponent->SetMaterial(MaterialIndex, MaterialInterface);
			}
		}
	}

	void ApplyImportedIdleAnimation(USkeletalMeshComponent* MeshComponent, EFantasyFrontierGender Gender)
	{
		if (!MeshComponent)
		{
			return;
		}

		if (const TSubclassOf<UAnimInstance> AnimClass = GetPreviewAnimClass())
		{
			if (MeshComponent->GetAnimationMode() != EAnimationMode::AnimationBlueprint || MeshComponent->GetAnimClass() != AnimClass)
			{
				MeshComponent->SetAnimationMode(EAnimationMode::AnimationBlueprint);
				MeshComponent->SetAnimInstanceClass(AnimClass);
			}
			return;
		}

		MeshComponent->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		MeshComponent->SetAnimInstanceClass(nullptr);

		if (UAnimationAsset* IdleAnimation = GetImportedIdleAnimation(Gender))
		{
			MeshComponent->SetAnimation(IdleAnimation);
			MeshComponent->PlayAnimation(IdleAnimation, true);
			return;
		}

		MeshComponent->SetAnimation(nullptr);
		MeshComponent->Stop();
	}
}

AFantasyFrontierCharacterPreviewActor::AFantasyFrontierCharacterPreviewActor()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	PreviewPivot = CreateDefaultSubobject<USceneComponent>(TEXT("PreviewPivot"));
	PreviewPivot->SetupAttachment(SceneRoot);

	PreviewMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PreviewMesh"));
	PreviewMesh->SetupAttachment(PreviewPivot);
	PreviewMesh->SetCastShadow(true);
	PreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewMesh->bCastInsetShadow = false;
	PreviewMesh->SetVisibility(true, true);
	PreviewMesh->SetHiddenInGame(false, true);
	if (USkeletalMesh* DefaultPreviewMesh = GetPreviewFemaleMesh())
	{
		PreviewMesh->SetSkeletalMesh(DefaultPreviewMesh);
	}
	if (const TSubclassOf<UAnimInstance> AnimClass = GetPreviewAnimClass())
	{
		PreviewMesh->SetAnimInstanceClass(AnimClass);
		PreviewMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	}
	else
	{
		PreviewMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		PreviewMesh->Stop();
	}
	PreviewMesh->EmptyOverrideMaterials();

	PreviewPresentationMesh = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("PreviewPresentationMesh"));
	PreviewPresentationMesh->SetupAttachment(PreviewPivot);
	PreviewPresentationMesh->SetCastShadow(true);
	PreviewPresentationMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewPresentationMesh->bCastInsetShadow = false;
	PreviewPresentationMesh->SetVisibility(false, true);
	PreviewPresentationMesh->SetHiddenInGame(true, true);
	if (USkeletalMesh* DefaultPreviewPresentationMesh = GetPreviewFemaleMesh())
	{
		PreviewPresentationMesh->SetSkeletalMesh(DefaultPreviewPresentationMesh);
	}
	PreviewPresentationMesh->EmptyOverrideMaterials();

	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
	SceneCapture->SetupAttachment(SceneRoot);
	SceneCapture->ProjectionType = ECameraProjectionMode::Perspective;
	SceneCapture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	SceneCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	SceneCapture->bCaptureEveryFrame = true;
	SceneCapture->bCaptureOnMovement = true;
	SceneCapture->FOVAngle = 24.0f;

	KeyLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("KeyLight"));
	KeyLight->SetupAttachment(SceneRoot);
	KeyLight->SetRelativeRotation(FRotator(-28.0f, 214.0f, 0.0f));
	KeyLight->Intensity = 4.6f;
	KeyLight->LightColor = FColor(255, 244, 224);
	KeyLight->SetCastShadows(false);

	FillLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("FillLight"));
	FillLight->SetupAttachment(SceneRoot);
	FillLight->SetRelativeRotation(FRotator(-12.0f, 32.0f, 0.0f));
	FillLight->Intensity = 3.0f;
	FillLight->LightColor = FColor(196, 226, 255);
	FillLight->SetCastShadows(false);

	auto ConfigureAdornment = [](UStaticMeshComponent* Component, USkeletalMeshComponent* Parent, UStaticMesh* Mesh, const FName SocketName)
	{
		if (!Component)
		{
			return;
		}

		Component->SetupAttachment(Parent, SocketName);
		Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Component->SetCastShadow(true);
		Component->SetStaticMesh(Mesh);
		if (UMaterialInterface* Material = GetPreviewStageBasicShapeMaterial())
		{
			Component->SetMaterial(0, Material);
		}
	};

	auto ConfigureStageMesh = [this](UStaticMeshComponent* Component, UStaticMesh* Mesh, const FVector& Scale, const FVector& Location, const FRotator& Rotation, const FLinearColor& Color)
	{
		if (!Component)
		{
			return;
		}

		Component->SetupAttachment(SceneRoot);
		Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Component->SetCastShadow(false);
		Component->SetStaticMesh(Mesh);
		Component->SetRelativeLocation(Location);
		Component->SetRelativeRotation(Rotation);
		Component->SetRelativeScale3D(Scale);
		if (UMaterialInterface* Material = GetPreviewStageBasicShapeMaterial())
		{
			Component->SetMaterial(0, Material);
		}
		ApplyColorToComponent(Component, Color);
	};

	StageBackdrop = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StageBackdrop"));
	StageHalo = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StageHalo"));
	StagePedestal = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StagePedestal"));
	StageAccentLeft = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StageAccentLeft"));
	StageAccentRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StageAccentRight"));

	ConfigureStageMesh(StageBackdrop, GetPreviewStageCubeMesh(), FVector(0.06f, 5.4f, 6.4f), FVector(-248.0f, 0.0f, 176.0f), FRotator::ZeroRotator, FLinearColor(0.16f, 0.25f, 0.33f, 1.0f));
	ConfigureStageMesh(StageHalo, GetPreviewStageSphereMesh(), FVector(1.8f, 0.04f, 1.8f), FVector(-232.0f, 0.0f, 220.0f), FRotator::ZeroRotator, FLinearColor(0.54f, 0.68f, 0.80f, 1.0f));
	ConfigureStageMesh(StagePedestal, GetPreviewStageCylinderMesh(), FVector(1.65f, 1.65f, 0.06f), FVector(0.0f, 0.0f, -112.0f), FRotator::ZeroRotator, FLinearColor(0.15f, 0.20f, 0.24f, 1.0f));
	ConfigureStageMesh(StageAccentLeft, GetPreviewStageCylinderMesh(), FVector(0.01f, 0.01f, 0.01f), FVector(-300.0f, -320.0f, -300.0f), FRotator::ZeroRotator, FLinearColor::Black);
	ConfigureStageMesh(StageAccentRight, GetPreviewStageCylinderMesh(), FVector(0.01f, 0.01f, 0.01f), FVector(-300.0f, 320.0f, -300.0f), FRotator::ZeroRotator, FLinearColor::Black);

	HairCap = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HairCap"));
	HairBack = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HairBack"));
	HairTail = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HairTail"));
	EyeLeft = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EyeLeft"));
	EyeRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EyeRight"));
	StarterTorso = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StarterTorso"));
	StarterBelt = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StarterBelt"));
	StarterShoulderLeft = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StarterShoulderLeft"));
	StarterShoulderRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StarterShoulderRight"));
	StarterBootLeft = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StarterBootLeft"));
	StarterBootRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StarterBootRight"));
	UnderwearBottom = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("UnderwearBottom"));
	UnderwearBra = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("UnderwearBra"));
	TattooBand = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TattooBand"));
	ScarStripe = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ScarStripe"));
	OriginCrest = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OriginCrest"));
	OriginHornLeft = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OriginHornLeft"));
	OriginHornRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OriginHornRight"));

	ConfigureAdornment(HairCap, PreviewMesh, GetPreviewStageSphereMesh(), TEXT("head"));
	ConfigureAdornment(HairBack, PreviewMesh, GetPreviewStageSphereMesh(), TEXT("head"));
	ConfigureAdornment(HairTail, PreviewMesh, GetPreviewStageCylinderMesh(), TEXT("head"));
	ConfigureAdornment(EyeLeft, PreviewMesh, GetPreviewStageSphereMesh(), TEXT("head"));
	ConfigureAdornment(EyeRight, PreviewMesh, GetPreviewStageSphereMesh(), TEXT("head"));
	ConfigureAdornment(StarterTorso, PreviewMesh, GetPreviewStageCubeMesh(), TEXT("spine_03"));
	ConfigureAdornment(StarterBelt, PreviewMesh, GetPreviewStageCylinderMesh(), TEXT("pelvis"));
	ConfigureAdornment(StarterShoulderLeft, PreviewMesh, GetPreviewStageCubeMesh(), TEXT("upperarm_l"));
	ConfigureAdornment(StarterShoulderRight, PreviewMesh, GetPreviewStageCubeMesh(), TEXT("upperarm_r"));
	ConfigureAdornment(StarterBootLeft, PreviewMesh, GetPreviewStageCubeMesh(), TEXT("calf_l"));
	ConfigureAdornment(StarterBootRight, PreviewMesh, GetPreviewStageCubeMesh(), TEXT("calf_r"));
	ConfigureAdornment(UnderwearBottom, PreviewMesh, GetPreviewStageCylinderMesh(), TEXT("pelvis"));
	ConfigureAdornment(UnderwearBra, PreviewMesh, GetPreviewStageCubeMesh(), TEXT("spine_03"));
	ConfigureAdornment(TattooBand, PreviewMesh, GetPreviewStageCylinderMesh(), TEXT("upperarm_l"));
	ConfigureAdornment(ScarStripe, PreviewMesh, GetPreviewStageCubeMesh(), TEXT("head"));
	ConfigureAdornment(OriginCrest, PreviewMesh, GetPreviewStageSphereMesh(), TEXT("spine_03"));
	ConfigureAdornment(OriginHornLeft, PreviewMesh, GetPreviewStageConeMesh(), TEXT("head"));
	ConfigureAdornment(OriginHornRight, PreviewMesh, GetPreviewStageConeMesh(), TEXT("head"));
}

void AFantasyFrontierCharacterPreviewActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CurrentPreviewYaw = FMath::FInterpTo(CurrentPreviewYaw, TargetPreviewYaw, DeltaSeconds, 10.0f);
	CurrentZoom = FMath::FInterpTo(CurrentZoom, TargetZoom, DeltaSeconds, 10.0f);
	PreviewPivot->SetRelativeRotation(FRotator(0.0f, CurrentPreviewYaw, 0.0f));
	RefreshPreviewFraming();
}

void AFantasyFrontierCharacterPreviewActor::SetPreviewRenderTarget(UTextureRenderTarget2D* InRenderTarget)
{
	SceneCapture->TextureTarget = InRenderTarget;
	SceneCapture->ShowOnlyComponents.Empty();
	RegisterShowOnlyComponent(PreviewMesh);
	RegisterShowOnlyComponent(TattooBand);
	RegisterShowOnlyComponent(ScarStripe);
	RegisterShowOnlyComponent(OriginCrest);
	RegisterShowOnlyComponent(OriginHornLeft);
	RegisterShowOnlyComponent(OriginHornRight);
}

void AFantasyFrontierCharacterPreviewActor::ApplyDraft(const FFantasyFrontierCharacterDraft& InDraft)
{
	ApplyDraftToPreviewMesh(InDraft);
}

void AFantasyFrontierCharacterPreviewActor::AddPreviewYaw(float DeltaYaw)
{
	TargetPreviewYaw = FMath::Clamp(TargetPreviewYaw + DeltaYaw, -90.0f, 90.0f);
}

void AFantasyFrontierCharacterPreviewActor::AddPreviewZoom(float DeltaZoom)
{
	TargetZoom = FMath::Clamp(TargetZoom + DeltaZoom, -0.35f, 0.35f);
}

void AFantasyFrontierCharacterPreviewActor::ApplyDraftToGameplayMesh(USkeletalMeshComponent* MeshComponent, const FFantasyFrontierCharacterDraft& InDraft)
{
	if (!MeshComponent)
	{
		return;
	}

	const bool bUseFemaleFrame = InDraft.Gender == EFantasyFrontierGender::Female;
	if (USkeletalMesh* MeshAsset = bUseFemaleFrame ? GetGameplayQuinnMesh() : GetGameplayMannyMesh())
	{
		MeshComponent->SetSkeletalMesh(MeshAsset);
	}
	MeshComponent->EmptyOverrideMaterials();
	MeshComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	ApplyImportedBodyMaterials(MeshComponent, InDraft.Gender, InDraft.SkinTone);
	ApplySharedBodyScales(MeshComponent, InDraft);
	MeshComponent->ClearRefPoseOverride();
	ApplyImportedIdleAnimation(MeshComponent, InDraft.Gender);
}

void AFantasyFrontierCharacterPreviewActor::ApplyDraftToPreviewMesh(const FFantasyFrontierCharacterDraft& InDraft)
{
	if (!PreviewMesh)
	{
		return;
	}

	const bool bUseFemaleFrame = InDraft.Gender == EFantasyFrontierGender::Female;
	if (USkeletalMesh* PreviewBodyMesh = bUseFemaleFrame ? GetPreviewFemaleMesh() : GetPreviewMaleMesh())
	{
		PreviewMesh->SetSkeletalMesh(PreviewBodyMesh);
		if (PreviewPresentationMesh)
		{
			PreviewPresentationMesh->SetSkeletalMesh(PreviewBodyMesh);
		}
	}
	PreviewMesh->EmptyOverrideMaterials();
	ApplyImportedBodyMaterials(PreviewMesh, InDraft.Gender, InDraft.SkinTone);
	ApplySharedBodyScales(PreviewMesh, InDraft);
	PreviewMesh->ClearRefPoseOverride();
	ApplyImportedIdleAnimation(PreviewMesh, InDraft.Gender);
	ApplyBodyPresentation(InDraft);
	ApplyHairPresentation(InDraft);
	ApplyMarkingsPresentation(InDraft);
	ApplyStarterGearPresentation(InDraft);
	ApplyOriginPresentation(InDraft);
	RefreshPreviewFraming();
}

void AFantasyFrontierCharacterPreviewActor::RefreshPreviewFraming()
{
	USceneComponent* FramingComponent = PreviewMesh ? static_cast<USceneComponent*>(PreviewMesh) : static_cast<USceneComponent*>(PreviewPresentationMesh);
	if (!FramingComponent || !SceneCapture)
	{
		return;
	}

	const FBoxSphereBounds LocalBounds = FramingComponent->CalcBounds(FTransform::Identity);
	const float FloorOffset = -(LocalBounds.Origin.Z - LocalBounds.BoxExtent.Z);
	FramingComponent->SetRelativeLocation(FVector(0.0f, 0.0f, FloorOffset));

	const float HalfHeight = FMath::Max(LocalBounds.BoxExtent.Z * 1.02f, 104.0f);
	const float CameraDistance = (HalfHeight / FMath::Tan(FMath::DegreesToRadians(SceneCapture->FOVAngle * 0.5f))) + 56.0f;
	SceneCapture->SetRelativeLocation(FVector(CameraDistance - CurrentZoom * 220.0f, 0.0f, HalfHeight * 0.74f));
	SceneCapture->SetRelativeRotation(FRotator(-4.0f - CurrentZoom * 1.2f, 180.0f, 0.0f));
}

void AFantasyFrontierCharacterPreviewActor::ApplyBodyPresentation(const FFantasyFrontierCharacterDraft& InDraft)
{
	PreviewMesh->SetRelativeRotation(FRotator(0.0f, -96.0f, 0.0f));
	EyeLeft->SetVisibility(false);
	EyeRight->SetVisibility(false);
	UnderwearBottom->SetVisibility(false);
	UnderwearBra->SetVisibility(false);

	ApplyColorToComponent(StageBackdrop, FLinearColor(0.15f, 0.24f, 0.33f, 1.0f));
	ApplyColorToComponent(StageHalo, FLinearColor(0.55f, 0.69f, 0.80f, 1.0f));
	ApplyColorToComponent(StagePedestal, FLinearColor(0.15f, 0.20f, 0.24f, 1.0f));
}

void AFantasyFrontierCharacterPreviewActor::ApplyHairPresentation(const FFantasyFrontierCharacterDraft& InDraft)
{
	HairCap->SetVisibility(false);
	HairBack->SetVisibility(false);
	HairTail->SetVisibility(false);
}

void AFantasyFrontierCharacterPreviewActor::ApplyMarkingsPresentation(const FFantasyFrontierCharacterDraft& InDraft)
{
	TattooBand->SetVisibility(false);
	ScarStripe->SetVisibility(false);
}

void AFantasyFrontierCharacterPreviewActor::ApplyStarterGearPresentation(const FFantasyFrontierCharacterDraft& InDraft)
{
	StarterTorso->SetVisibility(false);
	StarterBelt->SetVisibility(false);
	StarterShoulderLeft->SetVisibility(false);
	StarterShoulderRight->SetVisibility(false);
	StarterBootLeft->SetVisibility(false);
	StarterBootRight->SetVisibility(false);
}

void AFantasyFrontierCharacterPreviewActor::ApplyOriginPresentation(const FFantasyFrontierCharacterDraft& InDraft)
{
	const bool bShowOriginStyle = false;
	OriginCrest->SetVisibility(false);
	OriginHornLeft->SetVisibility(false);
	OriginHornRight->SetVisibility(false);

	if (!bShowOriginStyle)
	{
		return;
	}

	switch (InDraft.OriginStyle)
	{
	case EFantasyFrontierOriginStyle::Runeseeker:
		OriginCrest->SetRelativeLocation(FVector(-8.0f, 0.0f, 8.0f));
		OriginCrest->SetRelativeScale3D(FVector(0.22f, 0.08f, 0.22f));
		ApplyColorToComponent(OriginCrest, FLinearColor(0.38f, 0.72f, 0.88f, 1.0f));
		break;
	case EFantasyFrontierOriginStyle::VerdantVanguard:
		OriginCrest->SetRelativeLocation(FVector(-8.0f, 0.0f, 10.0f));
		OriginCrest->SetRelativeScale3D(FVector(0.26f, 0.08f, 0.20f));
		ApplyColorToComponent(OriginCrest, FLinearColor(0.43f, 0.74f, 0.49f, 1.0f));
		break;
	case EFantasyFrontierOriginStyle::FrontierWanderer:
	default:
		OriginCrest->SetRelativeLocation(FVector(-8.0f, 0.0f, 10.0f));
		OriginCrest->SetRelativeScale3D(FVector(0.26f, 0.08f, 0.18f));
		ApplyColorToComponent(OriginCrest, FLinearColor(0.86f, 0.70f, 0.44f, 1.0f));
		break;
	}
}

void AFantasyFrontierCharacterPreviewActor::ApplyColorToComponent(UStaticMeshComponent* MeshComponent, const FLinearColor& InColor)
{
	if (!MeshComponent)
	{
		return;
	}

	UMaterialInstanceDynamic* DynamicMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(0);
	if (!DynamicMaterial)
	{
		return;
	}

	DynamicMaterial->SetVectorParameterValue(TEXT("BodyColor"), InColor);
	DynamicMaterial->SetVectorParameterValue(TEXT("Color"), InColor);
	DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), InColor);
	DynamicMaterial->SetVectorParameterValue(TEXT("ShapeColor"), InColor);
	DynamicMaterial->SetVectorParameterValue(TEXT("Tint"), InColor);
	DynamicMaterial->SetVectorParameterValue(TEXT("Paint Tint"), InColor);
	DynamicMaterial->SetVectorParameterValue(TEXT("LogoTint"), InColor);
}

void AFantasyFrontierCharacterPreviewActor::ApplyImportedBodyMaterials(USkinnedMeshComponent* MeshComponent, EFantasyFrontierGender Gender, float SkinTone)
{
	if (!MeshComponent)
	{
		return;
	}

	RestoreImportedMeshMaterials(MeshComponent);
}

void AFantasyFrontierCharacterPreviewActor::RegisterShowOnlyComponent(UPrimitiveComponent* PrimitiveComponent)
{
	if (PrimitiveComponent)
	{
		SceneCapture->ShowOnlyComponent(PrimitiveComponent);
	}
}

void AFantasyFrontierCharacterPreviewActor::ApplySharedBodyScales(USceneComponent* MeshComponent, const FFantasyFrontierCharacterDraft& InDraft)
{
	if (!MeshComponent)
	{
		return;
	}

	const float HeightScale = FMath::Clamp(InDraft.HeightScale, 0.90f, 1.12f);
	const float BuildScale = FMath::Lerp(0.93f, 1.09f, FMath::Clamp(InDraft.Build, 0.0f, 1.0f));
	const float MuscleBias = FMath::Lerp(0.98f, 1.08f, FMath::Clamp(InDraft.Musculature, 0.0f, 1.0f));
	const float ShoulderBias = FMath::Lerp(0.98f, 1.06f, FMath::Clamp(InDraft.Build + InDraft.Musculature * 0.5f, 0.0f, 1.0f));
	MeshComponent->SetRelativeScale3D(FVector(BuildScale * ShoulderBias, BuildScale, HeightScale));
}

FLinearColor AFantasyFrontierCharacterPreviewActor::MakeSkinToneColor(float SkinTone)
{
	const float ToneAlpha = FMath::Clamp(SkinTone, 0.0f, 1.0f);
	const FLinearColor LightTone(0.94f, 0.82f, 0.74f, 1.0f);
	const FLinearColor DeepTone(0.34f, 0.22f, 0.16f, 1.0f);
	return FLinearColor(
		FMath::Lerp(LightTone.R, DeepTone.R, ToneAlpha),
		FMath::Lerp(LightTone.G, DeepTone.G, ToneAlpha),
		FMath::Lerp(LightTone.B, DeepTone.B, ToneAlpha),
		1.0f);
}
