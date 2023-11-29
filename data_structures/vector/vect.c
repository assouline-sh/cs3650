/**
 * Vector implementation.
 *
 * - Implement each of the functions to create a working growable array (vector).
 * - Do not change any of the structs
 * - When submitting, You should not have any 'printf' statements in your vector 
 *   functions.
 *
 * IMPORTANT: The initial capacity and the vector's growth factor should be 
 * expressed in terms of the configuration constants in vect.h
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "vect.h"

/** Main data structure for the vector. */
struct vect {
  char **data;             /* Array containing the actual data. */
  unsigned int size;       /* Number of items currently in the vector. */
  unsigned int capacity;   /* Maximum number of items the vector can hold before growing. */
};

/** Construct a new empty vector. */
vect_t *vect_new() {
  // Initialize all the components of a vector and allocate memory for vector and data
  vect_t *v = malloc(sizeof(vect_t));
  v->data = malloc(VECT_INITIAL_CAPACITY * sizeof(char *));
  v->size = 0;
  v->capacity = VECT_INITIAL_CAPACITY; 
  return v;
}

/** Delete the vector, freeing all memory it occupies. */
void vect_delete(vect_t *v) {
  // Free all individual data entries, and the array and vector
  for (int i = 0; i < v->size; i++) {
	free(v->data[i]);
  }
  free(v->data);
  free(v);
}

/** Get the element at the given index. */
const char *vect_get(vect_t *v, unsigned int idx) {
  // Return the element at the given index
  assert(v != NULL);
  assert(idx < v->size);
  return (v->data[idx]);
}

/** Get a copy of the element at the given index. The caller is responsible
 *  for freeing the memory occupied by the copy. */
char *vect_get_copy(vect_t *v, unsigned int idx) {
  // Create a copy of the element at the given index, and return the copy
  assert(v != NULL);
  assert(idx < v->size);
  char *copy = malloc(strlen(v->data[idx]) + 1);
  *copy = v->data[idx];
  return copy;
}

/** Set the element at the given index. */
void vect_set(vect_t *v, unsigned int idx, const char *elt) {
  // Free the original data at the given index, then allocate memory for the new element and copy
  // it to the given index in data
  assert(v != NULL);
  assert(idx < v->size);
  free(v->data[idx]);
  v->data[idx] = malloc(strlen(elt) + 1);
  strcpy(v->data[idx], elt);
}

/** Add an element to the back of the vector. */
void vect_add(vect_t *v, const char *elt) {
  assert(v != NULL);
  // If the size is greater than or equal to the capacity, increase the capacity and reallocate 
  // memory for data to be expanded
  if (v->size >= v->capacity) {
	v->capacity = v->capacity * VECT_GROWTH_FACTOR; 
	v->data = realloc(v->data, v->capacity * sizeof(char *));
  }
  // Allocate space for a copy of the given element, copy the element to this variable, and assign
  // the copy of the given element to be at the back of the vector
  char *copy = malloc((strlen(elt) + 1) * sizeof(char));
  strcpy(copy, elt);
  v->data[v->size] = copy;
  v->size = v->size + 1;	
}

/** Remove the last element from the vector. */
void vect_remove_last(vect_t *v) {
  // Decrement the size and free the data that is at the end of the vector
  assert(v != NULL);
  v->size = v->size - 1;
  free(v->data[v->size]);
}

/** The number of items currently in the vector. */
unsigned int vect_size(vect_t *v) {
  // Return the size
  assert(v != NULL);
  return(v->size);
}

/** The maximum number of items the vector can hold before it has to grow. */
unsigned int vect_current_capacity(vect_t *v) {
  // Return the capacity
  assert(v != NULL);
  return (v->capacity);
}

