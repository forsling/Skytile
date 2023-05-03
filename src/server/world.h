#ifndef WORLD_H
#define WORLD_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "../shared/game.h"
#include "../shared/vector.h"

bool load_world(World* world, const char* level_name);
void free_world(World* world);
void parse_layer_from_surface(SDL_Surface* surface, Layer* layer);
int parse_cell_definition(const char* line, Cell* def);
Cell* get_cell_definition_from_color(SDL_Color color, Cell* definitions, int num_definitions);
Cell* read_cell_definitions(const char* filename, int* num_definitions);
bool is_out_of_xy_bounds(Layer* layer, int x, int y);
bool is_within_xy_bounds(Layer* layer, int x, int y);
bool get_world_cell(World* world, ivec3 grid_position, Cell** out_cell);

#endif // WORLD_H
