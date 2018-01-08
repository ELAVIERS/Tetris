#pragma once
#include <stdlib.h>
#include <Windows.h>

/*
	FindFilesInDirectory

	Returns amount of files in directory
*/
unsigned int FindFilesInDirectory(const char *filepath, char ***out_files, DWORD filter);

/*
	FileRead
	
	Returns bytes read
*/
unsigned int FileRead(const char *filepath, char **buffer_out);
