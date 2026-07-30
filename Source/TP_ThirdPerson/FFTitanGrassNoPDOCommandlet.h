#pragma once

#include "Commandlets/Commandlet.h"
#include "FFTitanGrassNoPDOCommandlet.generated.h"

UCLASS()
class UFFTitanGrassNoPDOCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UFFTitanGrassNoPDOCommandlet();
	virtual int32 Main(const FString& Params) override;
};
