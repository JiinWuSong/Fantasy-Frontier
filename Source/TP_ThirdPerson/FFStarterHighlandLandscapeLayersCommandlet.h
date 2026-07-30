#pragma once

#include "Commandlets/Commandlet.h"
#include "FFStarterHighlandLandscapeLayersCommandlet.generated.h"

UCLASS()
class UFFStarterHighlandLandscapeLayersCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UFFStarterHighlandLandscapeLayersCommandlet();

	virtual int32 Main(const FString& Params) override;
};
