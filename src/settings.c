#include "settings.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SETTINGS 128
#define MAX_LINE_LENGTH 256

typedef struct {
    char key[MAX_LINE_LENGTH];
    Setting setting;
} KeyValuePair;

static KeyValuePair settings_array[MAX_SETTINGS];
static size_t settings_count = 0;

static Setting *find_setting(const char *key) {
    for (size_t i = 0; i < settings_count; ++i) {
        if (strcmp(settings_array[i].key, key) == 0) {
            return &settings_array[i].setting;
        }
    }
    return NULL;
}

bool load_settings(const char *file_name) {
    initialize_default_settings();

    FILE *file = fopen(file_name, "r");
    if (!file) {
        if (errno == ENOENT) {
            fprintf(stderr, "Warning: Settings file not found, creating with default values: %s\n", file_name);
            write_settings(file_name);
            return true;
        } else {
            fprintf(stderr, "Error: Could not open settings file: %s\n", file_name);
            return false;
        }
    }

    // Load settings from the file
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char key[128];
        char type_str[16];
        char value_str[128];

        if (sscanf(line, "%[^:]:%[^=]=%s", key, type_str, value_str) == 3) {
            SettingType type;
            if (strcmp(type_str, "int") == 0) {
                type = SETTING_TYPE_INT;
            } else if (strcmp(type_str, "float") == 0) {
                type = SETTING_TYPE_FLOAT;
            } else if (strcmp(type_str, "string") == 0) {
                type = SETTING_TYPE_STRING;
            } else if (strcmp(type_str, "bool") == 0) {
                type = SETTING_TYPE_BOOL;
            } else {
                fprintf(stderr, "Error: Invalid setting type in settings file: %s\n", type_str);
                continue;
            }

            set_setting(key, type, value_str);
        } else {
            fprintf(stderr, "Error: Invalid setting format in settings file: %s\n", line);
        }
    }

    fclose(file);
    return true;
}

void write_settings(const char *file_name) {
    FILE *file = fopen(file_name, "w");
    if (!file) {
        fprintf(stderr, "Error: Could not open settings file for writing: %s\n", file_name);
        return;
    }

    for (int i = 0; i < settings_count; ++i) {
        const char *type_str;
        const char *key = settings_array[i].key;
        Setting *setting = &settings_array[i].setting;
        switch (setting->type) {
            case SETTING_TYPE_INT:
                type_str = "int";
                fprintf(file, "%s:%s=%d\n", key, type_str, setting->value.int_value);
                break;
            case SETTING_TYPE_FLOAT:
                type_str = "float";
                fprintf(file, "%s:%s=%.2f\n", key, type_str, setting->value.float_value);
                break;
            case SETTING_TYPE_STRING:
                type_str = "string";
                fprintf(file, "%s:%s=%s\n", key, type_str, setting->value.string_value);
                break;
            case SETTING_TYPE_BOOL:
                type_str = "bool";
                fprintf(file, "%s:%s=%s\n", key, type_str, setting->value.int_value ? "true" : "false");
                break;
            default:
                fprintf(stderr, "Error: Invalid setting type: %d\n", setting->type);
                continue;
        }        
    }

    fclose(file);
}

void initialize_default_settings() {
    set_setting("screen_width", SETTING_TYPE_INT, "1280");
    set_setting("screen_height", SETTING_TYPE_INT, "720");
    set_setting("current_level", SETTING_TYPE_INT, "1");
    set_setting("gravity", SETTING_TYPE_FLOAT, "15.0f");
    set_setting("free_mode", SETTING_TYPE_BOOL, "false");
    set_setting("player_pos_x", SETTING_TYPE_FLOAT, "5.0f");
    set_setting("player_pos_y", SETTING_TYPE_FLOAT, "5.0f");
    set_setting("player_pos_z", SETTING_TYPE_FLOAT, "-2.0f");
}

void set_setting(const char *key, SettingType type, const char *value_str) {
    // Try to find the setting
    Setting *setting = find_setting(key);

    // If the setting doesn't exist, add it
    if (!setting) {
        if (settings_count >= MAX_SETTINGS) {
            return;
        }

        KeyValuePair *kv = &settings_array[settings_count++];
        strncpy(kv->key, key, MAX_LINE_LENGTH);
        kv->setting.type = type;
        setting = &kv->setting;
    } else if (setting->type != type) {
        // If the setting exists but has a different type, report an error and return
        fprintf(stderr, "Error: Setting type mismatch for key %s\n", key);
        return;
    }

    // Update the value based on the type
    switch (type) {
        case SETTING_TYPE_INT:
            setting->value.int_value = atoi(value_str);
            break;
        case SETTING_TYPE_FLOAT:
            setting->value.float_value = atof(value_str);
            break;
        case SETTING_TYPE_STRING:
            free(setting->value.string_value);
            setting->value.string_value = strdup(value_str);
            break;
        case SETTING_TYPE_BOOL:
            setting->value.int_value = strcmp(value_str, "true") == 0 ? 1 : 0;
            break;
    }
}

int get_setting_int(const char *key) {
    Setting *setting = find_setting(key);
    if (setting && setting->type == SETTING_TYPE_INT) {
        return setting->value.int_value;
    }
    char * message = "Error: Could not find an integer setting with key:";
    char full_message[128];
    snprintf(full_message, sizeof(full_message), "%s %s\n", message, key);
    fprintf(stderr, full_message);
    return 0;
}

float get_setting_float(const char *key) {
    Setting *setting = find_setting(key);
    if (setting && setting->type == SETTING_TYPE_FLOAT) {
        return setting->value.float_value;
    }
    char * message = "Error: Could not find a float setting with key:";
    char full_message[128];
    snprintf(full_message, sizeof(full_message), "%s %s\n", message, key);
    fprintf(stderr, full_message);
    return 0.0f;
}

const char *get_setting_string(const char *key) {
    Setting *setting = find_setting(key);
    if (setting && setting->type == SETTING_TYPE_STRING) {
        return setting->value.string_value;
    }
    char * message = "Error: Could not find a string setting with key:";
    char full_message[128];
    snprintf(full_message, sizeof(full_message), "%s %s\n", message, key);
    fprintf(stderr, full_message);
    return NULL;
}

bool get_setting_bool(const char *key) {
    Setting *setting = find_setting(key);
    if (setting && setting->type == SETTING_TYPE_BOOL) {
        return setting->value.int_value != 0;
    }
    char * message = "Error: Could not find a boolean setting with key:";
    char full_message[128];
    snprintf(full_message, sizeof(full_message), "%s %s\n", message, key);
    fprintf(stderr, full_message);
    return false;
}
