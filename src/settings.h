#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdbool.h>

typedef enum {
    SETTING_TYPE_INT,
    SETTING_TYPE_FLOAT,
    SETTING_TYPE_STRING
} SettingType;

typedef struct {
    SettingType type;
    union {
        int int_value;
        float float_value;
        char *string_value;
    } value;
} Setting;

bool load_settings(const char *file_name);
void add_setting(const char *key, SettingType type, const char *value);

const char *get_setting_string(const char *key);
int get_setting_int(const char *key);
float get_setting_float(const char *key);

#endif // SETTINGS_H