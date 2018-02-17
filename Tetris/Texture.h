#pragma once
#include <GL/glew.h>

/*
	Texture.h

	Texture management
*/

typedef struct {
	GLuint glid;

	unsigned short width, height;
} Texture;

void TextureFromFile(const char *filepath, Texture *out);