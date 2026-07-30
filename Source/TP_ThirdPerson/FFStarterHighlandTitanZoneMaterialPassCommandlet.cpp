#include "FFStarterHighlandTitanZoneMaterialPassCommandlet.h"

#if WITH_EDITOR
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "FileHelpers.h"
#include "LandscapeProxy.h"
#include "Materials/Material.h"
#include "Materials/MaterialInterface.h"
#endif

UFFStarterHighlandTitanZoneMaterialPassCommandlet::UFFStarterHighlandTitanZoneMaterialPassCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

#if WITH_EDITOR
namespace
{
	const TCHAR* HighlandMapPath = TEXT("/Game/Maps/FF_Starter_Highland_Blockout");
	const FName Phase1GrassTag(TEXT("FFPhase1GrassPlacement"));
	const FName VisualWaterPlaceholderTag(TEXT("FFVisualPassWaterPlaceholder"));

	const TCHAR* TitanGrassBladeMaterialPath = TEXT("/Game/Environment/Foliage/Materials/MI_GrassBlade.MI_GrassBlade");
	const TCHAR* TitanGrassBladeDarkerMaterialPath = TEXT("/Game/Environment/Foliage/Materials/MI_GrassBladeDarker.MI_GrassBladeDarker");
	const TCHAR* TitanFlatGrassMaterialPath = TEXT("/Game/Environment/_Global/Materials/FlatCol/MI_FlatCol_Green.MI_FlatCol_Green");
	const TCHAR* TitanFlatDarkGrassMaterialPath = TEXT("/Game/Environment/_Global/Materials/FlatCol/MI_FlatCol_Green_Dark.MI_FlatCol_Green_Dark");
	const TCHAR* TitanCliffMaterialPath = TEXT("/Game/Environment/Grassland/Materials/MI_Cliffside_Dirt_B.MI_Cliffside_Dirt_B");
	const TCHAR* TitanDirtRoadMaterialPath = TEXT("/Game/Environment/_Global/Core/Materials/M_DirtRoad.M_DirtRoad");
	const TCHAR* TitanGravelMaterialPath = TEXT("/Game/Environment/Grassland/Materials/MI_Gravel_Base.MI_Gravel_Base");
	const TCHAR* TitanWaterRiverMaterialPath = TEXT("/Game/Environment/_Global/Core/Materials/Water/Water_Material_River.Water_Material_River");

	UMaterialInterface* LoadRequiredMaterial(const TCHAR* Path, const TCHAR* Label, bool& bOutLoaded)
	{
		UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, Path);
		if (!Material)
		{
			UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandTitanZoneMaterialPass: missing %s material at %s"), Label, Path);
			bOutLoaded = false;
			return nullptr;
		}

		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandTitanZoneMaterialPass: %s=%s"), Label, *Material->GetPathName());
		return Material;
	}

	void EnsureInstancedGrassUsage(UMaterialInterface* Material)
	{
		if (!Material)
		{
			return;
		}

		if (UMaterial* BaseMaterial = Material->GetMaterial())
		{
			BaseMaterial->Modify();
			BaseMaterial->TwoSided = true;
			BaseMaterial->bUsedWithInstancedStaticMeshes = true;
			BaseMaterial->bUsedWithNanite = true;
			BaseMaterial->MarkPackageDirty();
		}
	}

	int32 SetAllPrimitiveSlots(UPrimitiveComponent* Component, UMaterialInterface* Material)
	{
		if (!Component || !Material)
		{
			return 0;
		}

		Component->Modify();
		const int32 SlotCount = FMath::Max(1, Component->GetNumMaterials());
		for (int32 SlotIndex = 0; SlotIndex < SlotCount; ++SlotIndex)
		{
			Component->SetMaterial(SlotIndex, Material);
		}
		Component->MarkPackageDirty();
		return SlotCount;
	}

	bool TextContainsAny(const FString& Text, std::initializer_list<const TCHAR*> Needles)
	{
		for (const TCHAR* Needle : Needles)
		{
			if (Text.Contains(Needle, ESearchCase::IgnoreCase))
			{
				return true;
			}
		}
		return false;
	}

	bool ActorOrMaterialContains(const AActor* Actor, const UPrimitiveComponent* Component, std::initializer_list<const TCHAR*> Needles)
	{
		if (Actor)
		{
			const FString ActorName = Actor->GetName();
			const FString ClassName = Actor->GetClass() ? Actor->GetClass()->GetName() : FString();
			if (TextContainsAny(ActorName, Needles) || TextContainsAny(ClassName, Needles))
			{
				return true;
			}
		}

		if (!Component)
		{
			return false;
		}

		const int32 SlotCount = FMath::Max(1, Component->GetNumMaterials());
		for (int32 SlotIndex = 0; SlotIndex < SlotCount; ++SlotIndex)
		{
			const UMaterialInterface* Material = Component->GetMaterial(SlotIndex);
			if (Material && TextContainsAny(Material->GetName(), Needles))
			{
				return true;
			}
		}

		return false;
	}

	bool IsNativeWaterBodyActor(const AActor* Actor)
	{
		if (!Actor || Actor->ActorHasTag(VisualWaterPlaceholderTag))
		{
			return false;
		}

		const FString ActorName = Actor->GetName();
		const FString ClassName = Actor->GetClass() ? Actor->GetClass()->GetName() : FString();
		return TextContainsAny(ActorName, { TEXT("WaterBodyCustom"), TEXT("WaterBodyRiver"), TEXT("WaterBodyLake") })
			|| TextContainsAny(ClassName, { TEXT("WaterBodyCustom"), TEXT("WaterBodyRiver"), TEXT("WaterBodyLake") });
	}

	bool IsBroadOceanActor(const AActor* Actor)
	{
		if (!Actor || IsNativeWaterBodyActor(Actor))
		{
			return false;
		}

		const FString ActorName = Actor->GetName();
		const FString ClassName = Actor->GetClass() ? Actor->GetClass()->GetName() : FString();
		return TextContainsAny(ActorName, { TEXT("Ocean"), TEXT("Sea") })
			|| TextContainsAny(ClassName, { TEXT("Ocean"), TEXT("Sea") });
	}

	void HideActorRendering(AActor* Actor)
	{
		if (!Actor)
		{
			return;
		}

		Actor->Modify();
		Actor->SetIsTemporarilyHiddenInEditor(true);
		Actor->SetActorHiddenInGame(true);
		TArray<UPrimitiveComponent*> PrimitiveComponents;
		Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
		for (UPrimitiveComponent* Component : PrimitiveComponents)
		{
			if (!Component)
			{
				continue;
			}
			Component->Modify();
			Component->SetVisibility(false, true);
			Component->SetHiddenInGame(true);
			Component->MarkPackageDirty();
		}
		Actor->MarkPackageDirty();
	}

	FBox GetPrimitiveBounds(const TArray<UPrimitiveComponent*>& PrimitiveComponents)
	{
		FBox Bounds(ForceInit);
		for (const UPrimitiveComponent* Component : PrimitiveComponents)
		{
			if (Component)
			{
				Bounds += Component->Bounds.GetBox();
			}
		}
		return Bounds;
	}

	bool HasVisiblePrimitive(const AActor* Actor, const TArray<UPrimitiveComponent*>& PrimitiveComponents)
	{
		if (!Actor || Actor->IsHidden())
		{
			return false;
		}

		for (const UPrimitiveComponent* Component : PrimitiveComponents)
		{
			if (Component && Component->IsVisible() && !Component->bHiddenInGame)
			{
				return true;
			}
		}
		return false;
	}
}
#endif

int32 UFFStarterHighlandTitanZoneMaterialPassCommandlet::Main(const FString& Params)
{
#if WITH_EDITOR
	UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandTitanZoneMaterialPass: loading %s without terrain/layout edits."), HighlandMapPath);
	UWorld* World = UEditorLoadingAndSavingUtils::LoadMap(HighlandMapPath);
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandTitanZoneMaterialPass: failed to load %s"), HighlandMapPath);
		return 1;
	}

	bool bAllMaterialsLoaded = true;
	// The dedicated Titan grass blade materials are loaded for reporting, but
	// are not forced onto HISM grass because packaged validation showed they
	// render blue/purple in this standalone blockout. Use Titan FlatCol greens
	// instead of inventing a local fallback material.
	UMaterialInterface* TitanGrassBladeMaterial = LoadRequiredMaterial(TitanGrassBladeMaterialPath, TEXT("grassBladeCandidate"), bAllMaterialsLoaded);
	UMaterialInterface* TitanGrassBladeDarkerMaterial = LoadRequiredMaterial(TitanGrassBladeDarkerMaterialPath, TEXT("darkGrassBladeCandidate"), bAllMaterialsLoaded);
	UMaterialInterface* GrassMaterial = LoadRequiredMaterial(TitanFlatGrassMaterialPath, TEXT("flatGrassApplied"), bAllMaterialsLoaded);
	UMaterialInterface* DarkGrassMaterial = LoadRequiredMaterial(TitanFlatDarkGrassMaterialPath, TEXT("flatDarkGrassApplied"), bAllMaterialsLoaded);
	UMaterialInterface* CliffMaterial = LoadRequiredMaterial(TitanCliffMaterialPath, TEXT("cliff"), bAllMaterialsLoaded);
	UMaterialInterface* DirtRoadMaterial = LoadRequiredMaterial(TitanDirtRoadMaterialPath, TEXT("dirtRoad"), bAllMaterialsLoaded);
	UMaterialInterface* GravelMaterial = LoadRequiredMaterial(TitanGravelMaterialPath, TEXT("gravel"), bAllMaterialsLoaded);
	UMaterialInterface* WaterRiverMaterial = LoadRequiredMaterial(TitanWaterRiverMaterialPath, TEXT("waterRiver"), bAllMaterialsLoaded);
	if (!bAllMaterialsLoaded)
	{
		return 1;
	}

	EnsureInstancedGrassUsage(GrassMaterial);
	EnsureInstancedGrassUsage(DarkGrassMaterial);
	EnsureInstancedGrassUsage(TitanGrassBladeMaterial);
	EnsureInstancedGrassUsage(TitanGrassBladeDarkerMaterial);

	int32 LandscapesLeftOnExistingMaterial = 0;
	for (TActorIterator<ALandscapeProxy> It(World); It; ++It)
	{
		++LandscapesLeftOnExistingMaterial;
		UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandTitanZoneMaterialPass: landscape=%s materialLeftAs=%s"),
			*It->GetName(),
			It->LandscapeMaterial ? *It->LandscapeMaterial->GetPathName() : TEXT("None"));
	}

	int32 GrassComponents = 0;
	int32 GrassMaterialSlots = 0;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor || !Actor->ActorHasTag(Phase1GrassTag))
		{
			continue;
		}

		TArray<UHierarchicalInstancedStaticMeshComponent*> GrassHISMComponents;
		Actor->GetComponents<UHierarchicalInstancedStaticMeshComponent>(GrassHISMComponents);
		for (int32 ComponentIndex = 0; ComponentIndex < GrassHISMComponents.Num(); ++ComponentIndex)
		{
			UHierarchicalInstancedStaticMeshComponent* Component = GrassHISMComponents[ComponentIndex];
			if (!Component)
			{
				continue;
			}

			++GrassComponents;
			UMaterialInterface* SelectedGrassMaterial = (ComponentIndex % 3 == 2) ? DarkGrassMaterial : GrassMaterial;
			GrassMaterialSlots += SetAllPrimitiveSlots(Component, SelectedGrassMaterial);
		}
	}

	int32 NativeWaterActors = 0;
	int32 NativeWaterSlots = 0;
	int32 HiddenFloodWaterActors = 0;
	int32 HiddenWaterPlaceholders = 0;
	int32 HiddenOceanActors = 0;
	int32 RockActors = 0;
	int32 RockSlots = 0;
	int32 DirtPathActors = 0;
	int32 DirtPathSlots = 0;
	int32 ShoreActors = 0;
	int32 ShoreSlots = 0;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor)
		{
			continue;
		}

		if (Actor->ActorHasTag(VisualWaterPlaceholderTag))
		{
			HideActorRendering(Actor);
			++HiddenWaterPlaceholders;
			continue;
		}

		if (IsBroadOceanActor(Actor))
		{
			HideActorRendering(Actor);
			++HiddenOceanActors;
			continue;
		}

		TArray<UPrimitiveComponent*> PrimitiveComponents;
		Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
		if (PrimitiveComponents.Num() == 0)
		{
			continue;
		}

		if (IsNativeWaterBodyActor(Actor))
		{
			const FBox WaterBounds = GetPrimitiveBounds(PrimitiveComponents);
			const FVector WaterExtent = WaterBounds.IsValid ? WaterBounds.GetExtent() : FVector::ZeroVector;
			const bool bWaterWasVisible = HasVisiblePrimitive(Actor, PrimitiveComponents);
			const bool bFloodRisk = WaterExtent.X > 90000.0f && WaterExtent.Y > 65000.0f;
			UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandTitanZoneMaterialPass: waterActor=%s class=%s visible=%s extent=(%.1f, %.1f, %.1f) floodRisk=%s"),
				*Actor->GetName(),
				Actor->GetClass() ? *Actor->GetClass()->GetName() : TEXT("None"),
				bWaterWasVisible ? TEXT("true") : TEXT("false"),
				WaterExtent.X,
				WaterExtent.Y,
				WaterExtent.Z,
				bFloodRisk ? TEXT("true") : TEXT("false"));

			if (!bWaterWasVisible || bFloodRisk)
			{
				HideActorRendering(Actor);
				++HiddenFloodWaterActors;
				continue;
			}

			for (UPrimitiveComponent* Component : PrimitiveComponents)
			{
				Component->Modify();
				Component->SetVisibility(true, true);
				Component->SetHiddenInGame(false);
				NativeWaterSlots += SetAllPrimitiveSlots(Component, WaterRiverMaterial);
			}
			Actor->MarkPackageDirty();
			++NativeWaterActors;
			continue;
		}

		for (UPrimitiveComponent* Component : PrimitiveComponents)
		{
			if (!Component)
			{
				continue;
			}

			if (ActorOrMaterialContains(Actor, Component, { TEXT("Canyon"), TEXT("Cliff"), TEXT("Rock"), TEXT("Mountain") }))
			{
				RockSlots += SetAllPrimitiveSlots(Component, CliffMaterial);
				++RockActors;
			}
			else if (ActorOrMaterialContains(Actor, Component, { TEXT("Shore"), TEXT("Bank"), TEXT("Gravel"), TEXT("Riverbed"), TEXT("RiverBed") }))
			{
				ShoreSlots += SetAllPrimitiveSlots(Component, GravelMaterial);
				++ShoreActors;
			}
			else if (ActorOrMaterialContains(Actor, Component, { TEXT("Path"), TEXT("DirtRoad"), TEXT("Village"), TEXT("Bridge"), TEXT("BossGate") }))
			{
				DirtPathSlots += SetAllPrimitiveSlots(Component, DirtRoadMaterial);
				++DirtPathActors;
			}
		}
	}

	World->MarkPackageDirty();
	const bool bSavedMap = UEditorLoadingAndSavingUtils::SaveMap(World, HighlandMapPath);
	const bool bSavedPackages = UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);

	UE_LOG(LogTemp, Display, TEXT("FFStarterHighlandTitanZoneMaterialPass: landscapesExistingMaterial=%d grassComponents=%d grassSlots=%d nativeWaterActors=%d nativeWaterSlots=%d rockActors=%d rockSlots=%d shoreActors=%d shoreSlots=%d dirtPathActors=%d dirtPathSlots=%d hiddenOceanActors=%d hiddenFloodWaterActors=%d hiddenWaterPlaceholders=%d savedMap=%s savedPackages=%s"),
		LandscapesLeftOnExistingMaterial,
		GrassComponents,
		GrassMaterialSlots,
		NativeWaterActors,
		NativeWaterSlots,
		RockActors,
		RockSlots,
		ShoreActors,
		ShoreSlots,
		DirtPathActors,
		DirtPathSlots,
		HiddenOceanActors,
		HiddenFloodWaterActors,
		HiddenWaterPlaceholders,
		bSavedMap ? TEXT("true") : TEXT("false"),
		bSavedPackages ? TEXT("true") : TEXT("false"));

	return (bSavedMap && bSavedPackages) ? 0 : 1;
#else
	UE_LOG(LogTemp, Error, TEXT("FFStarterHighlandTitanZoneMaterialPass can only run in editor builds."));
	return 1;
#endif
}
