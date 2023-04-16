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

bool init_engine();
void main_loop();
void cleanup_engine();

bool load_engine_assets();
void free_engine_assets();
void render_textured_quad(GLuint texture, Vec3 a, Vec3 b, Vec3 c, Vec3 d);
void render_world(World* world);
void process_input();
void process_mouse();
bool is_solid_cell(World *world, int x, int y);

#endif // ENGINE_H