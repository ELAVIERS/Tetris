#include "Networking.h"
#include "Console.h"
#include "Dvar.h"
#include <stdio.h>
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

char **port;

SOCKET socket_listen;

inline NetworkingError(const char *errorstr, int code) {
	char error[128];
	snprintf(error, 128, "ERROR: %s (%d)\n", errorstr, code);
	ConsolePrint(error);
}

void NetworkingInit() {
	int result;

	WSADATA data;
	if (result = WSAStartup(MAKEWORD(2, 2), &data))
		NetworkingError("Could not initialise networking", result);
	else ConsolePrint("Networking initialised\n");

	port = &AddDString("port", "7777")->value.string;
}

void NetworkingCreateSocket() {
	ADDRINFOA *result = NULL, 
	
	hints = {
		AI_PASSIVE,
		AF_INET,
		SOCK_STREAM,
		IPPROTO_TCP,	//Use TCP as messages only need to be sent when someone moves their piece
		0,
		NULL,
		NULL,
		NULL
	};

	if (getaddrinfo(NULL, *port, &hints, &result)) {
		NetworkingError("Could not get address info", WSAGetLastError());
		WSACleanup();
		return;
	}

	socket_listen = INVALID_SOCKET;
	socket_listen = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (socket_listen == INVALID_SOCKET) {
		NetworkingError("Could not create socket", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	if (bind(socket_listen, result->ai_addr, (int)result->ai_addrlen)) {
		NetworkingError("Could not bind socket", WSAGetLastError());
		closesocket(socket_listen);
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	freeaddrinfo(result);
	ConsolePrint("Socket created successfully\n");
	
}

void Listen() {
	if (listen(socket_listen, SOMAXCONN) == SOCKET_ERROR) {
		ConsolePrint("ERROR: listen failed\n");
		closesocket(socket_listen);
		WSACleanup();
	}
}

/*
typedef struct addrinfo
{
int                 ai_flags;       // AI_PASSIVE, AI_CANONNAME, AI_NUMERICHOST
int                 ai_family;      // PF_xxx
int                 ai_socktype;    // SOCK_xxx
int                 ai_protocol;    // 0 or IPPROTO_xxx for IPv4 and IPv6
size_t              ai_addrlen;     // Length of ai_addr
char *              ai_canonname;   // Canonical name for nodename
_Field_size_bytes_(ai_addrlen) struct sockaddr *   ai_addr;        // Binary address
struct addrinfo *   ai_next;        // Next structure in linked list
}
ADDRINFOA, *PADDRINFOA;
*/