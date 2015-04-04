#ifndef __OBJ_H__
#define __OBJ_H__

#include "array.h"

typedef struct {
  array_t *mtl_name;
  array_t *i_positions;
  array_t *i_texcoords;
  array_t *i_normals;
} obj_material_group_t;

typedef struct {
  array_t *mtl_filename;
  array_t *v_positions;
  array_t *v_texcoords;
  array_t *v_normals;
  array_t *mtl_groups;
} obj_parsed_data_t;

void obj_parse(obj_parsed_data_t *data, const char *filename);
void obj_parser_free_parsed_data(obj_parsed_data_t *data);

#endif // __OBJ_H__

