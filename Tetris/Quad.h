#pragma once
#include "GL.h"

/*
	Quad.h

	It's just a quad
*/

typedef struct Quad_s {
	GLuint vao;
	GLuint vbo;

	float uv_w, uv_h;
} Quad;

void QuadCreate(Quad*);
void QuadSetData(Quad*, float uvw, float uvh);
void QuadDelete(Quad*);
void QuadRender(const Quad*);
