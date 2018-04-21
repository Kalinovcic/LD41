#include "../libraries/lk_platform.h"
#include "../libraries/GL/glew.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../libraries/stb_image.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include <algorithm>
#include <vector>


constexpr float DEG2RAD = 0.01745329251;
constexpr float RAD2DEG = 57.2957795131;

typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef uint8 byte;

union Vector2
{
    struct { float x, y; };
    struct { float r, g; };
    float e[2];
};

union Vector3
{
    struct { float x, y, z; };
    struct { float r, g, b; };
    float e[3];
    Vector2 xy;
};

union Vector4
{
    struct { float x, y, z, w; };
    struct { float r, g, b, a; };
    float e[4];
    Vector2 xy;
    Vector3 xyz;
};

union Matrix4
{
    float m[4][4];
    float e[16];
};

#include "math_ops.inl"

struct Game
{
    LK_Platform* platform;

    struct
    {
        GLuint shader;
        GLuint atlas;
        GLuint vao;
        GLuint vbo;
        GLuint ibo;

        Matrix4 camera_transform;
    } renderer;

    bool initialized;

    char hotload_padding[512];
};


struct Vertex
{
    Vector2 position;
    Vector2 uv;
};

std::vector<Vertex> render_vertices;
std::vector<uint16> render_indices;

void push_rectangle(float x, float y, float width, float height,
                    float u1, float v1, float u2, float v2)
{
    float x1 = x;
    float x2 = x + width;
    float y1 = y;
    float y2 = y + height;

    uint16 base_index = render_vertices.size();

    render_vertices.push_back({ { x1, y1 }, { u1, v1 } });
    render_vertices.push_back({ { x2, y1 }, { u2, v1 } });
    render_vertices.push_back({ { x2, y2 }, { u2, v2 } });
    render_vertices.push_back({ { x1, y2 }, { u1, v2 } });

    render_indices.push_back(base_index + 0);
    render_indices.push_back(base_index + 1);
    render_indices.push_back(base_index + 2);
    render_indices.push_back(base_index + 0);
    render_indices.push_back(base_index + 2);
    render_indices.push_back(base_index + 3);
}

void init_renderer(Game* game)
{
    auto& r = game->renderer;

    glGenVertexArrays(1, &r.vao);
    glBindVertexArray(r.vao);

    glGenBuffers(1, &r.vbo);
    glGenBuffers(1, &r.ibo);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, r.vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(Vertex), (void*) offsetof(Vertex, position));
    glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(Vertex), (void*) offsetof(Vertex, uv));



    const char* vs_source =
        "#version 430\n"
        "uniform mat4 transform;\n"
        "layout(location = 0) in vec2 vertex_position;\n"
        "layout(location = 1) in vec2 vertex_uv;\n"
        "out vec2 fragment_uv;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(vertex_position, 0, 1) * transform;\n"
        "    fragment_uv = vertex_uv;\n"
        "}\n";

    const char* fs_source =
        "#version 430\n"
        "uniform sampler2D atlas;\n"
        "in vec2 fragment_uv;\n"
        "out vec4 pixel_color;\n"
        "void main()\n"
        "{\n"
        "    pixel_color = texture(atlas, fragment_uv);\n"
        "}\n";

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vs, 1, &vs_source, NULL);
    glShaderSource(fs, 1, &fs_source, NULL);
    glCompileShader(vs);
    glCompileShader(fs);

    r.shader = glCreateProgram();
    glAttachShader(r.shader, vs);
    glAttachShader(r.shader, fs);
    glLinkProgram(r.shader);

    glDetachShader(r.shader, vs);
    glDetachShader(r.shader, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);



    int atlas_width;
    int atlas_height;
    int dont_care;
    uint8* atlas = stbi_load("test.png", &atlas_width, &atlas_height, &dont_care, 4);

    glGenTextures(1, &r.atlas);
    glBindTexture(GL_TEXTURE_2D, r.atlas);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, atlas_width, atlas_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, atlas);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

void rendering_flush(Game* game)
{
    auto& r = game->renderer;

    glBindBuffer(GL_ARRAY_BUFFER, r.vbo);
    glBufferData(GL_ARRAY_BUFFER, render_vertices.size() * sizeof(Vertex), &render_vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, render_indices.size() * sizeof(uint16), &render_indices[0], GL_STATIC_DRAW);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, r.atlas);

    glUseProgram(r.shader);
    GLint transform = glGetUniformLocation(r.shader, "transform");
    GLint atlas = glGetUniformLocation(r.shader, "atlas");
    glUniformMatrix4fv(transform, 1, true, r.camera_transform.e);
    glUniform1i(atlas, 0);

    glBindVertexArray(r.vao);
    glDrawElements(GL_TRIANGLES, render_indices.size(), GL_UNSIGNED_SHORT, 0);

    render_vertices.clear();
    render_indices.clear();
}


LK_CLIENT_EXPORT
void lk_client_init(LK_Platform* platform)
{
    platform->window.title = "LD41";
    platform->window.width = 900;
    platform->window.height = 600;
    platform->window.backend = LK_WINDOW_OPENGL;
    platform->window.disable_animations = true;

    platform->opengl.major_version = 4;
    platform->opengl.minor_version = 3;
    platform->opengl.debug_context = 1;
    platform->opengl.swap_interval = 1;

    platform->audio.strategy = LK_AUDIO_MIXER;

    Game* game = (Game*) calloc(1, sizeof(Game));
    game->platform = platform;
    platform->client_data = game;
}

LK_CLIENT_EXPORT
void lk_client_frame(LK_Platform* platform)
{
    if (!glewExperimental)
    {
        glewExperimental = true;
        glewInit();
    }

    Game* game = (Game*) platform->client_data;
    if (!game->initialized)
    {
        init_renderer(game);
        game->initialized = true;
    }

    glClearColor(1, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glViewport(0, 0, platform->window.width, platform->window.height);

    float aspect = (float) platform->window.width / (float) platform->window.height;
    float camera_x = sin(platform->time.seconds) * 1;
    float camera_y = cos(platform->time.seconds) * 1;
    float camera_height = 3;
    float camera_width = camera_height * aspect;
    game->renderer.camera_transform = orthographic(
        camera_x - camera_width  * 0.5, camera_x + camera_width  * 0.5,
        camera_y - camera_height * 0.5, camera_y + camera_height * 0.5,
        -1, 1);

    push_rectangle(-0.5, -0.5, 1, 1, 0, 0, 1, 1);
    rendering_flush(game);
}

LK_CLIENT_EXPORT
void lk_client_close(LK_Platform* platform)
{
}
