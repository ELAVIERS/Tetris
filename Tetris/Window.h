#pragma once
#include <Windows.h>

/*
	Window.h

	Just functions that do stuff with the game window
*/

//Creates g_hwnd
void InitWindow(HINSTANCE instance);

void FullscreenToggle();
