#pragma once
#include "Dvar.h"
#include "Types.h"
#include <stdbool.h>

#define MSG_LEN 256

typedef struct MessageBuffer_s {
	byte buffer[MSG_LEN];
	byte *dynamic_buffer;

	uint16 length;
	uint16 bytes_left;

	int error;
} NetMessage;

typedef enum TextString_e {
	SVTEXT_DENIED
} TextString;

typedef enum MessageID_e {
	SVMSG_PING,

	SVMSG_INFO,
	/*
		CLIENT
			1:SLOT COUNT (BYTE)
			2:PLAYER ID (BYTE)
	*/

	SVMSG_COMMAND,
	/*
		SERVER
		CLIENT
			1:COMMAND (STRING)
	*/

	SVMSG_NAME,
	/*
		SERVER
			1:PLAYER NAME (STRING)

		CLIENT
			1:PLAYER ID (BYTE)
			2:PLAYER NAME (STRING)
	*/

	SVMSG_LEVEL,
	/*
			1:PLAYER ID (BYTE)
			2:LEVEL (INT16)
	*/

	SVMSG_SCORE,
	/*
		SERVER
			1:SCORE (INT32)
		CLIENT
			1:PLAYER ID (BYTE)
			2:SCORE (INT32)
	*/

	SVMSG_LINESCORE,
	/*
		CLIENT
			1:PLAYER ID (BYTE)
			1:LINE CLEARS (INT32)
	*/

	SVMSG_QUEUE,
	/*
		SERVER
			1:QUEUE LENGTH (BYTE)
			2:QUEUE (BYTE ARRAY)
		CLIENT
			1:PLAYER ID (BYTE)
			2:QUEUE LENGTH (BYTE)
			3:QUEUE (BYTE ARRAY)
	*/

	SVMSG_TEXT,
	/*
		CLIENT
			1:MESSAGE ID (TEXTSTRING)
	*/

	SVMSG_JOIN,
	/*
	CLIENT
		1:PLAYER ID (BYTE)
	*/

	SVMSG_CHAT,
	/*
		SERVER
			1:CHAT (STRING)
		CLIENT
			1:PLAYER ID (BYTE)
			2:CHAT (STRING)
	*/

	SVMSG_LEAVE,
	/*
		CLIENT
			1:PLAYER ID (BYTE)
			2:REASON (BYTE)
	*/

	SVMSG_BLOCKPOS,
	/*
		SERVER
			1:X (INT16)
			3:Y (INT16)
		CLIENT
			1:PLAYER ID (BYTE)
			2:X (INT16)
			4:Y (INT16)
	*/

	SVMSG_BLOCKDATA,
	/*
		SERVER
			1:SIZE (BYTE)
			2:DATA (BYTE ARRAY)
		CLIENT
			1:PLAYER ID (BYTE)
			2:SIZE (BYTE)
			3:DATA (BYTE ARRAY)
	*/

	SVMSG_PLACE,
	/*
		CLIENT
			1:PLAYER ID (BYTE)
	*/

	SVMSG_CLEAR,
	/*
		CLIENT
			1:PLAYER ID (BYTE)
	*/

	SVMSG_START,

	SVMSG_BOARD,
	/*
		CLIENT
			1:PLAYER ID (BYTE)
			2:BOARD DATA (BYTE ARRAY)
	*/

	SVMSG_REQUEST
} MessageID;

void MessageServer(const byte *data, uint16 length);

void ServerReceiveMessage(const byte *data, uint16 length, byte playerid);
void ClientReceiveMessage(const byte *data, uint16 length);

void MessageServerString(MessageID type, const char *string);
