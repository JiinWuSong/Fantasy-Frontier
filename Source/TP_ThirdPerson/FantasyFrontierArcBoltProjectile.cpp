#include "FantasyFrontierArcBoltProjectile.h"

#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Variant_Combat/Interfaces/CombatDamageable.h"

AFantasyFrontierArcBoltProjectile::AFantasyFrontierArcBoltProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->InitSphereRadius(12.0f);
	CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AFantasyFrontierArcBoltProjectile::HandleProjectileOverlap);

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshRef(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMeshRef.Succeeded())
	{
		VisualMesh->SetStaticMesh(SphereMeshRef.Object);
	}
	VisualMesh->SetRelativeScale3D(FVector(0.24f));

	GlowLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("GlowLight"));
	GlowLight->SetupAttachment(RootComponent);
	GlowLight->Intensity = 1800.0f;
	GlowLight->AttenuationRadius = 180.0f;
	GlowLight->LightColor = FColor(82, 214, 255);
	GlowLight->CastShadows = false;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 900.0f;
	ProjectileMovement->MaxSpeed = 900.0f;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;

	SetLifeSpan(5.0f);
}

void AFantasyFrontierArcBoltProjectile::InitializeProjectile(AActor* InOwnerActor, const FVector& InVelocity, float InDamage)
{
	OwnerActor = InOwnerActor;
	Damage = InDamage;
	ProjectileMovement->Velocity = InVelocity;

	if (UMaterialInstanceDynamic* MID = VisualMesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		const FLinearColor ArcColor(0.20f, 0.84f, 0.98f, 1.0f);
		MID->SetVectorParameterValue(TEXT("Color"), ArcColor);
		MID->SetVectorParameterValue(TEXT("BaseColor"), ArcColor);
		MID->SetVectorParameterValue(TEXT("Tint"), ArcColor);
	}
}

void AFantasyFrontierArcBoltProjectile::HandleProjectileOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this || OtherActor == OwnerActor)
	{
		return;
	}

	if (ICombatDamageable* Damageable = Cast<ICombatDamageable>(OtherActor))
	{
		const FVector Direction = ProjectileMovement->Velocity.GetSafeNormal();
		Damageable->ApplyDamage(Damage, OwnerActor, SweepResult.ImpactPoint, Direction * 260.0f + FVector::UpVector * 80.0f);
	}

	Destroy();
}
