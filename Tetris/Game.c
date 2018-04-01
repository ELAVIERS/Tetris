#include "Game.h"
#include "BlockManager.h"
#include "Board.h"
#include "Client.h"
#include "Console.h"
#include "Error.h"
#include "Globals.h"
#include "LevelManager.h"
#include "Lobby.h"
#include "Menu.h"
#include "Messaging.h"
#include "Resource.h"
#include "Server.h"
#include "Shader.h"
#include "Timing.h"
#include "Variables.h"
#include <GL/GL.h>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

float blockid_size;

Quad bgquad;
Mat3 bg_projection;
float bg_offset[2];

Board *boards;
unsigned int board_count = 0;

GLuint shader, textshader;

float *cl_gap;

void GameInputDrop(), GameInputCCW(), GameInputCW(), C_CLBlockIDSize(DvarValue);

short scoring_drop_start_y;

void C_CLGap(DvarValue value) {
	GameSizeUpdate(0, 0);
}

void DBGNextLevel() {
	if (IsRemoteClient()) {
		ConsolePrint("usuk\n");
		return;
	}

	if (board_count) {
		byte message[4] = { SVMSG_LEVEL, 0 };
		Int16ToBuffer(boards[0].level + 1, message + 2);
		ServerBroadcast(message, 4, true);

		ExecLevelBind(boards[0].level, 0);
	};
}

void GameInit() {
	HINSTANCE instance = GetModuleHandle(NULL);

	char *frag_src = LoadStringResource(instance, ID_SHADER_FRAG);
	char *vert_src = LoadStringResource(instance, ID_SHADER_VERT);
	char *text_frag_src = LoadStringResource(instance, ID_SHADER_TEXTFRAG);

	if (frag_src == NULL || vert_src == NULL || text_frag_src == NULL) {
		ErrorMessage("Would you care to explain as to why the shaders don't exist mate?");
		return;
	}

	QuadCreate(&bgquad);
	Mat3Identity(bg_projection);
	Mat3Scale(bg_projection, 2, 2);
	Mat3Translate(bg_projection, -1, -1);

	shader = CreateShaderProgram(frag_src, vert_src);
	textshader = CreateShaderProgram(text_frag_src, vert_src);

	AddDCall("drop", GameInputDrop, false);
	AddDCall("rotate_cw", GameInputCW, false);
	AddDCall("rotate_ccw", GameInputCCW, false);

	AddDCall("dbg_next_level", DBGNextLevel, false);

	cl_gap = &AddDFloatC("cl_gap", 0.f, C_CLGap, false)->value.number;
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
	static byte datamsg[256] = { SVMSG_BLOCKDATA };
	datamsg[1] = boards[0].block.size;

	int sizesq = SQUARE(boards[0].block.size);
	memcpy_s(datamsg + 2, sizesq, boards[0].block.data, sizesq);
	MessageServer(datamsg, 2 + sizesq);
}

void SendQueueMessage() {
	byte message[256] = { SVMSG_QUEUE };
	message[1] = (byte)boards[0].visible_queue_length;

	memcpy_s(message + 2, 255 - 2, boards[0].next_queue, boards[0].visible_queue_length);
	MessageServer(message, 2 + boards[0].visible_queue_length);
}

inline int GameBoardGetScore(int id, int clears) {
	int score = 0;

	switch (clears) {
	case 1:
		score = 40;
		break;
	case 2:
		score = 100;
		break;
	case 3:
		score = 300;
		break;
	case 4:
		score = 1200;
		break;
	}

	score *= boards[id].level + 1;

	if (scoring_drop_start_y)
		return score + (scoring_drop_start_y - boards[0].block.y);

	return score;
}

inline void GameBoardSubmitBlock(int id) {
	int clears = BoardSubmitBlock(boards + id);
	int score = GameBoardGetScore(id, clears);

	byte message[6] = { SVMSG_SCORE };

	if (score && id == 0) {
		Int32ToBuffer(boards[id].score + score, message + 1);
		MessageServer(message, 5);
	}

	if (!IsRemoteClient() && clears) {
		message[1] = (byte)id;

		boards[id].line_clears += clears;
		message[0] = SVMSG_LINESCORE;
		Int32ToBuffer(boards[id].line_clears, message + 2);
		ServerBroadcast(message, 6, false);

		boards[id].level_clears += clears;

		while (boards[id].level_clears >= (uint16)*sv_clears_per_level) {
			++boards[id].level;
			boards[id].level_clears -= (uint16)*sv_clears_per_level;
			message[0] = SVMSG_LEVEL; //Indicates we should send level message
		}

		if (message[0] == SVMSG_LEVEL) {
			Int16ToBuffer(boards[id].level, message + 2);
			ServerBroadcast(message, 4, false);

			ExecLevelBind(boards[id].level, id);
		}
	}
}

inline void GamePlaceBlock() {
	CurrentBlockIncrementCount();
	GameBoardSubmitBlock(0);
	BoardUseNextBlock(boards + 0);

	byte message = SVMSG_PLACE;
	MessageServer(&message, 1);
	SendBlockDataMessage(0);
	SendBlockPosMessage(0);

	SendQueueMessage();

	if (*axis_down)
		scoring_drop_start_y = boards[0].block.y;
	else
		scoring_drop_start_y = 0;
}

void MoveDown() {
	if (BoardInputDown(boards + 0))
		SendBlockPosMessage(0);
	else
		GamePlaceBlock();
}

void Moveside(int dir) {
	if (BoardInputX(boards + 0, (int)*axis_x))
		SendBlockPosMessage(0);
}

void GameFrame() {
	TimerStart();

	ServerFrame();
	ClientFrame();

	static float axis_x_prev = 0.f, axis_down_prev = 0.f;
	static float drop_timer = 0.f;
	static float das_timer = 0.f;

	if (board_count && !g_paused) {
		drop_timer += g_delta;

		if (*axis_down != axis_down_prev) {
			axis_down_prev = *axis_down;

			if (*axis_down)
				scoring_drop_start_y = boards[0].block.y;
			else
				scoring_drop_start_y = 0;
		}

		if (*axis_down) {
			if (drop_timer >= (*sv_drop_gravity_is_factor ? *sv_gravity : 1.f) * *sv_drop_gravity) {
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

	Mat3 stats_transform;
	Mat3Identity(stats_transform);
	Mat3Scale(stats_transform, 256, 512);

	if (board_count) {
		UseGLProgram(shader);
		ShaderSetUniformMat3(shader, "u_projection", bg_projection);
		ShaderSetUniformMat3(shader, "u_transform", g_mat3_identity);

		if (g_textures[TEX_BG].glid) {
			ShaderSetUniformFloat2(shader, "u_uvoffset", bg_offset[0], bg_offset[1]);
			glBindTexture(GL_TEXTURE_2D, g_textures[TEX_BG].glid);
			QuadRender(&bgquad);
		}

		ShaderSetUniformMat3(shader, "u_projection", g_projection);

		if (board_count == 1)
			RenderBlockPanel(stats_transform, 16, boards[0].level);

		for (unsigned int i = 0; i < board_count; ++i)
			BoardRender(boards + i, *sv_ghost);
	}

	UseGLProgram(textshader);
	ShaderSetUniformMat3(textshader, "u_projection", g_projection);

	if (board_count) {
		if (board_count == 1)
			RenderBlockCounts(stats_transform, 16);

		for (unsigned int i = 0; i < board_count; ++i)
			BoardRenderText(boards + i);
	}
	
	Menus_Render();

	SwapBuffers(g_devcontext);
}

/////////

void GameSetBlockIDSize(float id_size) {
	blockid_size = id_size;

	for (unsigned int i = 0; i < board_count; ++i)
		BoardSetIDSize(boards + i, blockid_size);
}

void GameSetQueueLength(byte visible_length) {
	if (board_count)
		BoardReallocNextQueue(boards + 0, visible_length, BlockTypesGetCount());
}

void GameSetQueueElementCount(byte element_count) {
	if (board_count)
		BoardReallocNextQueue(boards + 0, boards[0].visible_queue_length, element_count);
}

void GameSetBagSize(byte bag_size) {
	if (board_count)
		boards[0].bag_size = bag_size;
}


void GameBegin(int playercount) {
	CloseAllMenus();
	g_paused = false;

	if (board_count)
		GameEnd();

	board_count = playercount;
	boards = (Board*)malloc(board_count * sizeof(Board));

	byte rows = (byte)*sv_board_real_height;
	byte columns = (byte)*sv_board_width;
	byte render_rows = (byte)*sv_board_height;

	for (unsigned int i = 0; i < board_count; ++i) {
		boards[i].rows = rows;
		boards[i].columns = columns;
		boards[i].visible_rows = render_rows;

		BoardCreate(boards + i);
		BoardClear(boards + i);
		BoardSetIDSize(boards + i, blockid_size);
	}
	boards[0].bag_size = (byte)*sv_bag_size;
	BoardReallocNextQueue(boards + 0, (byte)*sv_queue_size, (byte)BlockTypesGetCount());
	BoardUseNextBlock(boards + 0);

	SendBlockDataMessage();
	SendBlockPosMessage();

	GameSizeUpdate(0, 0);

	if (IsRemoteClient()) {
		byte message = SVMSG_REQUEST;
		MessageServer(&message, 1);

		SendQueueMessage();
	}
	else
		GameBoardSetName(0, LobbyGetClientName(0)); //We do this because otherwise listen servers wouldn't have their name on their board as the name message is sent prior to this point
}

void GameRestart() {
	ClearBlockCounts();
	GameBoardClear(-1);
	BoardUseNextBlock(boards + 0);

	byte clear_message = SVMSG_CLEAR;
	MessageServer(&clear_message, 1);

	SendBlockPosMessage();
	SendBlockDataMessage();
	SendQueueMessage();
}

void GameEnd() {
	for (unsigned int i = 0; i < board_count; ++i)
		BoardFree(boards + i);

	free(boards);
	board_count = 0;
}

inline void ConstrainBoardNameTag(Board *board, float field_width) {
	size_t len = strlen(board->nametag->string);
	if (len * board->nametag->size > field_width)
		board->nametag->size = field_width / (float)len;
}

void GameSizeUpdate(unsigned short w, unsigned short h) {
	if (board_count == 0)
		return;

	if (w == 0) {
		RECT rect;
		GetClientRect(g_hwnd, &rect);
		w = (short)rect.right;
		h = (short)rect.bottom;
	}

	unsigned short rows = boards[0].rows + (g_drawborder ? 2 : 0) + 6;
	unsigned short columns = boards[0].columns + (g_drawborder ? 2 : 0) + 6;

	float cell_width = (h * (float)columns / (float)rows);
	float cell_height;

	float board_w, inner_board_w, board_h;

	float x = 0, y;
	float spacing = *cl_gap;
	float gap;

	if (cell_width * board_count + spacing * (board_count - 1) > w) {
		cell_width = (float)(w - (spacing * (board_count - 1))) / (float)board_count;
		cell_height = (cell_width * ((float)rows / (float)columns));
		y = ((float)h - cell_height) / 2.f;
		gap = cell_width + spacing;
	}
	else {
		cell_height = h;
		y = 0;

		if (board_count == 1) {
			x = (float)w / 2.f - cell_width / 2.f;
			gap = 0;
		}
		else
			gap = cell_width + (float)(w - cell_width * board_count) / (float)(board_count - 1);
	}

	board_w = (float)cell_width / (float)columns * (columns - 6);
	inner_board_w = (float)cell_width / (float)columns * (columns - 8);
	board_h = (float)cell_height / (float)rows * (rows - 6);
	y += (float)cell_height / (float)rows * 3;

	for (unsigned int i = 0; i < board_count; ++i) {
		boards[i].width = board_w;
		boards[i].height = board_h;
		boards[i].x = (short)(x + gap * i);
		boards[i].y = y;

		boards[i].nametag->size = (float)cell_height / (float)rows;
		ConstrainBoardNameTag(boards + i, inner_board_w);
		GenerateTextData(boards[i].nametag, Font_UVSize(&g_font));
	}

	float u = (float)w / (float)g_textures[TEX_BG].width * 8.f / ((float)cell_width / (float)columns);

	QuadSetData(&bgquad,
		u,
		(float)h / (float)g_textures[TEX_BG].height * 8.f / ((float)cell_height / (float)rows));

	bg_offset[0] = -1.f * ((float)x / (float)w) * u;
	bg_offset[1] = 0.f;
}

void GameInputDrop() {
	if (board_count && !g_paused && *sv_hard_drop) {
		scoring_drop_start_y = boards[0].block.y;

		signed short lastx, lasty;

		do {
			lastx = boards[0].block.x;
			lasty = boards[0].block.y;
		} while (BoardInputDown(boards + 0));

		SendPosMessage(lastx, lasty);
		GamePlaceBlock();
	}
}

void GameInputCW() {
	if (board_count && !g_paused && BoardInputCW(boards + 0))
		SendBlockDataMessage();
}

void GameInputCCW() {
	if (board_count && !g_paused && BoardInputCCW(boards + 0))
		SendBlockDataMessage();
}


//Net

void GameBoardSetName(int id, const char *name) {
	if (board_count == 0) return;

	free(boards[id].nametag->string);
	boards[id].nametag->string = DupString(name);
	GameSizeUpdate(0, 0);
}

void GameBoardSetLevel(int id, uint16 level) {
	if (board_count == 0) return;

	boards[id].level = level;
}

void GameBoardSetScore(int id, uint32 score) {
	if (board_count == 0) return;

	boards[id].score = score;
}

void GameBoardSetLineClears(int id, uint16 line_clears) {
	if (board_count == 0) return;

	boards[id].line_clears = line_clears;
}

void GameBoardSetBlockPos(int id, signed short x, signed short y) {
	if (id == 0 || board_count == 0) return;

	boards[id].block.x = x;
	boards[id].block.y = y;
}

void GameBoardSetBlockData(int id, int size, const byte *data) {
	if (id == 0 || board_count == 0) return;

	int sizesq = size * size;

	if (boards[id].block.size != size) {
		boards[id].block.data = (byte*)realloc(boards[id].block.data, sizesq);
		boards[id].block.size = size;
	}

	memcpy_s(boards[id].block.data, sizesq, data, sizesq);
}

void GameBoardSetQueue(int id, byte length, const byte *queue) {
	if (id == 0 || board_count == 0) return;

	boards[id].visible_queue_length = length;

	if (boards[id].queue_length != length) {
		boards[id].next_queue = (byte*)realloc(boards[id].next_queue, length);
		boards[id].queue_length = length;
	}

	memcpy_s(boards[id].next_queue, length, queue, length);
}

void GameBoardPlaceBlock(int id) {
	if (id == 0 || board_count == 0) return;

	GameBoardSubmitBlock(id);
}

void GameBoardClear(int id) {
	if (id == 0 || board_count == 0) return;
	if (id < 0) id = 0;

	BoardClear(boards + id);

	boards[id].block.size = 0;
	boards[id].block.data = NULL;

	boards[id].score = 0;
	boards[id].line_clears = 0;
	boards[id].level_clears = 0;
	boards[id].level = 0;

	if (id == 0) {
		boards[0].next_queue[0] = 0xFF;
		BoardRefillQueueSlots(boards + 0);
	}
	else
		boards[id].visible_queue_length = 0;

	if (!IsRemoteClient())
		ExecLevelBind(0, id);
}

void GameSendAllBoardData(int playerid) {
	uint16 board_data_length = boards[0].rows * boards[0].columns;
	uint16 message_length = board_data_length + 2;
	if (message_length < 6)
		message_length = 6;

	byte *message = (byte*)malloc(message_length);

	for (unsigned int i = 0; i < board_count; ++i) {
		message[1] = (byte)i;

		message[0] = SVMSG_BOARD;
		memcpy_s(message + 2, board_data_length, &boards[i].data[0][0], board_data_length);
		ServerSend(playerid, message, board_data_length + 2);

		message[0] = SVMSG_BLOCKPOS;
		Int16ToBuffer(boards[i].block.x, message + 2);
		Int16ToBuffer(boards[i].block.y, message + 4);
		ServerSend(playerid, message, 6);

		message[0] = SVMSG_BLOCKDATA;
		message[2] = boards[i].block.size;
		memcpy_s(message + 3, message_length - 3, boards[i].block.data, SQUARE(boards[i].block.size));
		ServerSend(playerid, message, SQUARE(boards[i].block.size) + 3);

		message[0] = SVMSG_QUEUE;
		message[2] = (byte)boards[i].visible_queue_length;
		memcpy_s(message + 3, message_length - 3, boards[i].next_queue, boards[i].visible_queue_length);
		ServerSend(playerid, message, boards[i].visible_queue_length + 3);
	}
}

void GameReceiveBoardData(int id, const byte *data, uint16 length) {
	uint16 board_data_lenth = boards[0].rows * boards[0].columns;
	
	memcpy_s(&boards[id].data[0][0], board_data_lenth, data, length);
}
