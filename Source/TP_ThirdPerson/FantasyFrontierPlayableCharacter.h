#pragma once

#include "CoreMinimal.h"
#include "FantasyFrontierCharacterCreatorTypes.h"
#include "Variant_Combat/CombatCharacter.h"
#include "FantasyFrontierPlayableCharacter.generated.h"

class UAnimMontage;
class UAnimationAsset;
class UInputAction;
class UInputComponent;
class UPoseableMeshComponent;

UCLASS()
class AFantasyFrontierPlayableCharacter : public ACombatCharacter
{
	GENERATED_BODY()

public:
	AFantasyFrontierPlayableCharacter();

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void DoComboAttackStart() override;
	virtual void DoChargedAttackStart() override;
	virtual void DoAttackTrace(FName DamageSourceBone) override;

	UFUNCTION(BlueprintCallable, Category = "FantasyFrontier|Progression")
	void UnlockSkillFusion();

	UFUNCTION(BlueprintCallable, Category = "FantasyFrontier|Progression")
	void ApplyRiskRewardModifier();

	UFUNCTION(BlueprintPure, Category = "FantasyFrontier|Progression")
	float GetStaminaNormalized() const;

	UFUNCTION(BlueprintPure, Category = "FantasyFrontier|Progression")
	bool IsSkillFusionUnlocked() const { return bSkillFusionUnlocked; }

	UFUNCTION(BlueprintPure, Category = "FantasyFrontier|Progression")
	bool HasRiskRewardModifier() const { return bRiskRewardModifierActive; }

	UFUNCTION(BlueprintPure, Category = "FantasyFrontier|Progression")
	float GetCurrentStamina() const { return CurrentStamina; }

	void TriggerDashForSmokeTest();
	void TriggerLightAttackForSmokeTest();
	void TriggerHeavyAttackForSmokeTest();
	void TriggerMovementPulseForSmokeTest();
	void ApplyPresentationBodyDraft(const FFantasyFrontierCharacterDraft& InDraft);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "FantasyFrontier|Presentation")
	TObjectPtr<UPoseableMeshComponent> PresentationBodyMesh;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> DashAction;

	UPROPERTY(EditAnywhere, Category = "Movement|Dash")
	TObjectPtr<UAnimMontage> DashMontage;

	UPROPERTY(EditAnywhere, Category = "Movement|Dash", meta = (ClampMin = 0, ClampMax = 5000, Units = "cm/s"))
	float DashStrength = 1050.0f;

	UPROPERTY(EditAnywhere, Category = "Movement|Dash", meta = (ClampMin = 0, ClampMax = 2, Units = "s"))
	float DashDuration = 0.18f;

	UPROPERTY(EditAnywhere, Category = "Progression|Stamina", meta = (ClampMin = 1, ClampMax = 500))
	float MaxStamina = 100.0f;

	UPROPERTY(VisibleAnywhere, Category = "Progression|Stamina")
	float CurrentStamina = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Progression|Stamina", meta = (ClampMin = 0, ClampMax = 100))
	float StaminaRegenPerSecond = 16.0f;

	UPROPERTY(EditAnywhere, Category = "Progression|Stamina", meta = (ClampMin = 0, ClampMax = 100))
	float LightAttackStaminaCost = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Progression|Stamina", meta = (ClampMin = 0, ClampMax = 100))
	float HeavyAttackStaminaCost = 26.0f;

	UPROPERTY(EditAnywhere, Category = "Progression|Stamina", meta = (ClampMin = 0, ClampMax = 100))
	float DashStaminaCost = 18.0f;

	UPROPERTY(EditAnywhere, Category = "Progression|Fusion", meta = (ClampMin = 0, ClampMax = 5, Units = "s"))
	float FusionWindowAfterDash = 1.3f;

private:
	void ClearSpawnPresentationIfNeeded();
	void RefreshGameplayPresentationAnimation();
	void HandleLegacyForwardPressed();
	void HandleLegacyForwardReleased();
	void HandleLegacyBackwardPressed();
	void HandleLegacyBackwardReleased();
	void HandleLegacyRightPressed();
	void HandleLegacyRightReleased();
	void HandleLegacyLeftPressed();
	void HandleLegacyLeftReleased();
	void HandleLegacyMouseX(float Value);
	void HandleLegacyMouseY(float Value);
	void HandleDashInput();
	void EndDash();
	bool TryConsumeStamina(float Cost, const TCHAR* FailureReason);
	void ConfigureDefaultCombatAssets();
	void ApplyEmpoweredAttackBuff(float& OutOriginalDamage, float& OutOriginalKnockback, float& OutOriginalLaunch, float& OutOriginalTraceDistance);
	void RestoreAttackBuff(float OriginalDamage, float OriginalKnockback, float OriginalLaunch, float OriginalTraceDistance);

	FTimerHandle DashTimer;
	float LastDashTimestamp = -1000.0f;
	bool bIsDashActive = false;
	bool bSkillFusionUnlocked = false;
	bool bRiskRewardModifierActive = false;
	bool bEmpoweredAttackPending = false;
	bool bUseLegacyInputFallback = false;
	bool bLegacyForwardPressed = false;
	bool bLegacyBackwardPressed = false;
	bool bLegacyRightPressed = false;
	bool bLegacyLeftPressed = false;
	bool bSpawnPresentationActive = false;
	FFantasyFrontierCharacterDraft CurrentPresentationDraft;
	TObjectPtr<UAnimationAsset> ActiveGameplayPresentationAnimation;
};
