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

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_GLContext gl_context = NULL;

World world;
Player player;

bool init_engine() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    window = SDL_CreateWindow("Game Engine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (gl_context == NULL) {
        printf("OpenGL context could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    // Set swap interval for Vsync
    SDL_GL_SetSwapInterval(1);

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return false;
    }

    if (!load_engine_assets()) {
        return false;
    }

    // Initialize player object
    player.position.x = world.width / 2;
    player.position.y = world.height / 2;
    player.position.z = 1.0f; // Adjust this value to set the initial height
    player.pitch = 0.0f;
    player.yaw = 0.0f;
    player.speed = 0.1f; // Adjust this value to set the movement speed

    // Initialize OpenGL
    glClearColor(0.2f, 0.1f, 0.15f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    return true;
}

void main_loop() {
    bool quit = false;
    SDL_Event event;

    SDL_SetRelativeMouseMode(SDL_TRUE);

    while (!quit) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }

        process_input();
        process_mouse();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        render_world(&world);

        SDL_GL_SwapWindow(window);
    }

    SDL_SetRelativeMouseMode(SDL_FALSE);
}

void update_player_position(Player *player, float dx, float dy, float dz) {
    player->position.x += dx * player->speed;
    player->position.y += dy * player->speed;
    player->position.z += dz * player->speed;
}

void process_input() {
    const Uint8 *state = SDL_GetKeyboardState(NULL);

    float dx = 0.0f;
    float dy = 0.0f;
    float dz = 0.0f;

    if (state[SDL_SCANCODE_W]) {
        dx += cosf(player.yaw);
        dy += sinf(player.yaw);
    }
    if (state[SDL_SCANCODE_S]) {
        dx -= cosf(player.yaw);
        dy -= sinf(player.yaw);
    }
    if (state[SDL_SCANCODE_A]) {
        dx -= sinf(player.yaw);
        dy += cosf(player.yaw);
    }
    if (state[SDL_SCANCODE_D]) {
        dx += sinf(player.yaw);
        dy -= cosf(player.yaw);
    }
    if (state[SDL_SCANCODE_SPACE]) {
        dz += 1.0f;
    }
    if (state[SDL_SCANCODE_LSHIFT]) {
        dz -= 1.0f;
    }

    update_player_position(&player, dx, dy, dz);
}

void process_mouse() {
    int mouseX, mouseY;
    SDL_GetRelativeMouseState(&mouseX, &mouseY);

    const float MOUSE_SENSITIVITY = 0.001f;
    player.yaw -= mouseX * MOUSE_SENSITIVITY;   // negate the value here
    player.pitch += mouseY * MOUSE_SENSITIVITY; // and here

    if (player.pitch < -M_PI / 2)
        player.pitch = -M_PI / 2;
    if (player.pitch > M_PI / 2)
        player.pitch = M_PI / 2;
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

GLuint load_texture(const char *filename) {
    SDL_Surface *surface = IMG_Load(filename);
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

void render_textured_quad(GLuint texture, Vec3 a, Vec3 b, Vec3 c, Vec3 d) {
    glBindTexture(GL_TEXTURE_2D, texture);
    glBegin(GL_QUADS);
    glColor3f(1.0f, 1.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(a.x, a.y, a.z);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(b.x, b.y, b.z);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(c.x, c.y, c.z);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(d.x, d.y, d.z);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
}

void render_world(World *world) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90.0, (double)SCREEN_WIDTH / (double)SCREEN_HEIGHT, 0.1, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(player.position.x, player.position.y, player.position.z,
            player.position.x + cosf(player.yaw), player.position.y + sinf(player.yaw), player.position.z - sinf(player.pitch),
            0.0f, 0.0f, 1.0f);

    const int DX[] = {1, 0, -1, 0};
    const int DY[] = {0, 1, 0, -1};
    Vec3 WALL_CORNERS[] = {
        {1.0f, 0.0f, 0.0f},
        {1.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f}
    };

    for (int y = 0; y < world->height; ++y) {
        for (int x = 0; x < world->width; ++x) {
            CellDefinition* cell = &world->cells[y][x];

            if (cell->type != CELL_VOID) {
                Vec3 vertices[4] = {
                    {x, y, 0.0f},
                    {x + 1, y, 0.0f},
                    {x + 1, y + 1, 0.0f},
                    {x, y + 1, 0.0f}
                };
                render_textured_quad(cell->floor_texture, vertices[0], vertices[1], vertices[2], vertices[3]);
            }

            if (cell->type == CELL_OPEN || cell->type == CELL_VOID) {
                for (int i = 0; i < 4; ++i) {
                    int nx = x + DX[i];
                    int ny = y + DY[i];

                    if (nx >= 0 && nx < world->width && ny >= 0 && ny < world->height) {
                        CellDefinition *neighbor = &world->cells[ny][nx];
                        
                        if (neighbor->type == CELL_SOLID) {
                            Vec3 a = {x + WALL_CORNERS[i].x, y + WALL_CORNERS[i].y, 0.0f};
                            Vec3 b = {x + WALL_CORNERS[(i + 1) % 4].x, y + WALL_CORNERS[(i + 1) % 4].y, 0.0f};
                            Vec3 c = {x + WALL_CORNERS[(i + 1) % 4].x, y + WALL_CORNERS[(i + 1) % 4].y, 1.0f};
                            Vec3 d = {x + WALL_CORNERS[i].x, y + WALL_CORNERS[i].y, 1.0f};

                            render_textured_quad(neighbor->wall_texture, a, b, c, d);
                        }
                    }
                }
            }
        }
    }
}