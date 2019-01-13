#include "Menu.h"

#include "Font.h"
#include "Globals.h"
#include "Shader.h"
#include <stdlib.h>

void Menu_Zero(Menu *menu) {
	menu->items = 0;
	menu->item_count = 0;
	menu->selected = 0;
	Mat3Identity(menu->transform);
	Mat3Scale(menu->transform, 32, 32);
	menu->closable = false;
}

void Menu_AddItem(Menu *menu, const char *text, void (*callback)()) {
	unsigned int item_id = menu->item_count;
	
	++(menu->item_count);
	menu->items = (MenuItem*)realloc(menu->items, menu->item_count * sizeof(MenuItem));

	Text *new_text = Font_NewText(&g_font);
	SetTextInfo(new_text, text, 0, item_id, 1.f);
	GenerateTextData(new_text, Font_UVSize(&g_font));

	menu->items[item_id].text =		new_text;
	menu->items[item_id].callback =	callback;
}

void Menu_Select(Menu *menu) {
	if (menu->item_count == 0)
		return;

	menu->items[menu->selected].callback();
}

void Menu_ChangeSelection(Menu *menu, int amount) {
	if (menu->item_count == 0)
		return;

	int selection = menu->selected + amount;

	while (selection < 0)
		selection += menu->item_count;

	menu->selected = selection;
	menu->selected %= menu->item_count;
}

void Menu_Free(Menu* menu) {
	//Free the text associated with the menu
	for (unsigned int i = 0; i < menu->item_count; ++i) {
		Font_RemoveText(&g_font, menu->items[i].text);
		FreeText(menu->items[i].text);
	}

	free(menu->items);
	menu->items = NULL;
	menu->item_count = 0;
}

float colour_normal[3] = { 1.f, 1.f, 1.f };
float colour_select[3] = { 0.f, 0.f, 1.f };

void Menu_Render(const Menu* menu) {
	ShaderSetUniformMat3(g_active_shader, "u_transform", menu->transform);
	ShaderSetUniformVec3(g_active_shader, "u_colour", colour_normal);

	for (unsigned int i = 0; i < menu->item_count; ++i) {
		if (i == menu->selected) {
			ShaderSetUniformVec3(g_active_shader, "u_colour", colour_select);
			RenderText(menu->items[i].text);
			ShaderSetUniformVec3(g_active_shader, "u_colour", colour_normal);
		}
		else RenderText(menu->items[i].text);
	}
}

////////////////////
#include "Client.h"
#include "Config.h"
#include "Game.h"
#include "Globals.h"
#include "IO.h"
#include "Messaging.h"
#include "Server.h"
#include "Settings.h"

#define MENU_COUNT 2
Menu menus[MENU_COUNT];
Menu *menuslot1 = menus + 0;
Menu *menuslot2 = menus + 1;

Menu* MenuGet() {
	return menuslot1;
}

void MenuInit() {
	Menu_Zero(menuslot1);
	Menu_Zero(menuslot2);

	Mat3Translate(menuslot2->transform, 288, 0);
}

unsigned int active_menu = 0;
unsigned int pause_menu_item_id;

char **modepaths;
unsigned int mode_count;

void play_startgame() {
	RunConfig(modepaths[menuslot2->selected]);

	FreeTokens(modepaths, mode_count);
	Menu_Free(menuslot1);
	Menu_Free(menuslot2);

	byte message = SVMSG_START;
	ServerBroadcast(&message, 1, -1);
}

void CreateMenuSecondary_Play() {
	active_menu = 1;
	menuslot2->selected = 0;
	menuslot2->closable = true;

	char **filenames;
	mode_count = FindFilesInDirectory("Modes/*.cfg", &filenames, FILTER_NONE);

	if (mode_count) {
		modepaths = (char**)malloc(mode_count * sizeof(char*));

		for (unsigned int i = 0; i < mode_count; ++i) {
			modepaths[i] = (char*)malloc(MAX_PATH);
			strcpy_s(modepaths[i], MAX_PATH, "Modes/");
			strcat_s(modepaths[i], MAX_PATH, filenames[i]);

			CutExt(filenames[i]);
			Menu_AddItem(menuslot2, filenames[i], play_startgame);

			free(filenames[i]);
		}

		free(filenames);
	}
}

void main_quit() {
	g_running = false;
}

void main_host() {
	StartOnlineServer();

	CreateMenuSecondary_Play();
}

void main_connect() {
	Client_RunConnectionDialog();

	if (IsRemoteClient())
		Menu_Free(menuslot1);
}

void pause_restart() {
	byte message = SVMSG_START;
	ServerBroadcast(&message, 1, -1);

	ActiveMenu_Close();
}

#include "Variables.h"

void pause_toggle_pause() {

	if (*sv_paused) {
		SetDvarFloat(GetDvar("sv_paused"), 0.f, true);

		Text *text = menuslot1->items[pause_menu_item_id].text;
		free(text->string);
		text->string = DupString("PAUSE");
		GenerateTextData(text, Font_UVSize(&g_font));
	}
	else {
		SetDvarFloat(GetDvar("sv_paused"), 1.f, true);
		
		Text *text = menuslot1->items[pause_menu_item_id].text;
		free(text->string);
		text->string = DupString("UNPAUSE");
		GenerateTextData(text, Font_UVSize(&g_font));
	}
}

void pause_endgame() {
	Menu_Free(menuslot1);

	if (IsRemoteClient()) {
		Client_Disconnect();
	}
	else {
		StopServer();
		StartLocalServer();
	}

	GameEnd();

	CreateMenu_Main();
}

void CreateMenu_Main() {
	active_menu = 0;
	menuslot1->selected = 0;
	menuslot1->closable = false;

	Mat3Identity(menuslot1->transform);
	Mat3Scale(menuslot1->transform, 32, 32);

	g_in_menu = true;

	Menu_AddItem(menuslot1, "PLAY", CreateMenuSecondary_Play);
	Menu_AddItem(menuslot1, "CONNECT", main_connect);
	Menu_AddItem(menuslot1, "HOST", main_host);
	Menu_AddItem(menuslot1, "SETTINGS", SettingsOpen);
	Menu_AddItem(menuslot1, "EXIT", main_quit);

	if (!IsRemoteClient()) SetDvarFloat(GetDvar("sv_paused"), 1.f, true);
}

void CreateMenu_Pause(bool closable) {
	active_menu = 0;
	menuslot1->selected = 0;
	menuslot1->closable = closable;

	g_in_menu = true;
	
	if (!IsRemoteClient()) {
		Menu_AddItem(menuslot1, "RESTART", pause_restart);

		if (ServerGetSlotCount() != 1) {
			pause_menu_item_id = menuslot1->item_count;
			Menu_AddItem(menuslot1, *sv_paused ? "UNPAUSE" : "PAUSE", pause_toggle_pause);
		}
		
		Menu_AddItem(menuslot1, "MODE", CreateMenuSecondary_Play);
	}
	
	Menu_AddItem(menuslot1, "SETTINGS", SettingsOpen);
	Menu_AddItem(menuslot1, "MENU", pause_endgame);
	Menu_AddItem(menuslot1, "EXIT", main_quit);

	if (!IsRemoteClient() && ServerGetSlotCount() == 1)
		SetDvarFloat(GetDvar("sv_paused"), 1.f, true);
}

void ActiveMenu_Close() {
	if (menus[active_menu].closable) {
		Menu_Free(menus + active_menu);

		if (active_menu == 0) {
			g_in_menu = false;

			if (!IsRemoteClient() && ServerGetSlotCount() == 1)
				SetDvarFloat(GetDvar("sv_paused"), 0.f, true);
		}
		else --active_menu;
	}
}

void ActiveMenu_ChangeSelection(int amount) {
	Menu_ChangeSelection(menus + active_menu, amount);
}

void ActiveMenu_Select() {
	Menu_Select(menus + active_menu);
}

void Menus_Render() {
	ShaderSetUniformFloat2(g_active_shader, "u_uvoffset", 0.f, 0.f);
	glBindTexture(GL_TEXTURE_2D, g_font.texture->glid);

	for (unsigned int i = 0; i < MENU_COUNT; ++i)
		Menu_Render(menus + i);
}

void FreeMenus() {
	for (unsigned int i = 0; i < MENU_COUNT; ++i)
		Menu_Free(menus + i);

	g_in_menu = false;
}
