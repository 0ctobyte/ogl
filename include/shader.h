#ifndef __SHADER_H__
#define __SHADER_H__

#include <stdint.h>

#include "mat.h"

uint32_t shader_load(const char *vertfile, const char *fragfile);
void shader_bind(uint32_t s_id, mat4_t *projection, mat4_t *view, mat4_t *model, uintptr_t vert_offset);
void shader_unbind(uint32_t s_id);
void shader_delete(uint32_t s_id);

#endif // __SHADER_H__

