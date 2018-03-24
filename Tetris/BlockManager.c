#include "BlockManager.h"
#include "Console.h"
#include "Game.h"
#include "Globals.h"
#include "Matrix.h"
#include "Rendering.h"
#include "Shader.h"
#include "Types.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int PerfectSqrt(int x) {
	for (int i = 1; i < 180; ++i)
		if (x == i * i)
			return i;
		else if (i * i > x)
			return 0;

	return 0;
}

typedef struct {
	unsigned int size;
	byte *data;

	byte start_row;
	byte start_column;
	char id;

	unsigned int count;
} BlockType;

BlockType *blocktypes = NULL;
BlockType *current_type;
unsigned int type_count = 0;

unsigned int BlockTypesGetCount() {
	return type_count;
}

void CreateNewBlock(int index, Block *block, unsigned short top) {
	current_type = blocktypes + index;

	unsigned int sizesq = SQUARE(current_type->size);
	block->data = (byte*)malloc(sizesq);
	block->id = current_type->id;
	block->size = current_type->size;

	for (unsigned int i = 0; i < sizesq; ++i)
		block->data[i] = current_type->data[i];

	block->y = top - current_type->start_row;
	block->x = current_type->start_column;
}

void RenderBlockByID(int index, Mat3 transform, const Quad *quad, unsigned int level) {
	RenderTileBuffer(blocktypes[index].data, blocktypes[index].size, blocktypes[index].size, transform, quad, level);
}

void CurrentBlockIncrementCount() {
	++current_type->count;
}

void SVAddBlock(const char **tokens, unsigned int count) {
	if (count < 4) return;
	int sizesq = (int)strlen(tokens[1]);
	int size = PerfectSqrt(sizesq);
	if (size == 0) return;

	int last = type_count;
	++type_count;
	blocktypes = (BlockType*)realloc(blocktypes, type_count * sizeof(BlockType));

	blocktypes[last].id = tokens[0][0];
	blocktypes[last].start_row = atoi(tokens[2]);
	blocktypes[last].start_column = atoi(tokens[3]);
	blocktypes[last].size = size;
	blocktypes[last].data = (byte*)malloc(sizesq);
	blocktypes[last].count = 0;

	for (int i = 0; i < sizesq; ++i)
		blocktypes[last].data[i] = tokens[1][i] != '0' ? tokens[0][0] : 0;

	ByteMatrixFlipColumns(blocktypes[last].data, blocktypes[last].size);

	GameSetQueueElementCount((byte)type_count);
}

void ClearBlocks() {
	for (unsigned int i = 0; i < type_count; ++i)
		free(blocktypes[i].data);

	free(blocktypes);
	blocktypes = NULL;
	type_count = 0;
}

void ClearBlockCounts() {
	for (unsigned int i = 0; i < type_count; ++i)
		blocktypes[i].count = 0;
}

void RenderBlockPanel(Mat3 transform, float border_width, unsigned int level) {
	ShaderSetUniformFloat2(g_active_shader, "u_uvoffset", 0.f, 0.f);
	ShaderSetUniformMat3(g_active_shader, "u_transform", transform);
	glBindTexture(GL_TEXTURE_2D, 0);
	QuadRender(g_quads + QUAD_SINGLE);

	if (g_drawborder)
		RenderBorder(0, 0, transform[0][0], transform[1][1], border_width, border_width);

	unsigned int total_rows = 0;
	for (unsigned int i = 0; i < type_count; ++i)
		total_rows += blocktypes[i].size + 1;

	float block_size = (transform[1][1] - border_width * 2.f - 16.f) / (float)total_rows;

	Mat3 block_transform;
	Mat3Identity(block_transform);
	Mat3Scale(block_transform, block_size, block_size);
	Mat3Translate(block_transform, transform[2][0] + border_width + 8, transform[2][1] + border_width + 8);

	glBindTexture(GL_TEXTURE_2D, g_textures[TEX_BLOCK].glid);

	for (unsigned int i = 0; i < type_count; ++i) {
		RenderTileBuffer(blocktypes[i].data, blocktypes[i].size, blocktypes[i].size, block_transform, g_quads + QUAD_BLOCK, level);

		Mat3Translate(block_transform, 0, (blocktypes[i].size + 1) * block_size);
	}
}

void RenderBlockCounts(Mat3 transform, float border_width) {
	unsigned int total_rows = 0;
	unsigned int min_size = ~0;
	for (unsigned int i = 0; i < type_count; ++i) {
		total_rows += blocktypes[i].size + 1;
		if (blocktypes[i].size < min_size)
			min_size = blocktypes[i].size;
	}

	float block_size = (transform[1][1] - border_width * 2.f - 16.f) / (float)total_rows;

	Mat3 char_transform;
	Mat3Identity(char_transform);
	Mat3Scale(char_transform, block_size * min_size, block_size * min_size);
	Mat3Translate(char_transform, transform[2][0] + border_width + 8 + block_size * 5.f, transform[2][1] + border_width + 8);

	glBindTexture(GL_TEXTURE_2D, g_textures[TEX_FONT].glid);

	char valuestring[13];

	for (unsigned int i = 0; i < type_count; ++i) {
		snprintf(valuestring, 13, "%d", blocktypes[i].count);
		RenderString(valuestring, char_transform);

		Mat3Translate(char_transform, 0, block_size * (blocktypes[i].size + 1));
	}
}

#include "Messaging.h"
#include "Server.h"
#include "String.h"

#define FIRST_ARG_LOC 15

void SendBlockInfo(int playerid) {
	byte message[MSG_LEN];
	message[0] = SVMSG_COMMAND;

	strcpy_s(message + 1, MSG_LEN - 1, "sv_blocks_clear");
	ServerSend(playerid, message, (uint16)strlen(message + 1) + 2);

	strcpy_s(message + 1, MSG_LEN - 1, "sv_blocks_add ");

	int msgindex;

	for (unsigned int i = 0; i < type_count; ++i) {
		msgindex = FIRST_ARG_LOC;
		message[msgindex++] = blocktypes[i].id;
		message[msgindex++] = ' ';

		for (unsigned int r = 0; r < blocktypes[i].size; ++r)
			for (unsigned int c = 0; c < blocktypes[i].size; ++c)
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

//Texture Levels

typedef struct {
	char block_id;
	short index;
} TextureBinding;

typedef struct TextureLevel_s {
	TextureBinding *texdata;
	unsigned int texdata_size;
} TextureLevel;

TextureLevel *levels = NULL;
unsigned int level_count = 0;

short TextureLevelIDIndex(unsigned int level, char id) {
	level %= level_count;

	for (unsigned int i = 0; i < levels[level].texdata_size; ++i)
		if (levels[level].texdata[i].block_id == id)
			return levels[level].texdata[i].index;

	return -1;
}

char **id_groups;
unsigned int group_count = 0;
unsigned int blockid_count;

void CLSetTextureIndexOrder(const char **tokens, unsigned int count) {
	FreeTokens(id_groups, group_count);

	id_groups = (char**)malloc(count * sizeof(char*));

	blockid_count = 0;
	for (unsigned int i = 0; i < count; ++i) {
		id_groups[i] = DupString(tokens[i]);
		blockid_count += (unsigned int)strlen(tokens[i]);
	}

	group_count = count;
}

void CLAddTextureLevel(const char **tokens, unsigned int count) {
	if (count != group_count) {
		ConsolePrint("Error : Invalid argument count\n");
		return;
	}

	unsigned int last = level_count;
	++level_count;
	levels = (TextureLevel*)realloc(levels, level_count * sizeof(TextureLevel));
	levels[last].texdata = (TextureBinding*)malloc(blockid_count * sizeof(TextureBinding));
	levels[last].texdata_size = blockid_count;

	unsigned int counter = 0;
	for (unsigned int i = 0; i < group_count; ++i) {
		short texid = (short)atoi(tokens[i]);
		for (const char *c = id_groups[i]; *c != '\0'; ++c) {
			levels[last].texdata[counter].block_id = *c;
			levels[last].texdata[counter].index = texid;
			++counter;
		}
	}
}

void ClearTextureLevels() {
	free(levels);
	levels = NULL;
	level_count = 0;
}
