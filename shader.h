#ifndef SHADER_H
#define SHADER_H

#include "linalg.h"
#include "external/glad.h"

#define SHADER_INFO_LOG_CAP 1024

typedef GLuint Shader;

bool shader_create_program(Shader *s, const char *vertex_path, const char *fragment_path);
bool shader_compile(const char *source, GLuint *shader, GLenum shader_type);
bool shader_link(GLuint *program);
void shader_use(Shader s);
void shader_set_int(Shader s, const char *uni, int value);
void shader_set_mat4(Shader s, const char *uni, Mat4 value);
void shader_set_vec3(Shader s, const char *uni, Vec3 value);
void shader_set_vec4(Shader s, const char *uni, Vec4 value);

#endif // SHADER_H
