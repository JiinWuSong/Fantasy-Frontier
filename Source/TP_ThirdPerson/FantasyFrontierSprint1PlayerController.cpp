#include "FantasyFrontierSprint1PlayerController.h"

#include "InputMappingContext.h"
#include "UObject/ConstructorHelpers.h"

AFantasyFrontierSprint1PlayerController::AFantasyFrontierSprint1PlayerController()
{
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> DefaultContextRef(TEXT("/Game/Input/IMC_Default.IMC_Default"));
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> MouseLookContextRef(TEXT("/Game/Input/IMC_MouseLook.IMC_MouseLook"));
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> CombatContextRef(TEXT("/Game/Variant_Combat/Input/IMC_Combat.IMC_Combat"));
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> PlatformingContextRef(TEXT("/Game/Variant_Platforming/Input/IMC_Platforming.IMC_Platforming"));

	if (DefaultContextRef.Succeeded())
	{
		DefaultMappingContexts.AddUnique(DefaultContextRef.Object);
	}

	if (MouseLookContextRef.Succeeded())
	{
		MobileExcludedMappingContexts.AddUnique(MouseLookContextRef.Object);
	}

	if (CombatContextRef.Succeeded())
	{
		MobileExcludedMappingContexts.AddUnique(CombatContextRef.Object);
	}

	if (PlatformingContextRef.Succeeded())
	{
		MobileExcludedMappingContexts.AddUnique(PlatformingContextRef.Object);
	}
}
