#pragma once
#include "Matrix.h"
#include "Quad.h"
#include "Types.h"

/*
	RenderTileBuffer

	buffer				data to render
	rows, columns		buffer dimensions
	transform			transformation to apply after block translation
	quad				the quad to render tiles with
*/
void RenderTileBuffer(const byte *buffer, byte rows, byte columns, Mat3 in_transform, const Quad* quad, unsigned int level);

void RenderBorder(float x, float y, float w, float h, float bw, float bh);

void RenderString(const char *string, Mat3 transform);
