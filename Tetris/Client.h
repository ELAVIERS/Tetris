#pragma once
#include "Types.h"
#include <stdbool.h>

typedef struct {
	char* name;
} Client;

bool IsRemoteClient();

void ClientFrame();
void Client_RunConnectionDialog();

void Client_MessageServer(const byte *buffer, uint16 count);

void Client_Disconnect();
