#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>
#include <stdio.h>

#include "../game.h"
#include "../game_logic.h"
#include "../utils.h"
#include "../settings.h"

#define SERVER_PORT 12345
#define BUFFER_SIZE 8192

int main(int argc, char *argv[]) {
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

    GameState game_state;

    // Start the level
    const char* level = "darkchasm";
    if (!start_level(&game_state, level)) {
        printf("Fail to start level %s \n", level);
        return 1;
    }

    while (1) {
        TCPsocket client_socket = SDLNet_TCP_Accept(server_socket);
        if (client_socket) {
            printf("Client connected!\n");

            // Loop until the client disconnects
            while (1) {
                InputState input_state;
                int received = SDLNet_TCP_Recv(client_socket, &input_state, sizeof(input_state));
                if (received <= 0) {
                    break; // Client disconnected or an error occurred
                }

                // Process the received input state and update the game state
                update(&game_state, &input_state);

                // Send the updated game state back to the client
                int sent = SDLNet_TCP_Send(client_socket, &game_state, sizeof(game_state));
                printf("Sent %d bytes to client \n", sent);
                if (sent < sizeof(game_state)) {
                    break; // An error occurred while sending the data or the client disconnected
                }
            }

            SDLNet_TCP_Close(client_socket);
            printf("Client disconnected.\n");
        }

        SDL_Delay(10);
    }

    SDLNet_TCP_Close(server_socket);
    SDLNet_Quit();
    SDL_Quit();

    return 0;
}