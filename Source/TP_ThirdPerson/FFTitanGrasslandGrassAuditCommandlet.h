#pragma once

#include "Commandlets/Commandlet.h"
#include "FFTitanGrasslandGrassAuditCommandlet.generated.h"

UCLASS()
class UFFTitanGrasslandGrassAuditCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UFFTitanGrasslandGrassAuditCommandlet();

	virtual int32 Main(const FString& Params) override;
};
