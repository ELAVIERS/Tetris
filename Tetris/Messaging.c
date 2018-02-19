#include "Messaging.h"
#include "Client.h"
#include "Console.h"
#include "Server.h"
#include <stdio.h>
#include <string.h>

void MessageServer(const byte *message, unsigned int count) {
	IFSERVER
		ServerReceiveMessage(message, count, 0);
	else
		Client_MessageServer(message, count);
}

void ServerReceiveMessage(const byte *message, unsigned int count, byte playerid) {
	static char full_buffer[256];
	static char *buffer = full_buffer + 1;
	full_buffer[0] = message[0];
	++message;

	switch (full_buffer[0]) {
	case SVMSG_JOIN:
		SetPlayerName(playerid, message);
		full_buffer[0] = SVMSG_TALK;
		strcpy_s(buffer, 255, message);
		strcat_s(buffer, 255, " joined the server");
		break;
	case SVMSG_NAME:
		SetPlayerName(playerid, message);
		return;
	case SVMSG_TALK:
		strcpy_s(buffer, 255, GetPlayerName(playerid));
		strcat_s(buffer, 255, " : ");
		strcat_s(buffer, 255, message);
		break;

	default:return;
	}

	ServerBroadcast(full_buffer, strlen(buffer) + 2);
}

void ClientReceiveMessage(const byte *message, unsigned int count) {
	byte id = message[0];
	++message;
	
	switch (id) {
	case SVMSG_TALK:
		ConsolePrint(message);
		ConsolePrint("\n");
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
		
	MessageServerString(SVMSG_TALK, message);
}
