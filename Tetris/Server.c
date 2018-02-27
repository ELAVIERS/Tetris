#include "Server.h"
#include "Client.h"
#include "Console.h"
#include "Dvar.h"
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

	MessageBuffer msg;
} ServerSlot;

SOCKET sock_server = INVALID_SOCKET;

byte next_slot;
byte slot_count;
ServerSlot *slots;

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

/*
	Send/Receive functions
*/

void ServerFreeSlot(int);	//Forward declaration

inline void ServerSend(int slot, const byte *buffer, uint16 count) {
	if (slots[slot].socket != INVALID_SOCKET)
		NetworkSend(slots[slot].socket, buffer, count);
	else if (slots[slot].local)
		ClientReceiveMessage(buffer);
}

void ServerBroadcast(const byte *buffer, uint16 count) {
	for (int i = 0; i < slot_count; ++i)
		ServerSend(i, buffer, count);
}

void ServerReceive(int id) {
	if (slots[id].socket != INVALID_SOCKET) {
		while (NetworkReceiveMsgBuffer(slots[id].socket, &slots[id].msg))
			ServerReceiveMessage(slots[id].msg.buffer, id);

		if (slots[id].msg.error) {
			ServerFreeSlot(id);

			char temp[64];
			snprintf(temp, 64, "(TEMP) Kicking client cos error %d\n", slots[id].msg.error);
			ConsolePrint(temp);

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

void ServerInitSlot(int id) {
	byte message[MSG_LEN] = { SVMSG_SERVERINFO, slot_count };
	ServerSend(id, message, 2);

	message[0] = SVMSG_NAME;
	for (int i = 0; i < slot_count; ++i) {
		if (LobbyGetClientName(i)[0] != '\0') {
			message[1] = (byte)i;
			strcpy_s(message + 2, MSG_LEN - 2, LobbyGetClientName(i));
			ServerSend(id, message, strlen(message + 2) + 3);
		}
	}
}

inline void ServerFreeSlot(int id) {
	closesocket(slots[id].socket);
	slots[id].socket = INVALID_SOCKET;

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

void StartServer() {
	IFSERVER return;

	sock_server = NetworkCreateListenSocket(GetDvar("port")->value.string);

	IFSERVER{
		slot_count = (int)GetDvar("playercount")->value.number;
		if (slot_count < 1) slot_count = 1;

		slots = (ServerSlot*)malloc(slot_count * sizeof(ServerSlot));

		for (int i = 0; i < slot_count; ++i) {
			slots[i].socket = INVALID_SOCKET;
			slots[i].local = false;
			slots[i].msg.bytes_left = 0;
		}

		slots[0].local = true;
		ServerInitSlot(0);

		MessageServerString(SVMSG_NAME, GetDvar("name")->value.string);

		next_slot = 1;
	}
}

void ServerFrame() {
	IFSERVER{
		if (next_slot >= 0)
			ServerAcceptSocket(next_slot);

		for (int i = 0; i < slot_count; ++i)
			if (slots[i].socket != INVALID_SOCKET)
				ServerReceive(i);
	}
}

void StopServer() {
	for (int i = 0; i < slot_count; ++i)
		if (slots[i].socket != INVALID_SOCKET)
			closesocket(slots[i].socket);

	closesocket(sock_server);
	free(slots);
}
