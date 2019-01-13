#pragma once
#include "Matrix.h"
#include "Text.h"
#include <stdbool.h>

/*
	Menu.h
*/

typedef struct {
	Text *text;
	void(*callback)();
} MenuItem;

typedef struct {
	MenuItem *items;
	unsigned int item_count;

	Mat3 transform;
	unsigned int selected;

	bool closable;
} Menu;

Menu *MenuGet();
void MenuInit();
void CreateMenu_Main();
void CreateMenu_Pause(bool closable);
void ActiveMenu_Close();
void ActiveMenu_ChangeSelection(int amount);
void ActiveMenu_Select();
void Menus_Render();
void FreeMenus();
