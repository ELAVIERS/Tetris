#include "Menu.h"

#include "Font.h"
#include "Globals.h"
#include "Matrix.h"
#include "Shader.h"
#include "Text.h"
#include <stdlib.h>

typedef struct {
	Text *text;
	void (*callback)();
} MenuItem;

typedef struct {
	MenuItem *items;
	unsigned int item_count;

	Mat3 transform;
	unsigned int selected;
} Menu;

void Menu_Zero(Menu *menu) {
	menu->items = 0;
	menu->item_count = 0;
	menu->selected = 0;
	Mat3Identity(menu->transform);
}

void Menu_AddItem(Menu *menu, const char *text, void (*callback)()) {
	unsigned int item_id = menu->item_count;
	
	++(menu->item_count);
	menu->items = (MenuItem*)realloc(menu->items, menu->item_count * sizeof(MenuItem));

	Text *new_text = Font_NewText(g_menu_font);
	SetTextInfo(new_text, text, 0, item_id * 32, 32);
	GenerateTextData(new_text, Font_UVSize(g_menu_font));

	menu->items[item_id].text =		new_text;
	menu->items[item_id].callback =	callback;
}

void Menu_Select(Menu *menu) {
	menu->items[menu->selected].callback();
}

void Menu_ChangeSelection(Menu *menu, int amount) {
	menu->selected += amount;

	while (menu->selected < 0)
		menu->selected += menu->item_count;

	menu->selected %= menu->item_count;
}

void Menu_Free(Menu* menu) {
	//Free the text associated with the menu
	for (unsigned int i = 0; i < menu->item_count; ++i) {
		Font_RemoveText(g_menu_font, menu->items[i].text);
		FreeText(menu->items[i].text);
	}

	free(menu->items);
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
#include "Config.h"
#include "Globals.h"
#include "IO.h"
#include "Settings.h"

typedef void Callback();

#define MENU_COUNT 2
Menu menus[MENU_COUNT];
Menu *m_main = menus + 0;
Menu *m_play = menus + 1;
unsigned int active_menu = 0;

Callback play_startgame;

char **modepaths;
unsigned int mode_count;

void CreateMenu_Play() {
	active_menu = (unsigned int)(m_play - menus);
	Menu_Zero(m_play);
	Mat3Translate(m_play->transform, 288, 0);

	char **filenames;
	mode_count = FindFilesInDirectory("Modes/*.cfg", &filenames, 0xFFFFFFFF);
	
	if (mode_count) {
		modepaths = (char**)malloc(mode_count * sizeof(char*));

		for (unsigned int i = 0; i < mode_count; ++i) {
			modepaths[i] = (char*)malloc(MAX_PATH);
			strcpy_s(modepaths[i], MAX_PATH, "Modes/");
			strcat_s(modepaths[i], MAX_PATH, filenames[i]);

			Menu_AddItem(m_play, filenames[i], play_startgame);

			free(filenames[i]);
		}

		free(filenames);
	}
}

void play_startgame() {
	RunConfig(modepaths[m_play->selected]);

	FreeTokens(modepaths, mode_count);
	Menu_Free(m_play);
	Menu_Free(m_main);

	g_board = BoardCreate();
	TEMP_UpdateBoardSize();

	g_menu_active = false;

}

void main_play() {
	CreateMenu_Play();
}

void main_settings() {
	SettingsOpen();
}

void main_quit() {
	g_running = false;
}

//Publicly available functions here

void CreateMainMenu() {
	active_menu = (unsigned int)(m_main - menus);
	Menu_Zero(m_main);

	Menu_AddItem(m_main, "PLAY", main_play);
	Menu_AddItem(m_main, "SETTINGS", main_settings);
	Menu_AddItem(m_main, "EXIT", main_quit);

	g_menu_active = true;
}

void ActiveMenu_ChangeSelection(int amount) {
	Menu_ChangeSelection(menus + active_menu, amount);
}

void ActiveMenu_Select() {
	Menu_Select(menus + active_menu);
}

void Menus_Render() {
	glBindTexture(GL_TEXTURE_2D, g_menu_font->texture->glid);

	for (unsigned int i = 0; i < MENU_COUNT; ++i)
		Menu_Render(menus + i);
}
