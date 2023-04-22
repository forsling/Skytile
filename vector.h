#ifndef VECTOR_H
#define VECTOR_H

typedef struct {
    float x;
    float y;
    float z;
} Vec3;

typedef struct {
    float x, y;
} Vec2;

typedef struct {
    int x, y, z;
} ivec3;

typedef struct {
    int x, y;
} ivec2;

Vec2 Vec2_subtract(Vec2 a, Vec2 b);
float Vec2_length(Vec2 v);
Vec2 Vec2_normalize(Vec2 v);
Vec2 Vec2_multiply_scalar(Vec2 v, float scalar);
Vec2 Vec2_add(Vec2 a, Vec2 b);
float point_to_aabb_distance(float px, float py, float x1, float y1, float x2, float y2);

#endif // VECTOR_H
