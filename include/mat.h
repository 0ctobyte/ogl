#ifndef __MAT_H__
#define __MAT_H__

#include "vec.h"

#define MAT4_INIT {{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}}

#define MAT4_IDENTITY {{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f}}

typedef struct {
  GLfloat m[16];
} mat4_t;

void mat4_identity(mat4_t *mat);
void mat4_frustum(mat4_t *mat, GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat znear, GLfloat zfar);
void mat4_perspective(mat4_t *mat, GLfloat fovy, GLfloat faspect, GLfloat znear, GLfloat zfar);
void mat4_orthographic(mat4_t *mat, GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat znear, GLfloat zfar);
void mat4_translatef(mat4_t *mat, GLfloat x, GLfloat y, GLfloat z);
void mat4_translate(mat4_t *mat, const vec3_t *v);
void mat4_untranslate(mat4_t *mat);
void mat4_rotatef(mat4_t *mat, GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
void mat4_rotate(mat4_t *mat, GLfloat angle, const vec3_t *u);
void mat4_transpose(mat4_t *mat);
void mat4_inverse(mat4_t *mat);
GLfloat mat4_cofactor(const mat4_t *mat, uint64_t column, uint64_t row);
GLfloat mat4_determinant(const mat4_t *mat);
void mat4_mult(mat4_t *mat1, const mat4_t *mat2);
vec4_t mat4_multv(const mat4_t *mat, const vec4_t *v);
void mat4_str(const mat4_t *mat, char *str);

#endif // __MAT_H__

