#include <stdio.h>
#include "world.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <dirent.h>

bool load_world(World* world) {
    DIR* dir;
    struct dirent* entry;
    int level_count = 0;

    dir = opendir("levels");
    if (dir == NULL) {
        printf("Failed to open levels directory.\n");
        return false;
    }

    // Count the number of level files
    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, "level-") != NULL && strstr(entry->d_name, ".bmp") != NULL) {
            level_count++;
        }
    }

    // Allocate memory for levels
    world->num_levels = level_count;
    world->levels = malloc(level_count * sizeof(Level));

    // Reset directory position
    rewinddir(dir);

    // Load each level
    int level_index = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, "level-") != NULL && strstr(entry->d_name, ".bmp") != NULL) {
            char level_path[256];
            snprintf(level_path, sizeof(level_path), "levels/%s", entry->d_name);

            SDL_Surface* level_surface = SDL_LoadBMP(level_path);
            if (level_surface == NULL) {
                printf("Failed to load level %s\n", entry->d_name);
                continue;
            }

            parse_level_from_surface(level_surface, &world->levels[level_index]);
            SDL_FreeSurface(level_surface);
            level_index++;
        }
    }

    closedir(dir);
    return true;
}

void free_world(World* world) {
    for (int i = 0; i < world->num_levels; ++i) {
        Level* level = &world->levels[i];
        for (int y = 0; y < level->height; ++y) {
            free(level->cells[y]);
        }
        free(level->cells);
    }
    free(world->levels);
    world->levels = NULL;
    world->num_levels = 0;
}

void parse_level_from_surface(SDL_Surface* surface, Level* level) {
    level->width = surface->w;
    level->height = surface->h;
    level->cells = malloc(level->height * sizeof(Cell*));

    for (int y = 0; y < level->height; ++y) {
        level->cells[y] = malloc(level->width * sizeof(Cell));
        for (int x = 0; x < level->width; ++x) {
            Uint8 r, g, b;
            SDL_GetRGB(get_pixel32(surface, x, y), surface->format, &r, &g, &b);
            SDL_Color color = {r, g, b, 255};
       
            level->cells[y][x] = get_cell_definition_from_color(color);
        }
    }
}

Cell get_cell_definition_from_color(SDL_Color color) {
    Cell cell_def;

    // #FF00FF (Magenta) - Void block
    if (color.r == 0xFF && color.g == 0x00 && color.b == 0xFF) {
        cell_def.type = CELL_SOLID;
        cell_def.color = color;
        cell_def.floor_texture = 0;
        cell_def.ceiling_texture = 0;
        cell_def.wall_texture = 0;
    }
    // #404040 - Brick wall
    else if (color.r == 0x40 && color.g == 0x40 && color.b == 0x40) {
        cell_def.type = CELL_SOLID;
        cell_def.color = color;
        cell_def.floor_texture = loadTexture("assets/earth1.bmp");
        cell_def.ceiling_texture = loadTexture("assets/grey_tile1.bmp");
        cell_def.wall_texture = loadTexture("assets/grey_brick1.bmp");
    }
    // #808080 - Stone floor marble pattern ceiling
    else if (color.r == 0x80 && color.g == 0x80 && color.b == 0x80) {
        cell_def.type = CELL_OPEN;
        cell_def.color = color;
        cell_def.floor_texture = loadTexture("assets/stone_floor1.bmp");
        cell_def.ceiling_texture = loadTexture("assets/marble_pattern1.bmp");
        cell_def.wall_texture = 0;
    }
    // Default - Open space
    else {
        cell_def.type = CELL_OPEN;
        cell_def.color = color;
        cell_def.floor_texture = 0;
        cell_def.ceiling_texture = 0;
        cell_def.wall_texture = 0;
    }

    return cell_def;
}

GLuint loadTexture(const char *filename) {
    SDL_Surface *image = IMG_Load(filename);
    if (!image) {
        printf("Error: %s\n", IMG_GetError());
        return 0;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    GLenum format = (image->format->BytesPerPixel == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, image->w, image->h, 0, format, GL_UNSIGNED_BYTE, image->pixels);

    glBindTexture(GL_TEXTURE_2D, 0);
    SDL_FreeSurface(image);

    return texture;
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
