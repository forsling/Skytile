#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <dirent.h>
#include <math.h>

#include "vector.h"
#include "render.h"
#include "world.h"
#include "utils.h"

Cell* cell_definitions;
int num_definitions;
SDL_Surface* base_bg_texture;
Cell default_cell;

const int CELL_XY_SCALE = 2;
const int CELL_Z_SCALE = 4;

bool load_world(World* world, const char* level_name) {
    DIR* dir;
    struct dirent* entry;
    int layer_count = 0;
    char leveldir[20];
    snprintf(leveldir, sizeof(leveldir), "levels/%s", level_name);
    dir = opendir(leveldir);
    if (dir == NULL) {
        printf("Failed to open level directory.\n");
        return false;
    }

    // Count the number of layer files
    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".bmp") != NULL) {
            layer_count++;
        }
    }

    base_bg_texture = load_surface("assets/bg.png");

    // Allocate memory for layers
    world->num_layers = layer_count;
    world->layers = malloc(layer_count * sizeof(Layer));

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

    // Load each layer
    int layer_index = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".bmp") != NULL) {
            char layer_path[256];
            snprintf(layer_path, sizeof(layer_path), "%s/%s", leveldir, entry->d_name);
            SDL_Surface* layer_surface = SDL_LoadBMP(layer_path);
            if (layer_surface == NULL) {
                printf("Failed to load layer %s at %s \n", entry->d_name, layer_path);
                continue;
            }

            parse_layer_from_surface(layer_surface, &world->layers[layer_index]);
            SDL_FreeSurface(layer_surface);
            layer_index++;
        }
    }

    closedir(dir);
    return true;
}

void free_world(World* world) {
    for (int i = 0; i < world->num_layers; ++i) {
        Layer* layer = &world->layers[i];
        for (int y = 0; y < layer->height; ++y) {
            free(layer->cells[y]);
        }
        free(layer->cells);
    }
    free(world->layers);
    world->layers = NULL;
    world->num_layers = 0;

    free(cell_definitions);
    cell_definitions = NULL;

    SDL_FreeSurface(base_bg_texture);
    base_bg_texture = NULL;
}

void parse_layer_from_surface(SDL_Surface* surface, Layer* layer) {
    layer->width = surface->w;
    layer->height = surface->h;
    layer->cells = malloc(layer->height * sizeof(Cell*));

    for (int y = 0; y < layer->height; ++y) {
        layer->cells[y] = malloc(layer->width * sizeof(Cell));
        for (int x = 0; x < layer->width; ++x) {
            Uint8 r, g, b;
            SDL_GetRGB(get_pixel32(surface, x, y), surface->format, &r, &g, &b);
            SDL_Color color = {r, g, b, 255};
       
            layer->cells[y][x] = *get_cell_definition_from_color(color, cell_definitions, num_definitions);
        }
    }
}

Cell* get_cell_definition_from_color(SDL_Color color, Cell* definitions, int num_definitions) {
    for (int i = 0; i < num_definitions; i++) {
        Cell* def = &definitions[i];
        if (color.r == def->color.r && color.g == def->color.g && color.b == def->color.b) {
            return def;
        }
    }

    return &default_cell;
}

Cell* read_cell_definitions(const char* filename, int* num_definitions) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Cannot open cell definitions file: %s\n", filename);
        return NULL;
    }

    int capacity = 10;
    Cell* definitions = (Cell *)malloc(capacity * sizeof(Cell));
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
        Cell* def = &definitions[*num_definitions];
        if (parse_cell_definition(line, def)) {
            // Increment the index only if the line was processed successfully
            (*num_definitions)++;
        }
    }

    fclose(file);
    return definitions;
}

int parse_cell_definition(const char* line, Cell* def) {
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

bool is_out_of_xy_bounds(Layer* layer, int x, int y) {
    return x < 0 || x >= layer->width || y < 0 || y >= layer->height;
}

bool is_within_xy_bounds(Layer* layer, int x, int y) {
    return x >= 0 && x < layer->width && y >= 0 && y < layer->height;
}

Cell* get_cell(Layer* layer, int x, int y) {
    if (is_out_of_xy_bounds(layer, x, y)) {
        return NULL;
    }
    return &layer->cells[y][x];
}

bool get_world_cell(World* world, ivec3 grid_position, Cell** out_cell) {
    if (grid_position.z < 0 || grid_position.z >= world->num_layers) {
        return false;
    }
    Layer* layer = &world->layers[grid_position.z];
    if (grid_position.y < 0 || grid_position.y >= layer->width || grid_position.x < 0 || grid_position.x >= layer->height) {
        return false;
    }
    *out_cell = &layer->cells[grid_position.y][grid_position.x];
    return true;
}
