#include "Text.h"

#include "Vertex.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

Text CreateText() {
	Text text;
	glGenVertexArrays(1, &text.vao);
	glGenBuffers(1, &text.vbo);
	return text;
}

void DeleteText(Text *text) {
	glDeleteVertexArrays(1, &text->vao);
	glDeleteBuffers(1, &text->vbo);

	text->vao = text->vbo = 0;
}

void RenderText(const Text *text) {
	glBindVertexArray(text->vao);
	glDrawArrays(GL_TRIANGLES, 0, text->index_count);
}

inline bool IsSpace(char c) {
	return c == ' ';
}

int CharIndex(char c) {
	if (IsSpace(c))
		return -1;

	//0-9  -> 0-9
	if (c >= '0' && c <= '9')
		return c - '0';

	//A-Z -> 10-35
	if (c >= 'A' && c <= 'Z')
		return c - 'A' + 10;

	return 63;
}

unsigned int CharCount(const char *string) {
	unsigned int length = 0;
	for (const char *c = string; *c != '\0'; ++c)
		if (!IsSpace(*c))
			++length;

	return length;
}

#define DIVS 8
#define SIZE 0.25f
#define GAP 0.25f

void SetTextString(Text *text, const char *string) {
	text->index_count = 6 * CharCount(string);
	Vertex_2P_2UV *verts = malloc(sizeof(Vertex_2P_2UV) * text->index_count);

	float uv_sz = 1.f / (float)DIVS;
	float x = -1.f;
	int i = 0;
	for (const char *c = string; *c != '\0'; ++c) {
		int index = CharIndex(*c);

		if (index >= 0)
		{
			float uvx = (index % DIVS) / (float)DIVS;
			float uvy = (DIVS - 1 - (index / DIVS)) / (float)DIVS;

			verts[i].position[0] = x;
			verts[i].position[1] = 0.f;
			verts[i].uv[0] = uvx;
			verts[i++].uv[1] = uvy;

			verts[i].position[0] = x + SIZE;
			verts[i].position[1] = 0.f;
			verts[i].uv[0] = uvx + uv_sz;
			verts[i++].uv[1] = uvy;

			verts[i].position[0] = x;
			verts[i].position[1] = SIZE;
			verts[i].uv[0] = uvx;
			verts[i++].uv[1] = uvy + uv_sz;

			verts[i].position[0] = x + SIZE;
			verts[i].position[1] = SIZE;
			verts[i].uv[0] = uvx + uv_sz;
			verts[i++].uv[1] = uvy + uv_sz;

			verts[i].position[0] = x;
			verts[i].position[1] = SIZE;
			verts[i].uv[0] = uvx;
			verts[i++].uv[1] = uvy + uv_sz;

			verts[i].position[0] = x + SIZE;
			verts[i].position[1] = 0.f;
			verts[i].uv[0] = uvx + uv_sz;
			verts[i++].uv[1] = uvy;
		}

		x += GAP;
	}

	glBindVertexArray(text->vao);
	glBindBuffer(GL_ARRAY_BUFFER, text->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex_2P_2UV) * text->index_count, verts, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_2P_2UV), (GLvoid*)offsetof(Vertex_2P_2UV, position));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_2P_2UV), (GLvoid*)offsetof(Vertex_2P_2UV, uv));


	//free(verts);
}
