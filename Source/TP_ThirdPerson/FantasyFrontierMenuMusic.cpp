#include "FantasyFrontierMenuMusic.h"

#include "Math/UnrealMathUtility.h"
#include "Sound/SoundGroups.h"

namespace
{
	constexpr int32 MenuMusicSampleRate = 48000;
	constexpr int32 MenuMusicChannels = 2;
	constexpr float TwoPi = 6.28318530718f;

	void WriteStereoSample(int16* PCM, int32 FrameIndex, float Left, float Right)
	{
		const float ClampedLeft = FMath::Clamp(Left, -0.98f, 0.98f);
		const float ClampedRight = FMath::Clamp(Right, -0.98f, 0.98f);

		PCM[FrameIndex * 2] = static_cast<int16>(ClampedLeft * 32767.0f);
		PCM[FrameIndex * 2 + 1] = static_cast<int16>(ClampedRight * 32767.0f);
	}
}

UFantasyFrontierMenuMusic::UFantasyFrontierMenuMusic(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NumChannels = MenuMusicChannels;
	SetSampleRate(MenuMusicSampleRate);
	Duration = 3600.0f;
	bLooping = true;
	SoundGroup = SOUNDGROUP_Music;
	NumBufferUnderrunSamples = 1024;
	NumSamplesToGeneratePerCallback = 4096;
}

int32 UFantasyFrontierMenuMusic::OnGeneratePCMAudio(TArray<uint8>& OutAudio, int32 NumSamples)
{
	OutAudio.SetNumUninitialized(NumSamples * sizeof(int16));
	int16* PCM = reinterpret_cast<int16*>(OutAudio.GetData());

	const int32 NumFrames = NumSamples / MenuMusicChannels;
	const float DeltaTime = 1.0f / static_cast<float>(MenuMusicSampleRate);

	for (int32 FrameIndex = 0; FrameIndex < NumFrames; ++FrameIndex)
	{
		const float Time = static_cast<float>(RunningTime);
		const float CycleTime = FMath::Fmod(Time, 24.0f);

		float Root = 73.42f;
		float Fifth = 110.0f;
		float High = 146.83f;
		if (CycleTime >= 8.0f && CycleTime < 16.0f)
		{
			Root = 82.41f;
			Fifth = 123.47f;
			High = 164.81f;
		}
		else if (CycleTime >= 16.0f)
		{
			Root = 98.0f;
			Fifth = 146.83f;
			High = 196.0f;
		}

		if (Time >= NextBellTime)
		{
			SpawnBell(Root, High);
			NextBellTime = Time + 3.75f + 0.85f * (0.5f + 0.5f * FMath::Sin(Time * 0.37f));
		}

		const float PadEnvelope = 0.72f + 0.28f * FMath::Sin(Time * 0.23f);
		const float Drone =
			0.26f * FMath::Sin(TwoPi * Root * Time) +
			0.19f * FMath::Sin(TwoPi * Fifth * Time + 0.8f) +
			0.16f * FMath::Sin(TwoPi * High * Time + 1.6f);
		const float Air =
			0.08f * FMath::Sin(TwoPi * (Root * 2.0f) * Time + FMath::Sin(Time * 0.41f) * 0.7f) +
			0.04f * FMath::Sin(TwoPi * (High * 2.5f) * Time + FMath::Cos(Time * 0.18f) * 0.9f);
		const float Sparkle =
			0.02f * FMath::Sin(TwoPi * 712.0f * Time) * (0.5f + 0.5f * FMath::Sin(Time * 0.91f)) +
			0.01f * FMath::Sin(TwoPi * 1180.0f * Time + 0.3f);

		float Left = (Drone * PadEnvelope + Air + Sparkle) * 0.34f;
		float Right = (Drone * PadEnvelope + Air - Sparkle * 0.5f) * 0.34f;

		for (int32 BellCursor = ActiveBells.Num() - 1; BellCursor >= 0; --BellCursor)
		{
			FBellVoice& Bell = ActiveBells[BellCursor];
			Bell.Age += DeltaTime;
			if (Bell.Age >= Bell.Lifetime)
			{
				ActiveBells.RemoveAtSwap(BellCursor, 1, EAllowShrinking::No);
				continue;
			}

			const float LifeAlpha = Bell.Age / Bell.Lifetime;
			const float Envelope = FMath::Exp(-3.6f * LifeAlpha) * FMath::Sin(FMath::Clamp(LifeAlpha, 0.0f, 1.0f) * PI);
			const float BellTone =
				FMath::Sin(TwoPi * Bell.Frequency * Bell.Age) +
				0.34f * FMath::Sin(TwoPi * Bell.Frequency * 2.01f * Bell.Age + 0.25f) +
				0.17f * FMath::Sin(TwoPi * Bell.Frequency * 3.02f * Bell.Age + 1.1f);
			const float BellSample = BellTone * Envelope * Bell.Gain;
			const float LeftPan = 0.5f - Bell.Pan * 0.35f;
			const float RightPan = 0.5f + Bell.Pan * 0.35f;
			Left += BellSample * LeftPan;
			Right += BellSample * RightPan;
		}

		WriteStereoSample(PCM, FrameIndex, Left, Right);
		RunningTime += DeltaTime;
	}

	return NumSamples;
}

void UFantasyFrontierMenuMusic::SpawnBell(float RootFrequency, float AccentFrequency)
{
	static const float BellPattern[] = { 2.0f, 2.5f, 3.0f, 4.0f, 3.0f, 2.5f };
	const float BaseFrequency = (BellIndex % 2 == 0) ? RootFrequency : AccentFrequency;

	FBellVoice Bell;
	Bell.Frequency = BaseFrequency * BellPattern[BellIndex % UE_ARRAY_COUNT(BellPattern)];
	Bell.Lifetime = 2.8f + 0.45f * ((BellIndex % 3) * 0.5f);
	Bell.Pan = -0.75f + 0.3f * static_cast<float>(BellIndex % 6);
	Bell.Gain = 0.18f + 0.02f * static_cast<float>(BellIndex % 4);
	ActiveBells.Add(Bell);
	++BellIndex;
}
