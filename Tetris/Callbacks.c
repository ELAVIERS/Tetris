#include "Callbacks.h"
#include "Board.h"
#include "Config.h"
#include "Game.h"
#include "Globals.h"
#include "Texture.h"

void C_RunAsConfig(DvarValue strvalue) {
	RunConfig(strvalue.string);
}

void C_CLFontTexture(DvarValue strvalue) {
	TextureFromFile(strvalue.string, g_font->texture);
	Font_RegenerateText(g_font);
}

void C_CLMenuFontTexture(DvarValue strvalue) {
	TextureFromFile(strvalue.string, g_menu_font->texture);
	Font_RegenerateText(g_menu_font);
}

void C_CLFontIDSize(DvarValue fvalue) {
	g_font->char_size = (unsigned int)fvalue.number;
	Font_RegenerateText(g_font);
}

void C_CLMenuFontIDSize(DvarValue fvalue) {
	g_menu_font->char_size = (unsigned int)fvalue.number;
	Font_RegenerateText(g_menu_font);
}
