#pragma once
#include "Timing.h"
#include <Windows.h>

UINT64 g_frequency = 1;
UINT64 g_last = 0;

void InitTimer() {
	LARGE_INTEGER value;
	QueryPerformanceFrequency(&value);
	g_frequency = value.QuadPart;
}

void StartTimer() {
	LARGE_INTEGER value;
	QueryPerformanceCounter(&value);
	g_last = value.QuadPart;
}

float GetDeltaTime() {
	LARGE_INTEGER value;
	QueryPerformanceCounter(&value);
	
	return (float)(value.QuadPart - g_last) / (float)g_frequency;
}
