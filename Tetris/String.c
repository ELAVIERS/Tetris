#include "String.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char* DupString(const char *source) {
	size_t length = strlen(source) + 1;
	char *string = (char*)malloc(length);
	strcpy_s(string, length, source);
	return string;
}


#define END_OF_LINE(CHAR) (CHAR == '\n' || CHAR == '\r')

unsigned int TakeLine(char dest[], const char *src, unsigned int count) {
	//Count the newlines we skip
	int skip = 0;
	while (END_OF_LINE(*src)) {
		if (*src == '\0')
			return 0;

		++src;
		++skip;
	}

	//Copy line from src to dest
	unsigned int line_length;
	for (line_length = 0; line_length < count && !END_OF_LINE(src[line_length]) && src[line_length] != '\0'; ++line_length) {
		//If we are at a #, increment src until src[line_length] is a letter
		if (src[line_length] == '#')
			do ++skip;
			while ((++src)[line_length] <= ' ');

		dest[line_length] = src[line_length];
	}

	dest[line_length] = '\0';

	//Return total of skipped characters and line characters
	return skip + line_length;
}

char* AllocStringFromFloat(float value) {
	char* string = (char*)malloc(46);
	sprintf_s(string, 46, "%f", value);
	return string;
}

char* AllocStringFromInt(int value) {
	char* string = (char*)malloc(13);
	sprintf_s(string, 13, "%d", value);
	return string;
}

inline bool IsLetter(char c) {
	return c > ' ';
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
		char **tokens = (char**)malloc(token_count * sizeof(char*));

		unsigned int token = 0;
		for (const char *c = string; *c != '\0';) {
			if (IsLetter(*c)) {
				unsigned int length = 0;
				for (const char *c2 = c; IsLetter(*c2); ++c2)
					++length;

				tokens[token] = (char*)malloc(length + 1);

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

void FreeTokens(char **tokens, unsigned int count) {
	if (count == 0)
		return;
	
	for (unsigned int i = 0; i < count; ++i)
		free(tokens[i]);
	
	free(tokens);
}

char* CombineTokens(const char **tokens, unsigned int count) {
	char *string;
	size_t string_size = 0;

	for (unsigned int i = 0; i < count; ++i)
		string_size += strlen(tokens[i]);

	string_size += count;

	string = (char*)malloc(string_size);

	unsigned int token = 0;
	unsigned int i = 0;
	while (i < string_size) {
		for (const char *c = tokens[token++]; *c != '\0'; ++c)
			string[i++] = *c;

		string[i++] = ' ';
	}

	string[i - 1] = '\0';

	return string;
}

char* SeperateDir(const char *filepath) {
	unsigned int last = 0;
	for (unsigned int i = 0; filepath[i] != '\0'; ++i)
		if (filepath[i] == '/')
			last = i;

	if (last == 0)
		return DupString("");

	char *dir = (char*)malloc(last + 2);
	for (unsigned int i = 0; i <= last; ++i)
		dir[i] = filepath[i];
	dir[last + 1] = '\0';

	return dir;
}

void CutExt(char *str) {
	char* lastdot = NULL;

	for (char *c = str; *c != '\0'; ++c)
		if (*c == '.')
			lastdot = c;

	if (lastdot)
		*lastdot = '\0';
}
