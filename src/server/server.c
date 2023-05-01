#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "../game.h"
#include "../game_logic.h"
#include "../utils.h"
#include "../settings.h"

#define SERVER_PORT 12345
#define BUFFER_SIZE 8192

World world;

int main(int argc, char *argv[]) {
    srand(time(NULL));

    if (SDL_Init(0) == -1 || SDLNet_Init() == -1) {
        printf("Error initializing SDL or SDL_net: %s\n", SDL_GetError());
        return 1;
    }

    if (!load_settings("server.txt", true)) {
        fprintf(stderr, "Failed to load settings\n");
        return 1;
    }

    IPaddress server_ip;
    if (SDLNet_ResolveHost(&server_ip, NULL, SERVER_PORT) == -1) {
        printf("Error resolving server IP: %s\n", SDLNet_GetError());
        return 1;
    }

    TCPsocket server_socket = SDLNet_TCP_Open(&server_ip);
    if (!server_socket) {
        printf("Error opening server socket: %s\n", SDLNet_GetError());
        return 1;
    }

    printf("Server listening on port %d...\n", SERVER_PORT);

    // Load level data
    const char* level_name = "darkchasm";
    if (!load_world(&world, level_name)) {
        printf("Failed to load world.\n");
        return false;
    }
    
    // Initialize game state
    GameState game_state;
    {
        int player_x = rand() % (world.layers[0].width + 1); 
        int player_y = rand() % (world.layers[0].height + 1); 
        float player_height = CELL_Z_SCALE / 2;
        game_state.player = (Player) {
            .position.x = player_x,
            .position.y = player_y,
            .position.z = 4 - player_height,
            .height = player_height,
            .speed = 10.0f,
            .jump_velocity = -8.0f,
            .size = 0.3 * CELL_XY_SCALE
        };
        game_state.delta_time = 0.0f;
        memset(game_state.projectiles, 0, sizeof(game_state.projectiles));
    }

    InitialGameState initial_game_state = {
        .world = world,
        .projectiles = {0},
        .player = game_state.player
    };
            
    const Uint32 targetTickRate = 60;
    const Uint32 targetTickTime = 1000 / targetTickRate; // 1000ms / target TPS
    Uint32 lastTickTime = 0;

    while (1) {
        Uint32 currentTickTime = SDL_GetTicks();
        float delta_time = fmin(((currentTickTime - lastTickTime) / 1000.0f), 0.1f);
        game_state.delta_time = delta_time;

        TCPsocket client_socket = SDLNet_TCP_Accept(server_socket);
        if (client_socket) {
            printf("Client connected!\n");

            // Send initial game state to the client
            memcpy(&initial_game_state.player, &game_state.projectiles, sizeof(game_state.projectiles));
            memcpy(&initial_game_state.player, &game_state.player, sizeof(game_state.player));
            int sent_initial = SDLNet_TCP_Send(client_socket, &initial_game_state, sizeof(initial_game_state));
            if (sent_initial < sizeof(initial_game_state)) {
                printf("Error sending initial game state to the client.\n");
                SDLNet_TCP_Close(client_socket);
                continue;
            }

            // Loop until the client disconnects
            while (1) {
                InputState input_state;
                int received = SDLNet_TCP_Recv(client_socket, &input_state, sizeof(input_state));
                if (received <= 0) {
                    break; // Client disconnected or an error occurred
                }

                // Process the received input state and update the game state
                update(&game_state, &world, &input_state);

                // Send the updated game state back to the client
                int sent = SDLNet_TCP_Send(client_socket, &game_state, sizeof(game_state));
                //printf("Sent %d bytes to client \n", sent);
                if (sent < sizeof(game_state)) {
                    break; // An error occurred while sending the data or the client disconnected
                }
            }

            SDLNet_TCP_Close(client_socket);
            printf("Client disconnected.\n");
        }

        // Cap the tick rate
        Uint32 elapsedTime = SDL_GetTicks() - currentTickTime;
        if (elapsedTime < targetTickTime) {
            SDL_Delay(targetTickTime - elapsedTime);
        }

        lastTickTime = currentTickTime;
    }

    SDLNet_TCP_Close(server_socket);
    SDLNet_Quit();
    SDL_Quit();

    return 0;
}