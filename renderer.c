#include "renderer.h"

#include <stdio.h>
#include <stddef.h>

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_timer.h>

bool renderer_init(Renderer *r, const char *title, int width, int height)
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        fprintf(stderr, "%s\n", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    SDL_Window *window = SDL_CreateWindow(
        title, width, height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );

    if (!window)
    {
        fprintf(stderr, "%s\n", SDL_GetError());
        return false;
    }

    r->window = window;

    SDL_GLContext context = SDL_GL_CreateContext(r->window);

    if (context == NULL)
    {
        fprintf(stderr, "%s\n", SDL_GetError());
        return false;
    }

    gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress);

    if (!SDL_SetWindowRelativeMouseMode(r->window, true))
    {
        fprintf(stderr, "%s\n", SDL_GetError());
        return false;
    }

    Shader shader_3d;
    if (!shader_create_program(&shader_3d, "shader.vert", "shader.frag")) return false;
    r->shader_3d = shader_3d;

    Camera camera = {0};
    camera.fov = radians(65.0);
    camera.aspect = width/height;
    camera.near = 0.1f;
    camera.far = 100.0f;

    camera.position = vec3(0, 0, 6.0);
    camera.target = vec3(0, 0, -1.0);
    camera.up = vec3(0, 1.0, 0);

    r->camera = camera;

    r->width = width;
    r->height = height;

    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);

    printf("[INFO] OpenGL Version: %s\n", glGetString(GL_VERSION));
    printf("[INFO] GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    return true;
}

void renderer_clear(Renderer *ren, float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    SDL_GetWindowSize(ren->window, &ren->width, &ren->height);
    glViewport(0, 0, ren->width, ren->height);
}

void renderer_present(Renderer *r)
{
    if (!r || !r->window) return;

    SDL_GL_SwapWindow(r->window);
}

Mesh mesh_create_cube(float size)
{
    Mesh mesh = {0};

    float half = size / 2.0f;

    Vertex vertices[] = {
        // Front face (Z+)
        {{-half, -half,  half}, {1.0f, 1.0f, 1.0f, 1.0f}/*, {0.0f, 0.0f, 1.0f} , {0.0f, 0.0f}*/},
        {{ half, -half,  half}, {1.0f, 1.0f, 1.0f, 1.0f}/*, {0.0f, 0.0f, 1.0f} , {1.0f, 0.0f}*/},
        {{ half,  half,  half}, {1.0f, 1.0f, 1.0f, 1.0f}/*, {0.0f, 0.0f, 1.0f} , {1.0f, 1.0f}*/},
        {{-half,  half,  half}, {1.0f, 1.0f, 1.0f, 1.0f}/*, {0.0f, 0.0f, 1.0f} , {0.0f, 1.0f}*/},

        // Back face (Z-)
        {{ half, -half, -half}, {1.0f, 1.0f, 1.0f, 1.0f}/*, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}*/},
        {{-half, -half, -half}, {1.0f, 1.0f, 1.0f, 1.0f}/*, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}*/},
        {{-half,  half, -half}, {1.0f, 1.0f, 1.0f, 1.0f}/*, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}*/},
        {{ half,  half, -half}, {1.0f, 1.0f, 1.0f, 1.0f}/*, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}*/},

        // Right face (X+)
        {{ half, -half,  half}, {1.0f, 1.0f, 1.0f, 1.0f}/*, {1.0f, 0.0f, 0.0f} , {0.0f, 0.0f}*/},
        {{ half, -half, -half}, {1.0f, 1.0f, 1.0f, 1.0f}/*, {1.0f, 0.0f, 0.0f} , {1.0f, 0.0f}*/},
        {{ half,  half, -half}, {1.0f, 1.0f, 1.0f, 1.0f}/*, {1.0f, 0.0f, 0.0f} , {1.0f, 1.0f}*/},
        {{ half,  half,  half}, {1.0f, 1.0f, 1.0f, 1.0f}/*, {1.0f, 0.0f, 0.0f} , {0.0f, 1.0f}*/},

        // Left face (X-)
        {{-half, -half, -half}, {1.0f, 1.0f, 1.0f, 1.0f}/*, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}*/},
        {{-half, -half,  half}, {1.0f, 1.0f, 1.0f, 1.0f}/*, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}*/},
        {{-half,  half,  half}, {1.0f, 1.0f, 1.0f, 1.0f}/*, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}*/},
        {{-half,  half, -half}, {1.0f, 1.0f, 1.0f, 1.0f}/*, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}*/},

        // Top face (Y+)
        {{-half,  half,  half}, {1.0f, 1.0f, 1.0f, 1.0f}/*, {0.0f, 1.0f, 0.0f} , {0.0f, 0.0f}*/},
        {{ half,  half,  half}, {1.0f, 1.0f, 1.0f, 1.0f}/*, {0.0f, 1.0f, 0.0f} , {1.0f, 0.0f}*/},
        {{ half,  half, -half}, {1.0f, 1.0f, 1.0f, 1.0f}/*, {0.0f, 1.0f, 0.0f} , {1.0f, 1.0f}*/},
        {{-half,  half, -half}, {1.0f, 1.0f, 1.0f, 1.0f}/*, {0.0f, 1.0f, 0.0f} , {0.0f, 1.0f}*/},

        // Bottom face (Y-)
        {{-half, -half, -half}, {1.0f, 1.0f, 1.0f, 1.0f}/*, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}*/},
        {{ half, -half, -half}, {1.0f, 1.0f, 1.0f, 1.0f}/*, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}*/},
        {{ half, -half,  half}, {1.0f, 1.0f, 1.0f, 1.0f}/*, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}*/},
        {{-half, -half,  half}, {1.0f, 1.0f, 1.0f, 1.0f}/*, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}*/}
    };

    unsigned int indices[] = {
        0, 1, 2, 2, 3, 0,       // Front
        4, 5, 6, 6, 7, 4,       // Back
        8, 9, 10, 10, 11, 8,    // Right
        12, 13, 14, 14, 15, 12, // Left
        16, 17, 18, 18, 19, 16, // Top
        20, 21, 22, 22, 23, 20  // Bottom
    };

    mesh_init_data(&mesh, vertices, 24, indices, 36);

    return mesh;
}

void mesh_init_data(Mesh *m, Vertex *vertices, size_t vertices_len, unsigned int *indices, size_t indices_len)
{
    m->vertices_len = vertices_len;
    m->indices_len = indices_len;

    // Generate and bind VAO
    glGenVertexArrays(1, &m->vao);
    glBindVertexArray(m->vao);

    // Generate and bind VBO
    glGenBuffers(1, &m->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices_len * sizeof(Vertex), vertices, GL_STATIC_DRAW);

    // Generate and bind EBO
    glGenBuffers(1, &m->ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_len * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, color));

    glBindVertexArray(0);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        fprintf(stderr, "[ERROR]: Failed to initialize mesh data: %d\n", error);
    }
}

void render_model_3d(Renderer *r, Mesh m, Vec3 pos, Vec3 rot, Vec3 scale)
{
    Mat4 translation = mat4_translate(pos);

    // Vec3 x_axis = {1.0f, 0.0f, 0.0f};
    // Vec3 y_axis = {0.0f, 1.0f, 0.0f};
    // Vec3 z_axis = {0.0f, 0.0f, 1.0f};
    //
    // Mat4 rotation_x = mat4_rotate(SDL_GetTicks()/1000.0f * radians(rot.x), x_axis);
    // Mat4 rotation_y = mat4_rotate(SDL_GetTicks()/1000.0f * radians(rot.y), y_axis);
    // Mat4 rotation_z = mat4_rotate(SDL_GetTicks()/1000.0f * radians(rot.z), z_axis);
    //
    // Mat4 rotation = mat4_multiply(rotation_z, rotation_y);
    // rotation = mat4_multiply(rotation, rotation_x);

    Mat4 rotation = mat4_rotate(
        SDL_GetTicks()/1000.0f * radians(50.0), rot
    );

    Mat4 scaled = mat4_scale(scale);

    Mat4 model = mat4_identity();
    model = mat4_multiply(model, rotation);
    model = mat4_multiply(model, translation);
    model = mat4_multiply(model, scaled);

    shader_use(r->shader_3d);

    shader_set_mat4(r->shader_3d, "model", model);
    shader_set_mat4(r->shader_3d, "view", r->camera.view);
    shader_set_mat4(r->shader_3d, "projection", r->camera.projection);

    glBindVertexArray(m.vao);
    glDrawElements(GL_TRIANGLES, m.indices_len, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void renderer_camera_update(Renderer *r)
{
    r->camera.aspect = r->width/r->height;

    // View
    Mat4 view = mat4_identity();
    Mat4 look = mat4_look_at(
        r->camera.position,
        vec3_add(r->camera.position, r->camera.target),
        r->camera.up
    );
    r->camera.view = mat4_multiply(view, look);

    // 3D Projection
    Mat4 projection = mat4_identity();
    Mat4 perspective = mat4_perspective(
        r->camera.fov, r->camera.aspect,
        r->camera.near, r->camera.far
    );
    r->camera.projection = mat4_multiply(projection, perspective);
}
