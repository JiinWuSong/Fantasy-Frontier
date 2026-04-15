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
	constexpr float BeatLength = 60.0f / 96.0f;

	struct FChord
	{
		float Root;
		float Third;
		float Fifth;
		float Accent;
	};

	static const FChord Progression[] =
	{
		{ 73.42f, 92.50f, 110.00f, 146.83f }, // D major
		{ 98.00f, 123.47f, 146.83f, 164.81f }, // G major
		{ 61.74f, 73.42f, 92.50f, 123.47f }, // B minor
		{ 55.00f, 69.30f, 82.41f, 110.00f }, // A major
	};

	static const int32 ArpeggioPattern[] = { 0, 1, 2, 1 };
	static const float MelodyPattern[] = { 146.83f, 164.81f, 185.00f, 164.81f, 146.83f, 123.47f, 110.00f, 123.47f };

	for (int32 FrameIndex = 0; FrameIndex < NumFrames; ++FrameIndex)
	{
		const float Time = static_cast<float>(RunningTime);
		const float SectionTime = FMath::Fmod(Time, BeatLength * 16.0f);
		const int32 ChordIndex = static_cast<int32>(SectionTime / (BeatLength * 4.0f)) % UE_ARRAY_COUNT(Progression);
		const FChord& Chord = Progression[ChordIndex];
		const float ChordLocalTime = FMath::Fmod(SectionTime, BeatLength * 4.0f);
		const float BeatLocalTime = FMath::Fmod(SectionTime, BeatLength);
		const float BeatAlpha = BeatLocalTime / BeatLength;
		const int32 ArpIndex = static_cast<int32>(ChordLocalTime / BeatLength) % UE_ARRAY_COUNT(ArpeggioPattern);
		const float ArpFrequency =
			ArpeggioPattern[ArpIndex] == 0 ? Chord.Root :
			ArpeggioPattern[ArpIndex] == 1 ? Chord.Third :
			Chord.Fifth;
		const int32 MelodyIndex = static_cast<int32>(SectionTime / (BeatLength * 2.0f)) % UE_ARRAY_COUNT(MelodyPattern);
		const float MelodyFrequency = MelodyPattern[MelodyIndex];

		if (Time >= NextBellTime)
		{
			SpawnBell(Chord.Fifth, MelodyFrequency);
			NextBellTime = Time + BeatLength * 2.0f;
		}

		const float PadEnvelope = 0.78f + 0.22f * FMath::Sin(Time * 0.18f);
		const float Pad =
			0.15f * FMath::Sin(TwoPi * Chord.Root * Time + 0.1f) +
			0.12f * FMath::Sin(TwoPi * Chord.Third * Time + 0.5f) +
			0.10f * FMath::Sin(TwoPi * Chord.Fifth * Time + 1.0f);
		const float Air =
			0.04f * FMath::Sin(TwoPi * (Chord.Root * 2.0f) * Time + FMath::Sin(Time * 0.32f) * 0.25f) +
			0.03f * FMath::Sin(TwoPi * (Chord.Accent * 2.0f) * Time + 0.65f);
		const float PluckEnvelope = FMath::Exp(-5.6f * BeatAlpha) * FMath::Min(1.0f, BeatLocalTime * 18.0f);
		const float Arpeggio =
			(
				FMath::Sin(TwoPi * ArpFrequency * Time) +
				0.34f * FMath::Sin(TwoPi * ArpFrequency * 2.0f * Time + 0.24f) +
				0.11f * FMath::Sin(TwoPi * ArpFrequency * 3.0f * Time + 0.9f)
			) * PluckEnvelope * 0.26f;
		const float MelodyPhase = FMath::Fmod(SectionTime, BeatLength * 2.0f);
		const float MelodyEnvelope = FMath::Sin(FMath::Clamp(MelodyPhase / (BeatLength * 2.0f), 0.0f, 1.0f) * PI);
		const float Melody =
			(
				FMath::Sin(TwoPi * MelodyFrequency * Time + FMath::Sin(Time * 5.1f) * 0.02f) +
				0.18f * FMath::Sin(TwoPi * MelodyFrequency * 2.0f * Time + 0.4f)
			) * MelodyEnvelope * 0.08f;
		const float Sparkle =
			0.006f * FMath::Sin(TwoPi * 880.0f * Time) * (0.45f + 0.55f * FMath::Sin(Time * 0.7f));

		float Left = (Pad * PadEnvelope + Air + Arpeggio + Melody + Sparkle) * 0.52f;
		float Right = (Pad * PadEnvelope + Air + Arpeggio * 0.96f + Melody - Sparkle * 0.4f) * 0.52f;

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
	static const float BellPattern[] = { 2.0f, 2.5f, 3.0f, 2.0f, 4.0f, 3.0f };
	const float BaseFrequency = (BellIndex % 2 == 0) ? RootFrequency : AccentFrequency;

	FBellVoice Bell;
	Bell.Frequency = BaseFrequency * BellPattern[BellIndex % UE_ARRAY_COUNT(BellPattern)];
	Bell.Lifetime = 2.2f + 0.30f * ((BellIndex % 3) * 0.5f);
	Bell.Pan = -0.45f + 0.18f * static_cast<float>(BellIndex % 6);
	Bell.Gain = 0.10f + 0.015f * static_cast<float>(BellIndex % 4);
	ActiveBells.Add(Bell);
	++BellIndex;
}
