#include "Messaging.h"
#include "Client.h"
#include "Console.h"
#include "Game.h"
#include "Globals.h"
#include "Lobby.h"
#include "Server.h"
#include "String.h"
#include <stdio.h>
#include <string.h>

void MessageBufferRemoveMsg(NetMessage *msg) {
	memcpy_s(msg->buffer, MSG_LEN, msg->buffer + msg->length, MSG_LEN - msg->length);
	msg->length = 0;
	msg->bytes_left = 0;
}

void MessageServer(const byte *message, uint16 length) {
	if (IsRemoteClient())
		Client_MessageServer(message, length);
	else
		ServerReceiveMessage(message, length, 0);
}

void ServerReceiveMessage(const byte *message, uint16 length, byte playerid) {
	static char buffer[MSG_LEN];

	buffer[0] = message[0];

	switch (message[0]) {
	case SVMSG_COMMAND:
		if (ServerClientIsAdmin(playerid)) {
			ServerBroadcast(message, strlen(message + 1) + 2);
		}
		else {
			byte deny[] = { SVMSG_TEXT, SVTEXT_DENIED };
			ServerSend(playerid, deny, sizeof(deny));
		}
		break;
		
	case SVMSG_NAME:
	case SVMSG_CHAT:
		buffer[1] = playerid;
		strcpy_s(buffer + 2, MSG_LEN - 3, message + 1);
		ServerBroadcast(buffer, (uint16)strlen(buffer + 2) + 3);
		break;

	case SVMSG_JOIN:
	case SVMSG_PLACE:
	case SVMSG_CLEAR:
		buffer[1] = playerid;
		ServerBroadcast(buffer, 2);
		break;

	case SVMSG_LEAVE:
		ServerDisconnectSlot(playerid);
		break;

	case SVMSG_BLOCKPOS:
		buffer[1] = playerid;
		buffer[2] = message[1];
		buffer[3] = message[2];
		buffer[4] = message[3];
		buffer[5] = message[4];
		ServerBroadcast(buffer, 6);
		break;

	case SVMSG_BLOCKDATA:
		buffer[1] = playerid;
		buffer[2] = message[1];
		memcpy_s(buffer + 3, MSG_LEN - 3, message + 2, buffer[2] * buffer[2]);
		ServerBroadcast(buffer, 3 + buffer[2] * buffer[2]);
		break;

	case SVMSG_REQUEST:
		GameSendAllBoardData(playerid);
		break;
	}
}

int currentid = 0;

inline int ClientIDToBoardID(int id) {
	if (id < currentid)
		return id + 1;

	if (id == currentid)
		return 0;

	return id;
}

void ClientReceiveMessage(const byte *message, uint16 length) {
	switch (message[0]) {
	case SVMSG_COMMAND:
		HandleCommandString(message, false);
		break;

	case SVMSG_INFO:
		LobbySetSize(message[1]);
		currentid = message[2];
		break;
	case SVMSG_NAME:
		if (LobbyGetClientName(message[1])[0] != '\0') {
			ConsolePrint(LobbyGetClientName(message[1]));
			ConsolePrint(" changed their name to ");
			ConsolePrint(message + 2);
			ConsolePrint("\n");
		}

		LobbySetClientName(message[1], message + 2);

		break;
	case SVMSG_TEXT:
		switch (message[1]) {
		case SVTEXT_DENIED:
			ConsolePrint("Access denied\n");
			break;
		}
		break;

	case SVMSG_JOIN:
		ConsolePrint(LobbyGetClientName(message[1]));
		ConsolePrint(" joined the server\n");
		break;
	case SVMSG_CHAT:
		ConsolePrint(LobbyGetClientName(message[1]));
		ConsolePrint(" : ");
		ConsolePrint(message + 2);
		ConsolePrint("\n");
		break;
	case SVMSG_LEAVE:
		ConsolePrint(LobbyGetClientName(message[1]));
		if (message[2])
			ConsolePrint(" timed out\n");
		else
			ConsolePrint(" dipped\n");

		LobbySetClientName(message[1], "");
		GameBoardClear(ClientIDToBoardID(message[1]));
		break;

	case SVMSG_BLOCKPOS:
		GameBoardSetBlockPos(ClientIDToBoardID(message[1]), BufferToInt16(message + 2), BufferToInt16(message + 4));
		break;
	case SVMSG_BLOCKDATA:
		GameBoardSetBlockData(ClientIDToBoardID(message[1]), message[2], message + 3);
		break;
	case SVMSG_PLACE:
		GameBoardPlaceBlock(ClientIDToBoardID(message[1]));
		break;
	case SVMSG_CLEAR:
		GameBoardClear(ClientIDToBoardID(message[1]));
		break;

	case SVMSG_START:
		GameBegin(LobbyGetSize());
		break;

	case SVMSG_BOARD:
		GameReceiveBoardData(ClientIDToBoardID(message[1]), message + 2, length - 2);
		break;
	}
}

void MessageServerString(MessageID id, const char *string) {
	byte buffer[MSG_LEN];
	buffer[0] = id;

	strcpy_s(buffer + 1, MSG_LEN - 1, string);

	MessageServer(buffer, (unsigned int)strlen(string) + 2);
}

void C_Name(DvarValue string) {
	MessageServerString(SVMSG_NAME, string.string);
}

void CFunc_Send(const char **tokens, unsigned int count) {
	if (count < 1) return;

	char message[MSG_LEN];

	unsigned int current = 0;
	for (unsigned int i = 0; i < count; ++i) {
		if (i > 0) message[current++] = ' ';

		for (const char *c = tokens[i]; *c != '\0'; ++c)
			message[current++] = *c;
	}

	message[current] = '\0';
		
	MessageServerString(SVMSG_CHAT, message);
}
