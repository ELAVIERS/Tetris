#pragma once
#include "Dvar.h"
#include "Types.h"
#include <stdbool.h>

#define MSG_LEN 256

typedef struct MessageBuffer_s {
	byte buffer[MSG_LEN];
	uint16 current_length;
	uint16 bytes_left;

	int error;
} MessageBuffer;

bool MessageBufferReady(MessageBuffer*);
void MessageBufferRemoveMsg(MessageBuffer*);

typedef enum {
	SVMSG_PING = 0,

	SVMSG_SERVERINFO = 1,
	/*
		CLIENT
			1:SLOT COUNT (BYTE)
	*/

	SVMSG_NAME = 2,
	/*
		SERVER
			1:PLAYER NAME (STRING)

		CLIENT
			1:PLAYER ID (BYTE)
			2:PLAYER NAME (STRING)
	*/

	SVMSG_JOIN = 3,
	/*
	CLIENT
	1:PLAYER ID (BYTE)
	*/

	SVMSG_TEXT = 4,
	/*
		SERVER
			1:CHAT (STRING)
		CLIENT
			1:PLAYER ID (BYTE)
			2:CHAT (STRING)
	*/

	SVMSG_LEAVE = 5
	/*
		CLIENT
			1:PLAYER ID (BYTE)
			2:REASON (BYTE)
	*/
} MessageID;

void MessageServer(const byte *data, unsigned int count);

void ServerReceiveMessage(const byte *data, byte playerid);
void ClientReceiveMessage(const byte *data);

void MessageServerString(MessageID type, const char *string);

void C_Name(DvarValue value);

void CFunc_Send(const char **tokens, unsigned int count);
