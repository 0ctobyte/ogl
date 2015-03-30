#include <stdio.h>
#include <string.h>
#include <math.h>

#include <mat.h>

void mat4_identity(mat4_t *mat) {
  *mat = (mat4_t){{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f}};
}

void mat4_frustum(mat4_t *mat, float left, float right, float bottom, float top, float znear, float zfar) {
  mat4_t A = MAT4_IDENTITY;
	A.m[0] = (2.0f*znear)/(right-left);
	A.m[5] = (2.0f*znear)/(top-bottom);
	A.m[8] = (right+left)/(right-left);
	A.m[9] = (top+bottom)/(top-bottom);
	A.m[10] = -((zfar+znear)/(zfar-znear));
	A.m[11] = -1.0f;
	A.m[14] = -((2.0f*zfar*znear)/(zfar-znear));
	A.m[15] = 0.0f;
  mat4_mult(mat, &A);
}

void mat4_perspective(mat4_t *mat, float fovy, float faspect, float znear, float zfar) {
  mat4_t A = MAT4_IDENTITY;
	float f = 1.0f/((float)tan((fovy*(M_PI/180.0f))/2.0f));
	A.m[0]=f/faspect;
	A.m[5]=f;
	A.m[10]=(zfar+znear)/(znear-zfar);
	A.m[11]=-1.0f;
	A.m[14]=(2.0f*zfar*znear)/(znear-zfar);
	A.m[15]=0.0f;
  mat4_mult(mat, &A);
}

void mat4_orthographic(mat4_t *mat, float left, float right, float bottom, float top, float znear, float zfar) {
  mat4_t A = MAT4_IDENTITY;
	A.m[0] = 2.0f/(right-left);
	A.m[5] = 2.0f/(top-bottom);
	A.m[10] = -2.0f/(zfar-znear);
	A.m[12] = -((right+left)/(right-left));
	A.m[13] = -((top+bottom)/(top-bottom));
	A.m[14] = -((zfar+znear)/(zfar-znear));
  mat4_mult(mat, &A);
}

void mat4_translatef(mat4_t *mat, float x, float y, float z) {
  mat4_t A = MAT4_IDENTITY;
	A.m[12] = x;
  A.m[13] = y;
  A.m[14] = z;
  mat4_mult(mat, &A);
}

void mat4_translate(mat4_t *mat, const vec3_t *v) {
  mat4_t A = MAT4_IDENTITY;
	A.m[12] = v->x;
  A.m[13] = v->y;
  A.m[14] = v->z;
  mat4_mult(mat, &A);
}

void mat4_untranslate(mat4_t *mat) {
	mat->m[12] = 0.0f;
  mat->m[13] = 0.0f;
  mat->m[14] = 0.0f;
}

void mat4_rotatef(mat4_t *mat, float angle, float x, float y, float z) {
  mat4_t A = MAT4_IDENTITY;
  vec3_t v = {x, y, z};

  // Calculate angle in radians
  angle = angle*180.0f/(float)M_PI;
  float c = (float)cos(angle);
  float s = (float)sin(angle);
  float oc = 1-c;
  v = vec3_normalize(&v);
  A.m[0] = (v.x*v.x)*oc+c;
  A.m[1] = v.y*v.x*oc+v.z*s;
  A.m[2] = v.x*v.z*oc-v.y*v.z;
  A.m[4] = v.x*v.y*oc-v.z*s;
  A.m[5] = (v.y*v.y)*oc+c;
  A.m[6] = v.y*v.z*oc+v.x*v.z;
  A.m[8] = v.x*v.z*oc+v.y*s;
  A.m[9] = v.y*v.z*oc+v.y*s;
  A.m[10] = (v.z*v.z)*oc+c;
  mat4_mult(mat, &A);
}

void mat4_rotate(mat4_t *mat, float angle, const vec3_t *u) {
  mat4_t A = MAT4_IDENTITY;
  vec3_t v = vec3_normalize(u);
  
  // Calculate angle in radians
  angle = angle*180.0f/(float)M_PI;
  float c = (float)cos(angle);
  float s = (float)sin(angle);
  float oc = 1-c;
  A.m[0] = (v.x*v.x)*oc+c;
  A.m[1] = v.y*v.x*oc+v.z*s;
  A.m[2] = v.x*v.z*oc-v.y*v.z;
  A.m[4] = v.x*v.y*oc-v.z*s;
  A.m[5] = (v.y*v.y)*oc+c;
  A.m[6] = v.y*v.z*oc+v.x*v.z;
  A.m[8] = v.x*v.z*oc+v.y*s;
  A.m[9] = v.y*v.z*oc+v.y*s;
  A.m[10] = (v.z*v.z)*oc+c;
  mat4_mult(mat, &A);
}

void mat4_transpose(mat4_t *mat) {
	mat4_t A;
	for(uint32_t c=0; c<4; c++)
		for(uint32_t r=0; r<4; r++)
			A.m[r*4+c] = mat->m[c*4+r];
  memcpy(mat, &A, sizeof(mat4_t));
}

void mat4_inverse(mat4_t *mat) {
	mat4_t A;
	float det = mat4_determinant(mat);
	if(fabs(det) < 0.000001f) return;
	for(uint32_t c=0; c<4; c++)
		for(uint32_t r=0; r<4; r++)
			A.m[c*4+r] = mat4_cofactor(mat, c, r)/det;
	mat4_transpose(&A);
  memcpy(mat, &A, sizeof(mat4_t));
}

float mat4_cofactor(const mat4_t *mat, uint32_t column, uint32_t row) {
	if(column > 3 || row > 3) return 0.0f;
	float cofactor = 0.0f;
	for(int32_t c = 0; c < 4; c++)
	{
		if(c == (int32_t)column) continue;
		float pfactor=1, nfactor=1;
		for(int32_t pcc = c, ncc = c, r = 0; r < 4; r++)
		{
			if(r == (int32_t)row) continue;
			if(pcc == (int32_t)column) pcc++;
			if(pcc > 3) pcc= (((int32_t)column == 0) ? 1 : 0);
			if(ncc== (int32_t)column) ncc--;
			if(ncc < 0) ncc = (((int32_t)column == 3) ? 2 : 3);
			pfactor *= mat->m[pcc*4+r];
			nfactor *= mat->m[ncc*4+r];
			pcc++, ncc--;
		}
		cofactor+=pfactor-nfactor;
	}
	return (((row+column)%2==0) ? cofactor : -1*cofactor);
}

float mat4_determinant(const mat4_t *mat) {
	//Basically, we have to take an arbitrary row (or column), in this case we
	//took the fourth row of the matrix, and multiply each element in that row
	//by it's signed cofactor, which is found by calculating the determinant of
	//the minor of that element, and then summing up all the values.
	return (mat->m[3]*mat4_cofactor(mat, 0, 3)+mat->m[7]*mat4_cofactor(mat, 1, 3)+mat->m[11]*mat4_cofactor(mat, 2, 3)+mat->m[15]*mat4_cofactor(mat, 3, 3));
}

void mat4_mult(mat4_t *mat1, const mat4_t *mat2) {
	mat4_t a;
	for(uint32_t i=0, offset=0, ii=0; i<16; i++, ii++)
	{
		a.m[i]=0, offset=((i%4==0) ? i : offset), ii=((ii>3) ? 0 : ii);
		for(uint32_t j=0, mult=0; j<4; mult++, j++)
		{
			a.m[i] += mat1->m[ii+mult*4] * mat2->m[offset+j];
		}
	}
  memcpy(mat1, &a, sizeof(mat4_t));
}

vec4_t mat4_multv(const mat4_t *mat, const vec4_t *v) {
	vec4_t u;
	u.x = mat->m[0]*v->x + mat->m[4]*v->y + mat->m[8]*v->z + mat->m[12]*v->w;
	u.y = mat->m[1]*v->x + mat->m[5]*v->y + mat->m[9]*v->z + mat->m[13]*v->w;
	u.z = mat->m[2]*v->x + mat->m[6]*v->y + mat->m[10]*v->z + mat->m[14]*v->w;
	u.w = mat->m[3]*v->x + mat->m[7]*v->y + mat->m[11]*v->z + mat->m[15]*v->w;
	return u;
}

void mat4_str(const mat4_t *mat, char *str) {
  int32_t n = 0;
  sprintf(str, "|%10.2f, %10.2f, %10.2f, %10.2f|\n%n", mat->m[0], mat->m[4], mat->m[8], mat->m[12], &n);
  str += n;
  sprintf(str, "|%10.2f, %10.2f, %10.2f, %10.2f|\n%n", mat->m[1], mat->m[5], mat->m[9], mat->m[13], &n);
  str += n;
  sprintf(str, "|%10.2f, %10.2f, %10.2f, %10.2f|\n%n", mat->m[2], mat->m[6], mat->m[10], mat->m[14], &n);
  str += n;
  sprintf(str, "|%10.2f, %10.2f, %10.2f, %10.2f|", mat->m[3], mat->m[7], mat->m[11], mat->m[15]);
}

