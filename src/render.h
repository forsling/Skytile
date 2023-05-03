#ifndef RENDER_H
#define RENDER_H

#include <GL/gl.h>
#include <SDL2/SDL_image.h>
#include "world.h"
#include "client.h"
#include "game_logic.h"

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
void render_world(World* world, Player* player, GLuint test_texture);
void render_players(Player* player, int current_player, int players_count, GLuint texture);

GLuint load_texture(const char* filename);
GLuint create_texture(SDL_Surface* image, int x, int y, int width, int height);
SDL_Surface* load_surface(const char* filename);
void render_projectiles(GameState* game_state, GLuint projectile_texture);

#endif // RENDER_H
