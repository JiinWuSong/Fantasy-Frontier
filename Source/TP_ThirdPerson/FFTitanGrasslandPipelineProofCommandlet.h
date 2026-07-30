#pragma once

#include "Commandlets/Commandlet.h"
#include "FFTitanGrasslandPipelineProofCommandlet.generated.h"

UCLASS()
class UFFTitanGrasslandPipelineProofCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UFFTitanGrasslandPipelineProofCommandlet();
	virtual int32 Main(const FString& Params) override;
};
