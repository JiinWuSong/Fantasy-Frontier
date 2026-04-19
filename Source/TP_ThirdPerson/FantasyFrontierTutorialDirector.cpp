#include "FantasyFrontierTutorialDirector.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "FantasyFrontierAmbientCreature.h"
#include "FantasyFrontierFunctionalNpc.h"
#include "FantasyFrontierHiddenScenarioTrigger.h"
#include "FantasyFrontierPlayableCharacter.h"
#include "FantasyFrontierRuinMysticEnemy.h"
#include "FantasyFrontierStalkerEnemy.h"
#include "GameFramework/Character.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

namespace
{
	UStaticMesh* GetCubeMesh()
	{
		static UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
		return Mesh;
	}

	UStaticMesh* GetCylinderMesh()
	{
		static UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
		return Mesh;
	}

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

	UMaterialInterface* GetBasicShapeMaterial()
	{
		static UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
		return Material;
	}
}

AFantasyFrontierTutorialDirector::AFantasyFrontierTutorialDirector()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
	SceneRoot->SetMobility(EComponentMobility::Static);

	auto MakeInstancedLayer = [this](const TCHAR* Name, UStaticMesh* Mesh) -> UInstancedStaticMeshComponent*
	{
		UInstancedStaticMeshComponent* Component = CreateDefaultSubobject<UInstancedStaticMeshComponent>(Name);
		Component->SetupAttachment(SceneRoot);
		Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Component->SetMobility(EComponentMobility::Static);
		if (Mesh)
		{
			Component->SetStaticMesh(Mesh);
		}
		if (UMaterialInterface* Material = GetBasicShapeMaterial())
		{
			Component->SetMaterial(0, Material);
		}
		return Component;
	};

	GroundInstances = MakeInstancedLayer(TEXT("GroundInstances"), GetCubeMesh());
	PathInstances = MakeInstancedLayer(TEXT("PathInstances"), GetCubeMesh());
	RuinInstances = MakeInstancedLayer(TEXT("RuinInstances"), GetCubeMesh());
	TreeTrunkInstances = MakeInstancedLayer(TEXT("TreeTrunkInstances"), GetCylinderMesh());
	TreeCanopyInstances = MakeInstancedLayer(TEXT("TreeCanopyInstances"), GetSphereMesh());
	CrystalInstances = MakeInstancedLayer(TEXT("CrystalInstances"), GetConeMesh());
	SmithInstances = MakeInstancedLayer(TEXT("SmithInstances"), GetCubeMesh());

	FunctionalNpcClass = AFantasyFrontierFunctionalNpc::StaticClass();
	WildlifeClass = AFantasyFrontierAmbientCreature::StaticClass();
	StalkerEnemyClass = AFantasyFrontierStalkerEnemy::StaticClass();
	RuinMysticEnemyClass = AFantasyFrontierRuinMysticEnemy::StaticClass();
	HiddenScenarioTriggerClass = AFantasyFrontierHiddenScenarioTrigger::StaticClass();
}

void AFantasyFrontierTutorialDirector::BeginPlay()
{
	Super::BeginPlay();

	if (!bBuiltEnvironment)
	{
		SuppressTemplateLevelActors();
		BuildEnvironment();
		SpawnRuntimeActors();
		PositionExistingPlayer();
		bBuiltEnvironment = true;
	}
}

void AFantasyFrontierTutorialDirector::HandleNpcInteraction(
	EFantasyFrontierNpcRole NpcRole,
	AFantasyFrontierPlayableCharacter* PlayerCharacter,
	bool bWasAlreadyTriggered)
{
	if (!PlayerCharacter)
	{
		return;
	}

	switch (NpcRole)
	{
	case EFantasyFrontierNpcRole::Guide:
		bGuideVisited = true;
		if (!bWasAlreadyTriggered && GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 7.0f, FColor::Cyan, TEXT("Guide: Follow the lantern path, then search the broken ridge. Some ruins answer only to movement."));
		}
		break;
	case EFantasyFrontierNpcRole::Smith:
		bSmithVisited = true;
		PlayerCharacter->ApplyHealing(5.0f, this);
		if (!bWasAlreadyTriggered && GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 6.5f, FColor::Orange, TEXT("Smith: Starter gear hooks are online. Later this station will handle upgrades and equipment layering."));
		}
		break;
	case EFantasyFrontierNpcRole::Trainer:
		bTrainerVisited = true;
		PlayerCharacter->UnlockSkillFusion();
		if (!bWasAlreadyTriggered && GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 6.5f, FColor::Emerald, TEXT("Trainer: Skill Fusion unlocked. Try Dash -> Heavy Attack for a Resonance strike."));
		}
		break;
	default:
		break;
	}
}

bool AFantasyFrontierTutorialDirector::TryUnlockHiddenScenario(AFantasyFrontierPlayableCharacter* PlayerCharacter)
{
	if (!PlayerCharacter || bHiddenScenarioUnlocked)
	{
		return false;
	}

	if (!bGuideVisited || !bTrainerVisited)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Silver, TEXT("The sigil remains dormant. Learn from the guide and the trainer first."));
		}
		return false;
	}

	if (PlayerCharacter->GetVelocity().Size2D() < 650.0f)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Turquoise, TEXT("The sigil flickers. Cross it with momentum to resonate with the ruin."));
		}
		return false;
	}

	bHiddenScenarioUnlocked = true;
	PlayerCharacter->ApplyRiskRewardModifier();
	PlayerCharacter->UnlockSkillFusion();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 9.0f, FColor::Green, TEXT("Unique Scenario: Swiftstep Covenant unlocked. Lower max health, cheaper dodge, faster stamina flow."));
	}

	return true;
}

void AFantasyFrontierTutorialDirector::BuildEnvironment()
{
	GroundInstances->ClearInstances();
	PathInstances->ClearInstances();
	RuinInstances->ClearInstances();
	TreeTrunkInstances->ClearInstances();
	TreeCanopyInstances->ClearInstances();
	CrystalInstances->ClearInstances();
	SmithInstances->ClearInstances();

	ApplyPalette(GroundInstances, FLinearColor(0.17f, 0.30f, 0.22f, 1.0f));
	ApplyPalette(PathInstances, FLinearColor(0.42f, 0.39f, 0.30f, 1.0f));
	ApplyPalette(RuinInstances, FLinearColor(0.36f, 0.43f, 0.50f, 1.0f));
	ApplyPalette(TreeTrunkInstances, FLinearColor(0.32f, 0.22f, 0.16f, 1.0f));
	ApplyPalette(TreeCanopyInstances, FLinearColor(0.24f, 0.48f, 0.30f, 1.0f));
	ApplyPalette(CrystalInstances, FLinearColor(0.28f, 0.92f, 0.82f, 1.0f));
	ApplyPalette(SmithInstances, FLinearColor(0.62f, 0.50f, 0.24f, 1.0f));

	for (int32 X = -2; X <= 9; ++X)
	{
		for (int32 Y = -4; Y <= 4; ++Y)
		{
			AddGroundTile(FVector(X * 400.0f, Y * 400.0f, -40.0f), FVector(4.0f, 4.0f, 0.12f));
		}
	}

	for (int32 Segment = 0; Segment < 8; ++Segment)
	{
		AddPathTile(FVector(450.0f + Segment * 320.0f, 0.0f, -6.0f), FVector(2.8f, 1.2f, 0.06f));
	}

	for (int32 Segment = 0; Segment < 4; ++Segment)
	{
		AddPathTile(FVector(1380.0f + Segment * 220.0f, 580.0f, -4.0f), FVector(1.8f, 1.0f, 0.06f));
	}

	AddRuinBlock(FVector(240.0f, 280.0f, 80.0f), FVector(0.8f, 0.8f, 1.8f));
	AddRuinBlock(FVector(560.0f, -240.0f, 110.0f), FVector(0.7f, 0.7f, 2.4f));
	AddRuinBlock(FVector(1560.0f, 720.0f, 120.0f), FVector(0.9f, 0.9f, 2.2f), FRotator(0.0f, 16.0f, 0.0f));
	AddRuinBlock(FVector(2250.0f, 80.0f, 90.0f), FVector(1.2f, 1.2f, 1.8f));
	AddRuinBlock(FVector(2360.0f, -220.0f, 150.0f), FVector(0.5f, 0.5f, 3.0f));
	AddRuinBlock(FVector(2480.0f, 260.0f, 110.0f), FVector(0.7f, 0.7f, 2.2f));

	for (const FVector TreeLocation : { FVector(-280.0f, -520.0f, 0.0f), FVector(260.0f, 620.0f, 0.0f), FVector(980.0f, -640.0f, 0.0f), FVector(1320.0f, 420.0f, 0.0f), FVector(1880.0f, -380.0f, 0.0f), FVector(2080.0f, 760.0f, 0.0f) })
	{
		AddTree(TreeLocation, 1.0f + (TreeLocation.X > 1500.0f ? 0.2f : 0.0f));
	}

	for (const FVector CrystalLocation : { FVector(1180.0f, 520.0f, 0.0f), FVector(1640.0f, 840.0f, 0.0f), FVector(2180.0f, -120.0f, 0.0f), FVector(2580.0f, 380.0f, 0.0f) })
	{
		AddCrystal(CrystalLocation, 1.0f);
	}

	SmithInstances->AddInstance(FTransform(FRotator::ZeroRotator, FVector(360.0f, -300.0f, 26.0f), FVector(0.9f, 0.6f, 0.3f)));
	SmithInstances->AddInstance(FTransform(FRotator::ZeroRotator, FVector(420.0f, -300.0f, 52.0f), FVector(0.25f, 0.25f, 1.2f)));
	SmithInstances->AddInstance(FTransform(FRotator::ZeroRotator, FVector(300.0f, -300.0f, 52.0f), FVector(0.25f, 0.25f, 1.2f)));
}

void AFantasyFrontierTutorialDirector::SpawnRuntimeActors()
{
	if (!GetWorld())
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	if (FunctionalNpcClass)
	{
		if (AFantasyFrontierFunctionalNpc* Guide = GetWorld()->SpawnActor<AFantasyFrontierFunctionalNpc>(FunctionalNpcClass, FVector(120.0f, 180.0f, 100.0f), FRotator(0.0f, 180.0f, 0.0f), SpawnParameters))
		{
			Guide->ConfigureNpc(EFantasyFrontierNpcRole::Guide, FText::FromString(TEXT("Liora, Frontier Guide")));
		}

		if (AFantasyFrontierFunctionalNpc* Smith = GetWorld()->SpawnActor<AFantasyFrontierFunctionalNpc>(FunctionalNpcClass, FVector(360.0f, -180.0f, 100.0f), FRotator(0.0f, 120.0f, 0.0f), SpawnParameters))
		{
			Smith->ConfigureNpc(EFantasyFrontierNpcRole::Smith, FText::FromString(TEXT("Brann, Smith")));
		}

		if (AFantasyFrontierFunctionalNpc* Trainer = GetWorld()->SpawnActor<AFantasyFrontierFunctionalNpc>(FunctionalNpcClass, FVector(880.0f, 120.0f, 100.0f), FRotator(0.0f, 165.0f, 0.0f), SpawnParameters))
		{
			Trainer->ConfigureNpc(EFantasyFrontierNpcRole::Trainer, FText::FromString(TEXT("Cael, Skill Trainer")));
		}
	}

	if (WildlifeClass)
	{
		if (AFantasyFrontierAmbientCreature* LumenMoth = GetWorld()->SpawnActor<AFantasyFrontierAmbientCreature>(WildlifeClass, FVector(620.0f, 260.0f, 120.0f), FRotator::ZeroRotator, SpawnParameters))
		{
			LumenMoth->ConfigureArchetype(EFantasyFrontierWildlifeArchetype::LumenMoth);
		}

		if (AFantasyFrontierAmbientCreature* Grazer = GetWorld()->SpawnActor<AFantasyFrontierAmbientCreature>(WildlifeClass, FVector(1420.0f, 760.0f, 24.0f), FRotator::ZeroRotator, SpawnParameters))
		{
			Grazer->ConfigureArchetype(EFantasyFrontierWildlifeArchetype::MeadowGrazer);
		}
	}

	if (StalkerEnemyClass)
	{
		GetWorld()->SpawnActor<AFantasyFrontierStalkerEnemy>(StalkerEnemyClass, FVector(2060.0f, -90.0f, 96.0f), FRotator(0.0f, 180.0f, 0.0f), SpawnParameters);
	}

	if (RuinMysticEnemyClass)
	{
		GetWorld()->SpawnActor<AFantasyFrontierRuinMysticEnemy>(RuinMysticEnemyClass, FVector(2400.0f, 200.0f, 96.0f), FRotator(0.0f, -150.0f, 0.0f), SpawnParameters);
	}

	if (HiddenScenarioTriggerClass)
	{
		GetWorld()->SpawnActor<AFantasyFrontierHiddenScenarioTrigger>(HiddenScenarioTriggerClass, FVector(1680.0f, 820.0f, 0.0f), FRotator::ZeroRotator, SpawnParameters);
	}
}

void AFantasyFrontierTutorialDirector::PositionExistingPlayer()
{
	if (ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0))
	{
		PlayerCharacter->SetActorLocation(FVector(-180.0f, 0.0f, 130.0f));
		PlayerCharacter->SetActorRotation(FRotator::ZeroRotator);
	}
}

void AFantasyFrontierTutorialDirector::SuppressTemplateLevelActors()
{
	if (!GetWorld())
	{
		return;
	}

	for (TActorIterator<AStaticMeshActor> It(GetWorld()); It; ++It)
	{
		AStaticMeshActor* StaticMeshActor = *It;
		if (!IsValid(StaticMeshActor))
		{
			continue;
		}

		StaticMeshActor->SetActorHiddenInGame(true);
		StaticMeshActor->SetActorEnableCollision(false);
		if (UStaticMeshComponent* StaticMeshComponent = StaticMeshActor->GetStaticMeshComponent())
		{
			StaticMeshComponent->SetVisibility(false, true);
			StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void AFantasyFrontierTutorialDirector::AddGroundTile(const FVector& Location, const FVector& Scale)
{
	GroundInstances->AddInstance(FTransform(FRotator::ZeroRotator, Location, Scale));
}

void AFantasyFrontierTutorialDirector::AddPathTile(const FVector& Location, const FVector& Scale)
{
	PathInstances->AddInstance(FTransform(FRotator::ZeroRotator, Location, Scale));
}

void AFantasyFrontierTutorialDirector::AddRuinBlock(const FVector& Location, const FVector& Scale, const FRotator& Rotation)
{
	RuinInstances->AddInstance(FTransform(Rotation, Location, Scale));
}

void AFantasyFrontierTutorialDirector::AddTree(const FVector& Location, float Scale)
{
	TreeTrunkInstances->AddInstance(FTransform(FRotator::ZeroRotator, Location + FVector(0.0f, 0.0f, 110.0f), FVector(0.22f, 0.22f, 2.2f * Scale)));
	TreeCanopyInstances->AddInstance(FTransform(FRotator::ZeroRotator, Location + FVector(0.0f, 0.0f, 260.0f), FVector(0.9f, 0.9f, 0.7f) * Scale));
	TreeCanopyInstances->AddInstance(FTransform(FRotator::ZeroRotator, Location + FVector(30.0f, 0.0f, 310.0f), FVector(0.65f, 0.65f, 0.48f) * Scale));
}

void AFantasyFrontierTutorialDirector::AddCrystal(const FVector& Location, float Scale)
{
	CrystalInstances->AddInstance(FTransform(FRotator(-12.0f, 0.0f, 0.0f), Location + FVector(0.0f, 0.0f, 72.0f), FVector(0.22f, 0.22f, 1.15f) * Scale));
}

void AFantasyFrontierTutorialDirector::ApplyPalette(UInstancedStaticMeshComponent* Component, const FLinearColor& Color) const
{
	if (UMaterialInstanceDynamic* MID = Component ? Component->CreateAndSetMaterialInstanceDynamic(0) : nullptr)
	{
		MID->SetVectorParameterValue(TEXT("Color"), Color);
		MID->SetVectorParameterValue(TEXT("BaseColor"), Color);
	}
}
