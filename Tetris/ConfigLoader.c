#include "ConfigLoader.h"

#include "Error.h"
#include "IO.h"
#include "String.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <Windows.h>

inline bool IsLetter(char c) {
	return c > (char)32;
}

unsigned int SplitTokens(const char *string, char ***out_tokens) {
	unsigned int token_count = 0;

	for (const char *c = string; *c != '\0';) {
		if (IsLetter(*c)) {
			++token_count;
			while (IsLetter(*(++c)));
		}
		else ++c;
	}

	if (token_count) {
		char **tokens = malloc(sizeof(char*) * token_count);

		unsigned int token = 0;
		for (const char *c = string; *c != '\0';) {
			if (IsLetter(*c)) {
				unsigned int length = 0;
				for (const char *c2 = c; IsLetter(*c2); ++c2)
					++length;

				tokens[token] = malloc(length + 1);

				for (unsigned int i = 0; i < length; ++i)
					tokens[token][i] = c[i];

				tokens[token][length] = '\0';

				++token;
				while (IsLetter(*(++c)));
			}
			else ++c;
		}

		*out_tokens = tokens;
	}

	return token_count;
}

#define LINE_MAX 512

unsigned int LoadConfigFile(const char *filepath, Statement **out_statements) {
	char *fbuffer;
	if (FileRead(filepath, &fbuffer) == 0)
		return 0;

	unsigned int line_count = 0;

	Statement *statements = malloc(0);

	char line[LINE_MAX];
	unsigned int index = 0;
	unsigned int bufferindex = 0;
	unsigned int chars_to_next_line;
	while (chars_to_next_line = TakeLine(line, fbuffer + bufferindex, LINE_MAX)) {
		if (line[0] != '/') {
			++line_count;
			statements = realloc(statements, line_count * sizeof(Statement));

			statements[index].count = SplitTokens(line, &statements[index].tokens);
			++index;
		}

		bufferindex += chars_to_next_line;
	}

	free(fbuffer);

	*out_statements = statements;
	return line_count;
}

unsigned int LoadConfigMulti(const char *directory, const char *filter, Configuration **out_conf) {
	char search_path[MAX_PATH];
	strcpy_s(search_path, MAX_PATH, directory);
	strcat_s(search_path, MAX_PATH, filter);
	
	char **files;
	unsigned int filecount = FindFilesInDirectory(search_path, &files, 0xFFFFFFFF);

	if (filecount == 0)
		return 0;

	Configuration *config = malloc(sizeof(Configuration) * filecount);

	char filepath[MAX_PATH];
	for (int fid = 0; fid < filecount; ++fid) {
		strcpy_s(filepath, MAX_PATH, directory);
		strcat_s(filepath, MAX_PATH, files[fid]);

		unsigned int pathlen = strlen(directory) + 1;
		config[fid].path = malloc(pathlen);
		strcpy_s(config[fid].path, pathlen, directory);

		unsigned int namelen = strlen(files[fid]) + 1;
		config[fid].name = malloc(namelen);
		strcpy_s(config[fid].name, namelen, files[fid]);

		free(files[fid]);

		config[fid].count = LoadConfigFile(filepath, &config[fid].statements);

		OutputDebugStringA("\nREAD CONFIG FILE \"");
		OutputDebugStringA(filepath);
		OutputDebugStringA("\"\n");

		for (int i = 0; i < config[fid].count; ++i) {
			for (int j = 0; j < config[fid].statements[i].count; ++j) {
				OutputDebugStringA(config[fid].statements[i].tokens[j]);
				OutputDebugStringA("|");
			}

			OutputDebugStringA("\n");
		}
	}

	free(files);

	*out_conf = config;
	return filecount;
}

unsigned int LoadConfigsFromDir(const char *directory, const char *filter, Configuration **out_configs) {
	char search_path[MAX_PATH];

	//Search for child directories
	strcpy_s(search_path, MAX_PATH, directory);
	strcat_s(search_path, MAX_PATH, "*");

	char **dirnames;
	unsigned int dir_count = FindFilesInDirectory(search_path, &dirnames, FILE_ATTRIBUTE_DIRECTORY);

	if (dir_count == 0)
		return 0;

	//Sort out . and ..
	dir_count -= 2;

	free(dirnames[0]);
	free(dirnames[1]);

	if (dir_count == 0) {
		free(dirnames);
		return 0;
	}

	Configuration *configs = NULL;
	unsigned int config_count = 0;

	char filepath[MAX_PATH];
	for (int i = 0; i < dir_count; ++i) {
		strcpy_s(filepath, MAX_PATH, directory);
		strcat_s(filepath, MAX_PATH, dirnames[i + 2]);
		strcat_s(filepath, MAX_PATH, "/");
		free(dirnames[i + 2]);

		Configuration *temp_configs;
		unsigned int temp_config_count = LoadConfigMulti(filepath, filter, &temp_configs);

		if (temp_config_count != 0) {
			int first = config_count;
			config_count += temp_config_count;
			configs = realloc(configs, config_count);

			for (int i = 0; i < temp_config_count; ++i) {
				configs[first + i] = temp_configs[i];
			}

			free(temp_configs);
		}
	}
	free(dirnames);

	*out_configs = configs;
	return dir_count;
}
