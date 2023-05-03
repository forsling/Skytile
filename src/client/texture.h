#ifndef TEXTURE_HANDLER_H
#define TEXTURE_HANDLER_H

#include <SDL2/SDL.h>

typedef struct TextureInfo {
    GLuint floor_texture;
    GLuint ceiling_texture;
    GLuint wall_texture;
} TextureInfo;

typedef struct TextureNode {
    SDL_Color color;
    TextureInfo texture_info;
    struct TextureNode* next;
} TextureNode;

void init_texture_handler(const char* cell_definitions_path, SDL_Surface* atlas);
TextureInfo* get_texture_info(SDL_Color color);
void free_texture_handler();
GLuint create_texture(SDL_Surface* image, int x, int y, int width, int height);

#endif // TEXTURE_HANDLER_H
