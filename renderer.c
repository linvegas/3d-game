#include "renderer.h"

#include <stdio.h>
#include <stddef.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_timer.h>

#define STBI_FAILURE_USERMSG
#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

// TODO: Bring it to the renderer
static Vertex vertices_data[1024];
static size_t vertices_data_len = 0;
static unsigned int indices_data[1024];
static size_t indices_data_len = 0;
static GLuint vao_2d, vbo_2d, ebo_2d;
static Glyph glyphs[128];

static void setup_2d_buffers(void)
{
    glGenVertexArrays(1, &vao_2d);
    glGenBuffers(1, &vbo_2d);
    glGenBuffers(1, &ebo_2d);
    glBindVertexArray(vao_2d);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_2d);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_data), vertices_data, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_2d);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_data), indices_data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coord));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
}

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
    Shader shader_2d;

    if (!shader_create_program(&shader_3d, "3d", "shader.frag")) return false;
    if (!shader_create_program(&shader_2d, "shader.vert", "shader.frag")) return false;

    r->shader_3d = shader_3d;
    r->shader_2d = shader_2d;

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

    setup_2d_buffers();

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

void render_begin_2d(Renderer *r)
{
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    shader_use(r->shader_2d);
}

void render_end_2d(Renderer *r)
{
    Mat4 projection = mat4_ortho(0, (float)r->width, (float)r->height, 0, -1.0, 1.0);
    shader_set_mat4(r->shader_2d, "uProjection", projection);

    glBindVertexArray(vao_2d);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_2d);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * vertices_data_len, vertices_data);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_2d);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(GLuint) * indices_data_len, indices_data);

    glDrawElements(GL_TRIANGLES, indices_data_len, GL_UNSIGNED_INT, 0);

    vertices_data_len = 0;
    indices_data_len = 0;

    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    shader_use(r->shader_3d);
}

void render_rect_2d(Renderer *r, int x, int y, int w, int h, Vec4 color)
{
    (void)r;
    w = x + w;
    h = y + h;
    // BOTTOM LEFT
    vertices_data[vertices_data_len].position  = vec3(x, h, 0);
    vertices_data[vertices_data_len].tex_coord = vec2(0, 0);
    vertices_data[vertices_data_len].color     = color;
    vertices_data_len += 1;

    // TOP LEFT
    vertices_data[vertices_data_len].position  = vec3(x, y, 0);
    vertices_data[vertices_data_len].tex_coord = vec2(0, 1);
    vertices_data[vertices_data_len].color     = color;
    vertices_data_len += 1;

    // BOTTOM RIGHT
    vertices_data[vertices_data_len].position  = vec3(w, h, 0);
    vertices_data[vertices_data_len].tex_coord = vec2(1, 0);
    vertices_data[vertices_data_len].color     = color;
    vertices_data_len += 1;

    // TOP RIGHT
    vertices_data[vertices_data_len].position  = vec3(w, y, 0);
    vertices_data[vertices_data_len].tex_coord = vec2(1, 1);
    vertices_data[vertices_data_len].color     = color;
    vertices_data_len += 1;

    unsigned int i = indices_data_len > 0 ? indices_data[indices_data_len-1]+1 : 0;

    indices_data[indices_data_len]     = i;
    indices_data[indices_data_len + 1] = i + 1;
    indices_data[indices_data_len + 2] = i + 2;
    indices_data[indices_data_len + 3] = i + 1;
    indices_data[indices_data_len + 4] = i + 2;
    indices_data[indices_data_len + 5] = i + 3;

    indices_data_len += 6;
}

void render_text_2d(const char *text, int x, int y, Vec4 color)
{
    float scale = 1.0;

    // TODO: Use font defined size
    float baseline_y = y + (44 * scale);

    for (const char *c = text; *c; c++)
    {
        Glyph glyph = glyphs[(int)*c];

        float x_pos = x + glyph.bearing_x * scale;
        float y_pos = baseline_y - glyph.bearing_y * scale;

        float w = glyph.pixel_width * scale;
        float h = glyph.pixel_height * scale;

        // BOTTOM LEFT
        vertices_data[vertices_data_len].position  = vec3(x_pos, y_pos + h, 0);
        vertices_data[vertices_data_len].tex_coord = vec2(glyph.x, glyph.y);
        vertices_data[vertices_data_len].color     = color;
        vertices_data_len += 1;

        // TOP LEFT
        vertices_data[vertices_data_len].position  = vec3(x_pos, y_pos, 0);
        vertices_data[vertices_data_len].tex_coord = vec2(glyph.x, glyph.y + glyph.height);
        vertices_data[vertices_data_len].color     = color;
        vertices_data_len += 1;

        // BOTTOM RIGHT
        vertices_data[vertices_data_len].position  = vec3(x_pos + w, y_pos + h, 0);
        vertices_data[vertices_data_len].tex_coord = vec2(glyph.x + glyph.width, glyph.y);
        vertices_data[vertices_data_len].color     = color;
        vertices_data_len += 1;

        // TOP RIGHT
        vertices_data[vertices_data_len].position  = vec3(x_pos + w, y_pos, 0);
        vertices_data[vertices_data_len].tex_coord = vec2(glyph.x + glyph.width, glyph.y + glyph.height);
        vertices_data[vertices_data_len].color     = color;
        vertices_data_len += 1;

        unsigned int i = indices_data_len > 0 ? indices_data[indices_data_len-1]+1 : 0;

        indices_data[indices_data_len]     = i;
        indices_data[indices_data_len + 1] = i + 1;
        indices_data[indices_data_len + 2] = i + 2;
        indices_data[indices_data_len + 3] = i + 1;
        indices_data[indices_data_len + 4] = i + 2;
        indices_data[indices_data_len + 5] = i + 3;

        indices_data_len += 6;

        x += glyph.advance * scale;
    }
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

    stbi_set_flip_vertically_on_load(1);
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

Texture texture_load_from_font(const char *fontpath, int size)
{
    Texture t = {0};

    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        fprintf(stderr,"ERROR: Could not init FreeType Library\n");
    }

    FT_Face face;
    if (FT_New_Face(ft, fontpath, 0, &face)) {
        fprintf(stderr,"ERROR: Failed to load font from file\n");
    }

    FT_Set_Pixel_Sizes(face, 0, size);

    unsigned int glyph_max_w = 0;
    unsigned int glyph_max_h = 0;

    for (unsigned char c = 0; c < 128; c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) continue;

        if (face->glyph->bitmap.width > glyph_max_w)
            glyph_max_w = face->glyph->bitmap.width;

        if (face->glyph->bitmap.rows > glyph_max_h)
            glyph_max_h = face->glyph->bitmap.rows;
    }

    int glyphs_row = 16;
    int glyphs_col = 8;

    t.width = glyphs_row * glyph_max_w;
    t.height = glyphs_col * glyph_max_h;

    unsigned char buffer[t.width*t.height];
    memset(buffer, 0, t.width*t.height);

    // int current_x = 0;
    // int current_y = 0;

    for (unsigned char c = 0; c < 128; c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            fprintf(stderr, "[ERROR] Failed to load character: %c\n", c);
            continue;
        }

        FT_GlyphSlot glyph = face->glyph;
        FT_Bitmap* bitmap = &glyph->bitmap;

        int glyph_x = (c % glyphs_row) * glyph_max_w;
        int glyph_y = (c / glyphs_row) * glyph_max_h;

        for (unsigned int row = 0; row < bitmap->rows; row++) {
            for (unsigned int col = 0; col < bitmap->width; col++)
            {
                int x = glyph_x + col;
                int y = glyph_y + row;
                // int y = (t.height - glyph_y - bitmap->rows) + row;
                int index = y * t.width + x;
                int bitmap_index = (bitmap->rows - 1 - row) * bitmap->width + col;

                buffer[index] = bitmap->buffer[bitmap_index];
            }
        }

        glyphs[c].x = (float)glyph_x / (float)t.width;
        glyphs[c].y = (float)glyph_y / (float)t.height;
        glyphs[c].width = (float)bitmap->width / (float)t.width;
        glyphs[c].height = (float)bitmap->rows / (float)t.height;
        glyphs[c].pixel_width = bitmap->width;
        glyphs[c].pixel_height = bitmap->rows;
        glyphs[c].bearing_x = glyph->bitmap_left;
        glyphs[c].bearing_y = glyph->bitmap_top;
        glyphs[c].advance = glyph->advance.x >> 6;
    }

    glGenTextures(1, &t.id);
    glBindTexture(GL_TEXTURE_2D, t.id);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RED,
        t.width,
        t.height,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        buffer
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

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
