#ifndef ENGINE_H
#define ENGINE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include "world.h"

typedef struct {
    float x;
    float y;
    float z;
} Vec3;

typedef struct {
    Vec3 position;
    float pitch, yaw; // Camera rotation
    float speed; // Movement speed
} Player;

typedef enum {
    DIR_EAST,
    DIR_NORTH,
    DIR_WEST,
    DIR_SOUTH,
    DIR_DOWN,
    DIR_UP,
} Direction;

bool init_engine();
void main_loop();
void cleanup_engine();

bool load_engine_assets();
void free_engine_assets();
void render_textured_quad(GLuint texture, Vec3 a, Vec3 b, Vec3 c, Vec3 d, float v_scale);
void render_world(World* world);
void process_input();
void process_mouse();

Cell *get_cell(World *world, int x, int y);
bool is_solid_cell(World *world, int x, int y);
bool is_out_of_bounds(World *world, int x, int y);
bool is_within_bounds(World *world, int x, int y);
void calculate_vertices(Vec3 vertices[4], int x, int y, Direction direction);

#endif // ENGINE_H