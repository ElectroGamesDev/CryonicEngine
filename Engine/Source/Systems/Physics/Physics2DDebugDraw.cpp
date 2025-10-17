#include "Physics2DDebugDraw.h"
#include "ThirdParty/Raylib/include/raylib.h"
#include "Utilities/ConsoleLogger.h"

void Physics2DDebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) {
    Color raylibColor = { static_cast<unsigned char>(color.r * 255),
                          static_cast<unsigned char>(color.g * 255),
                          static_cast<unsigned char>(color.b * 255),
                          255 };

    for (int i = 0; i < vertexCount - 1; ++i) {
        Vector2 point1 = { vertices[i].x, vertices[i].y };
        Vector2 point2 = { vertices[i + 1].x, vertices[i + 1].y };
        DrawLineV(point1, point2, raylibColor);
    }
    Vector2 lastPoint = { vertices[vertexCount - 1].x, vertices[vertexCount - 1].y };
    Vector2 firstPoint = { vertices[0].x, vertices[0].y };
    DrawLineV(lastPoint, firstPoint, raylibColor);
}

void Physics2DDebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
    Vector2* points = new Vector2[vertexCount];

    for (int i = 0; i < vertexCount; ++i) {
        points[i] = { vertices[i].x, vertices[i].y };
    }

    DrawTriangleFan(points, vertexCount, { static_cast<unsigned char>(color.r * 255), static_cast<unsigned char>(color.g * 255), static_cast<unsigned char>(color.b * 255), 255 });

    delete[] points;
}

void Physics2DDebugDraw::DrawCircle(const b2Vec2& center, float radius, const b2Color& color)
{
    int segments = 32;

    // Loop through segments and draw lines
    float angle = 0.0f;
    float angleStep = 2.0f * 3.1415926535f / segments;
    for (int i = 0; i < segments; ++i) {
        float x1 = center.x + radius * cosf(angle);
        float y1 = center.y + radius * sinf(angle);
        float x2 = center.x + radius * cosf(angle + angleStep);
        float y2 = center.y + radius * sinf(angle + angleStep);
        DrawLineV(Vector2{ x1, y1 }, Vector2{ x2, y2 }, { static_cast<unsigned char>(color.r * 255), static_cast<unsigned char>(color.g * 255), static_cast<unsigned char>(color.b * 255), 255 });
        angle += angleStep;
    }
}

void Physics2DDebugDraw::DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color)
{
    int segments = 32;

    Vector2* points = new Vector2[segments + 1];

    float angle = 0.0f;
    float angleStep = 2.0f * 3.1415926535f / segments;
    for (int i = 0; i <= segments; ++i) {
        float x = center.x + radius * cosf(angle);
        float y = center.y + radius * sinf(angle);
        points[i] = { x, y };
        angle += angleStep;
    }

    DrawTriangleFan(points, segments + 1, { static_cast<unsigned char>(color.r * 255), static_cast<unsigned char>(color.g * 255), static_cast<unsigned char>(color.b * 255), 255 });

    delete[] points;
}

void Physics2DDebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
    DrawLineV({ p1.x, p1.y }, { p2.x, p2.y }, { static_cast<unsigned char>(color.r * 255), static_cast<unsigned char>(color.g * 255), static_cast<unsigned char>(color.b * 255), 255 });
}

void Physics2DDebugDraw::DrawTransform(const b2Transform& xf) {
}

void Physics2DDebugDraw::DrawPoint(const b2Vec2& p, float size, const b2Color& color)
{
    DrawCircleV(Vector2{ p.x, p.y }, size / 2.0f, { static_cast<unsigned char>(color.r * 255), static_cast<unsigned char>(color.g * 255), static_cast<unsigned char>(color.b * 255), 255 });
}