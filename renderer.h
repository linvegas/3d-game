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

// TODO: Introduce a Light struct
typedef struct {
    SDL_Window *window;
    Camera camera;
    int width;
    int height;
    Shader shader_2d;
    Shader shader_3d;
    bool wireframes;
} Renderer;

bool renderer_init(Renderer *r, const char *title, int width, int height);
void renderer_clear(Renderer *ren, float r, float g, float b, float a);
void renderer_present(Renderer *r);

void render_begin_2d(Renderer *r);
void render_end_2d(Renderer *r);
void render_rect_2d(Renderer *r, int x, int y, int w, int h, Vec4 color);

void renderer_camera_update(Renderer *r);

typedef struct {
    GLuint id;
    int width;
    int height;
} Texture;

Texture texture_load_from_file(const char *filepath);
void    texture_bind(Texture t, int slot);
void    texture_unbind(void);

typedef struct Vertex {
    Vec3 position;
    Vec3 normal;
    Vec2 tex_coord;
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
Mesh mesh_create_plane(int width, int height, int subdivisions);
Mesh mesh_create_cube(float size);
void render_mesh_3d(Renderer *r, Mesh m, Vec3 pos, Vec3 rot, Vec3 scale, Vec4 color);

#endif // RENDERER_H
// vim:ft=c
