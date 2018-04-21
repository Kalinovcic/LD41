int32 max_i32(int32 a, int32 b) { return a > b ? a : b; }
int32 min_i32(int32 a, int32 b) { return a < b ? a : b; }

float max_f32(float a, float b) { return a > b ? a : b; }
float min_f32(float a, float b) { return a < b ? a : b; }


Vector2 vector2(float x, float y                  ) { return { x, y       }; }
Vector3 vector3(float x, float y, float z         ) { return { x, y, z    }; }
Vector4 vector4(float x, float y, float z, float w) { return { x, y, z, w }; }
Vector4 vector4(Vector3 v, float w) { return { v.x, v.y, v.z, w }; }

Vector2 operator-(Vector2 v) { return vector2(-v.x, -v.y            ); }
Vector3 operator-(Vector3 v) { return vector3(-v.x, -v.y, -v.z      ); }
Vector4 operator-(Vector4 v) { return vector4(-v.x, -v.y, -v.z, -v.w); }

Vector2 operator+(Vector2 lhs, Vector2 rhs) { return vector2(lhs.x + rhs.x, lhs.y + rhs.y); }
Vector2 operator+(Vector2 lhs, float   rhs) { return vector2(lhs.x + rhs,   lhs.y + rhs  ); }
Vector2 operator+(float   lhs, Vector2 rhs) { return vector2(lhs   + rhs.x, lhs   + rhs.y); }
Vector2 operator-(Vector2 lhs, Vector2 rhs) { return vector2(lhs.x - rhs.x, lhs.y - rhs.y); }
Vector2 operator-(Vector2 lhs, float   rhs) { return vector2(lhs.x - rhs,   lhs.y - rhs  ); }
Vector2 operator-(float   lhs, Vector2 rhs) { return vector2(lhs   - rhs.x, lhs   - rhs.y); }
Vector2 operator*(Vector2 lhs, Vector2 rhs) { return vector2(lhs.x * rhs.x, lhs.y * rhs.y); }
Vector2 operator*(Vector2 lhs, float   rhs) { return vector2(lhs.x * rhs,   lhs.y * rhs  ); }
Vector2 operator*(float   lhs, Vector2 rhs) { return vector2(lhs   * rhs.x, lhs   * rhs.y); }
Vector2 operator/(Vector2 lhs, Vector2 rhs) { return vector2(lhs.x / rhs.x, lhs.y / rhs.y); }
Vector2 operator/(Vector2 lhs, float   rhs) { return vector2(lhs.x / rhs,   lhs.y / rhs  ); }
Vector2 operator/(float   lhs, Vector2 rhs) { return vector2(lhs   / rhs.x, lhs   / rhs.y); }

Vector2& operator+=(Vector2& lhs, Vector2 rhs) { return lhs = lhs + rhs; }
Vector2& operator+=(Vector2& lhs, float   rhs) { return lhs = lhs + rhs; }
Vector2& operator-=(Vector2& lhs, Vector2 rhs) { return lhs = lhs - rhs; }
Vector2& operator-=(Vector2& lhs, float   rhs) { return lhs = lhs - rhs; }
Vector2& operator*=(Vector2& lhs, Vector2 rhs) { return lhs = lhs * rhs; }
Vector2& operator*=(Vector2& lhs, float   rhs) { return lhs = lhs * rhs; }
Vector2& operator/=(Vector2& lhs, Vector2 rhs) { return lhs = lhs / rhs; }
Vector2& operator/=(Vector2& lhs, float   rhs) { return lhs = lhs / rhs; }

Vector3 operator+(Vector3 lhs, Vector3 rhs) { return vector3(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z); }
Vector3 operator+(Vector3 lhs, float   rhs) { return vector3(lhs.x + rhs,   lhs.y + rhs,   lhs.z + rhs  ); }
Vector3 operator+(float   lhs, Vector3 rhs) { return vector3(lhs   + rhs.x, lhs   + rhs.y, lhs   + rhs.z); }
Vector3 operator-(Vector3 lhs, Vector3 rhs) { return vector3(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z); }
Vector3 operator-(Vector3 lhs, float   rhs) { return vector3(lhs.x - rhs,   lhs.y - rhs,   lhs.z - rhs  ); }
Vector3 operator-(float   lhs, Vector3 rhs) { return vector3(lhs   - rhs.x, lhs   - rhs.y, lhs   - rhs.z); }
Vector3 operator*(Vector3 lhs, Vector3 rhs) { return vector3(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z); }
Vector3 operator*(Vector3 lhs, float   rhs) { return vector3(lhs.x * rhs,   lhs.y * rhs,   lhs.z * rhs  ); }
Vector3 operator*(float   lhs, Vector3 rhs) { return vector3(lhs   * rhs.x, lhs   * rhs.y, lhs   * rhs.z); }
Vector3 operator/(Vector3 lhs, Vector3 rhs) { return vector3(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z); }
Vector3 operator/(Vector3 lhs, float   rhs) { return vector3(lhs.x / rhs,   lhs.y / rhs,   lhs.z / rhs  ); }
Vector3 operator/(float   lhs, Vector3 rhs) { return vector3(lhs   / rhs.x, lhs   / rhs.y, lhs   / rhs.z); }

Vector3& operator+=(Vector3& lhs, Vector3 rhs) { return lhs = lhs + rhs; }
Vector3& operator+=(Vector3& lhs, float   rhs) { return lhs = lhs + rhs; }
Vector3& operator-=(Vector3& lhs, Vector3 rhs) { return lhs = lhs - rhs; }
Vector3& operator-=(Vector3& lhs, float   rhs) { return lhs = lhs - rhs; }
Vector3& operator*=(Vector3& lhs, Vector3 rhs) { return lhs = lhs * rhs; }
Vector3& operator*=(Vector3& lhs, float   rhs) { return lhs = lhs * rhs; }
Vector3& operator/=(Vector3& lhs, Vector3 rhs) { return lhs = lhs / rhs; }
Vector3& operator/=(Vector3& lhs, float   rhs) { return lhs = lhs / rhs; }

Vector4 operator+(Vector4 lhs, Vector4 rhs) { return vector4(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w); }
Vector4 operator+(Vector4 lhs, float   rhs) { return vector4(lhs.x + rhs,   lhs.y + rhs,   lhs.z + rhs,   lhs.w + rhs  ); }
Vector4 operator+(float   lhs, Vector4 rhs) { return vector4(lhs   + rhs.x, lhs   + rhs.y, lhs   + rhs.z, lhs   + rhs.w); }
Vector4 operator-(Vector4 lhs, Vector4 rhs) { return vector4(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w); }
Vector4 operator-(Vector4 lhs, float   rhs) { return vector4(lhs.x - rhs,   lhs.y - rhs,   lhs.z - rhs,   lhs.w - rhs  ); }
Vector4 operator-(float   lhs, Vector4 rhs) { return vector4(lhs   - rhs.x, lhs   - rhs.y, lhs   - rhs.z, lhs   - rhs.w); }
Vector4 operator*(Vector4 lhs, Vector4 rhs) { return vector4(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w); }
Vector4 operator*(Vector4 lhs, float   rhs) { return vector4(lhs.x * rhs,   lhs.y * rhs,   lhs.z * rhs,   lhs.w * rhs  ); }
Vector4 operator*(float   lhs, Vector4 rhs) { return vector4(lhs   * rhs.x, lhs   * rhs.y, lhs   * rhs.z, lhs   * rhs.w); }
Vector4 operator/(Vector4 lhs, Vector4 rhs) { return vector4(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z, lhs.w / rhs.w); }
Vector4 operator/(Vector4 lhs, float   rhs) { return vector4(lhs.x / rhs,   lhs.y / rhs,   lhs.z / rhs,   lhs.w / rhs  ); }
Vector4 operator/(float   lhs, Vector4 rhs) { return vector4(lhs   / rhs.x, lhs   / rhs.y, lhs   / rhs.z, lhs   / rhs.w); }

Vector4& operator+=(Vector4& lhs, Vector4 rhs) { return lhs = lhs + rhs; }
Vector4& operator+=(Vector4& lhs, float   rhs) { return lhs = lhs + rhs; }
Vector4& operator-=(Vector4& lhs, Vector4 rhs) { return lhs = lhs - rhs; }
Vector4& operator-=(Vector4& lhs, float   rhs) { return lhs = lhs - rhs; }
Vector4& operator*=(Vector4& lhs, Vector4 rhs) { return lhs = lhs * rhs; }
Vector4& operator*=(Vector4& lhs, float   rhs) { return lhs = lhs * rhs; }
Vector4& operator/=(Vector4& lhs, Vector4 rhs) { return lhs = lhs / rhs; }
Vector4& operator/=(Vector4& lhs, float   rhs) { return lhs = lhs / rhs; }

float dot(Vector2 a, Vector2 b) { return a.x * b.x + a.y * b.y;                         }
float dot(Vector3 a, Vector3 b) { return a.x * b.x + a.y * b.y + a.z * b.z;             }
float dot(Vector4 a, Vector4 b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }

float length_squared(Vector2 v) { return v.x * v.x + v.y * v.y;                         }
float length_squared(Vector3 v) { return v.x * v.x + v.y * v.y + v.z * v.z;             }
float length_squared(Vector4 v) { return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w; }

float length(Vector2 v) { return sqrtf(v.x * v.x + v.y * v.y                        ); }
float length(Vector3 v) { return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z            ); }
float length(Vector4 v) { return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w); }

Vector2 noz(Vector2 v) { float l = length(v); if (l == 0) return v; return v / l; }
Vector3 noz(Vector3 v) { float l = length(v); if (l == 0) return v; return v / l; }
Vector4 noz(Vector4 v) { float l = length(v); if (l == 0) return v; return v / l; }

Vector3 cross(Vector3 a, Vector3 b)
{
    return vector3((a.y * b.z) - (a.z * b.y),
                   (a.z * b.x) - (a.x * b.z),
                   (a.x * b.y) - (a.y * b.x));
}


float   lerp(float a,   float b,   float t) { return a * (1 - t) + b * t; }
Vector2 lerp(Vector2 a, Vector2 b, float t) { return a * (1 - t) + b * t; }
Vector3 lerp(Vector3 a, Vector3 b, float t) { return a * (1 - t) + b * t; }
Vector4 lerp(Vector4 a, Vector4 b, float t) { return a * (1 - t) + b * t; }


#define SetMatrix(out, a00, a01, a02, a03, a10, a11, a12, a13, a20, a21, a22, a23, a30, a31, a32, a33) \
    (out) = {{{ a00, a10, a20, a30 }, { a01, a11, a21, a31 }, { a02, a12, a22, a32 }, { a03, a13, a23, a33 }}};

Matrix4 operator*(Matrix4 left, Matrix4 right)
{
    Matrix4 result;

    for (int x = 0; x < 4; x++)
        for (int y = 0; y < 4; y++)
        {
            float sum = 0;
            for (int k = 0; k < 4; k++)
                sum += left.m[k][y] * right.m[x][k];
            result.m[x][y] = sum;
        }

    return result;
}

void operator*=(Matrix4& left, Matrix4 right)
{
    left = left * right;
}

Matrix4 identity()
{
    Matrix4 result;
    SetMatrix(result,
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    );

    return result;
}

Matrix4 transpose(Matrix4 m)
{
    Matrix4 result;
    SetMatrix(result,
        m.m[0][0], m.m[0][1], m.m[0][2], m.m[0][3],
        m.m[1][0], m.m[1][1], m.m[1][2], m.m[1][3],
        m.m[2][0], m.m[2][1], m.m[2][2], m.m[2][3],
        m.m[3][0], m.m[3][1], m.m[3][2], m.m[3][3]
    );

    return result;
}

Matrix4 translate(Vector3 v)
{
    Matrix4 result;
    SetMatrix(result,
        1, 0, 0, v.x,
        0, 1, 0, v.y,
        0, 0, 1, v.z,
        0, 0, 0,   1
    );

    return result;
}

Matrix4 scale(Vector3 v)
{
    Matrix4 result;
    SetMatrix(result,
        v.x,   0,   0,   0,
          0, v.y,   0,   0,
          0,   0, v.z,   0,
          0,   0,   0,   1
    );

    return result;
}

Matrix4 rotate_x(float a)
{
    Matrix4 result;
    SetMatrix(result,
        1,      0,       0, 0,
        0, cos(a), -sin(a), 0,
        0, sin(a),  cos(a), 0,
        0,      0,       0, 1
    );

    return result;
}

Matrix4 rotate_y(float a)
{
    Matrix4 result;
    SetMatrix(result,
        cos(a), 0, sin(a), 0,
             0, 1,      0, 0,
       -sin(a), 0, cos(a), 0,
             0, 0,      0, 1
    );

    return result;
}

Matrix4 rotate_z(float a)
{
    Matrix4 result;
    SetMatrix(result,
        cos(a), -sin(a), 0, 0,
        sin(a),  cos(a), 0, 0,
             0,       0, 1, 0,
             0,       0, 0, 1
    );

    return result;
}

Matrix4 orthographic(float l, float r, float b, float t, float n, float f)
{
    float w = r - l;
    float h = t - b;
    float d = f - n;

    Matrix4 result;
    SetMatrix(result,
        2 / w,     0,     0, -(r + l) / w,
            0, 2 / h,     0, -(t + b) / h,
            0,     0, 2 / d, -(f + n) / d,
            0,     0,     0,            1
    );

    return result;
}

Matrix4 look_at(Vector3 eye, Vector3 at, Vector3 up)
{
    Vector3 z = noz(at - eye);
    Vector3 x = noz(cross(z, up));
    Vector3 y = cross(x, z);

    Matrix4 result;
    SetMatrix(result,
        x.x,  x.y,  x.z, -dot(x, eye),
        y.x,  y.y,  y.z, -dot(y, eye),
       -z.x, -z.y, -z.z,  dot(z, eye),
          0,    0,    0,             1
    );

    return result;
}

Matrix4 perspective(float n, float f, float fovy, float aspect)
{
    float t = n * tanf(fovy / 2);
    float r = t * aspect;
    float fpn = f + n;
    float fmn = f - n;
    float fn2 = f * n * 2;

    Matrix4 result;
    SetMatrix(result,
        n / r,     0,         0,           0,
            0, n / t,         0,           0,
            0,     0, -fpn / fmn, -fn2 / fmn,
            0,     0,         -1,          0
    );

    return result;
}