#include "linalg.h"

float radians(float degrees)
{
    return degrees * (M_PI / 180.0f);
}

#define vec3(x, y, z) (Vec3){(x), (y), (z)}

Vec3 vec3_add(Vec3 v1, Vec3 v2)
{
    return vec3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

Vec3 vec3_sub(Vec3 v1, Vec3 v2)
{
    return vec3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

Vec3 vec3_scale(Vec3 v1, float value)
{
    return vec3(v1.x * value, v1.y * value, v1.z * value);
}

Vec3 vec3_cross(Vec3 v1, Vec3 v2)
{
    return vec3(v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x);
}

Vec3 vec3_normalize(Vec3 v)
{
    Vec3 result = v;

    float length = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
    if (length != 0.0f)
    {
        float ilength = 1.0f/length;

        result.x *= ilength;
        result.y *= ilength;
        result.z *= ilength;
    }

    return result;
}

float vec3_length(Vec3 v)
{
    return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
}

void vec3_print(Vec3 v)
{
    printf("(%8.2f, %8.2f, %8.2f)\n", v.x, v.y, v.z);
}

Mat4 mat4_identity(void)
{
    return (Mat4){
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}

Mat4 mat4_add(Mat4 left, Mat4 right)
{
    Mat4 result = { 0 };

    result.m0  = left.m0 + right.m0;
    result.m1  = left.m1 + right.m1;
    result.m2  = left.m2 + right.m2;
    result.m3  = left.m3 + right.m3;
    result.m4  = left.m4 + right.m4;
    result.m5  = left.m5 + right.m5;
    result.m6  = left.m6 + right.m6;
    result.m7  = left.m7 + right.m7;
    result.m8  = left.m8 + right.m8;
    result.m9  = left.m9 + right.m9;
    result.m10 = left.m10 + right.m10;
    result.m11 = left.m11 + right.m11;
    result.m12 = left.m12 + right.m12;
    result.m13 = left.m13 + right.m13;
    result.m14 = left.m14 + right.m14;
    result.m15 = left.m15 + right.m15;

    return result;
}

Mat4 mat4_multiply(Mat4 left, Mat4 right)
{
    Mat4 result = {0};

    result.m0  = left.m0*right.m0  + left.m1*right.m4  + left.m2*right.m8   + left.m3*right.m12;
    result.m1  = left.m0*right.m1  + left.m1*right.m5  + left.m2*right.m9   + left.m3*right.m13;
    result.m2  = left.m0*right.m2  + left.m1*right.m6  + left.m2*right.m10  + left.m3*right.m14;
    result.m3  = left.m0*right.m3  + left.m1*right.m7  + left.m2*right.m11  + left.m3*right.m15;
    result.m4  = left.m4*right.m0  + left.m5*right.m4  + left.m6*right.m8   + left.m7*right.m12;
    result.m5  = left.m4*right.m1  + left.m5*right.m5  + left.m6*right.m9   + left.m7*right.m13;
    result.m6  = left.m4*right.m2  + left.m5*right.m6  + left.m6*right.m10  + left.m7*right.m14;
    result.m7  = left.m4*right.m3  + left.m5*right.m7  + left.m6*right.m11  + left.m7*right.m15;
    result.m8  = left.m8*right.m0  + left.m9*right.m4  + left.m10*right.m8  + left.m11*right.m12;
    result.m9  = left.m8*right.m1  + left.m9*right.m5  + left.m10*right.m9  + left.m11*right.m13;
    result.m10 = left.m8*right.m2  + left.m9*right.m6  + left.m10*right.m10 + left.m11*right.m14;
    result.m11 = left.m8*right.m3  + left.m9*right.m7  + left.m10*right.m11 + left.m11*right.m15;
    result.m12 = left.m12*right.m0 + left.m13*right.m4 + left.m14*right.m8  + left.m15*right.m12;
    result.m13 = left.m12*right.m1 + left.m13*right.m5 + left.m14*right.m9  + left.m15*right.m13;
    result.m14 = left.m12*right.m2 + left.m13*right.m6 + left.m14*right.m10 + left.m15*right.m14;
    result.m15 = left.m12*right.m3 + left.m13*right.m7 + left.m14*right.m11 + left.m15*right.m15;

    return result;
}

Mat4 mat4_translate(Vec3 translation)
{
    return (Mat4){
        1.0f, 0.0f, 0.0f, translation.x,
        0.0f, 1.0f, 0.0f, translation.y,
        0.0f, 0.0f, 1.0f, translation.z,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}

Mat4 mat4_scale(Vec3 scale)
{
    return (Mat4){
        scale.x,    0.0f,    0.0f, 0.0f,
        0.0f,    scale.y,    0.0f, 0.0f,
        0.0f,       0.0f, scale.z, 0.0f,
        0.0f,       0.0f,    0.0f, 1.0f
    };
}

Mat4 mat4_rotate(float angle, Vec3 axis)
{
    Mat4 result = {0};

    float x = axis.x, y = axis.y, z = axis.z;

    float len_squared = x*x + y*y + z*z;

    if ((len_squared != 1.0f) && (len_squared != 0.0f))
    {
        float inv_len = 1.0f/sqrtf(len_squared);
        x *= inv_len;
        y *= inv_len;
        z *= inv_len;
    }

    float sin = sinf(angle);
    float cos = cosf(angle);
    float t = 1.0f - cos;

    result.m0 = x*x*t + cos;
    result.m1 = y*x*t + z*sin;
    result.m2 = z*x*t - y*sin;
    result.m3 = 0.0f;

    result.m4 = x*y*t - z*sin;
    result.m5 = y*y*t + cos;
    result.m6 = z*y*t + x*sin;
    result.m7 = 0.0f;

    result.m8  = x*z*t + y*sin;
    result.m9  = y*z*t - x*sin;
    result.m10 = z*z*t + cos;
    result.m11 = 0.0f;

    result.m12 = 0.0f;
    result.m13 = 0.0f;
    result.m14 = 0.0f;
    result.m15 = 1.0f;

    return result;
}

Mat4 mat4_look_at(Vec3 pos, Vec3 target, Vec3 up)
{
    Mat4 result = {0};

    float length = 0.0f;
    float ilength = 0.0f;

    // vec3_sub(pos, target)
    Vec3 vz = { pos.x - target.x, pos.y - target.y, pos.z - target.z };

    // vec3_normalize(vz)
    Vec3 v = vz;
    length = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
    if (length == 0.0f) length = 1.0f;
    ilength = 1.0f/length;
    vz.x *= ilength;
    vz.y *= ilength;
    vz.z *= ilength;

    // vec3_cross(up, vz)
    Vec3 vx = { up.y*vz.z - up.z*vz.y, up.z*vz.x - up.x*vz.z, up.x*vz.y - up.y*vz.x };

    // vec3_normalize(x)
    v = vx;
    length = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
    if (length == 0.0f) length = 1.0f;
    ilength = 1.0f/length;
    vx.x *= ilength;
    vx.y *= ilength;
    vx.z *= ilength;

    // vec3_cross(vz, vx)
    Vec3 vy = { vz.y*vx.z - vz.z*vx.y, vz.z*vx.x - vz.x*vx.z, vz.x*vx.y - vz.y*vx.x };

    result.m0 = vx.x; result.m4 = vx.y; result.m8 = vx.z;  result.m12 = -(vx.x*pos.x + vx.y*pos.y + vx.z*pos.z); // vec3_dot(vx, pos)
    result.m1 = vy.x; result.m5 = vy.y; result.m9 = vy.z;  result.m13 = -(vy.x*pos.x + vy.y*pos.y + vy.z*pos.z); // vec3_dot(vy, pos)
    result.m2 = vz.x; result.m6 = vz.y; result.m10 = vz.z; result.m14 = -(vz.x*pos.x + vz.y*pos.y + vz.z*pos.z); // vec3_dot(vz, pos)
    result.m3 = 0.0f; result.m7 = 0.0f; result.m11 = 0.0f; result.m15 = 1.0f;

    return result;
}

void mat4_print(Mat4 mat)
{
    printf("\n"
        "[[%9.3f, %8.3f, %8.3f, %8.3f]\n"
        " [%9.3f, %8.3f, %8.3f, %8.3f]\n"
        " [%9.3f, %8.3f, %8.3f, %8.3f]\n"
        " [%9.3f, %8.3f, %8.3f, %8.3f]]\n",
        mat.m0, mat.m4,  mat.m8, mat.m12,
        mat.m1, mat.m5,  mat.m9, mat.m13,
        mat.m2, mat.m6, mat.m10, mat.m14,
        mat.m3, mat.m7, mat.m11, mat.m15
    );
}

float16 mat4_to_float(Mat4 mat)
{
    float16 result = {0};

    result.v[0]  = mat.m0;
    result.v[1]  = mat.m1;
    result.v[2]  = mat.m2;
    result.v[3]  = mat.m3;
    result.v[4]  = mat.m4;
    result.v[5]  = mat.m5;
    result.v[6]  = mat.m6;
    result.v[7]  = mat.m7;
    result.v[8]  = mat.m8;
    result.v[9]  = mat.m9;
    result.v[10] = mat.m10;
    result.v[11] = mat.m11;
    result.v[12] = mat.m12;
    result.v[13] = mat.m13;
    result.v[14] = mat.m14;
    result.v[15] = mat.m15;

    return result;
}

Mat4 mat4_perspective(double fov, double aspect, double near_plane, double far_plane)
{
    Mat4 result = {0};

    double top = near_plane*tan(fov*0.5);
    double bottom = -top;
    double right = top*aspect;
    double left = -right;

    // mat4_frustum(-right, right, -top, top, near, far);
    float rl = (float)(right - left);
    float tb = (float)(top - bottom);
    float fn = (float)(far_plane - near_plane);

    result.m0 = ((float)near_plane*2.0f)/rl;
    result.m5 = ((float)near_plane*2.0f)/tb;
    result.m8 = ((float)right + (float)left)/rl;
    result.m9 = ((float)top + (float)bottom)/tb;
    result.m10 = -((float)far_plane + (float)near_plane)/fn;
    result.m11 = -1.0f;
    result.m14 = -((float)far_plane*(float)near_plane*2.0f)/fn;

    return result;
}

Mat4 mat4_ortho(double left, double right, double bottom, double top, double near, double far)
{
    Mat4 result = {0};

    float rl = (float)(right - left);
    float tb = (float)(top - bottom);
    float fn = (float)(far - near);

    result.m0 = 2.0f/rl;
    result.m1 = 0.0f;
    result.m2 = 0.0f;
    result.m3 = 0.0f;
    result.m4 = 0.0f;
    result.m5 = 2.0f/tb;
    result.m6 = 0.0f;
    result.m7 = 0.0f;
    result.m8 = 0.0f;
    result.m9 = 0.0f;
    result.m10 = -2.0f/fn;
    result.m11 = 0.0f;
    result.m12 = -((float)left + (float)right)/rl;
    result.m13 = -((float)top + (float)bottom)/tb;
    result.m14 = -((float)far + (float)near)/fn;
    result.m15 = 1.0f;

    return result;
}
