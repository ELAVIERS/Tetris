#include "Board.h"
#include "Console.h"
#include "Dvar.h"
#include "Globals.h"
#include "Matrix.h"
#include "Quad.h"
#include "Shader.h"
#include "String.h"
#include <GL/glew.h>
#include <stdlib.h>
#include <string.h>

unsigned short tex_blocks_divx, tex_blocks_divy;

typedef struct {
	char block_id;
	short index;
} TextureBinding;

typedef struct TextureLevel_s {
	TextureBinding *texdata;
	unsigned int texdata_size;

	struct TextureLevel_s *next_level;
} TextureLevel;

TextureLevel *current_level;

void BoardUseNextBlock(Board *board) {
	BlockSetRandom(&board->block, board->rows - 1);
}

void BoardCreate(Board *board) {
	byte *data_block = (byte*)malloc(board->rows * board->columns);

	board->data = (byte**)malloc(board->rows * sizeof(byte*));
	for (int i = 0; i < board->rows; ++i)
		board->data[i] = data_block + (i * board->columns);

	QuadCreate(&board->quad);

	board->block.size = 0;
	board->block.data = NULL;
}

void BoardFree(Board *board) {
	QuadDelete(&board->quad);

	free(&board->data[0][0]);
	free(board->data);
	free(board->block.data);
}

void BoardClear(Board *board) {
	for (byte i = 0; i < board->rows; ++i)
		ZeroMemory(board->data[i], board->columns);
}

void BoardSetIDSize(Board *board, float id_size) {
	Texture* tex_block = g_textures + TEX_BLOCK;

	QuadSetData(&board->quad, id_size / (float)tex_block->width, id_size / (float)tex_block->height);

	tex_blocks_divx = tex_block->width / (unsigned short)id_size;
	tex_blocks_divy = tex_block->height / (unsigned short)id_size;
}

inline void ClearRow(Board *board, unsigned int row) {
	for (unsigned short r = row; r < board->rows - 1; ++r)
		for (unsigned short c = 0; c < board->columns; ++c)
			board->data[r][c] = board->data[r + 1][c];

	for (unsigned short c = 0; c < board->columns; ++c)
		board->data[board->rows - 1][c] = 0;
}

inline bool RowIsFull(Board *board, unsigned int row) {
	for (unsigned int c = 0; c < board->columns; ++c)
		if (board->data[row][c] == 0)
			return false;

	return true;
}

void boardCheckForClears(Board *board) {
	for (unsigned int r = 0; r < board->rows;)
		if (RowIsFull(board, r))
			ClearRow(board, r);
		else
			++r;
}

void BoardSubmitBlock(Board *board) {
	for (unsigned int r = 0; r < board->block.size; ++r)
		for (unsigned int c = 0; c < board->block.size; ++c)
			if (board->block.data[RC1D(board->block.size, r, c)])
				board->data[board->block.y + r][board->block.x + c] = board->block.data[RC1D(board->block.size, r, c)];

	boardCheckForClears(board);
}

bool BoardCheckMove(const Board *board, short x, short y) {
	unsigned int size = board->block.size;
	unsigned int sizesq = size * size;;

	short r, c;

	for (unsigned int i = 0; i < sizesq; ++i) {
		if (board->block.data[i]) {
			r = board->block.y + y + (i / size);
			c = board->block.x + x + (i % size);

			if (r < 0 || c < 0 || r >= board->rows || c >= board->columns || board->data[r][c])
				return false;
		}
	}

	return true;
}

short TextureLevelIDIndex(char id) {
	for (unsigned int i = 0; i < current_level->texdata_size; ++i)
		if (current_level->texdata[i].block_id == id)
			return current_level->texdata[i].index;

	return -1;
}

inline void RenderEdgePart(Mat3 transform, int id) {
	ShaderSetUniformMat3(g_active_shader, "u_transform", transform);
	glBindTexture(GL_TEXTURE_2D, g_textures[id].glid);
	QuadRender(&g_defquad);
}

void BoardRenderBorder(const Board *board, float bw, float bh) {
	Mat3 transform;
	float w = bw * (board->columns + 1);
	float h = bh * (board->rows + 1);
	
	Mat3Identity(transform);
	Mat3Scale(transform, bw, bh);
	Mat3Translate(transform, board->x, board->y);
	RenderEdgePart(transform, TEX_BL);

	Mat3Translate(transform, 0, h);
	RenderEdgePart(transform, TEX_UL);

	Mat3Translate(transform, w, 0);
	RenderEdgePart(transform, TEX_UR);

	Mat3Translate(transform, 0, -h);
	RenderEdgePart(transform, TEX_BR);

	//Horizontal edges
	Mat3Identity(transform);
	Mat3Scale(transform, w - bw, bh);
	Mat3Translate(transform, board->x + bw, board->y);
	RenderEdgePart(transform, TEX_B);

	Mat3Translate(transform, 0.f, h);
	RenderEdgePart(transform, TEX_U);

	//Vertical Edges
	Mat3Identity(transform);
	Mat3Scale(transform, bw, h - bh);
	Mat3Translate(transform, board->x, board->y + bh);
	RenderEdgePart(transform, TEX_L);

	Mat3Translate(transform, w, 0.f);
	RenderEdgePart(transform, TEX_R);
}

void BoardRender(const Board *board) {
	Mat3 transform;
	float block_w = (float)board->width / (float)(board->columns + (g_drawborder ? 2 : 0));
	float block_h = (float)board->height / (float)(board->rows + (g_drawborder ? 2 : 0));

	float x = board->x + (g_drawborder ? block_w : 0.f);
	float y = board->y + (g_drawborder ? block_h : 0.f);
	float w = block_w * board->columns;
	float h = block_h * board->rows;
	
	float uvoffset[2] = { 0.f, 0.001f / g_textures[TEX_BLOCK].height }; //stupid but it works

	glUniform2f(ShaderGetLocation(g_active_shader, "u_uvoffset"), 0.f, 0.f);

	Mat3Identity(transform);
	Mat3Scale(transform, w, h);
	Mat3Translate(transform, x, y);
	ShaderSetUniformMat3(g_active_shader, "u_transform", transform);
	glBindTexture(GL_TEXTURE_2D, 0);
	QuadRender(&board->quad);

	if (g_drawborder)
		BoardRenderBorder(board, block_w, block_h);

	Mat3Identity(transform);
	Mat3Scale(transform, block_w, block_h);
	Mat3Translate(transform, x, y);
	glBindTexture(GL_TEXTURE_2D, g_textures[TEX_BLOCK].glid);
	RenderTileBuffer(&board->data[0][0], board->rows, board->columns, tex_blocks_divx, tex_blocks_divy, transform, &board->quad, uvoffset);

	Mat3Translate(transform, board->block.x * block_w, board->block.y * block_h);
	RenderTileBuffer(board->block.data, board->block.size, board->block.size, tex_blocks_divx, tex_blocks_divy, transform, &board->quad, uvoffset);
}

//Input

bool BoardInputDown(Board *board) {
	if (BoardCheckMove(board, 0, -1)) {
		--board->block.y;
		return true;
	}

	BoardSubmitBlock(board);
	BoardUseNextBlock(board);

	return false;
}

bool BoardInputX(Board *board, int x) {
	if (BoardCheckMove(board, x, 0)) {
		board->block.x += x;
		return true;
	}

	return false;
}

bool BoardInputCCW(Board *board) {
	BlockRotateCCW(&board->block);
	
	if (!BoardCheckMove(board, 0, 0)) {
		BlockRotateCW(&board->block);
		return false;
	}

	return true;
}
bool BoardInputCW(Board *board) {
	BlockRotateCW(&board->block);

	if (!BoardCheckMove(board, 0, 0)) {
		BlockRotateCCW(&board->block);
		return false;
	}

	return true;
}


////

char **id_groups;
unsigned int group_count = 0;
unsigned int blockid_count;

TextureLevel *first_level;

void UseNextTextureLevel() {
	current_level = current_level->next_level;
	
	if (!current_level) current_level = first_level;
}

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

	TextureLevel *new_level = (TextureLevel*)malloc(sizeof(TextureLevel));
	new_level->next_level = NULL;
	new_level->texdata = (TextureBinding*)malloc(blockid_count * sizeof(TextureBinding));
	new_level->texdata_size = blockid_count;

	unsigned int counter = 0;
	for (unsigned int i = 0; i < group_count; ++i) {
		short texid = (short)atoi(tokens[i]);
		for (const char *c = id_groups[i]; *c != '\0'; ++c) {
			new_level->texdata[counter].block_id = *c;
			new_level->texdata[counter].index = texid;
			++counter;
		}
	}

	if (!first_level) {
		first_level = new_level;
		current_level = first_level;
	}
	else {
		TextureLevel *level = first_level;
		for (; level->next_level; level = level->next_level);

		level->next_level = new_level;
	}
}

void ClearTextureLevels() {
	TextureLevel *next;
	
	while (first_level) {
		next = first_level->next_level;
		free(first_level->texdata);
		free(first_level);
		first_level = next;
	}
}

void C_CLBlockTexture(DvarValue string) {
	TextureFromFile(string.string, g_textures + TEX_BLOCK);
}
