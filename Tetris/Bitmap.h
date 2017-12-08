#pragma once
#include <stdint.h>

/*
	Bitmap.h

	Anything to do with bitmaps (.bmp files) goes here
*/

typedef struct {
	uint8_t* buffer;
	uint32_t width;
	uint32_t height;
	uint16_t bitcount;
} Bitmap;

Bitmap LoadBMP(const char* filepath);
