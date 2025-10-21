#ifndef SHADER_H
#define SHADER_H

#include "linalg.h"
#include "external/glad.h"

#define SHADER_INFO_LOG_CAP 1024

typedef GLuint Shader;

Shader shader_create_program(const char *vertex_path, const char *fragment_path);
GLuint shader_compile(const char *source, GLenum shader_type);
void   shader_link(GLuint program);
void   shader_use(Shader s);
void   shader_set_mat4(Shader s, const char *uni, Mat4 value);

#endif // SHADER_H
