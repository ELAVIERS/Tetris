#include "Globals.h"

void G_Init() {
	for (unsigned int i = 0; i < QUAD_COUNT; ++i)
		QuadCreate(g_quads + i);

	QuadSetData(g_quads + QUAD_SINGLE, 1, 1);

	g_font.texture = g_textures + TEX_FONT;
}

void G_Free() {
	Font_Free(&g_font);

	for (unsigned int i = 0; i < TEX_COUNT; ++i)
		glDeleteTextures(1, &g_textures->glid);

	for (unsigned int i = 0; i < QUAD_COUNT; ++i)
		QuadDelete(g_quads + i);
}

void G_ClearTextures() {
	for (int i = 0; i < TEX_COUNT; ++i) {
		glDeleteTextures(1, &g_textures[i].glid);
		g_textures[i].glid = g_textures[i].width = g_textures[i].height = 0;
	}
}
