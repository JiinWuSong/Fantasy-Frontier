#include "FantasyFrontierPlayableCharacter.h"

#include "Animation/AnimationAsset.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Engine/Engine.h"
#include "EnhancedInputComponent.h"
#include "FantasyFrontierCharacterPreviewActor.h"
#include "FantasyFrontierCharacterCreatorTypes.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "InputAction.h"
#include "Misc/PackageName.h"
#include "TP_ThirdPerson.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	TSubclassOf<UAnimInstance> GetPlaceholderGameplayAnimClass()
	{
		static TSubclassOf<UAnimInstance> AnimClass = LoadClass<UAnimInstance>(nullptr, TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed.ABP_Unarmed_C"));
		return AnimClass;
	}

	UAnimationAsset* LoadRetargetedGameplayAnimation(EFantasyFrontierGender Gender, const TCHAR* FemalePath, const TCHAR* MalePath)
	{
		return Gender == EFantasyFrontierGender::Female
			? LoadObject<UAnimationAsset>(nullptr, FemalePath)
			: LoadObject<UAnimationAsset>(nullptr, MalePath);
	}

	UAnimationAsset* GetRetargetedGameplayIdle(EFantasyFrontierGender Gender)
	{
		return LoadRetargetedGameplayAnimation(
			Gender,
			TEXT("/Game/Characters/Mannequins/Anims/Unarmed/MM_Idle.MM_Idle"),
			TEXT("/Game/Characters/Mannequins/Anims/Unarmed/MM_Idle.MM_Idle"));
	}

	UAnimationAsset* GetRetargetedGameplayWalk(EFantasyFrontierGender Gender)
	{
		return LoadRetargetedGameplayAnimation(
			Gender,
			TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Walk/MF_Unarmed_Walk_Fwd.MF_Unarmed_Walk_Fwd"),
			TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Walk/MF_Unarmed_Walk_Fwd.MF_Unarmed_Walk_Fwd"));
	}

	UAnimationAsset* GetRetargetedGameplayJog(EFantasyFrontierGender Gender)
	{
		return LoadRetargetedGameplayAnimation(
			Gender,
			TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Jog/MF_Unarmed_Jog_Fwd.MF_Unarmed_Jog_Fwd"),
			TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Jog/MF_Unarmed_Jog_Fwd.MF_Unarmed_Jog_Fwd"));
	}

}

AFantasyFrontierPlayableCharacter::AFantasyFrontierPlayableCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(40.0f, 96.0f);
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = 510.0f;
	GetCharacterMovement()->JumpZVelocity = 560.0f;
	GetCharacterMovement()->AirControl = 0.40f;
	GetCharacterMovement()->BrakingDecelerationWalking = 1850.0f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->MaxSimulationIterations = 16;

	PresentationBodyMesh = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("PresentationBodyMesh"));
	PresentationBodyMesh->SetupAttachment(GetCapsuleComponent());
	PresentationBodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PresentationBodyMesh->SetCastShadow(true);
	PresentationBodyMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
	PresentationBodyMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	PresentationBodyMesh->SetVisibility(false, true);
	PresentationBodyMesh->SetHiddenInGame(true, true);

	ConfigureDefaultCombatAssets();
}

void AFantasyFrontierPlayableCharacter::BeginPlay()
{
	Super::BeginPlay();
	CurrentStamina = MaxStamina;

	FFantasyFrontierCharacterDraft DefaultDraft;
	DefaultDraft.Gender = EFantasyFrontierGender::Female;
	DefaultDraft.SkinTone = 0.52f;
	ApplyPresentationBodyDraft(DefaultDraft);
}

void AFantasyFrontierPlayableCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bSpawnPresentationActive)
	{
		const bool bHasMovementIntent = !GetLastMovementInputVector().IsNearlyZero() || GetVelocity().SizeSquared2D() > FMath::Square(5.0f) || bIsDashActive;
		if (bHasMovementIntent)
		{
			ClearSpawnPresentationIfNeeded();
		}
	}

	if (bUseLegacyInputFallback)
	{
		const float ForwardInput = (bLegacyForwardPressed ? 1.0f : 0.0f) - (bLegacyBackwardPressed ? 1.0f : 0.0f);
		const float RightInput = (bLegacyRightPressed ? 1.0f : 0.0f) - (bLegacyLeftPressed ? 1.0f : 0.0f);
		if (!FMath::IsNearlyZero(ForwardInput) || !FMath::IsNearlyZero(RightInput))
		{
			DoMove(RightInput, ForwardInput);
		}
	}

	if (!bIsDashActive)
	{
		CurrentStamina = FMath::Clamp(CurrentStamina + (StaminaRegenPerSecond * DeltaSeconds), 0.0f, MaxStamina);
	}

	RefreshGameplayPresentationAnimation();
}

void AFantasyFrontierPlayableCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}

		if (DashAction)
		{
			EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &AFantasyFrontierPlayableCharacter::HandleDashInput);
		}
	}

	bUseLegacyInputFallback = !(JumpAction && MoveAction && LookAction && MouseLookAction && ComboAttackAction && ChargedAttackAction && DashAction);
	if (!bUseLegacyInputFallback)
	{
		UE_LOG(LogTemp, Log, TEXT("FantasyFrontierPlayableCharacter: using Enhanced Input asset bindings for core gameplay."));
		return;
	}

	TArray<FString> MissingInputs;
	if (!JumpAction)
	{
		MissingInputs.Add(TEXT("JumpAction"));
	}
	if (!MoveAction)
	{
		MissingInputs.Add(TEXT("MoveAction"));
	}
	if (!LookAction)
	{
		MissingInputs.Add(TEXT("LookAction"));
	}
	if (!MouseLookAction)
	{
		MissingInputs.Add(TEXT("MouseLookAction"));
	}
	if (!ComboAttackAction)
	{
		MissingInputs.Add(TEXT("ComboAttackAction"));
	}
	if (!ChargedAttackAction)
	{
		MissingInputs.Add(TEXT("ChargedAttackAction"));
	}
	if (!DashAction)
	{
		MissingInputs.Add(TEXT("DashAction"));
	}

	UE_LOG(LogTemp, Warning, TEXT("FantasyFrontierPlayableCharacter: falling back to legacy safety bindings because these input assets were unavailable: %s"), *FString::Join(MissingInputs, TEXT(", ")));

	PlayerInputComponent->BindKey(EKeys::W, IE_Pressed, this, &AFantasyFrontierPlayableCharacter::HandleLegacyForwardPressed);
	PlayerInputComponent->BindKey(EKeys::W, IE_Released, this, &AFantasyFrontierPlayableCharacter::HandleLegacyForwardReleased);
	PlayerInputComponent->BindKey(EKeys::S, IE_Pressed, this, &AFantasyFrontierPlayableCharacter::HandleLegacyBackwardPressed);
	PlayerInputComponent->BindKey(EKeys::S, IE_Released, this, &AFantasyFrontierPlayableCharacter::HandleLegacyBackwardReleased);
	PlayerInputComponent->BindKey(EKeys::D, IE_Pressed, this, &AFantasyFrontierPlayableCharacter::HandleLegacyRightPressed);
	PlayerInputComponent->BindKey(EKeys::D, IE_Released, this, &AFantasyFrontierPlayableCharacter::HandleLegacyRightReleased);
	PlayerInputComponent->BindKey(EKeys::A, IE_Pressed, this, &AFantasyFrontierPlayableCharacter::HandleLegacyLeftPressed);
	PlayerInputComponent->BindKey(EKeys::A, IE_Released, this, &AFantasyFrontierPlayableCharacter::HandleLegacyLeftReleased);
	PlayerInputComponent->BindKey(EKeys::SpaceBar, IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindKey(EKeys::SpaceBar, IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AFantasyFrontierPlayableCharacter::DoComboAttackStart);
	PlayerInputComponent->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &AFantasyFrontierPlayableCharacter::DoChargedAttackStart);
	PlayerInputComponent->BindKey(EKeys::RightMouseButton, IE_Released, this, &AFantasyFrontierPlayableCharacter::DoChargedAttackEnd);
	PlayerInputComponent->BindKey(EKeys::LeftShift, IE_Pressed, this, &AFantasyFrontierPlayableCharacter::HandleDashInput);
	PlayerInputComponent->BindAxisKey(EKeys::MouseX).AxisDelegate.GetDelegateForManualSet().BindUObject(this, &AFantasyFrontierPlayableCharacter::HandleLegacyMouseX);
	PlayerInputComponent->BindAxisKey(EKeys::MouseY).AxisDelegate.GetDelegateForManualSet().BindUObject(this, &AFantasyFrontierPlayableCharacter::HandleLegacyMouseY);
}

void AFantasyFrontierPlayableCharacter::DoComboAttackStart()
{
	ClearSpawnPresentationIfNeeded();

	if (!TryConsumeStamina(LightAttackStaminaCost, TEXT("Nicht genug Ausdauer fuer den leichten Angriff.")))
	{
		return;
	}

	Super::DoComboAttackStart();
}

void AFantasyFrontierPlayableCharacter::DoChargedAttackStart()
{
	ClearSpawnPresentationIfNeeded();

	if (!TryConsumeStamina(HeavyAttackStaminaCost, TEXT("Nicht genug Ausdauer fuer den schweren Angriff.")))
	{
		return;
	}

	if (bSkillFusionUnlocked && (GetWorld()->GetTimeSeconds() - LastDashTimestamp) <= FusionWindowAfterDash)
	{
		bEmpoweredAttackPending = true;
	}

	Super::DoChargedAttackStart();
}

void AFantasyFrontierPlayableCharacter::DoAttackTrace(FName DamageSourceBone)
{
	float OriginalDamage = 0.0f;
	float OriginalKnockback = 0.0f;
	float OriginalLaunch = 0.0f;
	float OriginalTraceDistance = 0.0f;

	ApplyEmpoweredAttackBuff(OriginalDamage, OriginalKnockback, OriginalLaunch, OriginalTraceDistance);
	Super::DoAttackTrace(DamageSourceBone);
	RestoreAttackBuff(OriginalDamage, OriginalKnockback, OriginalLaunch, OriginalTraceDistance);
}

void AFantasyFrontierPlayableCharacter::UnlockSkillFusion()
{
	bSkillFusionUnlocked = true;
}

void AFantasyFrontierPlayableCharacter::ApplyRiskRewardModifier()
{
	if (bRiskRewardModifierActive)
	{
		return;
	}

	bRiskRewardModifierActive = true;
	MaxHP = FMath::Max(1.0f, MaxHP * 0.85f);
	CurrentHP = FMath::Min(CurrentHP, MaxHP);
	DashStaminaCost = FMath::Max(6.0f, DashStaminaCost * 0.72f);
	StaminaRegenPerSecond += 5.0f;
}

float AFantasyFrontierPlayableCharacter::GetStaminaNormalized() const
{
	return MaxStamina > 0.0f ? (CurrentStamina / MaxStamina) : 0.0f;
}

void AFantasyFrontierPlayableCharacter::TriggerDashForSmokeTest()
{
	HandleDashInput();
}

void AFantasyFrontierPlayableCharacter::TriggerLightAttackForSmokeTest()
{
	DoComboAttackStart();
}

void AFantasyFrontierPlayableCharacter::TriggerHeavyAttackForSmokeTest()
{
	DoChargedAttackStart();
}

void AFantasyFrontierPlayableCharacter::TriggerMovementPulseForSmokeTest()
{
	ClearSpawnPresentationIfNeeded();
	DoMove(0.0f, 1.0f);
	AddMovementInput(GetActorForwardVector(), 1.0f, true);

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->AddInputVector(GetActorForwardVector(), true);
		if (MovementComponent->Velocity.Size2D() < 80.0f)
		{
			LaunchCharacter(GetActorForwardVector() * 180.0f, false, false);
		}
	}
}

void AFantasyFrontierPlayableCharacter::ApplyPresentationBodyDraft(const FFantasyFrontierCharacterDraft& InDraft)
{
	if (!GetMesh())
	{
		return;
	}

	CurrentPresentationDraft = InDraft;
	AFantasyFrontierCharacterPreviewActor::ApplyDraftToGameplayMesh(GetMesh(), InDraft);
	ActiveGameplayPresentationAnimation = nullptr;
	GetMesh()->SetVisibility(true, true);
	GetMesh()->SetHiddenInGame(false, true);
	bSpawnPresentationActive = true;
}

void AFantasyFrontierPlayableCharacter::HandleLegacyForwardPressed()
{
	bLegacyForwardPressed = true;
}

void AFantasyFrontierPlayableCharacter::HandleLegacyForwardReleased()
{
	bLegacyForwardPressed = false;
}

void AFantasyFrontierPlayableCharacter::HandleLegacyBackwardPressed()
{
	bLegacyBackwardPressed = true;
}

void AFantasyFrontierPlayableCharacter::HandleLegacyBackwardReleased()
{
	bLegacyBackwardPressed = false;
}

void AFantasyFrontierPlayableCharacter::HandleLegacyRightPressed()
{
	bLegacyRightPressed = true;
}

void AFantasyFrontierPlayableCharacter::HandleLegacyRightReleased()
{
	bLegacyRightPressed = false;
}

void AFantasyFrontierPlayableCharacter::HandleLegacyLeftPressed()
{
	bLegacyLeftPressed = true;
}

void AFantasyFrontierPlayableCharacter::HandleLegacyLeftReleased()
{
	bLegacyLeftPressed = false;
}

void AFantasyFrontierPlayableCharacter::HandleLegacyMouseX(float Value)
{
	if (!FMath::IsNearlyZero(Value))
	{
		DoLook(Value, 0.0f);
	}
}

void AFantasyFrontierPlayableCharacter::HandleLegacyMouseY(float Value)
{
	if (!FMath::IsNearlyZero(Value))
	{
		DoLook(0.0f, Value);
	}
}

void AFantasyFrontierPlayableCharacter::HandleDashInput()
{
	ClearSpawnPresentationIfNeeded();

	if (bIsDashActive || !TryConsumeStamina(DashStaminaCost, TEXT("Nicht genug Ausdauer fuer den Ausweichschritt.")))
	{
		return;
	}

	bIsDashActive = true;
	LastDashTimestamp = GetWorld()->GetTimeSeconds();
	GetCharacterMovement()->BrakingFrictionFactor = 0.0f;

	const FVector DashDirection = GetLastMovementInputVector().IsNearlyZero()
		? GetActorForwardVector()
		: GetLastMovementInputVector().GetSafeNormal();

	LaunchCharacter(DashDirection * DashStrength, true, false);

	if (DashMontage)
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Play(DashMontage, 1.0f);
		}
	}

	GetWorldTimerManager().SetTimer(DashTimer, this, &AFantasyFrontierPlayableCharacter::EndDash, DashDuration, false);
}

void AFantasyFrontierPlayableCharacter::EndDash()
{
	bIsDashActive = false;
	GetCharacterMovement()->BrakingFrictionFactor = 1.0f;
}

bool AFantasyFrontierPlayableCharacter::TryConsumeStamina(float Cost, const TCHAR* FailureReason)
{
	if (CurrentStamina < Cost)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.2f, FColor::Yellow, FailureReason);
		}
		return false;
	}

	CurrentStamina = FMath::Clamp(CurrentStamina - Cost, 0.0f, MaxStamina);
	return true;
}

void AFantasyFrontierPlayableCharacter::ClearSpawnPresentationIfNeeded()
{
	if (!bSpawnPresentationActive || !GetMesh())
	{
		return;
	}

	GetMesh()->ClearRefPoseOverride();
	AFantasyFrontierCharacterPreviewActor::ApplyDraftToGameplayMesh(GetMesh(), CurrentPresentationDraft);
	ActiveGameplayPresentationAnimation = nullptr;
	bSpawnPresentationActive = false;
}

void AFantasyFrontierPlayableCharacter::RefreshGameplayPresentationAnimation()
{
	if (!GetMesh())
	{
		return;
	}

	if (const TSubclassOf<UAnimInstance> AnimClass = GetPlaceholderGameplayAnimClass())
	{
		if (GetMesh()->GetAnimationMode() != EAnimationMode::AnimationBlueprint || GetMesh()->GetAnimClass() != AnimClass)
		{
			GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
			GetMesh()->SetAnimInstanceClass(AnimClass);
		}
		ActiveGameplayPresentationAnimation = nullptr;
		return;
	}

	const float Speed = GetVelocity().Size2D();
	UAnimationAsset* DesiredAnimation = Speed <= 8.0f
		? GetRetargetedGameplayIdle(CurrentPresentationDraft.Gender)
		: (Speed <= 250.0f
			? GetRetargetedGameplayWalk(CurrentPresentationDraft.Gender)
			: GetRetargetedGameplayJog(CurrentPresentationDraft.Gender));
	if (!DesiredAnimation || DesiredAnimation == ActiveGameplayPresentationAnimation)
	{
		return;
	}

	GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	GetMesh()->SetAnimInstanceClass(nullptr);
	GetMesh()->SetAnimation(DesiredAnimation);
	GetMesh()->PlayAnimation(DesiredAnimation, true);
	ActiveGameplayPresentationAnimation = DesiredAnimation;
}

void AFantasyFrontierPlayableCharacter::ConfigureDefaultCombatAssets()
{
	const auto LoadOptionalInputAction = [](const TCHAR* PackagePath, const TCHAR* ObjectPath) -> UInputAction*
	{
		return FPackageName::DoesPackageExist(PackagePath) ? LoadObject<UInputAction>(nullptr, ObjectPath) : nullptr;
	};

	JumpAction = LoadOptionalInputAction(TEXT("/Game/Input/Actions/IA_Jump"), TEXT("/Game/Input/Actions/IA_Jump.IA_Jump"));
	MoveAction = LoadOptionalInputAction(TEXT("/Game/Input/Actions/IA_Move"), TEXT("/Game/Input/Actions/IA_Move.IA_Move"));
	LookAction = LoadOptionalInputAction(TEXT("/Game/Input/Actions/IA_Look"), TEXT("/Game/Input/Actions/IA_Look.IA_Look"));
	MouseLookAction = LoadOptionalInputAction(TEXT("/Game/Input/Actions/IA_MouseLook"), TEXT("/Game/Input/Actions/IA_MouseLook.IA_MouseLook"));
	ComboAttackAction = LoadOptionalInputAction(TEXT("/Game/Variant_Combat/Input/Actions/IA_ComboAttack"), TEXT("/Game/Variant_Combat/Input/Actions/IA_ComboAttack.IA_ComboAttack"));
	ChargedAttackAction = LoadOptionalInputAction(TEXT("/Game/Variant_Combat/Input/Actions/IA_ChargedAttack"), TEXT("/Game/Variant_Combat/Input/Actions/IA_ChargedAttack.IA_ChargedAttack"));
	ToggleCameraAction = LoadOptionalInputAction(TEXT("/Game/Variant_Combat/Input/Actions/IA_ToggleCameraSide"), TEXT("/Game/Variant_Combat/Input/Actions/IA_ToggleCameraSide.IA_ToggleCameraSide"));
	DashAction = LoadOptionalInputAction(TEXT("/Game/Variant_Platforming/Input/Actions/IA_Dash"), TEXT("/Game/Variant_Platforming/Input/Actions/IA_Dash.IA_Dash"));
	ComboAttackMontage = LoadObject<UAnimMontage>(nullptr, TEXT("/Game/Variant_Combat/Anims/AM_ComboAttack.AM_ComboAttack"));
	ChargedAttackMontage = LoadObject<UAnimMontage>(nullptr, TEXT("/Game/Variant_Combat/Anims/AM_ChargedAttack.AM_ChargedAttack"));
	DashMontage = LoadObject<UAnimMontage>(nullptr, TEXT("/Game/Variant_Platforming/Anims/AM_Dash.AM_Dash"));

	ComboSectionNames = { TEXT("Attack_01"), TEXT("Attack_02"), TEXT("Attack_03") };
	ChargeLoopSection = TEXT("Charge_Loop");
	ChargeAttackSection = TEXT("Attack_Release");

	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	if (USkeletalMesh* DefaultBodyMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple.SKM_Quinn_Simple")))
	{
		GetMesh()->SetSkeletalMesh(DefaultBodyMesh);
	}
	if (const TSubclassOf<UAnimInstance> AnimClass = GetPlaceholderGameplayAnimClass())
	{
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		GetMesh()->SetAnimInstanceClass(AnimClass);
	}

	FFantasyFrontierCharacterDraft DefaultDraft;
	DefaultDraft.Gender = EFantasyFrontierGender::Female;
	DefaultDraft.SkinTone = 0.52f;
	AFantasyFrontierCharacterPreviewActor::ApplyDraftToGameplayMesh(GetMesh(), DefaultDraft);
	GetMesh()->SetVisibility(true, true);
	GetMesh()->SetHiddenInGame(false, true);
}

void AFantasyFrontierPlayableCharacter::ApplyEmpoweredAttackBuff(
	float& OutOriginalDamage,
	float& OutOriginalKnockback,
	float& OutOriginalLaunch,
	float& OutOriginalTraceDistance)
{
	if (!bEmpoweredAttackPending)
	{
		return;
	}

	bEmpoweredAttackPending = false;
	OutOriginalDamage = MeleeDamage;
	OutOriginalKnockback = MeleeKnockbackImpulse;
	OutOriginalLaunch = MeleeLaunchImpulse;
	OutOriginalTraceDistance = MeleeTraceDistance;

	const float RiskMultiplier = bRiskRewardModifierActive ? 1.15f : 1.0f;
	MeleeDamage *= 1.65f * RiskMultiplier;
	MeleeKnockbackImpulse *= 1.35f;
	MeleeLaunchImpulse *= 1.20f;
	MeleeTraceDistance += 28.0f;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Cyan, TEXT("Resonance Dash -> Heavy Attack"));
	}
}

void AFantasyFrontierPlayableCharacter::RestoreAttackBuff(
	float OriginalDamage,
	float OriginalKnockback,
	float OriginalLaunch,
	float OriginalTraceDistance)
{
	if (OriginalDamage <= 0.0f)
	{
		return;
	}

	MeleeDamage = OriginalDamage;
	MeleeKnockbackImpulse = OriginalKnockback;
	MeleeLaunchImpulse = OriginalLaunch;
	MeleeTraceDistance = OriginalTraceDistance;
}
