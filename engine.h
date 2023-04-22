#ifndef ENGINE_H
#define ENGINE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <math.h>
#include "world.h"

#define MAX_CELLS 16

typedef struct {
    float x;
    float y;
    float z;
} Vec3;

typedef struct {
    float x, y;
} Vec2;

typedef struct {
    Cell* cell;
    Vec2 position;
} CellInfo;

typedef struct {
    Vec3 position;
    float pitch, yaw;
    float speed;
    float velocity_z;
    float jump_velocity;
    float height;
    float size;
} Player;

typedef enum {
    DIR_EAST,
    DIR_WEST,
    DIR_SOUTH,
    DIR_NORTH,
    DIR_DOWN,
    DIR_UP,
} Direction;

bool init_engine();
void main_loop();
void cleanup_engine();
bool load_engine_assets();
void free_engine_assets();

void process_input(World *world, float deltaTime);
void process_mouse();
void update_player_position(Player *player, World *world, float dx, float dy, float deltaTime);

void render_face(float x, float y, float z, float sizeX, float sizeY, GLuint texture, Direction direction);
void render_world(World* world);

Cell *get_cell(Level *level, int x, int y);
bool is_out_of_xy_bounds(Level *level, int x, int y);
bool is_within_xy_bounds(Level *level, int x, int y);
bool get_next_z_obstacle(World *world, int cell_x, int cell_y, float z_pos, float *out_obstacle_z);
Vec2 get_furthest_legal_position(Level *level, Vec2 source, Vec2 destination, float collision_buffer);
CellInfo *get_cells_for_vector(Level *level, Vec2 source, Vec2 destination, int *num_cells);

Vec2 Vec2_subtract(Vec2 a, Vec2 b);
float Vec2_length(Vec2 v);
Vec2 Vec2_normalize(Vec2 v);
Vec2 Vec2_multiply_scalar(Vec2 v, float scalar);
Vec2 Vec2_add(Vec2 a, Vec2 b);
float point_to_aabb_distance(float px, float py, float x1, float y1, float x2, float y2);

#endif // ENGINE_H