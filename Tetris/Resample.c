#include "Types.h"
#include "SoundManager.h"

#define LERP(FROM, TO, ALPHA) (((FROM) * (1 - (ALPHA))) + ((TO) * (ALPHA)))

uint32 Resample16(WavePlayer *src, int16 *dest, uint32 dest_rate, uint16 dest_channels, uint32 dest_frames, float mix) {
	int16 * const src_data = (int16*)src->wav->wav->data;
	const float src_frames_per_dest_frame = (float)src->wav->wav->sample_rate / (float)dest_rate;
	const float dest_frames_per_src_frame = (float)dest_rate / (float)src->wav->wav->sample_rate;

	//dest frames that we will write 
	uint32 dest_frames_to_write = (uint32)((float)((src->wav->wav->data_size / src->wav->wav->block_align) - src->currentFrame) * dest_frames_per_src_frame);
	if (dest_frames < dest_frames_to_write)
		dest_frames_to_write = dest_frames;

	uint16 src_channels = src->wav->wav->channel_count;
	float first_src_index = (float)(src->currentFrame * src_channels);

	//i is the FRAME INDEX
	for (uint32 i = 0; i < dest_frames_to_write; ++i) {
		//t = first read frame + i src frames
		float t = first_src_index + ((i * src_channels) * src_frames_per_dest_frame);
		uint32 src_frame_index = (uint32)t;
		float alpha = t - (float)src_frame_index;

		for (uint16 channel = 0; channel < dest_channels; ++channel) {
			//dest sample
			uint32 dest_index = (i * dest_channels) + channel;

			//amount = dest sample + mixed src sample at time
			float amount;
			
			if (i + 1 < dest_frames_to_write)
				amount = (float)dest[dest_index] + (mix * LERP(src_data[src_frame_index + channel], src_data[src_frame_index + channel + src_channels], alpha));
			else
				amount = (float)dest[dest_index] + (mix * src_data[src_frame_index + channel]);
			 
			 if (amount > 32767.f)
					amount = 32767.f;
			 else if (amount < -32768.f)
				 amount = -32768.f;

			 dest[dest_index] = (int16)amount;
		}
	}

	src->currentFrame += (uint32)(dest_frames_to_write * src_frames_per_dest_frame);
	return dest_frames_to_write;
}
