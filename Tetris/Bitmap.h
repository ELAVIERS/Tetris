#pragma once
#include "Types.h"
#include <stdbool.h>

/*
	Bitmap.h

	Anything to do with bitmaps (.bmp files) goes here
*/

typedef struct {
	byte *buffer;
	uint32 width;
	uint32 height;
	uint16 bitcount;
} Bitmap;

bool LoadBMP(const char *filepath, Bitmap *out_bmp);
