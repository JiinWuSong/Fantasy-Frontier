#pragma once

#include "CoreMinimal.h"
#include "FantasyFrontierEnemyBase.h"
#include "FantasyFrontierRuinMysticEnemy.generated.h"

class AFantasyFrontierArcBoltProjectile;

UCLASS()
class AFantasyFrontierRuinMysticEnemy : public AFantasyFrontierEnemyBase
{
	GENERATED_BODY()

public:
	AFantasyFrontierRuinMysticEnemy();

protected:
	virtual void PerformAttack() override;
	virtual void UpdateMovement(float DeltaSeconds, const FVector& TargetDirection, float DistanceToTarget) override;
	virtual FLinearColor GetThreatColor() const override;

	UPROPERTY(EditDefaultsOnly, Category = "FantasyFrontier|Enemy")
	TSubclassOf<AFantasyFrontierArcBoltProjectile> ProjectileClass;
};
