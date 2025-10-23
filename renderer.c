#include "renderer.h"

#include <stdio.h>
#include <stddef.h>

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_timer.h>

#define STBI_FAILURE_USERMSG
#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

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
    r->wireframes = false;

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

    if (ren->wireframes) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void renderer_present(Renderer *r)
{
    if (!r || !r->window) return;

    SDL_GL_SwapWindow(r->window);
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

Texture texture_load_from_file(const char *filepath)
{
    Texture t = {0};

    int n;

    unsigned char *data = stbi_load(filepath, &t.width, &t.height, &n, 0);

    if (data == 0)
        fprintf(stderr, "[ERROR] Texture: %s '%s'\n", stbi_failure_reason(), filepath);

    glGenTextures(1, &t.id);
    glBindTexture(GL_TEXTURE_2D, t.id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, t.width, t.height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    if (data != 0) printf("[INFO] Texture '%s' was loaded!\n", filepath);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);

    return t;
}

void texture_bind(Texture t, int slot)
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, t.id);
}

void texture_unbind(void)
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

Mesh mesh_create_plane(int width, int height, int subdivisions)
{
    Mesh mesh = {0};

    if (subdivisions < 1) subdivisions = 1;

    int vertices_len = (subdivisions + 1) * (subdivisions + 1);
    int indices_len = subdivisions * subdivisions * 6;

    float half_width = width/2.0;
    float half_height = height/2.0;
    float step_x = width / (float)subdivisions;
    float step_z = height / (float)subdivisions;
    float tex_step_x = 1.0f / (float)subdivisions;
    float tex_step_z = 1.0f / (float)subdivisions;

    Vertex vertices[vertices_len];

    size_t index = 0;

    for (int z = 0; z <= subdivisions; z++)
    {
        for (int x = 0; x <= subdivisions; x++)
        {
            float px = -half_width + (x * step_x);
            float pz = -half_height + (z * step_z);
            float tx = x * tex_step_x;
            float tz = z * tex_step_z;

            vertices[index].position = (Vec3){px, 0.0f, pz};
            vertices[index].normal = (Vec3){0.0f, 1.0f, 0.0f};
            vertices[index].tex_coord = (Vec2){tx, tz};
            vertices[index].color = (Vec4){1.0f, 1.0f, 1.0f, 1.0f};
            index += 1;
        }
    }

    unsigned int indices[indices_len];

    index = 0;

    for (int z = 0; z < subdivisions; z++)
    {
        for (int x = 0; x < subdivisions; x++)
        {
            int top_left = z * (subdivisions + 1) + x;
            int top_right = top_left + 1;
            int bottom_left = (z + 1) * (subdivisions + 1) + x;
            int bottom_right = bottom_left + 1;

            indices[index++] = top_left;
            indices[index++] = bottom_left;
            indices[index++] = top_right;

            indices[index++] = top_right;
            indices[index++] = bottom_right;
            indices[index++] = bottom_left;
        }
    }

    // Vertex vertices[] = {
    //     // Front face (Z+)
    //     {{-half_width, 0.0f, -half_height}, {0.0f, 1.0f, 0.0f} , {1.0f, 1.0f, 1.0f, 1.0f}/*, {0.0f, 0.0f}*/},
    //     {{ half_width, 0.0f, -half_height}, {0.0f, 1.0f, 0.0f} , {1.0f, 1.0f, 1.0f, 1.0f}/*, {1.0f, 0.0f}*/},
    //     {{ half_width, 0.0f,  half_height}, {0.0f, 1.0f, 0.0f} , {1.0f, 1.0f, 1.0f, 1.0f}/*, {1.0f, 1.0f}*/},
    //     {{-half_width, 0.0f,  half_height}, {0.0f, 1.0f, 0.0f} , {1.0f, 1.0f, 1.0f, 1.0f}/*, {0.0f, 1.0f}*/},
    // };

    // unsigned int indices[] = {
    //     0, 1, 2,
    //     2, 3, 0,
    // };

    mesh_init_data(&mesh, vertices, vertices_len, indices, indices_len);

    return mesh;
}

Mesh mesh_create_cube(float size)
{
    Mesh mesh = {0};

    float half = size / 2.0f;

    Vertex vertices[] = {
        // Front face (Z+)
        {{-half, -half,  half}, {0.0f, 0.0f, 1.0f} , {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{ half, -half,  half}, {0.0f, 0.0f, 1.0f} , {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{ half,  half,  half}, {0.0f, 0.0f, 1.0f} , {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{-half,  half,  half}, {0.0f, 0.0f, 1.0f} , {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},

        // Back face (Z-)
        {{ half, -half, -half}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{-half, -half, -half}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{-half,  half, -half}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{ half,  half, -half}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},

        // Right face (X+)
        {{ half, -half,  half}, {1.0f, 0.0f, 0.0f} , {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{ half, -half, -half}, {1.0f, 0.0f, 0.0f} , {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{ half,  half, -half}, {1.0f, 0.0f, 0.0f} , {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{ half,  half,  half}, {1.0f, 0.0f, 0.0f} , {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},

        // Left face (X-)
        {{-half, -half, -half}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{-half, -half,  half}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{-half,  half,  half}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{-half,  half, -half}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},

        // Top face (Y+)
        {{-half,  half,  half}, {0.0f, 1.0f, 0.0f} , {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{ half,  half,  half}, {0.0f, 1.0f, 0.0f} , {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{ half,  half, -half}, {0.0f, 1.0f, 0.0f} , {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{-half,  half, -half}, {0.0f, 1.0f, 0.0f} , {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},

        // Bottom face (Y-)
        {{-half, -half, -half}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{ half, -half, -half}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{ half, -half,  half}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{-half, -half,  half}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}
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
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, tex_coord));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, color));

    glBindVertexArray(0);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        fprintf(stderr, "[ERROR]: Failed to initialize mesh data: %d\n", error);
    }
}

void render_mesh_3d(Renderer *r, Mesh m, Vec3 pos, Vec3 rot, Vec3 scale, Vec4 color)
{
    Mat4 translation = mat4_translate(pos);

    Vec3 x_axis = {1.0f, 0.0f, 0.0f};
    Vec3 y_axis = {0.0f, 1.0f, 0.0f};
    Vec3 z_axis = {0.0f, 0.0f, 1.0f};

    Mat4 rotation_x = mat4_rotate(radians(rot.x), x_axis);
    Mat4 rotation_y = mat4_rotate(radians(rot.y), y_axis);
    Mat4 rotation_z = mat4_rotate(radians(rot.z), z_axis);

    Mat4 rotation = mat4_multiply(rotation_x, rotation_y);
    rotation = mat4_multiply(rotation, rotation_z);

    Mat4 scaled = mat4_scale(scale);

    Mat4 model = mat4_identity();
    model = mat4_multiply(model, rotation);
    model = mat4_multiply(model, translation);
    model = mat4_multiply(model, scaled);

    shader_use(r->shader_3d);

    shader_set_mat4(r->shader_3d, "uModel", model);
    shader_set_mat4(r->shader_3d, "uView", r->camera.view);
    shader_set_mat4(r->shader_3d, "uProjection", r->camera.projection);

    shader_set_vec4(r->shader_3d, "uColor", color);

    glBindVertexArray(m.vao);
    glDrawElements(GL_TRIANGLES, m.indices_len, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
