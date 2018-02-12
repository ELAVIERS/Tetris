#pragma once
#include "Dvar.h"
#include "Types.h"
#include <stdbool.h>

#define RC1D(S, R, C) ((R) * (S)) + (C)
#define SQUARE(X) X * X

typedef struct {
	byte start_row;
	char id;
	unsigned int size;
	bool *data;
	short x, y;
} Block;

void RenderBlock(const Block *block, float x_offset, float y_offset, float block_w, float block_h);
void UseNextBlock(Block*, unsigned short top);

void BlockRotateCCW(Block*);
void BlockRotateCW(Block*);

DFunc SVAddBlock, SVClearBlocks;
