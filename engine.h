#ifndef ENGINE_H
#define ENGINE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include "world.h"
#include "vector.h"

typedef struct {
    Vec3 position;
    float pitch, yaw;
    float speed;
    float velocity_z;
    float jump_velocity;
    float height;
    float size;
} Player;

typedef enum {
    DIR_EAST,
    DIR_WEST,
    DIR_SOUTH,
    DIR_NORTH,
    DIR_DOWN,
    DIR_UP,
} Direction;

bool init_engine();
void main_loop();
void cleanup_engine();
bool load_engine_assets();
void free_engine_assets();

void process_input(World *world, float deltaTime);
void process_mouse();
void update_player_position(Player *player, World *world, float dx, float dy, float deltaTime);

void render_face(float x, float y, float z, float sizeX, float sizeY, GLuint texture, Direction direction);
void render_world(World* world);

#endif // ENGINE_H
