#pragma once
#include "Types.h"
#include <stdbool.h>

typedef struct
{
	uint16 format;
	uint16 channel_count;
	uint32 sample_rate;
	uint32 byte_rate;
	uint16 block_align;			//bytes per sample
	uint16 bits_per_sample;

	byte *data;
	uint32 data_size;
} WaveFile;

bool LoadWAV(const char *filepath, WaveFile *out_wav);

void WAVMonoToStereo(WaveFile *wav);
