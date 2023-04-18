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

void process_input();
void process_mouse();

void render_faceX(float x, float y, float z, float size, Direction direction, GLuint texture);
void render_face(float x, float y, float z, float sizeX, float sizeY, GLuint texture, Direction direction);
void render_world(World* world);

int get_level_from_z(float z, World *world);
Cell *get_cell(Level *level, int x, int y);
bool is_solid_cell(Level *level, int x, int y);
bool is_out_of_bounds(Level *level, int x, int y);
bool is_within_bounds(Level *level, int x, int y);

#endif // ENGINE_H