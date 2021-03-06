#include "InputManager.h"
#include "Variables.h"
#include <string.h>

#define KEYNAME_COUNT 69

struct {
	char *string;
	WORD vk;
} key_names[KEYNAME_COUNT] = {
	{ ";", VK_OEM_1 },{ "/", VK_OEM_2 },{ "[", VK_OEM_4 },{ "\\", VK_OEM_5 },{ "]", VK_OEM_6 },{ "=", VK_OEM_PLUS },{ "-", VK_OEM_MINUS },{ ",", VK_OEM_COMMA },{ ".", VK_OEM_PERIOD },
	{ "f1", VK_F1 },{ "f2", VK_F2 },{ "f3", VK_F3 },{ "f4", VK_F4 },{ "f5", VK_F5 },{ "f6", VK_F6 },
	{ "f7", VK_F7 },{ "f8", VK_F8 },{ "f9", VK_F9 },{ "f10", VK_F10 },{ "f11", VK_F11 },{ "f12", VK_F12 },
	{ "tab", VK_TAB },{ "caps", VK_CAPITAL },{ "lshift", VK_LSHIFT },{ "rshift", VK_RSHIFT },{ "lctrl", VK_LCONTROL },{ "rctrl", VK_RCONTROL },{ "alt", VK_MENU },{ "space", VK_SPACE },
	{ "left", VK_LEFT },{ "up", VK_UP },{ "right", VK_RIGHT },{ "down", VK_DOWN },

	{ "0", '0' },{ "1", '1' },{ "2", '2' },{ "3", '3' },{ "4", '4' },{ "5", '5' },{ "6", '6' },{ "7", '7' },{ "8", '8' },{ "9", '9' },
	{ "a", 'A' },{ "b", 'B' },{ "c", 'C' },{ "d", 'D' },{ "e", 'E' },{ "f", 'F' },{ "g", 'G' },{ "h", 'H' },{ "i", 'I' },{ "j", 'J' },{ "k", 'K' },{ "l", 'L' },{ "m", 'M' },{ "n", 'N' },
	{ "o", 'O' },{ "p", 'P' },{ "q", 'Q' },{ "r", 'R' },{ "s", 'S' },{ "t", 'T' },{ "u", 'U' },{ "v", 'V' },{ "w", 'W' },{ "x", 'X' },{ "y", 'Y' },{ "z", 'Z' }
};

WORD VKCodeFromString(const char *key) {
	if (strlen(key) == 1) {
		if (key[0] >= '0' && key[0] <= '9')
			return key[0];
		if (key[0] >= 'a' && key[0] <= 'z')
			return key[0] - 'a' + 'A';
	}

	//We only check the first 33 here because of the former if statement
	for (unsigned int i = 0; i < 33; ++i)
		if (strcmp(key, key_names[i].string) == 0)
			return key_names[i].vk;
	
	return 0;
}

const char* StringFromVKCode(WORD vk) {
	for (unsigned int i = 0; i < KEYNAME_COUNT; ++i)
		if (vk == key_names[i].vk)
			return key_names[i].string;

	return "";
}

#include "Dvar.h"
#include "String.h"
#include <stdlib.h>

typedef struct KeyBind_s {
	WORD key;
	Dvar *dvar;

	struct BindData_s {
		enum {
			BIND_AXIS,
			BIND_COMMAND
		} type;

		union {
			float axisvalue;

			struct args_s {
				char **tokens;
				unsigned int count;
			} args;
		};
	} data;

	struct KeyBind_s *next;
} KeyBind;

KeyBind *binds;

KeyBind *AllocBind(WORD key) {
	KeyBind *bind = (KeyBind*)malloc(sizeof(KeyBind));
	bind->key = key;
	bind->next = NULL;

	if (!binds)
		binds = bind;
	else {
		if (key < binds->key) {
			KeyBind *copy = (KeyBind*)malloc(sizeof(KeyBind));
			memcpy_s(copy, sizeof(KeyBind), binds, sizeof(KeyBind));

			binds->data.type = 0;
			binds->data.axisvalue = 0.f;
			binds->dvar = NULL;
			binds->key = key;

			binds->next = copy;

			return binds;
		}

		KeyBind *currentbind = binds;
		while (currentbind->next && key > currentbind->next->key)
			currentbind = currentbind->next;

		if (currentbind->next) {
			bind->next = currentbind->next;
			currentbind->next = bind;
		}

		currentbind->next = bind;
	}

	return bind;
}

void FreeBind(KeyBind *bind)
{
	if (bind->data.args.count) {
		for (unsigned int i = 0; i < bind->data.args.count; ++i)
			free(bind->data.args.tokens[i]);

		free(bind->data.args.tokens);
	}
}

void Bind(const char **tokens, unsigned int count) {
	if (count < 2)
		return;

	Dvar *dvar = GetDvar(tokens[1]);
	if (!dvar || (dvar->type != DVT_CALL && count < 3))
		return;

	WORD key = VKCodeFromString(tokens[0]);
	if (key) {
		KeyBind *bind = AllocBind(key);
		bind->dvar = dvar;

		bind->data.type = BIND_COMMAND;
		bind->data.args.count = count - 2;

		if (bind->data.args.count) {
			bind->data.args.tokens = (char**)malloc(sizeof(char*) * bind->data.args.count);

			for (unsigned int i = 0; i < bind->data.args.count; ++i)
				bind->data.args.tokens[i] = DupString(tokens[i + 2]);
		}
	}
}

void BindAxis(const char **tokens, unsigned int count) {
	if (count < 3)
		return;

	Dvar *dvar = GetDvar(tokens[1]);
	if (!dvar || dvar->type != DVT_FLOAT)
		return;

	WORD key = VKCodeFromString(tokens[0]);
	if (key) {
		KeyBind *bind = AllocBind(key);
		bind->dvar = dvar;

		bind->data.type = BIND_AXIS;
		bind->data.axisvalue = (float)atof(tokens[2]);
	}
}

void ClearBinds() {
	while (binds) {
		KeyBind *next = binds->next;
		FreeBind(binds);
		binds = next;
	}
}

#include "Console.h"
#include "Globals.h"
#include "Menu.h"
#include "Window.h"

void KeyDown(WORD vk) {
	switch (vk) {
	case VK_OEM_3: ConsoleOpen(); return;
	case VK_F11: FullscreenToggle(); return;
	}

	if (!g_in_menu) {
		if (vk == VK_ESCAPE)
			CreateMenu_Pause(true);
	}
	else {
		switch (vk) {
		case VK_ESCAPE:
			ActiveMenu_Close();
			break;

		case VK_UP:
			ActiveMenu_ChangeSelection(1);
			break;
		case VK_DOWN:
			ActiveMenu_ChangeSelection(-1);
			break;
		case VK_RETURN:
			ActiveMenu_Select();
			break;
		}
	}

	for (KeyBind *bind = binds; bind && bind->key <= vk; bind = bind->next)
		if (bind->key == vk) {
			switch (bind->data.type) {
			case BIND_COMMAND:
				DvarCommand(bind->dvar, bind->data.args.tokens, bind->data.args.count, (bool)*bind_print, true);
				break;
			case BIND_AXIS:
				SetDvarFloat(bind->dvar, bind->dvar->value.number + bind->data.axisvalue, (bool)*bind_print);
				break;
			}
			return;
		}
}

void KeyUp(WORD vk) {
	for (KeyBind *bind = binds; bind && bind->key <= vk; bind = bind->next)
		if (bind->key == vk && bind->data.type == BIND_AXIS)
			SetDvarFloat(bind->dvar, bind->dvar->value.number - bind->data.axisvalue, (bool)*bind_print);
}

#include <stdio.h>

unsigned int BindsGetConfigString(char **out_string) {
	char *string = DupString("");
	size_t maxsize = 1;

	for (KeyBind *bind = binds; bind; bind = bind->next) {
		const char *keyname = StringFromVKCode(bind->key);
		size_t index = strlen(string);

		if (bind->data.type == BIND_AXIS) {
			size_t length = strlen("bindaxis   \n") + strlen(keyname) + strlen(bind->dvar->name) + 10;		//Bindaxis + spaces + newline + keyname + dvar name + float leeway + '\0'
			maxsize += length;
			string = (char*)realloc(string, maxsize);

			snprintf(string + index, maxsize,"bindaxis %s %s %3.6f\n", keyname, bind->dvar->name, bind->data.axisvalue);
		}
		else if (bind->data.type == BIND_COMMAND) {
			size_t length = strlen("bind  \n") + strlen(keyname) + strlen(bind->dvar->name) + 1;			//Bind + spaces + newline + keyname + dvar name + '\0'
			maxsize += length;
			string = (char*)realloc(string, maxsize);

			snprintf(string + index, maxsize, "bind %s %s", keyname, bind->dvar->name);

			index = strlen(string);
			for (unsigned int i = 0; i < bind->data.args.count; ++i) {
				length = strlen(bind->data.args.tokens[i]) + 1;
				maxsize += length;
				string = (char*)realloc(string, maxsize);

				snprintf(string + index, maxsize, " %s", bind->data.args.tokens[i]);
				index += length;
			}

			//the newline space has been accounted for, no alloc stuff needed
			snprintf(string + index, maxsize, "\n");
		}
	}

	*out_string = string;
	return (unsigned int)maxsize;
}
