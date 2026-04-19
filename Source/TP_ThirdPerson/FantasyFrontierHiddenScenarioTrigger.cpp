#include "FantasyFrontierHiddenScenarioTrigger.h"

#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "FantasyFrontierPlayableCharacter.h"
#include "FantasyFrontierTutorialDirector.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

AFantasyFrontierHiddenScenarioTrigger::AFantasyFrontierHiddenScenarioTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	TriggerVolume->SetupAttachment(SceneRoot);
	TriggerVolume->SetBoxExtent(FVector(90.0f, 90.0f, 140.0f));
	TriggerVolume->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AFantasyFrontierHiddenScenarioTrigger::HandleTriggerOverlap);

	CoreMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoreMesh"));
	CoreMesh->SetupAttachment(SceneRoot);
	CoreMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CoreMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 80.0f));
	CoreMesh->SetRelativeScale3D(FVector(0.35f, 0.35f, 0.55f));
	CoreMesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cone.Cone")));

	GlowLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("GlowLight"));
	GlowLight->SetupAttachment(SceneRoot);
	GlowLight->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f));
	GlowLight->SetLightColor(FColor(92, 255, 216));
	GlowLight->Intensity = 1900.0f;
	GlowLight->AttenuationRadius = 460.0f;
	GlowLight->CastShadows = false;

	TitleText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TitleText"));
	TitleText->SetupAttachment(SceneRoot);
	TitleText->SetRelativeLocation(FVector(0.0f, 0.0f, 180.0f));
	TitleText->SetHorizontalAlignment(EHTA_Center);
	TitleText->SetWorldSize(44.0f);
}

void AFantasyFrontierHiddenScenarioTrigger::BeginPlay()
{
	Super::BeginPlay();

	TitleText->SetText(FText::FromString(TEXT("Sealed Frontier Sigil")));

	if (UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")))
	{
		CoreMesh->SetMaterial(0, BaseMaterial);
		if (UMaterialInstanceDynamic* MID = CoreMesh->CreateAndSetMaterialInstanceDynamic(0))
		{
			const FLinearColor CoreTint(0.20f, 0.88f, 0.72f, 1.0f);
			MID->SetVectorParameterValue(TEXT("Color"), CoreTint);
			MID->SetVectorParameterValue(TEXT("BaseColor"), CoreTint);
		}
	}
}

void AFantasyFrontierHiddenScenarioTrigger::HandleTriggerOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (bScenarioUnlocked)
	{
		return;
	}

	if (AFantasyFrontierPlayableCharacter* PlayerCharacter = Cast<AFantasyFrontierPlayableCharacter>(OtherActor))
	{
		if (AFantasyFrontierTutorialDirector* Director = Cast<AFantasyFrontierTutorialDirector>(GetOwner()))
		{
			bScenarioUnlocked = Director->TryUnlockHiddenScenario(PlayerCharacter);
		}
	}
}
