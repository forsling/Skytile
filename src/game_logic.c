#include <stdio.h>
#include <assert.h>

#include "game_logic.h"
#include "game.h"
#include "vector.h"
#include "world.h"
#include "utils.h"
#include "settings.h"
#include "math.h"

#define MAX_CELLS 16

const float MOUSE_SENSITIVITY = 0.001f;

static vec2 process_input(Player* player, InputState* input_state, float delta_time);
static void process_mouse(Player* player, InputState* input_state);

static void calculate_projectile_direction(Player* player, vec3* direction);
static void create_projectile(Projectile* projectiles, Player* player);
static void update_projectile(World* world, Projectile* projectile, float deltaTime);

static void update_player_position(Player* player, World* world, float dx, float dy, float deltaTime);
static bool get_next_z_obstacle(World* world, int cell_x, int cell_y, float z_pos, float* out_obstacle_z);
static CellInfo* get_cells_for_vector(World* world, vec3 source, vec3 destination, int* num_cells);
static vec3 get_furthest_legal_position(World* world, vec3 source, vec3 destination, float collision_buffer);

void update(GameState* game_state, World* world, InputState* input_state, int player_index) {
    Player* player = &game_state->players[player_index];
    vec2 movement = process_input(player, input_state, game_state->delta_time);
    process_mouse(player, input_state);
        
    update_player_position(player, world, movement.x, movement.y, game_state->delta_time);

    // Update projectiles
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        update_projectile(world, &game_state->projectiles[i], game_state->delta_time);
    }

    if (input_state->mouse_button_1.is_down && !input_state->mouse_button_1.was_down) {
        create_projectile(game_state->projectiles, player);
    }
}


static vec2 process_input(Player* player, InputState* input_state, float delta_time) {
    vec2 movement = {0.0f, 0.0f};
    player->jumped = false;

    if (input_state->f.is_down && !input_state->f.was_down) {
        // Toggle free mode
        player->free_mode = !player->free_mode;
    }

    if (input_state->up.is_down) {
        movement.x += cosf(player->yaw);
        movement.y += sinf(player->yaw);
    }
    if (input_state->down.is_down) {
        movement.x -= cosf(player->yaw);
        movement.y -= sinf(player->yaw);
    }
    if (input_state->right.is_down) {
        movement.x -= sinf(player->yaw);
        movement.y += cosf(player->yaw);
    }
    if (input_state->left.is_down) {
        movement.x += sinf(player->yaw);
        movement.y -= cosf(player->yaw);
    }

    // Handle jumping and free mode
    if (input_state->space.is_down) {
        if (player->free_mode) {
            player->position.z -= player->speed * delta_time;
        } else if (player->velocity_z == 0.0f) { // Jump only when the player is on the ground
            player->velocity_z = player->jump_velocity;
            player->jumped = true;
        }
    }

    if (player->free_mode && input_state->shift.is_down) {
        player->position.z += player->speed * delta_time;
    }

    return movement;
}

static void process_mouse(Player* player, InputState* input_state) {
    // Update player's yaw and pitch based on mouse input
    player->yaw += input_state->mouse_state.dx * MOUSE_SENSITIVITY;
    player->pitch -= input_state->mouse_state.dy * MOUSE_SENSITIVITY;

    if (player->pitch < -M_PI / 2) {
        player->pitch = -M_PI / 2;
    }
    if (player->pitch > M_PI / 2) {
        player->pitch = M_PI / 2;
    }
}

static void create_projectile(Projectile* projectiles, Player* player) {
    // Create a new projectile
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (projectiles[i].ttl < 1) {
            Projectile* proj = &projectiles[i];
            proj->position = player->position;
            proj->speed = 20.0f;
            proj->size = 1.0f;
            proj->ttl = 1000;
            proj->active = true;
            calculate_projectile_direction(player, &proj->direction);
            break;
        }
    }
}

static void calculate_projectile_direction(Player* player, vec3* direction) {
    float forward_x = cosf(player->yaw) * cosf(player->pitch);
    float forward_y = sinf(player->yaw) * cosf(player->pitch);
    float forward_z = -sinf(player->pitch);

    direction->x = forward_x;
    direction->y = forward_y;
    direction->z = forward_z;
}

static void update_projectile(World* world, Projectile* projectile, float deltaTime) {
    if (projectile->ttl < 1) {
        return;
    }
    projectile->ttl--;
    if (!projectile->active) {
        return;
    }

    vec3 old_pos = {
        .x = projectile->position.x, 
        .y = projectile->position.y, 
        .z = projectile->position.z
    };
    vec3 new_pos = {
        .x = projectile->position.x + projectile->direction.x * projectile->speed * deltaTime,
        .y = projectile->position.y + projectile->direction.y * projectile->speed * deltaTime,
        .z = projectile->position.z + projectile->direction.z * projectile->speed * deltaTime
    };

    int num_cells;
    CellInfo* cell_infos = get_cells_for_vector(world, old_pos, new_pos, &num_cells);
    for (int i = 0; i < num_cells; i++) {
        CellInfo cell_info = cell_infos[i];
        Cell* cell = cell_info.cell;
        vec3 cell_position = cell_info.position;

        if (cell != NULL && cell->type == CELL_SOLID) {
            projectile->active = false;
            projectile->ttl = 100;
        }
    }
    projectile->position = new_pos;
}

static void update_player_position(Player* player, World* world, float dx, float dy, float deltaTime) {
    // Handle free mode unrestricted movement
    if (player->free_mode) {
        player->position.x += dx * player->speed * deltaTime;
        player->position.y += dy * player->speed * deltaTime;
        player->velocity_z = 0;
        debuglog(4, "x %f, y %f, z %f \n", player->position.x, player->position.y, player->position.z);
        return;
    }

    // Apply gravity
    float gravity = get_setting_float("gravity");
    player->velocity_z += gravity * deltaTime;

    // New player position (to be evaluated)
    float target_x = player->position.x + dx * player->speed * deltaTime;
    float target_y = player->position.y + dy * player->speed * deltaTime;
    float target_z = player->position.z + (player->velocity_z * deltaTime);
    ivec3 target_grid_pos = get_grid_pos3(target_x, target_y, target_z);

    int z_layer = (int)floor(player->position.z / CELL_Z_SCALE);
    Layer* layer = &world->layers[z_layer];

    // Calculate the destination position
    vec3 source = {player->position.x, player->position.y, player->position.z};
    vec3 destination = {target_x, target_y, target_z};

    // Update the player's position based on the furthest legal position
    if (z_layer >= 0) {
        vec3 furthest_legal_position = get_furthest_legal_position(world, source, destination, player->size);
        target_x = furthest_legal_position.x;
        target_y = furthest_legal_position.y;
    }

    // z-axis handling
    float next_z_obstacle;
    bool has_obstacle_down = get_next_z_obstacle(world, target_grid_pos.x, target_grid_pos.y, target_z, &next_z_obstacle);
    if (has_obstacle_down) {
        float highest_valid_z = next_z_obstacle - player->height;
        if (target_z > highest_valid_z) {
            // Player z movement is obstructed
            if (player->velocity_z >= 0) {
                target_z = highest_valid_z;
                player->velocity_z = 0.0f;
            } else {
                target_z = next_z_obstacle + 0.01f;
                player->velocity_z = 0.01f;
            }
        }
    }

    // Update player position if the target cell is not solid
    ivec3 newpos = get_grid_pos3(target_x, target_y, target_z);
    Cell* cell_candidate;
    bool got_cell = get_world_cell(world, newpos, &cell_candidate);
    if (!got_cell || cell_candidate->type != CELL_SOLID) {
        if (!(player->position.x == target_x && player->position.y == target_y && player->position.z == target_z)) {
            debuglog(1, "Player: %d,%d (%f, %f, %d) -> %d,%d (%f, %f, %d) \n", (int)(player->position.x / CELL_XY_SCALE), (int)(player->position.y / CELL_XY_SCALE), player->position.x, player->position.y, z_layer, target_grid_pos.x, target_grid_pos.y, target_x, target_y, (int)floor(target_z / CELL_Z_SCALE));
        }
        player->position.x = target_x;
        player->position.y = target_y;
        player->position.z = target_z;
    } else {
        debuglog(1, "Player: rejected: %d,%d (%f, %f, %d) -> %d,%d (%f, %f, %d) \n", (int)(player->position.x / CELL_XY_SCALE), (int)(player->position.y / CELL_XY_SCALE), player->position.x, player->position.y, z_layer, target_grid_pos.x, target_grid_pos.y, target_x, target_y, (int)floor(target_z / CELL_Z_SCALE));
        ivec3 old_grid_pos = get_grid_pos3(player->position.x, player->position.y, player->position.z);
        Cell* cell_candidate;
        bool got_cell = get_world_cell(world, old_grid_pos, &cell_candidate);
        if (got_cell && cell_candidate->type == CELL_SOLID) {
            player->position.z -= CELL_Z_SCALE;
        }
    }
}

static vec3 get_furthest_legal_position(World* world, vec3 source, vec3 destination, float collision_buffer) {
    int num_cells;
    CellInfo* cell_infos = get_cells_for_vector(world, source, destination, &num_cells);
    vec3 movement_vector = vec3_subtract(destination, source);
    float movement_length = vec3_length(movement_vector);
    vec3 movement_unit_vector = vec3_normalize(movement_vector);

    for (float distance = movement_length; distance >= 0.0f; distance -= collision_buffer) {
        vec3 candidate_position = vec3_add(source, vec3_multiply_scalar(movement_unit_vector, distance));
        bool is_valid = true;

        for (int i = 0; i < num_cells; i++) {
            CellInfo cell_info = cell_infos[i];
            Cell* cell = cell_info.cell;
            vec3 cell_position = cell_info.position;

            if (cell != NULL && cell->type == CELL_SOLID) {
                float distance_to_cell = point_to_aabb_distance_3d(candidate_position.x, candidate_position.y, candidate_position.z,
                                                                cell_position.x, cell_position.y, cell_position.z,
                                                                cell_position.x + CELL_XY_SCALE, cell_position.y + CELL_XY_SCALE, cell_position.z + CELL_Z_SCALE);
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

static CellInfo* get_cells_for_vector(World* world, vec3 source, vec3 destination, int* num_cells) {
    assert(num_cells != NULL);

    // Allocate memory for the cell information array
    static CellInfo cell_infos[MAX_CELLS];
    *num_cells = 0;

    // Convert source and destination to cell coordinates
    int x0 = (int)(source.x / CELL_XY_SCALE);
    int y0 = (int)(source.y / CELL_XY_SCALE);
    int z0 = (int)(source.z / CELL_Z_SCALE);
    int x1 = (int)(destination.x / CELL_XY_SCALE);
    int y1 = (int)(destination.y / CELL_XY_SCALE);
    int z1 = (int)(destination.z / CELL_Z_SCALE);

    // Bresenham's line algorithm in 3D
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int dz = abs(z1 - z0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int sz = (z0 < z1) ? 1 : -1;

    int dm = MAX(dx, MAX(dy, dz));
    int i;
    for (i = dm * 2; i > 0; i--) {
        // Check if the cell is within the world bounds
        if (x0 >= 0 && x0 < world->layers[0].width &&
            y0 >= 0 && y0 < world->layers[0].height &&
            z0 >= 0 && z0 < world->num_layers) {
            Cell* cell;
            if (get_world_cell(world, (ivec3){x0, y0, z0}, &cell)) {
                // Add cell information to the array
                cell_infos[*num_cells].cell = cell;
                cell_infos[*num_cells].position = (vec3){x0 * CELL_XY_SCALE, y0 * CELL_XY_SCALE, z0 * CELL_Z_SCALE};
                (*num_cells)++;
            }
        }

        if (x0 == x1 && y0 == y1 && z0 == z1) {
            break;
        }

        int x_err = 2 * abs(dm - dx);
        int y_err = 2 * abs(dm - dy);
        int z_err = 2 * abs(dm - dz);

        if (x_err <= dm) {
            dm -= dx;
            x0 += sx;
        }
        if (y_err <= dm) {
            dm -= dy;
            y0 += sy;
        }
        if (z_err <= dm) {
            dm -= dz;
            z0 += sz;
        }
    }

    return cell_infos;
}

static bool get_next_z_obstacle(World* world, int cell_x, int cell_y, float z_pos, float* out_obstacle_z) {
    int z_layer = (int)(z_pos / CELL_Z_SCALE);
    if (z_layer >= world->num_layers) {
        return false;
    }

    int first_check_layer = z_layer >= 0 ? z_layer : 0; 

    for (int i = first_check_layer; i < world->num_layers; i++) {
        Layer* layer = &world->layers[i];
        if (!is_within_xy_bounds(layer, cell_x, cell_y)) {
            continue;
        }
        Cell* cell = get_cell(layer, cell_x, cell_y);

        //Check ceiling if they are below player
        if (z_pos < (float)i * CELL_Z_SCALE) {
            if (cell->type == CELL_ROOM || cell->type == CELL_SOLID) {
                *out_obstacle_z = (float)i * CELL_Z_SCALE;
                //debuglog(1, "(zl %d) Found ceiling obstacle at %.2f (gridx: %d gridy: %d zlayer: %d z: %f) \n", i, *out_obstacle_z, cell_x, cell_y, z_layer, z_pos);
                return true;
            }
        }

        //Check floors
        if (cell->type != CELL_VOID) {
            *out_obstacle_z = (float)i * CELL_Z_SCALE + 4;
            //debuglog(1, "(zl %d) Found floor obstacle at %.2f (gridx: %d gridy: %d zlayer: %d z: %f) \n", i, *out_obstacle_z, cell_x, cell_y, z_layer, z_pos);
            return true;
        }
    }

    return false; // No obstacle found
}
