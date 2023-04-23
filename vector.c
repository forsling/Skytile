#include "vector.h"
#include "math.h"
#include "world.h"

// Calculate the length of a vec2 vector
float vec2_length(vec2 v) {
    return sqrtf(v.x * v.x + v.y * v.y);
}

// Subtract two vec2 vectors
vec2 vec2_subtract(vec2 a, vec2 b) {
    vec2 result = {a.x - b.x, a.y - b.y};
    return result;
}

// Normalize a vec2 vector
vec2 vec2_normalize(vec2 v) {
    float length = vec2_length(v);
    if (length == 0.0f) {
        return (vec2){0.0f, 0.0f};
    }
    return (vec2){v.x / length, v.y / length};
}

// Multiply a vec2 vector by a scalar
vec2 vec2_multiply_scalar(vec2 v, float scalar) {
    vec2 result = {v.x * scalar, v.y * scalar};
    return result;
}

vec2 vec2_add(vec2 a, vec2 b) {
    vec2 result = {a.x + b.x, a.y + b.y};
    return result;
}

ivec2 get_grid_pos2(float x, float y) {
    ivec2 gridpos = {
        .x = (int)(x / CELL_XY_SCALE),
        .y = (int)(y / CELL_XY_SCALE)
    };
    return gridpos;
}

ivec3 get_grid_pos3(float x, float y, float z) {
    ivec3 layerpos = {
        .x = (int)(x / CELL_XY_SCALE),
        .y = (int)(y / CELL_XY_SCALE),
        .z = (int)floor(z / CELL_Z_SCALE)
    };
    return layerpos;
}

float point_to_aabb_distance(float px, float py, float x1, float y1, float x2, float y2) {
    float clamped_x = fmaxf(x1, fminf(px, x2));
    float clamped_y = fmaxf(y1, fminf(py, y2));

    float dx = px - clamped_x;
    float dy = py - clamped_y;

    return sqrtf(dx * dx + dy * dy);
}
