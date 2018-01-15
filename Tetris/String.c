#include "String.h"

#include <stdlib.h>
#include <string.h>

char* DupString(const char *source) {
	unsigned int length = strlen(source) + 1;
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
	int line_length;
	for (line_length = 0; line_length < count && !END_OF_LINE(src[line_length]) && src[line_length] != '\0'; ++line_length)
		dest[line_length] = src[line_length];

	dest[line_length] = '\0';

	//Return total of skipped characters and line characters
	return skip + line_length;
}
