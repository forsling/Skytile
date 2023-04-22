#include "vector.h"
#include "math.h"

// Calculate the length of a Vec2 vector
float Vec2_length(Vec2 v) {
    return sqrtf(v.x * v.x + v.y * v.y);
}

// Subtract two Vec2 vectors
Vec2 Vec2_subtract(Vec2 a, Vec2 b) {
    Vec2 result = {a.x - b.x, a.y - b.y};
    return result;
}

// Normalize a Vec2 vector
Vec2 Vec2_normalize(Vec2 v) {
    float length = Vec2_length(v);
    if (length == 0.0f) {
        return (Vec2){0.0f, 0.0f};
    }
    return (Vec2){v.x / length, v.y / length};
}

// Multiply a Vec2 vector by a scalar
Vec2 Vec2_multiply_scalar(Vec2 v, float scalar) {
    Vec2 result = {v.x * scalar, v.y * scalar};
    return result;
}

Vec2 Vec2_add(Vec2 a, Vec2 b) {
    Vec2 result = {a.x + b.x, a.y + b.y};
    return result;
}

float point_to_aabb_distance(float px, float py, float x1, float y1, float x2, float y2) {
    float clamped_x = fmaxf(x1, fminf(px, x2));
    float clamped_y = fmaxf(y1, fminf(py, y2));

    float dx = px - clamped_x;
    float dy = py - clamped_y;

    return sqrtf(dx * dx + dy * dy);
}
