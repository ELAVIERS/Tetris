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

Texture tex_blocks;
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

inline void BoardUseNextBlock(Board *board) {
	UseNextBlock(&board->block, board->rows - 1);
}

Board* BoardCreate() {
	Board *board = (Board*)malloc(sizeof(Board));

	board->rows = (byte)HDFloatValue(GetDvar("sv_board_height"));
	board->columns = (byte)HDFloatValue(GetDvar("sv_board_width"));

	board->width = 384;
	board->height = 768;

	board->x = 0;
	board->y = 0;
	
	board->data = (char**)malloc(board->rows * sizeof(char*));
	for (byte i = 0; i < board->rows; ++i)
		board->data[i] = (char*)calloc(board->columns, sizeof(char));

	float texidsize = HDFloatValue(GetDvar("cl_texid_size"));
	QuadInit(texidsize / (float)tex_blocks.width, texidsize / (float)tex_blocks.height);

	BoardUseNextBlock(board);

	return board;
}

void BoardFree(Board *board) {
	free(board->block.data);
	free(board->data);
	free(board);
}

inline void ClearRow(Board *board, unsigned int row) {
	for (unsigned int r = row; r < board->rows - 1; ++r)
		for (unsigned int c = 0; c < board->columns; ++c)
			board->data[r][c] = board->data[r + 1][c];

	for (unsigned int c = 0; c < board->columns; ++c)
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

inline void BoardSubmitBlock(Board *board) {
	for (unsigned int r = 0; r < board->block.size; ++r)
		for (unsigned int c = 0; c < board->block.size; ++c)
			if (board->block.data[RC1D(board->block.size, r, c)])
				board->data[board->block.y + r][board->block.x + c] = board->block.id;

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

void BoardRender(const Board *board) {
	glBindTexture(GL_TEXTURE_2D, tex_blocks.glid);

	Mat3 transform;

	float block_w = (float)(int)((float)board->width / (float)board->columns);
	float block_h = (float)(int)((float)board->height / (float)board->rows);

	for (unsigned int r = 0; r < board->rows; ++r)
		for (unsigned int c = 0; c < board->columns; ++c) {
			if (board->data[r][c]) {
				Mat3Identity(transform);
				Mat3Translate(transform, (board->x / block_w) + (float)c, (board->y / block_h) + (float)r);
				Mat3Scale(transform, block_w, block_h);
				ShaderSetUniformMat3(g_active_shader, "u_transform", transform);

				short index = TextureLevelIDIndex(board->data[r][c]);
				glUniform2f(ShaderGetLocation(g_active_shader, "u_uvoffset"), 
					(float)(index % tex_blocks_divx) / (float)tex_blocks_divx, (float)(index / tex_blocks_divx) / (float)tex_blocks_divy);

				QuadRender();
			};
		}

	short index = TextureLevelIDIndex(board->block.id);
	glUniform2f(ShaderGetLocation(g_active_shader, "u_uvoffset"),
		(float)(index % tex_blocks_divx) / (float)tex_blocks_divx, (float)(index / tex_blocks_divx) / (float)tex_blocks_divy);

	RenderBlock(&board->block, board->x, board->y, block_w, block_h);
}

//Input

bool BoardInputDown(Board *board) {
	bool valid = BoardCheckMove(board, 0, -1);

	if (valid)
		--board->block.y;
	else {
		BoardSubmitBlock(board);
		BoardUseNextBlock(board);
	}

	return valid;
}

void BoardInputLeft(Board *board) {
	if (BoardCheckMove(board, -1, 0))
		--board->block.x;
}

void BoardInputRight(Board *board) {
	if (BoardCheckMove(board, 1, 0))
		++board->block.x;
}

void BoardInputCCW(Board *board) {
	BlockRotateCCW(&board->block);
	
	if (!BoardCheckMove(board, 0, 0))
		BlockRotateCW(&board->block);
}
void BoardInputCW(Board *board) {
	BlockRotateCW(&board->block);

	if (!BoardCheckMove(board, 0, 0))
		BlockRotateCCW(&board->block);
}


////

char **id_groups;
unsigned int group_count;
unsigned int blockid_count;

TextureLevel *first_level;

void UseNextTextureLevel() {
	current_level = current_level->next_level;
	
	if (!current_level) current_level = first_level;
}

void CLSetTextureIndexOrder(const char **tokens, unsigned int count) {
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

void CLClearTextureLevels(const char **tokens, unsigned int count) {
	TextureLevel *next;
	
	while (first_level) {
		next = first_level->next_level;
		free(first_level->texdata);
		free(first_level);
		first_level = next;
	}
}

void C_CLBlockTexture(DvarValue dstring) {
	TextureFromFile(dstring.dstring, &tex_blocks);
}

void C_CLBlockIDSize(DvarValue dfloat) {
	tex_blocks_divx = tex_blocks.width / (unsigned short)dfloat.dfloat;
	tex_blocks_divy = tex_blocks.height / (unsigned short)dfloat.dfloat;
}
