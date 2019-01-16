#include "Scoring.h"
#include "Board.h"
#include "Globals.h"
#include "SoundManager.h"
#include "Variables.h"

bool last_score_was_difficult = false;

int combo_count;

void CancelCombo() {
	combo_count = 0;
}

int GetScoreForLock(const ScoringStats *stats, const Board *board) {
	int score = 0;

	int lvl = board->level + 1;

	bool is_t;


	bool difficult = false;

	switch (stats->clears) {
	case 1:
		score = (*sv_scoring_nes ? 40 : 100) * lvl;
		break;
	case 2:
		score = (*sv_scoring_nes ? 100 : 300) * lvl;
		break;
	case 3:
		score = (*sv_scoring_nes ? 300 : 500) * lvl;
		break;
	case 4:
		difficult = true;
		score = (*sv_scoring_nes ? 1200 : 800) * lvl;
		break;
	}

	if (*sv_scoring_nes)
		score += (stats->drop_start_y - board->block.y);
	else {
		//T-Spins


		//Combos
		score += 50 * combo_count * lvl;
		combo_count++;

		//Perfect Clear
		bool pc = true;
		for (byte c = 0; c < board->columns; ++c)
			if (board->data[0][c]) {
				pc = false;
				break;
			}

		if (pc) {
			SMPlaySound(g_audio.perfectclear, false);
			difficult = true;
			score += 2000 * lvl;
		}


		//BTB
		if (difficult && last_score_was_difficult) {
			SMPlaySound(g_audio.backtoback, false);
			score += 1200 * lvl;
		}
	}

	last_score_was_difficult = difficult;
	return score;
}

int GetGarbageForLock(const ScoringStats *stats) {
	switch (stats->clears) {
	case 2:
		return 1;
	case 3:
		return 2;
	case 4:
		return 4;
	}

	return 0;
}
