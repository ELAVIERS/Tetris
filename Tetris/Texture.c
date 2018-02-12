#include "Texture.h"
#include "Bitmap.h"
#include <stdlib.h>

//Only supports bitdepths 1 and 24
void TextureFromFile(const char *filepath, Texture *out) {
	Bitmap bmp;
	LoadBMP(filepath, &bmp);
	
	if (bmp.buffer) {
		if (out->glid)
			glDeleteTextures(1, &out->glid);

		out->width = bmp.width;
		out->height = bmp.height;

		glGenTextures(1, &out->glid);
		glBindTexture(GL_TEXTURE_2D, out->glid);
		glTexImage2D(GL_TEXTURE_2D, 0, bmp.bitcount == 8 ? GL_R8 : GL_RGB8,
			bmp.width, bmp.height, 0, bmp.bitcount == 8 ? GL_RED : GL_BGR,
			GL_UNSIGNED_BYTE, bmp.buffer);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		free(bmp.buffer);
	}
}
