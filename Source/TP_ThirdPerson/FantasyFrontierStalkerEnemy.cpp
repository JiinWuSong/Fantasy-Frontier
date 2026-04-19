#include "FantasyFrontierStalkerEnemy.h"

#include "GameFramework/CharacterMovementComponent.h"

AFantasyFrontierStalkerEnemy::AFantasyFrontierStalkerEnemy()
{
	EnemyLabel = FText::FromString(TEXT("Ridge Stalker"));
	MaxHealth = 3.0f;
	CurrentHealth = MaxHealth;
	DetectionRange = 1400.0f;
	AttackRange = 150.0f;
	AttackDamage = 0.9f;
	AttackImpulse = 560.0f;
	AttackCooldown = 1.2f;
	TelegraphDuration = 0.35f;
	MoveSpeed = 560.0f;

	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
	SetActorScale3D(FVector(0.92f));
}

FLinearColor AFantasyFrontierStalkerEnemy::GetThreatColor() const
{
	return FLinearColor(0.86f, 0.34f, 0.18f, 1.0f);
}

void AFantasyFrontierStalkerEnemy::PerformAttack()
{
	const FVector ForwardLeap = GetActorForwardVector() * 220.0f + FVector::UpVector * 80.0f;
	LaunchCharacter(ForwardLeap, true, false);
	Super::PerformAttack();
}
