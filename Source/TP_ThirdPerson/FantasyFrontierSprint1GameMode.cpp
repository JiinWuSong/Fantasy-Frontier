#include "FantasyFrontierSprint1GameMode.h"

#include "FantasyFrontierPlayableCharacter.h"
#include "FantasyFrontierSprint1PlayerController.h"
#include "FantasyFrontierTutorialDirector.h"
#include "Kismet/GameplayStatics.h"

AFantasyFrontierSprint1GameMode::AFantasyFrontierSprint1GameMode()
{
	DefaultPawnClass = AFantasyFrontierPlayableCharacter::StaticClass();
	PlayerControllerClass = AFantasyFrontierSprint1PlayerController::StaticClass();
	TutorialDirectorClass = AFantasyFrontierTutorialDirector::StaticClass();
}

void AFantasyFrontierSprint1GameMode::BeginPlay()
{
	Super::BeginPlay();

	if (!GetWorld())
	{
		return;
	}

	SpawnedTutorialDirector = Cast<AFantasyFrontierTutorialDirector>(
		UGameplayStatics::GetActorOfClass(this, AFantasyFrontierTutorialDirector::StaticClass()));

	if (!SpawnedTutorialDirector && TutorialDirectorClass)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnedTutorialDirector = GetWorld()->SpawnActor<AFantasyFrontierTutorialDirector>(
			TutorialDirectorClass,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParameters);
	}
}
