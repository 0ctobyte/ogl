#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <SDL2/SDL_log.h>

#include "gl_core_4_1.h"
#include "mesh.h"
#include "vec.h"
#include "obj.h"
#include "mtl.h"

// A material definition. This structure holds the name of the material as well as all the relevant values for that material
typedef struct {
  material_t mtl;
  array_t *mtl_name;
} material_def_t;

void _mesh_gen_buffers(mesh_t *mesh) {
  // Generate the name for the vertex array object (VAO)
  glGenVertexArrays(1, &mesh->vao);
  glBindVertexArray(mesh->vao);

  // Create names for the vertex buffer object (VBO) and the index buffer object
  glGenBuffers(1, &mesh->vbo);
  glGenBuffers(1, &mesh->ibo);

  // Copy the index data into the index buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(array_size(mesh->indices)*sizeof(uint32_t)), array_data(mesh->indices), GL_STATIC_DRAW);

  // Copy the vertex data into the vertex buffer
  glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
  glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(array_size(mesh->vattributes)*3*sizeof(float)), array_data(mesh->vattributes), GL_STATIC_DRAW);

  // Set and enable the vertex attributes. 0 = vertex position, 1 = vertex texture coordinates, 2 = vertex normals
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (GLvoid*)0);

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (GLvoid*)(3*sizeof(float)));

  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (GLvoid*)(6*sizeof(float)));

  // Unbind VAO
  glBindVertexArray(0);
}

void _mesh_init_material(material_t *mtl) {
  mtl->diffuse[0] = 0.75f, mtl->diffuse[1] = 0.75f, mtl->diffuse[2] = 0.75f;
  mtl->ambient[0] = 0.0f, mtl->ambient[1] = 0.0f, mtl->ambient[2] = 0.0f;
  mtl->specular[0] = 1.0f, mtl->specular[1] = 1.0f, mtl->specular[2] = 1.0f;
  mtl->shininess = 80.0f;
  mtl->transparency = 1.0f;
  mtl->tex.texID = 0;
  mtl->tex.texture = NULL;
  mtl->tex.use_texture = false;
}

void _mesh_create_material_group(mesh_t *mesh) {
  // Create a new material group
  material_group_t grp;

  // Initialize the index vars
  grp.offset = 0;
  if(array_size(mesh->mtl_grps) > 0) {
    // The offset is the previous groups offset + count
    material_group_t *last_grp = (material_group_t*)array_back(mesh->mtl_grps);
    grp.offset = last_grp->offset+last_grp->count;
  }
  grp.count = 0;

  // Default values for the material in case none exist
  _mesh_init_material(&grp.mtl);

  // Add the material group to the mesh
  array_append(mesh->mtl_grps, &grp);
}

void _mesh_load_texture(mesh_t *mesh, material_t *mtl, const char *tex_filename) {
  // Load the BMP
  mtl->tex.use_texture = 0;
  SDL_Surface *surface = SDL_LoadBMP(tex_filename);

  // Make sure the file exists and was loaded properly
  if(surface == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error reading texture file: %s\n", tex_filename);
    return;
  }

  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Size of \'%s\': %u bytes, dimensions: %u x %u pixels\n", tex_filename, surface->w*surface->h*surface->format->BytesPerPixel, surface->w, surface->h);
  
  // Convert the surface to RGBA
  mtl->tex.texture = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA8888, 0);
  if(surface != NULL) SDL_FreeSurface(surface);
 
  glBindVertexArray(mesh->vao);

  // Generate the texture handle
  glGenTextures(1, &mtl->tex.texID);
  glBindTexture(GL_TEXTURE_2D, mtl->tex.texID);

  // Set the texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // Upload the texture pixel data
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)mtl->tex.texture->w, (GLsizei)mtl->tex.texture->h, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, mtl->tex.texture->pixels);

  // Unbind the texture
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindVertexArray(0);

  // Free the surface
  mtl->tex.use_texture = 1;
  SDL_FreeSurface(mtl->tex.texture);
}

bool _mesh_load_material(mesh_t *mesh, const char *mtl_filename, array_t *mtl_list) {
  // Initialize the parser struct
  mtl_parser_t p;
  if(mtl_parser_init(&p, mtl_filename) != 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not load mtllib: %s\n", mtl_filename);
    return false;
  }

  for(; p.token.type != MTL_ENDOFFILE; mtl_lexer_get_token(&p)) {
    // Get the latest material definition
    material_def_t *mtl_def = (array_size(mtl_list) > 0) ? (material_def_t*)array_back(mtl_list): NULL;
    
    switch(p.token.type) {
    case MTL_NEWMTLTAG:
      {
        // Append a new material definition to the list
        mtl_parser_expect(&p, MTL_IDENTIFIER);
        
        material_def_t newmtl;
        newmtl.mtl_name = array_create(array_size(p.token.lexeme), sizeof(char));
        
        array_copy(newmtl.mtl_name, p.token.lexeme);
        _mesh_init_material(&newmtl.mtl);
        
        array_append(mtl_list, &newmtl);
        break;
      }
    case MTL_NSTAG:
      {
        // Parse the shininess exponent
        if(mtl_parser_found(&p, MTL_FLOAT) || mtl_parser_found(&p, MTL_UINT)) {
          mtl_def->mtl.shininess = strtof(array_data(p.token.lexeme), NULL);
        }
        break;
      }
    case MTL_KATAG:
      {
        // Parse the ambient reflectivity
        for(uint64_t i = 0; i < 3; i++) {
          mtl_parser_expect(&p, MTL_FLOAT);
          mtl_def->mtl.ambient[i] = strtof(array_data(p.token.lexeme), NULL);
        }
        break;
      }
    case MTL_KDTAG:
      {
        // Parse the diffuse reflectivity
        for(uint64_t i = 0; i < 3; i++) {
          mtl_parser_expect(&p, MTL_FLOAT);
          mtl_def->mtl.diffuse[i] = strtof(array_data(p.token.lexeme), NULL);
        }
        break;
      }
    case MTL_KSTAG:
      {
        // Parse the specular reflectivity
        for(uint64_t i = 0; i < 3; i++) {
          mtl_parser_expect(&p, MTL_FLOAT);
          mtl_def->mtl.specular[i] = strtof(array_data(p.token.lexeme), NULL);
        }
        break;
      }
    case MTL_DTAG:
      {
        // Parse the dissolve/transparency value
        if(mtl_parser_found(&p, MTL_FLOAT) || mtl_parser_found(&p, MTL_UINT)) {
          mtl_def->mtl.transparency = strtof(array_data(p.token.lexeme), NULL);
        }
        break;
      }
    case MTL_MAPKDTAG:
      {
        // Parse the diffuse texture map
        mtl_parser_expect(&p, MTL_IDENTIFIER);
        
        array_t *texname = array_create(array_size(p.token.lexeme), sizeof(char));
        array_copy(texname, p.token.lexeme);
        array_prepend_str(texname, "resources/");
        
        // Load the texture from the file
        _mesh_load_texture(mesh, &mtl_def->mtl, array_data(texname));
        array_delete(texname);
        break;
      }
    default:
      break;
    }
  }

  // Delete the parser struct
  mtl_parser_free(&p);

  return true;
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
  mesh->indices = array_create(256, sizeof(uint32_t));
  mesh->mtl_grps = array_create(2, sizeof(material_group_t));
  
  // Grab the vertex attribute data and place them in separate arrays
  array_t *uv = array_create(256, 3*sizeof(float));
  array_t *normals = array_create(256, 3*sizeof(float));
  array_t *mtllib = array_create(16, sizeof(char));

  for(; p.token.type != OBJ_ENDOFFILE; obj_lexer_get_token(&p)) {
    switch(p.token.type) {
    case OBJ_VNTAG:
      // Parse the vertex normals
      obj_parser_vntag(&p, normals);
      break;
    case OBJ_VTTAG:
      // Parse the vertex texture coordinates
      obj_parser_vttag(&p, uv);
      break;
    case OBJ_VTAG:
      // Parse the vertex position coordinates
      obj_parser_vtag(&p, mesh->vattributes);

      // Now append zero vectors for the normal and texture coordinate attributes
      // The z value is initially set to -10.0f to indicate that the attribute is currently empty
      float u[3] = {0.0f, 0.0f, -10.0f};
      array_append(mesh->vattributes, &u);
      array_append(mesh->vattributes, &u);
      break;
    case OBJ_MTLLIBTAG:
      // Parse the name of the mtllib file
      obj_parser_mtllibtag(&p, mtllib);

      // Get the mtllib filename
      array_prepend_str(mtllib, "resources/");
      break;
    default:
      break;
    }
  }

  // If a mtllib file was specified, parse it
  array_t *mtl_list = array_create(2, sizeof(material_def_t));
  if(array_size(mtllib) > 0) _mesh_load_material(mesh, array_data(mtllib), mtl_list);

  array_t *i_positions = array_create(4, sizeof(uint32_t));
  array_t *i_texcoords = array_create(4, sizeof(uint32_t));
  array_t *i_normals = array_create(4, sizeof(uint32_t));

  // Parse the indices as they are read in and place the vertex attributes in the correct index in the vertex attribute array
  for(p.c_index = 0, p.token.type = OBJ_UNKNOWN; p.token.type != OBJ_ENDOFFILE; obj_lexer_get_token(&p)) {
    switch(p.token.type) {
    case OBJ_FTAG:
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

        // Add the indices to the last material group in the list
        material_group_t *grp = (material_group_t*)array_back(mesh->mtl_grps);

        // For each vertex attribute specified, add them to the material group's indices list duplicating the attribute if necessary
        for(uint64_t i = 0; i < 3; ++i) {
          
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
          array_append(mesh->indices, &index);
        }
        
        // Increment the count of indices for the material group
        grp->count += 3;

        break;
      }
    case OBJ_USEMTLTAG:
      {
        array_t *mtl_name = array_create(8, sizeof(char));
        obj_parser_usemtltag(&p, mtl_name);

        // Start a new material group using this material
        _mesh_create_material_group(mesh);
        
        // Load the material data for the material group
        // Have to find the material with the specified material name in the material definition list
        bool found_mtl = false;
        for(uint64_t i = 0; i < array_size(mtl_list); i++) {
          material_def_t *mtl_def = array_at(mtl_list, i);
          if(strcmp((char*)array_data(mtl_def->mtl_name), (char*)array_data(mtl_name)) == 0) {
            material_group_t *grp = (material_group_t*)array_back(mesh->mtl_grps);

            // Copy the material data
            memcpy(&grp->mtl, &mtl_def->mtl, sizeof(material_t));
            found_mtl = true;
            break;
          }
        }

        if(!found_mtl) {
          SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not find material: \'%s\'\n", (char*)array_data(mtl_name));
        }

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

  // Cleanup up the material definition list
  for(uint64_t i = 0; i < array_size(mtl_list); i++) {
    material_def_t *mtl_def = (material_def_t*)array_at(mtl_list, i);
    array_delete(mtl_def->mtl_name);
  }
  array_delete(mtl_list);
  
  // Delete the parser struct
  obj_parser_free(&p);

  // Generate and fill the OpenGL buffers
  _mesh_gen_buffers(mesh);

  // Print some stats
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Mesh loaded with %lu vertex attributes and %lu faces\n", array_size(mesh->vattributes)/3, mesh->num_faces);

  return true;
}

void mesh_bind(mesh_t *mesh) {
  glBindVertexArray(mesh->vao);
}

void mesh_unbind() {
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindVertexArray(0);
}

void mesh_delete(mesh_t *mesh) {
  // Delete the vertex & index buffer object
  glDeleteBuffers(1, &mesh->vbo);
  glDeleteBuffers(1, &mesh->ibo);

  // Delete the vertex attribute array
  array_delete(mesh->vattributes);

  // Delete the index array
  array_delete(mesh->indices);
 
  // Delete all the material groups
  size_t size = array_size(mesh->mtl_grps);
  for(uint64_t i = 0; i < size; i++) {
    material_group_t *grp = (material_group_t*)array_at(mesh->mtl_grps, i);

    // Delete the texture if it exists
    glDeleteTextures(1, &grp->mtl.tex.texID);
  }
  
  // Delete the material group array
  array_delete(mesh->mtl_grps);

  // Finally delete the vertex array object
  glDeleteVertexArrays(1, &mesh->vao);
}

