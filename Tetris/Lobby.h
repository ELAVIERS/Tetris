#pragma once
#include "Types.h"

void LobbySetSize(int size);
int LobbyGetSize();
void LobbySetClientName(byte id, const char *name);
void LobbySetClientLevel(byte id, uint16 level);
void LobbySetClientScore(byte id, uint32 score);
void LobbyAddClientScore(byte id, uint32 score);
void LobbySetClientLineScore(byte id, uint32 line_clears);
const char *LobbyGetClientName(byte id);

void LobbyInit();
void LobbyShow();
