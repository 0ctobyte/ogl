#ifndef __ARRAY_H__
#define __ARRAY_H__

#include <stdint.h>

typedef struct array array_t;

array_t* array_create(size_t capacity, size_t elem_size);
void array_append(array_t *a, void *datum);
void* array_at(array_t *a, uint32_t index);
void* array_data(array_t *a);
size_t array_size(array_t *a);
void array_delete(array_t *a);

#endif //__ARRAY_H__

