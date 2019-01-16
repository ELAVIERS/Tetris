#pragma once
#include "GL.h"
 
/*
	Text.h

	Manage vaos and vbos for text rendering inside a struct
 */

int GetCharID(char);

typedef struct {
	char			*string;
	unsigned int	x, y;
	float			size;

	GLuint			vao;
	GLuint			vbo;
	unsigned int	data_length;
} Text;

Text* AllocText();
void FreeText(Text*);
void RenderText(const Text*);
void GenerateTextData(Text*, float uv_size);

#include "String.h"

//Just an inline to set text struct stuff
inline void SetTextInfo(Text *text, const char *string, unsigned int x, unsigned int y, float size) {
	text->string = DupString(string);
	text->x = x;
	text->y = y;
	text->size = size;
}
