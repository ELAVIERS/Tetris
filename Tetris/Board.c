#include "Board.h"
#include "BlockManager.h"
#include "Console.h"
#include "Globals.h"
#include "Matrix.h"
#include "Quad.h"
#include "Rendering.h"
#include "RNG.h"
#include "Shader.h"
#include "String.h"
#include "Variables.h"
#include <GL/glew.h>
#include <stdlib.h>
#include <string.h>

void BoardRefillQueueSlots(Board *board) {
	for (unsigned int i = 0; i < board->visible_queue_length;)
		if (board->next_queue[i] == 0xFF) {
			GenerateBag(board->next_queue + i, board->bag_element_count);
			i += board->bag_size ? board->bag_size : board->bag_element_count;

			if (i < board->queue_length)
				board->next_queue[i] = 0xFF;
		}
		else ++i;
}

int GetNextIndexInQueue(Board *board) {
	int index = board->next_queue[0];

	for (unsigned int i = 1; i < board->queue_length; ++i)
		board->next_queue[i - 1] = board->next_queue[i];
	board->next_queue[board->queue_length - 1] = -1;

	BoardRefillQueueSlots(board);

	return index;
}

void BoardUseNextBlock(Board *board) {
	CreateNewBlock(GetNextIndexInQueue(board), &board->block, board->rows - 1);
}

void BoardCreate(Board *board) {
	byte *data_block = (byte*)malloc(board->rows * board->columns);

	board->data = (byte**)malloc(board->rows * sizeof(byte*));
	for (int i = 0; i < board->rows; ++i)
		board->data[i] = data_block + (i * board->columns);

	board->nametag = Font_NewText(&g_font);
	SetTextInfo(board->nametag, DupString("Empty"), 0, 0, 32);
	GenerateTextData(board->nametag, Font_UVSize(&g_font));

	board->block.size = 0;
	board->block.data = NULL;
	board->level = 0;
	board->score = 0;
	board->level_clears = 0;
	board->line_clears = 0;

	board->queue_length = 0;
	board->visible_queue_length = 0;
	board->bag_size = 0;
	board->bag_element_count = 0;
	board->next_queue = NULL;
}

void BoardReallocNextQueue(Board *board, byte visible_elements, byte bag_element_count) {
	unsigned int prev_queue_length = board->queue_length;
	board->visible_queue_length = visible_elements;
	board->bag_element_count = bag_element_count;

	board->queue_length = board->visible_queue_length + board->bag_element_count - 1;
	board->next_queue = (byte*)realloc(board->next_queue, board->queue_length);

	for (unsigned int i = prev_queue_length; i < board->queue_length; ++i)
		board->next_queue[i] = 0xFF;

	BoardRefillQueueSlots(board);
}

void BoardFree(Board *board) {
	free(&board->data[0][0]);
	free(board->data);
	free(board->block.data);
	free(board->next_queue);

	Font_RemoveText(&g_font, board->nametag);
	FreeText(board->nametag);
}

void BoardClear(Board *board) {
	ZeroMemory(&board->data[0][0], board->rows * board->columns);
}

void BoardSetIDSize(Board *board, float id_size) {
	QuadSetData(g_quads + QUAD_BLOCK, id_size / (float)g_textures[TEX_BLOCK].width, id_size / (float)g_textures[TEX_BLOCK].height);
}

inline bool RowIsFull(Board *board, unsigned int row) {
	for (unsigned int c = 0; c < board->columns; ++c)
		if (board->data[row][c] == 0)
			return false;

	return true;
}

inline void ClearRow(Board *board, unsigned int row) {
	for (unsigned short r = row; r < board->rows - 1; ++r)
		for (unsigned short c = 0; c < board->columns; ++c)
			board->data[r][c] = board->data[r + 1][c];

	for (unsigned short c = 0; c < board->columns; ++c)
		board->data[board->rows - 1][c] = 0;
}

int BoardClearFullLines(Board *board) {
	int clears = 0;

	for (unsigned int r = 0; r < board->rows;)
		if (RowIsFull(board, r)) {
			ClearRow(board, r);
			++clears;
		}
		else
			++r;

	return clears;
}

int BoardSubmitBlock(Board *board) {
	for (unsigned int r = 0; r < board->block.size; ++r)
		for (unsigned int c = 0; c < board->block.size; ++c)
			if (board->block.data[RC1D(board->block.size, r, c)])
				board->data[board->block.y + r][board->block.x + c] = board->block.data[RC1D(board->block.size, r, c)];

	return BoardClearFullLines(board);
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

#define CELL_SIZE 4

void BoardRender(const Board *board) {
	Mat3 transform;
	float block_w = (float)board->width / (float)(board->columns + (g_drawborder ? 2 : 0));
	float block_h = (float)board->height / (float)(board->rows + (g_drawborder ? 2 : 0));

	float x = board->x + (g_drawborder ? block_w : 0.f);
	float y = board->y + (g_drawborder ? block_h : 0.f);
	float h = block_h * (board->rows - (g_drawborder ? 0 : 2));

	float nextblock_size = (float)h / (float)(CELL_SIZE * board->visible_queue_length);
	if (nextblock_size > block_w)
		nextblock_size = block_w;
	
	glUniform2f(ShaderGetLocation(g_active_shader, "u_uvoffset"), 0.f, 0.f);

	glBindTexture(GL_TEXTURE_2D, 0);

	Mat3Identity(transform);
	Mat3Scale(transform, block_w * board->columns, block_h * board->rows);
	Mat3Translate(transform, x, y);
	ShaderSetUniformMat3(g_active_shader, "u_transform", transform);
	QuadRender(g_quads + QUAD_SINGLE);

	if (*sv_queue_size != 0) {
		Mat3Identity(transform);
		Mat3Scale(transform, block_w * CELL_SIZE, h);
		Mat3Translate(transform, board->x + board->width + block_w, board->y + block_h);
		ShaderSetUniformMat3(g_active_shader, "u_transform", transform);
		QuadRender(g_quads + QUAD_SINGLE);
	}

	if (g_drawborder) {
		RenderBorder(board->x, board->y, board->width, board->height, block_w, block_h);

		if (*sv_queue_size != 0)
			RenderBorder((float)(board->x + board->width), (float)board->y, block_w * (CELL_SIZE + 2), board->height, block_w, block_h);
	}

	glBindTexture(GL_TEXTURE_2D, g_textures[TEX_BLOCK].glid);

	Mat3Identity(transform);
	Mat3Scale(transform, block_w, block_h);
	Mat3Translate(transform, x, y);
	RenderTileBuffer(&board->data[0][0], board->rows, board->columns, transform, g_quads + QUAD_BLOCK, board->level);

	Mat3Translate(transform, board->block.x * block_w, board->block.y * block_h);
	RenderTileBuffer(board->block.data, board->block.size, board->block.size, transform, g_quads + QUAD_BLOCK, board->level);

	if (board->visible_queue_length) {
		Mat3Identity(transform);
		Mat3Scale(transform, nextblock_size, nextblock_size);

		transform[2][0] = board->x + board->width + block_w;
		transform[2][1] = board->y + board->height - block_h - (nextblock_size * CELL_SIZE);

		for (uint16 i = 0; i < board->visible_queue_length; ++i) {
			RenderBlockByID(board->next_queue[i], transform, g_quads + QUAD_BLOCK, board->level);
			Mat3Translate(transform, 0, -nextblock_size * CELL_SIZE);
		}
	}
}

void BoardRenderText(const Board *board) {
	Mat3 transform;
	float block_w = (float)board->width / (float)(board->columns + (g_drawborder ? 2 : 0));
	float block_h = (float)board->height / (float)(board->rows + (g_drawborder ? 2 : 0));

	Mat3Identity(transform);
	Mat3Translate(transform, board->x + block_w + 4, board->y + board->height - block_h - 36);
	ShaderSetUniformMat3(g_active_shader, "u_transform", transform);
	ShaderSetUniformFloat2(g_active_shader, "u_uvoffset", 0.f, 0.f);
	glBindTexture(GL_TEXTURE_2D, g_textures[TEX_FONT].glid);
	RenderText(board->nametag);

	Mat3Identity(transform);
	Mat3Scale(transform, 32, 32);
	Mat3Translate(transform, board->x + board->width - block_w - (32 * 6) - 4, board->y + block_h + 4);
	ShaderSetUniformMat3(g_active_shader, "u_transform", transform);

	static char score_string[7] = {0,0,0,0,0,0,'\0'};
	score_string[5] = '0' + board->score % 10;
	score_string[4] = '0' + board->score % 100 / 10;
	score_string[3] = '0' + board->score % 1000 / 100;
	score_string[2] = '0' + board->score % 10000 / 1000;
	score_string[1] = '0' + board->score % 100000 /	10000;
	score_string[0] = '0' + board->score % 1000000 / 100000;
	RenderString(score_string, transform);
}

unsigned short BoardCalculateExtraWidth(const Board *board, float width) {
	if (*sv_queue_size == 0)
		return 0;

	return (short)(width / (float)(board->columns + (g_drawborder ? 2 : 0))) * (CELL_SIZE + 2);
}

//Input

bool BoardInputDown(Board *board) {
	if (BoardCheckMove(board, 0, -1)) {
		--board->block.y;
		return true;
	}

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
