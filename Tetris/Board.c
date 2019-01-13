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


void BoardUpdateGhostY(Board *board) {
	if (board->block.size == 0) return;

	int i;
	for (i = 0; BoardCheckMove(board, 0, i - 1); --i);

	board->ghost_y = board->block.y + i;
}

void BoardUseNextBlock(Board *board) {
	CreateNewBlock(GetNextIndexInQueue(board), &board->block, board->visible_rows - 1);

	BoardUpdateGhostY(board);
}

void BoardCreate(Board *board) {
	byte *data_block = (byte*)malloc(board->rows * board->columns);

	board->data = (byte**)malloc(board->rows * sizeof(byte*));
	for (int i = 0; i < board->rows; ++i)
		board->data[i] = data_block + (i * board->columns);

	board->nametag = Font_NewText(&g_font);
	SetTextInfo(board->nametag, DupString("Empty"), 0, 0, 32);
	GenerateTextData(board->nametag, Font_UVSize(&g_font));
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
	for (byte c = 0; c < board->columns; ++c)
		if (board->data[row][c] == 0)
			return false;

	return true;
}

inline void ClearRow(Board *board, unsigned int row) {
	for (byte r = row; r < board->rows - 1; ++r)
		for (byte c = 0; c < board->columns; ++c)
			board->data[r][c] = board->data[r + 1][c];

	for (byte c = 0; c < board->columns; ++c)
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
	for (byte r = 0; r < board->block.size; ++r)
		for (byte c = 0; c < board->block.size; ++c)
			if (board->block.data[RC1D(board->block.size, r, c)])
				board->data[board->block.y + r][board->block.x + c] = board->block.data[RC1D(board->block.size, r, c)];

	return BoardClearFullLines(board);
}

byte BoardSubmitGarbageQueue(Board *board) {
	byte total_rows = 0;
	for (unsigned int i = 0; i < GARBAGE_QUEUE_SIZE; ++i)
		total_rows += board->garbage_queue[i].rows;

	if (total_rows) {
		for (byte r = board->rows - 1; r >= total_rows; --r)
			for (byte c = 0; c < board->columns; ++c)
				board->data[r][c] = board->data[r - total_rows][c];

		byte row = 0;

		for (unsigned int i = 0; i < GARBAGE_QUEUE_SIZE; ++i) {
			for (byte r = 0; r < board->garbage_queue[i].rows; ++r)
				for (byte c = 0; c < board->columns; ++c)
					if (c != board->garbage_queue[i].clear_column)
						board->data[row + r][c] = 'g';
					else
						board->data[row + r][c] = 0;

			row += board->garbage_queue[i].rows;
			board->garbage_queue[i].rows = 0;
		}
	}

	return total_rows;
}

void BoardAddGarbage(Board *board, byte rows, byte clear_column) {
	unsigned int queue_slot = 0;
	for (; board->garbage_queue[queue_slot].rows != 0 && queue_slot < GARBAGE_QUEUE_SIZE; ++queue_slot);

	board->garbage_queue[queue_slot].rows += rows;
	board->garbage_queue[queue_slot].clear_column = clear_column;
}

#define CELL_SIZE 4

void BoardRender(const Board *board, bool draw_ghost) {
	Mat3 transform;
	float block_w = board->width / (float)(board->columns + (g_drawborder ? 2 : 0));
	float block_h = board->height / (float)(board->visible_rows + (g_drawborder ? 2 : 0));

	float x = board->x + (g_drawborder ? block_w : 0.f);
	float y = board->y + (g_drawborder ? block_h : 0.f);
	float h = block_h * (board->visible_rows - (g_drawborder ? 0 : 2));

	float nextblock_size = (h - block_w * (CELL_SIZE + 2)) / (float)(CELL_SIZE * board->visible_queue_length);
	if (nextblock_size > block_w)
		nextblock_size = block_w;

	glUniform2f(ShaderGetLocation(g_active_shader, "u_uvoffset"), 0.f, 0.f);

	glBindTexture(GL_TEXTURE_2D, 0);

	if (g_drawborder) {
		RenderPanel(board->x, board->y, board->width, board->height, block_w, block_h);

		glBindTexture(GL_TEXTURE_2D, 0);
		RenderPanel(board->x, board->y - block_h * 3, board->width, block_h * 3, block_w, block_h);

		glBindTexture(GL_TEXTURE_2D, 0);
		RenderPanel(board->x, board->y + board->height, board->width, block_h * 3, block_w, block_h);

		glBindTexture(GL_TEXTURE_2D, 0);
		RenderPanel(board->x + board->width, board->y - block_h * 3, block_w * (CELL_SIZE + 2), block_h * 3, block_w, block_h);

		glBindTexture(GL_TEXTURE_2D, 0);
		RenderPanel(board->x + board->width, board->y + board->height, block_w * (CELL_SIZE + 2), block_h * 3, block_w, block_h);

		glBindTexture(GL_TEXTURE_2D, 0);
		RenderPanel(board->x + board->width, board->y + board->height - block_h * (CELL_SIZE + 2), block_w * (CELL_SIZE + 2), block_h * (CELL_SIZE + 2), block_w, block_h);

		glBindTexture(GL_TEXTURE_2D, 0);
		RenderPanel(board->x + board->width, board->y, block_w * (CELL_SIZE + 2), board->height - block_h * (CELL_SIZE + 2), block_w, block_h);
	}
	else {
		RenderRect(x, y, block_w * board->columns, block_h * board->visible_rows);

		RenderRect(board->x + board->width + block_w, board->y + block_h, block_w * CELL_SIZE, h);
	}

	glBindTexture(GL_TEXTURE_2D, g_textures[TEX_BLOCK].glid);

	Mat3Identity(transform);
	Mat3Scale(transform, block_w, block_h);
	Mat3Translate(transform, x, y);
	RenderTileBuffer(&board->data[0][0], board->visible_rows, board->columns, transform, g_quads + QUAD_BLOCK, board->level);

	if (draw_ghost) {
		Mat3Translate(transform, board->block.x * block_w, board->ghost_y * block_h);
		ShaderSetUniformFloat(g_active_shader, "u_transparency", 0.5f);
		RenderTileBuffer(board->block.data, board->block.size, board->block.size, transform, g_quads + QUAD_BLOCK, board->level);
		ShaderSetUniformFloat(g_active_shader, "u_transparency", 0.f);
	}
	else transform[2][0] = x + board->block.x * block_w;

	byte renderheight;
	if (board->block.y + board->block.size > board->visible_rows)
		renderheight = board->visible_rows - board->block.y;
	else renderheight = board->block.size;

	transform[2][1] = y + board->block.y * block_h;
	RenderTileBuffer(board->block.data, renderheight, board->block.size, transform, g_quads + QUAD_BLOCK, board->level);

	if (board->held_index != 0xFF) {
		transform[2][0] = board->x + board->width + block_w;
		transform[2][1] = board->y + board->height - block_h * (CELL_SIZE + 1);
		RenderBlockByIndex(board->held_index, transform, g_quads + QUAD_BLOCK, board->level);
	}

	if (board->visible_queue_length) {
		Mat3Identity(transform);
		Mat3Scale(transform, nextblock_size, nextblock_size);

		transform[2][0] = board->x + board->width + block_w;
		transform[2][1] = board->y + block_h;

		for (int16 i = board->visible_queue_length - 1; i >= 0; --i) {
			RenderBlockByIndex(board->next_queue[i], transform, g_quads + QUAD_BLOCK, board->level);
			Mat3Translate(transform, 0, nextblock_size * CELL_SIZE);
		}
	}
}

void BoardRenderText(const Board *board) {
	Mat3 transform;
	float block_w = board->width / (float)(board->columns + (g_drawborder ? 2 : 0));
	float block_h = board->height / (float)(board->visible_rows + (g_drawborder ? 2 : 0));

	Mat3Identity(transform);
	Mat3Translate(transform, board->x + block_w + 4, board->y + board->height + block_h);
	ShaderSetUniformMat3(g_active_shader, "u_transform", transform);
	ShaderSetUniformFloat2(g_active_shader, "u_uvoffset", 0.f, 0.f);
	glBindTexture(GL_TEXTURE_2D, g_textures[TEX_FONT].glid);
	RenderText(board->nametag);

	static char score_string[7] = { 0,0,0,0,0,0,'\0' };
	if (board->score <= 999999) {
		score_string[5] = '0' + board->score % 10;
		score_string[4] = '0' + board->score % 100 / 10;
		score_string[3] = '0' + board->score % 1000 / 100;
		score_string[2] = '0' + board->score % 10000 / 1000;
		score_string[1] = '0' + board->score % 100000 / 10000;
		score_string[0] = '0' + board->score % 1000000 / 100000;
	}
	else
		score_string[0] = score_string[1] = score_string[2] = score_string[3] = score_string[4] = score_string[5] = '9';

	static char line_string[4] = { 0, 0, 0,'\0' };
	if (board->line_clears <= 999) {
		line_string[2] = '0' + board->line_clears % 10;
		line_string[1] = '0' + board->line_clears % 100 / 10;
		line_string[0] = '0' + board->line_clears % 1000 / 100;
	}
	else
		line_string[0] = line_string[1] = line_string[2] = '9';

	static char level_string[3] = { 0, 0, '\0' };
	if (board->level < 99) {
		level_string[1] = '0' + board->level % 10;
		level_string[0] = '0' + board->level % 100 / 10;
	}
	else
		level_string[0] = level_string[1] = '9';

	Mat3Identity(transform);
	Mat3Scale(transform, block_h, block_h);
	Mat3Translate(transform, board->x + block_w * ((board->columns + 2.f) / 4.f), board->y - block_h * 2);
	ShaderSetUniformMat3(g_active_shader, "u_transform", transform);
	RenderString(score_string, transform);

	transform[2][0] = board->x + board->width + block_w * ((CELL_SIZE + 2.f) / 4.f);
	transform[2][1] = board->y + board->height + block_h;
	ShaderSetUniformMat3(g_active_shader, "u_transform", transform);
	RenderString(line_string, transform);

	transform[2][0] = board->x + board->width + block_w * ((CELL_SIZE + 4.f) / 4.f);
	transform[2][1] = board->y - block_h * 2;
	ShaderSetUniformMat3(g_active_shader, "u_transform", transform);
	RenderString(level_string, transform);
}

//Input

bool BoardInputDown(Board *board) {
	if (board->block.size == 0) return false;

	if (BoardCheckMove(board, 0, -1)) {
		--board->block.y;
		return true;
	}

	return false;
}

bool BoardInputX(Board *board, int x) {
	if (x == 0) return false;

	if (BoardCheckMove(board, x, 0)) {
		board->block.x += x;
		BoardUpdateGhostY(board);
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

	BoardUpdateGhostY(board);
	return true;
}
bool BoardInputCW(Board *board) {
	BlockRotateCW(&board->block);

	if (!BoardCheckMove(board, 0, 0)) {
		BlockRotateCCW(&board->block);
		return false;
	}

	BoardUpdateGhostY(board);
	return true;
}
