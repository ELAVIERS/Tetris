#pragma once
#include "Types.h"

void LobbySetSize(int size);
void LobbySetClientName(byte id, const char *name);
const char *LobbyGetClientName(byte id);

void LobbyInit();
void LobbyShow();
