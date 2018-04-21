struct Atlas
{
    int width;
    int height;
    uint8* data;

    int cursor_x;
    int cursor_y;
    int line_height;
};

struct Texture
{
    Vector2 uv1;
    Vector2 uv2;
};

void create_atlas(Atlas* atlas, int size)
{
    atlas->width = size;
    atlas->height = size;
    atlas->data = (uint8*) calloc(1, size * size * 4);

    atlas->cursor_x = 0;
    atlas->cursor_y = 0;
    atlas->line_height = 0;
}

Texture add_texture(Atlas* atlas, const char* path)
{
    int width;
    int height;
    int dont_care;
    uint8* pixels = stbi_load(path, &width, &height, &dont_care, 4);
    if (!pixels)
    {
        printf("Failed to load texture %s\n", path);
        return {};
    }

    if (width > atlas->width)
    {
        goto doesnt_fit;
    }

    if (atlas->cursor_x + width > atlas->width)
    {
        atlas->cursor_x = 0;
        atlas->cursor_y += atlas->line_height;
        atlas->line_height = 0;
    }

    if (atlas->cursor_y + height > atlas->height)
    {
        goto doesnt_fit;
    }

    atlas->line_height = max_i32(atlas->line_height, height);
    for (int y = 0, wy = atlas->cursor_y; y < height; y++, wy++)
    {
        int line_width = width * 4;
        uint8* source = pixels + y * line_width;
        uint8* destination = atlas->data + (wy * atlas->width + atlas->cursor_x) * 4;
        memcpy(destination, source, line_width);
    }

    stbi_image_free(pixels);

    Texture result;
    result.uv1.x = (float) atlas->cursor_x / (float) atlas->width;
    result.uv1.y = (float)(atlas->cursor_y + height) / (float) atlas->height;
    result.uv2.x = (float)(atlas->cursor_x + width) / (float) atlas->width;
    result.uv2.y = (float) atlas->cursor_y / (float) atlas->height;

    atlas->cursor_x += width;

    return result;

doesnt_fit:;
    printf("Texture %s doesnt fit!\n", path);
    return {};
}



struct Vertex
{
    Vector2 position;
    Vector2 uv;
    Vector4 color;
};

static std::vector<Vertex> render_vertices;
static std::vector<uint16> render_indices;

void push_rectangle(float x, float y, float width, float height,
                    float u1, float v1, float u2, float v2,
                    Vector4 color)
{
    float x1 = x;
    float x2 = x + width;
    float y1 = y;
    float y2 = y + height;

    uint16 base_index = render_vertices.size();

    render_vertices.push_back({ { x1, y1 }, { u1, v1 }, color });
    render_vertices.push_back({ { x2, y1 }, { u2, v1 }, color });
    render_vertices.push_back({ { x2, y2 }, { u2, v2 }, color });
    render_vertices.push_back({ { x1, y2 }, { u1, v2 }, color });

    render_indices.push_back(base_index + 0);
    render_indices.push_back(base_index + 1);
    render_indices.push_back(base_index + 2);
    render_indices.push_back(base_index + 0);
    render_indices.push_back(base_index + 2);
    render_indices.push_back(base_index + 3);
}

void push_rectangle(float x, float y, float width, float height, Texture texture, Vector4 color = { 1, 1, 1, 1 })
{
    push_rectangle(x, y, width, height, texture.uv1.x, texture.uv1.y, texture.uv2.x, texture.uv2.y, color);
}

void push_rectangle(Vector2 position, Vector2 size, Texture texture, Vector4 color = { 1, 1, 1, 1 })
{
    push_rectangle(position.x, position.y, size.x, size.y, texture.uv1.x, texture.uv1.y, texture.uv2.x, texture.uv2.y, color);
}

void push_centered_rectangle(Vector2 position, Vector2 size, Texture texture, Vector4 color = { 1, 1, 1, 1 })
{
    float x = position.x - size.x * 0.5;
    float y = position.y - size.y * 0.5;
    push_rectangle(x, y, size.x, size.y, texture.uv1.x, texture.uv1.y, texture.uv2.x, texture.uv2.y, color);
}



struct Renderer
{
    Atlas atlas;

    GLuint shader;
    GLuint atlas_texture;
    GLuint vao;
    GLuint vbo;
    GLuint ibo;

    Matrix4 camera_transform;
    float camera_width;
    float camera_height;
};

void init_renderer(Renderer* renderer)
{
    glGenVertexArrays(1, &renderer->vao);
    glBindVertexArray(renderer->vao);

    glGenBuffers(1, &renderer->vbo);
    glGenBuffers(1, &renderer->ibo);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(Vertex), (void*) offsetof(Vertex, position));
    glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(Vertex), (void*) offsetof(Vertex, uv));
    glVertexAttribPointer(2, 4, GL_FLOAT, false, sizeof(Vertex), (void*) offsetof(Vertex, color));



    const char* vs_source =
        "#version 430\n"
        "uniform mat4 transform;\n"
        "layout(location = 0) in vec2 vertex_position;\n"
        "layout(location = 1) in vec2 vertex_uv;\n"
        "layout(location = 2) in vec4 vertex_color;\n"
        "out vec2 fragment_uv;\n"
        "out vec4 fragment_color;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(vertex_position, 0, 1) * transform;\n"
        "    fragment_uv = vertex_uv;\n"
        "    fragment_color = vertex_color;\n"
        "}\n";

    const char* fs_source =
        "#version 430\n"
        "uniform sampler2D atlas;\n"
        "in vec2 fragment_uv;\n"
        "in vec4 fragment_color;\n"
        "out vec4 pixel_color;\n"
        "void main()\n"
        "{\n"
        "    pixel_color = texture(atlas, fragment_uv) * fragment_color;\n"
        "}\n";

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vs, 1, &vs_source, NULL);
    glShaderSource(fs, 1, &fs_source, NULL);
    glCompileShader(vs);
    glCompileShader(fs);

    GLuint shader = renderer->shader = glCreateProgram();
    glAttachShader(shader, vs);
    glAttachShader(shader, fs);
    glLinkProgram(shader);

    glDetachShader(shader, vs);
    glDetachShader(shader, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);


    Atlas* atlas = &renderer->atlas;
    glGenTextures(1, &renderer->atlas_texture);
    glBindTexture(GL_TEXTURE_2D, renderer->atlas_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, atlas->width, atlas->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, atlas->data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

void rendering_flush(Renderer* renderer)
{
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    glBufferData(GL_ARRAY_BUFFER, render_vertices.size() * sizeof(Vertex), &render_vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, render_indices.size() * sizeof(uint16), &render_indices[0], GL_STATIC_DRAW);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderer->atlas_texture);

    GLuint shader = renderer->shader;
    glUseProgram(shader);
    GLint transform = glGetUniformLocation(shader, "transform");
    GLint atlas = glGetUniformLocation(shader, "atlas");
    glUniformMatrix4fv(transform, 1, true, renderer->camera_transform.e);
    glUniform1i(atlas, 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(renderer->vao);
    glDrawElements(GL_TRIANGLES, render_indices.size(), GL_UNSIGNED_SHORT, 0);

    render_vertices.clear();
    render_indices.clear();
}