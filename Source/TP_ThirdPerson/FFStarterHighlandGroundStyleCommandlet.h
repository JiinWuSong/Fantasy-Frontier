#pragma once

#include "Commandlets/Commandlet.h"
#include "FFStarterHighlandGroundStyleCommandlet.generated.h"

UCLASS()
class TP_THIRDPERSON_API UFFStarterHighlandGroundStyleCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UFFStarterHighlandGroundStyleCommandlet();

	virtual int32 Main(const FString& Params) override;
};
