#include "Matrix.h"
#include <stdlib.h>

float** Mat3Alloc() {
	float **matrix = (float**)malloc(3 * sizeof(float*));
	matrix[0] = (float*)calloc(9, sizeof(float));
	matrix[1] = &matrix[0][3];
	matrix[2] = &matrix[1][3];
	return matrix;
}

void Mat3Free(float **matrix) {
	free(matrix[0]);
	free(matrix);
}

#define MATDOT(A, B, R, C) A[R][0] * B[0][C] + A[R][1] * B[1][C] + A[R][2] * B[2][C]

void Mat3Multiply(Mat3 a, const Mat3 b) {
	a[0][0] = MATDOT(a, b, 0, 0);
	a[0][1] = MATDOT(a, b, 0, 1);
	a[0][2] = MATDOT(a, b, 0, 2);
	a[1][0] = MATDOT(a, b, 1, 0);
	a[1][1] = MATDOT(a, b, 1, 1);
	a[1][2] = MATDOT(a, b, 1, 2);
	a[2][0] = MATDOT(a, b, 2, 0);
	a[2][1] = MATDOT(a, b, 2, 1);
	a[2][2] = MATDOT(a, b, 2, 2);
}

void Mat3Identity(Mat3 out) {
	out[0][1] = out[0][2] = out[1][0] = out[1][2] = out[2][0] = out[2][1] = 0.f;
	out[0][0] = out[1][1] = out[2][2] = 1.f;
}

void Mat3Ortho(Mat3 out, unsigned int width, unsigned int height) {
	out[0][0] = 2.f / width;
	out[1][1] = 2.f / height;
	out[2][2] = 1.f;
	out[2][0] = -1.f;
	out[2][1] = -1.f;
}

void Mat3Translate(Mat3 out, float x, float y) {
	Mat3 translation = {
		1,0,0,
		0,1,0,
		x,y,1
	};

	Mat3Multiply(out, translation);
}

void Mat3Scale(Mat3 out, float x, float y) {
	Mat3 scale = {
		x,0,0,
		0,y,0,
		0,0,1
	};

	Mat3Multiply(out, scale);
}
