#include "RNG.h"
#include <stdlib.h>

int RandomIntInRange(int min, int max) {
	return (rand() % (max - min)) + min;
}

void GenerateBag(byte *array, int bag_size) {
	for (int i = 0; i < bag_size; ++i)
		array[i] = i;

	int swap_index;
	int temp;
	for (int i = 0; i < bag_size; ++i) {
		swap_index = RandomIntInRange(i, bag_size);

		temp = array[i];
		array[i] = array[swap_index];
		array[swap_index] = temp;
	}
}
