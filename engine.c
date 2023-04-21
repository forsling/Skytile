#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
#include <GL/glu.h>
#include "engine.h"
#include "world.h"

const bool DEBUG_LOG = true;

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const int CELL_XY_SCALE  = 2;
const int CELL_Z_SCALE = 4;
const float COLLISION_BUFFER = 0.3f * CELL_XY_SCALE;
const float GRAVITY = 15.0f;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_GLContext gl_context = NULL;

World world;
Player player;
bool free_mode = false;

static bool quit = false;

void debuglog(int one_in_n_chance, const char* format, ...)
{
    if (!debuglog) {
        return;
    }
    if (rand() % one_in_n_chance == 0) {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
}

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

    srand(time(NULL));

    Level first_level = world.levels[0];

    // Initialize player object
    player.position.x = first_level.width / 2;
    player.position.y = first_level.height / 2;
    player.height = CELL_Z_SCALE / 2;
    player.position.z = -1 -player.height;
    player.velocity_z = 0.0f;
    player.pitch = 0.0f;
    player.yaw = 0.0f;
    player.speed = 10.0f;
    player.jump_velocity = -8.0f;

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

    Uint32 lastFrameTime = 0;

    while (!quit) {
        Uint32 currentFrameTime = SDL_GetTicks();
        float deltaTime = (currentFrameTime - lastFrameTime) / 1000.0f;

        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }

            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.scancode == SDL_SCANCODE_F) {
                    free_mode = !free_mode;
                    printf("Free mode set to %d\n", free_mode);
                }
            }
        }

        process_input(&world, deltaTime);
        process_mouse();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //test_render();
        render_world(&world);

        SDL_GL_SwapWindow(window);

        SDL_Delay(10);
        lastFrameTime = currentFrameTime;
    }

    SDL_SetRelativeMouseMode(SDL_FALSE);
}

void process_input(World *world, float deltaTime) {
    const Uint8 *state = SDL_GetKeyboardState(NULL);

    float dx = 0.0f;
    float dy = 0.0f;
    bool isJumping = false;

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
        if (free_mode) {
            player.position.z -= player.speed * deltaTime;
        } else if (player.velocity_z == 0.0f) { // Jump only when the player is on the ground
            player.velocity_z = player.jump_velocity;
            isJumping = true;
        } 
    }
    if (state[SDL_SCANCODE_LSHIFT]) {
        if (free_mode) {
            player.position.z += player.speed * deltaTime;        }
    }

    update_player_position(&player, world, dx, dy, deltaTime);
}

void update_player_position(Player *player, World *world,
                            float dx, float dy, float deltaTime) {

    // Handle free mode unrestricted movement
    if (free_mode) {
        player->position.x += dx * player->speed * deltaTime;
        player->position.y += dy * player->speed * deltaTime;
        player->velocity_z = 0;
        debuglog(4, "x %f, y %f, z %f \n", player->position.x, player->position.y, player->position.z);
        return;
    }

    // Apply gravity
    player->velocity_z += GRAVITY * deltaTime;

    // New player position (to be evaluated)
    float newX = player->position.x + dx * player->speed * deltaTime;
    float newY = player->position.y + dy * player->speed * deltaTime;
    float newZ = player->position.z + (player->velocity_z * deltaTime);
    int grid_x = (int)(newX / CELL_XY_SCALE);
    int grid_y = (int)(newY / CELL_XY_SCALE);

    // z-axis handling
    float next_z_obstacle;
    bool has_obstacle_down = get_next_z_obstacle(world, grid_x, grid_y, player->position.z, &next_z_obstacle);
    if (has_obstacle_down) {
        float highest_valid_z = next_z_obstacle - player->height;
        if (newZ > highest_valid_z) {
            // Player z movement is obstructed
            if (player->velocity_z >= 0) {
                player->position.z = highest_valid_z;
                player->velocity_z = 0.0f;
            } else {
                player->position.z = next_z_obstacle + 0.01f;
                player->velocity_z = 0.01f;
            }
        } else {
            player->position.z = newZ;
        }
    } else {
        player->position.z = newZ;
    }

    // Just move player if they are above or below the world
    int z_level = (int)floor(player->position.z / CELL_Z_SCALE);
    bool is_out_of_z_bounds = (z_level < 0 || z_level >= world->num_levels);
    if (is_out_of_z_bounds) {
        player->position.x = newX;
        player->position.y = newY;
        return;
    }

    Level *level = &world->levels[z_level];

    // Move player if they are outside of the level (XY)
    if (is_out_of_xy_bounds(level, grid_x, grid_y)) {
        player->position.x = newX;
        player->position.y = newY;
        player->position.z = newZ;  
        return;
    }

    // Get target cell info
    Cell *target_cell = get_cell(level, grid_x, grid_y);
    CellType target_cell_type = target_cell->type;
    bool target_is_solid = target_cell_type == CELL_SOLID;

    // Handle XY movement
    if (target_cell_type != CELL_SOLID) {
        player->position.x = newX;
        player->position.y = newY;
    }
}

bool get_next_z_obstacle(World *world, int cell_x, int cell_y, float z_pos, float *out_obstacle_z) {
    int z_level = (int)(z_pos / CELL_Z_SCALE);
    if (z_level >= world->num_levels) {
        return false;
    }

    int first_check_level = z_level >= 0 ? z_level : 0; 

    for (int i = first_check_level; i < world->num_levels; i++) {
        Level *level = &world->levels[i];
        if (!is_within_xy_bounds(level, cell_x, cell_y)) {
            continue;
        }
        Cell *cell = get_cell(level, cell_x, cell_y);

        //Check ceiling if they are below player
        if (z_pos < (float)i * CELL_Z_SCALE) {
            if (cell->ceiling_texture != 0 ||  (cell->type == CELL_SOLID)) {
                *out_obstacle_z = (float)i * CELL_Z_SCALE;
                //debuglog(8, "(zl %d) Found ceiling obstacle at %.2f (gridx: %d gridy: %d zlevel: %d z: %f) \n", i, *out_obstacle_z, cell_x, cell_y, z_level, z_pos);
                return true;
            }
        }

        //Check floors
        if (cell->floor_texture != 0 ||  (cell->type == CELL_SOLID)) {
            *out_obstacle_z = (float)i * CELL_Z_SCALE + 4;
            //debuglog(8, "(zl %d) Found floor obstacle at %.2f (gridx: %d gridy: %d zlevel: %d z: %f) \n", i, *out_obstacle_z, cell_x, cell_y, z_level, z_pos);
            return true;
        }
    }

    return false; // No obstacle found
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

    float ceiling_offset = 0.01f;
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
            glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y, z + ceiling_offset);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(x + width, y, z + ceiling_offset);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(x + width, y + height, z + ceiling_offset);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y + height, z + ceiling_offset);
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

    bool render_reference_block = true;
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

bool is_out_of_xy_bounds(Level *level, int x, int y) {
    return x < 0 || x >= level->width || y < 0 || y >= level->height;
}

bool is_within_xy_bounds(Level *level, int x, int y) {
    return x >= 0 && x < level->width && y >= 0 && y < level->height;
}

Cell *get_cell(Level *level, int x, int y) {
    if (is_out_of_xy_bounds(level, x, y)) {
        return NULL;
    }
    return &level->cells[y][x];
}
