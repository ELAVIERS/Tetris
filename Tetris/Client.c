#include "Client.h"
#include "Console.h"
#include "Game.h"
#include "Globals.h"
#include "Menu.h"
#include "Messaging.h"
#include "Networking.h"
#include "Resource.h"
#include "Server.h"
#include "String.h"
#include <CommCtrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <ws2ipdef.h>

SOCKET client_socket = INVALID_SOCKET;
NetMessage clientmsg;

void Client_CloseSocket();

bool IsRemoteClient() {
	return client_socket != INVALID_SOCKET;
}

void Client_ConnectToServer(const char *ip, const char *port) {
	client_socket = NetworkCreateClientSocket(ip, port);

	if (client_socket != INVALID_SOCKET) {
		StopServer();

		clientmsg.dynamic_buffer = NULL;

		MessageServerString(SVMSG_NAME, GetDvar("name")->value.string);
		byte message[] = { SVMSG_JOIN };
		MessageServer(message, sizeof(message));
	}
}

BOOL CALLBACK ConnectProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	static char ip[INET_ADDRSTRLEN];

	switch (msg) {
	case WM_INITDIALOG:
		SetFocus(GetDlgItem(hwnd, IDC_IPADDRESS));
		break;

	case WM_COMMAND:
		switch (LOWORD(wparam)) {
		case IDOK:
		{
			LPARAM ipparam;
			SendMessage(GetDlgItem(hwnd, IDC_IPADDRESS), IPM_GETADDRESS, 0, (LPARAM)&ipparam);

			snprintf(ip, INET_ADDRSTRLEN, "%d.%d.%d.%d", (BYTE)FIRST_IPADDRESS(ipparam), (BYTE)SECOND_IPADDRESS(ipparam), (BYTE)THIRD_IPADDRESS(ipparam), (BYTE)FOURTH_IPADDRESS(ipparam));
			
			char portstr[8];
			GetWindowTextA(GetDlgItem(hwnd, IDC_PORT), portstr, 8);
			
			Client_CloseSocket();
			Client_ConnectToServer(ip, portstr);
		}

		case IDCANCEL:
			EndDialog(hwnd, wparam);
			return TRUE;
		}
		break;
	}

	return FALSE;
}

void ClientFrame() {


	if (client_socket != INVALID_SOCKET) {
		while (NetworkReceiveMsgBuffer(client_socket, &clientmsg)) {
			if (clientmsg.dynamic_buffer) {
				ClientReceiveMessage(clientmsg.dynamic_buffer, clientmsg.length);
				free(clientmsg.dynamic_buffer);
				clientmsg.dynamic_buffer = NULL;
			}
			else
				ClientReceiveMessage(clientmsg.buffer, clientmsg.length);
		}

		if (clientmsg.error) {
			Client_CloseSocket();

			ConsolePrint("Lost connection to server\n");

			FreeMenus();
			GameEnd();
			CreateMenu_Main();
		}
	}
}

void Client_RunConnectionDialog() {
	DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(ID_DIALOG_CONNECT), g_hwnd, (DLGPROC)ConnectProc);
}

void Client_MessageServer(const byte *buffer, uint16 count) {
	if (client_socket != INVALID_SOCKET)
		NetworkSend(client_socket, buffer, count);
}

void Client_CloseSocket() {
	if (client_socket != INVALID_SOCKET) {
		shutdown(client_socket, SD_BOTH);
		closesocket(client_socket);
		client_socket = INVALID_SOCKET;

		free(clientmsg.dynamic_buffer);
	}

	StartLocalServer();
}

void Client_Disconnect() {
	if (client_socket != INVALID_SOCKET) {
		byte message = SVMSG_LEAVE;
		MessageServer(&message, 1);

		Client_CloseSocket();

		ConsolePrint("Disconnected from server\n");
	}
}
