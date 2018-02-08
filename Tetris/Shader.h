#pragma once
#include <GL/glew.h>

GLuint current_shader_program;

/*
	Shader.h

	Anything to do with OpenGL shaders/programs goes here
*/

GLuint CreateShaderProgram(const GLchar* frag_src, const GLchar* vert_src);

inline void UseGLProgram(GLuint program) {
	glUseProgram(program);
	current_shader_program = program;
}

GLint ShaderGetLocation(GLuint shader, const char *name);

void ShaderSetUniformMat3(GLuint program, const char *name, const float **data);
void ShaderSetUniformVec3(GLuint program, const char *name, const float *data);
