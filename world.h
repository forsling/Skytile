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
} World;

bool load_world(const char* file_path, World* world, SDL_Renderer* renderer);
void free_world(World* world);
Cell* get_cell_definition(World* world, int x, int y);
GLuint loadTexture(const char *filename);

#endif // WORLD_H