// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FantasyFrontierCharacterCreatorTypes.h"
#include "FantasyFrontierCharacterPreviewActor.generated.h"

class UDirectionalLightComponent;
class UPoseableMeshComponent;
class USceneCaptureComponent2D;
class USceneComponent;
class USkinnedMeshComponent;
class USkeletalMeshComponent;
class UStaticMeshComponent;
class UTextureRenderTarget2D;

UCLASS()
class AFantasyFrontierCharacterPreviewActor : public AActor
{
	GENERATED_BODY()

public:
	AFantasyFrontierCharacterPreviewActor();

	virtual void Tick(float DeltaSeconds) override;

	void SetPreviewRenderTarget(UTextureRenderTarget2D* InRenderTarget);
	void ApplyDraft(const FFantasyFrontierCharacterDraft& InDraft);
	void AddPreviewYaw(float DeltaYaw);
	void AddPreviewZoom(float DeltaZoom);

	static void ApplyDraftToGameplayMesh(USkeletalMeshComponent* MeshComponent, const FFantasyFrontierCharacterDraft& InDraft);
	static void ApplyImportedBodyMaterials(USkinnedMeshComponent* MeshComponent, EFantasyFrontierGender Gender, float SkinTone);

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> PreviewPivot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> PreviewMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPoseableMeshComponent> PreviewPresentationMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneCaptureComponent2D> SceneCapture;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UDirectionalLightComponent> KeyLight;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UDirectionalLightComponent> FillLight;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> StageBackdrop;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> StageHalo;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> StagePedestal;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> StageAccentLeft;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> StageAccentRight;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> HairCap;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> HairBack;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> HairTail;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> EyeLeft;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> EyeRight;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> StarterTorso;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> StarterBelt;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> StarterShoulderLeft;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> StarterShoulderRight;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> StarterBootLeft;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> StarterBootRight;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> UnderwearBottom;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> UnderwearBra;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> TattooBand;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> ScarStripe;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> OriginCrest;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> OriginHornLeft;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> OriginHornRight;

	void ApplyDraftToPreviewMesh(const FFantasyFrontierCharacterDraft& InDraft);
	void RefreshPreviewFraming();
	void ApplyBodyPresentation(const FFantasyFrontierCharacterDraft& InDraft);
	void ApplyHairPresentation(const FFantasyFrontierCharacterDraft& InDraft);
	void ApplyMarkingsPresentation(const FFantasyFrontierCharacterDraft& InDraft);
	void ApplyStarterGearPresentation(const FFantasyFrontierCharacterDraft& InDraft);
	void ApplyOriginPresentation(const FFantasyFrontierCharacterDraft& InDraft);
	void ApplyColorToComponent(UStaticMeshComponent* MeshComponent, const FLinearColor& InColor);
	void RegisterShowOnlyComponent(UPrimitiveComponent* PrimitiveComponent);
	static void ApplySharedBodyScales(USceneComponent* MeshComponent, const FFantasyFrontierCharacterDraft& InDraft);
	static FLinearColor MakeSkinToneColor(float SkinTone);

	float CurrentPreviewYaw = 0.0f;
	float TargetPreviewYaw = 0.0f;
	float CurrentZoom = 0.26f;
	float TargetZoom = 0.26f;
};
