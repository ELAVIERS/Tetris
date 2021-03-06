#pragma once
#include "Font.h"
#include "GL.h"
#include "Matrix.h"
#include "Quad.h"
#include "Texture.h"
#include <stdbool.h>
#include <Windows.h>

/*
	Globals.h

	Global variables
*/

uint32 g_music_id;

bool			g_in_game;
bool			g_in_menu;

HWND			g_hwnd;
HDC				g_devcontext;

bool			g_running;

float			g_delta;

Mat3			g_projection;

GLuint			g_active_shader;

#define TEX_COUNT	11
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

typedef char * CharPtr;

struct {
	CharPtr		mus1, 
				mus2, 
				mus3, 
				mus1f, 
				mus2f, 
				mus3f, 
		
				move,
				rotate,
				lock,
				clear,
				clear4,
				backtoback,
				perfectclear,
		
				levelup,
		
				gameover;

} g_audio;

#define QUAD_COUNT	3
#define QUAD_SINGLE 0
#define QUAD_FONT	1
#define QUAD_BLOCK	2

Quad			g_quads[QUAD_COUNT];

Font g_font;

void G_Init();
void G_Free();

void G_ClearTextures();
