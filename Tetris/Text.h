#pragma once
#include <GL/glew.h>
 
/*
	Text.h

	Manage vaos and vbos for text rendering inside a struct
 */

typedef struct {
	GLuint vao;
	GLuint vbo;
	unsigned int index_count;
} Text;

Text CreateText();
void DeleteText(Text*);
void RenderText(const Text*);
void SetTextString(Text*, const char *string);
