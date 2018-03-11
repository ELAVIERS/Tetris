#pragma once
#include "Font.h"
#include "Matrix.h"
#include "Quad.h"
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

#define TEX_COUNT 11

#define TEX_FONT	0
#define TEX_BLOCK	1
#define	TEX_UL		2
#define TEX_U		3
#define TEX_UR		4
#define TEX_L		5
#define TEX_R		6
#define TEX_BL		7
#define TEX_B		8
#define TEX_BR		9
#define TEX_BG		10

bool			g_drawborder;
Texture			g_textures[TEX_COUNT];

Quad			g_defquad;

extern Font* const g_font;
extern Font* const g_menu_font;

void G_Init();
void G_Free();
