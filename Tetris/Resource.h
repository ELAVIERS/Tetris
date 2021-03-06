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

#define ID_DIALOG_CONNECT	1069
#define IDC_IPADDRESS		1070
#define IDC_PORT			1071

#include <Windows.h>

//Gets a string from resource
char *LoadStringResource(HINSTANCE instance, int id);
