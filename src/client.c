#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>

#include "client.h"
#include "game.h"
#include "world.h"
#include "vector.h"
#include "utils.h"
#include "render.h"
#include "audio.h"
#include "settings.h"
#include "texture.h"

static bool quit = false;
const bool DEBUG_LOG = true;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_GLContext gl_context = NULL;
SDL_Surface* base_fg_texture;
SDL_Surface* base_bg_texture;

#define MAX_TEXTURES 128
TextureInfo texture_map[MAX_TEXTURES];

bool have_audio = false;
int sound_jump;

GLuint projectile_texture;
GLuint player_texture;
GameState game_state;
World world;

bool init_engine() {
    //Init SDL and create window
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    int SCREEN_WIDTH = get_setting_int("screen_width");
    int SCREEN_HEIGHT = get_setting_int("screen_height");
    window = SDL_CreateWindow("Game Engine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                            SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    // OpenGL
    gl_context = SDL_GL_CreateContext(window);
    if (gl_context == NULL) {
        printf("OpenGL context could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    init_opengl();

    // SDL Image library
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return false;
    }

    if (SDLNet_Init() == -1) {
        printf("Error initializing SDL_net: %s\n", SDLNet_GetError());
        return false;
    }

    // Setup audio
    have_audio = audio_init();
    float vol = get_setting_float("master_volume");
    audio_set_volume(vol);

    // Load assets
    if (!load_engine_assets(&game_state)) {
        return false;
    }

    return true;
}

ButtonState get_mouse_button_state(uint32_t button, InputState* prev_input_state) {
    ButtonState result = {0};
    int is_down = SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(button);
    result.was_down = prev_input_state->mouse_button_1.is_down;
    result.is_down = is_down;
    return result;
}

InputState process_input(InputState* previous_input_state) {
    SDL_Event event;
    InputState new_input_state = *previous_input_state;

    // Process keyboard input
    const Uint8* state = SDL_GetKeyboardState(NULL);

    new_input_state.esc.was_down = new_input_state.esc.is_down;
    new_input_state.esc.is_down = state[SDL_SCANCODE_ESCAPE];

    new_input_state.space.was_down = new_input_state.space.is_down;
    new_input_state.space.is_down = state[SDL_SCANCODE_SPACE];

    new_input_state.up.was_down = new_input_state.up.is_down;
    new_input_state.up.is_down = state[SDL_SCANCODE_W];

    new_input_state.down.was_down = new_input_state.down.is_down;
    new_input_state.down.is_down = state[SDL_SCANCODE_S];

    new_input_state.right.was_down = new_input_state.right.is_down;
    new_input_state.right.is_down = state[SDL_SCANCODE_D];

    new_input_state.left.was_down = new_input_state.left.is_down;
    new_input_state.left.is_down = state[SDL_SCANCODE_A];

    new_input_state.shift.was_down = new_input_state.shift.is_down;
    new_input_state.shift.is_down = state[SDL_SCANCODE_LSHIFT];

    new_input_state.f.was_down = new_input_state.f.is_down;
    new_input_state.f.is_down = state[SDL_SCANCODE_F];

    new_input_state.ctrl.was_down = new_input_state.ctrl.is_down;
    new_input_state.ctrl.is_down = state[SDL_SCANCODE_LCTRL];

    while (SDL_PollEvent(&event) != 0) {
        switch (event.type) {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYDOWN:
                if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                    quit = true;
                }
                break;
        }
    }

    // Process mouse input
    int mouseX, mouseY;
    SDL_GetRelativeMouseState(&mouseX, &mouseY);

    new_input_state.mouse_state.x += mouseX;
    new_input_state.mouse_state.y += mouseY;
    new_input_state.mouse_state.dx = mouseX;
    new_input_state.mouse_state.dy = mouseY;

    new_input_state.mouse_button_1 = get_mouse_button_state(SDL_BUTTON_LEFT, previous_input_state);
    new_input_state.mouse_button_2 = get_mouse_button_state(SDL_BUTTON_RIGHT, previous_input_state);

    return new_input_state;
}

bool load_engine_assets(GameState* gamestate) {
    base_fg_texture = load_surface("assets/fg.png");
    base_bg_texture = load_surface("assets/bg.png");
    projectile_texture = create_texture(base_fg_texture, 1088, 192, 32, 32);
    player_texture = create_texture(base_fg_texture, 32, 160, 32, 32);

    sound_jump = audio_load_sound("assets/jump1.wav");

    return true;
}

void free_engine_assets() {
    free_world(&world);
    audio_quit();
}

void cleanup_engine() {
    free_engine_assets();

    SDL_DestroyRenderer(renderer);
    renderer = NULL;

    SDL_DestroyWindow(window);
    window = NULL;

    SDL_GL_DeleteContext(gl_context);
    gl_context = NULL;

    IMG_Quit();
    SDL_Quit();
}

void play_sounds(GameState* game_state, int player_id) {
    if (game_state->players[player_id].jumped) {
        audio_play_sound(sound_jump, 0.2f);
        game_state->players[player_id].jumped = false;
    }
}

void main_loop() {
    SDL_SetRelativeMouseMode(SDL_TRUE);

    Uint32 lastFrameTime = 0;
    const Uint32 targetFrameRate = 60;
    const Uint32 targetFrameTime = 1000 / targetFrameRate; // 1000ms / target FPS

    InputState input_state = {0};
    InputState prev_input_state = {0};

    const char* SERVER_HOSTNAME = "127.0.0.1";
    const Uint16 SERVER_PORT = 12345;

    // Set up texture handler
    init_texture_handler("cell_definitions.txt", base_bg_texture);

    // Connect to the server
    IPaddress server_ip;
    TCPsocket server_socket = {0};
    while (!server_socket) {
        if (SDLNet_ResolveHost(&server_ip, SERVER_HOSTNAME, SERVER_PORT) == -1) {
            printf("Error resolving server IP: %s\n", SDLNet_GetError());
            return;
        } else {
            printf("Resolved server IP: %d.%d.%d.%d\n", server_ip.host & 0xFF, (server_ip.host >> 8) & 0xFF, (server_ip.host >> 16) & 0xFF, (server_ip.host >> 24) & 0xFF);
        }
        server_socket = SDLNet_TCP_Open(&server_ip);
        if (!server_socket) {
            printf("Unable to connect to server: %s\n", SDLNet_GetError());
            SDL_Delay(2000);
        }
    }

    // Receive initial game state from server
    InitialGameState initial_game_state;
    int received_initial = SDLNet_TCP_Recv(server_socket, &initial_game_state, sizeof(initial_game_state));
    if (received_initial <= 0) {
        printf("Error receiving initial game state from server.\n");
        SDLNet_TCP_Close(server_socket);
        SDL_SetRelativeMouseMode(SDL_FALSE);
        return;
    }

    // Store the player ID
    int player_id = initial_game_state.player_id;

    // Copy over intial game state
    world = initial_game_state.world;

    while (!quit) {
        Uint32 currentFrameTime = SDL_GetTicks();

        // Process input and send input state to server
        prev_input_state = input_state;
        input_state = process_input(&prev_input_state);
        SDLNet_TCP_Send(server_socket, &input_state, sizeof(input_state));

        // Receive game state from server
        int received = SDLNet_TCP_Recv(server_socket, &game_state, sizeof(game_state));
        if (received <= 0) {
            printf("Server disconnected or an error occurred.\n");
            break;
        }

        Player* player = &game_state.players[player_id];

        // Play sounds on the client
        play_sounds(&game_state, player_id);

        // Rendering
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        render_world(&world, player);
        render_projectiles(&game_state, projectile_texture);
        render_players(game_state.players, player_id, game_state.players_count, player_texture);
        SDL_GL_SwapWindow(window);

        // Cap the framerate
        Uint32 elapsedTime = SDL_GetTicks() - currentFrameTime;
        if (elapsedTime < targetFrameTime) {
            SDL_Delay(targetFrameTime - elapsedTime);
        }
        lastFrameTime = currentFrameTime;
    }

    SDLNet_TCP_Close(server_socket);
    SDL_SetRelativeMouseMode(SDL_FALSE);
}
