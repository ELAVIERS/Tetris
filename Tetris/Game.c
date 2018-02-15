#include "Game.h"
#include "Board.h"
#include "Error.h"
#include "Globals.h"
#include "Menu.h"
#include "Resource.h"
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

void GameInputDrop(), GameInputCCW(), GameInputCW();

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
	sv_autorepeat_delay =	&AddDFloat("sv_autorepeat_delay", 0.25f)->value.number;
	sv_autorepeat =			&AddDFloat("sv_autorepeat", 0.05f)->value.number;

	axis_x =	&AddDFloat("axis_x", 0)->value.number;
	axis_down = &AddDFloat("axis_down", 0)->value.number;
	AddDCall("drop",		GameInputDrop);
	AddDCall("rotate_cw",	GameInputCW);
	AddDCall("rotate_ccw",	GameInputCCW);

	AddDCall("dbg_next_texture_level", UseNextTextureLevel);
}

void GameFrame() {
	TimerStart();

	static float axis_x_prev;
	static float drop_timer = 0.f;
	static float das_timer = 0.f;
	static const int title_size = 64;
	static char title[64];

	snprintf(title, title_size, "Tetris (%d FPS)", (int)(1.f / g_delta));
	SetWindowTextA(g_hwnd, title);

	if (board_count) {
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
				das_timer = 0.f;
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

void GameBegin(int playercount) {
	g_menu_active = false;

	board_count = playercount;
	boards = (Board*)malloc(board_count * sizeof(Board));

	byte rows = (byte)GetDvar("sv_board_height")->value.number;
	byte columns = (byte)GetDvar("sv_board_width")->value.number;

	for (unsigned int i = 0; i < board_count; ++i) {
		boards[i].rows = rows;
		boards[i].columns = columns;

		SetupBoard(boards + i);
	}

	RECT size;
	GetClientRect(g_hwnd, &size);
	GameSizeUpdate((unsigned short)size.right, (unsigned short)size.bottom);
}

void GameEnd() {
	for (unsigned int i = 0; i < board_count; ++i)
		BoardFree(boards + i);

	free(boards);
	board_count = 0;

	CreateMainMenu();
	g_menu_active = true;
}

void GameSizeUpdate(unsigned short w, unsigned short h) {
	for (unsigned int i = 0; i < board_count; ++i) {
		boards[i].height = h;
		boards[i].width = h / boards[i].rows * boards[i].columns;
		boards[i].x = ((i + 1) * w / (board_count + 1)) - (boards[i].width / 2);
		boards[i].y = 0;
	}
}

void GameInputDrop() {
	while (BoardInputDown(boards + 0));
}

void GameInputCW() {
	BoardInputCW(boards + 0);
}

void GameInputCCW() {
	BoardInputCCW(boards + 0);
}
