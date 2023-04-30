#include <stdio.h>
#include <stdarg.h>
#include "utils.h"
#include <SDL2/SDL_image.h>

void debuglog(int one_in_n_chance, const char* format, ...)
{
    if (!debuglog) {
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