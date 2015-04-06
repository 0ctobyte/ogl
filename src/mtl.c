#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include <SDL2/SDL_log.h>

#include "mtl.h"

uint32_t mtl_lexer_get_token(mtl_parser_t *p) {
  mtl_token_type_t prev_type = p->token.type;
  p->token.type = UNKNOWN;

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

  // Check if this token is an identifier depending on the the previous token type
  if(prev_type >= MAPKATAG && prev_type <= NEWMTLTAG) {
    p->token.type = IDENTIFIER;
    return 0;
  }

  // Check if the token is a tag
  if(isalpha(lexeme[0])) {
    // Which kind of tag
    if(strcmp(lexeme, "Ns") == 0) p->token.type = NSTAG;
    else if(strcmp(lexeme, "Ka") == 0) p->token.type = KATAG;
    else if(strcmp(lexeme, "Kd") == 0) p->token.type = KDTAG;
    else if(strcmp(lexeme, "Ks") == 0) p->token.type = KSTAG;
    else if(strcmp(lexeme, "d") == 0) p->token.type = DTAG;
    else if(strcmp(lexeme, "map_Ka") == 0) p->token.type = MAPKATAG;
    else if(strcmp(lexeme, "map_Kd") == 0) p->token.type = MAPKDTAG;
    else if(strcmp(lexeme, "map_Ks") == 0) p->token.type = MAPKSTAG;
    else if(strcmp(lexeme, "map_Bump") == 0) p->token.type = MAPBUMPTAG;
    else if(strcmp(lexeme, "map_d") == 0) p->token.type = MAPDTAG;
    else if(strcmp(lexeme, "newmtl") == 0) p->token.type = NEWMTLTAG;
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

