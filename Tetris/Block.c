#include "Block.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	unsigned int size;
	bool *data;
} Block;

Block *blocks = NULL;
unsigned int block_count = 0;

/*
	Returns root between 1 and 180 (0 if not found)
*/
int PerfectSqrt(int x) {
	for (int i = 1; i < 180; ++i)
		if (x == i * i)
			return i;

	return 0;
}

void SVAddBlock(const char **tokens, unsigned int count) {
	if (count < 3) return;
	int sizesq = (int)strlen(tokens[1]);
	int size = PerfectSqrt(sizesq);
	if (size == 0) return;

	int rotation_point = atoi(tokens[0]);

	Block block;
	block.size = size;
	block.data = (bool*)malloc(sizesq * sizeof(bool));

	for (int i = 0; i < sizesq; ++i)
		block.data[i] = tokens[1][i] != '0';
}

void SVClearBlocks(const char **tokens, unsigned int count) {
	free(blocks);
	block_count = 0;
}
