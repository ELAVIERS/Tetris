#pragma once
#include <GL/glew.h>

typedef struct {
	GLuint glid;

	unsigned short width, height;
} Texture;

/*
	Texture.h

	Anything to do with OpenGL textures goes here
*/

void TextureFromFile(const char *filepath, Texture *out);