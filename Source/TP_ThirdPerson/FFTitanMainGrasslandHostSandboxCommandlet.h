#pragma once

#include "Commandlets/Commandlet.h"
#include "FFTitanMainGrasslandHostSandboxCommandlet.generated.h"

UCLASS()
class UFFTitanMainGrasslandHostSandboxCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UFFTitanMainGrasslandHostSandboxCommandlet();
	virtual int32 Main(const FString& Params) override;
};
