#include "Messaging.h"
#include "Client.h"
#include "Console.h"
#include "Lobby.h"
#include "Server.h"
#include <stdio.h>
#include <string.h>

bool MessageBufferReady(MessageBuffer *msg) {
	if (msg->bytes_left == 0) {
		msg->bytes_left = (uint16)&msg->buffer[0];
		msg->current_length += sizeof(uint16);
	}

	if (msg->current_length >= msg->bytes_left)
		return true;

	return false;
}

void MessageBufferRemoveMsg(MessageBuffer *msg) {
	memcpy_s(msg->buffer, MSG_LEN, msg->buffer + msg->current_length, MSG_LEN - msg->current_length);
	msg->current_length = 0;
	msg->bytes_left = 0;
}

void MessageServer(const byte *message, unsigned int count) {
	IFSERVER
		ServerReceiveMessage(message, 0);
	else
		Client_MessageServer(message, count);
}

void ServerReceiveMessage(const byte *message, byte playerid) {
	static char full_buffer[MSG_LEN];
	static char *buffer;

	full_buffer[0] = message[0];
	buffer = full_buffer + 1;
	++message;

	switch (full_buffer[0]) {
	case SVMSG_NAME:
	case SVMSG_TEXT:
		buffer[0] = playerid;
		strcpy_s(buffer + 1, MSG_LEN - 2, message);
		ServerBroadcast(full_buffer, strlen(buffer + 1) + 3);
		break;
	case SVMSG_JOIN:
		buffer[0] = playerid;
		ServerBroadcast(full_buffer, 2);
		break;
	case SVMSG_LEAVE:
		buffer[0] = playerid;
		buffer[1] = message[0];
		ServerBroadcast(full_buffer, 3);
	}
}

void ClientReceiveMessage(const byte *message) {
	byte id = message[0];
	++message;
	
	switch (id) {
	case SVMSG_SERVERINFO:
		LobbySetSize(message[0]);
		break;
	case SVMSG_NAME:
		if (LobbyGetClientName(message[0])[0] != '\0') {
			ConsolePrint(LobbyGetClientName(message[0]));
			ConsolePrint(" changed their name to ");
			ConsolePrint(message + 1);
			ConsolePrint("\n");
		}

		LobbySetClientName(message[0], message + 1);

		break;
	case SVMSG_JOIN:
		ConsolePrint(LobbyGetClientName(message[0]));
		ConsolePrint(" joined the server\n");
		break;
	case SVMSG_TEXT:
		ConsolePrint(LobbyGetClientName(message[0]));
		ConsolePrint(" : ");
		ConsolePrint(message + 1);
		ConsolePrint("\n");
		break;
	case SVMSG_LEAVE:
		ConsolePrint(LobbyGetClientName(message[0]));
		if (message[1])
			ConsolePrint(" timed out\n");
		else
			ConsolePrint(" dipped\n");

		LobbySetClientName(message[0], "");
		break;
	}
}

void MessageServerString(MessageID id, const char *string) {
	byte buffer[MSG_LEN];
	buffer[0] = id;

	strcpy_s(buffer + 1, MSG_LEN - 1, string);

	MessageServer(buffer, strlen(string) + 2);
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
		
	MessageServerString(SVMSG_TEXT, message);
}
