#include "Settings.h"
#include "Config.h"
#include "IO.h"
#include "Resource.h"
#include "String.h"
#include <stdlib.h>
#include <CommCtrl.h>
#include <Windows.h>

#define ROW_HEIGHT 24
#define ROW_HALF_WIDTH 248

HWND hwnd_settings = NULL;
HWND hwnd_setting_font_texture;
HWND hwnd_setting_name;

char **texture_config_filepaths = NULL;
unsigned int texture_config_count = 0;

inline void AddComboBoxEntries() {
	char **dirs;
	unsigned int dircount = FindFilesInDirectory("textures/*", &dirs, FILE_ATTRIBUTE_DIRECTORY);

	if (dircount == 0)
		return;

	free(dirs[0]);
	free(dirs[1]);

	char path[MAX_PATH];
	for (unsigned int i = 2; i < dircount; ++i) {
		strcpy_s(path, MAX_PATH, "textures/");
		strcat_s(path, MAX_PATH, dirs[i]);
		free(dirs[i]);
		size_t end = strlen(path) + 1;

		strcat_s(path, MAX_PATH, "/*.cfg");

		char **filenames;
		unsigned int filecount = FindFilesInDirectory(path, &filenames, FILTER_NONE);
		texture_config_filepaths = (char**)realloc(texture_config_filepaths, (texture_config_count + filecount) * sizeof(char*));

		for (unsigned int j = 0; j < filecount; ++j) {
			path[end] = '\0';
			strcat_s(path, MAX_PATH, filenames[j]);
			texture_config_filepaths[texture_config_count++] = DupString(path);

			CutExt(filenames[j]);
			SendMessage(hwnd_setting_font_texture, CB_ADDSTRING, 0, (LPARAM)filenames[j]);

			free(filenames[j]);
		}

		free(filenames);
	}

	free(dirs);
}

void SettingsFree() {
	FreeTokens(texture_config_filepaths, texture_config_count);
	texture_config_count = 0;
	texture_config_filepaths = NULL;
}

inline void AddStaticLabel(HINSTANCE instance, HWND parent, const char *text, unsigned int row) {
	HWND hwnd = CreateWindowA(WC_STATIC, NULL, SS_LEFT | WS_CHILD | WS_VISIBLE, 0, ROW_HEIGHT * row, ROW_HALF_WIDTH, ROW_HEIGHT, parent, NULL, instance, NULL);
	SetWindowTextA(hwnd, text);
}

LRESULT CALLBACK SettingsProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) {
	case WM_CREATE:
	{
		HINSTANCE instance = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);

		AddStaticLabel(instance, hwnd, "Texture Config", 0);

		hwnd_setting_font_texture = CreateWindowA(WC_COMBOBOXA, NULL, CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_VISIBLE, 
			ROW_HALF_WIDTH, 0, ROW_HALF_WIDTH, ROW_HEIGHT, hwnd, NULL, instance, NULL);

		AddStaticLabel(instance, hwnd, "Multiplayer Name", 1);

		hwnd_setting_name = CreateWindowExA(WS_EX_CLIENTEDGE, WC_EDITA, NULL, WS_CHILD | WS_VISIBLE, 
			ROW_HALF_WIDTH, ROW_HEIGHT, ROW_HALF_WIDTH, ROW_HEIGHT, hwnd, NULL, instance, NULL);

		

		AddComboBoxEntries();
		break;
	}

	case WM_COMMAND:
		switch (HIWORD(wparam)) {
		case CBN_SELCHANGE:
		{
			int index = (int)SendMessage((HWND)lparam, CB_GETCURSEL, 0, 0);
			SetDvarString(GetDvar("cfg_texture"), texture_config_filepaths[index]);

			SaveCvars();
		}
			break;
		case EN_CHANGE:
		{	
			char namebuffer[64];
			GetWindowTextA(hwnd_setting_name, namebuffer, 64);
			SetDvarString(GetDvar("name"), namebuffer);

			SaveCvars();
			break;
		}
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_DESTROY:
		SettingsFree();
		hwnd_settings = NULL;
		break;

	default: return DefWindowProc(hwnd, msg, wparam, lparam);
	}

	return 0;
}

void CB_texture(DvarValue);

void SettingsInit() {
	HINSTANCE instance = GetModuleHandle(NULL);
	WNDCLASSEXA windowclass = {
		sizeof(WNDCLASSEXA),
		0,
		SettingsProc,
		0,
		0,
		instance,
		LoadIcon(instance, MAKEINTRESOURCE(IDI_ICON)),
		LoadCursor(NULL, IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		NULL,
		"settings_window",
		LoadIcon(instance, MAKEINTRESOURCE(IDI_ICON))
	};
	RegisterClassExA(&windowclass);
}

void SettingsOpen() {
	if (!hwnd_settings) {
		hwnd_settings = CreateWindowA("settings_window", "Settings", WS_OVERLAPPED | WS_SYSMENU, CW_USEDEFAULT, 0, 512, 512, NULL, NULL, GetModuleHandle(NULL), NULL);
		
		ShowWindow(hwnd_settings, SW_SHOW);
	}
}
