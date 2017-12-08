#pragma once
#include <GL/glew.h>

/*
	Shader.h

	Anything to do with OpenGL shaders/programs goes here
*/

GLuint CreateShaderProgram(const GLchar* frag_src, const GLchar* vert_src);
