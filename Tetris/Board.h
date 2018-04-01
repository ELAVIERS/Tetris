#pragma once
#include "Block.h"
#include "Quad.h"
#include "Text.h"
#include "Texture.h"
#include "Types.h"
#include <stdbool.h>

/*
	Board.h

	It's all about rendering a bunch of tetrominos
*/

/*
	Board(struct)
	x, y			the board's position in pixels
	width, height	the board's size in pixels

	data			a 2d matrix representing the id of each grid cell
	rows, columns	the size of data

	block			the current block
*/
typedef struct Board_s {
	float x, y;
	float width, height;

	byte **data;
	byte rows, columns, visible_rows;

	Block block;
	short ghost_y;

	Text *nametag;

	uint16 level;
	uint16 level_clears;
	uint32 line_clears;
	uint32 score;

	byte *next_queue;
	byte queue_length;
	byte visible_queue_length;
	byte bag_size;
	byte bag_element_count;
} Board;

void BoardCreate(Board*);
void BoardFree(Board*);
void BoardClear(Board*);
void BoardSetIDSize(Board*, float id_size);
void BoardReallocNextQueue(Board*, byte visible_elements, byte bag_element_count);
void BoardRefillQueueSlots(Board *board);

//Returns the amount of lines cleared
int BoardSubmitBlock(Board*);

void BoardUseNextBlock(Board*);

void BoardRender(const Board*, bool draw_ghost);
void BoardRenderText(const Board*);

//Input functions
bool BoardInputDown(Board*);
bool BoardInputX(Board*, int x);
bool BoardInputCCW(Board*);
bool BoardInputCW(Board*);
