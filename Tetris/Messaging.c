#include "Messaging.h"
#include "Client.h"
#include "Console.h"
#include "Dvar.h"
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
	char buffer[256];

	buffer[0] = message[0];

	switch (message[0]) {
	case SVMSG_COMMAND:
		if (ServerClientIsAdmin(playerid))
			HandleCommandString(message + 1, false);
		else {
			byte deny[] = { SVMSG_TEXT, SVTEXT_DENIED };
			ServerSend(playerid, deny, sizeof(deny));
		}
		break;
		
	case SVMSG_NAME:
	case SVMSG_CHAT:
		buffer[1] = playerid;
		strcpy_s(buffer + 2, MSG_LEN - 3, message + 1);
		ServerBroadcast(buffer, (uint16)strlen(buffer + 2) + 3, -1);
		break;

	case SVMSG_JOIN:
	case SVMSG_PLACE:
	case SVMSG_STOP:
		buffer[1] = playerid;
		ServerBroadcast(buffer, 2, playerid);
		break;

	case SVMSG_CLEAR:
		buffer[1] = playerid;
		ServerBroadcast(buffer, 2, -1);
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
		ServerBroadcast(buffer, 6, playerid);
		break;

	case SVMSG_SCORE:
		buffer[1] = playerid;
		buffer[2] = message[1];
		buffer[3] = message[2];
		buffer[4] = message[3];
		buffer[5] = message[4];
		ServerBroadcast(buffer, 6, -1);
		break;

	case SVMSG_BLOCKDATA:
		buffer[1] = playerid;
		buffer[2] = message[1];		//id
		buffer[3] = message[2];		//size
		memcpy_s(buffer + 4, MSG_LEN - 4, message + 3, buffer[3] * buffer[3]);
		ServerBroadcast(buffer, 4 + buffer[3] * buffer[3], playerid);
		break;

	case SVMSG_QUEUE:
		buffer[1] = playerid;
		buffer[2] = message[1];
		memcpy_s(buffer + 3, MSG_LEN - 3, message + 2, buffer[2]);
		ServerBroadcast(buffer, 3 + buffer[2], playerid);
		break;

	case SVMSG_HOLD:
		buffer[1] = playerid;
		buffer[2] = message[1];
		ServerBroadcast(buffer, 3, playerid);
		break;

	case SVMSG_REQUEST:
		GameSendAllBoardData(playerid);
		break;
	}
}

int clientid = 0;

inline int ClientIDToBoardID(int id) {
	if (id < clientid)
		return id + 1;

	if (id == clientid)
		return 0;

	return id;
}

void ClientReceiveMessage(const byte *message, uint16 length) {
	byte boardid = length > 1 ? ClientIDToBoardID(message[1]) : 0;

	switch (message[0]) {
	case SVMSG_COMMAND:
		HandleCommandString(message, false);
		break;

	case SVMSG_INFO:
		LobbySetSize(message[1]);
		clientid = message[2];
		break;
	case SVMSG_NAME:
		if (LobbyGetClientName(message[1])[0] != '\0') {
			ConsolePrint(LobbyGetClientName(message[1]));
			ConsolePrint(" changed their name to ");
			ConsolePrint(message + 2);
			ConsolePrint("\n");
		}

		LobbySetClientName(message[1], message + 2);
		GameBoardSetName(boardid, message + 2);
		break;
	case SVMSG_LEVEL:
		LobbySetClientLevel(message[1], BufferToInt16(message + 2));
		GameBoardSetLevel(boardid, BufferToInt16(message + 2));
		break;
	case SVMSG_SCORE:
		LobbyAddClientScore(message[1], BufferToInt32(message + 2));
		GameBoardAddClientScore(boardid, BufferToInt32(message + 2));
		break;
	case SVMSG_LINESCORE:
		LobbySetClientLineScore(message[1], BufferToInt16(message + 2));
		GameBoardSetLineClears(boardid, BufferToInt16(message + 2));
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
		GameBoardClear(boardid);
		GameBoardSetName(boardid, "Empty");
		GameBoardSetVisible(boardid, false);
		break;

	case SVMSG_BLOCKPOS:
		GameBoardSetBlockPos(boardid, BufferToInt16(message + 2), BufferToInt16(message + 4));
		GameBoardSetVisible(boardid, true);
		break;
	case SVMSG_BLOCKDATA:
		GameBoardSetBlockData(boardid, message[2], message[3], message + 4);
		GameBoardSetVisible(boardid, true);
		break;
	case SVMSG_QUEUE:
		GameBoardSetQueue(boardid, message[2], message + 3);
		GameBoardSetVisible(boardid, true);
		break;
	case SVMSG_PLACE:
		GameBoardPlaceBlock(boardid);
		GameBoardSetVisible(boardid, true);
		break;
	case SVMSG_CLEAR:
		GameBoardClear(boardid);
		LobbySetClientLevel(message[1], 0);
		LobbySetClientScore(message[1], 0);
		LobbySetClientLineScore(message[1], 0);
		GameBoardSetVisible(boardid, true);
		break;

	case SVMSG_GARBAGE:
		GameBoardAddGarbage(boardid, message[2], message[3]);
		GameBoardSetVisible(boardid, true);
		break;

	case SVMSG_HOLD:
		GameBoardSetHeldBlock(boardid, message[2]);
		GameBoardSetVisible(boardid, true);
		break;

	case SVMSG_START:
		GameBegin(LobbyGetSize());
		break;

	case SVMSG_STOP:
		GameBoardFinished(boardid);
		GameBoardSetVisible(boardid, true);
		break;

	case SVMSG_BOARD:
		GameReceiveBoardData(boardid, message + 2, length - 2);
		GameBoardSetVisible(boardid, true);
		break;
	}
}

void MessageServerString(MessageID id, const char *string) {
	byte buffer[MSG_LEN];
	buffer[0] = id;

	strcpy_s(buffer + 1, MSG_LEN - 1, string);

	MessageServer(buffer, (uint16)strlen(string) + 2);
}
