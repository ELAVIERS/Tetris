#pragma once
float** Mat3Alloc();
void Mat3Free(float **p);
void Mat3Multiply(float **a, const float **b);

void Mat3Ortho(float **out, unsigned int width, unsigned int height);
