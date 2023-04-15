#include <stdio.h>
#include <math.h>
#include "engine.h"
#include "world.h"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

World world;
Player player;

bool init_engine() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    window = SDL_CreateWindow("Game Engine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return false;
    }

    if (!load_engine_assets()) {
        return false;
    }

    // Initialize player object
    player.x = world.width / 2;
    player.y = world.height / 2;
    player.z = 1.0f; // Adjust this value to set the initial height
    player.pitch = 0.0f;
    player.yaw = 0.0f;
    player.speed = 0.1f; // Adjust this value to set the movement speed

    return true;
}

void main_loop() {
    bool quit = false;
    SDL_Event event;

    while (!quit) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }

        const Uint8* keystate = SDL_GetKeyboardState(NULL);
        process_input(keystate);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        render_world(&world);

        SDL_RenderPresent(renderer);
    }
}

void process_input(const Uint8* keystate) {
    // Update player rotation based on mouse input
    int mouse_x_rel, mouse_y_rel;
    SDL_GetRelativeMouseState(&mouse_x_rel, &mouse_y_rel);
    player.yaw -= mouse_x_rel * 0.002f; // Adjust this value to set the mouse sensitivity
    player.pitch -= mouse_y_rel * 0.002f;

    // Update player position based on keyboard input
    float move_x = 0.0f, move_y = 0.0f;
    if (keystate[SDL_SCANCODE_W]) {
        move_x += cosf(player.yaw) * player.speed;
        move_y += sinf(player.yaw) * player.speed;
    }
    if (keystate[SDL_SCANCODE_S]) {
        move_x -= cosf(player.yaw) * player.speed;
        move_y -= sinf(player.yaw) * player.speed;
    }
    if (keystate[SDL_SCANCODE_A]) {
        move_x -= sinf(player.yaw) * player.speed;
        move_y += cosf(player.yaw) * player.speed;
    }
    if (keystate[SDL_SCANCODE_D]) {
        move_x += sinf(player.yaw) * player.speed;
        move_y -= cosf(player.yaw) * player.speed;
    }

    // Check for collisions and update player position
    // TODO: Implement proper collision detection
    player.x += move_x;
    player.y += move_y;
}


void cleanup_engine() {
    free_engine_assets();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    window = NULL;
    renderer = NULL;

    IMG_Quit();
    SDL_Quit();
}

bool load_engine_assets() {
    // Load world from a bitmap file
    if (!load_world("assets/world1.bmp", &world, renderer)) {
        printf("Failed to load world.\n");
        return false;
    }
    return true;
}

void free_engine_assets() {
    free_world(&world);
}

void render_world(World* world) {
    // TODO: Implement rendering logic for the world
    // This would include rendering floors, ceilings, and walls based on the cell definitions
}