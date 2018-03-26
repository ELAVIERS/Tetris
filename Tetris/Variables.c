#include "Variables.h"
#include "Config.h"
#include "Console.h"
#include "Game.h"
#include "Globals.h"
#include "Messaging.h"
#include "Server.h"
#include <stdlib.h>

/*
	Texture stuff
*/

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

void SetTextIndexSize(int id, float size) {
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

void FUNC_SetTextureIndexSize(const char **tokens, unsigned int count) {
	if (count < 2) return;

	int id = GetTexID(tokens[0]);
	if (id < 0) return;

	SetTextIndexSize(id, (float)atof(tokens[1]));
}

void FUNC_SetTexture(const char **tokens, unsigned int count) {
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
		SetTextIndexSize(id, (float)atof(tokens[2]));

	switch (id) {
		case TEX_FONT:
			if (count < 3)
				SetTextIndexSize(id, g_font.char_size);
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

/////////
//////////
////////////

void C_Name(DvarValue strvalue) {
	MessageServerString(SVMSG_NAME, strvalue.string);
}

void C_RunAsConfig(DvarValue strvalue) {
	RunConfig(strvalue.string);
}

void C_QueueSize(DvarValue floatvalue) {
	GameSetQueueLength((byte)floatvalue.number);

	GameSizeUpdate(0, 0);
}

void C_BagSize(DvarValue floatvalue) {
	GameSetBagSize((byte)floatvalue.number);
}

void FUNC_AddAdmin(const char **tokens, unsigned int count) {
	if (count == 0) return;
	ServerSetAdmin(atoi(tokens[0]));
}

void FUNC_Exit() {
	g_running = false;
}

void FUNC_Run(const char **tokens, unsigned int count) {
	if (count == 0) return;
	if (!RunConfig(tokens[0]))
		ConsolePrint("File not found\n");
}

void FUNC_Send(const char **tokens, unsigned int count) {
	if (count < 1) return;
	char message[MSG_LEN];

	unsigned int current = 0;
	for (unsigned int i = 0; i < count; ++i) {
		if (i > 0) message[current++] = ' ';

		for (const char *c = tokens[i]; *c != '\0'; ++c)
			message[current++] = *c;
	}

	message[current] = '\0';
	MessageServerString(SVMSG_CHAT, message);
}

void FUNC_Size(const char **tokens, unsigned int count) {
	if (count < 2) return;
	SetWindowPos(g_hwnd, NULL, 0, 0, atoi(tokens[0]), atoi(tokens[1]), 0);
}

void FUNC_SetBGColour(const char **tokens, unsigned int count) {
	if (count < 3) return;
	glClearColor((float)atof(tokens[0]), (float)atof(tokens[1]), (float)atof(tokens[2]), 0.f);
}

#include "BlockManager.h"
#include "RNG.h"
#include <stdio.h>

void DBGCreateBag() {
	ConsolePrint("{");

	unsigned int typecount = BlockTypesGetCount();
	byte *bag = (byte*)malloc(typecount);
	GenerateBag(bag, typecount);

	char string[13];

	for (unsigned int i = 0; i < typecount; ++i) {
		if (i) ConsolePrint(", ");
		snprintf(string, 13, "%d", bag[i]);
		ConsolePrint(string);
	}

	ConsolePrint("}\n");
}

#include "Board.h"
#include "InputManager.h"
#include "Lobby.h"

#define ValueAsFloatPtr(DVAR) &DVAR->value.number

void CreateVariables() {
	AddCvar(AddDStringC("cfg_texture", "", C_RunAsConfig, false));

	AddCvar(AddDStringC("name", "Player", C_Name, false));

	////
	AddDFunction("sv_admin_add", FUNC_AddAdmin, true);
	AddDFunction("run", FUNC_Run, false);
	AddDCall("exit", FUNC_Exit, false);
	AddDCall("save", SaveCvars, false);
	AddDFunction("say", FUNC_Send, false);
	AddDFunction("size", FUNC_Size, false);

	AddDCall("lobby", LobbyShow, false);

	AddDFunction("bind", Bind, false);
	AddDFunction("bindaxis", BindAxis, false);
	AddDCall("clear_binds", ClearBinds, false);
	bind_print = ValueAsFloatPtr(AddDFloat("bind_print", 0.f, false));

	AddDFloat("sv_playercount", 4, true);
	AddDString("sv_port", "7777", true);

	AddDFunction("sv_blocks_add", SVAddBlock, true);
	AddDCall("sv_blocks_clear", ClearBlocks, true);

	AddDFunction("cl_setbgcolour", FUNC_SetBGColour, false);

	AddDFunction("cl_blockids_add", CLAddTextureLevel, false);
	AddDFunction("cl_blockid_order", CLSetTextureIndexOrder, false);
	AddDCall("cl_blockids_clear", ClearTextureLevels, false);

	AddDFunction("cl_set_tex", FUNC_SetTexture, false);
	AddDFunction("cl_set_texid_size", FUNC_SetTextureIndexSize, false);
	AddDCall("cl_textures_clear", G_ClearTextures, false);

	sv_bag_size =			ValueAsFloatPtr(AddDFloatC("sv_bag_size", 0, C_BagSize, true));
	sv_queue_size =			ValueAsFloatPtr(AddDFloatC("sv_queue_size", 4, C_QueueSize, true));
	
	sv_gravity =			ValueAsFloatPtr(AddDFloat("sv_gravity", 0.5f, true));
	sv_drop_gravity =		ValueAsFloatPtr(AddDFloat("sv_drop_gravity", 0.05f, true));
	sv_autorepeat =			ValueAsFloatPtr(AddDFloat("sv_autorepeat", 0.05f, true));
	sv_autorepeat_delay =	ValueAsFloatPtr(AddDFloat("sv_autorepeat_delay", 0.25f, true));

	sv_board_width =		ValueAsFloatPtr(AddDFloat("sv_board_width", 10, true));
	sv_board_height =		ValueAsFloatPtr(AddDFloat("sv_board_height", 20, true));

	sv_clears_per_level =	ValueAsFloatPtr(AddDFloat("sv_clears_per_level", 10, true));

	sv_ghost =				ValueAsFloatPtr(AddDFloat("sv_ghost", 1.f, true));
	sv_hard_drop =			ValueAsFloatPtr(AddDFloat("sv_hard_drop", 1.f, true));

	axis_x =				ValueAsFloatPtr(AddDFloat("axis_x", 0, false));
	axis_down =				ValueAsFloatPtr(AddDFloat("axis_down", 0, false));

	AddDCall("dbg_create_bag", DBGCreateBag, false);
}
