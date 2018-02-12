#pragma once
#include "Board.h"
#include "Font.h"
#include "Texture.h"
#include <GL/glew.h>
#include <stdbool.h>

bool			g_running;
bool			g_menu_active;
unsigned int	g_width, g_height;
float			g_delta;

GLuint			g_active_shader;

Texture			g_tex_font;
Texture			g_tex_menu_font;

extern Font* const g_font;
extern Font* const g_menu_font;

Board *g_board;

void G_Init();
void G_Free();

void TEMP_UpdateBoardSize();
