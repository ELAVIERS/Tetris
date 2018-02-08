#include "Console.h"
#include "Callbacks.h"
#include "Block.h"
#include "Board.h"
#include "Config.h"
#include "Dvar.h"
#include "Globals.h"
#include "Resource.h"
#include "String.h"
#include <CommCtrl.h>
#include <stdlib.h>
#include <Windows.h>

#define CONSOLE_BUFFER_LENGTH 256
#define CONSOLE_COLOR_BG RGB(16,16,16)
#define CONSOLE_COLOR_FG RGB(0,255,0)
#define CONSOLEINPUT_COLOR_FG RGB(255,255,255)
#define CONSOLEINPUT_COLOR_BG RGB(0,0,100)
#define CONSOLE_INPUT_HEIGHT 16

WNDPROC DefaultEditProc;

HWND hwnd_console = NULL;
HWND hwnd_console_text;
HWND hwnd_console_input;

char *console_buffer = NULL;
size_t console_buffer_size;

HBRUSH brush_console_bg;
HBRUSH brush_console_input_bg;

inline void ConsoleResetBuffer() {
	free(console_buffer);
	console_buffer = (char*)malloc(1);
	console_buffer[0] = '\0';
	console_buffer_size = 1;
}

void SubmitCommand(const char* command) {
	char **tokens;
	unsigned int count = SplitTokens(command, &tokens);

	if (count) {
		HandleCommand(tokens, count);
		FreeTokens(tokens, count);
	}
}

LRESULT CALLBACK ConsoleEditProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) {
	case WM_SETCURSOR:
		if (hwnd == hwnd_console_text)
			SetCursor(LoadCursor(NULL, IDC_ARROW));
		else
			return CallWindowProc(DefaultEditProc, hwnd, msg, wparam, lparam);
		break;

	case WM_CHAR:
		switch (wparam) {
		case '\r':
		case '`':
		case '~':
			break;
		default: return CallWindowProc(DefaultEditProc, hwnd, msg, wparam, lparam);
		}
		break;

	case WM_KEYDOWN:
		switch (wparam) {
		case VK_OEM_3: break;

		case VK_RETURN:
			if (hwnd == hwnd_console_input) {
				char buffer[CONSOLE_BUFFER_LENGTH];
				GetWindowTextA(hwnd, buffer, CONSOLE_BUFFER_LENGTH);
				SetWindowTextA(hwnd, "");
				SubmitCommand(buffer);
			}
			break;
		}
		break;

	case WM_KEYUP:
		if (wparam == VK_OEM_3)
			DestroyWindow(hwnd_console);
		break;


	default: return CallWindowProc(DefaultEditProc, hwnd, msg, wparam, lparam);
	}

	return 0;
}

LRESULT CALLBACK ConsoleProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) {
	case WM_CREATE:
		hwnd_console_text = CreateWindowA(WC_EDITA, NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL | ES_READONLY | ES_MULTILINE | ES_LEFT, 0, 0, 0, 0, hwnd, NULL, GetModuleHandle(NULL), NULL);
		hwnd_console_input = CreateWindowA(WC_EDITA, NULL, WS_CHILD | WS_VISIBLE | ES_LEFT, 0, 0, 0, 0, hwnd, NULL, GetModuleHandle(NULL), NULL);

		DefaultEditProc = (WNDPROC)GetWindowLongPtr(hwnd_console_input, GWLP_WNDPROC);

		SetWindowLongPtr(hwnd_console_text, GWLP_WNDPROC, (LONG_PTR)ConsoleEditProc);
		SetWindowLongPtr(hwnd_console_input, GWLP_WNDPROC, (LONG_PTR)ConsoleEditProc);
		break;

	case WM_SHOWWINDOW:
		SetFocus(hwnd_console_input);
		break;

	case WM_SIZE:
		MoveWindow(hwnd_console_text, 0, 0, LOWORD(lparam), HIWORD(lparam) - CONSOLE_INPUT_HEIGHT, TRUE);
		MoveWindow(hwnd_console_input, 0, HIWORD(lparam) - CONSOLE_INPUT_HEIGHT, LOWORD(lparam), CONSOLE_INPUT_HEIGHT, TRUE);
		break;

	case WM_CTLCOLORSTATIC:
	{
		HDC hdc = (HDC)wparam;
		SetTextColor(hdc, CONSOLE_COLOR_FG);
		SetBkColor(hdc, CONSOLE_COLOR_BG);
		return (LRESULT)brush_console_bg;
	}
	case WM_CTLCOLOREDIT:
	{
		HDC hdc = (HDC)wparam;
		SetTextColor(hdc, CONSOLEINPUT_COLOR_FG);
		SetBkColor(hdc, CONSOLEINPUT_COLOR_BG);
		return (LRESULT)brush_console_input_bg;
	}

	case WM_KEYUP:
		if (wparam == VK_OEM_3)
			DestroyWindow(hwnd);
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		hwnd_console = NULL;
		break;

	default: return DefWindowProc(hwnd, msg, wparam, lparam);
	}

	return 0;
}

inline unsigned int CountNewLines(const char* str) {
	unsigned int val = 0;
	for (const char *c = str; *c != '\0'; ++c)
		if (*c == '\n')
			++val;

	return val;
}

void CopyString_TOCRLF(char *dest, size_t max, const char *src) {
	unsigned int start = 0;
	while (dest[start] != '\0')
		++start;

	for (size_t i = 0; start + i < max; ++i) {
		//Make line endings CRLF, of course... that's why I wrote this dumb function after all. I am not writing \r\n everytime I want a new line, Windows.
		if (src[i] == '\n') {
			dest[start + i] = '\r';
			++start;
		}

		dest[start + i] = src[i];

		if (src[i] == '\0') break;
	}
}

void ConsolePrint(const char *string) {
	console_buffer_size += strlen(string) + CountNewLines(string);
	console_buffer = (char*)realloc(console_buffer, console_buffer_size);
	CopyString_TOCRLF(console_buffer, console_buffer_size, string);
	SetWindowTextA(hwnd_console_text, console_buffer);
}

DFunc Console_Clear, Console_Exit, Console_List, Console_Run, Console_Save;

void ConsoleInit() {
	ConsoleResetBuffer();

	brush_console_bg = CreateSolidBrush(CONSOLE_COLOR_BG);
	brush_console_input_bg = CreateSolidBrush(CONSOLEINPUT_COLOR_BG);

	HINSTANCE instance = GetModuleHandle(NULL);
	WNDCLASSEXA windowclass = {
		sizeof(WNDCLASSEXA),
		0,
		ConsoleProc,
		0,
		0,
		instance,
		LoadIcon(instance, MAKEINTRESOURCE(IDI_ICON)),
		LoadCursor(NULL, IDC_ARROW),
		0,
		NULL,
		"console_window",
		LoadIcon(instance, MAKEINTRESOURCE(IDI_ICON))
	};
	RegisterClassExA(&windowclass);

	////
	//Config Variables
	////
	AddDStringC(	"cfg_texture",				"", C_RunAsConfig); AddCvar(GetDvar("cfg_texture"));

	////
	//
	////
	AddDFunction(	"clear",					Console_Clear);
	AddDFunction(	"exit",						Console_Exit);
	AddDFunction(	"list",						Console_List);
	AddDFunction(	"run",						Console_Run);
	AddDFunction(	"save",						Console_Save);

	AddDFunction(	"sv_blocks_add",			SVAddBlock);
	AddDFunction(	"sv_blocks_clear",			SVClearBlocks);

	AddDStringC(	"cl_font_texture",			"",	C_FontTexture);
	AddDFloatC(		"cl_fontid_size",			0, C_FontIDSize);

	AddDStringC(	"cl_menu_font_texture",		"", C_MenuFontTexture);
	AddDFloatC(		"cl_menu_fontid_size",		0, C_MenuFontIDSize);

	AddDStringC(	"cl_block_texture",			"", C_BlockTexture);
	AddDFloatC(		"cl_texid_size",			0, C_BlockIDSize);
	AddDFunction(	"cl_texid_order",			CLSetTextureIndexOrder);
	AddDFunction(	"cl_texids_add",			CLAddTextureLevel);
	AddDFunction(	"cl_texids_clear",			CLClearTextureLevels);
}

void Console_Clear(const char **tokens, unsigned int count) {
	ConsoleResetBuffer();
	SetWindowTextA(hwnd_console_text, console_buffer);
}

void Console_Exit(const char **tokens, unsigned int count) {
	g_running = false;
}

void Console_List(const char **tokens, unsigned int count) {
	ConsolePrint("\nDynamic variables:\n");
	ListDvars();
}

void Console_Run(const char **tokens, unsigned int count) {
	if (count == 0)
		return;

	if (!RunConfig(tokens[0]))
		ConsolePrint("File not found\n");
}

void Console_Save(const char **tokens, unsigned int count) {
	SaveCvars();
}

void ConsoleOpen() {
	if (!hwnd_console) {
		hwnd_console = CreateWindowA("console_window", "Console", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, 768, 512, NULL, NULL, GetModuleHandle(NULL), NULL);
		SetWindowTextA(hwnd_console_text, console_buffer);
		ShowWindow(hwnd_console, SW_SHOW);
	}
}
