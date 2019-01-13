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

short BlockGetLargestY(Block *block) {
	short result = -1;

	for (byte r = 0; r < block->size; ++r) {
		for (byte c = 0; c < block->size; ++c) {
			if (r > result && block->data[RC1D(block->size, r, c)]) {
				result = r;
				break;
			}
		}
	}

	if (result >= 0)
		result += block->y;

	return result;
}
