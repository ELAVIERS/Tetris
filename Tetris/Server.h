#pragma once
#include "Types.h"
#include <stdbool.h>

bool ServerIsActive();

#define IFSERVER if(ServerIsActive())

void StartServer();
void ServerFrame();
void StopServer();

void ServerBroadcast(const byte* buffer, uint16 size);
