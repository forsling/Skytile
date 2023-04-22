#ifndef RENDER_H
#define RENDER_H

#include <GL/gl.h>
#include <SDL2/SDL_image.h>
#include "world.h"
#include "engine.h"

extern const int SCREEN_WIDTH;
extern const int SCREEN_HEIGHT;

typedef enum {
    DIR_EAST,
    DIR_WEST,
    DIR_SOUTH,
    DIR_NORTH,
    DIR_DOWN,
    DIR_UP,
} Direction;

void init_opengl();
void render_face(float x, float y, float z, float width, float height, Direction direction, GLuint texture);
void render_world(World *world, Player *player);
GLuint load_texture(const char *filename);

#endif // RENDER_H
