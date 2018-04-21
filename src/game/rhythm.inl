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
    rhythm.length = 10;
    rhythm.window = 7;
    rhythm.time = fmod(the_game->platform->time.seconds, rhythm.length + 2) - 2;

    static bool created = false;
    if (rhythm.time < 0)
    {
        if (!created)
        {
            rhythm.notes.clear();
            for (int i = 0; i < 6; i++)
            {
                float at = rand() % 1000 / 1000.0f * 8;
                float duration = rand() % 1000 / 1000.0f * 2;
                if (duration < 1.5)
                {
                    duration = 0;
                }

                rhythm.notes.push_back({ rand() % 2, at, duration, -1, -1 });
            }

            memset(rhythm.holding, 0, sizeof(rhythm.holding));
        }
        created = true;
    }
    else
    {
        created = false;
    }

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
}

const float RHYTHM_EXIT_WIDTH = 1.5;

void draw_lanes()
{
    float width = the_game->renderer.camera_width;
    float side = RHYTHM_EXIT_WIDTH;

    for (int lane = 0; lane < 4; lane++)
    {
        LK_Key key = LANE_CONTROLS[lane];

        Vector4 color = vector4(1, 1, 1, 1);
        if (the_game->platform->keyboard.state[key].down)
        {
            color = vector4(2, 2, 2, 1);
        }

        push_rectangle({ side, (float) lane }, { width - side, 1 }, the_game->art.marker_bed, color);
    }

    push_rectangle({ 0, 0 }, { side, 4 }, the_game->art.white);
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

    Vector4 red = vector4(1, 0, 0, 1);
    Vector4 yellow = vector4(1, 1, 0, 1);
    Vector4 green = vector4(0, 1, 0, 1);

    Vector4 color;
    if (note->score < 0.5)
        color = lerp(red, yellow, note->score * 2);
    else
        color = lerp(yellow, green, (note->score - 0.5) * 2);

    if (note->pressed && note->score < 0.1)
    {
        color = vector4(0.3, 0.3, 0.3, 1);
    }

    push_rectangle({ x,                 y }, { 0.5f,  1 }, left,   color);
    push_rectangle({ x + 0.5f,          y }, { width, 1 }, middle, color);
    push_rectangle({ x + 0.5f + width,  y }, { 0.5f,  1 }, right,  color);
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
