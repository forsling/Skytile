#include <stdio.h>
#include <math.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
#include <GL/glu.h>
#include "engine.h"
#include "world.h"

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const float SCALE_FACTOR = 2.0f;
const int CELL_HEIGHT = 2;

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
    player.position.z = CELL_HEIGHT;
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

        render_world(&world);

        SDL_GL_SwapWindow(window);
    }

    SDL_SetRelativeMouseMode(SDL_FALSE);
}

void update_player_position(Player *player, World *world, float dx, float dy, float dz) {
    const float COLLISION_BUFFER = 0.3f * SCALE_FACTOR;

    float newX = player->position.x + dx * player->speed;
    float newY = player->position.y + dy * player->speed;
    float newZ = player->position.z + dz * player->speed;

    int gridX = (int)(newX / SCALE_FACTOR);
    int gridY = (int)(newY / SCALE_FACTOR);

    int levelIndex = get_level_from_z(newZ, world);

    printf("Level index: %d (x: %f y: %f z: %f) \n", levelIndex, newX, newY, newZ);

    bool canMoveX = true;
    bool canMoveY = true;

    if (levelIndex > 0) {
        // Get the corresponding level
        Level *level = &world->levels[levelIndex];

        canMoveX = !is_solid_cell(level, gridX, gridY);
        canMoveY = !is_solid_cell(level, gridX, gridY);

        if (canMoveX || canMoveY) {
            float cellCenterX = gridX * SCALE_FACTOR + SCALE_FACTOR / 2.0f;
            float cellCenterY = gridY * SCALE_FACTOR + SCALE_FACTOR / 2.0f;

            if (canMoveX && fabs(newX - cellCenterX) <= (SCALE_FACTOR / 2.0f + COLLISION_BUFFER)) {
                canMoveX = false;
            }

            if (canMoveY && fabs(newY - cellCenterY) <= (SCALE_FACTOR / 2.0f + COLLISION_BUFFER)) {
                canMoveY = false;
            }
        }

        // Handle corner cases to avoid getting stuck
        if (!canMoveX && !canMoveY) {
            float oldX = player->position.x;
            float oldY = player->position.y;

            player->position.x = newX;
            canMoveX = !is_solid_cell(level, (int)(player->position.x / SCALE_FACTOR), (int)(player->position.y / SCALE_FACTOR));

            player->position.x = oldX;
            player->position.y = newY;
            canMoveY = !is_solid_cell(level, (int)(player->position.x / SCALE_FACTOR), (int)(player->position.y / SCALE_FACTOR));

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

    update_player_position(&player, world, dx, dy, dz);
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

void render_textured_quad(GLuint texture, Vec3 a, Vec3 b, Vec3 c, Vec3 d, float v_scale) {
    Vec3 scaled_a = {a.x * SCALE_FACTOR, a.y * SCALE_FACTOR, a.z * SCALE_FACTOR};
    Vec3 scaled_b = {b.x * SCALE_FACTOR, b.y * SCALE_FACTOR, b.z * SCALE_FACTOR};
    Vec3 scaled_c = {c.x * SCALE_FACTOR, c.y * SCALE_FACTOR, c.z * SCALE_FACTOR};
    Vec3 scaled_d = {d.x * SCALE_FACTOR, d.y * SCALE_FACTOR, d.z * SCALE_FACTOR};

    glBindTexture(GL_TEXTURE_2D, texture);

    glBegin(GL_QUADS);
    {
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(scaled_a.x, scaled_a.y, scaled_a.z);

        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(scaled_b.x, scaled_b.y, scaled_b.z);

        glTexCoord2f(1.0f, 1.0f * v_scale); 
        glVertex3f(scaled_c.x, scaled_c.y, scaled_c.z);

        glTexCoord2f(0.0f, 1.0f * v_scale); 
        glVertex3f(scaled_d.x, scaled_d.y, scaled_d.z);
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
            0.0f, 0.0f, 1.0f);

    const int DX[] = {1, 0, -1, 0};
    const int DY[] = {0, 1, 0, -1};

    for (int l = 0; l < world->num_levels; l++) {
        Level *level = &world->levels[l];
        for (int y = 0; y < level->height; ++y) {
            for (int x = 0; x < level->width; ++x) {
                Cell *cell = &level->cells[y][x];
                Cell *neighbors[4] = {
                    get_cell(level, x + DX[0], y + DY[0]),
                    get_cell(level, x + DX[1], y + DY[1]),
                    get_cell(level, x + DX[2], y + DY[2]),
                    get_cell(level, x + DX[3], y + DY[3])
                };

                // Render floors
                if (cell->floor_texture != 0) {
                    Vec3 floor_vertices[4];
                    calculate_vertices(floor_vertices, x, y, DIR_DOWN);
                    render_textured_quad(cell->floor_texture, floor_vertices[0], floor_vertices[1], floor_vertices[2], floor_vertices[3], 1.0f);
                }

                // Render ceilings
                if (cell->ceiling_texture != 0) {
                    Vec3 ceiling_vertices[4];
                    calculate_vertices(ceiling_vertices, x, y, DIR_UP);
                    for (int i = 0; i < 4; ++i) {
                        ceiling_vertices[i].z *= 2.0f;
                    }
                    render_textured_quad(cell->ceiling_texture, ceiling_vertices[0], ceiling_vertices[1], ceiling_vertices[2], ceiling_vertices[3], 1.0f);
                }
                
                for (int i = 0; i < 4; ++i) {
                    Cell *neighbor = neighbors[i];
                    if (cell->type == CELL_OPEN && neighbor != NULL && neighbor->type == CELL_SOLID && neighbor->wall_texture != 0) {
                        //Render walls for solid edge blocks
                        Vec3 wall_vertices[4];
                        calculate_vertices(wall_vertices, x, y, i);
                        wall_vertices[2].z *= CELL_HEIGHT;
                        wall_vertices[3].z *= CELL_HEIGHT;
                        render_textured_quad(neighbor->wall_texture, wall_vertices[0], wall_vertices[1], wall_vertices[2], wall_vertices[3], CELL_HEIGHT);
                    } else if (cell->type == CELL_SOLID) {
                        //Render walls when untextured solid blocks borders textured solid blocks
                        bool isSolidEdgeBlock = (cell->wall_texture != 0 && neighbor == NULL);
                        bool isTransparentSolidWithSolidNeighbor = (cell->wall_texture == 0 && neighbor != NULL && neighbor->type == CELL_SOLID && neighbor->wall_texture != 0);
                        if (isSolidEdgeBlock || isTransparentSolidWithSolidNeighbor) {
                            Vec3 wall_vertices[4];
                            calculate_vertices(wall_vertices, x, y, i);
                            wall_vertices[2].z *= CELL_HEIGHT;
                            wall_vertices[3].z *= CELL_HEIGHT;
                            GLuint wall_texture = isSolidEdgeBlock ? cell->wall_texture : neighbor->wall_texture;
                            render_textured_quad(wall_texture, wall_vertices[0], wall_vertices[1], wall_vertices[2], wall_vertices[3], CELL_HEIGHT);
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
    int cell_height_offset = (int)((z - CELL_HEIGHT) / CELL_HEIGHT);
    int levelIndex = 1 + (-cell_height_offset);

    // Make sure the level index is within bounds, 0 to turn of collision detection
    if (levelIndex < 0 || levelIndex > world->num_levels) {
        levelIndex = 0;
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

void calculate_vertices(Vec3 vertices[4], int x, int y, Direction direction) {
    switch (direction) {
        case DIR_DOWN:
            vertices[0] = (Vec3){x, y, 0.0f};
            vertices[1] = (Vec3){x + 1, y, 0.0f};
            vertices[2] = (Vec3){x + 1, y + 1, 0.0f};
            vertices[3] = (Vec3){x, y + 1, 0.0f};
            break;
        case DIR_UP:
            vertices[0] = (Vec3){x, y, 1.0f};
            vertices[1] = (Vec3){x + 1, y, 1.0f};
            vertices[2] = (Vec3){x + 1, y + 1, 1.0f};
            vertices[3] = (Vec3){x, y + 1, 1.0f};
            break;
        case DIR_NORTH:
            vertices[0] = (Vec3){x, y + 1, 0.0f};
            vertices[1] = (Vec3){x + 1, y + 1, 0.0f};
            vertices[2] = (Vec3){x + 1, y + 1, 1.0f};
            vertices[3] = (Vec3){x, y + 1, 1.0f};
            break;
        case DIR_EAST:
            vertices[0] = (Vec3){x + 1, y, 0.0f};
            vertices[1] = (Vec3){x + 1, y + 1, 0.0f};
            vertices[2] = (Vec3){x + 1, y + 1, 1.0f};
            vertices[3] = (Vec3){x + 1, y, 1.0f};
            break;
        case DIR_SOUTH:
            vertices[0] = (Vec3){x, y, 0.0f};
            vertices[1] = (Vec3){x + 1, y, 0.0f};
            vertices[2] = (Vec3){x + 1, y, 1.0f};
            vertices[3] = (Vec3){x, y, 1.0f};
            break;
        case DIR_WEST:
            vertices[0] = (Vec3){x, y, 0.0f};
            vertices[1] = (Vec3){x, y + 1, 0.0f};
            vertices[2] = (Vec3){x, y + 1, 1.0f};
            vertices[3] = (Vec3){x, y, 1.0f};
            break;
    }
}