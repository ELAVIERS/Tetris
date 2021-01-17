#pragma once
#include "Types.h"
#include <stdbool.h>

void GameInit();
void GameFrame();
void GameRender();

void GameBegin(int playercount);
void GameRestart();
void GameEnd();

void GameSizeUpdate(unsigned short w, unsigned short h);

void GameSetBlockIDSize(float size);
void GameSetQueueLength(byte visible_length);
void GameSetQueueElementCount(byte element_count);
void GameSetBagSize(byte bag_size);

//Net
void GameBoardSetName(int id, const char *name);
void GameBoardSetLevel(int id, uint16 level);
void GameBoardAddClientScore(int id, uint32 score);
void GameBoardSetLineClears(int id, uint16 clears);
void GameBoardSetBlockData(int id, char tile_id, int size, const byte *data);
void GameBoardSetBlockPos(int id, signed short x, signed short y);
void GameBoardSetQueue(int id, byte length, const byte *queue);
void GameBoardSetHeldBlock(int id, byte blockid);
void GameBoardSetVisible(int id, bool visible);
void GameBoardPlaceBlock(int id);
void GameBoardClear(int id);
void GameBoardFinished(int id);
void GameBoardAddGarbage(int id, byte rows, byte clear_column);

void GameSendAllBoardData(int playerid);
void GameReceiveBoardData(int id, const byte *data, uint16 length);
