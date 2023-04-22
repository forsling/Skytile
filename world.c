#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "world.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <dirent.h>
#include "vector.h"
#include <assert.h>
#include "utils.h"
#include <math.h>

Cell *cell_definitions;
int num_definitions;
SDL_Surface* base_bg_texture;
Cell default_cell;

const int CELL_XY_SCALE = 2;
const int CELL_Z_SCALE = 4;

bool load_world(World* world) {
    DIR* dir;
    struct dirent* entry;
    int level_count = 0;

    dir = opendir("levels");
    if (dir == NULL) {
        printf("Failed to open levels directory.\n");
        return false;
    }

    // Count the number of level files
    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, "level-") != NULL && strstr(entry->d_name, ".bmp") != NULL) {
            level_count++;
        }
    }

    base_bg_texture = load_surface("assets/bg.png");

    // Allocate memory for levels
    world->num_levels = level_count;
    world->levels = malloc(level_count * sizeof(Level));

    // Reset directory position
    rewinddir(dir);

    default_cell.type = CELL_OPEN;
    SDL_Color cyan = {0, 255, 255};
    default_cell.color = cyan;
    default_cell.floor_texture = 0;
    default_cell.ceiling_texture = 0;
    default_cell.wall_texture = 0;

    // Load cell definitions
    num_definitions = 0;
    cell_definitions = read_cell_definitions("cell_definitions.txt", &num_definitions);

    // Load each level
    int level_index = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, "level-") != NULL && strstr(entry->d_name, ".bmp") != NULL) {
            char level_path[256];
            snprintf(level_path, sizeof(level_path), "levels/%s", entry->d_name);

            SDL_Surface* level_surface = SDL_LoadBMP(level_path);
            if (level_surface == NULL) {
                printf("Failed to load level %s\n", entry->d_name);
                continue;
            }

            parse_level_from_surface(level_surface, &world->levels[level_index]);
            SDL_FreeSurface(level_surface);
            level_index++;
        }
    }

    closedir(dir);
    return true;
}

void free_world(World* world) {
    for (int i = 0; i < world->num_levels; ++i) {
        Level* level = &world->levels[i];
        for (int y = 0; y < level->height; ++y) {
            free(level->cells[y]);
        }
        free(level->cells);
    }
    free(world->levels);
    world->levels = NULL;
    world->num_levels = 0;

    free(cell_definitions);
    cell_definitions = NULL;

    SDL_FreeSurface(base_bg_texture);
    base_bg_texture = NULL;
}

void parse_level_from_surface(SDL_Surface* surface, Level* level) {
    level->width = surface->w;
    level->height = surface->h;
    level->cells = malloc(level->height * sizeof(Cell*));

    for (int y = 0; y < level->height; ++y) {
        level->cells[y] = malloc(level->width * sizeof(Cell));
        for (int x = 0; x < level->width; ++x) {
            Uint8 r, g, b;
            SDL_GetRGB(get_pixel32(surface, x, y), surface->format, &r, &g, &b);
            SDL_Color color = {r, g, b, 255};
       
            level->cells[y][x] = *get_cell_definition_from_color(color, cell_definitions, num_definitions);
        }
    }
}

Cell* get_cell_definition_from_color(SDL_Color color, Cell *definitions, int num_definitions) {
    for (int i = 0; i < num_definitions; i++) {
        Cell *def = &definitions[i];
        if (color.r == def->color.r && color.g == def->color.g && color.b == def->color.b) {
            return def;
        }
    }

    return &default_cell;
}

Cell *read_cell_definitions(const char *filename, int *num_definitions) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Cannot open cell definitions file: %s\n", filename);
        return NULL;
    }

    int capacity = 10;
    Cell *definitions = (Cell *)malloc(capacity * sizeof(Cell));
    *num_definitions = 0;
    
    char line[256];
    while (fgets(line, sizeof(line), file) != NULL) {
        // Skip comment lines
        if (line[0] == '#') {
            continue;
        }

        // Check if we need to expand the capacity of the array
        if (*num_definitions >= capacity) {
            capacity *= 2;
            definitions = (Cell *)realloc(definitions, capacity * sizeof(Cell));
        }

        // Read and process the line
        Cell *def = &definitions[*num_definitions];
        if (parse_cell_definition(line, def)) {
            // Increment the index only if the line was processed successfully
            (*num_definitions)++;
        }
    }

    fclose(file);
    return definitions;
}

int parse_cell_definition(const char *line, Cell *def) {
    unsigned int r, g, b;
    char type_str[32];
    char c_str[32], f_str[32], w_str[32], name_str[64];
    int cx, cy, cw, ch, fx, fy, fw, fh, wx, wy, ww, wh;

    int num_parsed = sscanf(line, " %02X%02X%02X %31s %31s %31s %31s %255[^\n]",
                            &r, &g, &b,
                            type_str,
                            c_str,
                            f_str,
                            w_str,
                            name_str);

    if (num_parsed == 8) {
        def->color.r = (Uint8)r;
        def->color.g = (Uint8)g;
        def->color.b = (Uint8)b;

        if (strcmp(type_str, "SOLID") == 0) {
            def->type = CELL_SOLID;
        } else if (strcmp(type_str, "OPEN") == 0) {
            def->type = CELL_OPEN;
        } else {
            printf("Error: Invalid cell type: %s\n", type_str);
            return 0;
        }

        if (strcmp(c_str, "0") == 0) {
            def->ceiling_texture = 0;
        } else {
            sscanf(c_str, "%d,%d,%d,%d", &cx, &cy, &cw, &ch);
            def->ceiling_texture = create_texture(base_bg_texture, cx, cy, cw, ch);
        }

        if (strcmp(f_str, "0") == 0) {
            def->floor_texture = 0;
        } else {
            sscanf(f_str, "%d,%d,%d,%d", &fx, &fy, &fw, &fh);
            def->floor_texture = create_texture(base_bg_texture, fx, fy, fw, fh);
        }

        if (strcmp(w_str, "0") == 0) {
            def->wall_texture = 0;
        } else {
            sscanf(w_str, "%d,%d,%d,%d", &wx, &wy, &ww, &wh);
            def->wall_texture = create_texture(base_bg_texture, wx, wy, ww, wh);
        }

        printf("Loaded cell definition: %s\n", name_str);

        return 1;
    }

    printf("Error: Invalid cell definition line: %s\n", line);
    return 0;
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

SDL_Surface* load_surface(const char *filename) {
    SDL_Surface *image = IMG_Load(filename);
    if (!image) {
        printf("Error: %s\n", IMG_GetError());
        return NULL;
    }
    return image;
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

        // if (cell->type == CELL_SOLID) {
        //     *out_obstacle_z = (float)i * CELL_Z_SCALE;
        //     return true;
        // }

        //Check ceiling if they are below player
        if (z_pos < (float)i * CELL_Z_SCALE) {
            if (cell->ceiling_texture != 0 ||  (cell->type == CELL_SOLID)) {
                *out_obstacle_z = (float)i * CELL_Z_SCALE;
                //debuglog(1, "(zl %d) Found ceiling obstacle at %.2f (gridx: %d gridy: %d zlevel: %d z: %f) \n", i, *out_obstacle_z, cell_x, cell_y, z_level, z_pos);
                return true;
            }
        }

        //Check floors
        if (cell->floor_texture != 0 ||  (cell->type == CELL_SOLID)) {
            *out_obstacle_z = (float)i * CELL_Z_SCALE + 4;
            //debuglog(1, "(zl %d) Found floor obstacle at %.2f (gridx: %d gridy: %d zlevel: %d z: %f) \n", i, *out_obstacle_z, cell_x, cell_y, z_level, z_pos);
            return true;
        }
    }

    return false; // No obstacle found
}

CellInfo *get_cells_for_vector(Level *level, Vec2 source, Vec2 destination, int *num_cells) {
    assert(num_cells != NULL);

    // Allocate memory for the cell information array
    static CellInfo cell_infos[MAX_CELLS];
    *num_cells = 0;

    // Convert source and destination to cell coordinates
    int x0 = (int)(source.x / CELL_XY_SCALE);
    int y0 = (int)(source.y / CELL_XY_SCALE);
    int x1 = (int)(destination.x / CELL_XY_SCALE);
    int y1 = (int)(destination.y / CELL_XY_SCALE);

    // Bresenham's line algorithm
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2;
    int err2;

    while (1) {
        // Check if the cell is within the level bounds
        if (x0 >= 0 && x0 < level->width && y0 >= 0 && y0 < level->height) {
            Cell *cell = get_cell(level, x0, y0);
            if (cell != NULL) {
                // Add cell information to the array
                cell_infos[*num_cells].cell = cell;
                cell_infos[*num_cells].position = (Vec2){x0 * CELL_XY_SCALE, y0 * CELL_XY_SCALE};
                (*num_cells)++;
            }
        }

        if (x0 == x1 && y0 == y1) {
            break;
        }
        err2 = err;
        if (err2 > -dx) {
            err -= dy;
            x0 += sx;
        }
        if (err2 < dy) {
            err += dx;
            y0 += sy;
        }
    }

    return cell_infos;
}

Vec2 get_furthest_legal_position(Level *level, Vec2 source, Vec2 destination, float collision_buffer) {
    int num_cells;
    CellInfo *cell_infos = get_cells_for_vector(level, source, destination, &num_cells);

    Vec2 movement_vector = Vec2_subtract(destination, source);
    float movement_length = Vec2_length(movement_vector);
    Vec2 movement_unit_vector = Vec2_normalize(movement_vector);

    for (float distance = movement_length; distance >= 0.0f; distance -= collision_buffer) {
        Vec2 candidate_position = Vec2_add(source, Vec2_multiply_scalar(movement_unit_vector, distance));
        bool is_valid = true;

        for (int i = 0; i < num_cells; i++) {
            CellInfo cell_info = cell_infos[i];
            Cell *cell = cell_info.cell;
            Vec2 cell_position = cell_info.position;

            if (cell != NULL && cell->type == CELL_SOLID) {
                //debuglog(1, "Solid cell found at (%f, %f)\n", cell_info.position.x, cell_info.position.y);
                float distance_to_cell = point_to_aabb_distance(destination.x, destination.y, cell_position.x, cell_position.y, cell_position.x + CELL_XY_SCALE, cell_position.y + CELL_XY_SCALE);
                //debuglog(1, "Distance to solid cell: %f \n", distance_to_cell);
                if (distance_to_cell <= collision_buffer) {
                    is_valid = false;
                    break;
                }
            }
        }

        if (is_valid) {
            return candidate_position;
        }
    }

    return source;
}

ivec2 get_grid_pos2(float x, float y) {
    ivec2 gridpos = {
        .x = (int)(x / CELL_XY_SCALE),
        .y = (int)(y / CELL_XY_SCALE)
    };
    return gridpos;
}

ivec3 get_grid_pos3(float x, float y, float z) {
    ivec3 levelpos = {
        .x = (int)(x / CELL_XY_SCALE),
        .y = (int)(y / CELL_XY_SCALE),
        .z = (int)floor(z / CELL_Z_SCALE)
    };
    return levelpos;
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

bool get_world_cell(World *world, ivec3 grid_position, Cell** out_cell) {
    if (grid_position.z < 0 || grid_position.z >= world->num_levels) {
        return false;
    }
    Level *level = &world->levels[grid_position.z];
    if (grid_position.y >= level->width || grid_position.x >= level->height) {
        return false;
    }
    *out_cell = &level->cells[grid_position.y][grid_position.x];
    return true;
}