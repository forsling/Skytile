#ifndef ENGINE_H
#define ENGINE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include "world.h"
#include "vector.h"

typedef struct Player {
    vec3 position;
    float pitch, yaw;
    float speed;
    float velocity_z;
    float jump_velocity;
    float height;
    float size;
} Player;

typedef struct {
    vec3 position;
    vec3 direction;
    float speed;
    float size;
    GLuint texture;
    int ttl;
    bool active;
} Projectile;

bool init_engine();
void main_loop();
void cleanup_engine();
bool load_engine_assets();
void free_engine_assets();

void process_input(World *world, float deltaTime);
void process_mouse();
void update_player_position(Player *player , World *world, float dx, float dy, float deltaTime);
void calculate_projectile_direction(Player *player, vec3 *direction);

#endif // ENGINE_H
