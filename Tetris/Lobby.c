#pragma once
#include "Lobby.h"
#include "Client.h"
#include "Console.h"
#include "Resource.h"
#include "String.h"
#include <CommCtrl.h>
#include <stdlib.h>
#include <Windows.h>

typedef struct {
	char* name;

	unsigned int level;
	unsigned int score;
	unsigned int line_score;
} Client;

Client *clients = NULL;
int client_count = 0;

HWND hwnd_lobby;
HWND hwnd_clientlist;

void ClientListAddItems() {
	LVITEM lvi;
	lvi.mask = LVIF_TEXT | LVIF_STATE;
	lvi.iSubItem = 0;
	lvi.state = 0;
	lvi.stateMask = 0;

	for (int i = 0; i < client_count; ++i) {
		lvi.iItem = i;
		lvi.pszText = AllocStringFromInt(i);

		if (ListView_InsertItem(hwnd_clientlist, &lvi) == -1) {
			ConsolePrint("Listbox_InsertItem error\n");
			return;
		}

		free(lvi.pszText);

		ListView_SetItemText(hwnd_clientlist, i, 1, clients[i].name);
		
		char *string = AllocStringFromInt(clients[i].level);
		ListView_SetItemText(hwnd_clientlist, i, 2, string);
		free(string);
		string = AllocStringFromInt(clients[i].score);
		ListView_SetItemText(hwnd_clientlist, i, 3, string);
		free(string);
		string = AllocStringFromInt(clients[i].line_score);
		ListView_SetItemText(hwnd_clientlist, i, 4, string);
		free(string);
	}
}

void LobbySetSize(int size) {
	if (size < client_count)
		for (int i = size; i < client_count; ++i)
			free(clients[i].name);

	if (size > 0) {
		int last = client_count;
		client_count = size;

		clients = (Client*)realloc(clients, client_count * sizeof(Client));
		for (int i = last; i < client_count; ++i) {
			clients[i].name = DupString("");
			clients[i].level = clients[i].score = clients[i].line_score = 0;
		}

		if (hwnd_clientlist) {
			ListView_DeleteAllItems(hwnd_clientlist);
			ClientListAddItems();
		}
	}
}

int LobbyGetSize() {
	return client_count;
}

void LobbySetClientName(byte id, const char *name) {
	free(clients[id].name);

	clients[id].name = DupString(name);

	if (hwnd_clientlist)
		ListView_SetItemText(hwnd_clientlist, id, 1, clients[id].name);
}

void LobbySetClientLevel(byte id, uint16 level) {
	clients[id].level = level;

	if (hwnd_clientlist) {
		char *string = AllocStringFromInt(level);
		ListView_SetItemText(hwnd_clientlist, id, 2, string);
		free(string);
	}
}

void LobbySetClientScore(byte id, uint32 score) {
	clients[id].score = score;

	if (hwnd_clientlist) {
		char *string = AllocStringFromInt(score);
		ListView_SetItemText(hwnd_clientlist, id, 3, string);
		free(string);
	}
}

void LobbySetClientLineScore(byte id, uint32 line_clears) {
	clients[id].line_score = line_clears;

	if (hwnd_clientlist) {
		char *string = AllocStringFromInt(line_clears);
		ListView_SetItemText(hwnd_clientlist, id, 4, string);
		free(string);
	}
}

const char *LobbyGetClientName(byte id) {
	return clients[id].name;
}

/*
	Lobby Window stuff
*/

LRESULT CALLBACK LobbyProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) {
	case WM_CREATE:
		hwnd_clientlist = CreateWindowExA(WS_EX_CLIENTEDGE, WC_LISTVIEWA, NULL, LVS_REPORT | WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, NULL, GetModuleHandle(NULL), NULL);
		break;

	case WM_SIZE:
		MoveWindow(hwnd_clientlist, 16, 16, LOWORD(lparam) - 32, HIWORD(lparam) - 32, TRUE);
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		hwnd_lobby = NULL;
		break;

	default:
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}

	return 0;
}

void LobbyInit() {
	HINSTANCE instance = GetModuleHandle(NULL);

	WNDCLASSEXA windowclass = {
		sizeof(WNDCLASSEXA),
		0,
		LobbyProc,
		0,
		0,
		instance,
		LoadIcon(instance, MAKEINTRESOURCE(IDI_ICON)),
		LoadCursor(NULL, IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		NULL,
		"lobby_window",
		LoadIcon(instance, MAKEINTRESOURCE(IDI_ICON))
	};
	RegisterClassExA(&windowclass);
}

void LobbyShow() {
	if (client_count == 0) {
		MessageBoxA(NULL, "You can't open the lobby window\nwhen you're not in a server, dimwit.", "Oi", MB_OK);
		return;
	}

	if (!hwnd_lobby) {
		hwnd_lobby = CreateWindowA("lobby_window", "Lobby", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, 512, 512, NULL, NULL, GetModuleHandle(NULL), NULL);
		ShowWindow(hwnd_lobby, SW_SHOW);

		LVCOLUMN lvc;
		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvc.iSubItem = 0;
		lvc.cx = 24;
		lvc.pszText = "ID";
		lvc.fmt = LVCFMT_LEFT;

		ListView_InsertColumn(hwnd_clientlist, 0, &lvc);

		lvc.iSubItem = 1;
		lvc.cx = 96;
		lvc.pszText = "Name";

		ListView_InsertColumn(hwnd_clientlist, 1, &lvc);

		lvc.iSubItem = 2;
		lvc.pszText = "Level";

		ListView_InsertColumn(hwnd_clientlist, 2, &lvc);

		lvc.iSubItem = 3;
		lvc.pszText = "Score";
		
		ListView_InsertColumn(hwnd_clientlist, 3, &lvc);

		lvc.iSubItem = 4;
		lvc.pszText = "Line Clears";

		ListView_InsertColumn(hwnd_clientlist, 4, &lvc);

		ClientListAddItems();
	}
}
