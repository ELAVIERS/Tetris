#include "Globals.h"

Font fonts[2];

Font* const g_font = fonts + 0;
Font* const g_menu_font = fonts + 1;

void G_Init() {
	g_font->texture = &g_tex_font;
	g_menu_font->texture = &g_tex_menu_font;
}

void G_Free() {
	Font_Free(g_font);
	Font_Free(g_menu_font);

	glDeleteTextures(1, &g_tex_font.glid);
	glDeleteTextures(1, &g_tex_menu_font.glid);
}
