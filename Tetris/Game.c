#include "Game.h"
#include "Board.h"
#include "Client.h"
#include "Error.h"
#include "Globals.h"
#include "Menu.h"
#include "Messaging.h"
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

	sv_gravity = &AddDFloat("sv_gravity", 0.5f, true)->value.number;
	sv_drop_gravity = &AddDFloat("sv_drop_gravity", 0.05f, true)->value.number;
	sv_autorepeat = &AddDFloat("sv_autorepeat", 0.05f, true)->value.number;
	sv_autorepeat_delay = &AddDFloat("sv_autorepeat_delay", 0.25f, true)->value.number;

	axis_x = &AddDFloat("axis_x", 0, false)->value.number;
	axis_down = &AddDFloat("axis_down", 0, false)->value.number;

	AddDCall("drop", GameInputDrop, false);
	AddDCall("rotate_cw", GameInputCW, false);
	AddDCall("rotate_ccw", GameInputCCW, false);

	AddDCall("dbg_next_texture_level", UseNextTextureLevel, false);

	AddDFloatC("cl_blockid_size", 0, C_CLBlockIDSize, false);
}

void SendPosMessage(int16 x, int16 y) {
	static byte posmsg[] = { SVMSG_BLOCKPOS, 0, 0, 0, 0 };
	Int16ToBuffer(x, posmsg + 1);
	Int16ToBuffer(y, posmsg + 3);
	MessageServer(posmsg, sizeof(posmsg));
}

inline void SendBlockPosMessage() {
	SendPosMessage(boards[0].block.x, boards[0].block.y);
}

void SendBlockDataMessage() {
	static byte datamsg[MSG_LEN] = { SVMSG_BLOCKDATA };
	datamsg[1] = boards[0].block.size;

	int sizesq = SQUARE(boards[0].block.size);
	memcpy_s(datamsg + 2, sizesq, boards[0].block.data, sizesq);
	MessageServer(datamsg, 2 + sizesq);
}

inline void SendPlaceMessage() {
	byte message = SVMSG_PLACE;
	MessageServer(&message, 1);
	SendBlockDataMessage(0);
	SendBlockPosMessage(0);
}

void MoveDown() {
	if (BoardInputDown(boards + 0))
		SendBlockPosMessage(0);
	else
		SendPlaceMessage();
}

void Moveside(int dir) {
	if (BoardInputX(boards + 0, (int)*axis_x))
		SendBlockPosMessage(0);
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
				MoveDown();
			}
		}
		else if (drop_timer >= *sv_gravity) {
			drop_timer = 0.f;
			MoveDown();
		}

		if (*axis_x != axis_x_prev) {
			axis_x_prev = *axis_x;
			Moveside((int)*axis_x);

			if (axis_x)
				das_timer = -*sv_autorepeat_delay;
		}

		if (axis_x_prev) {
			das_timer += g_delta;
			if (das_timer >= *sv_autorepeat) {
				das_timer -= *sv_autorepeat;
				Moveside((int)axis_x_prev);
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
	CloseAllMenus();
	g_paused = false;

	if (board_count)
		GameEnd();

	board_count = playercount;
	boards = (Board*)malloc(board_count * sizeof(Board));

	byte rows = (byte)GetDvar("sv_board_height")->value.number;
	byte columns = (byte)GetDvar("sv_board_width")->value.number;
	float id_size = GetDvar("cl_blockid_size")->value.number;

	for (unsigned int i = 0; i < board_count; ++i) {
		boards[i].rows = rows;
		boards[i].columns = columns;

		BoardCreate(boards + i);
		BoardClear(boards + i);
		BoardSetIDSize(boards + i, id_size);

		boards[i].block.size = 0;
		boards[i].block.data = NULL;
	}

	BoardUseNextBlock(boards + 0);

	SendBlockDataMessage();
	SendBlockPosMessage();

	RECT size;
	GetClientRect(g_hwnd, &size);
	GameSizeUpdate((unsigned short)size.right, (unsigned short)size.bottom);

	if (IsRemoteClient()) {
		byte message = SVMSG_REQUEST;
		MessageServer(&message, 1);
	}
}

void GameRestart() {
	BoardClear(boards + 0);
	BoardUseNextBlock(boards + 0);

	byte clear_message = SVMSG_CLEAR;
	MessageServer(&clear_message, 1);

	SendBlockPosMessage(0);
	SendBlockDataMessage(0);
}

void GameEnd() {
	for (unsigned int i = 0; i < board_count; ++i)
		BoardFree(boards + i);

	free(boards);
	board_count = 0;
}

#define GAP 16

void GameSizeUpdate(unsigned short w, unsigned short h) {
	if (board_count == 0)
		return;

	unsigned short board_width = ((float)h / (float)boards[0].rows) * boards[0].columns;;
	unsigned short board_height;
	unsigned short x = 0, y;
	int gap;

	if (board_width * board_count + GAP * (board_count - 1) > w) {
		board_width = (w - (GAP * (board_count - 1))) / board_count;
		board_height = board_width * (boards[0].rows / boards[0].columns);
		y = (h - board_height) / 2;
		gap = board_width + GAP;
	}
	else {
		board_height = h;
		y = 0;

		if (board_count == 1) {
			gap = 0;
			x = w / 2 - board_width / 2;
		}
		else {
			gap = (w - (boards[0].width / 2) - board_width / 2) / (board_count - 1);
		}
	}

	for (unsigned int i = 0; i < board_count; ++i) {
		boards[i].width = board_width;
		boards[i].height = board_height;
		boards[i].x = x + gap * i;
		boards[i].y = y;
	}
}

void GameInputDrop() {
	signed short lastx, lasty;

	if (board_count) {
		do {
			lastx = boards[0].block.x;
			lasty = boards[0].block.y;
		} while (BoardInputDown(boards + 0));

		SendPosMessage(lastx, lasty);
		SendPlaceMessage();
	}
}

void GameInputCW() {
	if (board_count && BoardInputCW(boards + 0))
		SendBlockDataMessage();
}

void GameInputCCW() {
	if (board_count && BoardInputCCW(boards + 0))
		SendBlockDataMessage();
}

void GameBoardSetBlockPos(int id, signed short x, signed short y) {
	if (id == 0 || board_count == 0) return;

	boards[id].block.x = x;
	boards[id].block.y = y;
}

void GameBoardSetBlockData(int id, int size, const byte *data) {
	if (id == 0 || board_count == 0) return;

	free(boards[id].block.data);

	int sizesq = size * size;

	boards[id].block.size = size;
	boards[id].block.data = (bool*)malloc(sizesq);
	memcpy_s(boards[id].block.data, sizesq, data, sizesq);
}

void GameBoardPlaceBlock(int id) {
	if (id == 0 || board_count == 0) return;

	BoardSubmitBlock(boards + id);
}

void GameBoardClear(int id) {
	if (id == 0 || board_count == 0) return;
	BoardClear(boards + id);

	boards[id].block.size = 0;
	boards[id].block.data = NULL;
}

void GameSendAllBoardData(int playerid) {
	uint16 board_data_lenth = boards[0].rows * boards[0].columns;

	byte *message = (byte*)malloc(2 + board_data_lenth);

	for (unsigned int i = 0; i < board_count; ++i) {
		message[1] = (byte)i;

		message[0] = SVMSG_BOARD;
		memcpy_s(message + 2, board_data_lenth, &boards[i].data[0][0], board_data_lenth);
		ServerSend(playerid, message, board_data_lenth + 2);

		message[0] = SVMSG_BLOCKPOS;
		Int16ToBuffer(boards[i].block.x, message + 2);
		Int16ToBuffer(boards[i].block.y, message + 4);
		ServerSend(playerid, message, 6);

		message[0] = SVMSG_BLOCKDATA;
		message[2] = boards[i].block.size;
		memcpy_s(message + 3, MSG_LEN - 3, boards[i].block.data, SQUARE(boards[i].block.size));
		ServerSend(playerid, message, SQUARE(boards[i].block.size) + 3);
	}
}

void GameReceiveBoardData(int id, const byte *data, uint16 length) {
	uint16 board_data_lenth = boards[0].rows * boards[0].columns;
	
	memcpy_s(&boards[id].data[0][0], board_data_lenth, data, length);
}
