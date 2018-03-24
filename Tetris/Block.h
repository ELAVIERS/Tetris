#pragma once
#include "Types.h"

/*
	Block.h

	Tetrominos
*/

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

void BlockRotateCCW(Block*);
void BlockRotateCW(Block*);
