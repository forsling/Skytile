#ifndef VECTOR_H
#define VECTOR_H
#include <SDL2/SDL_image.h>

typedef struct {
    float x, y;
} vec2;

typedef struct {
    int x, y;
} ivec2;

typedef struct {
    float x;
    float y;
    float z;
} vec3;

typedef struct {
    int x, y, z;
} ivec3;

vec2 vec2_subtract(vec2 a, vec2 b);
float vec2_length(vec2 v);
vec2 vec2_normalize(vec2 v);
vec2 vec2_multiply_scalar(vec2 v, float scalar);
vec2 vec2_add(vec2 a, vec2 b);
ivec2 get_grid_pos2(float x, float y);
float point_to_aabb_distance(float px, float py, float x1, float y1, float x2, float y2);


vec3 vec3_subtract(vec3 a, vec3 b);
vec3 vec3_add(vec3 a, vec3 b);
vec3 vec3_normalize(vec3 a);
vec3 vec3_multiply_scalar(vec3 a, float scalar);
float vec3_length(vec3 a);

ivec3 get_grid_pos3(float x, float y, float z);
float point_to_aabb_distance_3d(float px, float py, float pz, float min_x, float min_y, float min_z, float max_x, float max_y, float max_z);

#endif // VECTOR_H
