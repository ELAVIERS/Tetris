#pragma once

/*
	DupString

	Returns a copy of source
*/
char* DupString(const char *source);

/*
	TakeLine

	Returns the amount of chars from the beginning of src to the next newline (NOT the length of the string)
	Returns 0 when a line could not be found
	
	NOTE:Removes all non-letter characters preceded by a '#' until next letter
*/
unsigned int TakeLine(char dest[], const char *src, unsigned int count);

char* AllocStringFromFloat(float value);

char* AllocStringFromInt(int value);

/*
	SplitTokens

	Splits string into 
*/
unsigned int SplitTokens(const char *string, char ***out_tokens);
void FreeTokens(char **tokens, unsigned int count);

char* SeperateDir(const char *filepath);
void CutExt(char *str);
