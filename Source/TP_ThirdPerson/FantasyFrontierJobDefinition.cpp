#include "FantasyFrontierJobDefinition.h"

FPrimaryAssetId UFantasyFrontierJobDefinition::GetPrimaryAssetId() const
{
	const FName EffectiveId = JobId.IsNone() ? GetFName() : JobId;
	return FPrimaryAssetId(TEXT("FantasyFrontierJob"), EffectiveId);
}
