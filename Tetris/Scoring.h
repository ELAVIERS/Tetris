#pragma once
#include "Board.h"
#include "Types.h"

typedef struct {

	byte clears;

	short drop_start_y;

	int drop_type; //1 = soft, 2 = hard
} ScoringStats;

void CancelCombo();

int GetScoreForLock(const ScoringStats *stats, const Board *board);

int GetGarbageForLock(const ScoringStats *stats);
