#include <stdio.h>
#include <math.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
#include <GL/glu.h>
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

    // Initialize OpenGL
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

        int window_width, window_height;
        SDL_GetWindowSize(window, &window_width, &window_height);
        render_world(&world, window_width, window_height);

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

GLuint load_texture(const char* filename) {
    SDL_Surface* surface = IMG_Load(filename);
    if (!surface) {
        printf("Error loading texture: %s\n", IMG_GetError());
        return 0;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);

    SDL_FreeSurface(surface);

    return texture;
}

void free_engine_assets() {
    free_world(&world);
}

void render_world(World* world, int window_width, int window_height) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up the projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, (GLfloat)window_width / (GLfloat)window_height, 0.1f, 100.0f);

    // Set up the modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(player.x, player.y, player.z,
              player.x + cosf(player.yaw) * cosf(player.pitch),
              player.y + sinf(player.yaw) * cosf(player.pitch),
              player.z + sinf(player.pitch),
              0.0f, 0.0f, 1.0f);

    // Render the world

    
    for (int y = 0; y < world->height; y++) {
        for (int x = 0; x < world->width; x++) {
            CellDefinition* cell = &world->cells[y][x];
            float xPos = x;
            float yPos = y;
            float zPos = 0;

            switch (cell->type) {
                case CELL_VOID:
                    // Render a transparent quad
                    break;
                case CELL_SOLID:
                    // Render wall texture
                    render_textured_quad(cell->wall_texture, xPos, yPos, zPos, 1.0f, 1.0f);
                    break;
                case CELL_OPEN:
                    // Render floor texture
                    render_textured_quad(cell->floor_texture, xPos, yPos, zPos, 1.0f, 1.0f);

                    // Render ceiling texture
                    render_textured_quad(cell->ceiling_texture, xPos, yPos, zPos + 1.0f, 1.0f, 1.0f);
                    break;
                default:
                    break;
            }
        }
    }
}

void render_textured_quad(GLuint texture, float x, float y, float z, float width, float height) {
    glBindTexture(GL_TEXTURE_2D, texture);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(x, y, z);
        glTexCoord2f(1, 0); glVertex3f(x + width, y, z);
        glTexCoord2f(1, 1); glVertex3f(x + width, y + height, z);
        glTexCoord2f(0, 1); glVertex3f(x, y + height, z);
    glEnd();
}

