#pragma once
#include "Types.h"
#include "WaveFile.h"

typedef enum {
	SC_MUSIC,
	SC_GENERIC
} SoundCategory;

typedef struct WaveFileNode_s {
	WaveFile *wav;

	const char *name;
	
	float volume;
	SoundCategory category;

	struct WaveFileNode_s *next;
} WaveFileNode;

typedef struct WavePlayer_s {
	const WaveFileNode *wav;

	uint32 currentFrame;
	bool paused;
	bool loop;
	uint32 id;

	struct WavePlayer_s *next;
} WavePlayer;

bool SMStart(uint32 buffer_duration_millis);
WaveFileNode *SMGetWav(const char *filename);
uint32 SMPlaySound(const char *filename, bool loop);
void SMFeedBuffer(float deltaTime);
void SMClearSounds();
void SMStop();

bool SMSoundIsPlaying(uint32 id);
void SMStopSound(uint32 id);
void SMPauseSound(uint32 id);
void SMResumeSound(uint32 id);
