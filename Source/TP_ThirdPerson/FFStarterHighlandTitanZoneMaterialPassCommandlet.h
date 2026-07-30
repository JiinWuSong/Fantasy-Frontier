#pragma once

#include "Commandlets/Commandlet.h"
#include "FFStarterHighlandTitanZoneMaterialPassCommandlet.generated.h"

UCLASS()
class UFFStarterHighlandTitanZoneMaterialPassCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UFFStarterHighlandTitanZoneMaterialPassCommandlet();

	virtual int32 Main(const FString& Params) override;
};
