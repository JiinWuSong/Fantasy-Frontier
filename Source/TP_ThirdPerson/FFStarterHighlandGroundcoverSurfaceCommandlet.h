#pragma once

#include "Commandlets/Commandlet.h"
#include "FFStarterHighlandGroundcoverSurfaceCommandlet.generated.h"

UCLASS()
class UFFStarterHighlandGroundcoverSurfaceCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UFFStarterHighlandGroundcoverSurfaceCommandlet();

	virtual int32 Main(const FString& Params) override;
};
