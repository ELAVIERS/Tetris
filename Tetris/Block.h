#pragma once
#include "Dvar.h"
#include "Quad.h"
#include "Types.h"
#include <stdbool.h>

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
	unsigned short size;
	bool *data;
	short x, y;
} Block;

/*
	RenderBlock

	block(in)			block to render
	quad(in)			the quad used to render the blocks
	x_offset, y_offset	the amount to offset the blocks positions in pixels
	block w, block h	the size of a block
*/
void RenderBlock(const Block *block, const Quad *quad, float x_offset, float y_offset, float block_w, float block_h);

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
