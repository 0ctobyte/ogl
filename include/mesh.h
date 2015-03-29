#ifndef __MESH_H__
#define __MESH_H__

#include <stdint.h>

#include "array.h"

#define MESH_NUM_BUFFERS (3)
#define MESH_VBO (0)
#define MESH_IBO (1)

typedef struct {
  array_t *vertices;
  array_t *indices;
  uint32_t vao;
  uint32_t buf_ids[MESH_NUM_BUFFERS];
} mesh_t;

void mesh_load(mesh_t *mesh, const char *objfile);
void mesh_bind(mesh_t *mesh);
void mesh_unbind();
void mesh_delete(mesh_t *mesh);

#endif // __MESH_H__

