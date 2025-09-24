#include <stdio.h>

#include "./external/glad/gl.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

#define SHADER_INFO_LOG_CAP 1024

const char *vertex_shader_src =
#if defined(GRAPHICS_API_OPENGL_21)
    "#version 120\n"
    "attribute vec3 vertexPos;\n"
    "attribute vec4 vertexColor;\n"
    "varying vec4 fragColor;\n"
#else
    "#version 430\n"
    "in vec3 vertexPos;\n"
    "in vec4 vertexColor;\n"
    "out vec4 fragColor;\n"
#endif
    "uniform mat4 transform;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "\n"
    "void main()\n"
    "{\n"
    "   gl_Position = projection * view * model * vec4(vertexPos, 1.0);\n"
    "   // gl_Position = vec4(vertexPos, 1.0);\n"
    "   fragColor = vertexColor;\n"
    "}\n";

const char *frag_shader_src =
#if defined(GRAPHICS_API_OPENGL_21)
    "#version 120\n"
    "varying vec4 fragColor;\n"
    "void main()\n"
    "{\n"
    "    gl_FragColor = fragColor;\n"
    "}\n";
#else
    "#version 430 core\n"
    "in vec4 fragColor;\n"
    "out vec4 FragColor;\n"
    "\n"
    "void main() {\n"
    "    FragColor = fragColor;\n"
    "}\n";
#endif

GLuint compile_shaders(void)
{
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, (const char **)&vertex_shader_src, NULL);
    glCompileShader(vertex_shader);

    int success;
    char info_log[SHADER_INFO_LOG_CAP];

    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex_shader, SHADER_INFO_LOG_CAP, NULL, info_log);
        fprintf(stderr, "[ERROR] Vertex shader:\n%s", info_log);
    }

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, (const char **)&frag_shader_src, NULL);
    glCompileShader(fragment_shader);

    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment_shader, SHADER_INFO_LOG_CAP, NULL, info_log);
        fprintf(stderr, "[ERROR] Fragment shader:\n%s", info_log);
    }

    GLuint shader_prg = glCreateProgram();

    glAttachShader(shader_prg, vertex_shader);
    glAttachShader(shader_prg, fragment_shader);
    glLinkProgram(shader_prg);

    glGetProgramiv(shader_prg, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader_prg, SHADER_INFO_LOG_CAP, NULL, info_log);
        fprintf(stderr, "ERROR: Shader linking:\n%s", info_log);
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return shader_prg;
}

