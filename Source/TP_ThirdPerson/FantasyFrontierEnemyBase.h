#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Variant_Combat/Interfaces/CombatDamageable.h"
#include "FantasyFrontierEnemyBase.generated.h"

class UPointLightComponent;
class UPoseableMeshComponent;
class UStaticMeshComponent;
class UTextRenderComponent;

UCLASS(Abstract)
class AFantasyFrontierEnemyBase : public ACharacter, public ICombatDamageable
{
	GENERATED_BODY()

public:
	AFantasyFrontierEnemyBase();

	virtual void Tick(float DeltaSeconds) override;

	virtual void ApplyDamage(float Damage, AActor* DamageCauser, const FVector& DamageLocation, const FVector& DamageImpulse) override;
	virtual void HandleDeath() override;
	virtual void ApplyHealing(float Healing, AActor* Healer) override;
	virtual void NotifyDanger(const FVector& DangerLocation, AActor* DangerSource) override;
	bool HasEngagedTarget() const { return CurrentTarget.IsValid() || TelegraphEndTime > 0.0f || NextAttackTime > 0.0f; }

protected:
	virtual void BeginPlay() override;
	virtual void PerformAttack();
	virtual void UpdateMovement(float DeltaSeconds, const FVector& TargetDirection, float DistanceToTarget);
	virtual FLinearColor GetThreatColor() const;
	AActor* GetCurrentTargetActor() const { return CurrentTarget.Get(); }

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ThreatMarker;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UTextRenderComponent> NamePlate;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UPoseableMeshComponent> PresentationBodyMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UPointLightComponent> ThreatLight;

	UPROPERTY(EditAnywhere, Category = "Enemy")
	FText EnemyLabel;

	UPROPERTY(EditAnywhere, Category = "Enemy")
	float MaxHealth = 4.0f;

	UPROPERTY(VisibleAnywhere, Category = "Enemy")
	float CurrentHealth = 4.0f;

	UPROPERTY(EditAnywhere, Category = "Enemy")
	float DetectionRange = 1200.0f;

	UPROPERTY(EditAnywhere, Category = "Enemy")
	float AttackRange = 170.0f;

	UPROPERTY(EditAnywhere, Category = "Enemy")
	float AttackDamage = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Enemy")
	float AttackImpulse = 420.0f;

	UPROPERTY(EditAnywhere, Category = "Enemy")
	float AttackCooldown = 1.8f;

	UPROPERTY(EditAnywhere, Category = "Enemy")
	float TelegraphDuration = 0.55f;

	UPROPERTY(EditAnywhere, Category = "Enemy")
	float MoveSpeed = 360.0f;

private:
	void AcquireTarget();
	void UpdateTelegraphVisuals(float Alpha);

	TWeakObjectPtr<AActor> CurrentTarget;
	float TelegraphEndTime = -1.0f;
	float NextAttackTime = 0.0f;
	bool bIsDead = false;
};
