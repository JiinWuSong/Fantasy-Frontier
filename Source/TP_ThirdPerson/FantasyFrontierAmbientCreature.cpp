#include "FantasyFrontierAmbientCreature.h"

#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

namespace
{
	UStaticMesh* GetSphereMesh()
	{
		static UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
		return Mesh;
	}

	UStaticMesh* GetConeMesh()
	{
		static UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cone.Cone"));
		return Mesh;
	}

	UStaticMesh* GetCylinderMesh()
	{
		static UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
		return Mesh;
	}

	UMaterialInterface* GetBasicShapeMaterial()
	{
		static UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
		return Material;
	}
}

AFantasyFrontierAmbientCreature::AFantasyFrontierAmbientCreature()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(SceneRoot);
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AccentMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AccentMesh"));
	AccentMesh->SetupAttachment(BodyMesh);
	AccentMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AccentLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("AccentLight"));
	AccentLight->SetupAttachment(SceneRoot);
	AccentLight->Intensity = 700.0f;
	AccentLight->AttenuationRadius = 260.0f;
	AccentLight->CastShadows = false;

	NamePlate = CreateDefaultSubobject<UTextRenderComponent>(TEXT("NamePlate"));
	NamePlate->SetupAttachment(SceneRoot);
	NamePlate->SetHorizontalAlignment(EHTA_Center);
	NamePlate->SetWorldSize(36.0f);
	NamePlate->SetRelativeLocation(FVector(0.0f, 0.0f, 110.0f));

	if (UMaterialInterface* Material = GetBasicShapeMaterial())
	{
		BodyMesh->SetMaterial(0, Material);
		AccentMesh->SetMaterial(0, Material);
	}
}

void AFantasyFrontierAmbientCreature::BeginPlay()
{
	Super::BeginPlay();
	HomeLocation = GetActorLocation();
	RefreshVisuals();
}

void AFantasyFrontierAmbientCreature::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	RunningTime += DeltaSeconds;

	if (Archetype == EFantasyFrontierWildlifeArchetype::LumenMoth)
	{
		const FVector Drift(
			FMath::Sin(RunningTime * 0.7f) * 80.0f,
			FMath::Cos(RunningTime * 1.1f) * 65.0f,
			72.0f + FMath::Sin(RunningTime * 1.8f) * 30.0f);
		SetActorLocation(HomeLocation + Drift);
		SetActorRotation(FRotator(0.0f, RunningTime * 60.0f, 0.0f));
	}
	else
	{
		const FVector GrazeOffset(
			FMath::Sin(RunningTime * 0.24f) * 110.0f,
			FMath::Sin(RunningTime * 0.17f + 1.2f) * 90.0f,
			FMath::Abs(FMath::Sin(RunningTime * 0.65f)) * 3.0f);
		SetActorLocation(HomeLocation + GrazeOffset);
		SetActorRotation(FRotator(0.0f, RunningTime * 18.0f, 0.0f));
	}
}

void AFantasyFrontierAmbientCreature::ConfigureArchetype(EFantasyFrontierWildlifeArchetype InArchetype)
{
	Archetype = InArchetype;
	RefreshVisuals();
}

void AFantasyFrontierAmbientCreature::RefreshVisuals()
{
	if (Archetype == EFantasyFrontierWildlifeArchetype::LumenMoth)
	{
		BodyMesh->SetStaticMesh(GetSphereMesh());
		BodyMesh->SetRelativeScale3D(FVector(0.18f, 0.12f, 0.08f));
		AccentMesh->SetStaticMesh(GetConeMesh());
		AccentMesh->SetRelativeLocation(FVector(-18.0f, 0.0f, 2.0f));
		AccentMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 90.0f));
		AccentMesh->SetRelativeScale3D(FVector(0.08f, 0.08f, 0.28f));
		NamePlate->SetText(FText::FromString(TEXT("Lumen Moth")));
		AccentLight->SetLightColor(FColor(94, 255, 221));
	}
	else
	{
		BodyMesh->SetStaticMesh(GetCylinderMesh());
		BodyMesh->SetRelativeScale3D(FVector(0.26f, 0.20f, 0.34f));
		AccentMesh->SetStaticMesh(GetConeMesh());
		AccentMesh->SetRelativeLocation(FVector(28.0f, 0.0f, 24.0f));
		AccentMesh->SetRelativeRotation(FRotator(-8.0f, 0.0f, 92.0f));
		AccentMesh->SetRelativeScale3D(FVector(0.08f, 0.08f, 0.16f));
		NamePlate->SetText(FText::FromString(TEXT("Mossback Grazer")));
		AccentLight->SetLightColor(FColor(165, 255, 146));
	}

	const FLinearColor BodyColor = Archetype == EFantasyFrontierWildlifeArchetype::LumenMoth
		? FLinearColor(0.18f, 0.34f, 0.46f, 1.0f)
		: FLinearColor(0.42f, 0.30f, 0.22f, 1.0f);
	const FLinearColor AccentColor = Archetype == EFantasyFrontierWildlifeArchetype::LumenMoth
		? FLinearColor(0.28f, 0.96f, 0.82f, 1.0f)
		: FLinearColor(0.62f, 0.92f, 0.46f, 1.0f);

	if (UMaterialInstanceDynamic* BodyMID = BodyMesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		BodyMID->SetVectorParameterValue(TEXT("Color"), BodyColor);
		BodyMID->SetVectorParameterValue(TEXT("BaseColor"), BodyColor);
	}

	if (UMaterialInstanceDynamic* AccentMID = AccentMesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		AccentMID->SetVectorParameterValue(TEXT("Color"), AccentColor);
		AccentMID->SetVectorParameterValue(TEXT("BaseColor"), AccentColor);
	}
}
