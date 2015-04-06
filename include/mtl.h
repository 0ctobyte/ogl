#ifndef __MTL_H__
#define __MTL_H__

#include "array.h"

typedef enum {
  UNKNOWN,
  NSTAG,
  KATAG,
  KDTAG,
  KSTAG,
  DTAG,
  MAPKATAG,
  MAPKDTAG,
  MAPKSTAG,
  MAPBUMPTAG,
  MAPDTAG,
  NEWMTLTAG,
  ERROR,
  ENDOFFILE
} mtl_token_type_t;

typedef struct {
  array_t *lexeme;
  mtl_token_type_t type;
} mtl_token_t;

typedef struct {
  char *fstring;
  size_t fsize;
  size_t c_index;
  mtl_token_t token;
} mtl_parser_t;

int32_t mtl_parser_init(mtl_parser_t *p, const char *filename);
void mtl_parser_free(mtl_parser_t *p);

uint32_t mtl_lexer_get_token(mtl_parser_t *p);

#endif // __MTL_H__

