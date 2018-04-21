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

struct Tile
{
    int z;
};

struct Note
{
    int lane;
    float at;
    float duration;

    float hold_start;
    float hold_end;
    float score;

    bool pressed;
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
        Texture multiply;
        Texture wall;
        Texture floor;
    } art;

    struct
    {
        float time;
        float length;
        float window;

        std::vector<Note> notes;
        Note* holding[4];
    } rhythm;

    struct
    {
        int width;
        int height;
        Tile* tiles;
    } level;

    char hotload_padding[512];
};

static Game* the_game;

void load_level_from_image(const char* path)
{
    int width;
    int height;
    int dont_care;
    uint8* pixels = stbi_load(path, &width, &height, &dont_care, 4);

    auto& level = the_game->level;
    level.width = width;
    level.height = height;
    if (level.tiles)
    {
        delete[] level.tiles;
    }

    level.tiles = new Tile[level.width * level.height];
    for (int tile_y = 0; tile_y < level.height; tile_y++)
    {
        for (int tile_x = 0; tile_x < level.width; tile_x++)
        {
            uint8* pixel = pixels + ((height - tile_y - 1) * width + tile_x) * 4;
            Tile* tile = level.tiles + tile_y * level.width + tile_x;

            if (pixel[0] ==   0 && pixel[1] ==   0 && pixel[2] == 255) tile->z = 0;
            if (pixel[0] ==   0 && pixel[1] == 255 && pixel[2] ==   0) tile->z = 1;
            if (pixel[0] == 255 && pixel[1] ==   0 && pixel[2] ==   0) tile->z = 2;
        }
    }

    stbi_image_free(pixels);
}

Vector4 get_tile_color(int x, int y)
{
    auto& level = the_game->level;
    Tile* tile = level.tiles + y * level.width + x;

    uint32 colors[] = { 0x3993ED, 0xA4F250, 0xD46F0B };
    return rgb(colors[tile->z]);
}

Texture get_multiply_texture_piece(int tx, int ty)
{
    float p = 1.0 / 40.0;
    ty = 3 - ty;
    Texture whole = the_game->art.multiply;

    Texture result;
    result.uv1.x = lerp(whole.uv1.x, whole.uv2.x, (float)(tx + 0) / 4.0 + p);
    result.uv2.x = lerp(whole.uv1.x, whole.uv2.x, (float)(tx + 1) / 4.0 - p);
    result.uv1.y = lerp(whole.uv1.y, whole.uv2.y, (float)(ty + 0) / 4.0 + p);
    result.uv2.y = lerp(whole.uv1.y, whole.uv2.y, (float)(ty + 1) / 4.0 - p);
    return result;
}

Texture get_tile_quarter_multiply(
    int x, int y, int dx, int dy,
    int ocx, int ocy, // outer corner
    int icx, int icy, // inner corner
    int fxx, int fxy, // flat X
    int fyx, int fyy) // flat Y
{
    auto& level = the_game->level;
    Tile* tile = level.tiles + y * level.width + x;

    bool x_in_bounds = (x + dx >= 0) && (x + dx < level.width);
    bool y_in_bounds = (y + dy >= 0) && (y + dy < level.height);
    Tile* tile_x = (x_in_bounds) ? (level.tiles + y * level.width + (x + dx)) : NULL;
    Tile* tile_y = (y_in_bounds) ? (level.tiles + (y + dy) * level.width + x) : NULL;
    Tile* tile_c = (x_in_bounds && y_in_bounds) ? (level.tiles + (y + dy) * level.width + (x + dx)) : NULL;
    bool empty_x = tile_x && (tile_x->z < tile->z);
    bool empty_y = tile_y && (tile_y->z < tile->z);
    bool empty_c = tile_c && (tile_c->z < tile->z);

    if (!empty_x && !empty_y && empty_c)
    {
        return get_multiply_texture_piece(icx, icy);
    }

    if (empty_x && empty_y)
    {
        Texture full = get_multiply_texture_piece(3, 3);
        Vector4 color = get_tile_color(x + dx, y);
        push_rectangle({ x + 0.25f * (dx + 1), y + 0.25f * (dy + 1) }, { 0.5f, 0.5f }, full, color);

        return get_multiply_texture_piece(ocx, ocy);
    }

    if (empty_x)
    {
        Texture full = get_multiply_texture_piece(3, 3);
        Vector4 color = get_tile_color(x + dx, y);
        push_rectangle({ x + 0.25f * (dx + 1), y + 0.25f * (dy + 1) }, { 0.5f, 0.5f }, full, color);

        return get_multiply_texture_piece(fxx, fxy);
    }

    if (empty_y)
    {
        Texture full = get_multiply_texture_piece(3, 3);
        Vector4 color = get_tile_color(x, y + dy);
        push_rectangle({ x + 0.25f * (dx + 1), y + 0.25f * (dy + 1) }, { 0.5f, 0.5f }, full, color);

        return get_multiply_texture_piece(fyx, fyy);
    }

    return get_multiply_texture_piece(3, 3);
}

void render_tile(int x, int y)
{
    Vector4 color = get_tile_color(x, y);

    Texture multiply00 = get_tile_quarter_multiply(x, y, -1, -1, 0, 1, 1, 2, 2, 1, 3, 1);
    Texture multiply10 = get_tile_quarter_multiply(x, y,  1, -1, 1, 1, 0, 2, 3, 0, 3, 1);
    Texture multiply11 = get_tile_quarter_multiply(x, y,  1,  1, 1, 0, 0, 3, 3, 0, 2, 0);
    Texture multiply01 = get_tile_quarter_multiply(x, y, -1,  1, 0, 0, 1, 3, 2, 1, 2, 0);

    push_rectangle({ (float) x + 0.0f, (float) y + 0.0f }, { 0.5f, 0.5f }, multiply00, color);
    push_rectangle({ (float) x + 0.5f, (float) y + 0.0f }, { 0.5f, 0.5f }, multiply10, color);
    push_rectangle({ (float) x + 0.5f, (float) y + 0.5f }, { 0.5f, 0.5f }, multiply11, color);
    push_rectangle({ (float) x + 0.0f, (float) y + 0.5f }, { 0.5f, 0.5f }, multiply01, color);
}

void render_level()
{
    auto& level = the_game->level;

    for (int tile_y = 0; tile_y < level.height; tile_y++)
    {
        for (int tile_x = 0; tile_x < level.width; tile_x++)
        {
            render_tile(tile_x, tile_y);
        }
    }
}

#include "rhythm.inl"

LK_CLIENT_EXPORT
void lk_client_init(LK_Platform* platform)
{
    platform->window.title = strdup("LD41");
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
        game->art.multiply = add_texture(atlas, "data/textures/multiply.png");
        game->art.wall = add_texture(atlas, "data/textures/wall.png");
        game->art.floor = add_texture(atlas, "data/textures/floor.png");

        init_renderer(&game->renderer);
        game->initialized = true;
    }

    if (!game->level.tiles)
    {
        load_level_from_image("data/level/level.png");
    }


    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glViewport(0, 0, platform->window.width, platform->window.height);

    float aspect = (float) platform->window.width / (float) platform->window.height;
    float camera_x = 16 + sin(platform->time.seconds * 0.3) * 6;
    float camera_y = 16 + cos(platform->time.seconds * 0.3) * 6;
    game->renderer.camera_height = 12;
    game->renderer.camera_width = game->renderer.camera_height * aspect;
    game->renderer.camera_transform = orthographic(
        camera_x - game->renderer.camera_width  * 0.5, camera_x + game->renderer.camera_width  * 0.5,
        camera_y - game->renderer.camera_height * 0.5, camera_y + game->renderer.camera_height * 0.5,
        -1, 1);

    render_level();
    rendering_flush(&game->renderer);


    {
        float aspect = (float) platform->window.width / (float) platform->window.height;
        game->renderer.camera_height = 8;
        game->renderer.camera_width = game->renderer.camera_height * aspect;
        game->renderer.camera_transform = orthographic(
            0, game->renderer.camera_width,
            0, game->renderer.camera_height, -1, 1);
    }

    rhythm_controls();
    rhythm_render();
}

LK_CLIENT_EXPORT
void lk_client_close(LK_Platform* platform)
{
}
