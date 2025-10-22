#ifndef RENDERER_H
#define RENDERER_H

#include <SDL3/SDL_video.h>
#include "external/glad.h"

#include "linalg.h"
#include "shader.h"

typedef struct {
    Vec3  position;
    Vec3  target;
    Vec3  up;
    float fov;
    float aspect;
    float near;
    float far;
    Mat4  view;
    Mat4  projection;
} Camera;

typedef struct {
    SDL_Window *window;
    Camera camera;
    int width;
    int height;
    Shader shader_3d;
} Renderer;

bool renderer_init(Renderer *r, const char *title, int width, int height);
void renderer_clear(Renderer *ren, float r, float g, float b, float a);
void renderer_present(Renderer *r);

void renderer_camera_update(Renderer *r);

typedef struct Vertex {
    Vec3 position;
    // Vec3 normal;
    Vec4 color;
} Vertex;

typedef struct {
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    size_t indices_len;
    size_t vertices_len;
} Mesh;

void mesh_init_data(Mesh *m, Vertex *vertices, size_t vertices_len, unsigned int *indices, size_t indices_len);
Mesh mesh_create_cube(float size);
void render_model_3d(Renderer *r, Mesh m, Vec3 pos, Vec3 rot, Vec3 scale);

#endif // RENDERER_H
// vim:ft=c
