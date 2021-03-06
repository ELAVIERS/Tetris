#pragma once
#include "Types.h"
#include <stdbool.h>

int ServerGetSlotCount();

void StartLocalServer();
void StartOnlineServer();
void ServerFrame();
void StopServer();

void ServerDisconnectSlot(int id);

void ServerSend(int id, const byte *buffer, uint16 size);
void ServerBroadcast(const byte *buffer, uint16 size, int sender);

void ServerSetAdmin(byte id);
bool ServerClientIsAdmin(byte id);
