#ifndef __MESH_H__
#define __MESH_H__

#include <stdint.h>

#include "vec.h"

typedef struct {
  vec3_t *vertices;
  uint32_t *indices;
  size_t num_vertices;
  size_t num_indices;
  uint32_t vbo;
  uint32_t ibo;
} mesh_t;

void mesh_load(mesh_t *mesh, const char *objfile);

#endif // __MESH_H__

