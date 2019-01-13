#pragma once
#include "SoundManager.h"


//returns number of FRAMES written to DEST!!!
uint32 Resample16(WavePlayer *src, int16 *dest, uint32 dest_rate, uint16 dest_channels, uint32 dest_frame_count, float mix);
