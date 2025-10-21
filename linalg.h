#ifndef LINALG_H
#define LINALG_H

#include <math.h>
#include <stdio.h>

float radians(float degrees);

typedef struct Vec3 { float x; float y; float z; } Vec3;
typedef struct Vec4 { float x; float y; float z; float w; } Vec4;

#define vec3(x, y, z) (Vec3){(x), (y), (z)}

Vec3 vec3_add(Vec3 v1, Vec3 v2);
Vec3 vec3_sub(Vec3 v1, Vec3 v2);
Vec3 vec3_scale(Vec3 v1, float value);
Vec3 vec3_normalize(Vec3 v);
Vec3 vec3_cross(Vec3 v1, Vec3 v2);

typedef struct Mat4 {
    float m0, m4,  m8, m12;
    float m1, m5,  m9, m13;
    float m2, m6, m10, m14;
    float m3, m7, m11, m15;
} Mat4;

typedef struct float16 { float v[16]; } float16;

float16 mat4_to_float(Mat4 mat);
Mat4 mat4_translate(Vec3 translation);
Mat4 mat4_rotate(float angle, Vec3 axis);
Mat4 mat4_scale(Vec3 scale);
Mat4 mat4_identity(void);
Mat4 mat4_multiply(Mat4 left, Mat4 right);
Mat4 mat4_look_at(Vec3 pos, Vec3 target, Vec3 up);
Mat4 mat4_perspective(double fov, double aspect, double near_plane, double far_plane);

#endif // LINALG_H
// vim:ft=c
