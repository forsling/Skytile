#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include "vector.h"

#define MAX_PROJECTILES 128
#define MAX_LAYERS 6
#define MAX_WIDTH 32
#define MAX_HEIGHT 32
#define MAX_PLAYERS 4

extern const int CELL_XY_SCALE;
extern const int CELL_Z_SCALE;

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
    bool alive;
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

typedef enum {
    CELL_VOID,
    CELL_SOLID,
    CELL_ROOM,
    CELL_FLOOR
} CellType;

typedef struct {
    CellType type;
    SDL_Color color;
} Cell;

typedef struct {
    int width;
    int height;
    Cell cells[MAX_HEIGHT][MAX_WIDTH];
} Layer;

typedef struct {
    int num_layers;
    Layer layers[MAX_LAYERS];
} World;

typedef struct GameState {
    Player players[4];
    int players_count;
    Projectile projectiles[MAX_PROJECTILES];
    float delta_time;
} GameState;

typedef struct InitialGameState {
    World world;
    int player_id;
} InitialGameState;

#endif // GAME_H