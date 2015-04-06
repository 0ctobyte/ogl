#ifndef __OBJ_H__
#define __OBJ_H__

#include "array.h"

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

int32_t obj_parser_init(obj_parser_t *p, const char *filename);
void obj_parser_free(obj_parser_t *p);
uint32_t obj_lexer_get_token(obj_parser_t *p);

void obj_parser_vtag(obj_parser_t *p, array_t *a);
void obj_parser_vttag(obj_parser_t *p, array_t *a);
void obj_parser_vntag(obj_parser_t *p, array_t *a);
void obj_parser_ftag(obj_parser_t *p, array_t *i_positions, array_t *i_texcoords, array_t *i_normals);
void obj_parser_mtllibtag(obj_parser_t *p, array_t *a);
void obj_parser_usemtltag(obj_parser_t *p, array_t *a);


#endif // __OBJ_H__

