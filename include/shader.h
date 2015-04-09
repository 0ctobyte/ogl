#ifndef __SHADER_H__
#define __SHADER_H__

#include <stdint.h>

#include "gl_core_4_1.h"

typedef GLuint shader_t;

typedef enum {
  SHADER_UNIFORM_MAT4,
  SHADER_UNIFORM_VEC3,
  SHADER_UNIFORM_FLOAT,
  SHADER_UNIFORM_UINT,
  SHADER_UNIFORM_INT
} shader_uniform_type_t;

shader_t shader_load(const char *vertfile, const char *fragfile);
void shader_bind(shader_t s_id);
void shader_set_uniform(shader_t s_id, const char *uniform_name, shader_uniform_type_t uniform_type, void *data);
void shader_unbind();
void shader_delete(shader_t s_id);

#endif // __SHADER_H__

