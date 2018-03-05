#pragma once
#include "Font.h"
#include "Matrix.h"
#include "Texture.h"
#include <GL/glew.h>
#include <stdbool.h>
#include <Windows.h>

/*
	Globals.h

	Global variables
*/

HWND			g_hwnd;
HDC				g_devcontext;

bool			g_running;

bool			g_paused;

float			g_delta;

Mat3			g_projection;

GLuint			g_active_shader;

Texture			g_tex_font;
Texture			g_tex_menu_font;

extern Font* const g_font;
extern Font* const g_menu_font;

void G_Init();
void G_Free();
