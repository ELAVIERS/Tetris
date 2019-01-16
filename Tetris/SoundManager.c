#include "SoundManager.h"
#include "Console.h"
#include "Error.h"
#include "Resample.h"
#include "Variables.h"
#include <Audioclient.h>
#include <mmdeviceapi.h>

WAVEFORMATEX wave_format = { 0 };

uint32 WAVDataTransfer(WavePlayer *player, byte *dest, uint32 dest_frames) {
	float vol = player->wav->volume * (*volume);
	if (player->wav->category == SC_MUSIC)
		vol *= (*volume_music);

	switch (player->wav->wav->bits_per_sample) {
	case 16:
	{
		uint32 written_frames = Resample16(player, (int16*)dest, wave_format.nSamplesPerSec, wave_format.nChannels, dest_frames, vol);

		while (player->loop && written_frames < dest_frames) {
			player->currentFrame = 0;
			written_frames += Resample16(player, (int16*)(dest + (written_frames * wave_format.nBlockAlign)), wave_format.nSamplesPerSec, wave_format.nChannels, dest_frames - written_frames, vol);
		}

		return written_frames;
	}

	default:
		ErrorMessage("This should not happen. Go and fix something in SoundManager.c or something idk");
		return 0;
	}
}


#define CHECK(result) if (!SUCCEEDED(result)) goto Finished

#define TRYRELEASE(noob) if (noob != NULL) noob->lpVtbl->Release(noob);

IMMDeviceEnumerator *enumerator = NULL;
IMMDevice *audio_device = NULL;
IAudioClient *audio_client = NULL;
WAVEFORMATEX *descriptor = NULL;
IAudioRenderClient *render_client = NULL;

UINT32 buffer_frame_count;

float time_to_next_buffer_feed = 0.f;

bool SMStart(uint32 buffer_duration_millis) {
	HRESULT result = CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, &IID_IMMDeviceEnumerator, (void**)&enumerator);
	CHECK(result);

	result = enumerator->lpVtbl->GetDefaultAudioEndpoint(enumerator, eRender, eMultimedia, &audio_device);
	CHECK(result);

	result = audio_device->lpVtbl->Activate(audio_device, &IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&audio_client);
	CHECK(result);

	REFERENCE_TIME defaultDevicePeriod, minimumDevicePeriod;

	result = audio_client->lpVtbl->GetDevicePeriod(audio_client, &defaultDevicePeriod, &minimumDevicePeriod);
	CHECK(result);

	const REFERENCE_TIME buffer_duration = max(buffer_duration_millis * 10000, minimumDevicePeriod);

	result = audio_client->lpVtbl->GetMixFormat(audio_client, &descriptor);
	CHECK(result);

	wave_format.wFormatTag = WAVE_FORMAT_PCM;
	wave_format.wBitsPerSample = 16;
	wave_format.nChannels = 2;
	wave_format.nSamplesPerSec = descriptor->nSamplesPerSec;
	wave_format.nBlockAlign = (wave_format.wBitsPerSample * wave_format.nChannels) / 8;
	wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec * wave_format.nBlockAlign;

	result = audio_client->lpVtbl->Initialize(audio_client, AUDCLNT_SHAREMODE_SHARED, 0, buffer_duration, 0 /*shared*/, &wave_format, NULL);
	CHECK(result);

	result = audio_client->lpVtbl->GetBufferSize(audio_client, &buffer_frame_count);
	CHECK(result);

	result = audio_client->lpVtbl->GetService(audio_client, &IID_IAudioRenderClient, (void**)&render_client);
	CHECK(result);

	result = audio_client->lpVtbl->Start(audio_client);
	CHECK(result);

	return true;

Finished:
	ErrorMessage("WASAPI init error");
	return false;
}

////////
WaveFileNode *first_wave_file = NULL;

WavePlayer *first_player = NULL;
uint32 next_id = 1;

WaveFileNode *SMGetWav(const char *filename) {
	if (filename == NULL)
		return NULL;
	
	if (first_wave_file) {
		WaveFileNode *wf = first_wave_file;

		do {
			if (strcmp(wf->name, filename) == 0)
				return wf;

			wf = wf->next;
		} while (wf);
	}

	WaveFile *wav = (WaveFile*)malloc(sizeof(WaveFile));
	if (LoadWAV(filename, wav)) {
		if (wav->channel_count == 1)
			WAVMonoToStereo(wav);

		if (wav->bits_per_sample != 16)
			ErrorMessage("Invalid bits per sample, must be 16");

		WaveFileNode *node = (WaveFileNode*)malloc(sizeof(WaveFileNode));
		node->wav = wav;
		node->next = NULL;
		node->name = filename;

		if (first_wave_file) {
			WaveFileNode *wf = first_wave_file;
			while (wf->next)
				wf = wf->next;

			wf->next = node;
		}
		else first_wave_file = node;

		return node;
	}
	else free(wav);

	return NULL;
}

uint32 SMPlaySound(const char *filename, bool loop) {
	WaveFileNode *wf = SMGetWav(filename);
	if (wf == NULL)
		return 0;

	time_to_next_buffer_feed = 0.f;

	WavePlayer *player = (WavePlayer*)malloc(sizeof(WavePlayer));

	player->wav = wf;
	player->paused = false;
	player->loop = loop;
	player->currentFrame = 0;
	player->next = NULL;
	player->id = next_id++;

	if (first_player) {
		WavePlayer *last_player = first_player;

		while (last_player->next)
			last_player = last_player->next;

		last_player->next = player;
	}
	else first_player = player;

	return player->id;
}

void SMFeedBuffer(float deltaTime) {
	
	if (first_player) {
		time_to_next_buffer_feed -= deltaTime;

		if (time_to_next_buffer_feed <= 0.f)
		{
			time_to_next_buffer_feed = buffer_frame_count / wave_format.nSamplesPerSec / 2.f;

			byte *buffer;

			UINT32 padding;
			HRESULT result = audio_client->lpVtbl->GetCurrentPadding(audio_client, &padding);
			CHECK(result);

			uint32 frames_available = buffer_frame_count - padding;

			if (frames_available != 0) {
				result = render_client->lpVtbl->GetBuffer(render_client, frames_available, &buffer);
				CHECK(result);

				ZeroMemory(buffer, frames_available * wave_format.nBlockAlign);

				uint32 max_frames_written = 0;
				WavePlayer *player = first_player;
				WavePlayer *prevPlayer = NULL;

				do {
					WavePlayer *nextPlayer = player->next;

					if (!player->paused) {
						uint32 frames_written = WAVDataTransfer(player, buffer, frames_available);

						if (frames_written == 0) {
							if (player == first_player)
								first_player = nextPlayer;
							else if (prevPlayer)
								prevPlayer->next = nextPlayer;
							else
								first_player = nextPlayer;

							free(player);
						}
						else
							prevPlayer = player;

						max_frames_written = max(max_frames_written, frames_written);
					}

					player = nextPlayer;
				} while (player);

				result = render_client->lpVtbl->ReleaseBuffer(render_client, max_frames_written, 0);
				CHECK(result);
			}
		}
	}

	return;

Finished:
	ErrorMessage("SMFeedBuffer error");
}

void SMClearSounds() {
	while (first_player) {
		WavePlayer *nextPlayer = first_player->next;
		free(first_player);

		first_player = nextPlayer;
	}
}

void SMStop() {
	if (audio_client) audio_client->lpVtbl->Stop(audio_client);

	CoTaskMemFree(descriptor);
	TRYRELEASE(render_client);
	TRYRELEASE(audio_client);
	TRYRELEASE(audio_device);
	TRYRELEASE(enumerator);

	
	while (first_wave_file) {
		WaveFileNode *nextNode = first_wave_file->next;
		free(first_wave_file->wav->data);
		free(first_wave_file->wav);
		free(first_wave_file);

		first_wave_file = nextNode;
	}
}

WavePlayer *GetWavPlayer(uint32 id) {
	if (id && first_player) {
		WavePlayer *player = first_player;

		while (player) {
			if (player->id == id)
				return player;

			player = player->next;
		}
	}

	return NULL;
}

bool SMSoundIsPlaying(uint32 id) {
	return GetWavPlayer(id) != NULL;
}

void SMStopSound(uint32 id) {
	if (first_player) {
		WavePlayer *prevPlayer = NULL;
		WavePlayer *player = first_player;

		while (player) {
			if (player->id == id) {
				if (prevPlayer == NULL)
					first_player = player->next;
				else
					prevPlayer->next = player->next;

				free(player);
				return;
			}

			player = player->next;
		}
	}
}

void SMPauseSound(uint32 id) {
	WavePlayer *player = GetWavPlayer(id);

	if (player) player->paused = true;
}

void SMResumeSound(uint32 id) {
	WavePlayer *player = GetWavPlayer(id);

	if (player) player->paused = false;
}
