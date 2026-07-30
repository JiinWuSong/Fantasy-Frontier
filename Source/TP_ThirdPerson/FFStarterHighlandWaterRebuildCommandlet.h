#pragma once

#include "Commandlets/Commandlet.h"
#include "FFStarterHighlandWaterRebuildCommandlet.generated.h"

UCLASS()
class UFFStarterHighlandWaterRebuildCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UFFStarterHighlandWaterRebuildCommandlet();

	virtual int32 Main(const FString& Params) override;
};
