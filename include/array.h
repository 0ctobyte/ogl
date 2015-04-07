#ifndef __ARRAY_H__
#define __ARRAY_H__

#include <stdint.h>

typedef struct array array_t;

array_t* array_create(size_t capacity, size_t elem_size);
void array_append(array_t *a, void *datum);
void* array_at(array_t *a, uint64_t index);
void* array_back(array_t *a);
void array_set(array_t *a, uint64_t index, void *datum);
const void* array_data(array_t *a);
size_t array_size(array_t *a);
void array_copy(array_t *dst, array_t *src);
void array_cat(array_t *dst, array_t *src);
void array_cat_str(array_t *dst, const char *str);
void array_prepend_str(array_t *dst, const char *str);
void array_clear(array_t *a);
void array_delete(array_t *a);

#endif //__ARRAY_H__

