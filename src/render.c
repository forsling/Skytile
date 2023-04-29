#include <stdio.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "render.h"
#include "world.h"
#include "settings.h"

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

void render_world(World *world, Player *player) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90.0, (double)get_setting_int("screen_width") / (double)get_setting_int("screen_height"), 0.01, 500.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(player->position.x, player->position.y, player->position.z,
            player->position.x + cosf(player->yaw), player->position.y + sinf(player->yaw), player->position.z - sinf(player->pitch),
            0.0f, 0.0f, -1.0f);

    Direction neighbor_dirs[] = {DIR_EAST, DIR_WEST, DIR_SOUTH, DIR_NORTH};

    bool render_reference_block = true;
    if (render_reference_block) {

        render_face(-4, -4, 0, CELL_XY_SCALE, CELL_XY_SCALE, DIR_UP, 1);
        render_face(-4, -4, 0, CELL_XY_SCALE, CELL_XY_SCALE, DIR_DOWN, 2);
        render_face(-4, -4, 0, CELL_XY_SCALE, CELL_Z_SCALE, DIR_NORTH, 1);
        render_face(-4, -4, 0, CELL_XY_SCALE, CELL_Z_SCALE, DIR_WEST, 1);
        render_face(-4, -4, 0, CELL_XY_SCALE, CELL_Z_SCALE, DIR_EAST, 1);
    }

    for (int z = 0; z < world->num_layers; z++) {
        Layer *layer = &world->layers[z];
        for (int y = 0; y < layer->height; ++y) {
            for (int x = 0; x < layer->width; ++x) {
                Cell *cell = &layer->cells[y][x];
                Cell *neighbors[4] = {
                    get_cell(layer, x + 1, y + 0),
                    get_cell(layer, x -1, y + 0),
                    get_cell(layer, x + 0, y + 1),
                    get_cell(layer, x + 0, y - 1)
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

// Function to create a GLuint texture from a sub-region of the given SDL_Surface
GLuint create_texture(SDL_Surface* image, int x, int y, int width, int height) {
    if (!image) {
        printf("Error: Invalid SDL_Surface\n");
        return 0;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    GLenum format = (image->format->BytesPerPixel == 4) ? GL_RGBA : GL_RGB;

    // Create a new SDL_Surface for the sub-region
    SDL_Surface* subImage = SDL_CreateRGBSurface(0, width, height, image->format->BitsPerPixel,
                                                 image->format->Rmask, image->format->Gmask,
                                                 image->format->Bmask, image->format->Amask);

    // Copy the sub-region to the new SDL_Surface
    SDL_Rect srcRect = {x, y, width, height};
    SDL_BlitSurface(image, &srcRect, subImage, NULL);

    // Create the texture from the sub-region surface
    glTexImage2D(GL_TEXTURE_2D, 0, format, subImage->w, subImage->h, 0, format, GL_UNSIGNED_BYTE, subImage->pixels);

    glBindTexture(GL_TEXTURE_2D, 0);
    SDL_FreeSurface(subImage);

    return texture;
}

Uint32 get_pixel32(SDL_Surface *surface, int x, int y) {
    int bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch (bpp) {
        case 1:
            return *p;
        case 2:
            return *(Uint16 *)p;
        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                return p[0] << 16 | p[1] << 8 | p[2];
            } else {
                return p[0] | p[1] << 8 | p[2] << 16;
            }
        case 4:
            return *(Uint32 *)p;
        default:
            return 0;
    }
}

void render_projectile(Projectile *projectile, GLuint texture) {
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


SDL_Surface* load_surface(const char *filename) {
    SDL_Surface *image = IMG_Load(filename);
    if (!image) {
        printf("Error: %s\n", IMG_GetError());
        return NULL;
    }
    return image;
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
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);

    SDL_FreeSurface(surface);

    return texture;
}
