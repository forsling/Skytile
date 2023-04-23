#ifndef VECTOR_H
#define VECTOR_H

typedef struct {
    float x;
    float y;
    float z;
} vec3;

typedef struct {
    float x, y;
} vec2;

typedef struct {
    int x, y, z;
} ivec3;

typedef struct {
    int x, y;
} ivec2;

vec2 vec2_subtract(vec2 a, vec2 b);
float vec2_length(vec2 v);
vec2 vec2_normalize(vec2 v);
vec2 vec2_multiply_scalar(vec2 v, float scalar);
vec2 vec2_add(vec2 a, vec2 b);
ivec2 get_grid_pos2(float x, float y);
ivec3 get_grid_pos3(float x, float y, float z);
float point_to_aabb_distance(float px, float py, float x1, float y1, float x2, float y2);

#endif // VECTOR_H
