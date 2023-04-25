#ifndef RENDER_H
#define RENDER_H

#include <GL/gl.h>
#include <SDL2/SDL_image.h>
#include "world.h"
#include "engine.h"

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
GLuint create_texture(SDL_Surface* image, int x, int y, int width, int height);
Uint32 get_pixel32(SDL_Surface *surface, int x, int y);
SDL_Surface* load_surface(const char *filename);
void render_projectile(Projectile *projectile);

#endif // RENDER_H
