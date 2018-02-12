#include "Callbacks.h"
#include "Board.h"
#include "Config.h"
#include "Globals.h"
#include "Texture.h"

void C_RunAsConfig(DvarValue strvalue) {
	RunConfig(strvalue.dstring);
}

void C_FontTexture(DvarValue strvalue) {
	TextureFromFile(strvalue.dstring, g_font->texture);
	Font_RegenerateText(g_font);
}

void C_MenuFontTexture(DvarValue strvalue) {
	TextureFromFile(strvalue.dstring, g_menu_font->texture);
	Font_RegenerateText(g_menu_font);
}

void C_FontIDSize(DvarValue fvalue) {
	g_font->char_size = (unsigned int)fvalue.dfloat;
	Font_RegenerateText(g_font);
}

void C_MenuFontIDSize(DvarValue fvalue) {
	g_menu_font->char_size = (unsigned int)fvalue.dfloat;
	Font_RegenerateText(g_menu_font);
}
