#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>
#include <SDL2/SDL_image.h>

#include "utils.h"
#include "vector.h"
#include "game.h"

bool enable_debuglog = false;

void debuglog(int one_in_n_chance, const char* format, ...)
{
    if (!enable_debuglog) {
        return;
    }
    if (rand() % one_in_n_chance == 0) {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
}

SDL_Surface* load_surface(const char* filename) {
    SDL_Surface* image = IMG_Load(filename);
    if (!image) {
        printf("Error: %s\n", IMG_GetError());
        return NULL;
    }
    return image;
}

Uint32 get_pixel32(SDL_Surface* surface, int x, int y) {
    int bpp = surface->format->BytesPerPixel;
    Uint8* p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

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

Cell* get_cell(Layer* layer, int x, int y) {
    if (is_out_of_xy_bounds(layer, x, y)) {
        return NULL;
    }
    return &layer->cells[y][x];
}

bool is_out_of_xy_bounds(Layer* layer, int x, int y) {
    return x < 0 || x >= layer->width || y < 0 || y >= layer->height;
}

bool is_within_xy_bounds(Layer* layer, int x, int y) {
    return x >= 0 && x < layer->width && y >= 0 && y < layer->height;
}

bool get_world_cell(World* world, ivec3 grid_position, Cell** out_cell) {
    if (grid_position.z < 0 || grid_position.z >= world->num_layers) {
        return false;
    }
    Layer* layer = &world->layers[grid_position.z];
    if (grid_position.y < 0 || grid_position.y >= layer->width || grid_position.x < 0 || grid_position.x >= layer->height) {
        return false;
    }
    *out_cell = &layer->cells[grid_position.y][grid_position.x];
    return true;
}

vec3 get_random_world_pos(World* world) {
    int z = rand() % (world->num_layers + 1);
    int x = rand() % (world->layers[0].width + 1);
    int y = rand() % (world->layers[0].height + 1);
    return (vec3) {
        .x = x * CELL_XY_SCALE,
        .y = y * CELL_XY_SCALE,
        .z = z
    };
}
