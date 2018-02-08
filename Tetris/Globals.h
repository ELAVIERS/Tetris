#pragma once
#include "Font.h"
#include <GL/glew.h>
#include <stdbool.h>

bool			g_running;
unsigned int	g_width, g_height;
float			g_delta;
GLuint			g_tex_blocks;

extern Font* const g_font;
extern Font* const g_menu_font;
