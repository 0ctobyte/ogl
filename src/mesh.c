#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <SDL2/SDL_log.h>
#include <OpenGL/gl3.h>

#include "mesh.h"
#include "vec.h"

void _mesh_gen_buffers(mesh_t *mesh) {
  // Generate the name for the vertex array object (VAO)
  glGenVertexArrays(1, &mesh->vao);
  glBindVertexArray(mesh->vao);

  // Create names for the vertex buffer object (VBO) and index buffer object (IBO)
  glGenBuffers(MESH_NUM_BUFFERS, mesh->buf_ids);

  // Copy the vertex data into the vertex buffer
  glBindBuffer(GL_ARRAY_BUFFER, mesh->buf_ids[MESH_VBO]);
  glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(array_size(mesh->vertices)*sizeof(vec3_t)), array_data(mesh->vertices), GL_STATIC_DRAW);

  // Copy the index data into the index buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->buf_ids[MESH_IBO]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(array_size(mesh->indices)*sizeof(uint32_t)), array_data(mesh->indices), GL_STATIC_DRAW);

  // Unbind everything
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

bool mesh_load(mesh_t *mesh, const char *objfile) {
  FILE *f = fopen(objfile, "r");

  // Make sure the file was opened
  if(f == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not open Wavefront OBJ file: %s\n", objfile);
    return false;
  }

  // Append the vertices and indices as they are read in
  mesh->vertices = array_create(256, sizeof(vec3_t));
  mesh->indices = array_create(256, sizeof(GLuint));
  char line[256];
  while(fgets(line, 256, f) != NULL) {
    switch(line[0]) {
      case 'v':
      {
        vec3_t v;
        char *pch = strtok(&line[1], " ");
        for(uint32_t i = 0; pch != NULL; ++i) {
          *(((GLfloat*)(&v))+i) = strtof(pch, NULL);
          pch = strtok(NULL, " ");
        }
        char s[256];
        vec3_str(&v, s);
        array_append(mesh->vertices, &v);
        break;
      }
      case 'f':
      {
        char *pch = strtok(&line[1], " ");
        while(pch != NULL) {
          // Indices start from 1 in the Wavefront OBJ format
          uint32_t index = (uint32_t)strtoul(pch, NULL, 10)-1;
          array_append(mesh->indices, &index);
          pch = strtok(NULL, " ");
        }
        break;
      }
      default:
        break;
    }
  }

  // Check for any errors
  if(ferror(f) != 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error reading file: %s\n", objfile);
    mesh_delete(mesh);
    return false;
  }

  // Generate and fill the OpenGL buffers
  _mesh_gen_buffers(mesh);

  /*char s[256];
  size_t size = array_size(mesh->vertices);
  vec3_t *data = (vec3_t*)array_data(mesh->vertices);
  for(uint32_t i = 0; i < size; i++) {
    vec3_str(&data[i], s);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s\n", s);
  }
  
  size = array_size(mesh->indices);
  uint32_t *dataf = (uint32_t*)array_data(mesh->indices);
  for(uint32_t i = 0; i < size; i += 3) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "f %u %u %u\n", dataf[i], dataf[i+1], dataf[i+2]);
  }*/

  return true;
}

void mesh_bind(mesh_t *mesh) {
  glBindVertexArray(mesh->vao);
  glBindBuffer(GL_ARRAY_BUFFER, mesh->buf_ids[MESH_VBO]);
}

void mesh_unbind() {
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void mesh_delete(mesh_t *mesh) {
  // Delete the buffers
  glDeleteBuffers(MESH_NUM_BUFFERS, mesh->buf_ids);

  // Delete the arrays
  array_delete(mesh->vertices);
  array_delete(mesh->indices);
}

