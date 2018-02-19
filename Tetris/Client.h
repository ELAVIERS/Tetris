#pragma once
#include "Types.h"
#include <stdbool.h>
#include <WinSock2.h>

typedef struct {
	SOCKET socket;

	unsigned int id;
	char* name;
} Client;

void ClientFrame();
void Client_OpenConnectionDialog();

void Client_MessageServer(const byte *buffer, unsigned int count);
