#include "Config.h"
#include "Console.h"
#include "Dvar.h"
#include "IO.h"
#include "String.h"
#include <stdlib.h>

#define LINE_MAX 512

bool RunConfig(const char *filepath) {
	char *fbuffer;
	if (FileRead(filepath, &fbuffer) == 0)
		return false;

	char *directory = SeperateDir(filepath);

	char line[LINE_MAX];
	unsigned int index = 0;
	unsigned int bufferindex = 0;
	unsigned int chars_to_next_line;
	while (chars_to_next_line = TakeLine(line, fbuffer + bufferindex, LINE_MAX)) {
		if (line[0] != '/') {
			char **tokens;
			unsigned int count = SplitTokens(line, &tokens);

			//Handle relative directories
			for (unsigned int i = 0; i < count; ++i) {
				if (tokens[i][0] == '/') {
					size_t newsize = strlen(directory) + strlen(tokens[i]) + 1;
					char *newtoken = (char*)malloc(newsize);
					strcpy_s(newtoken, newsize, directory);
					strcat_s(newtoken, newsize, tokens[i] + 1);
					free(tokens[i]);
					tokens[i] = newtoken;
				}
			}

			HandleCommand(tokens, count);
			FreeTokens(tokens, count);
		}

		bufferindex += chars_to_next_line;
	}

	free(directory);
	free(fbuffer);
	return true;
}

//Config Variables (CVARS) : remember which Dvars to save

typedef struct CVarNode_s {
	HDvar dvar;

	struct CVarNode_s *next;
} CVarNode;

CVarNode *first = NULL;

void AddCvar(HDvar dvar) {
	CVarNode *node = (CVarNode*)malloc(sizeof(CVarNode));

	node->dvar = dvar;
	node->next = first;
	first = node;
}

void FreeCvars() {
	for (CVarNode *node = first; node; node = node->next)
		free(node);
}

void SaveCvars() {
	char *buffer = NULL;
	size_t size = 1;

	for (CVarNode *node = first; node; node = node->next) {
		const char *name = HDvarName(node->dvar);
		char *value = HDvarValueAsString(node->dvar);

		if (value) {
			size += strlen(name) + 1 + strlen(value) + 1;
			buffer = (char*)realloc(buffer, size);

			strcpy_s(buffer, size, name);
			strcat_s(buffer, size, " ");
			strcat_s(buffer, size, value);
			strcat_s(buffer, size, "\n");

			free(value);
		}
	}

	FileWrite("config.cfg", buffer);
	ConsolePrint("Config variables saved\n");
}
