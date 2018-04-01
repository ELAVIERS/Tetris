#include "LevelManager.h"
#include "Console.h"
#include "Messaging.h"
#include "Server.h"
#include "String.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
	char block_id;
	short index;
} TextureBinding;

typedef struct TextureLevel_s {
	TextureBinding *texdata;
	unsigned int texdata_size;
} TextureLevel;

TextureLevel *levels = NULL;
unsigned int level_count = 0;

short TextureLevelIDIndex(unsigned int level, char id) {
	level %= level_count;

	for (unsigned int i = 0; i < levels[level].texdata_size; ++i)
		if (levels[level].texdata[i].block_id == id)
			return levels[level].texdata[i].index;

	return -1;
}

char **id_groups;
unsigned int group_count = 0;
unsigned int blockid_count;

void CLSetTextureIndexOrder(const char **tokens, unsigned int count) {
	FreeTokens(id_groups, group_count);

	id_groups = (char**)malloc(count * sizeof(char*));

	blockid_count = 0;
	for (unsigned int i = 0; i < count; ++i) {
		id_groups[i] = DupString(tokens[i]);
		blockid_count += (unsigned int)strlen(tokens[i]);
	}

	group_count = count;
}

void CLAddTextureLevel(const char **tokens, unsigned int count) {
	if (count != group_count) {
		ConsolePrint("Error : Invalid argument count\n");
		return;
	}

	unsigned int last = level_count;
	++level_count;
	levels = (TextureLevel*)realloc(levels, level_count * sizeof(TextureLevel));
	levels[last].texdata = (TextureBinding*)malloc(blockid_count * sizeof(TextureBinding));
	levels[last].texdata_size = blockid_count;

	unsigned int counter = 0;
	for (unsigned int i = 0; i < group_count; ++i) {
		short texid = (short)atoi(tokens[i]);
		for (const char *c = id_groups[i]; *c != '\0'; ++c) {
			levels[last].texdata[counter].block_id = *c;
			levels[last].texdata[counter].index = texid;
			++counter;
		}
	}
}

void ClearTextureLevels() {
	free(levels);
	levels = NULL;
	level_count = 0;
}


//Binding

typedef struct ValueBinding_s {
	Dvar *dvar;

	char *value;
} ValueBinding;

inline void ExecValueBinding(ValueBinding *bind, uint16 playerid) {
	byte message[256] = { SVMSG_COMMAND };
	strcpy_s(message + 1, 256 - 1, bind->dvar->name);
	strcat_s(message + 1, 256 - 1, " ");
	strcat_s(message + 1, 256 - 1, bind->value);
	ServerSend(playerid, message, (uint16)strlen(bind->dvar->name) + (uint16)strlen(bind->value) + 2);
}

typedef struct LevelBinding_s {
	uint16 level;

	uint16 bind_count;
	ValueBinding *binds;
} LevelBinding;

void AddBindToLevelBinding(LevelBinding *levelbind, Dvar *dvar, const char *value, bool overwrite) {
	for (uint16 i = 0; i < levelbind->bind_count; ++i)
		if (levelbind->binds[i].dvar == dvar) {
			if (overwrite) {
				free(levelbind->binds[i].value);
				levelbind->binds[i].value = DupString(value);
			}

			return;
		}

	uint16 last = levelbind->bind_count;
	++levelbind->bind_count;
	levelbind->binds = (ValueBinding*)realloc(levelbind->binds, levelbind->bind_count * sizeof(ValueBinding));

	levelbind->binds[last].dvar = dvar;
	levelbind->binds[last].value = DupString(value);
}

LevelBinding *levelbinds = NULL;
uint16 level_bind_count = 0;

void ExecLevelBind(uint16 level, uint16 playerid) {
	for (uint16 index = 0; index < level_bind_count; ++index)
		if (levelbinds[index].level == level) {
			for (uint16 i = 0; i < levelbinds[index].bind_count; ++i)
				ExecValueBinding(levelbinds[index].binds + i, playerid);
			
			break;
		}
}

void AddLevelBind(const char **tokens, unsigned int count) {
	if (count < 3) return;

	uint16 level = atoi(tokens[0]);

	Dvar *dvar;
	dvar = GetDvar(tokens[1]);
	if (dvar == NULL) {
		ConsolePrint("Variable not found\n");
		return;
	}

	DvarValue value;

	switch (dvar->type) {
	case DVT_FLOAT:
		value.number = (float)atof(tokens[2]);
		break;

	default:
		ConsolePrint("Variable type not supported for level binds\n");
		return;
	}

	if (level_bind_count == 0) {
		level_bind_count = 1;
		levelbinds = (LevelBinding*)calloc(1, sizeof(LevelBinding));
	}

	char *valuestr = DvarAllocValueString(dvar);
	AddBindToLevelBinding(levelbinds + 0, dvar, valuestr, false);
	free(valuestr);

	for (uint16 index = 0; index < level_bind_count; ++index)
		if (levelbinds[index].level == level) {
			AddBindToLevelBinding(levelbinds + index, dvar, tokens[2], true);
			return;
		}

	uint16 last = level_bind_count;
	++level_bind_count;
	levelbinds = (LevelBinding*)realloc(levelbinds, level_bind_count * sizeof(LevelBinding));

	levelbinds[last].level = level;
	levelbinds[last].binds = NULL;
	levelbinds[last].bind_count = 0;
	AddBindToLevelBinding(levelbinds + last, dvar, tokens[2], true);
}

void ClearLevelBinds() {
	//ExecLevelBind(0);
	free(levelbinds);
	levelbinds = NULL;
	level_bind_count = 0;
}
