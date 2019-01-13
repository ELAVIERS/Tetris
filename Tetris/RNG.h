#pragma once
#include "Types.h"

//Result will be >= min and < max.
int RandomIntInRange(int min, int max);

/*
	GenerateBag

	Outputs a shuffled array containing all integers 0...(bag_size-1)
*/
void GenerateBag(byte *array, int bag_size);
