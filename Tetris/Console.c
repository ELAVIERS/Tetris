#include "Console.h"
#include "Dvar.h"
#include "Resource.h"
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
		case VK_OEM_3: 
			ConsoleClose();
			break;

		case VK_RETURN:
			if (hwnd == hwnd_console_input) {
				char buffer[CONSOLE_BUFFER_LENGTH];
				GetWindowTextA(hwnd, buffer, CONSOLE_BUFFER_LENGTH);
				SetWindowTextA(hwnd, "");
				HandleCommandString(buffer, true);
			}
			break;
		}
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
			ConsoleClose();
		break;

	case WM_CLOSE:
		ConsoleClose();
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
	unsigned int newlines = 0;

	for (size_t i = 0; newlines + i < max; ++i) {
		//Make line endings CRLF, of course... that's why I wrote this dumb function after all. I am not writing \r\n everytime I want a new line, Windows.
		if (src[i] == '\n') {
			dest[newlines + i] = '\r';
			++newlines;
		}

		dest[newlines + i] = src[i];

		if (src[i] == '\0') break;
	}
}

void ConsolePrint(const char *string) {
	size_t string_length = strlen(string) + CountNewLines(string);

	char *crlf_str = (char*)malloc(string_length + 1);
	CopyString_TOCRLF(crlf_str, string_length + 1, string);

	if (hwnd_console) {
		SendMessageA(hwnd_console_text, EM_SETSEL, 0, -1);						//Select all text
		SendMessageA(hwnd_console_text, EM_SETSEL, -1, -1);						//Unselect all, cursor at end
		SendMessageA(hwnd_console_text, EM_REPLACESEL, 0, (LPARAM)crlf_str);	//Append
	}
	else {
		console_buffer_size += string_length;
		console_buffer = (char*)realloc(console_buffer, console_buffer_size);
		strcat_s(console_buffer, console_buffer_size, crlf_str);
	}
}

void FUNC_Clear() {
	ConsoleResetBuffer();
	SetWindowTextA(hwnd_console_text, console_buffer);
}

void FUNC_List() {
	ConsolePrint("\nDynamic variables:\n");
	ListDvars();
}

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

	AddDCall("clear", FUNC_Clear, false);
	AddDCall("list", FUNC_List, false);
}

void ConsoleOpen() {
	if (!hwnd_console) {
		hwnd_console = CreateWindowA("console_window", "Console", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, 768, 512, NULL, NULL, GetModuleHandle(NULL), NULL);
		SetWindowTextA(hwnd_console_text, console_buffer);
		SendMessageA(hwnd_console_text, EM_LINESCROLL, 0, CountNewLines(console_buffer));

		free(console_buffer);

		ShowWindow(hwnd_console, SW_SHOWNORMAL);
	}
}

void ConsoleClose() {
	if (hwnd_console) {
		SendMessageA(hwnd_console_text, EM_SETSEL, 0, -1);

		DWORD length;
		SendMessageA(hwnd_console_text, EM_GETSEL, (WPARAM)NULL, (LPARAM)&length);

		console_buffer_size = length + 1;
		console_buffer = (char*)malloc(console_buffer_size);

		GetWindowTextA(hwnd_console_text, console_buffer, (int)console_buffer_size);

		DestroyWindow(hwnd_console);
	}
}
