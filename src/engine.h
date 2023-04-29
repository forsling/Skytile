#ifndef ENGINE_H
#define ENGINE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include "world.h"
#include "vector.h"
#include "game_logic.h"

bool init_engine();
void main_loop();
void cleanup_engine();
bool load_engine_assets();
void free_engine_assets();

InputState process_input(InputState *previous_input_state, float deltaTime);
void process_mouse();

#endif // ENGINE_H
