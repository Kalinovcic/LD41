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
#include "renderer.inl"

struct Note
{
    int lane;
    float at;
    float duration;
};

struct Game
{
    bool initialized;
    LK_Platform* platform;

    Renderer renderer;

    struct
    {
        Texture marker;
        Texture marker_bed;
        Texture white;
    } art;

    struct
    {
        float time;
        float length;
        float window;

        std::vector<Note> notes;
    } rhythm;

    char hotload_padding[512];
};

static Game* the_game;

const float RHYTHM_EXIT_WIDTH = 1.5;

void draw_lanes()
{
    float width = the_game->renderer.camera_width;
    float side = RHYTHM_EXIT_WIDTH;

    for (int lane = 0; lane < 4; lane++)
    {
        push_rectangle({ side, (float) lane }, { width - side, 1 }, the_game->art.marker_bed);
    }

    push_rectangle({ 0, 0 }, { side, 4 }, the_game->art.white);
}

void draw_marker(int lane, float time, float duration)
{
    auto& rhythm = the_game->rhythm;

    float side = RHYTHM_EXIT_WIDTH;
    float x_scale = (the_game->renderer.camera_width - side) / rhythm.window;
    float x = (time - rhythm.time) * x_scale + side;
    float width = max_f32(duration * x_scale - 1, 0);
    float y = (float) lane;

    Texture texture = the_game->art.marker;
    float center_u = (texture.uv2.x - texture.uv1.x) * 0.5;

    Texture left = texture;
    Texture middle = texture;
    Texture right = texture;
    left.uv2.x = center_u;
    middle.uv1.x = middle.uv2.x = center_u;
    right.uv1.x = center_u;

    float t = sin(the_game->platform->time.seconds + time) * 0.5 + 0.5;
    Vector4 red = vector4(1, 0, 0, 1);
    Vector4 yellow = vector4(1, 1, 0, 1);
    Vector4 green = vector4(0, 1, 0, 1);

    Vector4 color;
    if (t < 0.5)
        color = lerp(red, yellow, t * 2);
    else
        color = lerp(yellow, green, (t - 0.5) * 2);

    push_rectangle({ x,                 y }, { 0.5f,  1 }, left,   color);
    push_rectangle({ x + 0.5f,          y }, { width, 1 }, middle, color);
    push_rectangle({ x + 0.5f + width,  y }, { 0.5f,  1 }, right,  color);
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
    the_game = game;

    if (!game->initialized)
    {
        Atlas* atlas = &game->renderer.atlas;
        create_atlas(atlas, 512);
        game->art.marker = add_texture(atlas, "data/textures/marker.png");
        game->art.marker_bed = add_texture(atlas, "data/textures/marker_bed.png");
        game->art.white = add_texture(atlas, "data/textures/white.png");

        init_renderer(&game->renderer);
        game->initialized = true;
    }

    glClearColor(1, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glViewport(0, 0, platform->window.width, platform->window.height);

    float aspect = (float) platform->window.width / (float) platform->window.height;
    float camera_x = 0;
    float camera_y = 0;
    game->renderer.camera_height = 3;
    game->renderer.camera_width = game->renderer.camera_height * aspect;
    game->renderer.camera_transform = orthographic(
        camera_x - game->renderer.camera_width  * 0.5, camera_x + game->renderer.camera_width  * 0.5,
        camera_y - game->renderer.camera_height * 0.5, camera_y + game->renderer.camera_height * 0.5,
        -1, 1);

    push_centered_rectangle({ -1, 0 }, { 1, 1 }, game->art.white, { 0, 1, 1, 1 });

    float t = platform->time.seconds;
    push_centered_rectangle({  sinf(t),  cosf(t) }, { 1, 1 }, game->art.marker);
    push_centered_rectangle({ -sinf(t), -cosf(t) }, { 1, 1 }, game->art.marker);
    push_centered_rectangle({ -cosf(t),  sinf(t) }, { 1, 1 }, game->art.marker);
    push_centered_rectangle({  cosf(t), -sinf(t) }, { 1, 1 }, game->art.marker);

    rendering_flush(&game->renderer);


    {
        float aspect = (float) platform->window.width / (float) platform->window.height;
        game->renderer.camera_height = 8;
        game->renderer.camera_width = game->renderer.camera_height * aspect;
        game->renderer.camera_transform = orthographic(
            0, game->renderer.camera_width,
            0, game->renderer.camera_height, -1, 1);
    }

    game->rhythm.length = 10;
    game->rhythm.window = 7;
    game->rhythm.time = fmod(platform->time.seconds * 3, game->rhythm.length + 2) - 2;

    static bool created = false;
    if (game->rhythm.time < 0)
    {
        if (!created)
        {
            game->rhythm.notes.clear();
            for (int i = 0; i < 6; i++)
            {
                float at = rand() % 1000 / 1000.0f * 8;
                float duration = rand() % 1000 / 1000.0f * 2;
                game->rhythm.notes.push_back({ rand() % 4, at, duration });
            }
        }
        created = true;
    }
    else
    {
        created = false;
    }

    draw_lanes();
    for (Note note : game->rhythm.notes)
    {
        draw_marker(note.lane, note.at, note.duration);
    }

    rendering_flush(&game->renderer);
}

LK_CLIENT_EXPORT
void lk_client_close(LK_Platform* platform)
{
}
