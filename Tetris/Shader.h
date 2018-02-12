#pragma once
#include "Globals.h"
#include "Matrix.h"
#include <GL/glew.h>

/*
	Shader.h

	Anything to do with OpenGL shaders/programs goes here
*/

GLuint CreateShaderProgram(const GLchar* frag_src, const GLchar* vert_src);

inline void UseGLProgram(GLuint program) {
	glUseProgram(program);
	g_active_shader = program;
}

GLint ShaderGetLocation(GLuint shader, const char *name);

void ShaderSetUniformMat3(GLuint program, const char *name, const Mat3 data);
void ShaderSetUniformVec3(GLuint program, const char *name, const float *data);
