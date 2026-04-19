#pragma once

#include "CoreMinimal.h"
#include "FantasyFrontierEnemyBase.h"
#include "FantasyFrontierStalkerEnemy.generated.h"

UCLASS()
class AFantasyFrontierStalkerEnemy : public AFantasyFrontierEnemyBase
{
	GENERATED_BODY()

public:
	AFantasyFrontierStalkerEnemy();

protected:
	virtual FLinearColor GetThreatColor() const override;
	virtual void PerformAttack() override;
};
