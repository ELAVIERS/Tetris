#pragma once
#include "Bitmap.h"
#include <GL/glew.h>
#include <stdbool.h>

/*
	Texture.h

	Anything to do with OpenGL textures goes here
*/

GLuint TextureFromBMP(Bitmap *bmp, bool delete);
