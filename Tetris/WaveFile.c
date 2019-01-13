#include "WaveFile.h"
#include "Console.h"
#include "IO.h"
#include <stdio.h>

bool LoadWAV(const char *filepath, WaveFile *out_wav) {
	FILE* file;
	fopen_s(&file, filepath, "rb");

	if (!file) {
		ConsolePrint("Error : could not load wave file \"");
		ConsolePrint(filepath);
		ConsolePrint("\"\n");
		return false;
	}

	if (!(fgetc(file) == 'R' && fgetc(file) == 'I' && fgetc(file) == 'F' && fgetc(file) == 'F'))
		return false;

	uint32 chunkSize = Read4B(file);

	if (!(fgetc(file) == 'W' && fgetc(file) == 'A' && fgetc(file) == 'V' && fgetc(file) == 'E'))
		return false;

	if (!(fgetc(file) == 'f' && fgetc(file) == 'm' && fgetc(file) == 't' && fgetc(file) == ' '))
		return false;

	uint32 fmtChunkSize = Read4B(file);
	out_wav->format = Read2B(file);
	out_wav->channel_count = Read2B(file);
	out_wav->sample_rate = Read4B(file);
	out_wav->byte_rate = Read4B(file);
	out_wav->block_align = Read2B(file);
	out_wav->bits_per_sample = Read2B(file);
	out_wav->data = NULL;

	if (!(fgetc(file) == 'd' && fgetc(file) == 'a' && fgetc(file) == 't' && fgetc(file) == 'a'))
		return false;

	out_wav->data_size = Read4B(file);
	out_wav->data = malloc(out_wav->data_size);
	size_t bytes_read = fread(out_wav->data, 1, out_wav->data_size, file);

	return true;
}

void WAVMonoToStereo(WaveFile *wav) {
	if (wav->channel_count != 1)
		return;

	uint32 new_size = wav->data_size * 2;
	byte *new_data = malloc(new_size);

	byte sampleSize = wav->bits_per_sample / 8;

	for (uint32 i = 0; i < wav->data_size; i += sampleSize) {
		CopyMemory(&new_data[i * 2],				&wav->data[i], sampleSize);
		CopyMemory(&new_data[(i * 2) + sampleSize],	&wav->data[i], sampleSize);
	}

	wav->data = new_data;
	wav->data_size = new_size;
	wav->channel_count = 2;
	wav->block_align *= 2;
	wav->byte_rate *= 2;
}
