#include "shader.h"

#include <stdio.h>

const char *vertex_shader_src =
    "#version 330\n"
    "layout (location = 0) in vec3 vPos;\n"
    "layout (location = 1) in vec3 vNormal;\n"
    "layout (location = 2) in vec2 vTexCoord;\n"
    "layout (location = 3) in vec4 vColor;\n"
    "out vec3 fPos;\n"
    "out vec3 fNormal;\n"
    "out vec2 fTexCoord;\n"
    "out vec4 fColor;\n"
    "uniform mat4 uModel;\n"
    "uniform mat4 uView;\n"
    "uniform mat4 uProjection;\n"
    "\n"
    "void main()\n"
    "{\n"
    "   gl_Position = uProjection * uView * uModel * vec4(vPos, 1.0);\n"
    "   // gl_Position = vec4(vPos, 1.0);\n"
    "   fPos = vec3(uModel * vec4(vPos, 1.0));\n"
    "   fNormal = mat3(transpose(inverse(uModel))) * vNormal;\n"
    "   fTexCoord = vTexCoord;\n"
    "   fColor = vColor;\n"
    "}\n";

const char *frag_shader_src =
    "#version 330 core\n"
    "in vec3 fPos;\n"
    "in vec3 fNormal;\n"
    "in vec2 fTexCoord;\n"
    "in vec4 fColor;\n"
    "out vec4 FragColor;\n"
    "uniform sampler2D uTexture;\n"
    "uniform bool uUseTexture;\n"
    "uniform vec3 uLightPos;\n"
    "uniform vec4 uColor;\n"
    "\n"
    "void main() {\n"
    "    vec3 lightColor = vec3(1.0);\n"
    "    float ambientStrength = 0.1;\n"
    "    vec3 ambient = ambientStrength * lightColor;;\n"
    "    vec3 norm = normalize(fNormal);\n"
    "    vec3 lightDir = normalize(uLightPos - fPos);\n"
    "    float diff = max(dot(norm, lightDir), 0.0);\n"
    "    vec3 diffuse = diff * lightColor;\n"
    "    vec4 texColor = uUseTexture ? texture(uTexture, fTexCoord) : vec4(1.0);\n"
    "    vec3 light = ambient + diffuse;\n"
    "    FragColor = vec4(light, 1.0) * texColor * uColor;\n"
    "}\n";

bool shader_compile(const char *source, GLuint *shader, GLenum shader_type)
{
    *shader = glCreateShader(shader_type);

    glShaderSource(*shader, 1, &source, NULL);
    glCompileShader(*shader);

    int success;

    glGetShaderiv(*shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        char info_log[SHADER_INFO_LOG_CAP];
        int info_log_len = 0;
        glGetShaderInfoLog(*shader, SHADER_INFO_LOG_CAP, &info_log_len, info_log);
        fprintf(stderr, "[ERROR] Failed to compile %s shader\n", shader_type == GL_VERTEX_SHADER ? "Vertex" : "Fragment");
        fprintf(stderr, "%.*s\n", info_log_len, info_log);
        return false;
    }

    return true;
}

bool shader_link(GLuint *program)
{
    glLinkProgram(*program);

    int success;

    glGetProgramiv(*program, GL_LINK_STATUS, &success);

    if (!success) {
        char info_log[SHADER_INFO_LOG_CAP];
        int info_log_len = 0;
        glGetProgramInfoLog(*program, SHADER_INFO_LOG_CAP, &info_log_len, info_log);
        fprintf(stderr, "[ERROR] Failed to link shaders\n");
        fprintf(stderr, "%.*s", info_log_len, info_log);
        return false;
    }

    return true;
}

bool shader_create_program(Shader *s, const char *vertex_path, const char *fragment_path)
{
    (void)vertex_path;
    (void)fragment_path;

    *s = glCreateProgram();

    GLuint vertex_shader;
    GLuint fragment_shader;

    if (!shader_compile(vertex_shader_src, &vertex_shader, GL_VERTEX_SHADER)) return false;
    if (!shader_compile(frag_shader_src, &fragment_shader, GL_FRAGMENT_SHADER)) return false;

    glAttachShader(*s, vertex_shader);
    glAttachShader(*s, fragment_shader);

    if (!shader_link(s)) return false;

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return true;
}

void shader_use(Shader s)
{
    glUseProgram(s);
}

void shader_set_mat4(Shader s, const char *uni, Mat4 value)
{
    glUniformMatrix4fv(glGetUniformLocation(s, uni), 1, GL_FALSE, mat4_to_float(value).v);
}

void shader_set_vec3(Shader s, const char *uni, Vec3 value)
{
    glUniform3f(glGetUniformLocation(s, uni), value.x, value.y, value.z);
}

void shader_set_vec4(Shader s, const char *uni, Vec4 value)
{
    glUniform4f(glGetUniformLocation(s, uni), value.x, value.y, value.z, value.w);
}

void shader_set_int(Shader s, const char *uni, int value)
{
    glUniform1i(glGetUniformLocation(s, uni), value);
}
