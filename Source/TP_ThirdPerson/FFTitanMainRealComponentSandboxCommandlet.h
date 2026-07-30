#pragma once

#include "Commandlets/Commandlet.h"
#include "FFTitanMainRealComponentSandboxCommandlet.generated.h"

UCLASS()
class UFFTitanMainRealComponentSandboxCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UFFTitanMainRealComponentSandboxCommandlet();
	virtual int32 Main(const FString& Params) override;
};
