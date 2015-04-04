#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include <SDL2/SDL_log.h>

#include "obj.h"

typedef enum {
  UNKNOWN,
  VTAG,
  VNTAG,
  VTTAG,
  FTAG,
  MTLLIBTAG,
  USEMTLTAG,
  IDENTIFIER,
  FLOAT,
  UINT,
  SEPARATOR,
  ERROR,
  ENDOFFILE
} token_type_t;

typedef struct {
  array_t *lexeme;
  token_type_t type;
} token_t;

typedef struct {
  char *fstring;
  size_t fsize;
  size_t c_index;
  token_t token;
} obj_parser_t;

/*static char *type_strings[16] = {
  "UNKNOWN", 
  "VTAG",
  "VNTAG",
  "VTTAG",
  "FTAG",
  "MTLLIBTAG",
  "USEMTLTAG",
  "IDENTIFIER",
  "FLOAT",
  "UINT",
  "SEPARATOR",
  "ERROR",
  "ENDOFFILE"
};*/

uint32_t obj_lexer_get_token(obj_parser_t *p) {
  token_type_t prev_type = p->token.type;
  p->token.type = UNKNOWN;

  // Skip comment lines
  while(p->fstring[p->c_index] == '#') {
    p->c_index += strcspn(&p->fstring[p->c_index], "\n");
    while(isspace(p->fstring[p->c_index])) p->c_index++;
  }

  // Skip all the white spaces and newlines
  while(isspace(p->fstring[p->c_index])) p->c_index++;

  // Check if end of file has been reached
  if(p->c_index >= p->fsize) {
    p->token.type = ENDOFFILE;
    return 1;
  }

  array_clear(p->token.lexeme);

  // Read characters until the next whitespace or separator 
  while(!isspace(p->fstring[p->c_index]) && p->fstring[p->c_index] != '/') {
    char c = (char)tolower(p->fstring[p->c_index++]);
    array_append(p->token.lexeme, &c);
  }
  size_t tok_len = array_size(p->token.lexeme);

  // If token length is zero, it must be the separator...right?
  if(tok_len == 0) {
    array_append(p->token.lexeme, &p->fstring[p->c_index++]);
    tok_len++;
  }

  // Get the actual data from the array
  char *lexeme = (char*)array_data(p->token.lexeme);

  // Check if it is a separator
  if(lexeme[0] == '/') {
    p->token.type = ERROR;

    if(tok_len != 1) return 1;

    p->token.type = SEPARATOR;
    return 0;
  }

  // Check if this token is an identifier depending on the previous type
  if(prev_type == MTLLIBTAG || prev_type == USEMTLTAG) {
    p->token.type = IDENTIFIER;
    return 0;
  }

  // Check if the token is a tag: r'vn|vt|v|f'
  if(lexeme[0] == 'v' || lexeme[0] == 'f' || lexeme[0] == 'u' || lexeme[0] == 'm') {
    // Which kind of tag?
    if(strcmp(lexeme, "vn") == 0) p->token.type = VNTAG;
    else if(strcmp(lexeme, "vt") == 0) p->token.type = VTTAG;
    else if(strcmp(lexeme, "v") == 0) p->token.type = VTAG;
    else if(strcmp(lexeme, "f") == 0) p->token.type = FTAG;
    else if(strcmp(lexeme, "mtllib") == 0) p->token.type = MTLLIBTAG;
    else if(strcmp(lexeme, "usemtl") == 0) p->token.type = USEMTLTAG;
    else p->token.type = ERROR;

    return (p->token.type == ERROR) ? 1 : 0;
  }

  // Check if it is a floating point number: -?[0-9]+\.[0-9]+
  if(strchr(lexeme, '.') != NULL && (lexeme[0] == '-' || isdigit(lexeme[0]))) {
    p->token.type = ERROR;

    // Confirm that this is a correctly formatted float
    size_t index = 0;
    if(lexeme[index] == '-') index++;
    while(isdigit(lexeme[index++]));

    // Must be a period
    if(lexeme[(index-1)] != '.') return 1;

    // Continue confirming digits in the decimal portion
    while(isdigit(lexeme[index++]));

    // If index is the number as tok_len then we have successfully confirmed a floating point number
    if(--index != tok_len) return 1;

    p->token.type = FLOAT;
    return 0;
  }

  // Check if it is a uint: [0-9]+
  if(isdigit(lexeme[0])) {
    p->token.type = ERROR;

    // Confirm that this is a correctly formatted uint
    size_t index = 0;
    while(isdigit(lexeme[index++]));

    // If index is the same number as tok_len then we have successfully confirmed a uint
    if(--index != tok_len) return 1;

    p->token.type = UINT;
    return 0;
  }

  return 0;
}

void obj_parser_expect(obj_parser_t *p, token_type_t expected) {
  // The next token must be of the specified type
  obj_lexer_get_token(p);

  assert(p->token.type == expected);
}

bool obj_parser_found_one_from_many(obj_parser_t *p, token_type_t *types, size_t num_types) {
  // Check if the next token is one of any of the given types
  obj_lexer_get_token(p);
  bool found = false;

  // See if the token type of the current lexeme matches one
  for(uint32_t i = 0;i < num_types; i++) {
    if(p->token.type == types[i]) {
      found = true;
      break;
    }
  }

  // Rewind character stream if false
  if(!found) p->c_index -= array_size(p->token.lexeme);

  return found;
}

bool obj_parser_found(obj_parser_t *p, token_type_t type) {
  // Check if the next token is of the given type
  obj_lexer_get_token(p);

  // Rewind character stream if false
  if(p->token.type != type) p->c_index -= array_size(p->token.lexeme);

  return (p->token.type == type);
}


void obj_parser_vtag(obj_parser_t *p, obj_parsed_data_t *d) {
  // v = 'v', whitespace, float, whitespace, float, whitespace, float
  // A vtag is followed by 3 floats
  float coords[3];

  for(uint32_t i = 0; i < 3; ++i) {
    obj_parser_expect(p, FLOAT);
    coords[i] = strtof(array_data(p->token.lexeme), NULL);
  }

  array_append(d->v_positions, &coords);
}

void obj_parser_vttag(obj_parser_t *p, obj_parsed_data_t *d) {
  // vt = "vt", whitespace, float, whitespace, float, [whitespace, float]
  float coords[3];

  for(uint32_t i = 0; i < 2; ++i) {
    obj_parser_expect(p, FLOAT);
    coords[i] = strtof(array_data(p->token.lexeme), NULL);
  }

  if(obj_parser_found(p, FLOAT)) {
    // In case we get a 3d texture coordinate
    coords[2] = strtof(array_data(p->token.lexeme), NULL);
  } else {
    coords[2] = 0.0f;
  }

  array_append(d->v_texcoords, &coords);
}

void obj_parser_vntag(obj_parser_t *p, obj_parsed_data_t *d) {
  // vn = "vn", whitespace, float, whitespace, float, whitespace, float
  float coords[3];

  for(uint32_t i = 0; i < 3; ++i) {
    obj_parser_expect(p, FLOAT);
    coords[i] = strtof(array_data(p->token.lexeme), NULL);
  }

  array_append(d->v_normals, &coords);
}

void obj_parser_ftag(obj_parser_t *p, obj_parsed_data_t *d) {
  // f = 'f', whitespace, ((uint, whitespace, uint, whitespace, uint
  // | uint, separator, uint, whitespace, uint, separator, uint, whitespace, uint, separator, uint
  // | uint, separator, uint, separator, uint, whitespace, uint, separator, uint, separator, uint, whitespace, uint, separator, uint, separator, uint
  // | uint, separator, separator, uint, whitespace, uint, separator, separator, uint, whitespace, uint, separator, separator, uint)
  
  // If there are no existing material groups, create one
  if(array_size(d->mtl_groups) == 0) {
    obj_material_group_t mtl_grp = {array_create(2, sizeof(char)), array_create(2, sizeof(uint32_t)), array_create(2, sizeof(uint32_t)), array_create(2, sizeof(uint32_t))};
    array_append(d->mtl_groups, &mtl_grp);
  }

  // Get the latest mtl group
  obj_material_group_t *mtl_group = array_back(d->mtl_groups);

  // 3 sets of indices for each vertex
  for(uint32_t i = 0; i < 3; ++i) {
    obj_parser_expect(p, UINT);
    uint32_t index = (uint32_t)strtoul(array_data(p->token.lexeme), NULL, 10);
    array_append(mtl_group->i_positions, &index);

    if(obj_parser_found(p, SEPARATOR)) {
      // Double separator means only normal index specified
      if(obj_parser_found(p, SEPARATOR)) {
        obj_parser_expect(p, UINT);

        index = (uint32_t)strtoul(array_data(p->token.lexeme), NULL, 10);
        array_append(mtl_group->i_normals, &index);
      } else {
        // One separator indicates a texcoord 
        obj_parser_expect(p, UINT);

        index = (uint32_t)strtoul(array_data(p->token.lexeme), NULL, 10);
        array_append(mtl_group->i_texcoords, &index);

        // If another separator is found then a normal is also specified
        if(obj_parser_found(p, SEPARATOR)) {
          obj_parser_expect(p, UINT);

          index = (uint32_t)strtoul(array_data(p->token.lexeme), NULL, 10);
          array_append(mtl_group->i_normals, &index);
        }
      }
    }
  }
}

void obj_parser_mtllibtag(obj_parser_t *p, obj_parsed_data_t *d) {
  // mtllib = "mtllib", whitespace, identifier
  // Expect an identifier that indentifies the filename of the mtllib
  obj_parser_expect(p, IDENTIFIER);

  // Copy the mtllib filename
  array_copy(d->mtl_filename, p->token.lexeme);
}

void obj_parser_usemtltag(obj_parser_t *p, obj_parsed_data_t *d) {
  // usemtl = "usemtl", whitespace, identifier
  // Expect an identifier that indicates the material to use in the mtllib
  obj_parser_expect(p, IDENTIFIER);

  // Create a new material group
  obj_material_group_t mtl_grp = {array_create(2, sizeof(char)), array_create(2, sizeof(uint32_t)), array_create(2, sizeof(uint32_t)), array_create(2, sizeof(uint32_t))};

  // Copy the material name
  array_copy(mtl_grp.mtl_name, p->token.lexeme);
  
  // Append the mtl group to the list of mtl groups
  array_append(d->mtl_groups, &mtl_grp);
}

void obj_parser_tag(obj_parser_t *p, obj_parsed_data_t *d) {
  // tag = v | vn | vt | f | usemtl | mtllib
  switch(p->token.type) {
  case VTAG:
    obj_parser_vtag(p, d);
    break;
  case VTTAG:
    obj_parser_vttag(p, d);
    break;
  case VNTAG:
    obj_parser_vntag(p, d);
    break;
  case FTAG:
    obj_parser_ftag(p, d);
    break;
  case MTLLIBTAG:
    obj_parser_mtllibtag(p, d);
    break;
  case USEMTLTAG:
    obj_parser_usemtltag(p, d);
    break;
  default:
    break;
  }
}

void obj_parser_free_parser(obj_parser_t *p) {
  if(p->fstring != NULL) free(p->fstring);
  array_delete(p->token.lexeme);
}

void obj_parse(obj_parsed_data_t *d, const char *filename) {
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "parsing: %s\n", filename);

  FILE *f = fopen(filename, "rb");

  // If file doesn't exist
  if(f == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open file: %s\n", filename);
    return;
  }

  // Determine the size of the file
  fseek(f, 0, SEEK_END);
  size_t fsize = (size_t)ftell(f);
  fseek(f, 0, SEEK_SET);

  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Size of \'%s\' file: %lu bytes\n", filename, fsize);

  // Read the whole file into memory
  char *fstring = (char*)malloc(fsize+1);
  fread(fstring, fsize, 1, f);
  fclose(f);

  // Null terminate the string
  fstring[fsize] = 0;

  // Initialize the parser object
  obj_parser_t p = {fstring, fsize, 0, {array_create(2, sizeof(char)), UNKNOWN}};
  
  // Initialize the parsed data object
  *d = (obj_parsed_data_t){array_create(2, sizeof(char)), array_create(2, 3*sizeof(float)), array_create(2, 3*sizeof(float)), array_create(2, 3*sizeof(float)), array_create(2, sizeof(obj_material_group_t))};

  // Start parsing the data
  token_type_t types[] = {VTAG, VNTAG, VTTAG, FTAG, MTLLIBTAG, USEMTLTAG, UNKNOWN};
  while(p.token.type != ERROR && p.token.type != ENDOFFILE) {
    // A valid tag must be found on each line
    if(!obj_parser_found_one_from_many(&p, types, sizeof(types)/sizeof(types[0]))) {
      if(p.token.type == ENDOFFILE) continue; 
      SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unexpected token: %s\n", array_data(p.token.lexeme));

      // Skip this token and continue
      obj_lexer_get_token(&p);
      continue;
    }

    // What if we get an unknown token
    if(p.token.type == UNKNOWN) {
      SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Unknown token: %s\n", array_data(p.token.lexeme));

      // Skip this token and continue
      obj_lexer_get_token(&p);
      continue; 
    }

    // Parse the tag
    obj_parser_tag(&p, d);
  }

  // Free the parser object
  obj_parser_free_parser(&p);
}

void obj_parser_free_parsed_data(obj_parsed_data_t *data) {
  // Delete all the arrays
  array_delete(data->mtl_filename);
  array_delete(data->v_positions);
  array_delete(data->v_texcoords);
  array_delete(data->v_normals);

  // Delete all the index arrays from the material groups
  if(data->mtl_groups != NULL) {
    size_t size = array_size(data->mtl_groups);
    obj_material_group_t *mtlgroups =  (obj_material_group_t*)array_data(data->mtl_groups);
    for(uint32_t i = 0; i < size; i++) {
      obj_material_group_t mgrp = mtlgroups[i];
      array_delete(mgrp.mtl_name);
      array_delete(mgrp.i_positions);
      array_delete(mgrp.i_texcoords);
      array_delete(mgrp.i_normals);
    }

    // Delete the material group array
    array_delete(data->mtl_groups);
  }
}

