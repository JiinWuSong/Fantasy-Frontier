#pragma once

#include "Commandlets/Commandlet.h"
#include "FFStarterHighlandVegetationLayerCommandlet.generated.h"

UCLASS()
class UFFStarterHighlandVegetationLayerCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UFFStarterHighlandVegetationLayerCommandlet();

	virtual int32 Main(const FString& Params) override;
};
