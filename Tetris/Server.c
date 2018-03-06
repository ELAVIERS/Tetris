#include "Server.h"
#include "Client.h"
#include "Config.h"
#include "Console.h"
#include "Dvar.h"
#include "Game.h"
#include "Lobby.h"
#include "Networking.h"
#include "Messaging.h"
#include "String.h"
#include <stdio.h> //snprintf
#include <stdlib.h>
#include <WS2tcpip.h>

typedef struct {
	SOCKET socket;
	bool local;

	NetMessage msg;

	bool admin;
} ServerSlot;

SOCKET sock_server = INVALID_SOCKET;

byte next_slot;
byte slot_count;
ServerSlot *slots;

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

/*
	Send/Receive functions
*/

void ServerDisconnectSlot(int);	//Forward declaration

void ServerSend(int slot, const byte *buffer, uint16 length) {
	if (slots[slot].local)
		ClientReceiveMessage(buffer, length);
	else if (slots[slot].socket != INVALID_SOCKET)
		NetworkSend(slots[slot].socket, buffer, length);
}

void ServerBroadcast(const byte *buffer, uint16 count) {
	for (int i = 0; i < slot_count; ++i)
		ServerSend(i, buffer, count);
}

void ServerReceive(int id) {
	if (slots[id].socket != INVALID_SOCKET) {
		while (NetworkReceiveMsgBuffer(slots[id].socket, &slots[id].msg)) {
			if (slots[id].msg.dynamic_buffer) {
				ServerReceiveMessage(slots[id].msg.dynamic_buffer, slots[id].msg.length, id);
				free(slots[id].msg.dynamic_buffer);
			}
			else
				ServerReceiveMessage(slots[id].msg.buffer, slots[id].msg.length, id);
		}

		if (slots[id].msg.error) {
			ServerDisconnectSlot(id);

			byte message[] = { SVMSG_LEAVE, (byte)id, 0 };
			ServerBroadcast(message, sizeof(message));
		}
	}
}

/*
Slot Handling
*/

void ServerUpdateNextSlot() {
	next_slot = -1;

	for (int i = 0; i < slot_count; ++i)
		if (!slots[i].local && slots[i].socket == INVALID_SOCKET) {
			next_slot = i;
			break;
		}
}

#include "Block.h"

void ServerInitSlot(int id) {
	static byte message[256];

	message[0] = SVMSG_INFO;
	message[1] = slot_count;
	message[2] = id;
	ServerSend(id, message, 3);

	message[0] = SVMSG_NAME;
	for (int i = 0; i < slot_count; ++i) {
		if (LobbyGetClientName(i)[0] != '\0') {
			message[1] = (byte)i;
			strcpy_s(message + 2, MSG_LEN - 2, LobbyGetClientName(i));
			ServerSend(id, message, (uint16)strlen(message + 2) + 3);
		}
	}

	if (slots[id].local == false) {
		SendServerDvars(id);
		SendBlockInfo(id);

		message[0] = SVMSG_START;
		ServerSend(id, message, 1);
	}
}

void ServerDisconnectSlot(int id) {
	if (slots[id].socket != INVALID_SOCKET) {
		shutdown(slots[id].socket, SD_BOTH);
		closesocket(slots[id].socket);
		slots[id].socket = INVALID_SOCKET;
	}
	else
		slots[id].local = false;

	slots[id].admin = false;

	free(slots[id].msg.dynamic_buffer);
	ServerUpdateNextSlot();
}

/*
	Socket handling
*/

void ServerAcceptSocket(int id) {
	static struct sockaddr addressinfo;
	static int addressinfo_sz = sizeof(addressinfo);

	slots[id].socket = accept(sock_server, &addressinfo, &addressinfo_sz);

	if (slots[id].socket != INVALID_SOCKET) {
		ServerInitSlot(id);

		char string[INET6_ADDRSTRLEN] = "unknown";
		GetAddressString(&addressinfo, string);

		ConsolePrint(string);
		ConsolePrint(" connected to the server\n");

		ServerUpdateNextSlot();
	}
}

/*
	Management
*/

void StartLocalServer() {
	if (sock_server != INVALID_SOCKET) return;

	slot_count = 1;
	slots = (ServerSlot*)malloc(sizeof(ServerSlot));

	slots[0].local = true;
	slots[0].admin = true;
	slots[0].socket = INVALID_SOCKET;
	slots[0].msg.dynamic_buffer = NULL;
	ServerInitSlot(0);
}

void StartOnlineServer() {
	if (sock_server != INVALID_SOCKET) return;

	sock_server = NetworkCreateListenSocket(GetDvar("sv_port")->value.string);

	if (sock_server != INVALID_SOCKET) {
		//If we're a local server, free slots
		if (slots) free(slots);

		slot_count = (int)GetDvar("sv_playercount")->value.number;
		if (slot_count < 1) slot_count = 1;

		slots = (ServerSlot*)malloc(slot_count * sizeof(ServerSlot));

		for (int i = 0; i < slot_count; ++i) {
			slots[i].socket = INVALID_SOCKET;
			slots[i].msg.dynamic_buffer = NULL;

			slots[i].local = false;
			slots[i].msg.bytes_left = 0;
			slots[i].admin = false;
		}

		slots[0].local = true;
		slots[0].admin = true;
		ServerInitSlot(0);

		MessageServerString(SVMSG_NAME, GetDvar("name")->value.string);

		next_slot = 1;
	}
}

void ServerFrame() {
	if (sock_server != INVALID_SOCKET) {
		if (next_slot >= 0)
			ServerAcceptSocket(next_slot);

		for (int i = 0; i < slot_count; ++i)
			if (slots[i].socket != INVALID_SOCKET)
				ServerReceive(i);
	}
}

void StopServer() {
	for (int i = 0; i < slot_count; ++i)
		if (slots[i].socket != INVALID_SOCKET) {
			shutdown(slots[i].socket, SD_BOTH);
			closesocket(slots[i].socket);
		}

	if (sock_server != INVALID_SOCKET) {
		shutdown(sock_server, SD_BOTH);
		closesocket(sock_server);
		sock_server = INVALID_SOCKET;
	}

	free(slots);
}

void ServerSetAdmin(byte id) {
	if (sock_server != INVALID_SOCKET && id < slot_count)
		slots[id].admin = true;
}

bool ServerClientIsAdmin(byte id) {
	return slots[id].admin;
}
