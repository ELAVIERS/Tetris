#pragma once

/*
	Timing.h

	High-resolution timer
*/

void TimerInit();

void TimerStart();

//Returns amount of seconds since TimerStart was called
float TimerDelta();
