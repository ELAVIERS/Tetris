#pragma once
#include "Dvar.h"

typedef const float* FloatPtr;

FloatPtr		bind_print,
				cl_fast_music_zone,
				sv_paused,
				sv_board_width, sv_board_height, sv_board_real_height, 
				sv_gravity, sv_drop_gravity, sv_drop_gravity_is_factor,
				sv_autorepeat, sv_autorepeat_delay, 
				sv_bag_size, sv_queue_size, sv_clears_per_level,
				sv_ghost, sv_hard_drop, sv_hold,
				sv_drop_delay, sv_lock_delay,
				volume,
				volume_music;

void CreateVariables();

#define ValueAsFloatPtr(DVAR) &DVAR->value.number
