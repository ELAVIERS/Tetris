#include "TextureManager.h"
#include "Font.h"
#include "Game.h"
#include "Globals.h"
#include "Quad.h"
#include <stdlib.h>

inline int GetTexID(const char *string) {
#define IF_ID(STRING, ID) if (strcmp(STRING, ID) == 0)

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

void SetTexIndexSize(int id, float size) {
	switch (id) {
	case TEX_FONT:
		g_font.char_size = size;

		Font_RegenerateText(&g_font);
		QuadSetData(g_quads + QUAD_FONT, size / (float)g_font.texture->width, size / (float)g_font.texture->height);
		break;

	case TEX_BLOCK:
		GameSetBlockIDSize(size);
		break;
	}
}

void CLSetTextureIndexSize(const char **tokens, unsigned int count) {
	if (count < 2) return;

	int id = GetTexID(tokens[0]);
	if (id < 0) return;

	SetTexIndexSize(id, (float)atof(tokens[1]));
}

void CLSetTexture(const char **tokens, unsigned int count) {
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
		SetTexIndexSize(id, (float)atof(tokens[2]));

	switch (id) {
	case TEX_FONT:
		if (count < 3)
			SetTexIndexSize(id, g_font.char_size);
		break;

	case TEX_BG:
		GameSizeUpdate(0, 0);
		break;
	}

	bool prev = g_drawborder;
	g_drawborder = ShouldDrawBorder();
	if (g_drawborder != prev)
		GameSizeUpdate(0, 0);
}

