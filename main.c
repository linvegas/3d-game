#include <stdio.h>
#include <stddef.h>
#include <math.h>

#include "linalg.h"

#define GLAD_GL_IMPLEMENTATION
#include "./external/glad/gl.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

#define VERTICES_CAP (4*1024)
#define SHADER_INFO_LOG_CAP 1024

typedef struct Vertex {
    Vec3 pos;
    Vec4 col;
} Vertex;

size_t vertices_len = 0;

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

int main(void)
{
    if (!SDL_Init(0))
    {
        fprintf(stderr, "%s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("3D", 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (!window)
    {
        fprintf(stderr, "%s\n", SDL_GetError());
        return 1;
    }

    SDL_GLContext context = SDL_GL_CreateContext(window);

    if (context == NULL)
    {
        fprintf(stderr, "%s\n", SDL_GetError());
        return 1;
    }

    int version = gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress);
    printf("[INFO] OpenGL Version: %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

    Vertex vertices[VERTICES_CAP] = {
        // [0] = { .pos = {-0.6, -0.4, 1.0}, .col = {0.8, 0.0, 0.3, 0.0} }, // RED
        // [1] = { .pos = { 0.6, -0.4, 1.0}, .col = {0.3, 0.8, 0.0, 0.0} }, // GREEN
        // [2] = { .pos = { 0.0,  0.6, 1.0}, .col = {0.0, 0.3, 0.8, 0.0} }, // BLUE

        [0] =  { .pos = { -0.5f, -0.5f, -0.5f}, .col = { 1.0f, 0.0f, 0.0f, 1.0f }},
        [1] =  { .pos = {  0.5f, -0.5f, -0.5f}, .col = { 0.0f, 1.0f, 0.0f, 1.0f }},
        [2] =  { .pos = {  0.5f,  0.5f, -0.5f}, .col = { 0.0f, 0.0f, 1.0f, 1.0f }},
        [3] =  { .pos = {  0.5f,  0.5f, -0.5f}, .col = { 0.0f, 0.0f, 1.0f, 1.0f }},
        [4] =  { .pos = { -0.5f,  0.5f, -0.5f}, .col = { 0.0f, 1.0f, 0.0f, 1.0f }},
        [5] =  { .pos = { -0.5f, -0.5f, -0.5f}, .col = { 1.0f, 0.0f, 0.0f, 1.0f }},

        [6] =  { .pos = { -0.5f, -0.5f,  0.5f}, .col = { 1.0f, 0.0f, 0.0f, 1.0f }},
        [7] =  { .pos = {  0.5f, -0.5f,  0.5f}, .col = { 0.0f, 1.0f, 0.0f, 1.0f }},
        [8] =  { .pos = {  0.5f,  0.5f,  0.5f}, .col = { 0.0f, 0.0f, 1.0f, 1.0f }},
        [9] =  { .pos = {  0.5f,  0.5f,  0.5f}, .col = { 0.0f, 0.0f, 1.0f, 1.0f }},
        [10] = { .pos = { -0.5f,  0.5f,  0.5f}, .col = { 0.0f, 1.0f, 0.0f, 1.0f }},
        [11] = { .pos = { -0.5f, -0.5f,  0.5f}, .col = { 1.0f, 0.0f, 0.0f, 1.0f }},

        [12] = { .pos = { -0.5f,  0.5f,  0.5f}, .col = { 1.0f, 0.0f, 0.0f, 1.0f }},
        [13] = { .pos = { -0.5f,  0.5f, -0.5f}, .col = { 0.0f, 1.0f, 0.0f, 1.0f }},
        [14] = { .pos = { -0.5f, -0.5f, -0.5f}, .col = { 0.0f, 0.0f, 1.0f, 1.0f }},
        [15] = { .pos = { -0.5f, -0.5f, -0.5f}, .col = { 0.0f, 0.0f, 1.0f, 1.0f }},
        [16] = { .pos = { -0.5f, -0.5f,  0.5f}, .col = { 0.0f, 1.0f, 0.0f, 1.0f }},
        [17] = { .pos = { -0.5f,  0.5f,  0.5f}, .col = { 1.0f, 0.0f, 0.0f, 1.0f }},

        [18] = { .pos = {  0.5f,  0.5f,  0.5f}, .col = { 1.0f, 0.0f, 0.0f, 1.0f }},
        [19] = { .pos = {  0.5f,  0.5f, -0.5f}, .col = { 0.0f, 1.0f, 0.0f, 1.0f }},
        [20] = { .pos = {  0.5f, -0.5f, -0.5f}, .col = { 0.0f, 0.0f, 1.0f, 1.0f }},
        [21] = { .pos = {  0.5f, -0.5f, -0.5f}, .col = { 0.0f, 0.0f, 1.0f, 1.0f }},
        [22] = { .pos = {  0.5f, -0.5f,  0.5f}, .col = { 0.0f, 1.0f, 0.0f, 1.0f }},
        [23] = { .pos = {  0.5f,  0.5f,  0.5f}, .col = { 1.0f, 0.0f, 0.0f, 1.0f }},

        [25] = { .pos = {  0.5f, -0.5f, -0.5f}, .col = { 1.0f, 0.0f, 0.0f, 1.0f }},
        [24] = { .pos = { -0.5f, -0.5f, -0.5f}, .col = { 0.0f, 1.0f, 0.0f, 1.0f }},
        [26] = { .pos = {  0.5f, -0.5f,  0.5f}, .col = { 0.0f, 0.0f, 1.0f, 1.0f }},
        [27] = { .pos = {  0.5f, -0.5f,  0.5f}, .col = { 0.0f, 0.0f, 1.0f, 1.0f }},
        [28] = { .pos = { -0.5f, -0.5f,  0.5f}, .col = { 0.0f, 1.0f, 0.0f, 1.0f }},
        [29] = { .pos = { -0.5f, -0.5f, -0.5f}, .col = { 1.0f, 0.0f, 0.0f, 1.0f }},

        [30] = { .pos = { -0.5f,  0.5f, -0.5f}, .col = { 1.0f, 0.0f, 0.0f, 1.0f }},
        [31] = { .pos = {  0.5f,  0.5f, -0.5f}, .col = { 0.0f, 1.0f, 0.0f, 1.0f }},
        [32] = { .pos = {  0.5f,  0.5f,  0.5f}, .col = { 0.0f, 0.0f, 1.0f, 1.0f }},
        [33] = { .pos = {  0.5f,  0.5f,  0.5f}, .col = { 0.0f, 0.0f, 1.0f, 1.0f }},
        [34] = { .pos = { -0.5f,  0.5f,  0.5f}, .col = { 0.0f, 1.0f, 0.0f, 1.0f }},
        [35] = { .pos = { -0.5f,  0.5f, -0.5f}, .col = { 1.0f, 0.0f, 0.0f, 1.0f }}
    };

    vertices_len = 36;

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*VERTICES_CAP, vertices, GL_DYNAMIC_DRAW);

    GLuint shader = compile_shaders();

    GLint vpos_loc = glGetAttribLocation(shader, "vertexPos");
    GLint vcol_loc = glGetAttribLocation(shader, "vertexColor");

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glEnableVertexAttribArray(vpos_loc);
    glVertexAttribPointer(vpos_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, pos));

    glEnableVertexAttribArray(vcol_loc);
    glVertexAttribPointer(vcol_loc, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, col));

    int running = 1;
    int screen_w = 800;
    int screen_h = 600;

    glEnable(GL_DEPTH_TEST);

    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT) running = 0;
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) running = 0;
        }

        SDL_GetWindowSize(window, &screen_w, &screen_h);

        Mat4 transform = mat4_rotate(SDL_GetTicks()/1000.0f * radians(50.0), vec3(0.5, 1.0, 0.0));

        Mat4 model = mat4_identity();
        model = mat4_multiply(model, transform);

        Mat4 view = mat4_identity();
        view = mat4_add(view, mat4_translate(vec3(0.0, 0.0, -7.0)));

        Mat4 projection = mat4_identity();
        double fov = radians(45.0);
        double aspect = screen_w/screen_h;
        double near = 0.1f;
        double far = 100.0f;
        projection = mat4_multiply(projection, mat4_perspective(fov, aspect, near, far));

        glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, mat4_to_float(model).v);
        glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, mat4_to_float(view).v);
        glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, mat4_to_float(projection).v);

        glUseProgram(shader);

        glViewport(0, 0, screen_w, screen_h);

        glClearColor(0.05, 0.05, 0.05, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * vertices_len, vertices);

        glBindVertexArray(vao);

        glDrawArrays(GL_TRIANGLES, 0, vertices_len);

        SDL_GL_SwapWindow(window);
    }

    return 0;
}
