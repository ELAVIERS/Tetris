#include "Texture.h"

//Only supports bitdepths 1 and 24
GLuint TextureFromBMP(Bitmap *bmp, bool delete) {
	if (!bmp || !bmp->buffer)
		return 0;

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, bmp->bitcount == 8 ? GL_R8 : GL_RGB8, 
		bmp->width, bmp->height, 0, bmp->bitcount == 8 ? GL_RED : GL_BGR, 
		GL_UNSIGNED_BYTE, bmp->buffer);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	if (delete) FreeBMPBuffer(bmp);

	return texture;
}
