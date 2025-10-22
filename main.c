#include <stdio.h>
#include <stddef.h>
#include <math.h>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_timer.h>

#include "renderer.h"
#include "linalg.h"

#define GLAD_GL_IMPLEMENTATION
#include "external/glad.h"

#define FACTOR 80
#define SCREEN_WIDTH FACTOR*16
#define SCREEN_HEIGHT FACTOR*9

int running = 1;

float yaw = -90.0f;
float pitch = 0.0f;
float last_x = SCREEN_WIDTH / 2.0;
float last_y = SCREEN_HEIGHT / 2.0;
int first_mouse = 1;

float vel = 5.0f;
float delta = 0.0f;

void handle_input(Renderer *r)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT) running = 0;
        if (event.type == SDL_EVENT_KEY_DOWN)
        {
            if (event.key.key == SDLK_ESCAPE) running = 0;

            if (event.key.key == SDLK_W)
                r->camera.position = vec3_add(r->camera.position, vec3_scale(r->camera.target, delta*vel));
            if (event.key.key == SDLK_S)
                r->camera.position = vec3_sub(r->camera.position, vec3_scale(r->camera.target, delta*vel));
            if (event.key.key == SDLK_A)
                r->camera.position = vec3_sub(r->camera.position, vec3_scale(vec3_normalize(vec3_cross(r->camera.target, r->camera.up)), delta*vel));
            if (event.key.key == SDLK_D)
                r->camera.position = vec3_add(r->camera.position, vec3_scale(vec3_normalize(vec3_cross(r->camera.target, r->camera.up)), delta*vel));
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
            r->camera.target = vec3_normalize(front);

            // printf("x: %f  ", event.motion.x);
            // printf("y: %f  ", event.motion.y);
            // printf("xrel: %f  ", event.motion.xrel);
            // printf("yrel: %f\n", event.motion.yrel);
        }
    }
}

int main(void)
{
    Renderer renderer = {0};

    if (!renderer_init(&renderer, "3D", SCREEN_WIDTH, SCREEN_HEIGHT))
    {
        return 1;
    }

    Mesh cube = mesh_create_cube(1.0);

    float last_time = SDL_GetTicks();

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    while (running)
    {
        Uint64 current_time = SDL_GetTicks();
        delta = (current_time - last_time) / 1000.0f;
        last_time = current_time;

        handle_input(&renderer);

        renderer_clear(&renderer, 0.05, 0.05, 0.05, 1.0);

        renderer_camera_update(&renderer);

        float rot = SDL_GetTicks()/1000.0f;

        {
            Vec3 light_pos = {-3.0f, 5.0f, 4.0f};
            Vec3 cube_rot = {0.0f, 0.0f, 0.0f};
            Vec3 cube_scale = {0.35f, 0.35f, 0.35f};
            shader_set_vec3(renderer.shader_3d, "lightPos", light_pos);
            shader_set_vec3(renderer.shader_3d, "objectColor", vec3(1.0, 1.0, 1.0));
            render_model_3d(&renderer, cube, light_pos, cube_rot, cube_scale);
        }

        {
            Vec3 cube_pos = {0.0f, 0.0f, 0.0f};
            Vec3 cube_rot = {50.0 * rot, 0.0f, 0.0f};
            Vec3 cube_scale = {1.0f, 1.0f, 1.0f};
            shader_set_vec3(renderer.shader_3d, "objectColor", vec3(1.0, 0.5, 0.31));
            render_model_3d(&renderer, cube, cube_pos, cube_rot, cube_scale);
        }

        {
            Vec3 cube_pos = {3.0f, 0.0f, 0.0f};
            Vec3 cube_rot = {0.0f, 50*rot, 0.0f};
            Vec3 cube_scale = {1.0f, 1.0f, 1.0f};
            render_model_3d(&renderer, cube, cube_pos, cube_rot, cube_scale);
        }

        {
            Vec3 cube_pos = {-3.0f, 0.0f, 0.0f};
            Vec3 cube_rot = {0.0f, 0.0f, 50*rot};
            Vec3 cube_scale = {1.0f, 1.0f, 1.0f};
            render_model_3d(&renderer, cube, cube_pos, cube_rot, cube_scale);
        }

        renderer_present(&renderer);
    }
}
