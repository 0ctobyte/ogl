#ifndef __MTL_H__
#define __MTL_H__

#include "array.h"

typedef enum {
  MTL_UNKNOWN = 0,
  MTL_FLOAT,
  MTL_UINT,
  MTL_NSTAG,
  MTL_KATAG,
  MTL_KDTAG,
  MTL_KSTAG,
  MTL_DTAG,
  MTL_MAPKATAG,
  MTL_MAPKDTAG,
  MTL_MAPKSTAG,
  MTL_MAPBUMPTAG,
  MTL_MAPDTAG,
  MTL_NEWMTLTAG,
  MTL_IDENTIFIER,
  MTL_ERROR,
  MTL_ENDOFFILE
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

void mtl_parser_expect(mtl_parser_t *p, mtl_token_type_t expected);
bool mtl_parser_found(mtl_parser_t *p, mtl_token_type_t type);

#endif // __MTL_H__

