#include "FantasyFrontierEnemyBase.h"

#include "Animation/AnimInstance.h"
#include "FantasyFrontierCharacterPreviewActor.h"
#include "Components/CapsuleComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"

namespace
{
	USkeletalMesh* GetEnemyMesh()
	{
		static USkeletalMesh* Mesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
		return Mesh;
	}

	UStaticMesh* GetThreatMesh()
	{
		static UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
		return Mesh;
	}
}

AFantasyFrontierEnemyBase::AFantasyFrontierEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	Tags.Add(TEXT("Enemy"));
	bUseControllerRotationYaw = false;

	GetCapsuleComponent()->InitCapsuleSize(38.0f, 92.0f);
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bRunPhysicsWithNoController = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
	GetCharacterMovement()->BrakingDecelerationWalking = 1600.0f;

	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	if (USkeletalMesh* EnemyMesh = GetEnemyMesh())
	{
		GetMesh()->SetSkeletalMesh(EnemyMesh);
	}
	GetMesh()->SetVisibility(true, true);
	GetMesh()->SetHiddenInGame(false, true);
	GetMesh()->EmptyOverrideMaterials();
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	PresentationBodyMesh = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("PresentationBodyMesh"));
	PresentationBodyMesh->SetupAttachment(RootComponent);
	PresentationBodyMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
	PresentationBodyMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	PresentationBodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PresentationBodyMesh->SetCastShadow(true);
	PresentationBodyMesh->SetVisibility(false, true);
	PresentationBodyMesh->SetHiddenInGame(true, true);
	if (USkeletalMesh* EnemyPresentationMesh = GetEnemyMesh())
	{
		PresentationBodyMesh->SetSkeletalMesh(EnemyPresentationMesh);
	}
	PresentationBodyMesh->EmptyOverrideMaterials();

	FFantasyFrontierCharacterDraft Draft;
	Draft.Gender = EFantasyFrontierGender::Male;
	Draft.SkinTone = 0.60f;
	AFantasyFrontierCharacterPreviewActor::ApplyDraftToGameplayMesh(GetMesh(), Draft);

	ThreatMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ThreatMarker"));
	ThreatMarker->SetupAttachment(RootComponent);
	ThreatMarker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ThreatMarker->SetCastShadow(false);
	if (UStaticMesh* ThreatMesh = GetThreatMesh())
	{
		ThreatMarker->SetStaticMesh(ThreatMesh);
	}

	NamePlate = CreateDefaultSubobject<UTextRenderComponent>(TEXT("NamePlate"));
	NamePlate->SetupAttachment(RootComponent);
	NamePlate->SetHorizontalAlignment(EHTA_Center);
	NamePlate->SetWorldSize(42.0f);
	NamePlate->SetRelativeLocation(FVector(0.0f, 0.0f, 140.0f));
	NamePlate->SetTextRenderColor(FColor::White);

	ThreatLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ThreatLight"));
	ThreatLight->SetupAttachment(RootComponent);
	ThreatLight->SetRelativeLocation(FVector(0.0f, 0.0f, 82.0f));
	ThreatLight->Intensity = 350.0f;
	ThreatLight->AttenuationRadius = 260.0f;
	ThreatLight->CastShadows = false;
}

void AFantasyFrontierEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
	NamePlate->SetText(EnemyLabel.IsEmpty() ? FText::FromString(TEXT("Frontier Threat")) : EnemyLabel);
	UpdateTelegraphVisuals(0.0f);
}

void AFantasyFrontierEnemyBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsDead)
	{
		return;
	}

	AcquireTarget();
	if (!CurrentTarget.IsValid())
	{
		UpdateTelegraphVisuals(0.0f);
		return;
	}

	const FVector ToTarget = CurrentTarget->GetActorLocation() - GetActorLocation();
	const float DistanceToTarget = ToTarget.Size();
	const float CurrentTime = GetWorld()->GetTimeSeconds();

	if (TelegraphEndTime > 0.0f)
	{
		const float Alpha = FMath::Clamp(1.0f - ((TelegraphEndTime - CurrentTime) / TelegraphDuration), 0.0f, 1.0f);
		UpdateTelegraphVisuals(Alpha);

		if (CurrentTime >= TelegraphEndTime)
		{
			TelegraphEndTime = -1.0f;
			PerformAttack();
			NextAttackTime = CurrentTime + AttackCooldown;
			UpdateTelegraphVisuals(0.0f);
		}
		return;
	}

	UpdateMovement(DeltaSeconds, ToTarget.GetSafeNormal(), DistanceToTarget);

	if (DistanceToTarget <= AttackRange && CurrentTime >= NextAttackTime)
	{
		TelegraphEndTime = CurrentTime + TelegraphDuration;
	}
}

void AFantasyFrontierEnemyBase::ApplyDamage(float Damage, AActor* DamageCauser, const FVector& DamageLocation, const FVector& DamageImpulse)
{
	if (bIsDead)
	{
		return;
	}

	CurrentHealth -= Damage;
	GetCharacterMovement()->AddImpulse(DamageImpulse, true);
	UpdateTelegraphVisuals(1.0f);

	if (CurrentHealth <= 0.0f)
	{
		HandleDeath();
	}
}

void AFantasyFrontierEnemyBase::HandleDeath()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;
	SetActorEnableCollision(false);
	GetCharacterMovement()->DisableMovement();
	GetMesh()->SetSimulatePhysics(true);
	NamePlate->SetText(FText::FromString(TEXT("Neutralized")));
	ThreatLight->SetVisibility(false);
	SetLifeSpan(8.0f);
}

void AFantasyFrontierEnemyBase::ApplyHealing(float Healing, AActor* Healer)
{
	if (bIsDead)
	{
		return;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth + Healing, 0.0f, MaxHealth);
}

void AFantasyFrontierEnemyBase::NotifyDanger(const FVector& DangerLocation, AActor* DangerSource)
{
	if (DangerSource)
	{
		CurrentTarget = DangerSource;
	}
}

void AFantasyFrontierEnemyBase::PerformAttack()
{
	if (!CurrentTarget.IsValid())
	{
		return;
	}

	if (ICombatDamageable* Damageable = Cast<ICombatDamageable>(CurrentTarget.Get()))
	{
		const FVector Direction = (CurrentTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		Damageable->ApplyDamage(AttackDamage, this, CurrentTarget->GetActorLocation(), Direction * AttackImpulse + FVector::UpVector * 120.0f);
	}
}

void AFantasyFrontierEnemyBase::UpdateMovement(float DeltaSeconds, const FVector& TargetDirection, float DistanceToTarget)
{
	if (DistanceToTarget < AttackRange * 0.88f || DistanceToTarget > DetectionRange)
	{
		return;
	}

	AddMovementInput(TargetDirection, 1.0f);
}

FLinearColor AFantasyFrontierEnemyBase::GetThreatColor() const
{
	return FLinearColor(0.92f, 0.28f, 0.20f, 1.0f);
}

void AFantasyFrontierEnemyBase::AcquireTarget()
{
	if (CurrentTarget.IsValid())
	{
		return;
	}

	if (ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0))
	{
		CurrentTarget = PlayerCharacter;
	}
}

void AFantasyFrontierEnemyBase::UpdateTelegraphVisuals(float Alpha)
{
	const FLinearColor BaseColor = GetThreatColor();
	if (UMaterialInstanceDynamic* MarkerMID = ThreatMarker->CreateAndSetMaterialInstanceDynamic(0))
	{
		const FLinearColor MarkerColor = FLinearColor::LerpUsingHSV(BaseColor * 0.45f, BaseColor, Alpha);
		MarkerMID->SetVectorParameterValue(TEXT("Color"), MarkerColor);
		MarkerMID->SetVectorParameterValue(TEXT("BaseColor"), MarkerColor);
		MarkerMID->SetVectorParameterValue(TEXT("Tint"), MarkerColor);
	}

	ThreatMarker->SetRelativeLocation(FVector(0.0f, 0.0f, 118.0f));
	ThreatMarker->SetRelativeScale3D(FVector(FMath::Lerp(0.10f, 0.20f, Alpha)));
	ThreatLight->SetLightColor(GetThreatColor().ToFColor(true));
	ThreatLight->SetIntensity(FMath::Lerp(350.0f, 1800.0f, Alpha));
}
