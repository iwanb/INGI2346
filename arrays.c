#include "arrays.h"

int is_in(int value, int * array, int size) {
	int i = 0;
	while (i < size) {
		if (value == *(array + i)) {
			return 1;
		}
		i++;
	}
	return 0;
}

int is_at(int value, int * array, int size) {
	int i = 0;
	while (i < size) {
		if (value == *(array + i)) {
			return i;
		}
		i++;
	}
	return i;
}

void insert(int value, int *array, int size) {
	int i = 0;
	int end = size;
	while (i < size && value > *(array + i)) {
		i++;
	}
	while (end > i) {
		*(array + end) = *(array + end - 1);
		end--;
	}
	*(array + i) = value;
}

int maximum(int * array, int size) {
	int max = INT_MIN, i = 0;
	while (i < size) {
		if (max < *(array + i)) {
			max = *(array + i);
		}
		i++;
	}
	return max;
}

void remove_from(int position, int *array, int size) {
	while (position < size-1) {
		*(array+position) = *(array+position+1);
		position++;
	}
}
