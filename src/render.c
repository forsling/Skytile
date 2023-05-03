#include <stdio.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "game.h"
#include "render.h"
#include "world.h"
#include "settings.h"
#include "texture.h"
#include "vector.h"

void init_opengl() {
    // Set swap interval for Vsync
    SDL_GL_SetSwapInterval(1);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    
    glClearColor(0.17f, 0.2f, 0.26f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, get_setting_int("screen_width"), get_setting_int("screen_height"));
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float)get_setting_int("screen_width") / (float)get_setting_int("screen_height"), 0.1f, 500.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void render_face(float x, float y, float z, float width, float height, Direction direction, GLuint texture) {
    glBindTexture(GL_TEXTURE_2D, texture);
    glBegin(GL_QUADS);

    float ceiling_offset = 0.01f;

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

void render_player_texture(Player* player, GLuint texture) {
    // Calculate the forward direction based on the player's yaw
    float s = sinf(player->yaw);
    float c = cosf(player->yaw);
    vec3 forward = {c, s, 0};

    // Calculate the right direction (cross product of forward and up)
    vec3 up = {0, 0, -1};
    vec3 right;
    vec3_cross(&forward, &up, &right);

    // Calculate the half-size vector of the textured quad
    vec3 half_size_right = vec3_multiply_scalar(right, player->height / 2);
    vec3 half_size_up = vec3_multiply_scalar(up, player->height / 2);

    // Calculate the four corner vertices in the world space
    vec3 world_vertices[4];
    world_vertices[2] = vec3_subtract(vec3_add(player->position, half_size_right), half_size_up);
    world_vertices[3] = vec3_subtract(vec3_subtract(player->position, half_size_right), half_size_up);
    world_vertices[0] = vec3_add(vec3_subtract(player->position, half_size_right), half_size_up);
    world_vertices[1] = vec3_add(vec3_add(player->position, half_size_right), half_size_up);

    // Render the textured quad
    glBindTexture(GL_TEXTURE_2D, texture);
    glBegin(GL_QUADS);

    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(world_vertices[0].x, world_vertices[0].y, world_vertices[0].z);

    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(world_vertices[1].x, world_vertices[1].y, world_vertices[1].z);

    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(world_vertices[2].x, world_vertices[2].y, world_vertices[2].z);

    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(world_vertices[3].x, world_vertices[3].y, world_vertices[3].z);

    glEnd();
}

void render_players(Player* players, int current_player, int players_count, GLuint texture) {
    for (int i = 0; i < players_count; i++) {
        if (i == current_player) {
            continue;
        }
        if (players[i].death_timer <= 0.0f) {
            render_player_texture(&players[i], texture);
        }
    }
}

void render_world(World* world, Player* player, GLuint test_texture) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90.0, (double)get_setting_int("screen_width") / (double)get_setting_int("screen_height"), 0.01, 500.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(player->position.x, player->position.y, player->position.z,
            player->position.x + cosf(player->yaw), player->position.y + sinf(player->yaw), player->position.z - sinf(player->pitch),
            0.0f, 0.0f, -1.0f);

    Direction neighbor_dirs[] = {DIR_EAST, DIR_WEST, DIR_SOUTH, DIR_NORTH};

    if (test_texture != 0) {
        render_face(-4, -4, 0, CELL_XY_SCALE, CELL_XY_SCALE, DIR_UP, test_texture);
        render_face(-4, -4, 0, CELL_XY_SCALE, CELL_XY_SCALE, DIR_DOWN, test_texture);
        render_face(-4, -4, 0, CELL_XY_SCALE, CELL_Z_SCALE, DIR_NORTH, test_texture);
        render_face(-4, -4, 0, CELL_XY_SCALE, CELL_Z_SCALE, DIR_WEST, test_texture);
        render_face(-4, -4, 0, CELL_XY_SCALE, CELL_Z_SCALE, DIR_EAST, test_texture);
    }

    for (int z = 0; z < world->num_layers; z++) {
        Layer* layer = &world->layers[z];
        for (int y = 0; y < layer->height; ++y) {
            for (int x = 0; x < layer->width; ++x) {
                Cell* cell = &layer->cells[y][x];
                Cell* neighbors[4] = {
                    get_cell(layer, x + 1, y + 0),
                    get_cell(layer, x -1, y + 0),
                    get_cell(layer, x + 0, y + 1),
                    get_cell(layer, x + 0, y - 1)
                };
                TextureInfo* cell_texture_info = get_texture_info(cell->color);

                // Render floors
                if (cell->type != CELL_VOID) {
                    render_face(x * CELL_XY_SCALE, y * CELL_XY_SCALE, z * CELL_Z_SCALE, CELL_XY_SCALE, CELL_XY_SCALE, DIR_DOWN, cell_texture_info->floor_texture);
                }

                // Render ceilings
                if (cell->type == CELL_SOLID || cell->type == CELL_ROOM) {
                    render_face(x * CELL_XY_SCALE, y * CELL_XY_SCALE, z * CELL_Z_SCALE, CELL_XY_SCALE, CELL_XY_SCALE, DIR_UP, cell_texture_info->ceiling_texture);
                }
                
                for (int i = 0; i < 4; ++i) {
                    Cell* neighbor = neighbors[i];
                    // Get texture info for the neighboring cell
                    TextureInfo* neighbor_texture_info = neighbor ? get_texture_info(neighbor->color) : NULL;

                    if (cell->type != CELL_SOLID && neighbor != NULL && neighbor->type == CELL_SOLID) {
                        if (neighbor && !neighbor_texture_info) {
                            printf("Failed to find texture for color: r=%d, g=%d, b=%d\n",
                                neighbor->color.r, neighbor->color.g, neighbor->color.b);
                            continue;
                        }
                        //Render walls for adjacent solid blocks
                        render_face(x * CELL_XY_SCALE, y * CELL_XY_SCALE, z * CELL_Z_SCALE, CELL_XY_SCALE, CELL_Z_SCALE, neighbor_dirs[i], neighbor_texture_info->wall_texture);

                    } else if (cell->type == CELL_SOLID) {
                        //Render walls at the world edge
                        if (neighbor == NULL) {
                            GLuint wall_texture = cell_texture_info->wall_texture;
                            render_face(x * CELL_XY_SCALE, y * CELL_XY_SCALE, z * CELL_Z_SCALE, CELL_XY_SCALE, CELL_Z_SCALE, neighbor_dirs[i], wall_texture);
                        }
                    }
                }
            }
        }
    }
}

void render_projectile(Projectile* projectile, GLuint texture) {
    glPushMatrix();
    glTranslatef(projectile->position.x, projectile->position.y, projectile->position.z);

    glBindTexture(GL_TEXTURE_2D, texture);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-projectile->size / 2, -projectile->size / 2, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( projectile->size / 2, -projectile->size / 2, 0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( projectile->size / 2,  projectile->size / 2, 0.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-projectile->size / 2,  projectile->size / 2, 0.0f);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glPopMatrix();
}

void render_projectiles(GameState* game_state, GLuint projectile_texture) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        Projectile* projectile = &game_state->projectiles[i];
        if (projectile->ttl > 0) {
            render_projectile(projectile, projectile_texture);
        }
    }
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
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);

    SDL_FreeSurface(surface);

    return texture;
}
