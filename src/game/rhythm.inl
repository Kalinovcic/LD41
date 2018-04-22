static byte* read_all_bytes_from_file(const char* path)
{
    FILE* file = fopen(path, "rb");
    if (!file)
    {
        printf("Failed to open file %s\n", path);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    uint64 size = ftell(file);
    fseek(file, 0, SEEK_SET);

    byte* data = (byte*) malloc(size);
    fread(data, size, 1, file);

    fclose(file);

    return data;
}

static LK_Wave load_wav_file(const char* path)
{
    #define ReadNext(Type) *((Type*)(file += sizeof(Type)) - 1)

    byte* file;
    LK_U32 chunk_id;
    LK_U32 chunk_size;
    auto seek_to_chunk = [&](LK_U32 search_id)
    {
        while (true)
        {
            chunk_id = ReadNext(LK_U32);
            chunk_size = ReadNext(LK_U32);
            if (chunk_id == search_id) return;
            file += chunk_size;
        }
    };

    file = read_all_bytes_from_file(path);
    if (!file)
    {
        goto failure;
    }

    chunk_id = ReadNext(LK_U32);
    chunk_size = ReadNext(LK_U32);
    LK_U32 riff_format = ReadNext(LK_U32);
    if (chunk_id    != 0x46464952) goto failure; // "RIFF" chunk
    if (riff_format != 0x45564157) goto failure; // "WAVE"

    struct Info
    {
        LK_U16 encoding;
        LK_U16 channels;
        LK_U32 frequency;
        LK_U32 byte_rate;
        LK_U16 block_align;
        LK_U16 bits_per_sample;
    };

    seek_to_chunk(0x20746D66); // "fmt " chunk
    Info info = ReadNext(Info);
    if (info.encoding != 1) goto failure; // PCM
    if (info.byte_rate != info.frequency * info.channels * (info.bits_per_sample / 8)) goto failure;
    if (info.block_align != info.channels * (info.bits_per_sample / 8)) goto failure;
    if (info.bits_per_sample != 16) goto failure;
    if (info.channels != 1 && info.channels != 2) goto failure;

    seek_to_chunk(0x61746164); // "data" chunk

    LK_Wave sound;
    sound.samples = (LK_S16*) malloc(chunk_size);
    sound.count = chunk_size / (2 * info.channels);
    sound.channels = info.channels;
    sound.frequency = info.frequency;
    memcpy(sound.samples, file, chunk_size);

    #undef ReadNext
    return sound;

    failure:
    {
        printf("Failed to load wave file %s!\n", path);

        LK_Wave sound;
        sound.samples = 0;
        sound.count = 0;
        sound.channels = 1;
        sound.frequency = 44100;
        return sound;
    }
}

static void play_sound(Game* game, LK_Wave* wave, float volume, float pitch, bool loop)
{
    LK_Sound* slots = game->platform->audio.mixer_slots;
    for (int sound_index = 0; sound_index < LK_MIXER_SLOT_COUNT; sound_index++)
    {
        LK_Sound* sound = slots + sound_index;
        if (sound->playing) continue;

        sound->playing = true;
        sound->wave = *wave;
        sound->wave.frequency = (LK_U32)(sound->wave.frequency * pitch);
        sound->loop = loop;
        sound->volume = volume;

        return;
    }
}

Note* find_closest_note(int lane)
{
    Note* closest = NULL;
    float closest_distance = FLT_MAX;

    float time = the_game->rhythm.time;
    for (Note& note : the_game->rhythm.notes)
    {
        if (note.lane != lane)
        {
            continue;
        }

        float front = note.at - time;
        float back = time - (note.at + note.duration);
        float distance = max_f32(front, back);
        if (note.pressed)
        {
            distance = max_f32(1, distance * 2);
        }

        if (distance < closest_distance)
        {
            closest = &note;
            closest_distance = distance;
        }
    }

    return closest;
}

void grade_note(Note* note)
{
    float hold_duration = note->hold_end - note->hold_start;

    if (note->duration == 0)
    {
        if (hold_duration > 0.4)
        {
            note->score = 0;
            return;
        }

        float distance = fabs(note->at - note->hold_start);
        note->score = max_f32(0, 1 - distance / 0.5);
        return;
    }

    float start = note->at;
    float end = note->at + note->duration;

    float overlap = min_f32(end, note->hold_end) - max_f32(start, note->hold_start);
    if (overlap < 0)
    {
        note->score = 0;
        return;
    }

    note->score = overlap / note->duration;
    note->score -= 2 * (hold_duration - overlap) / hold_duration;
    note->score = max_f32(note->score, 0);
}

static LK_Key LANE_CONTROLS[4] = { LK_KEY_A, LK_KEY_S, LK_KEY_D, LK_KEY_F };

void rhythm_controls()
{
    auto& rhythm = the_game->rhythm;

    for (int lane = 0; lane < 4; lane++)
    {
        LK_Key key = LANE_CONTROLS[lane];

        if (the_game->platform->keyboard.state[key].pressed && !rhythm.holding[lane])
        {
            Note* note = find_closest_note(lane);
            if (note && !note->pressed)
            {
                note->hold_start = rhythm.time;
                rhythm.holding[lane] = note;
            }
        }

        if (rhythm.holding[lane])
        {
            Note* note = rhythm.holding[lane];
            note->hold_end = rhythm.time;
            grade_note(note);

            if (the_game->platform->keyboard.state[key].released)
            {
                note->pressed = true;
                rhythm.holding[lane] = NULL;
            }
        }
    }

    rhythm.time += the_game->platform->time.delta_seconds;
    if (rhythm.time > rhythm.length)
    {
        the_game->combat.doing_rhythm = false;
    }




    LK_Wave* lane_to_sound[] =
    {
        &the_game->sounds.kick,
        &the_game->sounds.snare,
        &the_game->sounds.kick,
        &the_game->sounds.kick
    };

    rhythm.playback_time += the_game->platform->time.delta_seconds;
    for (Note& note : the_game->rhythm.notes)
    {
        if (!note.played && note.at <= the_game->rhythm.playback_time)
        {
            note.played = true;

            LK_Wave* sound = lane_to_sound[note.lane];
            play_sound(the_game, sound, 1, 1, false);
        }
    }

    for (int lane = 0; lane < 4; lane++)
    {
        LK_Key key = LANE_CONTROLS[lane];
        if (the_game->platform->keyboard.state[key].pressed)
        {
            LK_Wave* sound = lane_to_sound[lane];
            play_sound(the_game, sound, 1, 1, false);
        }
    }
}

Vector4 get_color_for_score(float score)
{
    Vector4 red = vector4(1, 0, 0, 1);
    Vector4 yellow = vector4(1, 1, 0, 1);
    Vector4 green = vector4(0, 1, 0, 1);

    if (score < 0.5)
    {
        return lerp(red, yellow, score * 2);
    }
    else
    {
        return lerp(yellow, green, (score - 0.5) * 2);
    }
}

const float RHYTHM_EXIT_WIDTH = 1.5;

void draw_lanes()
{
    float width = the_game->renderer.camera_width;
    float side = RHYTHM_EXIT_WIDTH;

    for (int lane = 0; lane < 2; lane++)
    {
        LK_Key key = LANE_CONTROLS[lane];

        Vector4 color = vector4(1, 1, 1, 1);
        if (the_game->platform->keyboard.state[key].down)
        {
            color = vector4(2, 2, 2, 1);
        }

        push_rectangle({ side, (float) lane }, { width - side, 1 }, the_game->art.marker_bed, color);
    }

    push_rectangle({ 0, 0 }, { side, 2 }, the_game->art.white);
}

void draw_note(Note* note)
{
    auto& rhythm = the_game->rhythm;

    int lane = note->lane;
    float time = note->at;
    float duration = note->duration;

    float side = RHYTHM_EXIT_WIDTH;
    float x_scale = (the_game->renderer.camera_width - side) / rhythm.window;
    float x = (time - rhythm.time) * x_scale + side;
    float width = max_f32(duration * x_scale - 1, 0);
    float y = (float) lane;

    Texture texture = the_game->art.marker;
    float center_u = (texture.uv1.x + texture.uv2.x) * 0.5;

    Texture left = texture;
    Texture middle = texture;
    Texture right = texture;
    left.uv2.x = center_u;
    middle.uv1.x = middle.uv2.x = center_u;
    right.uv1.x = center_u;

    Vector4 color = get_color_for_score(note->score);

    if (note->pressed && note->score < 0.1)
    {
        color = vector4(0.3, 0.3, 0.3, 1);
    }

    push_rectangle({ x,                 y }, { 0.5f,  1 }, left,   color);
    push_rectangle({ x + 0.5f,          y }, { width, 1 }, middle, color);
    push_rectangle({ x + 0.5f + width,  y }, { 0.5f,  1 }, right,  color);

    // static char percentage[50];
    // sprintf(percentage, "%d%%", (int)(note->score * 100 + 0.5));
    // render_string(percentage, x, y + 0.35, 0.3, 0.3);
}

void rhythm_render()
{
    draw_lanes();

    for (Note& note : the_game->rhythm.notes)
    {
        draw_note(&note);
    }

    rendering_flush(&the_game->renderer);
}

void generate_rhythm()
{ 
    auto& rhythm = the_game->rhythm;

    rhythm.length = 2;
    rhythm.window = 2;
    rhythm.playback_time = 0;
    rhythm.time = -2;
    
    int minimum_notes = 4;

    int note_count = 0;
    while (note_count < minimum_notes)
    {
        note_count = 0;
        rhythm.notes.clear();

        for (int i = 0; i < 8; i++)
        {
            float at = i * 0.2f;
            float duration = 0;

            float chance = (i % 2) ? 0.3 : 0.8;
            float roll = random_float();
            if (roll <= chance)
            {
                rhythm.notes.push_back({ 0, at, duration, -1, -1 });
                note_count++;
            }
        }

        for (int i = 0; i < 8; i++)
        {
            float at = i * 0.2f;
            float duration = 0;

            float chance = (i % 2) ? 0.1 : 0.05;
            float roll = random_float();
            if (roll <= chance)
            {
                rhythm.notes.push_back({ 1, at, duration, -1, -1 });
                note_count++;
            }
        }
    }
}

float get_rhythm_score()
{
    float score = 0;

    for (Note& note : the_game->rhythm.notes)
    {
        score += note.score;
    }

    float note_count = (float) the_game->rhythm.notes.size();
    return score / note_count;
}
