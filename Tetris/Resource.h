#pragma once

/*
	Resource.h

	Defines resource IDs
*/

#define	IDI_ICON		101

#define SHADER_FRAG		1001
#define SHADER_VERT		1002
#define SHADER_TEXTFRAG 1003

#include <Windows.h>

char *LoadStringResource(HINSTANCE instance, int id);
