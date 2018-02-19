#pragma once
#include <WinSock2.h>

void NetworkingError(const char *errorstr, int code);

void NetworkingInit();
SOCKET NetworkCreateClientSocket(const char *ip, const char *port);
SOCKET NetworkCreateListenSocket(const char *port);
