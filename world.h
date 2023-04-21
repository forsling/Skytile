#ifndef WORLD_H
#define WORLD_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

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

bool load_world(World* world);
void free_world(World* world);
void parse_level_from_surface(SDL_Surface* surface, Level* level);
Cell get_cell_definition_from_color(SDL_Color color);
Cell* get_cell(Level* level, int x, int y);
Uint32 get_pixel32(SDL_Surface *surface, int x, int y);
GLuint create_texture(SDL_Surface* image, int x, int y, int width, int height);
SDL_Surface* load_surface(const char *filename);
GLuint loadTexture(const char *filename);


#endif // WORLD_H