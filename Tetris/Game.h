#pragma once
#include "Types.h"

void GameInit();
void GameFrame();
void GameRender();

void GameBegin(int playercount);
void GameRestart();
void GameEnd();

void GameSizeUpdate(unsigned short w, unsigned short h);

void GameSetBlockIDSize(float size);

//Net

void GameBoardSetBlockData(int id, int size, const byte *data);
void GameBoardSetBlockPos(int id, signed short x, signed short y);
void GameBoardPlaceBlock(int id);
void GameBoardClear(int id);

void GameSendAllBoardData(int playerid);
void GameReceiveBoardData(int id, const byte *data, uint16 length);
