#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include <SDL2/SDL_log.h>

#include "mtl.h"

uint32_t mtl_lexer_get_token(mtl_parser_t *p) {
  // Skip comment lines
  while(p->fstring[p->c_index] == '#') {
    p->c_index += strcspn(&p->fstring[p->c_index], "\n");
    while(isspace(p->fstring[p->c_index])) p->c_index++;
  }

  // Skip all white spaces
  while(isspace(p->fstring[p->c_index])) p->c_index++;

  // Check if end of file has been reached
  if(p->c_index >= p->fsize) {
    p->token.type = ENDOFFILE;
    return 1;
  }

  array_clear(p->token.lexeme);

  // Read characters until the next whitespace or separator 
  while(!isspace(p->fstring[p->c_index]) && p->fstring[p->c_index] != '/') {
    char c = p->fstring[p->c_index++];
    array_append(p->token.lexeme, &c);
  }
  size_t tok_len = array_size(p->token.lexeme);

  // Get the actual data from the array
  char *lexeme = (char*)array_data(p->token.lexeme);

  if(tok_len == 0 || lexeme == NULL) return 1;

  return 0;
}

void mtl_parser_expect(mtl_parser_t *p, mtl_token_type_t expected) {
  // The next token must be of the specified type
  mtl_lexer_get_token(p);

  assert(p->token.type == expected);
}

bool mtl_parser_found(mtl_parser_t *p, mtl_token_type_t type) {
  // Check if the next token is of the given type
  mtl_lexer_get_token(p);

  // Rewind character stream if false
  if(p->token.type != type) p->c_index -= array_size(p->token.lexeme);

  return (p->token.type == type);
}

int32_t mtl_parser_init(mtl_parser_t *p, const char *filename) {
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Parsing: %s\n", filename);

  FILE *f = fopen(filename, "rb");

  // If file doesn't exist
  if(f == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open file: %s\n", filename);
    return 1;
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
  *p = (mtl_parser_t){fstring, fsize, 0, {array_create(2, sizeof(char)), UNKNOWN}};

  return 0;
}

void mtl_parser_free(mtl_parser_t *p) {
  if(p->fstring != NULL) free(p->fstring);
  array_delete(p->token.lexeme);
}

