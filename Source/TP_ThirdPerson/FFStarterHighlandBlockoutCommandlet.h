#pragma once

#include "Commandlets/Commandlet.h"
#include "FFStarterHighlandBlockoutCommandlet.generated.h"

UCLASS()
class UFFStarterHighlandBlockoutCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UFFStarterHighlandBlockoutCommandlet();

	virtual int32 Main(const FString& Params) override;
};
