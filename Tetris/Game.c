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
#include "RNG.h"
#include "Resource.h"
#include "Scoring.h"
#include "Server.h"
#include "Shader.h"
#include "SoundManager.h"
#include "Timing.h"
#include "Variables.h"
#include <GL/GL.h>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

const float stat_cells_w = 6;
const float stat_cells_h = 22;

bool music_fast = false;

float blockid_size;

Quad bgquad;
Mat3 bg_projection;
float bg_offset[2];

Board *boards;
unsigned int board_count = 0;

GLuint shader, textshader;

float *axis_x, *axis_down;
DvarCallback C_AxisDown;

void GameInputDrop(), GameInputCCW(), GameInputCW(), GameInputHold(), C_CLBlockIDSize(DvarValue);

short scoring_drop_start_y;
float drop_timer, drop_timer_target;
float next_block_countdown = -1.f;

bool is_locking = false;
bool can_use_held = false;

void DBGNextLevel() {
	if (IsRemoteClient()) {
		ConsolePrint("usuk\n");
		return;
	}

	if (board_count) {
		byte message[4] = { SVMSG_LEVEL, 0 };
		Int16ToBuffer(boards[0].level + 1, message + 2);
		ServerBroadcast(message, 4, -1);

		ExecLevelBind(boards[0].level, 0);
	};
}

void DBGAddGarbo(const char **tokens, unsigned int count) {
	if (count < 2 || board_count == 0) return;

	BoardAddGarbage(boards + 0, atoi(tokens[0]), atoi(tokens[1]));
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

	axis_x = ValueAsFloatPtr(AddDFloat("axis_x", 0, false));
	axis_down = ValueAsFloatPtr(AddDFloatC("axis_down", 0, C_AxisDown, false));

	AddDCall("drop", GameInputDrop, false);
	AddDCall("rotate_cw", GameInputCW, false);
	AddDCall("rotate_ccw", GameInputCCW, false);
	AddDCall("hold", GameInputHold, false);

	AddDCall("dbg_next_level", DBGNextLevel, false);
	AddDFunction("dbg_add_garbage", DBGAddGarbo, false);
}

void SendPosMessage(int16 x, int16 y) {
	byte posmsg[] = { SVMSG_BLOCKPOS, 0, 0, 0, 0 };
	Int16ToBuffer(x, posmsg + 1);
	Int16ToBuffer(y, posmsg + 3);
	MessageServer(posmsg, sizeof(posmsg));
}

inline void SendBlockPosMessage() {
	SendPosMessage(boards[0].block.x, boards[0].block.y);
}

void SendBlockDataMessage() {
	byte datamsg[256] = { SVMSG_BLOCKDATA };
	datamsg[1] = boards[0].block.id;
	datamsg[2] = boards[0].block.size;

	int sizesq = SQUARE(boards[0].block.size);
	memcpy_s(datamsg + 3, sizesq, boards[0].block.data, sizesq);
	MessageServer(datamsg, 3 + sizesq);
}

void SendQueueMessage() {
	byte message[256] = { SVMSG_QUEUE };
	message[1] = (byte)boards[0].visible_queue_length;

	memcpy_s(message + 2, 255 - 2, boards[0].next_queue, boards[0].visible_queue_length);
	MessageServer(message, 2 + boards[0].visible_queue_length);
}

void GameBoardSubmitBlock(int id) {
	int top_row = BlockGetLargestY(&boards[id].block);

	int clears = BoardSubmitBlock(boards + id);
	BoardSubmitGarbageQueue(boards + id);

	byte message[6] = { SVMSG_SCORE };

	if (id == 0) {
		SMPlaySound(g_audio.lock, false);
		
		if (top_row >= boards[id].visible_rows - (byte)*cl_fast_music_zone)
		{
			if (!music_fast) {
				music_fast = true;
				SMStopSound(g_music_id);
			}
		}
		else
		{
			if (music_fast) {
				bool row_is_clear = true;

				for (byte c = 0; c < boards[id].columns; ++c) {
					if (boards[id].data[boards[id].visible_rows - (byte)*cl_fast_music_zone][c]) {
						row_is_clear = false;
						break;
					}
				}

				if (row_is_clear) {
					music_fast = false;
					SMStopSound(g_music_id);
				}
			}
		}

		ScoringStats stats;
		stats.clears = clears;
		stats.drop_start_y = scoring_drop_start_y;
		stats.drop_type = 0;

		Int32ToBuffer(GetScoreForLock(&stats, boards + 0), message + 1);
		MessageServer(message, 5);

		if (clears)
		{
			if (clears == 4)
				SMPlaySound(g_audio.clear4, false);
			else
				SMPlaySound(g_audio.clear, false);
		}
		else
			CancelCombo();
	}

	if (!IsRemoteClient()) {
		if (clears) {
			message[1] = (byte)id;

			message[0] = SVMSG_LINESCORE;
			Int32ToBuffer(boards[id].line_clears + clears, message + 2);
			ServerBroadcast(message, 6, -1);

			boards[id].level_clears += clears;

			while (boards[id].level_clears >= (uint16)*sv_clears_per_level) {
				++boards[id].level;
				boards[id].level_clears -= (uint16)*sv_clears_per_level;
				message[0] = SVMSG_LEVEL; //Indicates we should send level message
			}

			if (message[0] == SVMSG_LEVEL) {
				SMPlaySound(g_audio.levelup, false);

				Int16ToBuffer(boards[id].level, message + 2);
				ServerBroadcast(message, 4, -1);

				ExecLevelBind(boards[id].level, 0);
			}

			message[0] = SVMSG_GARBAGE;

			ScoringStats stats;
			stats.clears = clears;
			int garbageLines = GetGarbageForLock(&stats);
			
			if (garbageLines)
			{
				for (byte i = 0; i < board_count; ++i) {
					if (boards[i].active && i != id) {
						message[1] = i;
						message[2] = garbageLines;
						message[3] = RandomIntInRange(0, boards[0].columns);	//Clear

						ServerBroadcast(message, 4, -1);
					}
				}
			}
		}
	}
}

void CheckLocking() {
	if (!BoardCheckMove(boards + 0, 0, -1) && *sv_lock_delay >= 0.f) {
		drop_timer_target = *sv_lock_delay;
		is_locking = true;
	}
	else {
		if (is_locking) {
			drop_timer = 0;

			is_locking = false;
		}

		drop_timer_target = *axis_down ? (*sv_drop_gravity_is_factor ? *sv_gravity * *sv_drop_gravity : *sv_drop_gravity) : *sv_gravity;
	}
}

void GameUseNextBlock() {
	BoardUseNextBlock(boards + 0);

	SendBlockPosMessage();
	SendBlockDataMessage();

	if (BoardCheckMove(boards + 0, 0, 0) == false)
	{
		byte message = SVMSG_STOP;
		MessageServer(&message, 1);
		GameBoardFinished(0);
	}
	else {
		CheckLocking();
		can_use_held = true;
		drop_timer = 0;
	}
}

void GamePlaceBlock() {
	if (boards[0].block.size == 0) return;

	GameBoardSubmitBlock(0);

	boards[0].block.size = 0;
	free(boards[0].block.data);
	boards[0].block.data = NULL;
	next_block_countdown = *sv_drop_delay;

	byte message = SVMSG_PLACE;
	MessageServer(&message, 1);
	SendBlockPosMessage();
	SendBlockDataMessage();

	SendQueueMessage();

	if (*axis_down)
		scoring_drop_start_y = boards[0].block.y;
	else
		scoring_drop_start_y = 0;
}

void MoveDown() {
	if (BoardInputDown(boards + 0)) {
		SendBlockPosMessage();

		CheckLocking();
	}
	else
		GamePlaceBlock();
}

void MoveSide(int dir) {
	if (BoardInputX(boards + 0, (int)*axis_x)) {
		SMPlaySound(g_audio.move, false);

		SendBlockPosMessage();
	
		CheckLocking();
	}
}

void C_AxisDown(DvarValue value) {
	if (board_count == 0) return;

	bool move = !g_in_menu && !*sv_paused;

	if (value.number) {
		scoring_drop_start_y = boards[0].block.y;
		
		if (move) MoveDown();

		if (!is_locking) {
			drop_timer_target = *sv_drop_gravity_is_factor ? *sv_gravity * *sv_drop_gravity : *sv_drop_gravity;
			if (move) drop_timer = 0;
		}
	}
	else if (!is_locking) {
		if (move) scoring_drop_start_y = 0;

		drop_timer_target = *sv_gravity;
	}
}

void GameFrame() {
	TimerStart();

	ServerFrame();
	ClientFrame();

	if (g_in_game) {
		static float axis_x_prev = 0.f;
		static float das_timer = 0.f;

		if (board_count && !*sv_paused) {
			if (!SMSoundIsPlaying(g_music_id)) {
				const char *music = NULL;

				switch (RandomIntInRange(0, 3)) {
				case 0:
					music = music_fast ? g_audio.mus1f : g_audio.mus1;
					break;
				case 1:
					music = music_fast ? g_audio.mus2f : g_audio.mus2;
					break;
				case 2:
					music = music_fast ? g_audio.mus3f : g_audio.mus3;
					break;
				}

				g_music_id = SMPlaySound(music, true);
			}

			drop_timer += g_delta;

			if (drop_timer >= (g_in_menu ? *sv_gravity : drop_timer_target)) {
				drop_timer = 0.f;
				MoveDown();
			}

			if (!g_in_menu) {
				if (*axis_x != axis_x_prev) {
					axis_x_prev = *axis_x;
					MoveSide((int)*axis_x);

					if (axis_x)
						das_timer = -*sv_autorepeat_delay;
				}

				if (axis_x_prev) {
					das_timer += g_delta;
					if (das_timer >= *sv_autorepeat) {
						das_timer -= *sv_autorepeat;
						MoveSide((int)axis_x_prev);
					}
				}
			}

			if (next_block_countdown >= 0.f) {
				next_block_countdown -= g_delta;
				if (next_block_countdown <= 0.f)
					GameUseNextBlock();
			}
		}
	}

	//

	SMFeedBuffer(g_delta);
	
	GameRender();
	g_delta = TimerDelta();
}

void GameRender() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	bool show_stats_panel = false;
	if (*cl_stats_mode) {
		if (*cl_stats_mode == 1)
			show_stats_panel = board_count == 1;
		else
			show_stats_panel = true;
	}

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

		for (unsigned int i = 0; i < board_count; ++i)
			if (boards[i].visible)
				BoardRender(boards + i, *sv_ghost, show_stats_panel);
	}

	UseGLProgram(textshader);
	ShaderSetUniformMat3(textshader, "u_projection", g_projection);

	for (unsigned int i = 0; i < board_count; ++i)
		if (boards[i].visible)
			BoardRenderText(boards + i, show_stats_panel);

	ShaderSetUniformBool(textshader, "u_masked", !g_in_game);
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
	music_fast = false;
	g_in_game = true;

	CancelCombo();

	FreeMenus();
	if (!IsRemoteClient())
		SetDvarFloat(GetDvar("sv_paused"), 0.f, false);

	if (board_count) {
		if (playercount == board_count) {
			GameRestart();
			return;
		}

		GameEnd();
	}

	board_count = playercount;
	boards = (Board*)calloc(board_count, sizeof(Board));

	byte rows = (byte)*sv_board_real_height;
	byte columns = (byte)*sv_board_width;
	byte render_rows = (byte)*sv_board_height;

	for (unsigned int i = 0; i < board_count; ++i) {
		boards[i].rows = rows;
		boards[i].columns = columns;
		boards[i].visible_rows = render_rows;
		boards[i].stats_rows = render_rows + 2;
		boards[i].stats_columns = 6;
		boards[i].held_index = 0xFF;
		boards[i].active = boards[i].visible = false;

		BoardCreate(boards + i);
		BoardClear(boards + i);
		BoardSetIDSize(boards + i, blockid_size);
	}

	boards[0].active = boards[0].visible = true;
	boards[0].bag_size = (byte)*sv_bag_size;
	BoardReallocNextQueue(boards + 0, (byte)*sv_queue_size, (byte)BlockTypesGetCount());
	GameUseNextBlock();

	GameSizeUpdate(0, 0);

	if (IsRemoteClient()) {
		SendQueueMessage();

		byte message = SVMSG_REQUEST;
		MessageServer(&message, 1);
	}
	else
		GameBoardSetName(0, LobbyGetClientName(0)); //We do this because otherwise listen servers wouldn't have their name on their board as the name message is sent prior to this point
}

void GameRestart() {
	SMClearSounds();

	byte clear_message = SVMSG_CLEAR;
	MessageServer(&clear_message, 1);

	SendBlockPosMessage();
	SendBlockDataMessage();
	SendQueueMessage();
}

void GameEnd() {
	SMClearSounds();

	g_in_game = false;

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

	int visible_board_count = 0;

	for (unsigned int i = 0; i < board_count; ++i)
		if (boards[i].visible)
			++visible_board_count;

	if (visible_board_count == 0)
		return;

	if (w == 0) {
		RECT rect;
		GetClientRect(g_hwnd, &rect);
		w = (short)rect.right;
		h = (short)rect.bottom;
	}

	bool show_stats_panel = false;
	if (*cl_stats_mode) {
		if (*cl_stats_mode == 1)
			show_stats_panel = LobbyGetSize() <= 1;
		else
			show_stats_panel = true;
	}

	const int extra_bottom_rows = 3;
	const int extra_top_rows = 3;
	const int extra_left_columns = show_stats_panel ? 6 : 0;
	const int extra_right_columns = 6;

	unsigned short board_inner_columns = boards[0].columns;
	unsigned short board_rows = boards[0].visible_rows + (g_drawborder ? 2 : 0);
	unsigned short board_columns = board_inner_columns + (g_drawborder ? 2 : 0);

	unsigned short rows = board_rows + extra_bottom_rows + extra_top_rows;
	unsigned short columns = board_columns + extra_left_columns + extra_right_columns;
	
	float aspect = (float)columns / (float)rows;

	float first_board_x = 0.f;
	float first_board_y = 0.f;
	float gap = 0.f;

	float cell_w;
	float cell_h;

	if (aspect * visible_board_count > (float)w / (float)h) {
		cell_w = (float)w / (float)visible_board_count;
		cell_h = cell_w / aspect;

		first_board_y = h / 2.f - cell_h / 2.f;
	}
	else {
		cell_h = (float)h;
		cell_w = cell_h * aspect;

		gap = ((float)w - cell_w * (float)visible_board_count) / ((float)visible_board_count - 1.f);

		if (visible_board_count == 1)
			first_board_x = w / 2.f - cell_w / 2.f;
	}

	float tile_w = (float)cell_w / (float)columns;
	float tile_h = (float)cell_h / (float)rows;

	float current_cell_x = first_board_x;
	float current_cell_y = first_board_y;

	for (unsigned int i = 0; i < board_count; ++i) {
		if (boards[i].visible)
		{
			boards[i].x = current_cell_x + tile_w * extra_left_columns;
			boards[i].y = current_cell_y + tile_h * extra_bottom_rows;
			boards[i].width = tile_w * board_columns;
			boards[i].height = tile_h * board_rows;

			boards[i].nametag->size = tile_h;
			ConstrainBoardNameTag(boards + i, tile_w * board_inner_columns);
			GenerateTextData(boards[i].nametag, Font_UVSize(&g_font));

			current_cell_x += cell_w + gap;
		}
	}

	float u = (float)w / (float)g_textures[TEX_BG].width * 8.f / tile_w;
	float v = (float)h / (float)g_textures[TEX_BG].height * 8.f / tile_h;

	QuadSetData(&bgquad, u, v);

	bg_offset[0] = -1.f * ((float)first_board_x / (float)w) * u;
	bg_offset[1] = -1.f * ((float)first_board_y / (float)h) * v;

	Menu *menu = MenuGet();
	Mat3Identity(menu->transform);
	Mat3Scale(menu->transform, tile_w, tile_h);
	Mat3Translate(menu->transform, boards[0].x + tile_w, boards[0].y + tile_h);
}

void GameInputDrop() {
	if (board_count && !g_in_menu && !*sv_paused && *sv_hard_drop) {
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
	if (board_count && !g_in_menu && !*sv_paused && BoardInputCW(boards + 0)) {
		SMPlaySound(g_audio.rotate, false);
		SendBlockDataMessage();
	}
}

void GameInputCCW() {
	if (board_count && !g_in_menu && !*sv_paused && BoardInputCCW(boards + 0)) {
		SMPlaySound(g_audio.rotate, false);
		SendBlockDataMessage();
	}
}

void GameInputHold() {
	if (board_count && !g_in_menu && !*sv_paused && *sv_hold && can_use_held) {
		if (boards[0].held_index == 0xFF) {
			boards[0].held_index = GetIndexOfBlockID(boards[0].block.id);
			GameUseNextBlock();
		}
		else {
			byte held_index_old = boards[0].held_index;
			boards[0].held_index = GetIndexOfBlockID(boards[0].block.id);
			
			free(boards[0].block.data);
			CreateNewBlock(held_index_old, &boards[0].block, boards[0].visible_rows - 1);

			BoardUpdateGhostY(boards + 0);

			SendBlockPosMessage();
			SendBlockDataMessage();
		}

		byte message[] = { SVMSG_HOLD, boards[0].held_index };
		MessageServer(message, sizeof(message));

		can_use_held = false;
	}
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

void GameBoardAddClientScore(int id, uint32 score) {
	if (board_count == 0) return;

	boards[id].score += score;
}

void GameBoardSetLineClears(int id, uint16 line_clears) {
	if (board_count == 0) return;

	boards[id].line_clears = line_clears;
}

void GameBoardSetBlockPos(int id, signed short x, signed short y) {
	if (board_count == 0) return;

	bool x_update = boards[id].block.x != x;

	boards[id].block.x = x;
	boards[id].block.y = y;

	if (x_update)
		BoardUpdateGhostY(boards + id);
}

void GameBoardSetBlockData(int id, char tile_id, int size, const byte *data) {
	if (board_count == 0) return;

	boards[id].active = true;

	boards[id].block.id = tile_id;

	int sizesq = size * size;

	if (boards[id].block.size != size) {
		boards[id].block.data = (byte*)realloc(boards[id].block.data, sizesq);
		boards[id].block.size = size;
	}

	memcpy_s(boards[id].block.data, sizesq, data, sizesq);

	BoardUpdateGhostY(boards + id);
}

void GameBoardSetQueue(int id, byte length, const byte *queue) {
	if (board_count == 0) return;

	boards[id].visible_queue_length = length;

	if (boards[id].queue_length != length) {
		boards[id].next_queue = (byte*)realloc(boards[id].next_queue, length);
		boards[id].queue_length = length;
	}

	memcpy_s(boards[id].next_queue, length, queue, length);
}

void GameBoardSetHeldBlock(int id, byte blockid) {
	if (board_count == 0) return;

	boards[id].held_index = blockid;
}

void GameBoardSetVisible(int id, bool visible) {
	if (board_count == 0) return;

	if (boards[id].visible != visible)
	{
		boards[id].visible = visible;

		GameSizeUpdate(0, 0);
	}
}

void GameBoardPlaceBlock(int id) {
	if (board_count == 0) return;

	GameBoardSubmitBlock(id);
}

void GameBoardClear(int id) {
	if (board_count == 0) return;

	if (id == 0)
		boards[id].active = true;

	BoardClear(boards + id);

	boards[id].block.size = 0;
	boards[id].block.data = NULL;

	boards[id].score = 0;
	boards[id].line_clears = 0;
	boards[id].level_clears = 0;
	boards[id].level = 0;
	boards[id].held_index = 0xFF;

	if (id == 0) {
		boards[0].next_queue[0] = 0xFF;
		BoardRefillQueueSlots(boards + 0);

		GameUseNextBlock();
	}
	else
		boards[id].visible_queue_length = 0;

	for (int i = 0; i < GARBAGE_QUEUE_SIZE; ++i)
		boards[id].garbage_queue[i].rows = 0;

	if (!IsRemoteClient())
		ExecLevelBind(0, id);
}

void GameBoardFinished(int id) {
	if (id == 0) {
		g_in_game = false;

		if (g_in_menu)
			MenuGet()->closable = false;
		else
			CreateMenu_Pause(false);

		SMClearSounds();
		SMPlaySound(g_audio.gameover, false);
	}
	
	boards[id].active = false;
	
	if (!IsRemoteClient()) {

		int active_players = 0;

		for (unsigned int i = 0; i < board_count; ++i)
			if (boards[i].active)
				++active_players;
		
		if (active_players <= 1) {

			for (unsigned int i = 0; i < board_count; ++i)
				if (boards[i].active) {
					byte message[2] = { SVMSG_STOP, (byte)i };
					ServerBroadcast(message, 2, -1);
				}

		}
	}

}

void GameBoardAddGarbage(int id, byte rows, byte clear_column) {
	if (board_count == 0) return;

	BoardAddGarbage(boards + id, rows, clear_column);
}

void GameSendAllBoardData(int playerid) {
	uint16 board_data_length = boards[0].rows * boards[0].columns;
	uint16 message_length = board_data_length + 2;
	if (message_length < 6)
		message_length = 6;

	byte *message = (byte*)malloc(message_length);

	for (unsigned int i = 0; i < board_count; ++i) {
		if (boards[i].visible && i != playerid) {
			message[1] = (byte)i;

			message[0] = SVMSG_BOARD;
			memcpy_s(message + 2, board_data_length, &boards[i].data[0][0], board_data_length);
			ServerSend(playerid, message, board_data_length + 2);

			message[0] = SVMSG_BLOCKDATA;
			message[2] = boards[i].block.id;
			message[3] = boards[i].block.size;
			memcpy_s(message + 4, message_length - 4, boards[i].block.data, SQUARE(boards[i].block.size));
			ServerSend(playerid, message, SQUARE(boards[i].block.size) + 4);

			message[0] = SVMSG_BLOCKPOS;
			Int16ToBuffer(boards[i].block.x, message + 2);
			Int16ToBuffer(boards[i].block.y, message + 4);
			ServerSend(playerid, message, 6);

			message[0] = SVMSG_QUEUE;
			message[2] = (byte)boards[i].visible_queue_length;
			memcpy_s(message + 3, message_length - 3, boards[i].next_queue, boards[i].visible_queue_length);
			ServerSend(playerid, message, boards[i].visible_queue_length + 3);
		}
	}
}

void GameReceiveBoardData(int id, const byte *data, uint16 length) {
	uint16 board_data_lenth = boards[0].rows * boards[0].columns;
	
	memcpy_s(&boards[id].data[0][0], board_data_lenth, data, length);
}
