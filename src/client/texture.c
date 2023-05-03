#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "texture.h"

#define HASH_TABLE_SIZE 256

static TextureNode* hash_table[HASH_TABLE_SIZE] = {0};
static SDL_Surface* texture_atlas = NULL;
static TextureInfo default_cell;

static unsigned int color_hash(SDL_Color color);
static void add_texture_info(SDL_Color color, TextureInfo info);
static int load_cell_definitions(const char* line);

void init_texture_handler(const char* cell_definitions_path, SDL_Surface* atlas) {
    texture_atlas = atlas;

    FILE* file = fopen(cell_definitions_path, "r");
    if (!file) {
        printf("Error: Could not open cell definitions file: %s\n", cell_definitions_path);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n') {
            continue; // Skip comments and empty lines
        }

        if (!load_cell_definitions(line)) {
            printf("Error: Invalid cell definition: %s\n", line);
        }
    }

    fclose(file);
    default_cell = (TextureInfo) {
        .ceiling_texture = 0,
        .floor_texture = 0,
        .wall_texture = 0
    };
}

GLuint create_texture(SDL_Surface* image, int x, int y, int width, int height) {
    if (!image) {
        printf("Error: Invalid SDL_Surface\n");
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

    // Create a new SDL_Surface for the sub-region
    SDL_Surface* subImage = SDL_CreateRGBSurface(0, width, height, image->format->BitsPerPixel,
                                                 image->format->Rmask, image->format->Gmask,
                                                 image->format->Bmask, image->format->Amask);

    // Copy the sub-region to the new SDL_Surface
    SDL_Rect srcRect = {x, y, width, height};
    SDL_BlitSurface(image, &srcRect, subImage, NULL);

    // Create the texture from the sub-region surface
    glTexImage2D(GL_TEXTURE_2D, 0, format, subImage->w, subImage->h, 0, format, GL_UNSIGNED_BYTE, subImage->pixels);

    glBindTexture(GL_TEXTURE_2D, 0);
    SDL_FreeSurface(subImage);

    return texture;
}

static int hash(SDL_Color color) {
    return (color.r ^ color.g ^ color.b) % HASH_TABLE_SIZE;
}

static void add_texture_info(SDL_Color color, TextureInfo info) {
    int index = hash(color);
    color.a = 0;
    TextureNode* new_node = (TextureNode*)malloc(sizeof(TextureNode));
    new_node->color = color;
    new_node->texture_info = info;
    new_node->next = hash_table[index];
    hash_table[index] = new_node;
}

TextureInfo* get_texture_info(SDL_Color color) {
    color.a = 0;
    if (color.r == 0 && color.g == 255 && color.b == 255) {
        return &default_cell;
    }
    int index = hash(color);
    TextureNode* node = hash_table[index];

    while (node) {
        if (SDL_memcmp(&color, &node->color, sizeof(SDL_Color)) == 0) {
            return &node->texture_info;
        }
        node = node->next;
    }

    return NULL;
}

void free_texture_handler() {
    for (int i = 0; i < HASH_TABLE_SIZE; ++i) {
        TextureNode* node = hash_table[i];
        while (node) {
            TextureNode* next_node = node->next;
            free(node);
            node = next_node;
        }
    }
}

unsigned int color_hash(SDL_Color color) {
    return (color.r * 31 + color.g) * 31 + color.b;
}

static int load_cell_definitions(const char* line) {
    unsigned int r, g, b;
    char type_str[32];
    char c_str[32], f_str[32], w_str[32], name_str[64];
    int cx, cy, cw, ch, fx, fy, fw, fh, wx, wy, ww, wh;

    int num_parsed = sscanf(line, " %02X%02X%02X %31s %31s %31s %31s %255[^\n]",
                            &r, &g, &b,
                            type_str,
                            c_str,
                            f_str,
                            w_str,
                            name_str);

    if (num_parsed == 8) {
        SDL_Color color = {(Uint8)r, (Uint8)g, (Uint8)b, 255};

        sscanf(c_str, "%d,%d,%d,%d", &cx, &cy, &cw, &ch);
        sscanf(f_str, "%d,%d,%d,%d", &fx, &fy, &fw, &fh);
        sscanf(w_str, "%d,%d,%d,%d", &wx, &wy, &ww, &wh);

        TextureInfo info;
        info.ceiling_texture = create_texture(texture_atlas, cx, cy, cw, ch);
        info.floor_texture = create_texture(texture_atlas, fx, fy, fw, fh);
        info.wall_texture = create_texture(texture_atlas, wx, wy, ww, wh);

        add_texture_info(color, info);

        printf("Loaded cell definition: %s\n", name_str);
        return 1;
    }

    printf("Error: Invalid cell definition line: %s\n", line);
    return 0;
}
