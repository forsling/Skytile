
#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include "vector.h"
#include "world.h"

#define MAX_PROJECTILES 128

typedef struct Player {
    vec3 position;
    float pitch, yaw;
    float speed;
    float velocity_z;
    float jump_velocity;
    float height;
    float size;
    bool free_mode;
    bool jumped;
} Player;

typedef struct Projectile {
    vec3 position;
    vec3 direction;
    float speed;
    float size;
    int ttl;
    bool active;
} Projectile;

typedef struct MouseState {
    int x, y;
    int dx, dy;
} MouseState;

typedef struct ButtonState {
    bool was_down;
    bool is_down;
} ButtonState;

typedef struct InputState {
    MouseState mouse_state;
    union
    {
        ButtonState Buttons[11];
        struct
        {
            ButtonState esc;
            ButtonState space;
            ButtonState up;
            ButtonState down;
            ButtonState right;
            ButtonState left;
            ButtonState shift;
            ButtonState ctrl;
            ButtonState f;
            ButtonState mouse_button_1;
            ButtonState mouse_button_2;
        };
    };
} InputState;

typedef struct GameState {
    World world;
    Player player;
    Projectile projectiles[MAX_PROJECTILES];
    float delta_time;
} GameState;

bool start_level(GameState* gamestate, const char* level);
void update(GameState *game_state, InputState *input_state);

#endif // GAME_LOGIC_H
