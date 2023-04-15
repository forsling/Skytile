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
void render_world(World* world, int window_width, int window_height);
void render_textured_quad(GLuint texture, float x, float y, float z, float width, float height);
void process_input(const Uint8* keystate);

typedef struct {
    float x, y, z; // Position
    float pitch, yaw; // Camera rotation
    float speed; // Movement speed
} Player;

#endif // ENGINE_H