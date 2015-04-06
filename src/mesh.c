#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_log.h>
#include <OpenGL/gl3.h>

#include "mesh.h"
#include "vec.h"
#include "obj.h"

void _mesh_gen_buffers(mesh_t *mesh) {
  // Generate the name for the vertex array object (VAO)
  glGenVertexArrays(1, &mesh->vao);
  glBindVertexArray(mesh->vao);

  // Create names for the vertex buffer object (VBO)
  glGenBuffers(1, &mesh->vbo);

  // Copy the vertex data into the vertex buffer
  glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
  glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(array_size(mesh->vattributes)*3*sizeof(float)), array_data(mesh->vattributes), GL_STATIC_DRAW);

  // Unbind everything
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void _mesh_gen_index_buffers(mesh_t *mesh) {
  glBindVertexArray(mesh->vao);

  // Generate index buffer for each material group and copy data to the GPU
  size_t size = array_size(mesh->mtl_grps);
  for(uint32_t i = 0; i < size; i++) {
    material_group_t *grp = (material_group_t*)array_at(mesh->mtl_grps, i);
    glGenBuffers(1, &grp->ibo);

    // Copy the index data into the index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, grp->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(array_size(grp->indices)*sizeof(uint32_t)), array_data(grp->indices), GL_STATIC_DRAW);
  }

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void _mesh_create_material_group(mesh_t *mesh) {
  // Create a new material group
  material_group_t grp;

  // Create the index array
  grp.indices = array_create(2, sizeof(uint32_t));

  // Default values for the material in case none exist
  grp.mtl.diffuse[0] = 0.75f, grp.mtl.diffuse[1] = 0.75f, grp.mtl.diffuse[2] = 0.75f;
  grp.mtl.ambient[0] = 0.0f, grp.mtl.ambient[1] = 0.0f, grp.mtl.ambient[2] = 0.0f;
  grp.mtl.specular[0] = 1.0f, grp.mtl.specular[1] = 1.0f, grp.mtl.specular[2] = 1.0f;
  grp.mtl.shininess = 80.0f;
  grp.mtl.transparency = 1.0f;
  grp.mtl.tex.texID = 0;
  grp.mtl.tex.texture = NULL;
  grp.mtl.tex.use_texture = false;

  // Add the material group to the mesh
  array_append(mesh->mtl_grps, &grp);
}

void _mesh_load_texture(mesh_t *mesh, const char *tex_filename) {
  // Get the latest material group
  material_group_t *grps = (material_group_t*)array_at(mesh->mtl_grps, (uint32_t)array_size(mesh->mtl_grps)-1);

  // Load the BMP
  grps->mtl.tex.use_texture = 0;
  SDL_Surface *surface = SDL_LoadBMP(tex_filename);

  // Make sure the file exists and was loaded properly
  if(surface == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error reading texture file: %s\n", tex_filename);
    return;
  }

  // Convert the surface to RGBA
  grps->mtl.tex.texture = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA8888, 0);
  if(surface != NULL) SDL_FreeSurface(surface);
  
  glBindVertexArray(mesh->vao);

  // Generate the texture handle
  glGenTextures(1, &grps->mtl.tex.texID);
  glBindTexture(GL_TEXTURE_2D, grps->mtl.tex.texID);

  // Set the texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // Upload the texture pixel data
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)grps->mtl.tex.texture->w, (GLsizei)grps->mtl.tex.texture->h, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, grps->mtl.tex.texture->pixels);

  // Unbind the texture
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindVertexArray(0);

  // Free the surface
  grps->mtl.tex.use_texture = 1;
  SDL_FreeSurface(grps->mtl.tex.texture);
}

void _mesh_load_mtl(mesh_t *mesh, const char *mtl_filename, const char *mtl_name) {
  FILE *mtl_file = fopen(mtl_filename, "r");

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
  material_group_t *grp = (material_group_t*)array_at(mesh->mtl_grps, (uint32_t)array_size(mesh->mtl_grps)-1);
  while(fgets(line, 256, mtl_file) != NULL) {
    char *pch = strtok(line, " ");

    // Break if we have reached the end of the material definition (i.e. a new material definition line has been reached)
    if(strcmp(pch, "newmtl") == 0) break;

    if(strcmp(pch, "Ns") == 0) {
      // The specular shininess
      pch = strtok(NULL, " ");
      if(pch == NULL) continue;
      grp->mtl.shininess = strtof(pch, NULL);
    } else if(strcmp(pch, "d") == 0) {
      // The alpha transparency
      pch = strtok(NULL, " ");
      if(pch == NULL) continue;
      grp->mtl.transparency = strtof(pch, NULL);
    } else if(strcmp(pch, "Ka") == 0) {
      // The material ambient color
      float v[3] = {0.0f, 0.0f, 0.0f};
      pch = strtok(NULL, " ");
      for(uint32_t i = 0; pch != NULL; ++i) {
        v[i] = strtof(pch, NULL);
        pch = strtok(NULL, " ");
      }
      memcpy(grp->mtl.ambient, v, 3*sizeof(float));
    } else if(strcmp(pch, "Kd") == 0) {
      // The material diffuse color
      float v[3] = {0.0f, 0.0f, 0.0f};
      pch = strtok(NULL, " ");
      for(uint32_t i = 0; pch != NULL; ++i) {
        v[i] = strtof(pch, NULL);
        pch = strtok(NULL, " ");
      }
      memcpy(grp->mtl.diffuse, v, 3*sizeof(float));
    } else if(strcmp(pch, "Ks") == 0) {
      // The material specular color
      float v[3] = {0.0f, 0.0f, 0.0f};
      pch = strtok(NULL, " ");
      for(uint32_t i = 0; pch != NULL; ++i) {
        v[i] = strtof(pch, NULL);
        pch = strtok(NULL, " ");
      }
      memcpy(grp->mtl.specular, v, 3*sizeof(float));
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

  // Close the file
  fclose(mtl_file);
}

bool mesh_load(mesh_t *mesh, const char *objfile) {
  // Initialize the parser struct
  obj_parser_t p;
  if(obj_parser_init(&p, objfile) != 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not load mesh: %s\n", objfile);
    return false;
  }

  // Initialize the arrays.
  mesh->vattributes = array_create(256, 3*sizeof(float));
  mesh->mtl_grps = array_create(2, sizeof(material_group_t));
  
  // Grab the vertex attribute data and place them in separate arrays
  array_t *uv = array_create(256, 3*sizeof(float));
  array_t *normals = array_create(256, 3*sizeof(float));
  array_t *mtllib = array_create(16, sizeof(char));

  for(; p.token.type != ENDOFFILE; obj_lexer_get_token(&p)) {
    switch(p.token.type) {
    case VNTAG:
      // Parse the vertex normals
      obj_parser_vntag(&p, normals);
      break;
    case VTTAG:
      // Parse the vertex texture coordinates
      obj_parser_vttag(&p, uv);
      break;
    case VTAG:
      // Parse the vertex position coordinates
      obj_parser_vtag(&p, mesh->vattributes);

      // Now append zero vectors for the normal and texture coordinate attributes
      // The z value is initially set to -10.0f to indicate that the attribute is currently empty
      float u[3] = {0.0f, 0.0f, -10.0f};
      array_append(mesh->vattributes, &u);
      array_append(mesh->vattributes, &u);
      break;
    case MTLLIBTAG:
      // Parse the name of the mtllib file
      obj_parser_mtllibtag(&p, mtllib);

      // Get the mtllib filename
      array_prepend_str(mtllib, "resources/");
      break;
    default:
      break;
    }
  }

  array_t *i_positions = array_create(4, sizeof(uint32_t));
  array_t *i_texcoords = array_create(4, sizeof(uint32_t));
  array_t *i_normals = array_create(4, sizeof(uint32_t));

  // Parse the indices as they are read in and place the vertex attributes in the correct index in the vertex attribute array
  for(p.c_index = 0, p.token.type = UNKNOWN; p.token.type != ENDOFFILE; obj_lexer_get_token(&p)) {
    switch(p.token.type) {
    case FTAG:
      {
        // Parse the face indices
        array_clear(i_positions);
        array_clear(i_texcoords);
        array_clear(i_normals);
        obj_parser_ftag(&p, i_positions, i_texcoords, i_normals);

        mesh->num_faces++;
     
        // Create a material group if none exist
        // We might have obj files with only one group of faces and no materials 
        if(array_size(mesh->mtl_grps) == 0) _mesh_create_material_group(mesh);

        // For each vertex attribute specified, add them to the material group's indices list duplicated the attribute if necessary
        for(uint32_t i = 0; i < 3; ++i) {
          // Add the indices to the last material group in the list
          material_group_t *grp = (material_group_t*)array_back(mesh->mtl_grps);

          // Indices start from 1 in the Wavefront OBJ format
          // Get all the indices for the each vertex attributes
          uint32_t index = *((uint32_t*)array_at(i_positions, i))-1;
          uint32_t v_index = (array_size(i_texcoords) > 0) ? *((uint32_t*)array_at(i_texcoords, i)) : 0;
          uint32_t n_index = (array_size(i_normals) > 0) ? *((uint32_t*)array_at(i_normals, i)) : 0;
         
          float v[3] = {0.0f, 0.0f, 0.0f};
          float n[3] = {0.0f, 0.0f, 0.0f};
          bool duplicate = false;

          // Make sure the texture coordinate was specified
          if(v_index != 0) {
            v_index--;
            memcpy(v, array_at(uv, v_index), 3*sizeof(float));

            // Check if the vertex needs to be duplicated, if the texture coordinates are different for the same vertex
            float *u = array_at(mesh->vattributes, index*3+1);

            if((u[2] != -10.0f)&& (u[0] != v[0] || u[1] != v[1])) duplicate = true;
          } 

          // Make sure a normal was specified
          if(n_index != 0) {
            n_index--;
            memcpy(n, array_at(normals, n_index), 3*sizeof(float));

            // Check if the vertex needs to be duplicated, if the normals are different for the same vertex
            float *u = array_at(mesh->vattributes, index*3+2);

            if((u[2] != -10.0f) && (u[0] != n[0] || u[1] != n[1] || u[2] != n[2])) duplicate = true;
          }

          // Duplicate the vertex attribute if needed
          if(duplicate) {
            float l[3] = {0.0f, 0.0f, 0.0f};
            memcpy(l, array_at(mesh->vattributes, index*3), 3*sizeof(float));
            array_append(mesh->vattributes, l);
            index = ((uint32_t)array_size(mesh->vattributes)-1)/3;
            array_append(mesh->vattributes, v);
            array_append(mesh->vattributes, n);
          } else {
            // Set the texture coordinate and normal in the vertex attribute array 
            array_set(mesh->vattributes, index*3+1, v);
            array_set(mesh->vattributes, index*3+2, n);
          }

          // Append the index into the index array
          array_append(grp->indices, &index);
        }

        break;
      }
    case USEMTLTAG:
      {
        array_t *mtl_name = array_create(8, sizeof(char));
        obj_parser_usemtltag(&p, mtl_name);

        // Start a new material group using this material
        _mesh_create_material_group(mesh);
        
        // Load the material data for the material group
        _mesh_load_mtl(mesh, array_data(mtllib), array_data(mtl_name));
        array_delete(mtl_name);
        break;
      }
    default:
      break;
    }
  }

  // Cleanup temp arrays
  array_delete(uv);
  array_delete(normals);
  array_delete(mtllib);
  array_delete(i_positions);
  array_delete(i_texcoords);
  array_delete(i_normals);

  // Delete the parser struct
  obj_parser_free(&p);

  // Generate and fill the OpenGL buffers
  _mesh_gen_buffers(mesh);
  _mesh_gen_index_buffers(mesh);

  // Print some stats
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Mesh loaded with %lu vertex attributes and %lu faces\n", array_size(mesh->vattributes)/3, mesh->num_faces);

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
  array_delete(mesh->vattributes);
 
  // Delete all the index arrays
  size_t size = array_size(mesh->mtl_grps);
  for(uint32_t i = 0; i < size; i++) {
    // Delete the index array
    material_group_t *grp = (material_group_t*)array_at(mesh->mtl_grps, i);
    array_delete(grp->indices);

    // Delete the texture if it exists
    if(grp->mtl.tex.texture != NULL) SDL_FreeSurface(grp->mtl.tex.texture);
    glDeleteTextures(1, &grp->mtl.tex.texID);

    // Delete the index buffer object
    glDeleteBuffers(1, &grp->ibo);
  }
  
  // Delete the material group array
  array_delete(mesh->mtl_grps);

  // Finally delete the vertex array object
  glDeleteVertexArrays(1, &mesh->vao);
}

