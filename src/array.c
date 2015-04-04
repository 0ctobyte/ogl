#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "array.h"

struct array {
  void *data;
  size_t size;
  size_t capacity;
  size_t elem_size;
};

// Source: http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2	
// Rounds integer to next highest power of 2
uint64_t _array_next_pow2(uint64_t num) {
	num--;
	num |= num >> 1;
	num |= num >> 2;
	num |= num >> 4;
	num |= num >> 8;
	num |= num >> 16;
	return (num++);
}

void _array_resize(array_t *a, size_t capacity) {
  // Capacity must be greater than size or else we lose information
  assert(capacity >= a->size);

  // Allocate a new array
  a->capacity = capacity;
  a->data = (void*)realloc(a->data, a->elem_size*a->capacity);
  assert(a->data != NULL);

  // Clear the new memory
  memset((char*)a->data+(a->elem_size*a->size), 0, (a->capacity-a->size)*a->elem_size);
}

array_t* array_create(size_t capacity, size_t elem_size) {
  array_t *a = (array_t*)malloc(sizeof(array_t));
  assert(a != NULL);

  a->size = 0;

  // Round capacity to next power of 2
  a->capacity = _array_next_pow2(capacity);
  a->elem_size = elem_size;

  // Make space for the data (capacity*elem_size)
  a->data = (void*)malloc(a->elem_size*a->capacity);
  assert(a->data != NULL);

  // Clear the data
  memset((char*)a->data, 0, a->capacity*a->elem_size);

  return a;
}

void array_append(array_t *a, void *datum) {
  assert(a != NULL);

  // Time to resize the array
  if(a->size >= a->capacity) _array_resize(a, a->capacity<<1);

  // Append the datum
  memcpy((char*)a->data+(a->elem_size*a->size), datum, a->elem_size);
  a->size++;
}

void* array_at(array_t *a, uint32_t index) {
  // Check for out of bounds
  assert(a != NULL && index < a->size);

  return (void*)((char*)a->data+(a->elem_size*index));
}

void* array_back(array_t *a) {
  assert(a->size > 0);

  return (void*)((char*)a->data+(a->elem_size*(a->size-1)));
}

void array_set(array_t *a, uint32_t index, void *datum) {
  assert(a != NULL && index < a->size);

  memcpy((char*)a->data+(a->elem_size*index), datum, a->elem_size);
}

const void* array_data(array_t *a) {
  assert(a != NULL);
  return a->data;
}

size_t array_size(array_t *a) {
  assert(a != NULL);
  return a->size;
}

void array_copy(array_t *dst, array_t *src) {
  assert(dst->elem_size == src->elem_size);

  // Reset the dst array
  array_delete(dst);
  dst = array_create(src->size, src->elem_size);

  // Copy the data
  memcpy((char*)dst->data, (char*)src->data, src->size*src->elem_size);
  dst->size = src->size;
}

void array_clear(array_t *a) {
  memset(a->data, 0, a->size*a->elem_size);
  a->size = 0;
}

void array_delete(array_t *a) {
  if(a != NULL && a->data != NULL) free(a->data);
  if(a != NULL) free(a);
}

