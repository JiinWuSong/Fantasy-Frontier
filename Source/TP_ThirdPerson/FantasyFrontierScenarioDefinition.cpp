#include "FantasyFrontierScenarioDefinition.h"

FPrimaryAssetId UFantasyFrontierScenarioDataAsset::GetPrimaryAssetId() const
{
	const FName EffectiveId = ScenarioId.IsNone() ? GetFName() : ScenarioId;
	return FPrimaryAssetId(TEXT("FantasyFrontierScenario"), EffectiveId);
}
