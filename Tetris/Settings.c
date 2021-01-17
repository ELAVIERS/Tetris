#include "Settings.h"
#include "Config.h"
#include "IO.h"
#include "Resource.h"
#include "String.h"
#include "Variables.h"
#include <stdlib.h>
#include <CommCtrl.h>
#include <Windows.h>

#define ROW_HEIGHT 24
#define ROW_HALF_WIDTH 248

HWND hwnd_settings = NULL;
HWND hwnd_setting_texture_cfg;
HWND hwnd_setting_audio_cfg;
HWND hwnd_setting_name;
HWND hwnd_setting_volume;
HWND hwnd_setting_volume_music;

char ** GetConfigFilepaths(const char *search, const char *dir, const char* currentcfg, HWND hwnd, unsigned int *out_count) {
	char **names = NULL;
	unsigned int count = 0;

	char **dirs;
	unsigned int dircount = FindFilesInDirectory(search, &dirs, FILE_ATTRIBUTE_DIRECTORY);

	if (dircount == 0)
		return NULL;

	//I don't want these because they are always ".." and "." ... not very useful directories, basically!
	free(dirs[0]);
	free(dirs[1]);

	char path[MAX_PATH];
	for (unsigned int i = 2; i < dircount; ++i) {
		strcpy_s(path, MAX_PATH, dir);
		strcat_s(path, MAX_PATH, dirs[i]);
		free(dirs[i]);
		size_t end = strlen(path) + 1;

		strcat_s(path, MAX_PATH, "/*.cfg");

		char **filenames;
		unsigned int filecount = FindFilesInDirectory(path, &filenames, FILTER_NONE);
		names = (char**)realloc(names, (count + filecount) * sizeof(char*));

		for (unsigned int j = 0; j < filecount; ++j) {
			path[end] = '\0';
			strcat_s(path, MAX_PATH, filenames[j]);
			
			CutExt(filenames[j]);
			SendMessage(hwnd, CB_ADDSTRING, NULL, (LPARAM)filenames[j]);

			//todo: this needn't be here, stricmp is a thing
			bool match = true;
			for (const char* a = path, const *b = currentcfg; ; ++a, ++b)
			{
				if (tolower(*a) != tolower(*b))
				{
					match = false;
					break;
				}

				if (*a == '\0') break;
			}

			if (match) 
				SendMessage(hwnd, CB_SETCURSEL, count, NULL);

			names[count++] = DupString(path);
			free(filenames[j]);
		}

		free(filenames);
	}

	free(dirs);

	*out_count = count;
	return names;
}

char **texture_config_filepaths = NULL;
char **audio_config_filepaths = NULL;
unsigned int texture_config_count = 0;
unsigned int audio_config_count = 0;

inline void AddComboBoxEntries() {
	texture_config_filepaths = GetConfigFilepaths("textures/*", "textures/", GetDvar("cfg_texture")->value.string, hwnd_setting_texture_cfg, &texture_config_count);
	audio_config_filepaths = GetConfigFilepaths("audio/*", "audio/", GetDvar("cfg_audio")->value.string, hwnd_setting_audio_cfg, &audio_config_count);
}

void SettingsFree() {
	FreeTokens(texture_config_filepaths, texture_config_count);
	FreeTokens(audio_config_filepaths, audio_config_count);
	texture_config_count = 0;
	audio_config_count = 0;
	texture_config_filepaths = NULL;
	audio_config_filepaths = NULL;
}

inline void AddStaticLabel(HINSTANCE instance, HWND parent, const char *text, unsigned int row) {
	HWND hwnd = CreateWindowA(WC_STATIC, NULL, SS_LEFT | WS_CHILD | WS_VISIBLE, 0, ROW_HEIGHT * row, ROW_HALF_WIDTH, ROW_HEIGHT, parent, NULL, instance, NULL);
	SetWindowTextA(hwnd, text);
}

bool window_created;

LRESULT CALLBACK SettingsProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) {
	case WM_CREATE:
	{
		HINSTANCE instance = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);

		AddStaticLabel(instance, hwnd, "Texture Config", 0);

		hwnd_setting_texture_cfg = CreateWindowA(WC_COMBOBOXA, NULL, CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_VISIBLE, 
			ROW_HALF_WIDTH, 0, ROW_HALF_WIDTH, ROW_HEIGHT, hwnd, (HMENU)0, instance, NULL);

		AddStaticLabel(instance, hwnd, "Audio Config", 1);

		hwnd_setting_audio_cfg = CreateWindowA(WC_COMBOBOXA, NULL, CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_VISIBLE,
			ROW_HALF_WIDTH, ROW_HEIGHT, ROW_HALF_WIDTH, ROW_HEIGHT, hwnd, (HMENU)1, instance, NULL);

		AddStaticLabel(instance, hwnd, "Multiplayer Name", 2);

		hwnd_setting_name = CreateWindowExA(WS_EX_CLIENTEDGE, WC_EDITA, GetDvar("name")->value.string, WS_CHILD | WS_VISIBLE, 
			ROW_HALF_WIDTH, ROW_HEIGHT * 2, ROW_HALF_WIDTH, ROW_HEIGHT, hwnd, NULL, instance, NULL);

		AddStaticLabel(instance, hwnd, "Master Volume", 3);

		hwnd_setting_volume = CreateWindowExA(WS_EX_CLIENTEDGE, TRACKBAR_CLASSA, "Volume", TBS_AUTOTICKS | WS_CHILD | WS_VISIBLE,
			ROW_HALF_WIDTH, ROW_HEIGHT * 3, ROW_HALF_WIDTH, ROW_HEIGHT, hwnd, 0, instance, NULL);

		SendMessage(hwnd_setting_volume, TBM_SETRANGE, FALSE, MAKELPARAM(0, 20));
		SendMessage(hwnd_setting_volume, TBM_SETPOS, TRUE, (int)((*volume) * 20 * 2));

		AddStaticLabel(instance, hwnd, "Music Volume", 4);

		hwnd_setting_volume_music = CreateWindowExA(WS_EX_CLIENTEDGE, TRACKBAR_CLASSA, "Music Volume", TBS_AUTOTICKS | WS_CHILD | WS_VISIBLE,
			ROW_HALF_WIDTH, ROW_HEIGHT * 4, ROW_HALF_WIDTH, ROW_HEIGHT, hwnd, 0, instance, NULL);

		SendMessage(hwnd_setting_volume_music, TBM_SETRANGE, FALSE, MAKELPARAM(0, 20));
		SendMessage(hwnd_setting_volume_music, TBM_SETPOS, TRUE, (int)((*volume_music) * 20));

		AddComboBoxEntries();
		break;
	}

	case WM_COMMAND:
		if (window_created) {
			switch (HIWORD(wparam)) {
			case CBN_SELCHANGE:
			{
				int index = (int)SendMessage((HWND)lparam, CB_GETCURSEL, 0, 0);

				switch (LOWORD(wparam))
				{
				case 0:
					SetDvarString(GetDvar("cfg_texture"), texture_config_filepaths[index], true);
					break;
				case 1:
					SetDvarString(GetDvar("cfg_audio"), audio_config_filepaths[index], true);
					break;
				}

				SaveCvars();
			}
			break;
			case EN_CHANGE:
			{

				char namebuffer[64];
				GetWindowTextA(hwnd_setting_name, namebuffer, 64);
				SetDvarString(GetDvar("name"), namebuffer, true);

				SaveCvars();
				break;
			}
			}
		}
		break;

	case WM_HSCROLL:
	{
		if (window_created) {
			if ((HWND)lparam == hwnd_setting_volume)
				SetDvarFloat(GetDvar("volume"), SendMessage(hwnd_setting_volume, TBM_GETPOS, 0, 0) / 20.f / 2.f, true);
			else if ((HWND)lparam == hwnd_setting_volume_music)
				SetDvarFloat(GetDvar("volume_music"), SendMessage(hwnd_setting_volume_music, TBM_GETPOS, 0, 0) / 20.f, true);

			SaveCvars();
		}
		break;
	}

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
		window_created = false;
		hwnd_settings = CreateWindowA("settings_window", "Settings", WS_OVERLAPPED | WS_SYSMENU, CW_USEDEFAULT, 0, 512, 40 + ROW_HEIGHT * 5, NULL, NULL, GetModuleHandle(NULL), NULL);
		window_created = true;

		ShowWindow(hwnd_settings, SW_SHOW);
	}
}
