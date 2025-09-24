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

typedef struct Vertex {
    Vec3 pos;
    Vec4 col;
} Vertex;

size_t vertices_len = 0;

GLuint compile_shaders(void);

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

    Vertex vertices[VERTICES_CAP] = {
        // [0] = { .pos = {-0.6, -0.4, 1.0}, .col = {0.8, 0.0, 0.3, 0.0} }, // RED
        // [1] = { .pos = { 0.6, -0.4, 1.0}, .col = {0.3, 0.8, 0.0, 0.0} }, // GREEN
        // [2] = { .pos = { 0.0,  0.6, 1.0}, .col = {0.0, 0.3, 0.8, 0.0} }, // BLUE

        [0] =  { .pos = { -0.5f, -0.5f, -0.5f}, .col = { 0.0f, 1.0f, 0.0f, 1.0f }},
        [1] =  { .pos = {  0.5f, -0.5f, -0.5f}, .col = { 1.0f, 0.0f, 0.0f, 1.0f }},
        [2] =  { .pos = {  0.5f,  0.5f, -0.5f}, .col = { 0.0f, 1.0f, 0.0f, 1.0f }},
        [3] =  { .pos = {  0.5f,  0.5f, -0.5f}, .col = { 0.0f, 1.0f, 0.0f, 1.0f }},
        [4] =  { .pos = { -0.5f,  0.5f, -0.5f}, .col = { 1.0f, 0.0f, 0.0f, 1.0f }},
        [5] =  { .pos = { -0.5f, -0.5f, -0.5f}, .col = { 0.0f, 1.0f, 0.0f, 1.0f }},

        [6] =  { .pos = { -0.5f, -0.5f,  0.5f}, .col = { 0.0f, 0.0f, 1.0f, 1.0f }},
        [7] =  { .pos = {  0.5f, -0.5f,  0.5f}, .col = { 0.0f, 1.0f, 0.0f, 1.0f }},
        [8] =  { .pos = {  0.5f,  0.5f,  0.5f}, .col = { 0.0f, 0.0f, 1.0f, 1.0f }},
        [9] =  { .pos = {  0.5f,  0.5f,  0.5f}, .col = { 0.0f, 0.0f, 1.0f, 1.0f }},
        [10] = { .pos = { -0.5f,  0.5f,  0.5f}, .col = { 0.0f, 1.0f, 0.0f, 1.0f }},
        [11] = { .pos = { -0.5f, -0.5f,  0.5f}, .col = { 0.0f, 0.0f, 1.0f, 1.0f }},

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

    Vec3 cam_pos   = vec3(0, 0, 6.0);
    Vec3 cam_front = vec3(0, 0, -1.0);
    Vec3 cam_up    = vec3(0, 1.0, 0);

    float yaw = -90.0f;
    float pitch = 0.0f;
    float last_x = 800.0 / 2.0;
    float last_y = 600.0 / 2.0;
    int first_mouse = 1;

    float vel = 5.0f;
    float last_time = SDL_GetTicks();
    float delta = 0.0f;

    glEnable(GL_DEPTH_TEST);

    while (running)
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

                printf("x: %f  ", event.motion.x);
                printf("y: %f  ", event.motion.y);
                printf("xrel: %f  ", event.motion.xrel);
                printf("yrel: %f\n", event.motion.yrel);
            }
        }

        SDL_GetWindowSize(window, &screen_w, &screen_h);

        Mat4 model = mat4_identity();
        Mat4 transform = mat4_rotate(
            SDL_GetTicks()/1000.0f * radians(50.0),
            vec3(0.4, 0.6, 0.0)
        );
        model = mat4_multiply(model, transform);

        Mat4 view = mat4_identity();
        Mat4 look = mat4_look_at(
            cam_pos,
            vec3_add(cam_pos, cam_front),
            cam_up
        );
        view = mat4_multiply(view, look);

        Mat4 projection = mat4_identity();
        double fov = radians(65.0);
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

        Uint64 current_time = SDL_GetTicks();
        delta = (current_time - last_time) / 1000.0f;
        last_time = current_time;
    }

    return 0;
}
