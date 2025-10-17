#pragma once

#include <cmath>

extern float deltaTime;
extern float fixedDeltaTime;

#define PI 3.14159265358979323846f
#define DEG2RAD (PI/180.0f)
#define RAD2DEG (180.0f/PI)

struct Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

struct Texture {
    unsigned int id;        // OpenGL texture id
    int width;              // Texture base width
    int height;             // Texture base height
    int mipmaps;            // Mipmap levels, 1 by default
    int format;             // Data format (PixelFormat type)
};

typedef Texture Texture2D;

const static Color RED = { 255, 0, 0, 255 };
const static Color GREEN = { 0, 255, 0, 255 };
const static Color BLUE = { 0, 0, 255, 255 };
const static Color YELLOW = { 255, 255, 0, 255 };
const static Color ORANGE = { 255, 165, 0, 255 };
const static Color PURPLE = { 128, 0, 128, 255 };
const static Color BLACK = { 0, 0, 0, 255 };
const static Color WHITE = { 255, 255, 255, 255 };
const static Color GRAY = { 128, 128, 128, 255 };
const static Color DARKGRAY = { 64, 64, 64, 255 };
const static Color LIGHTGRAY = { 192, 192, 192, 255 };
const static Color BEIGE = { 245, 245, 220, 255 };
const static Color BROWN = { 165, 42, 42, 255 };
const static Color MAROON = { 128, 0, 0, 255 };
const static Color GOLD = { 255, 215, 0, 255 };
const static Color LIME = { 0, 255, 0, 255 };
const static Color PINK = { 255, 192, 203, 255 };
const static Color DARKBLUE = { 0, 0, 139, 255 };
const static Color MAGENTA = { 255, 0, 255, 255 };
const static Color SKYBLUE = { 135, 206, 235, 255 };
const static Color VIOLET = { 238, 130, 238, 255 };

enum BodyType
{
    Dynamic,
    Kinematic,
    Static
};

struct Vector2
{
    float x;
    float y;

    Vector2(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}

    Vector2 Normalize()
    {
        float magnitudeSquared = x * x + y * y; 

        if (magnitudeSquared < 1e-6f)
            return { 0, 0 };

        float invMagnitude = 1.0f / sqrt(magnitudeSquared); // Todo: Consider using rsqrt as it's faster

        x *= invMagnitude;
        y *= invMagnitude;

        return { x, y };
    }

    Vector2 operator+(const Vector2& other) const {
        return { x + other.x, y + other.y };
    }

    Vector2 operator-(const Vector2& other) const {
        return { x - other.x, y - other.y };
    }

    Vector2 operator*(float scalar) const {
        return { x * scalar, y * scalar };
    }

    Vector2 operator/(float scalar) const {
        if (scalar != 0.0f)
            return { x / scalar, y / scalar };
        else
            return { 0.0f, 0.0f }; // Todo: Return error
    }

    Vector2 operator*(const Vector2& other) const {
        return { x * other.x, y * other.y };
    }

    Vector2 operator/(const Vector2& other) const {
        if (other.x != 0.0f && other.y != 0.0f)
            return { x / other.x, y / other.y };
        else
            return { 0.0f, 0.0f }; // Todo: Return error
    }

    Vector2& operator+=(const Vector2& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    Vector2& operator-=(const Vector2& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    Vector2& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    Vector2& operator/=(float scalar) {
        if (scalar != 0.0f) {
            x /= scalar;
            y /= scalar;
        }
        return *this;
    }

    Vector2& operator*=(const Vector2& other) {
        x *= other.x;
        y *= other.y;
        return *this;
    }

    Vector2& operator/=(const Vector2& other) {
        if (other.x != 0.0f && other.y != 0.0f) {
            x /= other.x;
            y /= other.y;
        }
        return *this;
    }

    bool operator==(const Vector2& other) const {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Vector2& other) const {
        return !(*this == other);
    }

    static Vector2 Up() { return { 0.0f, 1.0f }; }
    static Vector2 Down() { return { 0.0f, -1.0f }; }
    static Vector2 Left() { return { -1.0f, 0.0f }; }
    static Vector2 Right() { return { 1.0f, 0.0f }; }
};


struct Vector3
{
	float x;
	float y;
    float z;

    Vector3(float x = 0.0f, float y = 0.0f, float z = 0.0f) : x(x), y(y), z(z) {}

    Vector3 Normalize()
    {
        float magnitudeSquared = x * x + y * y + z * z;

        if (magnitudeSquared < 1e-6f)
            return { 0, 0, 0 };

        float invMagnitude = 1.0f / sqrt(magnitudeSquared); // Todo: Consider using rsqrt as it's faster

        x *= invMagnitude;
        y *= invMagnitude;
        z *= invMagnitude;

        return { x, y, z };
    }

    Vector3 operator+(const Vector3& other) const {
        return { x + other.x, y + other.y, z + other.z };
    }

    Vector3 operator-(const Vector3& other) const {
        return { x - other.x, y - other.y, z - other.z };
    }

    Vector3 operator*(float scalar) const {
        return { x * scalar, y * scalar, z * scalar };
    }
    Vector3 operator*(const Vector3& other) const {
        return { x * other.x, y * other.y, z * other.z };
    }

    Vector3 operator/(float scalar) const {
        if (scalar != 0.0f)
            return { x / scalar, y / scalar, z / scalar };
        else
            return { 0.0f, 0.0f, 0.0f }; // Todo: Send error
    }

    Vector3 operator/(const Vector3& other) const {
        if (other.x == 0 || other.y == 0 || other.z == 0) {
            // Todo: Send error
            return { 0.0f, 0.0f, 0.0f };
        }

        return { x / other.x, y / other.y, z / other.z };
    }

    Vector3& operator+=(const Vector3& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    Vector3& operator-=(const Vector3& other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    Vector3& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    Vector3& operator*=(const Vector3& other) {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        return *this;
    }

    Vector3& operator/=(float scalar) {
        if (scalar != 0.0f) {
            x /= scalar;
            y /= scalar;
            z /= scalar;
        }
        return *this;
    }

    Vector3& operator/=(const Vector3& other) {
        if (other.x != 0.0f && other.y != 0.0f && other.z != 0.0f) {
            x /= other.x;
            y /= other.y;
            z /= other.z;
        }
        return *this;
    }

    bool operator==(const Vector3& other) const {
        return x == other.x && y == other.y && z == other.z;
    }

    bool operator!=(const Vector3& other) const {
        return !(*this == other);
    }

    static Vector3 Forward() { return { 0.0f, 0.0f, 1.0f }; }
    static Vector3 Backward() { return { 0.0f, 0.0f, -1.0f }; }
    static Vector3 Up() { return { 0.0f, 1.0f, 0.0f }; }
    static Vector3 Down() { return { 0.0f, -1.0f, 0.0f }; }
    static Vector3 Right() { return { 1.0f, 0.0f, 0.0f }; }
    static Vector3 Left() { return { -1.0f, 0.0f, 0.0f }; }

    bool Zero() { return x == 0.0f && y == 0.0f && z == 0.0f; }
};

struct Vector4 {
    float x;
    float y;
    float z;
    float w;

    Vector4(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 0.0f) : x(x), y(y), z(z), w(w) {}

    Vector4 operator+(const Vector4& other) const {
        return { x + other.x, y + other.y, z + other.z, w + other.w };
    }

    Vector4 operator-(const Vector4& other) const {
        return { x - other.x, y - other.y, z - other.z, w - other.w };
    }

    Vector4 operator*(float scalar) const {
        return { x * scalar, y * scalar, z * scalar, w * scalar };
    }

    Vector4 operator*(const Vector4& other) const {
        //return { x * other.x, y * other.y, z * other.z, w * other.w };
        Vector4 result = { 0 };

        float qax = x, qay = y, qaz = z, qaw = w;
        float qbx = other.x, qby = other.y, qbz = other.z, qbw = other.w;

        result.x = qax * qbw + qaw * qbx + qay * qbz - qaz * qby;
        result.y = qay * qbw + qaw * qby + qaz * qbx - qax * qbz;
        result.z = qaz * qbw + qaw * qbz + qax * qby - qay * qbx;
        result.w = qaw * qbw - qax * qbx - qay * qby - qaz * qbz;

        return result;
    }

    Vector4 operator/(float scalar) const {
        if (scalar != 0.0f)
            return { x / scalar, y / scalar, z / scalar, w / scalar };
        else
            return { 0.0f, 0.0f, 0.0f, 0.0f }; // Todo: Send error
    }

    Vector4 operator/(const Vector4& other) const {
        if (other.x == 0 || other.y == 0 || other.z == 0 || other.w == 0)
            return { 0.0f, 0.0f, 0.0f }; // Todo: Send error

        return { x / other.x, y / other.y, z / other.z, w / other.w };
    }

    Vector4& operator+=(const Vector4& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        w += other.w;
        return *this;
    }

    Vector4& operator-=(const Vector4& other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        w -= other.w;
        return *this;
    }

    Vector4& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        w *= scalar;
        return *this;
    }

    Vector4& operator/=(float scalar) {
        if (scalar != 0.0f) {
            x /= scalar;
            y /= scalar;
            z /= scalar;
            w /= scalar;
        }
        return *this;
    }

    Vector4& operator*=(const Vector4& other) {
        float qax = x, qay = y, qaz = z, qaw = w;
        float qbx = other.x, qby = other.y, qbz = other.z, qbw = other.w;

        x = qax * qbw + qaw * qbx + qay * qbz - qaz * qby;
        y = qay * qbw + qaw * qby + qaz * qbx - qax * qbz;
        z = qaz * qbw + qaw * qbz + qax * qby - qay * qbx;
        w = qaw * qbw - qax * qbx - qay * qby - qaz * qbz;

        return *this;
    }

    Vector4& operator/=(const Vector4& other) {
        if (other.x != 0.0f && other.y != 0.0f && other.z != 0.0f && other.w != 0.0f) {
            x /= other.x;
            y /= other.y;
            z /= other.z;
            w /= other.w;
        }
        return *this;
    }

    bool operator==(const Vector4& other) const {
        return x == other.x && y == other.y && z == other.z && w == other.w;
    }

    bool operator!=(const Vector4& other) const {
        return !(*this == other);
    }

    static Vector4 Identity()
    {
        return { 0.0f, 0.0f, 0.0f, 1.0f };
    }

    bool Zero()
    {
        return x == 0.0f && y == 0.0f && z == 0.0f && w == 0.0f;
    }
};

typedef Vector4 Quaternion;

Vector3 RotateVector3ByQuaternion(Vector3 vector, Quaternion quaternion);
Quaternion EulerToQuaternion(float roll, float pitch, float yaw);
// Returns Vector3 in Radians.
Vector3 QuaternionToEuler(Quaternion quaternion);
void NormalizeEuler(Vector3& euler);
float GetDeltaTime();
float GetFixedDeltaTime();