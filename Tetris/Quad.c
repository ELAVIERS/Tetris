#include "Quad.h"
#include "Vertex.h"
#include <GL/glew.h>
#include <stdlib.h>

GLuint vao = 0;
GLuint vbo = 0;

void QuadInit(float uvx, float uvy) {
	Vertex_2P_2UV verts[6] = {
		0, 0, 0,	0,
		1, 0, uvx,	0,
		0, 1, 0,	uvy,
		1, 1, uvx,	uvy,
		0, 1, 0,	uvy,
		1, 0, uvx,	0
	};

	if (!vao) {
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
	}

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex_2P_2UV) * 6, verts, GL_STATIC_DRAW);
	
	//Todo : if vao do these	V
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_2P_2UV), (GLvoid*)offsetof(Vertex_2P_2UV, position));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_2P_2UV), (GLvoid*)offsetof(Vertex_2P_2UV, uv));

}

void QuadDelete() {
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
}

void QuadRender() {
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}
