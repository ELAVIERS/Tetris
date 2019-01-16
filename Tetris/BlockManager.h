#pragma once
#include "Block.h"
#include "Dvar.h"
#include "Matrix.h"
#include "Quad.h"


typedef struct {
	unsigned int size;
	byte *data;

	byte start_row;
	byte start_column;
	char id;
} BlockType;

typedef struct BlockCount_s {
	BlockType* type;

	unsigned int count;

	struct BlockCount_s *next;
} BlockCount;

/*
CreateNewBlock
Sets contents of block to the given index's blockdata

index				the index of the block data in the array
block(out)			the block to become random
top					which row to place the top row of the block at
*/
void CreateNewBlock(int index, Block *block, unsigned short top);
void RenderBlockByIndex(int index, Mat3 transform, const Quad *quad, unsigned int level);

int GetIndexOfBlockID(byte block_id);

unsigned int BlockTypesGetCount();

//sv_add_block
DFunc SVAddBlock;

//sv_clear_blocks
void ClearBlocks();

//Sends commands to a player defining block info
void SendBlockInfo(int playerid);

void RenderBlockPanel(Mat3 transform, float block_w, float block_h, unsigned int level);

BlockCount *CreateBlockCountList();
void FreeBlockCountList(BlockCount *list);

void ClearBlockCounts(BlockCount *first);
void IncrementBlockCount(BlockCount *first, char id);

void RenderBlockCounts(BlockCount *first, Mat3 transform, float block_w, float block_h);
