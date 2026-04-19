#pragma once

#include "CoreMinimal.h"
#include "FantasyFrontierVerticalSliceTypes.h"
#include "GameFramework/Character.h"
#include "FantasyFrontierFunctionalNpc.generated.h"

class UPointLightComponent;
class UPoseableMeshComponent;
class USphereComponent;
class UTextRenderComponent;

UCLASS()
class AFantasyFrontierFunctionalNpc : public ACharacter
{
	GENERATED_BODY()

public:
	AFantasyFrontierFunctionalNpc();

	virtual void BeginPlay() override;

	void ConfigureNpc(EFantasyFrontierNpcRole InRole, const FText& InDisplayName);
	EFantasyFrontierNpcRole GetNpcRole() const { return NpcRole; }

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> InteractionSphere;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPointLightComponent> AccentLight;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UTextRenderComponent> NamePlate;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPoseableMeshComponent> PresentationBodyMesh;

	UPROPERTY(EditAnywhere, Category = "FantasyFrontier|NPC")
	EFantasyFrontierNpcRole NpcRole = EFantasyFrontierNpcRole::Guide;

	UPROPERTY(EditAnywhere, Category = "FantasyFrontier|NPC")
	FText DisplayName;

	UFUNCTION()
	void HandleInteractionSphereBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

private:
	void RefreshVisuals();
	bool bTriggeredAtLeastOnce = false;
};
