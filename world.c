#include <stdio.h>
#include "world.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

SDL_Surface* load_bitmap(const char* file_path);
CellDefinition get_cell_definition_from_color(SDL_Color color, SDL_Renderer* renderer);
void parse_world_from_surface(SDL_Surface* surface, World* world, SDL_Renderer* renderer);

bool load_world(const char* file_path, World* world, SDL_Renderer* renderer) {
    SDL_Surface* surface = load_bitmap(file_path);
    if (!surface) {
        return false;
    }

    parse_world_from_surface(surface, world, renderer);
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

void parse_world_from_surface(SDL_Surface* surface, World* world, SDL_Renderer* renderer) {
    world->width = surface->w;
    world->height = surface->h;
    world->cells = malloc(world->height * sizeof(CellDefinition*));

    for (int y = 0; y < world->height; ++y) {
        world->cells[y] = malloc(world->width * sizeof(CellDefinition));
        for (int x = 0; x < world->width; ++x) {
            Uint8 r, g, b;
            SDL_GetRGB(get_pixel32(surface, x, y), surface->format, &r, &g, &b);
            SDL_Color color = {r, g, b, 255};
       
            world->cells[y][x] = get_cell_definition_from_color(color, renderer);
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

CellDefinition get_cell_definition_from_color(SDL_Color color, SDL_Renderer* renderer) {
    CellDefinition cell_def;

    // Magenta (#FF00FF) - Void block
    if (color.r == 0xFF && color.g == 0x00 && color.b == 0xFF) {
        cell_def.type = CELL_SOLID;
        cell_def.color = color;
        cell_def.floor_texture = 0;
        cell_def.ceiling_texture = 0;
        cell_def.wall_texture = 0;
    }
    // #404040 - Solid with grey brick texture
    else if (color.r == 0x40 && color.g == 0x40 && color.b == 0x40) {
        cell_def.type = CELL_SOLID;
        cell_def.color = color;
        cell_def.floor_texture = 0;
        cell_def.ceiling_texture = 0;
        cell_def.wall_texture = loadTexture("assets/grey_brick1.bmp");
    }
    // #808080 - Open with stone floor and marble pattern ceiling textures
    else if (color.r == 0x80 && color.g == 0x80 && color.b == 0x80) {
        cell_def.type = CELL_OPEN;
        cell_def.color = color;
        cell_def.floor_texture = loadTexture("assets/stone_floor1.bmp");
        cell_def.ceiling_texture = loadTexture("assets/marble_pattern1.bmp");
        cell_def.wall_texture = 0;
    }
    // Default - Open and transparent (no textures)
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
