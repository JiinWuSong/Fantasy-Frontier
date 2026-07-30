#pragma once

#include "Commandlets/Commandlet.h"
#include "FFStarterHighlandGrassNaturalizationCommandlet.generated.h"

UCLASS()
class UFFStarterHighlandGrassNaturalizationCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UFFStarterHighlandGrassNaturalizationCommandlet();

	virtual int32 Main(const FString& Params) override;
};
