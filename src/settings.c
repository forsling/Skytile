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
            } else {
                fprintf(stderr, "Error: Invalid setting type in settings file: %s\n", type_str);
                continue;
            }

            add_setting(key, type, value_str);
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
            default:
                fprintf(stderr, "Error: Invalid setting type: %d\n", setting->type);
                continue;
        }        
    }

    fclose(file);
}

void initialize_default_settings() {
    add_setting("screen_width", SETTING_TYPE_INT, "1280");
    add_setting("screen_height", SETTING_TYPE_INT, "720");
    add_setting("current_level", SETTING_TYPE_INT, "1");
    add_setting("gravity", SETTING_TYPE_FLOAT, "15.0f");
}

void add_setting(const char *key, SettingType type, const char *value_str) {
    if (settings_count >= MAX_SETTINGS) {
        return;
    }

    KeyValuePair *kv = &settings_array[settings_count++];
    strncpy(kv->key, key, MAX_LINE_LENGTH);
    kv->setting.type = type;

    switch (type) {
        case SETTING_TYPE_INT:
            kv->setting.value.int_value = atoi(value_str);
            break;
        case SETTING_TYPE_FLOAT:
            kv->setting.value.float_value = atof(value_str);
            break;
        case SETTING_TYPE_STRING:
            kv->setting.value.string_value = strdup(value_str);
            break;
    }
}

const char *get_setting_string(const char *key) {
    for (size_t i = 0; i < settings_count; ++i) {
        if (strcmp(settings_array[i].key, key) == 0 && settings_array[i].setting.type == SETTING_TYPE_STRING) {
            return settings_array[i].setting.value.string_value;
        }
    }
    char * message = "Failed to get setting string for key:";
    char full_message[128];
    snprintf(full_message, sizeof(full_message), "%s %s\n", message, key);
    fprintf(stderr, full_message);
    return NULL;
}

int get_setting_int(const char *key) {
    for (size_t i = 0; i < settings_count; ++i) {
        if (strcmp(settings_array[i].key, key) == 0 && settings_array[i].setting.type == SETTING_TYPE_INT) {
            return settings_array[i].setting.value.int_value;
        }
    }

    char * message = "Failed to get setting int for key:";
    char full_message[128];
    snprintf(full_message, sizeof(full_message), "%s %s\n", message, key);
    fprintf(stderr, full_message);
    return 0;
}

float get_setting_float(const char *key) {
    for (size_t i = 0; i < settings_count; ++i) {
        if (strcmp(settings_array[i].key, key) == 0 && settings_array[i].setting.type == SETTING_TYPE_FLOAT) {
            return settings_array[i].setting.value.float_value;
        }
    }

    char * message = "Failed to get setting float for key:";
    char full_message[128];
    snprintf(full_message, sizeof(full_message), "%s %s\n", message, key);
    fprintf(stderr, full_message);
    return 0.0f;
}
