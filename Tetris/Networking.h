#pragma once
#include "Messaging.h"
#include "Types.h"
#include <WinSock2.h>

void NetworkingError(const char *errorstr, int code);

void NetworkingInit();
SOCKET NetworkCreateClientSocket(const char *ip, const char *port);
SOCKET NetworkCreateListenSocket(const char *port);

bool NetworkReceiveMsgBuffer(SOCKET socket, NetMessage *msg);
void NetworkSend(SOCKET socket, const byte *buffer, uint16 size);
