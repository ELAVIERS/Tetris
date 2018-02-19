#include "Client.h"

#include "Globals.h"
#include "Messaging.h"
#include "Networking.h"
#include "Resource.h"
#include "String.h"
#include <CommCtrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

SOCKET client_socket = INVALID_SOCKET;

#define IP_LEN 16
char ip[IP_LEN];

BOOL CALLBACK ConnectProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) {
	case WM_COMMAND:
		switch (LOWORD(wparam)) {
		case IDOK:
		{
			LPARAM ipparam;
			SendMessage(GetDlgItem(hwnd, IDC_IPADDRESS), IPM_GETADDRESS, 0, (LPARAM)&ipparam);

			snprintf(ip, IP_LEN, "%d.%d.%d.%d", (BYTE)FIRST_IPADDRESS(ipparam), (BYTE)SECOND_IPADDRESS(ipparam), (BYTE)THIRD_IPADDRESS(ipparam), (BYTE)FOURTH_IPADDRESS(ipparam));
			
			char portstr[8];
			GetWindowTextA(GetDlgItem(hwnd, IDC_PORT), portstr, 8);
			
			client_socket = NetworkCreateClientSocket(ip, portstr);

			if (client_socket != INVALID_SOCKET)
				MessageServerString(SVMSG_JOIN, GetDvar("name")->value.string);
		}

		case IDCANCEL:
			EndDialog(hwnd, wparam);
			return TRUE;
		}
	}

	return FALSE;
}

void ClientFrame() {
	static int readlen;
	static int err;
	static char buffer[MSG_LEN];

	if (client_socket != INVALID_SOCKET) {
		readlen = recv(client_socket, buffer, MSG_LEN, 0);

		if (readlen == SOCKET_ERROR) {
			err = WSAGetLastError();
			if (err != WSAEWOULDBLOCK)
				NetworkingError("Receive error", WSAGetLastError());
		}
		else if (readlen > 0)
			ClientReceiveMessage(buffer, MSG_LEN);
	}
}

void Client_OpenConnectionDialog() {
	DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(ID_DIALOG_CONNECT), g_hwnd, (DLGPROC)ConnectProc);
}

void Client_MessageServer(const byte *buffer, unsigned int count) {
	if (client_socket != INVALID_SOCKET)
		send(client_socket, buffer, count, 0);
}
