#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FantasyFrontierHiddenScenarioTrigger.generated.h"

class UBoxComponent;
class UPointLightComponent;
class USceneComponent;
class UStaticMeshComponent;
class UTextRenderComponent;

UCLASS()
class AFantasyFrontierHiddenScenarioTrigger : public AActor
{
	GENERATED_BODY()

public:
	AFantasyFrontierHiddenScenarioTrigger();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> TriggerVolume;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> CoreMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPointLightComponent> GlowLight;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UTextRenderComponent> TitleText;

	UFUNCTION()
	void HandleTriggerOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

private:
	bool bScenarioUnlocked = false;
};
