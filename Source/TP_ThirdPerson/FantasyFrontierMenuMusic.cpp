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
	constexpr float BeatLength = 60.0f / 98.0f;

	struct FChord
	{
		float Root;
		float Third;
		float Fifth;
		float Sixth;
	};

	static const FChord Progression[] =
	{
		{ 196.00f, 246.94f, 293.66f, 329.63f }, // G major
		{ 146.83f, 185.00f, 220.00f, 246.94f }, // D major
		{ 164.81f, 196.00f, 246.94f, 293.66f }, // E minor 7 feel
		{ 130.81f, 164.81f, 196.00f, 220.00f }, // C major
	};

	static const int32 ArpeggioPattern[] = { 0, 1, 2, 1, 3, 2 };
	static const float MelodyPattern[] = { 392.00f, 440.00f, 493.88f, 587.33f, 493.88f, 440.00f, 392.00f, 369.99f, 392.00f, 440.00f, 493.88f, 659.25f };

	for (int32 FrameIndex = 0; FrameIndex < NumFrames; ++FrameIndex)
	{
		const float Time = static_cast<float>(RunningTime);
		const float SectionTime = FMath::Fmod(Time, BeatLength * 24.0f);
		const int32 ChordIndex = static_cast<int32>(SectionTime / (BeatLength * 6.0f)) % UE_ARRAY_COUNT(Progression);
		const FChord& Chord = Progression[ChordIndex];
		const float ChordLocalTime = FMath::Fmod(SectionTime, BeatLength * 6.0f);
		const float BeatLocalTime = FMath::Fmod(SectionTime, BeatLength);
		const float BeatAlpha = BeatLocalTime / BeatLength;
		const int32 ArpIndex = static_cast<int32>(ChordLocalTime / BeatLength) % UE_ARRAY_COUNT(ArpeggioPattern);
		const float ArpFrequency =
			ArpeggioPattern[ArpIndex] == 0 ? Chord.Root :
			ArpeggioPattern[ArpIndex] == 1 ? Chord.Third :
			ArpeggioPattern[ArpIndex] == 2 ? Chord.Fifth :
			Chord.Sixth;
		const int32 MelodyIndex = static_cast<int32>(SectionTime / (BeatLength * 2.0f)) % UE_ARRAY_COUNT(MelodyPattern);
		const float MelodyFrequency = MelodyPattern[MelodyIndex];

		if (Time >= NextBellTime)
		{
			SpawnBell(Chord.Fifth, MelodyFrequency);
			NextBellTime = Time + BeatLength * 4.0f;
		}

		const float PadEnvelope = 0.88f + 0.12f * FMath::Sin(Time * 0.16f);
		const float Pad =
			0.08f * FMath::Sin(TwoPi * Chord.Root * Time + 0.12f) +
			0.08f * FMath::Sin(TwoPi * Chord.Third * Time + 0.48f) +
			0.06f * FMath::Sin(TwoPi * Chord.Fifth * Time + 0.94f) +
			0.04f * FMath::Sin(TwoPi * Chord.Sixth * Time + 1.4f);
		const float Breeze =
			0.022f * FMath::Sin(TwoPi * (Chord.Root * 2.0f) * Time + FMath::Sin(Time * 0.24f) * 0.18f) +
			0.014f * FMath::Sin(TwoPi * (Chord.Fifth * 2.0f) * Time + 0.72f);
		const float HarpEnvelope = FMath::Exp(-4.2f * BeatAlpha) * FMath::Min(1.0f, BeatLocalTime * 18.0f);
		const float Harp =
			(
				FMath::Sin(TwoPi * ArpFrequency * Time) +
				0.24f * FMath::Sin(TwoPi * ArpFrequency * 2.0f * Time + 0.18f) +
				0.07f * FMath::Sin(TwoPi * ArpFrequency * 3.0f * Time + 0.54f)
			) * HarpEnvelope * 0.23f;
		const float MelodyPhase = FMath::Fmod(SectionTime, BeatLength * 2.0f);
		const float MelodyEnvelope = FMath::Sin(FMath::Clamp(MelodyPhase / (BeatLength * 2.0f), 0.0f, 1.0f) * PI) * (0.68f + 0.32f * FMath::Sin(Time * 0.26f));
		const float FluteLead =
			(
				FMath::Sin(TwoPi * MelodyFrequency * Time + FMath::Sin(Time * 2.8f) * 0.010f) +
				0.10f * FMath::Sin(TwoPi * MelodyFrequency * 2.0f * Time + 0.32f)
			) * MelodyEnvelope * 0.12f;
		const float WarmDrone =
			(
				FMath::Sin(TwoPi * (Chord.Root * 0.5f) * Time) +
				0.06f * FMath::Sin(TwoPi * Chord.Root * Time + 0.24f)
			) * 0.026f;
		const float Sparkle =
			0.0048f * FMath::Sin(TwoPi * 1046.5f * Time) * (0.45f + 0.55f * FMath::Sin(Time * 0.92f));

		const float MandolinPulse =
			(FMath::Sin(TwoPi * (ArpFrequency * 1.5f) * Time) + 0.18f * FMath::Sin(TwoPi * (ArpFrequency * 3.0f) * Time + 0.24f))
			* HarpEnvelope * 0.09f;

		float Left = (Pad * PadEnvelope + Breeze + Harp + MandolinPulse + FluteLead + WarmDrone + Sparkle) * 0.58f;
		float Right = (Pad * PadEnvelope + Breeze + Harp * 0.95f + MandolinPulse * 0.92f + FluteLead + WarmDrone - Sparkle * 0.30f) * 0.58f;

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
			const float Envelope = FMath::Exp(-3.2f * LifeAlpha) * FMath::Sin(FMath::Clamp(LifeAlpha, 0.0f, 1.0f) * PI);
			const float BellTone =
				FMath::Sin(TwoPi * Bell.Frequency * Bell.Age) +
				0.24f * FMath::Sin(TwoPi * Bell.Frequency * 2.01f * Bell.Age + 0.25f) +
				0.12f * FMath::Sin(TwoPi * Bell.Frequency * 3.02f * Bell.Age + 1.1f);
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
	static const float BellPattern[] = { 2.0f, 2.5f, 3.0f, 3.5f, 2.0f, 4.0f };
	const float BaseFrequency = (BellIndex % 2 == 0) ? RootFrequency : AccentFrequency;

	FBellVoice Bell;
	Bell.Frequency = BaseFrequency * BellPattern[BellIndex % UE_ARRAY_COUNT(BellPattern)];
	Bell.Lifetime = 1.10f + 0.10f * static_cast<float>(BellIndex % 3);
	Bell.Pan = -0.35f + 0.14f * static_cast<float>(BellIndex % 6);
	Bell.Gain = 0.022f + 0.006f * static_cast<float>(BellIndex % 4);
	ActiveBells.Add(Bell);
	++BellIndex;
}
