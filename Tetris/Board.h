#pragma once
#include "Block.h"
#include "BlockManager.h"
#include "Quad.h"
#include "Text.h"
#include "Texture.h"
#include "Types.h"
#include <stdbool.h>

/*
	Board.h

	It's all about rendering a bunch of tetrominos
*/

#define GARBAGE_QUEUE_SIZE 8

typedef struct Garbage_s {
	byte rows;
	byte clear_column;
} Garbage;

/*
	Board(struct)
	x, y			the board's position in pixels
	width, height	the board's size in pixels

	data			a 2d matrix representing the id of each grid cell
	rows, columns	the size of data

	block			the current block
*/
typedef struct Board_s {
	bool active;

	float x, y;
	float width, height;

	byte **data;
	byte rows, columns, visible_rows;
	byte stats_rows, stats_columns;

	Block block;
	short ghost_y;

	Text *nametag;

	int16 level;
	uint16 level_clears;
	uint32 line_clears;
	uint32 score;

	Garbage garbage_queue[GARBAGE_QUEUE_SIZE];
	byte *next_queue;
	byte queue_length;
	byte visible_queue_length;
	byte bag_size;
	byte bag_element_count;

	byte held_index;

	BlockCount *first_count_node;
} Board;

void BoardCreate(Board*);
void BoardFree(Board*);
void BoardClear(Board*);
bool BoardCheckMove(const Board*, short x, short y);
void BoardSetIDSize(Board*, float id_size);
void BoardReallocNextQueue(Board*, byte visible_elements, byte bag_element_count);
void BoardRefillQueueSlots(Board *board);
byte BoardSubmitGarbageQueue(Board *board);
void BoardAddGarbage(Board *board, byte rows, byte clear_column);
void BoardUpdateGhostY(Board *board);

//Returns the amount of lines cleared
int BoardSubmitBlock(Board*);

void BoardUseNextBlock(Board*);

void BoardRender(const Board*, bool draw_ghost, bool show_stats_panel);
void BoardRenderText(const Board*, bool show_stats_panel);

//Input functions
bool BoardInputDown(Board*);
bool BoardInputX(Board*, int x);
bool BoardInputCCW(Board*);
bool BoardInputCW(Board*);
