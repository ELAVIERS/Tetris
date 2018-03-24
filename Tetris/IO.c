#include "IO.h"
#include <stdbool.h>
#include <Windows.h>

unsigned int FindFilesInDirectory(const char *filepath, char ***out_files, DWORD filter) {
	WIN32_FIND_DATAA find_data;
	HANDLE h_find = FindFirstFileA(filepath, &find_data);

	if (h_find == INVALID_HANDLE_VALUE)
		return 0;

	int file_count = 0;
	do {
		if (find_data.dwFileAttributes & filter)
			++file_count;
	} while (FindNextFileA(h_find, &find_data));

	char **files = (char**)malloc(file_count * sizeof(char*));

	//I do not like doing this...
	h_find = FindFirstFileA(filepath, &find_data);
	int i = 0;
	do {
		if (find_data.dwFileAttributes & filter) {
			files[i] = (char*)malloc(MAX_PATH);
			strcpy_s(files[i], MAX_PATH, find_data.cFileName);
			++i;
		}
	} while (FindNextFile(h_find, &find_data));

	*out_files = files;
	return file_count;
}

VOID CALLBACK CallbackThatWindowsWantsElseEverythingGoesMental(__in  DWORD dwErrorCode, __in  DWORD dwNumberOfBytesTransfered, __in  LPOVERLAPPED lpOverlapped) {}

unsigned int FileRead(const char *filepath, char **buffer_out) {
	HANDLE file = CreateFileA(filepath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (!file)
		return 0;

	LARGE_INTEGER file_len;
	if (!GetFileSizeEx(file, &file_len))
		return 0;  

	char *buffer = (char*)malloc(file_len.LowPart + 1);
	
	OVERLAPPED ovl = {0};
	ReadFileEx(file, buffer, file_len.LowPart, &ovl, CallbackThatWindowsWantsElseEverythingGoesMental);
	buffer[file_len.LowPart] = '\0';

	CloseHandle(file);

	*buffer_out = buffer;
	return file_len.LowPart;
}

void FileWrite(const char *filepath, const char *buffer) {
	HANDLE file = CreateFileA(filepath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (!file) return;

	WriteFile(file, (LPCVOID)buffer, (DWORD)strlen(buffer), NULL, NULL);

	CloseHandle(file);
}
