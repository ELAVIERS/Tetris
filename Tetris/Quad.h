#pragma once
#include <GL/glew.h>

/*
	Quad.h

	It's just a quad
*/

typedef struct Quad_s {
	GLuint vao;
	GLuint vbo;
} Quad;

void QuadCreate(Quad*);
void QuadSetData(Quad*, float uvw, float uvh);
void QuadDelete(Quad*);
void QuadRender(const Quad*);
