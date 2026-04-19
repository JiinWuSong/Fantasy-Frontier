#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FantasyFrontierAmbientCreature.generated.h"

class UPointLightComponent;
class USceneComponent;
class UStaticMeshComponent;
class UTextRenderComponent;

UENUM(BlueprintType)
enum class EFantasyFrontierWildlifeArchetype : uint8
{
	LumenMoth,
	MeadowGrazer
};

UCLASS()
class AFantasyFrontierAmbientCreature : public AActor
{
	GENERATED_BODY()

public:
	AFantasyFrontierAmbientCreature();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void ConfigureArchetype(EFantasyFrontierWildlifeArchetype InArchetype);

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> AccentMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPointLightComponent> AccentLight;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UTextRenderComponent> NamePlate;

	UPROPERTY(EditAnywhere, Category = "FantasyFrontier|Wildlife")
	EFantasyFrontierWildlifeArchetype Archetype = EFantasyFrontierWildlifeArchetype::LumenMoth;

private:
	void RefreshVisuals();

	FVector HomeLocation = FVector::ZeroVector;
	float RunningTime = 0.0f;
};
