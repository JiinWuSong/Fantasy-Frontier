#pragma once

#include "Commandlets/Commandlet.h"
#include "FFStarterHighlandRiverMeshCommandlet.generated.h"

UCLASS()
class UFFStarterHighlandRiverMeshCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UFFStarterHighlandRiverMeshCommandlet();

	virtual int32 Main(const FString& Params) override;
};
