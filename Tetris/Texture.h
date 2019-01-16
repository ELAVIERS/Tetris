#pragma once
#include "GL.h"
/*
	Texture.h

	Texture management
*/

#define UV_WIDTH_OFFSET(TEXTURE) 0.5f/(float)(TEXTURE).width
#define UV_HEIGHT_OFFET(TEXTURE) 0.5f/(float)(TEXTURE).height

typedef struct {
	GLuint glid;

	unsigned short width, height;
} Texture;

void TextureFromFile(const char *filepath, Texture *out);