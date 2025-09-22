#include <stdio.h>

#include "linalg.h"

int main(void)
{
    Mat4 transform = mat4_identity();
    transform = mat4_multiply(transform, mat4_translate(vec3(0.0, 0.0, -3.0)));
    // transform = mat4_scale(transform, (Vec3){0.1, 0.1, 0.1});
    // transform = mat4_rotate(radians(90.0f), (Vec3){0.0, 0.0, 1.0});
    mat4_print(transform);
}
