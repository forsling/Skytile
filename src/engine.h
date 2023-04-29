#ifndef ENGINE_H
#define ENGINE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include "world.h"
#include "vector.h"
#include "game_logic.h"

// Engine initialization and cleanup functions
bool init_engine();
void main_loop();
void cleanup_engine();

// Asset loading and freeing functions
bool load_engine_assets();
void free_engine_assets();

#endif // ENGINE_H