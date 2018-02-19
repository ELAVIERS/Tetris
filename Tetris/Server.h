#pragma once
#include "Types.h"
#include <stdbool.h>

bool ServerIsActive();

#define IFSERVER if(ServerIsActive())

void StartServer();
void ServerFrame();
void StopServer();

void ServerBroadcast(const byte* buffer, unsigned int length);

void SetPlayerName(byte playerid, const char *name);
const char *GetPlayerName(byte playerid);
