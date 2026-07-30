#include "FantasyFrontierTutorialDirector.h"

#include "Components/CapsuleComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "FantasyFrontierAmbientCreature.h"
#include "FantasyFrontierEnemyBase.h"
#include "FantasyFrontierFunctionalNpc.h"
#include "FantasyFrontierHiddenScenarioTrigger.h"
#include "FantasyFrontierPlayableCharacter.h"
#include "FantasyFrontierRuinMysticEnemy.h"
#include "FantasyFrontierStalkerEnemy.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/DecalActor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/DecalComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "LandscapeProxy.h"
#include "Misc/CommandLine.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "TP_ThirdPerson.h"

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

	UClass* GetKashkehHouse1NoTreeClass()
	{
		static UClass* Class = LoadClass<AActor>(nullptr, TEXT("/Game/Environment/Grassland/Kashkeh/Buldings_and_Structures/BP_Kashkeh_House1_NoTree.BP_Kashkeh_House1_NoTree_C"));
		return Class;
	}

	UClass* GetKashkehHouse2NoTreeClass()
	{
		static UClass* Class = LoadClass<AActor>(nullptr, TEXT("/Game/Environment/Grassland/Kashkeh/Buldings_and_Structures/BP_Kashkeh_House2_NoTree.BP_Kashkeh_House2_NoTree_C"));
		return Class;
	}

	UClass* GetKashkehStorageHutClass()
	{
		static UClass* Class = LoadClass<AActor>(nullptr, TEXT("/Game/Environment/Grassland/Kashkeh/Buldings_and_Structures/BP_StorageHut.BP_StorageHut_C"));
		return Class;
	}

	UClass* GetKashkehFenceRowClass()
	{
		static UClass* Class = LoadClass<AActor>(nullptr, TEXT("/Game/Environment/Grassland/Kashkeh/Buldings_and_Structures/BP_Kashkeh_FenceRow.BP_Kashkeh_FenceRow_C"));
		return Class;
	}

	UStaticMesh* GetStarterVillageWellMesh()
	{
		static UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Environment/StarterIsland/Meshes/Ancient_Village/SM_AncientVillage_Well.SM_AncientVillage_Well"));
		if (!Mesh)
		{
			Mesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Environment/Grassland/Meshes/BunkinBurrows/BunkinBurrows_Decor/SM_BB_WellwWater.SM_BB_WellwWater"));
		}
		if (!Mesh)
		{
			Mesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Environment/StarterIsland/Meshes/Church/SM_StarterIsland_Fountain.SM_StarterIsland_Fountain"));
		}
		if (!Mesh)
		{
			Mesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Environment/Clifftop/Meshes/SM_Fountain_01.SM_Fountain_01"));
		}
		return Mesh;
	}

	UStaticMesh* GetLerigothiStarterVillageWellMesh()
	{
		static UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Environment/Grassland/Meshes/BunkinBurrows/BunkinBurrows_Decor/SM_BB_WellwWater.SM_BB_WellwWater"));
		if (!Mesh)
		{
			Mesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Environment/StarterIsland/Meshes/Church/SM_StarterIsland_Fountain.SM_StarterIsland_Fountain"));
		}
		if (!Mesh)
		{
			Mesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Environment/Clifftop/Meshes/SM_Fountain_01.SM_Fountain_01"));
		}
		if (!Mesh)
		{
			Mesh = GetStarterVillageWellMesh();
		}
		return Mesh;
	}

	UStaticMesh* GetLerigothiVillageMarkerMesh()
	{
		static UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Environment/Grassland/Meshes/Lerigothi_Arhitecture/SM_horsestatue_Lerigothi.SM_horsestatue_Lerigothi"));
		return Mesh;
	}

	TArray<FVector> GetLikanaValleySpawnCandidates()
	{
		return {
			FVector(-520.0f, -3585.0f, 520.0f),
			FVector(-360.0f, -3312.0f, 420.0f),
			FVector(-954.0f, 2010.0f, 460.0f),
			FVector(-640.0f, -3862.0f, 520.0f)
		};
	}

	FVector GetLikanaValleySpawnHint()
	{
		return FVector(-520.0f, -3585.0f, 520.0f);
	}

	FVector GetLikanaValleyCenterHint()
	{
		return FVector(-520.0f, -3585.0f, 520.0f);
	}

	FVector GetLikanaValleyViewTargetHint()
	{
		return FVector(2200.0f, -2500.0f, 540.0f);
	}

	TArray<FVector> GetGrasslandVillageSpawnCandidates()
	{
		return {
			FVector(81612.0f, -58846.0f, -26024.0f),
			FVector(79875.0f, -62511.0f, -26011.0f),
			FVector(73693.0f, -46006.0f, -26651.0f),
			FVector(87674.0f, -60385.0f, -25824.0f)
		};
	}

	FVector GetGrasslandVillageSpawnHint()
	{
		return FVector(81612.0f, -58846.0f, -26024.0f);
	}

	FVector GetGrasslandVillageCenterHint()
	{
		return FVector(83500.0f, -59200.0f, -25980.0f);
	}

	FVector GetGrasslandVillageViewTargetHint()
	{
		return FVector(90678.0f, -50730.0f, -26018.0f);
	}

	TArray<FVector> GetGrasslandMolehillSpawnCandidates()
	{
		return {
			FVector(2305.0f, 9682.4f, 3200.0f),
			FVector(1200.0f, 8600.0f, 3200.0f),
			FVector(4200.0f, 9100.0f, 3200.0f),
			FVector(-500.0f, 8200.0f, 3200.0f)
		};
	}

	FVector GetGrasslandMolehillSpawnHint()
	{
		return FVector(2305.0f, 9682.4f, 3200.0f);
	}

	FVector GetGrasslandMolehillCenterHint()
	{
		return FVector(2400.0f, 9200.0f, 3200.0f);
	}

	FVector GetGrasslandMolehillViewTargetHint()
	{
		return FVector(5200.0f, 9800.0f, 3200.0f);
	}

	TArray<FVector> GetGrasslandFarmSpawnCandidates()
	{
		return {
			FVector(0.0f, 0.0f, 3200.0f),
			FVector(1400.0f, 900.0f, 3200.0f),
			FVector(-1600.0f, 600.0f, 3200.0f)
		};
	}

	FVector GetGrasslandFarmSpawnHint()
	{
		return FVector(0.0f, 0.0f, 3200.0f);
	}

	FVector GetGrasslandFarmCenterHint()
	{
		return FVector(0.0f, 0.0f, 3200.0f);
	}

	FVector GetGrasslandFarmViewTargetHint()
	{
		return FVector(2600.0f, 1400.0f, 3200.0f);
	}

	TArray<FVector> GetShoreLakeSpawnCandidates()
	{
		return {
			FVector(-2380.0f, 5160.0f, 1480.0f),
			FVector(-3200.0f, 4680.0f, 1340.0f),
			FVector(-1200.0f, 5660.0f, 1510.0f),
			FVector(-1680.0f, 6030.0f, 1510.0f),
			FVector(-540.0f, 6400.0f, 1510.0f)
		};
	}

	FVector GetShoreLakeSpawnHint()
	{
		return FVector(-3200.0f, 4680.0f, 1340.0f);
	}

	FVector GetShoreLakeCenterHint()
	{
		return FVector(-2800.0f, 4500.0f, 1360.0f);
	}

	FVector GetShoreLakeViewTargetHint()
	{
		return FVector(-322.1f, 6069.2f, 1509.0f);
	}

	bool IsStarterForestWorld(const UWorld* World)
	{
		if (!World)
		{
			return false;
		}

		const FString MapName = World->GetMapName();
		if (MapName.Contains(TEXT("TitanMainGrasslandHostSandbox")) ||
			MapName.Contains(TEXT("TitanMainGrasslandRealComponentSandbox")))
		{
			return false;
		}

		return MapName.Contains(TEXT("LI_CozyLake")) ||
			MapName.Contains(TEXT("LI_StarterIsland_LowerIsland")) ||
			MapName.Contains(TEXT("LI_CelticVillage")) ||
			MapName.Contains(TEXT("LI_Grassland_Celtic_Village_Landscaping")) ||
			MapName.Contains(TEXT("LI_Grassland_MolehillGrove")) ||
			MapName.Contains(TEXT("LI_Grassland_Farm")) ||
			MapName.Contains(TEXT("LI_Likana_Valley")) ||
			MapName.Contains(TEXT("LI_ShoreLake_village")) ||
			MapName.Contains(TEXT("LI_Littlegarden")) ||
			MapName.Contains(TEXT("LI_Grassland_Village")) ||
			MapName.Contains(TEXT("LI_Grassland_FLerihnVillage_Lerigothi")) ||
			MapName.Contains(TEXT("LI_Kashkeh_")) ||
			MapName.Contains(TEXT("FF_Starter_GrasslandRegion")) ||
			MapName.Contains(TEXT("Lvl_StarterForest")) ||
			MapName.Contains(TEXT("TitanMain"));
	}

	bool IsLowerIslandWorld(const UWorld* World)
	{
		return World && World->GetMapName().Contains(TEXT("LI_StarterIsland_LowerIsland"));
	}

	bool IsTitanMainWorld(const UWorld* World)
	{
		return World && World->GetMapName().Contains(TEXT("TitanMain")) &&
			!World->GetMapName().Contains(TEXT("TitanMainGrasslandHostSandbox")) &&
			!World->GetMapName().Contains(TEXT("TitanMainGrasslandRealComponentSandbox"));
	}

	bool IsFFStarterGrasslandRegionWorld(const UWorld* World)
	{
		return World && World->GetMapName().Contains(TEXT("FF_Starter_GrasslandRegion"));
	}

	bool IsFFStarterHighlandBlockoutWorld(const UWorld* World)
	{
		return World && World->GetMapName().Contains(TEXT("FF_Starter_Highland_Blockout"));
	}

	bool IsTitanMainGrasslandHostSandboxWorld(const UWorld* World)
	{
		return World &&
			(World->GetMapName().Contains(TEXT("TitanMainGrasslandHostSandbox")) ||
				World->GetMapName().Contains(TEXT("TitanMainGrasslandRealComponentSandbox")));
	}

	bool IsKashkehWorld(const UWorld* World)
	{
		return World && World->GetMapName().Contains(TEXT("LI_Kashkeh_"));
	}

	bool UsesKashkehStarterBaseWorld(const UWorld* World)
	{
		return IsKashkehWorld(World) || IsTitanMainWorld(World) || IsFFStarterGrasslandRegionWorld(World);
	}

	FVector GetFFStarterHighlandSpawnHint()
	{
		return FVector(-18000.0f, 4000.0f, 0.0f);
	}

	FVector GetFFStarterHighlandViewTargetHint()
	{
		return FVector(26000.0f, 24000.0f, 0.0f);
	}

	bool TryGetHighlandBlockoutPlayerStart(const UObject* WorldContextObject, FVector& OutLocation, FRotator& OutRotation)
	{
		if (!WorldContextObject)
		{
			return false;
		}

		APlayerStart* PlayerStart = Cast<APlayerStart>(UGameplayStatics::GetActorOfClass(WorldContextObject, APlayerStart::StaticClass()));
		if (!PlayerStart)
		{
			return false;
		}

		OutLocation = PlayerStart->GetActorLocation();
		OutRotation = PlayerStart->GetActorRotation();
		return true;
	}

	bool IsLikanaValleyWorld(const UWorld* World)
	{
		return World && World->GetMapName().Contains(TEXT("LI_Likana_Valley"));
	}

	bool IsShoreLakeVillageWorld(const UWorld* World)
	{
		return World && World->GetMapName().Contains(TEXT("LI_ShoreLake_village"));
	}

	bool IsGrasslandVillageWorld(const UWorld* World)
	{
		return World && World->GetMapName().Contains(TEXT("LI_Grassland_Village"));
	}

	bool IsGrasslandMolehillWorld(const UWorld* World)
	{
		return World && World->GetMapName().Contains(TEXT("LI_Grassland_MolehillGrove"));
	}

	bool IsGrasslandFarmWorld(const UWorld* World)
	{
		return World && World->GetMapName().Contains(TEXT("LI_Grassland_Farm"));
	}

	bool IsCelticVillageWorld(const UWorld* World)
	{
		if (!World)
		{
			return false;
		}

		const FString MapName = World->GetMapName();
		return MapName.Contains(TEXT("LI_CelticVillage")) || MapName.Contains(TEXT("LI_Grassland_Celtic_Village_Landscaping"));
	}

	bool IsLerigothiWorld(const UWorld* World)
	{
		return World && World->GetMapName().Contains(TEXT("LI_Grassland_FLerihnVillage_Lerigothi"));
	}

	bool NameContainsAny(const FString& Source, std::initializer_list<const TCHAR*> Tokens)
	{
		for (const TCHAR* Token : Tokens)
		{
			if (Source.Contains(Token))
			{
				return true;
			}
		}

		return false;
	}

	bool IsStarterStructureHit(const FHitResult& Hit)
	{
		const FString ActorName = GetNameSafe(Hit.GetActor()).ToLower();
		const FString ActorClassName = IsValid(Hit.GetActor()) ? Hit.GetActor()->GetClass()->GetName().ToLower() : FString();
		const FString ComponentName = GetNameSafe(Hit.Component.Get()).ToLower();
		FString MeshName;
		if (const UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Hit.Component.Get()))
		{
			MeshName = GetNameSafe(StaticMeshComponent->GetStaticMesh()).ToLower();
		}

		return NameContainsAny(ActorName, { TEXT("roof"), TEXT("stairs"), TEXT("house"), TEXT("hut"), TEXT("storage"), TEXT("store"), TEXT("shop"), TEXT("building"), TEXT("platform"), TEXT("vine"), TEXT("tree"), TEXT("fountain"), TEXT("well"), TEXT("hearth"), TEXT("carrot"), TEXT("crop") }) ||
			NameContainsAny(ActorClassName, { TEXT("roof"), TEXT("stairs"), TEXT("house"), TEXT("hut"), TEXT("storage"), TEXT("store"), TEXT("shop"), TEXT("building"), TEXT("platform"), TEXT("vine"), TEXT("tree"), TEXT("fountain"), TEXT("well"), TEXT("hearth"), TEXT("carrot"), TEXT("crop") }) ||
			NameContainsAny(ComponentName, { TEXT("roof"), TEXT("stairs"), TEXT("house"), TEXT("hut"), TEXT("storage"), TEXT("store"), TEXT("shop"), TEXT("building"), TEXT("platform"), TEXT("vine"), TEXT("tree"), TEXT("fountain"), TEXT("well"), TEXT("hearth"), TEXT("carrot"), TEXT("crop") }) ||
			NameContainsAny(MeshName, { TEXT("roof"), TEXT("stairs"), TEXT("house"), TEXT("hut"), TEXT("storage"), TEXT("store"), TEXT("shop"), TEXT("building"), TEXT("platform"), TEXT("vine"), TEXT("tree"), TEXT("fountain"), TEXT("well"), TEXT("hearth"), TEXT("carrot"), TEXT("crop") });
	}

	bool IsLerigothiPreferredGroundHit(const FHitResult& Hit)
	{
		const FString ActorName = GetNameSafe(Hit.GetActor()).ToLower();
		const FString ActorClassName = IsValid(Hit.GetActor()) ? Hit.GetActor()->GetClass()->GetName().ToLower() : FString();
		const FString ComponentName = GetNameSafe(Hit.Component.Get()).ToLower();
		FString MeshName;
		FString MaterialNames;
		if (const UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Hit.Component.Get()))
		{
			MeshName = GetNameSafe(StaticMeshComponent->GetStaticMesh()).ToLower();
			for (const UMaterialInterface* Material : StaticMeshComponent->GetMaterials())
			{
				if (IsValid(Material))
				{
					MaterialNames += Material->GetName().ToLower();
					MaterialNames += TEXT(" ");
				}
			}
		}

		const bool bDirtActor = NameContainsAny(ActorName, { TEXT("bp_dirtbase"), TEXT("bp_grasscirclewtuffs"), TEXT("bp_path_logdirt"), TEXT("dirt"), TEXT("road"), TEXT("path") }) ||
			NameContainsAny(ActorClassName, { TEXT("bp_dirtbase"), TEXT("bp_grasscirclewtuffs"), TEXT("bp_path_logdirt") });
		const bool bDirtMesh = NameContainsAny(ComponentName, { TEXT("dirt"), TEXT("road"), TEXT("path") }) ||
			NameContainsAny(MeshName, { TEXT("sm_dirtcircles"), TEXT("road_dirt"), TEXT("path") });
		const bool bDirtMaterial = NameContainsAny(MaterialNames, { TEXT("mi_grassland_dirt_nograssblend"), TEXT("mi_grassroad_3"), TEXT("mi_grassland_rrvt1") });
		return bDirtActor || bDirtMesh || bDirtMaterial;
	}

	FVector GetLowerIslandSpawn()
	{
		return FVector(-609.1f, 3981.4f, 1703.8f);
	}

	TArray<FVector> GetLerigothiSpawnCandidates()
	{
		return {
			FVector(9515.0f, -5008.0f, 2558.0f),
			FVector(9395.0f, -4768.0f, 2558.0f),
			FVector(9155.0f, -4768.0f, 2558.0f),
			FVector(8795.0f, -4768.0f, 2558.0f),
			FVector(9145.5f, -4497.9f, 2559.3f),
			FVector(9169.2f, -4505.5f, 2587.6f),
			FVector(9236.2f, -4537.1f, 2564.5f),
			FVector(9269.4f, -4496.0f, 2595.6f),
			FVector(9143.3f, -4561.8f, 2558.4f),
			// Keep the older north-west band only as a late fallback once the clean Lerigothi south-east host fails.
			FVector(8500.0f, -3480.0f, 2908.4f),
			FVector(8540.0f, -3420.0f, 2908.4f),
			FVector(8795.6f, -3329.7f, 2910.6f),
			FVector(8795.6f, -3537.6f, 2788.4f),
			FVector(8338.3f, -3495.4f, 2740.2f),
			FVector(8420.0f, -3920.0f, 2709.4f),
			FVector(7283.3f, -5276.0f, 2466.6f)
		};
	}

	FVector GetLerigothiVillageSpawnHint()
	{
		return FVector(9515.0f, -5008.0f, 2558.0f);
	}

	FVector GetLerigothiVillageCenterHint()
	{
		return FVector(9395.0f, -5008.0f, 2558.0f);
	}

	FVector GetLerigothiVillageViewTargetHint()
	{
		return GetLerigothiVillageCenterHint();
	}

	FVector GetKashkehVillageCenterHint()
	{
		return FVector(1829.0f, 2943.0f, 3600.0f);
	}

	FVector GetKashkehViewTargetHint()
	{
		return FVector(3218.1f, 796.5f, 3628.4f);
	}

	TArray<FVector> GetKashkehSpawnCandidates()
	{
		return {
			FVector(1829.0f, 2943.0f, 3600.0f),
			FVector(2113.0f, 2817.0f, 3600.0f),
			FVector(1490.0f, 2700.0f, 3600.0f),
			FVector(1248.0f, 3125.0f, 3600.0f),
			FVector(1388.0f, 3340.0f, 3600.0f),
			FVector(1609.0f, 3003.0f, 3600.0f),
			FVector(1839.0f, 2920.0f, 3600.0f)
		};
	}

	TArray<FVector> GetCelticVillageSpawnCandidates()
	{
		return {
			FVector(4980.0f, 6220.0f, -700.0f),
			FVector(5210.0f, 6420.0f, -700.0f),
			FVector(4640.0f, 6060.0f, -700.0f),
			FVector(5450.0f, 6230.0f, -700.0f),
			FVector(4380.0f, 6500.0f, -700.0f)
		};
	}

TArray<FVector> GetLerigothiEnemyCandidates(const FVector& HubLocation)
{
	return {
		HubLocation + FVector(-2280.0f, 220.0f, 0.0f),
		HubLocation + FVector(-2540.0f, -280.0f, 0.0f),
		HubLocation + FVector(-2940.0f, 360.0f, 0.0f),
		HubLocation + FVector(-3260.0f, -120.0f, 0.0f),
		HubLocation + FVector(1620.0f, -140.0f, 0.0f),
		HubLocation + FVector(1760.0f, 180.0f, 0.0f),
		HubLocation + FVector(1940.0f, -320.0f, 0.0f),
		HubLocation + FVector(2140.0f, -80.0f, 0.0f)
	};
}

	TArray<FVector> GetCelticVillageEnemyCandidates(const FVector& VillageCenter)
	{
		return {
			VillageCenter + FVector(1980.0f, -220.0f, 0.0f),
			VillageCenter + FVector(2320.0f, 420.0f, 0.0f),
			VillageCenter + FVector(1700.0f, -980.0f, 0.0f),
			VillageCenter + FVector(-1850.0f, 960.0f, 0.0f)
		};
	}

	TArray<FVector> GetKashkehEnemyCandidates(const FVector& HubLocation)
	{
		return {
			FVector(2303.6f, 1384.9f, 3729.8f),
			FVector(3218.1f, 796.5f, 3628.4f),
			FVector(1970.8f, -98.7f, 3207.6f),
			FVector(1597.8f, -678.6f, 3136.2f),
			HubLocation + FVector(1680.0f, 240.0f, 0.0f)
		};
	}

	FName GetRoleTag(EFantasyFrontierNpcRole Role)
	{
		switch (Role)
		{
		case EFantasyFrontierNpcRole::Guide:
			return TEXT("FFGuideNpc");
		case EFantasyFrontierNpcRole::Smith:
			return TEXT("FFSmithNpc");
		case EFantasyFrontierNpcRole::Trainer:
			return TEXT("FFTrainerNpc");
		default:
			return NAME_None;
		}
	}

	template <typename TActorClass>
	void DestroyActorsWithTag(UWorld* World, FName Tag)
	{
		if (!World || Tag.IsNone())
		{
			return;
		}

		TArray<AActor*> Actors;
		UGameplayStatics::GetAllActorsOfClass(World, TActorClass::StaticClass(), Actors);
		for (AActor* ActorBase : Actors)
		{
			TActorClass* Actor = Cast<TActorClass>(ActorBase);
			if (IsValid(Actor) && Actor->ActorHasTag(Tag))
			{
				Actor->Destroy();
			}
		}
	}

	template <typename TActorClass>
	TActorClass* FindActorByTag(UWorld* World, FName Tag)
	{
		if (!World || Tag.IsNone())
		{
			return nullptr;
		}

		for (TActorIterator<TActorClass> It(World); It; ++It)
		{
			if (It->ActorHasTag(Tag))
			{
				return *It;
			}
		}

		return nullptr;
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

	if (bBuiltEnvironment)
	{
		return;
	}

	if (IsTitanMainGrasslandHostSandboxWorld(GetWorld()))
	{
		bBuiltEnvironment = true;
		return;
	}

	if (IsFFStarterHighlandBlockoutWorld(GetWorld()))
	{
		PositionExistingPlayer();
		bBuiltEnvironment = true;
		return;
	}

	if (ShouldUseStarterForestSlice())
	{
		EnsureLowerIslandSurroundingContext();
		if (((IsGrasslandFarmWorld(GetWorld()) || IsGrasslandMolehillWorld(GetWorld())) && GrasslandFarmShellLevel && !GrasslandFarmShellLevel->GetLoadedLevel()) ||
			IsLerigothiWorld(GetWorld()) ||
			UsesKashkehStarterBaseWorld(GetWorld()))
		{
			HandleLowerIslandSurroundingLevelShown();
			return;
		}
		EnsureStarterForestLighting();
		ApplyLerigothiVillageGroundCleanup();
		SpawnRuntimeActors();
		PositionExistingPlayer();
		ScheduleStarterForestSpawnValidation();
		bBuiltEnvironment = true;
		return;
	}

	SuppressTemplateLevelActors();
	BuildEnvironment();
	SpawnRuntimeActors();
	PositionExistingPlayer();
	bBuiltEnvironment = true;
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
			GEngine->AddOnScreenDebugMessage(-1, 7.0f, FColor::Cyan, TEXT("Guide: Follow the path out of the village. The first threat waits beyond the safe square."));
		}
		break;
	case EFantasyFrontierNpcRole::Smith:
		bSmithVisited = true;
		PlayerCharacter->ApplyHealing(5.0f, this);
		if (!bWasAlreadyTriggered && GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 6.5f, FColor::Orange, TEXT("Smith: The starter village is stable now. Later this station will handle upgrades and equipment layering."));
		}
		break;
	case EFantasyFrontierNpcRole::Trainer:
		bTrainerVisited = true;
		PlayerCharacter->UnlockSkillFusion();
		if (!bWasAlreadyTriggered && GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 6.5f, FColor::Green, TEXT("Trainer: Skill Fusion unlocked. Try Dash -> Heavy Attack for a Resonance strike."));
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
			GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::White, TEXT("The sigil remains dormant. Learn from the guide and the trainer first."));
		}
		return false;
	}

	if (PlayerCharacter->GetVelocity().Size2D() < 650.0f)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Cyan, TEXT("The sigil flickers. Cross it with momentum to resonate with the ruin."));
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

bool AFantasyFrontierTutorialDirector::TryResolveTutorialGroundLocation(const FVector& DesiredLocation, float HeightOffset, FVector& OutResolvedLocation) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		OutResolvedLocation = DesiredLocation;
		return false;
	}

	static const TArray<FVector> BaseOffsets = {
		FVector::ZeroVector,
		FVector(120.0f, 0.0f, 0.0f),
		FVector(-120.0f, 0.0f, 0.0f),
		FVector(0.0f, 120.0f, 0.0f),
		FVector(0.0f, -120.0f, 0.0f),
		FVector(180.0f, 180.0f, 0.0f),
		FVector(-180.0f, -180.0f, 0.0f)
	};

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ResolveTutorialGroundLocation), false, this);
	FHitResult BestHit;
	bool bFoundHit = false;
	float BestVerticalDelta = TNumericLimits<float>::Max();
	FHitResult BestPreferredGroundHit;
	bool bFoundPreferredGroundHit = false;
	float BestPreferredGroundScore = TNumericLimits<float>::Max();
	FHitResult BestHighlandLandscapeHit;
	bool bFoundHighlandLandscapeHit = false;
	float BestHighlandLandscapeScore = TNumericLimits<float>::Max();
	FHitResult BestFallbackHit;
	bool bFoundFallbackHit = false;
	float BestFallbackVerticalDelta = TNumericLimits<float>::Max();
	const bool bFilterStarterStructures = UsesKashkehStarterBaseWorld(World) || IsLerigothiWorld(World) || IsShoreLakeVillageWorld(World);
	const bool bPreferLerigothiGround = IsLerigothiWorld(World);
	const bool bPreferHighlandLandscapeGround = IsFFStarterHighlandBlockoutWorld(World);
	const float MaxAllowedHeightAboveDesired = IsLerigothiWorld(World) ? 220.0f : 420.0f;
	const float MaxAllowedDropBelowDesired = IsLerigothiWorld(World) ? 520.0f : 760.0f;
	TArray<FVector> ProbeOffsets = BaseOffsets;
	if (bPreferHighlandLandscapeGround)
	{
		ProbeOffsets.Append({
			FVector(240.0f, 0.0f, 0.0f),
			FVector(-240.0f, 0.0f, 0.0f),
			FVector(0.0f, 240.0f, 0.0f),
			FVector(0.0f, -240.0f, 0.0f),
			FVector(240.0f, 240.0f, 0.0f),
			FVector(-240.0f, 240.0f, 0.0f),
			FVector(240.0f, -240.0f, 0.0f),
			FVector(-240.0f, -240.0f, 0.0f),
			FVector(360.0f, 0.0f, 0.0f),
			FVector(-360.0f, 0.0f, 0.0f),
			FVector(0.0f, 360.0f, 0.0f),
			FVector(0.0f, -360.0f, 0.0f)
		});
	}

	for (const FVector& Offset : ProbeOffsets)
	{
		const FVector ProbeLocation = DesiredLocation + Offset;
		const FVector TraceStart = ProbeLocation + FVector(0.0f, 0.0f, 2200.0f);
		const FVector TraceEnd = ProbeLocation - FVector(0.0f, 0.0f, 3200.0f);
		TArray<FHitResult> Hits;
		bool bFoundTrace = World->LineTraceMultiByChannel(Hits, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
		if (!bFoundTrace)
		{
			Hits.Reset();
			bFoundTrace = World->LineTraceMultiByChannel(Hits, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams);
		}

		if (!bFoundTrace)
		{
			continue;
		}

		for (const FHitResult& Hit : Hits)
		{
			if (!Hit.bBlockingHit)
			{
				continue;
			}

			const float VerticalDelta = FMath::Abs(Hit.Location.Z - DesiredLocation.Z);
			if (!bFoundFallbackHit || VerticalDelta < BestFallbackVerticalDelta)
			{
				BestFallbackHit = Hit;
				BestFallbackVerticalDelta = VerticalDelta;
				bFoundFallbackHit = true;
			}

			if (Hit.ImpactNormal.Z < 0.55f)
			{
				continue;
			}

			if (bFilterStarterStructures)
			{
				if (Hit.Location.Z > DesiredLocation.Z + MaxAllowedHeightAboveDesired)
				{
					continue;
				}

				if (Hit.Location.Z < DesiredLocation.Z - MaxAllowedDropBelowDesired)
				{
					continue;
				}

				if (IsStarterStructureHit(Hit))
				{
					continue;
				}
			}

			if (bPreferLerigothiGround && IsLerigothiPreferredGroundHit(Hit))
			{
				const float HeightPenalty = FMath::Max(0.0f, Hit.Location.Z - DesiredLocation.Z) * 3.0f;
				const float PreferredGroundScore = VerticalDelta + HeightPenalty;
				if (!bFoundPreferredGroundHit || PreferredGroundScore < BestPreferredGroundScore)
				{
					BestPreferredGroundHit = Hit;
					BestPreferredGroundScore = PreferredGroundScore;
					bFoundPreferredGroundHit = true;
				}
			}

			if (bPreferHighlandLandscapeGround)
			{
				const AActor* HitActor = Hit.GetActor();
				const AActor* ComponentOwner = Hit.Component.IsValid() ? Hit.Component->GetOwner() : nullptr;
				const bool bIsLandscapeHit =
					(HitActor && HitActor->IsA<ALandscapeProxy>()) ||
					(ComponentOwner && ComponentOwner->IsA<ALandscapeProxy>());
				if (bIsLandscapeHit)
				{
					const float HeightPenalty = FMath::Max(0.0f, Hit.Location.Z - DesiredLocation.Z) * 3.0f;
					const float FlatnessPenalty = FMath::Max(0.0f, 0.97f - Hit.ImpactNormal.Z) * 1800.0f;
					const float OffsetPenalty = Offset.Size2D() * 0.12f;
					const float LandscapeScore = VerticalDelta + HeightPenalty + FlatnessPenalty + OffsetPenalty;
					if (!bFoundHighlandLandscapeHit || LandscapeScore < BestHighlandLandscapeScore)
					{
						BestHighlandLandscapeHit = Hit;
						BestHighlandLandscapeScore = LandscapeScore;
						bFoundHighlandLandscapeHit = true;
					}
				}
			}

			if (!bFoundHit || VerticalDelta < BestVerticalDelta)
			{
				BestHit = Hit;
				BestVerticalDelta = VerticalDelta;
				bFoundHit = true;
			}
		}
	}

	if (bFoundPreferredGroundHit)
	{
		OutResolvedLocation = BestPreferredGroundHit.Location + FVector(0.0f, 0.0f, HeightOffset);
		return true;
	}

	if (bFoundHighlandLandscapeHit)
	{
		OutResolvedLocation = BestHighlandLandscapeHit.Location + FVector(0.0f, 0.0f, HeightOffset);
		UE_LOG(
			LogTP_ThirdPerson,
			Display,
			TEXT("FFHighlandLandscapeResolve desired=(%.1f, %.1f, %.1f) resolved=(%.1f, %.1f, %.1f) normalZ=%.3f actor=%s component=%s"),
			DesiredLocation.X,
			DesiredLocation.Y,
			DesiredLocation.Z,
			OutResolvedLocation.X,
			OutResolvedLocation.Y,
			OutResolvedLocation.Z,
			BestHighlandLandscapeHit.ImpactNormal.Z,
			*GetNameSafe(BestHighlandLandscapeHit.GetActor()),
			*GetNameSafe(BestHighlandLandscapeHit.Component.Get()));
		return true;
	}

	const bool bFallbackLooksLikeStructure = bFoundFallbackHit && bFilterStarterStructures && IsStarterStructureHit(BestFallbackHit);
	const bool bFallbackTooHigh = bFoundFallbackHit && bFilterStarterStructures && BestFallbackHit.Location.Z > DesiredLocation.Z + MaxAllowedHeightAboveDesired;
	const bool bFallbackTooLow = bFoundFallbackHit && bFilterStarterStructures && BestFallbackHit.Location.Z < DesiredLocation.Z - MaxAllowedDropBelowDesired;
	if (!bFoundHit && bFoundFallbackHit && !bFallbackLooksLikeStructure && !bFallbackTooHigh && !bFallbackTooLow)
	{
		BestHit = BestFallbackHit;
		bFoundHit = true;
	}

	if (!bFoundHit)
	{
		OutResolvedLocation = DesiredLocation;
		return false;
	}

	OutResolvedLocation = BestHit.Location + FVector(0.0f, 0.0f, HeightOffset);
	return true;
}

FVector AFantasyFrontierTutorialDirector::ResolveTutorialGroundLocation(const FVector& DesiredLocation, float HeightOffset) const
{
	FVector ResolvedLocation = DesiredLocation;
	if (TryResolveTutorialGroundLocation(DesiredLocation, HeightOffset, ResolvedLocation))
	{
		return ResolvedLocation;
	}

	return DesiredLocation;
}

bool AFantasyFrontierTutorialDirector::ShouldUseStarterForestSlice() const
{
	return IsStarterForestWorld(GetWorld());
}

FVector AFantasyFrontierTutorialDirector::GetStarterForestPlayerLocation() const
{
	if (IsCelticVillageWorld(GetWorld()))
	{
		FVector Resolved = FVector::ZeroVector;
		for (const FVector& Candidate : GetCelticVillageSpawnCandidates())
		{
			if (TryResolveTutorialGroundLocation(Candidate, 2.0f, Resolved))
			{
				return Resolved;
			}
		}

		return ResolveTutorialGroundLocation(FVector(4980.0f, 6220.0f, -700.0f), 2.0f);
	}

	if (IsShoreLakeVillageWorld(GetWorld()))
	{
		FVector Resolved = FVector::ZeroVector;
		for (const FVector& Candidate : GetShoreLakeSpawnCandidates())
		{
			if (TryResolveTutorialGroundLocation(Candidate, 2.0f, Resolved))
			{
				return Resolved;
			}
		}

		return ResolveTutorialGroundLocation(GetShoreLakeSpawnHint(), 2.0f);
	}

	if (IsGrasslandVillageWorld(GetWorld()))
	{
		FVector Resolved = FVector::ZeroVector;
		for (const FVector& Candidate : GetGrasslandVillageSpawnCandidates())
		{
			if (TryResolveTutorialGroundLocation(Candidate, 2.0f, Resolved))
			{
				return Resolved;
			}
		}

		return ResolveTutorialGroundLocation(GetGrasslandVillageSpawnHint(), 2.0f);
	}

	if (IsGrasslandMolehillWorld(GetWorld()))
	{
		FVector Resolved = FVector::ZeroVector;
		for (const FVector& Candidate : GetGrasslandMolehillSpawnCandidates())
		{
			if (TryResolveTutorialGroundLocation(Candidate, 2.0f, Resolved))
			{
				return Resolved;
			}
		}

		return ResolveTutorialGroundLocation(GetGrasslandMolehillSpawnHint(), 2.0f);
	}

	if (IsGrasslandFarmWorld(GetWorld()))
	{
		FVector Resolved = FVector::ZeroVector;
		for (const FVector& Candidate : GetGrasslandFarmSpawnCandidates())
		{
			if (TryResolveTutorialGroundLocation(Candidate, 2.0f, Resolved))
			{
				return Resolved;
			}
		}

		return ResolveTutorialGroundLocation(GetGrasslandFarmSpawnHint(), 2.0f);
	}

	if (IsLerigothiWorld(GetWorld()))
	{
		FVector Resolved = FVector::ZeroVector;
		for (const FVector& Candidate : GetLerigothiSpawnCandidates())
		{
			if (TryResolveTutorialGroundLocation(Candidate, 2.0f, Resolved))
			{
				return Resolved;
			}
		}

		return ResolveTutorialGroundLocation(GetLerigothiVillageSpawnHint(), 2.0f);
	}

	if (IsLikanaValleyWorld(GetWorld()))
	{
		FVector Resolved = FVector::ZeroVector;
		for (const FVector& Candidate : GetLikanaValleySpawnCandidates())
		{
			if (TryResolveTutorialGroundLocation(Candidate, 2.0f, Resolved))
			{
				return Resolved;
			}
		}

		return ResolveTutorialGroundLocation(GetLikanaValleySpawnHint(), 2.0f);
	}

	if (UsesKashkehStarterBaseWorld(GetWorld()))
	{
		FVector Resolved = FVector::ZeroVector;
		for (const FVector& Candidate : GetKashkehSpawnCandidates())
		{
			if (TryResolveTutorialGroundLocation(Candidate, 2.0f, Resolved))
			{
				return Resolved;
			}
		}

		return ResolveTutorialGroundLocation(FVector(1609.0f, 3003.0f, 3600.0f), 2.0f);
	}

	if (IsLowerIslandWorld(GetWorld()))
	{
		return ResolveTutorialGroundLocation(GetLowerIslandSpawn(), 2.0f);
	}

	return ResolveTutorialGroundLocation(FVector::ZeroVector, 2.0f);
}

void AFantasyFrontierTutorialDirector::EnsureStarterForestLighting()
{
	UWorld* World = GetWorld();
	if (!World || !(IsLerigothiWorld(World) || UsesKashkehStarterBaseWorld(World) || IsLikanaValleyWorld(World) || IsShoreLakeVillageWorld(World) || IsGrasslandVillageWorld(World) || IsGrasslandMolehillWorld(World) || IsGrasslandFarmWorld(World)))
	{
		return;
	}

	const FName RuntimeLightingTag(TEXT("FFRuntimeStarterLighting"));
	bool bHasDirectionalLight = false;
	bool bHasSkyLight = false;
	bool bHasSkyAtmosphere = false;
	bool bHasHeightFog = false;
	bool bHasPostProcess = false;
	const bool bIsGrasslandFarm = IsGrasslandFarmWorld(World);
	const bool bIsFFStarterGrassland = IsFFStarterGrasslandRegionWorld(World);
	const FRotator DaylightRotation(-46.0f, 26.0f, 0.0f);
	const float DaylightDirectionalIntensity = 8.6f;
	const float DaylightSkylightIntensity = 0.78f;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!IsValid(Actor))
		{
			continue;
		}

		bHasDirectionalLight |= Actor->IsA<ADirectionalLight>();
		bHasSkyLight |= Actor->IsA<ASkyLight>();
		bHasSkyAtmosphere |= Actor->IsA<ASkyAtmosphere>();
		bHasHeightFog |= Actor->IsA<AExponentialHeightFog>();
		bHasPostProcess |= Actor->IsA<APostProcessVolume>();

		if (ADirectionalLight* DirectionalLight = Cast<ADirectionalLight>(Actor))
		{
			DirectionalLight->Tags.AddUnique(RuntimeLightingTag);
			DirectionalLight->SetActorRotation(DaylightRotation);
			if (UDirectionalLightComponent* LightComponent = Cast<UDirectionalLightComponent>(DirectionalLight->GetLightComponent()))
			{
				LightComponent->SetMobility(EComponentMobility::Movable);
				LightComponent->SetIntensity(bIsFFStarterGrassland ? 9.0f : (UsesKashkehStarterBaseWorld(World) ? 16.0f : DaylightDirectionalIntensity));
				LightComponent->SetLightColor(FLinearColor(1.0f, 0.96f, 0.90f));
				LightComponent->SetCastShadows(true);
				LightComponent->bAtmosphereSunLight = true;
				LightComponent->SetAtmosphereSunLightIndex(0);
			}
			continue;
		}

		if (ASkyLight* SkyLight = Cast<ASkyLight>(Actor))
		{
			SkyLight->Tags.AddUnique(RuntimeLightingTag);
			if (USkyLightComponent* SkyComponent = SkyLight->GetLightComponent())
			{
				SkyComponent->SetMobility(EComponentMobility::Movable);
				SkyComponent->SetIntensity(bIsFFStarterGrassland ? 0.8f : (UsesKashkehStarterBaseWorld(World) ? 2.0f : DaylightSkylightIntensity));
				SkyComponent->SetLightColor(FLinearColor(0.97f, 0.98f, 0.99f));
				// Grassland Farm currently stalls the packaged render thread when its
				// runtime Skylight flips into real-time capture on first view.
				SkyComponent->SetRealTimeCapture(false);
				if (!bIsGrasslandFarm)
				{
					SkyComponent->RecaptureSky();
				}
			}
			continue;
		}

		if (AExponentialHeightFog* HeightFog = Cast<AExponentialHeightFog>(Actor))
		{
			HeightFog->Tags.AddUnique(RuntimeLightingTag);
			if (UExponentialHeightFogComponent* FogComponent = HeightFog->GetComponent())
			{
				FogComponent->SetFogDensity(bIsFFStarterGrassland ? 0.00018f : (UsesKashkehStarterBaseWorld(World) ? 0.00005f : 0.00045f));
				FogComponent->SetFogHeightFalloff(0.2f);
				FogComponent->SetFogInscatteringColor(FLinearColor(0.84f, 0.85f, 0.82f, 1.0f));
				FogComponent->SetStartDistance(bIsFFStarterGrassland ? 2600.0f : (UsesKashkehStarterBaseWorld(World) ? 5200.0f : 2200.0f));
				FogComponent->SetFogMaxOpacity(bIsFFStarterGrassland ? 0.02f : (UsesKashkehStarterBaseWorld(World) ? 0.005f : 0.05f));
			}
			continue;
		}

		if (APostProcessVolume* PostProcess = Cast<APostProcessVolume>(Actor))
		{
			PostProcess->Tags.AddUnique(RuntimeLightingTag);
			PostProcess->bUnbound = true;
			PostProcess->Priority = FMath::Max(PostProcess->Priority, 50.0f);
			FPostProcessSettings& Settings = PostProcess->Settings;
			Settings.bOverride_AutoExposureMinBrightness = true;
			Settings.AutoExposureMinBrightness = bIsFFStarterGrassland ? 1.0f : (UsesKashkehStarterBaseWorld(World) ? 2.4f : 0.82f);
			Settings.bOverride_AutoExposureMaxBrightness = true;
			Settings.AutoExposureMaxBrightness = bIsFFStarterGrassland ? 1.0f : (UsesKashkehStarterBaseWorld(World) ? 2.4f : 0.92f);
			Settings.bOverride_AutoExposureBias = true;
			Settings.AutoExposureBias = bIsFFStarterGrassland ? 0.0f : (UsesKashkehStarterBaseWorld(World) ? 1.1f : -0.25f);
			Settings.bOverride_ColorSaturation = true;
			Settings.ColorSaturation = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
			Settings.bOverride_BloomIntensity = true;
			Settings.BloomIntensity = bIsFFStarterGrassland ? 0.02f : (UsesKashkehStarterBaseWorld(World) ? 0.1f : Settings.BloomIntensity);
			continue;
		}
	}

	const FVector VillageCenter = UsesKashkehStarterBaseWorld(World)
		? ResolveTutorialGroundLocation(GetKashkehVillageCenterHint(), 12.0f)
		: (IsShoreLakeVillageWorld(World)
			? ResolveTutorialGroundLocation(GetShoreLakeCenterHint(), 12.0f)
		: (IsLikanaValleyWorld(World)
			? ResolveTutorialGroundLocation(GetLikanaValleyCenterHint(), 12.0f)
			: ResolveTutorialGroundLocation(GetLerigothiVillageCenterHint(), 12.0f)));
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (!bHasDirectionalLight)
	{
		if (ADirectionalLight* DirectionalLight = World->SpawnActor<ADirectionalLight>(
			VillageCenter + FVector(-2800.0f, 1800.0f, 4200.0f),
			FRotator(-36.0f, 34.0f, 0.0f),
			SpawnParameters))
		{
			DirectionalLight->Tags.Add(RuntimeLightingTag);
			if (UDirectionalLightComponent* LightComponent = Cast<UDirectionalLightComponent>(DirectionalLight->GetLightComponent()))
			{
				LightComponent->SetMobility(EComponentMobility::Movable);
				LightComponent->SetIntensity(bIsFFStarterGrassland ? 9.0f : (UsesKashkehStarterBaseWorld(World) ? 16.0f : DaylightDirectionalIntensity));
				LightComponent->SetLightColor(FLinearColor(1.0f, 0.96f, 0.90f));
				LightComponent->SetCastShadows(true);
				LightComponent->bAtmosphereSunLight = true;
				LightComponent->SetAtmosphereSunLightIndex(0);
			}
		}
	}

	if (!bHasSkyLight)
	{
		if (ASkyLight* SkyLight = World->SpawnActor<ASkyLight>(
			VillageCenter + FVector(0.0f, 0.0f, 1400.0f),
			FRotator::ZeroRotator,
			SpawnParameters))
		{
			SkyLight->Tags.Add(RuntimeLightingTag);
			if (USkyLightComponent* SkyComponent = SkyLight->GetLightComponent())
			{
				SkyComponent->SetMobility(EComponentMobility::Movable);
				SkyComponent->SetIntensity(bIsFFStarterGrassland ? 0.8f : (UsesKashkehStarterBaseWorld(World) ? 2.0f : DaylightSkylightIntensity));
				SkyComponent->SetLightColor(FLinearColor(0.97f, 0.98f, 0.99f));
				SkyComponent->SetRealTimeCapture(false);
				if (!bIsGrasslandFarm)
				{
					SkyComponent->RecaptureSky();
				}
			}
		}
	}

	if (!bHasSkyAtmosphere)
	{
		if (ASkyAtmosphere* SkyAtmosphere = World->SpawnActor<ASkyAtmosphere>(
			VillageCenter,
			FRotator::ZeroRotator,
			SpawnParameters))
		{
			SkyAtmosphere->Tags.Add(RuntimeLightingTag);
		}
	}

	if (!bHasHeightFog)
	{
		if (AExponentialHeightFog* HeightFog = World->SpawnActor<AExponentialHeightFog>(
			VillageCenter + FVector(0.0f, 0.0f, 120.0f),
			FRotator::ZeroRotator,
			SpawnParameters))
		{
			HeightFog->Tags.Add(RuntimeLightingTag);
		}
	}

	if (!bHasPostProcess)
	{
		if (APostProcessVolume* PostProcess = World->SpawnActor<APostProcessVolume>(
			VillageCenter,
			FRotator::ZeroRotator,
			SpawnParameters))
		{
			PostProcess->Tags.Add(RuntimeLightingTag);
			PostProcess->bUnbound = true;
			PostProcess->Priority = 50.0f;
			FPostProcessSettings& Settings = PostProcess->Settings;
			Settings.bOverride_AutoExposureMinBrightness = true;
			Settings.AutoExposureMinBrightness = bIsFFStarterGrassland ? 1.0f : (UsesKashkehStarterBaseWorld(World) ? 2.4f : 0.82f);
			Settings.bOverride_AutoExposureMaxBrightness = true;
			Settings.AutoExposureMaxBrightness = bIsFFStarterGrassland ? 1.0f : (UsesKashkehStarterBaseWorld(World) ? 2.4f : 0.92f);
			Settings.bOverride_AutoExposureBias = true;
			Settings.AutoExposureBias = bIsFFStarterGrassland ? 0.0f : (UsesKashkehStarterBaseWorld(World) ? 1.1f : -0.25f);
			Settings.bOverride_ColorSaturation = true;
			Settings.ColorSaturation = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
			Settings.bOverride_BloomIntensity = true;
			Settings.BloomIntensity = bIsFFStarterGrassland ? 0.02f : (UsesKashkehStarterBaseWorld(World) ? 0.1f : Settings.BloomIntensity);
		}
	}
}

void AFantasyFrontierTutorialDirector::EnsureLowerIslandSurroundingContext()
{
	if (!GetWorld())
	{
		return;
	}

	auto EnsureLevelLoaded = [this](const TSoftObjectPtr<UWorld>& LevelAsset, TObjectPtr<ULevelStreamingDynamic>& LevelRef)
	{
		if (LevelRef || LevelAsset.IsNull())
		{
			return;
		}

		bool bLoaded = false;
		LevelRef = ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(
			this,
			LevelAsset,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			bLoaded);
		if (LevelRef)
		{
			LevelRef->SetShouldBeLoaded(true);
			LevelRef->SetShouldBeVisible(true);
			LevelRef->OnLevelShown.AddUniqueDynamic(this, &ThisClass::HandleLowerIslandSurroundingLevelShown);
		}
	};

	if (UsesKashkehStarterBaseWorld(GetWorld()))
	{
		EnsureLevelLoaded(KashkehLandscapeLevelAsset, KashkehLandscapeLevel);
		EnsureLevelLoaded(KashkehRoadsLevelAsset, KashkehRoadsLevel);
		EnsureLevelLoaded(KashkehFoliageLevelAsset, KashkehFoliageLevel);
		EnsureLevelLoaded(KashkehLightingLevelAsset, KashkehLightingLevel);
	}

	if (IsGrasslandFarmWorld(GetWorld()) || IsGrasslandMolehillWorld(GetWorld()))
	{
		EnsureLevelLoaded(GrasslandFarmShellLevelAsset, GrasslandFarmShellLevel);
	}

	if (IsCelticVillageWorld(GetWorld()))
	{
		EnsureLevelLoaded(CelticVillageLandscapingLevelAsset, CelticVillageLandscapingLevel);
	}

	if (IsLerigothiWorld(GetWorld()))
	{
		// Lerigothi itself is now the Titan ground host for the active starter band. Do not stack
		// mismatched Kashkeh support levels on top of it again.
		return;
	}
}

void AFantasyFrontierTutorialDirector::HandleLowerIslandSurroundingLevelShown()
{
	if (bBuiltEnvironment || !ShouldUseStarterForestSlice() || !GetWorld())
	{
		return;
	}

	if (UsesKashkehStarterBaseWorld(GetWorld()))
	{
		if ((KashkehLandscapeLevel && !KashkehLandscapeLevel->GetLoadedLevel()) ||
			(KashkehRoadsLevel && !KashkehRoadsLevel->GetLoadedLevel()) ||
			(KashkehFoliageLevel && !KashkehFoliageLevel->GetLoadedLevel()) ||
			(KashkehLightingLevel && !KashkehLightingLevel->GetLoadedLevel()))
		{
			return;
		}
	}

	if (IsCelticVillageWorld(GetWorld()))
	{
		if (CelticVillageLandscapingLevel && !CelticVillageLandscapingLevel->GetLoadedLevel())
		{
			return;
		}
	}

	if (IsGrasslandFarmWorld(GetWorld()))
	{
		if (GrasslandFarmShellLevel && !GrasslandFarmShellLevel->GetLoadedLevel())
		{
			return;
		}
	}

	EnsureStarterForestLighting();
	bAppliedLerigothiGroundCleanup = false;
	ApplyLerigothiVillageGroundCleanup();
	SpawnRuntimeActors();
	PositionExistingPlayer();
	ScheduleStarterForestSpawnValidation();
	bBuiltEnvironment = true;
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

	if (!ShouldUseStarterForestSlice())
	{
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

		return;
	}

	const FVector HubLocation = GetStarterForestPlayerLocation();
	const float CharacterHeightOffset = 96.0f;
	const float SafeZoneRadius = 1500.0f;
	FVector VillageCenter = HubLocation;
	if (IsCelticVillageWorld(GetWorld()))
	{
		VillageCenter = ResolveTutorialGroundLocation(FVector(5200.0f, 6580.0f, -700.0f), 12.0f);
	}
	else if (IsShoreLakeVillageWorld(GetWorld()))
	{
		VillageCenter = ResolveTutorialGroundLocation(GetShoreLakeCenterHint(), 12.0f);
	}
	else if (IsGrasslandVillageWorld(GetWorld()))
	{
		VillageCenter = ResolveTutorialGroundLocation(GetGrasslandVillageCenterHint(), 12.0f);
	}
	else if (IsGrasslandMolehillWorld(GetWorld()))
	{
		VillageCenter = ResolveTutorialGroundLocation(GetGrasslandMolehillCenterHint(), 12.0f);
	}
	else if (IsGrasslandFarmWorld(GetWorld()))
	{
		VillageCenter = ResolveTutorialGroundLocation(GetGrasslandFarmCenterHint(), 12.0f);
	}
	else if (IsLikanaValleyWorld(GetWorld()))
	{
		VillageCenter = ResolveTutorialGroundLocation(GetLikanaValleyCenterHint(), 12.0f);
	}
	else if (IsLerigothiWorld(GetWorld()))
	{
		VillageCenter = ResolveTutorialGroundLocation(GetLerigothiVillageCenterHint(), 12.0f);
	}
	else if (UsesKashkehStarterBaseWorld(GetWorld()))
	{
		VillageCenter = ResolveTutorialGroundLocation(GetKashkehVillageCenterHint(), 12.0f);
	}
	const FVector VillageAnchor = (IsCelticVillageWorld(GetWorld()) || IsShoreLakeVillageWorld(GetWorld()) || IsGrasslandVillageWorld(GetWorld()) || IsGrasslandMolehillWorld(GetWorld()) || IsGrasslandFarmWorld(GetWorld()) || IsLerigothiWorld(GetWorld()) || IsKashkehWorld(GetWorld()) || IsLikanaValleyWorld(GetWorld())) ? VillageCenter : HubLocation;

	for (TActorIterator<AFantasyFrontierEnemyBase> It(GetWorld()); It; ++It)
	{
		if (IsValid(*It) && FVector::DistSquared2D(It->GetActorLocation(), VillageCenter) <= FMath::Square(SafeZoneRadius))
		{
			It->Destroy();
		}
	}

	DestroyActorsWithTag<AFantasyFrontierFunctionalNpc>(GetWorld(), GetRoleTag(EFantasyFrontierNpcRole::Guide));
	DestroyActorsWithTag<AFantasyFrontierFunctionalNpc>(GetWorld(), GetRoleTag(EFantasyFrontierNpcRole::Smith));
	DestroyActorsWithTag<AFantasyFrontierFunctionalNpc>(GetWorld(), GetRoleTag(EFantasyFrontierNpcRole::Trainer));
	DestroyActorsWithTag<AFantasyFrontierStalkerEnemy>(GetWorld(), TEXT("FFVillageEnemy"));
	DestroyActorsWithTag<AFantasyFrontierRuinMysticEnemy>(GetWorld(), TEXT("FFVillageEnemy"));
	DestroyActorsWithTag<AFantasyFrontierAmbientCreature>(GetWorld(), TEXT("FFVillageWildlife"));
	DestroyActorsWithTag<AFantasyFrontierHiddenScenarioTrigger>(GetWorld(), TEXT("FFHiddenScenario"));
	DestroyActorsWithTag<AActor>(GetWorld(), TEXT("FFVillageProp"));

	if (IsLikanaValleyWorld(GetWorld()) || IsShoreLakeVillageWorld(GetWorld()) || IsGrasslandVillageWorld(GetWorld()) || IsGrasslandMolehillWorld(GetWorld()) || IsGrasslandFarmWorld(GetWorld()) || UsesKashkehStarterBaseWorld(GetWorld()))
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	auto SpawnVillageProp = [&](UClass* ActorClass, const FVector& Offset, float Yaw, const FName Tag)
	{
		if (!ActorClass)
		{
			return;
		}

		FVector SpawnLocation = FVector::ZeroVector;
		if (!TryResolveTutorialGroundLocation(VillageAnchor + Offset, 0.0f, SpawnLocation))
		{
			SpawnLocation = ResolveTutorialGroundLocation(VillageAnchor + Offset, 0.0f);
		}

		if (AActor* Actor = GetWorld()->SpawnActor<AActor>(ActorClass, SpawnLocation, FRotator(0.0f, Yaw, 0.0f), SpawnParameters))
		{
			Actor->Tags.AddUnique(Tag);
		}
	};

	if (IsKashkehWorld(GetWorld()) || IsCelticVillageWorld(GetWorld()))
	{
		if (UStaticMesh* WellMesh = GetStarterVillageWellMesh())
		{
			const float WellYaw = IsCelticVillageWorld(GetWorld()) ? 0.0f : 12.0f;
			FVector WellLocation = FVector::ZeroVector;
			if (TryResolveTutorialGroundLocation(VillageCenter, 0.0f, WellLocation))
			{
				if (AStaticMeshActor* WellActor = GetWorld()->SpawnActor<AStaticMeshActor>(WellLocation, FRotator(0.0f, WellYaw, 0.0f), SpawnParameters))
				{
					WellActor->Tags.AddUnique(TEXT("FFVillageProp"));
					if (UStaticMeshComponent* WellComponent = WellActor->GetStaticMeshComponent())
					{
						WellComponent->SetStaticMesh(WellMesh);
						WellComponent->SetMobility(EComponentMobility::Static);
						WellComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
						WellActor->SetActorScale3D(FVector(0.76f, 0.76f, 0.76f));
					}
				}
			}
			else if (AStaticMeshActor* WellActor = GetWorld()->SpawnActor<AStaticMeshActor>(ResolveTutorialGroundLocation(VillageCenter, 0.0f), FRotator(0.0f, WellYaw, 0.0f), SpawnParameters))
			{
				WellActor->Tags.AddUnique(TEXT("FFVillageProp"));
				if (UStaticMeshComponent* WellComponent = WellActor->GetStaticMeshComponent())
				{
					WellComponent->SetStaticMesh(WellMesh);
					WellComponent->SetMobility(EComponentMobility::Static);
					WellComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					WellActor->SetActorScale3D(FVector(0.76f, 0.76f, 0.76f));
				}
			}
		}
	}

	if (IsLerigothiWorld(GetWorld()))
	{
		// Lerigothi stays ground-first until the native Titan host band is fully stable.
		// Keep only the center well here and avoid extra runtime buildings/fences that can
		// block spawn, movement, or camera validation on otherwise valid host ground.
		if (UStaticMesh* WellMesh = GetLerigothiStarterVillageWellMesh())
		{
			const FVector WellLocation = ResolveTutorialGroundLocation(VillageCenter, 0.0f);
			if (AStaticMeshActor* WellActor = GetWorld()->SpawnActor<AStaticMeshActor>(WellLocation, FRotator(0.0f, 12.0f, 0.0f), SpawnParameters))
			{
				WellActor->Tags.AddUnique(TEXT("FFVillageProp"));
				if (UStaticMeshComponent* WellComponent = WellActor->GetStaticMeshComponent())
				{
					WellComponent->SetStaticMesh(WellMesh);
					WellComponent->SetMobility(EComponentMobility::Static);
					WellComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					WellActor->SetActorScale3D(FVector(0.54f, 0.54f, 0.54f));
				}
			}
		}
	}

	if (IsKashkehWorld(GetWorld()))
	{
		// Keep the Kashkeh village shell deliberately light so the authored Titan plaza and
		// surrounding ground stay visible during smoke validation.
		SpawnVillageProp(GetKashkehStorageHutClass(), FVector(780.0f, 640.0f, 0.0f), -104.0f, TEXT("FFVillageProp"));
		SpawnVillageProp(GetKashkehFenceRowClass(), FVector(1060.0f, 860.0f, 0.0f), 102.0f, TEXT("FFVillageProp"));
		SpawnVillageProp(GetKashkehFenceRowClass(), FVector(1180.0f, -980.0f, 0.0f), 66.0f, TEXT("FFVillageProp"));
	}

	auto SpawnNpc = [&](EFantasyFrontierNpcRole InRole, const FString& DisplayName, const FVector& Offset, float Yaw)
	{
		if (!FunctionalNpcClass)
		{
			return;
		}

		FVector SpawnLocation = FVector::ZeroVector;
		if (!TryResolveTutorialGroundLocation(VillageAnchor + Offset, CharacterHeightOffset, SpawnLocation))
		{
			SpawnLocation = ResolveTutorialGroundLocation(VillageAnchor + Offset, CharacterHeightOffset);
		}

		if (AFantasyFrontierFunctionalNpc* Npc = GetWorld()->SpawnActor<AFantasyFrontierFunctionalNpc>(FunctionalNpcClass, SpawnLocation, FRotator(0.0f, Yaw, 0.0f), SpawnParameters))
		{
			Npc->ConfigureNpc(InRole, FText::FromString(DisplayName));
			Npc->Tags.AddUnique(GetRoleTag(InRole));
		}
	};

	const FVector GuideOffset = IsLerigothiWorld(GetWorld()) ? FVector(360.0f, 260.0f, 0.0f) : (IsKashkehWorld(GetWorld()) ? FVector(320.0f, 140.0f, 0.0f) : FVector(240.0f, 120.0f, 0.0f));
	const FVector SmithOffset = IsLerigothiWorld(GetWorld()) ? FVector(420.0f, -260.0f, 0.0f) : (IsKashkehWorld(GetWorld()) ? FVector(120.0f, -360.0f, 0.0f) : FVector(-260.0f, -180.0f, 0.0f));
	const FVector TrainerOffset = IsLerigothiWorld(GetWorld()) ? FVector(40.0f, 520.0f, 0.0f) : (IsKashkehWorld(GetWorld()) ? FVector(-60.0f, 420.0f, 0.0f) : FVector(40.0f, 320.0f, 0.0f));
	SpawnNpc(EFantasyFrontierNpcRole::Guide, TEXT("Liora, Frontier Guide"), GuideOffset, -150.0f);
	SpawnNpc(EFantasyFrontierNpcRole::Smith, TEXT("Brann, Smith"), SmithOffset, 36.0f);
	SpawnNpc(EFantasyFrontierNpcRole::Trainer, TEXT("Cael, Skill Trainer"), TrainerOffset, -108.0f);

	if (WildlifeClass)
	{
		FVector WildlifeLocation = FVector::ZeroVector;
		if (TryResolveTutorialGroundLocation(VillageAnchor + FVector(-620.0f, 440.0f, 0.0f), 28.0f, WildlifeLocation))
		{
			if (AFantasyFrontierAmbientCreature* Wildlife = GetWorld()->SpawnActor<AFantasyFrontierAmbientCreature>(
					WildlifeClass,
					WildlifeLocation,
					FRotator::ZeroRotator,
					SpawnParameters))
			{
				Wildlife->ConfigureArchetype(EFantasyFrontierWildlifeArchetype::MeadowGrazer);
				Wildlife->Tags.AddUnique(TEXT("FFVillageWildlife"));
			}
		}
		else if (AFantasyFrontierAmbientCreature* Wildlife = GetWorld()->SpawnActor<AFantasyFrontierAmbientCreature>(
				WildlifeClass,
				ResolveTutorialGroundLocation(VillageAnchor + FVector(-620.0f, 440.0f, 0.0f), 28.0f),
				FRotator::ZeroRotator,
				SpawnParameters))
		{
			Wildlife->ConfigureArchetype(EFantasyFrontierWildlifeArchetype::MeadowGrazer);
			Wildlife->Tags.AddUnique(TEXT("FFVillageWildlife"));
		}
	}

	if (StalkerEnemyClass)
	{
		const TArray<FVector> EnemyCandidates = IsCelticVillageWorld(GetWorld())
			? GetCelticVillageEnemyCandidates(VillageCenter)
			: (IsKashkehWorld(GetWorld())
				? GetKashkehEnemyCandidates(VillageCenter)
				: GetLerigothiEnemyCandidates(VillageCenter));
		bool bSpawnedVillageEnemy = false;
		for (const FVector& EnemyCandidate : EnemyCandidates)
		{
			FVector SpawnLocation = FVector::ZeroVector;
			if (!TryResolveTutorialGroundLocation(EnemyCandidate, CharacterHeightOffset, SpawnLocation))
			{
				continue;
			}

			if (FVector::DistSquared2D(SpawnLocation, VillageCenter) <= FMath::Square(SafeZoneRadius))
			{
				continue;
			}

			if (AFantasyFrontierStalkerEnemy* Enemy = GetWorld()->SpawnActor<AFantasyFrontierStalkerEnemy>(
				StalkerEnemyClass,
				SpawnLocation,
				( VillageCenter - SpawnLocation ).Rotation(),
				SpawnParameters))
			{
				Enemy->Tags.AddUnique(TEXT("FFVillageEnemy"));
				bSpawnedVillageEnemy = true;
				break;
			}
		}

		if (!bSpawnedVillageEnemy && IsLerigothiWorld(GetWorld()))
		{
			const TArray<FVector> FallbackEnemyCandidates = {
				VillageCenter + FVector(-2480.0f, 140.0f, 0.0f),
				VillageCenter + FVector(-2860.0f, -320.0f, 0.0f),
				VillageCenter + FVector(-3220.0f, 240.0f, 0.0f)
			};
			for (const FVector& EnemyCandidate : FallbackEnemyCandidates)
			{
				FVector SpawnLocation = FVector::ZeroVector;
				if (!TryResolveTutorialGroundLocation(EnemyCandidate, CharacterHeightOffset, SpawnLocation))
				{
					continue;
				}

				if (FVector::DistSquared2D(SpawnLocation, VillageCenter) <= FMath::Square(SafeZoneRadius))
				{
					continue;
				}

				if (AFantasyFrontierStalkerEnemy* Enemy = GetWorld()->SpawnActor<AFantasyFrontierStalkerEnemy>(
					StalkerEnemyClass,
					SpawnLocation,
					( VillageCenter - SpawnLocation ).Rotation(),
					SpawnParameters))
				{
					Enemy->Tags.AddUnique(TEXT("FFVillageEnemy"));
					break;
				}
			}
		}
	}

	if (RuinMysticEnemyClass)
	{
		FVector MysticLocation = FVector::ZeroVector;
		if (TryResolveTutorialGroundLocation(
				IsCelticVillageWorld(GetWorld())
					? VillageCenter + FVector(3140.0f, -1200.0f, 0.0f)
					: (IsKashkehWorld(GetWorld())
						? HubLocation + FVector(3600.0f, -1600.0f, 0.0f)
						: HubLocation + FVector(1820.0f, -1280.0f, 0.0f)),
				CharacterHeightOffset,
				MysticLocation) &&
			FVector::DistSquared2D(MysticLocation, VillageCenter) > FMath::Square(SafeZoneRadius))
		{
			if (AFantasyFrontierRuinMysticEnemy* Enemy = GetWorld()->SpawnActor<AFantasyFrontierRuinMysticEnemy>(
				RuinMysticEnemyClass,
				MysticLocation,
				( VillageCenter - MysticLocation ).Rotation(),
				SpawnParameters))
			{
				Enemy->Tags.AddUnique(TEXT("FFVillageEnemy"));
			}
		}
	}

	if (HiddenScenarioTriggerClass)
	{
		FVector TriggerLocation = FVector::ZeroVector;
		if (TryResolveTutorialGroundLocation(
				IsCelticVillageWorld(GetWorld())
					? VillageCenter + FVector(920.0f, 1760.0f, 0.0f)
					: (IsKashkehWorld(GetWorld())
						? HubLocation + FVector(900.0f, 1650.0f, 0.0f)
						: HubLocation + FVector(1240.0f, 780.0f, 0.0f)),
				0.0f,
				TriggerLocation))
		{
			if (AFantasyFrontierHiddenScenarioTrigger* Trigger = GetWorld()->SpawnActor<AFantasyFrontierHiddenScenarioTrigger>(
					HiddenScenarioTriggerClass,
					TriggerLocation,
					FRotator::ZeroRotator,
					SpawnParameters))
			{
				Trigger->Tags.AddUnique(TEXT("FFHiddenScenario"));
			}
		}
		else if (AFantasyFrontierHiddenScenarioTrigger* Trigger = GetWorld()->SpawnActor<AFantasyFrontierHiddenScenarioTrigger>(
				HiddenScenarioTriggerClass,
				ResolveTutorialGroundLocation(
					IsCelticVillageWorld(GetWorld())
						? VillageCenter + FVector(920.0f, 1760.0f, 0.0f)
						: (IsKashkehWorld(GetWorld())
							? HubLocation + FVector(900.0f, 1650.0f, 0.0f)
							: HubLocation + FVector(1240.0f, 780.0f, 0.0f)),
					0.0f),
				FRotator::ZeroRotator,
				SpawnParameters))
		{
			Trigger->Tags.AddUnique(TEXT("FFHiddenScenario"));
		}
	}
}

void AFantasyFrontierTutorialDirector::PositionExistingPlayer()
{
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
	if (!PlayerCharacter)
	{
		return;
	}

	const float HeightOffset = PlayerCharacter->GetCapsuleComponent()
		? PlayerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 2.0f
		: 96.0f;
	const bool bIsHighlandBlockout = IsFFStarterHighlandBlockoutWorld(GetWorld());
	FVector HighlandPlayerStartLocation = FVector::ZeroVector;
	FRotator HighlandPlayerStartRotation = FRotator::ZeroRotator;
	const bool bHasHighlandPlayerStart = bIsHighlandBlockout && TryGetHighlandBlockoutPlayerStart(this, HighlandPlayerStartLocation, HighlandPlayerStartRotation);
	const FVector SpawnLocation = bIsHighlandBlockout
		? ResolveTutorialGroundLocation(bHasHighlandPlayerStart ? HighlandPlayerStartLocation : GetFFStarterHighlandSpawnHint(), HeightOffset)
		: (ShouldUseStarterForestSlice()
			? ResolveTutorialGroundLocation(GetStarterForestPlayerLocation(), HeightOffset)
			: FVector(-180.0f, 0.0f, 130.0f));
	const FVector KashkehVillageCenter = UsesKashkehStarterBaseWorld(GetWorld())
		? ResolveTutorialGroundLocation(GetKashkehVillageCenterHint(), 12.0f)
		: FVector::ZeroVector;
	FRotator SpawnRotation = FRotator::ZeroRotator;
	if (bIsHighlandBlockout)
	{
		SpawnRotation = bHasHighlandPlayerStart
			? FRotator(0.0f, HighlandPlayerStartRotation.Yaw, 0.0f)
			: FRotator(0.0f, (GetFFStarterHighlandViewTargetHint() - SpawnLocation).Rotation().Yaw, 0.0f);
	}
	else if (ShouldUseStarterForestSlice())
	{
		if (IsCelticVillageWorld(GetWorld()))
		{
			SpawnRotation = FRotator(0.0f, -32.0f, 0.0f);
		}
		else if (IsShoreLakeVillageWorld(GetWorld()))
		{
			SpawnRotation = FRotator(0.0f, (GetShoreLakeViewTargetHint() - SpawnLocation).Rotation().Yaw, 0.0f);
		}
		else if (IsGrasslandVillageWorld(GetWorld()))
		{
			SpawnRotation = FRotator(0.0f, (GetGrasslandVillageViewTargetHint() - SpawnLocation).Rotation().Yaw, 0.0f);
		}
		else if (IsGrasslandMolehillWorld(GetWorld()))
		{
			SpawnRotation = FRotator(0.0f, (GetGrasslandMolehillViewTargetHint() - SpawnLocation).Rotation().Yaw, 0.0f);
		}
		else if (IsGrasslandFarmWorld(GetWorld()))
		{
			SpawnRotation = FRotator(0.0f, (GetGrasslandFarmViewTargetHint() - SpawnLocation).Rotation().Yaw, 0.0f);
		}
		else if (IsLikanaValleyWorld(GetWorld()))
		{
			SpawnRotation = FRotator(0.0f, (GetLikanaValleyViewTargetHint() - SpawnLocation).Rotation().Yaw, 0.0f);
		}
		else if (UsesKashkehStarterBaseWorld(GetWorld()))
		{
			SpawnRotation = FRotator(0.0f, (KashkehVillageCenter - SpawnLocation).Rotation().Yaw, 0.0f);
		}
		else if (IsLerigothiWorld(GetWorld()))
		{
			SpawnRotation = FRotator(0.0f, (GetLerigothiVillageViewTargetHint() - SpawnLocation).Rotation().Yaw, 0.0f);
		}
		else
		{
			SpawnRotation = FRotator(0.0f, -92.0f, 0.0f);
		}
	}

	PlayerCharacter->SetActorLocation(SpawnLocation, false, nullptr, ETeleportType::ResetPhysics);
	PlayerCharacter->SetActorRotation(SpawnRotation);
	if (UCharacterMovementComponent* MovementComponent = PlayerCharacter->GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->SetMovementMode(MOVE_Walking);
	}

	if (ShouldUseStarterForestSlice())
	{
		UE_LOG(LogTP_ThirdPerson, Display, TEXT("StarterForestPlayerSpawn final=(%.1f, %.1f, %.1f) map=%s"),
			PlayerCharacter->GetActorLocation().X,
			PlayerCharacter->GetActorLocation().Y,
			PlayerCharacter->GetActorLocation().Z,
			*GetWorld()->GetMapName());
	}
	else if (bIsHighlandBlockout)
	{
		UE_LOG(LogTP_ThirdPerson, Display, TEXT("StarterHighlandPlayerSpawn final=(%.1f, %.1f, %.1f) source=%s map=%s"),
			PlayerCharacter->GetActorLocation().X,
			PlayerCharacter->GetActorLocation().Y,
			PlayerCharacter->GetActorLocation().Z,
			bHasHighlandPlayerStart ? TEXT("PlayerStart") : TEXT("LegacyHint"),
			*GetWorld()->GetMapName());
	}
}

void AFantasyFrontierTutorialDirector::ApplyLerigothiVillageGroundCleanup()
{
	if (!(IsLerigothiWorld(GetWorld()) || IsGrasslandFarmWorld(GetWorld()) || UsesKashkehStarterBaseWorld(GetWorld())) || bAppliedLerigothiGroundCleanup)
	{
		return;
	}

	bAppliedLerigothiGroundCleanup = true;
	if (IsFFStarterGrasslandRegionWorld(GetWorld()))
	{
		const FVector HubLocation = GetKashkehVillageCenterHint();
		const float ActiveStarterAreaRadius = 4200.0f;
		const float OpenSpawnClearRadius = 2400.0f;
		int32 UpdatedLandscapeProxies = 0;
		int32 SwappedGroundMaterials = 0;
		int32 RemovedFoliageInstances = 0;
		int32 HiddenGrassMeshComponents = 0;

		for (TActorIterator<AActor> It(GetWorld()); It; ++It)
		{
			AActor* Actor = *It;
			if (!IsValid(Actor))
			{
				continue;
			}

			if (FVector::DistSquared2D(Actor->GetActorLocation(), HubLocation) > FMath::Square(ActiveStarterAreaRadius))
			{
				continue;
			}

			TArray<UStaticMeshComponent*> MeshComponents;
			Actor->GetComponents<UStaticMeshComponent>(MeshComponents);
			for (UStaticMeshComponent* MeshComponent : MeshComponents)
			{
				if (!IsValid(MeshComponent))
				{
					continue;
				}

				const FString MeshName = GetNameSafe(MeshComponent->GetStaticMesh()).ToLower();
				const bool bNearSpawnOpenGround = FVector::DistSquared2D(Actor->GetActorLocation(), HubLocation) <= FMath::Square(OpenSpawnClearRadius);
				if (MeshName.Contains(TEXT("grassblade")) &&
					bNearSpawnOpenGround)
				{
					MeshComponent->SetVisibility(false, true);
					MeshComponent->SetHiddenInGame(true, true);
					MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
					++HiddenGrassMeshComponents;
					continue;
				}

				for (int32 MaterialIndex = 0; MaterialIndex < MeshComponent->GetNumMaterials(); ++MaterialIndex)
				{
					UMaterialInterface* CurrentMaterial = MeshComponent->GetMaterial(MaterialIndex);
					if (!CurrentMaterial)
					{
						continue;
					}
				}
			}

			TInlineComponentArray<UInstancedStaticMeshComponent*> InstancedComponents;
			Actor->GetComponents(InstancedComponents);
			for (UInstancedStaticMeshComponent* InstancedComponent : InstancedComponents)
			{
				if (!IsValid(InstancedComponent))
				{
					continue;
				}

				const FString ComponentName = GetNameSafe(InstancedComponent).ToLower();
				const FString MeshName = GetNameSafe(InstancedComponent->GetStaticMesh()).ToLower();
				if (!NameContainsAny(ComponentName, { TEXT("grass"), TEXT("clover"), TEXT("weed"), TEXT("plant") }) &&
					!NameContainsAny(MeshName, { TEXT("grass"), TEXT("clover"), TEXT("weed"), TEXT("plant") }))
				{
					continue;
				}

				for (int32 InstanceIndex = InstancedComponent->GetInstanceCount() - 1; InstanceIndex >= 0; --InstanceIndex)
				{
					FTransform InstanceTransform;
					if (!InstancedComponent->GetInstanceTransform(InstanceIndex, InstanceTransform, true))
					{
						continue;
					}

					if (FVector::DistSquared2D(InstanceTransform.GetLocation(), HubLocation) <= FMath::Square(OpenSpawnClearRadius))
					{
						InstancedComponent->RemoveInstance(InstanceIndex);
						++RemovedFoliageInstances;
					}
				}
			}
		}

		UE_LOG(LogTP_ThirdPerson, Display, TEXT("FFStarterGrasslandRegionCleanup updatedLandscapeProxies=%d swappedGroundMaterials=%d hiddenGrassMeshComponents=%d removedFoliageInstances=%d"), UpdatedLandscapeProxies, SwappedGroundMaterials, HiddenGrassMeshComponents, RemovedFoliageInstances);
		return;
	}

	if (UsesKashkehStarterBaseWorld(GetWorld()))
	{
		const FVector HubLocation = ResolveTutorialGroundLocation(GetKashkehVillageCenterHint(), 12.0f);
		const float ActiveStarterAreaRadius = 3200.0f;
		const float OpenSpawnClearRadius = 1400.0f;
		UMaterialInterface* StableGrassMaterial = LoadObject<UMaterialInterface>(
			nullptr,
			TEXT("/Game/Environment/Grassland/Kashkeh/Landscape/MI_Grassland_Clover.MI_Grassland_Clover"));
		UMaterialInterface* StableGrassBladeMaterial = LoadObject<UMaterialInterface>(
			nullptr,
			TEXT("/Game/Environment/Foliage/Materials/MI_GrassBladeDarker.MI_GrassBladeDarker"));
		UMaterialInterface* StableDirtMaterial = LoadObject<UMaterialInterface>(
			nullptr,
			TEXT("/Game/Environment/Grassland/Kashkeh/Landscape/MI_Grassland_Dirt_NoGrassBlend.MI_Grassland_Dirt_NoGrassBlend"));
		UMaterialInterface* StableRoadMaterial = LoadObject<UMaterialInterface>(
			nullptr,
			TEXT("/Game/Environment/Grassland/Meshes/Roads/MI_GrassRoad_3.MI_GrassRoad_3"));
		int32 HiddenActorCount = 0;
		int32 SwappedMaterialCount = 0;
		int32 RemovedFoliageInstances = 0;
		int32 SwappedFoliageMaterials = 0;

		for (TActorIterator<AActor> It(GetWorld()); It; ++It)
		{
			AActor* Actor = *It;
			if (!IsValid(Actor))
			{
				continue;
			}

			if (FVector::DistSquared2D(Actor->GetActorLocation(), HubLocation) > FMath::Square(ActiveStarterAreaRadius))
			{
				continue;
			}

			bool bHideActor = false;
			TArray<UStaticMeshComponent*> MeshComponents;
			Actor->GetComponents<UStaticMeshComponent>(MeshComponents);
			for (UStaticMeshComponent* MeshComponent : MeshComponents)
			{
				if (!IsValid(MeshComponent))
				{
					continue;
				}

				const FString MeshName = GetNameSafe(MeshComponent->GetStaticMesh());
				for (int32 MaterialIndex = 0; MaterialIndex < MeshComponent->GetNumMaterials(); ++MaterialIndex)
				{
					UMaterialInterface* CurrentMaterial = MeshComponent->GetMaterial(MaterialIndex);
					if (!CurrentMaterial)
					{
						continue;
					}

					const FString MaterialName = CurrentMaterial->GetName();
					const FString MaterialPath = CurrentMaterial->GetPathName();
					const bool bBrokenGrassBlend = MaterialName.Contains(TEXT("MI_Grassland_rRVT1")) ||
						MaterialPath.Contains(TEXT("/Environment/Grassland/Kashkeh/Landscape/MI_Grassland_rRVT1"));
					const bool bBrokenRoadBlend = MaterialName.Contains(TEXT("M_DirtRoad")) ||
						MaterialName.Contains(TEXT("MI_Road_DIrt_Inst")) ||
						MaterialName.Contains(TEXT("MI_GrassRoad_3"));
					const bool bLanternFallback = MaterialName.Contains(TEXT("M_FireflyLantern_Light")) ||
						MaterialPath.Contains(TEXT("/Environment/_Global/Materials/M_FireflyLantern_Light"));
					if (bBrokenGrassBlend && StableGrassMaterial)
					{
						MeshComponent->SetMaterial(MaterialIndex, StableGrassMaterial);
						++SwappedMaterialCount;
						continue;
					}

					if (bBrokenRoadBlend && (MeshName.Contains(TEXT("Road")) || MeshName.Contains(TEXT("Path")) || MeshName.Contains(TEXT("Dirt"))) && StableRoadMaterial)
					{
						MeshComponent->SetMaterial(MaterialIndex, StableRoadMaterial);
						++SwappedMaterialCount;
						continue;
					}

					if (bLanternFallback)
					{
						bHideActor = true;
						break;
					}
				}

				if (bHideActor)
				{
					break;
				}
			}

			if (bHideActor)
			{
				++HiddenActorCount;
				Actor->SetActorHiddenInGame(true);
				Actor->SetActorEnableCollision(false);
				for (UStaticMeshComponent* MeshComponent : MeshComponents)
				{
					if (IsValid(MeshComponent))
					{
						MeshComponent->SetVisibility(false, true);
						MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
					}
				}
			}

			TInlineComponentArray<UInstancedStaticMeshComponent*> InstancedComponents;
			Actor->GetComponents(InstancedComponents);
			for (UInstancedStaticMeshComponent* InstancedComponent : InstancedComponents)
			{
				if (!IsValid(InstancedComponent))
				{
					continue;
				}

				const FString ComponentName = GetNameSafe(InstancedComponent).ToLower();
				const FString MeshName = GetNameSafe(InstancedComponent->GetStaticMesh()).ToLower();
				FString MaterialNames;
				for (UMaterialInterface* Material : InstancedComponent->GetMaterials())
				{
					if (IsValid(Material))
					{
						MaterialNames += Material->GetName().ToLower();
						MaterialNames += TEXT(" ");
					}
				}

				const bool bLooksLikeStarterFoliage =
					NameContainsAny(ComponentName, { TEXT("grass"), TEXT("clover"), TEXT("weed"), TEXT("bush"), TEXT("shrub"), TEXT("plant"), TEXT("reed") }) ||
					NameContainsAny(MeshName, { TEXT("grass"), TEXT("clover"), TEXT("weed"), TEXT("bush"), TEXT("shrub"), TEXT("plant"), TEXT("reed") }) ||
					NameContainsAny(MaterialNames, { TEXT("grass"), TEXT("clover"), TEXT("weed"), TEXT("bush"), TEXT("shrub"), TEXT("plant"), TEXT("reed") });
				if (!bLooksLikeStarterFoliage)
				{
					continue;
				}

				if (StableGrassBladeMaterial)
				{
					for (int32 MaterialIndex = 0; MaterialIndex < InstancedComponent->GetNumMaterials(); ++MaterialIndex)
					{
						InstancedComponent->SetMaterial(MaterialIndex, StableGrassBladeMaterial);
						++SwappedFoliageMaterials;
					}
				}

				for (int32 InstanceIndex = InstancedComponent->GetInstanceCount() - 1; InstanceIndex >= 0; --InstanceIndex)
				{
					FTransform InstanceTransform;
					if (!InstancedComponent->GetInstanceTransform(InstanceIndex, InstanceTransform, true))
					{
						continue;
					}

					if (FVector::DistSquared2D(InstanceTransform.GetLocation(), HubLocation) <= FMath::Square(OpenSpawnClearRadius))
					{
						InstancedComponent->RemoveInstance(InstanceIndex);
						++RemovedFoliageInstances;
					}
				}
			}
		}

		UE_LOG(LogTP_ThirdPerson, Display, TEXT("KashkehStarterCleanup hiddenActors=%d swappedMaterials=%d swappedFoliageMaterials=%d removedFoliageInstances=%d"), HiddenActorCount, SwappedMaterialCount, SwappedFoliageMaterials, RemovedFoliageInstances);
		return;
	}

	if (IsGrasslandFarmWorld(GetWorld()))
	{
		const FVector HubLocation = ResolveTutorialGroundLocation(GetGrasslandFarmCenterHint(), 12.0f);
		const float ActiveStarterAreaRadius = 9500.0f;
		UMaterialInterface* StableGrassBladeMaterial = LoadObject<UMaterialInterface>(
			nullptr,
			TEXT("/Game/Environment/Foliage/Materials/MI_GrassBlade.MI_GrassBlade"));
		UMaterialInterface* StableCliffMaterial = LoadObject<UMaterialInterface>(
			nullptr,
			TEXT("/Game/Environment/Grassland/Materials/MI_Cliffside_Grass.MI_Cliffside_Grass"));
		int32 HiddenActorCount = 0;
		int32 SwappedGrassBladeCount = 0;
		int32 SwappedCliffMaterialCount = 0;

		for (TActorIterator<AActor> It(GetWorld()); It; ++It)
		{
			AActor* Actor = *It;
			if (!IsValid(Actor))
			{
				continue;
			}

			const float DistanceToHubSq = FVector::DistSquared2D(Actor->GetActorLocation(), HubLocation);
			if (DistanceToHubSq > FMath::Square(ActiveStarterAreaRadius))
			{
				continue;
			}

			bool bHideActor = false;
			TArray<UStaticMeshComponent*> MeshComponents;
			Actor->GetComponents<UStaticMeshComponent>(MeshComponents);
			for (UStaticMeshComponent* MeshComponent : MeshComponents)
			{
				if (!IsValid(MeshComponent))
				{
					continue;
				}

				for (int32 MaterialIndex = 0; MaterialIndex < MeshComponent->GetNumMaterials(); ++MaterialIndex)
				{
					UMaterialInterface* CurrentMaterial = MeshComponent->GetMaterial(MaterialIndex);
					if (!CurrentMaterial)
					{
						continue;
					}

					const FString MaterialName = CurrentMaterial->GetName();
					const FString MaterialPath = CurrentMaterial->GetPathName();
					const bool bBrokenGrassBlade = MaterialName.Contains(TEXT("MI_GrassBlade1")) ||
						MaterialName.Contains(TEXT("MI_GrassBladeDarker")) ||
						MaterialPath.Contains(TEXT("/Environment/Foliage/Materials/MI_GrassBlade1")) ||
						MaterialPath.Contains(TEXT("/Environment/Foliage/Materials/MI_GrassBladeDarker"));
					const bool bBrokenCliffBlend = MaterialName.Contains(TEXT("MI_Cliffside_Marshlands")) ||
						MaterialPath.Contains(TEXT("/Environment/Grassland/Materials/MI_Cliffside_Marshlands"));
					const bool bBrokenCardMaterial = MaterialName.Contains(TEXT("M_FX_Moving_HayCards")) ||
						MaterialName.Contains(TEXT("M_Marshlands_Decal_Master")) ||
						MaterialName.Contains(TEXT("M_ChimneySmoke_03")) ||
						MaterialName.Contains(TEXT("MI_DesertCards_Back")) ||
						MaterialPath.Contains(TEXT("/Environment/_Global/Materials/M_FX_Moving_HayCards")) ||
						MaterialPath.Contains(TEXT("/Environment/Marshland/Materials/Growth/DecalMaterials/")) ||
						MaterialPath.Contains(TEXT("/Environment/Sulfur/")) ||
						MaterialPath.Contains(TEXT("/Environment/Desert/"));

					if (bBrokenGrassBlade && StableGrassBladeMaterial)
					{
						MeshComponent->SetMaterial(MaterialIndex, StableGrassBladeMaterial);
						++SwappedGrassBladeCount;
						continue;
					}

					if (bBrokenCliffBlend && StableCliffMaterial)
					{
						MeshComponent->SetMaterial(MaterialIndex, StableCliffMaterial);
						++SwappedCliffMaterialCount;
						continue;
					}

					if (bBrokenCardMaterial)
					{
						bHideActor = true;
						break;
					}
				}

				if (bHideActor)
				{
					break;
				}
			}

			if (bHideActor)
			{
				++HiddenActorCount;
				Actor->SetActorHiddenInGame(true);
				Actor->SetActorEnableCollision(false);
				for (UStaticMeshComponent* MeshComponent : MeshComponents)
				{
					if (IsValid(MeshComponent))
					{
						MeshComponent->SetVisibility(false, true);
						MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
					}
				}
			}
		}

		UE_LOG(
			LogTP_ThirdPerson,
			Display,
			TEXT("GrasslandFarmGroundCleanup hiddenActors=%d swappedGrassBlades=%d swappedCliffs=%d"),
			HiddenActorCount,
			SwappedGrassBladeCount,
			SwappedCliffMaterialCount);
		return;
	}

	const FVector HubLocation = ResolveTutorialGroundLocation(GetLerigothiVillageCenterHint(), 12.0f);
	const FVector SpawnOpenLocation = ResolveTutorialGroundLocation(GetLerigothiVillageSpawnHint(), 12.0f);
	const float SafeZoneRadius = 1500.0f;
	const float ActiveStarterAreaRadius = 8000.0f;
	UMaterialInterface* StableGrassMaterial = LoadObject<UMaterialInterface>(
		nullptr,
		TEXT("/Game/Environment/Grassland/Kashkeh/Landscape/MI_Grassland_Clover_01.MI_Grassland_Clover_01"));
	UMaterialInterface* StableDirtMaterial = LoadObject<UMaterialInterface>(
		nullptr,
		TEXT("/Game/Environment/Grassland/Kashkeh/Landscape/MI_Grassland_Dirt.MI_Grassland_Dirt"));
	UMaterialInterface* StableRoadMaterial = LoadObject<UMaterialInterface>(
		nullptr,
		TEXT("/Game/Environment/Grassland/Meshes/Roads/MI_GrassRoad_3.MI_GrassRoad_3"));
	UMaterialInterface* StableHayMaterial = LoadObject<UMaterialInterface>(
		nullptr,
		TEXT("/Game/Environment/_Global/Materials/MI_Grassland_Hay_Trimsheet.MI_Grassland_Hay_Trimsheet"));
	UMaterialInterface* StableCliffMaterial = LoadObject<UMaterialInterface>(
		nullptr,
		TEXT("/Game/Environment/Grassland/Kashkeh/Landscape/MI_Grassland_CliffrockwithSandBlend.MI_Grassland_CliffrockwithSandBlend"));
	int32 HiddenGroundOverlayCount = 0;
	int32 HiddenActorCount = 0;
	int32 SwappedGrassMaterialCount = 0;
	int32 SwappedDirtMaterialCount = 0;
	int32 SwappedRoadMaterialCount = 0;
	int32 SwappedHayMaterialCount = 0;
	int32 SwappedRockMaterialCount = 0;

	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		AActor* Actor = *It;
		if (!IsValid(Actor))
		{
			continue;
		}

		const float DistanceToHubSq = FVector::DistSquared2D(Actor->GetActorLocation(), HubLocation);
		const float DistanceToSpawnSq = FVector::DistSquared2D(Actor->GetActorLocation(), SpawnOpenLocation);
	const bool bInsideVillageSafeZone = DistanceToHubSq <= FMath::Square(SafeZoneRadius);
	const bool bInsideActiveStarterArea = DistanceToHubSq <= FMath::Square(ActiveStarterAreaRadius);
	const bool bNearSpawnOpenZone = DistanceToSpawnSq <= FMath::Square(1100.0f);
	const bool bNearSpawnViewZone = DistanceToSpawnSq <= FMath::Square(2200.0f);
	const bool bNearSpawnGroundZone = DistanceToSpawnSq <= FMath::Square(2600.0f);
	const bool bNearSpawnStructureZone = DistanceToSpawnSq <= FMath::Square(1250.0f);
		if (!bInsideActiveStarterArea)
		{
			continue;
		}

		const FString ActorName = Actor->GetName();
		bool bHideActor = false;
		TArray<UStaticMeshComponent*> MeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(MeshComponents);
		for (UStaticMeshComponent* MeshComponent : MeshComponents)
		{
			if (!IsValid(MeshComponent))
			{
				continue;
			}

			const FString MeshName = GetNameSafe(MeshComponent->GetStaticMesh());
			if (bInsideVillageSafeZone && (ActorName.Contains(TEXT("Wheat")) || MeshName.Contains(TEXT("Wheat")) || ActorName.Contains(TEXT("Carrot")) || MeshName.Contains(TEXT("Carrot"))))
			{
				Actor->SetActorHiddenInGame(true);
				Actor->SetActorEnableCollision(false);
				MeshComponent->SetVisibility(false, true);
				MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				continue;
			}

			const bool bNearVillagePlaza = DistanceToHubSq <= FMath::Square(2600.0f);
			const bool bInsideVillageCore = DistanceToHubSq <= FMath::Square(2300.0f);
			const bool bBrokenVillageCoreStructure = bInsideVillageCore && (
				ActorName.Contains(TEXT("House")) ||
				MeshName.Contains(TEXT("House")) ||
				MeshName.Contains(TEXT("SM_GL_LerigothiKit_")) ||
				MeshName.Contains(TEXT("LerigothiKit_House")) ||
				MeshName.Contains(TEXT("flagpost")) ||
				ActorName.Contains(TEXT("roof")) ||
				MeshName.Contains(TEXT("roof")) ||
				ActorName.Contains(TEXT("hut")) ||
				MeshName.Contains(TEXT("hut")) ||
				ActorName.Contains(TEXT("storage")) ||
				MeshName.Contains(TEXT("storage")) ||
				ActorName.Contains(TEXT("platform")) ||
				MeshName.Contains(TEXT("platform")) ||
				MeshName.Contains(TEXT("HousePlatform")) ||
				MeshName.Contains(TEXT("stairs")) ||
				ActorName.Contains(TEXT("flag")) ||
				ActorName.Contains(TEXT("Rope")) ||
				MeshName.Contains(TEXT("Rope")) ||
				ActorName.Contains(TEXT("Pole")) ||
				MeshName.Contains(TEXT("Pole")) ||
				ActorName.Contains(TEXT("round_log")) ||
				MeshName.Contains(TEXT("round_log")));
			if (bBrokenVillageCoreStructure)
			{
				Actor->SetActorHiddenInGame(true);
				Actor->SetActorEnableCollision(false);
				MeshComponent->SetVisibility(false, true);
				MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				continue;
			}
			const bool bWrongBiomeVillageClutter = ActorName.Contains(TEXT("Sulfur")) ||
				MeshName.Contains(TEXT("Sulfur")) ||
				ActorName.Contains(TEXT("Path_LogDirt")) ||
				ActorName.Contains(TEXT("Desert")) ||
				MeshName.Contains(TEXT("Desert")) ||
				MeshName.Contains(TEXT("Nomad_Hearth")) ||
				MeshName.Contains(TEXT("ClayVase")) ||
				ActorName.Contains(TEXT("Nomad")) ||
				MeshName.Contains(TEXT("Nomad")) ||
				ActorName.Contains(TEXT("Hearth")) ||
				MeshName.Contains(TEXT("Hearth")) ||
				ActorName.Contains(TEXT("stairs")) ||
				MeshName.Contains(TEXT("stairs")) ||
				ActorName.Contains(TEXT("Grapple")) ||
				MeshName.Contains(TEXT("Grapple")) ||
				ActorName.Contains(TEXT("Fence")) ||
				MeshName.Contains(TEXT("Fence"));
			if (bNearVillagePlaza && bWrongBiomeVillageClutter)
			{
				Actor->SetActorHiddenInGame(true);
				Actor->SetActorEnableCollision(false);
				MeshComponent->SetVisibility(false, true);
				MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				continue;
			}

			const bool bBrokenVillagePlatform = MeshName.Contains(TEXT("HousePlatform")) ||
				MeshName.Contains(TEXT("extPlatform")) ||
				MeshName.Contains(TEXT("flagpost")) ||
				MeshName.Contains(TEXT("Pole")) ||
				MeshName.Contains(TEXT("Rope")) ||
				ActorName.Contains(TEXT("Rope")) ||
				MeshName.Contains(TEXT("round_log"));
			if (bNearVillagePlaza && bBrokenVillagePlatform)
			{
				Actor->SetActorHiddenInGame(true);
				Actor->SetActorEnableCollision(false);
				MeshComponent->SetVisibility(false, true);
				MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				continue;
			}

			const bool bSpawnOpenZoneClutter = bNearSpawnOpenZone && (
				MeshName.Contains(TEXT("flagpost")) ||
				ActorName.Contains(TEXT("flag")) ||
				MeshName.Contains(TEXT("Pole")) ||
				ActorName.Contains(TEXT("Pole")) ||
				MeshName.Contains(TEXT("Rope")) ||
				ActorName.Contains(TEXT("Rope")) ||
				MeshName.Contains(TEXT("Table")) ||
				MeshName.Contains(TEXT("Storage_Bucket")) ||
				MeshName.Contains(TEXT("Pitcher")) ||
				MeshName.Contains(TEXT("Mug")) ||
				MeshName.Contains(TEXT("Pottery")) ||
				ActorName.Contains(TEXT("Pottery")));
			if (bSpawnOpenZoneClutter)
			{
				Actor->SetActorHiddenInGame(true);
				Actor->SetActorEnableCollision(false);
				MeshComponent->SetVisibility(false, true);
				MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				continue;
			}

			const bool bSpawnViewOccluder = bNearSpawnViewZone && (
				MeshName.Contains(TEXT("Large_Rock_05")) ||
				MeshName.Contains(TEXT("SM_Bush_01b")) ||
				MeshName.Contains(TEXT("flagpost")) ||
				ActorName.Contains(TEXT("flag")) ||
				MeshName.Contains(TEXT("Pole")) ||
				ActorName.Contains(TEXT("Pole")) ||
				MeshName.Contains(TEXT("Rope")) ||
				ActorName.Contains(TEXT("Rope")));
			if (bSpawnViewOccluder)
			{
				Actor->SetActorHiddenInGame(true);
				Actor->SetActorEnableCollision(false);
				MeshComponent->SetVisibility(false, true);
				MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				continue;
			}

			const bool bSpawnViewBrokenVillageShell = bNearSpawnStructureZone && (
				MeshName.Contains(TEXT("SM_GL_LerigothiKit_HousePlatform")) ||
				MeshName.Contains(TEXT("SM_GL_LerigothiKit_Wall_")) ||
				MeshName.Contains(TEXT("SM_GL_LerigothiKit_extPlatform")) ||
				MeshName.Contains(TEXT("Window2")) ||
				MeshName.Contains(TEXT("roof")) ||
				ActorName.Contains(TEXT("House")) ||
				ActorName.Contains(TEXT("hut")));
			if (bSpawnViewBrokenVillageShell)
			{
				Actor->SetActorHiddenInGame(true);
				Actor->SetActorEnableCollision(false);
				MeshComponent->SetVisibility(false, true);
				MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				continue;
			}

			const bool bSpawnViewBrokenGrassOverlay = bNearSpawnGroundZone && ActorName.Contains(TEXT("BP_GrassCirclewTuffs"));
			if (bSpawnViewBrokenGrassOverlay)
			{
				Actor->SetActorHiddenInGame(true);
				Actor->SetActorEnableCollision(false);
				MeshComponent->SetVisibility(false, true);
				MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				continue;
			}

			for (int32 MaterialIndex = 0; MaterialIndex < MeshComponent->GetNumMaterials(); ++MaterialIndex)
			{
				UMaterialInterface* CurrentMaterial = MeshComponent->GetMaterial(MaterialIndex);
				if (!CurrentMaterial)
				{
					continue;
				}

				const FString MaterialName = CurrentMaterial->GetName();
				const FString MaterialPath = CurrentMaterial->GetPathName();
				const bool bStarterGroundMesh = MeshName.Contains(TEXT("DirtCircle")) || MeshName.Contains(TEXT("RaisedGround"));
				const bool bStarterRoadMesh = ActorName.Contains(TEXT("Path_LogDirt")) || MeshName.Contains(TEXT("Path_LogDirt")) || MeshName.Contains(TEXT("Road"));
				const bool bBrokenGrassBlend = MaterialName.Contains(TEXT("MI_Grassland_rRVT1")) ||
					MaterialPath.Contains(TEXT("/Environment/Grassland/Kashkeh/Landscape/MI_Grassland_rRVT1"));
				const bool bBrokenDirtBlend = MaterialName.Contains(TEXT("MI_Grassland_Dirt_NoGrassBlend")) ||
					MaterialPath.Contains(TEXT("/Environment/Grassland/Kashkeh/Landscape/MI_Grassland_Dirt_NoGrassBlend"));
				const bool bBrokenRoadBlend = MaterialName.Contains(TEXT("MI_Road_DIrt_Inst")) ||
					MaterialName.Contains(TEXT("MI_GrassRoad_3")) ||
					MaterialPath.Contains(TEXT("/Environment/Grassland/Meshes/Roads/MI_Road_DIrt_Inst")) ||
					MaterialPath.Contains(TEXT("/Environment/Grassland/Meshes/Roads/MI_GrassRoad_3"));
				const bool bBrokenHayCards = MaterialName.Contains(TEXT("M_FX_Moving_HayCards")) || MaterialPath.Contains(TEXT("/Environment/_Global/Materials/M_FX_Moving_HayCards"));
				const bool bBrokenMarshGrowth = MaterialName.Contains(TEXT("Marshlands_Growth")) ||
					MaterialName.Contains(TEXT("M_Marshlands_Decal_Master")) ||
					MaterialPath.Contains(TEXT("/Environment/Marshland/Materials/Growth/DecalMaterials/"));
				const bool bBrokenChimneyOrDesertCards = MaterialName.Contains(TEXT("M_ChimneySmoke_03")) ||
					MaterialName.Contains(TEXT("MI_DesertCards_Back")) ||
					MaterialPath.Contains(TEXT("/Environment/Sulfur/")) ||
					MaterialPath.Contains(TEXT("/Environment/Desert/"));
				const bool bBrokenVillageWoodShell = bInsideVillageCore &&
					MaterialPath.Contains(TEXT("/Environment/Grassland/Meshes/Lerigothi_Arhitecture/Unique_Materials/MI_woodparts_lerigothi1"));
				const bool bBrokenRockFallback = MaterialName.Contains(TEXT("MI_Rock_Blended")) ||
					MaterialPath.Contains(TEXT("MI_Rock_Blended"));
				if (bStarterGroundMesh && bBrokenGrassBlend && StableGrassMaterial)
				{
					MeshComponent->SetMaterial(MaterialIndex, StableGrassMaterial);
					++SwappedGrassMaterialCount;
					continue;
				}

				if (bStarterGroundMesh && bBrokenDirtBlend && StableDirtMaterial)
				{
					MeshComponent->SetMaterial(MaterialIndex, StableDirtMaterial);
					++SwappedDirtMaterialCount;
					continue;
				}

				if ((bStarterGroundMesh || bStarterRoadMesh) && bBrokenRoadBlend && StableRoadMaterial)
				{
					MeshComponent->SetMaterial(MaterialIndex, StableRoadMaterial);
					++SwappedRoadMaterialCount;
					continue;
				}

				if (bNearSpawnViewZone && bBrokenRockFallback && StableCliffMaterial)
				{
					MeshComponent->SetMaterial(MaterialIndex, StableCliffMaterial);
					++SwappedRockMaterialCount;
					continue;
				}

				if (bBrokenHayCards && StableHayMaterial)
				{
					MeshComponent->SetMaterial(MaterialIndex, StableHayMaterial);
					++SwappedHayMaterialCount;
				}

				if (bBrokenMarshGrowth || bBrokenChimneyOrDesertCards || bBrokenRockFallback || bBrokenVillageWoodShell)
				{
					bHideActor = true;
					break;
				}
			}
		}

		TArray<UDecalComponent*> DecalComponents;
		Actor->GetComponents<UDecalComponent>(DecalComponents);
		for (UDecalComponent* DecalComponent : DecalComponents)
		{
			if (!IsValid(DecalComponent))
			{
				continue;
			}

			UMaterialInterface* DecalMaterial = DecalComponent->GetDecalMaterial();
			if (!DecalMaterial)
			{
				continue;
			}

			const FString MaterialName = DecalMaterial->GetName();
			const FString MaterialPath = DecalMaterial->GetPathName();
			const bool bBrokenMarshGrowth = MaterialName.Contains(TEXT("Marshlands_Growth")) ||
				MaterialName.Contains(TEXT("M_Marshlands_Decal_Master")) ||
				MaterialPath.Contains(TEXT("/Environment/Marshland/Materials/Growth/DecalMaterials/"));
			if (bBrokenMarshGrowth)
			{
				bHideActor = true;
				break;
			}
		}

		if (bHideActor)
		{
			++HiddenActorCount;
			Actor->SetActorHiddenInGame(true);
			Actor->SetActorEnableCollision(false);
			for (UStaticMeshComponent* MeshComponent : MeshComponents)
			{
				if (IsValid(MeshComponent))
				{
					MeshComponent->SetVisibility(false, true);
					MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				}
			}
			for (UDecalComponent* DecalComponent : DecalComponents)
			{
				if (IsValid(DecalComponent))
				{
					DecalComponent->SetVisibility(false, true);
				}
			}
		}
	}

	UE_LOG(
		LogTP_ThirdPerson,
		Display,
		TEXT("LerigothiGroundCleanup hiddenGround=%d hiddenActors=%d swappedGrass=%d swappedDirt=%d swappedRoad=%d swappedHay=%d"),
		HiddenGroundOverlayCount,
		HiddenActorCount,
		SwappedGrassMaterialCount,
		SwappedDirtMaterialCount,
		SwappedRoadMaterialCount,
		SwappedHayMaterialCount);
}

void AFantasyFrontierTutorialDirector::ScheduleStarterForestSpawnValidation()
{
	if (!ShouldUseStarterForestSlice() || !GetWorld())
	{
		return;
	}

	const bool bIsLerigothi = IsLerigothiWorld(GetWorld());
	const bool bIsGrasslandFarm = IsGrasslandFarmWorld(GetWorld());
	const bool bIsSmokeTest = FParse::Param(FCommandLine::Get(), TEXT("FFSmokeTest"));
	const float ValidationInterval = bIsLerigothi ? 0.65f : 0.45f;
	const float InitialDelay = bIsLerigothi ? 1.25f : 0.45f;

	StarterForestSpawnValidationPassesRemaining = bIsLerigothi ? 6 : 10;
	GetWorldTimerManager().ClearTimer(StarterForestSpawnValidationTimer);
	GetWorldTimerManager().SetTimer(
		StarterForestSpawnValidationTimer,
		this,
		&ThisClass::ValidateStarterForestSpawn,
		ValidationInterval,
		true,
		InitialDelay);
}

void AFantasyFrontierTutorialDirector::ValidateStarterForestSpawn()
{
	if (!GetWorld() || StarterForestSpawnValidationPassesRemaining <= 0)
	{
		GetWorldTimerManager().ClearTimer(StarterForestSpawnValidationTimer);
		return;
	}

	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
	if (!PlayerCharacter)
	{
		return;
	}

	--StarterForestSpawnValidationPassesRemaining;

	const bool bIsLerigothi = IsLerigothiWorld(GetWorld());
	const bool bIsGrasslandFarm = IsGrasslandFarmWorld(GetWorld());
	if (bIsLerigothi || bIsGrasslandFarm)
	{
		// Re-run the narrow active-area cleanup during validation so late-loaded clutter
		// or bad material overrides do not reappear in front of the active spawn.
		bAppliedLerigothiGroundCleanup = false;
		ApplyLerigothiVillageGroundCleanup();
	}

	const float HeightOffset = PlayerCharacter->GetCapsuleComponent()
		? PlayerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 2.0f
		: 96.0f;
	const FVector HubLocation = ResolveTutorialGroundLocation(GetStarterForestPlayerLocation(), HeightOffset);
	const FVector CurrentLocation = PlayerCharacter->GetActorLocation();
	const FVector GroundProbe = ResolveTutorialGroundLocation(CurrentLocation, HeightOffset);

	const bool bMissingGround = FMath::Abs(GroundProbe.Z - CurrentLocation.Z) > 420.0f;
	const bool bTooFarFromHub = FVector::DistSquared2D(CurrentLocation, HubLocation) > FMath::Square(2200.0f);
	const bool bBelowExpectedBand = CurrentLocation.Z < HubLocation.Z - 700.0f;
	const bool bNeedsRecovery = bMissingGround || bTooFarFromHub || bBelowExpectedBand;
	const FVector KashkehVillageCenter = UsesKashkehStarterBaseWorld(GetWorld())
		? ResolveTutorialGroundLocation(GetKashkehVillageCenterHint(), 12.0f)
		: FVector::ZeroVector;
	const FRotator RecoveryRotation = IsCelticVillageWorld(GetWorld())
		? FRotator(0.0f, -32.0f, 0.0f)
		: (IsShoreLakeVillageWorld(GetWorld())
			? FRotator(0.0f, (GetShoreLakeViewTargetHint() - HubLocation).Rotation().Yaw, 0.0f)
		: (IsGrasslandVillageWorld(GetWorld())
			? FRotator(0.0f, (GetGrasslandVillageViewTargetHint() - HubLocation).Rotation().Yaw, 0.0f)
		: (IsGrasslandFarmWorld(GetWorld())
			? FRotator(0.0f, (GetGrasslandFarmViewTargetHint() - HubLocation).Rotation().Yaw, 0.0f)
		: (IsLikanaValleyWorld(GetWorld())
			? FRotator(0.0f, (GetLikanaValleyViewTargetHint() - HubLocation).Rotation().Yaw, 0.0f)
		: (UsesKashkehStarterBaseWorld(GetWorld())
			? FRotator(0.0f, (GetKashkehViewTargetHint() - HubLocation).Rotation().Yaw, 0.0f)
			: (IsLerigothiWorld(GetWorld())
				? FRotator(0.0f, (GetLerigothiVillageViewTargetHint() - HubLocation).Rotation().Yaw, 0.0f)
				: FRotator(0.0f, -92.0f, 0.0f)))))));
	if (bNeedsRecovery)
	{
		UE_LOG(LogTP_ThirdPerson, Display, TEXT("StarterForestSpawnRecovery current=(%.1f, %.1f, %.1f) hub=(%.1f, %.1f, %.1f) ground=(%.1f, %.1f, %.1f)"),
			CurrentLocation.X, CurrentLocation.Y, CurrentLocation.Z,
			HubLocation.X, HubLocation.Y, HubLocation.Z,
			GroundProbe.X, GroundProbe.Y, GroundProbe.Z);
		PlayerCharacter->SetActorLocation(HubLocation, false, nullptr, ETeleportType::ResetPhysics);
		PlayerCharacter->SetActorRotation(RecoveryRotation);
	}

	if (UCharacterMovementComponent* MovementComponent = PlayerCharacter->GetCharacterMovement())
	{
		if (bNeedsRecovery)
		{
			MovementComponent->StopMovementImmediately();
			MovementComponent->SetMovementMode(MOVE_Walking);
		}
		else if (MovementComponent->MovementMode == MOVE_None)
		{
			MovementComponent->SetMovementMode(MOVE_Walking);
		}
	}

	const float SafeZoneRadius = 1500.0f;
	FVector SafeZoneCenter = HubLocation;
	if (bIsLerigothi)
	{
		SafeZoneCenter = ResolveTutorialGroundLocation(GetLerigothiVillageCenterHint(), HeightOffset);
	}
	else if (IsGrasslandVillageWorld(GetWorld()))
	{
		SafeZoneCenter = ResolveTutorialGroundLocation(GetGrasslandVillageCenterHint(), HeightOffset);
	}
	else if (IsGrasslandMolehillWorld(GetWorld()))
	{
		SafeZoneCenter = ResolveTutorialGroundLocation(GetGrasslandMolehillCenterHint(), HeightOffset);
	}
	else if (IsGrasslandFarmWorld(GetWorld()))
	{
		SafeZoneCenter = ResolveTutorialGroundLocation(GetGrasslandFarmCenterHint(), HeightOffset);
	}
	else if (IsLikanaValleyWorld(GetWorld()))
	{
		SafeZoneCenter = ResolveTutorialGroundLocation(GetLikanaValleyCenterHint(), HeightOffset);
	}
	else if (UsesKashkehStarterBaseWorld(GetWorld()))
	{
		SafeZoneCenter = ResolveTutorialGroundLocation(GetKashkehVillageCenterHint(), HeightOffset);
	}
	for (TActorIterator<AFantasyFrontierEnemyBase> It(GetWorld()); It; ++It)
	{
		if (IsValid(*It) && FVector::DistSquared2D(It->GetActorLocation(), SafeZoneCenter) <= FMath::Square(SafeZoneRadius))
		{
			It->Destroy();
		}
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
