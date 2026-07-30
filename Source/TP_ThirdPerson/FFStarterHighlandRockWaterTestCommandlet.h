#pragma once

#include "Commandlets/Commandlet.h"
#include "FFStarterHighlandRockWaterTestCommandlet.generated.h"

UCLASS()
class UFFStarterHighlandRockWaterTestCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UFFStarterHighlandRockWaterTestCommandlet();

	virtual int32 Main(const FString& Params) override;
};
