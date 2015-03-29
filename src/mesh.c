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

  // Grab the vertex attribute data and place them in separate arrays
  array_t *positions = array_create(256, sizeof(vec3_t));
  array_t *uv = array_create(256, sizeof(vec3_t));
  array_t *normals = array_create(256, sizeof(vec3_t));
  char line[256];
  while(fgets(line, 256, f) != NULL) {
    if(line[0] == 'v') {
      switch(line[1]) {
      case 'n':
        {
          // Parse the vertex normals and normalize them
          vec3_t v = {0.0f, 0.0f, 0.0f};
          char *pch = strtok(&line[2], " ");
          for(uint32_t i = 0; pch != NULL; ++i) {
            *(((GLfloat*)(&v))+i) = strtof(pch, NULL);
            pch = strtok(NULL, " ");
          }
          v = vec3_normalize(&v);
          array_append(normals, &v);
          break;
        }
      case 't':
        {
          // Parse the texture coordinates
          vec3_t v = {0.0f, 0.0f, 0.0f};
          char *pch = strtok(&line[2], " ");
          for(uint32_t i = 0; pch != NULL; ++i) {
            *(((GLfloat*)(&v))+i) = strtof(pch, NULL);
            pch = strtok(NULL, " ");
          }
          array_append(uv, &v);
          break;
        }
      default:
        {
          // Parse the vertex position coordinates
          vec3_t v = {0.0f, 0.0f, 0.0f};
          char *pch = strtok(&line[1], " ");
          for(uint32_t i = 0; pch != NULL; ++i) {
            *(((GLfloat*)(&v))+i) = strtof(pch, NULL);
            pch = strtok(NULL, " ");
          }
          array_append(positions, &v);
          break;
        }
      }
    }
  }

  // Seek to the beginning of the file
  fseek(f, 0, SEEK_SET);

  // Fill the vertex attribute array with 0 vectors
  size_t num_attribs = 3*array_size(positions);
  mesh->vertices = array_create(num_attribs, sizeof(vec3_t));
  vec3_t z = {0.0f, 0.0f, 0.0f};
  for(uint32_t i = 0; i < num_attribs; ++i) {
    array_append(mesh->vertices, &z);
  }

  // Parse the indices as they are read in and place the vertex attributes in the correct index
  // in the vertex attribute array
  mesh->indices = array_create(256, sizeof(GLuint));
  while(fgets(line, 256, f) != NULL) {
    if(line[0] == 'f') {
      char *pch = strtok(&line[1], " ");
      while(pch != NULL) {
        char *next;
        // Indices start from 1 in the Wavefront OBJ format
        // First parse the vertex position index
        uint32_t index = (uint32_t)strtoul(pch, &next, 10)-1;
        array_append(mesh->indices, &index);
        
        // Now append the vertex position at that index into the interleaved vertex attribute array
        vec3_t *v = array_at(positions, index);
        array_set(mesh->vertices, index*3, v);

        if(next[0] == '/') {
          // Attempt to parse the texture index, check to make sure a texture index exists
          uint32_t t_index = (uint32_t)strtoul(next+1, &next, 10);
          if(t_index != 0) {
            // Now append the vertex texture coordinate into the interleaved vertex attribute array
            t_index--;
            v = array_at(uv, t_index);
            array_set(mesh->vertices, index*3+1, v);
          } else {
            next++;
          }

          // Attempt to parse the normal index, check to make sure a normal index exists
          uint32_t n_index = (uint32_t)strtoul(next, NULL, 10);
          if(n_index != 0) {
            // Now append the vertex normal attribute into the interleaved vertex attribute array
            n_index--;
            v = array_at(normals, n_index);
            array_set(mesh->vertices, index*3+2, v);
          }
        }

        pch = strtok(NULL, " ");
      }
    }
  }

  // Cleanup temp arrays
  array_delete(positions);
  array_delete(uv);
  array_delete(normals);

  // Check for any errors
  if(ferror(f) != 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error reading file: %s\n", objfile);
    mesh_delete(mesh);
    return false;
  }

  // Generate and fill the OpenGL buffers
  _mesh_gen_buffers(mesh);

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

