#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FantasyFrontierSprint1GameMode.generated.h"

class AFantasyFrontierTutorialDirector;

UCLASS()
class AFantasyFrontierSprint1GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFantasyFrontierSprint1GameMode();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "FantasyFrontier|Tutorial")
	TSubclassOf<AFantasyFrontierTutorialDirector> TutorialDirectorClass;

private:
	UPROPERTY(Transient)
	TObjectPtr<AFantasyFrontierTutorialDirector> SpawnedTutorialDirector;
};
