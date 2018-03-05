#include "Networking.h"
#include "Console.h"
#include "Messaging.h"
#include <stdio.h>
#include <stdlib.h>
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

/*
	NetworkReceive

	Receives data from a socket to a messagebuffer
	returns true if any data was received
*/
bool NetworkReceive(SOCKET connection, NetMessage *msg) {
	static int readlen;

	if (msg->bytes_left > MSG_LEN)
		readlen = recv(connection, msg->dynamic_buffer + msg->length, msg->bytes_left, 0);
	else
		readlen = recv(connection, msg->buffer + msg->length, msg->bytes_left, 0);
		
	if (readlen == SOCKET_ERROR) {
		msg->error = WSAGetLastError();
			
		if (msg->error != WSAEWOULDBLOCK) {
			char errorstr[64];
			snprintf(errorstr, 64, "Socket receive error %d\n", msg->error);
			ConsolePrint(errorstr);
		}
		else msg->error = 0;
	}
	else if (readlen > 0) {
		msg->length += readlen;
		msg->bytes_left -= readlen;

		return true;
	}

	return false;
}

/*
	NetworkReceiveMsgBuffer

	Receives message data from a socket to a messagebuffer
	returns true if the message is complete
*/
bool NetworkReceiveMsgBuffer(SOCKET connection, NetMessage *msg) {
	msg->error = 0;

	//If not currently reading a message..
	if (msg->bytes_left == 0) {
		msg->length = 0;
		msg->bytes_left = sizeof(uint16);

		if (NetworkReceive(connection, msg)) {
			while (msg->bytes_left) {
				NetworkReceive(connection, msg);

				if (msg->error) {
					msg->bytes_left = 0;
					return false;
				}
			}

			msg->bytes_left = BufferToInt16(msg->buffer);
			msg->length = 0;

			if (msg->bytes_left > MSG_LEN)
				msg->dynamic_buffer = (byte*)malloc(msg->bytes_left);
		}
		else {
			msg->bytes_left = 0;

			return false;
		}
	}

	if (NetworkReceive(connection, msg) && msg->bytes_left == 0) {
		//We have read the whole length
		return true;
	}
	
	return false;
}

void NetworkSend(SOCKET socket, const byte *data, uint16 size) {
	static int err;

	byte message_size_buffer[2];
	Int16ToBuffer(size, message_size_buffer);

	if (send(socket, message_size_buffer, 2, 0) == SOCKET_ERROR) {
		err = WSAGetLastError();
		return;
	}

	if (send(socket, data, size, 0) == SOCKET_ERROR) {
		err = WSAGetLastError();
	}
}
