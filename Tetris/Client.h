#pragma once
#include "Types.h"
#include <stdbool.h>

typedef struct {
	char* name;
} Client;

void ClientFrame();
void Client_OpenConnectionDialog();

void Client_MessageServer(const byte *buffer, uint16 count);

void Client_Disconnect();
