#include "Text.h"

#include "Vertex.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

Text* AllocText() {
	Text *text = (Text*)malloc(sizeof(Text));
	text->string = NULL;
	glGenVertexArrays(1, &text->vao);
	glGenBuffers(1, &text->vbo);
	return text;
}

void FreeText(Text *text) {
	glDeleteVertexArrays(1, &text->vao);
	glDeleteBuffers(1, &text->vbo);

	free(text->string);
	free(text);
}

void RenderText(const Text *text) {
	glBindVertexArray(text->vao);
	glDrawArrays(GL_TRIANGLES, 0, text->data_length);
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

	if (c >= 'a' && c <= 'z')
		return c - 'a' + 36;

	if (c == '.')
		return 62;

	return 63;
}

unsigned int CharCount(const char *string) {
	unsigned int length = 0;
	for (const char *c = string; *c != '\0'; ++c)
		if (!IsSpace(*c))
			++length;

	return length;
}

void GenerateTextData(Text *text, float uv_size) {
	text->data_length = 6 * CharCount(text->string);

	Vertex_2P_2UV *verts = (Vertex_2P_2UV*)malloc(sizeof(Vertex_2P_2UV) * text->data_length);

	float divs = 1.f / uv_size;
	float cx = text->x;
	int i = 0;
	for (const char *c = text->string; *c != '\0'; ++c) {
		int index = CharIndex(*c);

		if (index >= 0)
		{
			float uvx = (index % (int)divs) / divs;
			float uvy = (divs - 1 - (index / (int)divs)) / divs;

			verts[i].position[0] = cx;
			verts[i].position[1] = (float)text->y;
			verts[i].uv[0] = uvx;
			verts[i++].uv[1] = uvy;

			verts[i].position[0] = cx + text->size;
			verts[i].position[1] = (float)text->y;
			verts[i].uv[0] = uvx + uv_size;
			verts[i++].uv[1] = uvy;

			verts[i].position[0] = cx;
			verts[i].position[1] = (float)text->y + (float)text->size;
			verts[i].uv[0] = uvx;
			verts[i++].uv[1] = uvy + uv_size;

			verts[i].position[0] = cx + text->size;
			verts[i].position[1] = (float)text->y + (float)text->size;
			verts[i].uv[0] = uvx + uv_size;
			verts[i++].uv[1] = uvy + uv_size;

			verts[i].position[0] = cx;
			verts[i].position[1] = (float)text->y + (float)text->size;
			verts[i].uv[0] = uvx;
			verts[i++].uv[1] = uvy + uv_size;

			verts[i].position[0] = cx + text->size;
			verts[i].position[1] = (float)text->y;
			verts[i].uv[0] = uvx + uv_size;
			verts[i++].uv[1] = uvy;
		}

		cx += text->size;
	}

	glBindVertexArray(text->vao);
	glBindBuffer(GL_ARRAY_BUFFER, text->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex_2P_2UV) * text->data_length, verts, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_2P_2UV), (GLvoid*)offsetof(Vertex_2P_2UV, position));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_2P_2UV), (GLvoid*)offsetof(Vertex_2P_2UV, uv));
}
