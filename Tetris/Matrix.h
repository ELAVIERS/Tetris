#pragma once

/*
	Matrix.h

	Matrix operations for OpenGL
*/

typedef float Mat3[3][3];

void Mat3Multiply(Mat3 a, const Mat3 b);

void Mat3Identity(Mat3 out);
void Mat3Ortho(Mat3 out, unsigned int width, unsigned int height);

void Mat3Translate(Mat3 out, float x, float y);
void Mat3Scale(Mat3 out, float x, float y);
