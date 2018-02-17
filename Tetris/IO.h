#pragma once
#include <stdlib.h>
#include <Windows.h>

#define FILTER_NONE 0xFFFFFFFF

/*
	FindFilesInDirectory

	out_files: array of filenames found in directory

	Returns amount of files in directory
*/
unsigned int FindFilesInDirectory(const char *filepath, char ***out_files, DWORD filter);

/*
	FileRead
	Allocates a string and reads a specified file into it

	buffer_out: String to contain the contents of the file

	Returns the length of buffer_out
*/
unsigned int FileRead(const char *filepath, char **string_out);

/*
	FileWrite

	Writes a string to a file
*/
void FileWrite(const char *filepath, const char *string);
