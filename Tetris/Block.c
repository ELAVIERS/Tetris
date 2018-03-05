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
	byte start_column;
	char id;
	unsigned int size;
	byte *data;
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

void RenderTileBuffer(const byte *buffer, byte rows, byte columns, byte divsx, byte divsy, Mat3 in_transform, const Quad* quad) {
	Mat3 transform;

	for (unsigned int r = 0; r < rows; ++r)
		for (unsigned int c = 0; c < columns; ++c) {
			if (buffer[RC1D(columns, r, c)]) {
				Mat3Identity(transform);
				Mat3Translate(transform, (float)c, (float)r);
				Mat3Multiply(transform, in_transform);
				ShaderSetUniformMat3(g_active_shader, "u_transform", transform);

				short index = TextureLevelIDIndex(buffer[RC1D(columns, r, c)]);
				glUniform2f(ShaderGetLocation(g_active_shader, "u_uvoffset"),
					(float)(index % divsx) / (float)divsx, (float)(index / divsx) / (float)divsy);

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

	for (unsigned int i = 0; i < sizesq; ++i)
		block->data[i] = new_block->data[i];

	block->y = top - new_block->start_row;
	block->x = new_block->start_column;
}

void SVAddBlock(const char **tokens, unsigned int count) {
	if (count < 4) return;
	int sizesq = (int)strlen(tokens[1]);
	int size = PerfectSqrt(sizesq);
	if (size == 0) return;

	int last = type_count;
	++type_count;
	blocktypes = (BlockData*)realloc(blocktypes, type_count * sizeof(BlockData));

	blocktypes[last].id = tokens[0][0];
	blocktypes[last].start_row = atoi(tokens[2]);
	blocktypes[last].start_column = atoi(tokens[3]);
	blocktypes[last].size = size;
	blocktypes[last].data = (byte*)malloc(sizesq);

	for (int i = 0; i < sizesq; ++i)
		blocktypes[last].data[i] = tokens[1][i] != '0' ? tokens[0][0] : 0;

	BlockDataFlipColumns(blocktypes[last].data, blocktypes[last].size);
}

void ClearBlocks() {
	free(blocktypes);
	blocktypes = NULL;
	type_count = 0;
}

#include "Messaging.h"
#include "Server.h"
#include "String.h"

#define FIRST_ARG_LOC 15

void SendBlockInfo(int playerid) {
	byte message[MSG_LEN];
	message[0] = SVMSG_COMMAND;

	strcpy_s(message + 1, MSG_LEN - 1, "sv_blocks_add ");

	int msgindex;

	for (unsigned int i = 0; i < type_count; ++i) {
		msgindex = FIRST_ARG_LOC;
		message[msgindex++] = blocktypes[i].id;
		message[msgindex++] = ' ';

		for (int r = 0; r < blocktypes[i].size; ++r)
			for (int c = 0; c < blocktypes[i].size; ++c)
				message[msgindex++] = blocktypes[i].data[RC1D(blocktypes[i].size, blocktypes[i].size - 1 - r, c)] ? '1' : '0';

		message[msgindex++] = ' ';

		char *string = AllocStringFromInt(blocktypes[i].start_row);
		for (const char *c = string; *c != '\0'; ++c)
			message[msgindex++] = *c;

		free(string);

		message[msgindex++] = ' ';

		string = AllocStringFromInt(blocktypes[i].start_column);
		for (const char *c = string; *c != '\0'; ++c)
			message[msgindex++] = *c;

		message[msgindex] = '\0';

		free(string);

		ServerSend(playerid, message, msgindex + 1);
	}
}
