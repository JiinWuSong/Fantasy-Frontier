#pragma once

#include "CoreMinimal.h"
#include "Sound/SoundWaveProcedural.h"
#include "FantasyFrontierMenuMusic.generated.h"

UCLASS()
class UFantasyFrontierMenuMusic : public USoundWaveProcedural
{
	GENERATED_BODY()

public:
	UFantasyFrontierMenuMusic(const FObjectInitializer& ObjectInitializer);

	virtual int32 OnGeneratePCMAudio(TArray<uint8>& OutAudio, int32 NumSamples) override;

private:
	struct FBellVoice
	{
		float Frequency = 0.0f;
		float Age = 0.0f;
		float Lifetime = 0.0f;
		float Pan = 0.0f;
		float Gain = 0.0f;
	};

	void SpawnBell(float RootFrequency, float AccentFrequency);

	double RunningTime = 0.0;
	float NextBellTime = 1.25f;
	int32 BellIndex = 0;
	TArray<FBellVoice> ActiveBells;
};
