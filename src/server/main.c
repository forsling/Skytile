#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>
#include <stdio.h>

#define SERVER_PORT 12345

int main(int argc, char *argv[]) {
    if (SDL_Init(0) == -1 || SDLNet_Init() == -1) {
        printf("Error initializing SDL or SDL_net: %s\n", SDL_GetError());
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

    while (1) {
        TCPsocket client_socket = SDLNet_TCP_Accept(server_socket);
        if (client_socket) {
            printf("Client connected!\n");

            char buffer[1024];
            int received = SDLNet_TCP_Recv(client_socket, buffer, sizeof(buffer) - 1);
            if (received > 0) {
                buffer[received] = '\0';
                printf("Received data from client: %s\n", buffer);

                // Process the received data and update the game state here
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
