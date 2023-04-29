#ifndef WORLD_H
#define WORLD_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "vector.h"

#define MAX_CELLS 16
extern const int CELL_XY_SCALE;
extern const int CELL_Z_SCALE;

typedef enum {
    CELL_OPEN,
    CELL_SOLID
} CellType;

typedef struct {
    CellType type;
    SDL_Color color;
    GLuint floor_texture;
    GLuint ceiling_texture;
    GLuint wall_texture;
} Cell;

typedef struct {
    int width;
    int height;
    Cell** cells;
} Layer;

typedef struct {
    int num_layers;
    Layer* layers;
} World;

typedef struct {
    Cell* cell;
    vec3 position;
} CellInfo;

bool load_world(World* world, const char* level_name);
void free_world(World* world);
void parse_layer_from_surface(SDL_Surface* surface, Layer* layer);
int parse_cell_definition(const char* line, Cell* def);
Cell* get_cell(Layer* layer, int x, int y);
CellInfo* get_cells_for_vector_3d(World* world, vec3 source, vec3 destination, int* num_cells);
Cell* get_cell_definition_from_color(SDL_Color color, Cell* definitions, int num_definitions);
Cell* read_cell_definitions(const char* filename, int* num_definitions);
bool is_out_of_xy_bounds(Layer* layer, int x, int y);
bool is_within_xy_bounds(Layer* layer, int x, int y);
bool get_world_cell(World* world, ivec3 grid_position, Cell** out_cell);

#define MAX(a, b) ((a) > (b) ? (a) : (b))

#endif // WORLD_H
