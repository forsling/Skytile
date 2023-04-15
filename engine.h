#ifndef ENGINE_H
#define ENGINE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>

bool init_engine();
void main_loop();
void cleanup_engine();

#endif // ENGINE_H