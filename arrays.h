#include <limits.h>

/**
 * @return true if 'value' is in array
 */
int is_in(int value, int * array, int size);

/**
 * @return the maximum value in array
 */
int maximum(int * array, int size);

/**
 * @return the position of (first) element with value 'value' in array
 * 			or 'size' if the element was not found
 */
int is_at(int value, int * array, int size);

/**
 * Inserts integer 'value' into sorted array (at correct position)
 * 'size' is the current size of the array, 
 * the array must be able to hold 'size'+1 elements
 * After the call its new size is 'size' + 1
 */
void insert(int value, int *array, int size);

void remove_from(int position, int *array, int size);
