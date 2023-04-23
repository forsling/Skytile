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
    FILE *file = fopen(file_name, "r");
    if (file == NULL) {
        perror("Error opening settings file");
        return false;
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, MAX_LINE_LENGTH, file) != NULL) {
        char key[MAX_LINE_LENGTH];
        char type_str[MAX_LINE_LENGTH];
        char value[MAX_LINE_LENGTH];

        sscanf(line, "%[^:]:%[^=]=%s", key, type_str, value);

        SettingType type;
        if (strcmp(type_str, "int") == 0) {
            type = SETTING_TYPE_INT;
        } else if (strcmp(type_str, "float") == 0) {
            type = SETTING_TYPE_FLOAT;
        } else if (strcmp(type_str, "string") == 0) {
            type = SETTING_TYPE_STRING;
        } else {
            continue;
        }

        add_setting(key, type, value);
    }

    fclose(file);
    initialize_default_settings();
    return true;
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
