#include "FantasyFrontierRuinMysticEnemy.h"

#include "FantasyFrontierArcBoltProjectile.h"
#include "GameFramework/CharacterMovementComponent.h"

AFantasyFrontierRuinMysticEnemy::AFantasyFrontierRuinMysticEnemy()
{
	EnemyLabel = FText::FromString(TEXT("Ruin Mystic"));
	MaxHealth = 5.0f;
	CurrentHealth = MaxHealth;
	DetectionRange = 1750.0f;
	AttackRange = 820.0f;
	AttackDamage = 1.25f;
	AttackImpulse = 340.0f;
	AttackCooldown = 2.4f;
	TelegraphDuration = 0.95f;
	MoveSpeed = 260.0f;
	ProjectileClass = AFantasyFrontierArcBoltProjectile::StaticClass();

	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
	SetActorScale3D(FVector(1.06f));
}

void AFantasyFrontierRuinMysticEnemy::PerformAttack()
{
	if (!ProjectileClass || !GetWorld() || !GetCurrentTargetActor())
	{
		return;
	}

	const FVector SpawnLocation = GetActorLocation() + FVector(0.0f, 0.0f, 92.0f);
	const FVector ToTarget = (GetCurrentTargetActor()->GetActorLocation() + FVector(0.0f, 0.0f, 45.0f)) - SpawnLocation;
	const FVector Velocity = ToTarget.GetSafeNormal() * 920.0f;

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AFantasyFrontierArcBoltProjectile* Projectile = GetWorld()->SpawnActor<AFantasyFrontierArcBoltProjectile>(
		ProjectileClass,
		SpawnLocation,
		Velocity.Rotation(),
		SpawnParameters))
	{
		Projectile->InitializeProjectile(this, Velocity, AttackDamage);
	}
}

void AFantasyFrontierRuinMysticEnemy::UpdateMovement(float DeltaSeconds, const FVector& TargetDirection, float DistanceToTarget)
{
	if (DistanceToTarget < AttackRange * 0.65f)
	{
		AddMovementInput(-TargetDirection, 0.85f);
		return;
	}

	if (DistanceToTarget > AttackRange * 0.90f && DistanceToTarget <= DetectionRange)
	{
		AddMovementInput(TargetDirection, 0.55f);
	}
}

FLinearColor AFantasyFrontierRuinMysticEnemy::GetThreatColor() const
{
	return FLinearColor(0.20f, 0.76f, 0.98f, 1.0f);
}
