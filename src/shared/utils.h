#ifndef UTILS_H
#define UTILS_H

#include <stdarg.h>
#include <time.h>
#include <stdlib.h>
#include <SDL2/SDL_image.h>

#include "vector.h"
#include "game.h"

void debuglog(int one_in_n_chance, const char* format, ...);
Uint32 get_pixel32(SDL_Surface* surface, int x, int y);
SDL_Surface* load_surface(const char* filename);
vec3 get_random_world_pos(World* world);
Cell* get_cell(Layer* layer, int x, int y);
bool is_out_of_xy_bounds(Layer* layer, int x, int y);
bool is_within_xy_bounds(Layer* layer, int x, int y);
bool get_world_cell(World* world, ivec3 grid_position, Cell** out_cell);

#endif // UTILS_H
