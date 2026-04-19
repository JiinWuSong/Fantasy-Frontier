#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FantasyFrontierArcBoltProjectile.generated.h"

class UPointLightComponent;
class UProjectileMovementComponent;
class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class AFantasyFrontierArcBoltProjectile : public AActor
{
	GENERATED_BODY()

public:
	AFantasyFrontierArcBoltProjectile();

	void InitializeProjectile(AActor* InOwnerActor, const FVector& InVelocity, float InDamage);

protected:
	UFUNCTION()
	void HandleProjectileOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPointLightComponent> GlowLight;

private:
	UPROPERTY()
	TObjectPtr<AActor> OwnerActor;

	float Damage = 1.2f;
};
