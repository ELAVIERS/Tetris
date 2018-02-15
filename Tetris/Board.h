#pragma once
#include "Block.h"
#include "Texture.h"
#include "Types.h"
#include <stdbool.h>

typedef struct Board_s {
	unsigned short x, y, width, height;
	byte rows, columns;

	char **data;

	Block block;
} Board;

void SetupBoard(Board*);
void BoardRender(const Board*);
void BoardFree(const Board*);

bool BoardInputDown(Board*);
void BoardInputX(Board*, int x);
void BoardInputCCW(Board*);
void BoardInputCW(Board*);

short TextureLevelIDIndex(char id);
void UseNextTextureLevel();

DFunc
	CLSetTextureIndexOrder,
	CLAddTextureLevel;

void ClearTextureLevels();

DvarCallback
	C_CLBlockTexture,
	C_CLBlockIDSize;
