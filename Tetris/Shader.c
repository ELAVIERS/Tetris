#include "Shader.h"

#include "Error.h"
#include <stdlib.h>
#include <string.h>

#define ERROR_LOG_MAX 4096

int CompileShader(GLuint id, const GLchar* src) {
	glShaderSource(id, 1, &src, NULL);
	glCompileShader(id);

	GLuint value;
	glGetShaderiv(id, GL_COMPILE_STATUS, &value);
	if (value == 0) {
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &value);
		GLchar* error_str = (GLchar*)malloc(value);

		glGetShaderInfoLog(id, value, &value, error_str);

		char full_error[ERROR_LOG_MAX] = "Shader compiler error\n-----\n";
		strcat_s(full_error, ERROR_LOG_MAX, error_str);
		ErrorMessage(full_error);

		glDeleteShader(id);
		return 0;
	}

	return 1;
}

GLuint CreateShaderProgram(const GLchar* frag_src, const GLchar* vert_src) {
	GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint vert = glCreateShader(GL_VERTEX_SHADER);

	CompileShader(frag, frag_src);
	CompileShader(vert, vert_src);

	GLuint program = glCreateProgram();
	glAttachShader(program, frag);
	glAttachShader(program, vert);
	glLinkProgram(program);

	GLuint value;
	glGetProgramiv(program, GL_LINK_STATUS, &value);
	if (value == 0) {
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &value);
		GLchar* error_str = (GLchar*)malloc(value);

		glGetProgramInfoLog(program, value, &value, error_str);

		char full_error[ERROR_LOG_MAX] = "Program link error\n-----\n";
		strcat_s(full_error, ERROR_LOG_MAX, error_str);
		ErrorMessage(full_error);

		glDeleteShader(frag);
		glDeleteShader(vert);
		glDeleteProgram(program);
		return 0;
	}

	glDeleteShader(frag);
	glDeleteShader(vert);

	return program;
}
