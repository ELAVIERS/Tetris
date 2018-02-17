#include "Block.h"
#include "Board.h"
#include "Dvar.h"
#include "Matrix.h"
#include "Quad.h"
#include "Shader.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
	byte start_row;
	char id;
	unsigned int size;
	bool *data;
} BlockData;

BlockData *blocktypes = NULL;
unsigned int type_count = 0;

/*
	Returns root between 1 and 180 (0 if not found)
*/
int PerfectSqrt(int x) {
	for (int i = 1; i < 180; ++i)
		if (x == i * i)
			return i;
		else if (i * i > x)
			return 0;

	return 0;
}

inline void BlockDataTranspose(bool *data, unsigned int size) {
	for (unsigned int r = 0; r < size; ++r)
		for (unsigned int c = r; c < size; ++c) {
			bool temp = data[RC1D(size, r, c)];
			data[RC1D(size, r, c)] = data[RC1D(size, c, r)];
			data[RC1D(size, c, r)] = temp;
		}
}

inline void BlockDataFlipRows(bool *data, unsigned int size) {
	for (unsigned int r = 0; r < size; ++r)
		for (unsigned int c = 0; c < size / 2; ++c) {
			bool temp = data[RC1D(size, r, c)];
			data[RC1D(size, r, c)] = data[RC1D(size, r, size - 1 - c)];
			data[RC1D(size, r, size - 1 - c)] = temp;
		}
}

inline void BlockDataFlipColumns(bool *data, unsigned int size) {
	for (unsigned int c = 0; c < size; ++c)
		for (unsigned int r = 0; r < size / 2; ++r) {
			bool temp = data[RC1D(size, r, c)];
			data[RC1D(size, r, c)] = data[RC1D(size, size - 1 - r, c)];
			data[RC1D(size, size - 1 - r, c)] = temp;
		}
}

void BlockRotateCCW(Block *block) {
	BlockDataTranspose(block->data, block->size);
	BlockDataFlipRows(block->data, block->size);
}

void BlockRotateCW(Block *block) {
	BlockDataTranspose(block->data, block->size);
	BlockDataFlipColumns(block->data, block->size);
}

void RenderBlock(const Block *block, const Quad *quad, float x_offset, float y_offset, float block_w, float block_h) {
	if (!blocktypes)
		return;

	Mat3 transform;

	unsigned int sizesq = SQUARE(block->size);
	for (unsigned int i = 0; i < sizesq; ++i) {
		if (block->data[i]) {
			Mat3Identity(transform);
			Mat3Translate(transform, block->x + (x_offset / block_w) + (float)(i % block->size), block->y + (y_offset / block_h) + (float)(i / block->size));
			Mat3Scale(transform, block_w, block_h);
			ShaderSetUniformMat3(g_active_shader, "u_transform", transform);

			QuadRender(quad);
		};
	}
}

void BlockSetRandom(Block *block, unsigned short top) {
	BlockData *new_block = &blocktypes[(unsigned int)(((float)rand() / (float)(RAND_MAX + 1)) * type_count)];

	unsigned int sizesq = SQUARE(new_block->size);
	block->data = (bool*)malloc(sizesq * sizeof(bool));
	block->id = new_block->id;
	block->size = new_block->size;
	block->start_row = new_block->start_row;

	for (unsigned int i = 0; i < sizesq; ++i)
		block->data[i] = new_block->data[i];

	block->y = top - new_block->start_row;
	block->x = 2;
}

void SVAddBlock(const char **tokens, unsigned int count) {
	if (count < 3) return;
	int sizesq = (int)strlen(tokens[1]);
	int size = PerfectSqrt(sizesq);
	if (size == 0) return;

	int rotation_point = atoi(tokens[0]);

	int last = type_count;
	++type_count;
	blocktypes = (BlockData*)realloc(blocktypes, type_count * sizeof(BlockData));

	blocktypes[last].id = tokens[0][0];
	blocktypes[last].start_row = atoi(tokens[2]);
	blocktypes[last].size = size;
	blocktypes[last].data = (bool*)malloc(sizesq * sizeof(bool));

	for (int i = 0; i < sizesq; ++i)
		blocktypes[last].data[i] = tokens[1][i] != '0';

	BlockDataFlipColumns(blocktypes[last].data, blocktypes[last].size);
}

void ClearBlocks() {
	free(blocktypes);
	blocktypes = NULL;
	type_count = 0;
}
