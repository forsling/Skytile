// settings.c
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "settings.h"

setting *settings = NULL;
size_t settings_count = 0;

// settings.c
bool load_settings(const char *file_name) {
    FILE *file = fopen(file_name, "r");
    if (!file) {
        fprintf(stderr, "Failed to open settings file: %s\n", file_name);
        return false;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Remove newline character
        line[strcspn(line, "\n")] = '\0';

        // Find the equal sign
        char *equal_sign = strchr(line, '=');
        if (!equal_sign) {
            continue;
        }

        // Split the line into key and value
        *equal_sign = '\0';
        char *key = line;
        char *value = equal_sign + 1;

        // Add the setting to the dynamic array
        settings = realloc(settings, sizeof(setting) * (settings_count + 1));
        settings[settings_count].key = strdup(key);
        settings[settings_count].value = strdup(value);
        settings_count++;
    }

    fclose(file);
    return true;
}

const char *get_setting_string(const char *key) {
    for (size_t i = 0; i < settings_count; ++i) {
        if (strcmp(settings[i].key, key) == 0) {
            return settings[i].value;
        }
    }
    return NULL;
}

int get_setting_int(const char *key) {
    const char *value_str = get_setting_string(key);
    return value_str ? atoi(value_str) : 0;
}

float get_setting_float(const char *key) {
    const char *value_str = get_setting_string(key);
    return value_str ? atof(value_str) : 0.0f;
}

void unload_settings() {
    free(settings);
    settings = NULL;
    settings_count = 0;
}
