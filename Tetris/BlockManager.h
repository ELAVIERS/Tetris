#pragma once
#include "Block.h"
#include "Dvar.h"
#include "Matrix.h"
#include "Quad.h"

/*
CreateNewBlock
Sets contents of block to the given indexe's blockdata

index				the index of the block data in the array
block(out)			the block to become random
top					which row to place the top row of the block at
*/
void CreateNewBlock(int index, Block *block, unsigned short top);
void RenderBlockByIndex(int index, Mat3 transform, const Quad *quad, unsigned int level);

int GetIndexOfBlockID(byte block_id);

void CurrentBlockIncrementCount();

unsigned int BlockTypesGetCount();

//sv_add_block
DFunc SVAddBlock;

//sv_clear_blocks
void ClearBlocks();

//Sends commands to a player defining block info
void SendBlockInfo(int playerid);

void ClearBlockCounts();
void RenderBlockPanel(Mat3 transform, float block_size, unsigned int level);
void RenderBlockCounts(Mat3 transform, float block_size);
