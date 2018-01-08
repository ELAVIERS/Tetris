#pragma once

/*
	TakeLine

	Returns the amount of chars from the beginning of src to the next newline (NOT the length of the string)
	Returns 0 when a line could not be found
*/
unsigned int TakeLine(char dest[], const char *src, unsigned int count);
