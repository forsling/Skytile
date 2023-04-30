#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "texture_handler.h"

#define HASH_TABLE_SIZE 256

static TextureNode* texture_hash_table[HASH_TABLE_SIZE] = {0};
static SDL_Surface* texture_atlas = NULL;

static GLuint create_texture(SDL_Surface* image, int x, int y, int width, int height);
static unsigned int color_hash(SDL_Color color);
static void add_texture_info(SDL_Color color, TextureInfo info);
static int parse_cell_definition(const char* line);

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

        if (!parse_cell_definition(line)) {
            printf("Error: Invalid cell definition: %s\n", line);
        }
    }

    fclose(file);
}

TextureInfo* get_texture_info(SDL_Color color) {
    unsigned int index = color_hash(color);
    TextureNode* node = texture_hash_table[index];

    while (node) {
        if (node->color.r == color.r && node->color.g == color.g && node->color.b == color.b) {
            return &node->info;
        }
        node = node->next;
    }

    return NULL;
}

void free_texture_handler() {
    for (int i = 0; i < HASH_TABLE_SIZE; ++i) {
        TextureNode* node = texture_hash_table[i];
        while (node) {
            TextureNode* next_node = node->next;
            free(node);
            node = next_node;
        }
    }
}

// Function to create a GLuint texture from a sub-region of the given SDL_Surface
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

unsigned int color_hash(SDL_Color color) {
    return (color.r * 31 + color.g) * 31 + color.b;
}

void add_texture_info(SDL_Color color, TextureInfo info) {
    unsigned int index = color_hash(color);
    TextureNode* new_node = (TextureNode*)malloc(sizeof(TextureNode));

    new_node->color = color;
    new_node->info = info;
    new_node->next = texture_hash_table[index];

    texture_hash_table[index] = new_node;
}

int parse_cell_definition(const char* line) {
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
