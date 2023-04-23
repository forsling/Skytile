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
} Level;

typedef struct {
    int num_levels;
    Level* levels;
} World;

typedef struct {
    Cell* cell;
    Vec2 position;
} CellInfo;

bool load_world(World* world);
void free_world(World* world);
void parse_level_from_surface(SDL_Surface* surface, Level* level);
int parse_cell_definition(const char *line, Cell *def);
Cell* get_cell(Level* level, int x, int y);
Cell* get_cell_definition_from_color(SDL_Color color, Cell *definitions, int num_definitions);
Cell* read_cell_definitions(const char *filename, int *num_definitions);
CellInfo *get_cells_for_vector(Level *level, Vec2 source, Vec2 destination, int *num_cells);
Vec2 get_furthest_legal_position(Level *level, Vec2 source, Vec2 destination, float collision_buffer);
bool is_out_of_xy_bounds(Level *level, int x, int y);
bool is_within_xy_bounds(Level *level, int x, int y);
bool get_next_z_obstacle(World *world, int cell_x, int cell_y, float z_pos, float *out_obstacle_z);
bool get_world_cell(World *world, ivec3 grid_position, Cell** out_cell);

#endif // WORLD_H
