#pragma once

#include "Commandlets/Commandlet.h"
#include "FFStarterHighlandVisualPassCommandlet.generated.h"

UCLASS()
class UFFStarterHighlandVisualPassCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UFFStarterHighlandVisualPassCommandlet();

	virtual int32 Main(const FString& Params) override;
};
