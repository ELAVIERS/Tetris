#include "Client.h"
#include "Console.h"
#include "Globals.h"
#include "Messaging.h"
#include "Networking.h"
#include "Resource.h"
#include "String.h"
#include <CommCtrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <ws2ipdef.h>

SOCKET client_socket = INVALID_SOCKET;
MessageBuffer clientmsg;

void Client_ConnectToServer(const char *ip, const char *port) {
	client_socket = NetworkCreateClientSocket(ip, port);

	if (client_socket != INVALID_SOCKET) {
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
			
			Client_Disconnect();
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
		while (NetworkReceiveMsgBuffer(client_socket, &clientmsg))
			ClientReceiveMessage(clientmsg.buffer);

		if (clientmsg.error)
			Client_Disconnect();
	}
}

void Client_OpenConnectionDialog() {
	DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(ID_DIALOG_CONNECT), g_hwnd, (DLGPROC)ConnectProc);
}

void Client_MessageServer(const byte *buffer, uint16 count) {
	if (client_socket != INVALID_SOCKET)
		NetworkSend(client_socket, buffer, count);
}

void Client_Disconnect() {
	if (client_socket != INVALID_SOCKET) {
		closesocket(client_socket);
		client_socket = INVALID_SOCKET;

		ConsolePrint("Disconnected from server\n");
	}
}
