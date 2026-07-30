#pragma once

#include "Commandlets/Commandlet.h"
#include "FFTitanGrasslandRegionSandboxCommandlet.generated.h"

UCLASS()
class UFFTitanGrasslandRegionSandboxCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UFFTitanGrasslandRegionSandboxCommandlet();
	virtual int32 Main(const FString& Params) override;
};
