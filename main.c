#include <stdio.h>
#include <stddef.h>
#include <math.h>

#include "linalg.h"

#define GLAD_GL_IMPLEMENTATION
#include "./external/glad/gl.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

#define VERTICES_CAP (4*1024)
#define FACTOR 80
#define SCREEN_WIDTH FACTOR*16
#define SCREEN_HEIGHT FACTOR*9

GLuint compile_shaders(void);

typedef struct Vertex {
    Vec3 pos;
    Vec4 col;
} Vertex;

typedef struct Vertices {
    Vertex items[VERTICES_CAP];
    size_t len;
} Vertices;

void vertices_push(Vertices *vert, Vertex vtx)
{
    if (vert->len <= VERTICES_CAP) vert->items[vert->len++] = vtx;
}

typedef struct Renderer {
    GLuint vao;
    GLuint vbo;
    GLuint shader;
    Vertices vertices;
} Renderer;

Renderer renderer = {0};

void renderer_init(void)
{
    glGenBuffers(1, &renderer.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, renderer.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*VERTICES_CAP, renderer.vertices.items, GL_DYNAMIC_DRAW);

    renderer.shader = compile_shaders();

    GLint vpos_loc = glGetAttribLocation(renderer.shader, "vertexPos");
    GLint vcol_loc = glGetAttribLocation(renderer.shader, "vertexColor");

    glGenVertexArrays(1, &renderer.vao);
    glBindVertexArray(renderer.vao);

    glEnableVertexAttribArray(vpos_loc);
    glVertexAttribPointer(vpos_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, pos));

    glEnableVertexAttribArray(vcol_loc);
    glVertexAttribPointer(vcol_loc, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, col));
}

int running = 1;
int screen_w = 800;
int screen_h = 600;

Vec3 cam_pos   = vec3(0, 0, 6.0);
Vec3 cam_front = vec3(0, 0, -1.0);
Vec3 cam_up    = vec3(0, 1.0, 0);

float yaw = -90.0f;
float pitch = 0.0f;
float last_x = 800.0 / 2.0;
float last_y = 600.0 / 2.0;
int first_mouse = 1;

float vel = 5.0f;
float delta = 0.0f;

void render_3d_scene(void)
{
    glClearColor(0.05, 0.05, 0.05, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(renderer.shader);

    glViewport(0, 0, screen_w, screen_h);

    // Camera
    Mat4 view = mat4_identity();
    Mat4 look = mat4_look_at(
        cam_pos,
        vec3_add(cam_pos, cam_front),
        cam_up
    );
    view = mat4_multiply(view, look);

    // 3D projection
    Mat4 projection = mat4_identity();
    double fov = radians(65.0);
    double aspect = screen_w/screen_h;
    double near = 0.1f;
    double far = 100.0f;
    projection = mat4_multiply(projection, mat4_perspective(fov, aspect, near, far));

    for (int i = 0; i < 10; i++)
    {
        Mat4 model = mat4_identity();
        Mat4 transform = mat4_rotate(
            SDL_GetTicks()/1000.0f * radians(50.0),
            vec3(0.4, 0.6, 0.0)
        );
        Mat4 translate = mat4_translate(vec3(i*3.0, 1.0, 1.0));
        model = mat4_multiply(model, mat4_multiply(transform, translate));

        glUniformMatrix4fv(glGetUniformLocation(renderer.shader, "model"), 1, GL_FALSE, mat4_to_float(model).v);
        glUniformMatrix4fv(glGetUniformLocation(renderer.shader, "view"), 1, GL_FALSE, mat4_to_float(view).v);
        glUniformMatrix4fv(glGetUniformLocation(renderer.shader, "projection"), 1, GL_FALSE, mat4_to_float(projection).v);

        // glBindBuffer(GL_ARRAY_BUFFER, renderer.vbo);
        // glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * renderer.vertices.len, renderer.vertices.items);

        glBindVertexArray(renderer.vao);

        glDrawArrays(GL_TRIANGLES, 0, renderer.vertices.len);
    }
}

void handle_input(void)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT) running = 0;
        if (event.type == SDL_EVENT_KEY_DOWN)
        {
            if (event.key.key == SDLK_ESCAPE) running = 0;

            if (event.key.key == SDLK_W)
                cam_pos = vec3_add(cam_pos, vec3_scale(cam_front, delta*vel));
            if (event.key.key == SDLK_S)
                cam_pos = vec3_sub(cam_pos, vec3_scale(cam_front, delta*vel));
            if (event.key.key == SDLK_A)
                cam_pos = vec3_sub(cam_pos, vec3_scale(vec3_normalize(vec3_cross(cam_front, cam_up)), delta*vel));
            if (event.key.key == SDLK_D)
                cam_pos = vec3_add(cam_pos, vec3_scale(vec3_normalize(vec3_cross(cam_front, cam_up)), delta*vel));
        }
        if (event.type == SDL_EVENT_MOUSE_MOTION)
        {
            // TODO: Weird flickness while moving the camera and walking at the same time
            if (first_mouse)
            {
                last_x = event.motion.x;
                last_y = event.motion.y;
                first_mouse = 0;
            }

            float mouse_x = last_x + event.motion.xrel;
            float mouse_y = last_y + event.motion.yrel;

            float xoffset = mouse_x - last_x;
            float yoffset = last_y - mouse_y;
            last_x = mouse_x;
            last_y = mouse_y;

            float sensitivity = 0.2f;
            xoffset *= sensitivity;
            yoffset *= sensitivity;

            yaw += xoffset;
            pitch += yoffset;

            if (pitch > 89.0f) pitch = 89.0f;
            if (pitch < -89.0f) pitch = -89.0f;

            Vec3 front = {0};
            front.x = cosf(radians(yaw)) * cosf(radians(pitch));
            front.y = sinf(radians(pitch));
            front.z = sinf(radians(yaw)) * cosf(radians(pitch));
            cam_front = vec3_normalize(front);

            // printf("x: %f  ", event.motion.x);
            // printf("y: %f  ", event.motion.y);
            // printf("xrel: %f  ", event.motion.xrel);
            // printf("yrel: %f\n", event.motion.yrel);
        }
    }
}

// TODO: Implement a game struct
int main(void)
{
    if (!SDL_Init(0))
    {
        fprintf(stderr, "%s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("3D", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

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

    if (!SDL_SetWindowRelativeMouseMode(window, true))
    {
        fprintf(stderr, "%s\n", SDL_GetError());
        return 1;
    }

    int version = gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress);
    printf("[INFO] OpenGL Version: %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

    // TODO: Setup a draw cube function for this
    vertices_push(&renderer.vertices, (Vertex){.pos = {-0.5f, -0.5f, -0.5f}, .col = { 0.0f, 1.0f, 0.0f, 1.0f}});
    vertices_push(&renderer.vertices, (Vertex){.pos = { 0.5f, -0.5f, -0.5f}, .col = { 1.0f, 0.0f, 0.0f, 1.0f}});
    vertices_push(&renderer.vertices, (Vertex){.pos = { 0.5f,  0.5f, -0.5f}, .col = { 0.0f, 1.0f, 0.0f, 1.0f}});
    vertices_push(&renderer.vertices, (Vertex){.pos = { 0.5f,  0.5f, -0.5f}, .col = { 0.0f, 1.0f, 0.0f, 1.0f}});
    vertices_push(&renderer.vertices, (Vertex){.pos = {-0.5f,  0.5f, -0.5f}, .col = { 1.0f, 0.0f, 0.0f, 1.0f}});
    vertices_push(&renderer.vertices, (Vertex){.pos = {-0.5f, -0.5f, -0.5f}, .col = { 0.0f, 1.0f, 0.0f, 1.0f}});

    vertices_push(&renderer.vertices, (Vertex){.pos = {-0.5f, -0.5f,  0.5f}, .col = { 0.0f, 0.0f, 1.0f, 1.0f}});
    vertices_push(&renderer.vertices, (Vertex){.pos = { 0.5f, -0.5f,  0.5f}, .col = { 0.0f, 1.0f, 0.0f, 1.0f}});
    vertices_push(&renderer.vertices, (Vertex){.pos = { 0.5f,  0.5f,  0.5f}, .col = { 0.0f, 0.0f, 1.0f, 1.0f}});
    vertices_push(&renderer.vertices, (Vertex){.pos = { 0.5f,  0.5f,  0.5f}, .col = { 0.0f, 0.0f, 1.0f, 1.0f}});
    vertices_push(&renderer.vertices, (Vertex){.pos = {-0.5f,  0.5f,  0.5f}, .col = { 0.0f, 1.0f, 0.0f, 1.0f}});
    vertices_push(&renderer.vertices, (Vertex){.pos = {-0.5f, -0.5f,  0.5f}, .col = { 0.0f, 0.0f, 1.0f, 1.0f}});

    vertices_push(&renderer.vertices, (Vertex){.pos = {-0.5f,  0.5f,  0.5f}, .col = { 1.0f, 0.0f, 0.0f, 1.0f}});
    vertices_push(&renderer.vertices, (Vertex){.pos = {-0.5f,  0.5f, -0.5f}, .col = { 0.0f, 1.0f, 0.0f, 1.0f}});
    vertices_push(&renderer.vertices, (Vertex){.pos = {-0.5f, -0.5f, -0.5f}, .col = { 0.0f, 0.0f, 1.0f, 1.0f}});
    vertices_push(&renderer.vertices, (Vertex){.pos = {-0.5f, -0.5f, -0.5f}, .col = { 0.0f, 0.0f, 1.0f, 1.0f}});
    vertices_push(&renderer.vertices, (Vertex){.pos = {-0.5f, -0.5f,  0.5f}, .col = { 0.0f, 1.0f, 0.0f, 1.0f}});
    vertices_push(&renderer.vertices, (Vertex){.pos = {-0.5f,  0.5f,  0.5f}, .col = { 1.0f, 0.0f, 0.0f, 1.0f}});

    vertices_push(&renderer.vertices, (Vertex){.pos = { 0.5f,  0.5f,  0.5f}, .col = { 1.0f, 0.0f, 0.0f, 1.0f}});
    vertices_push(&renderer.vertices, (Vertex){.pos = { 0.5f,  0.5f, -0.5f}, .col = { 0.0f, 1.0f, 0.0f, 1.0f}});
    vertices_push(&renderer.vertices, (Vertex){.pos = { 0.5f, -0.5f, -0.5f}, .col = { 0.0f, 0.0f, 1.0f, 1.0f}});
    vertices_push(&renderer.vertices, (Vertex){.pos = { 0.5f, -0.5f, -0.5f}, .col = { 0.0f, 0.0f, 1.0f, 1.0f}});
    vertices_push(&renderer.vertices, (Vertex){.pos = { 0.5f, -0.5f,  0.5f}, .col = { 0.0f, 1.0f, 0.0f, 1.0f}});
    vertices_push(&renderer.vertices, (Vertex){.pos = { 0.5f,  0.5f,  0.5f}, .col = { 1.0f, 0.0f, 0.0f, 1.0f}});

    vertices_push(&renderer.vertices, (Vertex){.pos = { 0.5f, -0.5f, -0.5f}, .col = { 1.0f, 0.0f, 0.0f, 1.0f}});
    vertices_push(&renderer.vertices, (Vertex){.pos = {-0.5f, -0.5f, -0.5f}, .col = { 0.0f, 1.0f, 0.0f, 1.0f}});
    vertices_push(&renderer.vertices, (Vertex){.pos = { 0.5f, -0.5f,  0.5f}, .col = { 0.0f, 0.0f, 1.0f, 1.0f}});
    vertices_push(&renderer.vertices, (Vertex){.pos = { 0.5f, -0.5f,  0.5f}, .col = { 0.0f, 0.0f, 1.0f, 1.0f}});
    vertices_push(&renderer.vertices, (Vertex){.pos = {-0.5f, -0.5f,  0.5f}, .col = { 0.0f, 1.0f, 0.0f, 1.0f}});
    vertices_push(&renderer.vertices, (Vertex){.pos = {-0.5f, -0.5f, -0.5f}, .col = { 1.0f, 0.0f, 0.0f, 1.0f}});

    vertices_push(&renderer.vertices, (Vertex){.pos = {-0.5f,  0.5f, -0.5f}, .col = { 1.0f, 0.0f, 0.0f, 1.0f}});
    vertices_push(&renderer.vertices, (Vertex){.pos = { 0.5f,  0.5f, -0.5f}, .col = { 0.0f, 1.0f, 0.0f, 1.0f}});
    vertices_push(&renderer.vertices, (Vertex){.pos = { 0.5f,  0.5f,  0.5f}, .col = { 0.0f, 0.0f, 1.0f, 1.0f}});
    vertices_push(&renderer.vertices, (Vertex){.pos = { 0.5f,  0.5f,  0.5f}, .col = { 0.0f, 0.0f, 1.0f, 1.0f}});
    vertices_push(&renderer.vertices, (Vertex){.pos = {-0.5f,  0.5f,  0.5f}, .col = { 0.0f, 1.0f, 0.0f, 1.0f}});
    vertices_push(&renderer.vertices, (Vertex){.pos = {-0.5f,  0.5f, -0.5f}, .col = { 1.0f, 0.0f, 0.0f, 1.0f}});

    renderer_init();

    glEnable(GL_DEPTH_TEST);

    float last_time = SDL_GetTicks();

    while (running)
    {
        handle_input();

        SDL_GetWindowSize(window, &screen_w, &screen_h);

        render_3d_scene();

        SDL_GL_SwapWindow(window);

        Uint64 current_time = SDL_GetTicks();
        delta = (current_time - last_time) / 1000.0f;
        last_time = current_time;
    }

    return 0;
}
