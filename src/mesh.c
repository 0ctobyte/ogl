#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_log.h>
#include <OpenGL/gl3.h>

#include "mesh.h"
#include "vec.h"

void _mesh_gen_buffers(mesh_t *mesh) {
  // Generate the name for the vertex array object (VAO)
  glGenVertexArrays(1, &mesh->vao);
  glBindVertexArray(mesh->vao);

  // Create names for the vertex buffer object (VBO)
  glGenBuffers(1, &mesh->vbo);

  // Copy the vertex data into the vertex buffer
  glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
  glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(array_size(mesh->vertices)*sizeof(vec3_t)), array_data(mesh->vertices), GL_STATIC_DRAW);

  // Unbind everything
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void _mesh_gen_index_buffers(mesh_t *mesh) {
  glBindVertexArray(mesh->vao);

  // Generate index buffer for each face and copy data to the GPU
  size_t size = array_size(mesh->faces);
  for(uint32_t i = 0; i < size; i++) {
    face_group_t *face = (face_group_t*)array_at(mesh->faces, i);
    glGenBuffers(1, &face->ibo);

    // Copy the index data into the index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, face->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(array_size(face->indices)*sizeof(uint32_t)), array_data(face->indices), GL_STATIC_DRAW);
  }

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void _mesh_create_face_group(mesh_t *mesh) {
  // Create a new face group
  face_group_t face;

  // Create the index array
  face.indices = array_create(2, sizeof(uint32_t));

  // Default values for the material in case none exist
  face.mtl.diffuse = (vec3_t){0.75f, 0.75f, 0.75f};
  face.mtl.ambient = (vec3_t){0.0f, 0.0f, 0.0f};
  face.mtl.specular = (vec3_t){1.0f, 1.0f, 1.0f};
  face.mtl.shininess = 80.0f;
  face.mtl.transparency = 1.0f;
  face.mtl.tex.texID = 0;
  face.mtl.tex.texture = NULL;

  // Add the face to the mesh
  array_append(mesh->faces, &face);
}

void _mesh_load_texture(mesh_t *mesh, const char *tex_filename) {
  // Get the latest face group
  face_group_t *faces = (face_group_t*)array_at(mesh->faces, (uint32_t)array_size(mesh->faces)-1);

  // Load the BMP
  faces->mtl.tex.texture = SDL_LoadBMP(tex_filename);

  // Make sure the file exists and was loaded properly
  if(faces->mtl.tex.texture == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error reading tex file\n");
    return;
  }

  // Get the OpenGL format
  uint32_t internal_format = 0;
  if(faces->mtl.tex.texture->format->BytesPerPixel == 3) {
    faces->mtl.tex.format = (faces->mtl.tex.texture->format->Rmask == 0x000000ff) ? GL_RGB : GL_BGR;
    internal_format = GL_RGB;
  } else if(faces->mtl.tex.texture->format->BytesPerPixel == 4) {
    faces->mtl.tex.format = (faces->mtl.tex.texture->format->Rmask == 0x000000ff) ? GL_RGBA : GL_BGRA;
    internal_format = GL_RGBA;
  } else {
    SDL_FreeSurface(faces->mtl.tex.texture);
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unsupported pixel format for texture: %s\n", tex_filename);
    return;
  }
  
  glBindVertexArray(mesh->vao);

  // Generate the texture handle
  glGenTextures(1, &faces->mtl.tex.texID);
  glBindTexture(GL_TEXTURE_2D, faces->mtl.tex.texID);

  // Set the texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Upload the texture pixel data
  glTexImage2D(GL_TEXTURE_2D, 0, (GLint)internal_format, (GLsizei)faces->mtl.tex.texture->w, (GLsizei)faces->mtl.tex.texture->h, 0, faces->mtl.tex.format, GL_UNSIGNED_BYTE, faces->mtl.tex.texture->pixels);

  // Unbind the texture
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindVertexArray(0);
}

void _mesh_load_mtl(mesh_t *mesh, FILE *mtl_file, const char *mtl_name) {
  // Make sure the mtl_file exists
  if(mtl_file == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error reading mtl file\n");
    return;
  }

  // Find the material name in the material library
  char line[256];
  while(fgets(line, 256, mtl_file) != NULL) {
    char *pch = strtok(line, " ");
    if(pch != NULL && strcmp(pch, "newmtl") != 0) continue;
    pch = strtok(NULL, " ");
    if(pch == NULL) continue;
    pch[strlen(pch)-1] = '\0';
    if(strcmp(pch, mtl_name) == 0) break;
  }

  // Check for any errors or if EOF has been reached. This means the material could not be found in the material library
  if(ferror(mtl_file) != 0 || feof(mtl_file) != 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not find specified material: \'%s\' in the mtllib\n", mtl_name);
    fseek(mtl_file, 0, SEEK_SET);
    return;
  }

  // Start reading the material data
  face_group_t *face = (face_group_t*)array_at(mesh->faces, (uint32_t)array_size(mesh->faces)-1);
  while(fgets(line, 256, mtl_file) != NULL) {
    char *pch = strtok(line, " ");

    // Break if we have reached the end of the material definition (i.e. a new material definition line has been reached)
    if(strcmp(pch, "newmtl") == 0) break;

    if(strcmp(pch, "Ns") == 0) {
      // The specular shininess
      pch = strtok(NULL, " ");
      if(pch == NULL) continue;
      face->mtl.shininess = strtof(pch, NULL);
    } else if(strcmp(pch, "d") == 0) {
      // The alpha transparency
      pch = strtok(NULL, " ");
      if(pch == NULL) continue;
      face->mtl.transparency = strtof(pch, NULL);
    } else if(strcmp(pch, "Ka") == 0) {
      // The material ambient color
      vec3_t v = {0.0f, 0.0f, 0.0f};
      pch = strtok(NULL, " ");
      for(uint32_t i = 0; pch != NULL; ++i) {
        *(((GLfloat*)(&v))+i) = strtof(pch, NULL);
        pch = strtok(NULL, " ");
      }
      face->mtl.ambient = v;
    } else if(strcmp(pch, "Kd") == 0) {
      // The material diffuse color
      vec3_t v = {0.0f, 0.0f, 0.0f};
      pch = strtok(NULL, " ");
      for(uint32_t i = 0; pch != NULL; ++i) {
        *(((GLfloat*)(&v))+i) = strtof(pch, NULL);
        pch = strtok(NULL, " ");
      }
      face->mtl.diffuse = v;
    } else if(strcmp(pch, "Ks") == 0) {
      // The material specular color
      vec3_t v = {0.0f, 0.0f, 0.0f};
      pch = strtok(NULL, " ");
      for(uint32_t i = 0; pch != NULL; ++i) {
        *(((GLfloat*)(&v))+i) = strtof(pch, NULL);
        pch = strtok(NULL, " ");
      }
      face->mtl.specular = v;
    } else if(strcmp(pch, "map_Kd") == 0) {
      pch = strtok(NULL, " ");
      if(pch == NULL) continue;
      char s[256];
      pch[strlen(pch)-1] = '\0';
      snprintf(s, 256, "resources/%s", pch);
      _mesh_load_texture(mesh, s);
    }
  }

  // Check for any errors 
  if(ferror(mtl_file) != 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error reading mtllib\n");
  }

  // Rewind the file cursor
  fseek(mtl_file, 0, SEEK_SET);
}

bool mesh_load(mesh_t *mesh, const char *objfile) {
  FILE *mtl_file = NULL, *f = fopen(objfile, "r");

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

    // Open the mtl file if one exists
    if(line[0] == 'm') {
      char *pch = strtok(line, " ");
      if(strcmp(pch, "mtllib") != 0) continue;
      pch = strtok(NULL, " ");
      if(pch != NULL) {
        // TODO: not ideal!
        char s[256];
        pch[strlen(pch)-1] = '\0';
        snprintf(s, 256, "resources/%s", pch);
        mtl_file = fopen(s, "r");
      }
    }
  }

  // Seek to the beginning of the file
  fseek(f, 0, SEEK_SET);

  // Fill the vertex attribute array with 0 vectors
  size_t num_attribs = 3*array_size(positions);
  mesh->vertices = array_create(num_attribs, sizeof(vec3_t));
  vec3_t z = {0.0f, 0.0f, 0.0f};
  for(uint32_t i = 0; i < num_attribs; i+=3) {
    vec3_t u = {0.0f, 0.0f, -1.0f};
    array_append(mesh->vertices, &z);
    array_append(mesh->vertices, &u);
    array_append(mesh->vertices, &z);
  }

  // Parse the indices as they are read in and place the vertex attributes in the correct index in the vertex attribute array
  mesh->faces = array_create(2, sizeof(face_group_t));
  while(fgets(line, 256, f) != NULL) {
    switch(line[0]) {
    case 'f':
      {
        // Create a face if none exist
        // We might have obj files with only one group of faces and no materials 
        if(array_size(mesh->faces) == 0) _mesh_create_face_group(mesh);

        char *pch = strtok(&line[1], " ");
        while(pch != NULL) {
          // Add the indices to the last face in the list
          face_group_t *face = (face_group_t*)array_at(mesh->faces, (uint32_t)array_size(mesh->faces)-1);

          // Indices start from 1 in the Wavefront OBJ format
          // First parse the vertex position index
          char *next;
          uint32_t index = (uint32_t)strtoul(pch, &next, 10)-1;
          
          // Now append the vertex position at that index into the interleaved vertex attribute array
          vec3_t *v = array_at(positions, index);
          array_set(mesh->vertices, index*3, v);

          if(next[0] == '/') {
            // Attempt to parse the texture index, check to make sure a texture index exists
            uint32_t t_index = (uint32_t)strtoul(next+1, &next, 10);
            if(t_index != 0) {
              t_index--;
              v = array_at(uv, t_index);

              // Check if the vertex needs to be duplicated, if the texture coordinates are different for the same vertex
              vec3_t *u = array_at(mesh->vertices, index*3+1);

              if(!(u->z < 0.0) && (u->x != v->x || u->y != v->y)) {
                // Duplicate the vertex and update the index
                vec3_t *p = array_at(mesh->vertices, index*3);
                vec3_t *n = array_at(mesh->vertices, index*3+2);
                array_append(mesh->vertices, p);
                index = ((uint32_t)array_size(mesh->vertices)-1)/3;
                array_append(mesh->vertices, v);
                array_append(mesh->vertices, n);
              } else {
                // Now append the vertex texture coordinate into the interleaved vertex attribute array
                array_set(mesh->vertices, index*3+1, v);
              }
            } 

            // Attempt to parse the normal index, check to make sure a normal index exists
            uint32_t n_index = (uint32_t)strtoul(next+1, NULL, 10);
            if(n_index != 0) {
              // Now append the vertex normal attribute into the interleaved vertex attribute array
              n_index--;
              v = array_at(normals, n_index);
              array_set(mesh->vertices, index*3+2, v);
            }
          }

          // Append the index into the index array
          array_append(face->indices, &index);
          pch = strtok(NULL, " ");
        }
      }
    case 'u':
      {
        char *pch = strtok(line, " ");
        if(pch != NULL && strcmp(pch, "usemtl") != 0) continue;
       
        // Start a new face group using this material
        _mesh_create_face_group(mesh);
        
        // Load the material data for the face group
        pch = strtok(NULL, " ");
        if(pch == NULL) continue;
        pch[strlen(pch)-1] = '\0';
        _mesh_load_mtl(mesh, mtl_file, pch);
        break;
      }
    }
  }

  // Close files
  fclose(f);
  fclose(mtl_file);

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
  _mesh_gen_index_buffers(mesh);

  return true;
}

void mesh_bind(mesh_t *mesh) {
  glBindVertexArray(mesh->vao);
  glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
}

void mesh_unbind() {
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindVertexArray(0);
}

void mesh_delete(mesh_t *mesh) {
  // Delete the vertex buffer object
  glDeleteBuffers(1, &mesh->vbo);

  // Delete the vertex attribute array
  array_delete(mesh->vertices);
 
  // Delete all the index arrays
  size_t size = array_size(mesh->faces);
  for(uint32_t i = 0; i < size; i++) {
    // Delete the index array
    face_group_t *face = (face_group_t*)array_at(mesh->faces, i);
    array_delete(face->indices);

    // Delete the texture if it exists
    SDL_FreeSurface(face->mtl.tex.texture);
    glDeleteTextures(1, &face->mtl.tex.texID);

    // Delete the index buffer object
    glDeleteBuffers(1, &face->ibo);
  }
  
  // Delete the faces array
  array_delete(mesh->faces);

  // Finally delete the vertex array object
  glDeleteVertexArrays(1, &mesh->vao);
}

