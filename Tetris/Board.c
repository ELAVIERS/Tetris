#include "Board.h"

#include <stdlib.h>

typedef unsigned char byte;

byte **board = NULL;
int board_width;
int board_height;

void SetBoard(int width, int height) {
	for (int i = 0; i < board_width; ++i)
		free(board[i]);

	free(board);

	board_width = width;
	board_height = height;

	board = (byte**)malloc(board_width * sizeof(byte*));

	for (int i = 0; i < board_width; ++i)
		board[i] = (byte*)malloc(board_height);
}

void CLSetTextureIndexOrder(const char **tokens, unsigned int count) {
	//Todo
}

void CLAddTextureLevel(const char **tokens, unsigned int count) {
	//Todo
}

void CLClearTextureLevels(const char **tokens, unsigned int count) {
	//Todo
}
