#ifndef __VEC_H__
#define __VEC_H__

#include <stdbool.h>

#include "gl_core_4_1.h"

typedef struct {
  GLfloat x, y, z;
} vec3_t;

typedef struct {
  GLfloat x, y, z, w;
} vec4_t;

// VEC3

vec3_t vec3_convert(vec4_t *v);
GLfloat vec3_distance(const vec3_t *v1, const vec3_t *v2);
vec3_t vec3_normalize(const vec3_t *v);
GLfloat vec3_length(const vec3_t *v);
GLfloat vec3_angle(const vec3_t *v1, const vec3_t *v2);
vec3_t vec3_cross(const vec3_t *v1, const vec3_t *v2);
vec3_t vec3_add(const vec3_t *v1, const vec3_t *v2);
vec3_t vec3_sub(const vec3_t *v1, const vec3_t *v2);
GLfloat vec3_dot(const vec3_t *v1, const vec3_t *v2);
vec3_t vec3_scale(const vec3_t *v, GLfloat k);
vec3_t vec3_neg(const vec3_t *v);
bool vec3_compare(const vec3_t *v1, const vec3_t *v2);
void vec3_str(const vec3_t *v, char *str);

// VEC4

vec4_t vec4_convert(vec3_t *v, GLfloat w);
GLfloat vec4_distance(const vec4_t *v1, const vec4_t *v2);
vec4_t vec4_normalize(const vec4_t *v);
GLfloat vec4_length(const vec4_t *v);
GLfloat vec4_angle(const vec4_t *v1, const vec4_t *v2);
vec4_t vec4_add(const vec4_t *v1, const vec4_t *v2);
vec4_t vec4_sub(const vec4_t *v1, const vec4_t *v2);
GLfloat vec4_dot(const vec4_t *v1, const vec4_t *v2);
vec4_t vec4_scale(const vec4_t *v, GLfloat k);
vec4_t vec4_neg(const vec4_t *v);
bool vec4_compare(const vec4_t *v1, const vec4_t *v2);
void vec4_str(const vec4_t *v, char *str);

#endif // __VEC_H__

