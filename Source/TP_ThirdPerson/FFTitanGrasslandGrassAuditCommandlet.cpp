#include "FFTitanGrasslandGrassAuditCommandlet.h"

#if WITH_EDITOR
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/StaticMesh.h"
#include "LandscapeGrassType.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInterface.h"
#include "Modules/ModuleManager.h"
#include "UObject/SoftObjectPath.h"
#endif

UFFTitanGrasslandGrassAuditCommandlet::UFFTitanGrasslandGrassAuditCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

#if WITH_EDITOR
namespace
{
	const TCHAR* GrassTypePaths[] = {
		TEXT("/Game/Landscape/LGT/LGT_Grass.LGT_Grass"),
		TEXT("/Game/Landscape/LGT/LGT_DesertGrass.LGT_DesertGrass")
	};

	const TCHAR* GrassMeshPaths[] = {
		TEXT("/Game/Environment/Foliage/Meshes/SM_GrassBlade.SM_GrassBlade"),
		TEXT("/Game/Environment/Foliage/Grass/Ryegrass_Grass_A.Ryegrass_Grass_A"),
		TEXT("/Game/Environment/Foliage/Grass/Ryegrass_Grass_C.Ryegrass_Grass_C"),
		TEXT("/Game/Environment/Foliage/Grass/Ryegrass_Grass_D.Ryegrass_Grass_D"),
		TEXT("/Game/Environment/Foliage/Grass/Ryegrass_Rye_Tuft.Ryegrass_Rye_Tuft"),
		TEXT("/Game/Environment/Foliage/Grass/Ryegrass_Stalk_Green.Ryegrass_Stalk_Green"),
		TEXT("/Game/Environment/Foliage/Grass/Titan_Lilium_Reedgrass.Titan_Lilium_Reedgrass"),
		TEXT("/Game/Environment/Foliage/Grass/SM_Grass01.SM_Grass01"),
		TEXT("/Game/Environment/Foliage/Grass/SM_Grass_Forest_03.SM_Grass_Forest_03"),
		TEXT("/Game/Environment/Foliage/Grass/SM_Grass_Flaten.SM_Grass_Flaten")
	};

	const TCHAR* GrassMaterialPaths[] = {
		TEXT("/Game/Environment/Foliage/Materials/M_GrassBlade.M_GrassBlade"),
		TEXT("/Game/Environment/Foliage/Materials/MI_GrassBlade.MI_GrassBlade"),
		TEXT("/Game/Environment/Foliage/Materials/MI_GrassBladeDarker.MI_GrassBladeDarker"),
		TEXT("/Game/Environment/Clifftop/Materials/Foliage/MI_Clifftop_Ryegrass.MI_Clifftop_Ryegrass"),
		TEXT("/Game/Environment/Arctic/Materials/Foliage/MI_Grass_Forest_03.MI_Grass_Forest_03"),
		TEXT("/Game/Environment/StarterIsland/Meshes/SmallIsland/Foliage_wn/MI_Grass_Flat.MI_Grass_Flat"),
		TEXT("/Game/Environment/Tropic/Loong/Materials/MI_Tropic_Vine_002.MI_Tropic_Vine_002"),
		TEXT("/Game/Environment/Clifftop/Materials/Foliage/M_Reeds.M_Reeds"),
		TEXT("/Game/FantasyFrontier/Blockout/Materials/M_FF_Highland_GrassBlade_Soft.M_FF_Highland_GrassBlade_Soft")
	};

	void LogPackageDependencies(const FString& ObjectPath)
	{
		FSoftObjectPath SoftPath(ObjectPath);
		const FString PackageName = SoftPath.GetLongPackageName();
		if (PackageName.IsEmpty())
		{
			return;
		}

		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		TArray<FName> Dependencies;
		UE::AssetRegistry::FDependencyQuery DependencyQuery;
		AssetRegistryModule.Get().GetDependencies(FName(*PackageName), Dependencies, UE::AssetRegistry::EDependencyCategory::Package, DependencyQuery);

		UE_LOG(LogTemp, Display, TEXT("TitanGrassAudit: dependencies package=%s count=%d"), *PackageName, Dependencies.Num());
		for (const FName& Dependency : Dependencies)
		{
			const FString DependencyString = Dependency.ToString();
			if (DependencyString.Contains(TEXT("RVT")) ||
				DependencyString.Contains(TEXT("RT_Player")) ||
				DependencyString.Contains(TEXT("MPC_Player")) ||
				DependencyString.Contains(TEXT("FoliageInteraction")) ||
				DependencyString.Contains(TEXT("Wind")) ||
				DependencyString.Contains(TEXT("Grass")) ||
				DependencyString.Contains(TEXT("Landscape")) ||
				DependencyString.Contains(TEXT("Texture")))
			{
				UE_LOG(LogTemp, Display, TEXT("TitanGrassAudit: dependency %s -> %s"), *PackageName, *DependencyString);
			}
		}
	}

	void LogGrassMaterialAsset(const TCHAR* MaterialPath)
	{
		UMaterialInterface* MaterialInterface = LoadObject<UMaterialInterface>(nullptr, MaterialPath);
		if (!MaterialInterface)
		{
			UE_LOG(LogTemp, Warning, TEXT("TitanGrassAudit: missing material %s"), MaterialPath);
			return;
		}

		UMaterial* Material = MaterialInterface->GetMaterial();
		UMaterialInstance* MaterialInstance = Cast<UMaterialInstance>(MaterialInterface);
		UE_LOG(LogTemp, Display, TEXT("TitanGrassAudit: material path=%s class=%s parent=%s baseMaterial=%s twoSided=%s usedISM=%s"),
			*MaterialInterface->GetPathName(),
			*MaterialInterface->GetClass()->GetName(),
			MaterialInstance && MaterialInstance->Parent ? *MaterialInstance->Parent->GetPathName() : TEXT("None"),
			Material ? *Material->GetPathName() : TEXT("None"),
			Material && Material->TwoSided ? TEXT("true") : TEXT("false"),
			Material && Material->bUsedWithInstancedStaticMeshes ? TEXT("true") : TEXT("false"));

		TArray<FMaterialParameterInfo> ParameterInfos;
		TArray<FGuid> ParameterIds;
		MaterialInterface->GetAllScalarParameterInfo(ParameterInfos, ParameterIds);
		for (const FMaterialParameterInfo& ParameterInfo : ParameterInfos)
		{
			float Value = 0.0f;
			if (MaterialInterface->GetScalarParameterValue(ParameterInfo, Value))
			{
				UE_LOG(LogTemp, Display, TEXT("TitanGrassAudit: scalar material=%s name=%s value=%.6f"),
					*MaterialInterface->GetPathName(),
					*ParameterInfo.Name.ToString(),
					Value);
			}
		}

		ParameterInfos.Reset();
		ParameterIds.Reset();
		MaterialInterface->GetAllVectorParameterInfo(ParameterInfos, ParameterIds);
		for (const FMaterialParameterInfo& ParameterInfo : ParameterInfos)
		{
			FLinearColor Value = FLinearColor::Black;
			if (MaterialInterface->GetVectorParameterValue(ParameterInfo, Value))
			{
				UE_LOG(LogTemp, Display, TEXT("TitanGrassAudit: vector material=%s name=%s value=%s"),
					*MaterialInterface->GetPathName(),
					*ParameterInfo.Name.ToString(),
					*Value.ToString());
			}
		}

		ParameterInfos.Reset();
		ParameterIds.Reset();
		MaterialInterface->GetAllTextureParameterInfo(ParameterInfos, ParameterIds);
		for (const FMaterialParameterInfo& ParameterInfo : ParameterInfos)
		{
			UTexture* Value = nullptr;
			if (MaterialInterface->GetTextureParameterValue(ParameterInfo, Value))
			{
				UE_LOG(LogTemp, Display, TEXT("TitanGrassAudit: texture material=%s name=%s value=%s"),
					*MaterialInterface->GetPathName(),
					*ParameterInfo.Name.ToString(),
					Value ? *Value->GetPathName() : TEXT("None"));
			}
		}

		LogPackageDependencies(MaterialInterface->GetPathName());
		if (Material)
		{
			LogPackageDependencies(Material->GetPathName());
		}
	}

	void LogMesh(const TCHAR* MeshPath)
	{
		UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, MeshPath);
		if (!Mesh)
		{
			UE_LOG(LogTemp, Warning, TEXT("TitanGrassAudit: missing mesh %s"), MeshPath);
			return;
		}

		const FBoxSphereBounds Bounds = Mesh->GetBounds();
		UE_LOG(LogTemp, Display, TEXT("TitanGrassAudit: mesh path=%s boundsOrigin=%s boundsExtent=%s materials=%d"),
			*Mesh->GetPathName(),
			*Bounds.Origin.ToString(),
			*Bounds.BoxExtent.ToString(),
			Mesh->GetStaticMaterials().Num());

		for (int32 MaterialIndex = 0; MaterialIndex < Mesh->GetStaticMaterials().Num(); ++MaterialIndex)
		{
			UMaterialInterface* Material = Mesh->GetStaticMaterials()[MaterialIndex].MaterialInterface;
			UE_LOG(LogTemp, Display, TEXT("TitanGrassAudit: meshMaterial mesh=%s slot=%d material=%s"),
				*Mesh->GetPathName(),
				MaterialIndex,
				Material ? *Material->GetPathName() : TEXT("None"));
			if (Material)
			{
				LogPackageDependencies(Material->GetPathName());
				if (UMaterial* BaseMaterial = Material->GetMaterial())
				{
					LogPackageDependencies(BaseMaterial->GetPathName());
				}
			}
		}
	}

	void LogGrassType(const TCHAR* GrassTypePath)
	{
		ULandscapeGrassType* GrassType = LoadObject<ULandscapeGrassType>(nullptr, GrassTypePath);
		if (!GrassType)
		{
			UE_LOG(LogTemp, Warning, TEXT("TitanGrassAudit: missing grassType %s"), GrassTypePath);
			return;
		}

		UE_LOG(LogTemp, Display, TEXT("TitanGrassAudit: grassType path=%s varieties=%d"),
			*GrassType->GetPathName(),
			GrassType->GrassVarieties.Num());

		for (int32 VarietyIndex = 0; VarietyIndex < GrassType->GrassVarieties.Num(); ++VarietyIndex)
		{
			const FGrassVariety& Variety = GrassType->GrassVarieties[VarietyIndex];
			UE_LOG(LogTemp, Display,
				TEXT("TitanGrassAudit: variety=%d mesh=%s density=%.2f useGrid=%s jitter=%.2f cull=%d-%d scaleX=%.3f..%.3f scaleY=%.3f..%.3f scaleZ=%.3f..%.3f align=%s randomRot=%s"),
				VarietyIndex,
				Variety.GrassMesh ? *Variety.GrassMesh->GetPathName() : TEXT("None"),
				Variety.GrassDensity.Default,
				Variety.bUseGrid ? TEXT("true") : TEXT("false"),
				Variety.PlacementJitter,
				Variety.GetStartCullDistance(),
				Variety.GetEndCullDistance(),
				Variety.ScaleX.Min,
				Variety.ScaleX.Max,
				Variety.ScaleY.Min,
				Variety.ScaleY.Max,
				Variety.ScaleZ.Min,
				Variety.ScaleZ.Max,
				Variety.AlignToSurface ? TEXT("true") : TEXT("false"),
				Variety.RandomRotation ? TEXT("true") : TEXT("false"));
		}

		LogPackageDependencies(GrassType->GetPathName());
	}
}
#endif

int32 UFFTitanGrasslandGrassAuditCommandlet::Main(const FString& Params)
{
#if WITH_EDITOR
	UE_LOG(LogTemp, Display, TEXT("TitanGrassAudit: focused read-only Titan Grassland grass/ground audit"));

	for (const TCHAR* GrassTypePath : GrassTypePaths)
	{
		LogGrassType(GrassTypePath);
	}

	for (const TCHAR* MeshPath : GrassMeshPaths)
	{
		LogMesh(MeshPath);
	}

	for (const TCHAR* MaterialPath : GrassMaterialPaths)
	{
		LogGrassMaterialAsset(MaterialPath);
	}

	LogPackageDependencies(TEXT("/Game/Landscape/Materials/M_LandscapeMain.M_LandscapeMain"));
	LogPackageDependencies(TEXT("/Game/Landscape/Materials/Functions/MF_LL_Grassland_Grass.MF_LL_Grassland_Grass"));
	LogPackageDependencies(TEXT("/Game/Landscape/RVT/RVT_Titan_D.RVT_Titan_D"));
	LogPackageDependencies(TEXT("/Game/Landscape/RVT/RVT_Titan_H.RVT_Titan_H"));
	LogPackageDependencies(TEXT("/Game/Blueprint/FoliageInteraction/RT_Player.RT_Player"));
	LogPackageDependencies(TEXT("/Game/Blueprint/FoliageInteraction/MPC_Player.MPC_Player"));

	return 0;
#else
	UE_LOG(LogTemp, Error, TEXT("FFTitanGrasslandGrassAuditCommandlet can only run in editor builds."));
	return 1;
#endif
}
