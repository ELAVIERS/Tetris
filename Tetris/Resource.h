#pragma once

/*
	Resource.h

	Defines resource IDs
*/

#define ID_VERSION			1
#define	IDI_ICON			101

#define ID_SHADER_FRAG		1001
#define ID_SHADER_VERT		1002
#define ID_SHADER_TEXTFRAG	1003

#include <Windows.h>

char *LoadStringResource(HINSTANCE instance, int id);
