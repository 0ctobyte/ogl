#ifndef __SHADER_H__
#define __SHADER_H__

#include <stdint.h>

uint32_t shader_load(const char *vertfile, const char *fragfile);
void shader_bind(uint32_t s_id);
void shader_set_attribs(uint32_t s_id, size_t stride, uintptr_t vert_offset, uintptr_t uv_offset, uintptr_t norm_offset);
void shader_set_uniforms(uint32_t s_id, float *m_p, float *m_v, float *m_m);
void shader_unbind();
void shader_delete(uint32_t s_id);

#endif // __SHADER_H__

