#pragma once
#include "Dvar.h"

void GameInit();
void GameFrame();
void GameRender();

void GameBegin(int playercount);
void GameEnd();

void GameSetBlockDropTime(float time);

void GameSizeUpdate(unsigned short w, unsigned short h);

DFunc C_GameHardDrop;
