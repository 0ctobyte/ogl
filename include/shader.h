#ifndef __SHADER_H__
#define __SHADER_H__

#include <stdint.h>

typedef enum {
  SHADER_UNIFORM_MAT4,
  SHADER_UNIFORM_VEC3,
  SHADER_UNIFORM_FLOAT
} shader_uniform_type_t;

uint32_t shader_load(const char *vertfile, const char *fragfile);
void shader_bind(uint32_t s_id);
void shader_set_attrib(uint32_t s_id, const char *attrib_name, size_t stride, uintptr_t offset);
void shader_set_uniform(uint32_t s_id, const char *uniform_name, shader_uniform_type_t uniform_type, void *data);
void shader_unbind();
void shader_delete(uint32_t s_id);

#endif // __SHADER_H__

