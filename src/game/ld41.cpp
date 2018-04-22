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

enum Brain
{
    BRAIN_PLAYER,
    BRAIN_TREASURE,
    BRAIN_MONSTER,
};

struct Entity
{
    Brain brain;
    Vector2 position;
    Vector2 size;
    Texture texture;
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
        Texture font;
        Texture barrel;
        Texture shadow;
        Texture stones;
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

        std::vector<Entity> entities;

        Vector2 camera_position;
        Vector2 target_camera_position;
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

int random_int(int max)
{
    return rand() % max;
}

float random_float()
{
    return (float) rand() / (float) RAND_MAX;
}

static int generator_count_neighbors(Tile* read, int width, int height, int x, int y)
{
    int count = 0;

    for (int dy = -1; dy <= 1; dy++)
    {
        for (int dx = -1; dx <= 1; dx++)
        {
            int nx = x + dx;
            int ny = y + dy;
            if (nx < 0 || ny < 0 || nx >= width || ny >= height)
            {
                count++;
            }
            else
            {
                count += read[ny * width + nx].z;
            }
        }
    }

    return count;
}

void generate_level_cave(int width, int height)
{
    auto& level = the_game->level;
    level.width = width;
    level.height = height;
    if (level.tiles)
    {
        delete[] level.tiles;
    }

    level.entities.clear();

    Tile* read  = new Tile[width * height];
    Tile* write = new Tile[width * height];

    const float initial_chance = 0.4;
    const int birth_limit = 4;
    const int death_limit = 3;
    const int iteration_count = 5;

    const int treasure_limit = 4;
    const float treasure_chance = 0.4;

    // random seed

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            float roll = random_float();
            write[y * width + x].z = (int)(roll < initial_chance);
        }
    }

    // cellular automata

    for (int step = 0; step < iteration_count; step++)
    {
        std::swap(read, write);

        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int count = generator_count_neighbors(read, width, height, x, y);

                if (read[y * width + x].z)
                {
                    write[y * width + x].z = (int)(count > death_limit);
                }
                else
                {
                    write[y * width + x].z = (int)(count > birth_limit);
                }
            }
        }
    }

    // treasure

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            if (!write[y * width + x].z)
            {
                int count = generator_count_neighbors(write, width, height, x, y);
                if (count >= treasure_limit)
                {
                    float roll = random_float();
                    if (roll > treasure_chance)
                    {
                        continue;
                    }

                    Entity treasure;
                    treasure.brain = BRAIN_TREASURE;
                    treasure.position = vector2(x + 0.5, y + 0.5);
                    treasure.size = vector2(1, 1);
                    treasure.texture = the_game->art.barrel;
                    level.entities.push_back(treasure);
                }
            }
        }
    }

    // find columns

    for (int y = 0; y < height - 3; y++)
    {
        for (int x = 0; x < width - 3; x++)
        {
            if (write[(y + 0) * width + (x + 1)].z != 0) continue;
            if (write[(y + 0) * width + (x + 2)].z != 0) continue;
            if (write[(y + 1) * width + (x + 0)].z != 0) continue;
            if (write[(y + 1) * width + (x + 1)].z != 1) continue;
            if (write[(y + 1) * width + (x + 2)].z != 1) continue;
            if (write[(y + 1) * width + (x + 3)].z != 0) continue;
            if (write[(y + 2) * width + (x + 0)].z != 0) continue;
            if (write[(y + 2) * width + (x + 1)].z != 1) continue;
            if (write[(y + 2) * width + (x + 2)].z != 1) continue;
            if (write[(y + 2) * width + (x + 3)].z != 0) continue;
            if (write[(y + 3) * width + (x + 1)].z != 0) continue;
            if (write[(y + 3) * width + (x + 2)].z != 0) continue;

            write[(y + 1) * width + (x + 1)].z = 2;
            write[(y + 1) * width + (x + 2)].z = 2;
            write[(y + 2) * width + (x + 1)].z = 2;
            write[(y + 2) * width + (x + 2)].z = 2;
        }
    }

    // place the player

    while (true)
    {
        int x = random_int(width - 1);
        int y = random_int(height - 1);

        if (read[(y + 0) * width + (x + 0)].z) continue;
        if (read[(y + 0) * width + (x + 1)].z) continue;
        if (read[(y + 1) * width + (x + 1)].z) continue;
        if (read[(y + 1) * width + (x + 0)].z) continue;

        Entity player;
        player.brain = BRAIN_PLAYER;
        player.size = vector2(1.7, 1.7);
        player.position = vector2(x + 1, y + 1) - 0.5 * player.size;
        player.texture = the_game->art.white;
        level.entities.push_back(player);
        break;
    }

    // place monsters

    for (int i = 0; i < 80; i++)
    {
        int x = random_int(width - 1);
        int y = random_int(height - 1);

        if (read[(y + 0) * width + (x + 0)].z) continue;
        if (read[(y + 0) * width + (x + 1)].z) continue;
        if (read[(y + 1) * width + (x + 1)].z) continue;
        if (read[(y + 1) * width + (x + 0)].z) continue;

        Entity monster;
        monster.brain = BRAIN_MONSTER;
        monster.size = vector2(1.2, 1.2);
        monster.position = vector2(x + 1, y + 1) - 0.5 * monster.size;
        monster.texture = the_game->art.white;
        level.entities.push_back(monster);
    }

    level.tiles = write;
    delete[] read;
}

static bool intersect_aabb_aabb(Vector2 center1, Vector2 size1, Vector2 center2, Vector2 size2)
{
    size1 *= 0.5f;
    size2 *= 0.5f;
    Vector2 min1 = center1 - size1;
    Vector2 max1 = center1 + size1;
    Vector2 min2 = center2 - size2;
    Vector2 max2 = center2 + size2;
    return (max1.x > min2.x) && (min1.x < max2.x) &&
           (max1.y > min2.y) && (min1.y < max2.y);
}

static float collide_aabb_aabb_axis(float center1, float size1, float center2, float size2)
{
    if (center1 < center2)
    {
        float my_right = center1 + size1 * 0.5;
        float their_left = center2 - size2 * 0.5;
        return their_left - my_right;
    }
    else
    {
        float my_left = center1 - size1 * 0.5;
        float their_right = center2 + size2 * 0.5;
        return their_right - my_left;
    }
}

static Vector2 collide_aabb_aabb(Vector2 center1, Vector2 size1, Vector2 center2, Vector2 size2)
{
    if (!intersect_aabb_aabb(center1, size1, center2, size2))
    {
        return center1;
    }

    float x_axis = collide_aabb_aabb_axis(center1.x, size1.x, center2.x, size2.x);
    float y_axis = collide_aabb_aabb_axis(center1.y, size1.y, center2.y, size2.y);

    if (fabs(x_axis) < fabs(y_axis))
    {
        center1.x += x_axis;
    }
    else
    {
        center1.y += y_axis;
    }

    return center1;
}

static void move_entity(Entity* entity, Vector2 delta)
{
    auto& level = the_game->level;

    const float MAX_MOVE_DISTANCE = 0.1;
    float distance = length(delta);
    if (distance > MAX_MOVE_DISTANCE)
    {
        Vector2 my_delta = noz(delta) * MAX_MOVE_DISTANCE;
        move_entity(entity, delta - my_delta);

        distance = MAX_MOVE_DISTANCE;
        delta = my_delta;
    }

    entity->position += delta;
    Vector2 min_extreme = entity->position - entity->size * 0.5;
    Vector2 max_extreme = entity->position + entity->size * 0.5;

    int min_x = max_i32((int)(floorf(min_extreme.x) + 0.5f), 0);
    int min_y = max_i32((int)(floorf(min_extreme.y) + 0.5f), 0);
    int max_x = min_i32((int)(ceilf (max_extreme.x) + 0.5f), level.width  - 1);
    int max_y = min_i32((int)(ceilf (max_extreme.y) + 0.5f), level.height - 1);

    for (int y = min_y; y <= max_y; y++)
    {
        for (int x = min_x; x <= max_x; x++)
        {
            Tile* tile = level.tiles + y * level.width + x;
            if (!tile->z) continue;

            Vector2 tile_center = vector2(x + 0.5f, y + 0.5f);
            Vector2 tile_size = { 1, 1 };

            Vector2 old_position = entity->position;
            Vector2 new_position = collide_aabb_aabb(old_position, entity->size, tile_center, tile_size);
            entity->position = new_position;
        }
    }
}

void update_level()
{
    auto& level = the_game->level;

    if (the_game->platform->keyboard.state[LK_KEY_SPACE].pressed)
    {
        generate_level_cave(64, 64);
    }

    for (Entity& entity : level.entities)
    {
        switch (entity.brain)
        {

        case BRAIN_PLAYER:
        {
            auto& keyboard = the_game->platform->keyboard;

            Vector2 move = {};
            if (keyboard.state['D'].down) move.x += 1;
            if (keyboard.state['A'].down) move.x -= 1;
            if (keyboard.state['W'].down) move.y += 1;
            if (keyboard.state['S'].down) move.y -= 1;

            float move_distance = 6 * the_game->platform->time.delta_seconds;
            move_entity(&entity, noz(move) * move_distance);
            level.target_camera_position = entity.position;
        } break;

        case BRAIN_MONSTER:
        {
            float move_distance = 6 * the_game->platform->time.delta_seconds;
            Vector2 move = vector2(random_float() * 2 - 1, random_float() * 2 - 1);
            move = noz(move) * move_distance;

            move_entity(&entity, move);
        } break;

        }
    }

    float camera_t = min_f32(1, the_game->platform->time.delta_seconds * 1.2f);
    level.camera_position = lerp(level.camera_position, level.target_camera_position, camera_t);
}

Vector4 get_tile_color(int x, int y)
{
    auto& level = the_game->level;
    Tile* tile = level.tiles + y * level.width + x;

    uint32 colors[] = { 0x5FA1D9, 0x574E8C, 0x918DA8 };
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

    rendering_flush(&the_game->renderer);

    for (int tile_y = 0; tile_y < level.height; tile_y++)
    {
        for (int tile_x = 0; tile_x < level.width; tile_x++)
        {
            int variation = (tile_x * 59351 + tile_y * 903961) % 4;
            int vx = (variation & 1) * 0.5f;
            int vy = (variation >> 1) * 0.5f;

            Texture texture = the_game->art.stones;
            Vector2 middle = (texture.uv1 + texture.uv2) * 0.5f;
            if (vx) texture.uv1.x = middle.x;
            else    texture.uv2.x = middle.x;
            if (vy) texture.uv1.y = middle.y;
            else    texture.uv2.y = middle.y;

            push_centered_rectangle(vector2(tile_x + 0.5f, tile_y + 0.5f), { 1, 1 }, texture);
        }
    }

    rendering_flush(&the_game->renderer, true);

    for (Entity& entity : level.entities)
    {
        push_centered_rectangle(entity.position, entity.size * 1.5f, the_game->art.shadow);
        push_centered_rectangle(entity.position, entity.size, entity.texture);
    }

    rendering_flush(&the_game->renderer);
}

bool does_tile_create_shadow(int x, int y)
{
    auto& level = the_game->level;
    if (x < 0 || x >= level.width)  return true;
    if (y < 0 || y >= level.height) return true;
    return level.tiles[y * level.width + x].z;
}

void render_shadows()
{
    auto& level = the_game->level;
    auto& renderer = the_game->renderer;

    uint8* pixels = (uint8*) calloc(1, (level.width + 1) * (level.height + 1) * 4);

    for (int tile_y = 0; tile_y <= level.height; tile_y++)
    {
        for (int tile_x = 0; tile_x <= level.width; tile_x++)
        {
            int count = 0;
            count += does_tile_create_shadow(tile_x - 1, tile_y - 1) ? 1 : 0;
            count += does_tile_create_shadow(tile_x - 0, tile_y - 1) ? 1 : 0;
            count += does_tile_create_shadow(tile_x - 1, tile_y - 0) ? 1 : 0;
            count += does_tile_create_shadow(tile_x - 0, tile_y - 0) ? 1 : 0;

            const uint8 COUNT_TO_ALPHA[] = { 0, 20, 20, 50, 150 };
            uint8 alpha = COUNT_TO_ALPHA[count];
            pixels[(tile_y * (level.width + 1) + tile_x) * 4 + 3] = alpha;
        }
    }

    GLuint shadow_texture;
    glGenTextures(1, &shadow_texture);
    glBindTexture(GL_TEXTURE_2D, shadow_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, level.width + 1, level.height + 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    free(pixels);

    GLuint atlas = renderer.atlas_texture;
    renderer.atlas_texture = shadow_texture;

    push_rectangle(-0.5, -0.5, level.width + 1, level.height + 1, 0, 0, 1, 1, vector4(1, 1, 1, 1));
    rendering_flush(&renderer);

    renderer.atlas_texture = atlas;

    glDeleteTextures(1, &shadow_texture);
}

void render_string(const char* text, float x, float y, float sx, float sy)
{
    const char FONT_CHARS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ.,!?abcdefghijklmnopqrstuvwxyz    0123456789%";

    Texture font = the_game->art.font;

    float cx = x;
    while (*text)
    {
        int index;
        for (index = 0; index < sizeof(FONT_CHARS); index++)
            if (FONT_CHARS[index] == *text)
                break;
        if (index == sizeof(FONT_CHARS))
            index = 29;
        text++;

        int fx = index % 30;
        int fy = 2 - index / 30;

        float u1 = (fx * 5 + 0) / 150.0f + 1e-3;
        float u2 = (fx * 5 + 5) / 150.0f - 1e-3;
        float v1 = (fy * 7 + 0) /  21.0f + 1e-3;
        float v2 = (fy * 7 + 7) /  21.0f - 1e-3;

        Texture character;
        character.uv1.x = lerp(font.uv1.x, font.uv2.x, u1);
        character.uv2.x = lerp(font.uv1.x, font.uv2.x, u2);
        character.uv1.y = lerp(font.uv1.y, font.uv2.y, v1);
        character.uv2.y = lerp(font.uv1.y, font.uv2.y, v2);

        push_rectangle({ cx, y }, { sx, sy }, character);
        cx += sx * 1.1;
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
        game->art.font = add_texture(atlas, "data/textures/font.png");
        game->art.barrel = add_texture(atlas, "data/textures/barrel.png");
        game->art.shadow = add_texture(atlas, "data/textures/shadow.png");
        game->art.stones = add_texture(atlas, "data/textures/stones.png");

        init_renderer(&game->renderer);
        game->initialized = true;
    }

    if (!game->level.tiles)
    {
        generate_level_cave(64, 64);
        // load_level_from_image("data/level/level.png");
    }

    update_level();


    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glViewport(0, 0, platform->window.width, platform->window.height);

    float aspect = (float) platform->window.width / (float) platform->window.height;
    float camera_x = the_game->level.camera_position.x; // 16 + sin(platform->time.seconds * 0.3) * 6;
    float camera_y = the_game->level.camera_position.y; // 16 + cos(platform->time.seconds * 0.3) * 6;
    game->renderer.camera_height = 16;
    game->renderer.camera_width = game->renderer.camera_height * aspect;
    game->renderer.camera_transform = orthographic(
        camera_x - game->renderer.camera_width  * 0.5, camera_x + game->renderer.camera_width  * 0.5,
        camera_y - game->renderer.camera_height * 0.5, camera_y + game->renderer.camera_height * 0.5,
        -1, 1);

    render_level();
    render_shadows();

    // render_string("This font is horrible", camera_x - 4, camera_y, 0.5, 0.5);
    // render_string("THIS FONT IS TOLERABLE", camera_x - 4, camera_y + 1, 0.5, 0.5);


    {
        float aspect = (float) platform->window.width / (float) platform->window.height;
        game->renderer.camera_height = 8;
        game->renderer.camera_width = game->renderer.camera_height * aspect;
        game->renderer.camera_transform = orthographic(
            0, game->renderer.camera_width,
            0, game->renderer.camera_height, -1, 1);
    }

    rhythm_controls();
    // rhythm_render();
}

LK_CLIENT_EXPORT
void lk_client_close(LK_Platform* platform)
{
}
