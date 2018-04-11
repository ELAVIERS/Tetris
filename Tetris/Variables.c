#include "Variables.h"
#include "Config.h"
#include "Console.h"
#include "Game.h"
#include "Globals.h"
#include "LevelManager.h"
#include "Messaging.h"
#include "Server.h"
#include "TextureManager.h"
#include <stdlib.h>

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

	AddDFunction("sv_bindlevel", AddLevelBind, true);
	AddDCall("sv_clear_level_binds", ClearLevelBinds, true);

	AddDFloat("sv_playercount", 4, true);
	AddDString("sv_port", "7777", true);

	AddDFunction("sv_blocks_add", SVAddBlock, true);
	AddDCall("sv_blocks_clear", ClearBlocks, true);

	AddDFunction("cl_setbgcolour", FUNC_SetBGColour, false);

	AddDFunction("cl_bind_blockid", CLAddTextureBind, false);
	AddDFunction("cl_blockids_add", CLAddTextureLevel, false);
	AddDFunction("cl_blockid_order", CLSetTextureIndexOrder, false);
	AddDCall("cl_blockids_clear", ClearTextureLevels, false);

	AddDFunction("cl_set_tex", CLSetTexture, false);
	AddDFunction("cl_set_texid_size", CLSetTextureIndexSize, false);
	AddDCall("cl_textures_clear", G_ClearTextures, false);

	sv_bag_size =				ValueAsFloatPtr(AddDFloatC("sv_bag_size", 0, C_BagSize, true));
	sv_queue_size =				ValueAsFloatPtr(AddDFloatC("sv_queue_size", 4, C_QueueSize, true));
	
	sv_gravity =				ValueAsFloatPtr(AddDFloat("sv_gravity", 0.5f, true));
	sv_drop_gravity =			ValueAsFloatPtr(AddDFloat("sv_drop_gravity", 0.5f, true));
	sv_drop_gravity_is_factor =	ValueAsFloatPtr(AddDFloat("sv_drop_gravity_is_factor", 0, true));
	sv_autorepeat =				ValueAsFloatPtr(AddDFloat("sv_autorepeat", 0.05f, true));
	sv_autorepeat_delay =		ValueAsFloatPtr(AddDFloat("sv_autorepeat_delay", 0.25f, true));

	sv_board_width =			ValueAsFloatPtr(AddDFloat("sv_board_width", 10, true));
	sv_board_height =			ValueAsFloatPtr(AddDFloat("sv_board_height", 20, true));
	sv_board_real_height =		ValueAsFloatPtr(AddDFloat("sv_board_real_height", 40, true));

	sv_clears_per_level =		ValueAsFloatPtr(AddDFloat("sv_clears_per_level", 10, true));

	sv_ghost =					ValueAsFloatPtr(AddDFloat("sv_ghost", 1.f, true));
	sv_hard_drop =				ValueAsFloatPtr(AddDFloat("sv_hard_drop", 1.f, true));
	sv_hold =					ValueAsFloatPtr(AddDFloat("sv_hold", 1.f, true));

	sv_lock_delay =				ValueAsFloatPtr(AddDFloat("sv_lock_delay", .5f, true));
	sv_drop_delay =				ValueAsFloatPtr(AddDFloat("sv_drop_delay", .1f, true));

	AddDCall("dbg_create_bag", DBGCreateBag, false);
}
