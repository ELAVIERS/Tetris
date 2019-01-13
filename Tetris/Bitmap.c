#include "Bitmap.h"
#include "Console.h"
#include "IO.h"
#include <stdio.h>
#include <stdlib.h>

//Loads a BMP file from filepath
//http://www.dragonwins.com/domains/getteched/bmp/bmpfileformat.htm
bool LoadBMP(const char *filepath, Bitmap *bmp) {
	FILE* file;
	fopen_s(&file, filepath, "rb");

	if (!file) {
		ConsolePrint("Error : could not load bitmap \"");
		ConsolePrint(filepath);
		ConsolePrint("\"\n");
		return false;
	}
	
	//BMP Header
	if (!(fgetc(file) == 'B' && fgetc(file) == 'M'))
		return false;

	uint32 file_size = Read4B(file);
	fseek(file, 4, SEEK_CUR);				//Skip reserved bytes
	uint32 buffer_offset = Read4B(file);

	//Image Header
	uint32 ih_size = Read4B(file);
	bmp->width = Read4B(file);
	bmp->height = Read4B(file);
	fseek(file, 2, SEEK_CUR);				//Skip image plane count
	bmp->bitcount = Read2B(file);
	if (Read4B(file) != 0) ConsolePrint("Warning : Image is compressed\n");	//Compression
	uint32 buffer_len = Read4B(file);
	if (buffer_len == 0) ConsolePrint("Warning : Unknown buffer length\n");

	//Pixel data
	fseek(file, buffer_offset, SEEK_SET);
	bmp->buffer = (byte*)malloc(buffer_len);

	size_t bytes_read = fread(bmp->buffer, 1, buffer_len, file);
	fclose(file);

	return true;
}
