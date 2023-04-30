#ifndef UTILS_H
#define UTILS_H

#include <stdarg.h>
#include <time.h>
#include <stdlib.h>
#include <SDL2/SDL_image.h>

void debuglog(int one_in_n_chance, const char* format, ...);
Uint32 get_pixel32(SDL_Surface* surface, int x, int y);
SDL_Surface* load_surface(const char* filename);

#endif // UTILS_H
