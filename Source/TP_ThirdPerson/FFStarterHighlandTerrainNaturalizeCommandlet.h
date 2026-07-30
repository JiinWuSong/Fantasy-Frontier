#pragma once

#include "Commandlets/Commandlet.h"
#include "FFStarterHighlandTerrainNaturalizeCommandlet.generated.h"

UCLASS()
class UFFStarterHighlandTerrainNaturalizeCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UFFStarterHighlandTerrainNaturalizeCommandlet();

	virtual int32 Main(const FString& Params) override;
};
