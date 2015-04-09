#include <stdio.h>
#include <math.h>

#include "vec.h"

// VEC3

vec3_t vec3_convert(vec4_t *v) {
  return (vec3_t){v->x, v->y, v->z};
}

float vec3_distance(const vec3_t *v1, const vec3_t *v2) {
  vec3_t v = vec3_sub(v1, v2);
  return vec3_length(&v);
}

vec3_t vec3_normalize(const vec3_t *v) {
	float k = vec3_length(v);
	return (vec3_t){v->x/k, v->y/k, v->z/k};
}

float vec3_length(const vec3_t *v) {
	return (float)sqrt(v->x*v->x+v->y*v->y+v->z*v->z);
}

float vec3_angle(const vec3_t *v1, const vec3_t *v2) {
	return (float)acos(vec3_dot(v1, v2)/(vec3_length(v1)*vec3_length(v2)));
}

vec3_t vec3_cross(const vec3_t *v1, const vec3_t *v2) {
	return (vec3_t){v1->y*v2->z-v1->z*v2->y, v1->z*v2->x-v1->x*v2->z, v1->x*v2->y-v1->y*v2->x};
}

vec3_t vec3_add(const vec3_t *v1, const vec3_t *v2) {
	return (vec3_t){v1->x+v2->x, v1->y+v2->y, v1->z+v2->z};
}

vec3_t vec3_sub(const vec3_t *v1, const vec3_t *v2) {
	return (vec3_t){v1->x-v2->x, v1->y-v2->y, v1->z-v2->z};
}

float vec3_dot(const vec3_t *v1, const vec3_t *v2) {
	return (v1->x*v2->x+v1->y*v2->y+v1->z*v2->z);
}

vec3_t vec3_scale(const vec3_t *v, float k) {
	return (vec3_t){v->x*k, v->y*k, v->z*k};
}

vec3_t vec3_neg(const vec3_t *v) {
	return (vec3_t){-v->x, -v->y, -v->z};
}

bool vec3_compare(const vec3_t *v1, const vec3_t *v2) {
	return (v1->x==v2->x && v1->y==v2->y && v1->z==v2->z);
}

void vec3_str(const vec3_t *v, char *str) {
  sprintf(str, "[%10.6f, %10.6f, %10.6f]", v->x, v->y, v->z);
}

// VEC4

vec4_t vec4_convert(vec3_t *v, float w) {
  return (vec4_t){v->x, v->y, v->z, w};
}

float vec4_distance(const vec4_t *v1, const vec4_t *v2) {
  vec4_t v = vec4_sub(v1, v2);
  return vec4_length(&v);
}

vec4_t vec4_normalize(const vec4_t *v) {
	float k = vec4_length(v);
	return (vec4_t){v->x/k, v->y/k, v->z/k, v->w/k};
}

float vec4_length(const vec4_t *v) {
	return (float)sqrt(v->x*v->x+v->y*v->y+v->z*v->z*v->w*v->w);
}

float vec4_angle(const vec4_t *v1, const vec4_t *v2) {
	return (float)acos(vec4_dot(v1, v2)/(vec4_length(v1)*vec4_length(v2)));
}

vec4_t vec4_add(const vec4_t *v1, const vec4_t *v2) {
	return (vec4_t){v1->x+v2->x, v1->y+v2->y, v1->z+v2->z, v1->w+v2->w};
}

vec4_t vec4_sub(const vec4_t *v1, const vec4_t *v2) {
	return (vec4_t){v1->x-v2->x, v1->y-v2->y, v1->z-v2->z, v1->w-v2->w};
}

float vec4_dot(const vec4_t *v1, const vec4_t *v2) {
	return (v1->x*v2->x+v1->y*v2->y+v1->z*v2->z+v1->w*v2->w);
}

vec4_t vec4_scale(const vec4_t *v, float k) {
	return (vec4_t){v->x*k, v->y*k, v->z*k, v->w*k};
}

vec4_t vec4_neg(const vec4_t *v) {
	return (vec4_t){-v->x, -v->y, -v->z, -v->w};
}

bool vec4_compare(const vec4_t *v1, const vec4_t *v2) {
	return (v1->x==v2->x && v1->y==v2->y && v1->z==v2->z && v1->w==v2->w);
}

void vec4_str(const vec4_t *v, char *str) {
  sprintf(str, "[%10.6f, %10.6f, %10.6f, %10.6f]", v->x, v->y, v->z, v->w);
}

