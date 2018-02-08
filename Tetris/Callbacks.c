#include "Callbacks.h"
#include "Config.h"
#include "Console.h"
#include "Bitmap.h"
#include "Globals.h"
#include "Texture.h"

inline void FontStrTexture(Font *font, const char *filepath) {
	Bitmap bmp;

	if (LoadBMP(filepath, &bmp)) {
		glDeleteTextures(1, &font->texture);
		font->texture_size = bmp.width;
		font->texture = TextureFromBMP(&bmp, true);
		Font_RegenerateText(font);
	}
}

void C_RunAsConfig(DvarValue strvalue) {
	RunConfig(strvalue.dstring);
}

void C_FontTexture(DvarValue strvalue) {
	FontStrTexture(g_font, strvalue.dstring);
}

void C_MenuFontTexture(DvarValue strvalue) {
	FontStrTexture(g_menu_font, strvalue.dstring);
}

void C_FontIDSize(DvarValue fvalue) {
	g_font->char_size = (unsigned int)fvalue.dfloat;
	Font_RegenerateText(g_font);
}

void C_MenuFontIDSize(DvarValue fvalue) {
	g_menu_font->char_size = (unsigned int)fvalue.dfloat;
	Font_RegenerateText(g_menu_font);
}

void C_BlockTexture(DvarValue strvalue) {
	Bitmap bmp_blocks;

	if (LoadBMP(strvalue.dstring, &bmp_blocks)) {
		glDeleteTextures(1, &g_tex_blocks);
		g_tex_blocks = TextureFromBMP(&bmp_blocks, true);
	}
}

void C_BlockIDSize(DvarValue fvalue) {

}
