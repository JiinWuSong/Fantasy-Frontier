#include "FantasyFrontierFunctionalNpc.h"

#include "Components/CapsuleComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Components/TextRenderComponent.h"
#include "FantasyFrontierPlayableCharacter.h"
#include "FantasyFrontierCharacterPreviewActor.h"
#include "FantasyFrontierTutorialDirector.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

namespace
{
	USkeletalMesh* GetNpcMaleMesh()
	{
		static USkeletalMesh* Mesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
		return Mesh;
	}

	USkeletalMesh* GetNpcFemaleMesh()
	{
		static USkeletalMesh* Mesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple.SKM_Quinn_Simple"));
		return Mesh;
	}

}

AFantasyFrontierFunctionalNpc::AFantasyFrontierFunctionalNpc()
{
	PrimaryActorTick.bCanEverTick = false;
	Tags.Add(TEXT("NPC"));

	GetCapsuleComponent()->InitCapsuleSize(38.0f, 92.0f);
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->bOrientRotationToMovement = false;

	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetVisibility(true, true);
	GetMesh()->SetHiddenInGame(false, true);

	PresentationBodyMesh = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("PresentationBodyMesh"));
	PresentationBodyMesh->SetupAttachment(RootComponent);
	PresentationBodyMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
	PresentationBodyMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	PresentationBodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PresentationBodyMesh->SetCastShadow(true);
	PresentationBodyMesh->SetVisibility(false, true);
	PresentationBodyMesh->SetHiddenInGame(true, true);

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(RootComponent);
	InteractionSphere->InitSphereRadius(220.0f);
	InteractionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AFantasyFrontierFunctionalNpc::HandleInteractionSphereBeginOverlap);

	AccentLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("AccentLight"));
	AccentLight->SetupAttachment(RootComponent);
	AccentLight->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
	AccentLight->Intensity = 900.0f;
	AccentLight->AttenuationRadius = 360.0f;
	AccentLight->CastShadows = false;

	NamePlate = CreateDefaultSubobject<UTextRenderComponent>(TEXT("NamePlate"));
	NamePlate->SetupAttachment(RootComponent);
	NamePlate->SetRelativeLocation(FVector(0.0f, 0.0f, 140.0f));
	NamePlate->SetHorizontalAlignment(EHTA_Center);
	NamePlate->SetWorldSize(40.0f);
}

void AFantasyFrontierFunctionalNpc::BeginPlay()
{
	Super::BeginPlay();
	RefreshVisuals();
}

void AFantasyFrontierFunctionalNpc::ConfigureNpc(EFantasyFrontierNpcRole InRole, const FText& InDisplayName)
{
	NpcRole = InRole;
	DisplayName = InDisplayName;
	RefreshVisuals();
}

void AFantasyFrontierFunctionalNpc::HandleInteractionSphereBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (AFantasyFrontierPlayableCharacter* PlayerCharacter = Cast<AFantasyFrontierPlayableCharacter>(OtherActor))
	{
		if (AFantasyFrontierTutorialDirector* Director = Cast<AFantasyFrontierTutorialDirector>(GetOwner()))
		{
			Director->HandleNpcInteraction(NpcRole, PlayerCharacter, bTriggeredAtLeastOnce);
		}

		bTriggeredAtLeastOnce = true;
	}
}

void AFantasyFrontierFunctionalNpc::RefreshVisuals()
{
	const bool bUseFemaleBody = NpcRole == EFantasyFrontierNpcRole::Guide;
	FFantasyFrontierCharacterDraft Draft;
	Draft.Gender = bUseFemaleBody ? EFantasyFrontierGender::Female : EFantasyFrontierGender::Male;
	Draft.SkinTone = bUseFemaleBody ? 0.54f : 0.58f;
	AFantasyFrontierCharacterPreviewActor::ApplyDraftToGameplayMesh(GetMesh(), Draft);
	GetMesh()->SetVisibility(true, true);
	GetMesh()->SetHiddenInGame(false, true);

	const FLinearColor Accent = NpcRole == EFantasyFrontierNpcRole::Guide
		? FLinearColor(0.24f, 0.78f, 0.96f, 1.0f)
		: (NpcRole == EFantasyFrontierNpcRole::Smith
			? FLinearColor(0.92f, 0.61f, 0.28f, 1.0f)
			: FLinearColor(0.48f, 0.92f, 0.56f, 1.0f));

	AccentLight->SetLightColor(Accent.ToFColor(true));
	NamePlate->SetText(DisplayName.IsEmpty() ? FText::FromString(TEXT("Frontier NPC")) : DisplayName);
}
