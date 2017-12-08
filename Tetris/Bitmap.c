#include "Bitmap.h"

#include "Error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERROR_LEN 256

//Reads a little-endian 2 byte value from file
inline uint16_t Read2B(FILE* file) {
	return
		fgetc(file) +
		(fgetc(file) << 8);
}

//Reads a little-endian 4 byte value from file
inline uint32_t Read4B(FILE* file) {
	return	
		fgetc(file) +
		(fgetc(file) << 8) +
		(fgetc(file) << 16) +
		(fgetc(file) << 24);
}

//Loads a BMP file from filepath
//http://www.dragonwins.com/domains/getteched/bmp/bmpfileformat.htm
Bitmap LoadBMP(const char* filepath) {
	Bitmap bmp = { NULL, 0, 0 };

	FILE* file;
	fopen_s(&file, filepath, "rb");

	if (!file) {
		char error[ERROR_LEN] = "Could not load bmp file \"";
		strcat_s(error, ERROR_LEN, filepath);
		strcat_s(error, ERROR_LEN, "\"");
		ErrorMessage(error);
		return bmp;
	}

	fseek(file, 0, SEEK_END);
	long filesize = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	//BMP Header
	if (!(fgetc(file) == 'B' && fgetc(file) == 'M'))
		return bmp;

	uint32_t file_size = Read4B(file);
	fseek(file, 4, SEEK_CUR);				//Skip reserved bytes
	uint32_t buffer_offset = Read4B(file);

	//Image Header
	uint32_t ih_size = Read4B(file);
	bmp.width = Read4B(file);
	bmp.height = Read4B(file);
	fseek(file, 2, SEEK_CUR);				//Skip image plane count
	bmp.bitcount = Read2B(file);
	if (Read4B(file) != 0) ErrorMessage("Warning : Image is compressed");	//Compression
	uint32_t buffer_len = Read4B(file);
	if (buffer_len == 0) ErrorMessage("Warning : Unknown buffer length");

	//Pixel data
	fseek(file, buffer_offset, SEEK_SET);
	bmp.buffer = (unsigned char*)malloc(buffer_len);

	fread(bmp.buffer, 1, buffer_len, file);
	fclose(file);

	return bmp;
}
