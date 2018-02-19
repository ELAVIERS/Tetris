#include "Networking.h"
#include "Console.h"
#include <stdio.h>
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

void NetworkingError(const char *errorstr, int code) {
	char error[128];
	snprintf(error, 128, "NET ERROR: %s (%d)\n", errorstr, code);
	ConsolePrint(error);
}

void NetworkingInit() {
	int result;

	WSADATA data;
	if (result = WSAStartup(MAKEWORD(2, 2), &data))
		NetworkingError("Could not initialise networking", result);
	else ConsolePrint("Networking initialised\n");
}

SOCKET NetworkCreateClientSocket(const char *ip, const char *port) {
	ConsolePrint("Attempting connection to ");
	ConsolePrint(ip);
	ConsolePrint(":");
	ConsolePrint(port);
	ConsolePrint("...\n");

	ADDRINFOA *result = NULL, *current = NULL,

		hints = {
		AI_PASSIVE,
		AF_UNSPEC,
		SOCK_STREAM,
		IPPROTO_TCP,
		0,
		NULL,
		NULL,
		NULL
	};

	if (getaddrinfo(ip, port, &hints, &result)) {
		NetworkingError("Could not resolve address", WSAGetLastError());
		return INVALID_SOCKET;
	}
	
	SOCKET connection = INVALID_SOCKET;
	for (current = result; current; current = current->ai_next) {
		connection = socket(current->ai_family, current->ai_socktype, current->ai_protocol);
		if (connection == INVALID_SOCKET) {
			NetworkingError("Could not create socket", WSAGetLastError());
			freeaddrinfo(result);
			return INVALID_SOCKET;
		}
		
		if (connect(connection, current->ai_addr, (int)current->ai_addrlen) == SOCKET_ERROR) {
			NetworkingError("Socket connection failed", WSAGetLastError());
			closesocket(connection);
			connection = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (connection == INVALID_SOCKET) {
		NetworkingError("Connection failed!", 0);
		return INVALID_SOCKET;
	}

	u_long nonblocking = 1;
	if (ioctlsocket(connection, FIONBIO, &nonblocking) == SOCKET_ERROR) {
		NetworkingError("Could not set socket as non-blocking", WSAGetLastError());
		closesocket(connection);
		return INVALID_SOCKET;
	}
	
	ConsolePrint("Connected to server\n");

	return connection;
}

SOCKET NetworkCreateListenSocket(const char *port) {
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

	if (getaddrinfo(NULL, port, &hints, &result)) {
		NetworkingError("Could not resolve address", WSAGetLastError());
		return INVALID_SOCKET;
	}

	SOCKET connection = INVALID_SOCKET;
	connection = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (connection == INVALID_SOCKET) {
		NetworkingError("Could not create socket", WSAGetLastError());
		freeaddrinfo(result);
		return INVALID_SOCKET;
	}

	if (bind(connection, result->ai_addr, (int)result->ai_addrlen)) {
		NetworkingError("Could not bind socket", WSAGetLastError());
		closesocket(connection);
		freeaddrinfo(result);
		return INVALID_SOCKET;
	}

	freeaddrinfo(result);

	u_long nonblocking = 1;
	if (ioctlsocket(connection, FIONBIO, &nonblocking) == SOCKET_ERROR) {
		NetworkingError("Could not set socket as non-blocking", WSAGetLastError());
		closesocket(connection);
		return INVALID_SOCKET;
	}

	if (listen(connection, SOMAXCONN) == SOCKET_ERROR) {
		NetworkingError("Listen failed", WSAGetLastError());
		closesocket(connection);
		return INVALID_SOCKET;
	}

	ConsolePrint("Socket created successfully\n");

	return connection;
}

void NetworkFrame() {
	
}
