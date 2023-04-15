#ifndef WORLD_H
#define WORLD_H

#include <stdbool.h>
#include <SDL2/SDL.h>

typedef enum {
    CELL_OPEN,
    CELL_SOLID
} CellType;

typedef struct {
    CellType type;
    SDL_Color color;
    SDL_Texture* floor_texture;
    SDL_Texture* ceiling_texture;
    SDL_Texture* wall_texture;
} CellDefinition;

typedef struct {
    int width;
    int height;
    CellDefinition** cells;
} World;

bool load_world(const char* file_path, World* world, SDL_Renderer* renderer);
void free_world(World* world);
CellDefinition* get_cell_definition(World* world, int x, int y);

#endif // WORLD_H