#pragma once
#include "Dvar.h"
#include "Types.h"

short TextureLevelIDIndex(unsigned int level, char id);

DFunc
	CLSetTextureIndexOrder,
	CLAddTextureLevel;

void ClearTextureLevels();

void ExecLevelBind(uint16 level, uint16 player);
DFunc AddLevelBind;
void ClearLevelBinds();
