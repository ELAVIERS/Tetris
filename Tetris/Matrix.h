#pragma once
#include "Types.h"

/*
	Matrix.h

	Matrix operations for OpenGL
*/

typedef float Mat3[3][3];

extern Mat3 g_mat3_identity;

inline void Mat3Copy(Mat3 dest, const Mat3 src) {
	for (int i = 0; i < 9; ++i)
		dest[0][i] = src[0][i];
}

void Mat3Multiply(Mat3 a, const Mat3 b);

void Mat3Identity(Mat3 out);
void Mat3Ortho(Mat3 out, unsigned int width, unsigned int height);

void Mat3Translate(Mat3 out, float x, float y);
void Mat3Scale(Mat3 out, float x, float y);

#define RC1D(S, R, C) ((R) * (S)) + (C)

inline void ByteMatrixTranspose(byte *data, unsigned int size) {
	for (unsigned int r = 0; r < size; ++r)
		for (unsigned int c = r; c < size; ++c) {
			byte temp = data[RC1D(size, r, c)];
			data[RC1D(size, r, c)] = data[RC1D(size, c, r)];
			data[RC1D(size, c, r)] = temp;
		}
}

inline void ByteMatrixFlipRows(byte *data, unsigned int size) {
	for (unsigned int r = 0; r < size; ++r)
		for (unsigned int c = 0; c < size / 2; ++c) {
			byte temp = data[RC1D(size, r, c)];
			data[RC1D(size, r, c)] = data[RC1D(size, r, size - 1 - c)];
			data[RC1D(size, r, size - 1 - c)] = temp;
		}
}

inline void ByteMatrixFlipColumns(byte *data, unsigned int size) {
	for (unsigned int c = 0; c < size; ++c)
		for (unsigned int r = 0; r < size / 2; ++r) {
			byte temp = data[RC1D(size, r, c)];
			data[RC1D(size, r, c)] = data[RC1D(size, size - 1 - r, c)];
			data[RC1D(size, size - 1 - r, c)] = temp;
		}
}
