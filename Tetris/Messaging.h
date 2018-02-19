#pragma once
#include "Dvar.h"
#include "Types.h"

#define MSG_LEN 256

typedef enum {
	SVMSG_PING = 0,
	SVMSG_JOIN = 1,
	SVMSG_NAME = 2,
	SVMSG_TALK = 3
} MessageID;

void MessageServer(const byte *data, unsigned int count);

void ServerReceiveMessage(const byte *data, unsigned int count, byte playerid);
void ClientReceiveMessage(const byte *data, unsigned int count);

void MessageServerString(MessageID id, const char *string);

void C_Name(DvarValue value);

void CFunc_Send(const char **tokens, unsigned int count);
