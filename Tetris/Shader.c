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

#include "Console.h"

GLint ShaderGetLocation(GLuint program, const char *name) {
	GLint location = glGetUniformLocation(program, name);
	if (location < 0) {
		ConsolePrint("Uniform shader variable \"");
		ConsolePrint(name);
		ConsolePrint("\" not found!\n");
	}

	return location;
}

void ShaderSetUniformBool(GLuint program, const char *name, bool x) {
	glUniform1i(ShaderGetLocation(program, name), x ? 1 : 0);
}

void ShaderSetUniformMat3(GLuint program, const char *name, const Mat3 data) {
	glUniformMatrix3fv(ShaderGetLocation(program, name), 1, GL_FALSE, &data[0][0]);
}

void ShaderSetUniformVec3(GLuint program, const char *name, const float data[3]) {
	glUniform3f(ShaderGetLocation(program, name), data[0], data[1], data[2]);
}

void ShaderSetUniformFloat(GLuint program, const char *name, float x) {
	glUniform1f(ShaderGetLocation(program, name), x);
}

void ShaderSetUniformFloat2(GLuint program, const char *name, float x, float y) {
	glUniform2f(ShaderGetLocation(program, name), x, y);
}
