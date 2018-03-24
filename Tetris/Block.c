#include "Block.h"
#include "Matrix.h"

void BlockRotateCCW(Block *block) {
	ByteMatrixTranspose(block->data, block->size);
	ByteMatrixFlipRows(block->data, block->size);
}

void BlockRotateCW(Block *block) {
	ByteMatrixTranspose(block->data, block->size);
	ByteMatrixFlipColumns(block->data, block->size);
}
