#include <stdio.h>
#include "world.h"
#include <SDL2/SDL_image.h>

SDL_Surface* load_bitmap(const char* file_path);
void parse_world_from_surface(SDL_Surface* surface, World* world);

bool load_world(const char* file_path, World* world) {
    SDL_Surface* surface = load_bitmap(file_path);
    if (!surface) {
        return false;
    }

    parse_world_from_surface(surface, world);
    SDL_FreeSurface(surface);
    return true;
}

Uint32 get_pixel32(SDL_Surface *surface, int x, int y) {
    int bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch (bpp) {
        case 1:
            return *p;
        case 2:
            return *(Uint16 *)p;
        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                return p[0] << 16 | p[1] << 8 | p[2];
            } else {
                return p[0] | p[1] << 8 | p[2] << 16;
            }
        case 4:
            return *(Uint32 *)p;
        default:
            return 0;
    }
}

SDL_Surface* load_bitmap(const char* file_path) {
    SDL_Surface* surface = IMG_Load(file_path);
    if (!surface) {
        printf("Unable to load bitmap: %s\n", IMG_GetError());
    }
    return surface;
}

void parse_world_from_surface(SDL_Surface* surface, World* world) {
    world->width = surface->w;
    world->height = surface->h;
    world->cells = malloc(world->height * sizeof(CellDefinition*));

    for (int y = 0; y < world->height; ++y) {
        world->cells[y] = malloc(world->width * sizeof(CellDefinition));
        for (int x = 0; x < world->width; ++x) {
            Uint8 r, g, b;
            SDL_GetRGB(get_pixel32(surface, x, y), surface->format, &r, &g, &b);
            SDL_Color color = {r, g, b, 255};

            // TODO: Determine the cell type and assign textures based on the color
            // This can be done by comparing the color with a predefined mapping of colors to cell types and textures

            // Example (to be replaced with actual logic):
            world->cells[y][x].type = CELL_OPEN;
            world->cells[y][x].color = color;
            world->cells[y][x].floor_texture = NULL;
            world->cells[y][x].ceiling_texture = NULL;
            world->cells[y][x].wall_texture = NULL;
        }
    }
}

void free_world(World* world) {
    for (int y = 0; y < world->height; ++y) {
        free(world->cells[y]);
    }
    free(world->cells);
    world->cells = NULL;
}

CellDefinition* get_cell_definition(World* world, int x, int y) {
    if (x < 0 || x >= world->width || y < 0 || y >= world->height) {
        return NULL;
    }
    return &world->cells[y][x];
}
