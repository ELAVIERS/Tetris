#pragma once
#include "Dvar.h"
#include "Quad.h"
#include "Matrix.h"
#include "Texture.h"
#include "Types.h"

/*
	Block.h

	Tetrominos
*/

//Pretty basic defines that are here because Board.h might want to use them
#define RC1D(S, R, C) ((R) * (S)) + (C)
#define SQUARE(X) X * X

/*
	Block (struct)

	
	id			the block's character id
	data		represents a square matrix defining the shape of the block
	size		the amount of rows and columns data has
	x, y		the block's x and y coordinates
*/
typedef struct Block_s {
	char id;
	byte size;
	byte *data;
	short x, y;
} Block;

/*
	RenderBlock

	buffer				data to render
	rows, columns		buffer dimensions
	divsx, divsy		
	transform			transformation to apply after block translation
	quad				the quad to render tiles with
	uv_offset			offsets for correcting uv coordinates
*/
void RenderTileBuffer(const byte *buffer, byte rows, byte columns, byte divsx, byte divsy, Mat3 in_transform, const Quad* quad, float uv_offset[2]);

/*
	BlockSetRandom
	Retrieves a random block

	block(out)			the block to become random
	top					which row to place the top row of the block at
*/
void BlockSetRandom(Block *block, unsigned short top);

void BlockRotateCCW(Block*);
void BlockRotateCW(Block*);

//sv_add_block
DFunc SVAddBlock;

//sv_clear_blocks
void ClearBlocks();

//Sends commands to a player defining block info
void SendBlockInfo(int playerid);
