#include "Callbacks.h"
#include "Board.h"
#include "Config.h"
#include "Game.h"
#include "Globals.h"
#include "Texture.h"
#include <stdlib.h>

void C_RunAsConfig(DvarValue strvalue) {
	RunConfig(strvalue.string);
}

void C_CFGTexture(DvarValue strvalue) {
	for (int i = 0; i < TEX_COUNT; ++i) {
		glDeleteTextures(1, &g_textures[i].glid);
		g_textures[i].glid = g_textures[i].width = g_textures[i].height = 0;
	}

	C_RunAsConfig(strvalue);
}

void CLSetBGColour(const char **tokens, unsigned int count) {
	if (count < 3) return;

	glClearColor(atof(tokens[0]), atof(tokens[1]), atof(tokens[2]), 0.f);
}

#define IF_ID(STRING, ID) if (strcmp(STRING, ID) == 0)

inline int GetTexID(const char *string) {
			IF_ID(string, "font") return TEX_FONT;
	else	IF_ID(string, "block") return TEX_BLOCK;
	else	IF_ID(string, "ul") return TEX_UL;
	else	IF_ID(string, "u")	return TEX_U;
	else	IF_ID(string, "ur")	return TEX_UR;
	else	IF_ID(string, "l")	return TEX_L;
	else	IF_ID(string, "r")	return TEX_R;
	else	IF_ID(string, "bl")	return TEX_BL;
	else	IF_ID(string, "b")	return TEX_B;
	else	IF_ID(string, "br")	return TEX_BR;
	else	IF_ID(string, "bg") return TEX_BG;

	return -1;
}

inline bool ShouldDrawBorder() {
	if (g_textures[TEX_UL].glid != 0 || 
		g_textures[TEX_U].glid != 0 || 
		g_textures[TEX_UR].glid != 0 || 
		g_textures[TEX_L].glid != 0 ||
		g_textures[TEX_R].glid != 0 ||
		g_textures[TEX_BL].glid != 0 ||
		g_textures[TEX_B].glid != 0 ||
		g_textures[TEX_BR].glid != 0)
			return true;

	return false;
}

void SetTextIndexSize(int id, float size) {
	switch (id) {
	case TEX_FONT:
		g_font->char_size = size;
		g_menu_font->char_size = size;

		Font_RegenerateText(g_font);
		Font_RegenerateText(g_menu_font);
		break;

	case TEX_BLOCK:
		GameSetBlockIDSize(size);
		break;
	}
}

void CLSetTex(const char **tokens, unsigned int count) {
	if (count < 2) return;

	int id = GetTexID(tokens[0]);
	if (id < 0) return;

	TextureFromFile(tokens[1], g_textures + GetTexID(tokens[0]));

	if (id == TEX_BG) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	if (count > 2)
		SetTextIndexSize(id, atof(tokens[2]));
	else
		switch (id) {
		case TEX_FONT:
			Font_RegenerateText(g_font);
			Font_RegenerateText(g_menu_font);
			break;

		case TEX_BG:
			GameSizeUpdate(0, 0);
			break;
		}

	g_drawborder = ShouldDrawBorder();
}

void CLSetTexIndexSize(const char **tokens, unsigned int count) {
	if (count < 2) return;

	int id = GetTexID(tokens[0]);
	if (id < 0) return;

	SetTextIndexSize(id, atof(tokens[1]));
}
