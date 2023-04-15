#ifndef ENGINE_H
#define ENGINE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include "world.h"

bool init_engine();
void main_loop();
void cleanup_engine();

bool load_engine_assets();
void free_engine_assets();
void render_world(World* world);

#endif // ENGINE_H