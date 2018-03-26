#pragma once
#include "Dvar.h"

typedef const float* FloatPtr;

FloatPtr		axis_down, axis_x, 
				bind_print, 
				sv_board_width, sv_board_height, 
				sv_gravity, sv_drop_gravity,
				sv_autorepeat, sv_autorepeat_delay, 
				sv_bag_size, sv_queue_size, sv_clears_per_level,
				sv_ghost, sv_hard_drop;

void CreateVariables();
