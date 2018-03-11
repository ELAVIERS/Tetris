#include "Globals.h"

Font fonts[2];

Font* const g_font = fonts + 0;
Font* const g_menu_font = fonts + 1;

void G_Init() {
	QuadCreate(&g_defquad);
	QuadSetData(&g_defquad, 1.f, 1.f);

	g_font->texture = g_textures + TEX_FONT;
	g_menu_font->texture = g_textures + TEX_FONT;
}

void G_Free() {
	Font_Free(g_font);
	Font_Free(g_menu_font);

	for (unsigned int i = 0; i < TEX_COUNT; ++i)
		glDeleteTextures(1, &g_textures->glid);

	QuadDelete(&g_defquad);
}
