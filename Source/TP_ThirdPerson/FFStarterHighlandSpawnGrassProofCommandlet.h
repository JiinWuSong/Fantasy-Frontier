#pragma once

#include "Commandlets/Commandlet.h"
#include "FFStarterHighlandSpawnGrassProofCommandlet.generated.h"

UCLASS()
class UFFStarterHighlandSpawnGrassProofCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UFFStarterHighlandSpawnGrassProofCommandlet();

	virtual int32 Main(const FString& Params) override;
};
