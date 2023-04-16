#ifndef ENGINE_H
#define ENGINE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include "world.h"

bool init_engine();
void main_loop();
void cleanup_engine();

bool load_engine_assets();
void free_engine_assets();
void render_textured_quad(GLuint texture, float vertices[12]);
void render_world(World* world);
void process_input();
void process_mouse();

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

#endif // ENGINE_H