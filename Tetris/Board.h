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

Board* BoardCreate();
void BoardRender(const Board*);
void BoardFree(Board*);

bool BoardInputDown(Board*);
void BoardInputLeft(Board*);
void BoardInputRight(Board*);
void BoardInputCCW(Board*);
void BoardInputCW(Board*);

short TextureLevelIDIndex(char id);
void UseNextTextureLevel();

DFunc
	CLSetTextureIndexOrder,
	CLAddTextureLevel,
	CLClearTextureLevels;

DvarCallback
	C_CLBlockTexture,
	C_CLBlockIDSize;
