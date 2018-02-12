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

void TEMP_UpdateBoardSize() {
	if (g_board) {
		g_board->height = g_height;
		g_board->width = g_board->height / g_board->rows * g_board->columns;
		g_board->x = g_width / 2 - g_board->width / 2;
	}
}
