#include "Quad.h"
#include "Vertex.h"
#include <GL/glew.h>
#include <stdlib.h>

void QuadCreate(Quad *quad) {
	glGenVertexArrays(1, &quad->vao);
	glGenBuffers(1, &quad->vbo);
}

void QuadSetData(Quad *quad, float uvw, float uvh) {
	Vertex_2P_2UV verts[6] = {
		0, 0, 0,	0,
		1, 0, uvw,	0,
		0, 1, 0,	uvh,
		1, 1, uvw,	uvh,
		0, 1, 0,	uvh,
		1, 0, uvw,	0
	};

	glBindVertexArray(quad->vao);
	glBindBuffer(GL_ARRAY_BUFFER, quad->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex_2P_2UV) * 6, verts, GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_2P_2UV), (GLvoid*)offsetof(Vertex_2P_2UV, position));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_2P_2UV), (GLvoid*)offsetof(Vertex_2P_2UV, uv));
}

void QuadDelete(Quad *quad) {
	glDeleteBuffers(1, &quad->vbo);
	glDeleteVertexArrays(1, &quad->vao);
}

void QuadRender(const Quad *quad) {
	glBindVertexArray(quad->vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}
