#pragma once

#include "Commandlets/Commandlet.h"
#include "FFStarterHighlandGrassRVTTestCommandlet.generated.h"

UCLASS()
class UFFStarterHighlandGrassRVTTestCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UFFStarterHighlandGrassRVTTestCommandlet();

	virtual int32 Main(const FString& Params) override;
};
