#ifndef __MESH_H__
#define __MESH_H__

#include <stdint.h>
#include <stdbool.h>

#include <SDL2/SDL.h>

#include "array.h"

typedef struct {
  // Handle to the texture unit
  uint32_t texID;

  // The actual texture data
  SDL_Surface *texture;

  // True if texture is bound
  int32_t use_texture;
} texture_t;

typedef struct {
  float diffuse[3];
  float ambient[3];
  float specular[3];

  // This is essentially the specular exponent
  float shininess;
  float transparency;

  // The texture data if needed
  texture_t tex;
} material_t;

// A group of faces using the same material
typedef struct {
  // Handle to the OpenGL index buffer object
  uint32_t ibo;

  // The material used for this group of faces
  material_t mtl;

  // List of indices in this face group
  array_t *indices;
} material_group_t;

typedef struct {
  // Handle to the OpenGL vertex array object
  uint32_t vao;

  // Handle to the OpenGL vertex buffer object (vertex attribute data)
  uint32_t vbo;
  
  // List of vertex attributes (position, texture, normals)
  array_t *vattributes;

  // List of face groups
  array_t *mtl_grps;
  size_t num_faces;
} mesh_t;

bool mesh_load(mesh_t *mesh, const char *objfile);
void mesh_bind(mesh_t *mesh);
void mesh_unbind();
void mesh_delete(mesh_t *mesh);

#endif // __MESH_H__

