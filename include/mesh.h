#ifndef __MESH_H__
#define __MESH_H__

#include <stdbool.h>

#include <SDL2/SDL_Surface.h>

#include "gl_core_4_1.h"
#include "array.h"

typedef struct {
  // Handle to the texture unit
  GLuint texID;

  // True if texture is bound
  GLuint use_texture;
} texture_t;

typedef struct {
  GLfloat diffuse[3];
  GLfloat ambient[3];
  GLfloat specular[3];

  // This is essentially the specular exponent
  GLfloat shininess;
  GLfloat transparency;

  // The texture data if needed
  texture_t tex;
} material_t;

// A group of faces using the same material
typedef struct {
  // The material used for this group of faces
  material_t mtl;

  // The offset into the index list and the # of indices used by the material group
  GLuint offset;
  GLuint count;
} material_group_t;

typedef struct {
  // Handle to the OpenGL vertex array object
  GLuint vao;

  // Handle to the OpenGL vertex buffer object (vertex attribute data)
  GLuint vbo;
  
  // List of vertex attributes (position, texture, normals)
  array_t *vattributes;

  // Handle to the OpenGL index buffer object
  GLuint ibo;

  // List of indices into the vertex attribute array
  array_t *indices;

  // List of face groups
  array_t *mtl_grps;
  size_t num_faces;
} mesh_t;

bool mesh_load(mesh_t *mesh, const char *objfile);
void mesh_bind(mesh_t *mesh);
void mesh_unbind();
void mesh_delete(mesh_t *mesh);

#endif // __MESH_H__

