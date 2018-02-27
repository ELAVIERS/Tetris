#include "Game.h"
#include "Board.h"
#include "Client.h"
#include "Error.h"
#include "Globals.h"
#include "Menu.h"
#include "Resource.h"
#include "Server.h"
#include "Shader.h"
#include "Timing.h"
#include <GL/GL.h>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

Board *boards;
unsigned int board_count = 0;

GLuint shader, textshader;

float *sv_gravity, *sv_drop_gravity, *sv_autorepeat_delay, *sv_autorepeat;

float *axis_x, *axis_down;

void GameInputDrop(), GameInputCCW(), GameInputCW(), C_CLBlockIDSize(DvarValue);

void GameInit() {
	HINSTANCE instance = GetModuleHandle(NULL);

	char *frag_src = LoadStringResource(instance, ID_SHADER_FRAG);
	char *vert_src = LoadStringResource(instance, ID_SHADER_VERT);
	char *text_frag_src = LoadStringResource(instance, ID_SHADER_TEXTFRAG);

	if (frag_src == NULL || vert_src == NULL || text_frag_src == NULL) {
		ErrorMessage("Would you care to explain as to why the shaders don't exist mate?");
		return;
	}

	shader = CreateShaderProgram(frag_src, vert_src);
	textshader = CreateShaderProgram(text_frag_src, vert_src);

	sv_gravity =			&AddDFloat("sv_gravity", 0.5f)->value.number;
	sv_drop_gravity =		&AddDFloat("sv_drop_gravity", 0.05f)->value.number;
	sv_autorepeat =			&AddDFloat("sv_autorepeat", 0.05f)->value.number;
	sv_autorepeat_delay =	&AddDFloat("sv_autorepeat_delay", 0.25f)->value.number;

	axis_x =	&AddDFloat("axis_x", 0)->value.number;
	axis_down = &AddDFloat("axis_down", 0)->value.number;
	AddDCall("drop",		GameInputDrop);
	AddDCall("rotate_cw",	GameInputCW);
	AddDCall("rotate_ccw",	GameInputCCW);

	AddDCall("dbg_next_texture_level", UseNextTextureLevel);

	AddDFloatC("cl_blockid_size", 0, C_CLBlockIDSize);
}

void GameFrame() {
	TimerStart();

	ServerFrame();
	ClientFrame();

	static float axis_x_prev;
	static float drop_timer = 0.f;
	static float das_timer = 0.f;

	if (board_count && !g_paused) {
		drop_timer += g_delta;

		if (*axis_down) {
			if (drop_timer >= *sv_drop_gravity) {
				drop_timer = 0.f;
				BoardInputDown(boards + 0);
			}
		}
		else if (drop_timer >= *sv_gravity) {
			drop_timer = 0.f;
			BoardInputDown(boards + 0);
		}

		if (*axis_x != axis_x_prev) {
			axis_x_prev = *axis_x;
			BoardInputX(boards + 0, (int)*axis_x);

			if (axis_x)
				das_timer = -*sv_autorepeat_delay;
		}

		if (axis_x_prev) {
			das_timer += g_delta;
			if (das_timer >= *sv_autorepeat) {
				das_timer -= *sv_autorepeat;
				BoardInputX(boards + 0, (int)axis_x_prev);
			}
		}
	}

	//

	GameRender();
	g_delta = TimerDelta();
}

void GameRender() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (board_count) {
		UseGLProgram(shader);
		ShaderSetUniformMat3(shader, "u_projection", g_projection);

		for (unsigned int i = 0; i < board_count; ++i)
			BoardRender(boards + i);
	}

	UseGLProgram(textshader);
	ShaderSetUniformMat3(textshader, "u_projection", g_projection);
	Menus_Render();

	SwapBuffers(g_devcontext);
}

/////////

inline void GameSetBoardIDSize(float id_size) {
	for (unsigned int i = 0; i < board_count; ++i)
		BoardSetIDSize(boards + i, id_size);
}

void C_CLBlockIDSize(DvarValue number) {
	GameSetBoardIDSize(number.number);
}

void GameBegin(int playercount) {
	if (g_ingame)
		GameEnd();

	board_count = playercount;
	boards = (Board*)malloc(board_count * sizeof(Board));

	unsigned short rows = (unsigned short)GetDvar("sv_board_height")->value.number;
	unsigned short columns = (unsigned short)GetDvar("sv_board_width")->value.number;
	float id_size = GetDvar("cl_blockid_size")->value.number;

	for (unsigned int i = 0; i < board_count; ++i) {
		boards[i].rows = rows;
		boards[i].columns = columns;

		BoardCreate(boards + i);
		BoardNewGame(boards + i);
		BoardSetIDSize(boards + i, id_size);
	}

	RECT size;
	GetClientRect(g_hwnd, &size);
	GameSizeUpdate((unsigned short)size.right, (unsigned short)size.bottom);

	g_ingame = true;
}

void GameRestart() {
	//for (unsigned int i = 0; i < board_count; ++i)
	//	BoardNewGame(boards + i);

	unsigned int prev_board_count = board_count;
	GameEnd();
	GameBegin(prev_board_count);
}

void GameEnd() {
	for (unsigned int i = 0; i < board_count; ++i)
		BoardFree(boards + i);

	free(boards);
	board_count = 0;

	g_ingame = false;
}

inline unsigned short BoardWidth(const Board *board, unsigned short h) { return (unsigned short)((float)h / (float)board->rows) * board->columns; }

void GameSizeUpdate(unsigned short w, unsigned short h) {
	if (board_count == 0)
		return;

	if (board_count == 1) {
		boards[0].height = h;
		boards[0].width = BoardWidth(boards, h);
		boards[0].x = (w / 2) - (boards[0].width / 2);
		boards[0].y = 0;
	}
	else {
		int gap, start_x;

		for (unsigned int i = 0; i < board_count; ++i) {
			boards[i].height = h;
			boards[i].width = BoardWidth(boards + i, h);
			boards[i].y = 0;

			if (i == 0) {
				boards[i].x = 0;
				gap = (w - (boards[0].width / 2) - (BoardWidth(boards + board_count - 1, h)) / 2) / (board_count - 1);
				start_x = boards[0].width / 2;
			}
			else if (i == board_count - 1)
				boards[i].x = w - boards[i].width;
			else
				boards[i].x = start_x + (gap * i) - (boards[i].width / 2);
		}
	}
}

void GameInputDrop() {
	if (board_count)
		while (BoardInputDown(boards + 0));
}

void GameInputCW() {
	if (board_count)
		BoardInputCW(boards + 0);
}

void GameInputCCW() {
	if (board_count)
		BoardInputCCW(boards + 0);
}
