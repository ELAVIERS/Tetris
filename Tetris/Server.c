#include "Server.h"
#include "Client.h"
#include "Console.h"
#include "Dvar.h"
#include "Networking.h"
#include "Messaging.h"
#include "String.h"
#include <stdio.h> //snprintf
#include <stdlib.h>
#include <WS2tcpip.h> //inet_ntop

typedef struct {
	SOCKET socket;
	bool local;
	char *name;
} ServerClient;

SOCKET sock_server = INVALID_SOCKET;

int next_client_id;
int client_count;
ServerClient *clients;

bool ServerIsActive() {
	return sock_server != INVALID_SOCKET;
}

void GetAddressString(struct sockaddr *addr, char *out) {
	union {
		struct sockaddr *addr;
		struct sockaddr_in *as_ipv4;
		struct sockaddr_in6 *as_ipv6;
	} address = { .addr = addr };

	switch (addr->sa_family) {
	case AF_INET:
		inet_ntop(AF_INET, &address.as_ipv4->sin_addr, out, INET6_ADDRSTRLEN);
		break;
	case AF_INET6:
		inet_ntop(AF_INET, &address.as_ipv6->sin6_addr, out, INET6_ADDRSTRLEN);
		break;
	}
}

void ServerUpdateNextClient() {
	next_client_id = -1;

	for (int i = 0; i < client_count; ++i)
		if (!clients[i].local && clients[i].socket == INVALID_SOCKET) {
			next_client_id = i;
			break;
		}
}

void ServerCloseConnection(int id) {
	closesocket(clients[id].socket);
	clients[id].socket = INVALID_SOCKET;
	SetPlayerName(id, "");

	ServerUpdateNextClient();
}

void ServerReadSocket(int id) {
	static int readlen, error;
	static byte buffer[MSG_LEN];

	readlen = recv(clients[id].socket, buffer, MSG_LEN, 0);

	if (readlen == SOCKET_ERROR) {
		error = WSAGetLastError();

		switch (error) {
		case WSAEWOULDBLOCK:break;

		case WSAETIMEDOUT:
		{
			byte msgbuffer[MSG_LEN];
			msgbuffer[0] = SVMSG_TALK;
			snprintf(msgbuffer + 1, MSG_LEN - 1, "%s timed out", clients[id].name);
			ServerBroadcast(msgbuffer, strlen(msgbuffer + 1) + 2);
		}

			ServerCloseConnection(id);
			break;

		default:
		{
			byte msgbuffer[MSG_LEN];
			msgbuffer[0] = SVMSG_TALK;
			snprintf(msgbuffer + 1, MSG_LEN - 1, "%s dipped", clients[id].name);
			ServerBroadcast(msgbuffer, strlen(msgbuffer + 1) + 2);
		}

			ServerCloseConnection(id);
		}
	}
	else if (readlen > 0)
		ServerReceiveMessage(buffer, MSG_LEN, id);
}

void ServerAcceptSocket(int id) {
	static struct sockaddr addressinfo;
	static int addressinfo_sz = sizeof(addressinfo);

	clients[id].socket = accept(sock_server, &addressinfo, &addressinfo_sz);

	if (clients[id].socket != INVALID_SOCKET) {
		char string[INET6_ADDRSTRLEN] = "unknown";
		GetAddressString(&addressinfo, string);

		ConsolePrint(string);
		ConsolePrint(" connected to the server\n");

		ServerUpdateNextClient();
	}
}

void StartServer() {
	IFSERVER return;

	sock_server = NetworkCreateListenSocket(GetDvar("port")->value.string);

	IFSERVER{
		client_count = (int)GetDvar("playercount")->value.number;
		if (client_count < 1) client_count = 1;

		clients = (ServerClient*)malloc(client_count * sizeof(ServerClient));

		for (int i = 0; i < client_count; ++i) {
			clients[i].socket = INVALID_SOCKET;
			clients[i].name = DupString("");
			clients[i].local = false;
		}

		clients[0].local = true;

		MessageServerString(SVMSG_NAME, GetDvar("name")->value.string);

		next_client_id = 1;
	}
}

void ServerFrame() {
	IFSERVER{
		if (next_client_id >= 0)
			ServerAcceptSocket(next_client_id);

		for (int i = 0; i < client_count; ++i)
			if (clients[i].socket != INVALID_SOCKET)
				ServerReadSocket(i);
	}
}

void StopServer() {
	for (int i = 0; i < client_count; ++i)
		if (clients[i].socket != INVALID_SOCKET) {
			closesocket(clients[i].socket);
			free(clients[i].name);
		}

	closesocket(sock_server);
	free(clients);
}

void ServerBroadcast(const byte *buffer, unsigned int count) {
	for (int i = 0; i < client_count; ++i)
		if (clients[i].socket != INVALID_SOCKET)
			send(clients[i].socket, buffer, count, 0);
		else if (clients[i].local)
			ClientReceiveMessage(buffer, count);
}

void SetPlayerName(byte playerid, const char *name) {
	free(clients[playerid].name);
	clients[playerid].name = DupString(name);
}

const char *GetPlayerName(byte playerid) {
	return clients[playerid].name;
}
