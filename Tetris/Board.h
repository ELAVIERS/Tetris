#pragma once
#include "Block.h"
#include "Quad.h"
#include "Texture.h"
#include "Types.h"
#include <stdbool.h>

/*
	Board.h

	It's all about rendering a bunch of tetrominos
*/

/*
	Board(struct)
	x, y			the board's position in pixels
	width, height	the board's size in pixels

	data			a 2d matrix representing the id of each grid cell
	rows, columns	the size of data

	block			the current block

	quad_blocks		the quad to be used when rendering the board
*/
typedef struct Board_s {
	short x, y;
	unsigned short width, height;

	byte **data;
	byte rows, columns;

	Block block;

	Quad quad_blocks;
} Board;

void BoardCreate(Board*);
void BoardFree(Board*);
void BoardClear(Board*);
void BoardSetIDSize(Board*, float id_size);
void BoardSubmitBlock(Board*);

void BoardUseNextBlock(Board*);

void BoardRender(const Board*);

//Input functions
bool BoardInputDown(Board*);
bool BoardInputX(Board*, int x);
bool BoardInputCCW(Board*);
bool BoardInputCW(Board*);

short TextureLevelIDIndex(char id);
void UseNextTextureLevel();

DFunc
	CLSetTextureIndexOrder,
	CLAddTextureLevel;

void ClearTextureLevels();

DvarCallback
	C_CLBlockTexture,
	C_CLBlockIDSize;
