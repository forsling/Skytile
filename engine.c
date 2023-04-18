#include <stdio.h>
#include <math.h>
#include <string.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
#include <GL/glu.h>
#include "engine.h"
#include "world.h"

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const int CELL_XY_SCALE  = 2;
const int CELL_Z_SCALE = 4;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_GLContext gl_context = NULL;

World world;
Player player;

static bool quit = false;

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

    gl_context = SDL_GL_CreateContext(window);
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

    Level first_level = world.levels[0];

    // Initialize player object
    player.position.x = first_level.width / 2;
    player.position.y = first_level.height / 2;
    player.position.z = 0;
    player.pitch = 0.0f;
    player.yaw = 0.0f;
    player.speed = 0.1f; // Adjust this value to set the movement speed

    // Initialize OpenGL
    glClearColor(0.17f, 0.2f, 0.26f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 500.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    return true;
}

void main_loop() {
    SDL_Event event;

    SDL_SetRelativeMouseMode(SDL_TRUE);

    while (!quit) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }

        process_input(&world);
        process_mouse();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //test_render();
        render_world(&world);

        SDL_GL_SwapWindow(window);
    }

    SDL_SetRelativeMouseMode(SDL_FALSE);
    SDL_Delay(50);
}

void update_player_position(Player *player, World *world, float dx, float dy, float dz) {
    const float COLLISION_BUFFER = 0.3f * CELL_XY_SCALE;

    float newX = player->position.x + dx * player->speed;
    float newY = player->position.y + dy * player->speed;
    float newZ = player->position.z + dz * player->speed;

    int gridX = (int)(newX / CELL_XY_SCALE);
    int gridY = (int)(newY / CELL_XY_SCALE);

    int levelIndex = get_level_from_z(newZ, world);

    printf("Level index: %d (x: %f y: %f z: %f) \n", levelIndex, newX, newY, newZ);

    bool canMoveX = true;
    bool canMoveY = true;

    if (levelIndex >= 0) {
        // Get the corresponding level
        Level *level = &world->levels[levelIndex];

        canMoveX = !is_solid_cell(level, gridX, gridY);
        canMoveY = !is_solid_cell(level, gridX, gridY);

        if (canMoveX || canMoveY) {
            float cellCenterX = gridX * CELL_XY_SCALE + CELL_XY_SCALE / 2.0f;
            float cellCenterY = gridY * CELL_XY_SCALE + CELL_XY_SCALE / 2.0f;

            if (canMoveX && fabs(newX - cellCenterX) <= (CELL_XY_SCALE / 2.0f + COLLISION_BUFFER)) {
                canMoveX = false;
            }

            if (canMoveY && fabs(newY - cellCenterY) <= (CELL_XY_SCALE / 2.0f + COLLISION_BUFFER)) {
                canMoveY = false;
            }
        }

        // Handle corner cases to avoid getting stuck
        if (!canMoveX && !canMoveY) {
            float oldX = player->position.x;
            float oldY = player->position.y;

            player->position.x = newX;
            canMoveX = !is_solid_cell(level, (int)(player->position.x / CELL_XY_SCALE), (int)(player->position.y / CELL_XY_SCALE));

            player->position.x = oldX;
            player->position.y = newY;
            canMoveY = !is_solid_cell(level, (int)(player->position.x / CELL_XY_SCALE), (int)(player->position.y / CELL_XY_SCALE));

            player->position.y = oldY;
        }
    }
    
    if (canMoveX) {
        player->position.x = newX;
    }
    if (canMoveY) {
        player->position.y = newY;
    }
    player->position.z = newZ;
}

void process_input(World *world) {
    const Uint8 *state = SDL_GetKeyboardState(NULL);

    float dx = 0.0f;
    float dy = 0.0f;
    float dz = 0.0f;

    if (state[SDL_SCANCODE_ESCAPE]) {
        quit = true;
    }
    if (state[SDL_SCANCODE_W]) {
        dx += cosf(player.yaw);
        dy += sinf(player.yaw);
    }
    if (state[SDL_SCANCODE_S]) {
        dx -= cosf(player.yaw);
        dy -= sinf(player.yaw);
    }
    if (state[SDL_SCANCODE_D]) {
        dx -= sinf(player.yaw);
        dy += cosf(player.yaw);
    }
    if (state[SDL_SCANCODE_A]) {
        dx += sinf(player.yaw);
        dy -= cosf(player.yaw);
    }
    if (state[SDL_SCANCODE_SPACE]) {
        dz -= 1.0f;
    }
    if (state[SDL_SCANCODE_LSHIFT]) {
        dz += 1.0f;
    }

    update_player_position(&player, world, dx, dy, dz);
}

void process_mouse() {
    int mouseX, mouseY;
    SDL_GetRelativeMouseState(&mouseX, &mouseY);

    const float MOUSE_SENSITIVITY = 0.001f;
    player.yaw += mouseX * MOUSE_SENSITIVITY;
    player.pitch -= mouseY * MOUSE_SENSITIVITY;

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
    if (!load_world(&world)) {
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

void render_face(float x, float y, float z, float width, float height, Direction direction, GLuint texture) {
    glBindTexture(GL_TEXTURE_2D, texture);
    glBegin(GL_QUADS);

    //printf("Rendering face with dir %d at %f, %f, %f with width %f and height %f\n", direction, x, y, z, width, height);

    switch (direction) {
        case DIR_EAST:
            glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y, z);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(x + width, y + width, z);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(x + width, y + width, z + height);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y, z + height);
            break;
        case DIR_DOWN:
            glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y, z + CELL_Z_SCALE);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(x + width, y, z + CELL_Z_SCALE);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(x + width, y + height, z + CELL_Z_SCALE);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y + height, z + CELL_Z_SCALE);
            break;
        case DIR_WEST:
            glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y, z + height);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y + width, z + height);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y + width, z);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y, z);
            break;
        case DIR_UP:
            glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y, z);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(x + width, y, z);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(x + width, y + height, z);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y + height, z);
            break;
        case DIR_NORTH:
            glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y, z);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(x + width, y, z);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(x + width, y, z + height);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y, z + height);
            break;
        case DIR_SOUTH:
            glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y + width, z);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(x + width, y + width, z);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(x + width, y + width, z + height);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y + width, z + height);
            break;
    }

    glEnd();
}

void render_world(World *world) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90.0, (double)SCREEN_WIDTH / (double)SCREEN_HEIGHT, 0.01, 500.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(player.position.x, player.position.y, player.position.z,
            player.position.x + cosf(player.yaw), player.position.y + sinf(player.yaw), player.position.z - sinf(player.pitch),
            0.0f, 0.0f, -1.0f);

    Direction neighbor_dirs[] = {DIR_EAST, DIR_WEST, DIR_SOUTH, DIR_NORTH};

    bool render_reference_block = false;
    if (render_reference_block) {
        GLuint tex_wall = loadTexture("assets/grey_brick1.bmp");
        GLuint dirt = loadTexture("assets/earth1.bmp");
        render_face(-4, -4, 0, CELL_XY_SCALE, CELL_XY_SCALE, DIR_UP, tex_wall);
        render_face(-4, -4, 0, CELL_XY_SCALE, CELL_XY_SCALE, DIR_DOWN, dirt);
        render_face(-4, -4, 0, CELL_XY_SCALE, CELL_Z_SCALE, DIR_NORTH, tex_wall);
        render_face(-4, -4, 0, CELL_XY_SCALE, CELL_Z_SCALE, DIR_WEST, tex_wall);
        render_face(-4, -4, 0, CELL_XY_SCALE, CELL_Z_SCALE, DIR_EAST, tex_wall);
    }

    for (int z = 0; z < world->num_levels; z++) {
        Level *level = &world->levels[z];
        for (int y = 0; y < level->height; ++y) {
            for (int x = 0; x < level->width; ++x) {
                Cell *cell = &level->cells[y][x];
                Cell *neighbors[4] = {
                    get_cell(level, x + 1, y + 0),
                    get_cell(level, x -1, y + 0),
                    get_cell(level, x + 0, y + 1),
                    get_cell(level, x + 0, y - 1)
                };

                // Render floors
                if (cell->floor_texture != 0) {
                    render_face(x * CELL_XY_SCALE, y * CELL_XY_SCALE, z * CELL_Z_SCALE, CELL_XY_SCALE, CELL_XY_SCALE, DIR_DOWN, cell->floor_texture);
                }

                // Render ceilings
                if (cell->ceiling_texture != 0) {
                    render_face(x * CELL_XY_SCALE, y * CELL_XY_SCALE, z * CELL_Z_SCALE, CELL_XY_SCALE, CELL_XY_SCALE, DIR_UP, cell->ceiling_texture);
                }
                
                for (int i = 0; i < 4; ++i) {
                    Cell *neighbor = neighbors[i];
                    if (cell->type == CELL_OPEN && neighbor != NULL && neighbor->type == CELL_SOLID && neighbor->wall_texture != 0) {
                        //Render walls for adjacent solid blocks
                        render_face(x * CELL_XY_SCALE, y * CELL_XY_SCALE, z * CELL_Z_SCALE, CELL_XY_SCALE, CELL_Z_SCALE, neighbor_dirs[i], neighbor->wall_texture);

                    } else if (cell->type == CELL_SOLID) {
                        //Render walls when untextured solid blocks borders textured solid blocks
                        bool isSolidEdgeBlock = (cell->wall_texture != 0 && neighbor == NULL);
                        bool isTransparentSolidWithSolidNeighbor = (cell->wall_texture == 0 && neighbor != NULL && neighbor->type == CELL_SOLID && neighbor->wall_texture != 0);
                        if (isSolidEdgeBlock || isTransparentSolidWithSolidNeighbor) {
                            GLuint wall_texture = isSolidEdgeBlock ? cell->wall_texture : neighbor->wall_texture;
                            render_face(x * CELL_XY_SCALE, y * CELL_XY_SCALE, z * CELL_Z_SCALE, CELL_XY_SCALE, CELL_Z_SCALE, neighbor_dirs[i], wall_texture);
                        }
                    }
                }
            }
        }
    }
}

int get_level_from_z(float z, World *world) {
    // Calculate the level index based on the player's new height
    // First level is at the top, levels increase downwards
    int levelIndex = (int)(z / CELL_Z_SCALE);
    // Make sure the level index is within bounds, -1 to turn of collision detection
    if (z < 0 || levelIndex < 0 || levelIndex > (world->num_levels - 1)) {
        levelIndex = -1;
    }
    return levelIndex;
}

bool is_solid_cell(Level *level, int x, int y) {
    if (is_within_bounds(level, x, y)) {
        Cell *cell = &level->cells[y][x];
        return cell->type == CELL_SOLID;
    }
    return false;
}

bool is_out_of_bounds(Level *level, int x, int y) {
    return x < 0 || x >= level->width || y < 0 || y >= level->height;
}

bool is_within_bounds(Level *level, int x, int y) {
    return x >= 0 && x < level->width && y >= 0 && y < level->height;
}

Cell *get_cell(Level *level, int x, int y) {
    if (is_out_of_bounds(level, x, y)) {
        return NULL;
    }
    return &level->cells[y][x];
}
