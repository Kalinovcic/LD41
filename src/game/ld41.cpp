#include "../libraries/lk_platform.h"
#include "../libraries/GL/glew.h"

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

    char hotload_padding[512];
};


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

    glClearColor(1, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
}

LK_CLIENT_EXPORT
void lk_client_close(LK_Platform* platform)
{
}
