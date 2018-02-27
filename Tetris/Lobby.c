#pragma once
#include "Lobby.h"
#include "Client.h"
#include "Console.h"
#include "Resource.h"
#include "String.h"
#include <CommCtrl.h>
#include <stdlib.h>
#include <Windows.h>

Client *clients = NULL;
int client_count;

HWND hwnd_lobby;
HWND hwnd_clientlist;

void LobbySetSize(int size) {
	if (clients) {
		for (int i = 0; i < client_count; ++i)
			free(clients[i].name);

		free(clients);
	}

	if (client_count = size) {
		clients = (Client*)malloc(client_count * sizeof(Client));
		
		for (int i = 0; i < size; ++i) {
			clients[i].name = DupString("");
		}
	}
}

void LobbySetClientName(byte id, const char *name) {
	free(clients[id].name);

	clients[id].name = DupString(name);

	if (hwnd_clientlist)
		ListView_SetItemText(hwnd_clientlist, id, 1, clients[id].name);
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
		hwnd_clientlist = CreateWindowExA(WS_EX_CLIENTEDGE, WC_LISTVIEWA, NULL, LVS_REPORT | WS_CHILD | WS_VISIBLE, 16, 16, 196, 196, hwnd, NULL, GetModuleHandle(NULL), NULL);

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
		lvc.cx = 32;
		lvc.pszText = "ID";
		lvc.fmt = LVCFMT_LEFT;

		ListView_InsertColumn(hwnd_clientlist, 0, &lvc);

		lvc.iSubItem = 1;
		lvc.cx = 128;
		lvc.pszText = "Name";

		ListView_InsertColumn(hwnd_clientlist, 1, &lvc);

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

			ListView_SetItemText(hwnd_clientlist, i, 1, clients[i].name);

			free(lvi.pszText);
		}
	}
}
